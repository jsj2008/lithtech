
// This header defines the main system-dependent engine functions.

#ifndef __DSYS_INTERFACE_H__
#define __DSYS_INTERFACE_H__
	
	// Include windows.h or stdafx.h.
//	#define WIN32_LEAN_AND_MEAN

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


	
	#define DSYS_INCLUDED // Define this so bdefs.h doesn't include any windows stuff.

    #ifndef __BDEFS_H__
	#include "bdefs.h"
    #endif


	// Client globals.
	#ifdef DE_CLIENT_COMPILE
		#define MAX_KEYBUFFER		100

#ifdef USE_ABSTRACT_SOUND_INTERFACES

		#define SOUND_DRIVER_NAME_LEN	32
		#define SOUND_DRIVER_NAME_ARG	"+sounddll"

#endif	// USE_ABSTRACT_SOUND_INTERFACES

		class CClientMgr;

		typedef struct
		{
			BOOL			m_bProcessWindowMessages;
			jmp_buf			m_MemoryJmp;
			HWND			m_hMainWnd;
			
			HINSTANCE		m_hInstance;
			
			char			*m_WndClassName;
			const char		*m_WndCaption;

			CClientMgr		*m_pClientMgr;

			BOOL			m_bInitializingRenderer;
			BOOL			m_bBreakOnError; // Break in dsi_OnReturnError?
			BOOL			m_bClientActive;
			BOOL			m_bLostFocus;
			BOOL			m_bAppClosing;
			BOOL			m_bDialogUp;
			BOOL			m_bRendererShutdown;	// They called ShutdownRender so we shouldn't
													// reinitialize the renderer.

			BOOL			m_bHost;
			char			*m_pGameResources;

			const char		*m_pWorldName;
			char			m_CachePath[500];

			DWORD			m_KeyDowns[MAX_KEYBUFFER];
			DWORD			m_KeyUps[MAX_KEYBUFFER];
			BOOL			m_KeyDownReps[MAX_KEYBUFFER];

			WORD			m_nKeyDowns;
			WORD			m_nKeyUps;

			BOOL			m_bIsConsoleUp;

			BOOL			m_bInputEnabled;

			char			m_ExitMessage[500];

#ifdef USE_ABSTRACT_SOUND_INTERFACES

			char			m_acSoundDriverName[ SOUND_DRIVER_NAME_LEN ];

#endif	// USE_ABSTRACT_SOUND_INTERFACES

		} ClientGlob;

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
	LTRESULT dsi_InitClientShellDE(CClientMgr *pClientMgr);
	LTRESULT dsi_LoadServerObjects(CClassMgr *pInfo);

	// Called when we run out of memory.  Shuts down everything
	// and comes up with an error message.
	void dsi_OnMemoryFailure();


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
	LTBOOL dsi_IsClientActive();
	void dsi_OnClientShutdown( char *pMsg );
	
	const char* dsi_GetDefaultWorld();


	// Sets up a message for a LTRESULT.
	LTRESULT dsi_SetupMessage(char *pMsg, int maxMsgLen, LTRESULT dResult, va_list marker);
			  
	// Puts an error message in the console if the renderer is initialized or
	// a message box otherwise.
	LTRESULT dsi_DoErrorMessage(char *pMessage);



	void dsi_PrintToConsole(char *pMsg, ...);	// Print to console.

	void* dsi_GetInstanceHandle();	// Returns an HINSTANCE.
	void* dsi_GetResourceModule();	// Returns an HINSTANCE.
	void* dsi_GetMainWindow();		// Returns an HWND.

	// Message box.
	void dsi_MessageBox(const char *pMsg, const char *pTitle);

	// Get the version info of the executable.
	// Returns LT_OK or an error.
	LTRESULT dsi_GetVersionInfo(LTVersionInfo &info);


#endif  // __DSYS_INTERFACE_H__

