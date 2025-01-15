#ifndef _POLYGON_H_
#define _POLYGON_H_
#include "drawable.h"
#include "mesh/edge.h"
#include <vector>
#include "texture.h"

namespace cubicat{

struct IntersectionPoint{
    float x;
    uint16_t y;
    float u;
    float v;
};

class Polygon : public Drawable{
public:
    DECLARE_RTTI_SUB(Polygon, Drawable);
    static SharedPtr<Polygon> create(float width, float height, void* texData, bool hasMask, uint16_t maskColor = 0) {
        return SharedPtr<Polygon>(NEW Polygon(width, height, texData, hasMask, maskColor));
    }
    ~Polygon();
    void setTexture(TexturePtr pTexture) { m_pTexture = pTexture; }
    TexturePtr getTexture() { return m_pTexture; }
    const void* getTextureData() override { return m_pTexture->getTextureData(); }
    void setColor(uint16_t color) { m_Color = color; }
    uint32_t getColor() { return m_Color; }
    // The edge is sorted starting from the minimum Y coordinate in world space,
    // so the scan result is also sorted in world space(Y axis up).
    // @param y scanline position
    std::vector<IntersectionPoint> scanline(uint16_t y);
protected:
    Polygon(float width, float height, void* texData, bool hasMask, uint16_t maskColor);
    void caculateEdges();
    void updateBoundingBox() override;
    MeshPtr                 m_pMesh;
    TexturePtr              m_pTexture;
    // total edge table
    std::vector<Edge>       m_vET;
    // active edge table
    std::vector<Edge*>      m_vAET;
    uint16_t                m_Color = 0xFFFF;
};

}

#endif