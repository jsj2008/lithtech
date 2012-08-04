// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMgr.cpp
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "WeaponStringDefs.h"
#include "VKDefs.h"
#include "SoundMgr.h"
#include "InterfaceResMgr.h"
#include "VarTrack.h"
#include "ClientButeMgr.h"
#include "CharacterFX.h"
#include "GameButes.h"
#include "UberAssert.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "ClientMultiplayerMgr.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"
#include "ScreenPreload.h"
#include "ScreenPostload.h"
#include "timer.h"
#include "WaveFn.h"

CInterfaceMgr*  g_pInterfaceMgr = LTNULL;

#define IM_SPLASH_SOUND		"Interface\\Menu\\Snd\\theme_mp3.wav"

#ifdef _TO2DEMO
#define IM_SPLASH_SCREEN	"Interface\\demosplash.pcx"
#else
#define IM_SPLASH_SCREEN	"Interface\\Menu\\Art\\splash.pcx"
#endif

#define MAX_INTERFACE_SFX		100
#define MAX_INTERFACE_LIGHTS	5
#define INVALID_ANI			((HMODELANIM)-1)

static enum MovieOrderEnum 
{
	eSierra,
	eFox,
	eMonolith,
	eLithTech
};

//hack to track whether we are trying to host/join a LAN only or an internet game
bool g_bLAN = false;

static MovieOrderEnum gMovieOrder[] =
{
	eSierra, eFox, eMonolith, eLithTech
};

LTFLOAT      g_fSplashSndDuration = 0.0f;

LTFLOAT      g_fFovXTan = 0.0f;
LTFLOAT      g_fFovYTan = 0.0f;

LTVector     g_vOverlaySpriteScale(0.02f, 0.02f, 1.0f);
LTVector     g_vOverlayModelScale(1.0f, 1.0f, 1.0f);
LTFLOAT      g_fOverlaySpriteDist = 1.0f;
LTFLOAT      g_fOverlayModelDist = 0.25f;

LTVector     g_vBaseBackScale(0.8f, 0.6f, 1.0f);
LTFLOAT      g_fBackDist = 200.0f;

HSURFACE    g_hSplash = LTNULL;
HSURFACE    g_hDemo = LTNULL;
int			g_nDemo = 0;

#define NUM_DEMO_SCREENS 3
char  g_szDemoScreens[NUM_DEMO_SCREENS][64] =
{
	"Interface\\DemoEndSplash1.pcx",
	"Interface\\DemoEndSplash2.pcx",
	"Interface\\DemoEndSplash3.pcx",
};
uint8 g_nNumDemoScreens = sizeof(g_szDemoScreens)/sizeof(char*);


VarTrack	g_vtDrawInterface;
VarTrack	g_vtModelApplySun;
VarTrack	g_vtLetterBox;
VarTrack	g_vtLetterBoxFadeInTime;
VarTrack	g_vtLetterBoxFadeOutTime;
VarTrack	g_vtLetterBoxDisabled;
VarTrack	g_vtDisableMovies;
VarTrack	g_vtInterfaceFOVX;
VarTrack	g_vtInterfaceFOVY;
VarTrack	g_vtPauseTintAlpha;
VarTrack	g_vtSplashScreenFadeIn;
VarTrack	g_vtSplashScreenFadeOut;
VarTrack	g_vtSplashScreenTime;
VarTrack	g_vtMainScreenFadeIn;
VarTrack	g_vtProgressBarScaleToSkills;
VarTrack	g_vtExitLevelScreenFadeTime;

extern VarTrack g_vtFOVXNormal;
extern VarTrack g_vtFOVYNormal;

extern VarTrack g_vtScreenFadeInTime;

const char* c_GameStateNames[] =
{
	"GS_UNDEFINED",
	"GS_PLAYING",
	"GS_EXITINGLEVEL",
	"GS_LOADINGLEVEL",
	"GS_SPLASHSCREEN",
	"GS_MENU",
	"GS_POPUP",
	"GS_SCREEN",
	"GS_PAUSED",
	"GS_DEMOSCREEN",
	"GS_MOVIE",
};


static LTMatrix GetCameraTransform(HOBJECT hCamera)
{
    LTVector vPos, vRight, vUp, vForward;
    LTRotation rRot;

	g_pLTClient->GetObjectPos(hCamera, &vPos);
	g_pLTClient->GetObjectRotation(hCamera, &rRot);

	vPos.x = -vPos.x;
	vPos.y = -vPos.y;
	vPos.z = -vPos.z;

    LTMatrix mTran, mRot, mFull;

	mRot.SetBasisVectors((LTVector*)&rRot.Right(), (LTVector*)&rRot.Up(), (LTVector*)&rRot.Forward());
	MatTranspose3x3(&mRot);

	Mat_Identity(&mTran);
	mTran.m[0][3] = vPos.x;
	mTran.m[1][3] = vPos.y;
	mTran.m[2][3] = vPos.z;

	MatMul(&mFull, &mRot, &mTran);

	return mFull;
}


namespace
{
	LTBOOL g_bInGameFogEnabled = LTTRUE;

	HLTCOLOR hBackColor;

	//**********************************************************
	//*** definitions needed for rendering FX in the interface
	//**********************************************************
	typedef struct FXRenderInfo_t
	{
		HOBJECT hObj;
		uint8	nLayer;
	} FXRenderInfo;

	const int kMaxFX = MAX_INTERFACE_SFX + MAX_INTERFACE_LIGHTS + 2;
	static FXRenderInfo sRenderInfo[kMaxFX];
	typedef std::list<FXRenderInfo *> FXRenderList;
	FXRenderList renderList;


	typedef std::vector<int> KeystrokeList;
	KeystrokeList g_keys;

	bool g_bLockPopup = true;

	CTimer g_SplashScreenTimer;
}


static LTBOOL HandleDebugKey(int key )
{
#ifdef _FINAL
	return LTFALSE;
#endif

#ifdef _TO2DEMO
	//don't bother checking in demo build
	return LTFALSE;
#endif

	char szTempStr[256];
	char szVarName[256];

	int iID = -1;

	// Go through each debug key and see if it matches the key hit.
	// Favor modified versions of keys (e.g. shift F11).

	for( int i = 0; i < g_pClientButeMgr->GetNumDebugKeys(); ++i )
	{
		if( g_pClientButeMgr->GetDebugKeyId(i) == key )
		{
			if( IsKeyDown( g_pClientButeMgr->GetDebugModifierId(i) ) )
			{
				iID = i;	
			}
			else if( iID == -1 )
			{
				iID = i;
			}
		}
	}

	if( iID != -1 )
	{
		szVarName[0] = 0;
		g_pClientButeMgr->GetDebugName(iID,szVarName,256);

		if( !szVarName[0] )
		{
			return LTFALSE;
		}

		// Get the console variable associated with this debug key.
		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(szVarName);

		// Initialize the variable if it does not exist.
		if( !hVar )
		{
			sprintf(szTempStr, "\"%s\" \"%2f\"", szVarName, 0.0f );
			g_pLTClient->RunConsoleString( szTempStr );

			hVar = g_pLTClient->GetConsoleVar(szVarName);
			ASSERT( hVar );

			if( !hVar )
			{
				return LTFALSE;
			}
		}

		// Increment the current debug level, recycling to zero if it is above max levels.
		LTFLOAT fCurrentLevel = g_pLTClient->GetVarValueFloat(hVar);
		++fCurrentLevel;

		if( int(fCurrentLevel) >= g_pClientButeMgr->GetNumDebugLevels(iID) )
		{
			fCurrentLevel = 0.0f;
		}
		
		// Set the new debug level.
		sprintf(szTempStr, "\"%s\" \"%2f\"", szVarName, fCurrentLevel );
		g_pLTClient->RunConsoleString( szTempStr );

		g_pLTClient->CPrint("%s %2f", szVarName, fCurrentLevel );

		szTempStr[0] = 0;
		g_pClientButeMgr->GetDebugString(iID,uint8(fCurrentLevel), szTempStr, 256);
		if( szTempStr )
		{
			g_pLTClient->RunConsoleString( szTempStr );
#ifdef _DEBUG
			g_pLTClient->CPrint( szTempStr );
#endif
		}

		// Display message
		szTempStr[0] = 0;
		g_pClientButeMgr->GetDebugTitle(iID,uint8(fCurrentLevel), szTempStr, 256);
		if( szTempStr[0] )
		{
			g_pChatMsgs->AddMessage( szTempStr );
		}
		else
		{
			sprintf(szTempStr, "%s set to level %1.0f", szVarName, fCurrentLevel );

			g_pChatMsgs->AddMessage( szTempStr );
		}

		return LTTRUE;
	}

	return LTFALSE;
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
	m_eLastGameState		= GS_UNDEFINED;


	m_dwAdvancedOptions		= AO_DEFAULT_ENABLED;
	m_dwOrignallyEnabled	= 0;

	m_nClearScreenCount		= 0;
    m_bClearScreenAlways    = LTFALSE;

	m_fMenuSaveFOVx			= 0.0f;
	m_fMenuSaveFOVy			= 0.0f;

    m_bSwitchingModes       = LTFALSE;

    m_hSplashSound          = LTNULL;
	g_fSplashSndDuration	= 0.0f;

    m_nFailStringId         = LTNULL;

	m_bUseInterfaceCamera	= LTTRUE;
	m_hInterfaceCamera		= LTNULL;

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
/*
	m_nBorderSize		= 0;
*/
	m_fLetterBoxAlpha	= 0.0f;
	m_bLetterBox		= LTFALSE;
	m_bWasLetterBox		= LTFALSE;
	m_fLetterBoxFadeEndTime = 0.0f;

    m_bScreenFade       = LTFALSE;
    m_bFadeInitialized  = LTFALSE;
	m_fTotalFadeTime	= 0.0f;
	m_fCurFadeTime		= 0.0f;
    m_bFadeIn           = LTTRUE;

	m_bSavedGameMusic   = LTFALSE;

	m_eNextSound		= IS_NONE;

	m_bQuitAfterDemoScreens = LTFALSE;
	m_bSeenDemoScreens = LTFALSE;

	m_hMovie			= LTNULL;
	m_nCurMovie			= gMovieOrder[0];

	m_bLoadFailed		= LTFALSE;
	m_bCommandLineJoin = false;
	m_eLoadFailedScreen = SCREEN_ID_MAIN;
	m_nLoadFailedMsgId	= -1;

	m_hGamePausedSurface = LTNULL;

	m_fLastUpdateRequestTime = 0.0f;

	m_MouseFX.ClearLink();
	m_SelectFX.ClearLink();

	m_bInitialized = false;
	m_bHideHUDInPopup = false;
	m_ePrePopupHUDRenderLevel = kHUDRenderFull;

	m_bSuppressNextFlip = false;

	m_bEnteredScreenState = false;
	m_bIntentionalDisconnect = false;

	m_InterfaceTimer.SetTeamId( INVALID_TEAM );
	m_BlueInterfaceTimer.SetTeamId( 0 );
	m_RedInterfaceTimer.SetTeamId( 1 );

	m_bSkipPreLoadScreen = false;
	m_ePostLoadScreenID	= SCREEN_ID_POSTLOAD;
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
        g_pLTClient->SoundMgr()->KillSound(m_hSplashSound);
	}

	if (m_hScubaSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hScubaSound);
	}

	if (m_hSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSound);
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
	if (!m_ScreenSpriteMgr.Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize ScreenSpriteMgr!");
		return LTFALSE;
	}

    if (!m_CursorMgr.Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Cursor!");
        return LTFALSE;
	}

    g_vtDrawInterface.Init(g_pLTClient, "DrawInterface", NULL, 1.0f);
    g_vtModelApplySun.Init(g_pLTClient, "ModelApplySun", NULL, 1.0f);


	g_vtLetterBox.Init(g_pLTClient, "LetterBox", NULL, 0.0f);
	g_vtLetterBoxDisabled.Init(g_pLTClient, "LetterBoxDisabled", NULL, 0.0f);
    g_vtLetterBoxFadeInTime.Init(g_pLTClient, "LetterBoxFadeInTime", NULL, 0.5f);
    g_vtLetterBoxFadeOutTime.Init(g_pLTClient, "LetterBoxFadeOutTime", NULL, 1.0f);
    g_vtDisableMovies.Init(g_pLTClient, "NoMovies", NULL, 0.0f);

    g_vtInterfaceFOVX.Init(g_pLTClient, "FovXInterface", NULL, 90.0f);
    g_vtInterfaceFOVY.Init(g_pLTClient, "FovYInterface", NULL, 75.0f);

	g_vtPauseTintAlpha.Init(g_pLTClient, "PauseTintAlpha", NULL, 0.65f);

	g_vtSplashScreenFadeIn.Init(g_pLTClient, "SplashScreenFadeInTime", NULL, 2.5f);
	g_vtSplashScreenFadeOut.Init(g_pLTClient, "SplashScreenFadeOutTime", NULL, 2.5f);
	g_vtSplashScreenTime.Init(g_pLTClient, "SplashScreenTime", NULL, 3.0f);
	g_vtMainScreenFadeIn.Init(g_pLTClient, "MainScreenFadeInTime", NULL, 3.0f);
	
	g_vtProgressBarScaleToSkills.Init( g_pLTClient, "ProgressBarScaleToSkills", LTNULL, 1.0f );
	
	g_vtExitLevelScreenFadeTime.Init( g_pLTClient, "ExitLevelScreenFadeTime", LTNULL, 1.0f );

	ProcessAdvancedOptions();

	// Create the Interface camera...
    uint32 dwWidth = 640;
    uint32 dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_ObjectType = OT_CAMERA;

    m_hInterfaceCamera = g_pLTClient->CreateObject(&theStruct);
	_ASSERT(m_hInterfaceCamera);

    g_pLTClient->SetCameraRect(m_hInterfaceCamera, LTFALSE, 0, 0, dwWidth, dwHeight);
    g_pLTClient->SetCameraFOV(m_hInterfaceCamera, DEG2RAD(g_vtInterfaceFOVX.GetFloat()), DEG2RAD(g_vtInterfaceFOVY.GetFloat()));

	// read in the settings
    m_Settings.Init (g_pLTClient, g_pGameClientShell);


    if (!g_pLayoutMgr || !g_pLayoutMgr->Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize LayoutMgr!");
        return LTFALSE;
	}
	hBackColor =  g_pLayoutMgr->GetBackColor();

    if (!m_InterfaceResMgr.Init())
	{
		// If we couldn't init, something critical must have happened (like no render dlls)

        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize InterfaceResMgr!");
        return LTFALSE;
	}

    if (!GetScreenMgr( )->Init())
	{
		// If we couldn't init, something critical must have happened
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize ScreenMgr!");
        return LTFALSE;
	}

    if (!m_MenuMgr.Init())
	{
		// If we couldn't init, something critical must have happened
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize MenuMgr!");
        return LTFALSE;
	}



    m_ClientInfo.Init();

	if (!GetPlayerStats( )->Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize Player Stats!");
        return LTFALSE;
	}
	if (!GetHUDMgr()->Init())
	{
        g_pLTClient->ShutdownWithMessage("ERROR in CInterfaceMgr::Init():  Could not initialize HUDMgr!");
        return LTFALSE;
	}

	g_pPaused->Init();

	m_PopupText.Init();
	m_WeaponChooser.Init();
	m_AmmoChooser.Init();


	m_CursorPos.x = 0;
	m_CursorPos.y = 0;

	g_fFovXTan = (float)tan(DEG2RAD(g_vtInterfaceFOVX.GetFloat())/2);
	g_fFovYTan = (float)tan(DEG2RAD(g_vtInterfaceFOVY.GetFloat())/2);

	m_MessageBox.Init();
	m_FullScreenTint.Init();

	if (!m_PerformanceMgr.Init()) 
	{
		g_pLTClient->ShutdownWithMessage("ERROR in CPerformanceMgr::Init():  Could not initialize PerformanceMgr!");
		m_PerformanceMgr.Term();
		return LTFALSE;
	}


	if (!m_ProfileMgr.Init()) 
	{
		g_pLTClient->ShutdownWithMessage("ERROR in CProfileMgr::Init():  Could not initialize ProfileMgr!");
		m_ProfileMgr.Term();
		return LTFALSE;
	}


	// Create the surface used for making letterboxed cinematics...
    LTRect rcSrc;
	rcSrc.Init(0, 0, 2, 2);
    HLTCOLOR hTransColor = g_pLTClient->SetupColor1(1.0f, 1.0f, 1.0f, LTTRUE);

	m_hLetterBoxSurface = g_pLTClient->CreateSurface(2, 2);

	g_pLTClient->SetSurfaceAlpha(m_hLetterBoxSurface, 0.0f);
    g_pLTClient->FillRect(m_hLetterBoxSurface, &rcSrc, kBlack);
    g_pLTClient->OptimizeSurface(m_hLetterBoxSurface, hTransColor);

	if (!m_InterfaceFXMgr.Init(g_pLTClient,LTFALSE)) 
	{
		g_pLTClient->ShutdownWithMessage("ERROR in CClientFXMgr::Init():  Could not initialize interface FX mgr!");
		m_InterfaceFXMgr.Term();
		return LTFALSE;
	}
	m_InterfaceFXMgr.SetCamera(m_hInterfaceCamera);

	m_szMouseFXName[0] = 0;
	m_szSelectFXName[0] = 0;

	// Consider ourselves initialized.
	m_bInitialized = true;

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
	// Turn off the loading screen if it's currently up
	if (m_LoadingScreen.IsActive())
		m_LoadingScreen.Hide();

	if (m_hGamePausedSurface)
	{
		g_pInterfaceResMgr->FreeSharedSurface(m_hGamePausedSurface);
		m_hGamePausedSurface = LTNULL;
	}

	if (m_hInterfaceCamera)
	{
        g_pLTClient->RemoveObject(m_hInterfaceCamera);
        m_hInterfaceCamera = LTNULL;
	}

	//make sure that the interface manager isn't still holding onto a reference
	m_InterfaceFXMgr.SetCamera(NULL);

	// ABM TODO term the subroutinemgr and ratingsmgr

	GetScreenMgr( )->Term();
	m_MenuMgr.Term();
	if (g_pLayoutMgr)
		g_pLayoutMgr->Term();
	GetPlayerStats( )->Term();
	GetHUDMgr()->Term();
	m_WeaponChooser.Term();
	m_AmmoChooser.Term();
	m_InterfaceResMgr.Term();

	m_MessageBox.Term();
	m_FullScreenTint.Term();
	m_PopupText.Term();
	m_Credits.Term();

	m_CursorMgr.Term();

	m_ScreenSpriteMgr.Term();

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
        g_pLTClient->SoundMgr()->KillSound(m_hScubaSound);
		m_hScubaSound = LTNULL;
	}

	if (m_hSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSound);
		m_hSound = LTNULL;
	}

	// Unititialized.
	m_bInitialized = false;
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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	GetPlayerStats( )->OnEnterWorld(bRestoringGame);

	// Update every HUD element so they display accurate info
	GetHUDMgr()->QueueUpdate( kHUDAll );

	GetMenuMgr()->EnableMenus();
//		GetMenu(MENU_ID_MISSION)->Enable(g_pGameClientShell->GetGameType() != eGameTypeDeathmatch);
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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	GetPlayerStats( )->OnExitWorld();
/*
	g_pMissionText->Clear();
	g_pChatInput->Show(LTFALSE, LTFALSE);
	g_pSubtitles->Clear();
*/
	m_Credits.Term();
	m_PopupText.Close();
	m_InterfaceTimer.SetTime(0.0f,false);
	m_BlueInterfaceTimer.SetTime(0.0f,false);
	m_RedInterfaceTimer.SetTime(0.0f,false);
	m_FullScreenTint.TurnOn(false);
	
	g_pDisplayMeter->SetValue( 0 );

	if (m_AmmoChooser.IsOpen())
	{
		m_AmmoChooser.Close();
	}
	if (m_WeaponChooser.IsOpen())
	{
		m_WeaponChooser.Close();
	}


	for (int i =0; i < NUM_OVERLAY_MASKS; i++)
	{
		if (m_hOverlays[i])
            g_pLTClient->RemoveObject(m_hOverlays[i]);

        m_hOverlays[i] = LTNULL;
	}

    m_bFadeInitialized = LTFALSE;


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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// Don't clear the screen if the loading screen's up
	if (m_LoadingScreen.IsVisible())
		return LTTRUE;

	if (m_bClearScreenAlways)
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER, 0);
	}
	else if (m_nClearScreenCount)
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER, 0);
		m_nClearScreenCount--;
	}
	else
	{
        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_RENDER, 0);
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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	if (m_eGameState != GS_LOADINGLEVEL)
	{
		if (!m_bSuppressNextFlip)
			g_pLTClient->FlipScreen(0);
		else
			m_bSuppressNextFlip = false;
	}

	m_CursorMgr.CheckForReinit();

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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// Update based on the game state...
    LTBOOL bHandled = LTFALSE;
	switch (m_eGameState)
	{
		case GS_PLAYING:
		{
			UpdatePlayingState();
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
			UpdatePopupState();
            bHandled = LTFALSE;  // Allow further processing
		}
		break;

		case GS_EXITINGLEVEL:
		{
			UpdateExitingLevelState();
            bHandled = LTFALSE;
		}
		break;

		case GS_LOADINGLEVEL:
		{
			UpdateLoadingLevelState();
            bHandled = LTTRUE;
		}
		break;

		case GS_SCREEN :
		{
			UpdateScreenState();
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


	}

	//in playing state, message box is drawn in InterfaceMgr::Draw(),
	// otherwise draw it here
	if (bHandled)
	{
		if (m_MessageBox.IsVisible())
		{
			g_pLTClient->Start3D();
			g_pLTClient->StartOptimized2D();
			m_MessageBox.Draw();
			g_pLTClient->EndOptimized2D();
			g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
		}
		m_CursorMgr.Update();
	}

	m_ScreenSpriteMgr.Update();

	return bHandled;
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
//	ROUTINE:	CInterfaceMgr::UpdateScreenState()
//
//	PURPOSE:	Update screen state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateScreenState()
{
	if (GetScreenMgr( )->GetCurrentScreenID() == SCREEN_ID_NONE)
	{
		SwitchToScreen(SCREEN_ID_MAIN);
	}
	
	// [KLS 9/2/02] FogEnabled is cleared in PreScreenState, however it is possible for
	// it to get set to 1 while in the screen state.  This will ensure that we never
	// render the screen state with fog enabled.  PostScreenState will return FogEnabled
	// to whatever it was before the screen state was entered.
	WriteConsoleInt("FogEnable", 0);


	if (!GetScreenMgr( )->UpdateInterfaceSFX())
		m_bSuppressNextFlip = true;
	else
	{
		g_pLTClient->Start3D();

		DrawSFX();

		g_pLTClient->StartOptimized2D();
		m_InterfaceResMgr.DrawScreen();
		UpdateScreenFade();
		g_pLTClient->EndOptimized2D();
		g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
	}
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
	GetPlayerStats()->Update();

	// Update auto chooser switching...
	m_WeaponChooser.Update();
	m_AmmoChooser.Update();

	// Update the player stats...
	GetHUDMgr()->Update();
	g_pPaused->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateMenuState()
//
//	PURPOSE:	Update menu state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateMenuState()
{
	if (!m_MenuMgr.GetCurrentMenu())
	{
		SwitchToMenu(MENU_ID_SYSTEM);
	}

	m_MenuMgr.Update();
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
		ChangeState(m_eLastGameState);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateExitingLevelState()
//
//	PURPOSE:	Update exiting level state
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateExitingLevelState()
{
	// Update the screen fade.
	UpdateScreenFade();

	// Check if screen is done fading out or is fading in at all.
	if( ScreenFadedOut( ) || FadingScreenIn( ))
	{
		if (g_pMissionMgr->IsExitingMission())
		{
			switch( g_pGameClientShell->GetGameType( ))
			{
				case eGameTypeCooperative:
					SwitchToScreen(SCREEN_ID_END_COOP_MISSION);
					break;
				case eGameTypeTeamDeathmatch:
				case eGameTypeDeathmatch:
				case eGameTypeDoomsDay:
					SwitchToScreen(SCREEN_ID_END_DM_MISSION);
					break;
				default:
					SwitchToScreen(SCREEN_ID_END_MISSION);
					break;
			}
		}
		else
		{
			switch( g_pGameClientShell->GetGameType( ))
			{
				case eGameTypeTeamDeathmatch:
				case eGameTypeDeathmatch:
				case eGameTypeDoomsDay:
				{
					SwitchToScreen(SCREEN_ID_END_DM_MISSION);
				}
				break;
				
				case eGameTypeCooperative:
				default:
				{
					CScreenPreload *pPreload = (CScreenPreload *) (GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
					if (pPreload)
					{
						pPreload->SetWaitingToExit(true);
						SwitchToScreen(SCREEN_ID_PRELOAD);

					}
					else
					{
						g_pMissionMgr->FinishExitLevel();
					}
				}
				break;
			}			
		}
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
	if (g_pPlayerMgr->IsPlayerInWorld())
	{
		// Make sure the loading screen isn't busy
		m_LoadingScreen.Pause();
		// Let the game client shell do stuff that's time consuming so we're not stuck on a black screen
		g_pGameClientShell->PostLevelLoadFirstUpdate();
		// Turn off the loading screen
		m_LoadingScreen.Hide();


		if (IsTeamGameType() && !g_pClientMultiplayerMgr->HasSelectedTeam())
		{
			SwitchToScreen(SCREEN_ID_PLAYER_TEAM);
		}
		else if (m_LoadingScreen.NeedsPostLoadScreen() && GetConsoleInt("LoadScreenWait",1) )
		{
			SwitchToScreen( m_ePostLoadScreenID );
		}
		else 
		{
			// Tell the client we're ready to play.
			g_pGameClientShell->SendClientLoadedMessage( );

			ChangeState(GS_PLAYING);
		}
	}
	else if ((m_bLoadFailed) || (g_pLTClient->IsConnected() && IsKeyDown(VK_ESCAPE)))
	{
		m_bLoadFailed = LTFALSE;
		m_LoadingScreen.Hide();

		// We joined using the command line, we need to visit the main screen before
		// any other screen, since we didn't go through the normal screen
		// progression.
		if( GetCommandLineJoin( ))
		{
			SwitchToScreen( SCREEN_ID_MAIN );
		}
		else
		{
			SwitchToScreen(m_eLoadFailedScreen);
		}

		MBCreate mb;
		if( m_nLoadFailedMsgId == ( uint32 )-1 )
			m_nLoadFailedMsgId = IDS_LOADGAMEFAILED;

        ShowMessageBox( m_nLoadFailedMsgId, &mb );
	}
	else if (!m_LoadingScreen.IsActive() && m_LoadingScreen.IsVisible())
	{
		// Update the loading screen if it's not active, but it is visible
		// so we can see the console on failure conditions.
		m_LoadingScreen.Update();
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
	g_pLTClient->FillRect(g_pLTClient->GetScreenSurface(),&rcFull,hBackColor);


	if (m_hGamePausedSurface)
	{
        g_pLTClient->ScaleSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hGamePausedSurface, &rcFull, LTNULL);
	}

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
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
    g_fSplashSndDuration -= g_pGameClientShell->GetFrameTime();

	if(m_hSplashSound)
	{
		if((g_fSplashSndDuration <= 0.0f) || g_pLTClient->IsDone(m_hSplashSound))
		{
			g_pLTClient->CPrint("Current Time: %.4f", g_pLTClient->GetTime());
			g_pLTClient->CPrint("Splash sound done playing...");

			g_pLTClient->SoundMgr()->KillSound(m_hSplashSound);
			m_hSplashSound = LTNULL;
		}
	}

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
    g_pLTClient->End3D(END3D_CANDRAWCONSOLE);

    static LTBOOL bDidFadeOut = LTFALSE;

	if (!m_bScreenFade)
	{
		if (!bDidFadeOut && !m_hSplashSound)
		{
			// [KLS 7/28/02] See if it is time to start the fade out...
			if (!g_SplashScreenTimer.On())
			{
				g_SplashScreenTimer.Start(g_vtSplashScreenTime.GetFloat());
			}
			else if (g_SplashScreenTimer.Stopped())
			{
				StartScreenFadeOut(g_vtSplashScreenFadeOut.GetFloat());
				bDidFadeOut = LTTRUE;
			}
		}
	}
	else if (bDidFadeOut && m_fCurFadeTime <= 0.0f)
	{
		EndSplashScreen( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::EndSplashScreen
//
//	PURPOSE:	Ends the splash screen.
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::EndSplashScreen()
{
	// If the user didn't start a join from the command line, proceed
	// like normal to the movies.
	if( !GetCommandLineJoin( ))
	{
		ChangeState(GS_MOVIE);
		return;
	}

	// If user is joining a mp game from the command line,
	// start the connection here.
	char szIP[256] = "";
	LTStrCpy( szIP, GetConsoleTempString( "join", "" ), ARRAY_LEN( szIP ));

	char szPassword[256];
	LTStrCpy( szPassword, GetConsoleTempString( "password", "" ), ARRAY_LEN( szPassword ));

	char szGameType[256] = "";
	LTStrCpy( szGameType, GetConsoleTempString( "gametype", "" ), ARRAY_LEN( szGameType ));

	if( szGameType[0] == 0 )
	{
		LTStrCpy( szGameType, GameTypeToString( eGameTypeDeathmatch ), ARRAY_LEN( szGameType ));
	}
	
	GameType eGameType = GameStringTypeToGameType( szGameType );
	g_pGameClientShell->SetGameType( eGameType );

	if( !szIP[0] || !g_pClientMultiplayerMgr->SetupClient( szIP, NULL, szPassword ) || !g_pMissionMgr->StartGameAsClient( ))
	{
		// drop them into the join menu
		LoadFailed( SCREEN_ID_MAIN );

		MBCreate mb;
		ShowMessageBox(IDS_CANT_CONNECT_TO_SERVER, &mb);
	}
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
		SwitchToScreen(SCREEN_ID_MAIN);
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
#ifdef _TRON_E3_DEMO
	s_fDemoTime = 0.0f;
#endif

	if (m_bSeenDemoScreens)
	{
		if (m_bQuitAfterDemoScreens)
		{
			g_pLTClient->Shutdown();
		}
		else
		{
			SwitchToScreen(SCREEN_ID_MAIN);
		}
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

	if (rcSrc.right > rcDst.right)
	{
	    g_pLTClient->ScaleSurfaceToSurface(hScreen, g_hDemo, &rcDst, &rcSrc);
	}
	else
	{
		int xDest = (rcDst.right - (int)nWidth) / 2;
		int yDest = (rcDst.bottom - (int)nHeight) / 2;
		g_pLTClient->DrawSurfaceToSurface(hScreen, g_hDemo, &rcSrc, xDest, yDest);
	}

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SetupMusic()
//
//	PURPOSE:	Handles changes to the state of the menu / level music
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::SetupMusic()
{
	CMusic* pMusic = g_pGameClientShell->GetMusic();
    if (!pMusic) return LTFALSE;

	// Stop the music if it isn't enabled...

	if (!m_Settings.MusicEnabled() || !GetConsoleInt("MusicActive",0))
	{
		pMusic->Stop();
		return LTTRUE;
	}



	// Use the current level's music if we have a level loaded...

	if ( g_pLTClient->IsConnected() )
	{
		// Re-initialize music if necessary...

		if (!pMusic->IsInitialized())
		{
			if (!pMusic->Init(g_pLTClient))
			{
				return LTFALSE;
			}
		}
 
		g_pGameClientShell->RestoreMusic();

		pMusic->Play();
		
		return LTTRUE;
	}


	// Handle using the menu music if necessary...this should only occur if we
	// aren't in a level...

	CMusicState* pMS = pMusic->GetMusicState();

	int nIntensity = g_pLayoutMgr->GetScreenMusicIntensity(GetScreenMgr( )->GetCurrentScreenID());

	if (pMusic->IsPlaying())
	{
		if (pMS->nIntensity != nIntensity)
		{
			pMusic->ChangeIntensity(nIntensity);
		}
		return LTTRUE;
	}

	// Setup the menu music...

	char szFile[128] = "";
	g_pClientButeMgr->GetInterfaceAttributeString("MenuMusicCtrlFile",szFile,sizeof(szFile));
    if (!strlen(szFile)) return LTFALSE;

	if (!pMusic->IsInitialized())
	{
        if (!pMusic->Init(g_pLTClient))
		{
            return LTFALSE;
		}
	}

	if (!pMusic->IsLevelInitialized())
	{
		char szDir[128] = "";
		g_pClientButeMgr->GetInterfaceAttributeString("MenuMusicDir",szDir,sizeof(szDir));

		if (!strlen(szDir))
		{
            return LTFALSE;
		}

		CMusicState MusicState;
		MusicState.nIntensity = nIntensity;
		strcpy(MusicState.szDirectory, szDir);
		strcpy(MusicState.szControlFile, szFile);

		if (!pMusic->RestoreMusicState(MusicState))
		{
			return LTFALSE;
		}
	}

	pMusic->Play();

	if (pMS->nIntensity != nIntensity)
	{
		pMusic->ChangeIntensity(nIntensity);
	}

	m_ProfileMgr.ImplementMusicVolume();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnMessage()
//
//	PURPOSE:	Handle interface messages
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
	{
		return LTFALSE;
	}

	switch(messageID)
	{
		case MID_PLAYER_INFOCHANGE :
		{
            uint8 nThing    = pMsg->Readuint8();
            uint8 nType1    = pMsg->Readuint8();
            uint8 nType2    = pMsg->Readuint8();
            LTFLOAT fAmount  = pMsg->Readfloat();

			UpdatePlayerStats(nThing, nType1, nType2, fAmount);

            return LTTRUE;
		}
		break;

		case MID_PLAYER_SKILLS :
		{
			GetPlayerStats( )->UpdateSkills(pMsg);
			if (m_eGameState == GS_MENU)
			{
				m_MenuMgr.GetCurrentMenu()->OnCommand(MC_UPDATE,0,0);
            }
			return LTTRUE;
		}
		break;

		case MID_PLAYER_SUMMARY :
		{
            uint32 nID = pMsg->Readuint32();
            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);


			if (!IsMultiplayerGame() || nLocalID == nID)
			{
				GetPlayerStats( )->UpdateMissionStats(pMsg);
			}
			else
			{
				CLIENT_INFO *pClient;

				if(pClient = m_ClientInfo.GetClientByID(nID))
					pClient->sStats.ReadData(pMsg);

			}

//			if (m_eGameState == GS_MENU)
//				m_MenuMgr.GetCurrentMenu()->OnCommand(MC_UPDATE,0,0);
            return LTTRUE;
		}
		break;

		case MID_PLAYER_SCORE :
		{
            uint32 nID = pMsg->Readuint32();
			CLIENT_INFO *pClient;

			if(pClient = m_ClientInfo.GetClientByID(nID))
			{
				pClient->sScore.ReadData(pMsg);
				m_ClientInfo.UpdateClientSort(pClient);
			}


            return LTTRUE;
		}
		break;


		case MID_GEAR_PICKEDUP :
		{
            uint8 nGearId = pMsg->Readuint8();
			bool bPickedUp = pMsg->Readbool();

			g_pGameClientShell->HandleGearPickup(nGearId, bPickedUp);

			if (bPickedUp)
			{
				GetPlayerStats( )->UpdateGear(nGearId);
			}
            return LTTRUE;
		}
		break;

		case MID_INTEL_PICKEDUP :
		{
			// [KLS 6/20/02] - Removed due to Craig request
			// g_pChatMsgs->AddMessage(LoadTempString(IDS_INTELLIGENCE));

			uint32 nTextId = pMsg->Readuint32();
			uint8 nPopupId = pMsg->Readuint8();
			LTBOOL bIsIntel = (LTBOOL)pMsg->Readuint8();
			LTBOOL bShow = (LTBOOL)pMsg->Readuint8();
			LTBOOL bAdd = (LTBOOL)pMsg->Readuint8();

			if (bShow)
			{
				ShowPopup(nTextId, nPopupId, true);
			}

			if (bAdd)
			{
				GetPlayerStats( )->UpdateIntel(nTextId,nPopupId,bIsIntel);
			}
		}
		break;


		case MID_DISPLAY_METER :
		{
            uint8 val = pMsg->Readuint8();
			g_pDisplayMeter->SetValue( val );
			g_pHUDMgr->QueueUpdate( kHUDDisplayMeter );
            return LTTRUE;
		}
		break;

		case MID_DECISION :
		{
			g_pDecision->Show(pMsg);
            return LTTRUE;
		}
		break;

		case MID_PLAYER_CONNECTED:
		{
			char szName[MAX_PLAYER_NAME];
            pMsg->ReadString( szName, ARRAY_LEN( szName ));
            uint32 nID = (uint32) pMsg->Readuint32();
			g_pLTClient->CPrint("MID_PLAYER_CONNECTED : Client  %d (%s)", nID, szName);
			m_ClientInfo.PlayerConnected( szName, nID );
            return LTTRUE;
		}
		break;

		case MID_PLAYER_TEAM :
		{
			
			g_pChatMsgs->AddMessage(LoadTempString(IDS_IWILLCHANGE));
            return LTTRUE;
		}
		break;


		case MID_PLAYER_INFO:
		{
			// Only do something if we're in multiplayer...

			if (!IsMultiplayerGame()) break;

			char szName[MAX_PLAYER_NAME];
            pMsg->ReadString( szName, ARRAY_LEN( szName ));
            uint32 nID = pMsg->Readuint32();
			HOBJECT	hObj = pMsg->ReadObject();
			ModelId eModelId = (ModelId)pMsg->Readuint8();
			uint8	nTeamId = pMsg->Readuint8();
			bool bIsAdmin = pMsg->Readbool( );
            uint8 nInfoType = pMsg->Readuint8();

            uint32 nLocalID = 0;
			g_pLTClient->GetLocalClientID (&nLocalID);


			bool bChangedTeams = false;
			uint8 nOldTeam = INVALID_TEAM;
			if (IsTeamGameType())
			{
				CLIENT_INFO* pOldCI = m_ClientInfo.GetClientByID(nID,false);
				if (pOldCI && pOldCI->nTeamID != nTeamId)
				{
					bChangedTeams = true;
					nOldTeam = pOldCI->nTeamID;
				}
			}

			switch (nInfoType)
			{
			case MID_PI_JOIN:
				{
					//new player entering
					g_pLTClient->CPrint("MID_PLAYER_INFO:MID_PI_JOIN - Adding client %d (%s)", nID, szName);
					m_ClientInfo.AddClient(szName, bIsAdmin, nID, nTeamId);
					if( nLocalID != nID )
						g_pGameClientShell->CSPrint(FormatTempString(IDS_JOINEDGAME, szName));		
				} break;
			case MID_PI_EXIST:
				{
					//we are joining and this is info about a player already in the game
					g_pLTClient->CPrint("MID_PLAYER_INFO:MID_PI_EXIST - Adding client %d (%s)", nID, szName);
					m_ClientInfo.AddClient(szName, bIsAdmin, nID, nTeamId);
				} break;
			case MID_PI_UPDATE:
				{
					//this is an update about a player already in the game
					g_pLTClient->CPrint("MID_PLAYER_INFO:MID_PI_UPDATE - Updating client %d (%s)", nID, szName);
					m_ClientInfo.UpdateClient(szName, bIsAdmin, nID, nTeamId);

					//if this is a team game, and this is our player's update, update our team info
					if (IsTeamGameType() )
					{
						if(nLocalID == nID)
						{
							g_pClientMultiplayerMgr->SelectTeam(nTeamId,false);
						}

						//if we changed teams, we have to clean up our radar
						if (bChangedTeams)
						{
							HandlePlayerTeamChange();
							
							if (nTeamId != INVALID_TEAM && nOldTeam != INVALID_TEAM)
							{
								char szTmp[128] = "";
								char szTeam[32] = "";
								static uint32 dwTeamID[] = { IDS_TEAM_1, IDS_TEAM_2, };
								LoadString( dwTeamID[nTeamId], szTeam, ARRAY_LEN(szTeam) );

								if(nLocalID == nID)
								{
									FormatString(IDS_ICHANGED, szTmp,sizeof(szTmp), szTeam);
								}
								else
								{
									FormatString(IDS_HECHANGED, szTmp,sizeof(szTmp), szName, szTeam);
								}

								g_pChatMsgs->AddMessage(szTmp);

							}

						}
					}

				} break;
			};

			CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	        if (psfxMgr)
			{
				CCharacterFX* pCFX = (CCharacterFX*)psfxMgr->FindSpecialFX(SFX_CHARACTER_ID, hObj);
				if (pCFX)
				{
					pCFX->ChangeModel(eModelId);
					
					if( pCFX->m_cs.bRadarVisible )
					{
						if (bChangedTeams)
							g_pRadar->RemovePlayer(pCFX->GetServerObj());
						g_pRadar->AddPlayer( pCFX->GetServerObj(), pCFX->m_cs.nClientID );
					}
				}
			}


            return LTTRUE;
		}
		break;

		case MID_TEAM_INFO:
		{
			CTeamMgr::Instance().UpdateTeam(pMsg);
		}
		break;

		case MID_PLAYER_REMOVED:
		{
			// Only do something if we're in multiplayer...

			if (!IsMultiplayerGame()) break;

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);

            uint32 nID = pMsg->Readuint32();

			//if it's us we know we're leaving
			if (nID == nLocalID)
				return LTTRUE;

			//if we don't know who it is, let's not display our ignorance...
			if (!m_ClientInfo.GetClientByID(nID,false))
				return LTTRUE;

            g_pGameClientShell->CSPrint(FormatTempString(IDS_LEFTGAME, m_ClientInfo.GetPlayerName(nID)));

			m_ClientInfo.RemoveClient(nID);

            return LTTRUE;
		}
		break;

		case MID_PINGTIMES:
		{
			while(1)
			{
                uint16 id, ping;
				CLIENT_INFO *pClient;

                id = pMsg->Readuint16();
				if(id == 0xFFFF)
					break;

                ping = pMsg->Readuint16();
				if(pClient = m_ClientInfo.GetClientByID(id,false))
					pClient->nPing = ping;
			}

            return LTTRUE;
		}
		break;

		case MID_PLAYER_SCORED:
		{
			//Only do something if we're in multiplayer...

			if (!IsMultiplayerGame()) break;

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID(&nLocalID);

			bool bFragged = pMsg->Readbool( );
            uint32 nVictim = pMsg->Readuint32();
            uint32 nScorer = pMsg->Readuint32();

			CLIENT_INFO *pVictim = m_ClientInfo.GetClientByID(nVictim);
			CLIENT_INFO *pScorer = m_ClientInfo.GetClientByID(nScorer);

            char szTmp[128] = "";
			if (nVictim == nLocalID)
			{
				// Scored againsts self.
				if (nVictim == nScorer)
				{
					if( bFragged )
						FormatString(IDS_KILLEDMYSELF,szTmp,sizeof(szTmp));
					else
	                    FormatString(IDS_TAGGEDMYSELF,szTmp,sizeof(szTmp));
				}
				// Scored by ai.
				else if( nScorer == -1 )
				{
					if( bFragged )
	                    FormatString(IDS_IWASKILLED,szTmp,sizeof(szTmp));
				}
				// Killed by player.
				else
				{
					if( bFragged )
	                    FormatString(IDS_HEKILLEDME, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nScorer));
					else
						FormatString(IDS_HETAGGEDME, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nScorer));
				}

				g_pChatMsgs->AddMessage(szTmp);
			}
			else if (nScorer == nLocalID)
			{
				if( bFragged )
	                FormatString(IDS_IKILLEDHIM, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nVictim));
				else
					FormatString(IDS_ITAGGEDHIM, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nVictim));

                g_pLTClient->CPrint(szTmp);
				g_pChatMsgs->AddMessage(szTmp);//,eType);
			}
			else
			{
				if (nVictim == nScorer)
				{
					if( bFragged )
		                FormatString(IDS_HEKILLEDHIMSELF, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName(nVictim));
					else
						FormatString(IDS_HETAGGEDHIMSELF, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName(nVictim));
				}
				else if( nScorer == -1 )
				{
					if( bFragged )
	                    FormatString(IDS_HEWASKILLED, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName(nVictim));
				}
				else
				{
					if( bFragged )
	                    FormatString(IDS_HEKILLEDHIM, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nScorer), m_ClientInfo.GetPlayerName (nVictim));
					else
						FormatString(IDS_HETAGGEDHIM, szTmp,sizeof(szTmp), m_ClientInfo.GetPlayerName (nScorer), m_ClientInfo.GetPlayerName (nVictim));
				}

                g_pLTClient->CPrint(szTmp);
				g_pChatMsgs->AddMessage(szTmp);
			}

            return LTTRUE;
		}
		break;


		case MID_PLAYER_MESSAGE :
		case MID_PLAYER_GHOSTMESSAGE :
		{
			// retrieve the string from the message, play the chat sound, and display the message

			char szMessage[256];
			pMsg->ReadString(szMessage, sizeof(szMessage));
			uint32 clientID = pMsg->Readuint32();
			uint8 nTeam =  pMsg->Readuint8();

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);


			if( IsTeamGameType() && nTeam != INVALID_TEAM )
			{
				CLIENT_INFO *pLocalCI = m_ClientInfo.GetLocalClient();
				if( !pLocalCI )
					return LTFALSE;

				if( pLocalCI->nTeamID != nTeam )
					return LTFALSE;
			}

            g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\chat.wav");
			g_pRadar->SetPlayerTalk(clientID);
			
			if (IsMultiplayerGame())
			{
				// The dedicated server will have a ClientID of -1...

				if( clientID == -1 )
				{
					g_pChatMsgs->AddMessage( szMessage, kMsgChat );
				}
				else
				{

					char szTemp[256];
					sprintf(szTemp,"%s : %s", m_ClientInfo.GetPlayerName(clientID), szMessage);

					if( IsTeamGameType() && nTeam != INVALID_TEAM )
					{
						g_pChatMsgs->AddMessage(szTemp,kMsgTeam);
					}
					else
					{
						g_pChatMsgs->AddMessage(szTemp,kMsgChat);
					}
				}
			}
			else
			{
				g_pChatMsgs->AddMessage(szMessage,kMsgChat);
			};

			
            return LTTRUE;
		}
		break;

		case MID_PLAYER_CREDITS :
		{
            uint8 nMsg = pMsg->Readuint8();
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
			uint32 nActivePlayerClientId = pMsg->Readuint32( );
            uint32 dwId = pMsg->Readuint32();
            uint32 nSound = pMsg->Readuint32();
			uint8 nTeam =  pMsg->Readuint8();

			if( IsTeamGameType() && nTeam != INVALID_TEAM )
			{
				CLIENT_INFO *pLocalCI = m_ClientInfo.GetLocalClient();
				if( !pLocalCI )
					return LTFALSE;

				if( pLocalCI->nTeamID != nTeam )
					return LTFALSE;
			}


			if (nSound)
			{
				char szStr[128] = "";
				g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", nSound, szStr, sizeof(szStr));

				g_pClientSoundMgr->PlaySoundLocal(szStr,SOUNDPRIORITY_PLAYER_HIGH);
			}
			else
			{
				g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\transmission.wav");
			}

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);

			// Check if there wasn't an active player for this transmission.
			if( nActivePlayerClientId == -1 || nActivePlayerClientId == nLocalID )
			{
				g_pTransmission->Show(dwId);
				g_pChatMsgs->AddMessage(dwId,kMsgTransmission);
			}
			// There was an active player with this transmission.  Format it like this:
			// "<PlayerName>: <Message>".
			else
			{
				CString sTransmission;
				sTransmission.Format( "%s: %s", m_ClientInfo.GetPlayerName( nActivePlayerClientId ),
					LoadTempString( dwId ));
				
				g_pTransmission->Show( sTransmission );
				g_pChatMsgs->AddMessage(sTransmission,kMsgTransmission);
				g_pRadar->SetPlayerTalk(nActivePlayerClientId);

			}

            return LTTRUE;
		}
		break;


		case MID_PLAYER_OVERLAY :
		{
			// retrieve the string from the message, play the chat sound, and display the message

            uint8 nId = pMsg->Readuint8();
            bool bOn = !!pMsg->Readuint8();

			if (nId < NUM_OVERLAY_MASKS)
			{
				if (bOn)
					CreateOverlay((eOverlayMask)nId);
				else
					RemoveOverlay((eOverlayMask)nId);
				return LTTRUE;
			}
			return LTFALSE;
		}
		break;

		case MID_LOAD_FAILED:
		{
			LoadFailed();
			return LTFALSE; 
		}
		break;
		
		case MID_DOOMSDAY_MESSAGE:
		{
			uint8 nId = pMsg->Readuint8();

			switch( nId )
			{
				case MID_DOOMSDAY_PIECE_PICKEDUP:
				case MID_DOOMSDAY_PIECE_DROPPED:
				case MID_DOOMSDAY_PIECE_PLACED:
				case MID_DOOMSDAY_PIECE_STOLEN:
				{
					// Static tables to make creating the transmission text easier...

					static uint32 dwActionID[] = { IDS_DOOMSDAY_PICKEDUP,
												   IDS_DOOMSDAY_DROPPED,
												   IDS_DOOMSDAY_PLACED,
												   IDS_DOOMSDAY_STOLE, };

					static uint32 dwPieceID[] = { IDS_DOOMSDAY_TRANSMITTER, IDS_DOOMSDAY_BATTERIES, IDS_DOOMSDAY_CORE, };

					static uint32 dwTeamID[] = { IDS_TEAM_1, IDS_TEAM_2, };
					
					static eChatMsgType eChatType[] = { kMsgBlueTeam, kMsgRedTeam };

					
					DDPieceType Piece = (DDPieceType)pMsg->Readuint8();
					uint8		nTeam = pMsg->Readuint8();

					if( Piece >= kDoomsDay_MAXTYPES )
						return LTFALSE;
					if( nTeam >= MAX_TEAMS )
						return LTFALSE;

					// Build the string to display...

					char szTransmission[256] = {0};
					char szTeam[32] = {0};
					char szPiece[32] = {0};

					LoadString( dwTeamID[nTeam], szTeam, ARRAY_LEN(szTeam) );

					LoadString( dwPieceID[Piece], szPiece, ARRAY_LEN(szPiece) );
					
					FormatString( dwActionID[nId], szTransmission, ARRAY_LEN( szTransmission ), szTeam, szPiece );

					g_pChatMsgs->AddMessage( szTransmission, eChatType[nTeam] );

					// Play a sound...

					g_pClientSoundMgr->PlayInterfaceSound( "Interface\\Snd\\chat.wav" );
				}
				break;

				case MID_DOOMSDAY_DEVICE_COMPLETED:
				{
					// Get the team name.
					uint8		nTeam = pMsg->Readuint8();

					if( nTeam >= MAX_TEAMS )
						return LTFALSE;

					char szTeam[32] = {0};
					static uint32 dwTeamID[] = { IDS_TEAM_1, IDS_TEAM_2, };
					LoadString( dwTeamID[nTeam], szTeam, ARRAY_LEN(szTeam) );

					// Build the string to display...
					char szTransmission[256] = {0};
                    FormatString( IDS_DOOMSDAY_DEVICE_COMPLETED, szTransmission, ARRAY_LEN( szTransmission ), szTeam );

					g_pTransmission->Show( szTransmission );
					g_pChatMsgs->AddMessage( szTransmission, kMsgTransmission );

					// Play a sound...
					g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\transmission.wav");
				}
				break;
				
				case MID_DOOMSDAY_PIECE_RESPAWNED:
				{
					static uint32 dwPieceID[] = { IDS_DOOMSDAY_TRANSMITTER, IDS_DOOMSDAY_BATTERIES, IDS_DOOMSDAY_CORE, };
					
					DDPieceType Piece = (DDPieceType)pMsg->Readuint8();

					if( Piece >= kDoomsDay_MAXTYPES )
						return LTFALSE;

					char szPiece[32] = {0};
					LoadString( dwPieceID[Piece], szPiece, ARRAY_LEN(szPiece) );

					// Build the string to display...
					
					char szTransmission[256] = {0};
                    FormatString( IDS_DOOMSDAY_RESPAWNED, szTransmission, ARRAY_LEN( szTransmission ), szPiece );

					g_pChatMsgs->AddMessage( szTransmission );
					
					// Play a sound...

					g_pClientSoundMgr->PlayInterfaceSound( "Interface\\Snd\\chat.wav" );
				}
				break;

				default:
				break;
			}
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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

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


				m_InterfaceResMgr.DrawMessage(IDS_REINITIALIZING_RENDERER);

				// because of driver bugs, we need to wait a frame after reinitializing the renderer and
				// reinitialize the cursor
				m_CursorMgr.ScheduleReinit(2.0f);
			}
		}
		break;

		// Client disconnected from server.  dwParam will
		// be a error flag found in de_codes.h.

		case LTEVENT_DISCONNECT :
		{
			if( g_pGameClientShell->ShouldUseRadar() )
			{
				g_pRadar->Reset();
			}

			// Clean up our clientinfo's if we're in mp.
			if( IsMultiplayerGame( ))
				m_ClientInfo.RemoveAllClients();

			if ( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			{
				Disconnected(dwParam);
			}
			m_bIntentionalDisconnect = false;
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
			SwitchToScreen(SCREEN_ID_MAIN);
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

// ABM TODO move this function out of base class into TronInterfaceMgr.
void CInterfaceMgr::UpdatePlayerStats(uint8 nThing, uint8 nType1,
                                      uint8 nType2, LTFLOAT fAmount)
{
	switch (nThing)
	{
		case IC_WEAPON_OBTAIN_ID :
		{
            GetPlayerStats( )->UpdateAmmo(nType1, nType2, (uint32)fAmount, LTTRUE, LTFALSE);
		}
		break;

		case IC_WEAPON_PICKUP_ID :
		{
			if (nType2 != WMGR_INVALID_ID)
			{
				GetPlayerStats( )->UpdateAmmo(nType1, nType2, (uint32)fAmount, LTTRUE);
			}
			else
			{
				g_pGameClientShell->HandleWeaponPickup(nType1,false);
			}
		}
		break;

		case IC_MOD_PICKUP_ID :
		{
			bool bPickedUp = (bool) !!(nType1);
			bool bDisplayMsg = (bool) !!(fAmount);

			if (bDisplayMsg)
			{
				g_pGameClientShell->HandleModPickup(nType2, bPickedUp);
			}

			if (bPickedUp)
			{
				GetPlayerStats( )->UpdateMod(nType2);
			}
		}
		break;

		case IC_DEFAULTWEAPON_ID :
		{
			CClientWeaponMgr* pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			pClientWeaponMgr->SetDefaultWeapon( nType1 );
		}
		break;

		case IC_AMMO_ID :
		{
            GetPlayerStats( )->UpdateAmmo(nType1, nType2, (uint32)fAmount);
		}
		break;

		case IC_MAX_HEALTH_ID :
		{
            GetPlayerStats( )->UpdateMaxHealth((uint32)fAmount);
		}
		break;

		case IC_MAX_ENERGY_ID :
		{
           GetPlayerStats( )->UpdateMaxEnergy((uint32)fAmount);
		}
		break;

		case IC_ENERGY_ID :
		{
           GetPlayerStats( )->UpdateEnergy((uint32)fAmount);
		}
		break;

		case IC_MAX_ARMOR_ID :
		{
            GetPlayerStats( )->UpdateMaxArmor((uint32)fAmount);
		}
		break;

		case IC_HEALTH_ID :
		{
            GetPlayerStats( )->UpdateHealth((uint32)fAmount);
		}
		break;

		case IC_ARMOR_ID :
		{
            GetPlayerStats( )->UpdateArmor((uint32)fAmount);
		}
		break;

		case IC_AIRLEVEL_ID :
		{
			GetPlayerStats( )->UpdateAir(fAmount);
		}
		break;

		case IC_OUTOFAMMO_ID :
		{
			g_pChatMsgs->AddMessage(FormatTempString(IDS_OUTOFAMMO, GetWeaponString(nType1)));
		}
		break;

		case IC_OBJECTIVE_ID :
		case IC_OPTION_ID :
		case IC_PARAMETER_ID :
		{
            GetPlayerStats( )->UpdateObjectives(nThing, nType1, (uint32)fAmount);
		}
		break;

		case IC_KEY_ID :
		{
            GetPlayerStats( )->UpdateKeys(nType1, (uint16)fAmount);
		}
		break;

		case IC_SKILLS_ID :
		{
            GetPlayerStats( )->GainSkills(nType1, nType2, (uint16)fAmount);
		}
		break;

		case IC_FADE_SCREEN_ID :
		{
			// Fade the screen in or out...

			if (nType1)
			{
				StartScreenFadeIn(fAmount);
			}
			else
			{
				StartScreenFadeOut(fAmount);
			}
		}
		break;

		case IC_RESET_INVENTORY_ID :
		{
			GetPlayerStats( )->DropInventory((LTBOOL)nType1);
		}
		break;


		case IC_MISSION_TEXT_ID :
		{
			if (fAmount > 0.0f)
				g_pMissionText->Start((int)fAmount);
			else
				g_pMissionText->Clear();
		}
		break;

		case IC_MISSION_FAILED_ID :
		{
			MissionFailed((int)fAmount);
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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;
     
	// Take appropriate action
	if (	g_pDecision->IsVisible()	&&
			command >= COMMAND_ID_CHOOSE_1 &&
			command <= COMMAND_ID_CHOOSE_6 )
	{
		uint8 nChoice = command - COMMAND_ID_CHOOSE_1;
		g_pDecision->Choose(nChoice);
        return LTTRUE;
	}


	if (GS_MENU == m_eGameState || GS_PLAYING == m_eGameState)
	{
		if (m_MenuMgr.OnCommandOn(command) || GS_MENU == m_eGameState)
			return LTTRUE;
	}


	switch (command)
	{

		case COMMAND_ID_HOLSTER :
		{
			CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			IClientWeaponBase const *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
			if (pClientWeapon && !pClientWeaponMgr->WeaponsEnabled()) return LTTRUE;

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

		case COMMAND_ID_LASTWEAPON :
		{
			CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			IClientWeaponBase const *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
			if (pClientWeapon && !pClientWeaponMgr->WeaponsEnabled()) return LTTRUE;

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
			CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			IClientWeaponBase const *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
			if (pClientWeapon && !pClientWeaponMgr->WeaponsEnabled()) return LTTRUE;

			//can't bring up chooser while disabling a gadget target
			if (g_pPlayerMgr->IsDisabling()) return LTTRUE;

			if (m_AmmoChooser.IsOpen())
			{
				m_AmmoChooser.Close();
			}
			if (m_WeaponChooser.Open(0))
			{
				m_WeaponChooser.PrevWeapon();
			}

            return LTTRUE;

		}
		break;

		case COMMAND_ID_NEXT_WEAPON	:
		{
			NextWeapon(command);
            return LTTRUE;
		} break;

		case COMMAND_ID_NEXT_WEAPON_1 :
		case COMMAND_ID_NEXT_WEAPON_2 :
		case COMMAND_ID_NEXT_WEAPON_3 :
		case COMMAND_ID_NEXT_WEAPON_4 :
		case COMMAND_ID_NEXT_WEAPON_5 :
		case COMMAND_ID_NEXT_WEAPON_6 :
		{
			if (!g_pDecision->IsVisible())
				NextWeapon(command);
            return LTTRUE;
		}
		break;

		case COMMAND_ID_NEXT_AMMO :
		{
			CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			IClientWeaponBase const *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();

			//can't bring up chooser while disabling a gadget target
			if (g_pPlayerMgr->IsDisabling()) return LTTRUE;

			if (pClientWeapon && !pClientWeaponMgr->WeaponsEnabled()) return LTTRUE;

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
			{
				GetHUDMgr()->NextLayout();
	            g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\HudLayoutToggle.wav");
			}
            return LTTRUE;

		}
		break;
		case COMMAND_ID_PREV_LAYOUT :
		{
			if (m_eGameState == GS_PLAYING)
			{
				GetHUDMgr()->PrevLayout();
	            g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\HudLayoutToggle.wav");
			}
            return LTTRUE;

		}
		break;

		case COMMAND_ID_MISSION:
		{
			if (m_eGameState == GS_PLAYING && !IsCoopMultiplayerGameType() )
			{
				g_pScores->Show(true);
				return LTTRUE;
			};
		} break;

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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// Only process if not editing a message...

    if (g_pChatInput->IsVisible()) return LTTRUE;

	if (m_MenuMgr.OnCommandOff(command))
		return LTTRUE;


	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		{
            return LTFALSE;
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if (IsChoosingWeapon())
			{
                uint8 nCurrWeapon = m_WeaponChooser.GetCurrentSelection();
				m_WeaponChooser.Close();
				g_pPlayerMgr->ChangeWeapon( nCurrWeapon );

                return LTTRUE;

			}
			else if (IsChoosingAmmo())
			{
                uint8 nCurrAmmo = m_AmmoChooser.GetCurrentSelection();
				m_AmmoChooser.Close();

				IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
				if ( pClientWeapon )
				{
					pClientWeapon->ChangeAmmoWithReload( nCurrAmmo );
				}

                return LTTRUE;
			}

		}
		break;

		case COMMAND_ID_HOLSTER :
		{
			ASSERT( 0 != g_pPlayerMgr );
			g_pPlayerMgr->ToggleHolster( true );
			return LTTRUE;
		}
		break;

		case COMMAND_ID_LASTWEAPON :
		{
			ASSERT( 0 != g_pPlayerMgr );
			g_pPlayerMgr->LastWeapon( );
			return LTTRUE;
		}
		break;

		case COMMAND_ID_NEXT_WEAPON :
		case COMMAND_ID_NEXT_WEAPON_1 :
		case COMMAND_ID_NEXT_WEAPON_2 :
		case COMMAND_ID_NEXT_WEAPON_3 :
		case COMMAND_ID_NEXT_WEAPON_4 :
		case COMMAND_ID_NEXT_WEAPON_5 :
		case COMMAND_ID_NEXT_WEAPON_6 :
		{
			// End the timer for quick weapon changing...
			m_WeaponChooser.EndAutoSwitch();
		}
		break;

		case COMMAND_ID_PREV_WEAPON :
		{
			// End the timer for quick weapon changing...
			m_WeaponChooser.EndAutoSwitch(false);
		}
		break;

		case COMMAND_ID_NEXT_AMMO :
		{
			// End the timer for quick ammo changing...
			m_AmmoChooser.EndAutoSwitch();
		}
		break;

		case COMMAND_ID_MISSION:
		{
			g_pScores->Show(false);
		} break;

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
	// Make sure we're initialized.
	if ( !IsInitialized( ) )
	{
		return LTFALSE;
	}

	if (m_MessageBox.IsVisible())
	{
		return m_MessageBox.HandleKeyDown(key,rep);
	}

	switch (m_eGameState)
	{
		case GS_SCREEN :
		{
			GetScreenMgr( )->HandleKeyDown(key,rep);
            return LTTRUE;
		}
		break;

		case GS_PAUSED :
		{
			if( HandleDebugKey(key) )
			{
				return LTTRUE;
			}

			// They pressed a key - unpause the game

			ChangeState(GS_PLAYING);
            return LTTRUE;
		}
		break;

		case GS_SPLASHSCREEN :
		{
			// They pressed a key - end splash screen...
 			EndSplashScreen( );
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


		case GS_POPUP :
		{
			if(m_PopupText.IsVisible())
			{
				KeystrokeList::iterator iter = g_keys.begin();
				while (iter != g_keys.end() && (*iter) != key)
					iter++;
				if (iter == g_keys.end())
					m_PopupText.Close();

				
			}
		}
		break;

		case GS_MENU :
		{
			m_MenuMgr.HandleKeyDown(key,rep);
			return LTTRUE;
		}
		break;

		// ABM TODO write handlers for the subroutine and ratings states

		case GS_PLAYING :
		{
			// Are We Broadcasting a Message
			if (g_pChatInput->IsVisible())
			{
				g_pChatInput->HandleKeyDown(key, rep);
				return LTTRUE;
			}

			switch (key)
			{
				case VK_PAUSE:
				{
					if (IsMultiplayerGame()) return LTFALSE;

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
					bool bHandled = false;
					if (!g_pPlayerMgr->IsPlayerDead() && g_pPlayerMgr->IsPlayerInWorld())
					{
						if (IsChoosingWeapon())
						{
							m_WeaponChooser.Close();
							bHandled = true;
						}
						if (IsChoosingAmmo())
						{
							m_AmmoChooser.Close();
							bHandled = true;
						}
					}

					if (g_pDecision->IsVisible())
					{
						g_pDecision->Choose(MAX_DECISION_CHOICES);
						bHandled = true;
					}

					if (bHandled)
					{
						return LTTRUE;
					}
				}
				break;

				default : break;
			}

			if ( m_MenuMgr.HandleKeyDown(key,rep) )
			{
				return LTTRUE;
			}

			if ( HandleDebugKey(key) )
			{
				return LTTRUE;
			}

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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// if it's the tilde (~) key then the console has been turned off
	// (this is the only event that causes this key to ever get processed)
	// so clear the back buffer to get rid of any part of the console still showing
	if (key == VK_TILDE)
	{
		AddToClearScreenCount();
        return LTTRUE;
	}

	if (g_pChatInput->IsVisible())
	{
        return LTTRUE;
	}

	switch (m_eGameState)
	{
	case GS_SCREEN:
		{
			GetScreenMgr( )->HandleKeyUp(key);
			return LTTRUE;
		}
		break;

	case GS_MENU:
		{
			m_MenuMgr.HandleKeyUp(key);
			return LTTRUE;
		}
		break;

	case GS_LOADINGLEVEL :
		{
            return LTTRUE;
		}
		break;
	case GS_POPUP:
		{
			KeystrokeList::iterator iter = g_keys.begin();
			while (iter != g_keys.end() && (*iter) != key)
				iter++;
			bool bKeepLocked = false;
			if (iter != g_keys.end() && (*iter) == key)
			{
				g_keys.erase(iter);
				bKeepLocked = !g_keys.empty();
			}
			g_bLockPopup &= bKeepLocked;
		} break;

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

void CInterfaceMgr::OnChar(unsigned char c)
{
	if (c < ' ') return;


	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.HandleChar(c);
		return;
	}

	if (g_pChatInput->IsVisible())
	{
		g_pChatInput->HandleChar(c);
		return;
	}
	if (m_eGameState == GS_SCREEN)
	{
		GetScreenMgr( )->HandleChar(c);
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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	// Tint the screen if necessary...

	m_FullScreenTint.SetAlpha(g_vtPauseTintAlpha.GetFloat());
	m_FullScreenTint.Draw(g_pLTClient->GetScreenSurface());

	// Check for interface drawing override...

	if (g_vtDrawInterface.GetFloat())
	{
		// Update letter box...

		UpdateLetterBox();

		// Find out if we're in multiplayer...

		PlayerState ePlayerState = g_pPlayerMgr->GetPlayerState();

		if (GetGameState() == GS_PLAYING || GetGameState() == GS_POPUP)
		{
			// Draw the player stats (health,armor,ammo) if appropriate...
			GetHUDMgr()->Render();

			if (m_Credits.IsInited())
			{
				if (m_Credits.IsDone())
				{
					m_Credits.Term();
				}
				else
				{
					m_Credits.Update();
				}
			}

			if (m_InterfaceTimer.GetTime() > 0.0f)
			{
				m_InterfaceTimer.Draw();
			}

			if( IsTeamGameType( ))
			{
				if (m_BlueInterfaceTimer.GetTime() > 0.0f)
				{
					m_BlueInterfaceTimer.Draw();
				}

				if (m_RedInterfaceTimer.GetTime() > 0.0f)
				{
					m_RedInterfaceTimer.Draw();
				}
			}
		}

		if( g_pDamageFXMgr->IsDamageActive( DamageTypeToFlag(DT_SLEEPING) ))
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


		if (!IsMultiplayerGame())
		{
			if (GetGameState() == GS_PLAYING || GetGameState() == GS_POPUP || GetGameState() == GS_MENU)
			{
				g_pPaused->Render();
			}
		}


		// Update the screen fade alpha only if we are in the playing state and not paused...

		bool bUpdateAlpha = (GetGameState() == GS_PLAYING ? !g_pGameClientShell->IsGamePaused() : false);

		UpdateScreenFade(bUpdateAlpha);
	}

	if (GetGameState() == GS_MENU)
	{
		m_MenuMgr.Render();
	}
	if (GetGameState() == GS_POPUP)
	{
		m_PopupText.Draw();
	}

	//this should be last so it is always on top.
	if (m_MessageBox.IsVisible())
	{
		g_pLTClient->Start3D();
        g_pLTClient->StartOptimized2D();
        m_MessageBox.Draw();
        g_pLTClient->EndOptimized2D();
        g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
	}

	m_CursorMgr.Update();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreScreenState()
//
//	PURPOSE:	Initialize the Screen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreScreenState(GameState eCurState)
{
    if (eCurState == GS_SCREEN) return LTFALSE;

	m_InterfaceResMgr.Setup();

	// Pause the game...
    g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);

	// Disable light scaling when not in the playing state...
	g_pGameClientShell->GetLightScaleMgr()->Disable();
	g_pGameClientShell->GetScreenTintMgr()->ClearAll();


	CreateInterfaceBackground();

    SetHUDRenderLevel(kHUDRenderNone);
	ClearScreenAlways();

	// No fog in the menus...
	g_bInGameFogEnabled = (LTBOOL) GetConsoleInt("FogEnable", 1);
	WriteConsoleInt("FogEnable", 0);

	// Make sure menus and screens are full screen...
    memset(&m_rcMenuRestoreCamera, 0, sizeof (LTRect));

    uint32 nWidth, nHeight;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &nWidth, &nHeight);

	m_bUseInterfaceCamera = LTTRUE;

	if (eCurState == GS_LOADINGLEVEL) 
	{
		AbortScreenFade();
	}
	else
	{
		ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
	}

	m_bEnteredScreenState = true;
	
	// Initialize the music...

	SetupMusic();

    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostScreenState()
//
//	PURPOSE:	Handle leaving the Screen state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostScreenState(GameState eNewState)
{
    if (eNewState == GS_SCREEN) return LTFALSE;

	GetScreenMgr( )->ExitScreens();

	if (eNewState != GS_LOADINGLEVEL && eNewState != GS_DEMOSCREEN)
	{
		int nGameMode = GAMEMODE_NONE;
        g_pLTClient->GetGameMode(&nGameMode);
        if (nGameMode == GAMEMODE_NONE) return LTFALSE;
	}

    ClearScreenAlways(LTFALSE);
	AddToClearScreenCount();


	RemoveAllInterfaceSFX();

	//see what state we need to restore the HUD into (in case we go into the menus
	//while we are dead)
	if(g_pPlayerMgr->IsPlayerDead())
	{
		SetHUDRenderLevel(kHUDRenderDead);
	}
	else
	{
		SetHUDRenderLevel(kHUDRenderFull);
	}

	// m_InterfaceResMgr.Clean();

    g_pLTClient->ClearInput();


	// Reset fog value...

	WriteConsoleInt("FogEnable", (int) g_bInGameFogEnabled);

	m_bSuppressNextFlip = true;

	// Match the pause from PreScreeState...

    g_pGameClientShell->PauseGame( LTFALSE );

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

	if (eCurState != GS_MENU)
	{
		// Unpause the game...

        g_pGameClientShell->PauseGame(LTFALSE);
	}

	// Clear whatever input got us here...
	g_pLTClient->ClearInput();

	// Eanble light scaling...

	g_pGameClientShell->GetLightScaleMgr()->Enable();
	g_pPlayerMgr->RestorePlayerModes();

	g_pMissionText->Pause(LTFALSE);

    m_bUseInterfaceCamera = LTFALSE;

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

	g_pGameClientShell->GetScreenTintMgr()->ClearAll();
	g_pGameClientShell->GetLightScaleMgr()->ClearAllLightScales();

    g_pLTClient->SetModelHook((ModelHookFn)DefaultModelHook, this);

	g_pMissionText->Pause(LTTRUE);

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

	if (!m_MenuMgr.GetCurrentMenu())
	{
		if (!m_MenuMgr.GetLastMenu())
			return LTFALSE;

		m_MenuMgr.SetCurrentMenu(m_MenuMgr.GetLastMenu()->GetMenuID());
	}

	m_bUseInterfaceCamera = LTFALSE;

	// [KLS 7/28/02] Shouldn't need to fade in for the menu, screen fade
	// is drawn behind the menu...
	// ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

	// Pause the game...
    g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);
	g_pLTClient->SetInputState(LTTRUE);

	// If we're currently viewing a cinematic disable all the menu bar menus
	// except the system menu...

	if (g_pPlayerMgr->IsUsingExternalCamera())
	{
		m_MenuMgr.EnableMenuBar(false, (MB_ALL & ~MB_SYSTEM));
	}

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

	m_MenuMgr.ExitMenus();

	// Unpause the game...
    g_pGameClientShell->PauseGame(LTFALSE);
	g_pLTClient->ClearInput();

	// If we're currently viewing a cinematic re-enable the menu bar...

	if (g_pPlayerMgr->IsUsingExternalCamera())
	{
		m_MenuMgr.EnableMenuBar(true);
	}

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
	if ( !IsMultiplayerGame( ) )
	{
		g_pGameClientShell->PauseGame(LTTRUE, LTTRUE);
	}

	g_pPlayerMgr->AllowPlayerMovement(LTFALSE);
	g_pPlayerMgr->ClearPlayerModes(LTTRUE);

	m_bUseInterfaceCamera = LTFALSE;

	m_ePrePopupHUDRenderLevel = GetHUDMgr()->GetRenderLevel();

	if (m_bHideHUDInPopup)
	{	
		SetHUDRenderLevel(kHUDRenderNone);
	}

	g_keys.clear();
	for (int k = 0; k < 256; k++)
	{
		if (IsKeyDown(k))
			g_keys.push_back(k);
	}

	g_bLockPopup = !g_keys.empty();

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

	g_pPlayerMgr->AllowPlayerMovement(LTTRUE);
	g_pLTClient->ClearInput();

	SetHUDRenderLevel(m_ePrePopupHUDRenderLevel);
	m_bHideHUDInPopup = false;

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

	RemoveInterfaceFX();
	m_InterfaceFXMgr.UseSystemTime(true);

	m_bUseInterfaceCamera = LTTRUE;

	m_LoadingScreen.Show();
		
	// Turn off the music (this will be turned on when we start the
	// next level...
	//CMusic* pMusic = g_pGameClientShell->GetMusic();
    //if (pMusic)
	//{
	//	pMusic->Stop();
	//}

	// Turn off sound
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetVolume(0);

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

	RemoveAllInterfaceSFX();
	m_InterfaceFXMgr.UseSystemTime(false);

	// Turn back on the sound
	m_ProfileMgr.ImplementSoundVolume();


//	ClearAllScreenBuffers();
	m_bLoadFailed = LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreExitingLevelState()
//
//	PURPOSE:	Initialize the ExitingLevel state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PreExitingLevelState(GameState eCurState)
{
    if (eCurState == GS_EXITINGLEVEL) return LTFALSE;

	if (eCurState == GS_SCREEN && GetScreenMgr( )->GetCurrentScreenID() == SCREEN_ID_MAIN )
	{
		return LTFALSE;
	}

	StartScreenFadeOut( g_vtExitLevelScreenFadeTime.GetFloat() );

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostExitingLevelState()
//
//	PURPOSE:	Handle leaving the ExitingLevel state
//
// ----------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::PostExitingLevelState(GameState eNewState)
{
    if (eNewState == GS_EXITINGLEVEL) return LTFALSE;

	// Tell the missionmgr we're leaving exit level.
//	g_pMissionMgr->FinishExitLevel( );

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

    uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
	m_hSplashSound = g_pClientSoundMgr->PlayInterfaceSound(IM_SPLASH_SOUND);

	if (m_hSplashSound)
	{
		g_pLTClient->SoundMgr()->GetSoundDuration(m_hSplashSound, g_fSplashSndDuration);
		g_pLTClient->CPrint("Splash sound duration: %.4f", g_fSplashSndDuration);
		g_pLTClient->CPrint("Current Time: %.4f", g_pLTClient->GetTime());
	}

	// Create the splash screen...
    g_hSplash = g_pLTClient->CreateSurfaceFromBitmap(IM_SPLASH_SCREEN);
    if (!g_hSplash) return LTFALSE;

    g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER, 0 );

	// Fade into the splash screen...

	StartScreenFadeIn(g_vtSplashScreenFadeIn.GetFloat());
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
        g_pLTClient->SoundMgr()->KillSound(m_hSplashSound);
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

	g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN, 0);
	m_bUseInterfaceCamera = LTTRUE;

	m_nCurMovie = gMovieOrder[0];
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

    hVar = g_pLTClient->GetConsoleVar("DisableSoundFilters");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_SOUNDFILTERS;
	}

    hVar = g_pLTClient->GetConsoleVar("DisableHardwareSound");
    if (hVar && g_pLTClient->GetVarValueFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_HARDWARESOUND;
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
	if ( !(m_dwAdvancedOptions & AO_SOUNDFILTERS) )
	{
        g_pLTClient->RunConsoleString("SoundFilters 0");
	}
	else
	{
		// only set this on, if there isn't a current setting
		hVar = g_pLTClient->GetConsoleVar("SoundFilters");
		if ( !hVar )
	        g_pLTClient->RunConsoleString("SoundFilters 1");
	}

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
        g_pLTClient->RunConsoleString("BackBufferCount 1");
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
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return LTFALSE;

	DebugChangeState(eNewState);

	GameState eCurState = m_eGameState;

	// First make sure we change change to the new state from the the
	// state we are currently in...

	if (PreChangeState(eCurState, eNewState))
	{
		m_eGameState = eNewState;

		// Since the state changed, update the cursor
		UpdateCursorState();

//	    g_pLTClient->ClearInput();
		m_eLastGameState = eCurState;

        return LTTRUE;

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

		case GS_EXITINGLEVEL:
		{
            if (!PostExitingLevelState(eNewState)) return LTFALSE;
		}
		break;

		case GS_LOADINGLEVEL:
		{
            if (!PostLoadingLevelState(eNewState)) return LTFALSE;
		}
		break;

		case GS_SCREEN :
		{
            if (!PostScreenState(eNewState)) return LTFALSE;
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

		case GS_EXITINGLEVEL:
		{
			return PreExitingLevelState(eCurState);
		}
		break;

		case GS_LOADINGLEVEL:
		{
			return PreLoadingLevelState(eCurState);
		}
		break;

		case GS_SCREEN :
		{
			return PreScreenState(eCurState);
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

void CInterfaceMgr::Save(ILTMessage_Write *pMsg)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	GetPlayerStats( )->Save(pMsg);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Load
//
//	PURPOSE:	Load the interface info
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::Load(ILTMessage_Read *pMsg)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	GetPlayerStats( )->Load(pMsg);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetCurrentScreen()
//
//	PURPOSE:	Finds out what the current screen is
//				- returns SCREEN_ID_NONE if not in a screen state
//
// --------------------------------------------------------------------------- //

eScreenID CInterfaceMgr::GetCurrentScreen()
{
	if (m_eGameState != GS_SCREEN)
	{
		return SCREEN_ID_NONE;
	}
	return GetScreenMgr( )->GetCurrentScreenID();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SwitchToScreen
//
//	PURPOSE:	Go to the specified screen
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::SwitchToScreen(eScreenID screenID)
{
	if (m_eGameState != GS_SCREEN)
	{
		if (((m_eGameState == GS_SPLASHSCREEN) || (m_eGameState == GS_DEMOSCREEN)) && (screenID == SCREEN_ID_MAIN))
		{
			StartScreenFadeIn(g_vtMainScreenFadeIn.GetFloat());
		}

        if (!ChangeState(GS_SCREEN)) return LTFALSE;
	}

	GetScreenMgr( )->SetCurrentScreen(screenID);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::SwitchToMenu
//
//	PURPOSE:	Go to the specified menu
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::SwitchToMenu(eMenuID menuID)
{
	if (!m_MenuMgr.SetCurrentMenu(menuID)) return LTFALSE;

	if (m_eGameState != GS_MENU)
	{	
        if (!ChangeState(GS_MENU)) return LTFALSE;
		m_MenuMgr.SlideIn();
	}
	

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ForceScreenUpdate
//
//	PURPOSE:	Force the current screen to update
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::ForceScreenUpdate(eScreenID screenID)
{
    if (m_eGameState != GS_SCREEN) return LTFALSE;

	return GetScreenMgr( )->ForceScreenUpdate(screenID);
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
	// The player is no longer in the world...
	g_pGameClientShell->SetWorldNotLoaded();

	m_nFailStringId = nFailStringId;
	SwitchToScreen(SCREEN_ID_FAILURE);
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
	GetScreenMgr( )->ScreenDimsChanged();
	m_MenuMgr.ScreenDimsChanged();
	GetHUDMgr()->ScreenDimsChanged();

	// Update the camera rect...
    uint32 dwWidth = 640, dwHeight = 480;

    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	// This may need to be changed to support in-game cinematics...

	ResetMenuRestoreCamera(0, 0, dwWidth, dwHeight);
    g_pLTClient->SetCameraRect (m_hInterfaceCamera, LTTRUE, 0, 0, dwWidth, dwHeight);

	UpdateInterfaceBackground();

}



//mouse handling
void CInterfaceMgr::OnLButtonUp(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnLButtonUp(x,y);
		return;
	}

	switch (m_eGameState)
	{
	case GS_LOADINGLEVEL:
		{
		} break;
	case GS_SCREEN:
		{
			GetScreenMgr( )->OnLButtonUp(x,y);
		} break;
	case GS_MENU:
		{
			m_MenuMgr.OnLButtonUp(x,y);
		} break;
	case GS_POPUP:
		{
			if(m_PopupText.IsVisible() && !g_bLockPopup)
			{
				m_PopupText.Close();
			}
			g_bLockPopup = false;
		} break;
	}
}

void CInterfaceMgr::OnLButtonDown(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnLButtonDown(x,y);
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnLButtonDown(x,y);
		}	
		break;

		case GS_MENU:
		{
			m_MenuMgr.OnLButtonDown(x,y);
		} 
		break;

		case GS_SPLASHSCREEN:
		{
			// They pressed a mouse button - end splash screen...
			EndSplashScreen( );
		}	
		break;

		case GS_MOVIE:
		{
			// They pressed a mouse button - next movie
			NextMovie();
		}	
		break;

		case GS_DEMOSCREEN:
		{
			// They pressed a mouse button - next demo screen
			NextDemoScreen();
		}	
		break;

	}
}


void CInterfaceMgr::OnLButtonDblClick(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ) )
		return;

	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnLButtonDblClick(x,y);
		} 
		break;

		case GS_MENU:
		{
			m_MenuMgr.OnLButtonDblClick(x,y);
		} 
		break;
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
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonUp(x,y);
		} 
		break;
	
		case GS_MENU:
		{
			m_MenuMgr.OnRButtonUp(x,y);
		} break;

		case GS_POPUP:
		{
			if(m_PopupText.IsVisible() && !g_bLockPopup)
			{
				m_PopupText.Close();
			}
			g_bLockPopup = false;
		} break;
	}


}

void CInterfaceMgr::OnRButtonDown(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonDown(x,y);
		}	
		break;
		
		case GS_MENU:
		{
			m_MenuMgr.OnRButtonDown(x,y);
		} 
		break;
		
		case GS_SPLASHSCREEN:
		{
			// They pressed a button - end splash screen...
			EndSplashScreen( );
		}	
		break;
	
		case GS_MOVIE:
		{
			// They pressed a button - next movie...
			NextMovie();
		}	
		break;
		
		case GS_DEMOSCREEN:
		{
			// They pressed a button - next demo screen...
			NextDemoScreen();
		}	
		break;
	
	}
}

void CInterfaceMgr::OnRButtonDblClick(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		return;
	}
	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonDblClick(x,y);
		} 
		break;
		
		case GS_MENU:
		{
			m_MenuMgr.OnRButtonDblClick(x,y);
		} 
		break;
	}

}

void CInterfaceMgr::OnMouseMove(int x, int y)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	m_CursorPos.x = x;
	m_CursorPos.y = y;

	if (GetConsoleInt("ShowMousePos",0) > 0)
	{
		float fx = (float)x / g_pInterfaceResMgr->GetXRatio();
		float fy = (float)y / g_pInterfaceResMgr->GetYRatio();
		g_pLTClient->CPrint("MousePos (%0.0f,%0.0f)",fx,fy);
	}

	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.OnMouseMove(x,y);
		return;
	}

	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnMouseMove(x,y);
		}
		break;
		
		case GS_MENU:
		{
			m_MenuMgr.OnMouseMove(x,y);
		} 
		break;
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
	if (!m_nOverlayCount) return;


    LTVector vPos(0, 0, 0), vU, vR, vF, vTemp;
    LTRotation rRot;

	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	if (!hCamera) return;

    LTBOOL bDrawnOne = LTFALSE;
	for (int i = 0; i < NUM_OVERLAY_MASKS; i++)
	{
		if (m_hOverlays[i])
		{
			if ((!bDrawnOne || i >= OVM_NON_EXCLUSIVE) && GS_PLAYING == GetGameState())
			{
				if (i < OVM_NON_EXCLUSIVE)
				{
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

				g_pLTClient->SetObjectScale(m_hOverlays[i], &vTemp);

				g_pCommonLT->SetObjectFlags(m_hOverlays[i], OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

			}
			else
			{
				g_pCommonLT->SetObjectFlags(m_hOverlays[i], OFT_Flags, 0, FLAG_VISIBLE);
			}
		}
	}
}

void CInterfaceMgr::CreateOverlay(eOverlayMask eMask)
{
	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	if (!hCamera) return;

	// Already created this mask
	if (m_hOverlays[eMask]) return;


	m_fOverlayScaleMult[eMask] = g_pLayoutMgr->GetMaskScale(eMask);

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_SPRITE;

    LTVector vPos(0,0,0), vU, vR, vF, vTemp;
    LTRotation rRot;

//    g_pLTClient->GetObjectPos(hCamera, &vPos);
//    g_pLTClient->GetObjectRotation(hCamera, &rRot);
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	if (g_pLayoutMgr->IsMaskSprite(eMask))
	{

		VEC_MULSCALAR(vTemp, vF, g_fOverlaySpriteDist);
		VEC_ADD(vPos, vPos, vTemp);
		createStruct.m_Flags = FLAG_VISIBLE | FLAG_SPRITE_NOZ | FLAG_FOGDISABLE | FLAG_NOLIGHT | FLAG_REALLYCLOSE;
		VEC_COPY(createStruct.m_Pos, vPos);
		createStruct.m_Rotation = rRot;


		char sprName[128] = "";
		g_pLayoutMgr->GetMaskSprite(eMask,sprName,sizeof(sprName));

		if (sprName[0])
		{
			SAFE_STRCPY(createStruct.m_Filename, sprName);
			m_hOverlays[eMask] = g_pLTClient->CreateObject(&createStruct);
			m_nOverlayCount++;

			// [KLS 5/15/02] Change the alpha value of the sprite...
			// [JRG 9/1/02] Added alpha values to layout butes
			LTFLOAT r, g, b, a;
			g_pLTClient->GetObjectColor(m_hOverlays[eMask], &r, &g, &b, &a);

			a = g_pLayoutMgr->GetMaskAlpha(eMask);
			
			g_pLTClient->SetObjectColor(m_hOverlays[eMask], r, g, b, a);
		}
	}
	else
	{
		VEC_MULSCALAR(vTemp, vF, g_fOverlayModelDist);
		VEC_ADD(vPos, vPos, vTemp);
		createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
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
        g_pLTClient->RemoveObject(m_hOverlays[eMask]);
        m_hOverlays[eMask] = LTNULL;
	}
}



void CInterfaceMgr::BeginScope(LTBOOL bVisionMode, LTBOOL bCamera)
{
	if (bVisionMode)
	{
		CreateOverlay(OVM_STATIC);
	}

	CreateOverlay(OVM_SCOPE);

	if (bCamera)
	{
		CreateOverlay(OVM_CAMERA);
	}
}

void CInterfaceMgr::EndScope()
{
	RemoveOverlay(OVM_SCOPE);
	RemoveOverlay(OVM_STATIC);
	RemoveOverlay(OVM_CAMERA);
	RemoveOverlay(OVM_CAMERA_TARGET);

}

void CInterfaceMgr::BeginZoom(LTBOOL bIn)
{
	CreateOverlay(bIn ? OVM_ZOOM_IN : OVM_ZOOM_OUT);
}

void CInterfaceMgr::EndZoom()
{
	RemoveOverlay(OVM_ZOOM_IN);
	RemoveOverlay(OVM_ZOOM_OUT);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateScreenFade()
//
//	PURPOSE:	Update the screen fade
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::UpdateScreenFade(bool bUpdateAlpha /*=true*/)
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

		// The alpha value is based on the fade time, only update it if
		// we want the alpha value to change...

		if (bUpdateAlpha)
		{
			m_fCurFadeTime -= g_pGameClientShell->GetFrameTime();
		}
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

	        g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN, 0);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AbortScreenFade()
//
//	PURPOSE:	Cancel the screen fade
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::AbortScreenFade()
{
	if (m_hFadeSurface)
	{
        g_pLTClient->DeleteSurface(m_hFadeSurface);
        m_hFadeSurface = LTNULL;
	}

	m_bFadeInitialized  = LTFALSE;
	m_bScreenFade       = LTFALSE;
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
	LTBOOL bOn = (g_vtLetterBoxDisabled.GetFloat() ? LTFALSE : (m_bLetterBox || g_vtLetterBox.GetFloat()));

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
/*
	if (m_nBorderSize == 0)
	{
	    uint32 dwHeight = 448;
		m_nBorderSize = (int) (float(dwHeight) * fBorderPercent);
		m_LetterBox[0].verts[2].y = m_LetterBox[0].verts[3].y = m_nBorderSize;
		m_LetterBox[1].verts[0].y = m_LetterBox[1].verts[1].y = dwHeight - m_nBorderSize;
	}
	// Set the alpha
	m_LetterBox[0].rgba.a = m_LetterBox[1].rgba.a = 255.0 * m_fLetterBoxAlpha;

	g_pLTDrawprim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pLTDrawprim->SetColorOp(DRAWPRIM_MODULATE);
	g_pLTDrawprim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
	g_pLTDrawprim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pLTDrawprim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pLTDrawprim->SetClipMode(DRAWPRIM_FASTCLIP);
	g_pLTDrawprim->SetFillMode(DRAWPRIM_FILL);
		
	// draw the two polys
	g_pLTDrawprim->DrawPrim(m_LetterBox, 2);

*/
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
		case GS_MENU :
		case GS_POPUP :
            return LTFALSE;
		break;

		case GS_PLAYING:
			return  !g_pChatInput->IsVisible();
		break;

		default :
		break;
	}

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AllowCameraRotation
//
//	PURPOSE:	Can the camera rotate (while in this state)
//
// --------------------------------------------------------------------------- //

LTBOOL CInterfaceMgr::AllowCameraRotation()
{
	switch( m_eGameState )
	{
		case GS_MENU:
		case GS_POPUP:
			return LTFALSE;
		break;

		default:
		break;
	}

	return LTTRUE;
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
	char sprName[128] = "";

	BSCREATESTRUCT bcs;

    LTVector vTemp, vScale;
	LTVector vPos, vU, vR, vF;
    LTRotation rRot;

	g_pLTClient->GetObjectPos(m_hInterfaceCamera, &vPos);
	g_pLTClient->GetObjectRotation(m_hInterfaceCamera, &rRot);
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	if( !g_pLayoutMgr )
	{
		UBER_ASSERT( 0, "CInterfaceMgr::CreateInterfaceBackground: No LayoutMgr" );
		return;
	}

	vScale = g_vBaseBackScale * g_pLayoutMgr->GetBackSpriteScale();


	VEC_MULSCALAR(vTemp, vF, g_fBackDist);
//	VEC_MULSCALAR(vTemp, vTemp, g_pInterfaceResMgr->GetXRatio());
	VEC_ADD(vPos, vPos, vTemp);

	VEC_COPY(bcs.vPos, vPos);
    bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);

	VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
	bcs.bUseUserColors = LTTRUE;


	g_pLayoutMgr->GetBackSprite(sprName,sizeof(sprName));
	if ( sprName[0] )
	{
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
	}

//TEST: Mouse Light
/*
	if (!g_hLight)
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType	= OT_LIGHT;
		createStruct.m_Flags		= FLAG_VISIBLE | FLAG_ONLYLIGHTOBJECTS;

        g_pLTClient->GetObjectPos(m_hInterfaceCamera, &createStruct.m_Pos);
		//vPos -= (vF * (g_fBackDist / 2.0f));
		//createStruct.m_Pos = vPos;

		createStruct.m_Pos.x += 10.0f;
		createStruct.m_Pos.y += 25.0f;
		createStruct.m_Pos.z -= 300.0f;

        g_hLight = g_pLTClient->CreateObject(&createStruct);

        g_pLTClient->SetLightColor(g_hLight, 1.0f, 1.0f, 1.0f);
        g_pLTClient->SetLightRadius(g_hLight, 1500.0f);
	}
*/
// END TEST!!!

}

void CInterfaceMgr::UpdateInterfaceBackground()
{

/*
	if (GetConsoleInt("MouseLight",0))
	{
		LTVector vPos;
		LTRotation rRot;

		g_pLTClient->GetObjectPos(m_hInterfaceCamera, &vPos);
		g_pLTClient->GetObjectRotation(m_hInterfaceCamera, &rRot);

		vPos.z -= 300.0f;

		float x = ((float)m_CursorPos.x - ((float)g_pInterfaceResMgr->GetScreenWidth() / 2.0f));
		float y = (((float)g_pInterfaceResMgr->GetScreenHeight() / 2.0f) - (float)m_CursorPos.y);

		g_pLTClient->CPrint("light pos: (%0.2f,%0.2f)",x,y);
		vPos.x+=x;
		vPos.y+=y;

		g_pLTClient->SetObjectPos(g_hLight, &vPos);
	//	g_pLTClient->SetObjectPos(m_BackSprite.GetObject(), &vPos);
	}
*/
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

	SFXArray::iterator iter = m_InterfaceSFX.begin();
	uint32 index = 0;
	while (iter != m_InterfaceSFX.end() && (*iter) != pSFX)
	{
		iter++;
		index++;
	}
	if (index >= m_InterfaceSFX.size())
	{
		if (m_InterfaceSFX.size() < MAX_INTERFACE_SFX)
		{
			m_InterfaceSFX.push_back(pSFX);
		}
	}

	HOBJECT hObj = pSFX->GetObject();
	if (!hObj)
		return;

	if (GetObjectType(hObj)== OT_MODEL)
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
				pAniName = "World";
			break;

			case IFX_MENU_ATTACH:	// abm 3/7/02 new fx type for when an ani is prespecified
				pAniName = LTNULL;
			break;

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
	SFXArray::iterator iter = m_InterfaceSFX.begin();
	uint32 index = 0;
	while (iter != m_InterfaceSFX.end() && (*iter) != pSFX)
	{
		iter++;
	}
	if (iter != m_InterfaceSFX.end())
	{
		m_InterfaceSFX.erase(iter);	
	}
}

void CInterfaceMgr::AddInterfaceFX(CLIENTFX_LINK* pLink, char *pFXName, LTVector vPos, LTBOOL bLoop)
{
	uint32 dwFlags = 0;
	if (bLoop)
		dwFlags |= FXFLAG_LOOP;

	CLIENTFX_CREATESTRUCT	fxInit(pFXName , dwFlags, vPos ); 
	CLIENTFX_LINK fxLink;
	if (!pLink)
		pLink = &fxLink;

	m_InterfaceFXMgr.CreateClientFX(pLink, fxInit, LTTRUE );

	if (pLink->IsValid())
	{
		pLink->GetInstance()->SetPos(vPos,vPos);
		pLink->GetInstance()->Show();
	}
	else
	{
		g_pLTClient->CPrint("ERROR: Failed to create interface FX %s",pFXName);
	}

}
void CInterfaceMgr::RemoveInterfaceFX()
{
	m_InterfaceFXMgr.ShutdownAllFX();
	m_SelectFX.ClearLink();
	m_MouseFX.ClearLink();
}

void CInterfaceMgr::RemoveInterfaceFX(CLIENTFX_LINK	*pLink)
{
	m_InterfaceFXMgr.ShutdownClientFX(pLink);
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
	m_InterfaceSFX.clear();
	RemoveInterfaceBackground();
	RemoveInterfaceLights();

	m_InterfaceFXMgr.ShutdownAllFX();
	m_SelectFX.ClearLink();
	m_MouseFX.ClearLink();
}

void CInterfaceMgr::AddInterfaceLight(HOBJECT hLight)
{
	if (!hLight) return;

	ObjectArray::iterator iter = m_InterfaceLights.begin();
	while (iter != m_InterfaceLights.end() && (*iter) != hLight)
	{
		iter++;
	}
	if (iter == m_InterfaceLights.end())
	{
		if (m_InterfaceLights.size() < MAX_INTERFACE_LIGHTS)
		{
			m_InterfaceLights.push_back(hLight);
		}
	}

}

void CInterfaceMgr::RemoveInterfaceLights()
{
	ObjectArray::iterator iter = m_InterfaceLights.begin();
	while (iter != m_InterfaceLights.end())
	{
		if( *iter )
		{
		g_pLTClient->RemoveObject((*iter));
		}

		iter++;
	}
	m_InterfaceLights.clear();
}


void CInterfaceMgr::UpdateModelAnimations(LTFLOAT fCurFrameDelta)
{
    for (uint32 i = 0; i < m_InterfaceSFX.size(); i++)
	{
		uint32 nObjType;
		g_pCommonLT->GetObjectType(m_InterfaceSFX[i]->GetObject(), &nObjType);
		if (nObjType == OT_MODEL)
		{
			g_pModelLT->UpdateMainTracker(m_InterfaceSFX[i]->GetObject(), fCurFrameDelta);
		}
	}

	CLinkList<CLIENTFX_INSTANCE *>* pInstList = m_InterfaceFXMgr.GetActiveFXList();
	CLIENTFX_INSTANCE	*pInst;

	CLinkListNode<CLIENTFX_INSTANCE *> *pInstNode = pInstList->GetHead();
	while( pInstNode )
	{
		pInst	= pInstNode->m_Data;

		CLinkListNode<FX_LINK>				*pActiveNode;
		CBaseFX								*pFX;

		pActiveNode = pInst->m_collActiveFX.GetHead();
		while( pActiveNode )
		{
			pFX = pActiveNode->m_Data.m_pFX;
			if( pFX) 
			{
				uint32 nObjType;
				g_pCommonLT->GetObjectType(pFX->GetFXObject(), &nObjType);
				if (nObjType == OT_MODEL)
				{
					g_pModelLT->UpdateMainTracker(pFX->GetFXObject(), fCurFrameDelta);
				}
			}
			pActiveNode = pActiveNode->m_pNext;
		}
		pInstNode = pInstNode->m_pNext;
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

//TEST: Mouse Light
//	UpdateInterfaceBackground();

	static LTIntPt s_oldPos = m_CursorPos;
	int dist = abs(s_oldPos.x - m_CursorPos.x) + abs(s_oldPos.y - m_CursorPos.y);
	if (dist > 1)
		ShowMouseFX();
	else
	{
		if (m_MouseFX.IsValid())
			m_MouseFX.GetInstance()->Hide();
	}
	s_oldPos = m_CursorPos;

    uint32 numSfx = m_InterfaceSFX.size();
	HLOCALOBJ objs[kMaxFX];

	int next = 0;
	int infoIndex = 0;
    for (uint32 i = 0; i < numSfx; i++)
	{
		if (m_InterfaceSFX[i]->Update())
		{
			FXRenderList::iterator iter = renderList.begin();

			while (iter != renderList.end() && (*iter)->nLayer < m_InterfaceSFX[i]->GetMenuLayer())
			{
				iter++;
			}

			if(infoIndex < kMaxFX)
			{
				sRenderInfo[infoIndex].hObj = m_InterfaceSFX[i]->GetObject();
				sRenderInfo[infoIndex].nLayer = m_InterfaceSFX[i]->GetMenuLayer();
				renderList.insert(iter,&sRenderInfo[infoIndex]);
				infoIndex++;
			}
		}
	}
	CLinkList<CLIENTFX_INSTANCE *>* pInstList = m_InterfaceFXMgr.GetActiveFXList();
	CLIENTFX_INSTANCE	*pInst;

	CLinkListNode<CLIENTFX_INSTANCE *> *pInstNode = pInstList->GetHead();

	while( pInstNode )
	{
		pInst	= pInstNode->m_Data;

		CLinkListNode<FX_LINK>				*pActiveNode;
		CBaseFX								*pFX;

		pActiveNode = pInst->m_collActiveFX.GetHead();
		while( pActiveNode )
		{
			pFX = pActiveNode->m_Data.m_pFX;
			if( pFX) 
			{
				FXRenderList::iterator iter = renderList.begin();

				while (iter != renderList.end() && (*iter)->nLayer < pFX->GetMenuLayer())
				{
					iter++;
				}

				if(infoIndex < kMaxFX)
				{
					sRenderInfo[infoIndex].hObj = pFX->GetFXObject();
					sRenderInfo[infoIndex].nLayer = pFX->GetMenuLayer();
					renderList.insert(iter,&sRenderInfo[infoIndex]);
					infoIndex++;
				}
			}
			pActiveNode = pActiveNode->m_pNext;
		}
		pInstNode = pInstNode->m_pNext;
	}


	// Render the effects
	m_InterfaceFXMgr.RenderAllActiveFX( true );

	//step through the render list
	FXRenderList::iterator iter = renderList.begin();
	static char szTmp[256];
	static char szNum[16];
	while (iter != renderList.end())
	{
		next = 0;

		//get the menu layer of the first item
		uint8 layer = (*iter)->nLayer;

		//add each item of that same layer to the list of objects to be rendered during this pass
		while (iter != renderList.end() && next < kMaxFX && (*iter)->nLayer == layer)
		{
			objs[next] = (*iter)->hObj;
			next++;
			iter++;
		}

		//add the lights
		for ( ObjectArray::iterator iter = m_InterfaceLights.begin( ); iter !=  m_InterfaceLights.end() && 
			next < kMaxFX; iter++ )
		{
			HOBJECT hObj = *iter;
			if ( hObj )
			{
				objs[next] = hObj;
				next++;
			}
		}


		g_pLTClient->RenderObjects(m_hInterfaceCamera, objs, next,g_pLTClient->GetFrameTime());
	
	}


	//the updating of the active effects comes at the end of the update so that way chained
	//effects will be able to setup prior to the next rendering and prevent popping issues
	m_InterfaceFXMgr.UpdateAllActiveFX( true );

	renderList.clear();	
}


/******************************************************************/
void CInterfaceMgr::RequestInterfaceSound(InterfaceSound eIS)
{
	if (m_eNextSound <= eIS)
	{
//		ClearInterfaceSound();

		m_eNextSound = eIS;

//		m_hSound = UpdateInterfaceSound();
		UpdateInterfaceSound();
	}
}

void CInterfaceMgr::ClearInterfaceSound()
{
	m_eNextSound = IS_NONE;

	if (m_hSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSound);
		m_hSound = LTNULL;
	}
}

HLTSOUND CInterfaceMgr::UpdateInterfaceSound()
{
	HLTSOUND hSnd = LTNULL;

	switch (m_eNextSound)
	{
		case IS_SELECT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_CHANGE:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundChange(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_PAGE:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundPageChange(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_UP:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowUp(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_DOWN:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowDown(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_LEFT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowLeft(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_RIGHT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundArrowRight(), NULL /*PLAYSOUND_GETHANDLE*/);
		break;
		case IS_NO_SELECT:
			hSnd = g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundUnselectable(), NULL /*PLAYSOUND_GETHANDLE*/);
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

void CInterfaceMgr::NextMovie(bool bEndMovies /*=false*/)
{
	ILTVideoMgr* pVideoMgr = g_pLTClient->VideoMgr();
	if (!pVideoMgr) return;

	if (!(GetAdvancedOptions() & AO_MOVIES) || g_vtDisableMovies.GetFloat())
	{
		SwitchToScreen(SCREEN_ID_MAIN);
		return;
	}

	if (m_hMovie)
	{
		pVideoMgr->StopVideo(m_hMovie);
		m_hMovie = LTNULL;
	}

	char* pMovie = (bEndMovies ? LTNULL : GetCurrentMovie());

	if (!pMovie || pVideoMgr->StartOnScreenVideo(pMovie, PLAYBACK_FULLSCREEN, m_hMovie) != LT_OK)
	{
		m_nCurMovie = 0;
		m_hMovie = LTNULL;
		SwitchToScreen(SCREEN_ID_MAIN);
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
		case eSierra :
			pMovie = "Movies\\sierralogo.bik";
		break;

		case eFox :
			pMovie = "Movies\\foxlogo.bik";
		break;

		case eMonolith :
			pMovie = "Movies\\lithLogo.bik";
		break;

		case eLithTech :
			pMovie = "Movies\\ltLogo.bik";
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
    g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER, 0);

	if (g_hDemo)
	{
		g_pLTClient->DeleteSurface(g_hDemo);
		g_hDemo = LTNULL;
	}
	g_nDemo++;
	if (g_nDemo < g_nNumDemoScreens)
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
	eScreenID destination = SCREEN_ID_MULTI;
	// Get the disconnect code.
	uint32 nDisconnectCode = g_pClientMultiplayerMgr->GetDisconnectCode();
	const char *pDisconnectMsg = g_pClientMultiplayerMgr->GetDisconnectMsg();
	uint32 nMsgId = 0;
	switch (nDisconnectCode)
	{
		case eDisconnect_NotSameGUID:
			nMsgId = IDS_NETERR_NOTSAMEGUID;
			break;
		case eDisconnect_WrongPassword:
			nMsgId = IDS_NETERR_WRONGPASS;
			if (g_bLAN)
				destination = SCREEN_ID_JOIN_LAN;
			else
				destination = SCREEN_ID_JOIN;
			break;

		case 0 : // Avoid the warning..
		default :
		{
			if( nDisconnectFlag == LT_REJECTED )
			{
				nMsgId = IDS_KICKBAN;
			}
			else if ( nDisconnectFlag == LT_MISSINGWORLDFILE )
			{
				nMsgId = IDS_MISSING_WORLD;
			}
			else
			{
				nMsgId = IDS_DISCONNECTED_FROM_SERVER;
			}

			g_pLTClient->CPrint("CInterfaceMgr::Disconnected(%d)", nDisconnectCode);
		}
		break;
	}

	// We joined using the command line, we need to visit the main screen before
	// any other screen, since we didn't go through the normal screen
	// progression.
	if( GetCommandLineJoin( ))
	{
		destination = SCREEN_ID_MAIN;
	}

	g_pClientMultiplayerMgr->ClearDisconnectCode();

	if (GetGameState() == GS_LOADINGLEVEL)
	{
		LoadFailed( destination, nMsgId );
	}
	else
	{
		if (!m_bIntentionalDisconnect)
		{
			MBCreate mb;
			char szMsg[1024] = "";
			LoadString( nMsgId, szMsg, sizeof(szMsg));
			ShowMessageBox(szMsg,&mb);
		}

		ClearAllScreenBuffers();
		SwitchToScreen(destination);
	}


}

void CInterfaceMgr::ConnectionFailed(uint32 nConnectionError)
{
	if (GetGameState() == GS_LOADINGLEVEL)
	{
		if (IsMultiplayerGame())
		{
			if (g_bLAN)
				LoadFailed(SCREEN_ID_JOIN_LAN);
			else
				LoadFailed(SCREEN_ID_JOIN);

		}
		else
		{
			LoadFailed(SCREEN_ID_MAIN);
		}
	}
	else
	{
		ClearAllScreenBuffers();
		if (IsMultiplayerGame() && !GetCommandLineJoin( ))
		{
			SwitchToScreen(SCREEN_ID_MULTI);
		}
		else
		{
			SwitchToScreen(SCREEN_ID_MAIN);
		}
	}

	MBCreate mb;
	if (nConnectionError == LT_REJECTED)
	{
		ShowMessageBox(IDS_SERVERFULL,&mb);
	}
	else
	{
		ShowMessageBox(IDS_NETERR_JOINSESSION,&mb);
	}

}

// hides or shows cursor based on current game state
void CInterfaceMgr::UpdateCursorState()
{
	switch (m_eGameState)
	{
		case GS_EXITINGLEVEL:
		case GS_PLAYING:
		{	
			// 2nd param specifiies whether or not to lock the cursor to the
			// center of the screen
			m_CursorMgr.UseCursor(m_MessageBox.IsVisible(),!m_MessageBox.IsVisible());
		}
		break;

		case GS_LOADINGLEVEL:
		case GS_SPLASHSCREEN:
		case GS_POPUP:
		case GS_PAUSED:
		case GS_DEMOSCREEN:
		case GS_MOVIE:
		case GS_UNDEFINED:
		{
			m_CursorMgr.UseCursor(FALSE);
		}
		break;
	
		case GS_MENU:
		case GS_SCREEN:
		default:
		{
			m_CursorMgr.UseCursor(TRUE);
		}
	}
}



void CInterfaceMgr::UpdateClientList()
{
	if (IsMultiplayerGame())
	{
		// Don't send update requests more than once per second
		if ((g_pLTClient->GetTime() - m_fLastUpdateRequestTime) > 1.0f)
		{
			g_pLTClient->CPrint("Asking for clientlist update");
			SendEmptyServerMsg(MID_MULTIPLAYER_UPDATE, MESSAGE_GUARANTEED);
			m_fLastUpdateRequestTime = g_pLTClient->GetTime();
		}
	}
}



LTVector CInterfaceMgr::GetWorldFromScreenPos(LTIntPt pos, LTFLOAT fDepth)
{
	LTVector vPos;

	LTFLOAT fx = (2.0f * ((LTFLOAT)pos.x / (LTFLOAT)g_pInterfaceResMgr->GetScreenWidth() )) - 1.0f;
	LTFLOAT fy = 1.0f - (2.0f * ((LTFLOAT)pos.y / (LTFLOAT)g_pInterfaceResMgr->GetScreenHeight()) );

	vPos.x =  fx * g_fFovXTan * fDepth;
	vPos.y =  fy * g_fFovYTan * fDepth;
	vPos.z =  fDepth;

	return vPos;
}

//transforms world space to camera space to screen coordinates
//return vector's x and y are screen coordinates
//return vector's z is distance from the camera

LTVector CInterfaceMgr::GetScreenFromWorldPos(LTVector vPos, HOBJECT hCamera)
{
	LTVector tmp,pos;
	LTMatrix mCam, mInvCam;
	mCam = GetCameraTransform(hCamera);
	mCam.Normalize();

	mInvCam = mCam.MakeInverseTransform();
	mInvCam.Normalize();

	MatVMul((LTVector*)&tmp, &mCam, (LTVector*)&vPos);


	LTFLOAT fx = tmp.x / (g_fFovXTan * tmp.z);
	LTFLOAT fy = tmp.y / (g_fFovYTan * tmp.z);

	pos.x =  ((LTFLOAT)g_pInterfaceResMgr->GetScreenWidth() * (1.0f + fx) / 2.0f);
	pos.y =	 ((LTFLOAT)g_pInterfaceResMgr->GetScreenHeight() * (1.0f - fy) / 2.0f);
	pos.z =  tmp.z;

	return pos;
}


void CInterfaceMgr::SetMouseFX(char* pFXName)
{
	SAFE_STRCPY(m_szMouseFXName,pFXName);
	if (m_MouseFX.IsValid())
	{
		m_InterfaceFXMgr.ShutdownClientFX(&m_MouseFX);
	}
}

void CInterfaceMgr::SetSelectFX(char* pFXName)
{
	SAFE_STRCPY(m_szSelectFXName,pFXName);
}

void CInterfaceMgr::ShowSelectFX(const LTIntPt &pos)
{
	if (!strlen(m_szSelectFXName)) return;
	if (m_SelectFX.IsValid() && GetConsoleInt("ShutdownSelect",1))
	{
		m_InterfaceFXMgr.ShutdownClientFX(&m_SelectFX);
	}
	LTVector vPos = GetWorldFromScreenPos(pos,25.0f);
	CLIENTFX_CREATESTRUCT	fxInit(m_szSelectFXName , LTNULL, vPos ); 

	m_InterfaceFXMgr.CreateClientFX(&m_SelectFX, fxInit, LTTRUE );
	if( !m_SelectFX.IsValid() )
	{
		char szError[256];
		LTSNPrintF( szError, sizeof(szError), "CInterfaceMgr::ShowSelectFX:  Could not create client fx %s", m_szSelectFXName );
		UBER_ASSERT( 0, szError );
		return;
	}

	if(m_SelectFX.IsValid())
		m_SelectFX.GetInstance()->Show();

}
void CInterfaceMgr::ShowMouseFX()
{
	if (!strlen(m_szMouseFXName))
	{
		return;
	}
	LTVector vPos = GetWorldFromScreenPos(m_CursorPos,25.0f);

	if (!m_MouseFX.IsValid())
	{
		CLIENTFX_CREATESTRUCT	fxInit(m_szMouseFXName , FXFLAG_LOOP, vPos ); 
		m_InterfaceFXMgr.CreateClientFX(&m_MouseFX, fxInit, LTTRUE );
	}

	if (m_MouseFX.IsValid())
	{
		m_MouseFX.GetInstance()->SetPos(vPos,vPos);
		m_MouseFX.GetInstance()->Show();
	}
}


void CInterfaceMgr::NextWeapon(int nCommand)
{
	CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	IClientWeaponBase const *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	if (pClientWeapon && !pClientWeaponMgr->WeaponsEnabled()) return;

	//can't bring up chooser while disabling a gadget target
	if (g_pPlayerMgr->IsDisabling()) return;

	uint8 nClass = 0;
	switch (nCommand)
	{
	case COMMAND_ID_NEXT_WEAPON	:
		nClass = 0;
		break;
	case COMMAND_ID_NEXT_WEAPON_1 :
		nClass = 1;
		break;
	case COMMAND_ID_NEXT_WEAPON_2 :
		nClass = 2;
		break;
	case COMMAND_ID_NEXT_WEAPON_3 :
		nClass = 3;
		break;
	case COMMAND_ID_NEXT_WEAPON_4 :
		nClass = 4;
		break;
	case COMMAND_ID_NEXT_WEAPON_5 :
		nClass = 5;
		break;
	case COMMAND_ID_NEXT_WEAPON_6 :
		nClass = 6;
		break;
	default:
		return;
		
	}

	if (m_AmmoChooser.IsOpen())
	{
		m_AmmoChooser.Close();
	}
	if (m_WeaponChooser.Open(nClass))
	{
		m_WeaponChooser.NextWeapon(nClass);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnObjectRemove()
//
//	PURPOSE:	Handle removal of a server created object...
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnObjectRemove(HLOCALOBJ hObj)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (!hObj) return;

	if (g_pDecision->IsVisible() && g_pDecision->GetObject() == hObj)
		g_pDecision->OnObjectRemove(hObj);

	if( g_pGameClientShell->ShouldUseRadar())
		g_pRadar->RemoveObject( hObj );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::IsInGame()
//
//	PURPOSE:	Check if we're in one of the "in game" states.
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::IsInGame( )
{
	switch( GetGameState( ))
	{
		case GS_PLAYING:
		case GS_EXITINGLEVEL:
			return true;
		default:
			return false;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandleDisplayTimerMsg
//
//	PURPOSE:	Handle update to display timer.
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::HandleDisplayTimerMsg( ILTMessage_Read& msg )
{
    float fGameTimeEnd = msg.Readfloat();
	bool bPaused = !!msg.Readuint8();
	uint8 nTeamId = msg.Readuint8( );

	// Calculate time to the end.
	switch( nTeamId )
	{
		case 0:
			m_BlueInterfaceTimer.SetTime( fGameTimeEnd, bPaused );
			break;
		case 1:
			m_RedInterfaceTimer.SetTime( fGameTimeEnd, bPaused );
			break;
		default:
			m_InterfaceTimer.SetTime( fGameTimeEnd, bPaused );
			break;
	}
    return LTTRUE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::LoadFailed
//
//	PURPOSE:	Handle failed load attempt, specify what screen to go to
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::LoadFailed(eScreenID eScreenToShow /*= SCREEN_ID_NONE*/, uint32 nLoadFailedMsgId /* = -1 */ )  
{	
	m_bLoadFailed = LTTRUE;
	m_nLoadFailedMsgId = nLoadFailedMsgId;

	if (eScreenToShow == SCREEN_ID_NONE)
	{
		if (IsMultiplayerGame())
		{
			m_eLoadFailedScreen = SCREEN_ID_MULTI;
		}
		else
		{
			m_eLoadFailedScreen = SCREEN_ID_MAIN;
		}
	}
	else
		m_eLoadFailedScreen = eScreenToShow;

}

void CInterfaceMgr::HandlePlayerTeamChange()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	CSpecialFXList* pList = psfxMgr->GetFXList(SFX_CHARACTER_ID);
	if (pList)
	{
		int nNumChars = pList->GetSize();

		for (int i=0; i < nNumChars; i++)
		{
			if ((*pList)[i])
			{
				CCharacterFX* pCFX = (CCharacterFX*)(*pList)[i];
				if( pCFX->m_cs.bRadarVisible)
				{
					g_pRadar->RemoveObject(pCFX->GetServerObj());
					g_pRadar->AddPlayer( pCFX->GetServerObj(), pCFX->m_cs.nClientID );
				}
			}
		}
	}						
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::StartingNewGame
//
//	PURPOSE:	Called when client is starting its connection to the server.
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::StartingNewGame( )
{
	GetClientInfoMgr()->RemoveAllClients();
	SkipPreLoad( false );

	return true;
}
