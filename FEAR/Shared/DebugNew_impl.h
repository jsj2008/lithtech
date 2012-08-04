//////////////////////////////////////////////////////////////////////////////
// DebugNew implementation header (for the templates / wacky macros / etc)

//for the DISABLE_MEMORY_TRACKING definition
#include "iltmemory.h"

// The actual functions that dump the debug info
#ifndef DISABLE_MEMORY_TRACKING

	//determine what memory type to use
	#if defined MEMTRACK_SERVER
	#	define MEMORY_TYPE			LT_MEM_TYPE_OBJECTSHELL
	#elif defined MEMTRACK_CLIENT
	#	define MEMORY_TYPE			LT_MEM_TYPE_CLIENTSHELL
	#elif defined MEMTRACK_CLIENTFX
	#	define MEMORY_TYPE			LT_MEM_TYPE_CLIENTFX
	#else
	#	define MEMORY_TYPE			LT_MEM_TYPE_GAMECODE
	#endif

	void* operator new (size_t nSize, const char* pszFile, uint32 nLine, uint32 nAllocType);
	void operator delete (void* pData, const char* pszFile, uint32 nLine, uint32 nAllocType);
	void operator delete[] (void* pData, const char* pszFile, uint32 nLine, uint32 nAllocType);

	void dump_debug_info_new(const char *pFile, unsigned int nLine, unsigned int nSize);
	void dump_debug_info_delete(const char *pFile, unsigned int nLine);

	// The templated new support functions
	template<typename T>
	T* debug_new_fn(uint32 nLine, const char* pszFile)
	{
		return new (pszFile, nLine, MEMORY_TYPE) T;
	}

	template<typename T>
	T* debug_new_fna(int nCount, uint32 nLine, const char* pszFile)
	{
		return new (pszFile, nLine, MEMORY_TYPE) T[nCount];
	}

	template<typename T, typename P1>
	T* debug_new_fn_param(P1 param1, uint32 nLine, const char* pszFile)
	{
		return new (pszFile, nLine, MEMORY_TYPE) T(param1);
	}

	template<typename T, typename P1, typename P2>
	T* debug_new_fn_param(P1 param1, P2 param2, uint32 nLine, const char* pszFile)
	{
		return new (pszFile, nLine, MEMORY_TYPE) T(param1, param2);
	}

	template<typename T, typename P1, typename P2, typename P3>
	T* debug_new_fn_param(P1 param1, P2 param2, P3 param3, uint32 nLine, const char* pszFile)
	{
		return new (pszFile, nLine, MEMORY_TYPE) T(param1, param2, param3);
	}
	template<typename T, typename P1, typename P2, typename P3, typename P4>
	T* debug_new_fn_param(P1 param1, P2 param2, P3 param3, P4 param4, uint32 nLine, const char* pszFile)
	{
		return new (pszFile, nLine, MEMORY_TYPE) T(param1, param2, param3, param4);
	}
#else // DISABLE_MEMORY_TRACKING

	// The templated new support functions
	template<typename T>
	T* debug_new_fn()
	{
		return new T;
	}

	template<typename T>
	T* debug_new_fna(int nCount)
	{
		return new T[nCount];
	}

	template<typename T, typename P1>
	T* debug_new_fn_param(P1 param1)
	{
		return new T(param1);
	}

	template<typename T, typename P1, typename P2>
	T* debug_new_fn_param(P1 param1, P2 param2)
	{
		return new T(param1, param2);
	}

	template<typename T, typename P1, typename P2, typename P3>
	T* debug_new_fn_param(P1 param1, P2 param2, P3 param3)
	{
		return new T(param1, param2, param3);
	}

	template<typename T, typename P1, typename P2, typename P3, typename P4>
	T* debug_new_fn_param(P1 param1, P2 param2, P3 param3, P4 param4)
	{
		return new T(param1, param2, param3, param4);
	}

#endif // DISABLE_MEMORY_TRACKING

// The templated delete support functions
template<typename T>
void debug_delete_fn(T* pPtr)
{
	delete pPtr;
}

template<typename T>
void debug_delete_fna(T* pPtr)
{
	delete[] pPtr;
}

