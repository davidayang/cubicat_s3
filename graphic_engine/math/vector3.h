#ifndef _VECTOR3_H_
#define _VECTOR3_H_

template <class T>
struct Vector3
{
    Vector3():x(0),y(0),z(0) {}
    Vector3(T x, T y, T z)
    : x(x),y(y),z(z) 
    {
    }
    T       x;
    T       y;
    T       z;
};

typedef Vector3<int16_t>    Vector3h;
typedef Vector3<int32_t>    Vector3i;
typedef Vector3<uint32_t>    Vector3ui;
typedef Vector3<uint16_t>    Vector3uh;
typedef Vector3<float>      Vector3f;

#endif