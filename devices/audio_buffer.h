#ifndef _AUDIO_BUFFER_H_
#define _AUDIO_BUFFER_H_
#include <stdint.h>
#include "core/memory_allocator.h"
#include <malloc.h>
#include <string.h>
#include <cstdio>

struct AudioBuffer {
    uint8_t* data = nullptr;
    uint32_t len = 0;
    uint32_t id = 0;
    uint32_t capacity = 0;
    bool     managed = false;
    ~AudioBuffer() {
        release();
    }
    void allocate(uint32_t cap) {
        if (capacity == cap) return;
        if (data && managed) {
            data = (unsigned char*)psram_prefered_realloc(data, cap);
        } else {
            data = (unsigned char*)psram_prefered_malloc(cap);
        }
        managed = true;
        capacity = cap;
    }
    void release() {
        if (data && managed) {
            free(data);
            data = nullptr;
        }
    }
    void shift(size_t size) {
        if (!managed || size == 0) return;
        if (size >= len) {
            len = 0;
            return;
        }
        len -= size;
        memmove(data, data + size, len);
    }
    void append(const uint8_t* data, size_t size) {
        if (!this->data || !data || !managed)
            return;
        int totalSize = len + size;
        int overflow = totalSize - capacity;
        if (overflow > 0) {
            shift(overflow);
        }
        uint8_t* src = (uint8_t*)data;
        if (size > capacity) {
            int offset = size - capacity;
            size = capacity;
            src += offset;
        }
        memcpy(this->data + len, src, size);
        len += size;
    }
    int fill(FILE* file) {
        if (capacity == len || !managed) { // buffer full
            return 0;
        }
        int bytesRead = fread(data + len, 1, capacity - len, file);
        len += bytesRead;
        return bytesRead;
    }
    void clear() {
        len = 0;
    }
};

#endif