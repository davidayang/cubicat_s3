/*
* @author       Isaac
* @date         2024-12-07
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
*/
#ifndef _VECTOR3_H_
#define _VECTOR3_H_
#include <cmath>
#include <stdint.h>

namespace cubicat {

template <class T>
struct Vector3
{
    union {
        struct {
            T x, y, z;
        };
        T v[3];
    };
    Vector3():x(0),y(0),z(0) {}
    Vector3(T x, T y, T z)
    : x(x),y(y),z(z) 
    {
    }
    inline Vector3 operator*(const Vector3& vec) const noexcept {
        return Vector3(x * vec.x , y * vec.y , z * vec.z);
    }
    inline Vector3 operator/(const Vector3& vec) const noexcept {
        return Vector3(x / vec.x , y / vec.y , z / vec.z);
    }
    inline Vector3 operator*(float scalar) const noexcept {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    inline Vector3 operator+(const Vector3& vec) const noexcept {
        return Vector3(x + vec.x, y + vec.y, z + vec.z);
    }   
    inline void operator+=(const Vector3& vec) noexcept {
        x += vec.x;
        y += vec.y;
        z += vec.z;
    }
    inline Vector3 operator-(const Vector3& vec) const noexcept {
        return Vector3(x - vec.x, y - vec.y, z - vec.z);
    }
    inline Vector3 operator-() const noexcept {
        return Vector3(-x, -y, -z);
    }
    inline float dot(const Vector3& vec) const noexcept {
        return x * vec.x + y * vec.y + z * vec.z;
    }
    inline Vector3 cross(const Vector3& vec) const noexcept {
        return Vector3(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x);
    }
    inline float length() const noexcept {
        return sqrt(x * x + y * y + z * z);
    }
    inline Vector3& normalize(float* lenOut = nullptr) noexcept {
        float l = length();
        if (l > 0) {
            x /= l;
            y /= l;
            z /= l;
        }
        if (lenOut) {
            *lenOut = l;
        }
        return *this;
    }
    inline bool isUniform() const noexcept {
        return std::fabs(x - y) < std::numeric_limits<T>::epsilon() && std::fabs(y - z) < std::numeric_limits<T>::epsilon() && std::fabs(x - 1.0f) < std::numeric_limits<T>::epsilon();
    }
    inline bool operator<(const Vector3& vec) const noexcept {
        return x < vec.x && y < vec.y && z < vec.z;
    }
    inline static const Vector3& zero() { static const Vector3 Zero(0, 0, 0); return Zero; }
    inline static const Vector3& one() { static const Vector3 One(1, 1, 1); return One; }
    inline static const Vector3& right() { static const Vector3 Right(1, 0, 0); return Right; }
    inline static const Vector3& up() { static const Vector3 Up(0, 1, 0); return Up; }
    inline static const Vector3& forward() { static const Vector3 Forward(0, 0, 1); return Forward; }
};
template<class T>
inline Vector3<T> lerp(const Vector3<T>& vec0, const Vector3<T>& vec1, float p) {
    return {
        vec0.x + (vec1.x - vec0.x) * p,
        vec0.y + (vec1.y - vec0.y) * p,
        vec0.z + (vec1.z - vec0.z) * p};
} 
template<>
inline Vector3<uint8_t> lerp(const Vector3<uint8_t>& vec0, const Vector3<uint8_t>& vec1, float p) {
    return {
        static_cast<uint8_t>(vec0.x + (vec1.x - vec0.x) * p),
        static_cast<uint8_t>(vec0.y + (vec1.y - vec0.y) * p),
        static_cast<uint8_t>(vec0.z + (vec1.z - vec0.z) * p)
    };
}

typedef Vector3<int16_t>    Vector3h;
typedef Vector3<int32_t>    Vector3i;
typedef Vector3<uint32_t>    Vector3ui;
typedef Vector3<uint16_t>    Vector3us;
typedef Vector3<uint8_t>    Vector3uc;
typedef Vector3<float>      Vector3f;

} // namespace cubicat
#endif