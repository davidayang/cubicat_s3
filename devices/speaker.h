#ifndef _SPEAKER_H_
#define _SPEAKER_H_
#include <driver/i2s_std.h>
#include "audio_player/audio_player.h"
#include <atomic>

class Speaker{
    friend class Cubicat;
public:
    Speaker(const Speaker&) = delete;
    ~Speaker();
    
    void setSampleRate(uint16_t sampleRate);
    void setBitsPerSample(uint8_t bitPerSample);
    void setEnable(bool enable);
    void playFile(const char* filename, bool loop = false);
    void playRaw(const int16_t* pcm, size_t len, uint8_t channels);
    void stopPlay();
    void setVolume(float volume);
    bool isPlaying();
    bool isEnable();

    uint16_t getSampleRate();
    uint8_t getChannels();
private:
    Speaker(uint32_t sampleRate = 16000, uint8_t bitPerSample = 16);
    void init(int bclk, int ws, int dout, int enable,int busNum = I2S_NUM_1);
    int                 m_enablePin = -1;
    bool                m_bInited = false;
    bool                m_bEnable = false;
    std::atomic<bool>   m_bPlaying = false;
    float               m_fVolume = 1.0f;
    i2s_std_config_t    m_config;
    i2s_chan_handle_t   m_channelHandle = nullptr;
    AudioPlayer*        m_pAudioPlayer = nullptr;
    std::mutex          m_mutex;
};

#endif
