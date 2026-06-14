# Mushealy SmartFarm IoT Node (ESP32 + FreeRTOS + MQTT)

![C/C++](https://img.shields.io/badge/Language-C%2FC%2B%2B-blue.svg)
![ESP32](https://img.shields.io/badge/MCU-ESP32-red.svg)
![FreeRTOS](https://img.shields.io/badge/OS-FreeRTOS-green.svg)
![PlatformIO](https://img.shields.io/badge/Build-PlatformIO-orange.svg)
![MQTT](https://img.shields.io/badge/Protocol-MQTT-informational.svg)

## Table of Contents

1. [Project Overview](#project-overview)
2. [Features](#features)
3. [System Architecture](#system-architecture)
4. [Hardware & Pin Mapping](#hardware--pin-mapping)
5. [MQTT Design](#mqtt-design)
6. [Project Structure](#project-structure)
7. [Build & Deployment](#build--deployment)
8. [Runtime Behavior](#runtime-behavior)
9. [Edge Computing & Automation](#edge-computing--automation)

---

## Project Overview

**Mushealy SmartFarm** is a comprehensive smart farm system designed for precision agriculture and crop monitoring. This repository contains the **IoT Node Firmware** - the embedded system that runs on each field sensor node.

### Firmware Purpose
This firmware is responsible for:
- **Sensor Data Acquisition**: Continuously reading environmental data (temperature, humidity, soil moisture, light intensity)
- **Device Control**: Managing water pump relay and NeoPixel RGB LED strip
- **Cloud Communication**: Transmitting sensor telemetry and receiving commands via MQTT/CoreIoT
- **Autonomous Automation**: Implementing three distinct watering modes and smart irrigation logic

The system is built on the **ESP32** microcontroller running **FreeRTOS**, providing real-time multitasking and reliable edge computing capabilities.

### Three Watering Modes
1. **Time-Based Watering**: Automated watering at a scheduled time (configurable via cloud)
2. **Threshold-Based Watering**: Automatic pump activation when soil moisture falls below a configured threshold
3. **Manual Override (Remote Control)**: On-demand pump activation via mobile app/dashboard with automatic timeout (5 seconds)

### Key Capabilities
- Real-time multi-sensor data acquisition (10-second intervals)
- Three independent watering control modes with intelligent conflict prevention
- Cloud-based remote control and dynamic configuration updates
- Responsive LCD display for local status feedback
- Programmable grow lighting with NeoPixel RGB LEDs
- Edge computing for autonomous operation (works offline)
- MQTT pub/sub with CoreIoT ThingsBoard protocol

---

## Features

### Sensor Monitoring
- **DHT20 Sensor**: Temperature and humidity measurement (I2C interface)
- **Soil Moisture Sensor**: Analog reading for soil water content assessment
- **Light Intensity Sensor**: Analog reading for environmental brightness level
- **Real-time Data Pipeline**: Sensor data acquired every 10 seconds

### Watering Control System
The firmware implements three independent watering modes that intelligently prevent conflicts:
1. **Time-Based Watering**: Scheduled watering at a specific time each day (configurable via cloud dashboard)
2. **Threshold-Based Watering**: Automatic watering when soil moisture drops below a set threshold (edge-based intelligence)
3. **Manual Remote Control**: On-demand watering via mobile app with 5-second manual override timeout

### Grow Light Control
- RGB NeoPixel LED control (4 LEDs) for simulated sunrise/sunset or constant lighting
- Remotely controllable via cloud dashboard
- Support for custom color patterns and brightness levels

### Local & Remote Monitoring
- **LCD Display**: Real-time display of temperature, humidity, soil moisture, and light intensity
- **MQTT Publishing**: Continuous telemetry transmission to CoreIoT cloud (10-second interval)
- **Status Synchronization**: Real-time pump and light status attributes sent to cloud
- **Attribute Updates**: Cloud-based configuration changes propagated to edge device

### Network & Connectivity
- **WiFi Support**: Automatic connection and reconnection handling
- **NTP Time Synchronization**: GMT+7 timezone automatic sync for accurate scheduling
- **MQTT Protocol**: Standard MQTT 3.1.1 on port 1883 (non-TLS)
- **Persistent Connection**: Automatic reconnection with exponential backoff

---

## System Architecture

The Mushealy SmartFarm IoT Node uses a **multi-tasking architecture** based on FreeRTOS to manage concurrent operations efficiently. Each functional component runs as an independent task with coordinated synchronization.

### FreeRTOS Task Design

```
┌─────────────────────────────────────────────────────────────────┐
│              MUSHEALY SMARTFARM FIRMWARE SYSTEM                 │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────┐  ┌──────────────────┐  ┌──────────────┐   │
│  │  SENSOR READING  │  │  LCD DISPLAY     │  │SCHEDULE CHECK│   │
│  │  Task (P1)       │  │  Task (P1)       │  │ Task (P1)    │   │
│  │  - DHT20 (I2C)   │  │  - I2C Mutex     │  │ - Time-based │   │
│  │  - Light (ADC)   │  │  - LCD rendering │  │ - Threshold  │   │
│  │  - Soil (ADC)    │  │                  │  │ - Event Gen  │   │
│  │  - Queue PubSub  │  │                  │  │              │   │
│  └──────────────────┘  └──────────────────┘  └──────────────┘   │
│           │                    │                    │           │
│           └────────────────────┼────────────────────┘           │
│                                │                                │
│                         ┌──────▼──────┐                         │
│                         │ qSensorData │  (FreeRTOS Queue)       │
│                         │ egDeviceCtrl│  (Event Group)          │
│                         │ xMutexI2C   │  (Semaphore)            │
│                         └──────┬──────┘                         │
│                                │                                │
│  ┌──────────────────┐  ┌──────▼────────┐  ┌──────────────────┐  │
│  │  PUMP CONTROL    │  │ LIGHT CONTROL │  │ MQTT COMM TASK   │  │
│  │  Task (P2)       │  │  Task (P2)    │  │  Task (P2)       │  │
│  │  - Event wait    │  │  - NeoPixel   │  │  - WiFi setup    │  │
│  │  - Relay control │  │  - Event wait │  │  - MQTT conn     │  │
│  │  - Timeout logic │  │  - RGB color  │  │  - JSON pub/sub  │  │
│  └──────────────────┘  └───────────────┘  └──────────────────┘  │
│           │                    │                    │           │
│           └────────────────────┼────────────────────┘           │
│                                │                                │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │           HARDWARE PERIPHERALS & I/O PINS                  │ │
│  │  - I2C Bus (SDA:21, SCL:22) → DHT20 + LCD                  │ │
│  │  - ADC Pins (GPIO32, GPIO33) → Soil & Light Sensors        │ │
│  │  - GPIO26 → Pump Relay (Digital Out)                       │ │
│  │  - GPIO2  → NeoPixel RGB LED Strip (4 LEDs)                │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Task Descriptions

#### 1. **Sensor Reading Task** (Priority: 1, Stack: 4KB)
**Purpose**: Acquires environmental data from all sensors at regular intervals.

- **Operation**: Reads DHT20 (temperature/humidity), soil moisture sensor (ADC), and light sensor (ADC) every 10 seconds
- **I2C Safety**: Uses I2C Mutex to protect simultaneous access to DHT20 and LCD
- **Data Distribution**: Publishes sensor readings to `qSensorData` queue using `xQueueOverwrite()`
  - Only one data structure in queue; always holds the most recent reading
  - Other tasks peek without blocking to get latest values
- **Synchronization**: Coordinates with LCD Display Task via I2C Mutex

#### 2. **LCD Display Task** (Priority: 1, Stack: 4KB)
**Purpose**: Renders real-time sensor data and system status to the 16x2 LCD display.

- **Operation**: Updates display every 10 seconds with latest sensor values
- **I2C Synchronization**: Acquires I2C Mutex before LCD communication to prevent conflicts
- **Display Layout**:
  ```
  RT:25.3°C RH:65%
  LUX:3500  SM:45%
  ```

#### 3. **Schedule Check Task** (Priority: 1, Stack: 2KB)
**Purpose**: Monitors time-based and threshold-based conditions to trigger automation.

**Time-Based Watering:**
- Waits for NTP time synchronization (system checks every 10 seconds)
- Compares current time against `autoWaterHour` and `autoWaterMinute` (configured via MQTT)
- Sets `EVENT_PUMP_ON` bit in event group when time matches
- Prevents duplicate triggers within 60-second intervals

**Threshold-Based Watering:**
- Peeks at latest soil moisture reading from `qSensorData`
- Compares against `soilMoistureThreshold` (configurable via MQTT)
- Triggers pump if moisture < threshold AND pump not already active AND not in manual override mode
- Implements `TIME_RECOVERING_MS` (5 seconds) delay after activation to allow soil to absorb water

#### 4. **Pump Control Task** (Priority: 2, Stack: 2KB)
**Purpose**: Controls the water pump relay based on event signals and timing constraints.

**Control Logic:**
- Waits for `EVENT_PUMP_ON` or `EVENT_PUMP_OFF` bits in event group
- **Pump Activation**: Energizes GPIO26 relay for `TIME_PUMP_ACTIVE_MS` (10 seconds)
  - After 10 seconds, automatically deactivates even if event remains set
- **Manual Override**: When MQTT command received:
  - Sets `isPumpOverrideActive = true` for 5 seconds (OVERRIDE_DURATION_MS)
  - Executes the command immediately
  - After 5 seconds, override expires and normal automation resumes
- **State Tracking**: Updates `isPumpCurrentlyOn` flag for telemetry reporting

#### 5. **Light Control Task** (Priority: 2, Stack: 2KB)
**Purpose**: Controls NeoPixel RGB LED strip for grow lighting.

**Operation:**
- Responds to `EVENT_LIGHT_ON` and `EVENT_LIGHT_OFF` bits from event group
- Controls Adafruit NeoPixel library on GPIO2 (4 LEDs total)
- Set RGB color to violet for simulated daylight
- Remote control via MQTT: `setLightStatus` RPC method

#### 6. **MQTT Communication Task** (Priority: 2, Stack: 8KB)
**Purpose**: Manages network connectivity and bidirectional cloud communication.

**Initialization:**
- Connects to WiFi network specified in configuration
- Initializes PubSubClient for MQTT connection
- Sets up NTP time synchronization (GMT+7)

**Connection Management:**
- Attempts connection to broker every 5 seconds if disconnected
- Subscribes to control topics upon successful connection:
  - `v1/devices/me/rpc/request/+` (RPC commands)
  - `v1/devices/me/attributes` (configuration updates)

**Publishing Logic (10-second interval or on force trigger):**
- Constructs JSON telemetry payload with latest sensor data and device state
- Publishes to `v1/devices/me/telemetry`:
  ```json
  {
    "temperature": 25.3,
    "humidity": 65.2,
    "soil": 45,
    "light": 3500,
    "pump_status": 1,
    "light_status": 0
  }
  ```
- Publishes to `v1/devices/me/attributes`:
  ```json
  {
    "pump_status": true,
    "light_status": false
  }
  ```
- `forcePublish` flag triggers immediate publish on manual commands

**Incoming RPC Commands** (`setPumpStatus`, `setLightStatus`):
```json
{
  "method": "setPumpStatus",
  "params": true
}
```
- Activates manual override mode (5-second timeout)
- Sets corresponding event group bits
- Triggers immediate telemetry publish to sync cloud

**Configuration Updates** (Shared Attributes from cloud):
```json
{
  "schedule_hour": 14,
  "schedule_minute": 30,
  "soil_threshold": 40
}
```
- Updates edge computing variables dynamically
- No restart required for configuration changes

### Synchronization Primitives

| Name | Type | Purpose |
|------|------|---------|
| `qSensorData` | Queue (size=1) | Single-element buffer for latest sensor readings |
| `xMutexI2C` | Mutex | Protects I2C bus access (DHT20 & LCD) |
| `egDeviceControl` | Event Group | 4 event bits for pump/light ON/OFF commands |

### Data Flow

```
Sensors (DHT20, Soil, Light)
    ↓
[Sensor Reading Task]
    ↓
qSensorData Queue (xQueueOverwrite)
    ↓
┌─────────────────────────────────────────┐
│ • LCD Display Task (peek, render)       │
│ • Schedule Check Task (peek, compare)   │
│ • MQTT Task (peek, serialize, publish)  │
└─────────────────────────────────────────┘
    ↓
Device Outputs: Pump Relay & NeoPixel LEDs
    ↓
Cloud Storage & Visualization (CoreIoT)
```

---

## Hardware & Pin Mapping

### Board & Microcontroller
- **Microcontroller**: ESP32
- **Development Board**: Yolo:Bit (ESP32 breakout)
- **Communication Protocols**: I2C (DHT20, LCD), ADC (sensors)

### Pin Configuration

| Function | Pin | Type | Description |
|----------|-----|------|-------------|
| **I2C SDA** | GPIO 21 | I2C | I2C data line (DHT20 sensor & LCD display) |
| **I2C SCL** | GPIO 22 | I2C | I2C clock line (DHT20 sensor & LCD display) |
| **Soil Moisture** | GPIO 32 (P0) | ADC | Analog soil moisture sensor input |
| **Light Intensity** | GPIO 33 (P1) | ADC | Analog light sensor input |
| **Pump Relay** | GPIO 26 (P10.P13) | Digital Out | Controls water pump via 2-port USB control module |
| **NeoPixel LED** | GPIO 2 (P3) | Digital Out | RGB LED strip (4 LEDs, WS2812B protocol) |

---

## MQTT Design

### Broker Configuration

| Parameter | Value |
|-----------|-------|
| **Broker Address** | `app.coreiot.io` |
| **Port** | `1883` (non-TLS MQTT) |
| **Protocol** | MQTT 3.1.1 |
| **Client ID** | `MushealySmartFarm` |
| **Username (Token)** | Access token provided by CoreIoT |
| **Password** | Empty (CoreIoT uses token authentication) |

### Topic Structure (CoreIoT ThingsBoard Protocol)

#### Publishing Topics (Device → Cloud)

**1. Telemetry Topic**: `v1/devices/me/telemetry`
- **Publish Interval**: Every 10 seconds (or on force trigger)
- **Payload Format**: JSON with sensor readings and device state
- **Example**:
  ```json
  {
    "temperature": 25.3,
    "humidity": 65.2,
    "soil": 45,
    "light": 3500,
    "pump_status": 1,
    "light_status": 0
  }
  ```
- **Field Descriptions**:
  - `temperature` (float): Ambient temperature in °C
  - `humidity` (float): Relative humidity in %
  - `soil` (int): Soil moisture percentage (0-100%)
  - `light` (int): Light intensity in lux (0-4000+)
  - `pump_status` (int): 1=ON, 0=OFF
  - `light_status` (int): 1=ON, 0=OFF

**2. Attributes Topic**: `v1/devices/me/attributes`
- **Publish Interval**: Every 10 seconds (or on force trigger)
- **Purpose**: Sync device state (pump and light) with cloud
- **Payload Format**: JSON with current hardware state
- **Example**:
  ```json
  {
    "pump_status": true,
    "light_status": false
  }
  ```
- **Field Descriptions**:
  - `pump_status` (bool): True=pump relay active, False=inactive
  - `light_status` (bool): True=LEDs on, False=off

#### Subscription Topics (Cloud → Device)

**1. RPC Request Topic**: `v1/devices/me/rpc/request/+`
- **Purpose**: Receive remote procedure call commands for real-time control
- **Payload Format**: JSON with method and parameters
- **Example Payload - Pump Control**:
  ```json
  {
    "method": "setPumpStatus",
    "params": true
  }
  ```
  - `method = "setPumpStatus"`: Control pump
  - `params = true`: Activate pump
  - `params = false`: Deactivate pump

- **Example Payload - Light Control**:
  ```json
  {
    "method": "setLightStatus",
    "params": true
  }
  ```
  - `method = "setLightStatus"`: Control light
  - `params = true`: Turn lights on
  - `params = false`: Turn lights off

<!-- **Response Behavior**:
- Device immediately executes the command
- Manual override mode activated for 60 seconds
- Device publishes updated telemetry/attributes to reflect the change
- Cloud can then verify the action completed -->

**2. Shared Attributes Topic**: `v1/devices/me/attributes`
- **Purpose**: Receive edge computing configuration parameters
- **Update Frequency**: On-demand from cloud dashboard
- **Payload Format**: JSON with configuration values
- **Example Payload**:
  ```json
  {
    "schedule_hour": 14,
    "schedule_minute": 30,
    "soil_threshold": 40
  }
  ```
- **Parameter Descriptions**:
  - `schedule_hour` (int): Hour for scheduled watering (0-23)
  - `schedule_minute` (int): Minute for scheduled watering (0-59)
  - `soil_threshold` (int): Soil moisture threshold for auto-watering (0-100%)

<!-- - **Device Behavior on Update**:
  - Device immediately stores new values in global variables
  - No restart required; automation uses updated values immediately
  - Device logs update event to Serial output
  - Next automatic check uses new configuration -->

### Data Flow & Timing Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                   NORMAL OPERATION (10s Cycle)                  │
│                                                                 │
│  Sensor Reading              MQTT Publish                       │
│  Every 10s                   Every 10s                          │
│  ┌──────────────┐            ┌────────────────┐                 │
│  │ DHT20        │            │ Telemetry JSON │──────┐          │
│  │ Soil Sensor  │ ──────────→│ Attributes JSON│      │          │
│  │ Light Sensor │            │ (latest data)  │      │          │
│  └──────────────┘            └────────────────┘      │          │
│                                                      │          │
│                                               CoreIoT Broker    │
│                                              (app.coreiot.io)   │
│                                                      ▲          │
│                                                      │          │
│  ┌───────────────────────────────────────────────────┴──────┐   │
│  │               Dashboard / Mobile App                     │   │
│  │  • View sensor data in real-time                         │   │
│  │  • Historical charts and analytics                       │   │
│  │  • Alert triggers on threshold violations                │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│            REMOTE CONTROL SEQUENCE (Manual Override)            │
│                                                                 │
│  User Action on Dashboard                                       │
│  │                                                              │
│  └─→ [Send: setPumpStatus = true]                               │
│      │                                                          │
│      └─→ CoreIoT Broker                                         │
│          │                                                      │
│          └─→ ESP32 Receives RPC Request                         │
│              │                                                  │
│              ├─ Parse JSON                                      │
│              ├─ Activate Manual Override (5s timeout)           │
│              ├─ Set EVENT_PUMP_ON bit                           │
│              ├─ Set forcePublish = true                         │
│              │                                                  │
│              └─→ Pump Control Task                              │
│                  │                                              │
│                  ├─ Detect event, activate relay                │
│                  ├─ Keep ON for TIME_PUMP_ACTIVE_MS (10s)       │
│                  │                                              │
│                  └─→ Water Pump ACTIVATES                       │
│                      (Physical relay closure)                   │
│                                                                 │
│  Meanwhile: MQTT Task detects forcePublish flag                 │
│  │                                                              │
│  └─→ Immediately publish telemetry with:                        │
│      {                                                          │
│        "pump_status": 1,                                        │
│        "temperature": 25.3,                                     │
│        ...                                                      │
│      }                                                          │
│      │                                                          │
│      └─→ CoreIoT Broker                                         │
│          │                                                      │
│          └─→ Dashboard shows pump is now ON                     │
│                                                                 │
│  After 5s: Manual override expires                              │
│  └─ Normal automation resumes                                   │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│            EDGE CONFIGURATION UPDATE SEQUENCE                   │
│                                                                 │
│  Cloud Admin Changes Setting on Dashboard                       │
│  │                                                              │
│  ├─→ Set Scheduled Watering Time = 14:30                        │
│  └─→ CoreIoT Broker (Shared Attributes update)                  │
│      │                                                          │
│      └─→ ESP32 subscribes to attributes topic                   │
│          │                                                      │
│          └─→ mqttCallback() processes update                    │
│              ├─ Parse JSON: schedule_hour=14, schedule_minute=30│
│              ├─ Update global: autoWaterHour = 14               │
│              ├─ Update global: autoWaterMinute = 30             │
│              ├─ Log to Serial: "[MQTT] New Schedule Hour: 14"   │
│              │                                                  │
│              └─→ Schedule Check Task                            │
│                  │                                              │
│                  ├─ Next check at 14:30 uses new values         │
│                  ├─ (No restart required)                       │
│                  │                                              │
│                  └─ If 14:30 arrives → Pump Activation          │
└─────────────────────────────────────────────────────────────────┘
```

### Connection State Management

```
MQTT Connection Lifecycle:

┌──────────────────┐
│   Disconnected   │
└────────┬─────────┘
         │
         │ Attempt connect
         ▼
┌──────────────────┐
│    Connecting    │ ◄─┐
├──────────────────┤   │
│  Retry every 5s  │   │
└────────┬─────────┘   │
         │             │
         │ Success     │
         ▼             │
┌──────────────────┐   │
│    Connected     │   │
├──────────────────┤   │
│ Subscribe Topics │   │
│ Pub every 10s    │   │ Failure or disconnected
└────────┬─────────┘   │
         │             │
         │ Broker drops│
         └─────────────┘
```

### Error Handling & Reconnection

- **Connection Failure**: Retries every 5 seconds with exponential backoff feedback
- **Dropped Connection**: Automatic detection and reconnection attempt
- **Failed Publish**: Data re-sent on next publish cycle (most recent data is used)
- **Malformed RPC**: Logged to Serial, message ignored, device continues normal operation

---

## Project Structure

```
MultidisciplinaryProject/
├── platformio.ini              # PlatformIO project configuration
├── README.md
│
├── include/
│   ├── config.h                # Pin mapping and hardware configuration
│   ├── global.h                # FreeRTOS objects and shared variables
│   ├── dht20_sensor.h
│   ├── soil_mois_sensor.h
│   ├── light_sensor.h
│   ├── lcd_display.h
│   ├── light_control.h
│   ├── pump_relay.h
│   ├── schedule_check.h
│   ├── sensor_task.h
│   └── mqtt_connection.h
│
├── src/
│   ├── main.cpp                # FreeRTOS task creation and setup
│   ├── global.cpp              # RTOS object initialization
│   ├── dht20_sensor.cpp        # DHT20 sensor implementation
│   ├── soil_mois_sensor.cpp    # Soil moisture sensor implementation
│   ├── light_sensor.cpp        # Light sensor implementation
│   ├── sensor_task.cpp         # Sensor reading task implementation
│   ├── lcd_display.cpp         # LCD rendering implementation
│   ├── light_control.cpp       # NeoPixel control implementation
│   ├── pump_relay.cpp          # Pump relay logic implementation
│   ├── schedule_check.cpp      # Schedule and threshold automation
│   └── mqtt_connection.cpp     # MQTT and WiFi implementation
│
└── lib/
    ├── DHT20/                  # DHT20 sensor library
    │   ├── DHT20.cpp
    │   └── DHT20.h
    ├── LCD/                    # LiquidCrystal I2C library
    │   ├── LiquidCrystal_I2C.cpp
    │   └── LiquidCrystal_I2C.h
    └── PubSubClient/           # MQTT client library
        ├── PubSubClient.cpp
        └── PubSubClient.h
```

### File Descriptions

| File | Purpose |
|------|---------|
| `config.h` | Centralized hardware configuration (pin definitions, thresholds, timings) |
| `global.h/cpp` | FreeRTOS kernel objects (queues, mutexes, event groups) initialization |
| `main.cpp` | System boot and FreeRTOS task creation |
| `sensor_task.h/cpp` | Sensor reading task - DHT20, soil, light data acquisition |
| `lcd_display.h/cpp` | LCD rendering with I2C mutex protection |
| `pump_relay.h/cpp` | Pump control logic and relay activation |
| `light_control.h/cpp` | NeoPixel RGB LED control using Adafruit library |
| `schedule_check.h/cpp` | Automation logic (time-based and threshold-based watering) |
| `mqtt_connection.h/cpp` | WiFi and MQTT pub/sub, JSON serialization, command parsing |
| `dht20_sensor.h/cpp` | DHT20 library wrapper with I2C mutex locking |
| `soil_mois_sensor.h/cpp` | Soil moisture ADC reading and calibration |
| `light_sensor.h/cpp` | Light intensity ADC reading and calibration |

---

## Build & Deployment

### Prerequisites

1. Visual Studio Code
2. PlatformIO extension
3. ESP32 board connected via USB

### Configuration Before Build

**Step 1: Edit WiFi Credentials**
Open `src/mqtt_connection.cpp` and update:
```cpp
const char* WIFI_SSID     = "YourWiFiName";         // Enter your WiFi SSID
const char* WIFI_PASSWORD = "YourPassword";         // Enter your WiFi password
```

**Step 2: Edit CoreIoT Credentials**
In the same file, update:
```cpp
const char* MQTT_USER = "your_access_token_here";  // Your CoreIoT Access Token
const char* MQTT_PASS = "";                        // Leave empty (CoreIoT uses token auth)
```

**Step 3: Verify Hardware Pin Configuration** (Optional)
Check `include/config.h` for correct pin assignments. Default pins match Yolo:Bit board. If using different board, adjust accordingly.

### Building the Project

**Option 1: Using VS Code with PlatformIO Extension**

1. Open the project folder in VS Code
2. Click the PlatformIO icon in the left sidebar
3. Expand "esp32dev" environment
4. Build firmware:
   - PlatformIO: `Build`
5. Upload firmware:
   - PlatformIO: `Upload`
6. Monitor logs:
   - PlatformIO: `Monitor` (115200 baud)

**Option 2: Using PlatformIO CLI**

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```
<!-- 
### Serial Monitor / Debug Output

After successful upload, open the serial monitor to see system messages:

```bash
platformio run -e esp32dev --target monitor
```

**Expected boot sequence**:
```
=== MUSHEALY SMARTFARM BOOT ===
[System] Initializing RTOS objects...
[Sensor Task] Initialization complete!
[LCD Task] Started
[Schedule Task] Started. Waiting for time sync...
[Pump Task] Initialization complete!
[Light Task] Initialization complete!
[MQTT Task] Initialization complete!
[System] All tasks created successfully!
[WiFi] Connecting to SweepTosho...
..... (dots show connection attempts)
[WiFi] Connected successfully!
[WiFi] IP Address: 192.168.x.x
[NTP] Syncing time...
[Schedule Task] Time synchronized successfully.
[MQTT] Attempting connection...
[MQTT] Connected to Broker!
[Sensor Task] Temp: 25.3 °C | Hum: 65.0 % | Soil: 45 % | Light: 3500 lx
``` -->

---

## Runtime Behavior

### System Boot & Initialization
- ESP32 boots and initializes Arduino core
- `setup()` function creates all FreeRTOS objects (queues, mutexes, event groups) and spawns 6 concurrent tasks
- FreeRTOS scheduler takes control; tasks run concurrently with time-slicing

### Continuous Operation (10-Second Cycle)
1. **Sensor Reading Task** acquires data from DHT20, soil moisture, and light sensors every 10 seconds
2. **LCD Display Task** updates the display with latest sensor values (with I2C mutex protection)
3. **Schedule Check Task** monitors both time-based (NTP synchronized) and threshold-based (soil moisture) conditions
4. **MQTT Task** publishes telemetry and attributes to CoreIoT every 10 seconds
5. **Pump & Light Control Tasks** wait for event signals from schedule or manual commands

### Automation Behaviors
- **Time-Based Watering**: When the scheduled time arrives, pump activates for 10 seconds then automatically stops
- **Threshold-Based Watering**: When soil moisture falls below threshold, pump activates with 5-second recovery time to allow absorption
- **Manual Override**: Remote commands (via app) activate the pump/light with 5-second manual override timeout
- **Three modes coexist**: Intelligent logic prevents mode conflicts (manual override takes precedence)

### Network Behavior
- **WiFi**: Automatic connection on boot; auto-reconnects every 5 seconds if disconnected
- **MQTT**: Subscribes to RPC and attributes topics; publishes telemetry every 10 seconds
- **Offline Resilience**: All local automation (watering, lighting, LCD) continues even if cloud connection drops
- **Configuration Updates**: Shared attributes from cloud update edge parameters immediately without restart
- **Graceful Degradation**: Failed MQTT publishes are buffered and retried; sensor errors logged and retried next cycle

---

**Document Version**: 1.0  
**Last Updated**: 2026  
**Project**: Mushealy SmartFarm IoT Node  
