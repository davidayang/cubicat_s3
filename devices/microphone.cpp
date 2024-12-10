#include "microphone.h"
#include "esp_psram.h"

#define BuffLen  1024 * 4
char *audioBuffer = nullptr;
uint8_t *scaledBuffer = nullptr;
SemaphoreHandle_t buffLock;
Microphone::Microphone(uint16_t sampleRate, uint8_t bitPerSample, bool pdm)
: m_bPdm(pdm)
{
    auto mode = I2S_MODE_MASTER | I2S_MODE_RX;
    if (pdm)
        mode |= I2S_MODE_PDM;
    m_config.mode = i2s_mode_t(mode);
    m_config.sample_rate = sampleRate; // 设置采样率
    m_config.bits_per_sample = i2s_bits_per_sample_t(bitPerSample);
    m_config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT; // 只接收左声道数据
    if (pdm)
        m_config.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_PCM_SHORT);
    else
        m_config.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S);
    m_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
    m_config.dma_desc_num = 4;
    m_config.dma_frame_num = 512;
    m_config.use_apll = true;
}
size_t Microphone::getBuffLen() {
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

void readTask(void *param)
{
    int *pins = (int *)param;
    i2s_pin_config_t pinConfig;
    pinConfig.bck_io_num = pins[0];
    pinConfig.ws_io_num = pins[1];
    pinConfig.data_out_num = I2S_PIN_NO_CHANGE;
    pinConfig.data_in_num = pins[2];
    i2s_port_t busNum = (i2s_port_t)pins[3];
    delete pins;
    if (ESP_OK != i2s_set_pin(busNum, &pinConfig))
    {
        printf("i2s_set_pin failed! \n");
        return;
    }
    while (audioBuffer)
    {
        size_t bytesRead = 0;
        esp_err_t error = i2s_read(busNum, audioBuffer, BuffLen, &bytesRead, portMAX_DELAY);
        xSemaphoreTake(buffLock, 1000);
        i2s_adc_data_scale(scaledBuffer, audioBuffer, bytesRead);
        xSemaphoreGive(buffLock);
    }
}

void Microphone::init(int sclk, int ws, int sd,int busNum)
{
    if (m_bInited)
        return;
    buffLock = xSemaphoreCreateBinary();
    xSemaphoreGive(buffLock);
    if (m_bPdm)
        m_busNum = I2S_NUM_0;
    else
        m_busNum = busNum;
    if (esp_psram_init() == ESP_OK) {
        audioBuffer = (char *)heap_caps_malloc(BuffLen, MALLOC_CAP_SPIRAM);
        scaledBuffer = (uint8_t *)heap_caps_malloc(BuffLen, MALLOC_CAP_SPIRAM);
    } else {
        audioBuffer = (char *)calloc(BuffLen, sizeof(char));
        scaledBuffer = (uint8_t *)calloc(BuffLen, sizeof(char));
    }
    if (ESP_OK != i2s_driver_install((i2s_port_t)m_busNum, &m_config, 0, NULL))
    {
        printf("i2s_driver_install failed! \n");
        free(audioBuffer);
        free(scaledBuffer);
        return;
    }
    auto id = xPortGetCoreID();
    int idAudio = (id + 1) % portNUM_PROCESSORS;
    int *param = new int[4];
    param[0] = sclk;
    param[1] = ws;
    param[2] = sd;
    param[3] = m_busNum;
    xTaskCreatePinnedToCore(readTask, "audio task", 1024, param, 1, NULL, idAudio);
    m_bInited = true;
}

const uint8_t *Microphone::getAudioBuffer()
{
    const uint8_t *buff = nullptr;
    xSemaphoreTake(buffLock, 1000);
    buff = scaledBuffer;
    xSemaphoreGive(buffLock);
    return buff;
}
void Microphone::setEnable(bool enable)
{
    if (enable)
        i2s_start((i2s_port_t)m_busNum);
    else
        i2s_stop((i2s_port_t)m_busNum);
}