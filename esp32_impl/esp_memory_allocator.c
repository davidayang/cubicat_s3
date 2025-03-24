#include "esp_heap_caps.h"
#include "esp_psram.h"

bool psram_init() {
    return esp_psram_init();
}

void* psram_prefered_malloc(size_t size) {
    void* ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (ptr)
        return ptr; 
    return heap_caps_malloc(size, MALLOC_CAP_8BIT);
}

void* psram_malloc(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void* psram_prefered_realloc(void* p, size_t size) {
    void* ptr = heap_caps_realloc(p, size, MALLOC_CAP_SPIRAM);
    if (ptr)
        return ptr; 
    return heap_caps_realloc(p, size, MALLOC_CAP_DEFAULT);
}

void* dma_prefered_malloc(size_t size) {
    void* ptr = heap_caps_malloc(size, MALLOC_CAP_DMA);
    if (ptr)
        return ptr; 
    return heap_caps_malloc(size, MALLOC_CAP_8BIT);
}