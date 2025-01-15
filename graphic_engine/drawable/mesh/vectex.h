#ifndef _VECTEX_H_
#define _VECTEX_H_

namespace cubicat{

struct Vertex
{
    Vertex() : x(0), y(0), u(0), v(0) {}
    float x;
    float y;
    float u;
    float v;
    Vertex& operator=(const Vertex& other) {
        x = other.x;
        y = other.y;
        u = other.u;
        v = other.v;
        return *this;
    }
};

}

#endif