# ESP32 + TSL2591 Lux Meter Firmware Stages

Build in order and confirm each stage works before moving on, or flash `lux_meter_bt/` directly if your wiring is already verified.

| Folder | Stage | What it does |
| --- | --- | --- |
| [`i2c_scanner/`](i2c_scanner/) | Stage 1 | Scans the I2C bus; expect `0x29` for the TSL2591 and usually `0x3C` for the OLED |
| [`basic_read/`](basic_read/) | Stage 2 | Reads IR + full-spectrum channels and prints lux at fixed `25x` gain |
| [`auto_gain/`](auto_gain/) | Stage 3 | Adds automatic gain ranging (`1x`, `25x`, `428x`, `9876x`) |
| [`esp32_oled_test/`](esp32_oled_test/) | Stage 4 | Tests the SSD1306 OLED and onboard LED on their own (no sensor required) |
| [`lux_meter_buttons/`](lux_meter_buttons/) | Stage 5 | Full meter with OLED + HOLD and MODE buttons (no Bluetooth) |
| [`lux_meter_bt/`](lux_meter_bt/) | Stage 6 | Complete firmware: everything above plus classic Bluetooth serial CSV output |

## Wiring

| Part | ESP32 Pin |
| --- | --- |
| TSL2591 VIN / GND | `3V3` / `GND` |
| TSL2591 SDA / SCL | `GPIO 21` / `GPIO 22` |
| OLED VCC / GND / SDA / SCL | `3V3` / `GND` / `GPIO 21` / `GPIO 22` (shares the I2C bus) |
| HOLD button | `GPIO 4` to `GND` (two diagonal legs of a 4-pin tactile button) |
| MODE button | `GPIO 13` to `GND` (two diagonal legs) |
| TSL2591 3Vo, INT | Not connected |

## Setup in Arduino IDE

1. Install the **esp32** board package by Espressif Systems in Boards Manager.
2. Select **ESP32 Dev Module** and your COM port under **Tools**.
3. Install from the Library Manager:
   - `Adafruit TSL2591 Library`
   - `Adafruit Unified Sensor`
   - `Adafruit SSD1306`
   - `Adafruit GFX Library`
4. Open a sketch folder (Arduino IDE requires the `.ino` filename to match its parent folder), upload, and set the Serial Monitor to **115200** baud.

`BluetoothSerial.h` comes with the ESP32 package. Classic Bluetooth SPP requires a classic ESP32 board (not ESP32-S3 or ESP32-C3).

## Using `lux_meter_bt`

- **HOLD (`GPIO 4`):** Short press freezes or resumes the displayed reading. Measurement pauses while held, so min/max and Bluetooth output pause too.
- **MODE (`GPIO 13`):** Short press cycles `LUX`, `MIN/MAX`, and `RAW` screens. Hold for 1.5 s to reset min/max.
- **Bluetooth (`LuxMeter`):** Pair in your laptop or Android Bluetooth settings, then open [`../lux_logger/index.html`](../lux_logger/index.html) or a serial terminal on the Outgoing COM port. Output is CSV (`millis,lux,gain`), one line every 500 ms. An empty lux field (`500,,1x`) means no valid reading (saturated or out of range).
- **Calibration:** Set `CAL_SCALE` and `CAL_OFFSET` at the top of the sketch after comparing against a reference meter (`calibrated = raw * CAL_SCALE + CAL_OFFSET`).
