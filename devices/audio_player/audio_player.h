#ifndef _AUDIO_PLAYER_H_
#define _AUDIO_PLAYER_H_
#include <string>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <string.h>
#include "../audio_buffer.h"

typedef std::function<void(uint16_t)> SampleRateCallback;
typedef std::function<void(uint8_t)> BitDepthCallback;
typedef std::function<void(int16_t*, size_t, uint8_t)> PlayPCMCallback;
typedef std::function<void()> StopPlayCallback;

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
    bool play(const char* filename, bool loop = false);
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
    AudioBuffer         m_audioBuffer;
    QueueHandle_t       m_playTaskQueue = nullptr;
    TaskHandle_t        m_taskHandle = nullptr;
    SampleRateCallback  m_sampleRateCallback = nullptr;
    BitDepthCallback    m_bitDepthCallback = nullptr;
    PlayPCMCallback     m_playPCMCallback = nullptr;
    StopPlayCallback    m_stopPlayCallback = nullptr;
};

#endif
