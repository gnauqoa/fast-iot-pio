#ifndef FASTIOT_H
#define FASTIOT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

class FastIoT {
public:
    FastIoT();
    ~FastIoT();

    void begin(String url, int port, String token, String devId);
    bool connectWiFi(String ssid, String wifiPassword);
    bool connectMQTT();
    bool subscribe();
    void loop();
    void disconnect();
    bool isConnected();

    void setCallback(void (*callback)(String topic, String message));
    void onChannelChange(String channelName, void (*callback)(String channelName, JsonVariant value));
    void removeChannelCallback(String channelName);

    bool publishChannelUpdate(String channelName, bool channelValue);
    bool publishChannelUpdate(String channelName, int channelValue);
    bool publishChannelUpdate(String channelName, float channelValue);
    bool publishChannelUpdate(String channelName, String channelValue);

    String getDeviceTopic();
    String getUpdateTopic();

private:
    static FastIoT* instance;

    WiFiClient wifiClient;
    PubSubClient mqttClient;

    String brokerUrl;
    int brokerPort;
    String username;
    String password;
    String deviceId;
    String topic;
    String updateTopic;

    void (*messageCallback)(String topic, String message);

    struct ChannelCallback {
        String channelName;
        void (*callback)(String channelName, JsonVariant value);
        ChannelCallback* next;
    };

    ChannelCallback* channelCallbacks;

    static void internalCallback(char* topic, byte* payload, unsigned int length);
    void processChannelMessage(String message);
};

#endif
