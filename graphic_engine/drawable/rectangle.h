#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_
#include "drawable.h"

class Rectangle : public Drawable

{
public:
    DECLARE_RTTI_SUB(Rectangle, Drawable);
    static SharedPtr<Rectangle> create(uint32_t width, uint32_t height, uint16_t color, bool fill, uint32_t cornerRadius = 0,
     uint32_t lineWidth = 1) {
        return SharedPtr<Rectangle>(NEW Rectangle(width, height, color, fill, cornerRadius, lineWidth));
    }
    ~Rectangle();

    const void* getTextureData() override { return m_pData; }
    void setColor(uint16_t color);
private:
    // lineWidth 只有在fill为tfalse,即为轮廓模式时才有效
    Rectangle(uint32_t width, uint32_t height, uint16_t color, bool fill, uint32_t cornerRadius = 0, uint32_t lineWidth = 1);
    void setPixel(uint16_t x, uint16_t y);
    uint16_t*       m_pData;
};


#endif