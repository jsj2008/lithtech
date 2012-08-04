#include "bdefs.h"

#include "stringmgr.h"
#include "render.h"
#include "version_resource.h"
#include "appresource.h"
#include "sysfile.h"
#include "de_objects.h"
#include "servermgr.h"
#include "classbind.h"
#include "bindmgr.h"
#include "console.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

//IInstanceHandleClient
static IInstanceHandleClient *instance_handle_client;
define_holder(IInstanceHandleClient, instance_handle_client);

//IServerShell game server shell object.
#include "iservershell.h"
static IServerShell *i_server_shell;
define_holder(IServerShell, i_server_shell);

//IInstanceHandleServer
static IInstanceHandleServer *instance_handle_server;
define_holder(IInstanceHandleServer, instance_handle_server);





void dsi_OnReturnError(int err) {
#ifdef DE_CLIENT_COMPILE
    if (g_ClientGlob.m_bBreakOnError) {
        DebugBreak();
    }
#endif
}


LTBOOL g_bComInitialized = LTFALSE;
HINSTANCE g_hResourceModule = LTNULL;
HINSTANCE g_hModuleInstanceHandle = LTNULL;

#define DO_CODE(x)  LT_##x, IDS_##x

struct LTSysResultString {
    unsigned long dResult;
    unsigned long string_id;
};

LTSysResultString g_StringMap[] = {
    DO_CODE(SERVERERROR),
    DO_CODE(ERRORLOADINGRENDERDLL),
    DO_CODE(MISSINGWORLDMODEL),
    DO_CODE(CANTLOADGAMERESOURCES),
    DO_CODE(CANTINITIALIZEINPUT),
    DO_CODE(MISSINGSHELLDLL),
    DO_CODE(INVALIDSHELLDLL),
    DO_CODE(INVALIDSHELLDLLVERSION),
    DO_CODE(CANTCREATECLIENTSHELL),
    DO_CODE(UNABLETOINITSOUND),
    DO_CODE(MISSINGWORLDFILE),
    DO_CODE(INVALIDWORLDFILE),
    DO_CODE(INVALIDSERVERPACKET),
    DO_CODE(MISSINGSPRITEFILE),
    DO_CODE(INVALIDSPRITEFILE),
    DO_CODE(MISSINGMODELFILE),
    DO_CODE(INVALIDMODELFILE),
    DO_CODE(UNABLETORESTOREVIDEO),
    DO_CODE(MISSINGCLASS),
    DO_CODE(CANTCREATESERVERSHELL),
    DO_CODE(INVALIDOBJECTDLLVERSION),
    DO_CODE(ERRORINITTINGNETDRIVER),
    DO_CODE(USERCANCELED),
    DO_CODE(CANTRESTOREOBJECT),
    DO_CODE(NOGAMERESOURCES),
    DO_CODE(ERRORCOPYINGFILE),
    DO_CODE(INVALIDNETVERSION)
};

#define STRINGMAP_SIZE (sizeof(g_StringMap) / sizeof(g_StringMap[0]))



// --------------------------------------------------------------- //
// Internal functions.
// --------------------------------------------------------------- //

static LTBOOL dsi_LoadResourceModule() {
    g_hResourceModule = LoadLibrary("ltmsg.dll");

#if 0
#ifdef LITHTECH_ESD
		if (!g_hResourceModule)
		{
			char sFullPath[256] = { "" };
			DWORD dwRet = GetModuleFileName(NULL, sFullPath, 255);
			if (dwRet != 0)
			{
				char sDir[256]  = { "" };
				char sDrive[32] = { "" };
				_splitpath(sFullPath, sDrive, sDir, NULL, NULL);
				if (sDir[0] != '\0')
				{
					char sLoad[256];
					wsprintf(sLoad, "%s%s%s", sDrive, sDir, "ltmsg.dll");
					g_hResourceModule = LoadLibrary(sLoad);
				}
			}
		}
#endif // LITHTECH_ESD
#endif // 0

    return !!g_hResourceModule;
}

static void dsi_UnloadResourceModule() {
    if (g_hResourceModule) {
        FreeLibrary(g_hResourceModule);
        g_hResourceModule = LTNULL;
    }
}


LTRESULT dsi_SetupMessage(char *pMsg, int maxMsgLen, LTRESULT dResult, va_list marker) {
    int i;
    unsigned long resultCode, stringID;
    LTBOOL bFound;
    uint32 args[4], nBytes;
    char tempBuffer[500];

    pMsg[0] = 0;

    if (!g_hResourceModule) {
        LTSNPrintF(pMsg, maxMsgLen, "<missing resource DLL>");
        return LT_ERROR;
    }

    // Try to find the error code.
    bFound = LTFALSE;
    resultCode = ERROR_CODE(dResult);
    for (i=0; i < STRINGMAP_SIZE; i++) {
        if (g_StringMap[i].dResult == resultCode) {
            bFound = LTTRUE;
            stringID = g_StringMap[i].string_id;
            break;
        }
    }

    if (bFound) {
        nBytes = LoadString(g_hResourceModule, stringID, tempBuffer, sizeof(tempBuffer)-1);
        if (nBytes > 0) {
            // Format it.
            nBytes = FormatMessage(FORMAT_MESSAGE_FROM_STRING, tempBuffer, 0, 0, pMsg, maxMsgLen, &marker);

            if (nBytes > 0)
                return LT_OK;
        }
    }

    // Ok, format the default message.
    args[0] = resultCode;
    nBytes = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY,
        g_hResourceModule, IDS_GENERIC_ERROR, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        pMsg, maxMsgLen, (va_list*)args);

    if (nBytes > 0) {
        return LT_OK;
    }
    else {
        LTSNPrintF(pMsg, maxMsgLen, "<invalid resource DLL>");
        return LT_ERROR;
    }
}


// --------------------------------------------------------------- //
// External functions.
// --------------------------------------------------------------- //

int dsi_Init() 
{
    HRESULT hResult;

    hResult = CoInitialize(LTNULL);
    if (SUCCEEDED(hResult)) {
        g_bComInitialized = LTTRUE;
    }
    else {
        return 1;
    }

    dm_Init();  // Memory manager.
    str_Init(); // String manager.
    df_Init();  // File manager.

    if (dsi_LoadResourceModule()) {
        return 0;
    }
    else {
        dsi_Term();
        return 1;
    }
}


void dsi_Term() 
{
    df_Term();
    str_Term();
    dm_Term();

    dsi_UnloadResourceModule();
    
    if (g_bComInitialized) {
        CoUninitialize();
        g_bComInitialized = LTFALSE;
    }
}


void* dsi_GetResourceModule() {
    return g_hResourceModule;
}


LTRESULT GetOrCopyFile( char const* pszFilename, 
					   char* pszOutName, int outNameLen, bool& bFileCopied )
{
    HLTFileTree *hTree;
    LTRESULT dResult;
    
	bFileCopied = false;
    if (server_filemgr->DoesFileExist(pszFilename, &hTree, LTNULL) && 
        (df_GetTreeType(hTree) == DosTree)) 
    {
        int status = df_GetFullFilename(hTree, ( char* )pszFilename, pszOutName, outNameLen);
        ASSERT(status != 0);
        return LT_OK;
    }

    // Get the temp path.
	char szTempPath[MAX_PATH*2] = "";
    if( !GetTempPath( sizeof( szTempPath ), szTempPath )) 
		{
        strcpy( szTempPath, ".\\" );
        }

	// Get a temp filename.
    char szTempFilename[MAX_PATH*2] = "";
	if( !GetTempFileName( szTempPath, pszFilename, 0, szTempFilename ))
				{
		strcpy( szTempFilename, pszFilename );
                }

	// Copy the file.
    dResult = server_filemgr->CopyFile( pszFilename, szTempFilename );
    if( dResult != LT_OK )
	{
                    return dResult;
                }

	bFileCopied = true;
    LTStrCpy( pszOutName, szTempFilename, outNameLen );

    return LT_OK;
        }

LTRESULT dsi_LoadServerObjects(CClassMgr *pClassMgr) 
{
    char fileName[256];
    int status;
    char *pDLLName;

    pDLLName = "object.lto";

    //Copy the object.lto file out of the res so we can run it.
	bool bFileCopied = false;
    if (GetOrCopyFile( pDLLName, fileName, sizeof(fileName), bFileCopied) != LT_OK) 
	{
        sm_SetupError(LT_ERRORCOPYINGFILE, pDLLName);
        RETURN_ERROR_PARAM(1, LoadServerObjects, LT_ERRORCOPYINGFILE, pDLLName);
    }

    //load the object.lto DLL.
    int version;
    status = cb_LoadModule(fileName, bFileCopied, pClassMgr->m_ClassModule, &version);

    //check for errors.
    if (status == CB_CANTFINDMODULE) {
        sm_SetupError(LT_INVALIDOBJECTDLL, pDLLName);
        RETURN_ERROR_PARAM(1, LoadObjectsInDirectory, LT_INVALIDOBJECTDLL, pDLLName);
    }
    else if (status == CB_NOTCLASSMODULE) {
        sm_SetupError(LT_INVALIDOBJECTDLL, pDLLName);
        RETURN_ERROR_PARAM(1, LoadObjectsInDirectory, LT_INVALIDOBJECTDLL, pDLLName);
    }
    else if (status == CB_VERSIONMISMATCH) {
        sm_SetupError(LT_INVALIDOBJECTDLLVERSION, pDLLName, version, SERVEROBJ_VERSION);
        RETURN_ERROR_PARAM(1, LoadObjectsInDirectory, LT_INVALIDOBJECTDLLVERSION, pDLLName);
    }

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
    
    return LT_OK;
}

void dsi_ServerSleep(uint32 ms) {
    if (ms > 0) {
        Sleep(ms);
    }
}

#ifdef DE_CLIENT_COMPILE
#include "clientmgr.h"

extern int32 g_ScreenWidth, g_ScreenHeight; // Console variables.

static void dsi_GetDLLModes(char *pDLLName, RMode **pMyList) {
    RMode *pMyMode;
    RMode *pListHead, *pCur;

    pListHead = rdll_GetSupportedModes();
    
    // Copy the mode list.
    pCur = pListHead;
    while (pCur)
    {
        LT_MEM_TRACK_ALLOC(pMyMode = (RMode*)dalloc(sizeof(RMode)),LT_MEM_TYPE_MISC);
        memcpy(pMyMode, pCur, sizeof(RMode));

        pMyMode->m_pNext = *pMyList;
        *pMyList = pMyMode;
        
        pCur = pCur->m_pNext;
    }                       

    rdll_FreeModeList(pListHead);
}


RMode* dsi_GetRenderModes() {
    RMode *pList = LTNULL;

    dsi_GetDLLModes("integrated", &pList);

 
    return pList;
}


void dsi_RelinquishRenderModes(RMode *pMode) {
    RMode *pCur, *pNext;

    pCur = pMode;
    while (pCur) {
        pNext = pCur->m_pNext;
        dfree(pCur);
        pCur = pNext;
    }
}


LTRESULT dsi_GetRenderMode(RMode *pMode) {
    memcpy(pMode, &g_RMode, sizeof(RMode));
    return LT_OK;
}


LTRESULT dsi_SetRenderMode(RMode *pMode) {
    RMode currentMode;
    char message[256];
    
    if (r_TermRender(1, false) != LT_OK) {
        dsi_SetupMessage(message, sizeof(message)-1, LT_UNABLETORESTOREVIDEO, LTNULL);
        dsi_OnClientShutdown(message);
        RETURN_ERROR(0, SetRenderMode, LT_UNABLETORESTOREVIDEO);
    }

    memcpy(&currentMode, &g_RMode, sizeof(RMode));

    // Try to set the new mode.
    if (r_InitRender(pMode) != LT_OK) {
        // Ok, try to restore the old mode.
        if (r_InitRender(&currentMode) != LT_OK) {
            //dsi_SetupMessage(message, sizeof(message)-1, LT_UNABLETORESTOREVIDEO, LTNULL);
            //dsi_OnClientShutdown(message);
            RETURN_ERROR(0, SetRenderMode, LT_UNABLETORESTOREVIDEO);
        }

        RETURN_ERROR(1, SetRenderMode, LT_KEPTSAMEMODE);
    }

    g_ClientGlob.m_bRendererShutdown = LTFALSE;
    return LT_OK;
}


LTRESULT dsi_ShutdownRender(uint32 flags) {
    r_TermRender(1, true);

    if (flags & RSHUTDOWN_MINIMIZEWINDOW) {
        ShowWindow(g_ClientGlob.m_hMainWnd, SW_MINIMIZE);
    }

    if (flags & RSHUTDOWN_HIDEWINDOW) {
        ShowWindow(g_ClientGlob.m_hMainWnd, SW_HIDE);
    }

    g_ClientGlob.m_bRendererShutdown = LTTRUE;
    return LT_OK;
}


LTRESULT GetOrCopyClientFile( char const* pszFilename, 
							 char* pszOutName, int outNameLen, bool& bFileCopied ) 
{
    FileIdentifier *pIdent;
    FileRef ref;
    LTRESULT dResult;

	bFileCopied = false;
    ref.m_FileType = FILE_ANYFILE;
    ref.m_pFilename = pszFilename;
    pIdent = client_file_mgr->GetFileIdentifier(&ref, TYPECODE_DLL);
    if (pIdent && ((df_GetTreeType(pIdent->m_hFileTree) == DosTree))) 
	{
        int status = df_GetFullFilename(pIdent->m_hFileTree, ( char* )pszFilename, pszOutName, outNameLen);
        ASSERT(status != 0);
        return LT_OK;
    }

    // Get the temp path.
	char szTempPath[MAX_PATH*2] = "";
    if( !GetTempPath( sizeof( szTempPath ), szTempPath )) 
		{
        strcpy( szTempPath, ".\\" );
        }

	// Get a temp filename.
    char szTempFilename[MAX_PATH*2] = "";
	if( !GetTempFileName( szTempPath, pszFilename, 0, szTempFilename ))
				{
		strcpy( szTempFilename, pszFilename );
                }

	// Copy the file.
    dResult = client_file_mgr->CopyFile( pszFilename, szTempFilename );
    if( dResult != LT_OK )
				{
                    return dResult;
                }

	bFileCopied = true;
    LTStrCpy( pszOutName, szTempFilename, outNameLen );

    return LT_OK;
            }



LTRESULT dsi_InitClientShellDE() 
{
    char fileName[MAX_PATH];
    int status;
    LTRESULT dResult;

    g_pClientMgr->m_hClientResourceModule = LTNULL;
    g_pClientMgr->m_hLocalizedClientResourceModule = LTNULL;
    g_pClientMgr->m_hShellModule = LTNULL;

    // Setup the cshell.dll file.
	bool bFileCopied = false;
    dResult = GetOrCopyClientFile( "cshell.dll", fileName, sizeof(fileName), bFileCopied );
    if (dResult != LT_OK) {
        g_pClientMgr->SetupError(LT_ERRORCOPYINGFILE, "cshell.dll");
        RETURN_ERROR_PARAM(1, InitClientShellDE, LT_ERRORCOPYINGFILE, "cshell.dll");
    }

    //load the DLL.
    status = bm_BindModule(fileName, bFileCopied, g_pClientMgr->m_hShellModule);

    //check if it loaded correctly.
    if (status == BIND_CANTFINDMODULE) {
        g_pClientMgr->SetupError(LT_MISSINGSHELLDLL, "cshell.dll");
        RETURN_ERROR(1, InitClientShellDE, LT_MISSINGSHELLDLL);
    }

    //check if we now have the IClientShell interface instantiated.
    if (i_client_shell == NULL) {
        g_pClientMgr->SetupError(LT_INVALIDSHELLDLL, "cshell.dll");
        RETURN_ERROR(1, InitClientShellDE, LT_INVALIDSHELLDLL);
    }

    //
    // Try to setup cres.dll.
    //

    //copy the file out of the res file.
	bFileCopied = false;
    dResult = GetOrCopyClientFile( "cres.dll", fileName, sizeof(fileName), bFileCopied );
    if (dResult != LT_OK) 
	{
        g_pClientMgr->SetupError(LT_ERRORCOPYINGFILE, "cres.dll");
        RETURN_ERROR_PARAM(1, InitClientShellDE, LT_ERRORCOPYINGFILE, "cres.dll");
    }

    //load the DLL.
    status = bm_BindModule(fileName, bFileCopied, g_pClientMgr->m_hClientResourceModule);

    //check if it was loaded.
    if (status == BIND_CANTFINDMODULE) {
        //unload cshell.dll.
        bm_UnbindModule(g_pClientMgr->m_hShellModule);
        g_pClientMgr->m_hShellModule = LTNULL;

        g_pClientMgr->SetupError(LT_INVALIDSHELLDLL, "cres.dll");
        RETURN_ERROR_PARAM(1, InitClientShellDE, LT_INVALIDSHELLDLL, "cres.dll");
    }

    //let the dll know it's instance handle.
    if (instance_handle_client != NULL) {
        instance_handle_client->SetInstanceHandle(g_pClientMgr->m_hShellModule);
    }

    return LT_OK;
}


 
void dsi_OnMemoryFailure() {
	ASSERT(!"Out of memory");
    longjmp(g_ClientGlob.m_MemoryJmp, 1);
}


 
// Client-only functions.
void dsi_ClientSleep(uint32 ms) {
    if (ms > 0) {
        Sleep(ms);
    }
}


 
LTBOOL dsi_IsInputEnabled() {
    return g_ClientGlob.m_bInputEnabled;
}


 
uint16 dsi_NumKeyDowns() {
    return g_ClientGlob.m_nKeyDowns;
}


 
uint16 dsi_NumKeyUps() {
    return g_ClientGlob.m_nKeyUps;
}


 
uint32 dsi_GetKeyDown(uint32 i) {
    ASSERT(i < MAX_KEYBUFFER);
    return g_ClientGlob.m_KeyDowns[i];
}


 
uint32 dsi_GetKeyDownRep(uint32 i) {
    ASSERT(i < MAX_KEYBUFFER);
    return g_ClientGlob.m_KeyDownReps[i];
}


uint32 dsi_GetKeyUp(uint32 i) {
    ASSERT(i < MAX_KEYBUFFER);
    return g_ClientGlob.m_KeyUps[i];
}

void dsi_ClearKeyDowns() {
    g_ClientGlob.m_nKeyDowns=0;
}

void dsi_ClearKeyUps() {
    g_ClientGlob.m_nKeyUps=0;
}

void dsi_ClearKeyMessages() {
    MSG msg;
    int i;

    for (i = 0; i < 500; i++) {
        if (!PeekMessage(&msg, g_ClientGlob.m_hMainWnd, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE)) {
            break;
        }
    }

    for (i = 0; i < 500; i++) {
        if (!PeekMessage(&msg, g_ClientGlob.m_hMainWnd, WM_KEYUP, WM_KEYUP, PM_REMOVE)) {
            break;
        }
    }
}

LTBOOL dsi_IsConsoleUp() {
	if (dsi_IsConsoleEnabled() == LTFALSE) 
		return LTFALSE;
    return g_ClientGlob.m_bIsConsoleUp;
}

void dsi_SetConsoleUp(LTBOOL bUp) {
    g_ClientGlob.m_bIsConsoleUp = bUp;
}

void dsi_SetConsoleEnable (bool bEnabled)
{
	g_ClientGlob.m_bConsoleEnabled = bEnabled;
	if (bEnabled == LTFALSE)
	{
		//	put console away if we disabled it
		dsi_SetConsoleUp (LTFALSE);
	}
}

bool dsi_IsConsoleEnabled ()
{
	return	g_ClientGlob.m_bConsoleEnabled;
}

LTBOOL dsi_IsClientActive() {
    return g_ClientGlob.m_bClientActive;
}

void dsi_OnClientShutdown(char *pMsg) {
    if (pMsg && pMsg[0]) {
        LTStrCpy(g_ClientGlob.m_ExitMessage, pMsg, sizeof(g_ClientGlob.m_ExitMessage));
        g_ClientGlob.m_ExitMessage[ sizeof(g_ClientGlob.m_ExitMessage) - 1 ] = '\0';
    }
    else {
        g_ClientGlob.m_ExitMessage[0] = '\0';
    }
	
    if (g_ClientGlob.m_bProcessWindowMessages) {
        PostQuitMessage(0);
    }
}

const char* dsi_GetDefaultWorld() {
    return g_ClientGlob.m_pWorldName;
}



void dsi_PrintToConsole(const char *pMsg, ...) {
    char msg[500];
    va_list marker;
	
    va_start(marker, pMsg);
    LTVSNPrintF(msg, sizeof(msg), pMsg, marker);
    va_end(marker);
	
	int32 len = strlen(msg);
	if (msg[len-1] != '\n')
	{
		msg[len] = '\n';
		msg[len+1] = '\0';
	}

    con_PrintString(CONRGB(255,255,0), 0, msg);
}

void* dsi_GetInstanceHandle() {
    return (void*)g_ClientGlob.m_hInstance;
}

void* dsi_GetMainWindow() {
    return (void*)g_ClientGlob.m_hMainWnd;
}

LTRESULT dsi_DoErrorMessage(const char *pMessage) {
    con_PrintString(CONRGB(255,255,255), 0, pMessage);
    
    if (!g_Render.m_bInitted) {
        MessageBox(g_ClientGlob.m_hMainWnd, pMessage, g_ClientGlob.m_WndCaption, MB_OK);
    }

    return LT_OK;
}

void dsi_MessageBox(const char *pMessage, const char *pTitle) {
    int i;
    i = MessageBox(g_ClientGlob.m_hMainWnd, pMessage, pTitle, MB_OK);
}

LTRESULT dsi_GetVersionInfo(LTVersionInfo &info) {
    return GetLTExeVersion(g_ClientGlob.m_hInstance, info);
}



//----------------------------------------------------------------
#else //----------------------------------------------------------
//----------------------------------------------------------------





#include "server_interface.h"

extern CServerMgr *g_pServerMgr;


void dsi_PrintToConsole(const char *pMsg, ...) {
    va_list marker;
    char msg[1000];

    if (g_pServerMgr && g_pServerMgr->m_pServerAppHandler) {
        va_start(marker, pMsg);
        _vsnprintf(msg, 999, pMsg, marker);
        va_end(marker);

        g_pServerMgr->m_pServerAppHandler->ConsoleOutputFn(msg);
    }
}

void* dsi_GetInstanceHandle() {
    return (void*)g_hModuleInstanceHandle;
}

void* dsi_GetMainWindow() {
    return LTNULL;
}

void dsi_OnMemoryFailure() {
    // Let the server app shutdown.
    if (g_pServerMgr && g_pServerMgr->m_pServerAppHandler) {
        g_pServerMgr->m_pServerAppHandler->OutOfMemory();
    }

    // They better have exited.. make sure.
    MessageBox(GetDesktopWindow(), "Out of memory", "LithTech", MB_OK);
    exit(2222);
}


#endif


