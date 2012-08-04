
// This header defines the main system-dependent engine functions.

#ifndef __DSYS_INTERFACE_H__
#define __DSYS_INTERFACE_H__
    
#ifndef __WINDOWS_H__
#include <windows.h>
#define __WINDOWS_H__
#endif

#ifndef __TCHAR_H__
#include <tchar.h>
#define __TCHAR_H__
#endif

#ifndef __MMSYSTEM_H__
#include <mmsystem.h>
#define __MMSYSTEM_H__
#endif

#ifndef __SETJMP_H__
#include <setjmp.h>
#define __SETJMP_H__
#endif

// Client globals.
#ifdef DE_CLIENT_COMPILE
    #define MAX_KEYBUFFER       100

    #define SOUND_DRIVER_NAME_LEN   32
    #define SOUND_DRIVER_NAME_ARG   "+sounddll"

    class CClientMgr;

	class ClientGlob {
		public:
			ClientGlob () {
				m_bIsConsoleUp		= FALSE;
				m_bConsoleEnabled	= TRUE;
				m_bInputEnabled		= TRUE;

				m_pGameResources	= NULL;
				m_pWorldName		= NULL; }

        BOOL            m_bProcessWindowMessages;
        jmp_buf         m_MemoryJmp;
        HWND            m_hMainWnd;
        
        HINSTANCE       m_hInstance;
        
        char            *m_WndClassName;
        const char      *m_WndCaption;

        BOOL            m_bInitializingRenderer;
        BOOL            m_bBreakOnError; // Break in dsi_OnReturnError?
        BOOL            m_bClientActive;
        BOOL            m_bLostFocus;
        BOOL            m_bAppClosing;
        BOOL            m_bDialogUp;
        BOOL            m_bRendererShutdown;    // They called ShutdownRender so we shouldn't
                                                // reinitialize the renderer.

        BOOL            m_bHost;
        char            *m_pGameResources;

        const char      *m_pWorldName;
        char            m_CachePath[500];

        DWORD           m_KeyDowns[MAX_KEYBUFFER];
        DWORD           m_KeyUps[MAX_KEYBUFFER];
        BOOL            m_KeyDownReps[MAX_KEYBUFFER];

        WORD            m_nKeyDowns;
        WORD            m_nKeyUps;

        BOOL            m_bIsConsoleUp;

        BOOL            m_bInputEnabled;

        char            m_ExitMessage[500];

        char            m_acSoundDriverName[ SOUND_DRIVER_NAME_LEN ];

		bool			m_bConsoleEnabled;
    };

    extern ClientGlob g_ClientGlob;
#endif

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

#ifndef __VERSION_INFO_H__
#include "version_info.h"
#endif

class CClientMgr;
class CClassMgr;


// These are called in the startup code (in client.cpp and server_interface.cpp).
// They initialize the system-dependent modules.
// 0 = success
// 1 = couldn't load resource module (de_msg.dll).
int dsi_Init();
void dsi_Term();

// Called (in debug version) when any function uses RETURN_ERROR.
// (Good place to keep a breakpoint!)
void dsi_OnReturnError(int err);

// ClientDE implementations.
RMode* dsi_GetRenderModes();
void dsi_RelinquishRenderModes(RMode *pMode);
LTRESULT dsi_GetRenderMode(RMode *pMode);
LTRESULT dsi_SetRenderMode(RMode *pMode);
LTRESULT dsi_ShutdownRender(uint32 flags);

// Initializes the cshell and cres DLLs (copies them into a temp directory).    
LTRESULT dsi_InitClientShellDE();
LTRESULT dsi_LoadServerObjects(CClassMgr *pInfo);

// Called when we run out of memory.  Shuts down everything
// and comes up with an error message.
void dsi_OnMemoryFailure();

void dsi_ServerSleep(uint32 ms);

// Client-only things.
void dsi_ClientSleep(uint32 ms);

LTBOOL dsi_IsInputEnabled();

uint16 dsi_NumKeyDowns();
uint16 dsi_NumKeyUps();
uint32 dsi_GetKeyDown(uint32 i);
uint32 dsi_GetKeyDownRep(uint32 i);
uint32 dsi_GetKeyUp(uint32 i);
void dsi_ClearKeyDowns();
void dsi_ClearKeyUps();
void dsi_ClearKeyMessages();

LTBOOL dsi_IsConsoleUp();
void dsi_SetConsoleUp(LTBOOL bUp);
void dsi_SetConsoleEnable(bool bEnabled);
bool dsi_IsConsoleEnabled();
LTBOOL dsi_IsClientActive();
void dsi_OnClientShutdown(char *pMsg);

const char* dsi_GetDefaultWorld();


// Sets up a message for a LTRESULT.
LTRESULT dsi_SetupMessage(char *pMsg, int maxMsgLen, LTRESULT dResult, va_list marker);
          
// Puts an error message in the console if the renderer is initialized or
// a message box otherwise.
LTRESULT dsi_DoErrorMessage(const char *pMessage);

// Retrieves path to a file.  If the file is outside of a rez file, then 
// the full input path is returned.  If the file is inside a rez file,
// a temporary file is created based on the pTempPath parameter.
LTRESULT GetOrCopyFile(char const* pszTempPath, char const* pszFilename, 
					   char* pszOutName, int outNameLen, bool& bFileCopied);
LTRESULT GetOrCopyClientFile( char const* pszFilename, char* pszOutName, 
							 int outNameLen, bool& bFileCopied);


void dsi_PrintToConsole(const char *pMsg, ...);   // Print to console.

void* dsi_GetInstanceHandle();  // Returns an HINSTANCE.
void* dsi_GetResourceModule();  // Returns an HINSTANCE.
void* dsi_GetMainWindow();      // Returns an HWND.

// Message box.
void dsi_MessageBox(const char *pMsg, const char *pTitle);

// Get the version info of the executable.
// Returns LT_OK or an error.
LTRESULT dsi_GetVersionInfo(LTVersionInfo &info);


#endif  // __DSYS_INTERFACE_H__

