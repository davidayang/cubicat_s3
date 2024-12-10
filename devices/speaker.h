#ifndef _SPEAKER_H_
#define _SPEAKER_H_
#include <driver/i2s.h>

class Speaker{
    friend class Cubicat;
public:
    void init(int bclk, int ws, int din, int enable,int busNum = I2S_NUM_1);
    void setSampleRate(uint16_t sampleRate);
    void setEnable(bool enable);
private:
    Speaker(uint16_t sampleRate = 8000, uint8_t bitPerSample = 16);
    i2s_config_t        m_config;
    int                 m_busNum;
};

#endif
