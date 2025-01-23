#ifndef _HELPER_H_
#define _HELPER_H_
#include <string>
#include <vector>
#include <freertos/FreeRTOSConfig.h>
#include <time.h>
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

#define BENCHMARK(code)                \
    {                                  \
        int64_t startTime = micros();  \
        code;                          \
        int64_t endTime = micros();    \
        printf("file:%s line[%d]: %ld ms (%ld us)\n", __FILE__, __LINE__, uint32_t(endTime - startTime)/1000, uint32_t(endTime - startTime)); \
    }

#define MEMORY_REPORT memoryReport(__FILE__, __LINE__);

const int headerSize = 44;
extern void wavHeader(uint8_t* header, int wavSize);

inline int getMainCoreId() {
#ifdef CONFIG_ESP_MAIN_TASK_AFFINITY_CPU0
    return 0;
#else
    return 1;
#endif
}

inline int getSubCoreId() {
    return (getMainCoreId() + 1) % portNUM_PROCESSORS;
}
inline std::vector<std::string> splitString(const std::string& str, const char* delimiter) {
    std::vector<std::string> result;
    std::size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(str.substr(start));
    return result;
}
inline uint32_t millis() {
    return esp_timer_get_time() / 1000;
}
inline int64_t micros() {
    return esp_timer_get_time();
}
inline void memoryReport(const char* file, int line) {
    printf("<=====Memory report in %s:%d ======>\n", file, line);
    auto size = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    printf("Free internal heap: %d bytes\n", size);
    size = heap_caps_get_free_size(MALLOC_CAP_DMA);
    printf("Free dma heap: %d bytes\n", size);
    size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    printf("Free spiram heap: %d bytes\n\n", size);
}

inline time_t timeStringToTimestamp(const std::string& timeString) {
    std::tm tm = {};
    std::istringstream ss(timeString);
    ss >> std::get_time(&tm, "%y:%m:%d %H:%M:%S");
    if (ss.fail()) {
        return 0;
    }
    // 将 tm 转换为 time_t（Unix 时间戳）
    return std::mktime(&tm);
}
inline std::string timestampToTimeString(time_t timestamp) {
    std::tm tm = *std::localtime(&timestamp);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%y:%m:%d %H:%M:%S");
    return ss.str();
}

inline void* psram_prefered_malloc(size_t size) {
    void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (ptr)
        return ptr; 
    return heap_caps_malloc(size, MALLOC_CAP_8BIT);
}

inline void* dma_prefered_malloc(size_t size) {
    void* ptr = heap_caps_malloc(size, MALLOC_CAP_DMA);
    if (ptr)
        return ptr; 
    return heap_caps_malloc(size, MALLOC_CAP_8BIT);
}

#endif