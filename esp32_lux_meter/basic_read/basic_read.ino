/*
  Step 2: Basic TSL2591 read
  Reads the IR and full-spectrum channels together, then converts to lux
  with the library's calculateLux(full, ir).

  Fixed gain (25x) and 100 ms integration. Cover the sensor with a hand,
  then shine a lamp on it: lux should fall and rise.
*/
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);  // 2591 = sensor ID tag

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!tsl.begin(&Wire)) {
    Serial.println("TSL2591 not found. Check wiring.");
    while (1) delay(10);
  }
  tsl.setGain(TSL2591_GAIN_MED);                 // 25x
  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // 100 ms
}

void loop() {
  uint32_t lum  = tsl.getFullLuminosity();
  uint16_t ir   = lum >> 16;      // high word: infrared
  uint16_t full = lum & 0xFFFF;   // low word: IR + visible
  float lux = tsl.calculateLux(full, ir);

  Serial.print("IR: ");      Serial.print(ir);
  Serial.print("  Full: ");  Serial.print(full);
  Serial.print("  Visible: "); Serial.print(full - ir);
  Serial.print("  Lux: ");   Serial.println(lux, 2);
  delay(500);
}
