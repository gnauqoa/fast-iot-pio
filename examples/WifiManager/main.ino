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