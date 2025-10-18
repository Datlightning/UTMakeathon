#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Vihas's iPhone";
const char* password = "l1gHtN1Ng";

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  
  // Fetch data from website
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://wifitest.adafruit.com/testwifi/index.html");
    
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("HTTP Response:");
      Serial.println(payload);  // prints HTML content
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}

void loop() {
  // Nothing here
}
