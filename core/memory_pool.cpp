#include "memory_pool.h"
#include <stdlib.h>
#include <string.h>

using namespace std;

MemoryPool::MemoryPool(size_t blockSize, size_t blockCount)
    : m_blockSize(blockSize), m_blockCount(blockCount), m_poolList(nullptr), m_poolCount(0),m_freeList(nullptr) {
    void* pool = allocPool(blockCount);

    m_freeList = static_cast<void**>(malloc(blockCount * sizeof(void*)));
    for (size_t i = 0; i < blockCount; ++i) {
        m_freeList[i] = static_cast<char*>(pool) + i * blockSize;
    }
    m_freeCount = blockCount;
}
MemoryPool::~MemoryPool() {
    for (size_t i = 0; i < m_poolCount; ++i) {
        free(m_poolList[i]);
    }
    free(m_freeList);
}
void* MemoryPool::allocPool(size_t blockCount) {
    void* newPool = malloc(m_blockSize * blockCount);
    if (!newPool) {
        throw std::bad_alloc();
    }
    m_poolCount++;
    void **newPoolList = static_cast<void**>(malloc(m_poolCount * sizeof(void*)));
    if (m_poolList) {
        memccpy(newPoolList, m_poolList, 0,(m_poolCount-1) * sizeof(void*));
        free(m_poolList);
    }
    newPoolList[m_poolCount - 1] = newPool;
    m_poolList = newPoolList;
    return newPool;
}
void MemoryPool::expandPool() {
    // 计算新的块数
    size_t expandCount = m_blockCount; // 双倍扩展
    void* newPool = allocPool(expandCount);
    // 将新分配的内存块加入空闲列表
    void *oldFreeList = m_freeList;
    m_freeList = static_cast<void**>(malloc((m_blockCount + expandCount) * sizeof(void*)));
    memccpy(m_freeList + m_blockCount * sizeof(void*), oldFreeList, 0, m_blockCount * sizeof(void*));
    free (oldFreeList);
    for (size_t i = 0; i < expandCount; ++i) {
        m_freeList[m_freeCount++] = static_cast<char*>(newPool) + (i * m_blockSize);
    }

    // 更新成员变量
    m_blockCount += expandCount;
}