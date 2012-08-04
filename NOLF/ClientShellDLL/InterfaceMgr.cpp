// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMgr.cpp
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "ClientRes.h"
#include "WeaponStringDefs.h"
#include "VKDefs.h"
#include "SoundMgr.h"
#include "InterfaceResMgr.h"
#include "SCDefs.h"
#include "VarTrack.h"
#include "ClientButeMgr.h"
#include "CharacterFX.h"
#include "GameButes.h"
#include "FolderPerformance.h"

// TESTING DYNAMIC LIGHT IN INTERFACE
/*
#include "FXButeMgr.h"
static HOBJECT g_hLight = LTNULL;
*/
// END TEST

extern CGameClientShell* g_pGameClientShell;

CInterfaceMgr*  g_pInterfaceMgr = LTNULL;

#define IM_SPLASH_SOUND		"Menu\\Snd\\theme_mp3.wav"
#define IM_SPLASH_SCREEN	"Menu\\Art\\splash.pcx"

#define MAX_INTERFACE_SFX	50
#define MAX_FRAME_DELTA		0.1f
#define INVALID_ANI			((HMODELANIM)-1)

#define MOVIE_FOX_LOGO		0
#define MOVIE_MONOLITH_LOGO	1
#define MOVIE_LITHTECH_LOGO	2

LTFLOAT      g_fSplashSndDuration = 0.0f;
LTFLOAT      g_fFailScreenDuration = 0.0f;

LTFLOAT      g_fFovXTan = 0.0f;
LTFLOAT      g_fFovYTan = 0.0f;

LTVector     g_vOverlaySpriteScale(0.02f, 0.02f, 1.0f);
LTVector     g_vOverlayModelScale(1.0f, 1.0f, 1.0f);
LTFLOAT      g_fOverlaySpriteDist = 10.0f;
LTFLOAT      g_fOverlayModelDist = 0.25f;

LTVector     g_vBaseBackScale(0.8f, 0.6f, 1.0f);
LTFLOAT      g_fBackDist = 200.0f;

#define NUM_DEMO_SCREENS 2
HSURFACE    g_hSplash = LTNULL;
HSURFACE    g_hDemo = LTNULL;
int			g_nDemo = 0;
char  g_szDemoScreens[NUM_DEMO_SCREENS][32] =
{
	"Interface\\Marketing1.pcx",
	"Interface\\Marketing2.pcx",
};

HLTCOLOR    hDefaultTransColor = SETRGB_T(255,0,255);

VarTrack	g_vtDrawInterface;
VarTrack	g_vtModelApplySun;
VarTrack	g_vtChooserAutoSwitchTime;
VarTrack	g_vtChooserAutoSwitchFreq;
VarTrack	g_vtCursorHack;
VarTrack	g_vtLetterBox;
VarTrack	g_vtLetterBoxFadeInTime;
VarTrack	g_vtLetterBoxFadeOutTime;
VarTrack	g_vtDisableMovies;

extern VarTrack g_vtFOVXNormal;
extern VarTrack g_vtFOVYNormal;

extern VarTrack g_vtScreenFadeInTime;

const char* c_GameStateNames[] =
{
	"GS_UNDEFINED",
	"GS_PLAYING",
	"GS_LOADINGLEVEL",
	"GS_SPLASHSCREEN",
	"GS_DIALOGUE",
	"GS_MENU",
	"GS_POPUP",
	"GS_FOLDER",
	"GS_PAUSED",
	"GS_FAILURE",
	"GS_DEMOSCREEN",
	"GS_MOVIE"
};

namespace
{
	LTBOOL g_bLockPopup = LTFALSE;
	LTBOOL g_bLockStats = LTFALSE;
	LTBOOL g_bLockInv = LTFALSE;
	LTBOOL g_bLockFolder = LTFALSE;
	LTBOOL g_bFirstStateUpdate = LTFALSE;
	LTBOOL g_bInGameFogEnabled = LTTRUE;

	HLTCOLOR hShadeColor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::CInterfaceMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CInterfaceMgr::CInterfaceMgr()
{
	g_pInterfaceMgr	= this;

	m_eGameState			= GS_UNDEFINED;

    m_eCurrOverlay          = OVM_NONE;

	m_dwAdvancedOptions		= AO_DEFAULT_ENABLED;
	m_dwOrignallyEnabled	= 0;

	m_nClearScreenCount		= 0;
    m_bClearScreenAlways    = LTFALSE;

	m_fMenuSaveFOVx			= 0.0f;
	m_fMenuSaveFOVy			= 0.0f;

    m_bDrawPlayerStats      = LTTRUE;
    m_bDrawFragCount        = LTFALSE;
    m_bDrawInterface        = LTTRUE;
    m_bSwitchingModes       = LTFALSE;

    m_hSplashSound          = LTNULL;
	g_fSplashSndDuration	= 0.0f;

    m_nFailStringId         = LTNULL;
    m_hFailBack             = LTNULL;

    m_bMenuRectRestore      = LTTRUE;
	memset(&m_rcMenuRestoreCamera, 0, sizeof(m_rcMenuRestoreCamera));

	m_bUseInterfaceCamera	= LTTRUE;

	for (int i =0; i < NUM_OVERLAY_MASKS; i++)
	{
        m_hOverlays[i] = LTNULL;
		m_fOverlayScaleMult[i] = 1.0f;
	}
	m_nOverlayCount = 0;

    m_hScubaSound		= LTNULL;
	m_hSound			= LTNULL;

    m_hFadeSurface      = LTNULL;
    m_hLetterBoxSurface = LTNULL;
	m_fLetterBoxAlpha	= 0.0f;
	m_bLetterBox		= LTFALSE;
	m_bWasLetterBox		= LTFALSE;
	m_fLetterBoxFadeEndTime = 0.0f;

    m_bScreenFade       = LTFALSE;
    m_bFadeInitialized  = LTFALSE;
	m_fTotalFadeTime	= 0.0f;
	m_fCurFadeTime		= 0.0f;
    m_bFadeIn           = LTTRUE;
    m_bExitAfterFade    = LTFALSE;

    m_bUseCursor         = LTFALSE;
    m_bUseHardwareCursor = LTFALSE;

	m_eSunglassMode		= SUN_NONE;

    m_bSavedGameMusic   = LTFALSE;

	m_eNextSound		= IS_NONE;

	m_bQuitAfterDemoScreens = LTFALSE;
	m_bSeenDemoScreens = LTFALSE;

	m_hMovie			= LTNULL;
	m_nCurMovie			= 0;

	m_bLoadFailed		= LTFALSE;
	m_bStartedNew		= LTFALSE;

	m_hGamePausedSurface = LTNULL;

	m_fLastUpdateRequestTime = 0.0f;

	m_nLoadWorldCount = 0;
	m_nOldLoadWorldCount = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::~CInterfaceMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CInterfaceMgr::~CInterfaceMgr()
{
	if (m_hSplashSound)
	{
        g_pLTClient->KillSound(m_hSplashSound);
	}

	if (m_hScubaSound)
	{
        g_pLTClient->KillSound(m_hScubaSound);
	}

	if (m_hSound)
	{
        g_pLTClient->KillSound(m_hSound);
	}

	if (m_hFailBack)
	{
        g_pLTClient->DeleteSurface(m_hFailBack);
	}

	if (m_hFadeSurface)
	{
        g_pLTClient->DeleteSurface(m_hFadeSurface);
	}

	if (m_hLetterBoxSurface)
	{
        g_pLTClient->DeleteSurface(m_hLetterBoxSurface);
	}

    g_pInterfaceMgr = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
LTBOOL CInterfaceMgr::Init()
{

    if (!InitCursor())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Cursor!");
        return LTFALSE;
	}



	UseHardwareCursor( GetConsoleInt("HardwareCursor",0) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);

    g_vtDrawInterface.Init(g_pLTClient, "DrawInterface", NULL, 1.0f);
    g_vtModelApplySun.Init(g_pLTClient, "ModelApplySun", NULL, 1.0f);

	g_vtChooserAutoSwitchTime.Init(g_pLTClient, "ChooserAutoSwitchTime", NULL, 0.25f);
	g_vtChooserAutoSwitchFreq.Init(g_pLTClient, "ChooserAutoSwitchFreq", NULL, 0.175f);

    g_vtCursorHack.Init(g_pLTClient, "CursorHack", NULL, 0.0f);

	g_vtLetterBox.Init(g_pLTClient, "LetterBox", NULL, 0.0f);
    g_vtLetterBoxFadeInTime.Init(g_pLTClient, "LetterBoxFadeInTime", NULL, 0.5f);
    g_vtLetterBoxFadeOutTime.Init(g_pLTClient, "LetterBoxFadeOutTime", NULL, 1.0f);
    g_vtDisableMovies.Init(g_pLTClient, "NoMovies", NULL, 0.0f);

	ProcessAdvancedOptions();

    if (!m_ServerOptionMgr.Init(g_pLTClient))
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize ServerOptionMgr!");
        return LTFALSE;
	}

	// read in the settings
    m_Settings.Init (g_pLTClient, g_pGameClientShell);


    if (!m_LayoutMgr.Init(g_pLTClient))
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize LayoutMgr!");
        return LTFALSE;
	}
	hShadeColor =  m_LayoutMgr.GetShadeColor();

    if (!m_InterfaceResMgr.Init(g_pLTClient, g_pGameClientShell))
	{
		// If we couldn't init, something critical must have happened (like no render dlls)

        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize InterfaceResMgr!");
        return LTFALSE;
	}

    if (!m_FolderMgr.Init(g_pLTClient, g_pGameClientShell))
	{
		// If we couldn't init, something critical must have happened
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize FolderMgr!");
        return LTFALSE;
	}


	m_messageMgr.Init();
    m_messageMgr.Enable(LTTRUE);

    m_ClientInfo.Init();

	if (!m_stats.Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Player Stats!");
        return LTFALSE;
	}

	m_MissionText.Init();
	m_Subtitle.Init();
	m_PopupText.Init();

	// Create the main wnd (this is necessary to use the dialog wnd below)...

    if (!m_MainWnd.Init(0,"Main Window",LTNULL,g_pLTClient->GetScreenSurface()))
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Main Wnd!");
        return LTFALSE;
	}

	// Dialogue Window
	DIALOGUEWNDCREATESTRUCT dwcs;
	char szDlgBack[256]= "";
	char szDlgFrame[256]= "";
	char szDecBack[256]= "";

	dwcs.pParentWnd = &m_MainWnd;
	int dlgPos = g_pLayoutMgr->GetDialoguePosition();
	dwcs.yPos = dlgPos;

    LTIntPt dlgSz = g_pLayoutMgr->GetDialogueSize();
	dwcs.nWidth = dlgSz.x;
	dwcs.nHeight = dlgSz.y;

	g_pLayoutMgr->GetDialogueBackground(szDlgBack,sizeof(szDlgBack));
	dwcs.szBitmap = szDlgBack;

	g_pLayoutMgr->GetDialogueFrame(szDlgFrame,sizeof(szDlgFrame));
	dwcs.szFrame = szDlgFrame;

	g_pLayoutMgr->GetDecisionBackground(szDecBack,sizeof(szDecBack));
	dwcs.szDecisionBitmap = szDecBack;

	dwcs.fAlpha = g_pLayoutMgr->GetDialogueAlpha();
	dwcs.fDecisionAlpha = g_pLayoutMgr->GetDecisionAlpha();
	dwcs.pFont = m_InterfaceResMgr.GetMediumFont();

	dwcs.bFrame = (strlen(szDlgFrame) > 0);
	dwcs.bDecisionFrame = (strlen(szDlgFrame) > 0);


	dwcs.dwFlags = LTWF_SIZEABLE | LTWF_TOPMOST;
	dwcs.dwState = LTWS_CLOSED;

	if(!m_DialogueWnd.Init(&dwcs))
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Dialog Wnd!");
        return LTFALSE;
	}
    m_DialogueWnd.SetImmediateDecisions(LTTRUE);

	char szOpenSnd[256]= "";
	char szCloseSnd[256]= "";
	g_pLayoutMgr->GetDialogueOpenSound(szOpenSnd,sizeof(szOpenSnd));
	g_pLayoutMgr->GetDialogueCloseSound(szCloseSnd,sizeof(szCloseSnd));
	m_DialogueWnd.SetShowSounds(szOpenSnd,szCloseSnd);

	// Dialogue Window
	MENUWNDCREATESTRUCT mwcs;

	mwcs.pParentWnd = &m_MainWnd;
	dlgPos = g_pLayoutMgr->GetMenuPosition();
	mwcs.yPos = dlgPos;

	g_pLayoutMgr->GetMenuBackground(szDlgBack,sizeof(szDlgBack));
	mwcs.szBitmap = szDlgBack;

	g_pLayoutMgr->GetMenuFrame(szDlgFrame,sizeof(szDlgFrame));
	mwcs.szFrame = szDlgFrame;

	mwcs.fAlpha = g_pLayoutMgr->GetMenuAlpha();
	mwcs.pFont = m_InterfaceResMgr.GetSmallFont();

	mwcs.bFrame = (strlen(szDlgFrame) > 0);

	mwcs.dwFlags = LTWF_SIZEABLE | LTWF_TOPMOST;
	mwcs.dwState = LTWS_CLOSED;

	if(!m_MenuWnd.Init(&mwcs))
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Menu Wnd!");
        return LTFALSE;
	}
	g_pLayoutMgr->GetMenuOpenSound(szOpenSnd,sizeof(szOpenSnd));
	g_pLayoutMgr->GetMenuCloseSound(szCloseSnd,sizeof(szCloseSnd));
	m_MenuWnd.SetShowSounds(szOpenSnd,szCloseSnd);

	m_MultiplayerMenu.Init(&m_MenuWnd);
	m_TeamMenu.Init(&m_MenuWnd);
	m_OptionMenu.Init(&m_MenuWnd);

	m_CursorPos.x = 0;
	m_CursorPos.y = 0;

	g_fFovXTan = (float)tan(DEG2RAD(g_vtFOVXNormal.GetFloat())/2);
	g_fFovYTan = (float)tan(DEG2RAD(g_vtFOVYNormal.GetFloat())/2);

	m_MessageBox.Init();
	m_InterfaceMeter.Init();

	// Create the surface used for making letterboxed cinematics...
    LTRect rcSrc;
	rcSrc.Init(0, 0, 2, 2);
    HLTCOLOR hTransColor = g_pLTClient->SetupColor1(1.0f, 1.0f, 1.0f, LTTRUE);

	m_hLetterBoxSurface = g_pLTClient->CreateSurface(2, 2);

	g_pLTClient->SetSurfaceAlpha(m_hLetterBoxSurface, 0.0f);
    g_pLTClient->FillRect(m_hLetterBoxSurface, &rcSrc, kBlack);
    g_pLTClient->OptimizeSurface(m_hLetterBoxSurface, hTransColor);

	int nLevel = GetConsoleInt("PerformanceLevel",1);
	if (nLevel < 3)
	{
		CFolderPerformance* pFolder = (CFolderPerformance*)m_FolderMgr.GetFolderFromID(FOLDER_ID_PERFORMANCE);
		pFolder->SetOverall(nLevel);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::InitCursor
//
//	PURPOSE:	Init the cursor
//
// ----------------------------------------------------------------------- //
LTBOOL CInterfaceMgr::InitCursor()
{

	if (g_pLTClient->Cursor()->LoadCursorBitmapResource(MAKEINTRESOURCE(IDC_POINTER), m_hCursor) != LT_OK)
	{
		g_pLTClient->CPrint("can't load cursor resource.");
        return LTFALSE;
	}

    if (g_pLTClient->Cursor()->SetCursor(m_hCursor) != LT_OK)
	{
		g_pLTClient->CPrint("can't set cursor.");
        return LTFALSE;
	}


	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Term()
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::Term()
{
	if (m_hGamePausedSurface)
	{
		g_pInterfaceResMgr->FreeSharedSurface(m_hGamePausedSurface);
		m_hGamePausedSurface = LTNULL;
	}

	m_FolderMgr.Term();
	m_LayoutMgr.Term();
    m_ServerOptionMgr.Term();
    m_messageMgr.Term();
	m_stats.Term();
	m_WeaponChooser.Term();
	m_AmmoChooser.Term();
	m_InterfaceResMgr.Term();
	m_DialogueWnd.Term();
	m_MenuWnd.Term();
	m_MainWnd.Term();
	m_MessageBox.Term();
	m_InterfaceMeter.Term();
	m_PopupText.Term();
	m_Subtitle.Clear();
	m_Credits.Term();

	if ((m_dwOrignallyEnabled & AO_SOUND) && !(m_dwAdvancedOptions & AO_SOUND))
	{
        g_pLTClient->RunConsoleString("SoundEnable 1");
	}

	if ((m_dwOrignallyEnabled & AO_MUSIC) && !(m_dwAdvancedOptions & AO_MUSIC))
	{
        g_pLTClient->RunConsoleString("MusicEnable 1");
	}

	if (m_hScubaSound)
	{
        g_pLTClient->KillSound(m_hScubaSound);
		m_hScubaSound = LTNULL;
	}

	if (m_hSound)
	{
        g_pLTClient->KillSound(m_hSound);
		m_hSound = LTNULL;
	}


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnEnterWorld()
//
//	PURPOSE:	Handle entering new world
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnEnterWorld(LTBOOL bRestoringGame)
{
	m_stats.OnEnterWorld(bRestoringGame);
	++m_nLoadWorldCount;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnExitWorld()
//
//	PURPOSE:	Handle exiting a world
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnExitWorld()
{
	m_stats.OnExitWorld();
	m_ClientInfo.RemoveAllClients();
	m_MissionText.Clear();
	m_messageMgr.SetEditingState(LTFALSE, LTFALSE);
	m_Subtitle.Clear();
	m_Credits.Term();
	m_PopupText.Clear();
	m_InterfaceTimer.SetTime(0.0f,LTFALSE);
	m_InterfaceMeter.SetValue(0);
	if (m_AmmoChooser.IsOpen())
	{
		m_AmmoChooser.Close();
	}
	if (m_WeaponChooser.IsOpen())
	{
		m_WeaponChooser.Close();
	}

	if (m_DialogueWnd.IsVisible())
	{
		CLTDecisionWnd*	pDecWnd = m_DialogueWnd.GetDecisionWnd();
		if (pDecWnd && pDecWnd->IsVisible())
		{
			g_pLTClient->CPrint("ERROR: Decision window should not be visible on level exit.");
			pDecWnd->ShowWindow(LTFALSE,LTFALSE,LTFALSE);
		}

		g_pLTClient->CPrint("ERROR: Dialogue window should not be visible on level exit.");
		m_DialogueWnd.ShowWindow(LTFALSE,LTFALSE,LTFALSE);
	}

	if (m_MenuWnd.IsVisible()) m_MenuWnd.ShowWindow(LTFALSE,LTFALSE,LTFALSE);

	for (int i =0; i < NUM_OVERLAY_MASKS; i++)
	{
		if (m_hOverlays[i])
            g_pLTClient->DeleteObject(m_hOverlays[i]);

        m_hOverlays[i] = LTNULL;
	}

    m_bFadeInitialized = LTFALSE;
	m_bExitAfterFade = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreUpdate()
//
//	PURPOSE:	Handle pre-updates
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreUpdate()
{
	// Don't clear the screen if the loading screen's up
	if (m_LoadingScreen.IsVisible())
		return LTTRUE;

	if (m_bClearScreenAlways)
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	}
	else if (m_nClearScreenCount)
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
		m_nClearScreenCount--;
	}
	else
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_RENDER);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostUpdate()
//
//  PURPOSE:    Handle post-updates (return LTTRUE to FLIP
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostUpdate()
{
	if (/*m_eGameState != GS_SPLASHSCREEN &&*/ m_eGameState != GS_LOADINGLEVEL)
	{
        g_pLTClient->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);
	}

	// because of driver bugs, we need to wait a frame after reinitializing the renderer and
	// reinitialize the cursor
	int nCursorHackFrameDelay = (int)g_vtCursorHack.GetFloat();
	if (nCursorHackFrameDelay)
	{
		nCursorHackFrameDelay--;
		g_vtCursorHack.SetFloat((LTFLOAT)nCursorHackFrameDelay);
		if (nCursorHackFrameDelay == 1)
			InitCursor();
	}
    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Update()
//
//	PURPOSE:	Handle updating the interface
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::Update()
{
	// Update based on the game state...
    LTBOOL bHandled = LTFALSE;
	switch (m_eGameState)
	{
		case GS_PLAYING:
		{
			if (g_bFirstStateUpdate)
			{
				g_bFirstStateUpdate = LTFALSE;
				g_bLockStats = LTTRUE;
				g_bLockInv = LTTRUE;
				g_bLockFolder = LTTRUE;
			}
			else
			{
				if (g_bLockStats && !IsKeyDown(GetCommandKey(COMMAND_ID_FRAGCOUNT)))
					g_bLockStats = LTFALSE;
				if (g_bLockInv && !IsKeyDown(GetCommandKey(COMMAND_ID_INVENTORY)))
					g_bLockInv = LTFALSE;
				if (g_bLockFolder && !IsKeyDown(VK_ESCAPE))
					g_bLockFolder = LTFALSE;
            }
			UpdatePlayingState();
            bHandled = LTFALSE;  // Allow further processing
		}
		break;

		case GS_DIALOGUE:
		{
			UpdateDialogueState();
            bHandled = LTFALSE;  // Allow further processing
		}
		break;

		case GS_MENU:
		{
			UpdateMenuState();
            bHandled = LTFALSE;  // Allow further processing
		}
		break;

		case GS_POPUP:
		{
			if (g_bLockPopup && !g_pLTClient->IsCommandOn(COMMAND_ID_ACTIVATE))
			{
				g_bLockPopup = LTFALSE;
			}

			UpdatePopupState();
            bHandled = LTFALSE;  // Allow further processing
		}
		break;

		case GS_LOADINGLEVEL:
		{
			UpdateLoadingLevelState();
            bHandled = LTTRUE;
		}
		break;

		case GS_FOLDER :
		{
			if (g_bFirstStateUpdate)
			{
				g_bFirstStateUpdate = LTFALSE;
				g_bLockFolder = LTTRUE;
			}
			else
			{
				if (g_bLockFolder && !IsKeyDown(VK_ESCAPE))
					g_bLockFolder = LTFALSE;
            }
			UpdateFolderState();
            bHandled = LTTRUE;
		}
		break;

		case GS_PAUSED:
		{
			UpdatePausedState();
            bHandled = LTTRUE;
		}
		break;

		case GS_SPLASHSCREEN:
		{
			UpdateSplashScreenState();
            bHandled = LTTRUE;
		}
		break;

		case GS_MOVIE:
		{
			UpdateMovieState();
            bHandled = LTTRUE;
		}
		break;

		case GS_DEMOSCREEN:
		{
			UpdateDemoScreenState();
            bHandled = LTTRUE;
		}
		break;

		case GS_FAILURE:
		{
			UpdateFailureState();
            bHandled = LTTRUE;
		}
		break;

	}

	//in playing state, message box is drawn in InterfaceMgr::Draw(),
	// otherwise draw it here
	if (bHandled)
	{
		if (m_MessageBox.IsVisible())
		{
			g_pLTClient->Start3D();
			g_pLTClient->StartOptimized2D();
			m_MessageBox.Draw(g_pLTClient->GetScreenSurface());
			g_pLTClient->EndOptimized2D();
			g_pLTClient->End3D();
		}
		UpdateCursor();
	}


	return bHandled;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateWeaponStats()
//
//	PURPOSE:	Update the player's weapon stats display
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateWeaponStats(uint8 nWeaponId, uint8 nAmmoId, uint32 dwAmmo)
{
	m_stats.UpdateAmmo(nWeaponId, nAmmoId, dwAmmo);
	m_stats.UpdatePlayerWeapon(nWeaponId, nAmmoId);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawSFX()
//
//	PURPOSE:	Renders the currently active special effects
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::DrawSFX()
{
    LTFLOAT fApplySun = g_vtModelApplySun.GetFloat();
	g_vtModelApplySun.SetFloat(0.0f);
	UpdateInterfaceSFX();
	g_vtModelApplySun.SetFloat(fApplySun);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateFolderState()
//
//	PURPOSE:	Update folder state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateFolderState()
{
	if (m_FolderMgr.GetCurrentFolderID() == FOLDER_ID_NONE)
		SwitchToFolder(FOLDER_ID_MAIN);
	m_FolderMgr.UpdateInterfaceSFX();
    g_pLTClient->Start3D();

	DrawSFX();

    g_pLTClient->StartOptimized2D();
	m_InterfaceResMgr.DrawFolder();
	UpdateScreenFade();
	g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePlayingState()
//
//	PURPOSE:	Update playing state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePlayingState()
{
	// Update the player stats...
	if (m_bDrawPlayerStats)
	{
		m_stats.Update();
	}

	m_Subtitle.Update();
	m_MissionText.Update();

	// Update auto chooser switching...

	UpdateChooserAutoSwitch();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateChooserAutoSwitch()
//
//	PURPOSE:	Update auto switching the choosers
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateChooserAutoSwitch()
{
	// If Weapon chooser is being drawn, see if we want to change weapons...

	if (m_WeaponChooser.IsOpen())
	{
		// See if we should switch to the next weapon...

		if (m_NextWeaponKeyDownTimer.On() && m_NextWeaponKeyDownTimer.Stopped())
		{
			if (m_AutoSwitchTimer.On())
			{
				if (m_AutoSwitchTimer.Stopped())
				{
					m_WeaponChooser.NextWeapon();
					m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
				}
			}
			else
			{
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
		else if (m_PrevWeaponKeyDownTimer.On() && m_PrevWeaponKeyDownTimer.Stopped())
		{
			if (m_PrevWeaponKeyDownTimer.On())
			{
				if (m_AutoSwitchTimer.Stopped())
				{
					m_WeaponChooser.PrevWeapon();
					m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
				}
			}
			else
			{
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
	}
	else if (m_AmmoChooser.IsOpen())
	{
		// See if we should switch to the next ammo type...

		if (m_NextAmmoKeyDownTimer.On() && m_NextAmmoKeyDownTimer.Stopped())
		{
			if (m_AutoSwitchTimer.On())
			{
				if (m_AutoSwitchTimer.Stopped())
				{
					m_AmmoChooser.NextAmmo();
					m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
				}
			}
			else
			{
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateDialogueState()
//
//	PURPOSE:	Update dialogue state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateDialogueState()
{
	// Update the main window...
	if (!m_DialogueWnd.IsVisible())
	{
		ChangeState(GS_PLAYING);
	}
    m_MainWnd.Update(g_pGameClientShell->GetFrameTime());


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateMenuState()
//
//	PURPOSE:	Update dialogue state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateMenuState()
{
	// Update the main window...

	if (!m_MenuWnd.IsVisible())
	{
		ChangeState(GS_PLAYING);
	}

    m_MainWnd.Update(g_pGameClientShell->GetFrameTime());

}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePopupState()
//
//	PURPOSE:	Update dialogue state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePopupState()
{
	if (m_PopupText.IsVisible())
	{
		m_PopupText.Update();
	}
	else
	{
		ChangeState(GS_PLAYING);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateLoadingLevelState()
//
//	PURPOSE:	Update loading level state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateLoadingLevelState()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if ((g_pGameClientShell->IsInWorld() && (m_nOldLoadWorldCount != m_nLoadWorldCount)) &&
		hPlayerObj)
	{
		// Turn off the loading screen
		m_LoadingScreen.Hide();

		ChangeState(GS_PLAYING);
	}
	else if ((m_bLoadFailed) || (g_pLTClient->IsConnected() && IsKeyDown(VK_ESCAPE)))
	{
		m_LoadingScreen.Hide();
		ChangeState(GS_FOLDER);

		// If they were in the middle of connecting and aborted, disconnect
		if (g_pLTClient->IsConnected())
			g_pLTClient->Disconnect();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HideLoadScreen()
//
//	PURPOSE:	Called externally to hide the loading screen
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::HideLoadScreen()
{
	m_LoadingScreen.Hide();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetLoadLevelString()
//
//	PURPOSE:	Sets the name of the world to be displayed on the loading screen
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::SetLoadLevelString(HSTRING hWorld)
{
	m_LoadingScreen.SetWorldName(hWorld);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetLoadLevelPhoto()
//
//	PURPOSE:	Sets the photo of the world to be displayed on the loading screen
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::SetLoadLevelPhoto(char *pszPhoto)
{
	m_LoadingScreen.SetWorldPhoto(pszPhoto);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePausedState()
//
//	PURPOSE:	Update paused state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePausedState()
{
    g_pLTClient->Start3D();

	UpdateInterfaceSFX();

    g_pLTClient->StartOptimized2D();

	LTRect rcFull(0,0,g_pInterfaceResMgr->GetScreenWidth()-1,g_pInterfaceResMgr->GetScreenHeight()-1);
	g_pLTClient->FillRect(g_pLTClient->GetScreenSurface(),&rcFull,hShadeColor);


	if (m_hGamePausedSurface)
	{
		int xo = g_pInterfaceResMgr->GetXOffset();
		int yo = g_pInterfaceResMgr->GetYOffset();

        g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hGamePausedSurface, LTNULL, xo,yo);
	}

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateSplashScreenState()
//
//	PURPOSE:	Update splash screen state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateSplashScreenState()
{
    //g_fSplashSndDuration -= g_pGameClientShell->GetFrameTime();

	//if (!m_hSplashSound || g_fSplashSndDuration <= 0.0f
    //  /* || g_pLTClient->IsDone(m_hSplashSound*/)
	//{
    //  g_pLTClient->CPrint("Current Time: %.4f", g_pLTClient->GetTime());
    //  g_pLTClient->CPrint("Splash sound done playing...");

    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nWidth = 0;
    uint32 nHeight = 0;

    g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

    LTRect rcDst;
	rcDst.Init(0, 0, nWidth, nHeight);

    g_pLTClient->GetSurfaceDims(g_hSplash, &nWidth, &nHeight);

    LTRect rcSrc;
	rcSrc.Init(0, 0, nWidth, nHeight);

    g_pLTClient->Start3D();
    g_pLTClient->StartOptimized2D();

    g_pLTClient->ScaleSurfaceToSurface(hScreen, g_hSplash, &rcDst, &rcSrc);

	UpdateScreenFade();

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D();

    static LTBOOL bDidFadeOut = LTFALSE;

	if (!m_bScreenFade)
	{
		if (!bDidFadeOut)
		{
			StartScreenFadeOut(3.0);
            bDidFadeOut = LTTRUE;
		}
	}
	else if (bDidFadeOut && m_fCurFadeTime <= 0.0f)
	{
		ChangeState(GS_MOVIE);
		//SwitchToFolder(FOLDER_ID_MAIN);
	}
//a SwitchToFolder(FOLDER_ID_MAIN);
	//}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateMovieState()
//
//	PURPOSE:	Update movie state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateMovieState()
{

	ILTVideoMgr* pVideoMgr = g_pLTClient->VideoMgr();
	if (!pVideoMgr || !m_hMovie)
	{
		SwitchToFolder(FOLDER_ID_MAIN);
		return;
	}

	// Update and check if we're finished...

	if (pVideoMgr->UpdateVideo(m_hMovie) != LT_OK ||
		pVideoMgr->GetVideoStatus(m_hMovie) == LT_FINISHED)
	{
		NextMovie();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateDemoScreenState()
//
//	PURPOSE:	Update demo screen state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateDemoScreenState()
{

	if (m_bSeenDemoScreens)
	{
		if (m_bQuitAfterDemoScreens)
			g_pLTClient->Shutdown();
		else
			SwitchToFolder(FOLDER_ID_MAIN);
		return;
	}


    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nWidth = 0;
    uint32 nHeight = 0;

    g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

    LTRect rcDst;
	rcDst.Init(0, 0, nWidth, nHeight);

    g_pLTClient->GetSurfaceDims(g_hDemo, &nWidth, &nHeight);

    LTRect rcSrc;
	rcSrc.Init(0, 0, nWidth, nHeight);

    g_pLTClient->Start3D();
    g_pLTClient->StartOptimized2D();

    g_pLTClient->ScaleSurfaceToSurface(hScreen, g_hDemo, &rcDst, &rcSrc);


    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateFailureState()
//
//	PURPOSE:	Update failure state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateFailureState()
{
    g_fFailScreenDuration += g_pGameClientShell->GetFrameTime();
    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nScrnWidth = g_pInterfaceResMgr->GetScreenWidth();
    uint32 nScrnHeight = g_pInterfaceResMgr->GetScreenHeight();

	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();



    HSTRING hStr = g_pLTClient->FormatString(m_nFailStringId);

	LTRect rcDst;
	rcDst.left = rcDst.top = 0;
	rcDst.right = nScrnWidth;
	rcDst.bottom = nScrnHeight;
    g_pLTClient->FillRect(hScreen, &rcDst, hShadeColor);

    g_pLTClient->Start3D();
    g_pLTClient->StartOptimized2D();

	if (m_hFailBack)
        g_pLTClient->DrawSurfaceToSurface(hScreen, m_hFailBack, LTNULL, xo, yo);

	int nStringWidth = (int)(0.8f * (LTFLOAT)nScrnWidth);
    LTIntPt failPos = g_pLayoutMgr->GetFailStringPos();
	LTIntPt sz = m_InterfaceResMgr.GetTitleFont()->GetTextExtentsFormat(hStr,nStringWidth);
	failPos.x = (int)( (float)failPos.x * m_InterfaceResMgr.GetXRatio() );
	failPos.y = (int)( (float)failPos.y * m_InterfaceResMgr.GetYRatio() );
	failPos.x -= sz.x/2;
	failPos.y -= sz.y/2;
	m_InterfaceResMgr.GetTitleFont()->DrawFormat(hStr,hScreen,failPos.x+2,failPos.y+2,nStringWidth,kBlack);
	m_InterfaceResMgr.GetTitleFont()->DrawFormat(hStr,hScreen,failPos.x,failPos.y,nStringWidth,kWhite);

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D();

    g_pLTClient->FreeString(hStr);

	if (g_fFailScreenDuration >= g_pLayoutMgr->GetFailScreenDelay())
	{
		if (g_pGameClientShell->IsCustomLevel())
			SwitchToFolder(FOLDER_ID_MAIN);
		else
			SwitchToFolder(FOLDER_ID_FAILURE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetMenuMusic()
//
//	PURPOSE:	Turns menu / load level music on or off
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::SetMenuMusic(LTBOOL bMusicOn)
{
	CMusic* pMusic = g_pGameClientShell->GetMusic();
    if (!pMusic) return LTFALSE;

	{ // BL 09/26/00
		// If we're playing, DON'T SET THE MENU MUSIC!!!

		if (g_pGameClientShell->IsPlayerInWorld())
		{
			if (!g_pGameClientShell->IsPlayerDead() || IsMultiplayerGame())
			{
				return LTTRUE;
			}
		}
	}

	if (bMusicOn)
	{
		CString strFile = g_pClientButeMgr->GetInterfaceAttributeString("MenuMusicCtrlFile");
        if (strFile.IsEmpty()) return LTFALSE;

		CMusicState* pMState = pMusic->GetMusicState();
        if (!pMState) return LTFALSE;

		if (!m_bSavedGameMusic)
		{
            m_bSavedGameMusic = LTTRUE;
			m_GameMusicState.Copy(*pMState);
		}

		int nIntensity = g_pLayoutMgr->GetFolderMusicIntensity(m_FolderMgr.GetCurrentFolderID());

		if (pMusic->IsPlaying())
		{
			if (stricmp((char *)(LPCSTR)strFile, pMState->szControlFile) == 0)
			{
				// Make sure we're using the correct intensity...

				// kls - 9/19/00 - Don't change intensity in the menu...
				//if (pMState->nIntensity != nIntensity)
				//{
				//	pMusic->ChangeIntensity(nIntensity);
				//}
                return LTTRUE;
			}
		}

		if (!pMusic->IsInitialized())
		{
            if (!pMusic->Init(g_pLTClient))
			{
                return LTFALSE;
			}
		}

        LTBOOL bLevelInited = pMusic->IsLevelInitialized();

		if (!bLevelInited)
		{
			CString strDir = g_pClientButeMgr->GetInterfaceAttributeString("MenuMusicDir");

			if (strDir.IsEmpty())
			{
                return LTFALSE;
			}

			CMusicState MusicState;
			strcpy(MusicState.szDirectory, (char *)(LPCSTR)strDir);
			strcpy(MusicState.szControlFile, (char *)(LPCSTR)strFile);

			if (!pMusic->RestoreMusicState(MusicState))
			{
				return LTFALSE;
			}
		}

		if (!pMusic->IsPlaying())
		{
			pMusic->Play();
			pMusic->ChangeIntensity(nIntensity);
		}
		else if (pMState->nIntensity != nIntensity)
		{
			// kls - 9/19/00 Don't change intensity in the menu...
			// pMusic->ChangeIntensity(nIntensity);
		}

		m_Settings.ImplementMusicVolume();
	}
	else
	{
		pMusic->Stop();
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::RestoreGameMusic()
//
//	PURPOSE:	Restore to the game music
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::RestoreGameMusic()
{
	// kls 9/19/00 - Music doesn't change so don't restore it...
	return LTTRUE;

	CMusic* pMusic = g_pGameClientShell->GetMusic();
    if (!pMusic || !pMusic->MusicEnabled()) return LTFALSE;

	if (m_bSavedGameMusic)
	{
        m_bSavedGameMusic = LTFALSE;
		pMusic->RestoreMusicState(m_GameMusicState);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnMessage()
//
//	PURPOSE:	Handle interface messages
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnMessage(uint8 messageID, HMESSAGEREAD hMessage)
{
	switch(messageID)
	{
		case MID_PLAYER_INFOCHANGE :
		{
            uint8 nThing    = g_pLTClient->ReadFromMessageByte(hMessage);
            uint8 nType1    = g_pLTClient->ReadFromMessageByte(hMessage);
            uint8 nType2    = g_pLTClient->ReadFromMessageByte(hMessage);
            LTFLOAT fAmount  = g_pLTClient->ReadFromMessageFloat(hMessage);

			UpdatePlayerStats(nThing, nType1, nType2, fAmount);

            return LTTRUE;
		}
		break;

		case MID_GEAR_PICKEDUP :
		{
            uint8 nGearId = g_pLTClient->ReadFromMessageByte(hMessage);
			g_pGameClientShell->HandleGearPickup(nGearId);
			m_stats.UpdateGear(nGearId);
            return LTTRUE;
		}
		break;

		case MID_DISPLAY_TIMER :
		{
            LTFLOAT fTime = g_pLTClient->ReadFromMessageFloat(hMessage);
			LTBOOL  bPaused = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
			m_InterfaceTimer.SetTime(fTime,bPaused);
            return LTTRUE;
		}
		break;

		case MID_DISPLAY_METER :
		{
            uint8 val = g_pLTClient->ReadFromMessageByte(hMessage);
			m_InterfaceMeter.SetValue(val);
            return LTTRUE;
		}
		break;

		case MID_PLAYER_CONNECTED:
		{
            HSTRING hstrName = g_pLTClient->ReadFromMessageHString (hMessage);
            uint32 nID = (uint32) g_pLTClient->ReadFromMessageFloat (hMessage);
            uint8 teamId = g_pLTClient->ReadFromMessageByte(hMessage);
			int nTeam1Score = (int) g_pLTClient->ReadFromMessageFloat (hMessage);
			int nTeam2Score = (int) g_pLTClient->ReadFromMessageFloat (hMessage);
			m_ClientInfo.AddClient(hstrName, nID, 0, teamId);
			m_ClientInfo.SetScores(nTeam1Score,nTeam2Score);
            return LTTRUE;

		}
		break;

		case MID_PLAYER_ADDED:
		{
			// Only do something if we're in multiplayer...

			if (!g_pGameClientShell->IsMultiplayerGame()) break;

            HSTRING hstrName = g_pLTClient->ReadFromMessageHString (hMessage);
            uint32 nID = (uint32) g_pLTClient->ReadFromMessageFloat (hMessage);
			HOBJECT	hObj = g_pLTClient->ReadFromMessageObject(hMessage);
            int nFrags = (int) g_pLTClient->ReadFromMessageFloat (hMessage);
            uint8 teamId = g_pLTClient->ReadFromMessageByte(hMessage);
			uint8 nLives = g_pLTClient->ReadFromMessageByte(hMessage);
            LTBOOL bJoining = (LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage);
			m_ClientInfo.AddClient(hstrName, nID, nFrags, teamId);
            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);
			if (nID == nLocalID && nLives != 255)
				m_ClientInfo.SetLives(nID, nLives);


			if (bJoining)
			{
				if (teamId == 0)
				{
                    HSTRING hStr = g_pLTClient->FormatString(IDS_JOINEDGAME, g_pLTClient->GetStringData(hstrName));
					g_pGameClientShell->CSPrint(g_pLTClient->GetStringData (hStr));
                    g_pLTClient->FreeString(hStr);
				}
				else
				{
                    HSTRING hStr = g_pLTClient->FormatString(IDS_JOINEDTEAM, g_pLTClient->GetStringData(hstrName), teamId);
					g_pGameClientShell->CSPrint(g_pLTClient->GetStringData (hStr));
                    g_pLTClient->FreeString(hStr);
				}
			}

			CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	        if (psfxMgr)
			{
				CCharacterFX* pCFX = (CCharacterFX*)psfxMgr->FindSpecialFX(SFX_CHARACTER_ID, hObj);
				if (pCFX) pCFX->ResetMarkerState();
			}


            return LTTRUE;
		}
		break;

		case MID_PLAYER_REMOVED:
		{
			// Only do something if we're in multiplayer...

			if (!g_pGameClientShell->IsMultiplayerGame()) break;

            uint32 nID = (uint32) g_pLTClient->ReadFromMessageFloat(hMessage);

			if (m_ClientInfo.GetClientByID(nID))
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_LEFTGAME, m_ClientInfo.GetPlayerName(nID));
				g_pGameClientShell->CSPrint(g_pLTClient->GetStringData (hStr));
				g_pLTClient->FreeString(hStr);

				m_ClientInfo.RemoveClient(nID);
			}

            return LTTRUE;
		}
		break;

		case MID_PINGTIMES:
		{
			while(1)
			{
                uint16 id, ping;
				CLIENT_INFO *pClient;

                id = g_pLTClient->ReadFromMessageWord(hMessage);
				if(id == 0xFFFF)
					break;

                ping = g_pLTClient->ReadFromMessageWord(hMessage);
				if(pClient = m_ClientInfo.GetClientByID(id))
					pClient->m_Ping = ping;
			}

            return LTTRUE;
		}
		break;

		case MID_PLAYER_FRAGGED:
		{
			//Only do something if we're in multiplayer...

			if (!g_pGameClientShell->IsMultiplayerGame()) break;

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);

            uint32 nVictim = (uint32) g_pLTClient->ReadFromMessageFloat (hMessage);
            uint32 nKiller = (uint32) g_pLTClient->ReadFromMessageFloat (hMessage);
            uint8  nLives =  g_pLTClient->ReadFromMessageByte (hMessage);

			CLIENT_INFO *pVictim = m_ClientInfo.GetClientByID(nVictim);
			CLIENT_INFO *pKiller = m_ClientInfo.GetClientByID(nKiller);
			eMessageType eType = MMGR_DEFAULT;

			// Sanity check...
			if (!pVictim || !pKiller)
			{
				// Huh..  Dunno who you're talking about.  Never mind.
				return LTTRUE;
			}

			if (nVictim == nKiller)
			{
				m_ClientInfo.RemoveFrag(nKiller);
			}
			else
			{
				if ((g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT) &&
					(pVictim->team == pKiller->team))
				{
					m_ClientInfo.RemoveFrag(nKiller);
				}
				else
				{
					m_ClientInfo.AddFrag(nKiller);
				}
			}

			if (nVictim == nLocalID)
			{
                HSTRING hStr = LTNULL;

				if (nVictim == nKiller)
				{
                    hStr = g_pLTClient->FormatString(IDS_KILLEDMYSELF);
				}
				else
				{

                    hStr = g_pLTClient->FormatString(IDS_HEKILLEDME, m_ClientInfo.GetPlayerName (nKiller));
					if (pKiller)
					{
						if (pKiller->team == 1)
							eType = MMGR_TEAM_1;
						else if (pKiller->team == 2)
							eType = MMGR_TEAM_2;
					}
				}

				m_messageMgr.AddLine(hStr,eType);
                g_pLTClient->CPrint(g_pLTClient->GetStringData(hStr));
				if (nLives != 255)
				{
	                g_pGameClientShell->CSPrint("%d lives left.",nLives);
					m_ClientInfo.SetLives(nLocalID, nLives);
				}
                g_pLTClient->FreeString (hStr);
			}
			else if (nKiller == nLocalID)
			{
                HSTRING hStr = LTNULL;
                hStr = g_pLTClient->FormatString(IDS_IKILLEDHIM, m_ClientInfo.GetPlayerName (nVictim));
                g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));
				m_messageMgr.AddLine(hStr,eType);
                g_pLTClient->FreeString(hStr);
			}
			else
			{
                HSTRING hStr = LTNULL;

				if (nVictim == nKiller)
				{
                    hStr = g_pLTClient->FormatString(IDS_HEKILLEDHIMSELF, m_ClientInfo.GetPlayerName(nKiller));
				}
				else
				{
                    hStr = g_pLTClient->FormatString(IDS_HEKILLEDHIM, m_ClientInfo.GetPlayerName (nKiller), m_ClientInfo.GetPlayerName (nVictim));
					if (pKiller)
					{
						if (pKiller->team == 1)
							eType = MMGR_TEAM_1;
						else if (pKiller->team == 2)
							eType = MMGR_TEAM_2;
					}
				}

                g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));
				m_messageMgr.AddLine(hStr,eType);
                g_pLTClient->FreeString(hStr);
			}

            return LTTRUE;
		}
		break;

		case MID_TEAM_SCORED:
		{
			//Only do something if we're in multiplayer...

			if (!g_pGameClientShell->IsMultiplayerGame()) break;

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);

            uint32 nPlayerID =  g_pLTClient->ReadFromMessageDWord(hMessage);
            uint8  nTeamID =  g_pLTClient->ReadFromMessageByte(hMessage);
            uint8  nScore =  g_pLTClient->ReadFromMessageByte(hMessage);

			uint8 nLocalTeam = 0;
			CLIENT_INFO* pInfo = m_ClientInfo.GetLocalClient();
			if (pInfo)
			{
				nLocalTeam = pInfo->team;
			}

			eMessageType eType = MMGR_DEFAULT;
			if (nTeamID == 1)
				eType = MMGR_TEAM_1;
			else if (nTeamID == 2)
				eType = MMGR_TEAM_2;
			if (nLocalID == nPlayerID)
			{
                HSTRING hStr = g_pLTClient->FormatString(IDS_ISCORED, nScore);
                g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));
				m_messageMgr.AddLine(hStr,eType);
                g_pLTClient->FreeString(hStr);
				g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\mp_youscore.wav");
			}
			else if (nTeamID == nLocalTeam)
			{

                HSTRING hStr = g_pLTClient->FormatString(IDS_HESCORED_US, m_ClientInfo.GetPlayerName (nPlayerID), nScore);
                g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));
				m_messageMgr.AddLine(hStr,eType);
                g_pLTClient->FreeString(hStr);
				g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\mp_youscore.wav");
			}
			else
			{

                HSTRING hStr = g_pLTClient->FormatString(IDS_HESCORED_THEM, m_ClientInfo.GetPlayerName (nPlayerID), nScore);
                g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));
				m_messageMgr.AddLine(hStr,eType);
                g_pLTClient->FreeString(hStr);
				g_pClientSoundMgr->PlayInterfaceSound("interface\\snd\\mp_otherscore.wav");
			}

			m_ClientInfo.AddScore (nPlayerID, nTeamID, nScore);


			return LTTRUE;
		}
		break;

		case SCM_PLAY_DIALOGUE:
		{
			char szAvatar[100];
			char szDecisions[200];

			DWORD dwID = hMessage->ReadDWord();
			hMessage->ReadStringFL(szAvatar, sizeof(szAvatar));
			BOOL bStayOpen = !!hMessage->ReadByte();
			hMessage->ReadStringFL(szDecisions, sizeof(szDecisions));

			m_DialogueWnd.DisplayText(dwID,szAvatar,bStayOpen,szDecisions);

			ChangeState(GS_DIALOGUE);
            return LTTRUE;
		}
		break;

		case MID_CHANGING_LEVELS :
		{
			if (g_pGameClientShell->GetGameType() != SINGLE)
			{
				HSTRING hWorldPath = g_pLTClient->ReadFromMessageHString(hMessage);
				char szWorldPath[256];
				strcpy(szWorldPath,g_pLTClient->GetStringData(hWorldPath));
				g_pLTClient->FreeString(hWorldPath);

				char *pWorld = strrchr(szWorldPath,'\\');
				pWorld++;

				HSTRING hWorld = g_pLTClient->CreateString(pWorld);
				m_LoadingScreen.SetWorldName(hWorld);
				g_pLTClient->FreeString(hWorld);


				strtok(szWorldPath,".");
				strcat(szWorldPath,".pcx");
				g_pInterfaceMgr->SetLoadLevelPhoto(szWorldPath);

				ChangeState(GS_LOADINGLEVEL);
				m_stats.GetObjectives()->Clear();
				m_stats.GetCompletedObjectives()->Clear();
			}
            return LTTRUE;
		}
		break;

		case MID_PLAYER_MESSAGE :
		case MID_PLAYER_GHOSTMESSAGE :
		case MID_PLAYER_TEAMMESSAGE :
		{
			// retrieve the string from the message, play the chat sound, and display the message

			char szMessage[100];
			hMessage->ReadStringFL(szMessage, sizeof(szMessage));

            g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\chat.wav");

			if (messageID == MID_PLAYER_TEAMMESSAGE)
				m_messageMgr.AddLine(szMessage,MMGR_TEAMCHAT);
			else
				m_messageMgr.AddLine(szMessage,MMGR_CHAT);
            return LTTRUE;
		}
		break;

		case MID_PLAYER_CREDITS :
		{
            uint8 nMsg = g_pLTClient->ReadFromMessageByte(hMessage);
			switch (nMsg)
			{
			case 0:
				m_Credits.Term();
				break;
			case 1:
				m_Credits.Init();
				break;
			case 2:
				m_Credits.Init(CM_INTRO);
				break;
			}
		}
		break;

		case MID_PLAYER_TRANSMISSION :
		{
			// retrieve the string from the message, play the chat sound, and display the message

            uint32 dwId = g_pLTClient->ReadFromMessageDWord(hMessage);
            uint8 nTeam = g_pLTClient->ReadFromMessageByte(hMessage);
            uint32 nSound = g_pLTClient->ReadFromMessageDWord(hMessage);
			if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT && nTeam != 0)
			{
				CLIENT_INFO* pInfo = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
				if (!pInfo || nTeam != pInfo->team)
				{
					return LTFALSE;
				}
			}


			if (nSound)
			{
				CString csSound = g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", nSound);

				g_pClientSoundMgr->PlaySoundLocal((char *)(LPCTSTR)csSound,SOUNDPRIORITY_PLAYER_HIGH);
			}
			else
			{
				g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\chat.wav");
			}

            HSTRING hStr = g_pLTClient->FormatString(dwId);
			m_messageMgr.AddLine(hStr,MMGR_TEAMCHAT);
            g_pLTClient->FreeString(hStr);

            return LTTRUE;
		}
		break;

		case MID_PLAYER_POPUPTEXT :
		{
			// retrieve the string from the message, play the chat sound, and display the message
			ChangeState(GS_POPUP);
 			m_PopupText.Show(hMessage);
			return LTTRUE;
		}
		break;

		default : break;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnEvent(uint32 dwEventID, uint32 dwParam)
{

	switch (dwEventID)
	{
		// Called when the renderer has switched into
		// the new mode but before it reloads all the textures
		// so you can display a loading screen.

		case LTEVENT_RENDERALMOSTINITTED :
		{
			if (m_bSwitchingModes)
			{
				ClearAllScreenBuffers();

                g_pLTClient->Start3D();
                g_pLTClient->StartOptimized2D();

				m_InterfaceResMgr.DrawMessage(m_InterfaceResMgr.GetSmallFont(),
					IDS_REINITIALIZING_RENDERER);

                g_pLTClient->EndOptimized2D();
                g_pLTClient->End3D();
                g_pLTClient->FlipScreen(0);

				// because of driver bugs, we need to wait a frame after reinitializing the renderer and
				// reinitialize the cursor
				g_vtCursorHack.SetFloat(2.0f);
			}
		}
		break;

		// Client disconnected from server.  dwParam will
		// be a error flag found in de_codes.h.

		case LTEVENT_DISCONNECT :
		{
			if (g_pGameClientShell->IsMultiplayerGame())
			{
				Disconnected(dwParam);
			}
		}
		break;

		// Engine shutting down.  dwParam will be a error
		// flag found in de_codes.h.

		case LTEVENT_SHUTDOWN :
		break;

		// The renderer was initialized.  This is called if
		// you call SetRenderMode or if focus was lost and regained.

		case LTEVENT_RENDERINIT :
		{
			if (m_LoadingScreen.IsVisible())
			{
				// Hide it to get rid of any resources that may have been around
				m_LoadingScreen.Hide();
				// And then start it up again
				m_LoadingScreen.Show();
			}
		}
		break;

		// The renderer is being shutdown.  This happens when
		// ShutdownRender is called or if the app loses focus.

		case  LTEVENT_RENDERTERM :
		{
			// Stop drawing the loading screen
			if (m_LoadingScreen.IsVisible())
				m_LoadingScreen.Pause();
		}
		break;

        case LTEVENT_LOSTFOCUS:
		case LTEVENT_GAINEDFOCUS:
		break;

		default :
		{
            uint32 nStringID = IDS_UNSPECIFIEDERROR;
			SwitchToFolder(FOLDER_ID_MAIN);
			//DoMessageBox(nStringID, TH_ALIGN_CENTER);
		}
		break;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePlayerStats()
//
//	PURPOSE:	Update the player's stats
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePlayerStats(uint8 nThing, uint8 nType1,
                                      uint8 nType2, LTFLOAT fAmount)
{
	switch (nThing)
	{
		case IC_WEAPON_OBTAIN_ID :
		{
            m_stats.UpdateAmmo(nType1, nType2, (uint32)fAmount, LTTRUE, LTFALSE);
		}
		break;

		case IC_WEAPON_PICKUP_ID :
		{
            m_stats.UpdateAmmo(nType1, nType2, (uint32)fAmount, LTTRUE);
		}
		break;

		case IC_MOD_PICKUP_ID :
		{
			g_pGameClientShell->HandleModPickup(nType2);
			m_stats.UpdateMod(nType2);
		}
		break;

		case IC_WEAPON_ID :
		{
            m_stats.UpdateAmmo(nType1, nType2, (uint32)fAmount);
		}
		break;

		case IC_AMMO_ID :
		{
            m_stats.UpdateAmmo(nType1, nType2, (uint32)fAmount);
		}
		break;

		case IC_MAX_HEALTH_ID :
		{
            m_stats.UpdateMaxHealth((uint32)fAmount);
		}
		break;

		case IC_MAX_ARMOR_ID :
		{
            m_stats.UpdateMaxArmor((uint32)fAmount);
		}
		break;

		case IC_HEALTH_ID :
		{
            m_stats.UpdateHealth((uint32)fAmount);
		}
		break;

		case IC_ARMOR_ID :
		{
            m_stats.UpdateArmor((uint32)fAmount);
		}
		break;

		case IC_AIRLEVEL_ID :
		{
			m_stats.UpdateAir(fAmount);
		}
		break;

		case IC_OUTOFAMMO_ID :
		{
            HSTRING hStr = g_pLTClient->FormatString(IDS_OUTOFAMMO, GetWeaponString(nType1));
			m_messageMgr.AddLine(hStr);
            g_pLTClient->FreeString(hStr);
		}
		break;

		case IC_OBJECTIVE_ID :
		{
            m_stats.UpdateObjectives(nType1, nType2, (uint32)fAmount);

		}
		break;



		case IC_FADE_SCREEN_ID :
		{
			// Fade the screen in or out...

			if (m_bExitAfterFade) return;

			if (nType1)
			{
				StartScreenFadeIn(fAmount);
			}
			else
			{
				StartScreenFadeOut(fAmount);
			}

            m_bExitAfterFade = (LTBOOL) !!nType2;

			if (m_bExitAfterFade)
			{
				// Pause the server until the fade is done...
                //g_pGameClientShell->PauseGame(LTTRUE);
			}
		}
		break;

		case IC_RESET_INVENTORY_ID :
		{
			m_stats.DropInventory((LTBOOL)nType1);
		}
		break;


		case IC_MISSION_TEXT_ID :
		{
			if (fAmount > 0.0f)
				m_MissionText.Start((int)fAmount);
			else
				m_MissionText.Clear();
		}
		break;

		case IC_MISSION_FAILED_ID :
		{
			MissionFailed((int)fAmount);
		}
		break;

		case IC_INTEL_PICKUP_ID :
		{
			if (g_pGameClientShell->GetGameType() != COOPERATIVE_ASSAULT)
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_INTELLIGENCE);
				m_messageMgr.AddLine(hStr);
				g_pLTClient->FreeString(hStr);

				g_pClientSoundMgr->PlayInterfaceSound("Powerups\\Snd\\intelPU.wav");
			}

		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnCommandOn()
//
//	PURPOSE:	Handle command on
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnCommandOn(int command)
{
	if (FadingToExit()) return LTTRUE;

    LTBOOL bMultiplayer = g_pGameClientShell->IsMultiplayerGame();

    if (m_messageMgr.GetEditingState()) return LTTRUE;

	// Take appropriate action

	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		{
			if (m_eGameState == GS_POPUP)
			{
				if(m_PopupText.IsVisible() && !g_bLockPopup)
				{
					m_PopupText.Clear();
				}
			}
		}
		break;

		case COMMAND_ID_CROSSHAIRTOGGLE :
		{
			m_stats.ToggleCrosshair();
		}
		break;

		case COMMAND_ID_INVENTORY :
		{
			if (m_eGameState == GS_PLAYING && !g_bLockInv)
			{
				SwitchToFolder(FOLDER_ID_VIEW_INV);
			}
            return LTTRUE;
		}
		break;

		case COMMAND_ID_FRAGCOUNT :
		{
			if (bMultiplayer)
			{
                m_bDrawFragCount = LTTRUE;
			}
			else if (m_eGameState == GS_PLAYING && !g_bLockStats)
			{
				SwitchToFolder(FOLDER_ID_STATS);
			}

            return LTTRUE;
		}
		break;

		case COMMAND_ID_MISSION :
		{
			if (m_eGameState == GS_PLAYING)
			{
//				m_stats.ShowObjectives();
                return LTTRUE;
			}
		}
		break;

		case COMMAND_ID_MESSAGE :
		{
			if (m_eGameState == GS_PLAYING && !g_pGameClientShell->IsSpectatorMode())
			{
				if (m_AmmoChooser.IsOpen())
				{
					m_AmmoChooser.Close();
				}
				if (m_WeaponChooser.IsOpen())
				{
					m_WeaponChooser.Close();
				}

                m_messageMgr.SetEditingState(LTTRUE);
                g_pGameClientShell->SetInputState(LTFALSE);
			}

            return LTTRUE;
		}
		break;

		case COMMAND_ID_TEAM_MESSAGE :
		{
			if ((g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT) && m_eGameState == GS_PLAYING && !g_pGameClientShell->IsSpectatorMode())
			{
				if (m_AmmoChooser.IsOpen())
				{
					m_AmmoChooser.Close();
				}
				if (m_WeaponChooser.IsOpen())
				{
					m_WeaponChooser.Close();
				}

				m_messageMgr.SetEditingState(LTTRUE,LTTRUE);
                g_pGameClientShell->SetInputState(LTFALSE);
			}

            return LTTRUE;
		}
		break;

		case COMMAND_ID_HOLSTER :
		{
            if (g_pGameClientShell->GetWeaponModel()->IsDisabled()) return LTTRUE;

			if (m_AmmoChooser.IsOpen())
			{
				m_AmmoChooser.Close();
			}
			if (m_WeaponChooser.IsOpen())
			{
				m_WeaponChooser.Close();
			}
            return LTTRUE;

		}
		break;

		case COMMAND_ID_PREV_WEAPON :
		{
            if (g_pGameClientShell->GetWeaponModel()->IsDisabled()) return LTTRUE;

			m_PrevWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());

			if (m_AmmoChooser.IsOpen())
			{
				m_AmmoChooser.Close();
			}
			if (m_WeaponChooser.Open())
			{
				m_WeaponChooser.PrevWeapon();
			}
            return LTTRUE;

		}
		break;

		case COMMAND_ID_NEXT_WEAPON :
		{
            if (g_pGameClientShell->GetWeaponModel()->IsDisabled()) return LTTRUE;

			m_NextWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());

			if (m_AmmoChooser.IsOpen())
			{
				m_AmmoChooser.Close();
			}
			if (m_WeaponChooser.Open())
			{
				m_WeaponChooser.NextWeapon();
			}
            return LTTRUE;
		}
		break;

		case COMMAND_ID_NEXT_AMMO :
		{
            if (g_pGameClientShell->GetWeaponModel()->IsDisabled()) return LTTRUE;

			m_NextAmmoKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());

			if (m_WeaponChooser.IsOpen())
			{
				m_WeaponChooser.Close();
			}
			if (m_AmmoChooser.Open())
			{
				m_AmmoChooser.NextAmmo();
			}
            return LTTRUE;
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if (IsChoosingWeapon() || IsChoosingAmmo())
			{
                return LTTRUE;
			}
		}
		break;

		case COMMAND_ID_NEXT_LAYOUT :
		{
			if (m_eGameState == GS_PLAYING)
				m_stats.NextLayout();
            return LTTRUE;

		}
		break;
		case COMMAND_ID_PREV_LAYOUT :
		{
			if (m_eGameState == GS_PLAYING)
				m_stats.PrevLayout();
            return LTTRUE;

		}
		break;

		default :
		break;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnCommandOff(int command)
{
	switch (command)
	{
		case COMMAND_ID_INVENTORY :
		{
			g_bLockInv = LTFALSE;
            return LTTRUE;
		}
		break;
		case COMMAND_ID_FRAGCOUNT :
		{
            m_bDrawFragCount = LTFALSE;
			g_bLockStats = LTFALSE;
            return LTTRUE;
		}
		break;
		case COMMAND_ID_MISSION :
		{
			if (m_eGameState == GS_PLAYING)
			{
//				m_stats.HideObjectives();
                return LTTRUE;
			}

		} break;

		case COMMAND_ID_FIRING :
		{
			if (IsChoosingWeapon())
			{
                uint8 nCurrWeapon = m_WeaponChooser.GetCurrentSelection();
				m_WeaponChooser.Close();
				g_pGameClientShell->GetWeaponModel()->ChangeWeapon(g_pWeaponMgr->GetCommandId(nCurrWeapon));

                return LTTRUE;

			}
			else if (IsChoosingAmmo())
			{
                uint8 nCurrAmmo = m_AmmoChooser.GetCurrentSelection();
				m_AmmoChooser.Close();
				g_pGameClientShell->GetWeaponModel()->ChangeAmmo(nCurrAmmo);

                return LTTRUE;
			}
		}
		break;

		case COMMAND_ID_HOLSTER :
		{
			g_pGameClientShell->GetWeaponModel()->ToggleHolster();
			return LTTRUE;
		}
		break;

		case COMMAND_ID_NEXT_WEAPON :
		{
			// End the timer for quick weapon changing...
			m_NextWeaponKeyDownTimer.Stop();
			m_AutoSwitchTimer.Stop();
		}
		break;

		case COMMAND_ID_PREV_WEAPON :
		{
			// End the timer for quick weapon changing...
			m_PrevWeaponKeyDownTimer.Stop();
			m_AutoSwitchTimer.Stop();
		}
		break;

		case COMMAND_ID_NEXT_AMMO :
		{
			// End the timer for quick ammo changing...
			m_NextAmmoKeyDownTimer.Stop();
			m_AutoSwitchTimer.Stop();
		}
		break;

		default : break;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyDown()
//
//	PURPOSE:	Handle OnKeyDown messages
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnKeyDown(int key, int rep)
{
	if (FadingToExit()) return LTTRUE;

	if (m_MessageBox.IsVisible())
		return m_MessageBox.HandleKeyDown(key,rep);

	switch (m_eGameState)
	{
		case GS_LOADINGLEVEL :
		{
            return LTTRUE;
		}
		break;

		case GS_FOLDER :
		{
			if (key != VK_ESCAPE || !g_bLockFolder)
				m_FolderMgr.HandleKeyDown(key,rep);
            return LTTRUE;
		}
		break;

		case GS_PAUSED :
		{
			// They pressed a key - unpause the game

			ChangeState(GS_PLAYING);
            return LTTRUE;
		}
		break;

		case GS_SPLASHSCREEN :
		{
			// They pressed a key - end splash screen...
			// SwitchToFolder(FOLDER_ID_MAIN);
 			ChangeState(GS_MOVIE);
			return LTTRUE;
		}
		break;

		case GS_MOVIE :
		{
			// They pressed a key - end splash screen...

			NextMovie();
            return LTTRUE;
		}
		break;

		case GS_DEMOSCREEN :
		{
			// They pressed a key - go to next screen...
			NextDemoScreen();
            return LTTRUE;
		}
		break;

		case GS_FAILURE :
		{
			if (key == VK_F9)
			{
				g_pGameClientShell->QuickLoad();
			}
			else if (g_fFailScreenDuration > 1.0f)
			{
				// They pressed a key - end fail screen if been here for a second
				if (g_pGameClientShell->IsCustomLevel())
					SwitchToFolder(FOLDER_ID_MAIN);
				else
					SwitchToFolder(FOLDER_ID_FAILURE);
			}
            return LTTRUE;
		}
		break;

		case GS_POPUP :
		{
			if (m_PopupText.OnKeyDown(key, rep))
			{
				return LTTRUE;
			}
		}
		break;

		case GS_DIALOGUE :
		{
			if(m_DialogueWnd.GetDecisionWnd() && m_DialogueWnd.GetDecisionWnd()->IsVisible())
			{
				if(m_DialogueWnd.GetDecisionWnd()->OnKeyDown(key,rep))
					return LTTRUE;
			}

		}
		break;

		case GS_MENU :
		{
			if(m_MenuWnd.OnKeyDown(key,rep))
				return LTTRUE;

		}
		break;

		default : break;
	}


	// Are We Broadcasting a Message

	if (m_messageMgr.GetEditingState())
	{
		m_messageMgr.HandleKeyDown(key, rep);
        return LTTRUE;
	}

	switch (key)
	{
		case VK_PAUSE:
		{
            if (g_pGameClientShell->IsMultiplayerGame() || m_eGameState != GS_PLAYING) return LTFALSE;

			if (!g_pGameClientShell->IsGamePaused())
			{
				ChangeState(GS_PAUSED);
			}

            g_pGameClientShell->PauseGame(!g_pGameClientShell->IsGamePaused(), LTTRUE);
            return LTTRUE;
		}
		break;

		// Escape Key Handling
		case VK_ESCAPE:
		{
			LTBOOL bPlayerDead = g_pGameClientShell->IsPlayerDead();

			if (m_eGameState == GS_PLAYING &&
				g_pGameClientShell->IsPlayerInWorld())
			{
				if (IsChoosingWeapon() && !bPlayerDead)
				{
					m_WeaponChooser.Close();
				}
				else if (IsChoosingAmmo() && !bPlayerDead)
				{
					m_AmmoChooser.Close();
				}
				else
				{
					// Allow the multiplayer menu to be brought up
					// when the player is dead...

					if (g_pGameClientShell->IsMultiplayerGame())
					{
                        m_MultiplayerMenu.Show(LTTRUE);
						ChangeState(GS_MENU);
					}
					else if (!g_bLockFolder && !bPlayerDead)
					{
						SwitchToFolder(FOLDER_ID_ESCAPE);
					}
				}
			}
			else if (m_eGameState == GS_MENU && m_MenuWnd.IsVisible())
			{
				m_MenuWnd.Close();
			}

            return LTTRUE;
		}
		break;

		default : break;
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyUp(int key)
//
//	PURPOSE:	Handle key up notification
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnKeyUp(int key)
{
	// if it's the tilde (~) key then the console has been turned off
	// (this is the only event that causes this key to ever get processed)
	// so clear the back buffer to get rid of any part of the console still showing
	if (FadingToExit()) return LTTRUE;

	if (key == VK_TILDE)
	{
		AddToClearScreenCount();
        return LTTRUE;
	}

	if (m_messageMgr.GetEditingState())
	{
		m_messageMgr.HandleKeyUp(key);
        return LTTRUE;
	}

	if (m_eGameState == GS_FOLDER)
	{
		m_FolderMgr.HandleKeyUp(key);
        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnChar()
//
//	PURPOSE:	Handle OnChar messages
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnChar(char c)
{
	if (c < ' ') return;
	if (m_messageMgr.GetEditingState())
	{
		m_messageMgr.HandleChar(c);
		return;
	}
	if (m_eGameState == GS_FOLDER)
	{
		m_FolderMgr.HandleChar(c);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Draw()
//
//	PURPOSE:	Draws any interface stuff that may need to be drawn
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::Draw()
{

	// Check for interface drawing override...

	if (g_vtDrawInterface.GetFloat())
	{
		// Update letter box...

		UpdateLetterBox();

		// Find out if we're in multiplayer...

        LTBOOL bMultiplayer = g_pGameClientShell->IsMultiplayerGame();
		PlayerState ePlayerState = g_pGameClientShell->GetPlayerState();


		// Draw the player stats (health,armor,ammo) if appropriate...

		if (m_bDrawInterface)
		{
			m_stats.Draw(m_bDrawPlayerStats);
		}
		m_MissionText.Draw();

		if (GetGameState() == GS_PLAYING || GetGameState() == GS_POPUP)
		{
			m_Subtitle.Draw();

			if (m_Credits.IsInited())
			{
				if (m_Credits.IsDone())
					m_Credits.Term();
				else
					m_Credits.Update();
			}

			if (m_InterfaceTimer.GetTime() > 0.0f)
			{
				// Testing
				//g_pLTClient->CPrint("Client Time Left: %.2f", m_InterfaceTimer.GetTime());
				m_InterfaceTimer.Draw(g_pLTClient->GetScreenSurface());
			}

			if (m_InterfaceMeter.GetValue() > 0)
			{
				m_InterfaceMeter.Draw(g_pLTClient->GetScreenSurface());
			}
		}

		if (g_pGameClientShell->GetDamageFXMgr()->IsSleeping())
		{
			if (IsChoosingWeapon())
			{
				m_WeaponChooser.Close();
			}

			if (IsChoosingAmmo())
			{
				m_AmmoChooser.Close();
			}
		}
		else
		{
			if (IsChoosingWeapon())
			{
				if (m_bDrawInterface)
					m_WeaponChooser.Draw();
				else
					m_WeaponChooser.Close();
			}

			if (IsChoosingAmmo())
			{
				if (m_bDrawInterface)
					m_AmmoChooser.Draw();
				else
					m_AmmoChooser.Close();
			}
		}

		// Draw any frag counts (ours only and maybe all) if appropriate
		m_ClientInfo.Draw(m_bDrawInterface, m_bDrawFragCount || (ePlayerState == PS_DEAD && bMultiplayer ));
		// Handle message editing...

		m_messageMgr.Draw();

		// Handle drawing the main window...

        m_MainWnd.Draw(g_pLTClient->GetScreenSurface());

		if (GetGameState() == GS_POPUP)
		{
			m_PopupText.Draw();
		}

		// Update the screen fade...

		UpdateScreenFade();
	}

	if (m_MessageBox.IsVisible())
	{
        g_pLTClient->Start3D();
        g_pLTClient->StartOptimized2D();
        m_MessageBox.Draw(g_pLTClient->GetScreenSurface());
        g_pLTClient->EndOptimized2D();
        g_pLTClient->End3D();
	}

	UpdateCursor();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreFolderState()
//
//	PURPOSE:	Initialize the Folder state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreFolderState(GameState eCurState)
{
    if (eCurState == GS_FOLDER) return LTFALSE;

	m_InterfaceResMgr.Setup();

	// Pause the game...
    g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);

	// Stop DamageFX when not in the playing state...
	g_pGameClientShell->GetDamageFXMgr()->Clear();

	// Disable light scaling when not in the playing state...
	g_pGameClientShell->GetLightScaleMgr()->Disable();


	CreateInterfaceBackground();

    SetDrawInterface(LTFALSE);
	ClearScreenAlways();

    UseCursor(LTTRUE);

	// No fog in the menus...
	g_bInGameFogEnabled = (LTBOOL) GetConsoleInt("FogEnable", 1);
	WriteConsoleInt("FogEnable", 0);

	// Make sure menus and folders are full screen...
    memset(&m_rcMenuRestoreCamera, 0, sizeof (LTRect));

    uint32 nWidth, nHeight;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &nWidth, &nHeight);

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
	if (hCamera)
	{
        g_pLTClient->GetCameraRect(hCamera, &m_bMenuRectRestore, &m_rcMenuRestoreCamera.left, &m_rcMenuRestoreCamera.top, &m_rcMenuRestoreCamera.right, &m_rcMenuRestoreCamera.bottom);
        g_pLTClient->SetCameraRect(hCamera, LTTRUE, 0, 0, (int)nWidth, (int)nHeight);
	}
	m_bUseInterfaceCamera = LTTRUE;
	g_bFirstStateUpdate = LTTRUE;

	ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostFolderState()
//
//	PURPOSE:	Handle leaving the Folder state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostFolderState(GameState eNewState)
{
    if (eNewState == GS_FOLDER) return LTFALSE;

	m_FolderMgr.ExitFolders();

	if (eNewState != GS_LOADINGLEVEL && eNewState != GS_DEMOSCREEN)
	{
		int nGameMode = GAMEMODE_NONE;
        g_pLTClient->GetGameMode(&nGameMode);
        if (nGameMode == GAMEMODE_NONE) return LTFALSE;

		HOBJECT hCamera = g_pGameClientShell->GetCamera();
		if (hCamera && (m_rcMenuRestoreCamera.left != 0 || m_rcMenuRestoreCamera.top != 0 || m_rcMenuRestoreCamera.right != 0 || m_rcMenuRestoreCamera.bottom != 0))
		{
            g_pLTClient->SetCameraRect(hCamera, m_bMenuRectRestore, m_rcMenuRestoreCamera.left, m_rcMenuRestoreCamera.top, m_rcMenuRestoreCamera.right, m_rcMenuRestoreCamera.bottom);
		}
	}

    ClearScreenAlways(LTFALSE);
	AddToClearScreenCount();


	RemoveAllInterfaceSFX();

	if (!g_pGameClientShell->IsPlayerDead())
	{
		SetDrawInterface(LTTRUE);
	}

    UseCursor(LTFALSE);

	// m_InterfaceResMgr.Clean();

    g_pLTClient->ClearInput();

	g_bFirstStateUpdate = LTTRUE;

	// Reset fog value...
	WriteConsoleInt("FogEnable", (int) g_bInGameFogEnabled);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePauseState()
//
//	PURPOSE:	Initialize the Pause state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PrePauseState(GameState eCurState)
{
    if (eCurState == GS_PAUSED) return LTFALSE;

	CreateInterfaceBackground();

	// Create the "paused" surface...
	m_hGamePausedSurface = g_pInterfaceResMgr->GetSharedSurface("interface\\pause.pcx");

	m_bUseInterfaceCamera = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPauseState()
//
//	PURPOSE:	Handle leaving the Pause state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostPauseState(GameState eNewState)
{
    if (eNewState == GS_PAUSED) return LTFALSE;

	RemoveAllInterfaceSFX();

	// Remove the "paused" surface...
	if (m_hGamePausedSurface)
	{
		g_pInterfaceResMgr->FreeSharedSurface(m_hGamePausedSurface);
		m_hGamePausedSurface = LTNULL;
	}



    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePlayingState()
//
//	PURPOSE:	Initialize the Playing state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PrePlayingState(GameState eCurState)
{
    if (eCurState == GS_PLAYING) return LTFALSE;

	if (eCurState != GS_DIALOGUE && eCurState != GS_MENU)
	{
		// Unpause the game...

        g_pGameClientShell->PauseGame(LTFALSE);
	}

	// Eanble light scaling...

	g_pGameClientShell->GetLightScaleMgr()->Enable();

	m_MissionText.Pause(LTFALSE);

	UseCursor(LTFALSE);

    m_bUseInterfaceCamera = LTFALSE;

	RestoreGameMusic();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPlayingState()
//
//	PURPOSE:	Handle leaving the Playing state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostPlayingState(GameState eNewState)
{
    if (eNewState == GS_PLAYING) return LTFALSE;

	// Stop DamageFX when not in the playing state...
	g_pGameClientShell->GetDamageFXMgr()->Clear();

	// Disable light scaling when not in the playing state...
	g_pGameClientShell->GetLightScaleMgr()->Disable();

	m_MissionText.Pause(LTTRUE);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreDialogueState()
//
//	PURPOSE:	Initialize the Dialogue state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreDialogueState(GameState eCurState)
{
    if (eCurState == GS_DIALOGUE) return LTFALSE;

	// Pause game (client only)...

    // g_pGameClientShell->PauseGame(LTTRUE);
    UseCursor(LTTRUE);
	m_bUseInterfaceCamera = LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostDialogueState()
//
//	PURPOSE:	Handle leaving the Dialogue state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostDialogueState(GameState eNewState)
{
    if (eNewState == GS_DIALOGUE) return LTFALSE;

	// Unpause the game...

    // g_pGameClientShell->PauseGame(LTFALSE);
    UseCursor(LTFALSE);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreMenuState()
//
//	PURPOSE:	Initialize the menu state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreMenuState(GameState eCurState)
{
    if (eCurState == GS_MENU) return LTFALSE;

    UseCursor(LTTRUE);
	m_bUseInterfaceCamera = LTFALSE;

	ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostMenuState()
//
//	PURPOSE:	Handle leaving the menu state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostMenuState(GameState eNewState)
{
    if (eNewState == GS_MENU) return LTFALSE;

    UseCursor(LTFALSE);
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePopupState()
//
//	PURPOSE:	Initialize the Popup state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PrePopupState(GameState eCurState)
{
    if (eCurState == GS_POPUP) return LTFALSE;

	// Pause the game...
//    g_pGameClientShell->PauseGame(LTTRUE);
	g_pGameClientShell->AllowPlayerMovement(LTFALSE);

	g_bLockPopup = LTTRUE;

    UseCursor(LTFALSE);
	m_bUseInterfaceCamera = LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPopupState()
//
//	PURPOSE:	Handle leaving the Popup state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostPopupState(GameState eNewState)
{
    if (eNewState == GS_POPUP) return LTFALSE;

	g_pGameClientShell->AllowPlayerMovement(LTTRUE);
	g_pLTClient->ClearInput();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreLoadingLevelState()
//
//	PURPOSE:	Initialize the LoadingLevel state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreLoadingLevelState(GameState eCurState)
{
    if (eCurState == GS_LOADINGLEVEL) return LTFALSE;

	// Disable light scaling when not in the playing state...
	g_pGameClientShell->GetLightScaleMgr()->ClearAllLightScales();

	m_bUseInterfaceCamera = LTTRUE;

	m_nOldLoadWorldCount = m_nLoadWorldCount;

	m_LoadingScreen.Show();

	// Turn off the music (this will be turned on when we start the
	// next level...
	CMusic* pMusic = g_pGameClientShell->GetMusic();
    if (pMusic)
	{
		pMusic->Stop();
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostLoadingLevelState()
//
//	PURPOSE:	Handle leaving the LoadingLevel state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostLoadingLevelState(GameState eNewState)
{
    if (eNewState == GS_LOADINGLEVEL) return LTFALSE;

	// Don't allow the loading state to go away until the loading screen has been hidden
	if (m_LoadingScreen.IsVisible())
		return LTFALSE;

	ClearAllScreenBuffers();
	m_bLoadFailed = LTFALSE;
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreSplashScreenState()
//
//	PURPOSE:	Initialize the SplashScreen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreSplashScreenState(GameState eCurState)
{
    if (eCurState == GS_SPLASHSCREEN) return LTFALSE;

	// Since we're going to always go to the menu state next, load the
	// surfaces here...

	m_InterfaceResMgr.Setup();


	// Play splash screen sound...

    //uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
	//m_hSplashSound = g_pClientSoundMgr->PlayInterfaceSound(IM_SPLASH_SOUND);
    //g_pLTClient->GetSoundDuration(m_hSplashSound, &g_fSplashSndDuration);
    //g_pLTClient->CPrint("Splash sound duration: %.4f", g_fSplashSndDuration);
    //g_pLTClient->CPrint("Current Time: %.4f", g_pLTClient->GetTime());


	// Create the splash screen...
// Image too large on PS2, skip for now
    g_hSplash = g_pLTClient->CreateSurfaceFromBitmap(IM_SPLASH_SCREEN);
    if (!g_hSplash) return LTFALSE;

    g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);

	// Fade into the splash screen...

	StartScreenFadeIn(3.0f);
	m_bUseInterfaceCamera = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostSplashScreenState()
//
//	PURPOSE:	Handle leaving the SplashScreen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostSplashScreenState(GameState eNewState)
{
    if (eNewState == GS_SPLASHSCREEN) return LTFALSE;

	if (g_hSplash)
	{
        g_pLTClient->DeleteSurface(g_hSplash);
        g_hSplash = LTNULL;
	}

	// Stop splash screen sound (if playing)...

	if (m_hSplashSound)
	{
        g_pLTClient->KillSound(m_hSplashSound);
        m_hSplashSound = LTNULL;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreMovieState()
//
//	PURPOSE:	Initialize the movie state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreMovieState(GameState eCurState)
{
    if (eCurState == GS_MOVIE) return LTFALSE;

	g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);
	m_bUseInterfaceCamera = LTTRUE;

	m_nCurMovie = 0;
	NextMovie();


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostMovieState()
//
//	PURPOSE:	Handle leaving the movie state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostMovieState(GameState eNewState)
{
    if (eNewState == GS_MOVIE) return LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreDemoScreenState()
//
//	PURPOSE:	Initialize the DemoScreen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreDemoScreenState(GameState eCurState)
{
    if (eCurState == GS_DEMOSCREEN) return LTFALSE;


	g_nDemo = -1;
	NextDemoScreen();

    g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);

	m_bUseInterfaceCamera = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostDemoScreenState()
//
//	PURPOSE:	Handle leaving the DemoScreen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostDemoScreenState(GameState eNewState)
{
    if (eNewState == GS_DEMOSCREEN) return LTFALSE;

	if (g_hDemo)
	{
        g_pLTClient->DeleteSurface(g_hDemo);
        g_hDemo = LTNULL;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreFailureState()
//
//	PURPOSE:	Initialize the failure state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreFailureState(GameState eCurState)
{
    if (eCurState == GS_FAILURE) return LTFALSE;

	// Pause the game...
    g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);

	g_fFailScreenDuration = 0.0f;

	char szStr[128] = "";
	g_pLayoutMgr->GetFailScreenBackground(szStr,ARRAY_LEN(szStr));
	if (!m_hFailBack)
	{
        m_hFailBack = g_pLTClient->CreateSurfaceFromBitmap(szStr);
	}

	// Since we're going to always go to the menu state next, load the
	// surfaces here...

	m_InterfaceResMgr.Setup();

    g_pLTClient->ClearInput();
	m_bUseInterfaceCamera = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostFailureState()
//
//	PURPOSE:	Handle leaving the failure state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostFailureState(GameState eNewState)
{
    if (eNewState == GS_FAILURE) return LTFALSE;
    g_pLTClient->DeleteSurface(m_hFailBack);
    m_hFailBack = LTNULL;

    return LTTRUE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetLoadGameMenu()
//
//	PURPOSE:	Turn the load game menu on
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::SetLoadGameMenu()
{
	SwitchToFolder(FOLDER_ID_LOAD);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ProcessAdvancedOptions
//
//	PURPOSE:	Process the advanced options
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::ProcessAdvancedOptions()
{
	// Check advanced options...

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("DisableMusic");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_MUSIC;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableSound");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_SOUND;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableMovies");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_MOVIES;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableFog");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_FOG;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableLines");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_LINES;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableJoystick");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_JOYSTICK;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableTripBuf");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_TRIPLEBUFFER;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableTJuncs");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_TJUNCTIONS;
	}

	// Record the original state of sound and music

    hVar = g_pLTClient->GetConsoleVar("SoundEnable");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_SOUND;
	}

    hVar = g_pLTClient->GetConsoleVar("MusicEnable");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_MUSIC;
	}

	// Implement any advanced options here (before renderer is started)

    hVar = g_pLTClient->GetConsoleVar("SoundEnable");
	if (!hVar && (m_dwAdvancedOptions & AO_SOUND))
	{
        g_pLTClient->RunConsoleString("SoundEnable 1");
	}

    hVar = g_pLTClient->GetConsoleVar("MusicEnable");
	if (!hVar && (m_dwAdvancedOptions & AO_MUSIC))
	{
        g_pLTClient->RunConsoleString("MusicEnable 1");
	}


	if (!(m_dwAdvancedOptions & AO_TRIPLEBUFFER))
	{
        g_pLTClient->RunConsoleString("TripleBuffer 0");
	}

	if (!(m_dwAdvancedOptions & AO_FOG))
	{
        g_pLTClient->RunConsoleString("FogEnable 0");
	}

	if (!(m_dwAdvancedOptions & AO_JOYSTICK))
	{
        g_pLTClient->RunConsoleString("JoystickDisable 1");
	}

	if (!(m_dwAdvancedOptions & AO_CURSOR))
	{
        g_pLTClient->RunConsoleString("HardwareCursor 0");
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ChangeState
//
//	PURPOSE:	Change the game state.  This allows us to do pre/post state
//				change handling
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::ChangeState(GameState eNewState)
{
	DebugChangeState(eNewState);

	GameState eCurState = m_eGameState;

	// First make sure we change change to the new state from the the
	// state we are currently in...

	if (PreChangeState(eCurState, eNewState))
	{
		// Make sure the client shell has no problems with us changing
		// to this state...

		if (g_pGameClientShell->PreChangeGameState(eNewState))
		{
			m_eGameState = eNewState;

			if (g_pGameClientShell->PostChangeGameState(eCurState))
			{
				// State changed successfully...

                return LTTRUE;
			}

			// State NOT Changed!

			m_eGameState = eCurState;
		}
	}

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DebugChangeState
//
//	PURPOSE:	Print out debugging info about changing states
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::DebugChangeState(GameState eNewState)
{
#ifdef _DEBUG
//#define _DEBUG_INTERFACE_MGR
#ifdef _DEBUG_INTERFACE_MGR
    g_pLTClient->CPrint("CInterfaceMgr::ChangeState() :");
    g_pLTClient->CPrint("  Old State: %s", c_GameStateNames[m_eGameState]);
    g_pLTClient->CPrint("  New State: %s", c_GameStateNames[eNewState]);
#endif
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreChangeState
//
//	PURPOSE:	Handle pre setting of game state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreChangeState(GameState eCurState, GameState eNewState)
{
	// Do any clean up of the old (current) state...

	switch (eCurState)
	{
		case GS_PLAYING :
		{
            if (!PostPlayingState(eNewState)) return LTFALSE;
		}
		break;

		case GS_DIALOGUE :
		{
            if (!PostDialogueState(eNewState)) return LTFALSE;
		}
		break;

		case GS_MENU :
		{
            if (!PostMenuState(eNewState)) return LTFALSE;
		}
		break;

		case GS_POPUP :
		{
            if (!PostPopupState(eNewState)) return LTFALSE;
		}
		break;

		case GS_LOADINGLEVEL:
		{
            if (!PostLoadingLevelState(eNewState)) return LTFALSE;
		}
		break;

		case GS_FOLDER :
		{
            if (!PostFolderState(eNewState)) return LTFALSE;
		}
		break;

		case GS_PAUSED :
		{
            if (!PostPauseState(eNewState)) return LTFALSE;
		}
		break;

		case GS_SPLASHSCREEN :
		{
            if (!PostSplashScreenState(eNewState)) return LTFALSE;
		}
		break;

		case GS_MOVIE :
		{
            if (!PostMovieState(eNewState)) return LTFALSE;
		}
		break;

		case GS_DEMOSCREEN :
		{
            if (!PostDemoScreenState(eNewState)) return LTFALSE;
		}
		break;

		case GS_FAILURE :
		{
            if (!PostFailureState(eNewState)) return LTFALSE;
		}
		break;

		default : break;
	}

	// Do any initial setup for the new state...

	switch (eNewState)
	{
		case GS_PLAYING :
		{
			return PrePlayingState(eCurState);
		}
		break;

		case GS_DIALOGUE :
		{
			return PreDialogueState(eCurState);
		}
		break;

		case GS_MENU :
		{
			return PreMenuState(eCurState);
		}
		break;

		case GS_POPUP :
		{
			return PrePopupState(eCurState);
		}
		break;

		case GS_LOADINGLEVEL:
		{
			return PreLoadingLevelState(eCurState);
		}
		break;

		case GS_FOLDER :
		{
			return PreFolderState(eCurState);
		}
		break;

		case GS_PAUSED :
		{
			return PrePauseState(eCurState);
		}
		break;

		case GS_SPLASHSCREEN :
		{
			return PreSplashScreenState(eCurState);
		}
		break;

		case GS_MOVIE :
		{
			return PreMovieState(eCurState);
		}
		break;

		case GS_DEMOSCREEN :
		{
			return PreDemoScreenState(eCurState);
		}
		break;

		case GS_FAILURE :
		{
			return PreFailureState(eCurState);
		}
		break;

		default : break;
	}

    return LTTRUE;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Save
//
//	PURPOSE:	Save the interface info
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::Save(HMESSAGEWRITE hWrite)
{
	m_stats.Save(hWrite);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Load
//
//	PURPOSE:	Load the interface info
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::Load(HMESSAGEREAD hRead)
{
	m_stats.Load(hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetCurrentFolder()
//
//	PURPOSE:	Finds out what the current folder is
//				- returns FOLDER_ID_NONE if not in a folder state
//
// --------------------------------------------------------------------------- //

eFolderID CInterfaceMgr::GetCurrentFolder()
{
	if (m_eGameState != GS_FOLDER)
	{
		return FOLDER_ID_NONE;
	}
	return m_FolderMgr.GetCurrentFolderID();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SwitchToFolder
//
//	PURPOSE:	Go to the specified folder
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::SwitchToFolder(eFolderID folderID)
{
	if (m_eGameState != GS_FOLDER)
	{
		if (m_eGameState == GS_SPLASHSCREEN && folderID == FOLDER_ID_MAIN)
		{
			StartScreenFadeIn(3.0);
		}

        if (!ChangeState(GS_FOLDER)) return LTFALSE;
	}

	m_FolderMgr.SetCurrentFolder(folderID);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ForceFolderUpdate
//
//	PURPOSE:	Force the current folder to update
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::ForceFolderUpdate(eFolderID folderID)
{
    if (m_eGameState != GS_FOLDER) return LTFALSE;

	return m_FolderMgr.ForceFolderUpdate(folderID);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::MissionFailed
//
//	PURPOSE:	Go to the mission failure state
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::MissionFailed(int nFailStringId)
{
	if (m_eGameState == GS_FAILURE) return;

	// The player is no longer in the world...
	g_pGameClientShell->SetPlayerNotInWorld();

	m_nFailStringId = nFailStringId;
	ChangeState(GS_FAILURE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ScreenDimsChanged
//
//	PURPOSE:	Handle the screen dims changing
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::ScreenDimsChanged()
{
	m_InterfaceResMgr.ScreenDimsChanged();

	// Update the camera rect...
    uint32 dwWidth = 640, dwHeight = 480;

    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	// This may need to be changed to support in-game cinematics...

	ResetMenuRestoreCamera(0, 0, dwWidth, dwHeight);
    g_pLTClient->SetCameraRect (g_pGameClientShell->GetInterfaceCamera(), LTTRUE, 0, 0, dwWidth, dwHeight);

	UpdateInterfaceBackground();

	m_Subtitle.ScreenDimsChanged();
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SendMissionDataToServer
//
//	PURPOSE:	Send mission data to the server
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::SendMissionDataToServer(LTBOOL bUpdatePlayer)
{
	// Update the server...

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_MISSION_INFO);
    m_MissionData.WriteToMessage(g_pLTClient, hMessage);
    g_pLTClient->WriteToMessageByte(hMessage, (uint8)bUpdatePlayer);
    g_pLTClient->EndMessage(hMessage);


	// Update the client to reflect this info...

	m_stats.Setup(&m_MissionData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DoMissionOutfitCheat
//
//	PURPOSE:	Give the player all the defaults for this mission
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::DoMissionOutfitCheat()
{
	// Setup m_MissionData with the defaults for this mission...

	GetFolderMgr()->SkipOutfitting();

	// Act like we're just starting the mission...

	m_MissionData.SetLevelNum(0);

 	// Update the server...

	SendMissionDataToServer(LTTRUE);
}


//mouse handling
void CInterfaceMgr::OnLButtonUp(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnLButtonUp(x,y);
		return;
	}

	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			int relX = x - g_pInterfaceResMgr->GetXOffset();
			int relY = y - g_pInterfaceResMgr->GetYOffset();
			m_FolderMgr.OnLButtonUp(relX,relY);
		} break;
	case GS_DIALOGUE:
	case GS_MENU:
		{
			m_MainWnd.HandleLButtonUp(x,y);
		} break;

	}
}

void CInterfaceMgr::OnLButtonDown(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnLButtonDown(x,y);
		return;
	}
	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			int relX = x - g_pInterfaceResMgr->GetXOffset();
			int relY = y - g_pInterfaceResMgr->GetYOffset();
			m_FolderMgr.OnLButtonDown(relX,relY);
		}	break;
	case GS_SPLASHSCREEN:
		{
			// They pressed a mouse button - end splash screen...
			//SwitchToFolder(FOLDER_ID_MAIN);
			ChangeState(GS_MOVIE);
		}	break;
	case GS_MOVIE:
		{
			// They pressed a mouse button - next movie
			NextMovie();
		}	break;
	case GS_DEMOSCREEN:
		{
			// They pressed a mouse button - next demo screen
			NextDemoScreen();
		}	break;
	case GS_FAILURE:
		{
			// They pressed a key - end failure screen...
			if (g_fFailScreenDuration > 1.0f)
			{
				if (g_pGameClientShell->IsCustomLevel())
					SwitchToFolder(FOLDER_ID_MAIN);
				else
					SwitchToFolder(FOLDER_ID_FAILURE);
			}
		}	break;
	case GS_DIALOGUE:
	case GS_MENU:
		{
			m_MainWnd.HandleLButtonDown(x,y);
		}	break;
	}
}


void CInterfaceMgr::OnLButtonDblClick(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			int relX = x - g_pInterfaceResMgr->GetXOffset();
			int relY = y - g_pInterfaceResMgr->GetYOffset();
			m_FolderMgr.OnLButtonDblClick(relX,relY);
		} break;
	case GS_DIALOGUE:
	case GS_MENU:
		{
			m_MainWnd.HandleLButtonDblClick(x,y);
		} break;
	}

}

void CInterfaceMgr::OnRButtonUp(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			int relX = x - g_pInterfaceResMgr->GetXOffset();
			int relY = y - g_pInterfaceResMgr->GetYOffset();
			m_FolderMgr.OnRButtonUp(relX,relY);
		} break;
	case GS_DIALOGUE:
	case GS_MENU:
		{
			m_MainWnd.HandleRButtonUp(x,y);
		} break;
	}


}

void CInterfaceMgr::OnRButtonDown(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			int relX = x - g_pInterfaceResMgr->GetXOffset();
			int relY = y - g_pInterfaceResMgr->GetYOffset();
			m_FolderMgr.OnRButtonDown(relX,relY);
		}	break;
	case GS_SPLASHSCREEN:
		{
			// They pressed a button - end splash screen...
			// SwitchToFolder(FOLDER_ID_MAIN);
			ChangeState(GS_MOVIE);
		}	break;
	case GS_MOVIE:
		{
			// They pressed a button - next movie...
			NextMovie();
		}	break;
	case GS_DEMOSCREEN:
		{
			// They pressed a button - next demo screen...
			NextDemoScreen();
		}	break;
	case GS_FAILURE:
		{
			// They pressed a button - end failure screen...
			if (g_fFailScreenDuration > 1.0f)
			{
				if (g_pGameClientShell->IsCustomLevel())
					SwitchToFolder(FOLDER_ID_MAIN);
				else
					SwitchToFolder(FOLDER_ID_FAILURE);
			}
		}	break;
	case GS_DIALOGUE:
	case GS_MENU:
		{
			m_MainWnd.HandleRButtonDown(x,y);
		}	break;
	}
}

void CInterfaceMgr::OnRButtonDblClick(int x, int y)
{
	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			int relX = x - g_pInterfaceResMgr->GetXOffset();
			int relY = y - g_pInterfaceResMgr->GetYOffset();
			m_FolderMgr.OnRButtonDblClick(relX,relY);
		} break;
	}

}

void CInterfaceMgr::OnMouseMove(int x, int y)
{
	int relX = x - g_pInterfaceResMgr->GetXOffset();
	int relY = y - g_pInterfaceResMgr->GetYOffset();
	m_CursorPos.x = relX;
	m_CursorPos.y = relY;

	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnMouseMove(x,y);
		return;
	}

	switch (m_eGameState)
	{
	case GS_FOLDER:
		{
			m_FolderMgr.OnMouseMove(relX,relY);
		} break;
	case GS_DIALOGUE:
	case GS_MENU:
		{
			m_MainWnd.HandleMouseMove(x,y);
		} break;
	}


}

void CInterfaceMgr::UseCursor(LTBOOL bUseCursor)
{
	m_bUseCursor = bUseCursor;
	if (m_bUseCursor)
	{
        g_pLTClient->RunConsoleString("CursorCenter 0");

		if (m_bUseHardwareCursor)
		{
			g_pLTClient->Cursor()->SetCursorMode(CM_Hardware);
		}
	}
	else
	{
        g_pLTClient->RunConsoleString ("CursorCenter 1");
        g_pLTClient->Cursor()->SetCursorMode(CM_None);

	}
}

void CInterfaceMgr::UseHardwareCursor(LTBOOL bUseHardwareCursor)
{
	m_bUseHardwareCursor = bUseHardwareCursor;
	if (m_bUseHardwareCursor)
	{
		if (m_bUseCursor)
		{
			g_pLTClient->Cursor()->SetCursorMode(CM_Hardware);
		}
	}
	else
	{
		if (m_bUseCursor)
		{
			g_pLTClient->Cursor()->SetCursorMode(CM_None);
		}

	}

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateOverlays()
//
//	PURPOSE:	Update the overlay used as a scope crosshair
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateOverlays()
{
	m_eCurrOverlay = OVM_NONE;
	if (!m_nOverlayCount) return;


    LTVector vPos(0, 0, 0), vU, vR, vF, vTemp;
    LTRotation rRot;
	rRot.Init();

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
	if (!hCamera) return;

	// Updated to use camera-relative pos/rot...
    // g_pLTClient->GetObjectPos(hCamera, &vPos);
    // g_pLTClient->GetObjectRotation(hCamera, &rRot);

//    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
//	VEC_MULSCALAR(vTemp, vF, g_fOverlayDist);
//	VEC_ADD(vPos, vPos, vTemp);

    LTFLOAT fFovX = DEG2RAD(g_vtFOVXNormal.GetFloat());
    LTFLOAT fFovY = DEG2RAD(g_vtFOVYNormal.GetFloat());
    g_pLTClient->GetCameraFOV(hCamera, &fFovX, &fFovY);
    LTFLOAT ratioX = (float)tan(fFovX/2) / g_fFovXTan;
    LTFLOAT ratioY = (float)tan(fFovY/2) / g_fFovYTan;


    LTBOOL bDrawnOne = LTFALSE;
	for (int i = 0; i < NUM_OVERLAY_MASKS; i++)
	{
		if (m_hOverlays[i])
		{
			if ((!bDrawnOne || i >= OVM_NON_EXCLUSIVE) && g_pGameClientShell->IsFirstPerson())
			{
				if (i < OVM_NON_EXCLUSIVE)
				{
					m_eCurrOverlay = (eOverlayMask)i;
                    bDrawnOne = LTTRUE;
				}

				if (g_pLayoutMgr->IsMaskSprite((eOverlayMask)i))
				{
					VEC_COPY(vTemp,g_vOverlaySpriteScale);
				}
				else
				{
					VEC_COPY(vTemp,g_vOverlayModelScale);
				}
				VEC_MULSCALAR(vTemp, vTemp, m_fOverlayScaleMult[i]);
				vTemp.x *= ratioX;
				vTemp.y *= ratioY;

				g_pLTClient->SetObjectScale(m_hOverlays[i], &vTemp);

				uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hOverlays[i]);
				g_pLTClient->SetObjectFlags(m_hOverlays[i], dwFlags | FLAG_VISIBLE);


				// Check for special case of camera-activate mode...

				if (i == OVM_CAMERA && m_hOverlays[i])
				{
					ObjectCreateStruct createStruct;
					INIT_OBJECTCREATESTRUCT(createStruct);

					char sprName[128] = "";
					if (m_stats.DrawingActivateGadget())
					{
						g_pLayoutMgr->GetCameraActivateSprite(sprName,sizeof(sprName));
					}
					else
					{
						g_pLayoutMgr->GetMaskSprite(OVM_CAMERA,sprName,sizeof(sprName));
					}

					if (sprName[0])
					{
						SAFE_STRCPY(createStruct.m_Filename, sprName);
						g_pLTClient->Common()->SetObjectFilenames(m_hOverlays[i], &createStruct);
					}
				}
			}
			else
			{
                uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hOverlays[i]);
                g_pLTClient->SetObjectFlags(m_hOverlays[i], dwFlags & ~FLAG_VISIBLE);
			}
		}
	}
}

void CInterfaceMgr::CreateOverlay(eOverlayMask eMask)
{
	HOBJECT hCamera = g_pGameClientShell->GetCamera();
	if (!hCamera) return;

	// Already created this mask
	if (m_hOverlays[eMask]) return;


	m_fOverlayScaleMult[eMask] = g_pLayoutMgr->GetMaskScale(eMask);

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_SPRITE;

    LTVector vPos(0,0,0), vU, vR, vF, vTemp;
    LTRotation rRot;
	rRot.Init();

//    g_pLTClient->GetObjectPos(hCamera, &vPos);
//    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	if (g_pLayoutMgr->IsMaskSprite(eMask))
	{

		VEC_MULSCALAR(vTemp, vF, g_fOverlaySpriteDist);
		VEC_ADD(vPos, vPos, vTemp);
		createStruct.m_Flags = FLAG_REALLYCLOSE | FLAG_VISIBLE | FLAG_SPRITE_NOZ | FLAG_FOGDISABLE | FLAG_NOLIGHT;
		VEC_COPY(createStruct.m_Pos, vPos);
		createStruct.m_Rotation = rRot;


		char sprName[128] = "";
		g_pLayoutMgr->GetMaskSprite(eMask,sprName,sizeof(sprName));

		if (sprName[0])
		{
			SAFE_STRCPY(createStruct.m_Filename, sprName);
			m_hOverlays[eMask] = g_pLTClient->CreateObject(&createStruct);
			m_nOverlayCount++;
		}
	}
	else
	{
		VEC_MULSCALAR(vTemp, vF, g_fOverlayModelDist);
		VEC_ADD(vPos, vPos, vTemp);
		createStruct.m_Flags = FLAG_REALLYCLOSE | FLAG_VISIBLE | FLAG_NOLIGHT | FLAG_ENVIRONMENTMAP;
		VEC_COPY(createStruct.m_Pos, vPos);
		createStruct.m_Rotation = rRot;

		char modName[128] = "";
		char skinName[128] = "";
		g_pLayoutMgr->GetMaskModel(eMask,modName,sizeof(modName));
		g_pLayoutMgr->GetMaskSkin(eMask,skinName,sizeof(skinName));

		createStruct.m_ObjectType = OT_MODEL;

		if (modName[0] && skinName[0])
		{
			SAFE_STRCPY(createStruct.m_Filename, modName);
			SAFE_STRCPY(createStruct.m_SkinName, skinName);
			m_hOverlays[eMask] = g_pLTClient->CreateObject(&createStruct);
/*
		    LTVector vColor;
			LTFLOAT a;
			g_pLTClient->GetObjectColor(m_hOverlays[eMask], &(vColor.x), &(vColor.y), &(vColor.z), &a);
			g_pLTClient->SetObjectColor(m_hOverlays[eMask], vColor.x, vColor.y, vColor.z, 0.999f);
*/
			m_nOverlayCount++;
		}
	}


	// Update to make sure everything is scaled correctly before the next
	// frame is rendered...

	UpdateOverlays();
}

void CInterfaceMgr::RemoveOverlay(eOverlayMask eMask)
{
	if (m_hOverlays[eMask])
	{
		m_nOverlayCount--;
        g_pLTClient->DeleteObject(m_hOverlays[eMask]);
        m_hOverlays[eMask] = LTNULL;
	}
}



void CInterfaceMgr::BeginScope(LTBOOL bNightVision)
{
	if (bNightVision)
		CreateOverlay(OVM_STATIC);
	CreateOverlay(OVM_SCOPE);
//    DrawPlayerStats(LTFALSE);
}


void CInterfaceMgr::EndScope()
{
	RemoveOverlay(OVM_SCOPE);
	RemoveOverlay(OVM_STATIC);

//	DrawPlayerStats(LTTRUE);
}

void CInterfaceMgr::BeginZoom(LTBOOL bIn)
{
	if (bIn)
		CreateOverlay(OVM_ZOOM_IN);
	else
		CreateOverlay(OVM_ZOOM_OUT);
}

void CInterfaceMgr::EndZoom()
{
	RemoveOverlay(OVM_ZOOM_IN);
	RemoveOverlay(OVM_ZOOM_OUT);
}

void CInterfaceMgr::BeginUnderwater()
{
	if (m_stats.HaveAirSupply())
	{
		CreateOverlay(OVM_SCUBA);
		if (!m_hScubaSound)
		{
            uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP;
			m_hScubaSound = g_pClientSoundMgr->PlaySoundLocal("Chars\\Snd\\Player\\breathscuba.wav", SOUNDPRIORITY_PLAYER_LOW, dwFlags);
		}
	}
}


void CInterfaceMgr::EndUnderwater()
{
	RemoveOverlay(OVM_SCUBA);
	if (m_hScubaSound)
	{
		g_pLTClient->KillSound(m_hScubaSound);
		m_hScubaSound = LTNULL;
	}
}

void CInterfaceMgr::BeginSpacesuit()
{
	CreateOverlay(OVM_SPACE);
}


void CInterfaceMgr::EndSpacesuit()
{
	RemoveOverlay(OVM_SPACE);
}

void CInterfaceMgr::SetSunglassMode(eSunglassMode mode)
{
	if (mode == m_eSunglassMode)
		return;

	//clear effects based on current mode
	switch (m_eSunglassMode)
	{
	case SUN_NONE:
		break;
	case SUN_CAMERA:
		RemoveOverlay(OVM_CAMERA);
		break;
	case SUN_MINES:
		g_pGameClientShell->EndMineMode();
		break;
	case SUN_IR:
		g_pGameClientShell->EndInfrared();
		break;
	}


	//create effects based on new mode
	switch (mode)
	{
	case SUN_NONE:
		break;
	case SUN_CAMERA:
		CreateOverlay(OVM_CAMERA);
		break;
	case SUN_MINES:
		{
 			g_pGameClientShell->BeginMineMode();
		}
		break;
	case SUN_IR:
		g_pGameClientShell->BeginInfrared();
		break;
	}

	m_eSunglassMode = mode;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateScreenFade()
//
//	PURPOSE:	Update the screen fade
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateScreenFade()
{
	if (!m_bScreenFade || m_fTotalFadeTime < 0.1f) return;

    LTRect rcSrc;
	rcSrc.Init(0, 0, 2, 2);

    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetSurfaceDims(hScreen, &dwWidth, &dwHeight);

    HLTCOLOR hTransColor = g_pLTClient->SetupColor1(1.0f, 1.0f, 1.0f, LTTRUE);


	// Initialize the surface if necessary...

	if (!m_bFadeInitialized)
	{
		if (m_hFadeSurface)
		{
            g_pLTClient->DeleteSurface(m_hFadeSurface);
		}

        m_hFadeSurface = g_pLTClient->CreateSurface(2, 2);
		if (!m_hFadeSurface) return;

        g_pLTClient->SetSurfaceAlpha(m_hFadeSurface, 1.0f);
        g_pLTClient->FillRect(m_hFadeSurface, &rcSrc, kBlack);
        g_pLTClient->OptimizeSurface(m_hFadeSurface, hTransColor);

		m_fCurFadeTime = m_fTotalFadeTime;
        m_bFadeInitialized = LTTRUE;
	}


	// Fade the screen if the surface is around...

	if (m_hFadeSurface)
	{
        LTFLOAT fTimeDelta = m_bFadeIn ? m_fCurFadeTime : (m_fTotalFadeTime - m_fCurFadeTime);

        LTFLOAT fLinearAlpha = (fTimeDelta / m_fTotalFadeTime);
		fLinearAlpha = fLinearAlpha < 0.0f ? 0.0f : (fLinearAlpha > 1.0f ? 1.0f : fLinearAlpha);

        LTFLOAT fAlpha = 1.0f - WaveFn_SlowOn(1.0f - fLinearAlpha);
		fAlpha = fAlpha < 0.0f ? 0.0f : (fAlpha > 1.0f ? 1.0f : fAlpha);

        g_pLTClient->SetSurfaceAlpha(m_hFadeSurface, fAlpha);

        LTRect rcDest;
		rcDest.Init(0, 0, dwWidth, dwHeight);

        g_pLTClient->ScaleSurfaceToSurfaceTransparent(hScreen, m_hFadeSurface, &rcDest, &rcSrc, hTransColor);

        m_fCurFadeTime -= (g_pGameClientShell->GetFrameTime() < MAX_FRAME_DELTA ? g_pGameClientShell->GetFrameTime() : MAX_FRAME_DELTA);
	}

	// See if we're done...

	if (ScreenFadeDone())
	{
		if (m_hFadeSurface)
		{
            g_pLTClient->DeleteSurface(m_hFadeSurface);
            m_hFadeSurface = LTNULL;
		}

		// If we faded in we're done...

		if (m_bFadeIn)
		{
            m_bFadeInitialized  = LTFALSE;
            m_bScreenFade       = LTFALSE;
		}
		else
		{
			// We need to keep the screen black until it is faded
			// back in, so we'll just make sure the screen is clear
			// (and not set m_bScreenFade so we'll be called to
			// clear the screen every frame until we fade back in)...

            g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);


			// Exit the level if necessary...

			if (m_bExitAfterFade)
			{
                m_bFadeIn = LTTRUE;
                m_bExitAfterFade = LTFALSE;
				g_pGameClientShell->ExitLevel();
			}
		}
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateLetterBox()
//
//	PURPOSE:	Update the letterbox
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateLetterBox()
{
	LTBOOL bOn = (m_bLetterBox || g_vtLetterBox.GetFloat());

	LTFLOAT fTime = g_pLTClient->GetTime();

	if (bOn && !m_bWasLetterBox)
	{
		// Handle being turned on...

		m_fLetterBoxAlpha = 0.0f;
		m_fLetterBoxFadeEndTime = fTime + g_vtLetterBoxFadeInTime.GetFloat();
	}
	else if (!bOn && m_bWasLetterBox)
	{
		// Handle being turned off...

		m_fLetterBoxAlpha = 1.0f;
		m_fLetterBoxFadeEndTime = fTime + g_vtLetterBoxFadeOutTime.GetFloat();
	}

	// Store the current state of being on or off...

	m_bWasLetterBox = bOn;


	LTFLOAT fTimeLeft = m_fLetterBoxFadeEndTime - fTime;
	if (fTimeLeft < 0.0f) fTimeLeft = 0.0f;

	// Calculate the current alpha...

	if (bOn)
	{
		// Fading in...

		LTFLOAT fFadeTime = g_vtLetterBoxFadeInTime.GetFloat();

		if (fFadeTime <= 0.0f || fTimeLeft <= 0.0f)
		{
			m_fLetterBoxAlpha = 1.0f;
		}
		else
		{
			m_fLetterBoxAlpha = 1.0f - (fTimeLeft / fFadeTime);
		}
	}
	else
	{
		// Fading out...

		LTFLOAT fFadeTime = g_vtLetterBoxFadeOutTime.GetFloat();

		if (fFadeTime <= 0.0f || fTimeLeft <= 0.0f)
		{
			m_fLetterBoxAlpha = 0.0f;
		}
		else
		{
			m_fLetterBoxAlpha = fTimeLeft / fFadeTime;
		}
	}


	// If not using letter box, and we faded it out, we're done...

	if (!bOn && m_fLetterBoxAlpha <= 0.0f) return;


	// Determine the border size...

	float fBorderPercent = 0.0f;
	float fLetterBoxPercent = g_pClientButeMgr->GetCameraAttributeFloat(CAMERA_BUTE_LETTERBOXPERCENT);

	if (fLetterBoxPercent > 0.01f && fLetterBoxPercent <= 1.0f)
	{
		fBorderPercent = (1.0f - fLetterBoxPercent) / 2.0f;
	}
    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetSurfaceDims(hScreen, &dwWidth, &dwHeight);

	g_pLTClient->SetSurfaceAlpha(m_hLetterBoxSurface, m_fLetterBoxAlpha);

	int nBorderSize = (int) (float(dwHeight) * fBorderPercent);

 	LTRect rcDest;
	rcDest.Init(0, 0, dwWidth, nBorderSize);

	LTRect rcSrc;
	rcSrc.Init(0, 0, 2, 2);

	HLTCOLOR hTransColor = g_pLTClient->SetupColor1(1.0f, 1.0f, 1.0f, LTTRUE);

	// Cover top of screen...

	g_pLTClient->ScaleSurfaceToSurfaceTransparent(hScreen, m_hLetterBoxSurface,
	   &rcDest, &rcSrc, hTransColor);

	// Cover bottom of screen...

	rcDest.Init(0, dwHeight - nBorderSize, dwWidth, dwHeight);
 	g_pLTClient->ScaleSurfaceToSurfaceTransparent(hScreen, m_hLetterBoxSurface,
	   &rcDest, &rcSrc, hTransColor);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AllowCameraMovement
//
//	PURPOSE:	Can the camera move (while in this state)
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::AllowCameraMovement()
{
	switch (m_eGameState)
	{
		case GS_DIALOGUE :
		case GS_MENU :
		case GS_POPUP :
            return LTFALSE;
		break;

		default :
		break;
	}

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ChooseTeam
//
//	PURPOSE:	Display the choose team menu
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::ChooseTeam()
{
	if (g_pGameClientShell->GetPlayerState() == PS_GHOST)
	{
		m_messageMgr.AddLine(IDS_NO_RESPAWN);
	}
	else
		m_TeamMenu.Show(LTTRUE);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ViewOptions
//
//	PURPOSE:	Display the choose team menu
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::ViewOptions()
{
	m_OptionMenu.Show(LTTRUE);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::CreateInterfaceBackground
//
//	PURPOSE:	Create the sprite used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::CreateInterfaceBackground()
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;

	char sprName[128] = "";

	BSCREATESTRUCT bcs;

    LTVector vPos, vU, vR, vF, vTemp, vScale;
    LTRotation rRot;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	vScale = g_vBaseBackScale * g_pLayoutMgr->GetBackSpriteScale();


	VEC_MULSCALAR(vTemp, vF, g_fBackDist);
	VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
	VEC_ADD(vPos, vPos, vTemp);

	VEC_COPY(bcs.vPos, vPos);
    bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);

	g_pLayoutMgr->GetBackSprite(sprName,sizeof(sprName));

	bcs.pFilename = sprName;
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;
	bcs.nType = OT_SPRITE;
	bcs.fInitialAlpha = 1.0f;
	bcs.fFinalAlpha = 1.0f;
	bcs.fLifeTime = 1000000.0f;

	if (m_BackSprite.Init(&bcs))
	{
        m_BackSprite.CreateObject(g_pLTClient);
	}

	// TESTING - ADD A LIGHT TO LIGHT UP MODELS IN THE INTERFACE...
/*
	if (!g_hLight)
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType	= OT_LIGHT;
		createStruct.m_Flags		= FLAG_VISIBLE;

        g_pLTClient->GetObjectPos(hCamera, &createStruct.m_Pos);
		//vPos -= (vF * (g_fBackDist / 2.0f));
		//createStruct.m_Pos = vPos;

		createStruct.m_Pos.y += 30.0f;

        g_hLight = g_pLTClient->CreateObject(&createStruct);

        g_pLTClient->SetLightColor(g_hLight, 0.5f, 0.5f, 0.5f);
        g_pLTClient->SetLightRadius(g_hLight, 500.0f);
	}
*/
	// END TEST!!!
}

void CInterfaceMgr::UpdateInterfaceBackground()
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;


    LTVector vPos, vU, vR, vF, vTemp, vScale;
    LTRotation rRot;

    g_pLTClient->GetObjectPos(hCamera, &vPos);
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vTemp, vF, g_fBackDist);
	VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
	VEC_ADD(vPos, vPos, vTemp);

    g_pLTClient->SetObjectPos(m_BackSprite.GetObject(), &vPos);
}

void CInterfaceMgr::RemoveInterfaceBackground()
{
	m_BackSprite.Term();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AddInterfaceSFX
//
//	PURPOSE:	Add a SFX object to the interface
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::AddInterfaceSFX(CSpecialFX* pSFX, ISFXType eType)
{
	if (!pSFX) return;

    uint32 index = m_InterfaceSFX.FindElement(pSFX);
	if (index >= m_InterfaceSFX.GetSize())
	{
		if (m_InterfaceSFX.GetSize() < MAX_INTERFACE_SFX)
		{
			m_InterfaceSFX.Add(pSFX);
		}
	}

	HOBJECT hObj = pSFX->GetObject();
	if (g_pLTClient->GetObjectType(hObj) == OT_MODEL)
	{
		char* pAniName = LTNULL;
		switch (eType)
		{
			case IFX_NORMAL :
				pAniName = "Interface";
			break;

			case IFX_ATTACH :
				pAniName = "Hand";
			break;

			case IFX_WORLD :
			default :
				pAniName = "World";
			break;
		}

		if (pAniName)
		{
			uint32 dwAni = g_pLTClient->GetAnimIndex(hObj, pAniName);
			if (dwAni != INVALID_ANI)
			{
				g_pLTClient->SetModelAnimation(hObj, dwAni);
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::RemoveInterfaceSFX
//
//	PURPOSE:	Remove a SFX object from the interface
//
// --------------------------------------------------------------------------- //
void CInterfaceMgr::RemoveInterfaceSFX(CSpecialFX* pSFX)
{
    uint32 index = m_InterfaceSFX.FindElement(pSFX);
	if (index < m_InterfaceSFX.GetSize())
	{
//      g_pLTClient->CPrint("removing SFX[%d]",index);
		m_InterfaceSFX.Remove(index);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RemoveAllInterfaceSFX
//
//	PURPOSE:	Remove the 3D objects used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::RemoveAllInterfaceSFX()
{
	while (m_InterfaceSFX.GetSize() > 0)
	{
		//m_InterfaceSFX[0]->Term();
		m_InterfaceSFX.Remove(0);
	}

	RemoveInterfaceBackground();
}

void CInterfaceMgr::UpdateModelAnimations(LTFLOAT fCurFrameDelta)
{
    for (uint32 i = 0; i < m_InterfaceSFX.GetSize(); i++)
	{
		if (g_pLTClient->GetObjectType(m_InterfaceSFX[i]->GetObject()) == OT_MODEL)
			g_pModelLT->UpdateMainTracker(m_InterfaceSFX[i]->GetObject(), fCurFrameDelta);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateInterfaceSFX
//
//	PURPOSE:	Update the 3D Objects used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateInterfaceSFX()
{
	HOBJECT hCamera = g_pGameClientShell->GetInterfaceCamera();
	if (!hCamera) return;

    uint32 numSfx = m_InterfaceSFX.GetSize();
	HLOCALOBJ objs[MAX_INTERFACE_SFX + 2];
	//HLOCALOBJ objs[MAX_INTERFACE_SFX + 1];

	m_BackSprite.Update();
	objs[0] = m_BackSprite.GetObject();

	int next = 1;
    for (uint32 i = 0; i < numSfx && next < MAX_INTERFACE_SFX; i++)
	{
		if (m_InterfaceSFX[i]->Update())
		{
			objs[next] = m_InterfaceSFX[i]->GetObject();

			next++;
		}
	}
    g_pLTClient->RenderObjects(hCamera, objs, next);

	// TESTING DYNAMIC LIGHT IN INTERFACE
	/*
	if (g_hLight)
	{
		objs[i+1] = g_hLight;
        g_pLTClient->RenderObjects(hCamera, objs, numSfx + 2);
	}
	else
	{
        g_pLTClient->RenderObjects(hCamera, objs, numSfx + 1);
	}
	*/
	// END TEST
}


/******************************************************************/
void CInterfaceMgr::RequestInterfaceSound(InterfaceSound eIS)
{
	if (m_eNextSound <= eIS)
	{
		ClearInterfaceSound();

		m_eNextSound = eIS;

		m_hSound = UpdateInterfaceSound();
	}
}

void CInterfaceMgr::ClearInterfaceSound()
{
	m_eNextSound = IS_NONE;

	if (m_hSound)
	{
        g_pLTClient->KillSound(m_hSound);
		m_hSound = LTNULL;
	}
}

HLTSOUND CInterfaceMgr::UpdateInterfaceSound()
{
	HLTSOUND hSnd = LTNULL;

	switch (m_eNextSound)
	{
		case IS_SELECT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect(), PLAYSOUND_GETHANDLE);
		break;
		case IS_CHANGE:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundChange(), PLAYSOUND_GETHANDLE);
		break;
		case IS_PAGE:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundPageChange(), PLAYSOUND_GETHANDLE);
		break;
		case IS_UP:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowUp(), PLAYSOUND_GETHANDLE);
		break;
		case IS_DOWN:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowDown(), PLAYSOUND_GETHANDLE);
		break;
		case IS_LEFT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowLeft(), PLAYSOUND_GETHANDLE);
		break;
		case IS_RIGHT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowRight(), PLAYSOUND_GETHANDLE);
		break;
		case IS_NO_SELECT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundUnselectable(), PLAYSOUND_GETHANDLE);
		break;
		default :
		break;
	}
	m_eNextSound = IS_NONE;

	return hSnd;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::NextMovie
//
//	PURPOSE:	Go to the next movie, if there is one
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::NextMovie()
{
	ILTVideoMgr* pVideoMgr = g_pLTClient->VideoMgr();
	if (!pVideoMgr) return;

	if (!(GetAdvancedOptions() & AO_MOVIES) || g_vtDisableMovies.GetFloat())
	{
		SwitchToFolder(FOLDER_ID_MAIN);
		return;
	}

	if (m_hMovie)
	{
		pVideoMgr->StopVideo(m_hMovie);
		m_hMovie = LTNULL;
	}

	char* pMovie = GetCurrentMovie();

	if (!pMovie || pVideoMgr->StartOnScreenVideo(pMovie, PLAYBACK_FULLSCREEN, m_hMovie) != LT_OK)
	{
		m_nCurMovie = 0;
		m_hMovie = LTNULL;
		SwitchToFolder(FOLDER_ID_MAIN);
		return;
	}

 	m_nCurMovie++;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetCurrentMovie
//
//	PURPOSE:	Get the name of the current movie to play...
//
// --------------------------------------------------------------------------- //

char* CInterfaceMgr::GetCurrentMovie()
{
	char* pMovie = LTNULL;

	switch (m_nCurMovie)
	{
		case MOVIE_FOX_LOGO :
			pMovie = "Movies\\foxpc.bik";
		break;

		case MOVIE_MONOLITH_LOGO :
			pMovie = "Movies\\LithLogo.bik";
		break;

		case MOVIE_LITHTECH_LOGO :
			pMovie = "Movies\\LTLogo.bik";
		break;

		default : break;
	}

	return pMovie;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::NextDemoScreen
//
//	PURPOSE:	Go to the next marketing screen, if there is one
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::NextDemoScreen()
{

	if (g_hDemo)
	{
		g_pLTClient->DeleteSurface(g_hDemo);
		g_hDemo = LTNULL;
	}
	g_nDemo++;
	if (g_nDemo < NUM_DEMO_SCREENS)
	{
		g_hDemo = g_pLTClient->CreateSurfaceFromBitmap(g_szDemoScreens[g_nDemo]);
	}
	if (!g_hDemo)
	{
		m_bSeenDemoScreens = LTTRUE;
	}
}


void CInterfaceMgr::ShowDemoScreens(LTBOOL bQuit)
{
	m_bQuitAfterDemoScreens = bQuit;
	m_bSeenDemoScreens = LTFALSE;
	ChangeState(GS_DEMOSCREEN);
}


void CInterfaceMgr::UpdateCursor()
{
	LTBOOL bHWC = (GetConsoleInt("HardwareCursor",0) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);
	if (bHWC != m_bUseHardwareCursor)
		UseHardwareCursor(bHWC);
	if (m_bUseCursor && !m_bUseHardwareCursor)
	{
		g_pLTClient->Start3D();
		g_pLTClient->StartOptimized2D();
		int curX = m_CursorPos.x + g_pInterfaceResMgr->GetXOffset();
		int curY = m_CursorPos.y + g_pInterfaceResMgr->GetYOffset();

		g_pLTClient->DrawSurfaceToSurfaceTransparent(g_pLTClient->GetScreenSurface(), m_InterfaceResMgr.GetSurfaceCursor(), LTNULL,
												   curX, curY, hDefaultTransColor);

		g_pLTClient->EndOptimized2D();
		g_pLTClient->End3D();

	}
}

// Find the filename in the missing filename result..
const char *FindFilename(const char *pFileName)
{
	// Jump to the end..
	const char *pResult = &pFileName[strlen(pFileName)];
	// Look for a backslash
	while ((pResult != pFileName) && (pResult[-1] != '\\'))
		--pResult;
	// Return that..
	return pResult;
}

void CInterfaceMgr::Disconnected(uint32 nDisconnectFlag)
{
	if (!m_bStartedNew)
	{
		if (GetGameState() == GS_LOADINGLEVEL)
		{
			LoadFailed();
		}
		else
		{
			ClearAllScreenBuffers();
			SwitchToFolder(FOLDER_ID_MAIN);
		}

		uint32 nDisconnectCode = g_pGameClientShell->GetDisconnectCode();
		uint32 nDisconnectSubCode = g_pGameClientShell->GetDisconnectSubCode();
		const char *pDisconnectMsg = g_pGameClientShell->GetDisconnectMsg();
		HSTRING hString;
		switch (nDisconnectCode)
		{
			case LT_DISCON_MISSINGFILE :
			{
				hString = g_pLTClient->FormatString(IDS_DISCON_MISSINGFILE, FindFilename(pDisconnectMsg));
			}
			break;
			case LT_DISCON_CONNECTTERM :
			{
				hString = g_pLTClient->FormatString(IDS_DISCON_CONNECTTERM);
			}
			break;
			case LT_DISCON_SERVERBOOT :
			{
				switch (nDisconnectSubCode)
				{
					case GAME_DISCON_BADHANDSHAKE :
					{
						hString = g_pLTClient->FormatString(IDS_DISCON_BADHANDSHAKE);
					}
					break;
					case GAME_DISCON_BADWEAPONS :
					{
						hString = g_pLTClient->FormatString(IDS_DISCON_BADWEAPONS);
					}
					break;
					case GAME_DISCON_BADCSHELL :
					{
						hString = g_pLTClient->FormatString(IDS_DISCON_BADCSHELL);
					}
					break;
					default :
					{
						hString = g_pLTClient->FormatString(IDS_DISCON_SERVERBOOT);
					}
					break;
				}
			}
			break;
			case LT_DISCON_TIMEOUT :
			{
				hString = g_pLTClient->FormatString(IDS_DISCON_TIMEOUT);
			}
			break;

			default :
			{
				hString = g_pLTClient->FormatString(IDS_DISCONNECTED_FROM_SERVER, pDisconnectMsg);
			}
			break;
		}
		g_pGameClientShell->ClearDisconnectCode();
		g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL,g_pInterfaceResMgr->IsEnglish());
		g_pLTClient->FreeString(hString);
	}
	m_bStartedNew = LTFALSE;

}

void CInterfaceMgr::ConnectionFailed(uint32 nConnectionError)
{
	if (GetGameState() == GS_LOADINGLEVEL)
	{
		LoadFailed();
	}
	else
	{
		ClearAllScreenBuffers();
		SwitchToFolder(FOLDER_ID_MAIN);
	}

	HSTRING hString = g_pLTClient->FormatString(IDS_NETERR_JOINSESSION);
	g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
	g_pLTClient->FreeString(hString);

}

// hides or shows cursor based on current game state
void CInterfaceMgr::UpdateCursorState()
{
	switch (m_eGameState)
	{
	case GS_PLAYING:
	case GS_LOADINGLEVEL:
	case GS_SPLASHSCREEN:
	case GS_POPUP:
	case GS_PAUSED:
	case GS_FAILURE:
	case GS_DEMOSCREEN:
	case GS_MOVIE:
	case GS_UNDEFINED:
		UseCursor(m_MessageBox.IsVisible());
		break;
	case GS_DIALOGUE:
	case GS_MENU:
	case GS_FOLDER:
	default:
		UseCursor(LTTRUE);
	}
}



void CInterfaceMgr::UpdateClientList()
{
	if (IsMultiplayerGame())
	{
		// Don't send update requests more than once per second
		if ((g_pLTClient->GetTime() - m_fLastUpdateRequestTime) > 1.0f)
		{
			HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_MULTIPLAYER_UPDATE);
			g_pLTClient->EndMessage2(hWrite, MESSAGE_NAGGLEFAST | MESSAGE_GUARANTEED);
			m_fLastUpdateRequestTime = g_pLTClient->GetTime();
		}
	}
}

