#include "material.h"
using namespace cubicat;

Material::Material() {

}
void Material::setBlendMode(BlendMode mode) {
    if (m_eBlendMode != mode) {
        m_eBlendMode = mode; 
        markDirty();
        if (mode == Additive)
            m_bTransparent = true; 
    }
} 
void Material::setBilinearFilter(bool b) {
    if (m_bBilinearFilter != b) {
        m_bBilinearFilter = b; 
        markDirty();
    }
}
void Material::setTransparent(bool t) {
    if (m_bTransparent != t) {
        m_bTransparent = t; 
        markDirty();
    }
}
bool Material::isTransparent() {
    return m_bTransparent || m_pTexture->hasAlpha(); 
}

void Material::setEmissive(float e) {
    if (e >= 0) {
        m_fEmissive = e; 
        markDirty();
    }
}

void Material::setColor(uint16_t color) {
    if (m_color != color) {
        m_color = color; 
        markDirty(); 
    }
}

void Material::setTexture(TexturePtr texture) {
    if (m_pTexture != texture) {
        m_pTexture = texture; 
        markDirty();
    }
}
void Material::setMask(bool hasMask) {
    if (m_bHasMask != hasMask) {
        m_bHasMask = hasMask; 
        markDirty();
    }
}
void Material::setMaskColor(uint16_t color) {
    if (m_maskColor != color) {
        m_maskColor = color; 
        markDirty();
    }
}