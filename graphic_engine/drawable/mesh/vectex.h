#ifndef _VECTEX_H_
#define _VECTEX_H_
#include "../../math/vector3.h"
#include "../../math/vector2.h"
namespace cubicat{

struct Vertex2
{
    Vertex2() : x(0), y(0), u(0), v(0) {}
    union {
        struct {
            float x,y;
        };
        Vector2f pos;
    };
    union {
        struct {
            float u,v;
        };
        Vector2f uv;
    };
    Vertex2& operator=(const Vertex2& other) {
        x = other.x;
        y = other.y;
        u = other.u;
        v = other.v;
        return *this;
    }
};

struct Vertex3
{
    Vertex3() : x(0), y(0), z(0), u(0), v(0), nx(0), ny(0), nz(0) {}
    Vertex3(float x, float y, float z, float u, float v, float normalx, float normaly, float normalz) : x(x), y(y), z(z), u(u), v(v), nx(normalx), ny(normaly), nz(normalz) {}
    union {
        struct {
            float x, y, z;
        };
        Vector3f pos;
    };
    union {
        struct {
            float u,v;
        };
        Vector2f uv;
    };
    union {
        struct {
            float nx, ny, nz;
        };
        Vector3f nor;
    };
    Vertex3& operator=(const Vertex3& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        u = other.u;
        v = other.v;
        nx = other.nx;
        ny = other.ny;
        nz = other.nz;
        return *this;
    }
};

}

#endif