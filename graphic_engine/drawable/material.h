/*
* @author       Isaac
* @date         2025-01-25
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
*/
#ifndef _MATERIAL_H_
#define _MATERIAL_H_
#include "core/shared_pointer.h"
#include "core/memory_object.h"
#include "texture.h"
#include "../dirtyable.h"

namespace cubicat {
    
enum BlendMode {
    Normal,
    Additive,
    Multiply
};
class Material;
typedef DirtyNotifyPointer<Texture,Material> MaterialTexture;

class Material : public MemoryObject, public Dirtyable {
public:
    static SharedPtr<Material> create() { return SharedPtr<Material>(new Material()); }
    MaterialTexture getTexture() { return MaterialTexture(m_pTexture, this); }
    // [JS_BINDING_BEGIN]
    void setTexture(TexturePtr texture);
    TexturePtr texturePtr() { return m_pTexture; }
    void setColor(uint16_t color);
    void setMask(bool hasMask);
    void setMaskColor(uint16_t color);
    void setEmissive(float e);
    void setBlendMode(BlendMode mode);
    void setBilinearFilter(bool b);
    void setTransparent(bool t);
    // [JS_BINDING_END]

    uint16_t getColor() { return m_color; }
    bool hasMask() { return m_bHasMask; }
    uint16_t getMaskColor() { return m_maskColor; }
    float getEmissive() { return m_fEmissive; }
    BlendMode getBlendMode() { return m_eBlendMode; }
    bool isBilinearFilter() { return m_bBilinearFilter; }
    bool isTransparent();
private:
    Material();
    TexturePtr      m_pTexture;
    uint16_t        m_color = 0xffff;
    bool            m_bHasMask = false;
    uint16_t        m_maskColor = 0xffff;
    float           m_fEmissive = 0.0f;
    bool            m_bBilinearFilter = false;
    BlendMode       m_eBlendMode = Normal;
    bool            m_bTransparent = false;
};

typedef SharedPtr<Material> MaterialPtr;

}
#endif