#ifndef _AUDIO_BUFFER_H_
#define _AUDIO_BUFFER_H_
#include "ring_buffer.h"

using AudioBuffer16 = RingBuffer<int16_t>;

struct AudioBuffer8 : public RingBuffer<uint8_t> {
    int fill(FILE* file) {
        if (capacity == len || !managed) { // buffer full
            return 0;
        }
        int bytesRead = fread(data + len, 1, capacity - len, file);
        len += bytesRead;
        return bytesRead;
    }
};

#endif