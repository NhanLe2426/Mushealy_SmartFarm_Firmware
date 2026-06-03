#include "lcd_display.h"

// Initialize the LCD object (I2C Address: 0x21 (33), Columns: 16, Rows: 2)
LiquidCrystal_I2C lcd(0x21, 16, 2);

void initLCD() {
    lcd.begin();
    lcd.backlight();
    lcd.clear();
}

void taskDisplayLCD(void *pvParameters) {
    /* I2C MUTEX PROTECTION: Accquire the I2C bus lock before Initialize the LCD hardware
     * This prevents collisions if another task is currently using the I2C bus (e.g., Sensor Task)
     */
    if (xSemaphoreTake(xMutexI2C, portMAX_DELAY) == pdTRUE) {
        initLCD();

        // Row 1 (Index 0): Center "YoloFarm"
        lcd.setCursor(4, 0); 
        lcd.print("YoloFarm");

        // Row 2 (Index 1): Center "Starting..."
        lcd.setCursor(2, 1);
        lcd.print("Starting...");

        // Release the I2C bus
        xSemaphoreGive(xMutexI2C);
    }

    // Keep the splash screen visible for 2 seconds
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Clear the screen to prepare for real-time sensor data
    if (xSemaphoreTake(xMutexI2C, portMAX_DELAY) == pdTRUE) {
        lcd.clear();
        xSemaphoreGive(xMutexI2C);
    }

    SensorData displayData;
    Serial.println("[LCD Task] UI Initialized. Waiting for data...");

    // FreeRTOS Task lifecycle
    while (1) {
        if (xQueueReceive(qSensorData, &displayData, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(xMutexI2C, portMAX_DELAY) == pdTRUE) {
                
                // --- Row 1 ---
                lcd.setCursor(0, 0); 
                lcd.print("T:");
                lcd.setCursor(3, 0);
                lcd.print(displayData.temperature, 1); 

                lcd.setCursor(8, 0);
                lcd.print("H:");
                lcd.setCursor(11, 0);
                lcd.print(displayData.humidity, 1);

                // --- Row 2 ---
                lcd.setCursor(0, 1);
                lcd.print("S:");
                lcd.setCursor(3, 1);
                lcd.printf("%-3d  ", displayData.soilMoisture); 

                lcd.setCursor(8, 1);
                lcd.print("L:");
                lcd.setCursor(11, 1);
                lcd.printf("%-3d  ", displayData.lightIntensity);

                xSemaphoreGive(xMutexI2C);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}