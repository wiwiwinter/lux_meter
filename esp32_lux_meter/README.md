# ESP32 + TSL2591 Lux Meter: Code Package

Build in order and confirm each stage works before moving on.

| Folder | Stage | What it does |
| --- | --- | --- |
| `i2c_scanner/` | Step 1 | Lists I2C devices; expect 0x29 for the TSL2591 and usually 0x3C for the OLED |
| `basic_read/` | Step 2 | Reads IR + full-spectrum channels, prints lux (fixed 25x gain) |
| `auto_gain/` | Step 3 | Adds automatic gain ranging (1x / 25x / 428x / 9876x) |
| `lux_meter_buttons/` | Step 4 | OLED + HOLD and MODE buttons, no Bluetooth |
| `lux_meter_bt/` | Step 5 | Everything above plus classic Bluetooth serial CSV to a laptop |

## Wiring

| Part | ESP32 |
| --- | --- |
| TSL2591 VIN / GND | 3V3 / GND |
| TSL2591 SDA / SCL | GPIO 21 / GPIO 22 |
| OLED VCC / GND / SDA / SCL | 3V3 / GND / GPIO 21 / GPIO 22 (shares the bus) |
| HOLD button | GPIO 4 to GND (diagonal legs of a 4-pin tactile button) |
| MODE button | GPIO 13 to GND (diagonal legs) |
| TSL2591 3Vo, INT | not connected |

[Unverified] GPIO 21/22 are commonly the default I2C pins on ESP32 DevKit boards; confirm on your board's pinout. [Unverified] I believe GPIO 4 and 13 are not strapping pins; confirm as well.

## Setup

1. Install the Arduino IDE and the ESP32 board package from Espressif.
2. Select your board and port under Tools.
3. Install from the Library Manager:
   - Adafruit TSL2591 Library
   - Adafruit Unified Sensor
   - Adafruit SSD1306 and Adafruit GFX Library
4. Open a sketch folder (the `.ino` name must match its folder), upload, and set the Serial Monitor to **115200** baud.

`BluetoothSerial.h` comes with the ESP32 package; no extra install. Select **ESP32 Dev Module** as the board. [Unverified] I did not check this against the current package.

## Using lux_meter_bt

- **HOLD** (GPIO 4): freezes the displayed reading; press again to resume. Measurement pauses while held, so min/max and Bluetooth output pause too.
- **MODE** (GPIO 13): short press cycles LUX, MIN/MAX, RAW. Hold 1.5 s to reset min/max.
- **Bluetooth**: device name `LuxMeter`. Pair it on the laptop, open the COM port it creates in a serial terminal. Output is CSV, `millis,lux,gain`, one line per measurement (about every 500 ms). The time column is milliseconds since boot; there is no real-time clock.
- **Calibration**: set `CAL_SCALE` and `CAL_OFFSET` after comparing against a reference meter (`calibrated = raw * CAL_SCALE + CAL_OFFSET`).

## Battery power (optional)

USB 5V to the TP4056 charger, LiPo to its battery pads, TP4056 OUT+ through a slider switch to the boost converter input, boost output (5V) to the ESP32 VIN and GND. See the wiring section of the build document.

- [Inference] A single LiPo cell is likely too low a voltage to run the board through VIN directly, which is why a boost converter is used.
- [Inference] Do not wire the battery straight to the 3V3 pin.
- [Unverified] Pad names (IN+, OUT+, B+) vary between TP4056 boards; check polarity before connecting the battery.
- [Inference] Avoid powering the board from USB and the boost converter at the same time unless your board is built to handle it.

## Verification status

- Library calls (`begin`, `setGain`, `setTiming`, `getFullLuminosity`, `calculateLux`), gain levels and the 0x29 address were checked against Adafruit's TSL2591 pages.
- [Unverified] None of these sketches have been compiled or run on hardware.
- [Unverified] The OLED, button and Bluetooth code is written from memory of the Adafruit SSD1306/GFX and ESP32 Arduino APIs.
- The button and Bluetooth logic was run against stand-in (mock) libraries on a PC: HOLD freeze/resume, MODE cycling, debounce, and Bluetooth output only while a client is connected behaved as written. That does not test the real libraries or hardware.
- [Inference] `TOO_BRIGHT` (30000) and `TOO_DIM` (800) are design choices, not datasheet values; tune them.
- [Unverified] Whether `tsl.begin(&Wire)` re-initializes the bus is not confirmed. If it fails on non-default pins, try `tsl.begin()`.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| Scanner finds nothing | Power, ground, SDA/SCL wiring; try swapping SDA and SCL |
| Lux reads 0 in a lit room | Sensor covered or saturated; lower gain or shorten integration |
| Huge or negative lux | Overflow; lower gain or shorten integration |
| Gain flips back and forth | Widen the gap between `TOO_DIM` and `TOO_BRIGHT` |
| OLED blank | Wrong `OLED_ADDR`; check library installed |
| Laptop cannot see LuxMeter | Board must be a classic ESP32; remove old pairing and pair again |
| Buttons do nothing | Check diagonal legs; press reads LOW |
| Upload fails | Use a data-capable USB cable; reselect board and port |
