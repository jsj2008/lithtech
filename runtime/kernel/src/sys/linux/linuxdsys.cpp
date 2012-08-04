
// This module implements all the dsi_interface functions.

#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

#include "bdefs.h"
#include "stdlterror.h"
#include "stringmgr.h"
#include "sysfile.h"
#include "de_objects.h"
#include "servermgr.h"
#include "classbind.h"
#include "bindmgr.h"
#include "console.h"


//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);



void dsi_OnReturnError(int err)
{
}

static LTBOOL dsi_LoadResourceModule()
{
return LTTRUE;      // DAN - temporary
}

static void dsi_UnloadResourceModule()
{
}


LTRESULT dsi_SetupMessage(char *pMsg, int maxMsgLen, LTRESULT dResult, va_list marker)
{
return LT_OK;      // DAN - temporary
}


int dsi_Init()
{
	dm_Init();	// Memory manager.
	str_Init();	// String manager.
	df_Init();	// File manager.
//	obj_Init();	// Object manager.
//	packet_Init();
return 0;      // DAN - temporary
}

void dsi_Term()
{
//	packet_Term();
//	obj_Term();
	df_Term();
	str_Term();
	dm_Term();
	return;
}

void* dsi_GetResourceModule()
{
return NULL;      // DAN - temporary
}


LTRESULT _GetOrCopyFile(char *pTempPath, char *pFilename, char *pOutName, int outNameLen)
{
    return LTTRUE;      // DAN - temporary
}


LTRESULT dsi_LoadServerObjects(CClassMgr *pInfo)
{
	char* pGameServerObjectName = "libobject.so";

    //load the GameServer shared object
    int version;
    int status = cb_LoadModule(pGameServerObjectName, false, pInfo->m_ClassModule, &version);

    //check for errors.
    if (status == CB_CANTFINDMODULE) 
	{
        return LT_INVALIDOBJECTDLL;
    }
    else if (status == CB_NOTCLASSMODULE)
	{
        return LT_INVALIDOBJECTDLL;
    }
    else if (status == CB_VERSIONMISMATCH) 
	{
		return LT_INVALIDOBJECTDLLVERSION;
	}
	
/*	
	    // Get sres.dll.
	bFileCopied = false;
    if ((GetOrCopyFile("sres.dll", fileName, sizeof(fileName),bFileCopied) != LT_OK)
        || (bm_BindModule(fileName, bFileCopied, pClassMgr->m_hServerResourceModule) != BIND_NOERROR))
    {
		cb_UnloadModule( pClassMgr->m_ClassModule );

        sm_SetupError(LT_ERRORCOPYINGFILE, "sres.dll");
        RETURN_ERROR_PARAM(1, LoadServerObjects, LT_ERRORCOPYINGFILE, "sres.dll");
    }

    //let the dll know it's instance handle.
    if (instance_handle_server != NULL) 
	{
        instance_handle_server->SetInstanceHandle( pClassMgr->m_ClassModule.m_hModule );
    }
*/
	
	//cb_LoadModule(fileName, false, pInfo->m_ClassModule, &version);

	/*
	pInfo->m_hShellModule = (ShellModule*)malloc(sizeof(ShellModule));
	pInfo->m_hShellModule->m_hModule = NULL;
	pInfo->m_CreateShellFn =
		pInfo->m_hShellModule->m_CreateFn = (CreateShellFn) CreateServerShell;
	pInfo->m_DeleteShellFn =
		pInfo->m_hShellModule->m_DeleteFn = (DeleteShellFn) DeleteServerShell;
	*/

	return LT_OK;
}

void dsi_Sleep(uint32 ms)
{
// Several possible implementations:
// poll (requires sys/poll.h)
//	poll(NULL, 0, ms);
// select (requires sys/time.h, sys/types.h, unistd.h)
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = ms*1000;
	select(0, NULL, NULL, NULL, &timeout);
// SIGALRM (requires sys/time.h, unistd.h)
//	itimerval timerconfig;
//	memset(&timerconfig, 0, sizeof(timerconfig));
//	timerconfig.it_value.tv_usec = ms*1000;
//	setitimer(ITIMER_REAL, &timerconfig, NULL);
//	pause();
}

void dsi_ServerSleep(uint32 ms)
{ dsi_Sleep(ms); }

extern int32 g_ScreenWidth, g_ScreenHeight;	// Console variables.

static void dsi_GetDLLModes(char *pDLLName, RMode **pMyList)
{
}


RMode* dsi_GetRenderModes()
{
return NULL;      // DAN - temporary
}

void dsi_RelinquishRenderModes(RMode *pMode)
{
}

LTRESULT dsi_GetRenderMode(RMode *pMode)
{
return LTTRUE;      // DAN - temporary
}

LTRESULT dsi_SetRenderMode(RMode *pMode)
{
return LTTRUE;      // DAN - temporary
}

LTRESULT dsi_ShutdownRender(uint32 flags)
{
return LTTRUE;      // DAN - temporary
}

LTRESULT _GetOrCopyClientFile(char *pTempPath, char *pFilename, char *pOutName, int outNameLen)
{
return LTTRUE;      // DAN - temporary
}

LTRESULT dsi_InitClientShellDE()
{
//	g_pClientMgr->m_hShellModule = NULL;
//	g_pClientMgr->m_hClientResourceModule = NULL;

	// have the user's cshell and the clientMgr exchange info
	if ((i_client_shell == NULL ))
    {
		CRITICAL_ERROR("dsys_interface", "Can't create CShell\n");
	}

	return LT_OK;
}

void dsi_OnMemoryFailure()
{
}

// Client-only functions.
void dsi_ClientSleep(uint32 ms)
{
	dsi_ServerSleep(ms);
}

LTBOOL dsi_IsInputEnabled()
{
return LTTRUE;      // DAN - temporary
}

uint16 dsi_NumKeyDowns()
{
return 0;      // DAN - temporary
}

uint16 dsi_NumKeyUps()
{
return 0;     // DAN - temporary
}

uint32 dsi_GetKeyDown(uint32 i)
{
return 0;     // DAN - temporary
}

uint32 dsi_GetKeyDownRep(uint32 i)
{
return 0;     // DAN - temporary
}

uint32 dsi_GetKeyUp(uint32 i)
{
return 0;     // DAN - temporary
}

void dsi_ClearKeyDowns()
{
}

void dsi_ClearKeyUps()
{
}

void dsi_ClearKeyMessages()
{
}

LTBOOL dsi_IsConsoleUp()
{
    return LTFALSE;
}

void dsi_SetConsoleUp(LTBOOL bUp)
{
}

LTBOOL dsi_IsClientActive()
{
	return TRUE;
}

void dsi_OnClientShutdown( char *pMsg )
{
}

char* dsi_GetDefaultWorld()
{
return NULL;     // DAN - temporary
}


#include "server_interface.h"

extern CServerMgr *g_pServerMgr;

void dsi_PrintToConsole(const char *pMsg, ...) {
    va_list marker;
    char msg[1000];

    if (g_pServerMgr && g_pServerMgr->m_pServerAppHandler) {
        va_start(marker, pMsg);
        vsnprintf(msg, 999, pMsg, marker);
        va_end(marker);

        g_pServerMgr->m_pServerAppHandler->ConsoleOutputFn(msg);
    }
}

void* dsi_GetInstanceHandle()
{
return NULL;     // DAN - temporary
}

void* dsi_GetMainWindow()
{
return NULL;     // DAN - temporary
}

LTRESULT dsi_DoErrorMessage(char *pMessage)
{
return LT_OK;      // DAN - temporary
}

void dsi_MessageBox(char *pMessage, char *pTitle)
{
}
