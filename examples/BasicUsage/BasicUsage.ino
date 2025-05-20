#include "FastIoT.h"

// WiFi credentials
const char* ssid = "your_wifi_ssid";
const char* wifiPassword = "your_wifi_password";

// MQTT Configuration
const String mqttUrl = "localhost";  // or your MQTT broker IP
const int mqttPort = 1883;
const String token = "1eea43335fb2183303a9c3b72ca39776-34681cc95988732ae4a335cc7b967130";
const String deviceId = "789";

// Create FastIoT client instance
FastIoT iotClient;

// Variables for LED control
bool ledState = false;
unsigned long lastToggle = 0;
const unsigned long toggleInterval = 10000; // 10 seconds

void onV1ChannelChange(String channelName, JsonVariant value);
void onV3ChannelChange(String channelName, JsonVariant value);
void onLedChannelChange(String channelName, JsonVariant value);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("FastIoT Client Example");
    
    // Initialize built-in LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // LED is off (inverted logic)
    
    // Initialize IoT client
    iotClient.begin(mqttUrl, mqttPort, token, deviceId);
    
    // Set channel-specific callbacks
    iotClient.onChannelChange("v1", onV1ChannelChange);
    iotClient.onChannelChange("v3", onV3ChannelChange);
    iotClient.onChannelChange("led", onLedChannelChange);
    
    // Connect to WiFi
    if (iotClient.connectWiFi(ssid, wifiPassword)) {
        // Connect to MQTT broker
        if (iotClient.connectMQTT()) {
            Serial.println("Setup completed successfully!");
        } else {
            Serial.println("Failed to connect to MQTT broker");
        }
    } else {
        Serial.println("Failed to connect to WiFi");
    }
}

void loop() {
    // Keep MQTT connection alive
    iotClient.loop();
    
    // Auto-toggle LED and publish every 10 seconds
    if (millis() - lastToggle > toggleInterval) {
        ledState = !ledState;
        
        // Control built-in LED
        digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH); // Inverted logic
        
        // Publish channel updates
        iotClient.publishChannelUpdate("v1", ledState);
        iotClient.publishChannelUpdate("v3", 50);
        
        Serial.println("LED State: " + String(ledState ? "ON" : "OFF"));
        
        lastToggle = millis();
    }
    
    // Check for serial input (like the readline in Node.js)
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        input.toLowerCase();
        
        if (input == "on" || input == "off") {
            bool state = (input == "on");
            
            // Update LED
            digitalWrite(LED_BUILTIN, state ? LOW : HIGH); // Inverted logic
            ledState = state;
            
            // Publish updates
            iotClient.publishChannelUpdate("v1", state);
            iotClient.publishChannelUpdate("v3", 50);
            
            Serial.println("Manual control - LED State: " + String(state ? "ON" : "OFF"));
        } else {
            Serial.println("Invalid input. Please type 'on' or 'off'.");
        }
        Serial.println("Type 'on' or 'off' to control LED:");
    }
    
    delay(100);
}

// Channel-specific callback functions
void onV1ChannelChange(String channelName, JsonVariant value) {
    Serial.println("V1 Channel changed!");
    Serial.println("Channel: " + channelName);
    Serial.print("Value: ");
    Serial.println(value.as<String>());
    
    // Handle v1 channel (LED control)
    if (value.is<bool>()) {
        bool ledValue = value.as<bool>();
        digitalWrite(LED_BUILTIN, ledValue ? LOW : HIGH); // Inverted logic
        ledState = ledValue;
        Serial.println("LED controlled via channel callback: " + String(ledValue ? "ON" : "OFF"));
    }
}

void onV3ChannelChange(String channelName, JsonVariant value) {
    Serial.println("V3 Channel changed!");
    Serial.println("Channel: " + channelName);
    Serial.print("Value: ");
    Serial.println(value.as<String>());
    
    // Handle v3 channel (sensor value or other numeric data)
    if (value.is<int>()) {
        int sensorValue = value.as<int>();
        Serial.println("Sensor value updated: " + String(sensorValue));
        
        // You can add specific logic here based on the sensor value
        if (sensorValue > 75) {
            Serial.println("High sensor value detected!");
        }
    }
}

void onLedChannelChange(String channelName, JsonVariant value) {
    Serial.println("LED Channel changed!");
    Serial.println("Channel: " + channelName);
    Serial.print("Value: ");
    Serial.println(value.as<String>());
    
    // Handle led channel (direct LED control)
    if (value.is<bool>()) {
        bool ledValue = value.as<bool>();
        digitalWrite(LED_BUILTIN, ledValue ? LOW : HIGH); // Inverted logic
        ledState = ledValue;
        Serial.println("LED controlled via LED channel: " + String(ledValue ? "ON" : "OFF"));
    } else if (value.is<String>()) {
        String ledCommand = value.as<String>();
        if (ledCommand == "on" || ledCommand == "true") {
            digitalWrite(LED_BUILTIN, LOW);
            ledState = true;
            Serial.println("LED turned ON via string command");
        } else if (ledCommand == "off" || ledCommand == "false") {
            digitalWrite(LED_BUILTIN, HIGH);
            ledState = false;
            Serial.println("LED turned OFF via string command");
        }
    }
}