#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>  // Use ESP32-compatible Servo library

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG_PIN1 18
#define ECHO_PIN1 19
#define TRIG_PIN2 2
#define ECHO_PIN2 4
#define BUTTON_PIN 5
#define SERVO_PIN 23

#define SERVO_FORWARD 0       // full speed forward
#define SERVO_STOP 92         // calibrated stop value
#define DEGREES_PER_SECOND 60 // estimated rotation speed

bool test = false;
Servo myServo;
void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  myServo.attach(SERVO_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  String startupText = "Starting...";
  int charWidth = 6;
  int textWidth = startupText.length() * charWidth;
  int x = (SCREEN_WIDTH - textWidth) / 2;
  display.setCursor(x, 28);
  display.println(startupText);
  display.display();
  delay(1000);
}

void loop() {
  long duration = 0;
  float distance = 0.0;

  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    test = !test;
    Serial.println(test ? "Switched to Sensor THT" : "Switched to Sensor FLT");
    delay(50); // debounce
  }
  lastButtonState = currentButtonState;


  if (!test) {
    digitalWrite(TRIG_PIN1, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN1, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN1, LOW);
    duration = pulseIn(ECHO_PIN1, HIGH, 30000);
    distance = duration * 0.0343 / 2;
  } else {
    digitalWrite(TRIG_PIN2, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN2, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN2, LOW);
    duration = pulseIn(ECHO_PIN2, HIGH, 30000);
    distance = duration * 0.0343 / 2;
  }

  if (!test && distance > 30) {
    setAngle(90);
  } else {
    setAngle(0);
  }

  String text = String("Dist (") + (!test ? "THT" : "FLT") + "): " + String(distance, 1) + " cm";
  int x = (SCREEN_WIDTH - text.length() * 6) / 2;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, 28);
  display.print(text);
  display.display();

  delay(250);
}

void debugButtonPress(int pin) {
  if (digitalRead(pin) == LOW) {
    Serial.print("Button pressed on GPIO ");
    Serial.println(pin);
  }
}

void setAngle(int targetAngle) {
  targetAngle = constrain(targetAngle, 0, 360);

  float durationSec = targetAngle / DEGREES_PER_SECOND;
  unsigned long durationMs = (unsigned long)(durationSec * 1000);

  myServo.write(SERVO_FORWARD);
  Serial.print(targetAngle);
  Serial.print("° for ");
  Serial.print(durationMs);
  Serial.println(" ms");

  delay(durationMs);
  myServo.write(SERVO_STOP); // stop
}