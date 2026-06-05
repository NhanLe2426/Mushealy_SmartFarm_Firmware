#ifndef __LIGHT_CONTROL_H__
#define __LIGHT_CONTROL_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "global.h"
#include "config.h"

// Initialize the NeoPixel hardware
void initLight();

// Set the color for all LEDs on the module
void setLightColor(uint8_t r, uint8_t g, uint8_t b);

// FreeRTOS Task to handle light control independently
void taskLightControl(void *pvParameters);

#endif