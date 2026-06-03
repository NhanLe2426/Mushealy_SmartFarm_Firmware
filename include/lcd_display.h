#ifndef __LCD_DISPLAY_H__
#define __LCD_DISPLAY_H__

#include <Arduino.h>
#include "global.h"
#include "LiquidCrystal_I2C.h"

// Function to initialize the LCD hardware
void initLCD();

// FreeRTOS Task to handle LCD rendering independently
void taskDisplayLCD(void *pvParameters);

#endif