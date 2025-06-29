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

struct ChannelUpdate
{
    String name;
    JsonVariant value;
};

class FastIoT
{
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
    void onChannelChange(String name, void (*callback)(String name, JsonVariant value));
    void removeChannelCallback(String name);

    bool publishChannelUpdate(String name, bool channelValue);
    bool publishChannelUpdate(String name, int channelValue);
    bool publishChannelUpdate(String name, float channelValue);
    bool publishChannelUpdate(String name, String channelValue);
    bool publishChannelUpdates(ChannelUpdate updates[], size_t count);
    bool updateLocation(float latitude, float longitude);
    String getDeviceTopic();
    String getUpdateTopic();

private:
    static FastIoT *instance;

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

    struct ChannelCallback
    {
        String name;
        void (*callback)(String name, JsonVariant value);
        ChannelCallback *next;
    };

    ChannelCallback *channelCallbacks;

    static void internalCallback(char *topic, byte *payload, unsigned int length);
    void processChannelMessage(String message);
};

#endif
