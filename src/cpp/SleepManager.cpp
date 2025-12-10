#include "SleepManager.h"

// ----------------------------------------------------------------------
// -------------------- DEEP SLEEP FUNCTIONS ----------------------------
// ----------------------------------------------------------------------
void checkWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
    Serial.println("\n*** Woke up from Deep Sleep due to MPU Motion! ***");
  } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("\nWoke up from Deep Sleep by Timer");
  } else {
    Serial.println("\nNormal boot or reset.");
  }

  esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  Serial.printf("EXT1 wakeup configured for GPIO %d (HIGH level)\n", MPU_INTERRUPT_PIN);
}

void enterDeepSleep() {
  Serial.println("\n=== ENTERING DEEP SLEEP ===");
  Serial.println("Device has been still for 30 seconds");
  Serial.println("Will wake on motion (GPIO27 HIGH)");
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println(F("Sleep"));
  display.setTextSize(1);
  display.setCursor(10, 40);
  display.println(F("Move to wake"));
  display.display();
  delay(2000);
  
  if (wifiConnected) {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }

  display.oled_command(SH110X_DISPLAYOFF);
  
  if (mpuOk) {
    mpuReadRegister(MPU6050_RA_INT_STATUS);
    mpuWriteRegister(MPU6050_RA_INT_ENABLE, 0x40);
    delay(50);
    
    Serial.printf("INT pin state before sleep: %d\n", digitalRead(MPU_INTERRUPT_PIN));
  }
  
  Serial.println("Entering deep sleep NOW...");
  Serial.flush();
  
  esp_deep_sleep_start();
}

void handleSleepMode(unsigned long now) {
    if (now - lastMovementTime >= DEEP_SLEEP_TIMEOUT) {
        Serial.printf("No movement for %lu ms, preparing for sleep\n", DEEP_SLEEP_TIMEOUT);
        enterDeepSleep();
    } else {
        static unsigned long lastCountdown = 0;
        if (now - lastCountdown >= 5000) {
            lastCountdown = now;
            unsigned long remaining = DEEP_SLEEP_TIMEOUT - (now - lastMovementTime);
            Serial.printf("Sleep countdown: %lu seconds remaining\n", remaining / 1000);
        }
    }
}
