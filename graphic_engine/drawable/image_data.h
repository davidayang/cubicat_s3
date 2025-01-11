#ifndef _IMAGEDATA_H_
#define _IMAGEDATA_H_
#include <cstdint>
struct ImageData
{
    uint16_t width;
    uint16_t height;
    char col;
    char row;
    bool hasMask;
    bool hasAlpha;
    uint16_t maskColor;
    const void* data;
    const uint16_t* palette;
    uint8_t bpp;
};
#endif