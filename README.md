# ESP32 IMU Web Visualizer

ESP-IDF application that serves a WebGL page directly from an ESP32 and streams
motion data to it over WebSocket. The firmware reads acceleration from an
MPU6050 connected over I²C.

## Architecture

```text
MPU6050 -> Mpu6050Source -> Sample -> JSON -> WebSocket /ws -> WebGL page
                                      |
ESP-IDF network -> web_server.cpp -----+---- HTTP / -> embedded ws_test.html
```

- `main/app_main.cpp` initializes NVS/networking and owns connection lifecycle.
- `WebServer` owns the HTTP server and telemetry publishing task.
- `telemetry::Source` is the sensor abstraction; `Mpu6050Source` reads the IMU over I²C.
- `telemetry::Sample` keeps sensor data independent from JSON serialization.
- `main/ws_test.html` contains the embedded WebGL client.
- `json/` contains the nlohmann/json submodule.

The WebSocket endpoint is derived from the page URL, so no device IP address is
hard-coded in the browser client.

## Requirements

- ESP32 development board
- ESP-IDF (the project follows the ESP-IDF CMake workflow)
- Python and the toolchain installed by ESP-IDF
- Wi-Fi credentials available through ESP-IDF example connection settings

## Clone

The JSON dependency is a Git submodule:

```bash
git clone --recurse-submodules <repository-url>
cd Web-Server-on-esp32-with-visualisation-imu-sensors
```

For an existing clone:

```bash
git submodule update --init --recursive
```

## Configure, build, and flash

Open an ESP-IDF shell, then run:

```bash
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p <serial-port> flash monitor
```

In `menuconfig`, configure the Wi-Fi SSID and password under the example
connection settings. Replace `<serial-port>` with the board port, such as
`COM5` on Windows or `/dev/ttyUSB0` on Linux.

After the board receives an IP address, open `http://<esp32-ip>/` in a modern
browser. The page connects to `ws://<esp32-ip>/ws` automatically.

## Telemetry protocol

The server publishes a JSON object approximately every 50 ms:

```json
{
  "uptime": 12.34,
  "translation": [10.0, 0.0, -500.0],
  "quaternion": [1.0, 0.0, 0.0, 0.0]
}
```

`translation` is `[x, y, z]`. `quaternion` is `[w, x, y, z]` and should be
normalized. Keep this schema stable when replacing the MPU6050 implementation.

## Connecting a real IMU

The MPU6050 integration uses SDA on GPIO 21 and SCL on GPIO 22. To support a
different sensor, provide another `telemetry::Source` implementation. Keeping
sensor access out of the HTTP task makes it possible to test and change the IMU
independently of the transport and visualization.

## Known limitations

- The visualization page is currently a single large embedded HTML file.
- Samples are sent independently to every connected WebSocket client.
- There are no host-side tests yet; verification requires an ESP-IDF toolchain
  and, for end-to-end behavior, an ESP32 board.
