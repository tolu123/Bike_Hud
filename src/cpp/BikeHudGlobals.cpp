#include "BikeHudGlobals.h"

// ---------- WIFI + OTA ----------
const char* WIFI_SSID     = "Vaibhav";     
const char* WIFI_PASSWORD = "vaibhav07";  
const char* OTA_HOSTNAME  = "esp32-bike-hud";
bool wifiConnected = false; 

// ---------- DISPLAY ----------
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool displayOn = true; 

// ---------- MPU6050 ----------
Adafruit_MPU6050 mpu;
bool  mpuOk        = false;
float leanOffset   = 0.0;
float filteredLean = 0.0;
const uint8_t MPU_MOTION_THRESHOLD = 7;

// ---------- BME680 ----------
Adafruit_BME680 bme;
bool  bmeOk        = false;
float lastTempC    = 0.0;
float lastHum      = 0.0;
float lastAQI      = 0.0;
float lastCO2ppm   = 400.0;

// ---------- BATTERY ----------
int   batteryPercent = 100;

// ---------- TIMING & SLEEP LOGIC ----------
unsigned long lastLeanUpdate   = 0;
unsigned long lastEnvUpdate    = 0;
unsigned long lastBatteryRead  = 0;
unsigned long lastMovementTime = 0; 

const unsigned long leanInterval      = 100;
const unsigned long envInterval       = 2000;
const unsigned long batteryInterval   = 10000;
const unsigned long DEEP_SLEEP_TIMEOUT = 30000;

// ---------- PREFS ----------
Preferences prefs;   // ⬅ NEW



// ---------- FIRMWARE VERSION ----------
const char* FW_VERSION = "1.2.1";   // <-- bump this when you do a new release