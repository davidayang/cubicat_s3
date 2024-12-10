#ifndef _MATRIX3_H_
#define _MATRIX3_H_
#include <string.h>
#include "vector2.h"

struct Matrix2
{
    float m[2][2];
    Matrix2() {
        unit();
    }
    Matrix2(const Matrix2& mat) {
        memcpy(m, mat.m, sizeof(float) * 9);
    }
    void unit() {
        m[0][0] = 1;m[0][1] = 0;
        m[1][0] = 0;m[1][1] = 1;
    }
    Matrix2 operator*(const Matrix2& mat) {
        Matrix2 retMat;
        for (size_t row = 0; row < 2; row++)
        {
            for (size_t col = 0; col < 2; col++)
            {
                retMat.m[row][col] =
                    m[row][0]*mat.m[0][col] +
                    m[row][1]*mat.m[1][col];
            }
        }
        return retMat;
    }
};

inline Vector2 operator*(const Vector2& v, const Matrix2& mat)
{
    Vector2 vec;
    vec.x = v.x*mat.m[0][0] + v.y*mat.m[1][0];
    vec.y = v.x*mat.m[0][1] + v.y*mat.m[1][1];
    return vec;
}
#endif