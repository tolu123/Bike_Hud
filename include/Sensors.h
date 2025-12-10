#pragma once
#include "BikeHudGlobals.h"

// MPU raw register access
uint8_t mpuReadRegister(uint8_t reg);
void    mpuWriteRegister(uint8_t reg, uint8_t value);

// MPU configuration & lean
void enableMpuMotionInterrupt();
void calibrateLeanOffset();
void updateLean(float &leanDeg);

// BME680 + env
void updateEnvFromBME();

// Battery
int readBatteryPercent();
