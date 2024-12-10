#include "speaker.h"
#include <driver/gpio.h>

Speaker::Speaker(uint16_t sampleRate, uint8_t bitPerSample)
{
    m_config.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX);
    m_config.sample_rate = sampleRate; // 设置采样率
    m_config.bits_per_sample = i2s_bits_per_sample_t(bitPerSample);
    m_config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT; // 只接收左声道数据
    m_config.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S);
    m_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    m_config.dma_desc_num = 4;
    m_config.dma_frame_num = 512;
    m_config.use_apll = false;
    m_config.mclk_multiple = I2S_MCLK_MULTIPLE_256;
    m_config.bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT;
}
int enablePin = -1;
void Speaker::init(int bclk, int ws, int din, int enable, int busNum)
{
    m_busNum = busNum;
    enablePin = enable;
    if (ESP_OK != i2s_driver_install((i2s_port_t)m_busNum, &m_config, 0, NULL))
    {
        printf("speaker i2s_driver_install failed! \n");
        return;
    }
    i2s_pin_config_t pinConfig;
    pinConfig.bck_io_num = bclk;
    pinConfig.ws_io_num = ws;
    pinConfig.data_out_num = din;
    pinConfig.mck_io_num = I2S_PIN_NO_CHANGE;
    pinConfig.data_in_num = I2S_PIN_NO_CHANGE;
    if (ESP_OK != i2s_set_pin((i2s_port_t)m_busNum, &pinConfig))
    {
        printf("speaker i2s_set_pin failed! \n");
        return;
    }
    gpio_set_direction((gpio_num_t)enable, GPIO_MODE_OUTPUT);
    setEnable(false);
}
void Speaker::setSampleRate(uint16_t sampleRate)
{
    m_config.sample_rate = sampleRate;
    i2s_set_sample_rates((i2s_port_t)m_busNum, sampleRate);
}
void Speaker::setEnable(bool enable)
{
    gpio_set_level((gpio_num_t)enablePin, enable);
}