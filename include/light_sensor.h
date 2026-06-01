#ifndef __LIGHT_SENSOR_H__
#define __LIGHT_SENSOR_H__

#include <Arduino.h>
#include "config.h"
#include "global.h"

// Function to initialize the light sensor pin
void initLightSensor();

// Function to read the analog value from the light sensor
// Parameter is passed by reference to update the external variable
void readLightIntensity(int &lightIntensity);

#endif