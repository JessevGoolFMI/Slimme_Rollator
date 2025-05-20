#ifndef SENSORTASK_H
#define SENSORTASK_H

#include <Arduino.h>
#include "freertos/semphr.h"

void initSensorTask();
extern float afstandR, afstandL;
extern SemaphoreHandle_t sensorMutex;

#endif
