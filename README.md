# FastIoT Library for ESP8266

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
#include "FastIoT.h"
#include <WiFiManager.h>

FastIoT iotClient;
WiFiManager wifiManager;

// Variables for WiFiManager
bool shouldSaveConfig = false;
char mqtt_server[40] = "localhost";
char mqtt_port[6] = "1883";
char mqtt_token[80] = "token-password";
char mqtt_device_id[20] = "789";

void onV1ChannelChange(String channelName, JsonVariant value);
void saveConfigCallback();

void setup() {
    Serial.begin(115200);
    
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
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    wifiManager.autoConnect("FastIoT-Setup");
    
    // Read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_token, custom_mqtt_token.getValue());
    strcpy(mqtt_device_id, custom_mqtt_device_id.getValue());

    // Initialize IoT client with parameters from WiFiManager
    iotClient.begin(mqtt_server, atoi(mqtt_port), mqtt_token, mqtt_device_id);

    // Set channel-specific callbacks
    iotClient.onChannelChange("v1", onV1ChannelChange);

    // Connect to MQTT broker
    iotClient.connectMQTT();
}

void loop() {
    iotClient.loop();
    
    // Your code here
    
    delay(100);
}

// Callback when configuration is saved
void saveConfigCallback() {
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

void onV1ChannelChange(String channelName, JsonVariant value) {
    Serial.println("V1 Channel changed: " + value.as<String>());
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
