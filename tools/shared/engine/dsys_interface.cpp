
// This module implements all the dsi_interface functions.

#include "bdefs.h"
#include "dsys_interface.h"
//#include "packet.h"
#include "version_resource.h"



void dsi_OnReturnError(int err)
{
#ifdef DE_CLIENT_COMPILE
	if(g_ClientGlob.m_bBreakOnError)
	{
		DebugBreak();
	}
#endif
}


#ifdef DIRECTEDITOR_BUILD

	// DEdit wants this..
	void dsi_PrintToConsole(char *pMsg, ...)
	{
	}

	void dsi_OnMemoryFailure()
	{
	}


#else
	#include "servermgr.h"
	#include "appresource.h"
	
	LTBOOL g_bComInitialized = LTFALSE;
	HINSTANCE g_hResourceModule = LTNULL;
	HINSTANCE g_hModuleInstanceHandle = LTNULL;

	#define DO_CODE(x)	LT_##x, IDS_##x

	struct LTSysResultString
	{
		unsigned long dResult;
		unsigned long string_id;
	};
	
	LTSysResultString g_StringMap[] =
	{
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

	static LTBOOL dsi_LoadResourceModule()
	{
	//	ASSERT(!g_hResourceModule);

		g_hResourceModule = LoadLibrary("ltmsg.dll");
		return !!g_hResourceModule;
	}

	static void dsi_UnloadResourceModule()
	{
		if(g_hResourceModule)
		{
			FreeLibrary(g_hResourceModule);
			g_hResourceModule = LTNULL;
		}
	}


	LTRESULT dsi_SetupMessage(char *pMsg, int maxMsgLen, LTRESULT dResult, va_list marker)
	{
		int i;
		unsigned long resultCode, stringID;
		LTBOOL bFound;
		uint32 args[4], nBytes;
		char tempBuffer[500];

		pMsg[0] = 0;

		if(!g_hResourceModule)
		{
			sprintf(pMsg, "<missing resource DLL>");
			return LT_ERROR;
		}

		// Try to find the error code.
		bFound = LTFALSE;
		resultCode = ERROR_CODE(dResult);
		for(i=0; i < STRINGMAP_SIZE; i++)
		{
			if(g_StringMap[i].dResult == resultCode)
			{
				bFound = LTTRUE;
				stringID = g_StringMap[i].string_id;
				break;
			}
		}

		if(bFound)
		{
			nBytes = LoadString(g_hResourceModule, stringID, tempBuffer, sizeof(tempBuffer)-1);
			if(nBytes > 0)
			{
				// Format it.
				nBytes = FormatMessage(FORMAT_MESSAGE_FROM_STRING,
					tempBuffer,
					0,
					0,
					pMsg,
					maxMsgLen,
					&marker
					);

				if(nBytes > 0)
					return LT_OK;
			}
		}

		// Ok, format the default message.
		args[0] = resultCode;
		nBytes = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_ARGUMENT_ARRAY,
			g_hResourceModule,
			IDS_GENERIC_ERROR,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			pMsg,
			maxMsgLen,
			(va_list*)args
			);

		if(nBytes > 0)
		{
			return LT_OK;
		}
		else
		{
			sprintf(pMsg, "<invalid resource DLL>");
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
		if(SUCCEEDED(hResult))
		{
			g_bComInitialized = LTTRUE;
		}
		else
		{
			return 1;
		}

		dm_Init();	// Memory manager.
		str_Init();	// String manager.
		df_Init();	// File manager.
		obj_Init();	// Object manager.
		packet_Init();

		if(dsi_LoadResourceModule())
		{
			return 0;
		}
		else
		{
			dsi_Term();
			return 1;
		}
	}


	void dsi_Term()
	{
		packet_Term();
		obj_Term();
		df_Term();
		str_Term();
		dm_Term();

		dsi_UnloadResourceModule();
		
		if(g_bComInitialized)
		{
			CoUninitialize();
			g_bComInitialized = LTFALSE;
		}
	}


	void* dsi_GetResourceModule()
	{
		return g_hResourceModule;
	}


	LTRESULT _GetOrCopyFile(CServerMgr *pMgr, char *pTempPath, char *pFilename, char *pOutName, int outNameLen)
	{
		ServerFileTree *pTree;
		int status;
		LTRESULT dResult;
		char tempFilename[MAX_PATH];
		
		if(sf_DoesFileExist(&pMgr->m_FileMgr, pFilename, &pTree, LTNULL) && (df_GetTreeType(pTree->m_hFileTree) == DosTree))
		{
			status = df_GetFullFilename(pTree->m_hFileTree, pFilename, pOutName, outNameLen);
			ASSERT(status != 0);
			return LT_OK;
		}
		else
		{		
			sprintf(pOutName, "%s%s", pTempPath, pFilename);
			dResult = sf_CopyFile(&pMgr->m_FileMgr, pFilename, pOutName);
			if(dResult == LT_OK)
			{
				return LT_OK;
			}
			else
			{
				// Try a temp file.
				if(GetTempFileName(pTempPath, pFilename, 0, tempFilename))
				{
					dResult = sf_CopyFile(&pMgr->m_FileMgr, pFilename, tempFilename);
					if(dResult == LT_OK)
					{
						strncpy(pOutName, tempFilename, outNameLen-1);
						return LT_OK;
					}
					else
					{
						return dResult;
					}
				}
				else
				{
					return dResult;
				}
			}
		}
	}


	LTRESULT dsi_LoadServerObjects(CClassMgr *pInfo)
	{
		char fileName[256], path[200];
		int status, version;
		CServerMgr *pMgr;
		char *pDLLName;


		pMgr = pInfo->m_pServerMgr;

		// Get the temp path.
		if(!GetTempPath(sizeof(path), path))
		{
			strcpy(path, ".\\");
		}

		pDLLName = "object.lto";

		// Setup the object DLL.
		if(_GetOrCopyFile(pMgr, path, pDLLName, fileName, sizeof(fileName)) == LT_OK)
		{
		}
		else
		{
			sm_SetupError(pMgr, LT_ERRORCOPYINGFILE, pDLLName);
			RETURN_ERROR_PARAM(1, LoadServerObjects, LT_ERRORCOPYINGFILE, pDLLName);
		}

		status = cb_LoadModule(fileName, pMgr->m_pServerDE, &pInfo->m_hClassModule, &version);
		if(status == CB_NOERROR)
		{
			// Load the shell module (out of the same file).
			status = sb_LoadModule(fileName, "ServerShell", 
				&pInfo->m_hShellModule, SERVERSHELL_VERSION, &version);
			if(status == SB_CANTFINDMODULE)
			{
				sm_SetupError(pMgr, LT_MISSINGSHELLDLL, fileName);
				RETURN_ERROR_PARAM(1, LoadServerObjects, LT_MISSINGSHELLDLL, pDLLName);
			}
			else if(status == SB_NOTSHELLMODULE)
			{
				sm_SetupError(pMgr, LT_INVALIDSHELLDLL, fileName);
				RETURN_ERROR_PARAM(1, LoadServerObjects, LT_INVALIDSHELLDLL, pDLLName);
			}
			else if(status == SB_VERSIONMISMATCH)
			{
				sm_SetupError(pMgr, LT_INVALIDSHELLDLLVERSION, fileName, version, SERVERSHELL_VERSION);
				RETURN_ERROR_PARAM(1, LoadServerObjects, LT_INVALIDSHELLDLLVERSION, pDLLName);
			}
			else
			{
				// Cool!
				sb_GetShellFunctions(pInfo->m_hShellModule, &pInfo->m_CreateShellFn, &pInfo->m_DeleteShellFn);

				// Get sres.dll.
				if((_GetOrCopyFile(pMgr, path, "sres.dll", fileName, sizeof(fileName)) == LT_OK)
					&& (bm_BindModule(fileName, &pInfo->m_hServerResourceModule) == BIND_NOERROR))
				{
				}
				else
				{
					sb_UnloadModule(pInfo->m_hShellModule);
					pInfo->m_hShellModule = LTNULL;

					sm_SetupError(pMgr, LT_ERRORCOPYINGFILE, "sres.dll");
					RETURN_ERROR_PARAM(1, LoadServerObjects, LT_ERRORCOPYINGFILE, "sres.dll");
				}
				
				return LT_OK;
			}
		}
		else if(status == CB_CANTFINDMODULE)
		{
			sm_SetupError(pMgr, LT_INVALIDOBJECTDLL, pDLLName);
			RETURN_ERROR_PARAM(1, LoadObjectsInDirectory, LT_INVALIDOBJECTDLL, pDLLName);
		}
		else if(status == CB_NOTCLASSMODULE)
		{
			sm_SetupError(pMgr, LT_INVALIDOBJECTDLL, pDLLName);
			RETURN_ERROR_PARAM(1, LoadObjectsInDirectory, LT_INVALIDOBJECTDLL, pDLLName);
		}
		else ///if(status == CB_VERSIONMISMATCH)
		{
			sm_SetupError(pMgr, LT_INVALIDOBJECTDLLVERSION, pDLLName, version, SERVEROBJ_VERSION);
			RETURN_ERROR_PARAM(1, LoadObjectsInDirectory, LT_INVALIDOBJECTDLLVERSION, pDLLName);
		}
	}



	#ifdef DE_CLIENT_COMPILE
		#include "console.h"
		#include "consolecommands.h"
		#include "clientmgr.h"
		#include "sysclientde_impl.h"
		#include "engine_surfaceeffects.h"
		#include <io.h>

		extern int32 g_ScreenWidth, g_ScreenHeight;	// Console variables.
		extern CClientMgr *g_pClientMgr; // For the ILTClient implementations.


		static void dsi_GetDLLModes(char *pDLLName, RMode **pMyList)
		{
			HINSTANCE hDLL;
			GetSupportedModesFn getModes;
			FreeModeListFn freeModes;
			RMode *pMyMode;
			RMode *pListHead, *pCur;

			hDLL = LoadLibrary(pDLLName);
			if(hDLL)
			{
				getModes = (GetSupportedModesFn)GetProcAddress(hDLL, "GetSupportedModes");
				freeModes = (FreeModeListFn)GetProcAddress(hDLL, "FreeModeList");
				
				if(getModes && freeModes)
				{
					pListHead = getModes();
					
					// Copy the mode list.
					pCur = pListHead;
					while(pCur)
					{
						pMyMode = (RMode*)dalloc(sizeof(RMode));
						memcpy(pMyMode, pCur, sizeof(RMode));
						strncpy(pMyMode->m_RenderDLL, pDLLName, sizeof(pMyMode->m_RenderDLL)-1);

						pMyMode->m_pNext = *pMyList;
						*pMyList = pMyMode;
						
						pCur = pCur->m_pNext;
					}						
				
					freeModes(pListHead);
				}

				FreeLibrary(hDLL);
			}
		}


		RMode* dsi_GetRenderModes()
		{
			RMode *pList;
			_finddata_t data;
			long handle, status;


			pList = LTNULL;

			handle = status = _findfirst("*.ren", &data);
			while(status != -1)
			{
				if(!(data.attrib & _A_SUBDIR))
				{
					dsi_GetDLLModes(data.name, &pList);
				}

				status = _findnext(handle, &data);
			}

			return pList;
		}


		void dsi_RelinquishRenderModes(RMode *pMode)
		{
			RMode *pCur, *pNext;

			pCur = pMode;
			while(pCur)
			{
				pNext = pCur->m_pNext;
				dfree(pCur);
				pCur = pNext;
			}
		}


		LTRESULT dsi_GetRenderMode(RMode *pMode)
		{
			memcpy(pMode, &g_RMode, sizeof(RMode));
			return LT_OK;
		}


		LTRESULT dsi_SetRenderMode(RMode *pMode)
		{
			RMode currentMode;
			char message[256];
			
			if(r_TermRender(g_pClientMgr, 1, false) != LT_OK)
			{
				dsi_SetupMessage(message, sizeof(message)-1, LT_UNABLETORESTOREVIDEO, LTNULL);
				dsi_OnClientShutdown( message );
				RETURN_ERROR(0, SetRenderMode, LT_UNABLETORESTOREVIDEO);
			}

			memcpy(&currentMode, &g_RMode, sizeof(RMode));

			// Try to set the new mode.
			if(r_InitRender(g_pClientMgr, pMode) != LT_OK)
			{
				// Ok, try to restore the old mode.
				if(r_InitRender(g_pClientMgr, &currentMode) != LT_OK)
				{
					//dsi_SetupMessage(message, sizeof(message)-1, LT_UNABLETORESTOREVIDEO, LTNULL);
					//dsi_OnClientShutdown( message );
					RETURN_ERROR(0, SetRenderMode, LT_UNABLETORESTOREVIDEO);
				}

				RETURN_ERROR(1, SetRenderMode, LT_KEPTSAMEMODE);
			}

			g_ClientGlob.m_bRendererShutdown = LTFALSE;
			return LT_OK;
		}

		
		LTRESULT dsi_ShutdownRender(uint32 flags)
		{
			r_TermRender(g_pClientMgr, 1, false);

			if(flags & RSHUTDOWN_MINIMIZEWINDOW)
			{
				ShowWindow(g_ClientGlob.m_hMainWnd, SW_MINIMIZE);
			}

	   		if(flags & RSHUTDOWN_HIDEWINDOW)
			{
				ShowWindow(g_ClientGlob.m_hMainWnd, SW_HIDE);
			}

			g_ClientGlob.m_bRendererShutdown = LTTRUE;
			return LT_OK;
		}

	
		LTRESULT _GetOrCopyClientFile(CClientMgr *pMgr, char *pTempPath, char *pFilename, char *pOutName, int outNameLen)
		{
			FileIdentifier *pIdent;
			FileRef ref;
			int status;
			LTRESULT dResult;
			char tempFilename[MAX_PATH];

						
			ref.m_FileType = FILE_ANYFILE;
			ref.m_pFilename = pFilename;
			pIdent = cf_GetFileIdentifier(pMgr->m_hFileMgr, &ref, TYPECODE_DLL);
			if(pIdent && ((df_GetTreeType(pIdent->m_hFileTree) == DosTree)))
			{
				status = df_GetFullFilename(pIdent->m_hFileTree, pFilename, pOutName, outNameLen);
				ASSERT(status != 0);
				return LT_OK;
			}
			else
			{
				sprintf(pOutName, "%s%s", pTempPath, pFilename);
				dResult = cf_CopyFile(pMgr->m_hFileMgr, pFilename, pOutName);
				if(dResult == LT_OK)
				{
					return LT_OK;
				}
				else
				{
					// Try a temp file.
					if(GetTempFileName(pTempPath, pFilename, 0, tempFilename))
					{
						dResult = cf_CopyFile(pMgr->m_hFileMgr, pFilename, tempFilename);
						if(dResult == LT_OK)
						{
							strncpy(pOutName, tempFilename, outNameLen-1);
							return LT_OK;
						}
						else
						{
							return dResult;
						}
					}
					else
					{
						return dResult;
					}
				}
			}
		}


		LTRESULT dsi_InitClientShellDE(CClientMgr *pClientMgr)
		{
			char fileName[256], path[200];
			int status, version;
			LTRESULT dResult;
			CreateShellFn createFn;
			DeleteShellFn deleteFn;


			pClientMgr->m_hClientResourceModule = LTNULL;
			pClientMgr->m_hLocalizedClientResourceModule = LTNULL;
			pClientMgr->m_hShellModule = LTNULL;

			// Get the temp path.
			if(!GetTempPath(sizeof(path), path))
			{
				strcpy(path, ".\\");
			}

			// Setup the cshell.dll.
			dResult = _GetOrCopyClientFile(pClientMgr, path, "cshell.dll", fileName, sizeof(fileName));
			if(dResult != LT_OK)
			{
				cm_SetupError(pClientMgr, LT_ERRORCOPYINGFILE, "cshell.dll");
				RETURN_ERROR_PARAM(1, InitClientShellDE, LT_ERRORCOPYINGFILE, "cshell.dll");
			}

			status = sb_LoadModule(fileName, "ClientShell", &pClientMgr->m_hShellModule, CLIENTSHELL_VERSION, &version);

			// Get clientres.dll out of this directory..
			if(status == SB_NOERROR)
			{
				// Try to setup cres.dll.
				dResult = _GetOrCopyClientFile(pClientMgr, path, "cres.dll", fileName, sizeof(fileName));
				if((dResult == LT_OK) &&
					(bm_BindModule(fileName, &pClientMgr->m_hClientResourceModule) == BIND_NOERROR))
				{
				}
				else
				{
					sb_UnloadModule(pClientMgr->m_hShellModule);
					pClientMgr->m_hShellModule = LTNULL;
					cm_SetupError(pClientMgr, LT_ERRORCOPYINGFILE, "cres.dll");
					RETURN_ERROR_PARAM(1, InitClientShellDE, LT_ERRORCOPYINGFILE, "cres.dll");
				}

				// Try to load and bind to the localized resources.
				dResult = _GetOrCopyClientFile(pClientMgr, path, "cresl.dll", fileName, sizeof(fileName));
				bm_BindModule(fileName, &pClientMgr->m_hLocalizedClientResourceModule);

				// Try to create a client shell.
				sb_GetShellFunctions(pClientMgr->m_hShellModule, &createFn, &deleteFn);
				
				pClientMgr->m_pClientShell = (IClientShell*)createFn(pClientMgr->m_pClientDE);
				if(!pClientMgr->m_pClientShell)
				{
					RETURN_ERROR(1, InitClientShellDE, LT_CANTCREATECLIENTSHELL);
				}

				ee_InitSurfaceEffects(pClientMgr);
				return LT_OK;
			}
			else if(status == SB_CANTFINDMODULE)
			{
				cm_SetupError(pClientMgr, LT_MISSINGSHELLDLL, "cshell.dll");
				RETURN_ERROR(1, InitClientShellDE, LT_MISSINGSHELLDLL);
			}
			else if(status == SB_NOTSHELLMODULE)
			{
				cm_SetupError(pClientMgr, LT_INVALIDSHELLDLL, "cshell.dll");
				RETURN_ERROR(1, InitClientShellDE, LT_INVALIDSHELLDLL);
			}
			else// if(status == SB_VERSIONMISMATCH)
			{
				cm_SetupError(pClientMgr, LT_INVALIDSHELLDLLVERSION, "cshell.dll", version, CLIENTSHELL_VERSION);
				RETURN_ERROR(1, InitClientShellDE, LT_INVALIDSHELLDLLVERSION);
			}
		}


		void dsi_OnMemoryFailure()
		{
			longjmp(g_ClientGlob.m_MemoryJmp, 1);
		}


		// Client-only functions.
		void dsi_ClientSleep(uint32 ms)
		{
			if(ms > 0)
			{
				Sleep(ms);
			}
		}

		LTBOOL dsi_IsInputEnabled()
		{
			return g_ClientGlob.m_bInputEnabled;
		}

		uint16 dsi_NumKeyDowns()
		{
			return g_ClientGlob.m_nKeyDowns;
		}

		uint16 dsi_NumKeyUps()
		{
			return g_ClientGlob.m_nKeyUps;
		}

		uint32 dsi_GetKeyDown(uint32 i)
		{
			ASSERT(i < MAX_KEYBUFFER);
			return g_ClientGlob.m_KeyDowns[i];
		}

		uint32 dsi_GetKeyDownRep(uint32 i)
		{
			ASSERT(i < MAX_KEYBUFFER);
			return g_ClientGlob.m_KeyDownReps[i];
		}

		uint32 dsi_GetKeyUp(uint32 i)
		{
			ASSERT(i < MAX_KEYBUFFER);
			return g_ClientGlob.m_KeyUps[i];
		}

		void dsi_ClearKeyDowns()
		{
			g_ClientGlob.m_nKeyDowns=0;
		}

		void dsi_ClearKeyUps()
		{
			g_ClientGlob.m_nKeyUps=0;
		}

		void dsi_ClearKeyMessages()
		{
			MSG msg;
			int i;

			for (i = 0; i < 500; i++)
			{
				if (!PeekMessage(&msg, g_ClientGlob.m_hMainWnd, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
				{
					break;
				}
			}

			for (i = 0; i < 500; i++)
			{
				if (!PeekMessage(&msg, g_ClientGlob.m_hMainWnd, WM_KEYUP, WM_KEYUP, PM_REMOVE))
				{
					break;
				}
			}
		}

		LTBOOL dsi_IsConsoleUp()
		{
			return g_ClientGlob.m_bIsConsoleUp;
		}

		void dsi_SetConsoleUp(LTBOOL bUp)
		{
			g_ClientGlob.m_bIsConsoleUp = bUp;
		}

		LTBOOL dsi_IsClientActive()
		{
			return g_ClientGlob.m_bClientActive;
		}

		void dsi_OnClientShutdown( char *pMsg )
		{
			if( pMsg && pMsg[0] )
			{
				strncpy( g_ClientGlob.m_ExitMessage, pMsg, sizeof( g_ClientGlob.m_ExitMessage ) - 1 );
				g_ClientGlob.m_ExitMessage[ sizeof( g_ClientGlob.m_ExitMessage ) - 1 ] = '\0';
			}
			else
			{
				g_ClientGlob.m_ExitMessage[0] = '\0';
			}

			if(	g_ClientGlob.m_bProcessWindowMessages )
			{
				PostQuitMessage(0);
			}
		}

		const char* dsi_GetDefaultWorld()
		{
			return g_ClientGlob.m_pWorldName;
		}



		void dsi_PrintToConsole(char *pMsg, ...)
		{
			char msg[500];
			va_list marker;

			va_start(marker, pMsg);
			_vsnprintf(msg, 499, pMsg, marker);
			va_end(marker);

			con_PrintString(CONRGB(255,255,0), 0, msg);
		}

		void* dsi_GetInstanceHandle()
		{
			return (void*)g_ClientGlob.m_hInstance;
		}

		void* dsi_GetMainWindow()
		{
			return (void*)g_ClientGlob.m_hMainWnd;
		}

		LTRESULT dsi_DoErrorMessage(char *pMessage)
		{
			con_PrintString(CONRGB(255,255,255), 0, pMessage);
			
			if(!g_Render.m_bInitted)
			{
				MessageBox(g_ClientGlob.m_hMainWnd, pMessage, g_ClientGlob.m_WndCaption, MB_OK);
			}

			return LT_OK;
		}

		void dsi_MessageBox(const char *pMessage, const char *pTitle)
		{
			int i;
			i = MessageBox(g_ClientGlob.m_hMainWnd, pMessage, pTitle, MB_OK);
		}

		LTRESULT dsi_GetVersionInfo(LTVersionInfo &info)
		{
			return GetLTExeVersion(g_ClientGlob.m_hInstance, info);
		}

	#else
		extern CServerMgr *g_pServerMgr;


		void dsi_PrintToConsole(char *pMsg, ...)
		{
			va_list marker;
			char msg[1000];

			if(g_pServerMgr && g_pServerMgr->m_pServerAppHandler)
			{
				va_start(marker, pMsg);
				_vsnprintf(msg, 999, pMsg, marker);
				va_end(marker);

				g_pServerMgr->m_pServerAppHandler->ConsoleOutputFn(msg);
			}
		}

		void* dsi_GetInstanceHandle()
		{
			return (void*)g_hModuleInstanceHandle;
		}

		void* dsi_GetMainWindow()
		{
			return LTNULL;
		}

		void dsi_OnMemoryFailure()
		{
			// Let the server app shutdown.
			if(g_pServerMgr && g_pServerMgr->m_pServerAppHandler)
			{
				g_pServerMgr->m_pServerAppHandler->OutOfMemory();
			}

			// They better have exited.. make sure.
			MessageBox(GetDesktopWindow(), "Out of memory", "LithTech", MB_OK);
			exit(2222);
		}
	#endif


#endif  // DIRECTEDITOR_BUILD
