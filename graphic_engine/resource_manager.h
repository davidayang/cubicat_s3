/*
* @author       Isaac
* @date         2025-02-22
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
* @description  resource manager class for graphic engine, loading and unloading resources
*/
#ifndef _RESOURCE_MANAGER_H_
#define _RESOURCE_MANAGER_H_
#include <unordered_map>
#include <string>
#include "drawable/texture.h"

extern FILE* openFileFlash(const char* filename, bool binary);
extern FILE* openFileSD(const char* filename, bool binary);

namespace cubicat {

class ResourceManager {
public:
    // [JS_BINDING_BEGIN]
    TexturePtr loadTexture(const std::string& name, bool fromFlash = true);
    void removeTexture(const std::string& name);
    TexturePtr getTexture(const std::string& name);
    // [JS_BINDING_END]
    void purge();
private:
    std::unordered_map<std::string, TexturePtr>                 m_textures;
};

}

#endif