#ifndef _MICROPHONE_H_
#define _MICROPHONE_H_
#include <freertos/FreeRTOS.h>
#include "driver/i2s_types.h"
#include <driver/i2s_pdm.h>
#include <driver/i2s_std.h>
#include "audio_buffer.h"
#include <vector>

namespace cubicat {

class Microphone {
    friend class Cubicat;
    friend class AudioCodec;
public:
    Microphone(const Microphone&) = delete; // no copy constructor
    ~Microphone();

    void setSampleRate(uint16_t sampleRate);
    std::vector<int16_t> popAudioBuffer(size_t maxSize); // maxSize = 0 means pop all
    // 如果使用pdm模式，ws不需要配置,且bus number必须为0,因为esp32s3的pdm模式只有I2S_NUM_0支持
    void stop();
    void start();
    void shutdown();
    bool isRunning() { return !m_bStop; }

    // internal use only
    void readData();
private:
    Microphone();
    void init(int clk, int ws, int din, uint16_t sampleRate, uint8_t bitWidth, i2s_chan_handle_t channelHandle, bool pdm = false);
    bool                m_pdm;
    i2s_pdm_rx_config_t m_pdmConfig;
    i2s_std_config_t    m_stdConfig;
    i2s_chan_handle_t   m_channelHandle = nullptr;
    bool                m_bInited = false;
    bool                m_bStop = true;
    AudioBuffer16       m_audioBuffer;
    TaskHandle_t        m_taskHandle = nullptr;
    SemaphoreHandle_t   m_buffLock = nullptr;
    uint32_t            m_lastBuffID = 0;
};

} // namespace cubicat
#endif
