#ifndef _MATRIX3_H_
#define _MATRIX3_H_
#include <string.h>
#include "vector3.h"

struct Matrix3
{
    float m[3][3];
    Matrix3() {
        unit();
    }
    Matrix3(const Matrix3& mat) {
        memcpy(m, mat.m, sizeof(float) * 9);
    }
    void unit() {
        m[0][0] = 1;m[0][1] = 0;m[0][2] = 0;
        m[1][0] = 0;m[1][1] = 1;m[1][2] = 0;
        m[2][0] = 0;m[2][1] = 0;m[2][2] = 1;
    }
    Matrix3 operator*(const Matrix3& mat) {
        Matrix3 retMat;
        for (size_t row = 0; row < 3; row++)
        {
            for (size_t col = 0; col < 3; col++)
            {
                retMat.m[row][col] =
                    m[row][0]*mat.m[0][col] +
                    m[row][1]*mat.m[1][col] +
                    m[row][2]*mat.m[2][col];
            }
        }
        return retMat;
    }
};

inline Vector3f operator*(const Vector3f& v, const Matrix3& mat)
{
    Vector3f vec;
    vec.x = v.x*mat.m[0][0] + v.y*mat.m[1][0] + v.z*mat.m[2][0];
    vec.y = v.x*mat.m[0][1] + v.y*mat.m[1][1] + v.z*mat.m[2][1];
    vec.z = v.x*mat.m[0][2] + v.y*mat.m[1][2] + v.z*mat.m[2][2];
    return vec;
}
#endif