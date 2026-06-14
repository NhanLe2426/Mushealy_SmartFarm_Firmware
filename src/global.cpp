#include "global.h"

QueueHandle_t qSensorData = NULL;
SemaphoreHandle_t xMutexI2C = NULL;
EventGroupHandle_t egDeviceControl = NULL;

volatile bool isPumpOverrideActive = false;
volatile TickType_t pumpOverrideEndTime = 0;
volatile bool isPumpCurrentlyOn = false;
volatile bool isLightCurrentlyOn = false;
volatile bool forcePublish = false;

// Initialize with default safe values
volatile int autoWaterHour = 16;           // Default: 15:xx PM
volatile int autoWaterMinute = 29;          // Default: xx:00
volatile int soilMoistureThreshold = 40;   // Default: 40% moisture

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