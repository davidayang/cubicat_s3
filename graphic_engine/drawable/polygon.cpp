#include "polygon.h"
#include <set>
#include "esp_heap_caps.h"
#include "utils/logger.h"
#include "utils/helper.h"

Vertex* g_pWorldVertexCache = nullptr;
uint16_t g_iWorldVertexCacheSize = 0;

#define CREATE_EDGE(p0, p1) \
    Edge& edge = m_vET[m_nETSize++]; \
    Vertex& vLow = p0.y < p1.y ? p0 : p1; \
    Vertex& vHigh = p0.y < p1.y ? p1 : p0; \
    edge.x = vLow.x; \
    edge.yMin = vLow.y; \
    edge.yMax = vHigh.y; \
    edge.dx = (vLow.x - vHigh.x) / (vLow.y - vHigh.y); \
    edge.du = (vLow.u - vHigh.u) / (vLow.y - vHigh.y); \
    edge.dv = (vLow.v - vHigh.v) / (vLow.y - vHigh.y); \
    edge.u = vLow.u; \
    edge.v = vLow.v;

IntersectionPoint* Polygon::s_vIntersectionPoints = nullptr;

Polygon::Polygon(float width, float height, bool hasMask, uint16_t maskColor)
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
    if (m_vET) {
        free(m_vET);
    }
    if (m_vAET) {
        free(m_vAET);
    }
}
const void* Polygon::getTextureData() {
    if (!m_pTexture) {
        return nullptr;
    }
    return m_pTexture->getTextureData();
}

void Polygon::caculateEdges() {
    m_nETSize = 0;
    int vCount = m_pMesh->getVertexCount();
    if (vCount > g_iWorldVertexCacheSize) {
        if (g_pWorldVertexCache) {
            heap_caps_free(g_pWorldVertexCache);
        }
        g_pWorldVertexCache = (Vertex*)psram_prefered_malloc(vCount * sizeof(Vertex));
        g_iWorldVertexCacheSize = vCount;
    }
    // transform vertices to world pos
    auto& scale = getScale();
    auto angle = getAngle();
    int16_t sina = getSinValue(angle);
    int16_t cosa = getCosValue(angle);
    int32_t minX = 99999;
    int32_t minY = 99999;
    int32_t maxX = -99999;
    int32_t maxY = -99999;
    for (int i=0; i<vCount; i++) {
        auto v = m_pMesh->getVertex(i);
        Vertex& vertex = g_pWorldVertexCache[i];
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
    // too small for render
    if (m_boundingBox.w <= 1 || m_boundingBox.h <= 1) {
        return;
    }
    int faceCount = m_pMesh->getIndexCount() / 3;
    if (!m_vET) {
        m_vET = (Edge*)psram_prefered_malloc(faceCount * 3 * sizeof(Edge));
    }
    for (int i = 0; i < faceCount; i++) {
        auto& p0 = g_pWorldVertexCache[m_pMesh->getVertexIndexByIndex(i*3)];
        auto& p1 = g_pWorldVertexCache[m_pMesh->getVertexIndexByIndex(i*3 + 1)];
        auto& p2 = g_pWorldVertexCache[m_pMesh->getVertexIndexByIndex(i*3 + 2)];
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
    // create aet buffer with size m_nETSize
    if (!m_vAET) {
        m_vAET = (Edge**)psram_prefered_malloc(m_nETSize * sizeof(Edge*));
    }
    // create intersection point buffer with size m_nETSize
    if (!s_vIntersectionPoints) {
        s_vIntersectionPoints = (IntersectionPoint*)psram_prefered_malloc(m_nETSize * sizeof(IntersectionPoint));
    }
}
void Polygon::updateBoundingBox() {
    caculateEdges();
}

IntersectionPointVec Polygon::scanline(uint16_t y) {
    m_nLastScanY = y;
    // m_vAET.clear();
    int m_nAETSize = 0;
    for (int i=0;i<m_nETSize;++i) {
        if (y <= m_vET[i].yMax) {
            if (y >= m_vET[i].yMin) {
                m_vAET[m_nAETSize++] = &m_vET[i];
            }
        }
    } 
    IntersectionPointVec retVec;
    retVec.size = m_nAETSize;
    retVec.points = s_vIntersectionPoints;
    for (int i=0;i<m_nAETSize;i++) {
        auto e = m_vAET[i];
        retVec.points[i].x = e->x + e->dx * (y - e->yMin);
        retVec.points[i].y = y;
        retVec.points[i].u = e->u + e->du * (y - e->yMin);
        retVec.points[i].v = e->v + e->dv * (y - e->yMin);
    }
    // sort intersection points by x
    std::sort(retVec.points, retVec.points + m_nAETSize);
    return retVec;
}
