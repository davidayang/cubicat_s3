#include "speaker.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include "utils/logger.h"

void volumeAdjust(uint8_t* audio_data, size_t length, float volumePercent) {
    if ( volumePercent == 1.0) return;
    for (size_t i = 0; i < length / 2; ++i) {
        int16_t sample = (audio_data[2 * i + 1] << 8) | audio_data[2 * i];
        sample *= volumePercent;
        audio_data[2 * i] = (uint8_t)(sample & 0xFF);          // 低字节
        audio_data[2 * i + 1] = (uint8_t)((sample >> 8) & 0xFF); // 高字节
    }
}

struct PlaySoundData
{
    const uint8_t* buff;
    size_t len;
    i2s_chan_handle_t channelHandle;
};

#define SegmentSize 1024
void playTask(void* arg) {
    QueueHandle_t playQueue = (QueueHandle_t)arg;
    while (1) {
        PlaySoundData data;
        if (xQueueReceive(playQueue, &data, portMAX_DELAY) == pdTRUE) {
            size_t dateRemain = data.len;
            while (dateRemain > 0)
            {
                size_t sizeWritten = 0;
                size_t writeSize = dateRemain > SegmentSize ? SegmentSize : dateRemain;
                i2s_channel_write(data.channelHandle, data.buff + data.len - dateRemain, writeSize, &sizeWritten, portMAX_DELAY);
                dateRemain -= sizeWritten;
            }
            gpio_set_level((gpio_num_t)13, 0);
        }
    }
}

Speaker::Speaker(uint32_t sampleRate, uint32_t bitPerSample)
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
        if (m_taskHandle)
            vTaskDelete(m_taskHandle);
        m_bInited = false;
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
    gpio_set_direction((gpio_num_t)enable, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)enable, 0);
    i2s_channel_enable(m_channelHandle);
    m_playQueue = xQueueCreate(1, sizeof(PlaySoundData));
    if (xTaskCreatePinnedToCore(playTask, "speaker play task", 1024*4, m_playQueue, 1, &m_taskHandle, xPortGetCoreID()) != pdPASS) {
        LOGE("speaker play task create failed");
        i2s_del_channel(m_channelHandle);
        m_channelHandle = nullptr;
        return;
    }
    m_bInited = true;
}
void Speaker::setSampleRate(uint16_t sampleRate)
{
    if (m_bInited == false) {
        return;
    }
    m_config.clk_cfg.sample_rate_hz = sampleRate;
    i2s_channel_init_std_mode(m_channelHandle, &m_config);
}
void Speaker::setEnable(bool enable)
{
    gpio_set_level((gpio_num_t)m_enablePin, enable);
    m_bEnable = enable;
}
void Speaker::play(const char* path) {
    if (m_bInited && m_channelHandle) {
    }
}
void Speaker::playRaw(const uint8_t* data, size_t len) {
    if (m_bInited && m_channelHandle) {
        PlaySoundData taskData = { data, len, m_channelHandle };
        xQueueSend(m_playQueue, &taskData, 0);
    }
}