#include "unified_storage.h"
#include "esp_spiffs.h"
#include "driver/sdmmc_default_configs.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "nvs_flash.h"
#include <string.h>

#define FLASHPATH "/spiffs"
#define SDPATH "/sdcard"
#define NVSNS "storage"
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


void UnifiedStorage::init()
{
    // init nvs
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    // init SPIFFS 
    if (!m_bFlashInited) {
        esp_vfs_spiffs_conf_t conf = {
            .base_path = FLASHPATH,
            .partition_label = "spiffs",
            .max_files = 5,
            .format_if_mount_failed = false
        };
        ret = esp_vfs_spiffs_register(&conf);
        if (ret != ESP_OK) {
            printf("Failed to mount SPIFFS (%s), maybe no spiffs partition specified?", esp_err_to_name(ret));
        } else {
            m_bFlashInited = true;
        }
    }
    // init SD card
    if (!m_bSDInited) {
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
void UnifiedStorage::deinit() {
    
}
nvs_handle_t nvsWriteHandle() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVSNS, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        return -1;
    }
    return nvs_handle;
}
nvs_handle_t nvsReadHandle() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVSNS, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return -1;
    }
    return nvs_handle;
}
void UnifiedStorage::setString(const char* key, const char* value) {
    nvs_set_str(nvsWriteHandle(), key, value);
}   
void UnifiedStorage::setInt(const char* key, int value) {
    nvs_set_i32(nvsWriteHandle(), key, value);
}
void UnifiedStorage::setFloat(const char* key, float value) {
    char str[32] = {0};
    sprintf(str, "%f", value);
    setString(key, str);
}
int UnifiedStorage::getInt(const char* key) {
    int32_t out = 0;
    nvs_get_i32(nvsReadHandle(), key, &out);
    return out;
}
float UnifiedStorage::getFloat(const char* key) {
    const char* str = getString(key);
    return atof(str);
}
const char* UnifiedStorage::getString(const char* key) {
    static char temp[4000];
    memset(temp, 0, 4000);
    size_t len = 0;
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
void UnifiedStorage::saveFileFlash(const char* filename, const char* content, int len, bool binary, bool append) {
    if (!m_bFlashInited) {
        printf("flash not inited yet\n");
        return;
    }
    char path[PATH_MAX] = {0};
    sprintf(path, "%s/%s", FLASHPATH, filename);
    saveFile(path, content, len, binary, append);
}
FILE* UnifiedStorage::openFileFlash(const char* filename, bool binary) {
    if (!m_bFlashInited) {
        printf("flash not inited yet\n");
        return NULL;
    }
    char path[PATH_MAX] = {0};
    sprintf(path, "%s/%s", FLASHPATH, filename);
    return fopen(path, binary?"rb":"r");
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