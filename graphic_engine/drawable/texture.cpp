#include "texture.h"

using namespace cubicat;

Texture::Texture(uint16_t width, uint16_t height, const void* data, uint16_t col, uint16_t row,const uint16_t* palette,
uint8_t bpp, bool hasAlpha):m_pData(data), m_col(col), m_row(row), m_width(width), m_height(height), m_palette(palette),
m_bpp(bpp), m_hasAlpha(hasAlpha) {
    m_frameWidth = width / col;
    m_frameHeight = height / row;
    setFrame(0);
}

Texture::~Texture() {
}

const void* Texture::getFrameData(uint16_t idx) {
    int frameSize = getFrameWidth() * getFrameHeight();
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
}