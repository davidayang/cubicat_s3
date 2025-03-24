#ifndef _UNIFIED_STORAGE_H_
#define _UNIFIED_STORAGE_H_

#include "core/memory_object.h"
#include <cstdio>
#include <string>
#include <vector>

class UnifiedStorage : public MemoryObject
{
    friend class Cubicat;
public:
    UnifiedStorage(const UnifiedStorage&) = delete;
    ~UnifiedStorage();

    // CAN NOT use MICPHONE and SD simultaneously
    void init(bool sdEnable);
    void deinit();
    bool setString(const char* key, const char* value);
    bool setInt(const char* key, int value);
    bool setFloat(const char* key, float value);
    int getInt(const char* key);
    float getFloat(const char* key);
    std::string getString(const char* key);
    void saveFileSD(const char* filename, const char* content, int len, bool binary = true, bool append = false);
    FILE* openFileSD(const char* filename, bool binary = true);
    bool partitionSelect(const char* label);
    const std::vector<std::string>& getPartitions();
    std::string getActivePartition();
    void saveFileFlash(const char* filename, const char* content, int len, bool binary = true, bool append = false);
    FILE* openFileFlash(const char* filename, bool binary = true);
    uint32_t getFlashFreeSpace(const char* partition_label = "spiffs");
    uint32_t getSDFreeSpace();
private:
    UnifiedStorage();
    bool isSPIFFSInit();
    bool isSDInit();
    bool m_bSDInited = false;
    bool m_bFlashInited = false;
    std::vector<std::string> m_vFlashPartitions;
    std::string m_currentPartition;
};


#endif