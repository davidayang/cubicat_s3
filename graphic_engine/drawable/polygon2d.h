/*
* @author       Isaac
* @date         2024-11-04
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
*/
#ifndef _POLYGON2D_H_
#define _POLYGON2D_H_
#include "drawable.h"
#include "mesh/edge.h"
#include <vector>
#include "texture.h"
#include "utils/helper.h"

using namespace cubicat;

struct IntersectionPoint{
    float x;
    float u;
    float v;
    float illum;
    float z;
    float w;
};

class Polygon2D : public Drawable{
public:
    DECLARE_RTTI_SUB(Polygon2D, Drawable);
    static SharedPtr<Polygon2D> create(Mesh2DPtr mesh) {
        return SharedPtr<Polygon2D>(new Polygon2D(mesh));
    }
    ~Polygon2D();
    
    void setMesh(Mesh2DPtr mesh) { m_pMesh = mesh; }
    Mesh2DPtr getMesh() { return m_pMesh; }
    Edge* getEdgeList() {return m_vET;}
    uint32_t getEdgeCount() {return m_nETSize;}
protected:
    Polygon2D(Mesh2DPtr mesh);
    void caculateEdges();
    void updateRegion() override;
    Mesh2DPtr                   m_pMesh;
    Edge*                       m_vET = nullptr;
    uint16_t                    m_nETSize = 0;
    uint16_t                    m_nLastScanedIndex = 0;
};

typedef SharedPtr<Polygon2D> PolygonPtr;

#endif