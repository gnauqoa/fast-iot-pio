#include "FastIoT.h"

// Static instance for callback
FastIoT* FastIoT::instance = nullptr;

FastIoT::FastIoT() : mqttClient(wifiClient) {
    instance = this;
    messageCallback = nullptr;
    channelCallbacks = nullptr;
    brokerPort = 1883;
}

FastIoT::~FastIoT() {
    // Clean up channel callbacks
    ChannelCallback* current = channelCallbacks;
    while (current != nullptr) {
        ChannelCallback* next = current->next;
        delete current;
        current = next;
    }
}

void FastIoT::begin(String url, int port, String token, String devId) {
    brokerUrl = url;
    brokerPort = port;
    deviceId = devId;
    
    // Parse token (username-password format)
    int dashIndex = token.indexOf('-');
    if (dashIndex > 0) {
        username = token.substring(0, dashIndex);
        password = token.substring(dashIndex + 1);
    } else {
        username = token;
        password = "";
    }
    
    // Set up topics
    topic = "device/" + deviceId;
    updateTopic = "device/" + deviceId + "/update";
    
    // Configure MQTT client
    mqttClient.setServer(brokerUrl.c_str(), brokerPort);
    mqttClient.setCallback(internalCallback);
    
    Serial.println("FastIoT Client initialized");
    Serial.println("Device ID: " + deviceId);
    Serial.println("Subscribe Topic: " + topic);
    Serial.println("Publish Topic: " + updateTopic);
}

bool FastIoT::connectWiFi(String ssid, String wifiPassword) {
    WiFi.begin(ssid.c_str(), wifiPassword.c_str());
    
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println();
        Serial.println("WiFi connection failed!");
        return false;
    }
}

bool FastIoT::connectMQTT() {
    if (!WiFi.isConnected()) {
        Serial.println("WiFi not connected. Cannot connect to MQTT.");
        return false;
    }
    
    Serial.print("Connecting to MQTT broker...");
    
    String clientId = "ESP8266Client-" + deviceId + "-" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), username.c_str(), password.c_str())) {
        Serial.println(" connected!");
        Serial.println("Connected to MQTT broker. Subscribed to topic: " + topic);
        return subscribe();
    } else {
        Serial.print(" failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" retrying in 5 seconds");
        return false;
    }
}

void FastIoT::setCallback(void (*callback)(String topic, String message)) {
    messageCallback = callback;
}

void FastIoT::onChannelChange(String name, void (*callback)(String name, JsonVariant value)) {
    // Check if callback already exists for this channel
    ChannelCallback* current = channelCallbacks;
    while (current != nullptr) {
        if (current->name == name) {
            // Update existing callback
            current->callback = callback;
            Serial.println("Updated callback for channel: " + name);
            return;
        }
        current = current->next;
    }
    
    // Create new callback entry
    ChannelCallback* newCallback = new ChannelCallback();
    newCallback->name = name;
    newCallback->callback = callback;
    newCallback->next = channelCallbacks;
    channelCallbacks = newCallback;
    
    Serial.println("Added callback for channel: " + name);
}

void FastIoT::removeChannelCallback(String name) {
    ChannelCallback* current = channelCallbacks;
    ChannelCallback* previous = nullptr;
    
    while (current != nullptr) {
        if (current->name == name) {
            if (previous == nullptr) {
                // Removing first element
                channelCallbacks = current->next;
            } else {
                previous->next = current->next;
            }
            delete current;
            Serial.println("Removed callback for channel: " + name);
            return;
        }
        previous = current;
        current = current->next;
    }
    
    Serial.println("Callback not found for channel: " + name);
}

bool FastIoT::subscribe() {
    if (mqttClient.connected()) {
        bool result = mqttClient.subscribe(topic.c_str());
        if (result) {
            Serial.println("Successfully subscribed to: " + topic);
        } else {
            Serial.println("Failed to subscribe to: " + topic);
        }
        return result;
    }
    return false;
}

bool FastIoT::publishChannelUpdate(String name, bool channelValue) {
    DynamicJsonDocument doc(64);
    doc["value"] = channelValue;

    ChannelUpdate updates[1] = {
        { name, doc["value"] }
    };

    return publishChannelUpdates(updates, 1);
}

bool FastIoT::publishChannelUpdate(String name, int channelValue) {
    DynamicJsonDocument doc(64);
    doc["value"] = channelValue;

    ChannelUpdate updates[1] = {
        { name, doc["value"] }
    };

    return publishChannelUpdates(updates, 1);
}

bool FastIoT::publishChannelUpdate(String name, float channelValue) {
    DynamicJsonDocument doc(64);
    doc["value"] = channelValue;

    ChannelUpdate updates[1] = {
        { name, doc["value"] }
    };

    return publishChannelUpdates(updates, 1);
}

bool FastIoT::publishChannelUpdate(String name, String channelValue) {
    DynamicJsonDocument doc(64);
    doc["value"] = channelValue;

    ChannelUpdate updates[1] = {
        { name, doc["value"] }
    };

    return publishChannelUpdates(updates, 1);
}

bool FastIoT::publishChannelUpdates(ChannelUpdate updates[], size_t count) {
    if (!mqttClient.connected()) {
        Serial.println("MQTT not connected. Cannot publish.");
        return false;
    }
    DynamicJsonDocument doc(1024);
    doc["id"] = deviceId.toInt();

    JsonArray channels = doc.createNestedArray("channels");

    for (size_t i = 0; i < count; i++) {
        JsonObject chanObj = channels.createNestedObject();
        chanObj["name"] = updates[i].name;
        chanObj["value"] = updates[i].value;
    }

    String payload;
    serializeJson(doc, payload);

    bool result = mqttClient.publish(updateTopic.c_str(), payload.c_str());

    if (result) {
        Serial.println("Published: " + payload);
    } else {
        Serial.println("Failed to publish message");
    }

    return result;
}

bool FastIoT::updateLocation(float latitude, float longitude) {
    if (!mqttClient.connected()) {
        Serial.println("MQTT not connected. Cannot publish.");
        return false;
    }

    DynamicJsonDocument doc(256);
    doc["id"] = deviceId.toInt();
    doc["latitude"] = String(latitude, 6);   // giữ 6 chữ số sau dấu chấm
    doc["longitude"] = String(longitude, 6);

    String payload;
    serializeJson(doc, payload);

    bool result = mqttClient.publish(updateTopic.c_str(), payload.c_str());

    if (result) {
        Serial.println("Published location: " + payload);
    } else {
        Serial.println("Failed to publish location");
    }

    return result;
}

void FastIoT::loop() {
    if (!mqttClient.connected()) {
        Serial.println("MQTT connection lost. Attempting to reconnect...");
        if (connectMQTT()) {
            Serial.println("MQTT reconnected successfully.");
        } else {
            delay(5000);
        }
    }
    mqttClient.loop();
}

bool FastIoT::isConnected() {
    return mqttClient.connected();
}

void FastIoT::disconnect() {
    mqttClient.disconnect();
    Serial.println("Disconnected from MQTT broker");
}

String FastIoT::getDeviceTopic() {
    return topic;
}

String FastIoT::getUpdateTopic() {
    return updateTopic;
}

void FastIoT::processChannelMessage(String message) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Serial.println("Failed to parse message JSON: " + String(error.c_str()));
        return;
    }

    if (doc.is<JsonArray>()) {
        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject obj : arr) {
            if (obj.containsKey("name") && obj.containsKey("value")) {
                String name = obj["name"];
                JsonVariant value = obj["value"];

                Serial.println("Channel update - " + name + ": " + value.as<String>());

                ChannelCallback* current = channelCallbacks;
                while (current != nullptr) {
                    if (current->name == name && current->callback != nullptr) {
                        current->callback(name, value);
                        break;
                    }
                    current = current->next;
                }
            }
        }
    } else if (doc.is<JsonObject>()) {
        if (doc.containsKey("name") && doc.containsKey("value")) {
            String name = doc["name"];
            JsonVariant value = doc["value"];

            Serial.println("Channel update - " + name + ": " + value.as<String>());

            ChannelCallback* current = channelCallbacks;
            while (current != nullptr) {
                if (current->name == name && current->callback != nullptr) {
                    current->callback(name, value);
                    break;
                }
                current = current->next;
            }
        }
    } else {
        Serial.println("Invalid message format");
    }
}

// Static callback function
void FastIoT::internalCallback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to string
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    String topicStr = String(topic);
    
    Serial.println("Received message on " + topicStr + ": " + message);
    
    // Process channel-specific callbacks first
    if (instance) {
        instance->processChannelMessage(message);
    }
    
    // Call general message callback if set
    if (instance && instance->messageCallback) {
        instance->messageCallback(topicStr, message);
    }
}