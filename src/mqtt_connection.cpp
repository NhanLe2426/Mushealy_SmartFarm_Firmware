#include "mqtt_connection.h"

// ---------------------------------------------------------
// Network Credentials
// ---------------------------------------------------------
const char* WIFI_SSID     = "SweepTosho";               // WiFi Name
const char* WIFI_PASSWORD = "tencuatuia";           // WiFi Password

// ---------------------------------------------------------
// CoreIoT Configuration (ThingsBoard Protocol)
// ---------------------------------------------------------
const char* MQTT_BROKER   = "app.coreiot.io"; 
const int   MQTT_PORT     = 1883; // Standard non-TLS MQTT port

// For CoreIoT, the Device Access Token acts as the username
const char* MQTT_USER     = "1omr8yulbsmbyugm9yof";         // Your CoreIOT Access Token
const char* MQTT_PASS     = "";                             // Password is left empty

// Standard CoreIoT topics
const char* TOPIC_PUB_DATA = "v1/devices/me/telemetry";
const char* TOPIC_PUB_ATTR = "v1/devices/me/attributes";
const char* TOPIC_SUB_CTRL = "v1/devices/me/rpc/request/+";

// Use standard WiFiClient for port 1883
WiFiClient espClient; 
PubSubClient mqttClient(espClient);

// ---------------------------------------------------------
// CoreIoT MQTT Callback
// ---------------------------------------------------------
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("[MQTT] RPC Request arrived on topic: %s\n", topic);
    String topicStr = String(topic);
    
    // Convert payload to String
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println("[MQTT] RPC Payload: " + message);
    Serial.println("[MQTT] Incoming Message on: " + topicStr);

    // CoreIoT sends control commands in JSON format via RPC.
    // Example payload: {"method": "setPumpStatus", "params": true}
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) return;

    // 1. Handle RPC Manual Control Commands
    if (topicStr.indexOf("rpc") >= 0) {
        String method = doc["method"].as<String>();
        bool isTurnOn = doc["params"].as<bool>();

        // Handle Pump Control
        if (method == "setPumpStatus") {
            // 1. Activate manual override mode
            isPumpOverrideActive = true;
    
            // 2. Set the deadline for the override to expire
            pumpOverrideEndTime = xTaskGetTickCount() + pdMS_TO_TICKS(OVERRIDE_DURATION_MS);

            // 3. Execute the manual command
            if (isTurnOn == true) {
                Serial.println("[MQTT] Manual Command: Turn Pump ON");
                xEventGroupSetBits(egDeviceControl, EVENT_PUMP_ON);
            } else {
                Serial.println("[MQTT] Manual Command: Turn Pump OFF");
                xEventGroupSetBits(egDeviceControl, EVENT_PUMP_OFF);
            }

            // Trigger immediate telemetry publish to sync cloud database
            forcePublish = true;
        }

        // Handle Light Control
        if (method == "setLightStatus") {
            if (isTurnOn == true) {
                Serial.println("[MQTT] Manual Command: Turn Light ON");
                xEventGroupSetBits(egDeviceControl, EVENT_LIGHT_ON);
            } else {
                Serial.println("[MQTT] Manual Command: Turn Light OFF");
                xEventGroupSetBits(egDeviceControl, EVENT_LIGHT_OFF);
            }

            // Trigger immediate telemetry publish to sync cloud database
            forcePublish = true;
        }
    }
    // 2. Handle Cloud Configuration Updates (Edge Computing)
    else if (topicStr.indexOf("attributes") >= 0) {
        Serial.println("[MQTT] Edge parameters updated from Cloud");
        
        if (doc.containsKey("schedule_hour")) {
            autoWaterHour = doc["schedule_hour"].as<int>();
            Serial.printf("[MQTT] New Schedule Hour: %d\n", autoWaterHour);
        }
        if (doc.containsKey("schedule_minute")) {
            autoWaterMinute = doc["schedule_minute"].as<int>();
            Serial.printf("[MQTT] New Schedule Minute: %d\n", autoWaterMinute);
        }
        if (doc.containsKey("soil_threshold")) {
            soilMoistureThreshold = doc["soil_threshold"].as<int>();
            Serial.printf("[MQTT] New Soil Threshold: %d\n", soilMoistureThreshold);
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

    // Sync time via NTP server 
    // Timezone offset is 7 hours (7 * 3600 seconds) for GMT+7
    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("[NTP] Syncing time...");
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
                // Subscribe to Shared Attributes updates from the cloud
                mqttClient.subscribe("v1/devices/me/attributes");
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
        if ((xTaskGetTickCount() - lastPublishTime) >= PUBLISH_INTERVAL || forcePublish) {

            // Reset the flag to prevent continuous publishing
            forcePublish = false;
            
            // Peek at the queue to get data without removing it
            if (xQueuePeek(qSensorData, &dataToSend, 0) == pdTRUE) {
                
                // Construct a JSON payload using ArduinoJson
                StaticJsonDocument<384> docTelemetry;
                docTelemetry["temperature"]  = dataToSend.temperature;
                docTelemetry["humidity"]     = dataToSend.humidity;
                docTelemetry["soil"]         = dataToSend.soilMoisture;
                docTelemetry["light"]        = dataToSend.lightIntensity;    
                docTelemetry["pump_status"]  = isPumpCurrentlyOn ? 1 : 0;
                docTelemetry["light_status"] = isLightCurrentlyOn ? 1 : 0;

                char jsonTelemetry[384];
                serializeJson(docTelemetry, jsonTelemetry);
                mqttClient.publish(TOPIC_PUB_DATA, jsonTelemetry);

                // Send Attribute data (for Switch state synchronization)
                StaticJsonDocument<128> docAttr;
                docAttr["pump_status"]  = isPumpCurrentlyOn;     // Real-time hardware state
                docAttr["light_status"] = isLightCurrentlyOn;

                char jsonAttr[128];
                serializeJson(docAttr, jsonAttr);
                mqttClient.publish(TOPIC_PUB_ATTR, jsonAttr);    // Publish to Attributes

                // Publish to the telemetry topic
                if (mqttClient.publish(TOPIC_PUB_DATA, jsonTelemetry)) {
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