#include "mqtt_connection.h"

// ---------------------------------------------------------
// Network Credentials
// ---------------------------------------------------------
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---------------------------------------------------------
// CoreIoT Configuration (ThingsBoard Protocol)
// ---------------------------------------------------------
const char* MQTT_BROKER   = "mqtt.coreiot.vn"; 
const int   MQTT_PORT     = 1883; // Standard non-TLS MQTT port

// For CoreIoT, the Device Access Token acts as the username
const char* MQTT_USER     = "YOUR_COREIOT_ACCESS_TOKEN"; 
const char* MQTT_PASS     = ""; // Password is left empty

// Standard CoreIoT topics
const char* TOPIC_PUB_DATA = "v1/devices/me/telemetry";
const char* TOPIC_SUB_CTRL = "v1/devices/me/rpc/request/+";

// Use standard WiFiClient for port 1883
WiFiClient espClient; 
PubSubClient mqttClient(espClient);

// ---------------------------------------------------------
// CoreIoT MQTT Callback
// ---------------------------------------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("[MQTT] RPC Request arrived on topic: %s\n", topic);
    
    // Convert payload to String
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println("[MQTT] RPC Payload: " + message);

    // CoreIoT sends control commands in JSON format via RPC.
    // Example payload: {"method": "setPumpStatus", "params": true}
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (!error) {
        String method = doc["method"].as<String>();
        bool params = doc["params"].as<bool>();

        if (method == "setPumpStatus") {
            if (params == true) {
                Serial.println("[MQTT] Command: Turn Pump ON");
                xEventGroupSetBits(egPumpControl, EVENT_PUMP_ON);
            } else {
                Serial.println("[MQTT] Command: Turn Pump OFF");
                xEventGroupSetBits(egPumpControl, EVENT_PUMP_OFF);
            }
        }
    }
}

// ---------------------------------------------------------
// Helper: Connect to Wi-Fi
// ---------------------------------------------------------
void setupWiFi() {
    Serial.printf("[WiFi] Connecting to %s...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
    }
    Serial.println("\n[WiFi] Connected successfully!");
    Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
}

// ---------------------------------------------------------
// Main MQTT Task
// ---------------------------------------------------------
void taskMQTTCommunication(void *pvParameters) {
    // 1. Initialize Network connection
    setupWiFi();
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    SensorData dataToSend;
    TickType_t lastPublishTime = xTaskGetTickCount();
    const TickType_t PUBLISH_INTERVAL = pdMS_TO_TICKS(10000); // Publish every 10 seconds

    Serial.println("[MQTT Task] Initialization complete!");

    // FreeRTOS Task lifecycle for Network Handling 
    while (1) {
        // Reconnect logic if MQTT drops
        if (!mqttClient.connected()) {
            Serial.println("[MQTT] Attempting connection...");
            // Create a random client ID
            String clientId = "MushealySmartFarm";
            
            if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
                Serial.println("[MQTT] Connected to Broker!");
                // Subscribe to control topics upon successful connection
                mqttClient.subscribe(TOPIC_SUB_CTRL);
            } else {
                Serial.printf("[MQTT] Connection failed, rc=%d. Retrying in 5s...\n", mqttClient.state());
                vTaskDelay(pdMS_TO_TICKS(5000));
                continue; // Skip the rest of the loop and retry
            }
        }

        // Maintain MQTT keep-alive and process incoming messages
        mqttClient.loop();

        // 3. Periodic Publishing Logic
        // Check if it's time to publish and if we have fresh data in the queue
        if ((xTaskGetTickCount() - lastPublishTime) >= PUBLISH_INTERVAL) {
            
            // Peek at the queue to get data without removing it
            if (xQueuePeek(qSensorData, &dataToSend, 0) == pdTRUE) {
                
                // Construct a JSON payload using ArduinoJson
                StaticJsonDocument<256> doc;
                doc["temperature"] = dataToSend.temperature;
                doc["humidity"]    = dataToSend.humidity;
                doc["soil"]        = dataToSend.soilMoisture;
                doc["light"]       = dataToSend.lightIntensity;

                char jsonBuffer[256];
                serializeJson(doc, jsonBuffer);

                // Publish to the telemetry topic
                if (mqttClient.publish(TOPIC_PUB_DATA, jsonBuffer)) {
                    Serial.println("[MQTT] Payload published successfully.");
                } else {
                    Serial.println("[MQTT] Failed to publish payload.");
                }
                
                lastPublishTime = xTaskGetTickCount();
            }
        }

        // Small delay to yield CPU to other tasks and prevent watchdog trigger
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}