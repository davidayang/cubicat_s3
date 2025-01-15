#include "sprite.h"

Sprite::Sprite(uint16_t width, uint16_t height, const void* data, bool hasMask, uint16_t maskColor, const uint16_t *palette, uint8_t bpp)
: Drawable(Vector2(width, height), hasMask, maskColor, palette, bpp), m_Data(data)
{
}

Sprite::~Sprite()
{
}