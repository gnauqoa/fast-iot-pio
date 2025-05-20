# FastIoT Library for ESP8266

An Arduino IDE library for ESP8266 to connect to MQTT brokers with username/password authentication and channel-specific callbacks, designed to replicate and extend the functionality of the provided Node# ESP8266 MQTT Client Library

An Arduino IDE library for ESP8266 to connect to MQTT brokers with username/password authentication, designed to replicate the functionality of the provided Node.js MQTT client.

## Features

- WiFi connection management
- MQTT connection with username/password authentication
- Token parsing (splits on '-' character)
- Subscribe to device-specific topics
- Publish channel updates with JSON formatting
- Automatic reconnection handling
- Message callback support
- Multiple data type support (bool, int, float, string)

## Installation

1. Download the library files:
   - `ESP8266MQTTClient.h`
   - `ESP8266MQTTClient.cpp`

2. Place them in your Arduino libraries folder:
   - Windows: `Documents/Arduino/libraries/ESP8266MQTTClient/`
   - Mac: `~/Documents/Arduino/libraries/ESP8266MQTTClient/`
   - Linux: `~/Arduino/libraries/ESP8266MQTTClient/`

3. Install required dependencies through Arduino IDE Library Manager:
   - **ESP8266WiFi** (usually pre-installed with ESP8266 board package)
   - **PubSubClient** by Nick O'Leary
   - **ArduinoJson** by Benoit Blanchon

## Dependencies

```cpp
#include <ESP8266WiFi.h>      // For WiFi connectivity
#include <PubSubClient.h>     // For MQTT functionality
#include <ArduinoJson.h>      // For JSON message formatting
```

## Basic Usage

```cpp
#include "ESP8266MQTTClient.h"

ESP8266MQTTClient mqttClient;

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

## API Reference

### Constructor

```cpp
ESP8266MQTTClient()
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
ESP8266 MQTT Client initialized
Device ID: 789
Subscribe Topic: device/789
Publish Topic: device/789/update
Connected to MQTT broker. Subscribed to topic: device/789
Published: {"id":789,"channelName":"v1","channelValue":true}
Published: {"id":789,"channelName":"v3","channelValue":50}
```

## Troubleshooting

1. **WiFi connection fails**: Check SSID and password
2. **MQTT connection fails**: Verify broker URL, port, and credentials
3. **Messages not received**: Ensure subscription was successful
4. **Publish fails**: Check MQTT connection status

For more detailed debugging, monitor the Serial output which provides comprehensive status information.