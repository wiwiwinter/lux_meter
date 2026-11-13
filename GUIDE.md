# Build and Use Guide

Follow these steps in order. Do not move on until the current step works. The code is built in stages, so a problem is easy to find.

## 1. Get the parts and print the case

- Parts list: `DIY_ESP32_Lux_Meter_Parts_List.xlsx`
- Case: print `3d_files/MAIN BODY.stl` and `3d_files/COVER.stl`
- Full build and battery wiring: `DIY ESP32 Lux Meter (TSL2591) Build Documentation.docx`

You need a classic ESP32 board (such as ESP32 DevKit V1). ESP32-S3 and C3 boards do not work, because they have no classic Bluetooth.

## 2. Wire it

| Part | ESP32 pin |
| --- | --- |
| TSL2591: VIN, GND | 3V3, GND |
| TSL2591: SDA, SCL | GPIO 21, GPIO 22 |
| OLED: VCC, GND | 3V3, GND |
| OLED: SDA, SCL | GPIO 21, GPIO 22 (same wires as the sensor) |
| HOLD button | GPIO 4 to GND |
| MODE button | GPIO 13 to GND |

Use two diagonal legs of each 4-pin button. No resistor is needed.

## 3. Set up Arduino IDE (one time)

1. Boards Manager: install **esp32** by Espressif Systems.
2. Library Manager: install `Adafruit TSL2591 Library`, `Adafruit Unified Sensor`, `Adafruit SSD1306`, `Adafruit GFX Library`.
3. Tools > Board > **ESP32 Dev Module**. Pick your COM port.

A sketch file must sit in a folder with the same name (`lux_meter_bt/lux_meter_bt.ino`). Open it from inside the `esp32_lux_meter/` folder.

## 4. Test in stages

Open each sketch from `esp32_lux_meter/`, upload it, and open the Serial Monitor at **115200** baud.

| Step | Sketch | What you should see |
| --- | --- | --- |
| 1 | `i2c_scanner` | `0x29` (sensor) and usually `0x3C` (OLED) |
| 2 | `basic_read` | IR, Full, Visible and Lux values. Cover the sensor and shine a lamp on it: the lux should fall and rise. |
| 3 | `auto_gain` | Lux values with a gain label (1x, 25x, 428x, 9876x) that changes with the light |
| 4 | `esp32_oled_test` | "OLED test OK" on the screen and a blinking board LED (use this to check power and the screen on their own) |
| 5 | `lux_meter_buttons` | Lux on the OLED. HOLD freezes it. MODE cycles the screens. |
| 6 | `lux_meter_bt` | Everything above, plus Bluetooth output |

If a step shows nothing, fix it before continuing:
- Step 1 finds nothing: check power, ground and the SDA/SCL wires.
- OLED not at `0x3C`: change `OLED_ADDR` in the sketch.
- Upload stuck at `Connecting....`: hold the board's BOOT button, and use a data-capable USB cable.

## 5. Use the meter

- **HOLD** (short press): freeze or resume the reading.
- **MODE** (short press): LUX, MIN/MAX, RAW screens.
- **MODE** (hold 1.5 s): reset min/max.
- The screen shows "Too bright" when even 1x gain is saturated.

## 6. Calibrate (optional, recommended)

1. Put your meter and a reference lux meter side by side under the same steady light.
2. Note both readings at two or three brightness levels.
3. Set `CAL_SCALE` (reference ÷ your reading) and `CAL_OFFSET` at the top of `lux_meter_bt.ino`, then upload again.

The calibrated value is `raw × CAL_SCALE + CAL_OFFSET`.

## 7. Log data to Windows or Android

1. Upload `lux_meter_bt`. The Serial Monitor prints `BT client connected` when something connects.
2. Pair **LuxMeter** in the Bluetooth settings of your laptop or phone.
3. Open `lux_logger/index.html` and click **Connect**.
   - **Windows (Chrome or Edge):** pick the **Outgoing** COM port (Bluetooth settings > More Bluetooth options > COM Ports).
   - **Android (Chrome 138 or newer):** the page must be served over `https://`. Host the `lux_logger` folder (for example on GitHub Pages or Netlify) and open that link.
4. The page shows the live lux value, a graph, and the latest rows. Click **Download CSV** to save the log with date and time. Do this often, because the browser keeps only about 12 hours.

iPhone is not supported (iOS has no classic Bluetooth serial).

## 8. If something goes wrong

| Symptom | Check |
| --- | --- |
| Page says "Connected" but shows no data | HOLD is on; the OLED says "TSL2591 not found"; wrong COM port on Windows (use Outgoing) |
| Laptop or phone can't see LuxMeter | Not a classic ESP32; remove the old pairing and pair again |
| Lux reads 0 in a lit room | Sensor covered, or at the wrong gain; check the RAW screen |
| Gain flips back and forth | Widen the gap between `TOO_DIM` and `TOO_BRIGHT` |
| Buttons do nothing | Use the diagonal legs; a press reads LOW |
