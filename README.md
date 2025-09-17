# ESP32 SwitchBot BLE Scanner

This project uses an ESP32 development board to scan for SwitchBot Meter devices via Bluetooth Low Energy (BLE). It was originally tested on an ESP32-C3 module, but due to limitations in its BLE stack, the ESP32 DEV Module is recommended for reliable scanning.

## Features

* Scans for nearby SwitchBot Meter devices
* Displays BLE advertisement data
* Optional LED signaling for scan status
* Compatible with ESP32 DEV Module
* ElegantOTA support for over-the-air updates
* MQTT topic subscription to signal OTA update mode

## Hardware Requirements

* ESP32 DEV Module (recommended), ESP32C3 may work but miss some SwitchBotMeters
* Optional: onboard LED (GPIO2) for status indication

## Getting Started

1. Clone this repository
2. Open the project in your preferred IDE (e.g., Arduino IDE or PlatformIO)
3. Upload the code to your ESP32 DEV Module
4. Monitor the serial output to see detected SwitchBot devices

## License

This project is licensed under the **Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)** license.

You are free to:

* Share — copy and redistribute the material in any medium or format
* Adapt — remix, transform, and build upon the material

Under the following terms:

* **Attribution** — You must give appropriate credit, provide a link to this GitHub project, and indicate if changes were made.
* **NonCommercial** — You may not use the material for commercial purposes.

For full license details, see: <https://creativecommons.org/licenses/by-nc/4.0/>

## Author

Created by Schlaubstar.

## Notes

This project includes code fragments originally written in Python, adapted from the following sources:

* <https://github.com/ronschaeffer/sbm2mqtt>
* <https://github.com/IanHarvey/bluepy>

The implementation is tailored to specific needs: sensors are read cyclically, and MAC addresses of target devices must be provided in advance. 
It basically acts as "range-extender" for SwitchBot Meter devices that are too far away from the Bluetooth receiver.


## Contributions

Feel free to fork the repository and submit pull requests. All contributions are welcome!

## Troubleshooting

If your ESP32-C3 module does not detect SwitchBot devices reliably, try the following:

* Use the ESP32 DEV Module instead
* Increase scan duration and buffer size
* Use active scanning mode
* Log raw BLE advertisements for debugging

## LED Signaling

To use the onboard LED for signaling scan status, connect to GPIO2 and use simple blink patterns to indicate scanning, success, or errors.

## OTA Update Signaling

The device supports ElegantOTA for over-the-air firmware updates. It subscribes to a specific MQTT topic that signals when an OTA update is desired.

If the topic value is set to `"TRUE"`, the device will remain awake and ready to receive the OTA update instead of entering sleep mode.
