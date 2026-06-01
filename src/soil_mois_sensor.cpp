#include "soil_mois_sensor.h"

void initSoilSensor() {
    // Configure the specified pin as an analog input
    pinMode(SOIL_MOISTURE_PIN, INPUT);
}

void readSoilMoisture(int &moisture) {
    // Read the 12-bit analog value (0 - 4095 on ESP32)
    int data = analogRead(SOIL_MOISTURE_PIN);
    moisture = constrain(map(data, 0, 4095, 100, 0), 0, 100);
}