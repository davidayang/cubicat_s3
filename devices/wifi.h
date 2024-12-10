#ifndef _WIFI_H_
#define _WIFI_H_
#include <string>

class Wifi {
    friend class Cubicat;
public:
    static void init();
    static bool connect(const char* ssid, const char* password);
    static bool isConnected() { return m_bConnected; }
    static void onConnected(const char* ip) { m_bConnected = true; m_ip = ip; }
private:
    Wifi();
    static bool                 m_bConnected;   
    static std::string          m_ip;
};
#endif