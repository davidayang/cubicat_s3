#include "quad.h"

Quad::Quad(uint16_t width, uint16_t height)
: Polygon2D(Mesh2DPtr(nullptr)), m_width(width), m_height(height) {
    Vertex2* vertices = (Vertex2*)malloc(4 * sizeof(Vertex2));
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

    uint16_t* indices = (uint16_t*)malloc(6 * sizeof(uint16_t));
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 2;
    indices[4] = 3;
    indices[5] = 0; 
    m_pMesh = Mesh2D::create(vertices, 4, indices, 6, true);
}

void Quad::setPivot(float x, float y) {
    Drawable::setPivot(x, y);
    for (int i=0; i<4; i++) {
        auto v = m_pMesh->getVertex(i);
        v->x -= m_width * x;
        v->y -= m_height * y;
    }
}