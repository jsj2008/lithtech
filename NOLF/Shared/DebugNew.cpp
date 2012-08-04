//////////////////////////////////////////////////////////////////////////////
// Debug new handler implementation

#include "StdAfx.h"

#ifdef _DEBUG

#include <windows.h>
#include <stdio.h>

static int32 g_nDebugAlloc_AllocCount = 0;

void dump_debug_info_new(const char *pFile, unsigned int nLine, unsigned int nSize)
{
	// Count the allocation
	++g_nDebugAlloc_AllocCount;

#ifdef _CLIENTBUILD
	if (g_pLTClient)
	{
		HCONSOLEVAR hConVar = g_pLTClient->GetConsoleVar("DebugAlloc");
		ILTCSBase *pInterface = g_pLTClient;
		const char *pName = "Client";
#else // _CLIENTBUILD
	if (g_pLTServer)
	{
		HCONSOLEVAR hConVar = g_pLTServer->GetGameConVar("DebugAlloc");
		ILTCSBase *pInterface = g_pLTServer;
		const char *pName = "Server";
#endif // _CLIENTBUILD
		if (hConVar)
		{
			uint32 nFlags = (uint32)pInterface->GetVarValueFloat(hConVar);
			if (nFlags & 1)
				pInterface->CPrint("new at %s:%d (%d)", pFile, nLine, nSize);
			if (nFlags & 2)
				pInterface->CPrint("%s allocations: %d", pName, g_nDebugAlloc_AllocCount);
		}
	}
	else
	{
		char temp[512];
		sprintf(temp, "new at %s:%d (%d)\n", pFile, nLine, nSize);
		OutputDebugString(temp);
	}
}

void dump_debug_info_delete(const char *pFile, unsigned int nLine)
{
	// Count the deallocation
	--g_nDebugAlloc_AllocCount;

#ifdef _CLIENTBUILD
	if (g_pLTClient)
	{
		HCONSOLEVAR hConVar = g_pLTClient->GetConsoleVar("DebugAlloc");
		ILTCSBase *pInterface = g_pLTClient;
		const char *pName = "Client";
#else // _CLIENTBUILD
	if (g_pLTServer)
	{
		HCONSOLEVAR hConVar = g_pLTServer->GetGameConVar("DebugAlloc");
		ILTCSBase *pInterface = g_pLTServer;
		const char *pName = "Server";
#endif // _CLIENTBUILD
		if (hConVar)
		{
			uint32 nFlags = (uint32)pInterface->GetVarValueFloat(hConVar);
			if (nFlags & 1)
				pInterface->CPrint("delete at %s:%d", pFile, nLine);
			if (nFlags & 2)
				pInterface->CPrint("%s allocations: %d", pName, g_nDebugAlloc_AllocCount);
		}
	}
	else
	{
		char temp[512];
		sprintf(temp, "delete at %s:%d\n", pFile, nLine);
		OutputDebugString(temp);
	}
}

#endif // _DEBUG