<div id="top">

<!-- HEADER STYLE: CONSOLE -->
<div align="center">

<h1 align="center">
  <br>
  <a href="http://www.amitmerchant.com/electron-markdownify"><img src="https://raw.githubusercontent.com/gnauqoa/fast-iot-fe/c8f4e737913bd522e44882d34bd66b855ba340f1/public/fast-iot-no-text.svg"alt="Markdownify" width="200"></a>
  <br>Fast IoT - ESP32 & ESP8266
  <br>
</h1>

<h4>A full-stack framework to fast-track your IoT development.</h4>
</div>
<p align="center">
 <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#dependencies">Dependencies</a> •
  <a href="#basic-usage">Basic Usage</a> •
  <a href="#using-with-platformio">Using with PlatformIO</a> •
  <a href="#using-wifimanager">Using WiFiManager</a> •
  <a href="#api-reference">API Reference</a> •
  <a href="#message-format">Message Format</a> •
  <a href="#topics">Topics</a> •
  <a href="#error-handling">Error Handling</a> •
  <a href="#notes">Notes</a> •
  <a href="#example-output">Example Output</a> •
  <a href="#troubleshooting">Troubleshooting</a> •
  <a href="#license">License</a>
</p>
<div align="center">
  <!-- Platform & Boards -->
  <img src="https://img.shields.io/badge/PlatformIO-FF6600.svg?style=flat-square&logo=platformio&logoColor=white" alt="PlatformIO" />
  <img src="https://img.shields.io/badge/ESP32-3C3C3C.svg?style=flat-square&logo=espressif&logoColor=white" alt="ESP32" />
  <img src="https://img.shields.io/badge/ESP8266-3C3C3C.svg?style=flat-square&logo=espressif&logoColor=white" alt="ESP8266" />

  <!-- Language & Core Libraries -->
  <img src="https://img.shields.io/badge/C++-00599C.svg?style=flat-square&logo=c%2B%2B&logoColor=white" alt="C++" />
  <img src="https://img.shields.io/badge/ArduinoJson-28A745.svg?style=flat-square&logo=arduino&logoColor=white" alt="ArduinoJson" />
  <img src="https://img.shields.io/badge/PubSubClient-4CAF50.svg?style=flat-square" alt="PubSubClient" />
  <img src="https://img.shields.io/badge/WiFiManager-009688.svg?style=flat-square" alt="WiFiManager" />

  <!-- Protocol -->
  <img src="https://img.shields.io/badge/MQTT-660066.svg?style=flat-square&logo=mqtt&logoColor=white" alt="MQTT" />
</div>
</div>

An Arduino IDE library for ESP8266 to connect to MQTT brokers with username/password authentication and channel-specific callbacks, designed to replicate and extend the functionality of the provided Node.js MQTT Client Library.

## Features

- WiFi connection management with optional WiFiManager support
- MQTT connection with username/password authentication
- Token parsing (splits on '-' character)
- Subscribe to device-specific topics
- Publish channel updates with JSON formatting
- Automatic reconnection handling
- Message callback support
- Channel-specific callbacks
- Multiple data type support (bool, int, float, string)

## Installation

### Arduino IDE

1. Download the library files:

   - `FastIoT.h`
   - `FastIoT.cpp`

2. Place them in your Arduino libraries folder:

   - Windows: `Documents/Arduino/libraries/FastIoT/`
   - Mac: `~/Documents/Arduino/libraries/FastIoT/`
   - Linux: `~/Arduino/libraries/FastIoT/`

3. Install required dependencies through Arduino IDE Library Manager:
   - **ESP8266WiFi** (usually pre-installed with ESP8266 board package)
   - **PubSubClient** by Nick O'Leary
   - **ArduinoJson** by Benoit Blanchon
   - **WiFiManager** by tzapu (for WiFiManager functionality)

### PlatformIO

1. Create a new PlatformIO project or open an existing one
2. Add the following dependencies to your `platformio.ini` file:

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
lib_deps =
  knolleary/PubSubClient @ ^2.8
  bblanchon/ArduinoJson @ ^6.21.3
  tzapu/WiFiManager @ ^0.16.0
  https://github.com/gnauqoa/fast-iot-pio.git
```

3. Create a folder named `lib/FastIoT` in your project
4. Copy `FastIoT.h` and `FastIoT.cpp` into this folder

## Dependencies

```cpp
#include <ESP8266WiFi.h>      // For WiFi connectivity
#include <PubSubClient.h>     // For MQTT functionality
#include <ArduinoJson.h>      // For JSON message formatting
#include <WiFiManager.h>      // Optional, for WiFi configuration portal
```

## Basic Usage

### Standard WiFi Connection Example

```cpp
#include "FastIoT.h"

FastIoT mqttClient;

void onMessageReceived(String topic, String message);

void setup() {
    Serial.begin(115200);

    // Initialize MQTT client
    mqttClient.begin("localhost", 1883, "token-password", "deviceId");

    // Set message callback
    mqttClient.setCallback(onMessageReceived);

    // Connect to WiFi
    mqttClient.connectWiFi("your_ssid", "your_password");

    // Connect to MQTT
    mqttClient.connectMQTT();
}

void loop() {
    mqttClient.loop();

    // Publish channel update
    mqttClient.publishChannelUpdate("v1", true);
    mqttClient.publishChannelUpdate("v3", 50);

    delay(5000);
}

void onMessageReceived(String topic, String message) {
    Serial.println("Topic: " + topic);
    Serial.println("Message: " + message);
}
```

### WiFiManager Integration Example

```cpp
#include <FastIoT.h>
#include <WiFiManager.h> // Include WiFiManager library

// MQTT Configuration
const String mqttUrl = "192.168.1.59"; // or your MQTT broker IP
const int mqttPort = 1883;
const String token = "2c485ca869edc979a22a465f048743c9-e8216664d1ba6e586a4751f186eff902";
const String deviceId = "377";

// Create FastIoT client instance
FastIoT iotClient;
WiFiManager wifiManager;

// Variables for LED control
bool ledState = false;
unsigned long lastToggle = 0;
const unsigned long toggleInterval = 10000; // 10 seconds

// Variables for WiFiManager
bool shouldSaveConfig = false;
char mqtt_server[40] = "192.168.1.59";
char mqtt_port[6] = "1883";
char mqtt_token[80] = "2c485ca869edc979a22a465f048743c9-e8216664d1ba6e586a4751f186eff902";
char mqtt_device_id[20] = "377";

void onV1ChannelChange(String channelName, JsonVariant value);
void onV2ChannelChange(String channelName, JsonVariant value);
void onLedChannelChange(String channelName, JsonVariant value);
void saveConfigCallback();

void setup()
{
    Serial.begin(9600);
    delay(1000);

    Serial.println("FastIoT Client with WiFiManager Example");

    // Initialize built-in LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // LED is off (inverted logic)

    // Set callback for saving config
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    // Add custom parameters to WiFiManager
    WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 40);
    WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_token("token", "MQTT Token", mqtt_token, 80);
    WiFiManagerParameter custom_mqtt_device_id("device_id", "Device ID", mqtt_device_id, 20);

    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_token);
    wifiManager.addParameter(&custom_mqtt_device_id);

    // Set custom AP name
    wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
    wifiManager.autoConnect("FastIoT-Setup");

    Serial.println("WiFi connected!");

    // Read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_token, custom_mqtt_token.getValue());
    strcpy(mqtt_device_id, custom_mqtt_device_id.getValue());

    // Initialize IoT client with parameters from WiFiManager
    iotClient.begin(mqtt_server, atoi(mqtt_port), mqtt_token, mqtt_device_id);

    // Set channel-specific callbacks
    iotClient.onChannelChange("v1", onV1ChannelChange);
    iotClient.onChannelChange("v2", onV2ChannelChange);
    iotClient.onChannelChange("led", onLedChannelChange);

    // Connect to MQTT broker
    if (iotClient.connectMQTT())
    {
        Serial.println("Connected to MQTT broker!");
        Serial.println("Setup completed successfully!");
    }
    else
    {
        wifiManager.resetSettings();
        Serial.println("Failed to connect to MQTT broker");
    }
}

void loop()
{
    // Keep MQTT connection alive
    iotClient.loop();

    // Auto-toggle LED and publish every 10 seconds
    if (millis() - lastToggle > toggleInterval)
    {
        // Publish channel updates
        int randomV1 = random(0, 100); // Giá trị ngẫu nhiên từ 0 đến 99
        int randomV2 = random(0, 100);
        float fakeLat = 10.1289929;
        float fakeLng = 106.3272224;

        iotClient.publishChannelUpdate("v1", randomV1);
        iotClient.publishChannelUpdate("v2", randomV2);
        iotClient.updateLocation(fakeLat, fakeLng);

        lastToggle = millis();
    }

    // Check for serial input (like the readline in Node.js)
    if (Serial.available())
    {
        String input = Serial.readStringUntil('\n');
        input.trim();
        input.toLowerCase();

        if (input == "on" || input == "off")
        {
            bool state = (input == "on");

            // Update LED
            digitalWrite(LED_BUILTIN, state ? HIGH : LOW); // Inverted logic
            ledState = state;

            iotClient.publishChannelUpdate("led", state);
            Serial.println("Manual control - LED State: " + String(state ? "ON" : "OFF"));
        }
        else if (input == "reset")
        {
            Serial.println("Resetting WiFi settings...");
            wifiManager.resetSettings();
            ESP.restart();
        }
        else
        {
            Serial.println("Invalid input. Please type 'on', 'off', or 'reset'.");
        }
        Serial.println("Type 'on' or 'off' to control LED, or 'reset' to reconfigure WiFi:");
    }

    delay(100);
}

// Callback when configuration is saved
void saveConfigCallback()
{
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

// Channel-specific callback functions
void onV1ChannelChange(String channelName, JsonVariant value)
{
    Serial.println("V1 Channel changed!");
    Serial.println("Channel: " + channelName);
    Serial.print("Value: ");
    Serial.println(value.as<String>());

    // Handle v1 channel (LED control)
    if (value.is<bool>())
    {
        bool ledValue = value.as<bool>();
        digitalWrite(LED_BUILTIN, ledValue ? LOW : HIGH); // Inverted logic
        ledState = ledValue;
        Serial.println("LED controlled via channel callback: " + String(ledValue ? "ON" : "OFF"));
    }
}

void onV2ChannelChange(String channelName, JsonVariant value)
{
    Serial.println("V2 Channel changed!");
    Serial.println("Channel: " + channelName);
    Serial.print("Value: ");
    Serial.println(value.as<String>());

    if (value.is<int>())
    {
        int sensorValue = value.as<int>();
        Serial.println("Sensor value updated: " + String(sensorValue));

        if (sensorValue > 75)
        {
            Serial.println("High sensor value detected!");
        }
    }
}

void onLedChannelChange(String channelName, JsonVariant value)
{
    Serial.println("LED Channel changed!");
    Serial.println("Channel: " + channelName);
    Serial.print("Value: ");
    Serial.println(value.as<String>());

    if (value.is<bool>())
    {
        bool ledValue = value.as<bool>();
        digitalWrite(LED_BUILTIN, ledValue ? HIGH : LOW);
        ledState = ledValue;
        Serial.println("LED controlled via LED channel: " + String(ledValue ? "ON" : "OFF"));
    }
    else if (value.is<String>())
    {
        String ledCommand = value.as<String>();
        if (ledCommand == "on" || ledCommand == "true")
        {
            digitalWrite(LED_BUILTIN, HIGH);
            ledState = true;
            Serial.println("LED turned ON via string command");
        }
        else if (ledCommand == "off" || ledCommand == "false")
        {
            digitalWrite(LED_BUILTIN, LOW);
            ledState = false;
            Serial.println("LED turned OFF via string command");
        }
    }
}
```

## Using with PlatformIO

### Project Setup

1. **Create a new project**:

   ```bash
   pio init --board nodemcuv2
   ```

2. **Directory Structure**:

   ```
   project_folder/
   ├── lib/
   │   └── FastIoT/
   │       ├── FastIoT.h
   │       └── FastIoT.cpp
   ├── src/
   │   └── main.cpp
   └── platformio.ini
   ```

3. **Configure platformio.ini**:

   ```ini
   [env:nodemcuv2]
   platform = espressif8266
   board = nodemcuv2
   framework = arduino
   lib_deps =
     knolleary/PubSubClient @ ^2.8
     bblanchon/ArduinoJson @ ^6.21.3
     tzapu/WiFiManager @ ^0.16.0
   monitor_speed = 115200
   ```

4. **Basic main.cpp**:

   ```cpp
   #include <Arduino.h>
   #include "FastIoT.h"
   #include <WiFiManager.h>

   // Your code from examples above
   ```

### Building and Uploading

1. Build the project:

   ```bash
   pio run
   ```

2. Upload to your device:

   ```bash
   pio run --target upload
   ```

3. Monitor serial output:

   ```bash
   pio device monitor
   ```

## Using WiFiManager

WiFiManager creates a configuration portal if your ESP8266 can't connect to a previously saved WiFi network.

### How it Works

1. **First Boot**: The device creates a WiFi access point (default: "FastIoT-Setup")
2. **Configuration**:
   - Connect to this access point with your phone/computer
   - A captive portal appears (or browse to 10.0.1.1)
   - Enter your WiFi credentials and MQTT settings
   - Save configuration
3. **Normal Operation**: Device connects to your WiFi and MQTT server using saved credentials
4. **Reset**: To reset configuration, you can add code to clear settings:

   ```cpp
   // Add to setup() for factory reset:
   // wifiManager.resetSettings();

   // Or trigger via a button or command
   if (digitalRead(RESET_PIN) == LOW) {
       wifiManager.resetSettings();
       ESP.restart();
   }
   ```

### Custom Parameters

WiFiManager allows adding custom parameters for MQTT configuration:

1. **Define variables** to store configuration
2. **Create parameter objects** with WiFiManagerParameter
3. **Add parameters** to WiFiManager with addParameter()
4. **Read values** after connection

### Configuration Portal Timeout

Set how long the configuration portal stays active:

```cpp
wifiManager.setConfigPortalTimeout(180); // 3 minutes timeout
```

## API Reference

### Constructor

```cpp
FastIoT()
```

### Methods

#### begin()

```cpp
void begin(String url, int port, String token, String deviceId)
```

Initialize the MQTT client with connection parameters.

**Parameters:**

- `url`: MQTT broker URL/IP address
- `port`: MQTT broker port (typically 1883)
- `token`: Authentication token in format "username-password"
- `deviceId`: Unique device identifier

#### connectWiFi()

```cpp
bool connectWiFi(String ssid, String password)
```

Connect to WiFi network.

**Returns:** `true` if connection successful, `false` otherwise

#### connectMQTT()

```cpp
bool connectMQTT()
```

Connect to MQTT broker using configured credentials.

**Returns:** `true` if connection successful, `false` otherwise

#### setCallback()

```cpp
void setCallback(void (*callback)(String topic, String message))
```

Set callback function for received messages.

#### onChannelChange()

```cpp
void onChannelChange(String channelName, void (*callback)(String channelName, JsonVariant value))
```

Set callback function for a specific channel.

**Parameters:**

- `channelName`: Name of the channel to watch (e.g., "v1", "led")
- `callback`: Function to call when this channel changes

#### subscribe()

```cpp
bool subscribe()
```

Subscribe to the device topic (`device/{deviceId}`).

**Returns:** `true` if subscription successful, `false` otherwise

#### publishChannelUpdate()

```cpp
bool publishChannelUpdate(String channelName, bool channelValue)
bool publishChannelUpdate(String channelName, int channelValue)
bool publishChannelUpdate(String channelName, float channelValue)
bool publishChannelUpdate(String channelName, String channelValue)
```

Publish channel update to topic `device/{deviceId}/update`.

**Parameters:**

- `channelName`: Name of the channel (e.g., "v1", "v3")
- `channelValue`: Value to publish (supports multiple types)

**Returns:** `true` if publish successful, `false` otherwise

#### loop()

```cpp
void loop()
```

Maintain MQTT connection and handle incoming messages. Call this in your main loop.

#### isConnected()

```cpp
bool isConnected()
```

Check if MQTT client is connected.

**Returns:** `true` if connected, `false` otherwise

#### disconnect()

```cpp
void disconnect()
```

Disconnect from MQTT broker.

#### getDeviceTopic()

```cpp
String getDeviceTopic()
```

Get the device subscription topic.

**Returns:** Topic string (`device/{deviceId}`)

#### getUpdateTopic()

```cpp
String getUpdateTopic()
```

Get the device update/publish topic.

**Returns:** Topic string (`device/{deviceId}/update`)

## Message Format

Published messages follow this JSON format:

```json
{
  "id": 789,
  "channelName": "v1",
  "channelValue": true
}
```

## Topics

- **Subscribe topic:** `device/{deviceId}`
- **Publish topic:** `device/{deviceId}/update`

## Error Handling

The library includes:

- Automatic reconnection for lost MQTT connections
- WiFi connection status checking
- Serial output for debugging connection issues
- Return values for all connection methods

## Notes

- The library automatically handles reconnection if the MQTT connection is lost
- Token format should be "username-password" (will be split on the '-' character)
- Built-in LED control is included in the example (inverted logic for ESP8266)
- Serial monitor output provides detailed connection and message information

## Example Output

```
Device ID: 789
Subscribe Topic: device/789
Publish Topic: device/789/update
Connected to MQTT broker. Subscribed to topic: device/789
Published: {"id":789,"channelName":"v1","channelValue":true}
Published: {"id":789,"channelName":"v3","channelValue":50}
```

## Troubleshooting

1. **WiFi connection fails**:

   - Check SSID and password
   - Try using WiFiManager to avoid hardcoded credentials

2. **MQTT connection fails**:

   - Verify broker URL, port, and credentials
   - Check network connectivity

3. **Messages not received**:

   - Ensure subscription was successful
   - Verify topic format

4. **WiFiManager portal not appearing**:

   - Make sure you're connecting to the "FastIoT-Setup" access point
   - Try accessing 10.0.1.1 in a browser
   - Check serial output for status information

5. **PlatformIO build errors**:
   - Ensure library dependencies are correctly specified in platformio.ini
   - Check file paths and import statements

For more detailed debugging, monitor the Serial output which provides comprehensive status information.

## License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.