#ifndef _REGION_H_
#define _REGION_H_
#include <cstdint>
#include <algorithm>

struct Region {
    int16_t     x; // left top corner x
    int16_t     y; // left top corner y
    uint16_t    w;
    uint16_t    h;
    Region():x(0),y(0),w(0),h(0) {}
    Region(int16_t x, int16_t y, uint16_t w, uint16_t h):x(x),y(y),w(w),h(h) {}
    bool valid() const { return w > 0 && h > 0; }
    void zero() {
        x = 0;
        y = 0;
        w = 0;
        h = 0;
    }
    const Region& combine(const Region& other) {
        if (other.w == 0 || other.h == 0)
            return *this;
        if (w == 0 && h == 0) {
            x = other.x;
            y = other.y;
            w = other.w;
            h = other.h;
        } else {
            int16_t endx = x + w;
            int16_t endy = y + h;
            int16_t otherEndx = other.x + other.w;
            int16_t otherEndy = other.y + other.h;
            x = std::min(x, other.x);
            y = std::min(y, other.y);
            w = std::max(endx, otherEndx) - x;
            h = std::max(endy, otherEndy) - y;
        }
        return *this;
    }
    bool operator==(const Region& other) const {
        return x == other.x && y == other.y && w == other.w && h == other.h;
    }
};
#endif