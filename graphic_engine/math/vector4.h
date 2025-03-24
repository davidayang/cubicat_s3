/*
* @author       Isaac
* @date         2025-01-07
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
*/
#ifndef _VECTOR4_H_
#define _VECTOR4_H_
#include "vector3.h"

namespace cubicat {

template <class T>
struct Vector4
{
    union {
        struct {
            T x, y, z, w;
        };
        T v[4];
    };
    Vector4():x(0),y(0),z(0),w(0) {}
    Vector4(T x, T y, T z, T w)
    : x(x),y(y),z(z),w(w)
    {
    }
    Vector4(const Vector3<T>& vec, float w) {
        x = vec.x;
        y = vec.y;
        z = vec.z;
        this->w = w;
    }

    inline Vector4 operator*(const Vector4& vec) const noexcept {
        return Vector4<T>(x * vec.x , y * vec.y , z * vec.z, w * vec.w);
    }
    inline Vector4 operator*(float scalar) const noexcept {
        return Vector4<T>(x * scalar, y * scalar, z * scalar, w * scalar);
    }
    inline Vector4 operator+(const Vector4& vec) const noexcept {
        return Vector4(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
    }
    inline Vector4 operator-(const Vector4& vec) const noexcept {
        return Vector4(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
    }
    inline Vector3<T> toVector3() const noexcept { return Vector3<T>(x, y, z); }
};
typedef Vector4<float>      Vector4f;

} // namespace cubicat
#endif