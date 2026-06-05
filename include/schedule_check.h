#ifndef __SCHEDULE_CHECK_H__
#define __SCHEDULE_CHECK_H__

#include <Arduino.h>
#include <time.h>
#include "global.h"
#include "config.h"

// FreeRTOS Task to handle scheduled watering
void taskScheduleCheck(void *pvParameters);

#endif