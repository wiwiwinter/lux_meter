/*
  OLED-only test (no sensor, no buttons, no Bluetooth)

  Purpose: check that the ESP32 + SSD1306 OLED run when powered from the battery path.

  Wiring:
    OLED  VCC -> 3V3   GND -> GND   SDA -> GPIO21   SCL -> GPIO22
  The TSL2591 can stay unplugged.

  What you should see:
    OLED:  "OLED test", "OK", and an uptime counter that counts up every second.
    Board LED (often GPIO2 on DevKit V1): blinks once per second.
           It blinks even if the OLED is dark, so it shows the sketch is running.
    If the OLED is not found, the board LED blinks fast (about 5 times a second).

  Libraries: Adafruit SSD1306, Adafruit GFX Library.

  NOTE: not compiled or run on hardware by the author.
  [Unverified] The board LED pin (GPIO2) is from memory and may differ on your board.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN    21
#define SCL_PIN    22
#define LED_PIN    2
#define OLED_ADDR  0x3C   // your scan found 0x3C

Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool oledOk = false;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  oledOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  Serial.println(oledOk ? "OLED found" : "OLED NOT found");
}

void loop() {
  static uint32_t lastTick = 0;
  uint32_t now = millis();

  if (!oledOk) {                       // fast blink = OLED problem
    digitalWrite(LED_PIN, (now / 100) % 2);
    return;
  }

  if (now - lastTick >= 1000 || lastTick == 0) {
    lastTick = now;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));   // slow blink = running
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("OLED test");
    display.println("OK");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("up ");
    display.print(now / 1000);
    display.print(" s");
    display.display();
  }
}
