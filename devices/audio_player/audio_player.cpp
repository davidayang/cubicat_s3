#include "audio_player.h"
#include "mp3_decoder/mp3_decoder.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "utils/helper.h"
#include "utils/logger.h"
#include "core/memory_allocator.h"

extern FILE* openFileFlash(const char* filename, bool binary);

void stereoToMono(int16_t* stereoData, int16_t* monoData, int stereoDataLen) {
    for (size_t i = 0; i < stereoDataLen; i += 2) {
        int16_t left = stereoData[i];
        int16_t right = stereoData[i + 1];
        monoData[i/2] = left;
    }
}

struct AudioTaskData
{
    AudioPlayer* player;
};

void AudioPlayerTask(void* arg) {
    QueueHandle_t playQueue = (QueueHandle_t)arg;
    while (1) {
        AudioTaskData data;
        if (xQueueReceive(playQueue, &data, portMAX_DELAY) == pdTRUE) {
            auto player = data.player;
            while (player->proceed()) {
            }
        }
    }
}

AudioPlayer::AudioPlayer() {
    
}
AudioPlayer::~AudioPlayer() {
    clear();
}
bool AudioPlayer::init() {
    if (!m_playTaskQueue) {
        m_playTaskQueue = xQueueCreate(1, sizeof(AudioTaskData));
        if (xTaskCreatePinnedToCore(AudioPlayerTask, "audio task", 1024*8, m_playTaskQueue, 1, &m_taskHandle, getSubCoreId()) != pdPASS) {
            LOGE("Audio player init failed");
            vQueueDelete(m_playTaskQueue);
            m_playTaskQueue = nullptr;
            return false;
        }
    }
    return true;
}
void AudioPlayer::clear() {
    stop();
    if (m_taskHandle) {
        vTaskDelete(m_taskHandle);
        m_taskHandle = nullptr;
    }
    if (m_playTaskQueue) {
        vQueueDelete(m_playTaskQueue);
        m_playTaskQueue = nullptr;
    }
}
bool AudioPlayer::play(const char* filename, bool loop) {
    if (m_bPlaying) {
        stop();
    }
    if (!init()) {
        return false;
    }
    m_bLoop = loop;
    m_pAudioFile = openFileFlash(filename, true);
    if (!m_pAudioFile) {
        LOGW("Speaker open file %s failed\n", filename);
        return false;
    }
    if (endsWith(filename, ".mp3")) {
        m_eCodec = CODEC_MP3;
    } else if (endsWith(filename, ".wav")) {
        m_eCodec = CODEC_WAV;
    }
    playImmediately();
    return true;
}
void AudioPlayer::playImmediately() {
    if (!m_pAudioFile) {
        return;
    }
    m_audioBuffer.allocate(1600);
    rewind(m_pAudioFile);
    m_bPlaying = false;
    if (m_eCodec == CODEC_MP3) {
        MP3Decoder_AllocateBuffers();
        // 读取文件数据到缓冲区
        m_audioBuffer.fill(m_pAudioFile);
        while (m_audioBuffer.len > 0) {
            int syncOffset = 0;
            if (!m_bPlaying) {
                // 查找帧同步头
                syncOffset = MP3FindSyncWord(m_audioBuffer.data, m_audioBuffer.len);
                if (syncOffset < 0) {
                    LOGW("Sync word not found, skipping bytes.\n");
                    stop();
                    break;
                }
                if (syncOffset == 0) {
                    m_bPlaying = true;
                    AudioTaskData data;
                    data.player = this;
                    xQueueSend(m_playTaskQueue, &data, 0);
                    break;
                }
            }
            m_audioBuffer.shift(syncOffset);  // 移动剩余数据到缓冲区头部
            m_audioBuffer.fill(m_pAudioFile);
        }
    } else if (m_eCodec == CODEC_WAV) {

    }
}
void AudioPlayer::stop() {
    m_bPlaying = false;
    m_bLoop = false;
    if (m_pAudioFile) {
        fclose(m_pAudioFile);
        m_pAudioFile = nullptr;
    }
    MP3Decoder_FreeBuffers();
    m_audioBuffer.release();
    if (m_stopPlayCallback)
        m_stopPlayCallback();
}
void AudioPlayer::pause() {
    
}
void AudioPlayer::setCallback(SampleRateCallback sampleRateCB, BitDepthCallback bitDepthCB,
     PlayPCMCallback playCB, StopPlayCallback stopPlayCB) {
    m_sampleRateCallback = sampleRateCB;
    m_bitDepthCallback = bitDepthCB;
    m_playPCMCallback = playCB;
    m_stopPlayCallback = stopPlayCB;
}

bool AudioPlayer::proceed() {
    // 解码当前帧
    int bytesLeft = m_audioBuffer.len;
    int len = bytesLeft;
    uint8_t* buf = m_audioBuffer.data;
    short outbuf[1152*2];
    int ret = MP3Decode(buf, &bytesLeft, outbuf, 0);
    if (ret != 0) {
        LOGE("Decode error: %d\n", ret);
        stop();
        return false;
    }
    if (bytesLeft <= 0) {
        LOGE("Decode error: %d\n", bytesLeft);
        stop();
        return false;
    }
    int decodedBytes = len - bytesLeft;
    // remove the decoded bytes from the buffer
    m_audioBuffer.shift(decodedBytes);
    // 获取解码后的PCM信息
    int samps = MP3GetOutputSamps();  // 本帧输出的PCM样本数
    int channels = MP3GetChannels();  // 声道数（1或2）
    int sampleRate = MP3GetSampRate(); // 采样率（如44100）
    int bitDepth = MP3GetBitsPerSample(); // 位深度（如16）
    if (bitDepth != 16) {
        LOGE("bit depth %d not supported", bitDepth);
        stop();
        return false;
    }
    if (m_sampleRateCallback)
        m_sampleRateCallback(sampleRate);
    if (m_bitDepthCallback)
        m_bitDepthCallback(bitDepth);
    if (m_playPCMCallback)
        m_playPCMCallback(outbuf, samps, channels);
    int readBytes = m_audioBuffer.fill(m_pAudioFile);
    if (readBytes <= 0) { // eof
        onFinish();
        return false;
    }
    return true;
}
void AudioPlayer::onFinish() {
    if (m_bLoop) {
        playImmediately();
    } else {
        stop();
    }
}

