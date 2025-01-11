#ifndef _DRAWABLE_H_
#define _DRAWABLE_H_
#include <cstdint>
#include "core/shared_pointer.h"
#include "../math/vector2.h"
#include "core/rtti.h"
#include "core/memory_object.h"
#include "../region.h"
#include "../math/constants.h"


class Drawable : public RTTI, public MemoryObject
{
public:
    DECLARE_RTTI_ROOT(Drawable);
    virtual ~Drawable() {}
    virtual const void* getDrawData()  __attribute__((always_inline)) = 0;
    virtual void update(const Vector2& pos,const Vector2& scale, int16_t rotation);
    uint32_t getId() {return m_id;}
    const Vector2& getSize() {return m_size;}
    const Vector2& getScale() {return m_scale;}
    const Vector2& getPos() {return m_pos;}
    int16_t getRot() {return m_rotation;}
    void setMaskColor(uint16_t color) {m_maskColor = color;setRedraw(true);}
    uint16_t getMaskColor() {return m_maskColor;}
    bool hasMask() {return m_bHasMask;}
    bool hasAlpha() {return m_bpp == 24;}
    const uint16_t* getPalette() {return m_palette;}
    void setPalette(const uint16_t* palette) {m_palette = palette;}
    // bit per pixel
    uint8_t getBPP() {return m_bpp;}
    const Vector2& getPivot() {return m_pivot;} 
    void setPivot(float x, float y) {m_pivot.x = x; m_pivot.y = y;}
    bool readPixel(int16_t x, int16_t y, uint32_t* value);
    // read pixel without bounds checking
    uint32_t readPixelUnsafe(uint16_t x, uint16_t y) __attribute__((always_inline));
    void setRedraw(bool redraw) {m_bRedraw = redraw;}
    bool needRedraw() {return m_bRedraw;}
    Region getBoundingBox() {return m_boundingBox;}
protected:
    Drawable(const Vector2& size, bool hasMask = false,uint16_t maskColor = 0, const uint16_t* palette = nullptr, uint8_t bpp = 1)
    : m_size(size), m_bHasMask(hasMask), m_maskColor(maskColor), m_palette(palette), m_bpp(bpp), m_pivot(Vector2(0.5f, 0.5)),m_bRedraw(true) {
        m_id = m_idCounter++;
    }
    Vector2             m_size;
    Vector2             m_scale;
    Vector2             m_pos;
    int16_t             m_rotation;
    bool                m_bHasMask;
    uint16_t            m_maskColor;
    const uint16_t*     m_palette;
    const uint8_t       m_bpp;
    Vector2             m_pivot;
    uint32_t            m_id;
private:
    void updateBoundingBox();
    bool                m_bRedraw;
    Region              m_boundingBox;
    static uint32_t     m_idCounter;
};
inline uint32_t Drawable::m_idCounter = 0;
inline void Drawable::update(const Vector2& pos,const Vector2& scale, int16_t rotation) {
    m_scale = scale;
    m_pos= pos;
    m_rotation = rotation % 360;
    if (m_rotation < 0)
        m_rotation = 360 + m_rotation;
    if (m_bRedraw) {
        updateBoundingBox();
    }
}
inline bool Drawable::readPixel(int16_t x, int16_t y, uint32_t* value) {
    if (x < 0 || x >= m_size.x || y < 0 || y >= m_size.y)
        return false;
    *value = readPixelUnsafe(x, y);
    return true;
}

inline uint32_t Drawable::readPixelUnsafe(uint16_t x, uint16_t y) {
    const void* data = getDrawData();
    uint32_t offset = y * (uint16_t)m_size.x + x;
    uint8_t subIndex = 0;
    if (m_palette) {
        uint8_t divider = 16 / m_bpp;
        subIndex = offset % divider;
        offset /= divider;
    }
    if (m_bpp == 16) {
        const uint16_t* address = (uint16_t*)data + offset;
        uint32_t value = *(address);
        if (m_palette) {
            value = m_palette[value >> (15-subIndex) & 0x1];
        }
        return value;
    } else if (m_bpp == 24) {
        const uint32_t* address = (uint32_t*)data + offset;
        uint32_t value = *(address);
        return value;
    } else {
        return 0;
    }
}
typedef SharedPtr<Drawable>       DrawablePtr;

inline uint16_t* generateSinglePalette(uint16_t color) {
    uint16_t maskColor = 0xFFFF - color;
    uint16_t* palette = new uint16_t[2];
    palette[0] = maskColor;
    palette[1] = color;
    return palette;
}
#define RotatePoint(x, y, sina, cosa) { \
    uint16_t x1 = (x * cosa - y * sina) >> FP_SCALE; \
    uint16_t y1 = (x * sina + y * cosa) >> FP_SCALE; \
    x = x1; \
    y = y1; } \

inline void Drawable::updateBoundingBox() {
    bool hasScale = abs(m_scale.x - 1) > 0.001 || abs(m_scale.y - 1) > 0.001;
    bool hasRot = m_rotation != 0;
    uint16_t w = m_size.x;
    uint16_t h = m_size.y;
    int16_t p1x = -w * m_pivot.x; //图片局部坐标系
    int16_t p1y = h * (1 - m_pivot.y);
    if (hasScale) {
        p1x *= abs(m_scale.x);
        p1y *= abs(m_scale.y);
        w = ceil(w * abs(m_scale.x));
        h = ceil(h * abs(m_scale.y));
    }
    int16_t ltx = p1x; int16_t lty = p1y;
    if (hasRot) {
        int16_t sina = getSinValue(m_rotation);
        int16_t cosa = getCosValue(m_rotation);
        int16_t p2x = p1x + w;
        int16_t p2y = p1y;
        int16_t p3x = p2x;
        int16_t p3y = p2y - h;
        int16_t p4x = p1x;
        int16_t p4y = p3y;
        RotatePoint(p1x, p1y, sina, cosa)
        RotatePoint(p2x, p2y, sina, cosa)
        RotatePoint(p3x, p3y, sina, cosa)
        RotatePoint(p4x, p4y, sina, cosa)
        // 计算AABB
        int16_t xmin = std::min(p1x, std::min(p2x, std::min(p3x, p4x)));
        int16_t xmax = std::max(p1x, std::max(p2x, std::max(p3x, p4x)));
        int16_t ymin = std::min(p1y, std::min(p2y, std::min(p3y, p4y)));
        int16_t ymax = std::max(p1y, std::max(p2y, std::max(p3y, p4y)));
        ltx = xmin;
        lty = ymax;
        w = xmax - xmin;
        h = ymax - ymin;
    }
    //x,y 为图片左上角坐标（屏幕坐标系）
    int16_t x = m_pos.x + ltx;
    int16_t y = m_pos.y + lty;
    m_boundingBox.x = x;
    m_boundingBox.y = y;
    m_boundingBox.w = w;
    m_boundingBox.h = h;
}

#endif