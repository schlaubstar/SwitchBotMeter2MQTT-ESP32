// config.h
#pragma once

// WiFi credentials
#define WIFI_SSID     "YourWifiSSID"
#define WIFI_PASSWORD "YourSecretPassword"

// WiFi IP-Settings
IPAddress localIP(192, 168, 179, 2);
IPAddress gatewayIP(192, 168, 179, 1);
IPAddress subnetMask(255, 255, 255, 0);

// MQTT settings
#define MQTT_SERVER   "192.168.179.3"
#define MQTT_PORT     1883
#define MQTT_USER     "UserForMQTTServer"
#define MQTT_PASS     "PasswordForMQTTServer"
#define MQTT_CLIENT   "ClientIDUsedToRegister"
#define MQTT_PUBTOPIC "SwitchBot2MQTT" //the "Root" MQTT topic, that will be appended with /SwitchBotMac/values 
#define MQTT_SUBTOPIC "SwitchBot2MQTTC3/OTA" //the topic the ESP32 subscribes to, to stay awake and wait for an OTA Update 
//be aware that you need to set the value to 'true' and retain the message on the MQTT Broker so it gets submitted to the ESP32 device

//OTA specific (Credentials, FW-Revision/Build Date
#define OTA_USER     "UserForElegantOTAFrontend"    //device is reachable for OTA-Update unter the assignedIP: http://localIP/update with the supplied user&password
#define OTA_PWD      "PasswordForElegantOTAFrontend" 
#define FW_VERSION   "2025-09-11"     //will be displayed under: http://localIP/status


// Debug flag
 //if true Serial Monitor displays debug infos, make sure: Tools > USB CDC On Boot > is set to "Enabled", if you want to use this on ESP32C3
#define DEBUG_ENABLED true

//ScanTime in MILLISECONDS (5-7 seconds should be fine to read all devices in range)
#define SCAN_TIME 30000

//Deep-Sleep-Time in MINUTES between each "wake up > read out > submit via MQTT cycle "
#define SLEEP_TIME 10

//LED_PIN of blue LED, should be fixed 8 on an ESP32C3 and 2 on a classical ESP32
#define LED_PIN 2

//The Mapping (Filterlist) of known devices needs to be maintained in the Code (.ino) file