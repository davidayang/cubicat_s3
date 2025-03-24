#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include "core/memory_object.h"
#include "core/shared_pointer.h"
#include "core/rtti.h"
#include "../math/vector2.h"
#include "../dirtyable.h"

namespace cubicat {

class Texture : public MemoryObject, public RTTI, public Dirtyable
{
public:
    DECLARE_RTTI_ROOT(Texture);
    static SharedPtr<Texture> create(uint16_t width, uint16_t height, const void* data, bool manageData, uint16_t col, uint16_t row,
    const uint16_t* palette = nullptr, uint8_t bpp = 16, bool hasAlpha = false) {
        return SharedPtr<Texture>(new Texture(width, height, data, manageData, col, row, palette, bpp, hasAlpha));
    }
    Texture(uint16_t width, uint16_t height, const void* data, bool manageData, uint16_t col, uint16_t row,const uint16_t* palette, 
    uint8_t bpp, bool hasAlpha);
    virtual ~Texture();
    const void* getTextureData() { return m_pData; }
    // [JS_BINDING_BEGIN]
    void setAsSpriteSheet(uint16_t row, uint16_t col);
    void setFrame(int nth);
    Texture* shallowCopy();
    // [JS_BINDING_END]
    // texure size equal to original texture size when there is only one frame
    Vector2us getTextureSize() { return Vector2us(m_frameWidth, m_frameHeight); }
    const void* getFrameData() { return m_pFramePtr; }
    uint16_t getFrameCount();
    bool hasAlpha() { return m_hasAlpha;}
    uint8_t getColorDepth() { return m_bpp; }
    virtual bool readPixel(int32_t x, int32_t y, uint32_t* value);
    // read pixel without border checking
    virtual uint32_t readPixelUnsafe(uint32_t x, uint32_t y);
    const uint16_t* getPalette() { return m_palette; }
private:
    const void* getFrameData(uint16_t idx);
    const void*     m_pData;
    const void*     m_pFramePtr;
    uint16_t        m_col;
    uint16_t        m_row;
    uint16_t        m_width;
    uint16_t        m_height;
    uint16_t        m_frameWidth;
    uint16_t        m_frameHeight;
    bool            m_hasAlpha;
    const uint16_t* m_palette;
    const uint8_t   m_bpp;      //bit per pixel
    uint16_t        m_frame = 0;
    bool            m_bDataManaged = false;
    bool            m_bReordered = false;
};
typedef SharedPtr<Texture> TexturePtr;

inline bool Texture::readPixel(int32_t x, int32_t y, uint32_t* value) {
    if (x < 0 || x >= m_frameWidth || y < 0 || y >= m_frameHeight)
        return false;
    *value = readPixelUnsafe(x, y);
    return true;
}

inline uint32_t Texture::readPixelUnsafe(uint32_t x, uint32_t y) {
    uint32_t offset = y * (uint32_t)m_frameWidth + x;
    uint8_t subIndex = 0;
    if (m_palette) {
        uint8_t divider = 16 / m_bpp;
        subIndex = offset % divider;
        offset /= divider;
    }
    if (m_bpp == 16) {
        const uint16_t* address = (uint16_t*)m_pFramePtr + offset;
        uint32_t value = *(address);
        if (m_palette) {
            value = m_palette[value >> (15-subIndex) & 0x1];
        }
        return value;
    } else if (m_bpp == 32) {
        return *((uint32_t*)m_pFramePtr + offset);
    } else {
        assert(0 && "Invalid color depth");
        return 0;
    }
}

class SolidTexture : public Texture {
public:
    DECLARE_RTTI_SUB(SolidTexture, Texture);
    SolidTexture(uint16_t color = 0xffff);
    void setAsSpriteSheet(uint16_t row, uint16_t col) = delete;
    void setFrame(int nth) = delete;
    Texture* copy() = delete;
    bool readPixel(int32_t x, int32_t y, uint32_t* value) override { *value = m_color; return true; }
    uint32_t readPixelUnsafe(uint32_t x, uint32_t y) override { return m_color; }
private:
    uint16_t    m_color;
};

} // namespace cubicat
#endif