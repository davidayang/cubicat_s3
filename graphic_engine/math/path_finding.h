#ifndef _PATH_FINDING_H_
#define _PATH_FINDING_H_
#include <vector>
#include <stdint.h>
// 定义网格单元格的状态
enum class CellType { PATH, OBSTACLE };

// 定义网格单元格的坐标
struct Coordinate {
    uint16_t x, y;
    bool operator==(const Coordinate& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Coordinate& other) const {
        return x != other.x || y != other.y;
    }
};

extern std::vector<Coordinate> findPath(Coordinate start, Coordinate end, std::vector<std::vector<CellType>>& grid);
#endif