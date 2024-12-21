#include "microphone.h"
#include <driver/i2s_pdm.h>
#include <driver/i2s_std.h>
#include "esp_psram.h"
#include <string.h>
#include "utils/logger.h"

#define BuffLen  1024 * 64

struct AudioTaskParam {
    i2s_chan_handle_t handle;
    char* cacheBuffer;
    uint8_t* audioBuffer;
    SemaphoreHandle_t buffLock;
};
Microphone::Microphone(uint16_t sampleRate, uint8_t bitPerSample)
: m_sampleRate(sampleRate), m_bitPerSample(bitPerSample)
{
}
Microphone::~Microphone() {
    shutdown();
}
size_t Microphone::getBufferLen() {
    return BuffLen;
}
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

void audioReadTask(void *param)
{
    AudioTaskParam* audioParam = (AudioTaskParam *)param;
    i2s_chan_handle_t handle = audioParam->handle;
    char* cacheBuffer = audioParam->cacheBuffer;
    uint8_t* audioBuffer = audioParam->audioBuffer;
    SemaphoreHandle_t buffLock = audioParam->buffLock;
    delete audioParam;
    while (1)
    {
        size_t bytesRead = 0;
        i2s_channel_read(handle, cacheBuffer, BuffLen, &bytesRead, portMAX_DELAY);
        if (xSemaphoreTake(buffLock, 1000) == pdPASS) {
            i2s_adc_data_scale(audioBuffer, cacheBuffer, bytesRead);
            xSemaphoreGive(buffLock);
        }
    }
}

void Microphone::init(int clk, int din, int ws, int busNum, bool pdm)
{
    if (m_bInited)
        return;
    i2s_port_t i2sPort = pdm?I2S_NUM_0:(i2s_port_t)busNum;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(i2sPort, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, nullptr, &m_channelHandle));
    if (pdm) {
        i2s_pdm_rx_config_t config = {
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
        ESP_ERROR_CHECK(i2s_channel_init_pdm_rx_mode(m_channelHandle, &config));
    } else {
        i2s_std_config_t config = {
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
        ESP_ERROR_CHECK(i2s_channel_init_std_mode(m_channelHandle, &config));
    }
    ESP_ERROR_CHECK(i2s_channel_enable(m_channelHandle));
    if (esp_psram_init() == ESP_OK) {
        m_cacheBuffer = (char *)heap_caps_malloc(BuffLen, MALLOC_CAP_SPIRAM);
        m_audioBuffer = (uint8_t *)heap_caps_malloc(BuffLen, MALLOC_CAP_SPIRAM);
    } else {
        m_cacheBuffer = (char *)calloc(BuffLen, sizeof(char));
        m_audioBuffer = (uint8_t *)calloc(BuffLen, sizeof(char));
    }
    assert(m_cacheBuffer && m_audioBuffer);
    m_buffLock = xSemaphoreCreateMutex();
    xSemaphoreGive(m_buffLock);
    auto id = xPortGetCoreID();
    int coreId = (id + 1) % portNUM_PROCESSORS;
    AudioTaskParam* param = new AudioTaskParam();
    param->handle = m_channelHandle;
    param->cacheBuffer = m_cacheBuffer;
    param->audioBuffer = m_audioBuffer;
    param->buffLock = m_buffLock;
    if (xTaskCreatePinnedToCore(audioReadTask, "audio task", 1024*8, param, 1, &m_taskHandle, coreId) != pdPASS) {
        LOGE("Microphone create task failed");
        i2s_channel_disable(m_channelHandle);
        i2s_del_channel(m_channelHandle);
        free(m_cacheBuffer);
        m_cacheBuffer = nullptr;
        free(m_audioBuffer);
        m_audioBuffer = nullptr;
        vSemaphoreDelete(m_buffLock);
        return;
    }
    m_bInited = true;
}

const uint8_t *Microphone::getAudioBuffer()
{
    const uint8_t *buff = nullptr;
    if (xSemaphoreTake(m_buffLock, 1000) == pdPASS) {
        buff = m_audioBuffer;
        xSemaphoreGive(m_buffLock);
    }
    return buff;
}
void Microphone::stop()
{
    if (m_channelHandle)
        i2s_channel_disable(m_channelHandle);
}
void Microphone::resume()
{
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
    if (m_cacheBuffer) {
        free(m_cacheBuffer);
        m_cacheBuffer = nullptr;
    }
    if (m_audioBuffer) {
        free(m_audioBuffer);
        m_audioBuffer = nullptr;
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