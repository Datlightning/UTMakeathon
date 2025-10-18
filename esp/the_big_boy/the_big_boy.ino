#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ====== WiFi & Server Setup ======
const char* ssid = "VIHASLENOVO";
const char* password = "vihaslenovo";
const char* serverURL = "http://10.145.17.126:5000/handle-esp32-data?password=password123";

String rawData = "";
DynamicJsonDocument jsonData(4096);

String getData();
void sendQuantities(int q1, int q2);

// ====== OLED Setup ======
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====== Ultrasonic + Servo + Buttons ======
#define TRIG_PIN1 18
#define ECHO_PIN1 19
#define TRIG_PIN2 2
#define ECHO_PIN2 4
#define TOGGLE_BUTTON_PIN 5   // toggles between test sensors
#define UPDATE_BUTTON_PIN 15  // triggers POST update
#define SERVO_PIN 23

#define SERVO_FORWARD 0
#define SERVO_STOP 92
#define DEGREES_PER_SECOND 85

bool test = false;
Servo myServo;

// ====== Function Declarations ======
void setAngle(int targetAngle);
float getDistance(bool useSecondSensor);
void displayMessage(String text);

// ===================================================
// ================= SETUP ============================
// ===================================================
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(TOGGLE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UPDATE_BUTTON_PIN, INPUT_PULLUP);

  myServo.attach(SERVO_PIN);

  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true)
      ;
  }
  displayMessage("Starting...");

  // ===== WiFi Connection =====
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  // ===== Get initial server data =====
  rawData = getData();

  Serial.println("Raw data from server:");
  Serial.println(rawData);

  // ===== Parse JSON =====
  DeserializationError error = deserializeJson(jsonData, rawData);
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.f_str());
  } else {
    Serial.println("Parsed JSON:");
    serializeJsonPretty(jsonData, Serial);
    Serial.println();
  }
}

// ===================================================
// ================= MAIN LOOP ========================
// ===================================================
void loop() {
  static bool lastToggleState = HIGH;
  static bool lastUpdateState = HIGH;

  // ===== Toggle Button =====
  bool currentToggleState = digitalRead(TOGGLE_BUTTON_PIN);
  if (lastToggleState == HIGH && currentToggleState == LOW) {
    test = !test;
    Serial.println(test ? "Switched to Sensor THT" : "Switched to Sensor FLT");
    delay(200);
  }
  lastToggleState = currentToggleState;

  // ===== Distance Reading =====
  float distance = getDistance(true);
  float distance2 = getDistance(false);
  Serial.print("Distance (");
  Serial.print(test ? "THT" : "FLT");
  Serial.print("): ");
  Serial.print(distance);
  Serial.println(" cm");

  // ===== Servo Logic =====
  if (!test && distance > 30) {
    setAngle(90);
  } else {
    setAngle(0);
  }

  // ===== OLED Display =====
  String containerType = "Unknown";

  // Make sure JSON is valid and has at least two containers
  JsonArray root = jsonData.as<JsonArray>();
  if (root.size() >= 2) {
    // Select container based on 'test' flag
    JsonObject targetContainer = root[test ? 1 : 0];
    containerType = targetContainer["type"].as<String>();
    // Read 'type' from the first item if available
  }
  float current_distance = getDistance(test);
  // Display distance with container type
  String text = "Dist (" + containerType + "): " + String(current_distance, 1) + " cm";
  displayMessage(text);

  // ===== Update Button =====
  bool currentUpdateState = digitalRead(UPDATE_BUTTON_PIN);

  if (lastUpdateState == HIGH && currentUpdateState == LOW) {
    Serial.println("Update button pressed!");

    // Example: new quantity = (100 - distance) / 4
    int newQuantity = max(0, (int)((5.9 - distance)));
    int newQuantity2 = max(0, (int)((100 - distance2) / 4));

    int q1 = 0;
    int q2 = 0;

    // Decide which quantity to send based on test flag
    q1 = newQuantity;
      q2 = newQuantity2;
             Serial.print("Sending quantities: ");
    Serial.print(q1);
    Serial.print(", ");
    Serial.println(q2);

    // Send just the two quantities + password
    sendQuantities(q1, q2);

    displayMessage(String("Sent: Q1=") + q1 + " Q2=" + q2);

    delay(200);  // debounce delay
    rawData = getData();

    Serial.println("Raw data from server:");
    Serial.println(rawData);

    // ===== Parse JSON =====
    DeserializationError error = deserializeJson(jsonData, rawData);
    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.f_str());
    } else {
      Serial.println("Parsed JSON:");
      serializeJsonPretty(jsonData, Serial);
      Serial.println();
    }
  }

  lastUpdateState = currentUpdateState;
  delay(50);  // small debounce outside the press check
}

// ===================================================
// =============== NETWORK FUNCTIONS ==================
// ===================================================
String getData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String payload = http.getString();
      http.end();
      return payload;
    } else {
      Serial.print("Error on HTTP GET: ");
      Serial.println(httpResponseCode);
      http.end();
      return "{}";
    }
  } else {
    Serial.println("WiFi not connected!");
    return "{}";
  }
}

void sendQuantities(int q1, int q2) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Build the URL with parameters
    String url = String(serverURL) + "&q1=" + String(q1) + "&q2=" + String(q2);

    Serial.println("\nSending POST request to:");
    Serial.println(url);

    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Send an empty body (URL contains all info)
    int httpResponseCode = http.POST("");

    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server response:");
      Serial.println(response);
    } else {
      Serial.println("POST failed");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

// ===================================================
// =============== HELPER FUNCTIONS ===================
// ===================================================
float getDistance(bool useSecondSensor) {
  long duration;
  float distance;
  int trig = useSecondSensor ? TRIG_PIN2 : TRIG_PIN1;
  int echo = useSecondSensor ? ECHO_PIN2 : ECHO_PIN1;

  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH, 30000);
  distance = duration * 0.0343 / 2.0;
  return distance;
}

void setAngle(int targetAngle) {
  targetAngle = constrain(targetAngle, 0, 360);
  float durationSec = targetAngle / DEGREES_PER_SECOND;
  unsigned long durationMs = (unsigned long)(durationSec * 1000);
  myServo.write(SERVO_FORWARD);
  delay(durationMs);
  myServo.write(SERVO_STOP);
}

void displayMessage(String text) {
  int x = (SCREEN_WIDTH - text.length() * 6) / 2;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, 28);
  display.print(text);
  display.display();
}
