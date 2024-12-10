#ifndef _MICROPHONE_H_
#define _MICROPHONE_H_
#include <driver/i2s.h>

class Microphone {
    friend class Cubicat;
public:
    const uint8_t* getAudioBuffer();
    size_t getBuffLen();
    // 如果使用pdm模式，sclk 可以填-1, 使用ws作为时钟信号,且bus number必须为0
    void init(int sclk, int ws, int sd,int busNum = I2S_NUM_0);
    void setEnable(bool enable);
private:
    Microphone(uint16_t sampleRate = 16000, uint8_t bitPerSample = 16, bool pdm = true);
    i2s_config_t        m_config;
    int                 m_busNum;
    bool                m_bPdm;
    bool                m_bInited = false;
};
#endif
