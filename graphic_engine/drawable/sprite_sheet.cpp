#include "sprite_sheet.h"
#include <cmath>
SpriteSheet::SpriteSheet(uint16_t width, uint16_t height,const void* data, uint16_t col, uint16_t row, bool hasMask, uint16_t maskColor,
const uint16_t* palette, uint8_t bpp)
: Drawable(Vector2(width, height), hasMask, maskColor, palette, bpp),m_data(data),m_col(col),m_row(row),m_currentFrame(0),m_frameDataPtr(nullptr)
{
    m_originSize = m_size / Vector2(m_col, m_row);
    m_size = m_originSize;
    m_originMaskColor = m_maskColor;
    setFrame(0);
}
void SpriteSheet::setFrame(int nth) {
    if (m_frameDataPtr && nth == m_currentFrame)
        return;
    setRedraw(true);
    m_currentFrame = nth;
    m_size = m_originSize;
    uint16_t w = (uint16_t)m_size.x;
    uint16_t h = (uint16_t)m_size.y;
    if (m_bpp <= 16) { // no alpha
        m_frameDataPtr = (uint16_t*)m_data + m_currentFrame*w*h / (16 / m_bpp);
    } else {
        m_frameDataPtr = (uint32_t*)m_data + m_currentFrame*w*h;
    }
    m_maskColor = m_originMaskColor;
}
void SpriteSheet::setFrameData(const void* data, uint16_t width, uint16_t height, uint16_t maskColor) {
    if (m_frameDataPtr == data && m_size.x == width && m_size.y == height)
        return;
    setRedraw(true);
    m_frameDataPtr = data;
    m_size.x = width;
    m_size.y = height;
    m_maskColor = maskColor;
}