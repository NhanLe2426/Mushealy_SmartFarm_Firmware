#include "light_control.h"

// Declare the NeoPixel object
Adafruit_NeoPixel pixels(NEO_PIXEL_NUM, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void initLight() {
    pixels.begin();             // Initialize the NeoPixel library
    pixels.setBrightness(255);  // Set maximum brightness (0-255)
    pixels.show();              // Turn off all pixels initially
}

void setLightColor(uint8_t r, uint8_t g, uint8_t b) {
    // Loop through all 4 LEDs on the board and set their color
    for(int i = 0; i < NEO_PIXEL_NUM; i++) {
        pixels.setPixelColor(i, pixels.Color(r, g, b));
    }
    pixels.show();              // Send the updated color data to the hardware
}

void taskLightControl(void *pvParameters) {
    // Initialization
    initLight();
    Serial.println("[Light Task] Started and waiting for control events...");

    // FreeRTOS Task lifecycle
    while (1) {
        // Wait indefinitely for either EVENT_LIGHT_ON or EVENT_LIGHT_OFF
        EventBits_t uxBits = xEventGroupWaitBits(
            egDeviceControl,
            EVENT_LIGHT_ON | EVENT_LIGHT_OFF, 
            pdTRUE,   
            pdFALSE,  
            portMAX_DELAY 
        );

        if ((uxBits & EVENT_LIGHT_ON) != 0) {
            Serial.println("[Light Task] Event received: Turn ON light.");
            
            // Magenta/Purple (Full Red + Full Blue) is ideal for plant photosynthesis
            setLightColor(255, 0, 255); 
            isLightCurrentlyOn = true;
            forcePublish = true;
        }
        else if ((uxBits & EVENT_LIGHT_OFF) != 0) {
            Serial.println("[Light Task] Event received: Turn OFF light.");
            
            // Turn off all LEDs
            setLightColor(0, 0, 0); 
            isLightCurrentlyOn = false;
            forcePublish = false;
        }
    }
}