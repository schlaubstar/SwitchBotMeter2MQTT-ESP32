#include <WiFi.h>
#include <PubSubClient.h>
#include <NimBLEDevice.h>
#include <map>
#include <vector>
#include <string>
//OTA-Capabilities
#include <ElegantOTA.h>
#include <WebServer.h>
//Configuration
#include "config.h"  // Contains credentials, IPs, and knownDevices

// WiFi &  MQTT clients + WebServer for OTA-Updates
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WebServer server(80);

bool OTAStarted = false;  //avoids going to sleep if the request for an OTA Update was detected
unsigned long otaStartTime;
unsigned long ota_progress_millis = 0;

//Mapping with known SwitchBot-Meter Devices and the rooms where they are
std::map<std::string, std::string> knownDevices = {
  { "ce:21:31:41:51:61", "SleepingRoom" },
  { "dd:42:52:62:72:82", "Bathroom" },
  { "fb:e2:52:62:72:82", "RoomXYZ" },
  };

//----------------------------------------------------------------------------
// MQTT specific
//----------------------------------------------------------------------------
// Publish decoded metrics to individual MQTT topics
void publishToMQTT(const std::string& mac, float temp, int hum, int bat, int rssi) {
  std::string baseTopic = std::string(MQTT_PUBTOPIC) + "/" + mac;

  struct Metric {
    const char* suffix;
    std::string value;
  };

  char tempStr[8];
  snprintf(tempStr, sizeof(tempStr), "%.2f", temp);
  std::string tmpvalue = tempStr;

  Metric metrics[] = {
    { "/temperature", tmpvalue },
    { "/humidity", std::to_string(hum) },
    { "/battery", std::to_string(bat) },
    { "/rssi", std::to_string(rssi) }
  };

  if (DEBUG_ENABLED) {
    Serial.printf("[MQTT] Publishing metrics for device: %s\n", mac.c_str());
  }

  for (auto& m : metrics) {
    std::string topic = baseTopic + m.suffix;
    if (DEBUG_ENABLED) Serial.printf("[MQTT]   %s => %s\n", topic.c_str(), m.value.c_str());
    mqttClient.publish(topic.c_str(), m.value.c_str());
  }
  mqttClient.loop();
}

//handler of incoming MQTT Messages after a Subscription
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  String payloadStr = "";

  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  if (DEBUG_ENABLED) Serial.printf("[MQTT] Received the following MQTT Message: %s for topic %s\n", payloadStr.c_str(), topicStr.c_str());
  if (payloadStr == "true") {
    if (DEBUG_ENABLED) Serial.printf("[MQTT] Received a 'true' for OTA update ! > Starting Server\n");
    OTAStarted = true;
    startOTAServer();
    otaStartTime = millis();
    mqttClient.publish(topic, "false", false);  // true = retain message
  } else {
    if (DEBUG_ENABLED) Serial.printf("[MQTT] Received a 'false' for OTA update !\n");
  }
}

//----------------------------------------------------------------------------
// ElegantOTA specific
//----------------------------------------------------------------------------
void onOTAStart() {
  // Log when OTA has started
    if (DEBUG_ENABLED) Serial.println("[OTA ] OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
      if (DEBUG_ENABLED) Serial.printf("[OTA ] OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    if (DEBUG_ENABLED) Serial.println("[OTA ] OTA update finished successfully!");
  } else {
    if (DEBUG_ENABLED) Serial.println("[OTA ] There was an error during OTA update!");
  }
}

//Start the WebServer for OTA-Updates
void startOTAServer() {
  server.on("/status", HTTP_GET, []() {
    server.send(200, "text/html", "<h2>Device Status</h2><p>Firmware version: " + String(FW_VERSION) + "</p>");
  });
  ElegantOTA.begin(&server);  // OTA endpoint at /update
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  ElegantOTA.setAuth(OTA_USER, OTA_PWD);
  ElegantOTA.setAutoReboot(true);  //Auto-Rebootafter Update
  server.begin();
  if (DEBUG_ENABLED) Serial.println("[OTA ] OTA Webserver started");
}
//----------------------------------------------------------------------------
// BLE Scanning, Value Decoding etc.
//----------------------------------------------------------------------------


// BLE scan callback class
class MyScanCallbacks : public NimBLEScanCallbacks {

  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
    std::string mac = advertisedDevice->getAddress().toString();
    int rssi = advertisedDevice->getRSSI();

    if (DEBUG_ENABLED) {
      Serial.printf("-------------------------------------\n");
      Serial.printf("[BLE ] Got Scan Result of device: %s with RSSI %d\n", mac.c_str(), rssi);
    }

    //if you do not implement filterin on known MAC-addresses, please implement
    //additional logic to make sure, you found a SwitchBot device and the Data is suitable for parsing
    auto fd = knownDevices.find(mac);
    if (fd == knownDevices.end()) {
      if (DEBUG_ENABLED) Serial.println("[BLE ] -> Unknown device. Skipping.");
      return;
    }

    std::string serviceData = advertisedDevice->getServiceData();
    std::vector<uint8_t> rawBytes(serviceData.begin(), serviceData.end());
    
    if (rawBytes.size() < 6 || rawBytes[0] != 0x69) return;

    if (DEBUG_ENABLED) {
      Serial.printf("[SBOT] Device is in room: %s\n", fd->second.c_str());
      Serial.print("[SBOT] Raw ServiceData: ");
      for (auto b : rawBytes) Serial.printf("%02X ", b);
      Serial.println();
    }

    // Decode SwitchBot Meter payload
    uint8_t humidity = rawBytes[5] & 0b01111111;
    uint8_t battery = rawBytes[2] & 0b01111111;

    float temp = static_cast<int>(rawBytes[4] & 0b01111111) + (rawBytes[3] & 0b00001111) / 10.0f;

    if (!(rawBytes[4] & 0b10000000)) temp = -temp;  // Negative temperature
    if (rawBytes[5] & 0b10000000) {                 // Fahrenheit flag
      temp = std::round(((temp - 32.0f) * 5.0f / 9.0f) * 10.0f) / 10.0f;
    }
    //limit to two decimals
    temp = std::round(temp * 100.0f) / 100.0f;

    if (DEBUG_ENABLED) {
      Serial.printf("[SBOT] Decoded temperature: %.2f °C\n", temp);
      Serial.printf("[SBOT] Decoded humidity: %d %%rH\n", humidity);
      Serial.printf("[SBOT] Decoded battery: %d %%\n", battery);
      Serial.printf("-------------------------------------\n");
    }

    publishToMQTT(mac, temp, humidity, battery, rssi);
  }

  void onScanEnd(const NimBLEScanResults& results, int reason) override {
    if (DEBUG_ENABLED) Serial.printf("[BLE ] Scan ended. Reason: %d\n", reason);
    if (!OTAStarted) {
      goToSleep();
    } else {
      if (DEBUG_ENABLED) Serial.printf("[ESP ] Stying awake one Sleep-Cycle for an OTA-Update  !\n");
    }  //go to sleep if not the "OTA = true" signal was received
  }

    /** Initial discovery, advertisement data only. */
    void onDiscovered(const NimBLEAdvertisedDevice* advertisedDevice) override {
        if (DEBUG_ENABLED) printf("[BLE ] Discovered Device: %s\n", advertisedDevice->toString().c_str());
    }

};

static MyScanCallbacks scanCallbacks;

//----------------------------------------------------------------------------
// WiFi Setup Helper
//----------------------------------------------------------------------------
void connectToWiFi(bool autoReconnect = true, bool persistent = false) {
  WiFi.persistent(persistent);
  WiFi.setAutoReconnect(autoReconnect);
  WiFi.config(localIP, gatewayIP, subnetMask);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (DEBUG_ENABLED) Serial.print("[WiFi] Connecting to WiFi");

  for (int retries = 0; WiFi.status() != WL_CONNECTED && retries < 20; ++retries) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (DEBUG_ENABLED) {
      Serial.println("\n[WiFi] WiFi connected!");
      Serial.print("[WiFi] IP Address: ");
      Serial.println(WiFi.localIP());
    }
  } else {
    if (DEBUG_ENABLED) Serial.println("\n[WiFi] WiFi connection failed.");
  }
}

//----------------------------------------------------------------------------
// MQTT Connection Management
//----------------------------------------------------------------------------
void ensureMQTTConnection() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);  //attach handler for incoming MQTT Messages

  for (int retries = 0; !mqttClient.connected() && retries < 10; ++retries) {
    if (mqttClient.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS)) {
      mqttClient.subscribe(MQTT_SUBTOPIC);    //subscribe to the topic signalling an OTA Update
      if (DEBUG_ENABLED) Serial.println("[MQTT] Connected to MQTT broker.");
    } else {
      if (DEBUG_ENABLED) Serial.printf("[MQTT] error: %d\n", mqttClient.state());
      delay(1000);
    }
  }
}

// Put device into deep sleep
void goToSleep() {
  if (DEBUG_ENABLED) Serial.printf("[ESP ] Going to sleep !\n");
  mqttClient.disconnect();
  WiFi.disconnect(true);
  esp_bt_controller_disable(); 
  esp_sleep_enable_timer_wakeup(SLEEP_TIME * 60 * 1000000);  // Convert minutes to microseconds
  digitalWrite(LED_PIN, LOW);  // Turn LED off (device running)
  esp_deep_sleep_start();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (DEBUG_ENABLED) {
    Serial.begin(115200);
    delay(100);
  }

  // Indicate the device is up (solid LED during scan)
  digitalWrite(LED_PIN, HIGH);
  delay(10);

  // Connect WiFi & MQTT
  connectToWiFi(true, true);
  ensureMQTTConnection();

  mqttClient.loop();

  // Initialize BLE
  NimBLEDevice::setScanFilterMode(2); // Filter by address and data, advertisements
  NimBLEDevice::setScanDuplicateCacheSize(400);
  NimBLEDevice::init("");
  NimBLEScan* pScan = NimBLEDevice::getScan();

  // Attach the static callback instance (no heap allocation)
  pScan->setScanCallbacks(&scanCallbacks, false); // false = don't delete after use
  pScan->setInterval(320); // 320 * 0.625ms = 200 ms
  pScan->setWindow(320);   // 200 ms listen (100% duty)
  pScan->setActiveScan(true);
  //pScan->setMaxResults(0);
  pScan->setDuplicateFilter(true);
  pScan->start(SCAN_TIME, true, false);  // async scan
}

void loop() {  //loop block will only be reached if an OTA update was signalled
  static bool ledState = false; // keep state across loop() calls
     // regular housekeeping
  mqttClient.loop();
  server.handleClient();
  ElegantOTA.loop();

  if (OTAStarted) {
    // Blink LED while waiting for OTA activity
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    // watchdog for OTA phase
    if ((millis() - otaStartTime) > (SLEEP_TIME * 60 * 1000000)) {
      ESP.restart();
    }
    delay(500);
  } else {
    // Keep LED solid while scanning / running normally
    digitalWrite(LED_PIN, HIGH);
    // small yield
    delay(10);
  }
}