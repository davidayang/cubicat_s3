#ifndef _POLYGON_H_
#define _POLYGON_H_
#include "drawable.h"
#include "mesh/edge.h"
#include <vector>
#include "texture.h"
#include "utils/helper.h"

namespace cubicat{

struct IntersectionPoint{
    float x;
    uint16_t y;
    float u;
    float v;
    bool operator<(const IntersectionPoint& other) const {return x < other.x;}
    bool operator>(const IntersectionPoint& other) const {return x > other.x;}
};

struct IntersectionPointVec{
    IntersectionPoint* points;
    int size;
};

class Polygon : public Drawable{
public:
    DECLARE_RTTI_SUB(Polygon, Drawable);
    static SharedPtr<Polygon> create(float width, float height, bool hasMask = false, uint16_t maskColor = 0) {
        return SharedPtr<Polygon>(NEW Polygon(width, height, hasMask, maskColor));
    }
    ~Polygon();
    void setTexture(TexturePtr texture) { m_pTexture = texture; }
    TexturePtr getTexture() { return m_pTexture; }
    const void* getTextureData() override;
    void setColor(uint16_t color) { m_Color = color; }
    uint32_t getColor() { return m_Color; }
    // The edge is sorted starting from the minimum Y coordinate in world space,
    // so the scan result is also sorted in world space(Y axis up).
    // @param y scanline position
    IntersectionPointVec scanline(uint16_t y);
    MeshPtr getMesh() { return m_pMesh; }
protected:
    Polygon(float width, float height, bool hasMask, uint16_t maskColor);
    void caculateEdges();
    void updateBoundingBox() override;
    MeshPtr                     m_pMesh;
    TexturePtr                  m_pTexture;
    uint16_t                    m_nLastScanY = 0xFFFF;
    // total edge table
    // std::vector<Edge>        m_vET;
    Edge*                       m_vET = nullptr;
    uint16_t                    m_nETSize = 0;
    uint16_t                    m_nLastScanedIndex = 0;
    // active edge table
    // std::vector<Edge*>       m_vAET;
    Edge**                      m_vAET = nullptr;
    uint32_t                    m_nAETCacheSize = 0;
    uint16_t                    m_nAETSize = 0;
    uint16_t                    m_Color = 0xFFFF;
    // intersection points size is same as total edge table size
    static IntersectionPoint*   s_vIntersectionPoints;
    static uint16_t             s_nIntersectionPointsSize;
};

typedef SharedPtr<Polygon> PolygonPtr;

}

#endif