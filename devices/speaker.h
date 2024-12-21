#ifndef _SPEAKER_H_
#define _SPEAKER_H_
#include <driver/i2s_std.h>
#include <freertos/FreeRTOS.h>

class Speaker{
    friend class Cubicat;
public:
    Speaker(const Speaker&) = delete;
    ~Speaker();
    void init(int bclk, int ws, int dout, int enable,int busNum = I2S_NUM_1);
    void setSampleRate(uint16_t sampleRate);
    void setEnable(bool enable);
    void play(const char* path);
    void playRaw(const uint8_t* data, size_t len);
private:
    Speaker(uint32_t sampleRate = 16000, uint32_t bitPerSample = 16);
    int                 m_enablePin = -1;
    bool                m_bInited = false;
    bool                m_bEnable = false;
    i2s_std_config_t    m_config;
    i2s_chan_handle_t   m_channelHandle = nullptr;
    TaskHandle_t        m_taskHandle = nullptr;
    QueueHandle_t       m_playQueue = nullptr;
};

#endif
