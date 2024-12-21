#ifndef _MICROPHONE_H_
#define _MICROPHONE_H_
#include <freertos/FreeRTOS.h>
#include "driver/i2s_types.h"

class Microphone {
    friend class Cubicat;
public:
    Microphone(const Microphone&) = delete; // no copy constructor
    ~Microphone();

    const uint8_t* getAudioBuffer();
    size_t getBufferLen();
    // 如果使用pdm模式，ws不需要配置,且bus number必须为0,因为esp32s3的pdm模式只有I2S_NUM_0支持
    void init(int clk, int din, int ws = -1, int busNum = I2S_NUM_0, bool pdm = true);
    void stop();
    void resume();
    void shutdown();
private:
    Microphone(uint16_t sampleRate = 16000, uint8_t bitPerSample = 16);
    uint16_t            m_sampleRate = 0;
    uint8_t             m_bitPerSample = 0;
    i2s_chan_handle_t   m_channelHandle = nullptr;
    bool                m_bInited = false;
    char*               m_cacheBuffer = nullptr;
    uint8_t*            m_audioBuffer = nullptr;
    TaskHandle_t        m_taskHandle = nullptr;
    SemaphoreHandle_t   m_buffLock = nullptr;
};
#endif
