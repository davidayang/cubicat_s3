#include "material.h"
using namespace cubicat;

Material::Material() {

}
void Material::setBlendMode(BlendMode mode) {
    if (m_eBlendMode != mode) {
        m_eBlendMode = mode; 
        addDirty(true);
        if (mode == Additive)
            m_bTransparent = true; 
    }
} 
void Material::setBilinearFilter(bool b) {
    if (m_bBilinearFilter != b) {
        m_bBilinearFilter = b; 
        addDirty(true);
    }
}
void Material::setTransparent(bool t) {
    if (m_bTransparent != t) {
        m_bTransparent = t; 
        addDirty(true);
    }
}
bool Material::isTransparent() {
    return m_bTransparent || m_pTexture->hasAlpha(); 
}

void Material::setEmissive(float e) {
    if (e >= 0) {
        m_fEmissive = e; 
        addDirty(true);
    }
}

void Material::setColor(uint16_t color) {
    if (m_color != color) {
        m_color = color; 
        addDirty(true); 
    }
}

void Material::setTexture(TexturePtr texture) {
    if (m_pTexture != texture) {
        m_pTexture = texture; 
        addDirty(true);
    }
}
void Material::setMask(bool hasMask) {
    if (m_bHasMask != hasMask) {
        m_bHasMask = hasMask; 
        addDirty(true);
    }
}
void Material::setMaskColor(uint16_t color) {
    if (m_maskColor != color) {
        m_maskColor = color; 
        addDirty(true);
    }
}