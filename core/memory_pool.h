/*
* @author       Isaac
* @date         2024-8-20
* @license      MIT License
* @copyright    Copyright (c) 2024 Deer Valley
* @description  A simple implementation of a memory pool using a kind of linked list
*/
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
    size_t* allocPool(size_t blockCount);
    void expandPool();
    size_t          m_blockSize;    // 每块大小
    size_t          m_blockCount;   // 总块数
    size_t**        m_poolList;     // 内存池
    size_t          m_poolCount;    // 内存池块数
    size_t**        m_freeStack;     // 空闲块地址栈
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
    // TODO: 这里有严重Bug，对于大于1个块的分配，分配出的块可能不是连续的，暂时不使用这个内存池
    size_t* blockPtr = ((size_t**)m_freeStack)[m_freeCount - requiredBlocks];
    m_freeCount -= requiredBlocks;
    *blockPtr = size; // 存储大小信息
    return blockPtr + 1;
}

inline void MemoryPool::deallocate(void* ptr) {
    // 找回大小信息
    size_t* blockPtr = (size_t*)ptr - 1;
    size_t size = *blockPtr;
    size_t requiredBlocks = (size + sizeof(size_t) - 1) / m_blockSize + 1;
    for (size_t i = 0; i < requiredBlocks; ++i) {
        m_freeStack[m_freeCount++] = &blockPtr[i * m_blockSize / sizeof(size_t)];
    }
}

#endif