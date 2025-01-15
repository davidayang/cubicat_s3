#include "polygon.h"
#include <set>
#include "esp_heap_caps.h"
#include "utils/logger.h"
#include "utils/helper.h"

#define CREATE_EDGE(p0, p1) \
    Edge edge; \
    Vertex& vLow = p0.y < p1.y ? p0 : p1; \
    Vertex& vHigh = p0.y < p1.y ? p1 : p0; \
    edge.x = vLow.x; \
    edge.yMin = vLow.y; \
    edge.yMax = vHigh.y; \
    edge.dx = (vLow.x - vHigh.x) / (vLow.y - vHigh.y); \
    edge.du = (vLow.u - vHigh.u) / (vLow.y - vHigh.y); \
    edge.dv = (vLow.v - vHigh.v) / (vLow.y - vHigh.y); \
    edge.u = vLow.u; \
    edge.v = vLow.v; \
    m_vET.push_back(edge);

Polygon::Polygon(float width, float height, void* texData, bool hasMask, uint16_t maskColor)
 : Drawable(Vector2(width, height), hasMask, maskColor) {
    Vertex* vertices = new Vertex[4]; 
    vertices[0].x = 0;
    vertices[0].y = height;
    vertices[0].u = 0;
    vertices[0].v = 0;

    vertices[1].x = width;
    vertices[1].y = height;
    vertices[1].u = 1;
    vertices[1].v = 0;

    vertices[2].x = width;
    vertices[2].y = 0;
    vertices[2].u = 1;
    vertices[2].v = 1;

    vertices[3].x = 0;
    vertices[3].y = 0;
    vertices[3].u = 0;
    vertices[3].v = 1;

    uint16_t* indices = new uint16_t[6];
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 3;
    indices[5] = 0; 
    m_pMesh = Mesh::create(vertices, 4, indices, 6);
}
Polygon::~Polygon() {
    
}
void Polygon::caculateEdges() {
    m_vET.clear();
    Vertex* worldVertices = (Vertex*)heap_caps_malloc(m_pMesh->getVertexCount() * sizeof(Vertex), MALLOC_CAP_SPIRAM);
    // transform vertices to world pos
    auto& scale = getScale();
    auto angle = getAngle();
    int16_t sina = getSinValue(angle);
    int16_t cosa = getCosValue(angle);
    int32_t minX = 99999;
    int32_t minY = 99999;
    int32_t maxX = -99999;
    int32_t maxY = -99999;
    for (int i=0; i<m_pMesh->getVertexCount(); i++) {
        auto v = m_pMesh->getVertex(i);
        Vertex& vertex = worldVertices[i];
        vertex = *v;
        vertex.x -= m_size.x * m_pivot.x;
        vertex.y -= m_size.y * m_pivot.y;
        // scale
        if (scale.x != 1.0f || scale.y != 1.0f) {
            vertex.x *= scale.x;
            vertex.y *= scale.y;   
        }
        // rotate 
        if (angle != 0) {
            RotatePointFloat(vertex.x, vertex.y, sina, cosa);
        }
        vertex.x += m_pos.x;
        vertex.y += m_pos.y;
        if (vertex.x < minX) minX = vertex.x;
        if (vertex.y < minY) minY = vertex.y;
        if (vertex.x > maxX) maxX = ceil(vertex.x);
        if (vertex.y > maxY) maxY = ceil(vertex.y);
    }
    m_boundingBox.x = minX;
    m_boundingBox.y = maxY;
    m_boundingBox.w = maxX - minX;
    m_boundingBox.h = maxY - minY;
    int faceCount = m_pMesh->getIndexCount() / 3;
    for (int i = 0; i < faceCount; i++) {
        auto& p0 = worldVertices[m_pMesh->getIndex(i*3)];
        auto& p1 = worldVertices[m_pMesh->getIndex(i*3 + 1)];
        auto& p2 = worldVertices[m_pMesh->getIndex(i*3 + 2)];
        if (p0.y != p1.y) { // ignore parallel edges
            CREATE_EDGE(p0, p1);
        }
        if (p1.y != p2.y) { // ignore parallel edges
            CREATE_EDGE(p1, p2);
        }
        if (p2.y != p0.y) { // ignore parallel edges
            CREATE_EDGE(p2, p0);
        }
    }
    std::sort(m_vET.begin(), m_vET.end(), [](const Edge& e0, const Edge& e1) {
        return e0.yMin < e1.yMin;
    });
    free(worldVertices);
}
void Polygon::updateBoundingBox() {
    caculateEdges();
}
std::vector<IntersectionPoint> Polygon::scanline(uint16_t y) {
    std::vector<IntersectionPoint> points;
    // todo need optimize
    m_vAET.clear();
    for (auto& e : m_vET) {
        if (e.yMin <= y && y <= e.yMax) {
            m_vAET.push_back(&e);
        }
    }
    for (Edge* e : m_vAET) {
        auto x = e->x + e->dx * (y - e->yMin);
        auto u = e->u + e->du * (y - e->yMin);
        auto v = e->v + e->dv * (y - e->yMin);
        points.push_back({ x, y, u, v });
    }
    // sort intersection points by x
    std::sort(points.begin(), points.end(), [](const IntersectionPoint& v0, const IntersectionPoint& v1) {
        return v0.x < v1.x;
    });
    if (points.size() % 2 != 0) {
        LOGW("IntersectionPoint count:%d at y: %d\n",points.size(), y);
        points.pop_back();
    }
    return points;
}
