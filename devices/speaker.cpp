#include "speaker.h"
#include <driver/gpio.h>
#include "utils/logger.h"
#include "utils/helper.h"
#include "audio_player/mp3_decoder/mp3_decoder.h"
#include <string.h>
#include "core/memory_allocator.h"

void volumeAdjust(uint8_t* audio_data, size_t length, float volumePercent) {
    if ( volumePercent == 1.0) return;
    int16_t* data = (int16_t*)audio_data;
    int len = length / 2;
    for (size_t i = 0; i < len; ++i) {
        int16_t sample = data[i];
        sample *= volumePercent;
        data[i] = sample;
    }
}

struct PlaySoundData
{
    const uint8_t* buff;
    size_t len;
    i2s_chan_handle_t channelHandle;
    float volume;
};

Speaker::Speaker(uint32_t sampleRate, uint8_t bitPerSample)
{
    m_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate),
        .slot_cfg = {
            .data_bit_width = (i2s_data_bit_width_t)bitPerSample,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,
            .ws_width = 1,
            .ws_pol = true,
            .bit_shift = true,
            .left_align = true,
            .big_endian = false,
            .bit_order_lsb = false,
        },
    };
}

Speaker::~Speaker() {
    if (m_bInited) {
        i2s_channel_disable(m_channelHandle);
        i2s_del_channel(m_channelHandle);
        gpio_set_level((gpio_num_t)m_enablePin, 0);
        m_bInited = false;
        if (m_pAudioPlayer)
            delete m_pAudioPlayer;
    }
}

void Speaker::init(int bclk, int ws, int dout, int enable, int busNum)
{
    if (m_bInited) {
        return;
    }
    m_enablePin = enable;
    // set pin config
    m_config.gpio_cfg = {
        .mclk = GPIO_NUM_NC,
        .bclk = (gpio_num_t)bclk,
        .ws = (gpio_num_t)ws,
        .dout = (gpio_num_t)dout,
        .din = GPIO_NUM_NC,
        .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false
        },
    };
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG((i2s_port_t)busNum, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &m_channelHandle, nullptr));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(m_channelHandle, &m_config));
    gpio_set_direction((gpio_num_t)m_enablePin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)m_enablePin, 0);
    m_bInited = true;
    m_pAudioPlayer = new AudioPlayer();
}

void Speaker::setSampleRate(uint16_t sampleRate)
{
    if (!m_bInited || sampleRate == m_config.clk_cfg.sample_rate_hz) {
        return;
    }
    i2s_channel_disable(m_channelHandle);
    m_config.clk_cfg.sample_rate_hz = sampleRate;
    i2s_channel_reconfig_std_clock(m_channelHandle, &m_config.clk_cfg);
    i2s_channel_enable(m_channelHandle);
}
void Speaker::setEnable(bool enable)
{
    if (m_bEnable == enable) {
        return;
    }
    gpio_set_level((gpio_num_t)m_enablePin, enable);
    m_bEnable = enable;
    if (enable) {
        i2s_channel_enable(m_channelHandle);
    } else {
        i2s_channel_disable(m_channelHandle);
    }
}
void Speaker::setBitsPerSample(uint8_t bitPerSample) {
    if (!m_bInited || bitPerSample == m_config.slot_cfg.data_bit_width) {
        return;
    }
    i2s_channel_disable(m_channelHandle);
    m_config.slot_cfg.data_bit_width = (i2s_data_bit_width_t)bitPerSample;
    i2s_channel_reconfig_std_slot(m_channelHandle, &m_config.slot_cfg);
    i2s_channel_enable(m_channelHandle);
}

void Speaker::playFile(const char* filename, bool loop) {
    if (!m_bInited || !m_channelHandle || !m_pAudioPlayer) {
        LOGI("Speaker not inited!\n");
        return;
    }
    m_pAudioPlayer->setCallback([this](uint16_t sampleRate) {
        this->setSampleRate(sampleRate);
    }, [this](uint8_t bitDepth) {
        this->setBitsPerSample(bitDepth);
    }, [this](uint8_t* data, size_t len) {
        this->playRaw(data, len);
    }, [this]() {
        this->setEnable(false);
    });
    setEnable(true);
    m_pAudioPlayer->play(filename, loop);
}

void Speaker::playRaw(const uint8_t* data, size_t len) {
    const int SegmentSize = 1024;
    size_t dateRemain = len;
    while (dateRemain > 0)
    {
        size_t sizeWritten = 0;
        size_t writeSize = dateRemain > SegmentSize ? SegmentSize : dateRemain;
        const uint8_t* ptr = data + len - dateRemain;
        volumeAdjust((uint8_t*)ptr, writeSize, m_fVolume);
        i2s_channel_write(m_channelHandle, ptr, writeSize, &sizeWritten, portMAX_DELAY);
        dateRemain -= sizeWritten;
    }
}

void Speaker::setVolume(float volume) {
    if (volume < 0) {
        volume = 0;
    }
    m_fVolume = volume;
}
void Speaker::stopPlay() {
    if (m_pAudioPlayer)
        m_pAudioPlayer->stop();
}
