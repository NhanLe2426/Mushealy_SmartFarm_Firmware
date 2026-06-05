#include "global.h"

QueueHandle_t qSensorData = NULL;
SemaphoreHandle_t xMutexI2C = NULL;
EventGroupHandle_t egDeviceControl = NULL;

volatile bool isPumpOverrideActive = false;
volatile TickType_t pumpOverrideEndTime = 0;
volatile bool isPumpCurrentlyOn = false;
volatile bool forcePublish = false;

// This func will be called in the setup() in main.cpp to allocate memory
void initGlobal_RTOS_Objects() {
    // Create a queue which can store one SensorData element
    // (use xQueueOverwrite in sender tasks to update to the latest version)
    qSensorData = xQueueCreate(1, sizeof(SensorData));

    // Create Mutex Semaphore for I2C bus protection
    xMutexI2C = xSemaphoreCreateMutex();

    // Create Event Group for pump control signaling
    egDeviceControl = xEventGroupCreate();
}