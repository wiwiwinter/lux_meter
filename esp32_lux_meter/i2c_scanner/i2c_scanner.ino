/*
  Step 1: I2C scanner
  Lists every device that answers on the I2C bus.

  Expected with the TSL2591 wired: a device at 0x29.
  Adafruit's guide says 0x28 is also taken by the chip, so seeing both is not a fault.
  If an SSD1306 OLED is also wired, it usually shows up at 0x3C (confirm here).
*/
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(500);
  Serial.println("Scanning I2C bus...");

  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) Serial.println("No I2C devices found. Check wiring.");
  else Serial.println("Scan done.");
}

void loop() {}
