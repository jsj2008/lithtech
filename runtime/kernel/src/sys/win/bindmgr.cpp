#include "bdefs.h"

#include "bindmgr.h"
#include "ltmodule.h"
#include "syslibraryloader.h"

#define BINDTYPE_SERVER	0
#define BINDTYPE_DLL	1

#include <string>

typedef struct
{
	HLTMODULE	m_hInstance;
	int			m_Type;

	// Holds name of dll file so that it can be
	// deleted when freed.
	std::string m_sTempFileName;

} WinBind;

typedef void (*SetInstanceHandleFn)(void *handle);

// --------------------------------------------------------- //
// Main interface functions.
// --------------------------------------------------------- //

int bm_BindModule(const char *pModuleName, bool bTempFile, CBindModuleType *&pModule)
{
	// Check if we already have this module loaded.  If we do
	// then we don't need to do the setmasterdatabase.
	bool bModuleAlreadyLoaded = LTLibraryLoader::IsLibraryLoaded(pModuleName);

	HLTMODULE hModule = LTLibraryLoader::OpenLibrary(pModuleName);
	if (hModule == NULL)
	{
		return BIND_CANTFINDMODULE;
    }

	// If this is the first time the module is getting loaded,
	// then tell it about our master database.
	if( !bModuleAlreadyLoaded )
	{
		//merge our interface database with the database in the DLL we just loaded.
		TSetMasterFn pSetMasterFn = (TSetMasterFn)LTLibraryLoader::GetProcAddress(hModule, "SetMasterDatabase");
		
		//check if the function existed.
		if (pSetMasterFn != NULL)
		{
			//merge our database with theirs
			pSetMasterFn(GetMasterDatabase());
		}
	}

	pModule = bm_CreateHandleBinding(bTempFile ? pModuleName : "", (void*)hModule);
	return BIND_NOERROR;
}

CBindModuleType *bm_CreateHandleBinding(const char *pModuleName, void *pHandle)
{
	if (pHandle == NULL)
	{
		pHandle = LTLibraryLoader::GetMainHandle();
	}
	WinBind *pBind;
	LT_MEM_TRACK_ALLOC(( pBind = new WinBind ),LT_MEM_TYPE_MISC);
	pBind->m_hInstance = (HLTMODULE)pHandle;
	pBind->m_Type = BINDTYPE_DLL;
	pBind->m_sTempFileName = pModuleName;

	return (CBindModuleType *)pBind;
}

void bm_UnbindModule(CBindModuleType *hModule)
{
	WinBind *pBind = (WinBind*)hModule;

	ASSERT(pBind);

	if(pBind->m_Type == BINDTYPE_DLL)
	{
		LTLibraryLoader::CloseLibrary(pBind->m_hInstance);
	}

	// Delete the temporary file.
	if( pBind->m_sTempFileName.length( ))
	{

#ifdef __LINUX
		::remove(pBind->m_sTempFileName.c_str( ));
#else
		DeleteFile( pBind->m_sTempFileName.c_str( ));
#endif

		pBind->m_sTempFileName = "";

	}

	delete pBind;
	pBind = NULL;
}


LTRESULT bm_SetInstanceHandle(CBindModuleType *hModule)
{
	SetInstanceHandleFn fn;
	WinBind *pBind;


	pBind = (WinBind*)hModule;
	if(!pBind)
	{
		RETURN_ERROR(1, bm_SetInstanceHandle, LT_INVALIDPARAMS);
	}

	fn = (SetInstanceHandleFn)LTLibraryLoader::GetProcAddress(pBind->m_hInstance, "SetInstanceHandle");
	if(fn)
	{
		fn((void*)pBind->m_hInstance);
	}

	return LT_OK;	
}


LTRESULT bm_GetInstanceHandle(CBindModuleType *hModule, void **pHandle)
{
	WinBind *pBind;

	pBind = (WinBind*)hModule;
	if(!pBind)
	{
		RETURN_ERROR(1, bm_GetInstanceHandle, LT_INVALIDPARAMS);
	}

	*pHandle = (void*)pBind->m_hInstance;
	return LT_OK;
}


void* bm_GetFunctionPointer(CBindModuleType *hModule, const char *pFunctionName)
{
	WinBind *pBind = (WinBind*)hModule;

	ASSERT(pBind);

	return (void*)LTLibraryLoader::GetProcAddress(pBind->m_hInstance, pFunctionName);
}







