# Arduino IDE Examples for ESP32

Arduino sketches for the IoT course.

## Available Sketches

| Sketch | Description | QEMU |
|--------|-------------|------|
| `01-blink` | Blink LED on GPIO 2 with Serial output | Yes |
| `02-serial-output` | Chip info, counters, timing, simulated ADC | Yes |

## Running in QEMU (No Hardware Needed)

```bash
cd esp32-qemu
./run-arduino-example.sh 01-blink
./run-arduino-example.sh 02-serial-output
```

Press `Ctrl+A` then `X` to exit QEMU.

**Note:** Only sketches that don't use WiFi/BLE work in QEMU. GPIO, Serial, timers, and basic logic are fully supported.

## Running on Real Hardware

### Using Arduino IDE

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support: File > Preferences > Additional Board Manager URLs:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Tools > Board > Board Manager > Search "esp32" > Install
4. Select board: Tools > Board > ESP32 Dev Module
5. Open a sketch, click Upload

### Using arduino-cli

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 01-blink/
arduino-cli upload --fqbn esp32:esp32:esp32 --port /dev/ttyUSB0 01-blink/
arduino-cli monitor --port /dev/ttyUSB0 --config baudrate=115200
```
