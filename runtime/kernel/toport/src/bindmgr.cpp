
#include "bdefs.h"
#include "bindmgr.h"
#include "netmgr.h"
#include "servermgr.h"

static int bm_BindToServer(CBindModuleType *&pModule)
{
return 0;   // DAN - temporary
}

static void bm_UnbindFromServer()
{
}


// --------------------------------------------------------- //
// Main interface functions.
// --------------------------------------------------------- //

int bm_BindModule(const char *pModuleName, CBindModuleType *&pModule)
{
return 0;   // DAN - temporary
}


void bm_UnbindModule(CBindModuleType *hModule)
{
}


LTRESULT bm_SetInstanceHandle(CBindModuleType *hModule)
{
return LT_OK;   // DAN - temporary
}


LTRESULT bm_GetInstanceHandle(CBindModuleType *hModule, void **pHandle)
{
return LT_OK;   // DAN - temporary
}


void* bm_GetFunctionPointer(CBindModuleType *hModule, const char *pFunctionName)
{
return NULL;   // DAN - temporary
}

















