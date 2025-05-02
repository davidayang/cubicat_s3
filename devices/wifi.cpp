#include "wifi.h"
#include <stdlib.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "esp_smartconfig.h"
#include "lwip/snmp.h"

#define MAX_RETRY 5
int g_retry = 0;
Wifi::ConnectState Wifi::m_sState = Wifi::CONNECT_FAIL;
std::string Wifi::m_sIP = "";
QueueHandle_t Wifi::m_sQueueHandle = nullptr;
bool Wifi::m_sbUseSmartConfig = false;
ConnectCallback Wifi::m_sCallbackFunc = nullptr;
SmartConfigStatusCallback Wifi::m_sSmartConfigStatusCallback = nullptr;
std::string Wifi::m_sSSID = "";
std::string Wifi::m_sPASSWD = "";

struct ConnectResult {
    bool success;
    char ip[16];
};
void initialize_sntp() {
    esp_sntp_stop();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

Wifi::Wifi() {
}
Wifi::~Wifi() {
}
void Wifi::wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        if (m_sbUseSmartConfig) {
            // start smartconfig
            ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
            smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
            ESP_ERROR_CHECK( esp_smartconfig_start(&cfg) );
            if (m_sSmartConfigStatusCallback) {
                m_sSmartConfigStatusCallback(SmartConfigState::StartFinding);
            }
        } else {
            esp_wifi_connect();
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (g_retry < MAX_RETRY) {
            printf("Retry connecting to the wifi AP\n");
            g_retry++;
            vTaskDelay(300 / portTICK_PERIOD_MS);
            esp_wifi_connect(); // 重新连接
        } else {
            wifi_event_sta_disconnected_t* disconnection = (wifi_event_sta_disconnected_t*)event_data;
            printf("Disconnected from SSID: %s, reason: %d\n", disconnection->ssid, disconnection->reason);
            static ConnectResult result;
            result.success = false;
            xQueueSend(m_sQueueHandle, &result, 0);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        static ConnectResult result;
        result.success = true;
        sprintf(result.ip, IPSTR, IP2STR(&event->ip_info.ip));
        xQueueSend(m_sQueueHandle, &result, 0);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        char ssid[33] = { 0 };
        char password[65] = { 0 };
        memcpy(ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(password, evt->password, sizeof(wifi_config.sta.password));
        disconnect();
        connectAsync(ssid, password, m_sCallbackFunc);
        if (m_sSmartConfigStatusCallback) {
            m_sSmartConfigStatusCallback(SmartConfigState::StartConnecting);
        }
    }
}
void Wifi::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase()); // 如果没有可用的分区，擦除 NVS
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(esp_netif_init());
    // 创建事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    // 初始化 Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_wifi_set_mode(WIFI_MODE_STA);
    // 注册事件处理
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifiEventHandler, NULL);
    esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL);
    m_sQueueHandle = xQueueCreate(1, sizeof(ConnectResult));
}
void Wifi::smartConnect(ConnectCallback callback, SmartConfigStatusCallback statusCallback) {
    if (isConnected())
        return;
    if (isConnecting())
        return;
    setState(CONNECTING);
    m_sbUseSmartConfig = true;
    m_sCallbackFunc = callback;
    m_sSmartConfigStatusCallback = statusCallback;
    ESP_ERROR_CHECK(esp_wifi_start());
}
bool Wifi::connect(std::string ssid, std::string password) {
    if (isConnected())
        return true;
    if (isConnected())
        return false;
    // use last saved ssid and password
    if (ssid.empty()) {
        readStoredSSIDAndPassword();
    } else {
        m_sSSID = ssid;
        m_sPASSWD = password;
    }
    if (m_sSSID.empty()) {
        return false;
    }
    m_sbUseSmartConfig = false;
    g_retry = 0;
    // config wifi ssid and password
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char*)wifi_config.sta.ssid, m_sSSID.c_str());
    if (!m_sPASSWD.empty())
        strcpy((char*)wifi_config.sta.password, m_sPASSWD.c_str());
    wifi_config.sta.threshold.authmode = m_sPASSWD.length() ? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_OPEN;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // 等待连接成功和获取 IP 地址
    ConnectResult result;
    if (xQueueReceive(m_sQueueHandle, &result, portMAX_DELAY) == pdTRUE) {
        onConnected(result.success, result.ip);
    }
    return isConnected();
}
void Wifi::connectAsync(std::string ssid, std::string password, ConnectCallback callback) {
    if (isConnecting())
        return;
    if (isConnected()) {
        if (callback)
            callback(true, m_sIP.c_str());
        return;
    }
    if (ssid.empty()) {
        readStoredSSIDAndPassword();
    } else {
        m_sSSID = ssid;
        m_sPASSWD = password;
    }
    if (m_sSSID.empty()) {
        if (callback)
            callback(false, "");
        return;
    }
    setState(CONNECTING);
    m_sCallbackFunc = callback;
    m_sbUseSmartConfig = false;
    g_retry = 0;
    // config wifi ssid and password
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char*)wifi_config.sta.ssid, m_sSSID.c_str());
    if (!m_sPASSWD.empty())
        strcpy((char*)wifi_config.sta.password, m_sPASSWD.c_str());
    wifi_config.sta.threshold.authmode = m_sPASSWD.length() ? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_OPEN;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}
void Wifi::onConnected(bool success, const char* ip) {
    setState(success ? CONNECT_SUCCESS : CONNECT_FAIL);
    if (success) {
        // save ssid and password
        nvs_handle_t nvs_handle;
        ESP_ERROR_CHECK(nvs_open("wifi_config", NVS_READWRITE, &nvs_handle));
        nvs_set_str(nvs_handle, "wifi_ssid", m_sSSID.c_str());
        nvs_set_str(nvs_handle, "wifi_password", m_sPASSWD.c_str());
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        m_sIP = ip;
        initialize_sntp();
    } else {
        disconnect();
    }
    if (m_sCallbackFunc) {
        m_sCallbackFunc(success, m_sIP.c_str());
        m_sCallbackFunc = nullptr;
    }
}
void Wifi::disconnect() {
    ESP_ERROR_CHECK(esp_wifi_stop());
    setState(CONNECT_FAIL);
    m_sbUseSmartConfig = false;
}

void Wifi::eventLoop() {
    if (isConnecting()) {
        ConnectResult result;
        if (xQueueReceive(m_sQueueHandle, &result, 0) == pdTRUE) {
            onConnected(result.success, result.ip);
        }
    }
}
void Wifi::readStoredSSIDAndPassword() {
    nvs_handle_t nvs_handle;
    if (nvs_open("wifi_config", NVS_READONLY, &nvs_handle) == ESP_OK) {
        char ssidbuf[32] = {0};
        char passwordbuf[64] = {0};
        size_t ssid_len = sizeof(ssidbuf);
        size_t password_len = sizeof(passwordbuf);
        nvs_get_str(nvs_handle, "wifi_ssid", ssidbuf, &ssid_len);
        nvs_get_str(nvs_handle, "wifi_password", passwordbuf, &password_len);
        if (ssid_len)
            m_sSSID = ssidbuf;
        if (password_len)
            m_sPASSWD = passwordbuf;
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    }
}
void Wifi::removeStoredSSID() {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("wifi_config", NVS_READWRITE, &nvs_handle));
    nvs_erase_key(nvs_handle, "wifi_ssid");
    nvs_erase_key(nvs_handle, "wifi_password");
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
}