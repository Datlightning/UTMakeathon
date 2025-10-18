#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // No reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG_PIN 25
#define ECHO_PIN 33

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();
  delay(1000);
}

void loop() {
  // Trigger distance sensor
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms
  float distance = duration * 0.0343 / 2;

  // Serial print
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // OLED print
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Dist: ");
  display.print(distance, 1);
  display.println(" cm");
  display.display();

  delay(500);
}