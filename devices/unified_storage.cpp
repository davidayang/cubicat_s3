#include "unified_storage.h"
#include "esp_spiffs.h"
#include "driver/sdmmc_default_configs.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "nvs_flash.h"
#include <string.h>
#include "utils/logger.h"

#define SDPATH "/sdcard"
#define NVSNS "nvs"
#define MMC_HOST_SLOT SDMMC_HOST_SLOT_0

#define SD_SCL      GPIO_NUM_2
#define SD_CMD      GPIO_NUM_14
#define SD_DATA0    GPIO_NUM_9
#define SD_DATA1    GPIO_NUM_8
#define SD_DATA2    GPIO_NUM_16
#define SD_DATA3    GPIO_NUM_15

UnifiedStorage::UnifiedStorage()
{
}

UnifiedStorage::~UnifiedStorage()
{
}


void UnifiedStorage::init(bool sd)
{
    // init nvs
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    // init SPIFFS 
    if (!m_bFlashInited) {
        esp_partition_iterator_t partItr = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
        if (partItr == NULL) {
            printf("No SPIFFS partition found\n");
        } else {
            m_bFlashInited = true;
            uint8_t spiffsCount = 0;
            while (partItr != NULL) {
                const esp_partition_t* part = esp_partition_get(partItr);
                // Todo: hack code to skip the model partition, need optimize
                partItr = esp_partition_next(partItr);
                if (strcmp(part->label, "model") == 0) {
                    continue;
                }
                char basePath[32] = {0};
                if (strlen(part->label) > 0) {
                    snprintf(basePath, sizeof(basePath), "/%s", part->label);
                } else {
                    sprintf(basePath, "/spiffs%d", spiffsCount);
                }
                if (m_currentPartition.empty()) {
                    m_currentPartition = basePath;
                }
                esp_vfs_spiffs_conf_t conf = {
                    .base_path = basePath,
                    .partition_label = part->label,
                    .max_files = 5,
                    .format_if_mount_failed = true
                };
                ret = esp_vfs_spiffs_register(&conf);
                if (ret != ESP_OK) {
                    LOGE("Failed to mount SPIFFS: [%s] (%d), maybe no spiffs partition specified?\n", part->label, ret);
                    m_bFlashInited = false;
                    break;
                }
                m_vFlashPartitions.push_back(basePath);
                spiffsCount++;
            }
            esp_partition_iterator_release(partItr);
        }
    }
    // init SD card
    if (!m_bSDInited && sd) {
        sdmmc_host_t host = SDMMC_HOST_DEFAULT();
        host.slot = SDMMC_HOST_SLOT_0;
        host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
        sdmmc_slot_config_t slot_config = {
            .clk = SD_SCL,
            .cmd = SD_CMD,
            .d0 = SD_DATA0,
            .d1 = SD_DATA1,
            .d2 = SD_DATA2,
            .d3 = SD_DATA3,
            .d4 = GPIO_NUM_NC,
            .d5 = GPIO_NUM_NC,
            .d6 = GPIO_NUM_NC,
            .d7 = GPIO_NUM_NC,
            .cd = SDMMC_SLOT_NO_CD,
            .wp = SDMMC_SLOT_NO_WP,
            .width   = 4, // 4 line mmc
            .flags = 0,
        };
        esp_vfs_fat_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
            .allocation_unit_size = 0,
            .disk_status_check_enable = false,
            .use_one_fat = true
        };
        sdmmc_card_t *card;
        esp_err_t ret = esp_vfs_fat_sdmmc_mount(SDPATH, &host, &slot_config, &mount_config, &card);
        if (ret != ESP_OK) {
            printf("Failed to mount SD card (%s), maybe there is no SD card?", esp_err_to_name(ret));
        } else {
            m_bSDInited = true;
        }
    }
}
bool UnifiedStorage::isSPIFFSInit() {
    return m_bFlashInited;
}
bool UnifiedStorage::isSDInit() {
    return m_bSDInited;
}
void UnifiedStorage::deinit() {
    auto ret = esp_vfs_fat_sdmmc_unmount();
    if (ret != ESP_OK) {
        LOGE("Failed to unmount FAT filesystem. Error: %s\n", esp_err_to_name(ret));
        return;
    }
    m_bSDInited = false;
}
nvs_handle_t nvsWriteHandle() {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open(NVSNS, NVS_READWRITE, &nvs_handle));
    return nvs_handle;
}
nvs_handle_t nvsReadHandle() {
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVSNS, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        // use NVS_READWRITE to create namespace if not exist
        return nvsWriteHandle();
    }
    return nvs_handle;
}
bool UnifiedStorage::setString(const char* key, const char* value) {
    if (!isSPIFFSInit()) {
        return false;
    }
    nvs_set_str(nvsWriteHandle(), key, value);
    return true;
}   
bool UnifiedStorage::setInt(const char* key, int value) {
    if (!isSPIFFSInit()) {
        return false;
    }
    nvs_set_i32(nvsWriteHandle(), key, value);
    return true;
}
bool UnifiedStorage::setFloat(const char* key, float value) {
    if (!isSPIFFSInit()) {
        return false;
    }
    char str[32] = {0};
    sprintf(str, "%f", value);
    setString(key, str);
    return true;
}
int UnifiedStorage::getInt(const char* key) {
    if (!isSPIFFSInit()) {
        return 0;
    }
    int32_t out = 0;
    nvs_get_i32(nvsReadHandle(), key, &out);
    return out;
}
float UnifiedStorage::getFloat(const char* key) {
    if (!isSPIFFSInit()) {
        return 0.0f;
    }
    const char* str = getString(key).c_str();
    return atof(str);
}
std::string UnifiedStorage::getString(const char* key) {
    if (!isSPIFFSInit()) {
        return "";
    }
    static char temp[4*1024];
    size_t len = sizeof(temp);
    nvs_get_str(nvsReadHandle(), key, temp, &len);
    return temp;
}
void saveFile(const char* path, const char* content, int len, bool binary, bool append) {
    char mode[3] = {0};
    if (binary) {
        sprintf(mode, append ? "ab" : "wb");
    } else {
        sprintf(mode, append ? "a" : "w");
    }
    FILE* f = fopen(path, mode);
    if (f) {
        fwrite(content, 1, len, f);
        fclose(f);
    }
}

void UnifiedStorage::saveFileSD(const char* filename, const char* content, int len, bool binary, bool append) {
    if (!m_bSDInited) {
        printf("sd not inited yet\n");
        return;
    }
    char path[PATH_MAX] = {0};
    sprintf(path, "%s/%s", SDPATH, filename);
    saveFile(path, content, len, binary, append);
}
FILE* UnifiedStorage::openFileSD(const char* filename, bool binary) {
    if (!m_bSDInited) {
        printf("sd not inited yet\n");
        return NULL;
    }
    char path[PATH_MAX] = {0};
    sprintf(path, "%s/%s", SDPATH, filename);
    return fopen(path, binary?"rb":"r");
}
bool UnifiedStorage::partitionSelect(const char* label) {
    if (!m_bFlashInited) {
        LOGW("select partition failed flash not inited yet\n");
        return false;
    }
    std::string basePath(label);   
    if (basePath[0] != '/') {
        basePath = "/" + basePath;
    }
    for (auto& partition : m_vFlashPartitions) {
        if (partition == basePath) {
            m_currentPartition = partition;
            return true;
        }
    }
    return false;
}
std::string UnifiedStorage::getActivePartition() {
    return m_currentPartition;
}
const std::vector<std::string>& UnifiedStorage::getPartitions() {
    return m_vFlashPartitions;
}
void UnifiedStorage::saveFileFlash(const char* filename, const char* content, int len, bool binary, bool append) {
    if (!m_bFlashInited) {
        LOGW("save failed flash not inited yet\n");
        return;
    }
    char path[PATH_MAX] = {0};
    sprintf(path, "%s/%s", m_currentPartition.c_str(), filename);
    saveFile(path, content, len, binary, append);
}
FILE* UnifiedStorage::openFileFlash(const char* filename, bool binary) {
    if (!m_bFlashInited) {
        LOGW("open failed flash not inited yet\n");
        return NULL;
    }
    if (filename == nullptr) {
        LOGW("filename is null\n");
        return NULL;
    }
    char path[PATH_MAX] = {0};
    if (filename[0] != '/') {
        sprintf(path, "%s/%s", m_currentPartition.c_str(), filename);
    } else {
        sprintf(path, "%s", filename);
    }
    FILE* f = fopen(path, binary?"rb":"r");
    if (!f) {
        LOGE("open file %s failed\n", path);
    }
    return f;
}
uint32_t UnifiedStorage::getFlashFreeSpace(const char* partition_label) {
    size_t totalBytes = 0;
    size_t usedBytes = 0;
    if (esp_spiffs_info(partition_label, &totalBytes, &usedBytes) == ESP_OK) {
        return totalBytes - usedBytes;
    } else {
        return 0;
    }
}
uint32_t UnifiedStorage::getSDFreeSpace() {
    FATFS* fs;
    DWORD free_clusters, free_sectors;
    FRESULT res = f_getfree(SDPATH, &free_clusters, &fs);
    if (res == FR_OK) {
        free_sectors = free_clusters * fs->csize;
        return free_sectors * 512;
    } else {
        printf("Failed to get free space: %s", esp_err_to_name(res));
        return 0;
    }
}