#include "wifi.h"
#include <stdlib.h>
#include "esp_wifi.h"
#include <string.h>
#include "nvs_flash.h"

#define MAX_RETRY 5
int g_retry = 0;
bool Wifi::m_bConnected = false;
std::string Wifi::m_ip = "";
static EventGroupHandle_t g_wifiEventGroup;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

Wifi::Wifi() {
}
Wifi::~Wifi() {
}
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (g_retry < MAX_RETRY) {
            printf("Retry connecting to the wifi AP\n");
            g_retry++;
            vTaskDelay(500 / portTICK_PERIOD_MS);
            esp_wifi_connect(); // 重新连接
        } else {
            wifi_event_sta_disconnected_t* disconnection = (wifi_event_sta_disconnected_t*)event_data;
            printf("Disconnected from SSID: %s, reason: %d\n", disconnection->ssid, disconnection->reason);
            xEventGroupSetBits(g_wifiEventGroup, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        char ipstr[16] = {0};
        sprintf(ipstr, IPSTR, IP2STR(&event->ip_info.ip));
        Wifi::onConnected(ipstr);
        xEventGroupSetBits(g_wifiEventGroup, WIFI_CONNECTED_BIT);
    }
}
void Wifi::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase()); // 如果没有可用的分区，擦除 NVS
        ret = nvs_flash_init();
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
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);
    g_wifiEventGroup = xEventGroupCreate();
}
bool Wifi::connect(const char* ssid, const char* password) {
    if (m_bConnected)
        return true;
    g_retry = 0;
    // 配置 Wi-Fi 连接
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);
    wifi_config.sta.threshold.authmode = strlen(password)? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_OPEN;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // 等待连接成功和获取 IP 地址
    EventBits_t bits = xEventGroupWaitBits(g_wifiEventGroup, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                            pdTRUE, pdFALSE, portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        printf("Wi-Fi connected. got IP address. %s\n", m_ip.c_str());
        return true;
    }
    if (bits & WIFI_FAIL_BIT) {
        printf("Failed to connect to the AP\n");
        return false;
    }
    return false;
}