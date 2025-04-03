#include "microphone.h"
#include "esp_psram.h"
#include <string.h>
#include "utils/logger.h"
#include "utils/helper.h"

uint32_t buffID = 0;

struct AudioTaskParam {
    i2s_chan_handle_t handle;
    char* cacheBuffer;
    int bufLen;
    uint8_t* audioBuffer;
    SemaphoreHandle_t buffLock;
    // SemaphoreHandle_t workingLock;
    bool*           stopSign;
};

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
    bool* stopSign = audioParam->stopSign;
    int bufLen = audioParam->bufLen;
    delete audioParam;
    while (1)
    {
        if (*stopSign) {
            vTaskDelay(pdTICKS_TO_MS(10));
        } else {
            size_t bytesRead = 0;
            i2s_channel_read(handle, cacheBuffer, bufLen, &bytesRead, portMAX_DELAY);
            if (xSemaphoreTake(buffLock, 1000) == pdPASS) {
                i2s_adc_data_scale(audioBuffer, cacheBuffer, bytesRead);
                buffID++;
                xSemaphoreGive(buffLock);
            }
        }
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
    resizeBuffer(m_nBuffLen);
    if (!m_cacheBuffer || !m_audioBuffer) {
        LOGE("Microphone malloc buffer failed");
        i2s_del_channel(m_channelHandle);
        return;
    }
    m_buffLock = xSemaphoreCreateMutex();
    auto id = xPortGetCoreID();
    int coreId = (id + 1) % portNUM_PROCESSORS;
    AudioTaskParam* param = new AudioTaskParam();
    param->handle = m_channelHandle;
    param->cacheBuffer = m_cacheBuffer;
    param->audioBuffer = m_audioBuffer;
    param->buffLock = m_buffLock;
    param->stopSign = &m_bStop;
    param->bufLen = m_nBuffLen;
    if (xTaskCreatePinnedToCoreWithCaps(audioReadTask, "audio task", 1024*4,
         param, 1, &m_taskHandle, coreId, MALLOC_CAP_SPIRAM) != pdPASS) {
        LOGE("Microphone create task failed");
        i2s_del_channel(m_channelHandle);
        free(m_cacheBuffer);
        free(m_audioBuffer);
        m_cacheBuffer = nullptr;
        m_audioBuffer = nullptr;
        delete param;
        vSemaphoreDelete(m_buffLock);
        return;
    }
    m_bInited = true;
}

AudioBuffer Microphone::popAudioBuffer()
{
    AudioBuffer buf;
    if (m_bInited && !m_bStop) {
        if (xSemaphoreTake(m_buffLock, 1000) == pdPASS) {
            if (buffID > m_lastBuffID) {
                buf.data = m_audioBuffer;
                buf.len = m_nBuffLen;
                buf.id = buffID;
                m_lastBuffID = buffID;
            }
            xSemaphoreGive(m_buffLock);
        }
    } else {
        LOGE("Microphone not inited or not running %d %d\n", m_bInited, m_bStop);
    }
    return buf;
}
void Microphone::stop()
{
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
    buffID = 0;
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

void Microphone::resizeBuffer(uint32_t size) {
    if (m_audioBuffer && m_nBuffLen == size) {
        return;
    }
    stop();
    free(m_cacheBuffer);
    free(m_audioBuffer);
    m_cacheBuffer = (char*)psram_prefered_malloc(size);
    m_audioBuffer = (uint8_t*)psram_prefered_malloc(size);
    m_nBuffLen = size;
}