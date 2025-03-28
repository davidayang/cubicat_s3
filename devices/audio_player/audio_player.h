#ifndef _AUDIO_PLAYER_H_
#define _AUDIO_PLAYER_H_
#include <string>
#include <functional>
#include <freertos/FreeRTOS.h>
#include "core/memory_allocator.h"
#include <string.h>

typedef std::function<void(uint16_t)> SampleRateCallback;
typedef std::function<void(uint8_t)> BitDepthCallback;
typedef std::function<void(uint8_t*, size_t)> PlayPCMCallback;
typedef std::function<void()> StopPlayCallback;

struct AudioCacheBuffer {
    uint8_t* data = nullptr;
    uint16_t pos = 0;
    const uint16_t capacity = 1600;
    void init() {
        if (!data) {
            data = (unsigned char*)psram_prefered_malloc(capacity);
        }
        pos = 0;
    }
    void release() {
        if (data) {
            free(data);
            data = nullptr;
        }
    }
    void shift(size_t size) {
        if (size == 0) return;
        if (size >= pos) {
            pos = 0;
            return;
        }
        pos -= size;
        memmove(data, data + size, pos);
    }
    int append(FILE* file) {
        if (capacity == pos) { // buffer full
            return 0;
        }
        int bytesRead = fread(data + pos, 1, capacity - pos, file);
        pos += bytesRead;
        return bytesRead;
    }
};

class AudioPlayer {
public:
    enum CodecType {
        CODEC_NONE,
        CODEC_WAV,
        CODEC_MP3
    };
    AudioPlayer();
    ~AudioPlayer();

    void setCallback(SampleRateCallback sampleRateCallback, BitDepthCallback bitDepthCallback, 
                    PlayPCMCallback playPCMCallback, StopPlayCallback stopPlayCallback);
    void play(const char* filename, bool loop = false);
    void pause();
    void stop();
    bool isPlaying() { return m_bPlaying; }

    bool proceed();
private:
    bool init();
    void clear();
    void onFinish();
    void playImmediately();
    CodecType           m_eCodec;
    bool                m_bLoop = false;
    bool                m_bPlaying = false;
    FILE*               m_pAudioFile = nullptr;
    AudioCacheBuffer    m_audioBuffer;
    QueueHandle_t       m_playTaskQueue = nullptr;
    TaskHandle_t        m_taskHandle = nullptr;
    SampleRateCallback  m_sampleRateCallback = nullptr;
    BitDepthCallback    m_bitDepthCallback = nullptr;
    PlayPCMCallback     m_playPCMCallback = nullptr;
    StopPlayCallback    m_stopPlayCallback = nullptr;
};

#endif
