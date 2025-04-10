#include "polygon2d.h"
#include <set>
#include "esp_heap_caps.h"
#include "utils/logger.h"
#include "utils/helper.h"

Vertex2* g_vWorld2DVertexCache = nullptr;
uint16_t g_nWorld2DVertexCacheSize = 0;

#define CREATE_EDGE(p0, p1) \
{ \
    Edge& edge = m_vET[m_nETSize++]; \
    if (p0.y != p1.y) { \
        const Vertex2& vLow = p0.y < p1.y ? p0 : p1; \
        const Vertex2& vHigh = p0.y < p1.y ? p1 : p0; \
        edge.x = vLow.x; \
        edge.yMin = vLow.y; \
        edge.yMax = vHigh.y; \
        edge.dx = (vLow.x - vHigh.x) / (vLow.y - vHigh.y); \
        edge.du = (vLow.u - vHigh.u) / (vLow.y - vHigh.y); \
        edge.dv = (vLow.v - vHigh.v) / (vLow.y - vHigh.y); \
        edge.u = vLow.u; \
        edge.v = vLow.v; \
        edge.w = 1; \
        edge.dw = 0; \
    } else { \
        edge.invalidate(); \
    } \
}


Polygon2D::Polygon2D(Mesh2DPtr mesh)
 : m_pMesh(mesh) {
}
Polygon2D::~Polygon2D() {
    if (m_vET) {
        free(m_vET);
    }
}

void Polygon2D::caculateEdges() {
    if (!m_pMesh)
        return;
    int vCount = m_pMesh->getVertexCount();
    if (vCount == 0)
        return;
    m_nETSize = 0;
    if (vCount > g_nWorld2DVertexCacheSize) {
        if (g_vWorld2DVertexCache) {
            heap_caps_free(g_vWorld2DVertexCache);
        }
        g_vWorld2DVertexCache = (Vertex2*)psram_prefered_malloc(vCount * sizeof(Vertex2));
        g_nWorld2DVertexCacheSize = vCount;
    }
    int faceCount = m_pMesh->getFaceCount();
    int maxEdgeCount = faceCount * 3;
    if (maxEdgeCount > m_nETBufferSize) {
        if (m_vET) {
            heap_caps_free(m_vET);
        }
        m_vET = (Edge*)psram_prefered_malloc(maxEdgeCount * sizeof(Edge));
        m_nETBufferSize = maxEdgeCount;
    }
    // transform vertices to world pos
    auto scale = getScale();
    auto angle = getAngle();
    int16_t sina = getSinValue(angle);
    int16_t cosa = getCosValue(angle);
    int32_t minX = 99999;
    int32_t minY = 99999;
    int32_t maxX = -99999;
    int32_t maxY = -99999;
    auto pos = getPosition();
    for (int i=0; i<vCount; i++) {
        auto v = m_pMesh->getVertex(i);
        Vertex2& vertex = g_vWorld2DVertexCache[i];
        vertex = *v;
        // scale
        if (scale.x != 1.0f || scale.y != 1.0f) {
            vertex.x *= scale.x;
            vertex.y *= scale.y;
        }
        // rotate 
        if (angle % 360 != 0) {
            RotatePointFloat(vertex.x, vertex.y, sina, cosa);
        }
        vertex.x += pos.x;
        vertex.y += pos.y;
        if (vertex.x < minX) minX = vertex.x;
        if (vertex.y < minY) minY = vertex.y;
        if (vertex.x > maxX) maxX = ceil(vertex.x);
        if (vertex.y > maxY) maxY = ceil(vertex.y);
    }
    m_region.x = minX;
    m_region.y = maxY;
    m_region.w = maxX - minX;
    m_region.h = maxY - minY;
    // too small for render
    if (m_region.w <= 1 || m_region.h <= 1) {
        return;
    }
    uint16_t idx0, idx1, idx2;
    for (int i = 0; i < faceCount; i++) {
        m_pMesh->getFaceVerticesIndex(i, idx0, idx1, idx2);
        const auto& p0 = g_vWorld2DVertexCache[idx0];
        const auto& p1 = g_vWorld2DVertexCache[idx1];
        const auto& p2 = g_vWorld2DVertexCache[idx2];
        CREATE_EDGE(p0, p1);
        CREATE_EDGE(p1, p2);
        CREATE_EDGE(p2, p0);
    }
}
void Polygon2D::updateRegion() {
    caculateEdges();
}
