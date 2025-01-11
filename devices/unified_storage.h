#ifndef _UNIFIED_STORAGE_H_
#define _UNIFIED_STORAGE_H_

#include "core/memory_object.h"
#include "core/macros.h"
#include <cstdio>
#include <string>

class UnifiedStorage : public MemoryObject
{
    friend class Cubicat;
public:
    UnifiedStorage(const UnifiedStorage&) = delete;
    ~UnifiedStorage();

    // CAN NOT use MICPHONE and SD simultaneously
    void init(bool sd = false);
    void deinit();
    void setString(const char* key, const char* value);
    void setInt(const char* key, int value);
    void setFloat(const char* key, float value);
    int getInt(const char* key);
    float getFloat(const char* key);
    std::string getString(const char* key);
    void saveFileSD(const char* filename, const char* content, int len, bool binary = true, bool append = false);
    FILE* openFileSD(const char* filename, bool binary = true);
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
};


#endif