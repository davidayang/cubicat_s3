#include "esp_timer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

uint32_t millis() {
    return esp_timer_get_time() / 1000;
}
int64_t micros() {
    return esp_timer_get_time();
}

void memoryReport(const char* file, int line) {
    printf("<=====Memory report in %s:%d ======>\n", file, line);
    int watermark = uxTaskGetStackHighWaterMark(NULL);
    printf("Free stack(minimal): %d bytes\n", watermark);
    size_t size = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    printf("Free internal heap: %d bytes\n", size);
    size = heap_caps_get_free_size(MALLOC_CAP_DMA);
    printf("Free dma heap: %d bytes\n", size);
    size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    printf("Free spiram heap: %d bytes\n\n", size);
}
