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

    // Parse token
    int dashIndex = token.indexOf('-');
    if (dashIndex > 0) {
        username = token.substring(0, dashIndex);
        password = token.substring(dashIndex + 1);
    } else {
        username = token;
        password = "";
    }

    topic = "device/" + deviceId;
    updateTopic = "device/" + deviceId + "/update";

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

    // Ensure random is seeded once (only needed if not already done elsewhere)
    static bool seeded = false;
    if (!seeded) {
        randomSeed(micros());
        seeded = true;
    }

    String clientId = "ESP32Client-" + deviceId + "-" + String(random(0xffff), HEX);

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

void FastIoT::onChannelChange(String channelName, void (*callback)(String channelName, JsonVariant value)) {
    ChannelCallback* current = channelCallbacks;
    while (current != nullptr) {
        if (current->channelName == channelName) {
            current->callback = callback;
            Serial.println("Updated callback for channel: " + channelName);
            return;
        }
        current = current->next;
    }

    ChannelCallback* newCallback = new ChannelCallback();
    newCallback->channelName = channelName;
    newCallback->callback = callback;
    newCallback->next = channelCallbacks;
    channelCallbacks = newCallback;

    Serial.println("Added callback for channel: " + channelName);
}

void FastIoT::removeChannelCallback(String channelName) {
    ChannelCallback* current = channelCallbacks;
    ChannelCallback* previous = nullptr;

    while (current != nullptr) {
        if (current->channelName == channelName) {
            if (previous == nullptr) {
                channelCallbacks = current->next;
            } else {
                previous->next = current->next;
            }
            delete current;
            Serial.println("Removed callback for channel: " + channelName);
            return;
        }
        previous = current;
        current = current->next;
    }

    Serial.println("Callback not found for channel: " + channelName);
}

bool FastIoT::subscribe() {
    if (mqttClient.connected()) {
        bool result = mqttClient.subscribe(topic.c_str());
        Serial.println(result ? "Successfully subscribed to: " + topic
                              : "Failed to subscribe to: " + topic);
        return result;
    }
    return false;
}

bool FastIoT::publishChannelUpdate(String channelName, bool channelValue) {
    DynamicJsonDocument doc(1024);
    doc["id"] = deviceId.toInt();
    doc["channelName"] = channelName;
    doc["channelValue"] = channelValue;

    String payload;
    serializeJson(doc, payload);

    bool result = mqttClient.publish(updateTopic.c_str(), payload.c_str());
    Serial.println(result ? "Published: " + payload : "Failed to publish message");
    return result;
}

bool FastIoT::publishChannelUpdate(String channelName, int channelValue) {
    DynamicJsonDocument doc(1024);
    doc["id"] = deviceId.toInt();
    doc["channelName"] = channelName;
    doc["channelValue"] = channelValue;

    String payload;
    serializeJson(doc, payload);

    bool result = mqttClient.publish(updateTopic.c_str(), payload.c_str());
    Serial.println(result ? "Published: " + payload : "Failed to publish message");
    return result;
}

bool FastIoT::publishChannelUpdate(String channelName, float channelValue) {
    DynamicJsonDocument doc(1024);
    doc["id"] = deviceId.toInt();
    doc["channelName"] = channelName;
    doc["channelValue"] = channelValue;

    String payload;
    serializeJson(doc, payload);

    bool result = mqttClient.publish(updateTopic.c_str(), payload.c_str());
    Serial.println(result ? "Published: " + payload : "Failed to publish message");
    return result;
}

bool FastIoT::publishChannelUpdate(String channelName, String channelValue) {
    DynamicJsonDocument doc(1024);
    doc["id"] = deviceId.toInt();
    doc["channelName"] = channelName;
    doc["channelValue"] = channelValue;

    String payload;
    serializeJson(doc, payload);

    bool result = mqttClient.publish(updateTopic.c_str(), payload.c_str());
    Serial.println(result ? "Published: " + payload : "Failed to publish message");
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

    if (doc.containsKey("channelName") && doc.containsKey("channelValue")) {
        String channelName = doc["channelName"];
        JsonVariant channelValue = doc["channelValue"];

        Serial.println("Channel update - " + channelName + ": " + channelValue.as<String>());

        ChannelCallback* current = channelCallbacks;
        while (current != nullptr) {
            if (current->channelName == channelName && current->callback != nullptr) {
                current->callback(channelName, channelValue);
                break;
            }
            current = current->next;
        }
    }
}

void FastIoT::internalCallback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    String topicStr = String(topic);
    Serial.println("Received message on " + topicStr + ": " + message);

    if (instance) {
        instance->processChannelMessage(message);
        if (instance->messageCallback) {
            instance->messageCallback(topicStr, message);
        }
    }
}
