#ifndef __PUMP_RELAY_H__
#define __PUMP_RELAY_H__

#include <Arduino.h>
#include "config.h"
#include "global.h"

// Function to initialize the pump relay pin
void initPump();

// FreeRTOS Task to handle pump control independently
void taskPumpControl(void *pvParameters);

#endif