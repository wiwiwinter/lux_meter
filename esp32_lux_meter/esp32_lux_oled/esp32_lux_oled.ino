/*
  ESP32 + TSL2591 + SSD1306 OLED lux meter (test build)

  Only these three parts. No SD card, button, Wi-Fi or battery.

  Wiring (OLED and sensor share the same two I2C wires):
    TSL2591  VIN -> 3V3   GND -> GND   SDA -> GPIO21   SCL -> GPIO22
    OLED     VCC -> 3V3   GND -> GND   SDA -> GPIO21   SCL -> GPIO22

  Libraries (Library Manager):
    Adafruit TSL2591 Library
    Adafruit Unified Sensor
    Adafruit SSD1306
    Adafruit GFX Library

  Serial Monitor: 115200 baud.

  NOTE: not compiled or run on hardware by the author. Expect to adjust.
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- pins and display ----------------
#define SDA_PIN   21
#define SCL_PIN   22
#define SCREEN_W  128
#define SCREEN_H  64
#define OLED_ADDR 0x3C      // confirm with the I2C scanner; some modules use 0x3D

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);
bool oledOk = false;

// ---------------- sensor ----------------
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

// ---------------- calibration (optional) ----------------
// calibrated = raw * CAL_SCALE + CAL_OFFSET
const float CAL_SCALE  = 1.00;
const float CAL_OFFSET = 0.00;

// ---------------- auto-gain tuning (design choices, not datasheet values) ----------------
const tsl2591Gain_t gains[]    = { TSL2591_GAIN_LOW, TSL2591_GAIN_MED,
                                   TSL2591_GAIN_HIGH, TSL2591_GAIN_MAX };
const char*         gainName[] = { "1x", "25x", "428x", "9876x" };
int gainIdx = 1;                    // start at 25x

const uint16_t TOO_BRIGHT = 30000;  // raw counts: step gain down above this
const uint16_t TOO_DIM    = 200;    // raw counts: step gain up below this
const uint16_t SETTLE_MS  = 120;    // wait after a gain change

// ---------------- state ----------------
float lux = 0;
bool  valid = false;
bool  saturated = false;
float minLux =  1e12;
float maxLux = -1e12;

void applyGain() {
  tsl.setGain(gains[gainIdx]);
  delay(SETTLE_MS);
}

// Returns true when a valid lux value was produced.
bool readLux(float &out, bool &sat) {
  sat = false;
  for (int i = 0; i < 4; i++) {
    uint32_t lum  = tsl.getFullLuminosity();
    uint16_t ir   = lum >> 16;      // high word: infrared
    uint16_t full = lum & 0xFFFF;   // low word: IR + visible

    if (full > TOO_BRIGHT) {
      if (gainIdx > 0) { gainIdx--; applyGain(); continue; }
      sat = true;                   // already at lowest gain
      return false;
    }
    if (full < TOO_DIM && gainIdx < 3) {
      gainIdx++; applyGain(); continue;
    }

    float raw = tsl.calculateLux(full, ir);
    if (raw < 0) return false;      // negative = overflow per Adafruit docs
    out = raw * CAL_SCALE + CAL_OFFSET;
    if (out < 0) out = 0;
    return true;
  }
  return false;                     // did not settle in 4 tries
}

void showOled() {
  if (!oledOk) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Lux meter  gain ");
  display.print(gainName[gainIdx]);

  display.setTextSize(2);
  display.setCursor(0, 20);
  if (valid)          display.print(lux, 1);
  else if (saturated) display.print("Too bright");
  else                display.print("Range!");

  display.setTextSize(1);
  display.setCursor(0, 44);
  display.print("lux");

  if (maxLux >= minLux) {
    display.setCursor(0, 54);
    display.print("min ");  display.print(minLux, 0);
    display.print(" max "); display.print(maxLux, 0);
  }
  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED first, so any sensor error can be shown on screen too.
  if (display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    oledOk = true;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Lux meter");
    display.println("Starting...");
    display.display();
  } else {
    Serial.println("OLED not found. Check wiring and OLED_ADDR. Continuing with serial only.");
  }

  if (!tsl.begin(&Wire)) {
    Serial.println("TSL2591 not found. Check wiring.");
    if (oledOk) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("TSL2591 not found");
      display.println("Check wiring");
      display.display();
    }
    while (1) delay(10);
  }
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
  applyGain();
}

void loop() {
  valid = readLux(lux, saturated);

  if (valid) {
    if (lux < minLux) minLux = lux;
    if (lux > maxLux) maxLux = lux;
    Serial.print(lux, 2);
    Serial.print(" lux  (gain ");
    Serial.print(gainName[gainIdx]);
    Serial.println(")");
  } else if (saturated) {
    Serial.println("Saturated: too bright for this sensor setup");
  } else {
    Serial.println("No valid reading");
  }

  showOled();
  delay(500);
}
