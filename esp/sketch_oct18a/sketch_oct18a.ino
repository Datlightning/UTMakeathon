#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "VIHASLENOVO";
const char* password = "vihaslenovo";

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Trying to connect to ");
    Serial.println(ssid);
  }
  Serial.println("\nConnected!");
  getData();

  // Fetch data from website
}
void getData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Add the route + query parameter
    String serverPath = "http://10.145.63.190:5000/handle-esp32-data?password=password123";
    http.begin(serverPath);

    // Send GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("HTTP Response:");
      Serial.println(payload);  // prints JSON or message
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }

    http.end();  // Free resources
  }
}
void sendData() {
}
void loop() {
  // Nothing here
}
