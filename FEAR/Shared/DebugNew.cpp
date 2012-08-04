//////////////////////////////////////////////////////////////////////////////
// Debug new handler implementation

#include "Stdafx.h"

#ifndef DISABLE_MEMORY_TRACKING

	//operator new that also tracks where the memory was allocated from
	void* operator new (size_t nSize, const char* pszFile, uint32 nLine, uint32 nAllocType)
	{
		LTMemPushMemoryContext(pszFile, nLine, nAllocType);
		void* pRV = (void*)(new uint8[nSize]);
		LTMemPopMemoryContext();
			return pRV;
		}

	//matching delete for our tracked allocator
	void operator delete (void* pData, const char* pszFile, uint32 nLine, uint32 nAllocType)
	{
		LTUNREFERENCED_PARAMETER( pszFile );
		LTUNREFERENCED_PARAMETER( nLine );
		LTUNREFERENCED_PARAMETER( nAllocType );
		delete [] ((uint8*)pData);
	}

	//matching delete for our tracked allocator
	void operator delete[] (void* pData, const char* pszFile, uint32 nLine, uint32 nAllocType)
	{
		LTUNREFERENCED_PARAMETER( pszFile );
		LTUNREFERENCED_PARAMETER( nLine );
		LTUNREFERENCED_PARAMETER( nAllocType );
		delete [] ((uint8*)pData);
	}

#endif //LTMEMTRACK
