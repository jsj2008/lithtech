
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


	typedef void (*cleanup_handler)(void *pUser);


	// Init/term the memory system.
	void dm_Init();
	void dm_Term();

	
	// Memory stats.
	uint32 dm_GetBytesAllocated();
	uint32 dm_GetNumAllocations();


	// Add a cleanup handler to the system.  If one of the allocation
	// routines can't allocate enough memory, it'll call ALL the cleanup
	// handlers, which should try to free as much as possible, and try one more time.
	void dm_AddCleanupHandler(cleanup_handler fn, void *pUser);

	// Get rid of a cleanup handler.
	void dm_RemoveCleanupHandler(cleanup_handler fn);


	// C++ dnew/ddelete functions.
	#ifdef __cplusplus
		extern void* operator new(size_t size, void *ptr, char z);

		#define dnew(type) ::new((void*)0, (char)'q') (x);
		#define ddelete(x) delete (x);
	#endif


	// C dalloc/dfree functions.
	void* dalloc(uint32 size);
	void* dalloc_z(uint32 size);	// Allocate and zero-init.

	void dfree(void *ptr);


#endif  // __DE_MEMORY_H__




