//------------------------------------------------------------------
//
//  FILE      : ClientMgr.cpp
//
//  PURPOSE   : This is the Mgr for the client side DirectEngine
//              libraries.
//
//  CREATED   : November 7 1996
//
//  COPYRIGHT : LithTech Inc., 1996-2000
//
//------------------------------------------------------------------

// Includes....
#ifdef LITHTECH_ESD
#include "ltrealaudio_impl.h"
#include "ltrealconsole_impl.h"
#include "ltrealvideo_impl.h"
#endif // LITHTECH_ESD

#include "bdefs.h"

#include "build_options.h"

#include "renderstruct.h"
#include "render.h"
#include "musicdriver.h"

#include "soundmgr.h"
#include "clientmgr.h"
#include "consolecommands.h"
#include "console.h"
#include "memorywatch.h"
#include "clientde_impl.h"
#include "sysinput.h"
#include "errorlog.h"
#include "sysvideo.h"
#include "bindmgr.h"
#include "clientshell.h"
#include "servermgr.h"
#include "systimer.h"
#include "iltclient.h"
#include "ltbenchmark_impl.h"
#include "sprite.h"
#include "cmoveabstract.h"
#include "ltmessage_client.h"
#include "stringmgr.h"
#include "debuggeometry.h"
#include "setupobject.h"
#include "systimer.h"
#include "ltobjectcreate.h"
#include "particlesystem.h"
#include "client_ticks.h"
#include "iltdrawprim.h"

#include "dtxmgr.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------


//pointer to the client debug graph mgr module.
#include "client_graphmgr.h"
#include "debuggraphmgr.h"
static IClientDebugGraphMgr *graph_mgr;
define_holder(IClientDebugGraphMgr, graph_mgr);

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//IClientFileMgr
#include "client_filemgr.h"
static IClientFileMgr *client_file_mgr;
define_holder(IClientFileMgr, client_file_mgr);

//IClientShell game client shell object.
#include "iclientshell.h"
static IClientShell *i_client_shell;
define_holder(IClientShell, i_client_shell);

//ILTBenchmarkMgr game interface.
#include "iltbenchmark.h"
static ILTBenchmarkMgr *ilt_benchmark;
define_holder(ILTBenchmarkMgr, ilt_benchmark);


//ILTFontManager holder
#include "iltfontmanager.h"
static ILTFontManager *font_manager;
define_holder(ILTFontManager, font_manager);

//holder for command line argument mgr interface.
#include "icommandlineargs.h"
static ICommandLineArgs *command_line_args;
define_holder(ICommandLineArgs, command_line_args);


#ifdef DE_LOCAL_SERVERBIND
// only include this if server-client are on same.
//server file mgr.
#include "server_filemgr.h"
static IServerFileMgr *server_filemgr;
define_holder(IServerFileMgr, server_filemgr);
#endif

//Interface to the internal instance of DrawPrim
static ILTDrawPrim* g_pLTDrawPrimInternal;
define_holder_to_instance(ILTDrawPrim, g_pLTDrawPrimInternal, Internal);

//------------------------------------------------------------------
//------------------------------------------------------------------
// Console stuff
//------------------------------------------------------------------
//------------------------------------------------------------------

extern int32    g_CV_BitDepth;
extern int32    g_ClientSleepMS;
extern int32    g_bShowMemStats;
extern int32    g_nConsoleLines;
extern int32 g_CV_ShowFrameRate;
extern int32 g_ScreenWidth, g_ScreenHeight;
extern int32 g_CV_FullLightScale;
extern LTVector g_ConsoleModelAdd;
extern float g_CV_MaxFPS;
extern int32 g_CV_ForceConsole;
extern int32 g_CV_RenderEnable;
extern int32 g_CV_ForceSoundDisable;
extern int32 g_CV_DrawDebugGeometry;

#ifdef LITHTECH_ESD
ILTRealAudioPlayer* g_pRealAudioPlayer = LTNULL;
ILTRealVideoPlayer* g_pRealVideoPlayer = LTNULL;
LTBOOL g_bShowRealPlayTime = LTFALSE;
#endif // LITHTECH_ESD

// Timing information
extern uint32 g_Ticks_Music;
extern uint32 g_Ticks_Sound;
extern uint32 g_Ticks_Input;
extern uint32 g_Ticks_Render;

ObjectBank<LTLink> g_DLinkBank(64, 1024);

CClientMgr *g_pClientMgr = NULL;

SMusicMgr *GetMusicMgr() { return g_pClientMgr->GetMusicMgr(); }

extern LTBOOL g_bMusicEnable;
extern LTBOOL g_bSoundEnable;
extern uint32 g_nSoundDebugLevel;
extern LTBOOL g_bSoundShowCounts;
extern LTBOOL g_bNullRender;
extern LTBOOL g_bHWTnLDisabled;

// ----------------------------------------------------------------------- //
// CClientMgr functions.
// ----------------------------------------------------------------------- //

void _GetRModeFromConsoleVariables(RMode *pMode)
{
    LTCommandVar *pVar;

    // Init the renderer.
    memset(pMode, 0, sizeof(*pMode));
    pMode->m_Width = g_ScreenWidth;
    pMode->m_Height = g_ScreenHeight;
    pMode->m_BitDepth = g_CV_BitDepth;
	pMode->m_bHWTnL = !g_bHWTnLDisabled;

    // Use a specific card description if they want.
    pVar = cc_FindConsoleVar(&g_ClientConsoleState, "carddesc");
    if (pVar && pVar->pStringVal)
    {
        strncpy(pMode->m_InternalName, pVar->pStringVal, sizeof(pMode->m_InternalName)-1);
    }
}


void MaybeDrawConsole()
{
    RenderStruct *pStruct;

    if (g_CV_ForceConsole)
    {
        pStruct = r_GetRenderStruct();
        if (pStruct->m_bInitted && g_CV_ForceConsole)
        {
            con_Draw();
            pStruct->SwapBuffers(0);
        }
    }
}


//
// CClientMgr
//

CClientMgr::CClientMgr()
{
	//initialize member variables
	m_pVideoMgr = NULL;

}

LTRESULT CClientMgr::Init(const char *resTrees[MAX_RESTREES], uint32 nResTrees, uint32 nNumConfigs, const char **ppConfigFileName)
{
    const char *pDLLPath;
    char cmd[64];
    TreeType treeTypes[MAX_RESTREES];
    int nTreesLoaded;
    LTRESULT dResult;
    RMode defaultMode;
    char versionStr[32];
    LTRect rGraphMgr;

	// Set up the net handler for default
	m_NetMgr.SetNetHandler(&m_cDefaultNetHandler);

    r_InitRenderStruct(true);

    mw_ResetWatches();

    m_hShellModule = NULL;

	m_LastReceiveBandwidth = 0;
    m_bInputState = true;
    m_bTrackingInputDevices = false;
	SetFrameCode(0); //

    world_bsp_client->ClientTree()->InitWorldTree(&m_ObjectMgr);

    // Init our ILTClient interface..
    CreateLTClient();


    // Setup some default data.
    m_AxisOffsets[0] = m_AxisOffsets[1] = m_AxisOffsets[2] = 0.0f;
    memset(m_Commands, 0, sizeof(m_Commands));
    m_iCurInputSlot = 0;


    // Add files to the file manager.
    if (nResTrees > MAX_RESTREES)
        nResTrees = MAX_RESTREES;

    if (nResTrees == 0)
    {
        SetupError(LT_NOGAMERESOURCES);
        ProcessError(LT_NOGAMERESOURCES | ERROR_SHUTDOWN);
        RETURN_ERROR(1, CClientMgr::Init, LT_NOGAMERESOURCES);
    }

    memcpy(m_ResTrees, resTrees, sizeof(m_ResTrees));
    m_nResTrees = nResTrees;

    client_file_mgr->AddResourceTrees(resTrees, nResTrees, treeTypes, &nTreesLoaded);
    if (nTreesLoaded == 0)
    {
        SetupError(LT_CANTLOADGAMERESOURCES, resTrees[0]);
        ProcessError(LT_CANTLOADGAMERESOURCES);
        RETURN_ERROR(1, CClientMgr::Init, LT_CANTLOADGAMERESOURCES);
    }

    // The path to the DLLs is either the dos tree or NULL, in which case
    // it'll check the registry.
    pDLLPath = NULL;
    if (treeTypes[0] == DosTree) {
        pDLLPath = resTrees[0];
    }

    // initialize the font manager
    font_manager->Init();

    // Setup the ClientShellDE.
    dResult = dsi_InitClientShellDE();
    if (dResult != LT_OK)
    {
        ProcessError(dResult | ERROR_SHUTDOWN);
        return dResult;
    }

    c_InitConsoleCommands();
    con_InitBare();

    // Add the version console variable..
    dsi_GetVersionInfo(m_VersionInfo);
    m_VersionInfo.GetString(versionStr, sizeof(versionStr));
    LTSNPrintF(cmd, sizeof(cmd), "version %s", versionStr);
    cc_HandleCommand(&g_ClientConsoleState, cmd);

    // Initialize input.
    if (dsi_IsInputEnabled())
    {
        if (!m_InputMgr->Init(m_InputMgr, &g_ClientConsoleState))
        {
            ProcessError(LT_CANTINITIALIZEINPUT | ERROR_SHUTDOWN);
            RETURN_ERROR(1, CClientMgr::Init, LT_CANTINITIALIZEINPUT);
        }
    }


    //run all of the configuration files
	uint32 nCurrConfig;
	for(nCurrConfig = 0; nCurrConfig < nNumConfigs; nCurrConfig++)
	{
		RunAutoConfig(ppConfigFileName[nCurrConfig], CC_NOCOMMANDS);
	}

    RunCommandLine(CC_NOCOMMANDS);


    // Open up the error log.
    InitErrorLog();

    // Init some music stuff..
    m_MusicMgr.m_hWnd = dsi_GetMainWindow();
    m_MusicMgr.m_hInstance = dsi_GetInstanceHandle();

    if (g_bNullRender)
    {
        dsi_ConsolePrint("Warning: NullRender is ON.");
    }

    // Rerun the config stuff with commands only.
	for(nCurrConfig = 0; nCurrConfig < nNumConfigs; nCurrConfig++)
	{
		RunAutoConfig(ppConfigFileName[nCurrConfig], CC_NOVARS);
	}
    RunCommandLine(CC_NOVARS);
    m_bCanSaveConfigFile = true;

    m_pCollisionInfo = NULL;

    // Initialize the listener info.
//  m_bListenerInClient = true;
//  VEC_INIT(m_vListenerPos);
//  ROT_INIT(m_rListenerRot);
//  VEC_INIT(m_vLastListenerPos);

    // Tell the client shell that we are ready to rock!
    _GetRModeFromConsoleVariables(&defaultMode);
    dResult = i_client_shell->OnEngineInitialized(&defaultMode, &m_NetMgr.m_guidApp);

    // VideoMgr MUST be initialized AFTER the ClientShell, so that the SoundMgr is
    // properly initialized.


//
//	!!!!! BINK IS NOT ENABLED
//
//		IHAVEPURCHASEDBINK Define that allows bink video player to function. ( Separate license/SDK available from rad game tools http://www.radgametools.com/)
//
//		You must recompile Exe_Lithtech with IHAVEPURCHASEDBINK defined in the project settings: 
//
//    From the SDK you purchased from bink place bink.h rad.h radbase.h and smack.h into the Engine/runtime/kernel/src/sys/win directory.
// 	Also requires bink32.dll in your path when running 
//

#if defined( IHAVEPURCHASEDBINK )

    m_pVideoMgr = CreateVideoMgr("BINK");

#else

//
//---- NOTE: Directshow video not available in version 68 or lower 
// 

	 m_pVideoMgr = CreateVideoMgr("DIRECTSHOW");

#endif

    // Tell the video stuff.
    if (m_pVideoMgr) 
	 {
        m_pVideoMgr->OnRenderInit(); 
	 }

    // Initialize the debug graph manager
    rGraphMgr.left	 = 0;
    rGraphMgr.top	 = 0;
    rGraphMgr.right	 = defaultMode.m_Width;
    rGraphMgr.bottom = defaultMode.m_Height;
    graph_mgr->Mgr()->Init(&rGraphMgr);

#ifdef COMPILE_JUPITER_EVAL
	m_WaterMark.Init();
#endif // COMPILE_JUPITER_EVAL

    return dResult;
}


void CClientMgr::TermClientShellDE()
{
    LTExtraCommandStruct *pCommand;
    LTLink *pCur, *pNext;


    // Clear rendering hooks.
    m_ModelHookFn = NULL;


    // Remove their console programs.
    pCur = g_ClientConsoleState.m_ExtraCommands.m_pNext;
    while (pCur && (pCur != &g_ClientConsoleState.m_ExtraCommands))
    {
        pNext = pCur->m_pNext;

        pCommand = (LTExtraCommandStruct*)pCur->m_pData;
        if (pCommand->flags & CMD_USERCOMMAND)
        {
            cc_RemoveCommand(&g_ClientConsoleState, pCommand);
        }

        pCur = pNext;
    }


    if (m_hShellModule)
    {
        bm_UnbindModule(m_hShellModule);
        m_hShellModule = NULL;
    }

    if (m_hClientResourceModule)
    {
        bm_UnbindModule(m_hClientResourceModule);
        m_hClientResourceModule = NULL;
    }

    if (m_hLocalizedClientResourceModule)
    {
        bm_UnbindModule(m_hLocalizedClientResourceModule);
        m_hLocalizedClientResourceModule = NULL;
    }

}


void CClientMgr::OnEnterWorld(CClientShell *pShell) {
    if (i_client_shell != NULL) {
        i_client_shell->OnEnterWorld();
    }
}


void CClientMgr::OnExitWorld(CClientShell *pShell)
{
    if (i_client_shell != NULL)
	{
        i_client_shell->OnExitWorld();
    }

	UncacheModels();

}


LTRESULT CClientMgr::StartShell(StartGameRequest *pRequest)
{
    CClientShell *pShell;
    LTRESULT dResult;
    char errorString[256];
    CBaseDriver *pDriver;
    char *pWorldName, playbackWorldName[256];


    // Kill the old server connection if there is one.
    EndShell();

    LT_MEM_TRACK_ALLOC(pShell = new CClientShell, LT_MEM_TYPE_MISC);

    //  Start a shell.
    pShell->m_ShellMode = pRequest->m_Type;
    pShell->Init();

    dsi_SetConsoleUp(false);

#ifdef _PUBLIC_DEVELOPMENT
	if( pRequest->m_Type != STARTGAME_NORMAL )
    {
        RETURN_ERROR(1, StartGame, LT_NOTINITIALIZED);
    }
#endif // _PUBLIC_DEVELOPMENT

    switch(pRequest->m_Type)
    {
        case STARTGAME_CLIENT:
        {
            // Use the first net driver..
            pDriver = m_NetMgr.GetMainDriver();
            if (!pDriver)
            {
                RETURN_ERROR(1, StartGame, LT_NOTINITIALIZED);
            }

            // Try to actually connect...

            if (pRequest->m_flags & SG_LOBBY)
            {
                if (!pDriver->JoinLobbyLaunchSession())
                {
                    RETURN_ERROR(1, StartGame, LT_ERROR);
                }
            }
            else
            {
                if (!pRequest->m_pNetSession)
                {
                    RETURN_ERROR_PARAM(1, StartGame, LT_INVALIDPARAMS, "Missing m_pNetSession");
                }

                dResult = pDriver->JoinSession(pRequest->m_pNetSession);
                if (dResult != LT_OK)
                {
                    return dResult;
                }
            }

            // Setup the shell.
            dResult = pShell->StartupClient(pDriver);
            if (dResult != LT_OK)
                return dResult;
        }
        break;

        case STARTGAME_CLIENTTCP:
        {
            // Use the first net driver..
            pDriver = m_NetMgr.GetDriver("internet");
            if (!pDriver)
            {
                RETURN_ERROR(1, StartGame, LT_NOTINITIALIZED);
            }

            // Connect to the given address.
            dResult = pDriver->ConnectTCP(pRequest->m_TCPAddress);
            if (dResult != LT_OK)
                return dResult;

            // Setup the shell.
            dResult = pShell->StartupClient(pDriver);
            if (dResult != LT_OK)
                return dResult;
        }
        break;

        default:
        {
            pDriver = NULL;
            if (pRequest->m_Type == STARTGAME_HOST)
            {
                pDriver = m_NetMgr.GetMainDriver();
                if (!pDriver)
                {
                    RETURN_ERROR(1, StartGame, LT_NOTINITIALIZED);
                }

                if (pRequest->m_flags & SG_LOBBY)
                {
                    if (pDriver->HostLobbyLaunchSession(&pRequest->m_HostInfo) != LT_OK)
                    {
                        RETURN_ERROR(1, StartGame, LT_ERROR);
                    }
                }
                else
                {
                    dResult = pDriver->HostSession(&pRequest->m_HostInfo);
                    if (dResult != LT_OK)
                    {
                        return dResult;
                    }
                }
            }

            // Setup the shell.
            dResult = pShell->StartupLocal(pRequest, pRequest->m_Type==STARTGAME_HOST, pDriver);
            if (dResult != LT_OK)
            {
                delete pShell;
                return dResult;
            }

            pWorldName = pRequest->m_WorldName;
            dResult = LT_OK;
            if (pWorldName[0] != 0)
            {
                dResult = g_pServerMgr->DoStartWorld(pWorldName, LOADWORLD_LOADWORLDOBJECTS|LOADWORLD_RUNWORLD, m_nCurUpdateTimeMS);
            }

            if (dResult != LT_OK)
            {
                g_pServerMgr->GetErrorString(errorString, 256);
                SetupError(dResult, errorString);
                delete pShell;
                RETURN_ERROR_PARAM(1, CClientMgr::StartShell, dResult, errorString);
            }

            // If they asked for a playdemo, fill in the world name.
            if (pRequest->m_PlaybackFilename[0] != 0)
            {
                SAFE_STRCPY(pRequest->m_WorldName, playbackWorldName);
            }
        }
        break;
    }

    // 'Enter' the server.
    m_pCurShell = pShell;
    OnEnterServer();

    // Send the hello message.
    if (m_pCurShell)
    {
		CPacket_Write cHello;
		cHello.Writeuint8(CMSG_HELLO);
		cHello.Writeuint16((uint16)pRequest->m_ClientDataLen);
		cHello.WriteData(pRequest->m_pClientData, pRequest->m_ClientDataLen * 8);
        m_NetMgr.SendPacket(CPacket_Read(cHello), m_pCurShell->m_HostID);
    }

    return LT_OK;
}


void CClientMgr::EndShell()
{
    if (m_pCurShell)
    {
		if (m_bInCurShellUpdate)
		{
			// Don't delete the old shell yet -- we may still have it on the
			// call stack.  As we may be starting a new shell however, store
			// off a pointer to it to be deleted later.
			ASSERT(m_pOldShell == NULL);
			m_pCurShell->Term();
			m_pOldShell = m_pCurShell;
		}
		else
		{
        delete m_pCurShell;
		}
        m_pCurShell = NULL;
    }
	// Set us back to the default net handler
	m_NetMgr.SetNetHandler(&m_cDefaultNetHandler);
}

LTRESULT CClientMgr::AppInitMusic(const char *pszMusicDLL)
{

    if (!g_bMusicEnable || g_CV_ForceSoundDisable)
        RETURN_ERROR(1, CClientMgr::AppInitMusic, LT_ERROR);

    if (pszMusicDLL)
        LTStrCpy(m_MusicDLLName, pszMusicDLL, sizeof(m_MusicDLLName));
    else
        LTStrCpy(m_MusicDLLName, "cdaudio.dll", sizeof(m_MusicDLLName));

    // Initialize music...
    switch(music_InitDriver(m_MusicDLLName, &m_MusicMgr))
    {
        case MUSICDRIVER_CANTLOADLIBRARY:
            SetupError(LT_MISSINGMUSICDLL, m_MusicDLLName);
            RETURN_ERROR(1, CClientMgr::AppInitMusic, LT_MISSINGMUSICDLL);
            break;
        case MUSICDRIVER_INVALIDDLL:
            SetupError(LT_INVALIDMUSICDLL, m_MusicDLLName);
            RETURN_ERROR(1, CClientMgr::AppInitMusic, LT_INVALIDMUSICDLL);
            break;
        case MUSICDRIVER_INVALIDOPTIONS:
            SetupError(LT_UNABLETOINITMUSICDLL, m_MusicDLLName);
            RETURN_ERROR(1, CClientMgr::AppInitMusic, LT_UNABLETOINITMUSICDLL);
            break;
    }

    return LT_OK;
}

void CClientMgr::AppTermMusic()
{
    music_TermDriver();
}

bool CClientMgr::BindClientShellWorlds()
{
    if (m_pCurShell) {
        m_pCurShell->BindWorlds();
    }

    return true;
}


void CClientMgr::UnbindClientShellWorlds()
{
    if (m_pCurShell)
        m_pCurShell->UnbindWorlds();
}


void CClientMgr::BindSharedTextures()
{
    LTLink *pCur, *pListHead;

    // Read in the latest console variables..
    if (g_Render.m_bInitted && g_Render.ReadConsoleVariables)
        g_Render.ReadConsoleVariables();

    pListHead = &m_SharedTextures.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        r_BindTexture((SharedTexture*)pCur->m_pData, false);
    }
}

void CClientMgr::UnbindSharedTextures(bool bUnLoad_EngineData)
{
    LTLink *pCur, *pListHead;

    pListHead = &m_SharedTextures.m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext) {
        r_UnbindTexture((SharedTexture*)pCur->m_pData, bUnLoad_EngineData); }
}

void CClientMgr::InitConsole()
{
    RenderStruct *pRender = r_GetRenderStruct();
    ASSERT(pRender);
    LTRect rect (0, 0, pRender->m_Width, pRender->m_Height/2);

    // Init the console.
    con_Term(false);
    con_Init(&rect, c_CommandHandler, pRender);
}

void CClientMgr::TermConsole()
{
    con_Term(false);
}


void cm_HandleMaxFPS(uint32 frameTicks)
{
    float frameSeconds, wantedSeconds, startTime, curTime;
    uint32 i;

    if (g_CV_MaxFPS == 0.0f)
        return;

    g_CV_MaxFPS = LTCLAMP(g_CV_MaxFPS, 1.0f, 200.0f);

    wantedSeconds = 1.0f / g_CV_MaxFPS;
    frameSeconds = (float)frameTicks / cnt_NumTicksPerSecond();
    if (frameSeconds < wantedSeconds)
    {
        startTime = time_GetTime();
        for (i=0; i < 500; i++)
        {
            curTime = time_GetTime();
            if ((curTime - startTime) > (wantedSeconds - frameSeconds))
                break;

            dsi_ClientSleep(1); // Sleep for 1 millisecond.
        }
    }
}



LTRESULT CClientMgr::Update()
{
    uint16 i;
    Counter totalCounter;
    LTRESULT dResult;
    static LTRect rFPS (480,10,630,80);

#ifndef _FINAL
    uint32 frameTicks;

	ctik_ResetCounters();
    cnt_StartCounter(totalCounter);

#endif

    // Update videos.
    if (m_pVideoMgr)
	{
        m_pVideoMgr->UpdateVideos();
    }

    // Hack while there's a bug in Shogo..
    if (g_CV_FullLightScale)
	{
        VEC_SET(m_GlobalLightScale, 1, 1, 1);
	}

    // Update framerate (don't want to update it if the client's not active).
    if (dsi_IsClientActive())
	{
		m_nCurUpdateTimeMS	= time_GetMSTime();
		m_nFrameTimeMS		= m_nCurUpdateTimeMS - m_nLastUpdateTimeMS;
		m_nLastUpdateTimeMS	= m_nCurUpdateTimeMS;

        m_FrameTime			= m_nFrameTimeMS / 1000.0f;
		m_CurTime			= m_nCurUpdateTimeMS / 1000.0f;

        UpdateFrameRate();
    }

    // Sleep in between frames.. helpful for debugging so it doesn't hog
    // all the processor time.
    dsi_ClientSleep(g_ClientSleepMS);

    // Puase music if disabled...
    if (GetMusicMgr()->m_bValid)
	{
        CountAdder cntAdd(&g_Ticks_Music);

        if ((g_bMusicEnable != 0) != GetMusicMgr()->m_bEnabled)
		{
            GetMusicMgr()->m_bEnabled = (g_bMusicEnable != 0);
            if (!g_bMusicEnable)
			{
                GetMusicMgr()->Pause(MUSIC_IMMEDIATE);
            }
            else
			{
                GetMusicMgr()->Resume();
            }
        }
    }

    // Pause sound if disabled...
    if (GetClientILTSoundMgrImpl()->IsValid())
	{
        CountAdder cntAdd(&g_Ticks_Sound);

        if ((g_bSoundEnable != 0) != GetClientILTSoundMgrImpl()->IsEnabled())
		{
            GetClientILTSoundMgrImpl()->SetEnable(g_bSoundEnable != 0);
            if (!g_bSoundEnable)
			{
                GetClientILTSoundMgrImpl()->PauseSounds();
            }
            else
			{
                GetClientILTSoundMgrImpl()->ResumeSounds();
            }
        }
    }

    // Forward windows messages to the scripts.
    if (!dsi_IsConsoleUp())
	{
        ForwardMessagesToScript();
    }

    // Gather and send current input.
    if (dsi_IsClientActive())
	{
        CountAdder cntAdd(&g_Ticks_Input);
        ProcessAllInput(false);
    }

    // Update client shells.
    if (m_pCurShell)
    {
		// Insure that the shell doesn't delete itself while updating.
		// EndShell switches on these bools to determine if it should delay
		// the deletion of m_pCurShell or if it should do it immediately.
		m_bInCurShellUpdate = true;
        dResult = m_pCurShell->Update();
		m_bInCurShellUpdate = false;

		// A shell was deleted this update.  Delete it now that we know it 
		// is off the call stack
		if (m_pOldShell != NULL)
		{
			delete m_pOldShell;
			m_pOldShell = NULL;
		}

        if (dResult != LT_OK)
            return ProcessError(dResult);
    }
    else if (i_client_shell != NULL)
	{
        // If we don't have any client shells right now, do a 'fake' update.
        m_NetMgr.Update("Client: ", m_CurTime);

        i_client_shell->PreUpdate();

        i_client_shell->Update();

        UpdateObjects();

        i_client_shell->PostUpdate();
    }

    // Update all sounds.
	UpdateAllSounds();

    // Update the console if it's up.
    if (dsi_IsConsoleUp())
    {
        for (i=0; i < dsi_NumKeyDowns(); i++)
        {
            con_OnKeyPress(dsi_GetKeyDown(i));
        }

        dsi_ClearKeyDowns();
        dsi_ClearKeyUps();
    }

// Disable the debug stuff in the final build
#ifndef _FINAL

	// Only log to the console if this value is 1.
	// If the graph is up, there's no reason to log to the console
    if (g_CV_ShowFrameRate == 1)
    {
        con_Printf(CONRGB(255,100,100), 0, "FPS: %.2f", m_FramerateTracker.GetRate());
    }

    if (graph_mgr->Mgr()->CheckGraph(&g_CV_ShowFrameRate, g_CV_ShowFrameRate >= 2))
    {
        DGParams foo ("Framerate (0 - 60)", 0.0f, 60.0f, GetDGColorsRYG());
        graph_mgr->Mgr()->UpdateGraph(&g_CV_ShowFrameRate,
            1.0f / m_FrameTime, foo);
    }

    // Memory stats.
    if (g_bShowMemStats)
    {
        con_WhitePrintf("Memory allocated: %dk", dm_GetBytesAllocated() / 1000);
        con_WhitePrintf("Number of allocations: %d", dm_GetNumAllocations());
    }

    if (graph_mgr->Mgr()->CheckGraph(&g_bShowMemStats, g_bShowMemStats >= 2))
    {
        DGParams foo ("Memory", 0.0f,
            (float)dm_GetBytesAllocated() / 500.0f, GetDGColorsKB());
        graph_mgr->Mgr()->UpdateGraph(&g_bShowMemStats,
            (float)dm_GetBytesAllocated() / 1000.0f, foo);
    }

    // Tick counts.
    frameTicks = cnt_EndCounter(totalCounter);
	g_Ticks_Total += frameTicks;

	ctik_ShowTickStatus();
    mw_ShowMemoryWatches();

    // For debugging...
    MaybeDrawConsole();

    cm_HandleMaxFPS(frameTicks);                    // Lock framerate if they want.

#endif

    GraphicsBenchmarkingUpdate();  // Update the benchmark mgr

    #ifdef LITHTECH_ESD
    m_pRealAudioMgr->Update();                      // Update the Real video manager
    #endif

    return LT_OK;                                   // All done
}

void CClientMgr::ProcessAllInput(bool bForceClear) {

    int32 changes[MAX_CLIENT_COMMANDS], nChanges = 0;
    int32 commandsOn[MAX_CLIENT_COMMANDS], nCommandsOn = 0;


    uint8 *pPrevSlot, *pCurSlot;
    pPrevSlot = m_Commands[m_iCurInputSlot];
    pCurSlot = m_Commands[!m_iCurInputSlot];

    memset(pCurSlot, 0, MAX_CLIENT_COMMANDS);
    if (!m_bTrackingInputDevices)
    {
        m_InputMgr->ReadInput(m_InputMgr, pCurSlot, m_AxisOffsets);
    }

    if (!m_bInputState || (bForceClear || dsi_IsConsoleUp()))
    {
        memset(pCurSlot, 0, MAX_CLIENT_COMMANDS);
        memset(m_AxisOffsets, 0, sizeof(m_AxisOffsets));
    }

    // Find out how many have changed.
    int32 i;
    for (i=0; i < MAX_CLIENT_COMMANDS; i++)
    {
        if (pCurSlot[i])
        {
            commandsOn[nCommandsOn] = i;
            nCommandsOn++;
        }

        if (pCurSlot[i] != pPrevSlot[i])
        {
            changes[nChanges] = i;
            nChanges++;
        }
    }

    m_iCurInputSlot = !m_iCurInputSlot;

    ForwardCommandChanges(&changes[0], nChanges);

	if (m_pCurShell && m_pCurShell->m_bWorldOpened && m_pCurShell->m_HostID)
	{
		SendUpdate(&m_NetMgr, m_pCurShell->m_HostID);
	}
}


LTRESULT CClientMgr::ClearInput()
{
    ProcessAllInput(true);
    dsi_ClearKeyDowns();
    dsi_ClearKeyUps();
    dsi_ClearKeyMessages();
    m_InputMgr->ClearInput();

    return LT_OK;
}

bool CClientMgr::IsCommandOn(uint32 nCommand) const
{
	if (nCommand >= MAX_CLIENT_COMMANDS)
		return false;
	return m_Commands[m_iCurInputSlot][nCommand] != 0;
}

int CClientMgr::GetErrorCode()
{
    return m_LastErrorCode;
}


void CClientMgr::ShowDrawSurface(uint flags)
{
	#ifdef LITHTECH_ESD
	m_pRealVideoMgr->Update(LTRV_UPDATE_2D);
	m_pRealVideoMgr->Update(LTRV_UPDATE_3D);
	#endif // LITHTECH_ESD

    graph_mgr->Mgr()->Draw();

    if (r_GetRenderStruct()->m_bInitted) {
        r_GetRenderStruct()->SwapBuffers(flags); }
}


LTRESULT CClientMgr::PlaySound(PlaySoundInfo *pPlaySoundInfo, FileRef *pFile, float fOffsetTime)
{
    FileIdentifier *pIdent;
    const char *pFileName;

 	LTRESULT result;

    if (!pPlaySoundInfo) {
        return LT_ERROR;
    }

    if (!GetClientILTSoundMgrImpl()->IsValid() || !GetClientILTSoundMgrImpl()->IsEnabled()) {
        return LT_ERROR;
    }

    pIdent = client_file_mgr->GetFileIdentifier(pFile, TYPECODE_SOUND);
    if (!pIdent) {
        pFileName = client_file_mgr->GetFilename(pFile);

        if (g_DebugLevel >= 2)
        {
            dsi_PrintToConsole("Missing sound file %s", pFileName);
        }

        return LT_ERROR;
    }

 	result = GetClientILTSoundMgrImpl()->PlaySound( *pPlaySoundInfo, *pIdent, ( uint32 )( fOffsetTime * 1000.0f + 0.5f ));

    if (result != LT_OK) {
        return result;
    }

    // Tell the client shell we're playing a sound
    i_client_shell->OnPlaySound(pPlaySoundInfo);

    return result;
}


void CClientMgr::UpdateAllSounds()
{
	CountAdder cntAdd(&g_Ticks_Sound);

    if (!GetClientILTSoundMgrImpl()->IsValid() || !GetClientILTSoundMgrImpl()->IsEnabled())
        return;

    GetClientILTSoundMgrImpl()->Update();

	ILTSoundSys* pSoundSys = GetSoundSys( );
	if( pSoundSys )
	{
		g_Ticks_SoundThread += pSoundSys->GetThreadedSoundTicks( );
	}

    if (g_bSoundShowCounts)
    {
        con_Printf(CONRGB(255,0,0), 0, "Sounds: playing:%d, heard:%d", GetClientILTSoundMgrImpl()->GetNumSoundsPlaying(),
            GetClientILTSoundMgrImpl()->GetNumSoundsHeard());
    }
}


// ------------------------------------------------------------------ //
// Internal helpers.
// ------------------------------------------------------------------ //

static void ClientStringWhine(const char *pString, void *pUser)
{
    con_WhitePrintf("Unfreed string: %s", pString);
}



static void FreeSpriteList(LTList *pList)
{
    LTLink *pCur, *pNext, *pListHead;

    pListHead = &pList->m_Head;
    pCur = pListHead->m_pNext;
    while (pCur != pListHead)
    {
        pNext = pCur->m_pNext;
        spr_Destroy((Sprite*)pCur->m_pData);
        pCur = pNext;
    }

    dl_InitList(pList);
}


// ------------------------------------------------------------------ //
// C routines.
// ------------------------------------------------------------------ //

void cm_Init()
{
    LT_MEM_TRACK_ALLOC(g_pClientMgr = new CClientMgr, LT_MEM_TYPE_MISC);

    // fail if we can't allocate the client mgr
    if (g_pClientMgr == NULL)
		return;

    g_pClientMgr->m_ObjectMap = NULL;
    g_pClientMgr->m_ObjectMapSize = 0;

    #ifdef LITHTECH_ESD
    // set up the real audio manager
    g_pClientMgr->m_pRealAudioMgr = new CLTRealAudioMgr;

    // set up the real audio player
    g_pRealAudioPlayer = LTNULL;

    // set up the real video manager
    g_pClientMgr->m_pRealVideoMgr = new CLTRealVideoMgr;

    // set up the real video player
    g_pRealVideoPlayer = LTNULL;

    // set up the real console manager
    g_pClientMgr->m_pRealConsoleMgr = new CLTRealConsoleMgr;
    #endif

    // set up the benchmark manager
    ilt_benchmark->Init();

    // No errors so far...
    g_pClientMgr->m_ErrorString[0] = '\0';

    g_pClientMgr->m_bNotifyRemoves = true;
    g_pClientMgr->m_pCurShell = NULL;
    g_pClientMgr->m_pOldShell = NULL;
    g_pClientMgr->m_bInCurShellUpdate = false;
    g_pClientMgr->m_hShellModule = NULL;
    g_pClientMgr->m_hClientResourceModule = NULL;
    g_pClientMgr->m_hLocalizedClientResourceModule = NULL;
    LTStrCpy(g_pClientMgr->m_MusicDLLName, "cdaudio.dll", sizeof(g_pClientMgr->m_MusicDLLName));
    g_pClientMgr->m_MusicMgr.m_bValid = 0;

    g_pClientMgr->m_bCanSaveConfigFile = false;

    dl_InitList(&g_pClientMgr->m_Sprites);

    VEC_SET(g_pClientMgr->m_GlobalLightScale, 1.0f, 1.0f, 1.0f);

    //initialize the client file mgr.
    client_file_mgr->Init();

    // Initialize all the object banks.
    om_Init(&g_pClientMgr->m_ObjectMgr, true);

    LT_MEM_TRACK_ALLOC(sb_Init2(&g_pClientMgr->m_FileIDInfoBank, sizeof(FileIDInfo), 32, 32), LT_MEM_TYPE_FILE);

    LT_MEM_TRACK_ALLOC(g_pClientMgr->m_SharedTextureBank.Init(64, 64), LT_MEM_TYPE_TEXTURE);

    // Initialize all the lists.
    dl_InitList(&g_pClientMgr->m_SharedTextures);

    input_GetManager(&g_pClientMgr->m_InputMgr);

    // Init the net manager.
    g_pClientMgr->m_NetMgr.Init("Player");

    g_pClientMgr->m_bRendering = false;

	g_pClientMgr->m_nLastUpdateTimeMS = time_GetMSTime();
	g_pClientMgr->m_nCurUpdateTimeMS = 0;
	g_pClientMgr->m_nFrameTimeMS = 0;

	g_pClientMgr->m_FrameTime = 0.0f;
	g_pClientMgr->m_CurTime = 0.0f;

    g_pClientMgr->m_fLastPingTime = 0.0f;

    g_pClientMgr->m_nSkyObjects = 0;
    g_pClientMgr->m_ExceptionFlags = 0;
    g_pClientMgr->m_bInputState = false;
    g_pClientMgr->m_bTrackingInputDevices = false;
    g_pClientMgr->m_ModelHookFn = NULL;
    g_pClientMgr->m_ModelHookUser = NULL;
    g_pClientMgr->m_nResTrees = 0;

    LT_MEM_TRACK_ALLOC(g_pClientMgr->m_MoveAbstract = new CMoveAbstract(), LT_MEM_TYPE_MISC);

    // Check everything..
    if (!g_pClientMgr->m_MoveAbstract)
    {
        g_pClientMgr->Term();
        delete g_pClientMgr;
    }
}

CClientMgr::~CClientMgr() {
	ASSERT(m_pOldShell == NULL);    
}

void CClientMgr::Term()
{
    int i;

    m_bNotifyRemoves = false;

    // Tell the client shell we're going away.
    if (i_client_shell) {
        i_client_shell->OnEngineTerm();
    }

	// Turn off rendering
    r_TermRender(2, true);

    // Kill all the shells.  This must be before saving the config so the console state can
    // be restored for any demo playbacks.
    EndShell();

#ifdef COMPILE_JUPITER_EVAL
	m_WaterMark.Term();
#endif // COMPILE_JUPITER_EVAL

	if (m_pOldShell != NULL)
	{
		delete m_pOldShell;
		m_pOldShell = NULL;
	}

    TermClientShellDE();

    // Get rid of ALL objects.  Should be done before closing the shell (and world)
    // so WorldModel's WorldData pointers are valid.
    for (i=0; i < NUM_OBJECTTYPES; i++) {
        RemoveObjectsInList(&m_ObjectMgr.m_ObjectLists[i], false);
    }

    SaveAutoConfig("autoexec.cfg");

    // Stop getting input.
    m_InputMgr->Term(m_InputMgr);

    memset(m_SkyObjects, 0xFF, sizeof(m_SkyObjects));

    GetClientILTSoundMgrImpl()->Term();

    // Kill the music driver...
    AppTermMusic();

    // Kill the graph manager
    graph_mgr->Mgr()->Term();


    FreeSpriteList(&m_Sprites);

    con_Term(true);

    m_NetMgr.Term();

    FreeSharedTextures();   // This must come after r_TermRender because
                                    // r_TermRender accesses the shared textures.
                                    // ALSO: this MUST come after TermClientShellDE because
                                    // if it comes before, the client shell could have HTEXTUREs
                                    // (SharedTexture*'s) to invalid data.


    c_TermConsoleCommands();


    om_Term(&m_ObjectMgr);


    // Cleanup the object map.
    if (m_ObjectMap)
    {
        dfree(m_ObjectMap);
        m_ObjectMap = NULL;
    }

    m_ObjectMapSize = 0;


    sb_Term(&m_FileIDInfoBank);

    // Kill the ClientDE interface..
    ci_Term();
    str_ShowAllStringsAllocated(ClientStringWhine, NULL);

    // Kill the file manager.
    client_file_mgr->Term();

    // terminate the font manager
    font_manager->Term();

    TermErrorLog();

    if (m_MoveAbstract) {
        delete m_MoveAbstract;
        m_MoveAbstract = NULL;
    }
}

bool CClientMgr::IsModelCached( const char *pFilename )
{
	Model *pModel = g_ModelMgr.Find( pFilename );
	// Model::Find( pFilename);
	if( pModel )
	{
		if( pModel->m_Flags & MODELFLAG_CACHED_CLIENT )
			return true ;
	}

	return false ;
}


bool CClientMgr::IsModelCached( const FileRef &file_ref )
{

	Model *pModel = g_ModelMgr.Find( file_ref.m_FileID );

	if( pModel )
	{
		if( pModel->m_Flags & MODELFLAG_CACHED_CLIENT )
			return true ;
	}

	return false ;
}


LTRESULT CClientMgr::CacheModelFile( const char * pFilename )
{
	if( IsModelCached( pFilename ) == false )
	{
		Model *pModel ; // new model
		if( LoadModel( pFilename, pModel ) == LT_OK )
		{
			DEBUG_MODEL_REZ(("model-rez: client CacheModelFile %s", pFilename ));
			pModel->AddRef();
			pModel->m_Flags |= MODELFLAG_CACHED_CLIENT;
			return LT_OK;
		}
		else
		{
			DEBUG_MODEL_REZ(("model-rez: client CacheModelFile could not load %s ", pFilename ));
			return LT_ERROR;
		}
	}else
	{
		DEBUG_MODEL_REZ(("model-rez: client CacheModelFile %s already loaded.", pFilename ));
		return LT_OK ;
	}
}

LTRESULT CClientMgr::CacheModelFile( const FileRef &file_ref  )
{
	if( IsModelCached( file_ref ) == false )
	{
		Model *pModel ; // new model
		if( LoadModel( file_ref, pModel ) == LT_OK )
		{
			DEBUG_MODEL_REZ(("model-rez: client CacheModelFile %s", pModel->GetFilename() ));
			pModel->AddRef();
			pModel->m_Flags |= MODELFLAG_CACHED_CLIENT;
			return LT_OK;
		}
		else
		{
			DEBUG_MODEL_REZ(("model-rez: client CacheModelFile could not load %d ",  file_ref.m_FileID ));
			return LT_ERROR;
		}
	}
	else
	{
		DEBUG_MODEL_REZ(("model-rez: client CacheModelFile %d already loaded.", file_ref.m_FileID ));
		return LT_OK ;
	}
}

LTRESULT CClientMgr::UncacheModelFile( Model *pModel )
{
	// is the model cached ?
	if( pModel && (pModel->m_Flags & MODELFLAG_CACHED_CLIENT ) )
	{
		pModel->m_Flags &= ~MODELFLAG_CACHED_CLIENT ;
		DEBUG_MODEL_REZ(("model-rez: client UncacheModelFile fileID :%d %s", pModel->m_FileID, pModel->GetFilename()));
		pModel->Release();
		return LT_OK;
	}
	return LT_ERROR;
}

/* ------------------------------------------------------------------------
 lt-result LoadModel( file-ref , retmodel )
 returns load errors state, or lt-ok.

 The client side load model has to fulfill the following jobs:
 1. work in a stand alone client,
 2. work in conjuntion with the server where model databases are shared
    among the client and server.

 Case 1: the client has to match up the client side files
 with the server side files. Generally this is easy because the server tells
 the client which files to load and what their 'file-id' number is. When the client
 loads a file, the client has no id for that file, and confusion can arise when
 the server tells the client to load a file that has been loaded but has no fild id.
 this is solved by associating a client file with a server side id by searching for files
 with filenames first.

 Case 2 is ideal, because the model dbs are shared; information is kept by the dbs.
 when the server sets the db, the client spontaneously knows of the change.
------------------------------------------------------------------------ */
LTRESULT CClientMgr::LoadModel( const FileRef & file_ref , Model *&pModel )
{
	FileIdentifier* pIdent;

	pIdent = client_file_mgr->GetFileIdentifier( const_cast<FileRef*>(&file_ref), TYPECODE_MODEL );

	if( pIdent )
	{
		// The client was asked to load a file by the server.
		if( file_ref.m_FileType == FILE_SERVERFILE )
		{
			pModel = g_ModelMgr.Find(file_ref.m_FileID );

			// let's try searching again with the filename.
			// the server may tell client to load files after the client has already loaded it.
			// this happens when a model loads a child model.
			if( pModel == NULL )
			{
				if( pIdent->m_Filename )
				{
					pModel = g_ModelMgr.Find( pIdent->m_Filename );
					// inherit the server's file id, if we got a model.
					if(pModel)
						pModel->m_FileID = file_ref.m_FileID ;
				}
			}
		}
		else
		{
			pModel = g_ModelMgr.Find( file_ref.m_pFilename );
		}

		// the file has been loaded already... move on.
		if( pModel )
		{
			// AddDebugMessge( "using existing file " );
			return LT_OK ;
		}

		// else we must load the data from disk.
		LTRESULT dResult ;
		dResult = LoadModelData( pIdent, pModel);

		if(dResult == LT_OK )
		{
			// add-debug-msg client-load-model pmodel-name, refcount, file id
			DEBUG_MODEL_REZ(("model-rez: client loadmodel [%d] %s", pModel, pModel->GetFilename() )) ;
			// if the server is not synoptic with the client, the client model
			// does not know that what its file id is, even though its been
			// requested from the server, and as in the synoptic version, should know
			// its file id. This is a redundant op if client and server are synoptic.
			if( file_ref.m_FileType == FILE_SERVERFILE )
			{
				pModel->m_FileID = file_ref.m_FileID ;
			}
		}
		else
		{
			DEBUG_MODEL_REZ(("model-rez: client loadmodel failed [%d] %s",  ((pIdent)?pIdent->m_FileID:-1), ((pIdent)?pIdent->m_Filename:"null") )) ;
		}

		return dResult ;
	}
	else // file missing.
    {
        RETURN_ERROR(1, CClientMgr_LoadModel, LT_MISSINGFILE);
		DEBUG_MODEL_REZ(("model-rez: client loadmodel missing file :[%d] %s", ((pIdent)?pIdent->m_FileID:-1), ((pIdent)?pIdent->m_Filename:"null") )) ;

    }
}


LTRESULT CClientMgr::LoadModel( const char *filename, Model*& pModel )
{
	FileRef         file_ref;

	file_ref.m_FileType = FILE_ANYFILE;
	file_ref.m_pFilename = filename ;

	return LoadModel( file_ref, pModel );
}


static LTRESULT ClientLoadChildModelCB(ModelLoadRequest *pRequest, Model **ppModel)
{
    const int kFilenameLen = _MAX_PATH + 1;
    char basePath[ kFilenameLen ], newFilename[ kFilenameLen ];
    char Filename [ kFilenameLen ];
    char *BaseFilename =NULL;
    *ppModel = LTNULL;

    BaseFilename = (char*)pRequest->m_pLoadFnUserData;

    // Add the filename to the previous path.
    CHelpers::ExtractNames(BaseFilename, basePath, LTNULL, LTNULL, LTNULL);
    if (basePath[0] == 0)
        LTSNPrintF(Filename, sizeof(Filename), "%s", pRequest->m_pFilename);
    else
        LTSNPrintF(Filename, sizeof(Filename), "%s\\%s", basePath, pRequest->m_pFilename);

	CHelpers::FormatFilename(Filename, newFilename, kFilenameLen);

	return g_pClientMgr->LoadModel(newFilename,*ppModel) ;
}


LTRESULT CClientMgr::LoadModelData(FileIdentifier *pIdent, Model* &pModel)
{
    ModelLoadRequest request;
    LTRESULT dResult;

	const char *pFilename = pIdent->m_Filename ;

    request.m_pFile = client_file_mgr->OpenFileIdentifier(pIdent);
    if (!request.m_pFile)
    {
        SetupError(LT_MISSINGMODELFILE, pFilename);
        RETURN_ERROR_PARAM(1, LoadModelData, LT_MISSINGMODELFILE, pFilename);
    }

	LT_MEM_TRACK_ALLOC(pModel = new Model, LT_MEM_TYPE_MODEL);
    if (!pModel)
    {
	    request.m_pFile->Release();
        RETURN_ERROR(1, cm_LoadModelData, LT_OUTOFMEMORY);
    }

	request.m_LoadChildFn = ClientLoadChildModelCB;
    request.m_pLoadFnUserData = (void*)pFilename;
    dResult = pModel->Load(&request, pFilename);

    request.m_pFile->Release();

    if (dResult != LT_OK )
    {
        delete pModel;
		pModel = NULL ;
        DEBUG_MODEL_REZ(("Error %d loading model %s", dResult, pFilename));
        return dResult;
    }
    else if (!request.m_bTreesValid)
    {
        delete pModel;
        DEBUG_MODEL_REZ(("Child model trees invalid in model %s", pFilename));
        RETURN_ERROR(1, LoadModelData, LT_INVALIDMODELFILE);
    }
    else if (!request.m_bAllChildrenLoaded)
    {
        delete pModel;
        DEBUG_MODEL_REZ(("Missing one or more child models in model %s", pFilename));
        RETURN_ERROR(1, LoadModelData, LT_INVALIDMODELFILE);
    }

    return dResult;
}

// ------------------------------------------------------------------------
// UncacheModels
// if a model was cached on client, un-cache.
// ------------------------------------------------------------------------
void  CClientMgr::UncacheModels()
{
	g_ModelMgr.UncacheClientModels( );
}


void CClientMgr::OnEnterServer() {
    ASSERT(m_pCurShell);

    client_file_mgr->OnConnect(m_pCurShell->m_HostID);

    // Init the server mirror state.
    memset(&m_ServerConsoleMirror, 0, sizeof(m_ServerConsoleMirror));
    m_ServerConsoleMirror.Alloc = dalloc;
    m_ServerConsoleMirror.Free = dfree;
    cc_InitState(&m_ServerConsoleMirror);
}


void CClientMgr::OnExitServer(CClientShell *pShell) {
    if (pShell != m_pCurShell)
        return;

    memset(m_SkyObjects, 0xFF, sizeof(m_SkyObjects));

    // Tell the file manager.
    client_file_mgr->OnDisconnect();

    // Shutdown the server mirror console state.
    cc_TermState(&m_ServerConsoleMirror);
}

void CClientMgr::RunCommandLine(uint32 flags)
{
	uint32 nNumArgs = command_line_args->Argc();

	if(nNumArgs == 0)
		return;

    char fullCmd[512];
    for (uint32 nCurrArg = 0; nCurrArg < nNumArgs - 1; nCurrArg++)
    {
		const char* pszCurrArg = command_line_args->Argv(nCurrArg);

        if (pszCurrArg[0] == '+')
        {
			const char* pszCurrArgParam = command_line_args->Argv(nCurrArg + 1);
            LTSNPrintF(fullCmd, sizeof(fullCmd), "(%s) (%s)", &pszCurrArg[1], pszCurrArgParam);

            cc_HandleCommand2(&g_ClientConsoleState, fullCmd, flags);
        }
    }
}


void CClientMgr::RunAutoConfig(const char *pFilename, uint32 flags) {
    cc_RunConfigFile(&g_ClientConsoleState, pFilename, flags, VARFLAG_SAVE);
}


void CClientMgr::SaveAutoConfig(const char *pFilename) {
    if (m_bCanSaveConfigFile)
    {
        cc_SaveConfigFile(&g_ClientConsoleState, pFilename);
    }
}


LTRESULT CClientMgr::StartRenderFromGlobals() {
    RMode mode;

    _GetRModeFromConsoleVariables(&mode);
    return r_InitRender(&mode);
}

void CClientMgr::RebindTextures()
{
	r_TerminateAllRecreatableTextureData();
    UnbindSharedTextures(false);
    BindSharedTextures();
}

//generates a log to the specified file detailing the texture memory breakdown
void CClientMgr::LogTextureMemory(const char* pszFilename)
{
	//first off try and open up the file for writing
	FILE* fp = fopen(pszFilename, "wt");

	if(!fp)
	{
		dsi_ConsolePrint("Error opening file %s for writing to generate texture log", pszFilename);
		return;
	}

	//create our file header
	fprintf(fp, "Texture name, Loaded, Memory Used (k), Width, Height, Mip Maps\n");

	//alright, now run through our lists and generate the log
    LTLink* pListHead = &m_SharedTextures.m_Head;
    for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
    {
		//alright, we have a shared texture, write out its info
		SharedTexture* pTexture = (SharedTexture*)pCur->m_pData;

		//make sure it is valid
		if(!pTexture)
			continue;

		//get the name of the texture
		const char* pszFilename = (pTexture->m_pFile) ? pTexture->m_pFile->m_Filename : "User Texture";

		//determine if this has been loaded yet
		bool bLoaded = (pTexture->m_pEngineData != NULL);

		//now determine info if it is loaded, or just use defaults
		uint32 nMemoryUsage = 0;
		uint32 nNumMips = 0;
		uint32 nWidth = 0;
		uint32 nHeight = 0;

		if(bLoaded)
		{
			TextureData* pData = (TextureData*)pTexture->m_pEngineData;
			nMemoryUsage	= pData->m_AllocSize;
			nWidth			= pData->m_Header.m_BaseWidth;
			nHeight			= pData->m_Header.m_BaseHeight;
			nNumMips		= pData->m_Header.GetNumMipmaps();
		}

		//now write out our data
		fprintf(fp, "%s, %s, %d, %d, %d, %d\n", pszFilename, bLoaded ? "Yes" : "No", nMemoryUsage / 1024,
												 nWidth, nHeight, nNumMips);
	}

	fclose(fp);
}


void CClientMgr::RemoveObjectsInList(LTList *pList, bool bServerOnly) {
    LTLink *pCur, *pNext, *pListHead;
    LTObject *pObj;

    pListHead = &pList->m_Head;
    pCur = pListHead->m_pNext;
    while (pCur != pListHead)
    {
        pNext = pCur->m_pNext;

        // Only remove objects that came from the server.
        pObj = (LTObject*)pCur->m_pData;

        if (bServerOnly) {
            if (pObj->m_ObjectID != (uint16)-1) {
                RemoveObjectFromClientWorld(pObj);
            }
        }
        else {
            RemoveObjectFromClientWorld(pObj);
        }

        pCur = pNext;
    }
}

// ---------------------------------------------------------------------- //
// Leeches.
// ---------------------------------------------------------------------- //

// Note : This is JUST to clean up the object lists.  Other functions do the actual
// notifications and whatnot
void CClientMgr::RemoveClientObject(LTObject *pObj)
{
	LTLink *pListHead = &m_ObjectMgr.m_ObjectLists[pObj->GetObjType()].m_Head;
    LTLink *pCur = pListHead->m_pNext;
    while (pCur != pListHead)
    {
        LTLink *pNext = pCur->m_pNext;
		if ((LTObject*)pCur->m_pData == pObj)
			dl_Remove(pCur);
		pCur = pNext;
	}
}

///////////////////////////////////////////////////////////////////////////////
// BSP world rep functionality
bool CClientMgr::Render(CameraInstance *pCamera, int drawMode, LTObject **pObjects, int nObjects, float fFrameTime)
{
    uint32 width, height, i;
    SceneDesc *pDesc;
    int renderStatus;
    RenderStruct *pRenderStruct;
    Counter renderTicks;
    SceneDesc sceneDesc;
    LTObject *skyObjects[MAX_SKYOBJECTS];

    if (!r_IsRenderInitted() || m_bRendering)
        return false;

    pDesc = &sceneDesc;
    pRenderStruct = r_GetRenderStruct();

    pDesc->m_DrawMode = drawMode;

    // Clear the background.
    width = pRenderStruct->m_Width;
    height = pRenderStruct->m_Height;

    // Get stuff from the camera object.
    if (m_pCurShell)
        pDesc->m_hRenderContext = (HRENDERCONTEXT)world_bsp_client->RenderContext();
    else
        pDesc->m_hRenderContext = NULL;

    pDesc->m_Pos = pCamera->GetPos();
    pDesc->m_Rotation = pCamera->m_Rotation;
    pDesc->m_xFov = pCamera->m_xFov;
    pDesc->m_yFov = pCamera->m_yFov;

    if (pCamera->m_bFullScreen)
    {
        pDesc->m_Rect.left = pDesc->m_Rect.top = 0;
        pDesc->m_Rect.right = width;
        pDesc->m_Rect.bottom = height;
    }
    else
    {
        pDesc->m_Rect.left = pCamera->m_Left;
        pDesc->m_Rect.top = pCamera->m_Top;
        pDesc->m_Rect.right = pCamera->m_Right;
        pDesc->m_Rect.bottom = pCamera->m_Bottom;
    }

    pDesc->m_GlobalLightScale = m_GlobalLightScale;
    pDesc->m_GlobalLightAdd = pCamera->m_LightAdd;

    pDesc->m_GlobalModelLightAdd = g_ConsoleModelAdd;

	pDesc->m_pTicks_Render_Objects = &g_Ticks_Render_Objects;
	pDesc->m_pTicks_Render_Models = &g_Ticks_Render_Models;
	pDesc->m_pTicks_Render_Sprites = &g_Ticks_Render_Sprites;
	pDesc->m_pTicks_Render_WorldModels = &g_Ticks_Render_WorldModels;
	pDesc->m_pTicks_Render_ParticleSystems = &g_Ticks_Render_ParticleSystems;
	pDesc->m_FrameTime = fFrameTime;
	pDesc->m_fActualFrameTime = m_FrameTime;

    pDesc->m_pObjectList = pObjects;
    pDesc->m_ObjectListSize = nObjects;

    // Sky info.
    memcpy(&pDesc->m_SkyDef, &m_SkyDef, sizeof(pDesc->m_SkyDef));

    pDesc->m_nSkyObjects = 0;
    pDesc->m_SkyObjects = skyObjects;
    for (i=0; i < MAX_SKYOBJECTS; i++)
    {
        skyObjects[pDesc->m_nSkyObjects] = FindObject(m_SkyObjects[i]);
        if (skyObjects[pDesc->m_nSkyObjects])
            ++pDesc->m_nSkyObjects;
    }

    // Model hook stuff.
    pDesc->m_ModelHookFn = m_ModelHookFn;
    pDesc->m_ModelHookUser = m_ModelHookUser;

    m_bRendering = true;

	{
		CountAdder cntAdd(&g_Ticks_Render);

        // Set Internal DrawPrim's camera to the current rendering camera
        // This is required for model OBBs to draw properly.
        g_pLTDrawPrimInternal->SetCamera(static_cast<HOBJECT>(pCamera));

		if (g_CV_RenderEnable) {
			renderStatus = pRenderStruct->RenderScene(&sceneDesc);
		}

#ifdef COMPILE_JUPITER_EVAL
    	m_WaterMark.Draw();
#endif // COMPILE_JUPITER_EVAL

        // Reset the camera to NULL
        // This is required for the console to draw properly
        g_pLTDrawPrimInternal->SetCamera(NULL);

		if (g_CV_DrawDebugGeometry) {
			drawAllDebugGeometry(); }                       // Draw all debug geometry here...
	}

    m_bRendering = false;

	#ifdef LITHTECH_ESD
	if (g_pRealVideoPlayer)
		g_pRealVideoPlayer->Render();
	#endif // LITHTECH_ESD

    return true;
}

bool CClientMgr::MakeCubicEnvMap(CameraInstance *pCamera, uint32 nSize, const char* pszPrefix)
{
    uint32 width, height, i;
    RenderStruct *pRenderStruct;
    Counter renderTicks;
    SceneDesc Desc;
    LTObject *skyObjects[MAX_SKYOBJECTS];

    if (!r_IsRenderInitted() || m_bRendering)
        return false;

    pRenderStruct = r_GetRenderStruct();

    Desc.m_DrawMode = DRAWMODE_NORMAL;

    // Clear the background.
    width = nSize;
    height = nSize;

    // Get stuff from the camera object.
    if (m_pCurShell)
        Desc.m_hRenderContext = (HRENDERCONTEXT)world_bsp_client->RenderContext();
    else
        Desc.m_hRenderContext = NULL;

    Desc.m_Pos = pCamera->GetPos();
    Desc.m_Rotation = pCamera->m_Rotation;
    Desc.m_xFov = pCamera->m_xFov;
    Desc.m_yFov = pCamera->m_yFov;

    if (pCamera->m_bFullScreen)
    {
		Desc.m_Rect.left = Desc.m_Rect.top = 0;
    }
    else
    {
        Desc.m_Rect.left = pCamera->m_Left;
        Desc.m_Rect.top = pCamera->m_Top;
    }

    Desc.m_Rect.right  = Desc.m_Rect.left + width;
    Desc.m_Rect.bottom = Desc.m_Rect.top + height;

    Desc.m_GlobalLightScale = m_GlobalLightScale;
    Desc.m_GlobalLightAdd = pCamera->m_LightAdd;

    Desc.m_GlobalModelLightAdd = g_ConsoleModelAdd;

    Desc.m_pObjectList		= NULL;
    Desc.m_ObjectListSize	= 0;
	Desc.m_pTicks_Render_Objects = &g_Ticks_Render_Objects;
	Desc.m_pTicks_Render_Models = &g_Ticks_Render_Models;
	Desc.m_pTicks_Render_Sprites = &g_Ticks_Render_Sprites;
	Desc.m_pTicks_Render_WorldModels = &g_Ticks_Render_WorldModels;
	Desc.m_pTicks_Render_ParticleSystems = &g_Ticks_Render_ParticleSystems;
	Desc.m_FrameTime		= 0.0f;
	Desc.m_fActualFrameTime	= 0.0f;

    // Sky info.
    memcpy(&Desc.m_SkyDef, &m_SkyDef, sizeof(Desc.m_SkyDef));

    Desc.m_nSkyObjects = 0;
    Desc.m_SkyObjects = skyObjects;
    for (i=0; i < MAX_SKYOBJECTS; i++)
    {
        skyObjects[Desc.m_nSkyObjects] = FindObject(m_SkyObjects[i]);
        if (skyObjects[Desc.m_nSkyObjects])
            ++Desc.m_nSkyObjects;
    }

    // Model hook stuff.
    Desc.m_ModelHookFn = m_ModelHookFn;
    Desc.m_ModelHookUser = m_ModelHookUser;

    m_bRendering = true;

    if (g_CV_RenderEnable)
	{
        pRenderStruct->MakeCubicEnvMap(pszPrefix, nSize, Desc);
    }

    m_bRendering = false;

    return true;
}

LTRESULT CClientMgr::AddObjectToClientWorld(uint16 objectID,
    InternalObjectSetup *pSetup, LTObject **ppObject, bool bMove, bool bRotate)
{
    LTObject *pObject;
    LTRESULT dResult;

    *ppObject = NULL;

    // Create the main object.
    dResult = om_CreateObject(&m_ObjectMgr, pSetup->m_pSetup, &pObject);
    if (dResult != LT_OK)
        return dResult;

    // Init the object and client-specific data.
    dResult = so_ExtraInit(pObject, pSetup, false);
    if (dResult != LT_OK)
    {
        om_DestroyObject(&m_ObjectMgr, pObject);
        return dResult;
    }

    // Setup its object ID and other data.
    pObject->m_ObjectID = objectID;
    if (pObject->m_ObjectID != OBJID_CLIENTCREATED)
    {
        // If it's a server object, add it to the server object list.
        AddToObjectMap(pObject->m_ObjectID);
        m_ObjectMap[pObject->m_ObjectID].m_nRecordType = RECORDTYPE_LTOBJECT;
        m_ObjectMap[pObject->m_ObjectID].m_pRecordData = pObject;
    }


    // Now put it where they specified.
    if (bMove && bRotate)
        MoveAndRotateObject(pObject, &pSetup->m_pSetup->m_Pos, &pSetup->m_pSetup->m_Rotation);
    else if (bMove)
        MoveObject(pObject, &pSetup->m_pSetup->m_Pos, true);
    else if (bRotate)
        RotateObject(pObject, &pSetup->m_pSetup->m_Rotation);

    // Possibly scale it.
    if (pSetup->m_pSetup->m_Scale.x != 1.0f || pSetup->m_pSetup->m_Scale.y != 1.0f || pSetup->m_pSetup->m_Scale.z != 1.0f)
        ScaleObject(pObject, &pSetup->m_pSetup->m_Scale);

    if (HasWorldModel(pObject))
    {
        InitialWorldModelRotate(ToWorldModel(pObject));
    }

    *ppObject = pObject;
    return LT_OK;
}


LTRESULT CClientMgr::RemoveObjectFromClientWorld(LTObject *pObject)
{
    // Detach it from whatever it's standing on.
    DetachObjectStanding(pObject);
    DetachObjectsStandingOn(pObject);

    // Notify them if they want notification..
    if (pObject->cd.m_ClientFlags & CF_NOTIFYREMOVE) {
        if (i_client_shell != NULL) {
            i_client_shell->OnObjectRemove((HLOCALOBJ)pObject);
        }
    }

	// Notify the object's reference list
	pObject->NotifyObjRefList_Delete();

    // Remove it from the object map.
    if (pObject->m_ObjectID != (uint16)-1) {
        ClearObjectMapEntry(pObject->m_ObjectID);
    }

	RemoveClientObject(pObject);

    // Call the object's termination function
    so_ExtraTerm(pObject);

    // Unload un-used model textures..
    if (pObject->m_ObjectType == OT_MODEL)
	{
		//This function is currently not being called because it would constantly remove textures
		//and ruin caching. This leads to some wasted texture memory during the course of a level,
		//but does help prevent hitching
        //FreeUnusedModelTextures(pObject);
    }


    om_DestroyObject(&m_ObjectMgr, pObject);

    return LT_OK;
}


void CClientMgr::UpdateParticleSystems()
{
    LTParticleSystem  *pSystem;
    int nNewParticles;
    LTLink *pCur, *pListHead;
    uint32 flags;
    LTBOOL bChanged;

    if (m_nFrameTimeMS > 0)
    {
        pListHead = &m_ObjectMgr.m_ObjectLists[OT_PARTICLESYSTEM].m_Head;
        for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
        {
            pSystem = (LTParticleSystem*)pCur->m_pData;

            // Set its FLAG_WASDRAWN appropriately and avoid updating if possible.
            flags = pSystem->m_Flags;
            pSystem->m_Flags &= ~(FLAG_WASDRAWN | FLAG_INTERNAL1);
            if (!(flags & FLAG_UPDATEUNSEEN) && !(flags & FLAG_INTERNAL1) && (pSystem->m_nChangedParticles == 0))
            {
                continue;
            }
            pSystem->m_Flags |= FLAG_WASDRAWN;

			//now if we are paused though, we don't want to continue updating
			if(pSystem->IsPaused())
				continue;

            if (pSystem->m_pSprite)
            {
                spr_UpdateTracker(&pSystem->m_SpriteTracker, m_nFrameTimeMS);
                if (pSystem->m_SpriteTracker.m_pCurFrame)
                    pSystem->m_pCurTexture = pSystem->m_SpriteTracker.m_pCurFrame->m_pTex;
                else
                    pSystem->m_pCurTexture = LTNULL;
            }

            nNewParticles = pSystem->m_nChangedParticles;

            ps_StartUpdatingPositions(pSystem);
            ps_UpdateParticles(pSystem, m_FrameTime);
            bChanged = ps_EndUpdatingPositions(pSystem);

			// Possibly do a MoveObject() on it to get it into the correct node.
			if(nNewParticles > 0 || bChanged)
			{
				MoveObject(pSystem, &pSystem->GetPos(), LTTRUE);
			}

            pSystem->m_nChangedParticles = 0;
        }
    }
}

// update sprite anims only actually.
void CClientMgr::UpdateAnimations()
{
    LTLink *pCur, *pListHead;

    if (m_nFrameTimeMS > 0)
    {
        // Update sprite instances.
        pListHead = &m_ObjectMgr.m_ObjectLists[OT_SPRITE].m_Head;
        for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
        {
			SpriteInstance* pSprite = (SpriteInstance*)pCur->m_pData;

			if(!pSprite->IsPaused())
			{
				spr_UpdateTracker(&pSprite->m_SpriteTracker, m_nFrameTimeMS);
			}
        }
    }
}

void CClientMgr::UpdatePolyGrids() {
    LTLink *pListHead, *pCur;
    LTPolyGrid *pGrid;
    uint32 flags, msFrameTime;

    msFrameTime = m_nFrameTimeMS;
    if (msFrameTime)
    {
        pListHead = &m_ObjectMgr.m_ObjectLists[OT_POLYGRID].m_Head;
        for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
        {
            pGrid = (LTPolyGrid*)pCur->m_pData;

            // Set its FLAG_WASDRAWN appropriately and avoid updating if possible.
            flags = pGrid->m_Flags;
            pGrid->m_Flags &= ~(FLAG_WASDRAWN | FLAG_INTERNAL1);
            if (!(flags & FLAG_UPDATEUNSEEN) && !(flags & FLAG_INTERNAL1))
            {
                continue;
            }
            pGrid->m_Flags |= FLAG_WASDRAWN;

            if (!pGrid->IsPaused() && pGrid->m_pSprite)
            {
                spr_UpdateTracker(&pGrid->m_SpriteTracker, msFrameTime);
            }
        }
    }
}


void CClientMgr::UpdateLineSystems() {
    LTLink *pListHead, *pCur;
    LineSystem *pSystem;

    pListHead = &m_ObjectMgr.m_ObjectLists[OT_LINESYSTEM].m_Head;
    for (pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
    {
        pSystem = (LineSystem*)pCur->m_pData;

		if(pSystem->m_bChanged)
		{
			// Do MoveObject to get it located correctly.
			MoveObject(pSystem, &pSystem->GetPos(), LTTRUE);
		}

        pSystem->m_bChanged = LTFALSE;
    }
}



uint32 g_Ticks_UpdateObjects;

static void ClientStringKeyCallback(LTAnimTracker *pTracker, AnimKeyFrame *pFrame)
{
    ArgList argList;
    ConParse parse;
    ModelInstance *pInst;


    pInst = pTracker->GetModelInstance();
    if (!pInst)
        return;

    if (!(pInst->cd.m_ClientFlags & CF_NOTIFYMODELKEYS))
        return;

    if (i_client_shell == NULL) {
        return;
    }

    parse.Init(pFrame->m_pString);
    argList.argv = parse.m_Args;

    while (parse.Parse()) {
        if (parse.m_nArgs > 0) {
            argList.argc = parse.m_nArgs;
            i_client_shell->OnModelKey(pInst, &argList);
        }
    }
}


// Called by the server in a local game.
#ifdef USE_LOCAL_STUFF
    void clienthack_ModelKey(LTObject *pObject, ArgList *pArgList)
    {
        g_pClientMgr->m_pClientShellDE->OnModelKey(g_pClientMgr->m_pClientShellDE,
            (HLOCALOBJ)pObject, pArgList);
    }
#endif

void CClientMgr::UpdateModels()
{
	//determine if any time has even elapsed
    if (m_nFrameTimeMS)
    {
		//it has, so we need to run through and update all models
        LTLink *pListHead = &m_ObjectMgr.m_ObjectLists[OT_MODEL].m_Head;

		ModelInstance *pModel;
		uint32         i;

		for (LTLink *pCur = pListHead->m_pNext; pCur != pListHead; pCur = pCur->m_pNext)
        {
            pModel = (ModelInstance*)pCur->m_pData;

			//but don't update paused models
			if(!pModel->IsPaused())
			{
				//setup the string callback and update the model animation
				pModel->SetStringKeyCallback(ClientStringKeyCallback);
				pModel->ClientUpdate(m_nFrameTimeMS);

				//count down how many sprites we have to update. This is mainly because most
				//don't have sprites, or only a single one so it avoids the need to go through
				//every texture index since there can be quite a few
				uint32 nSpritesLeft = pModel->m_nNumSprites;

				for (i = 0; nSpritesLeft && (i < MAX_MODEL_TEXTURES); i++)
				{
					if (pModel->m_pSprites[i])
					{
						spr_UpdateTracker(&pModel->m_SpriteTrackers[i], m_nFrameTimeMS);

						FileRef        skinName;
						skinName.m_pFilename = pModel->m_SpriteTrackers[i].m_pCurFrame->m_pTex->m_pFile->m_Filename;
						skinName.m_FileType = FILE_CLIENTFILE;
						AddSharedTexture2(&skinName, pModel->m_pSkins[i]);

						//we have one sprite left to update
						nSpritesLeft--;
					}
				}
			}
        }
    }
}


void CClientMgr::UpdateObjects()
{
	g_Ticks_UpdateObjects = 0;
	CountAdder cntUpdate(&g_Ticks_UpdateObjects);

    UpdateAnimations();
    UpdateParticleSystems();
    UpdatePolyGrids();
    UpdateLineSystems();
    UpdateModels();

	IncrementFrameCode();

}


//
// CClientMgr::CDefaultNetHandler
//

void CClientMgr::CDefaultNetHandler::HandleUnknownPacket(const CPacket_Read &cPacket, uint8 senderAddr[4], uint16 senderPort)
{
    // Tell the client shell about unknown packets.
    if (i_client_shell != NULL)
        i_client_shell->ProcessPacket(CLTMsgRef_Read(CLTMessage_Read_Client::Allocate_Client(cPacket)), senderAddr, senderPort);
}

///////////////////////////////////////////////////////////////////////////////


