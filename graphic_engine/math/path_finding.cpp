#include "path_finding.h"
#include <cmath>
#include <algorithm>

// 计算两个坐标之间的曼哈顿距离
inline int heuristic(Coordinate a, Coordinate b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

// 定义路径节点
struct PathNode {
    Coordinate coord;
    int g_cost; // 从起点到该节点的实际路径长度
    int h_cost; // 该节点到终点的估计路径长度
    int f_cost; // g_cost + h_cost
    PathNode* parent;

    PathNode(Coordinate c, int g, int h, PathNode* p) :
        coord(c), g_cost(g), h_cost(h), f_cost(g + h), parent(p) {}
};


// 寻找最短路径
std::vector<Coordinate> findPath(Coordinate start, Coordinate end, std::vector<std::vector<CellType>>& grid) {
    std::vector<PathNode*> open_set;
    int width = grid.size();
    int height = grid[0].size(); 
    std::vector<std::vector<bool>> closed_set(width, std::vector<bool>(height, false));
    std::vector<PathNode*> pathNodes;
    PathNode* node = new PathNode(start, 0, heuristic(start, end), nullptr);
    open_set.push_back(node);
    pathNodes.push_back(node);

    while (!open_set.empty()) {
        PathNode* current = open_set[0];
        open_set.erase(open_set.begin());
        if (current->coord.x == end.x && current->coord.y == end.y) {
            // 找到终点，构建路径
            std::vector<Coordinate> path;
            while (current->parent) {
                path.push_back(current->coord);
                current = current->parent;
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            // 清理路径节点
            for (PathNode* node : pathNodes) {
                delete node;
            }
            return path;
        }
        if (closed_set[current->coord.x][current->coord.y]) {
            continue;
        }
        closed_set[current->coord.x][current->coord.y] = true;
        // 检查周围四个方向的邻居节点
        for (int16_t dx : {-1, 0, 1}) {
            for (int16_t dy : {-1, 0, 1}) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                if (dx != 0 && dy != 0) {
                    continue;
                }
                int16_t x = current->coord.x + dx;
                int16_t y = current->coord.y + dy;
                if (x < 0 || x >= width || y < 0 || y >= height) {
                    continue;
                }
                Coordinate neighbor = { (uint16_t)x, (uint16_t)y };
                if (grid[neighbor.x][neighbor.y] != CellType::PATH) {
                    continue;
                }
                int g_cost = current->g_cost + 1;
                int h_cost = heuristic(neighbor, end);
                node = new PathNode(neighbor, g_cost, h_cost, current);
                open_set.push_back(node);
                std::sort(open_set.begin(), open_set.end(), [](PathNode* a, PathNode* b) { return a->f_cost < b->f_cost; });
                pathNodes.push_back(node);
            }
        }
    }
    // 未找到路径
    // 清理路径节点
    for (PathNode* node : pathNodes) {
        delete node;
    }
    return {};
}
