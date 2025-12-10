#include "WifiOta.h"

// ----------------------------------------------------------------------
// ---------- WIFI + OTA SETUP (Non-blocking on failure) ----------------
// ----------------------------------------------------------------------
void setupWiFiAndOTA() {
  if (WiFi.status() != WL_CONNECTED) {
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Connecting WiFi..."));
    display.display();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startAttemptTime = millis();
    const unsigned long wifiTimeout = 5000; 

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
      delay(200); 
    }
  }
  
  display.clearDisplay();
  display.setTextSize(1);

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true; 
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.onStart([]() { Serial.println("OTA Update Start"); });
    ArduinoOTA.onEnd([]() { Serial.println("\nOTA Update End"); });
    ArduinoOTA.onError([](ota_error_t error) { Serial.printf("OTA Error[%u]\n", error); });
    ArduinoOTA.begin();
    Serial.println("WiFi connected, OTA enabled");
  } else {
    wifiConnected = false; 
    Serial.println("WiFi failed, OTA disabled");
  }
  display.clearDisplay();
  display.display();
}
