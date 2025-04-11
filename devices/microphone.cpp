#include "microphone.h"
#include "esp_psram.h"
#include <string.h>
#include "utils/logger.h"
#include "utils/helper.h"
#include <atomic>

#define AUDIO_BUFFER_SIZE 1024 * 16
void i2s_adc_data_scale(uint8_t *d_buff, char *s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint32_t dac_value = 0;
    for (int i = 0; i < len - 1; i += 2)
    {
        dac_value = (uint16_t(s_buff[i + 1] & 0xf) << 8) | s_buff[i + 0];
        d_buff[j++] = 0;
        d_buff[j++] = dac_value * 256 / 2048;
    }
}

void AudioReadTask(void *param)
{
    Microphone* mic = (Microphone*)param;
    while (1)
    {
        mic->readData();
    }
}

Microphone::Microphone(uint16_t sampleRate, uint8_t bitPerSample)
: m_sampleRate(sampleRate), m_bitPerSample(bitPerSample)
{
}
Microphone::~Microphone() {
    shutdown();
}
void Microphone::setSampleRate(uint16_t sampleRate)
{
    if (!m_bInited || m_sampleRate == sampleRate) {
        return;
    }
    m_sampleRate = sampleRate;
    i2s_channel_disable(m_channelHandle);
    if (m_pdm) {
        m_pdmConfig.clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(m_sampleRate);
        ESP_ERROR_CHECK(i2s_channel_reconfig_pdm_rx_clock(m_channelHandle, &m_pdmConfig.clk_cfg));
    } else {
        m_stdConfig.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(m_sampleRate);
        ESP_ERROR_CHECK(i2s_channel_reconfig_std_clock(m_channelHandle, &m_stdConfig.clk_cfg));
    }
    i2s_channel_enable(m_channelHandle);
}

void Microphone::init(int clk, int din, int ws, int busNum, bool pdm)
{
    if (m_bInited)
        return;
    m_pdm = pdm;
    i2s_port_t i2sPort = pdm?I2S_NUM_0:(i2s_port_t)busNum;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(i2sPort, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, nullptr, &m_channelHandle));
    if (m_pdm) {
        m_pdmConfig = {
            .clk_cfg = I2S_PDM_RX_CLK_DEFAULT_CONFIG(m_sampleRate),
            .slot_cfg = I2S_PDM_RX_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)m_bitPerSample, I2S_SLOT_MODE_MONO),
            .gpio_cfg = {
                .clk = (gpio_num_t)clk,
                .din = (gpio_num_t)din,
                .invert_flags = {
                    .clk_inv = false,
                },
            },
        };
        ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(m_channelHandle, &m_pdmConfig));
    } else {
        m_stdConfig = {
            .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(m_sampleRate),
            .slot_cfg = I2S_STD_PCM_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)m_bitPerSample, I2S_SLOT_MODE_MONO),
            .gpio_cfg = {
                .mclk = GPIO_NUM_NC,
                .bclk = (gpio_num_t)clk,
                .ws = (gpio_num_t)ws,
                .dout = GPIO_NUM_NC,
                .din = (gpio_num_t)din,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv = false,
                },
            },
        };
        ESP_ERROR_CHECK(i2s_channel_init_std_mode(m_channelHandle, &m_stdConfig));
    }
    m_audioBuffer.allocate(AUDIO_BUFFER_SIZE);
    if (!m_audioBuffer.data) {
        LOGE("Microphone malloc buffer failed");
        i2s_del_channel(m_channelHandle);
        return;
    }
    m_buffLock = xSemaphoreCreateMutex();
    if (xTaskCreatePinnedToCoreWithCaps(AudioReadTask, "audio task", 1024*4,
         this, 1, &m_taskHandle, getSubCoreId(), MALLOC_CAP_SPIRAM) != pdPASS) {
        LOGE("Microphone create task failed");
        i2s_del_channel(m_channelHandle);
        vSemaphoreDelete(m_buffLock);
        return;
    }
    m_bInited = true;
}

std::vector<uint8_t> Microphone::popAudioBuffer(size_t size)
{
    std::vector<uint8_t> buf;
    if (m_bInited && !m_bStop) {
        if (xSemaphoreTake(m_buffLock, portMAX_DELAY) == pdPASS) {
            if (size > m_audioBuffer.len || size == 0) {
                size = m_audioBuffer.len;
            }
            buf = std::move(std::vector<uint8_t>(m_audioBuffer.data, m_audioBuffer.data + size));
            m_audioBuffer.shift(size);
            xSemaphoreGive(m_buffLock);
        }
    } else {
        LOGE("Microphone not inited or not running %d %d\n", m_bInited, m_bStop);
    }
    return buf;
}
void Microphone::stop()
{
    LOGI("Microphone stop\n");
    if (m_bStop)
        return;
    m_bStop = true;
    if (m_channelHandle)
        i2s_channel_disable(m_channelHandle);
}
void Microphone::start()
{
    if (!m_bStop)
        return;
    m_bStop = false;
    if (m_channelHandle)
        i2s_channel_enable(m_channelHandle);
}
void Microphone::shutdown()
{
    stop();
    m_bInited = false;
    if (m_taskHandle) {
        vTaskDelete(m_taskHandle);
        m_taskHandle = nullptr;
    }
    if (m_channelHandle) {
        i2s_del_channel(m_channelHandle);
        m_channelHandle = nullptr;
    }
    if (m_buffLock) {
        vSemaphoreDelete(m_buffLock);
        m_buffLock = nullptr;
    }
}

void Microphone::readData() {
    size_t bytesRead = 0;
    char cacheBuffer[512];
    i2s_channel_read(m_channelHandle, cacheBuffer, sizeof(cacheBuffer), &bytesRead, portMAX_DELAY);
    if (xSemaphoreTake(m_buffLock, portMAX_DELAY) == pdPASS) {
        m_audioBuffer.append((uint8_t*)cacheBuffer, bytesRead);
        xSemaphoreGive(m_buffLock);
    }
}