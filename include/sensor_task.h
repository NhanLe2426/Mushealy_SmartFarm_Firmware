#ifndef __SENSOR_TASK_H__
#define __SENSOR_TASK_H__

#include <Arduino.h>
#include "global.h"
#include "config.h"

#include "dht20_sensor.h"
#include "light_sensor.h"
#include "soil_mois_sensor.h"

// Task function to read all sensors periodically
void taskSensorReading(void *pvParameters);

#endif