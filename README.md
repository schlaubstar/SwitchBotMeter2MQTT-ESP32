ESP32 SwitchBot BLE Scanner / SwitchBotMeter2MQTT

This project uses an ESP32 based board to scan for SwitchBot Meter devices via Bluetooth Low Energy (BLE). It was originally tested on an ESP32-C3 module, but due to limitations in its BLE stack, the ESP32 DEV Module is recommended for reliable scanning.

Features

Scans for nearby SwitchBot Meter devices

Displays BLE advertisement data

Optional LED signaling for scan status

Compatible with all ESP32 Modules but ESP32C3 will most likely not show all SwitchBotMeters although it actually should

Hardware Requirements

ESP32 DEV Module (recommended), ESP32C3 et.

Optional: onboard LED (GPIO2) for status indication

Getting Started

Clone this repository

Open the project in your preferred IDE (e.g., Arduino IDE or PlatformIO)

Upload the code to your ESP32 Module

Monitor the serial output, with debug set to true, to see detected SwitchBot devices

License

This project is licensed under the Attribution-NonCommercial License.

You are free to:

Copy and redistribute the material in any medium or format

Remix, transform, and build upon the material

Under the following terms:

Attribution: You must give appropriate credit, provide a link to this GitHub project, and indicate if changes were made.

NonCommercial: You may not use the material for commercial purposes.

Author

Created by Schlaubstar.

Contributions

Feel free to fork the repository and submit pull requests. All contributions are welcome!

Troubleshooting

If your ESP32-C3 module does not detect SwitchBot devices reliably, try the following:

Use the ESP32 DEV Module instead

Increase scan duration and buffer size

Log raw BLE advertisements for debugging

The project includes some code fragments that originally were written in Python from the following sources and repositories:
- https://github.com/ronschaeffer/sbm2mqtt
- https://github.com/IanHarvey/bluepy

It is tailored to my needs, i.e., the sensors are read cyclically and you have to provide the MAC addresses of the devices you are interested in upfront otherwise the ESP32 module sleeps.
