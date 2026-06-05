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
        // Get current local time
        if (getLocalTime(&timeinfo)) {
            int currentHour = timeinfo.tm_hour;
            int currentMinute = timeinfo.tm_min;

            // 1. Check if it is the scheduled time
            if (currentHour == SCHEDULED_HOUR && currentMinute == SCHEDULED_MINUTE) {
                
                // 2. Check the manual override firewall
                if (!isPumpOverrideActive) {
                    Serial.printf("[Schedule Task] It is %02d:%02d. Triggering scheduled watering.\n", currentHour, currentMinute);
                    xEventGroupSetBits(egDeviceControl, EVENT_PUMP_ON);
                } else {
                    Serial.println("[Schedule Task] Scheduled watering bypassed due to active manual override.");
                }

                // Delay for exactly 60 seconds to avoid triggering multiple times within the same minute
                vTaskDelay(pdMS_TO_TICKS(60000));
                continue; 
            }
        }
        
        // Check time every 10 seconds
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}