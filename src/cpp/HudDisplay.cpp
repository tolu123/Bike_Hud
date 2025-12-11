#include "HudDisplay.h"

// ----------------------------------------------------------------------
// -------------------- DISPLAY HUD (Final Layout) ----------------------
// ----------------------------------------------------------------------
void showHud(float leanDeg, float tempC, float hum, float aqi, float co2ppm, int battPct) {
  display.clearDisplay();
  
  // -------- TOP LEFT/CENTER: TEMP, HUMIDITY, and BATTERY --------
  int envX = 0;
  int envY = 0;

  display.setTextSize(1);
  display.setCursor(envX, envY);
  display.print("T");
  display.setTextSize(2);
  display.setCursor(envX + 8, envY);
  display.print((int)tempC);
  display.setTextSize(1);
  display.print((char)248); 
  
  display.setTextSize(1);
  display.setCursor(envX + 45, envY);
  display.print("H");
  display.setTextSize(2);
  display.setCursor(envX + 53, envY);
  display.print((int)hum);
  display.setTextSize(1);
  display.print("%");
  
  int battX = 0;
  int battY = 20;
  display.setTextSize(1);
  display.setCursor(battX, battY);
  display.print("B:");
  display.print(battPct);
  display.print("%");

  // -------- TOP RIGHT: LEAN ANGLE --------
  int angleX = 100;
  int angleY = 5;
  
  display.setTextSize(2);
  display.setCursor(angleX, angleY);
  display.print((int)leanDeg);
  display.setTextSize(1);
  display.print((char)248);

  // -------- MIDDLE: HORIZONTAL LEAN BAR --------
  int barX      = 5;
  int barY      = 30;
  int barWidth  = SCREEN_WIDTH - 10;
  int barHeight = 10;
  int barCenterX = barX + barWidth / 2;

  float clampedLean = leanDeg;
  if (clampedLean > 45.0f) clampedLean = 45.0f;
  if (clampedLean < -45.0f) clampedLean = -45.0f;

  float absLean = fabs(clampedLean);
  float frac = absLean / 45.0f;
  int halfWidth = barWidth / 2;
  int leanFillWidth = (int)(frac * halfWidth); 

  if (leanFillWidth > 0) {
    if (clampedLean > 0.0f) {
      int xStart = barCenterX;
      display.fillRect(xStart, barY + 1, leanFillWidth, barHeight - 2, SH110X_WHITE);
    } else if (clampedLean < 0.0f) {
      int xStart = barCenterX - leanFillWidth;
      display.fillRect(xStart, barY + 1, leanFillWidth, barHeight - 2, SH110X_WHITE);
    }
  }

  display.setTextSize(1);
  if (clampedLean < -5.0f) { 
      display.setCursor(barCenterX - 15, barY - 10);
      display.print("LEFT");
  } else if (clampedLean > 5.0f) { 
      display.setCursor(barCenterX - 15, barY - 10); 
      display.print("RIGHT");
  }

  // -------- BOTTOM LEFT: AQI and WIFI Status --------
  int aqiX = 0;
  int aqiY = 48; 
  
  display.setTextSize(1);
  display.setCursor(aqiX, aqiY);
  display.print("AQI");
  
  display.setTextSize(2);
  display.setCursor(aqiX + 24, aqiY - 2); 
  display.print((int)aqi);
  
  if (!wifiConnected) {
    int wifiX = aqiX + 55; 
    int wifiY = aqiY + 2;
    display.drawPixel(wifiX + 4, wifiY + 6, SH110X_WHITE);
    display.drawFastHLine(wifiX + 3, wifiY + 4, 3, SH110X_WHITE);
    display.drawPixel(wifiX + 1, wifiY + 2, SH110X_WHITE);
    display.drawPixel(wifiX + 7, wifiY + 2, SH110X_WHITE);
    display.drawLine(wifiX + 1, wifiY + 1, wifiX + 7, wifiY + 7, SH110X_WHITE);
  }

  // -------- BOTTOM RIGHT: CO2 --------
  int co2X = 80; 
  int co2Y = 45; 
  int co2BarWidth = SCREEN_WIDTH - co2X; 

  display.setTextSize(1);
  display.setCursor(co2X, co2Y);
  display.print("CO2");

  display.setTextSize(1);
  display.setCursor(co2X + 24, co2Y); 
  display.print((int)co2ppm);

  int barOutlineX = co2X;
  int barOutlineY = co2Y + 12; 
  int barOutlineH = 7;
  display.drawRect(barOutlineX, barOutlineY, co2BarWidth, barOutlineH, SH110X_WHITE);

  float co2Min = 400.0f;
  float co2Max = 2000.0f;
  
  float co2Clamped = co2ppm;
  if (co2Clamped < co2Min) co2Clamped = co2Min;
  if (co2Clamped > co2Max) co2Clamped = co2Max;
  
  float co2Range = co2Max - co2Min; 
  float co2Frac = (co2Clamped - co2Min) / co2Range; 
  
  int co2FillWidth = (int)(co2Frac * (co2BarWidth - 2)); 

  if (co2FillWidth > 0) {
    display.fillRect(barOutlineX + 1, barOutlineY + 1, co2FillWidth, barOutlineH - 2, SH110X_WHITE);
  }
  
  display.display();
}
