#ifndef _TEXTURE_H_
#define _TEXTURE_H_
#include "core/memory_object.h"
#include "core/shared_pointer.h"

namespace cubicat {

class Texture : public MemoryObject
{
public:
    static SharedPtr<Texture> create(uint16_t width, uint16_t height, const void* data, uint16_t col, uint16_t row, 
    const uint16_t* palette = nullptr, uint8_t bpp = 16, bool hasAlpha = false) {
        return SharedPtr<Texture>(new Texture(width, height, data, col, row, palette, bpp, hasAlpha));
    }
    Texture(uint16_t width, uint16_t height, const void* data, uint16_t col, uint16_t row,const uint16_t* palette, 
    uint8_t bpp, bool hasAlpha);
    ~Texture();
    uint16_t getWidth() const { return m_width; }
    uint16_t getHeight() const { return m_height; }
    const void* getTextureData() { return m_pData; }
    uint16_t getFrameWidth() { return m_frameWidth; }
    uint16_t getFrameHeight() { return m_frameHeight; }
    const void* getFrameData(uint16_t idx);
    const void* getFrameData() { return m_pFramePtr; }
    void setFrame(int nth);
    uint16_t getFrameCount();
    bool hasAlpha() { return m_hasAlpha;}
    uint8_t getColorDepth() { return m_bpp; }
    bool readPixel(int32_t x, int32_t y, uint32_t* value);
    uint32_t readPixelUnsafe(uint32_t x, uint32_t y);
private:
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
    const uint8_t   m_bpp;
    uint16_t        m_frame = 0;
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
        const uint32_t* address = (uint32_t*)m_pFramePtr + offset;
        return *address;
    } else {
        assert(0 && "Invalid color depth");
        return 0;
    }
}

} // namespace cubicat
#endif