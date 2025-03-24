#include "memory_pool.h"
#include <stdlib.h>
#include <string.h>

using namespace std;

MemoryPool::MemoryPool(size_t blockSize, size_t blockCount)
    : m_blockSize(blockSize), m_blockCount(blockCount), m_poolList(nullptr), m_poolCount(0),m_freeStack(nullptr) {
    size_t* pool = allocPool(blockCount);

    m_freeStack = (size_t**)malloc(blockCount * sizeof(size_t*));
    for (size_t i = 0; i < blockCount; ++i) {
        m_freeStack[i] = pool + i * blockSize / sizeof(size_t);
    }
    m_freeCount = blockCount;
}
MemoryPool::~MemoryPool() {
    for (size_t i = 0; i < m_poolCount; ++i) {
        free(m_poolList[i]);
    }
    free(m_freeStack);
}
size_t* MemoryPool::allocPool(size_t blockCount) {
    size_t* newPool = static_cast<size_t*>(malloc(m_blockSize * blockCount));
    if (!newPool) {
        throw std::bad_alloc();
    }
    m_poolCount++;
    size_t **newPoolList = static_cast<size_t**>(malloc(m_poolCount * sizeof(size_t*)));
    if (m_poolList) {
        memccpy(newPoolList, m_poolList, 0,(m_poolCount-1) * sizeof(size_t*));
        free(m_poolList);
    }
    newPoolList[m_poolCount - 1] = newPool;
    m_poolList = newPoolList;
    return newPool;
}
void MemoryPool::expandPool() {
    // 计算新的块数
    size_t expandCount = m_blockCount; // 双倍扩展
    size_t* newPool = allocPool(expandCount);
    // 将新分配的内存块加入空闲列表
    void *oldFreeList = m_freeStack;
    m_freeStack = static_cast<size_t**>(malloc((m_blockCount + expandCount) * sizeof(void*)));
    memccpy(m_freeStack + m_blockCount * sizeof(void*), oldFreeList, 0, m_blockCount * sizeof(void*));
    free (oldFreeList);
    for (size_t i = 0; i < expandCount; ++i) {
        m_freeStack[m_freeCount++] = newPool + (i * m_blockSize / sizeof(size_t));
    }

    // 更新成员变量
    m_blockCount += expandCount;
}