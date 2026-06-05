#include "light_sensor.h"

void initLightSensor() {
    // Configure the specified pin as an analog input
    pinMode(LIGHT_SENSOR_PIN, INPUT);
}

void readLightIntensity(int &lightIntensity) {
    // Read the 12-bit analog value (0 - 4095 on ESP32)
    int data = analogRead(LIGHT_SENSOR_PIN);
    lightIntensity = data;
}