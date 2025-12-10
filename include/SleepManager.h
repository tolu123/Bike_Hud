#pragma once
#include "Sensors.h"  // also brings in BikeHudGlobals

void checkWakeupReason();
void enterDeepSleep();
void handleSleepMode(unsigned long now);
