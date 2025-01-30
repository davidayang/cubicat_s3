#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <cmath>
#include "../definitions.h"


// Todo: need to optimize
struct Vector2
{
    float       x;
    float       y;
    Vector2();
    Vector2(const Vector2& vec);
    Vector2(float x, float y);
    
    Vector2 operator*(const Vector2& vec) const;
    Vector2 operator/(const Vector2& vec) const;
    Vector2 operator/(float value) const;
    void operator/=(float value);
    Vector2 operator*(float scalar) const;
    Vector2 operator+(const Vector2& vec) const;
    void operator+=(const Vector2& vec);
    Vector2 operator-() const;
    Vector2 operator-(const Vector2& vec) const;
    bool operator==(const Vector2& vec) const;
    float dotProduct(const Vector2& vec);
    float crossProduct(const Vector2& vec);
    float length();
    float lengthSqr();
    float normalize();
    void rotate(float angle);
    static Vector2 Zero;
};
inline Vector2 Vector2::Zero;
inline Vector2::Vector2():x(0),y(0) {
}
inline Vector2::Vector2(const Vector2& vec):x(vec.x),y(vec.y) {
}
inline Vector2::Vector2(float x, float y)
:x(x),y(y)
{
}

inline Vector2 Vector2::operator*(const Vector2& vec) const {
    return Vector2(x * vec.x , y * vec.y);
}
inline Vector2 Vector2::operator/(const Vector2& vec) const {
    return Vector2(x / vec.x , y / vec.y);
}
inline Vector2 Vector2::operator/(float value) const {
    return Vector2(x / value, y / value);
}
inline void Vector2::operator/=(float value) {
    x /= value;
    y /= value;
}

inline Vector2 Vector2::operator*(float scalar) const {
    return Vector2(x * scalar, y * scalar);
}

inline float Vector2::dotProduct(const Vector2& vec) {
    return x * vec.x + y * vec.y;
}
inline float Vector2::crossProduct(const Vector2& vec) {
    return  x * vec.y - y * vec.x;
}
inline Vector2 Vector2::operator+(const Vector2& vec) const {
    return Vector2(x + vec.x, y + vec.y);
}
inline void Vector2::operator+=(const Vector2& vec) {
    x += vec.x;
    y += vec.y;
}
inline Vector2 Vector2::operator-() const {
    return Vector2(-x, -y);
}
inline Vector2 Vector2::operator-(const Vector2& vec) const {
    return Vector2(x - vec.x, y - vec.y);
}
inline bool Vector2::operator==(const Vector2& vec) const {
    return abs(x - vec.x) <= std::numeric_limits<float>::epsilon() && abs(y - vec.y) <= std::numeric_limits<float>::epsilon();
}
inline Vector2 lerp(const Vector2& start, const Vector2& end, float t) {
    if (t > 1) t = 1;
    if (t < 0) t = 0;
    float lerpedX = start.x + (end.x - start.x) * t;
    float lerpedY = start.y + (end.y - start.y) * t;
    return Vector2(lerpedX, lerpedY);
}
inline float Vector2::length() {
    return sqrt(x * x + y * y);
}
inline float Vector2::lengthSqr() {
    return x * x + y * y;
}
inline float Vector2::normalize() {
    float l = sqrt(x * x + y * y);
    if (l > 0) {
        x /= l;
        y /= l;
    }
    return l;
}
inline void Vector2::rotate(float angle) {
    int scaler = 1 << FP_SCALE_SHIFT;
    int16_t cosa = cos(angle * ANGLE_2_RAD) * scaler;
    int16_t sina = sin(angle * ANGLE_2_RAD) * scaler;
    float _x = ((int16_t)x * cosa - (int16_t)y * sina) >> FP_SCALE_SHIFT;
    float _y = ((int16_t)x * sina + (int16_t)y * cosa) >> FP_SCALE_SHIFT;
    x = _x;
    y = _y;
}
#endif