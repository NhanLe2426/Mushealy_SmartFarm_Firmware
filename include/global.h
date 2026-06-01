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
extern EventGroupHandle_t egPumpControl;    // Event group to trigger water pump actions

// Event Group Bits for Pump Control
#define EVENT_PUMP_ON  (1 << 0)
#define EVENT_PUMP_OFF (1 << 1)

// Function prototype to initialize all RTOS objects
void initGlobal_RTOS_Objects();

#endif