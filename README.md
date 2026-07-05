# ESP32 IMU Web Visualizer

ESP32 reads an MPU6050 motion sensor and displays its data as an interactive 3D
visualization in a browser. The page is served directly by the ESP32, and sensor
updates are streamed in real time over WebSocket.

## Features

- MPU6050 acceleration acquisition over I²C
- Browser-based WebGL visualization
- Real-time WebSocket updates
- Automatic connection to the ESP32 hosting the page
- Support for multiple connected browser clients
- Manual position, rotation, scale, and field-of-view controls

## Hardware

- ESP32 development board
- MPU6050 module

Default wiring:

| MPU6050 | ESP32 |
|---------|-------|
| SDA     | GPIO 21 |
| SCL     | GPIO 22 |
| VCC     | 3.3 V |
| GND     | GND |

## Build and run

Clone the repository with its dependencies:

```bash
git clone --recurse-submodules https://github.com/Alexandr4702/Web-Server-on-esp32-with-visualisation-imu-sensors.git
cd Web-Server-on-esp32-with-visualisation-imu-sensors
```

In an ESP-IDF shell:

```bash
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p <serial-port> flash monitor
```

Configure the Wi-Fi SSID and password in `menuconfig`. After the ESP32 connects,
open its IP address in a modern browser:

```text
http://<esp32-ip>/
```

The page connects to the WebSocket endpoint automatically and starts displaying
sensor updates. If the MPU6050 is unavailable, the visualization remains
accessible but receives neutral sensor values.
