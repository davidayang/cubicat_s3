#ifndef _MEMORYOBJECT_H_
#define _MEMORYOBJECT_H_
#include <sys/types.h>
#define MEMORY_TRACK 0

#if MEMORY_TRACK
#define NEW new(__FILE__, __LINE__)
#else
#define NEW new
#endif

class MemoryObject
{
public:
	void* operator new(size_t size);
    void* operator new(size_t size,const char* file,int line);
	void* operator new[](size_t size);
    void* operator new[](size_t size,const char* file,int line);

	void operator delete(void* buffer);
	void operator delete[](void* buffer);
};
#endif