//////////////////////////////////////////////////////////////////////////////
// Debug new handler implementation

#ifdef LTMEMTRACK

	#include "StdAfx.h"
	#include "iltmemory.h"

	//the global ILTMemory interface for use	
	typedef ILTMemory* (*TGetILTMemoryFn)();
	static ILTMemory* g_pILTMemory;

	//this function will verify that the ILTMemory interface is hooked up correctly,
	//and will return true if it is. Otherwise it will return false and not try again
	static bool VerifyMemoryInterface()
	{
		static bool s_bFailedMemInit = false;

		if(!g_pILTMemory && !s_bFailedMemInit)
		{
			//get the calling module
			HMODULE hModule = GetModuleHandle(NULL);
			FARPROC hProcedure = GetProcAddress(hModule, "LTGetILTMemory");

			if(hProcedure)
			{
				TGetILTMemoryFn MemoryFn = (TGetILTMemoryFn)hProcedure;
				g_pILTMemory = MemoryFn();
			}
			else
			{
				DWORD nError = GetLastError();
				s_bFailedMemInit = true;
				return NULL;
			}
		}

		return g_pILTMemory != NULL;
	}


	//overload operator new
	void* operator new (size_t nSize)
	{
		if(VerifyMemoryInterface())
		{
			//we have a call to new that did not go through debug new, at least filter it
			//into the appropriate game code section
			g_pILTMemory->SetAllocInfo(__FILE__, __LINE__, LT_MEM_TYPE_UNKNOWN);
			void* pRV = g_pILTMemory->Allocate(nSize);
			g_pILTMemory->ReleaseAllocInfo();

			return pRV;
		}

		return malloc(nSize);
	}

	//overload operator delete
	void operator delete(void* pData)
	{
		if(VerifyMemoryInterface())
		{
			g_pILTMemory->Free(pData);
			return;
		}

		free(pData);
	}

	//operator new that also tracks where the memory was allocated from
	void* operator new (size_t nSize, const char* pszFile, uint32 nLine, uint32 nAllocType)
	{
		if(VerifyMemoryInterface())
		{
			g_pILTMemory->SetAllocInfo(pszFile, nLine, nAllocType);
			void* pRV = g_pILTMemory->Allocate(nSize);
			g_pILTMemory->ReleaseAllocInfo();

			return pRV;
		}

		return malloc(nSize);
	}

	//matching delete for our tracked allocator
	void operator delete (void* pData, const char* pszFile, uint32 nLine, uint32 nAllocType)
	{
		if(VerifyMemoryInterface())
		{
			g_pILTMemory->Free(pData);
			return;
		}

		free(pData);
	}

#endif //LTMEMTRACK
