#ifndef __ILTMEMORY_H__
#define __ILTMEMORY_H__

//standard symbols
#ifndef __LTBASEDEFS_H__
#	include "ltbasedefs.h"
#endif

class ILTMemory
{
public:

	//sets up information about any allocations called before the matching call to
	//ReleaseAllocInfo. This information allows the engine to properly track memory
	//through its interfaces when tracking is enabled
	virtual void	SetAllocInfo(const char* pszFilename, uint32 nLine, uint32 nMemoryType) = 0;

	//Releases the allocation information so that other lines can use it to specify their
	//data
	virtual void	ReleaseAllocInfo() = 0;

	//called to acquire a block of memory from the engine. It will return NULL if no memory
	//could be allocated.
	virtual void*	Allocate(uint32 nSize) = 0;

	//called to release a block of memory from the engine. This will properly handle NULL
	//values and should only be called on memory allocated with the Allocate function
	virtual void	Free(void* pData) = 0;

protected:

	//prevent instantiation
	ILTMemory()		{}
};

#endif //__ILTMEMORY_H__
