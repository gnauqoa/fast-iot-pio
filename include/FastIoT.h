#ifndef FASTIOT_H
#define FASTIOT_H
    
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

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

    // Callback setters
    void setCallback(void (*callback)(String topic, String message));
    void onChannelChange(String channelName, void (*callback)(String channelName, JsonVariant value));
    void removeChannelCallback(String channelName);

    // Publishing data
    bool publishChannelUpdate(String channelName, bool channelValue);
    bool publishChannelUpdate(String channelName, int channelValue);
    bool publishChannelUpdate(String channelName, float channelValue);
    bool publishChannelUpdate(String channelName, String channelValue);

    // Topic getters
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
