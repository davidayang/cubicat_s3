#ifndef _AUDIO_CODEC_H_
#define _AUDIO_CODEC_H_

#include "microphone.h" 
#include "speaker.h"

namespace cubicat{

class AudioCodec {
    friend class Cubicat;
public:
    ~AudioCodec() {};
    uint16_t getSampleRate();
    void setSampleRate(uint16_t sampleRate);

    std::vector<int16_t> read(size_t maxSize = 0); // maxSize = 0 means read all
    void write(const int16_t* pcm, size_t len, uint8_t channels = 1);

    Microphone& getMic() { return m_microphone; }
    Speaker& getSpeaker() { return m_speaker; }
protected:
    AudioCodec() {};

    void init(int bclk, int ws, int dout, int din, int outEnable, int sampleRate, int bitWidth, bool pdmInput, int busNum = 0);

    i2s_std_config_t    m_config;

    Microphone          m_microphone;
    Speaker             m_speaker;
};

} // namespace Cubicat

#endif