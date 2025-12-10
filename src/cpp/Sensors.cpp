#include "Sensors.h"

// ----------------------------------------------------------------------
// -------------------- MPU RAW I2C REGISTER ACCESS ---------------------
// ----------------------------------------------------------------------
uint8_t mpuReadRegister(uint8_t reg) {
    Wire.beginTransmission(MPU6050_ADDRESS_AD0_LOW);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)MPU6050_ADDRESS_AD0_LOW, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}

void mpuWriteRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(MPU6050_ADDRESS_AD0_LOW);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission(true);
}

// ----------------------------------------------------------------------
// -------------------- MPU INTERRUPT SETUP FUNCTION --------------------
// ----------------------------------------------------------------------
void enableMpuMotionInterrupt() {
    Serial.println("Configuring MPU6050 Motion Interrupt...");
    
    uint8_t pwrMgmt = mpuReadRegister(MPU6050_RA_PWR_MGMT_1);
    pwrMgmt &= ~0x40;
    mpuWriteRegister(MPU6050_RA_PWR_MGMT_1, pwrMgmt);
    delay(100);
    
    mpuWriteRegister(MPU6050_RA_INT_PIN_CFG, 0x20); 
    delay(10);
    
    mpuWriteRegister(MPU6050_RA_MOT_THR, MPU_MOTION_THRESHOLD);
    Serial.printf("Motion threshold set to: %d (= %dmg)\n", MPU_MOTION_THRESHOLD, MPU_MOTION_THRESHOLD * 2);
    delay(10);
    
    mpuWriteRegister(MPU6050_RA_MOT_DUR, 1);
    delay(10);
    
    mpuWriteRegister(MPU6050_RA_MOT_DETECT_CTRL, 0x15); 
    delay(10);
    
    mpuWriteRegister(MPU6050_RA_INT_ENABLE, 0x40); 
    delay(10);
    
    uint8_t intStatus = mpuReadRegister(MPU6050_RA_INT_STATUS);
    Serial.printf("Initial INT_STATUS: 0x%02X\n", intStatus);
    
    Serial.println("--- MPU6050 Configuration ---");
    Serial.printf("PWR_MGMT_1: 0x%02X\n", mpuReadRegister(MPU6050_RA_PWR_MGMT_1));
    Serial.printf("INT_PIN_CFG: 0x%02X\n", mpuReadRegister(MPU6050_RA_INT_PIN_CFG));
    Serial.printf("INT_ENABLE: 0x%02X\n", mpuReadRegister(MPU6050_RA_INT_ENABLE));
    Serial.printf("MOT_THR: 0x%02X\n", mpuReadRegister(MPU6050_RA_MOT_THR));
    Serial.printf("MOT_DUR: 0x%02X\n", mpuReadRegister(MPU6050_RA_MOT_DUR));
    Serial.printf("MOT_DETECT_CTRL: 0x%02X\n", mpuReadRegister(MPU6050_RA_MOT_DETECT_CTRL));
    Serial.println("-----------------------------");
    
    Serial.printf("MPU Motion Interrupt enabled on GPIO %d\n", MPU_INTERRUPT_PIN);
}

// ----------------------------------------------------------------------
// -------------------- CALIBRATE LEAN ----------------------------------
// ----------------------------------------------------------------------
void calibrateLeanOffset() {
  if (!mpuOk) {
    leanOffset = 0.0;
    return;
  }

  const int samples = 200;
  float sum = 0;

  for (int i = 0; i < samples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    // CORRECTED: Calculate lean based on X-axis tilt (bike leaning left/right)
    float leanRad = atan2(a.acceleration.x, a.acceleration.y);
    float leanDeg = leanRad * 180.0 / PI;
    
    sum += leanDeg;
    delay(10);
  }

  leanOffset = sum / samples;
  filteredLean = 0.0;
  Serial.printf("Lean calibration complete. Offset: %.2f degrees\n", leanOffset);
}

// ----------------------------------------------------------------------
// ----------------------- UPDATE LEAN ----------------------------------
// ----------------------------------------------------------------------
void updateLean(float &leanDeg) {
  if (!mpuOk) {
    leanDeg = 0.0;
    filteredLean = 0.0;
    return;
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Movement detection - check total acceleration magnitude
  float accMag = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));
  
  if (fabs(accMag - 9.81f) > 0.5f) {
    lastMovementTime = millis();
  }

  // CORRECTED: Calculate lean angle based on X-axis tilt
  float leanRad = atan2(a.acceleration.x, a.acceleration.y);
  leanDeg = leanRad * 180.0 / PI;
  
  // Apply calibration offset
  leanDeg -= leanOffset;

  // Dead zone for small jitters
  if (fabs(leanDeg) < 1.0) {
    leanDeg = 0.0;
  }
  
  // Smooth filter (85% old + 15% new)
  filteredLean = (filteredLean * 0.85f) + (leanDeg * 0.15f);
  
  // Debug output
  Serial.printf("Accel X:%.2f Y:%.2f Z:%.2f | Raw Lean:%.1f° | Filtered:%.1f°\n", 
                a.acceleration.x, a.acceleration.y, a.acceleration.z, leanDeg, filteredLean);
}

// ----------------------------------------------------------------------
// ----------------- UPDATE ENVIRONMENT FROM BME680 ---------------------
// ----------------------------------------------------------------------
void updateEnvFromBME() {
  if (!bmeOk) return;

  if (!bme.performReading()) return;

  lastTempC = bme.temperature;
  lastHum   = bme.humidity;

  float gasOhms = bme.gas_resistance;
  float gasK = gasOhms / 1000.0f; 

  float humidityScore = 100.0f - fabs(lastHum - 40.0f) * 2.0f;
  if (humidityScore < 0) humidityScore = 0;
  if (humidityScore > 100.0f) humidityScore = 100.0f;

  float gasScore = (gasK - 10.0f) * (100.0f / (80.0f)); 
  if (gasScore < 0) gasScore = 0;
  if (gasScore > 100.0f) gasScore = 100.0f;

  float iaqScore = (humidityScore * 0.25f) + (gasScore * 0.75f);

  float aqi = 500.0f - (iaqScore * 5.0f);
  if (aqi < 0) aqi = 0;
  if (aqi > 500.0f) aqi = 500.0f;
  lastAQI = aqi;

  float co2 = 400.0f + ((100.0f - iaqScore) / 100.0f) * 1600.0f;
  if (co2 < 400.0f) co2 = 400.0f;
  if (co2 > 2000.0f) co2 = 2000.0f;
  lastCO2ppm = co2;
}

// ----------------------------------------------------------------------
// -------------------- READ BATTERY PERCENT ----------------------------
// ----------------------------------------------------------------------
int readBatteryPercent() {
  // Take multiple samples for stable reading
  int raw = 0;
  for (int i = 0; i < 10; i++) {
    raw += analogRead(BATTERY_PIN);
    delay(10);
  }
  raw /= 10;
  
  float v = (raw / 4095.0f) * 3.3f;
  v *= 2.0f; 

  // Adjusted voltage-to-percentage mapping for 500mAh LiPo
  float pct;
  
  if (v >= 4.15f) {
    pct = 100.0f;  // 4.15V+ = 100%
  } else if (v >= 4.0f) {
    pct = 90.0f + ((v - 4.0f) / 0.15f) * 10.0f;  // 4.0-4.15V = 90-100%
  } else if (v >= 3.85f) {
    pct = 70.0f + ((v - 3.85f) / 0.15f) * 20.0f; // 3.85-4.0V = 70-90%
  } else if (v >= 3.75f) {
    pct = 40.0f + ((v - 3.75f) / 0.10f) * 30.0f; // 3.75-3.85V = 40-70%
  } else if (v >= 3.6f) {
    pct = 15.0f + ((v - 3.6f) / 0.15f) * 25.0f;  // 3.6-3.75V = 15-40%
  } else if (v >= 3.3f) {
    pct = 5.0f + ((v - 3.3f) / 0.3f) * 10.0f;    // 3.3-3.6V = 5-15%
  } else if (v >= 3.0f) {
    pct = ((v - 3.0f) / 0.3f) * 5.0f;            // 3.0-3.3V = 0-5%
  } else {
    pct = 0.0f;  // Below 3.0V = 0%
  }

  int p = (int)(pct + 0.5f);
  if (p < 0) p = 0;
  if (p > 100) p = 100;

  // Debug output
  Serial.printf("Battery - Raw: %d, Voltage: %.2fV, Percent: %d%%\n", raw, v, p);

  return p;
}
