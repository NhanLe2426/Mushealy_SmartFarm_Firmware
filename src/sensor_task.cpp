#include "sensor_task.h"

void taskSensorReading(void *pvParameters) {
    // Initialize all hardware sensors
    initDHT20();
    initLightSensor();
    initSoilSensor();

    // Create a local structure to hold the reading
    SensorData currentData;

    Serial.println("[Sensor Task] Initialization complete! Starting data acquisition...");

    // FreeRTOS Task lifecycle
    while (1) {
        // Read data from DHT20 (using I2C Mutex internally)
        readDHT20(currentData.temperature, currentData.humidity);

        // Read analog values from Soil and Light sensors
        readSoilMoisture(currentData.soilMoisture);
        readLightIntensity(currentData.lightIntensity);

        // Print to Serial Monitor for debugging
        Serial.printf("[Sensor Task] Temp: %.1f °C | Hum: %.1f %% | Soil: %d %% | Light: %d lx\n",
                      currentData.temperature, 
                      currentData.humidity, 
                      currentData.soilMoisture, 
                      currentData.lightIntensity);

        // Send the packed data to the FreeRTOS Queue.
        // We use xQueueOverwrite because the queue size is 1. 
        // It ensures the queue always holds the most recent data.
        xQueueOverwrite(qSensorData, &currentData);

        // Delay for a specific period before the next reading
        // Using pdMS_TO_TICKS ensures accurate timing in the RTOS environment
        vTaskDelay(pdMS_TO_TICKS(10000)); // Read every 10 seconds
    }
}