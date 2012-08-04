//////////////////////////////////////////////////////////////////////////////
// DebugNew implementation header (for the templates / wacky macros / etc)

#include "DebugNew.h"

// The actual functions that dump the debug info
#ifdef _DEBUG
void dump_debug_info_new(const char *pFile, unsigned int nLine, unsigned int nSize);
void dump_debug_info_delete(const char *pFile, unsigned int nLine);
#endif

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

