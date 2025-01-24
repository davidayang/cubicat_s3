#ifndef _MESH_H_
#define _MESH_H_
#include "vectex.h"
#include <stdint.h>
#include "core/shared_pointer.h"
#include "core/memory_object.h"

namespace cubicat{

class Mesh : public MemoryObject {
public:
    static SharedPtr<Mesh> create(Vertex* pVertex, uint16_t iVertexCount, uint16_t* pIndices, uint16_t iIndexCount) {
        return SharedPtr<Mesh>(NEW Mesh(pVertex, iVertexCount, pIndices, iIndexCount));
    }
    ~Mesh();
    inline Vertex* getVertex(uint32_t index) {
        if (index >= m_iVertexCount) {
            return nullptr;
        }
        return &m_pVertex[index];
    }
    inline uint16_t getVertexIndexByIndex(uint32_t index) { assert(index < m_iIndexCount); return m_pIndices[index]; }
    uint16_t getVertexCount() { return m_iVertexCount; }
    uint16_t getIndexCount() { return m_iIndexCount; }
    void updateVertices(float* pos, uint16_t vertexCount);
    void updateIndices(uint16_t* pIndices, uint16_t indexCount);
    void updateUVs(float* uvs, uint16_t uvCount);
private:
    Mesh(Vertex* pVertex, uint16_t iVertexCount, uint16_t* pIndices, uint16_t iIndexCount);
    Vertex*     m_pVertex;
    uint16_t    m_iVertexCount;
    uint16_t*   m_pIndices;
    uint16_t    m_iIndexCount;
};
typedef SharedPtr<Mesh> MeshPtr;

}

#endif