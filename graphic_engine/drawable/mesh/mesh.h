#ifndef _MESH_H_
#define _MESH_H_
#include "vectex.h"
#include <stdint.h>
#include "core/shared_pointer.h"
#include "core/memory_object.h"
#include "core/id_object.h"
#include "utils/helper.h"
#include <string.h>
#include "core/rtti.h"
#include "core/memory_allocator.h"

namespace cubicat{

template <class T, class U>
class Mesh : public MemoryObject, public RTTI, public IDObject {
public:
    DECLARE_RTTI_ROOT(Mesh);
    static_assert(std::is_same<T, Vertex2>::value || std::is_same<T, Vertex3>::value, "T must be Vertex2 or Vertex3");
    static SharedPtr<Mesh> create(T* vertices, uint16_t iVertexCount, uint16_t* pIndices, uint16_t iIndexCount, bool manageData) {
        return SharedPtr<Mesh>(new Mesh(vertices, iVertexCount, pIndices, iIndexCount, manageData));
    }
    virtual ~Mesh() {
        if (!m_bDataManaged) return;
        if (m_pVertices) free(m_pVertices);
        if (m_pIndices) free(m_pIndices);
    }
    virtual uint16_t getVertexCount() { return m_iVertexCount; }
    virtual uint16_t getNormalCount() { return m_iVertexCount; }
    virtual uint16_t getFaceCount() { return m_iIndexCount / 3; }
    virtual void updateVertices(float* pos, uint16_t vertexCount);
    virtual inline T* getVertex(uint32_t index) {
        assert (index < m_iVertexCount);
        if (!m_pVertices) return nullptr;
        return &m_pVertices[index];
    }
    virtual inline const U& getPosition(uint32_t index) {
        assert (index < m_iVertexCount);
        return m_pVertices[index].pos;
    }
    uint16_t getPositionCount() { return m_iVertexCount; }
    inline void getFaceVerticesIndex(uint16_t faceIdx, uint16_t& idx0, uint16_t& idx1, uint16_t& idx2) {
        auto addr = m_pIndices + faceIdx * 3;
        idx0 = *(addr++);
        idx1 = *(addr++);
        idx2 = *addr;
    }
    void updateIndices(uint16_t* pIndices, uint16_t indexCount);
    void updateUVs(float* uvs, uint16_t uvCount);
protected:
    Mesh(T* vertices, uint16_t iVertexCount, uint16_t* pIndices, uint16_t iIndexCount, bool manageData)
    : m_pVertices(vertices), m_pIndices(pIndices), m_bDataManaged(manageData), m_iVertexCount(iVertexCount), m_iIndexCount(iIndexCount) {}
    T*          m_pVertices = nullptr;
    uint16_t    m_nVertexBufferSize = 0;
    uint16_t*   m_pIndices = nullptr;
    uint16_t    m_nIndexBufferSize = 0;
    bool        m_bDataManaged;
    uint16_t    m_iVertexCount;
    uint16_t    m_iIndexCount;
};

template <class T, class U>
void Mesh<T,U>::updateIndices(uint16_t* pIndices, uint16_t indexCount) {
    if (indexCount > m_nIndexBufferSize) {
        if (m_pIndices) free(m_pIndices);
        m_pIndices = (uint16_t*)psram_prefered_malloc(indexCount * sizeof(uint16_t));
        m_nIndexBufferSize = indexCount;
    }
    m_iIndexCount = indexCount;
    memcpy(m_pIndices, pIndices, indexCount * sizeof(uint16_t));
}
template <class T, class U>
void Mesh<T, U>::updateUVs(float* uvs, uint16_t uvCount) {
    int vertexCount = std::min((int)m_iVertexCount, (int)uvCount >> 1);
    for (int i = 0; i < vertexCount; i++) {
        m_pVertices[i].u = uvs[i * 2];
        m_pVertices[i].v = uvs[i * 2 + 1];
    }
}

// Specialization Mesh class for Vertex2
template <>
inline void Mesh<Vertex2, Vector2f>::updateVertices(float* pos, uint16_t vertexCount) {
    if (vertexCount > m_nVertexBufferSize) {
        if (m_pVertices && m_bDataManaged) free(m_pVertices);
        m_pVertices = (Vertex2*)psram_prefered_malloc(vertexCount * sizeof(Vertex2));
        m_nVertexBufferSize = vertexCount;
        m_bDataManaged = true;
    }
    m_iVertexCount = vertexCount;
    for (int i = 0; i < vertexCount; i++) {
        m_pVertices[i].x = pos[i * 2];
        m_pVertices[i].y = pos[i * 2 + 1];
    }
}

// Specialization Mesh class for Vertex3
template <>
inline void Mesh<Vertex3, Vector3f>::updateVertices(float* pos, uint16_t vertexCount) {
    if (vertexCount > m_iVertexCount) {
        if (m_pVertices && m_bDataManaged) free(m_pVertices);
        m_pVertices = (Vertex3*)psram_prefered_malloc(vertexCount * sizeof(Vertex3));
        m_iVertexCount = vertexCount;
        m_bDataManaged = true;
    }
    for (int i = 0; i < vertexCount; i++) {
        m_pVertices[i].x = pos[i * 3];
        m_pVertices[i].y = pos[i * 3 + 1];
        m_pVertices[i].z = pos[i * 3 + 2];
    }
}

typedef Mesh<Vertex2, Vector2f> Mesh2D;
typedef SharedPtr<Mesh2D> Mesh2DPtr;

}

#endif