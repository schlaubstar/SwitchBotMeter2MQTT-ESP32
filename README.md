This project is a quick solution to my problem, which is that I like to use SwitchBotMeter temperature and humidity sensors, but they are often too far away from my Raspberry, which I use as a “hub” to read the sensors and then send the data to my home automation system via MQTT. 

That's why I built a device based on the ESP32 modules, specifically the small ESP32C module, that can also work decentrally—it reads the sensors in its vicinity and sends the data to the broker via WiFi and MQTT.

The project includes code converted from Python to C from the following sources and repositories:
- https://github.com/ronschaeffer/sbm2mqtt

It is tailored to my needs, i.e., the sensors are read cyclically, otherwise the ESP32 module sleeps, and it is filtered for known BLE devices, i.e., you have to read and store the MAC addresses of new SwitchBotMeters beforehand. 
