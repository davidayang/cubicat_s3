#include "texture.h"
#include "core/memory_allocator.h"
#include "utils/logger.h"
#include <string.h>

using namespace cubicat;

Texture::Texture(uint16_t width, uint16_t height, const void* data, bool manageData, uint16_t col, uint16_t row,const uint16_t* palette,
uint8_t bpp, bool hasAlpha):m_pData(data), m_bDataManaged(manageData), m_col(col), m_row(row), m_width(width), m_height(height), m_palette(palette),
m_bpp(bpp), m_hasAlpha(hasAlpha) {
    m_frameWidth = m_width / m_col;
    m_frameHeight = m_height / m_row;
    setFrame(0);
}

Texture::~Texture() {
    if (m_bDataManaged) {
        free((void*)m_pData);
    }
}
void Texture::setAsSpriteSheet(uint16_t row, uint16_t col) {
    if (!m_bDataManaged) {
        LOGI("Texture is not managed, can't set as sprite sheet");
        return;
    }
    if ((row == m_row && col == m_col) || (row == 0 && col == 0))
        return;
    if (m_bReordered) {
        // Todo: transfer data to unordered texture 
        assert(false);
    }
    m_row = row;
    m_col = col;
    m_frameWidth = m_width / m_col;
    m_frameHeight = m_height / m_row;
    int horizontalRemainder = m_width % m_col;
    float pixelBytes = m_bpp <= 16 ? m_bpp / 8.0f : 4;
    float sizeForTexLine = m_width * pixelBytes;
    float sizeForFrameLine = m_frameWidth * pixelBytes;
    if (col > 1) {
        void* newData = psram_prefered_malloc(ceil(m_width * m_height * pixelBytes));
        m_bDataManaged = true;
        auto frameSize = m_frameWidth * m_frameHeight * pixelBytes;
        for (int r = 0; r < m_row; r++) {
            for (int c = 0; c < m_col; c++) {
                for (int h = 0; h < m_frameHeight; h++) {
                    const void* src = (char*)m_pData + (int)((r * m_frameHeight + h) * sizeForTexLine + c * m_frameWidth * pixelBytes);
                    void* dst = (char*)newData + (int)((r * m_col + c) * frameSize + h * sizeForFrameLine);
                    memcpy(dst, src, sizeForFrameLine);
                }
            }
        }
        free((void*)m_pData);
        m_pData = newData;
        m_bReordered = true;
    }
    setFrame(0);
}

const void* Texture::getFrameData(uint16_t idx) {
    int frameSize = m_frameWidth * m_frameHeight;
    if (m_bpp <= 16) { // no alpha
        return (uint16_t*)m_pData + idx * frameSize / (16 / m_bpp);
    } else {
        return (uint32_t*)m_pData + idx * frameSize;
    }
}

uint16_t Texture::getFrameCount() {
    return m_col * m_row;
}
void Texture::setFrame(int nth) {
    int c = getFrameCount();
    nth = nth % c;
    if (nth < 0) 
        nth += c;
    m_frame = nth;
    m_pFramePtr = getFrameData(m_frame);
    addDirty(true);
}
Texture* Texture::shallowCopy() {
    return NEW Texture(m_width, m_height, m_pData, false, m_col, m_row, m_palette, m_bpp, m_hasAlpha);
}

SolidTexture::SolidTexture(uint16_t color) : Texture(1, 1, nullptr, false, 1, 1, nullptr, 16, false), m_color(color) {

}

