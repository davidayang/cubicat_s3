#include "mesh.h"
#include <string.h>
using namespace cubicat;

Mesh::Mesh(Vertex* pVertex, uint16_t iVertexCount, uint16_t* pIndices, uint16_t iIndexCount)
: m_pVertex(pVertex), m_iVertexCount(iVertexCount), m_pIndices(pIndices), m_iIndexCount(iIndexCount) {

}
Mesh::~Mesh() {
    delete[] m_pVertex;
    delete[] m_pIndices;
}