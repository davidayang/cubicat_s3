#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <cmath>
#include "common.h"

namespace cubicat {

template <class T>
struct Vector2
{
    T       x, y;
    Vector2(): x(0),y(0) {}
    Vector2(const Vector2& vec): x(vec.x), y(vec.y) {}
    Vector2(T x, T y): x(x), y(y) {}
    
    inline void operator=(const Vector2& vec) noexcept {
        x = vec.x;
        y = vec.y;
    }
    inline Vector2 operator*(const Vector2& vec) const noexcept {
        return Vector2(x * vec.x , y * vec.y);
    }
    inline void operator*=(const Vector2& vec) noexcept {
        x *= vec.x;
        y *= vec.y;
    }

    inline Vector2 operator/(const Vector2& vec) const noexcept {
        return Vector2(x / vec.x, y / vec.y);
    }
    inline Vector2 operator/(float value) const noexcept {
        return Vector2(x / value, y / value);
    }
    inline void operator/=(float value) noexcept {
        if (value == 0) return;
        x /= value;
        y /= value;
    }
    inline Vector2 operator*(float scalar) const noexcept {
        return Vector2(x * scalar, y * scalar);
    }
    inline Vector2 operator+(const Vector2& vec) const noexcept {
        return Vector2(x + vec.x, y + vec.y);
    }
    inline void operator+=(const Vector2& vec) noexcept {
        x += vec.x;
        y += vec.y;
    }
    inline Vector2 operator-() const noexcept {
        return Vector2(-x, -y);
    }
    inline Vector2 operator-(const Vector2& vec) const noexcept {
        return Vector2(x - vec.x, y - vec.y);
    }
    inline bool operator==(const Vector2& vec) const noexcept {
        return abs(x - vec.x) <= std::numeric_limits<float>::epsilon() && abs(y - vec.y) <= std::numeric_limits<float>::epsilon();
    }
    inline T dot(const Vector2& vec) noexcept {
        return x * vec.x + y * vec.y;
    }
    inline T cross(const Vector2& vec) noexcept {
        return  x * vec.y - y * vec.x;
    }
    inline float length() noexcept {
        return sqrt(x * x + y * y);
    }
    inline float lengthSqr() noexcept {
        return x * x + y * y;
    }
    inline const Vector2 normalize(float* len = nullptr) noexcept {
        float l = sqrt(x * x + y * y);
        if (len) *len = l;
        if (l > 0) {
            x /= l;
            y /= l;
        }
        return *this;
    }
    void rotate(float angle) noexcept {
        int scaler = 1 << FP_SCALE_SHIFT;
        int16_t cosa = cos(ANGLE_2_RAD(angle)) * scaler;
        int16_t sina = sin(ANGLE_2_RAD(angle)) * scaler;
        float _x = ((int16_t)x * cosa - (int16_t)y * sina) >> FP_SCALE_SHIFT;
        float _y = ((int16_t)x * sina + (int16_t)y * cosa) >> FP_SCALE_SHIFT;
        x = _x;
        y = _y;
    }
};

template <class  T>
inline Vector2<T> lerp(const Vector2<T>& start, const Vector2<T>& end, float t) noexcept {
    if (t > 1) t = 1;
    if (t < 0) t = 0;
    float lerpedX = start.x + (end.x - start.x) * t;
    float lerpedY = start.y + (end.y - start.y) * t;
    return Vector2(lerpedX, lerpedY);
}

typedef Vector2<int16_t>    Vector2h;
typedef Vector2<int32_t>    Vector2i;
typedef Vector2<uint32_t>   Vector2ui;
typedef Vector2<uint16_t>   Vector2us;
typedef Vector2<float>      Vector2f;

} // namespace cubicat
#endif