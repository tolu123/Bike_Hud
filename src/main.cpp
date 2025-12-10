#include <Arduino.h>
#include "BikeHudGlobals.h"
#include "Sensors.h"
#include "SleepManager.h"
#include "WifiOta.h"
#include "HudDisplay.h"

void setup() {
  Serial.begin(115200);
  delay(500);

  checkWakeupReason();

  pinMode(MPU_INTERRUPT_PIN, INPUT_PULLDOWN);

  Wire.begin(21, 22);
  Wire.setClock(400000);

  // ---- DISPLAY INIT ----
  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 allocation failed"));
  } else {
    display.setTextColor(SH110X_WHITE);

    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(0, 5);
    display.println(F("BIKE"));
    display.setCursor(0, 30);
    display.println(F("HUD"));
    display.display();
    delay(1000);

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println(F("Bike HUD"));
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(F("Initializing sensors..."));
    display.setCursor(0, 30);
    display.println(F("V 1.3"));
    display.display();
    delay(1000);
  }

  // ---- MPU6050 INIT ----
  mpuOk = true;
  if (!mpu.begin(0x68)) {
    if (!mpu.begin(0x69)) {
      mpuOk = false;
      Serial.println("Failed to find MPU6050");
    }
  }

  if (mpuOk) {
    Serial.println("MPU6050 found!");
    
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    enableMpuMotionInterrupt(); 
    
    lastMovementTime = millis();
  }

  // ---- BME680 INIT ----
  bmeOk = true;
  if (!bme.begin(0x76)) {
    if (!bme.begin(0x77)) {
      bmeOk = false;
      Serial.println("Failed to find BME680");
    }
  }

  if (bmeOk) {
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); 
  }

  // ---- WIFI + OTA INIT ----
  setupWiFiAndOTA(); 

  // ---- CALIBRATION SCREEN ----
  if (mpuOk) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Hold bike upright"));
    display.println(F("Calibrating lean..."));
    display.display();

    calibrateLeanOffset();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Calibration done"));
    display.println(F("Ride safe!"));
    display.display();
    delay(1000);
  }

  batteryPercent = readBatteryPercent();
}

void loop() {
  unsigned long now = millis();
  
  if (mpuOk) {
    uint8_t intStatus = mpuReadRegister(MPU6050_RA_INT_STATUS);
    if (intStatus & 0x40) {
      Serial.println("Motion detected by MPU!");
      lastMovementTime = now;
    }
  }
  
  if (mpuOk) {
    handleSleepMode(now);
  }

  if (wifiConnected) {
    ArduinoOTA.handle();
  }

  if (now - lastEnvUpdate >= envInterval) {
    lastEnvUpdate = now;
    updateEnvFromBME();
  }

  if (now - lastBatteryRead >= batteryInterval) {
    lastBatteryRead = now;
    batteryPercent  = readBatteryPercent();
  }

  if (now - lastLeanUpdate >= leanInterval) {
    lastLeanUpdate = now;

    float leanDeg = 0.0;
    updateLean(leanDeg);  
    
    showHud(filteredLean, lastTempC, lastHum, lastAQI, lastCO2ppm, batteryPercent);
  }
}
