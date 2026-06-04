#ifndef __MQTT_CONNECTION_H__
#define __MQTT_CONNECTION_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "PubSubClient.h"
#include "global.h"
#include "config.h"

// FreeRTOS Task to handle Wi-Fi and MQTT connectivity
void taskMQTTCommunication(void *pvParameters);

#endif