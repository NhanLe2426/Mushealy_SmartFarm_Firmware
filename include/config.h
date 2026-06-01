#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>

// ---------------------------------------------------------
// Yolo:Bit (ESP32) Pin Mapping
// ---------------------------------------------------------

// I2C Pins for DHT20 Sensor and LCD Display
const int SDA_PIN = 21;
const int SCL_PIN = 22;

// Analog pin for Soil Moisture Sensor 
const int SOIL_MOISTURE_PIN = 33;               // P1

// Analog pin for Light Sensor 
const int LIGHT_SENSOR_PIN = 27;                // P2

// Digital pin for Water Pump Relay 
const int PUMP_RELAY_PIN = 26;                  // P10.P13

// ---------------------------------------------------------
// System Parameters & Thresholds
// ---------------------------------------------------------

// Thresholds for triggering automated actions
const int THRESHOLD_SOIL_MOISTURE = 1500;
const int THRESHOLD_LIGHT = 2000;

// Timing configurations (in milliseconds)
const uint32_t TIME_PUMP_ACTIVE_MS = 3000;   // How long the pump stays on
const uint32_t TIME_RECOVERING_MS = 5000;    // Wait time for water to absorb into soil

// P0 = 32
// P1 = 33
// P14.P15 = 19
// P10.P13 = 26
// P3.P6 = 2
// P2 = 27
// P16.P12 = 5 --> Not for sensor

#endif