#include "pump_relay.h"

void initPump() {
    // Configure the relay pin and ensure the pump is OFF at startup
    pinMode(PUMP_RELAY_PIN, OUTPUT);
    digitalWrite(PUMP_RELAY_PIN, LOW);
    isPumpCurrentlyOn = false;              // initial state
}

void taskPumpControl(void *pvParameters) {
    // Initialization
    initPump();
    Serial.println("[Pump Task] Started and waiting for control events...");

    // FreeRTOS Task lifecycle
    while (1) {
        // Wait indefinitely for either EVENT_PUMP_ON or EVENT_PUMP_OFF to be set.
        // This function blocks the task (consuming 0 CPU time) until an event occurs.
        EventBits_t uxBits = xEventGroupWaitBits(
            egPumpControl,                  // The event group handle
            EVENT_PUMP_ON | EVENT_PUMP_OFF, // The bits to wait for
            pdTRUE,                         // Clear the bits automatically after reading
            pdFALSE,                        // Wait for ANY of the bits (not ALL)
            portMAX_DELAY                   // Wait forever
        );

        // Check which event triggered the task
        if ((uxBits & EVENT_PUMP_ON) != 0) {
            Serial.println("[Pump Task] Event received: Turn ON pump.");
            
            // Activate the pump
            digitalWrite(PUMP_RELAY_PIN, HIGH);
            isPumpCurrentlyOn = true;
            forcePublish = true;                    // Trigger instant update to Dashboard
            
            // SMART DELAY: Wait for a manual OFF command, but timeout after TIME_PUMP_ACTIVE_MS
            EventBits_t offBits = xEventGroupWaitBits(
                egPumpControl, 
                EVENT_PUMP_OFF, 
                pdTRUE, 
                pdFALSE, 
                pdMS_TO_TICKS(TIME_PUMP_ACTIVE_MS) // Auto-timeout if user forgets to turn off the pump
            );

            // Check WHY the smart delay finished
            if ((offBits & EVENT_PUMP_OFF) != 0) {
                Serial.println("[Pump Task] Pump OFF (Interrupted by Manual Switch).");
            } else {
                Serial.println("[Pump Task] Pump OFF (Auto Safety Timeout).");
            }
            
            // Turn off the pump automatically after watering
            Serial.println("[Pump Task] Watering finished. Turn OFF pump.");
            digitalWrite(PUMP_RELAY_PIN, LOW);
            isPumpCurrentlyOn = false;
            forcePublish = true;                    // Trigger instant update to Dashboard
            
            // Wait for water to absorb into the soil before accepting new ON commands
            // This prevents continuous watering that could flood the farm
            Serial.println("[Pump Task] Recovering state... waiting for water to absorb.");
            vTaskDelay(pdMS_TO_TICKS(TIME_RECOVERING_MS));
            Serial.println("[Pump Task] Ready for next operation.");
        }
        else if ((uxBits & EVENT_PUMP_OFF) != 0) {
            // Manual override or safety cutoff
            Serial.println("[Pump Task] Event received: Turn OFF pump (Manual override).");
            digitalWrite(PUMP_RELAY_PIN, LOW);
            isPumpCurrentlyOn = false;
            forcePublish = true;                    // Trigger instant update to Dashboard
        }
    }
}