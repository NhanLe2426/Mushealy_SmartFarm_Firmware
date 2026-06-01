#ifndef __SOIL_MOIS_SENSOR_H__
#define __SOIL_MOIS_SENSOR_H__

#include <Arduino.h>
#include "config.h"
#include "global.h"

// Function to initialize the soil moisture sensor pin
void initSoilSensor();

// Function to read the analog value from the soil moisture sensor
// Parameter is passed by reference to update the external variable
void readSoilMoisture(int &moisture);

#endif