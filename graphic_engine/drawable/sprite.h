#ifndef _SPRITE_H_
#define _SPRITE_H_
#include "drawable.h"
class Sprite : public Drawable
{
public:
    DECLARE_RTTI_SUB(Sprite, Drawable);
    static SharedPtr<Sprite> create(uint16_t width, uint16_t height, uint16_t* data, bool hasMask, uint16_t maskColor = 0,
    const uint16_t *palette = nullptr, uint8_t bpp = 16) {
        return SharedPtr<Sprite>(NEW Sprite(width, height, data, hasMask, maskColor, palette, bpp));
    }
    ~Sprite();

    const uint16_t* getDrawData();
private:
    Sprite(uint16_t width, uint16_t height, uint16_t* data, bool hasMask, uint16_t maskColor = 0, 
    const uint16_t *palette = nullptr, uint8_t bpp = 16);
    uint16_t*     m_Data;
};
typedef SharedPtr<Sprite>       SpritePtr;
inline const uint16_t* Sprite::getDrawData() {
    return m_Data;
}
#endif