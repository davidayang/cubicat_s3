#ifndef _MEMORY_ALLOCATOR_H_
#define _MEMORY_ALLOCATOR_H_

extern "C" bool psram_init();
extern "C" void* psram_malloc(unsigned int size);
extern "C" void* psram_prefered_malloc(unsigned int size);
extern "C" void* psram_prefered_realloc(void* p, unsigned int size);
extern "C" void* dma_prefered_malloc(unsigned int size);

#endif