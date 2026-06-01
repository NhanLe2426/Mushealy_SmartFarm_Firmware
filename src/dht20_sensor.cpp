#include "dht20_sensor.h"

// Create a DHT20 object
DHT20 dht20;

void initDHT20() {
    // Standard initialization for the DHT20 sensor
    dht20.begin();
}

void readDHT20(float &temperature, float &humidity) {
    // Request access to the I2C bus
    if (xSemaphoreTake(xMutexI2C, portMAX_DELAY) == pdTRUE) {
        // Read data from the physical sensor
        dht20.read();

        // Update the variables passed by reference
        temperature = dht20.getTemperature();
        humidity = dht20.getHumidity();

        // Release the I2C bus for the other modules to use
        xSemaphoreGive(xMutexI2C);
    }
}