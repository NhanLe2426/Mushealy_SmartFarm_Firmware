#ifndef __DHT20_SENSOR_H__
#define __DHT20_SENSOR_H__

#include <Arduino.h>
#include "global.h"
#include "DHT20.h"

// Function to initialize the DHT20 sensor on the I2C bus
void initDHT20();

// Function to read sensor data safely using I2C Mutex
// Parameters are passed by reference to update the external variables
void readDHT20(float &temperature, float &humidity);

#endif