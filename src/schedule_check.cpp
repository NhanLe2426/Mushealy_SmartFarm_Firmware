#include "schedule_check.h"

void taskScheduleCheck(void *pvParameters) {
    Serial.println("[Schedule Task] Started. Waiting for time sync...");
    
    // Wait for NTP time to be updated (year will be > 1970)
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    Serial.println("[Schedule Task] Time synchronized successfully.");

    // Infinite Loop for Network Handling
    while (1) {
        // --- 1. TIME-BASED AUTOMATION ---
        if (getLocalTime(&timeinfo)) {
            int currentHour = timeinfo.tm_hour;
            int currentMinute = timeinfo.tm_min;

            // Check against dynamically updated schedule variables
            if (currentHour == autoWaterHour && currentMinute == autoWaterMinute) {
                if (!isPumpOverrideActive) {
                    Serial.printf("[Schedule Task] It is %02d:%02d. Triggering scheduled watering.\n", currentHour, currentMinute);
                    xEventGroupSetBits(egDeviceControl, EVENT_PUMP_ON);
                }
                vTaskDelay(pdMS_TO_TICKS(60000));
                continue; 
            }
        }

        // --- 2. SENSOR-BASED AUTOMATION (Edge Threshold Logic) ---
        SensorData currentData;
        // Peek at the latest sensor data without removing it from the queue
        if (xQueuePeek(qSensorData, &currentData, 0) == pdTRUE) {
            
            // Trigger pump if soil is too dry, provided manual mode is not active and pump is currently off
            if (currentData.soilMoisture < soilMoistureThreshold) {
                if (!isPumpOverrideActive && !isPumpCurrentlyOn) {
                    Serial.printf("[Schedule Task] Soil moisture (%d) below threshold (%d). Triggering pump.\n", 
                                  currentData.soilMoisture, soilMoistureThreshold);
                    xEventGroupSetBits(egDeviceControl, EVENT_PUMP_ON);
                    
                    // Wait for the recovering time before checking again to avoid flooding
                    vTaskDelay(pdMS_TO_TICKS(TIME_RECOVERING_MS));
                }
            }
        }
        
        // Check time every 10 seconds
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}