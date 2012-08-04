
// Defines all the memory management routines for DirectEngine.
// new and dnew will ALWAYS do an em_ThrowError() on failure.
// malloc can return NULL if it runs out of memory.
// dmalloc will do an em_ThrowError() on failure.

#ifndef __DE_MEMORY_H__
#define __DE_MEMORY_H__

#ifndef __MALLOC_H__
#include <malloc.h>
#define __MALLOC_H__
#endif

// Init/term the memory system.
void dm_Init();
void dm_Term();


// Memory stats.
uint32 dm_GetBytesAllocated();
uint32 dm_GetNumAllocations();

extern void* operator new(size_t size, void *ptr, char z);

// C dalloc/dfree functions.
void* dalloc(uint32 size);
void* dalloc_z(uint32 size);	// Allocate and zero-init.

void dfree(void *ptr);

#endif  // __DE_MEMORY_H__




