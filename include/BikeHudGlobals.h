#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <esp_sleep.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <math.h>

// ---------- I2C Address and Register Definitions ----------
#define MPU6050_ADDRESS_AD0_LOW  0x68
#define MPU6050_RA_PWR_MGMT_1    0x6B
#define MPU6050_RA_INT_ENABLE    0x38
#define MPU6050_RA_INT_STATUS    0x3A
#define MPU6050_RA_MOT_THR       0x1F
#define MPU6050_RA_MOT_DUR       0x20
#define MPU6050_RA_MOT_DETECT_CTRL 0x69
#define MPU6050_RA_INT_PIN_CFG   0x37

// ---------- WAKEUP PIN CONFIGURATION ----------
#define MPU_INTERRUPT_PIN 27 
#define WAKEUP_PIN_BITMASK (1ULL << MPU_INTERRUPT_PIN)

// ---------- WIFI + OTA ----------
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern const char* OTA_HOSTNAME;
extern bool wifiConnected;

// ---------- DISPLAY ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
extern Adafruit_SH1106G display;
extern bool displayOn;

// ---------- MPU6050 ----------
extern Adafruit_MPU6050 mpu;
extern bool  mpuOk;
extern float leanOffset;
extern float filteredLean;
extern const uint8_t MPU_MOTION_THRESHOLD;

// ---------- BME680 ----------
extern Adafruit_BME680 bme;
extern bool  bmeOk;
extern float lastTempC;
extern float lastHum;
extern float lastAQI;
extern float lastCO2ppm;

// ---------- BATTERY ----------
#define BATTERY_PIN 34      
extern int   batteryPercent;

// ---------- TIMING & SLEEP LOGIC ----------
extern unsigned long lastLeanUpdate;
extern unsigned long lastEnvUpdate;
extern unsigned long lastBatteryRead;
extern unsigned long lastMovementTime;

extern const unsigned long leanInterval;
extern const unsigned long envInterval;
extern const unsigned long batteryInterval;
extern const unsigned long DEEP_SLEEP_TIMEOUT;
