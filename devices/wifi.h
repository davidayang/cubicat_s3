#ifndef _WIFI_H_
#define _WIFI_H_
#include <string>
#include "esp_wifi.h"
#include <functional>

typedef std::function<void(bool success,const char* ip)> ConnectCallback;
enum SmartConfigState {
    StartFinding,
    StartConnecting,
    StopFinding
};
typedef std::function<void(SmartConfigState status)> SmartConfigStatusCallback;

class Wifi {
    friend class Cubicat;
public:
    enum ConnectState {
        CONNECT_SUCCESS,
        CONNECT_FAIL,
        CONNECTING,  
    };
    Wifi(const Wifi&) = delete;
    Wifi& operator=(const Wifi&) = delete;
    ~Wifi();
    // [JS_BINDING_BEGIN]
    static bool connect(std::string ssid, std::string password);
    // [JS_BINDING_END]
    static void connectAsync(std::string ssid, std::string password, ConnectCallback callback);
    // connect to wifi with smartconfig
    static void smartConnect(ConnectCallback callback, SmartConfigStatusCallback statusCallback = nullptr);
    static bool isConnected() { return m_sState == CONNECT_SUCCESS; }
    static bool isConnecting() { return m_sState == CONNECTING; }
    static void disconnect();
    std::string getSSID() { return m_sSSID; }
    void removeStoredSSID();
private:
    Wifi();
    static void init();
    static void setState(ConnectState state) { m_sState = state; }
    static void eventLoop();
    static void onConnected(bool success, const char* ip);
    static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    static void readStoredSSIDAndPassword();
    static ConnectState         m_sState;
    static std::string          m_sIP;
    static bool                 m_sbUseSmartConfig;
    static ConnectCallback      m_sCallbackFunc;
    static SmartConfigStatusCallback m_sSmartConfigStatusCallback;
    static QueueHandle_t        m_sQueueHandle;
    static std::string          m_sSSID;
    static std::string          m_sPASSWD;
};
#endif