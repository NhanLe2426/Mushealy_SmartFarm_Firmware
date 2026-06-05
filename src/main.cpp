#include <Arduino.h>

// Include global RTOS objects and configurations
#include "global.h"

// Include task modules
#include "sensor_task.h"
#include "lcd_display.h"
#include "schedule_check.h"
#include "pump_relay.h"
#include "light_control.h"
#include "mqtt_connection.h"

void setup() {
    // Initialize Serial communication for debugging
    Serial.begin(115200);

    Serial.println("\n=== MUSHEALY SMARTFARM BOOT ===");

    // Initialize RTOS communication objects
    initGlobal_RTOS_Objects();

    // ------ Create FreeRTOS Tasks ------

    // Task Sensor Reading: Responsible for acquiring data from DHT20, soil and light sensors.
    xTaskCreate(taskSensorReading, "Sensor Reading Task", 4096, NULL, 1, NULL);

    // Task LCD Display: Responsible for rendering sensor data to the LCD safely using I2C Mutex.
    xTaskCreate(taskDisplayLCD, "LCD Display Task", 4096, NULL, 1, NULL);

    // Task Schedule Check: Responsible for checking schedule to control the pump
    xTaskCreate(taskScheduleCheck, "Schedule Checking Task", 2048, NULL, 1, NULL);

    // Task Pump Control: Control the pump ON/OFF
    xTaskCreate(taskPumpControl, "Pump Control Task", 2048, NULL, 2, NULL);

    // Task Light Control: Control the NeoPixel LED ON/OFF (behavior like a normal LED)
    xTaskCreate(taskLightControl, "Light Control Task", 2048, NULL, 2, NULL);

    // Task MQTT Network Communication: Handles WiFi, MQTT pub/sub, and JSON parsing.
    xTaskCreate(taskMQTTCommunication, "MQTT Communication Task", 8192, NULL, 2, NULL);

    Serial.println("[System] All tasks created successfully!");
}

void loop() {
    // In a FreeRTOS architecture, the default loop() runs as a low-priority task.
    vTaskDelete(NULL);
}