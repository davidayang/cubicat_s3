#ifndef _QUAD_H_
#define _QUAD_H_
#include "polygon2d.h"

class Quad : public Polygon2D {
public:
    DECLARE_RTTI_SUB(Quad, Polygon2D)
    static SharedPtr<Quad> create(uint16_t width, uint16_t height) {
        return SharedPtr<Quad>(new Quad(width, height));
    }
    void setPivot(float x, float y);
private:
    Quad(uint16_t width, uint16_t height);
    uint16_t        m_width;
    uint16_t        m_height;
};
#endif