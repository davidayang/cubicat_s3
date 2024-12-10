#ifndef _MOUNTING_POINT_H_
#define _MOUNTING_POINT_H_
#include <stdint.h>

// 序列帧动画挂接点
struct MountingPoint
{
    uint16_t            len;
    // 挂接点数据[x,y,z]为一个单元，x：x坐标，y： y坐标，z： 旋转角度
    const int16_t*      data;
};

#endif