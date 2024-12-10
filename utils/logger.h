#ifndef _LOGGER_H_
#define _LOGGER_H_  
#include "esp_log.h"

#define LOGI(x, ...) \
ESP_LOGI("", x, ##__VA_ARGS__)
#define LOGD(x, ...) \
ESP_LOGD("", x, ##__VA_ARGS__)
#define LOGE(x, ...) \
ESP_LOGE("", x, ##__VA_ARGS__)

#endif