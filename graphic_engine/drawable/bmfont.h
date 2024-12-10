#ifndef _BMFONT_H_
#define _BMFONT_H_
#include "drawable.h"
#include "font.h"

class BMFont : public Drawable
{
public:
    DECLARE_RTTI_SUB(BMFont, Drawable);
    static void setFontData(FontData data) { m_sFontData = data; }
    static const FontData& getFontData() { return m_sFontData; }
    static uint8_t getFontSize() { return m_sFontData.fontSize; }
    static Vector2 calculateUTFStringRect(const char* text);
    static SharedPtr<BMFont> create(const Vector2& size,const char* text, uint16_t color = 0x0) {
        return SharedPtr<BMFont>(NEW BMFont(size, text, color));
    }
    ~BMFont();
    const uint16_t* getDrawData() { return m_pData; }
    void setColor(uint16_t color);
private:
    BMFont(const Vector2& size,const char* text, uint16_t color = 0x0);
    uint16_t*       m_pData;
    static FontData m_sFontData;
    Vector2         m_cursorPos;
};
#endif