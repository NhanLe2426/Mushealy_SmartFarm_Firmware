#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

/// Sensor Data Structure for Smart Farm
struct SensorData
{
    float temperature;
    float humidity;
    int soilMoisture;
    int lightIntensity;
};

// RTOS HANDLES
extern QueueHandle_t qSensorData;           // The queue contains sensor data
extern SemaphoreHandle_t xMutexI2C;         // Mutex to protect the I2C bus (DHT20 & LCD)
extern EventGroupHandle_t egDeviceControl;    // Event group to trigger pump and light actions

// Override mechanism flags
extern volatile bool isPumpOverrideActive;
extern volatile TickType_t pumpOverrideEndTime;
extern volatile bool isPumpCurrentlyOn;             // The physical state of the pump
extern volatile bool isLightCurrentlyOn;            // The physical state of the light
extern volatile bool forcePublish;                  // Flag to trigger instant MQTT publish

// Edge Computing variables for dynamic automation
extern volatile int autoWaterHour;
extern volatile int autoWaterMinute;
extern volatile int soilMoistureThreshold;

// Event Group Bits for Pump Control
#define EVENT_PUMP_ON   (1 << 0)
#define EVENT_PUMP_OFF  (1 << 1)
// Event Group Bits for LED Control
#define EVENT_LIGHT_ON  (1 << 2)
#define EVENT_LIGHT_OFF (1 << 3)

// Define override duration (e.g., 1 minutes = 60000 ms)
#define OVERRIDE_DURATION_MS 60000

// Function prototype to initialize all RTOS objects
void initGlobal_RTOS_Objects();

#endif