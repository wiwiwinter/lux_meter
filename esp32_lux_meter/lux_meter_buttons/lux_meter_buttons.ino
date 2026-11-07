/*
  ESP32 DevKit V1 + TSL2591 + SSD1306 OLED lux meter, with HOLD and MODE buttons

  Wiring (OLED and sensor share the same two I2C wires):
    TSL2591  VIN -> 3V3   GND -> GND   SDA -> GPIO21   SCL -> GPIO22
    OLED     VCC -> 3V3   GND -> GND   SDA -> GPIO21   SCL -> GPIO22
    HOLD button  one leg -> GPIO4    other leg -> GND
    MODE button  one leg -> GPIO13   other leg -> GND
    (4-pin tactile buttons: use two diagonal legs. No resistor needed; the
     internal pull-up is used, so a press reads LOW.)

  What the buttons do:
    HOLD (short press)  Freezes the reading on the screen. Press again to resume.
                        Use it when you must move the meter, or step aside, before
                        reading the number. While held, nothing is measured, so
                        min/max do not change either.
    MODE (short press)  Cycles the screen: LUX -> MIN/MAX -> RAW -> LUX ...
    MODE (hold 1.5 s)   Resets min/max.

  Screens:
    LUX      big lux value
    MIN/MAX  lowest and highest lux since power-up or reset
    RAW      infrared, full-spectrum and visible counts, plus lux (for debugging)

  Libraries (Library Manager):
    Adafruit TSL2591 Library, Adafruit Unified Sensor, Adafruit SSD1306, Adafruit GFX Library

  Serial Monitor: 115200 baud.

  NOTE: not compiled or run on hardware by the author. Expect to adjust.
  Bluetooth output is not included in this sketch.
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- pins and display ----------------
#define SDA_PIN    21
#define SCL_PIN    22
#define HOLD_PIN   4
#define MODE_PIN   13
#define SCREEN_W   128
#define SCREEN_H   64
#define OLED_ADDR  0x3C     // confirm with the I2C scanner; some modules use 0x3D

const uint32_t READ_MS     = 500;   // time between measurements
const uint32_t DEBOUNCE_MS = 30;    // button debounce
const uint32_t LONG_MS     = 1500;  // MODE long-press time

Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);
bool oledOk = false;

// ---------------- sensor ----------------
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

// calibrated = raw * CAL_SCALE + CAL_OFFSET (see the calibration section of the document)
const float CAL_SCALE  = 1.00;
const float CAL_OFFSET = 0.00;

// Auto-gain tuning: design choices, not datasheet values.
const tsl2591Gain_t gains[]    = { TSL2591_GAIN_LOW, TSL2591_GAIN_MED,
                                   TSL2591_GAIN_HIGH, TSL2591_GAIN_MAX };
const char*         gainName[] = { "1x", "25x", "428x", "9876x" };
int gainIdx = 1;                    // start at 25x

const uint16_t TOO_BRIGHT = 30000;  // raw counts: step gain down above this
const uint16_t TOO_DIM    = 800;    // raw counts: step gain up below this (x25 biggest step stays under TOO_BRIGHT)

// ---------------- state ----------------
float    lux = 0;
bool     valid = false;
bool     saturated = false;
uint16_t rawIr = 0, rawFull = 0;
float    minLux =  1e12;
float    maxLux = -1e12;
bool     holdOn = false;

enum { MODE_LUX, MODE_MINMAX, MODE_RAW, MODE_COUNT };
int mode = MODE_LUX;

// ---------------- buttons ----------------
struct Button {
  uint8_t  pin;
  bool     state;       // true = released (pull-up), false = pressed
  bool     lastRaw;
  uint32_t lastChange;
  uint32_t downAt;
  bool     longFired;
};
Button holdBtn = { HOLD_PIN, true, true, 0, 0, false };
Button modeBtn = { MODE_PIN, true, true, 0, 0, false };

enum { EVT_NONE, EVT_PRESS, EVT_RELEASE };

uint8_t pollButton(Button &b) {
  bool raw = digitalRead(b.pin);
  uint32_t now = millis();
  if (raw != b.lastRaw) { b.lastRaw = raw; b.lastChange = now; }
  if ((now - b.lastChange) >= DEBOUNCE_MS && raw != b.state) {
    b.state = raw;
    if (!raw) { b.downAt = now; b.longFired = false; return EVT_PRESS; }
    return EVT_RELEASE;
  }
  return EVT_NONE;
}

// ---------------- sensor helpers ----------------
void applyGain() {
  tsl.setGain(gains[gainIdx]);
}

// Returns true when a valid lux value was produced.
bool readLux(float &out, bool &sat) {
  sat = false;
  for (int i = 0; i < 4; i++) {
    uint32_t lum  = tsl.getFullLuminosity();
    uint16_t ir   = lum >> 16;      // high word: infrared
    uint16_t full = lum & 0xFFFF;   // low word: IR + visible
    rawIr = ir;
    rawFull = full;

    if (full > TOO_BRIGHT || ir > TOO_BRIGHT) {
      if (gainIdx > 0) { gainIdx--; applyGain(); continue; }
      sat = true;                   // already at lowest gain
      return false;
    }
    if (full < TOO_DIM && gainIdx < 3) {
      gainIdx++; applyGain(); continue;
    }

    float v = tsl.calculateLux(full, ir);
    if (v < 0) {                    // negative = overflow per Adafruit docs
      if (gainIdx > 0) { gainIdx--; applyGain(); continue; }
      sat = true;
      return false;
    }
    out = v * CAL_SCALE + CAL_OFFSET;
    if (out < 0) out = 0;
    return true;
  }
  return false;                     // did not settle in 4 tries
}

void resetMinMax() {
  minLux =  1e12;
  maxLux = -1e12;
}

// ---------------- display ----------------
// Decimal places that fit the value: 0.01 in the dark, whole numbers in daylight.
int dec(float v) { return v < 10 ? 2 : (v < 1000 ? 1 : 0); }

void drawHeader(const char* name) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(name);
  display.print(" ");
  display.print(gainName[gainIdx]);
  if (holdOn) {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);   // inverted HOLD badge
    display.setCursor(98, 0);
    display.print(" HOLD");
    display.setTextColor(SSD1306_WHITE);
  }
}

void draw() {
  if (!oledOk) return;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (mode == MODE_LUX) {
    drawHeader("LUX");
    display.setTextSize(2);
    display.setCursor(0, 22);
    if (valid)          display.print(lux, dec(lux));
    else if (saturated) display.print("Too bright");
    else                display.print("Range!");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("lux");

  } else if (mode == MODE_MINMAX) {
    drawHeader("MIN/MAX");
    display.setTextSize(1);
    display.setCursor(0, 12);
    display.print("now ");
    if (valid) display.print(lux, dec(lux)); else display.print("--");
    bool have = (maxLux >= minLux);
    display.setTextSize(2);
    display.setCursor(0, 24);
    display.print("lo ");
    if (have) display.print(minLux, dec(minLux)); else display.print("--");
    display.setCursor(0, 44);
    display.print("hi ");
    if (have) display.print(maxLux, dec(maxLux)); else display.print("--");

  } else {  // MODE_RAW
    drawHeader("RAW");
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.print("IR   "); display.print(rawIr);
    display.setCursor(0, 28);
    display.print("Full "); display.print(rawFull);
    display.setCursor(0, 40);
    display.print("Vis  "); display.print((int)rawFull - (int)rawIr);
    display.setCursor(0, 52);
    display.print("Lux  ");
    if (valid) display.print(lux, dec(lux)); else display.print("--");
  }
  display.display();
}

// ---------------- setup / loop ----------------
void setup() {
  Serial.begin(115200);
  delay(300);
  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(HOLD_PIN, INPUT_PULLUP);
  pinMode(MODE_PIN, INPUT_PULLUP);

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
  bool redraw = false;

  // HOLD: toggle on press.
  if (pollButton(holdBtn) == EVT_PRESS) {
    holdOn = !holdOn;
    if (holdOn) {
      Serial.print("HOLD on: ");
      if (valid) { Serial.print(lux, 2); Serial.println(" lux"); }
      else       { Serial.println("no valid reading"); }
    } else {
      Serial.println("HOLD off");
    }
    redraw = true;
  }

  // MODE: short press cycles screens (on release); long press resets min/max.
  uint8_t m = pollButton(modeBtn);
  if (m == EVT_RELEASE && !modeBtn.longFired) {
    mode = (mode + 1) % MODE_COUNT;
    redraw = true;
  }
  if (!modeBtn.state && !modeBtn.longFired && (millis() - modeBtn.downAt) >= LONG_MS) {
    resetMinMax();
    modeBtn.longFired = true;
    Serial.println("Min/max reset");
    redraw = true;
  }

  // Measure (paused while HOLD is on).
  static uint32_t lastRead = 0;
  if (!holdOn && (millis() - lastRead) >= READ_MS) {
    lastRead = millis();
    float prev = lux;
    bool hadPrev = valid;
    valid = readLux(lux, saturated);
    // Steady light (within 10%): blend to stop LED flicker jitter. Bigger change: jump at once.
    if (valid && hadPrev && fabsf(lux - prev) < 0.1f * prev) lux = prev * 0.3f + lux * 0.7f;

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
    redraw = true;
  }

  if (redraw) draw();
}
