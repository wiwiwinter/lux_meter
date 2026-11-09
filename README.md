# DIY ESP32 Lux Meter

A handheld light meter built from an ESP32, a TSL2591 light sensor and an SSD1306 OLED. It shows lux on the screen and streams readings over Bluetooth to a web page that graphs and logs them on Windows or Android.

> **Status:** the code has not been fully tested on hardware by the author. Expect to adjust pins, the OLED address, and thresholds.

## What's in this repo

| Path | What it is |
| --- | --- |
| [`esp32_lux_meter/`](esp32_lux_meter/) | Arduino sketches, built in 5 stages (see its [README](esp32_lux_meter/README.md)) |
| [`esp32_lux_meter/lux_meter_bt/`](esp32_lux_meter/lux_meter_bt/) | **The final sketch to upload**: sensor, auto-gain, OLED, buttons, Bluetooth |
| [`lux_logger/index.html`](lux_logger/index.html) | Web page that receives the Bluetooth data, draws a graph, keeps a log, exports CSV |
| `3d_files/` | Enclosure: `MAIN BODY.stl`, `COVER.stl`, `lux_meter_basemodel.3mf` |
| `DIY ESP32 Lux Meter (TSL2591) Build Documentation.docx` | Full build guide, including the battery wiring |
| `DIY_ESP32_Lux_Meter_Parts_List.xlsx` | Parts list |

## Hardware

| Part | ESP32 pin |
| --- | --- |
| TSL2591 and OLED, SDA | GPIO 21 |
| TSL2591 and OLED, SCL | GPIO 22 |
| TSL2591 and OLED, power | 3V3 and GND |
| HOLD button | GPIO 4 to GND |
| MODE button | GPIO 13 to GND |

The sensor and OLED share the same two I2C wires. Use a **classic ESP32** (such as ESP32 DevKit V1). The S3 and C3 boards have no classic Bluetooth.

## Upload the firmware

1. Install the **esp32** board package (Espressif) in Arduino IDE.
2. Install these libraries from Library Manager: `Adafruit TSL2591 Library`, `Adafruit Unified Sensor`, `Adafruit SSD1306`, `Adafruit GFX Library`.
3. Open `esp32_lux_meter/lux_meter_bt/lux_meter_bt.ino` (the file name must match its folder name).
4. Pick **ESP32 Dev Module** and your COM port, then click Upload. If it hangs at `Connecting....`, hold the board's BOOT button.
5. Open the Serial Monitor at **115200** baud.

If the screen stays blank, upload `esp32_lux_meter/i2c_scanner` first. The OLED is usually at `0x3C` and the sensor at `0x29`.

## Using the meter

- **HOLD** (short press): freeze or resume the reading.
- **MODE** (short press): cycle the screens LUX, MIN/MAX, RAW.
- **MODE** (hold 1.5 s): reset min/max.
- Gain steps automatically (1x, 25x, 428x, 9876x). The screen shows "Too bright" when even 1x is saturated.
- Calibration: set `CAL_SCALE` and `CAL_OFFSET` at the top of the sketch after comparing against a reference meter.

## Log data with the web page

The meter advertises as **`LuxMeter`** and sends one CSV line about every 500 ms:

```
millis,lux,gain
12345,342.18,25x
```

An empty lux field (`500,,1x`) means there was no valid reading. Nothing is sent while HOLD is on.

1. Pair `LuxMeter` in your device's Bluetooth settings.
2. Open `lux_logger/index.html` and click **Connect**.
   - **Windows:** use Chrome or Edge. Pick the **Outgoing** COM port (Bluetooth settings > More Bluetooth options > COM Ports).
   - **Android:** use Chrome 138 or newer. Chrome needs an `https://` page for this, so host the `lux_logger` folder (for example on GitHub Pages or Netlify) and open that link.
3. The page shows the live value, a graph (1 min, 10 min, 1 hour or all), and the last 20 rows. **Download CSV** saves the whole log with date and time.

Limits: the log is stored in the browser (about 12 hours at 2 rows per second, so download the CSV often), and Android may pause the page when the screen turns off. iPhone is not supported, because iOS has no classic Bluetooth serial.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| Page says "Connected" but shows no data | Is HOLD on? Is the Serial Monitor showing `BT client connected`? On Windows, did you pick the Outgoing port? Does the OLED show "TSL2591 not found"? |
| Laptop can't see `LuxMeter` | Board must be a classic ESP32. Remove the old pairing and pair again. |
| Upload fails | Use a data-capable USB cable, reselect board and port, hold BOOT. |
| OLED blank | Check `OLED_ADDR` (`0x3C` or `0x3D`) and the library install. |
| Buttons do nothing | Use two diagonal legs of the 4-pin button. A press reads LOW. |
