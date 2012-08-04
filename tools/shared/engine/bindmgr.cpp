
#include "bdefs.h"
#include "bindmgr.h"
#include "winbind.h"



typedef void (*SetInstanceHandleFn)(void *handle);


// --------------------------------------------------------- //
// Main interface functions.
// --------------------------------------------------------- //

int bm_BindModule(const char *pModuleName, HBINDMODULE *pModule)
{
	HINSTANCE hInstance = LoadLibrary(pModuleName);
    if (hInstance == NULL) {
		return BIND_CANTFINDMODULE;
    }

	WinBind *pBind = (WinBind*)malloc(sizeof(WinBind));
	pBind->m_hInstance = hInstance;
	pBind->m_Type = BINDTYPE_DLL;

	*pModule = (HBINDMODULE)pBind;
	return BIND_NOERROR;
}


void bm_UnbindModule(HBINDMODULE hModule)
{
	WinBind *pBind = (WinBind*)hModule;

	ASSERT(pBind);

	if(pBind->m_Type == BINDTYPE_DLL)
	{
		FreeLibrary(pBind->m_hInstance);
	}
	
	free(pBind);
}


LTRESULT bm_SetInstanceHandle(HBINDMODULE hModule)
{
	SetInstanceHandleFn fn;
	WinBind *pBind;


	pBind = (WinBind*)hModule;
	if(!pBind)
		RETURN_ERROR(1, bm_SetInstanceHandle, LT_INVALIDPARAMS);

	fn = (SetInstanceHandleFn)GetProcAddress(pBind->m_hInstance, "SetInstanceHandle");
	if(fn)
	{
		fn((void*)pBind->m_hInstance);
	}

	return LT_OK;	
}


LTRESULT bm_GetInstanceHandle(HBINDMODULE hModule, void **pHandle)
{
	WinBind *pBind;

	pBind = (WinBind*)hModule;
	if(!pBind)
		RETURN_ERROR(1, bm_GetInstanceHandle, LT_INVALIDPARAMS);

	*pHandle = (void*)pBind->m_hInstance;
	return LT_OK;
}


void* bm_GetFunctionPointer(HBINDMODULE hModule, const char *pFunctionName)
{
	WinBind *pBind = (WinBind*)hModule;

	ASSERT(pBind);

	return (void*)GetProcAddress(pBind->m_hInstance, pFunctionName);
}







