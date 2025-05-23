#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_
#include <stdint.h>
#include "core/memory_allocator.h"
#include <malloc.h>
#include <string.h>
#include <cstdio>

template <typename T>
struct RingBuffer {
    T* data = nullptr;
    uint32_t len = 0;
    uint32_t id = 0;
    uint32_t capacity = 0;
    bool     managed = false;
    ~RingBuffer() {
        release();
    }
    void allocate(uint32_t cap) {
        if (capacity == cap) return;
        if (data && managed) {
            data = (T*)psram_prefered_realloc(data, cap*sizeof(T));
        } else {
            data = (T*)psram_prefered_malloc(cap*sizeof(T));
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
    void shift(size_t count) {
        if (!managed || count == 0) return;
        if (count >= len) {
            len = 0;
            return;
        }
        len -= count;
        memmove(data, data + count, len*sizeof(T));
    }
    void append(const T* data, size_t count) {
        if (!this->data || !data || !managed)
            return;
        int totalSize = len + count;
        int overflow = totalSize - capacity;
        if (overflow > 0) {
            shift(overflow);
        }
        T* src = (T*)data;
        if (count > capacity) {
            int offset = count - capacity;
            count = capacity;
            src += offset;
        }
        memcpy(this->data + len, src, count*sizeof(T));
        len += count;
    }

    void clear() {
        len = 0;
    }
};

#endif