#include "bmfont.h"
#include "font.h"
#include <cmath>
#include "utils/helper.h"
#include <algorithm>
#include "gb2312font_20.h"

FontData BMFont::m_sFontData = DefaultFontData;

BMFont::BMFont(const Vector2& size, const char* text, uint16_t color)
: Drawable(size, true, 0xFFFF - color, generateSinglePalette(color)) {
    setPivot(0, 0);
    auto characters = splitUTF8(text);
    const uint16_t SingleGlyphDataLen = 4 + ceil(m_sFontData.fontSize * m_sFontData.fontSize / 8.0f);
    uint32_t len = (uint32_t)ceil(m_size.x * m_size.y / sizeof(uint16_t));
    m_pData = new uint16_t[len];
    memset(m_pData, 0, len * sizeof(uint16_t));
    for (auto& character : characters) {
        uint8_t glyphWidth = m_sFontData.fontSize;
        int index = getCharIndex((const char*)m_sFontData.charSet, character.c_str());
        if (index >= 0) {
            auto ptr = m_sFontData.glyphData + index * SingleGlyphDataLen;
            // 字模数据前4字节为字模bbox数据
            uint8_t data[4] = { 0 };
            memcpy(data, (uint8_t*)ptr, 4);
            uint8_t x_offset = data[0];
            uint8_t y_offset = data[1];
            glyphWidth = data[2] - data[0];
            uint8_t glyphHeight = data[3] - data[1];
            ptr += 4; // 跳过前面4字节bbox数据，开始字模数据
            for (uint8_t y = 0; y < glyphHeight; y++) {
                for (uint8_t x = 0; x < glyphWidth; x++) {
                    uint16_t canvasX = m_cursorPos.x + x + x_offset;
                    uint16_t canvasY = m_cursorPos.y + y + y_offset;
                    // 检测是否超出了label画布范围
                    if (canvasX > m_size.x || canvasY > m_size.y) {
                        continue;
                    }
                    // 字模中的数据偏移量
                    auto offset = y * glyphWidth + x;
                    uint8_t v = ptr[offset / 8] & (1 << (7 - offset % 8));
                    if (v) {
                        // 计算在整个label画布中的偏移量
                        offset = canvasY * m_size.x + canvasX;
                        m_pData[offset / 16] |= 0x1 << (15 - offset % 16);
                    }
                }
            }
        }
        m_cursorPos.x += glyphWidth;
        if (character == "\n") {
            m_cursorPos.x = 0;
            m_cursorPos.y += m_sFontData.fontSize;
        }
    }
    
}
BMFont::~BMFont() {
    delete[] m_pData;
    delete[] m_palette;
}
void BMFont::setColor(uint16_t color) {
    setMaskColor(0xFFFF - color);
    setPalette(generateSinglePalette(color));
}
Vector2 BMFont::calculateUTFStringRect(const char* text) {
    const uint16_t SingleGlyphDataLen = 4 + ceil(m_sFontData.fontSize * m_sFontData.fontSize / 8.0f);
    auto lines = splitString(text, "\n");
    uint16_t height = (m_sFontData.fontSize * 1.5) * lines.size();//加半个字模高度，为了不同字符的高低对齐
    uint16_t width = 0;
    for (auto& line : lines) {
        auto characters = splitUTF8(line.c_str());
        uint16_t lineWidth = 0;
        for (auto& character : characters) {
            int index = getCharIndex((const char*)m_sFontData.charSet, character.c_str());
            if (index >= 0) {
                auto ptr = m_sFontData.glyphData + index * SingleGlyphDataLen;
                // 字模数据前4字节为字模bbox数据
                uint8_t data[4] = { 0 };
                memcpy(data, (uint8_t*)ptr, 4);
                auto glyphWidth = data[2] - data[0];
                lineWidth += glyphWidth;
            }
        }
        width = std::max(width, lineWidth);
    }
    return Vector2(width, height);

}