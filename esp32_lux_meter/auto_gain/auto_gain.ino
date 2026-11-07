/*
  Step 3: Automatic gain ranging
  Steps the gain through 1x / 25x / 428x / 9876x based on the raw full-spectrum count.

  TOO_BRIGHT and TOO_DIM are design choices, not datasheet values.
  Tune them on your hardware.
*/
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);

const tsl2591Gain_t gains[]    = { TSL2591_GAIN_LOW, TSL2591_GAIN_MED,
                                   TSL2591_GAIN_HIGH, TSL2591_GAIN_MAX };
const char*         gainName[] = { "1x", "25x", "428x", "9876x" };
int gainIdx = 1;                    // start at medium (25x)

const uint16_t TOO_BRIGHT = 30000;  // raw counts: step gain down above this
const uint16_t TOO_DIM    = 800;    // raw counts: step gain up below this (x25 biggest step stays under TOO_BRIGHT)

void applyGain() {
  tsl.setGain(gains[gainIdx]);
}

// Returns true when lux is valid. 'saturated' is set if even 1x gain is too bright.
bool readLux(float &lux, bool &saturated) {
  saturated = false;
  for (int i = 0; i < 4; i++) {
    uint32_t lum  = tsl.getFullLuminosity();
    uint16_t ir   = lum >> 16;
    uint16_t full = lum & 0xFFFF;

    if (full > TOO_BRIGHT || ir > TOO_BRIGHT) {
      if (gainIdx > 0) { gainIdx--; applyGain(); continue; }
      saturated = true;             // already at lowest gain
      return false;
    }
    if (full < TOO_DIM && gainIdx < 3) {
      gainIdx++; applyGain(); continue;
    }

    lux = tsl.calculateLux(full, ir);
    if (lux < 0) {                  // negative = overflow per Adafruit docs
      if (gainIdx > 0) { gainIdx--; applyGain(); continue; }
      saturated = true;
      return false;
    }
    return true;
  }
  return false;                     // did not settle in 4 tries
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!tsl.begin(&Wire)) {
    Serial.println("TSL2591 not found. Check wiring.");
    while (1) delay(10);
  }
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
  applyGain();
}

void loop() {
  float lux = 0; bool sat = false;
  if (readLux(lux, sat)) {
    Serial.print(lux, 2);
    Serial.print(" lux  (gain ");
    Serial.print(gainName[gainIdx]);
    Serial.println(")");
  } else if (sat) {
    Serial.println("Saturated: too bright for this sensor setup");
  } else {
    Serial.println("No valid reading yet");
  }
  delay(500);
}
