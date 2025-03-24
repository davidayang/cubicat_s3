// #include "rectangle.h"
// #include <string.h>

// // Rectangle 每像素占1bit，使用2色调色盘，0 颜色为 maskColor，1 颜色为 color, mask color为color的反色。像素地址需要除以 16
// const uint8_t divider = 16;

// Rectangle::Rectangle(uint32_t width, uint32_t height, uint16_t color, bool fill, uint32_t cornerRadius, uint32_t lineWidth)
// : Drawable(Vector2(width, height), cornerRadius > 0 || !fill, 0xFFFF - color, generateSinglePalette(color)) {
//     setPivot(0, 0);
//     // todo 暂时不考虑lineWidth 大于 cornerRadius的情况
//     if (lineWidth > cornerRadius) {
//         lineWidth = cornerRadius;
//     }
//     uint32_t size = ceil(width * height / 16.0f);
//     m_pData = new uint16_t[size];
//     memset(m_pData, 0, size * sizeof(uint16_t));
//     auto minValue = min(width, height);
//     if (cornerRadius > (minValue >> 1)) {
//         cornerRadius = (minValue >> 1);
//     }
//     for (uint16_t y= 0; y < height; ++y) {
//         for (uint16_t x = 0; x < width; ++x) {
//             // 四个圆角的区域
//             bool leftTop = x < cornerRadius && y < cornerRadius;
//             bool rightTop = x >= width - cornerRadius && y < cornerRadius;
//             bool leftBottom = x < cornerRadius && y >= height - cornerRadius;
//             bool rightBottom = x >= width - cornerRadius && y >= height - cornerRadius;
//             int16_t _x , _y;
//             if (leftTop) {
//                 _x = x - cornerRadius;
//                 _y = y - cornerRadius;
//             } else if (rightTop) {
//                 _x = x - width + cornerRadius;
//                 _y = y - cornerRadius;
//             } else if (leftBottom) {
//                 _x = x - cornerRadius;
//                 _y = y - height + cornerRadius;
//             } else if (rightBottom) {
//                 _x = x - width + cornerRadius;
//                 _y = y - height + cornerRadius;
//             }
//             if (leftTop || rightTop || leftBottom || rightBottom) {
//                 auto distSqr = _x * _x + _y * _y;
//                 if (distSqr <= cornerRadius * cornerRadius) {
//                     if (fill || distSqr >= (cornerRadius - lineWidth) * (cornerRadius - lineWidth))
//                         setPixel(x, y);
//                 }
//             } else { // 中间区域
//                 if (fill || x < lineWidth || x >= width - lineWidth || y < lineWidth || y >= height - lineWidth)
//                     setPixel(x, y);
//             }
//         }
//     }
// }
// Rectangle::~Rectangle() {
//     delete[] m_pData;
//     delete[] m_palette;
// }
// void Rectangle::setPixel(uint16_t x, uint16_t y) {
//     assert (x >= 0 && x < (int)m_size.x && y >= 0 && y < (int)m_size.y);
//     uint32_t offset = y * (uint16_t)m_size.x + x;
//     uint8_t subIndex = offset % divider;
//     offset /= divider;
//     m_pData[offset] |= 0x1 << (15-subIndex);
// }
// void Rectangle::setColor(uint16_t color) {
//     if (m_palette)
//         delete[] m_palette;
//     m_palette = generateSinglePalette(color);
//     setMaskColor(m_palette[0]);
// }

