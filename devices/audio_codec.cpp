#include "audio_codec.h"
#include <esp_err.h>
#include <driver/i2s_std.h>

using namespace cubicat;


void AudioCodec::init(int bclk, int ws, int dout, int din, int outEnable, int sampleRate, int bitWidth, bool pdmInput, int busNum) {
    if (pdmInput)
        busNum = 0;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG((i2s_port_t)busNum, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    assert(din > 0 && dout > 0);
    i2s_chan_handle_t txHandle = nullptr;
    i2s_chan_handle_t rxHandle = nullptr;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &txHandle, &rxHandle));
    m_microphone.init(bclk, ws, din, sampleRate, bitWidth, rxHandle, pdmInput);
    m_speaker.init(bclk, ws, dout, outEnable, sampleRate, bitWidth, txHandle);
}

uint16_t AudioCodec::getSampleRate() {
    return m_speaker.getSampleRate();
}

void AudioCodec::setSampleRate(uint16_t sampleRate) {
    m_microphone.setSampleRate(sampleRate);
    m_speaker.setSampleRate(sampleRate);
}

std::vector<int16_t> AudioCodec::read(size_t maxSize) {
    return m_microphone.popAudioBuffer(maxSize);
}
void AudioCodec::write(const int16_t* pcm, size_t len, uint8_t channels) {
    m_speaker.playRaw(pcm, len, channels);
}