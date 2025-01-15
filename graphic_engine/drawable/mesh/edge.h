#ifndef _EDGE_H_
#define _EDGE_H_

struct Edge {
    float x;    // x坐标初始值位yMin处的x坐标
    float dx;   // x坐标增量等同于 1 / slope
    float yMin; // y坐标最小值
    float yMax; // y坐标最大值
    float u;    // yMin处的纹理坐标u
    float v;    // yMin处的纹理坐标v
    float du;   // 纹理坐标u增量
    float dv;   // 纹理坐标v增量
    // 重载运算符以便在集合中使用
    bool operator<(const Edge& other) const {
        return std::tie(x, dx, yMax) < std::tie(other.x, other.dx, other.yMax);
    }
};

#endif