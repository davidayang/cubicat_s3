#include "mesh.h"
#include <string.h>
#include "utils/helper.h"
#include <algorithm>

using namespace cubicat;

Mesh::Mesh(Vertex* pVertex, uint16_t iVertexCount, uint16_t* pIndices, uint16_t iIndexCount)
: m_pVertex(pVertex), m_iVertexCount(iVertexCount), m_pIndices(pIndices), m_iIndexCount(iIndexCount) {

}
Mesh::~Mesh() {
    if (m_pVertex) free(m_pVertex);
    if (m_pIndices) free(m_pIndices);
}
void Mesh::updateVertices(float* pos, uint16_t vertexCount) {
    if (vertexCount > m_iVertexCount) {
        if (m_pVertex) free(m_pVertex);
        m_pVertex = (Vertex*)psram_prefered_malloc(vertexCount * sizeof(Vertex));
        m_iVertexCount = vertexCount;
    }
    for (int i = 0; i < vertexCount; i++) {
        m_pVertex[i].x = pos[i * 2];
        m_pVertex[i].y = pos[i * 2 + 1];
    }
}
void Mesh::updateIndices(uint16_t* pIndices, uint16_t indexCount) {
    if (indexCount > m_iIndexCount) {
        if (m_pIndices) free(m_pIndices);
        m_pIndices = (uint16_t*)psram_prefered_malloc(indexCount * sizeof(uint16_t));
        m_iIndexCount = indexCount;
    }
    memcpy(m_pIndices, pIndices, indexCount * sizeof(uint16_t));
}
void Mesh::updateUVs(float* uvs, uint16_t uvCount) {
    int vertexCount = std::min((int)m_iVertexCount, (int)uvCount >> 1);
    for (int i = 0; i < vertexCount; i++) {
        m_pVertex[i].u = uvs[i * 2];
        m_pVertex[i].v = uvs[i * 2 + 1];
    }
}