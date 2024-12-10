#ifndef _MEMORY_POOL_H_
#define _MEMORY_POOL_H_

#include <stdlib.h>
#include <assert.h>
#include <new>
#include <stdio.h>

class MemoryPool {
public:
    MemoryPool(size_t blockSize, size_t blockCount);
    ~MemoryPool();

    void* allocate(size_t size);
    void deallocate(void* ptr);
    size_t getBlockSize() const { return m_blockSize; }
private:
    void* allocPool(size_t blockCount);
    void expandPool();
    size_t          m_blockSize;    // 每块大小
    size_t          m_blockCount;   // 总块数
    void**          m_poolList;     // 内存池
    size_t          m_poolCount;    // 内存池块数
    void**          m_freeList;     // 空闲块列表
    size_t          m_freeCount;    // 当前空闲块数
};

inline void* MemoryPool::allocate(size_t size) {
    // 向上取整并且包含内存块头部额外一个字节，如果size=blockSize，则requiredBlocks=2，因为要额外多分配一个字节
    size_t requiredBlocks = (size + sizeof(size_t) - 1) / m_blockSize + 1;
    if (requiredBlocks > m_freeCount) {
        // 没有足够的空闲块，扩展内存池
        expandPool();
    }

    // 分配多个块,包含头部大小信息
    char* blockPtr = ((char**)m_freeList)[m_freeCount - requiredBlocks];
    m_freeCount -= requiredBlocks;
    *blockPtr = size; // 存储大小信息
    return blockPtr + sizeof(size_t);
}

inline void MemoryPool::deallocate(void* ptr) {
    // 找回大小信息
    void* blockPtr = (char*)ptr - sizeof(size_t);
    size_t size = *(static_cast<char*>(blockPtr));
    size_t requiredBlocks = (size + sizeof(size_t) - 1) / m_blockSize + 1;
    void** allocatedBlocks = static_cast<void**>(blockPtr);
    for (size_t i = 0; i < requiredBlocks; ++i) {
        m_freeList[m_freeCount++] = &allocatedBlocks[i * m_blockSize];
    }
}

#endif