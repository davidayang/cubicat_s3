#ifndef _MICROPHONE_H_
#define _MICROPHONE_H_
#include <freertos/FreeRTOS.h>
#include "driver/i2s_types.h"
#include <driver/i2s_pdm.h>
#include <driver/i2s_std.h>
#include "audio_buffer.h"
#include <vector>

class Microphone {
    friend class Cubicat;
public:
    Microphone(const Microphone&) = delete; // no copy constructor
    ~Microphone();

    void setSampleRate(uint16_t sampleRate);
    std::vector<uint8_t> popAudioBuffer(size_t maxSize); // maxSize = 0 means pop all
    // 如果使用pdm模式，ws不需要配置,且bus number必须为0,因为esp32s3的pdm模式只有I2S_NUM_0支持
    void stop();
    void start();
    void shutdown();
    bool isRunning() { return !m_bStop; }

    // internal use only
    void readData();
private:
    Microphone(uint16_t sampleRate = 16000, uint8_t bitPerSample = 16);
    void init(int clk, int din, int ws = -1, int busNum = I2S_NUM_0, bool pdm = true);
    bool                m_pdm;
    i2s_pdm_rx_config_t m_pdmConfig;
    i2s_std_config_t    m_stdConfig;
    uint16_t            m_sampleRate = 0;
    uint8_t             m_bitPerSample = 0;
    i2s_chan_handle_t   m_channelHandle = nullptr;
    bool                m_bInited = false;
    bool                m_bStop = true;
    AudioBuffer         m_audioBuffer;
    TaskHandle_t        m_taskHandle = nullptr;
    SemaphoreHandle_t   m_buffLock = nullptr;
    uint32_t            m_lastBuffID = 0;
};
#endif
