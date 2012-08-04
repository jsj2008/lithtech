// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMgr.cpp
//
// PURPOSE : Manage all interface related functionality
//
// CREATED : 4/6/99
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "CommandIDs.h"
#include "MsgIDs.h"
#include "VKDefs.h"
#include "SoundMgr.h"
#include "InterfaceResMgr.h"
#include "VarTrack.h"
#include "CharacterFX.h"
#include "GameButes.h"
#include "ClientWeaponMgr.h"
#include "ClientConnectionMgr.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"
#include "WaveFn.h"
#include "iltrenderer.h"
#include "direct.h"
#include "WinUtil.h"
#include "resourceextensions.h"
#include "HUDMessageQueue.h"
#include "HUDChatInput.h"
#include "HUDDebugInput.h"
#include "HUDSubtitles.h"
#include "HUDTimer.h"
#include "HUDDecision.h"
#include "HUDTransmission.h"
#include "HUDEndRoundMessage.h"
#include "HUDNavMarkerMgr.h"
#include "HUDOverlay.h"
#include "HUDRadio.h"
#include "HUDScores.h"
#include "sys/win/mpstrconv.h"
#include "PlayerCamera.h"
#include "BindMgr.h"
#include "ClientDB.h"
#include "PickupItemFX.h"
#include "NavMarkerFX.h"
#include "ProjectileFX.h"
#include "GameModeMgr.h"
#include "Credits.h"
#include "ltprofileutils.h"
#include "iltresourcemgr.h"
#include "BroadcastDB.h"
#include "CMoveMgr.h"
#include "ScreenPreload.h"
#include "ObjectiveDB.h"
#include "lttimeutils.h"
#include "ltgamecfg.h"
#include "ScreenMutePlayer.h"

#if !defined(PLATFORM_XENON)
#include "ILTGameUtil.h"
static ILTGameUtil *g_pLTGameUtil;
define_holder(ILTGameUtil, g_pLTGameUtil);
#endif // !defined(PLATFORM_XENON)

CInterfaceMgr*  g_pInterfaceMgr = NULL;

#define MAX_INTERFACE_SFX			100
#define MAX_INTERFACE_LIGHTS		5
#define INTERFACE_ASSET_LIST_FILE	"Interface.asset00"

LTVector	g_vBaseBackScale(0.8f, 0.6f, 1.0f);
float		g_fBackDist = 200.0f;

float		g_fSplashSndDuration = 0.0f;
float		g_fSplashSndMinEndTime = 0.0f;

TextureReference    g_hSplashTex;
TextureReference    g_hMPFreeSplashTex;

TextureReference    g_hDemoTexture;
uint32		g_nDemoScreen = 0;

// PunkBuster error prefix
const char* const g_pszPunkBusterErrorPrefix = "error\n";

VarTrack	g_vtDrawInterface;
VarTrack	g_vtModelApplySun;
VarTrack	g_vtLetterBox;
VarTrack	g_vtLetterBoxFadeInTime;
VarTrack	g_vtLetterBoxFadeOutTime;
VarTrack	g_vtLetterBoxDisabled;
VarTrack	g_vtDisableMovies;
VarTrack	g_vtInterfaceFOVY;
VarTrack	g_vtInterfaceFOVAspectRatioScale;
VarTrack	g_vtSplashScreenFadeIn;
VarTrack	g_vtSplashScreenFadeOut;
VarTrack	g_vtSplashScreenTime;
VarTrack	g_vtMainScreenFadeIn;
VarTrack	g_vtExitLevelScreenFadeTime;
VarTrack	g_vtChooserAutoSwitchTime;
VarTrack	g_vtChooserAutoSwitchFreq;
VarTrack	g_vtWaitForResourcesToLoad;

extern VarTrack g_vtScreenFadeInTime;

const char* c_GameStateNames[] =
{
	"GS_UNDEFINED",
	"GS_PLAYING",
	"GS_EXITINGLEVEL",
	"GS_LOADINGLEVEL",
	"GS_SPLASHSCREEN",
	"GS_MENU",
	"GS_SCREEN",
	"GS_PAUSED",
	"GS_DEMOSCREEN",
	"GS_MOVIE",
};

namespace
{
	bool g_bInGameFogEnabled = true;
	bool g_bNavMarkersToggled = false;

	//**********************************************************
	//*** definitions needed for rendering FX in the interface
	//**********************************************************
	struct FXRenderInfo
	{
		bool operator<(const FXRenderInfo& rhs)
		{
			return nLayer < rhs.nLayer;
		}

		HOBJECT hObj;
		uint32	nLayer;
	};

	typedef std::vector<FXRenderInfo, LTAllocator<FXRenderInfo, LT_MEM_TYPE_CLIENTSHELL> > FXRenderList;
	FXRenderList g_FxRenderList;

	typedef std::vector<int, LTAllocator<int, LT_MEM_TYPE_CLIENTSHELL> > KeystrokeList;
	KeystrokeList g_keys;

	bool bHasAnsweredPatchRequest = false;
	// ----------------------------------------------------------------------- //
	//
	//  ROUTINE:	GetPatchCallBack
	//
	//  PURPOSE:	Callback for the Getpatch messagebox...
	//
	// ----------------------------------------------------------------------- //
	void GetPatchCallBack(bool bReturn, void *pData, void* pUserData)
	{
		bHasAnsweredPatchRequest = true;
		if( bReturn )
		{
			// Launch the updater.
			LaunchApplication::LaunchPatchUpdate( g_pInterfaceMgr->GetPatchURL( ));
		}
	}

}

static bool HandleDebugKey( HRECORD hDebugKey )
{
#if defined(_FINAL) || defined(_DEMO)
	return false;
#else // !(FINAL || DEMO)

	char szTempStr[256];
	char szVarName[256];

	ClientDB& ClientDatabase = ClientDB::Instance();

	szVarName[0] = 0;
	LTStrCpy(szVarName, ClientDatabase.GetRecordName(hDebugKey), LTARRAYSIZE(szVarName));

	if( !szVarName[0] )
	{
		return false;
	}

	// Get the console variable associated with this debug key.
	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable(szVarName);

	// Initialize the variable if it does not exist.
	if( !hVar )
	{
		g_pLTClient->SetConsoleVariableFloat(szVarName, 0.0f);

		hVar = g_pLTClient->GetConsoleVariable(szVarName);
		ASSERT( hVar );

		if( !hVar )
		{
			return false;
		}
	}

	// Increment the current debug level, recycling to zero if it is above max levels.
	float fCurrentLevel = g_pLTClient->GetConsoleVariableFloat(hVar);
	++fCurrentLevel;


	if( uint32(fCurrentLevel) >= ClientDatabase.GetNumValues(hDebugKey, CDB_DebugKeyTitle) )
	{
		fCurrentLevel = 0.0f;
	}

	// Set the new debug level.
	g_pLTClient->SetConsoleVariableFloat(szVarName, fCurrentLevel );

	g_pLTClient->CPrint("%s %2f", szVarName, fCurrentLevel );
	szTempStr[0] = 0;

	LTStrCpy(szTempStr, ClientDatabase.GetString(hDebugKey, CDB_DebugKeyString, (int)fCurrentLevel), LTARRAYSIZE(szTempStr) );
	if( szTempStr )
	{
		g_pLTClient->RunConsoleCommand(szTempStr);
#ifdef _DEBUG
		g_pLTClient->CPrint( szTempStr );
#endif
	}

	// Display message
	szTempStr[0] = 0;
	LTStrCpy(szTempStr, ClientDatabase.GetString(hDebugKey, CDB_DebugKeyTitle, (int)fCurrentLevel), LTARRAYSIZE(szTempStr) );
	if( !szTempStr[0] )
	{
		LTSNPrintF(szTempStr, LTARRAYSIZE(szTempStr), "%s set to level %1.0f", szVarName, fCurrentLevel );
	}

	g_pGameMsgs->AddMessage( MPA2W(szTempStr).c_str() );

	return true;
#endif // !(FINAL || DEMO)
}

static bool HandleDebugKey(int key )
{
#if defined(_FINAL) || defined(_DEMO) || defined(PLATFORM_XENON)
	return false;
#else

	HRECORD hDebugKey = NULL;

	// Go through each debug key and see if it matches the key hit.
	// Favor modified versions of keys (e.g. shift F11).


	ClientDB& ClientDatabase = ClientDB::Instance();
	
	uint8 nDebugKeys = g_pLTDatabase->GetNumRecords(ClientDatabase.GetDebugKeyCategory());

	for( uint8 i = 0; i < nDebugKeys; ++i )
	{
		HRECORD hDebugKeyRecord = g_pLTDatabase->GetRecordByIndex(ClientDatabase.GetDebugKeyCategory(), i);
		int iDebugKey = ClientDatabase.GetInt32(hDebugKeyRecord, CDB_DebugKeyId);
		int iDebugKeyModifier = ClientDatabase.GetInt32(hDebugKeyRecord, CDB_DebugModifierId);

		if( iDebugKey == key )
		{
			if( IsKeyDown( iDebugKeyModifier ))
			{
				hDebugKey = hDebugKeyRecord;	
			}
			else if( ( iDebugKeyModifier == 0 ) &&
					 ( NULL == hDebugKey ) )
			{
				hDebugKey = hDebugKeyRecord;	
			}
		}
	}

	// Return false on error
	if( NULL == hDebugKey )
	{
		return false;
	}
	
	// Handle the debug key by record
	HandleDebugKey( hDebugKey );

	return false;
#endif // !(_FINAL || _DEMO || PLATFORM_XENON)
}

static void DebugKeyConsoleProgramFn( int argc, char **argv )
{
#if defined(_FINAL) || defined(_DEMO)
	return;
#else // !(FINAL || DEMO)

	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;
	
	// Check the parameter count
	if ( argc < 1 )
	{
		g_pLTClient->CPrint("Debug key error: Debug key not provided");
		return;
	}

	ClientDB& ClientDatabase = ClientDB::Instance();
	HRECORD hDebugKey = NULL;
	
	// Fetch the record
	hDebugKey = ClientDatabase.GetRecord( ClientDatabase.GetDebugKeyCategory(), argv[0] );
	if ( NULL == hDebugKey )
	{
		g_pLTClient->CPrint( "Debug key error: Debug key %s not found", argv[0] );
		return;
	}
	
	// Hand it off to the actual handling function
	HandleDebugKey( hDebugKey );
	
#endif // !(FINAL || DEMO)

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

	m_fMenuSaveFOVx			= 0.0f;
	m_fMenuSaveFOVy			= 0.0f;

    m_bSwitchingModes       = false;

	m_hSplashSound          = NULL;
	g_fSplashSndDuration	= 0.0f;
	g_fSplashSndMinEndTime	= 0.0f;

    m_szFailStringId         = "";

	m_bUseInterfaceCamera	= true;
	m_hInterfaceCamera		= NULL;

    m_hScubaSound		= NULL;

	m_fLetterBoxAlpha	= 0.0f;
	m_bLetterBox		= false;
	m_bWasLetterBox		= false;
	m_fLetterBoxFadeEndTime = 0.0f;

    m_bScreenFade			= false;
    m_bFadeInitialized		= false;
	m_fScreenFadeAlpha		= 0.0f;
	m_fTotalFadeTime		= 0.0f;
	m_fCurFadeTime			= 0.0f;
    m_bFadeIn				= true;
	m_bOverrideInitialFade  = false;

	m_eNextSound		= IS_NONE;

	m_bQuitAfterDemoScreens = false;
	m_bSeenDemoScreens = false;

	m_hMovie			= NULL;
	m_nCurMovie			= 0;

	m_bLoadFailed			 = false;
	m_bCommandLineJoin	     = false;
	m_eLoadFailedScreen		 = SCREEN_ID_MAIN;
	m_sLoadFailedMsg		 = L"";
	m_bShowLoadFailedMessage = true;

	m_hGamePausedTex = NULL;

	m_fLastUpdateRequestTime = 0.0f;

	m_bInitialized = false;

	m_bSuppressNextFlip = false;

	m_bIntentionalDisconnect = false;
	m_bNotifyTeamSizeBalanced = false;
	m_bNotifyTeamScoreBalanced = false;

	m_pGameSpyPatchInfo = NULL;
	m_bHasRequestedPatchInfo = false;
	m_bHasAskedToPatch = false;
	m_bHasCheckedForSoundErrors = false;

	m_bOptimizeVideoMemory = false;
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

    g_pInterfaceMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CInterfaceMgr::Init()
{

    if (!m_CursorMgr.Init())
	{
        g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CInterfaceMgr::Init():  Could not initialize Cursor!"));
        return false;
	}

	Credits::Instance( ).Init( );

    g_vtDrawInterface.Init(g_pLTClient, "DrawInterface", NULL, 1.0f);
    g_vtModelApplySun.Init(g_pLTClient, "ModelApplySun", NULL, 1.0f);


	g_vtLetterBox.Init(g_pLTClient, "LetterBox", NULL, 0.0f);
	g_vtLetterBoxDisabled.Init(g_pLTClient, "LetterBoxDisabled", NULL, 0.0f);
    g_vtLetterBoxFadeInTime.Init(g_pLTClient, "LetterBoxFadeInTime", NULL, 0.5f);
    g_vtLetterBoxFadeOutTime.Init(g_pLTClient, "LetterBoxFadeOutTime", NULL, 1.0f);
    g_vtDisableMovies.Init(g_pLTClient, "NoMovies", NULL, 0.0f);

    g_vtInterfaceFOVY.Init(g_pLTClient, "FovYInterface", NULL, 75.0f);
	g_vtInterfaceFOVAspectRatioScale.Init(g_pLTClient, "FovAspectRatioScaleInterface", NULL, GetDefaultFOVAspectRatioScale());

	g_vtSplashScreenFadeIn.Init(g_pLTClient, "SplashScreenFadeInTime", NULL, 2.5f);
	g_vtSplashScreenFadeOut.Init(g_pLTClient, "SplashScreenFadeOutTime", NULL, 2.5f);
	g_vtSplashScreenTime.Init(g_pLTClient, "SplashScreenTime", NULL, 3.0f);
	g_vtMainScreenFadeIn.Init(g_pLTClient, "MainScreenFadeInTime", NULL, 3.0f);
	
	
	g_vtExitLevelScreenFadeTime.Init( g_pLTClient, "ExitLevelScreenFadeTime", NULL, 1.0f );
	g_vtChooserAutoSwitchTime.Init(g_pLTClient, "ChooserAutoSwitchTime", NULL, 0.175f);
	g_vtChooserAutoSwitchFreq.Init(g_pLTClient, "ChooserAutoSwitchFreq", NULL, 0.1f);

	g_vtWaitForResourcesToLoad.Init(g_pLTClient, "WaitForResourcesToLoad", NULL, 1.0f);

	ProcessAdvancedOptions();

    m_Settings.Init (g_pLTClient, g_pGameClientShell);

	m_SplashScreenTimer.SetEngineTimer( RealTimeTimer::Instance( ));
	m_NextWeaponKeyDownTimer.SetEngineTimer( RealTimeTimer::Instance( ));
	m_PrevWeaponKeyDownTimer.SetEngineTimer( RealTimeTimer::Instance( ));
	m_AutoSwitchTimer.SetEngineTimer( RealTimeTimer::Instance( ));

	if( !g_pLayoutDB )
	{
		CLayoutDB &lDB = CLayoutDB::Instance( );
		if( !lDB.Init( DB_Default_Localized_File ) )
		{
			g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CInterfaceMgr::Init():  Could not initialize LayoutDB"));
			return false;
		}
	}

	g_pCursorMgr->SetCursor( g_pLayoutDB->GetDefaultCursor() );


    if (!m_InterfaceResMgr.Init())
	{
		// If we couldn't init, something critical must have happened (like no render dlls)

        g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CInterfaceMgr::Init():  Could not initialize InterfaceResMgr!"));
        return false;
	}

    if (!GetScreenMgr( )->Init())
	{
		// If we couldn't init, something critical must have happened
        g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CInterfaceMgr::Init():  Could not initialize ScreenMgr!"));
        return false;
	}

	if (!m_UserMenuMgr.Init())
	{
		// If we couldn't init, something critical must have happened
		g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CInterfaceMgr::Init():  Could not initialize UserMenuMgr!"));
        return false;
	}

	if (!m_SystemMenuMgr.Init())
	{
		// If we couldn't init, something critical must have happened
		g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CInterfaceMgr::Init():  Could not initialize UserMenuMgr!"));
		return false;
	}

    m_ClientInfo.Init();

	if (!GetPlayerStats( )->Init())
	{
        g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CInterfaceMgr::Init():  Could not initialize Player Stats!"));
        return false;
	}
	if (!GetHUDMgr()->Init())
	{
        g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CInterfaceMgr::Init():  Could not initialize HUDMgr!"));
        return false;
	}

	g_pPaused->Init();

	m_CursorPos.x = 0;
	m_CursorPos.y = 0;

	m_MessageBox.Init();

	if (!m_ProfileMgr.Init()) 
	{
		m_ProfileMgr.Term();
		g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CProfileMgr::Init():  Could not initialize ProfileMgr!"));
		return false;
	}

#if defined (PLATFORM_XENON )
#ifndef _FINAL
	g_XLiveMgr.GetProfileMgr().SetupDefaultSettings();
#endif
#endif

	//create the interface camera
	ObjectCreateStruct theStruct;
	theStruct.m_ObjectType = OT_CAMERA;
	m_hInterfaceCamera = g_pLTClient->CreateObject(&theStruct);
	LTASSERT(m_hInterfaceCamera, "Error: Unable to create interface camera object");

	//setup the camera properties
	g_pLTClient->SetCameraRect(m_hInterfaceCamera, g_pInterfaceMgr->GetViewportRect());

	LTVector2 vFOV = g_pInterfaceResMgr->GetScreenFOV(DEG2RAD(g_vtInterfaceFOVY.GetFloat()), g_vtInterfaceFOVAspectRatioScale.GetFloat());
	g_pLTClient->SetCameraFOV(m_hInterfaceCamera, vFOV.x, vFOV.y);

	if (!m_InterfaceFXMgr.Init(g_pLTClient, RealTimeTimer::Instance())) 
	{
		m_InterfaceFXMgr.Term();
		g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("ERROR in CClientFXMgr::Init():  Could not initialize interface FX mgr!"));
		return false;
	}
	m_InterfaceFXMgr.SetCamera(m_hInterfaceCamera);


#if !defined(_FINAL) && !defined(_DEMO)
	// Register the debug key console program handler
	g_pLTClient->RegisterConsoleProgram( "DebugKey", DebugKeyConsoleProgramFn );
#endif

	//prefetch our interface asset list so that way we don't get hitches going between menus
	//as we load in resources
	m_hInterfaceAssetList = g_pLTClient->ResourceMgr()->LoadUserAssetList(INTERFACE_ASSET_LIST_FILE);
	if(m_hInterfaceAssetList)
		g_pLTClient->ResourceMgr()->PrefetchUserAssetList(m_hInterfaceAssetList);

	
	// Consider ourselves initialized.
	m_bInitialized = true;

	return true;
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
	//discard all of our assets so that they can be unloaded
	if(m_hInterfaceAssetList)
	{
		g_pLTClient->ResourceMgr()->DiscardUserAssetList(m_hInterfaceAssetList);
		g_pLTClient->ResourceMgr()->ReleaseUserAssetList(m_hInterfaceAssetList);
		m_hInterfaceAssetList = NULL;
	}

	// Turn off the loading screen if it's currently up
	if (m_LoadingScreen.IsActive())
		m_LoadingScreen.Hide();

	if (m_hInterfaceCamera)
	{
        g_pLTClient->RemoveObject(m_hInterfaceCamera);
        m_hInterfaceCamera = NULL;
	}

	if (m_hMovie)
	{
		g_pLTClient->GetVideoTexture()->ReleaseVideoTexture(m_hMovie);
		m_hMovie = NULL;
	}

	//make sure that the interface manager isn't still holding onto a reference
	m_InterfaceFXMgr.SetCamera(NULL);

	RemoveAllInterfaceFX();
	

	GetScreenMgr( )->Term();
	m_UserMenuMgr.Term();
	m_SystemMenuMgr.Term();
	GetPlayerStats( )->Term();
	GetHUDMgr()->Term();
	m_InterfaceResMgr.Term();

	m_MessageBox.Term();

	m_CursorMgr.Term();

	m_InterfaceFXMgr.Term();

	if (m_hScubaSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hScubaSound);
		m_hScubaSound = NULL;
	}

	// Unititialized.
	m_bInitialized = false;

	g_hSplashTex.Free();
	g_hMPFreeSplashTex.Free();
	g_hDemoTexture.Free();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnEnterWorld()
//
//	PURPOSE:	Handle entering new world
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnEnterWorld(bool bRestoringGame)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	GetPlayerStats( )->OnEnterWorld(bRestoringGame);

	// Update every HUD element so they display accurate info
	GetHUDMgr()->QueueUpdate( kHUDAll );

	m_LoadingScreen.ClearContentDownloadInfo();

	if (g_pRadio)
	{
		g_pRadio->ResetText();
	}

	if (g_pNavMarkerMgr)
	{
		g_pNavMarkerMgr->SetMultiplayerFilter(g_pProfileMgr->GetCurrentProfile()->m_bFilterNavMarkers);
	}
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

	g_pHUDMgr->OnExitWorld();
	Credits::Instance().StopAll();

	g_pClientConnectionMgr->OnExitWorld();
	
    m_bFadeInitialized = false;

	if (g_bNavMarkersToggled)
	{
		g_pProfileMgr->GetCurrentProfile()->m_bFilterNavMarkers = g_pNavMarkerMgr->MultiplayerFilter();
		g_pProfileMgr->GetCurrentProfile()->Save();
	}
	g_bNavMarkersToggled = false;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreUpdate()
//
//	PURPOSE:	Handle pre-updates
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PreUpdate()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	// Don't clear the screen if the loading screen's up
	if (m_LoadingScreen.IsVisible())
		return true;

	if(m_eGameState != GS_PLAYING)
		g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_ALL, 0);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostUpdate()
//
//  PURPOSE:    Handle post-updates (return true to FLIP
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostUpdate()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	if (m_eGameState != GS_LOADINGLEVEL)
	{
		if (!m_bSuppressNextFlip)
			g_pLTClient->GetRenderer()->FlipScreen();
		else
			m_bSuppressNextFlip = false;
	}

	m_CursorMgr.CheckForReinit();

	if	(m_bOptimizeVideoMemory && (GS_PLAYING == m_eGameState))
	{
		m_bOptimizeVideoMemory = false;
		if (GetConsoleBool("RestartRenderBetweenMaps",false))
		{
			g_pLTClient->RunConsoleCommand("RestartRender");
		}
		else
		{
			g_pLTClient->GetRenderer()->OptimizeVideoMemory();
		}
		

		if( IsMultiplayerGameClient())
		{
		// Re-init the diplays for the characters just in case thier textures got messed up.
		CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
		while (iter != CCharacterFX::GetCharFXList( ).end( ) ) 
		{
			(*iter)->ResetDisplays();
			iter++;
		}
	}
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::Update()
//
//	PURPOSE:	Handle updating the interface
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::Update()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	// Update based on the game state...
    bool bHandled = false;
	switch (m_eGameState)
	{
		case GS_PLAYING:
		{
			UpdatePlayingState();
            bHandled = false;  // Allow further processing
		}
		break;

		case GS_MENU:
		{
			UpdateMenuState();
            bHandled = false;  // Allow further processing
		}
		break;

		case GS_EXITINGLEVEL:
		{
			UpdateExitingLevelState();
            bHandled = false;
		}
		break;

		case GS_LOADINGLEVEL:
		{
			UpdateLoadingLevelState();
            bHandled = true;
		}
		break;

		case GS_SCREEN :
		{
			UpdateScreenState();
            bHandled = true;
		}
		break;

		case GS_PAUSED:
		{
			UpdatePausedState();
            bHandled = true;
		}
		break;

		case GS_SPLASHSCREEN:
		{
			UpdateSplashScreenState();
            bHandled = true;
		}
		break;

		case GS_MOVIE:
		{
			UpdateMovieState();
            bHandled = true;
		}
		break;

		case GS_DEMOSCREEN:
		{
			UpdateDemoScreenState();
            bHandled = true;
		}
		break;


	}

	//in playing state, message box is drawn in InterfaceMgr::Draw(),
	// otherwise draw it here
	if (bHandled)
	{
		if (m_MessageBox.IsVisible())
		{
			if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
			{
				m_MessageBox.Draw();
				g_pLTClient->RenderConsoleToRenderTarget();
				g_pLTClient->GetRenderer()->End3D();
			}
		}
		m_CursorMgr.Update();
	}


	// Update the patch info.
	UpdatePatchInfo( );
	UpdateSoundInitStatus();

	return bHandled;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::DrawSFX()
//
//	PURPOSE:	Renders the currently active special effects
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::DrawSFX()
{
	UpdateInterfaceFX();

	return true;
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
		if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
		{
			m_InterfaceResMgr.DrawScreen();
			DrawSFX();
			UpdateScreenFade();

			g_pLTClient->RenderConsoleToRenderTarget();
			g_pLTClient->GetRenderer()->End3D();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePatchInfo()
//
//	PURPOSE:	Checks for patch information.
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePatchInfo()
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// We don't need to do anything if we've already requested patch info and deleted the object.
	if(HasReceivedPatchInfo() || g_pLTClient->IsConnected( ))
		return;

	// Check if we need to make the request.
	if( !m_bHasRequestedPatchInfo )
	{
		// Make our request object.
		if( !m_pGameSpyPatchInfo )
		{
			IGameSpyPatchInfo::StartupInfo startupInfo;
			startupInfo.m_sVersion = g_pVersionMgr->GetPatchVersion( );
			// Check if they provided a distributionid override on the commandline.
			int32 nDistributionIdOverride = ( int32 )GetConsoleFloat( "DistributionId", -1.0f );
			if( nDistributionIdOverride == -1 )
			{
				// The distribution id changes between mpfree and retail.
				char const* pszStringId = LTGameCfg::IsMPFreeProduct() ? "IDS_NETWORK_DISTRIBUTIONID_MPFREE" : "IDS_NETWORK_DISTRIBUTIONID";
				startupInfo.m_nPatchInfoDistributionId = _wtoi( LoadString( pszStringId ));
			}
			else
			{
				startupInfo.m_nPatchInfoDistributionId = nDistributionIdOverride;
			}
			m_pGameSpyPatchInfo = g_pLTGameUtil->CreateGameSpyPatchInfo( startupInfo );
		}

		// Make the request.
		if( m_pGameSpyPatchInfo )
		{
			// If not successful, we'll need to make the request again later.
			// It could fail if gamespy is busy on the network.
			if( m_pGameSpyPatchInfo->RequestPatchInfo( ))
				m_bHasRequestedPatchInfo = true;
		}
	}

	// Check if our request was successful.
	if( !m_bHasRequestedPatchInfo )
		return;

	switch( m_pGameSpyPatchInfo->GetPatchInfoStatus( ))
	{
		// Still processing, don't do anything for now.
		case IGameSpyPatchInfo::eBrowserStatus_Processing:
			break;
		// We have the results.
		case IGameSpyPatchInfo::eBrowserStatus_Complete:
		{
			// Get the results.
			IGameSpyPatchInfo::PatchInfoResults patchInfoResults;
			if( m_pGameSpyPatchInfo->GetPatchInfoResults( patchInfoResults ) &&
				patchInfoResults.m_bNewVersionAvailable )
			{
				// Save off the URL.  ScreenMain will poll for this.
				m_sPatchUrl = patchInfoResults.m_sURL;
			}

			// Don't need the patch update object anymore.
			g_pLTGameUtil->DestroyGameSpyPatchInfo( m_pGameSpyPatchInfo );
			m_pGameSpyPatchInfo = NULL;
			break;
		}
		// Something's wrong.
		case IGameSpyPatchInfo::eBrowserStatus_Idle:
		case IGameSpyPatchInfo::eBrowserStatus_Error:
		{
			// If we're not processing or there was an error,
			// just dump the pachinfo object.  We'll check
			// next time they launch.
			g_pLTGameUtil->DestroyGameSpyPatchInfo( m_pGameSpyPatchInfo );
			m_pGameSpyPatchInfo = NULL;
			break;
		}
	}

#endif // !PLATFORM_XENON

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateSoundInitStatus()
//
//	PURPOSE:	Checks for errors with sound driver status
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdateSoundInitStatus()
{
	// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	if (m_bHasCheckedForSoundErrors)
	{
		return;
	}

	if (!m_MessageBox.IsVisible( ))
	{
		ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();

		if (pSoundMgr)
		{
			char sounderror[80];
			bool bError = pSoundMgr->GetSoundInitError(sounderror, 80);
			if (bError)
			{
				MBCreate mb;
				mb.eType = LTMB_OK;
				ShowMessageBox(MPA2W(sounderror),&mb);
			}
		}
		m_bHasCheckedForSoundErrors = true;
	}

#endif // !PLATFORM_XENON

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
	if (m_bNotifyTeamScoreBalanced)
	{
		Credits::Instance().Start("AutoBalance_Score");
		g_pGameMsgs->AddMessage(LoadString( "AutoBalance_TeamScoreBalanced" ),kMsgDefault);
		g_pClientSoundMgr->PlayInterfaceDBSound("MPAutobalance");
	}
	else if (m_bNotifyTeamSizeBalanced)
	{
		Credits::Instance().Start("AutoBalance_Size");
		g_pGameMsgs->AddMessage(LoadString( "AutoBalance_TeamSizeBalanced" ),kMsgDefault);
		g_pClientSoundMgr->PlayInterfaceDBSound("MPAutobalance");
	}

	m_bNotifyTeamScoreBalanced = false;
	m_bNotifyTeamSizeBalanced = false;


	// Draw fx.
	DrawSFX();

	GetPlayerStats()->Update();

	// Update auto chooser switching...
	UpdateWeaponSwitch();

	// Update the player stats...
	GetHUDMgr()->Update();
	g_pPaused->Update();

	Credits::Instance().Update();

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
		// For mp, switching between rounds on the same level doesn't change the screen.
		if( IsMultiplayerGameClient( ) && !g_pMissionMgr->IsNewMission())
		{
			g_pMissionMgr->FinishExitLevel();
			return;
		}

		SwitchToScreen(SCREEN_ID_PRELOAD);
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
	// Check if we just entered the world.
	if( g_pGameClientShell->IsWorldLoaded())
	{
		// We may have been told to exit the world while we were loading the last one.
		// Ignore the fact that we entered the world in this case.
		if( g_pMissionMgr->IsExitingLevel( ))
		{
			return;
		}

		//if we can get the loading state and determine that there are still resources left to load,
		//prevent leaving this state. This is largely to prevent seeing resources being paged in at
		//the start of a level.
		if(g_vtWaitForResourcesToLoad.GetFloat() != 0.0f)
		{
			uint32 nNumLoadingResources = 0;
			if(g_pLTClient->ResourceMgr()->GetNumLoadingResources(nNumLoadingResources) == LT_OK)
			{
				if(nNumLoadingResources > 0)
					return;
			}
		}

		// If we haven't sent the loading message yet, then do so now.
		if( g_pClientConnectionMgr->GetRecvClientConnectionState() == eClientConnectionState_Loading &&
			g_pClientConnectionMgr->GetSentClientConnectionState() != eClientConnectionState_Loading )
		{
			// Send acknowledgment back that we are loaded.
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_CLIENTCONNECTION);
			cMsg.Writeuint8(eClientConnectionState_Loading);
			g_pLTClient->SendToServer(cMsg.Read(),MESSAGE_GUARANTEED);

			g_pClientConnectionMgr->SetSentClientConnectionState( eClientConnectionState_Loading );
		}

		// Wait until the server wants us in the world before we consider ourselves ready to play.
		if( g_pClientConnectionMgr->GetRecvClientConnectionState() == eClientConnectionState_InWorld )
		{
			// Make sure the loading screen isn't busy
			m_LoadingScreen.Pause();
			// Let the game client shell do stuff that's time consuming so we're not stuck on a black screen
			g_pGameClientShell->PostLevelLoadFirstUpdate();
			// Turn off the loading screen
			m_LoadingScreen.Hide();

			UpdatePostLoad();
		}
	}


	if (m_bLoadFailed)
	{
		m_bLoadFailed = false;
		m_LoadingScreen.Hide();

		// We joined using the command line, we need to visit the main screen before
		// any other screen, since we didn't go through the normal screen
		// progression.
		if( GetCommandLineJoin( ) )
		{
			SwitchToScreen(SCREEN_ID_MAIN);
		}
		else
		{
			SwitchToScreen(m_eLoadFailedScreen);
		}

		MBCreate mb;
		mb.nFlags = eMBFlag_IgnoreESC;

		if( m_sLoadFailedMsg.empty())
			m_sLoadFailedMsg = LoadString( "IDS_LOADGAMEFAILED" );

		// show the failure dialog if necessary
		if (m_bShowLoadFailedMessage)
		{
	        ShowMessageBox( m_sLoadFailedMsg.c_str(), &mb );
		}

		// reset the flag for the next time
		m_bShowLoadFailedMessage = true;
		
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
	if (LT_OK != g_pLTClient->GetRenderer()->Start3D())
		return;
	


	UpdateInterfaceFX();

	if (m_hGamePausedTex)
	{
		//draw the pause texture to the screen
		LT_POLYGT4 Quad;
		DrawPrimSetXYWH(Quad, -0.5f, -0.5f, (float)g_pInterfaceResMgr->GetScreenWidth(), (float)g_pInterfaceResMgr->GetScreenHeight());
		DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, 0xFF);
		LTVector2 vHalfTexel(0.5f / (float)g_pInterfaceResMgr->GetScreenWidth(), 0.5f / (float)g_pInterfaceResMgr->GetScreenHeight());
		SetupQuadUVs(Quad, m_hGamePausedTex, -vHalfTexel.x, -vHalfTexel.y, 1.0f, 1.0f);

		g_pLTClient->GetDrawPrim()->SetTexture(m_hGamePausedTex);
		g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);

		g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
	}

	g_pLTClient->RenderConsoleToRenderTarget();
    g_pLTClient->GetRenderer()->End3D();
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
	// If we don't have an image or sound... go ahead and end it!
	if( !m_hSplashSound && !g_hSplashTex )
	{
		EndSplashScreen( );
	}

	ScreenFadedIn();
	
	if(m_hSplashSound)
	{
		if( g_pLTClient->IsDone(m_hSplashSound))
		{
			DebugCPrint(1,"Current Time: %.4f", RealTimeTimer::Instance( ).GetTimerAccumulatedS( ));
			DebugCPrint(1,"Splash sound done playing...");

			g_pLTClient->SoundMgr()->KillSound(m_hSplashSound);
			m_hSplashSound = NULL;
		}
	}

    uint32 nWidth = 0;
    uint32 nHeight = 0;
    g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(nWidth, nHeight);


	if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
	{
		//draw the splash screen texture to the screen
		if(g_hSplashTex)
		{
			LT_POLYGT4 Quad;
			DrawPrimSetXYWH(Quad, -0.5f, -0.5f, (float)nWidth, (float)nHeight);
			DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, 0xFF);
			LTVector2 vHalfTexel(0.5f / (float)g_pInterfaceResMgr->GetScreenWidth(), 0.5f / (float)g_pInterfaceResMgr->GetScreenHeight());
			SetupQuadUVs(Quad, g_hSplashTex, -vHalfTexel.x, -vHalfTexel.y, 1.0f, 1.0f);

			g_pLTClient->GetDrawPrim()->SetTexture(g_hSplashTex);
			g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);

			g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
		}
		//draw the MPFree splash screen overlay texture to the screen
		if(g_hMPFreeSplashTex)
		{
			LT_POLYGT4 Quad;
			DrawPrimSetXYWH(Quad, -0.5f, -0.5f, (float)nWidth, (float)nHeight/2.0f);
			DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, 0xFF);
			LTVector2 vHalfTexel(0.5f / (float)g_pInterfaceResMgr->GetScreenWidth(), 0.5f / (float)g_pInterfaceResMgr->GetScreenHeight());
			SetupQuadUVs(Quad, g_hSplashTex, -vHalfTexel.x, -vHalfTexel.y, 1.0f, 1.0f);

			g_pLTClient->GetDrawPrim()->SetTexture(g_hMPFreeSplashTex);
			g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

			g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
		}

		UpdateScreenFade();

		g_pLTClient->RenderConsoleToRenderTarget();
		g_pLTClient->GetRenderer()->End3D();
	}

    static bool bDidFadeOut = false;

	if (GetCommandLineJoin())
	{
		if (!HasReceivedPatchInfo())
		{
			return;
		}

		if (!m_bHasAskedToPatch)
		{
			AskToPatch();
			return;
		}

		if (!bHasAnsweredPatchRequest)
		{
			return;
		}
		
	}

	if (!m_bScreenFade)
	{
		if (!bDidFadeOut && !m_hSplashSound)
		{
			// [KLS 7/28/02] See if it is time to start the fade out...
			if( !m_SplashScreenTimer.IsStarted( ))
			{
				m_SplashScreenTimer.Start(g_vtSplashScreenTime.GetFloat());
			}
			else if (m_SplashScreenTimer.IsTimedOut())
			{
				StartScreenFadeOut(g_vtSplashScreenFadeOut.GetFloat());
				bDidFadeOut = true;
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

	if (!HasReceivedPatchInfo())
	{
		return;
	}

	if (!m_bHasAskedToPatch)
	{
		AskToPatch();
		return;
	}

	if (!bHasAnsweredPatchRequest)
	{
		return;
	}



	// If user is joining a mp game from the command line,
	// start the connection here.
	char szIP[256] = "";
	LTStrCpy( szIP, GetConsoleTempString( "join", "" ), ARRAY_LEN( szIP ));

	wchar_t wszPassword[256];
	LTStrCpy( wszPassword, MPA2W( GetConsoleTempString( "password", "" )).c_str(), LTARRAYSIZE( wszPassword ));

	// Check if we know this is a public server and better try nat negotiations.
	bool bDoNatNegotiations = !!atoi( GetConsoleTempString( "publicserver", "1" ));
	if( !g_pClientConnectionMgr->DoConnectToIP( szIP, wszPassword, bDoNatNegotiations ))
	{
		// drop them into the join menu
		LoadFailed( SCREEN_ID_MAIN );

		MBCreate mb;
		ShowMessageBox("IDS_CANT_CONNECT_TO_SERVER", &mb);
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
	ILTVideoTexture* pVideoTex = g_pLTClient->GetVideoTexture();

	//if we are in a failure state, just switch to the main screen
	if (!pVideoTex || !m_hMovie)
	{
		SwitchToScreen(SCREEN_ID_MAIN);
		return;
	}

	//render our movie to the screen
	uint32 nScreenWidth = 0;
    uint32 nScreenHeight = 0;
    g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(nScreenWidth, nScreenHeight);

	//get the dimensions of the video
	LTVector2n vnVideoDims(0, 0);
	LTVector2n vnTextureDims(0, 0);

	if(pVideoTex->GetVideoTextureDims(m_hMovie, vnVideoDims, vnTextureDims) == LT_OK)
	{
		//we can basically fit either the width or height to the screen width or height and maintain
		//aspect ratio, so we will simply try both
		uint32 nWidth = nScreenWidth;
		uint32 nHeight = nWidth * vnVideoDims.y / vnVideoDims.x;

		if(nHeight > nScreenHeight)
		{
			nHeight = nScreenHeight;
			nWidth  = nHeight * vnVideoDims.x / vnVideoDims.y;
		}

		//determine the UV extents
		float fUOffset = 0.5f / (float)vnTextureDims.x;
		float fVOffset = 0.5f / (float)vnTextureDims.y;
		float fUWidth  = (float)vnVideoDims.x / (float)vnTextureDims.x;
		float fVHeight = (float)vnVideoDims.y / (float)vnTextureDims.y;

		//draw the video to the screen
		LT_POLYGT4 Quad;
		DrawPrimSetXYWH(Quad, (nScreenWidth - nWidth) * 0.5f, (nScreenHeight - nHeight) * 0.5f, (float)nWidth, (float)nHeight);
		DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, 0xFF);
		DrawPrimSetUVWH(Quad, fUOffset, fVOffset, fUWidth - fUOffset, fVHeight - fVOffset);

		if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
		{
			g_pLTClient->GetDrawPrim()->SetVideoTexture(m_hMovie);
			g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);
			
			g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);

			g_pLTClient->RenderConsoleToRenderTarget();
			g_pLTClient->GetRenderer()->End3D();
		}
	}


	//see if we are in an invalid state, or if we are finished with the movie
	bool bFinished = false;
	LTRESULT lResult = pVideoTex->IsVideoTextureFinished(m_hMovie, bFinished);

	if((lResult != LT_OK) || bFinished)
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
		{
			g_pLTClient->Shutdown();
		}
		else
		{
			SwitchToScreen(SCREEN_ID_MAIN);
		}
		return;
	}

    uint32 nWidth = 0;
    uint32 nHeight = 0;
    g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(nWidth, nHeight);

	//draw the splash screen texture to the screen
	LT_POLYGT4 Quad;
	DrawPrimSetXYWH(Quad, -0.5f, -0.5f, (float)nWidth, (float)nHeight);
	DrawPrimSetRGBA(Quad, 0xFF, 0xFF, 0xFF, 0xFF);
	LTVector2 vHalfTexel(0.5f / (float)nWidth, 0.5f / (float)nHeight);
	SetupQuadUVs(Quad, g_hDemoTexture, -vHalfTexel.x, -vHalfTexel.y, 1.0f, 1.0f);

	g_pLTClient->GetDrawPrim()->SetTexture(g_hDemoTexture);
	g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);
	
	g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);

	g_pLTClient->RenderConsoleToRenderTarget();
    g_pLTClient->GetRenderer()->End3D();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnMessage()
//
//	PURPOSE:	Handle interface messages
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
	{
		return false;
	}

	switch(messageID)
	{
	case MID_PLAYER_INFOCHANGE :
		{
			UpdatePlayerStats( *pMsg );

			return true;
		}
		break;

	case MID_PLAYER_INTERFACE :
		{
			UpdatePlayerInterface( *pMsg );

			return true;
		}
		break;


		case MID_PLAYER_SUMMARY :
		{
            uint32 nID = pMsg->Readuint32();
            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);


			if (!IsMultiplayerGameClient() || nLocalID == nID)
			{
				GetPlayerStats( )->UpdateMissionStats(pMsg);
			}
			else
			{
				CLIENT_INFO *pClient = m_ClientInfo.GetClientByID(nID);
				if( pClient )
					pClient->sStats.ReadData(pMsg);

			}

            return true;
		}
		break;

		case MID_PLAYER_SCORE :
		{
            uint32 nID = pMsg->Readuint32();
			CLIENT_INFO *pClient = m_ClientInfo.GetClientByID(nID);
			if( pClient )
			{
				pClient->sScore.ReadData(pMsg);
				m_ClientInfo.UpdateClientSort(pClient);
			}


			return true;
		}
		break;

		case MID_DECISION :
		{
			g_pDecision->Show(pMsg);
            return true;
		}
		break;

		case MID_PLAYER_TEAM :
		{
			
			g_pGameMsgs->AddMessage(LoadString("IDS_IWILLCHANGE"));
            return true;
		}
		break;


		case MID_PLAYER_INFO:
		{
			// Only do something if we're in multiplayer...

			if (!IsMultiplayerGameClient()) break;

			wchar_t szName[MAX_PLAYER_NAME];
            pMsg->ReadWString( szName, LTARRAYSIZE( szName ));

			// Get the insignia.  Only sent as a file title to reduce bandwidth, so we have to recreate the full path.
			char szPatchTitle[MAX_PATH] = "";
			pMsg->ReadString(szPatchTitle,LTARRAYSIZE(szPatchTitle));
			char szPatchDir[MAX_PATH] = "";
			g_pModelsDB->GetInsigniaFolder( szPatchDir, LTARRAYSIZE( szPatchDir ));
			char szPatchPath[MAX_PATH*2];
			LTSNPrintF( szPatchPath, LTARRAYSIZE( szPatchPath ), "%s%s.dds", szPatchDir, szPatchTitle );

            uint32 nID = pMsg->Readuint32();
			HOBJECT	hObj = pMsg->ReadObject();
			uint8 nMPModelIndex = pMsg->Readuint8( );
			uint8	nTeamId = pMsg->Readuint8();
			bool bIsAdmin = pMsg->Readbool( );
            uint8 nInfoType = pMsg->Readuint8();
            uint32 nLocalID = 0;
			g_pLTClient->GetLocalClientID (&nLocalID);

			bool bChangedTeams = false;
			uint8 nOldTeam = INVALID_TEAM;
			if (GameModeMgr::Instance( ).m_grbUseTeams)
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
					DebugCPrint(0,"MID_PLAYER_INFO:MID_PI_JOIN - Adding client %d (%S)", nID, szName);
					m_ClientInfo.AddClient(szName, szPatchPath, bIsAdmin, nID, nTeamId);
					if( nLocalID != nID )
					{
						wchar_t wszBuffer[128];
						FormatString( "IDS_JOINEDGAME", wszBuffer, LTARRAYSIZE(wszBuffer), szName );
						g_pGameMsgs->AddMessage(wszBuffer);		
					}
				} break;
			case MID_PI_EXIST:
				{
					//we are joining and this is info about a player already in the game
					DebugCPrint(0,"MID_PLAYER_INFO:MID_PI_EXIST - Adding client %d (%S)", nID, szName);
					m_ClientInfo.AddClient(szName, szPatchPath, bIsAdmin, nID, nTeamId);
				} break;
			case MID_PI_UPDATE:
				{
					//this is an update about a player already in the game
					DebugCPrint(1,"MID_PLAYER_INFO:MID_PI_UPDATE - Updating client %d (%S)", nID, szName);
					m_ClientInfo.UpdateClient(szName, szPatchPath, bIsAdmin, nID, nTeamId);

					//if this is a team game, and this is our player's update, update our team info
					if (GameModeMgr::Instance( ).m_grbUseTeams )
					{
						if(nLocalID == nID)
						{
							g_pClientConnectionMgr->SelectTeam(nTeamId,false);
						}

						//if we changed teams, we have to clean up our radar
						if (bChangedTeams)
						{
							HandlePlayerTeamChange();
							
							if (nTeamId != INVALID_TEAM && nOldTeam != INVALID_TEAM)
							{
								wchar_t wszTmp[128] = L"";
								
								const wchar_t* pwszTeam = L"";
								CTeam* pTeam = CTeamMgr::Instance().GetTeam(nTeamId);
								if( pTeam )
								{
									pwszTeam = pTeam->GetName();
								}

								if(nLocalID == nID)
								{
									FormatString("IDS_ICHANGED", wszTmp, LTARRAYSIZE(wszTmp), pwszTeam);
									//removed from chat queue
									//g_pChatMsgs->AddMessage(wszTmp);

								}
								else
								{
									FormatString("IDS_HECHANGED", wszTmp, LTARRAYSIZE(wszTmp), szName, pwszTeam);
								}

								g_pGameMsgs->AddMessage(wszTmp);

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
					if( !pCFX->IsUsingMPModel( nMPModelIndex ))
						pCFX->ChangeMPModel(nMPModelIndex);
				}
			}

			GetHUDMgr()->QueueUpdate( kHUDPlayers );

            return true;
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

			if (!IsMultiplayerGameClient()) break;

            uint32 nLocalID = 0;
            g_pLTClient->GetLocalClientID (&nLocalID);

            uint32 nID = pMsg->Readuint32();

			//if it's us we know we're leaving
			if (nID == nLocalID)
				return true;

			//if we don't know who it is, let's not display our ignorance...
			if (!m_ClientInfo.GetClientByID(nID,false))
				return true;

			wchar_t wszBuffer[128];
			FormatString( "IDS_LEFTGAME", wszBuffer, LTARRAYSIZE(wszBuffer), m_ClientInfo.GetPlayerName(nID) );

            g_pGameMsgs->AddMessage( wszBuffer );

			m_ClientInfo.RemoveClient(nID);
			UnmutePlayer(nID);
			CScreenMutePlayer* pScreen = (CScreenMutePlayer*)GetScreenMgr()->GetScreenFromID(SCREEN_ID_MUTE);
			if (pScreen)
			{
				pScreen->UpdateLists();
			}
			GetHUDMgr()->QueueUpdate( kHUDPlayers );

            return true;
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
				pClient = m_ClientInfo.GetClientByID(id,false);
				if( pClient )
					pClient->nPing = ping;
			}

            return true;
		}
		break;

		case MID_PLAYER_SCORED:
		{
			HandlePlayerScoredMessage(pMsg);
            return true;
		}
		break;


		case MID_PLAYER_MESSAGE :
		case MID_PLAYER_GHOSTMESSAGE :
		{
			// retrieve the string from the message, play the chat sound, and display the message

			wchar_t szMessage[256];
			pMsg->ReadWString(szMessage, LTARRAYSIZE(szMessage));
			uint32 nClientID = pMsg->Readuint32();
			uint8 nTeam =  pMsg->Readuint8();

			return HandleChat(szMessage,nClientID,nTeam);
		}
		break;

		case MID_PLAYER_TEXT :
		{
			char szCredits[256];
			pMsg->ReadString(szCredits,LTARRAYSIZE(szCredits));

			if (!szCredits[0] || LTStrIEquals(szCredits,"Off"))
			{
				Credits::Instance().StopAll();
			}
			else
			{
				Credits::Instance().Start(szCredits);
			}
		}
		break;

		case MID_PLAYER_EVENT:
		{
			return HandlePlayerEventMessage(pMsg);
		}
		break;

		case MID_LOAD_FAILED:
		{
			LoadFailed();
			return false; 
		}
		break;

		case MID_PICKUPITEM_ACTIVATE:
		{
			bool bSuccess = pMsg->Readbool();
			if (!bSuccess)
			{
				g_pClientSoundMgr->PlayInterfaceDBSound("PickupFailed");
			
			}
			return true; 
		}
		break;


		default : break;
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::OnEvent(uint32 dwEventID, uint32 dwParam)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	switch (dwEventID)
	{
		// Called when the renderer has switched into
		// the new mode but before it reloads all the textures
		// so you can display a loading screen.

		case LTEVENT_RENDERALMOSTINITTED :
		{
			if (m_bSwitchingModes)
			{
				m_InterfaceResMgr.DrawMessage("IDS_REINITIALIZING_RENDERER");

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
			m_LoadingScreen.ClearContentDownloadInfo();
			// Clean up our clientinfo's if we're in mp.
			if ( IsMultiplayerGameClient( ) )
			{
				m_ClientInfo.RemoveAllClients();
			}

			if ( !m_bIntentionalDisconnect )
			{
				Disconnected(dwParam);
			}
		
			m_bIntentionalDisconnect = false;

			//we've disconnected so we don't need these notifications
			m_bNotifyTeamSizeBalanced = false;
			m_bNotifyTeamScoreBalanced = false;

			m_setMutedClients.clear();


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
				m_LoadingScreen.Show(false);
			}

			CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
			while (iter != CCharacterFX::GetCharFXList( ).end( ) ) 
			{
				(*iter)->ResetDisplays();
				iter++;
			}
		}
		break;

		// The renderer is being shutdown.  This happens when
		// ShutdownRender is called or if the app loses focus.

		case  LTEVENT_RENDERTERM :
		{
			// Stop drawing the loading screen
			if (m_LoadingScreen.IsVisible())
			{
				m_LoadingScreen.Pause();
			}
		}
		break;

        case LTEVENT_LOSTFOCUS:
			{
				m_CursorMgr.Term();
			}
		break;

		case LTEVENT_GAINEDFOCUS:
		{
			m_CursorMgr.Init();
		}
		break;

		default :
		{
			SwitchToScreen(SCREEN_ID_MAIN);
		}
		break;
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePlayerStats()
//
//	PURPOSE:	Update the player's stats
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePlayerStats(ILTMessage_Read& msg )
{
    uint8 nId    = msg.Readuint8();

	switch (nId)
	{
		case IC_WEAPON_OBTAIN_ID :
		{
			HWEAPON	hWeapon	= msg.ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
			HAMMO	hAmmo	= msg.ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
			uint32	nAmount	= msg.Readuint32();
			uint8	nSlot	= msg.Readuint8();

			GetPlayerStats( )->UpdateAmmo( hWeapon, hAmmo, nAmount, true, false, nSlot );
/*
			DebugCPrint(0,"CInterfaceMgr::UpdatePlayerStats() : IC_WEAPON_OBTAIN_ID");
			DebugCPrint(0,"    Weapon                         : %s", g_pWeaponDB->GetRecordName(hWeapon));
			DebugCPrint(0,"    Ammo                           : %s", g_pWeaponDB->GetRecordName(hAmmo));
			DebugCPrint(0,"    Amount                         : %d", nAmount);
			DebugCPrint(0,"    Slot                           : %d", nSlot);
*/
		}
		break;

		case IC_WEAPON_REMOVE_ID :
		{
			HWEAPON	hWeapon	= msg.ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
/*
			DebugCPrint(0,"CInterfaceMgr::UpdatePlayerStats() : IC_WEAPON_REMOVE_ID");
			DebugCPrint(0,"    Weapon                         : %s", g_pWeaponDB->GetRecordName(hWeapon));
*/
			GetPlayerStats( )->RemoveWeapon( hWeapon );
		}
		break;


		case IC_WEAPON_PICKUP_ID :
		{
			HWEAPON	hWeapon	= msg.ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
			HAMMO	hAmmo	= msg.ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
			uint32	nAmount	= msg.Readuint32();
			uint8	nSlot	= msg.Readuint8();

/*
 	
			DebugCPrint(0,"CInterfaceMgr::UpdatePlayerStats() : IC_WEAPON_PICKUP_ID");
			DebugCPrint(0,"    Weapon                         : %s", g_pWeaponDB->GetRecordName(hWeapon));
			DebugCPrint(0,"    Ammo                           : %s", g_pWeaponDB->GetRecordName(hAmmo));
			DebugCPrint(0,"    Amount                         : %d", nAmount);
			DebugCPrint(0,"    Slot                           : %d", nSlot);
*/
			GetPlayerStats( )->UpdateAmmo( hWeapon, hAmmo, nAmount, true, true, nSlot );
		}
		break;

		case IC_MOD_PICKUP_ID :
		{
			bool	bPickedUp	= msg.Readbool();
			HMOD	hMod		= msg.ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetModsCategory() );
			bool	bDisplayMsg	= msg.Readbool();

			if (bPickedUp)
			{
				GetPlayerStats( )->UpdateMod( hMod );
			}
		}
		break;

		case IC_DEFAULTWEAPON_ID :
		{
			HWEAPON	hWeapon	= msg.ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );

			CClientWeaponMgr* pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			pClientWeaponMgr->SetDefaultWeapon( hWeapon );
		}
		break;

		case IC_MAX_HEALTH_ID :
		{
			uint8 nType1	= msg.Readuint8();
			uint8 nType2	= msg.Readuint8();
			float fAmount	= msg.Readfloat();

			GetPlayerStats( )->UpdateMaxHealth((uint32)fAmount);
		}
		break;

		case IC_MAX_ARMOR_ID :
		{
			uint8 nType1	= msg.Readuint8();
			uint8 nType2	= msg.Readuint8();
			float fAmount	= msg.Readfloat();

            GetPlayerStats( )->UpdateMaxArmor((uint32)fAmount);
		}
		break;

		case IC_OUTOFAMMO_ID :
		{
			uint8 nType1	= msg.Readuint8();
			uint8 nType2	= msg.Readuint8();
			float fAmount	= msg.Readfloat();

		}
		break;

		case IC_FADE_SCREEN_ID :
		{
			uint8 nType1	= msg.Readuint8();
			m_bOverrideInitialFade = msg.Readbool();
			float fAmount	= msg.Readfloat();

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
			uint8 nType1	= msg.Readuint8();
			uint8 nType2	= msg.Readuint8();
			float fAmount	= msg.Readfloat();

			GetPlayerStats( )->DropInventory(!!nType1);
		}
		break;


		case IC_MISSION_FAILED_ID :
		{
			uint8 nType1		= msg.Readuint8();
			uint8 nType2		= msg.Readuint8();
			uint32 dwStringID	= msg.Readuint32();

			MissionFailed( StringIDFromIndex(dwStringID) );
		}
		break;

		case IC_WEAPONCAP_ID:
		{
			uint8 nCap = msg.Readuint8();
			GetPlayerStats()->SetWeaponCapacity(nCap);
		} break;


		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePlayerStats()
//
//	PURPOSE:	Update the player's stats
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePlayerInterface(ILTMessage_Read& msg )
{
	if( msg.Readbool( ))
	{
		bool bForceReset = msg.Readbool( );
		uint8 nNumAmmo = g_pWeaponDB->GetNumAmmo();

		// If forcing reset, change all ammo to 0, then read in the non-zero's
		if( bForceReset )
		{
			for( uint8 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
			{
				HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( nAmmo );
				// Skip it if it doesn't have ammo data for us.
				HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo, false);
				if( !hAmmoData )
					continue;
				GetPlayerStats( )->UpdateAmmo( NULL, hAmmo, 0, false, false, WDB_INVALID_WEAPON_INDEX );
			}
		}

		// Read each ammo detail written.
		uint8 nNumAmmoWritten = msg.Readuint8( );
		for( uint8 nAmmo = 0; nAmmo < nNumAmmoWritten; nAmmo++ )
		{
			uint8 nAmmoIndex = msg.Readuint8( );
			HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( nAmmoIndex );
			bool bReadAmmo = msg.Readbool();
			uint32 nAmount = 0;
			if( bReadAmmo )
				nAmount = msg.Readuint16( );
			GetPlayerStats( )->UpdateAmmo( NULL, hAmmo, nAmount, false, true, WDB_INVALID_WEAPON_INDEX );
		}
	}

	if( msg.Readbool())
	{
		uint16 nVal = msg.Readuint16();
		GetPlayerStats( )->UpdateHealth(nVal);
	}

	if( msg.Readbool())
	{
		uint16 nVal = msg.Readuint16();
		GetPlayerStats( )->UpdateArmor(nVal);
	}

	if( msg.Readbool())
	{
		float fAmount	= msg.Readuint8() / 255.0f;
		GetPlayerStats( )->UpdateAir(fAmount);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnCommandOn()
//
//	PURPOSE:	Handle command on
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::OnCommandOn(int command)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	if( g_pChatInput )
	{
		if (g_pChatInput->IsVisible())
			return true;
	}
	if( g_pDebugInput )
	{
		if (g_pDebugInput->IsVisible())
			return true;
	}

	if (g_pRadio && g_pRadio->IsVisible())
	{
		if (command >= COMMAND_ID_CHOOSE_1 &&
			command <= COMMAND_ID_CHOOSE_9 )
		{
			uint8 nChoice = command - COMMAND_ID_CHOOSE_1;
			if (g_pRadio->Choose(nChoice))
				return true;
		} 
		if (command == COMMAND_ID_CHOOSE_0 )
		{
			if (g_pRadio->Choose(COMMAND_ID_CHOOSE_0))
				return true;
		} 
	}
     
	// Take appropriate action
	if (g_pDecision && g_pDecision->IsVisible())
	{
		if (command >= COMMAND_ID_CHOOSE_1 &&
			command <= COMMAND_ID_CHOOSE_6 )
		{
			uint8 nChoice = command - COMMAND_ID_CHOOSE_1;
			g_pDecision->Choose(nChoice);
			return true;
		}
	}

	if (m_UserMenuMgr.OnCommandOn(command))
			return true;

	if( m_SystemMenuMgr.OnCommandOn( command ))
		return true;

	switch (command)
	{
		case COMMAND_ID_DEBUGMSG :
		{
			if( g_pDebugInput )
			{
				g_pDebugInput->Show(true );
			}

		}
		break;

		case COMMAND_ID_MESSAGE :
		case COMMAND_ID_TEAM_MESSAGE :
		{
			if (!GameModeMgr::Instance( ).m_grbUseTeams && (command == COMMAND_ID_TEAM_MESSAGE))
				break;

			if (m_eGameState == GS_PLAYING && 
				!FadingScreen() && 
				g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() != CPlayerCamera::kCM_Cinematic &&
				!g_pPlayerMgr->InStoryMode())
			{
				if( g_pChatInput )
				{
					g_pChatInput->Show(true, (command == COMMAND_ID_TEAM_MESSAGE) );
				}

				if( g_pScores )
				{
					g_pScores->Show(false);
				}
			}

			return true;
		}
		break;

		case COMMAND_ID_RADIO :
		{
			if (m_eGameState == GS_PLAYING &&  GameModeMgr::Instance( ).m_grbUseTeams && g_pPlayerMgr->IsPlayerAlive())
			{
				// [KLS 9/9/02] Made toggle and added sounds...
				if( g_pRadio )
				{
					bool bWasVisible = g_pRadio->IsVisible();
					g_pRadio->Show(!bWasVisible);

					if (g_pRadio->IsVisible())
					{
						g_pClientSoundMgr->PlayInterfaceDBSound("RadioOn");
					}
					else
					{
						if (bWasVisible)
						{
							g_pClientSoundMgr->PlayInterfaceDBSound("RadioOff");
						}
						else
						{
							g_pClientSoundMgr->PlayInterfaceDBSound("RadioFailed");
						}
						
					}
				}
			}

			return true;
		}
		break;

		case COMMAND_ID_MEDKIT :
		{
			if (!g_pPlayerMgr->IsPlayerAlive())
				return false;
			HGEAR hMedKit = g_pWeaponDB->GetGearRecord("MedKit");
			if (!hMedKit || g_pPlayerStats->GetGearCount(hMedKit) == 0)
				return false;

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_GEAR);
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hMedKit );
			cMsg.Writeuint8(kGearUse);
		
			return (LT_OK == g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED));
		}
		break;

		case COMMAND_ID_LASTWEAPON :
		{
			CClientWeaponMgr const *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
			CClientWeapon const *pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
			if (pClientWeapon && !pClientWeaponMgr->WeaponsEnabled()) return true;

            return true;

		}
		break;

		case COMMAND_ID_PREV_WEAPON :
		{
			PreviousWeapon();
            return true;
		}
		break;

		case COMMAND_ID_NEXT_WEAPON	:
		{
			NextWeapon();
			return true;
		} break;

		case COMMAND_ID_NEXT_GRENADE:
		{
			GetPlayerStats()->NextGrenade();
			return true;
		} break;

		case COMMAND_ID_STATUS:
			{
				g_pHUDMgr->ResetAllFades();
				if (m_eGameState == GS_PLAYING && IsMultiplayerGameClient() )
				{
					// Don't show score screen if we're ending the round.
					if( g_pMissionMgr->GetServerGameState() != EServerGameState_ShowScore && 
						g_pMissionMgr->GetServerGameState() != EServerGameState_ExitingLevel )
						g_pScores->Show(true);
				};
				return true;
			}
			break;

		case COMMAND_ID_MISSION:
			{
				if (m_eGameState == GS_PLAYING && !IsMultiplayerGameClient() )
				{
					if (GetPlayerStats()->GetObjective() || !LTStrEmpty(GetPlayerStats()->GetMission()))
					{
						GetUserMenuMgr().SetCurrentMenu(MENU_ID_MISSION);
					}
					else
					{
						RequestInterfaceSound(IS_NO_SELECT);
					}
					
				};
				return true;
			}
			break;

		case COMMAND_ID_TOGGLE_NAVMARKER:
			{
				if (m_eGameState == GS_PLAYING && IsMultiplayerGameClient() )
				{
					bool bNavFilter = !g_pNavMarkerMgr->MultiplayerFilter();
					//don't save this now, since it could have a framerate hit from disk access
					// instead set a flag to save it later
					g_bNavMarkersToggled = true;

					if (g_pNavMarkerMgr)
					{
						g_pNavMarkerMgr->SetMultiplayerFilter(bNavFilter);
					}
				};
				return true;
			}
			break;

		default :
		break;
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::OnCommandOff(int command)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	// Only process if not editing a message...

	if( g_pChatInput )
	{
		if (g_pChatInput->IsVisible()) return true;
	}
	if( g_pDebugInput )
	{
		if (g_pDebugInput->IsVisible()) return true;
	}

	//we need to restore the normal bindings at this point
	if (g_pRadio && !g_pRadio->IsVisible() && g_pRadio->AreBindingsSet())
	{
		g_pRadio->SetBindings(false);
	}

	if (m_UserMenuMgr.OnCommandOff(command))
		return true;

	if( m_SystemMenuMgr.OnCommandOff( command ))
		return true;

	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		{
            return false;
		}
		break;

		case COMMAND_ID_LASTWEAPON :
		{
			ASSERT( 0 != g_pPlayerMgr );
			g_pClientWeaponMgr->LastWeapon( );
			return true;
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

		case COMMAND_ID_STATUS:
		{
			if( g_pScores )
			{
				g_pScores->Show(false);
			}
		} break;

		default : break;
	}

	return false;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyDown()
//
//	PURPOSE:	Handle OnKeyDown messages
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::OnKeyDown(int key, int rep)
{
	
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// Make sure we're initialized.
	if ( !IsInitialized( ) )
	{
		return false;
	}

	//handle stuff before default handling
	if( g_pRadio )
	{
		if (g_pRadio->IsVisible() && m_eGameState == GS_PLAYING && key == VK_ESCAPE)
		{
			g_pRadio->Show(false);
			g_pClientSoundMgr->PlayInterfaceDBSound("RadioOff");
			return true;
		}
	}

	if (m_MessageBox.IsVisible() && m_eGameState != GS_LOADINGLEVEL)
	{
		return m_MessageBox.HandleKeyDown(key,rep);
	}

	// Are we entering a Message
	if( g_pChatInput )
	{
		if (g_pChatInput->IsVisible())
		{
			g_pChatInput->HandleKeyDown(key, rep);
			return true;
		}
	}
	if( g_pDebugInput )
	{
		if (g_pDebugInput->IsVisible())
		{
			g_pDebugInput->HandleKeyDown(key, rep);
			return true;
		}
	}


	if( m_UserMenuMgr.HandleKeyDown(key,rep))
		return true;
	if( m_SystemMenuMgr.HandleKeyDown(key,rep))
		return true;

	switch (m_eGameState)
	{

		case GS_SCREEN :
		{
			GetScreenMgr( )->HandleKeyDown(key,rep);
			return true;
		}
		break;
		
		case GS_LOADINGLEVEL :
		{
			if (key == VK_ESCAPE)
			{
				// allow the user to abort during the initial connection, but not once the
				// world starts loading.
				if (g_pClientConnectionMgr->IsConnectedToRemoteServer() 
					&& g_pClientConnectionMgr->IsConnectionInProgress()
					&& m_LoadingScreen.IsVisible())
				{
					LoadFailed(SCREEN_ID_MULTI, NULL, false);
				}
			}
			return true;
		}
		break;

		case GS_PAUSED :
		{
			if( HandleDebugKey(key) )
			{
				return true;
			}

			// They pressed a key - unpause the game

			ChangeState(GS_PLAYING);
            return true;
		}
		break;

		case GS_SPLASHSCREEN :
		{
			// They pressed a key - end splash screen...
			double fCurTime = LTTimeUtils::GetTimeS();
			if ( fCurTime > g_fSplashSndMinEndTime )
			{
				EndSplashScreen( );
			}
			return true;
		}
		break;

		case GS_MOVIE :
		{
			// They pressed a key - end movie...
			NextMovie( LTProfileUtils::ReadUint32( "Game", "GameRuns", 0, g_pVersionMgr->GetGameSystemIniFile()) > 1 );
            return true;
		}
		break;

		case GS_DEMOSCREEN :
		{
			// They pressed a key - go to next screen...
			NextDemoScreen();
            return true;
		}
		break;

		// ABM TODO write handlers for the subroutine and ratings states

		case GS_PLAYING :
		{
			switch (key)
			{
				case VK_PAUSE:
				{
					if (IsMultiplayerGameClient()) return false;

					if (!g_pGameClientShell->IsGamePaused())
					{
						ChangeState(GS_PAUSED);
					}

					g_pGameClientShell->PauseGame(!g_pGameClientShell->IsGamePaused(), true);
					return true;
				}
				break;

				// Escape Key Handling
				case VK_ESCAPE:
				{
					bool bHandled = false;

					if( g_pDecision )
					{
						if (g_pDecision->IsVisible())
						{
							g_pDecision->Choose(MAX_DECISION_CHOICES);
							bHandled = true;
						}
					}

					if (bHandled)
					{
						return true;
					}
				}
				break;

				default : break;
			}

			if ( HandleDebugKey(key) )
			{
				return true;
			}

		}
		break;

		default : break;
	}

#endif // !PLATFORM_XENON


	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnKeyUp(int key)
//
//	PURPOSE:	Handle key up notification
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::OnKeyUp(int key)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	// if it's the tilde (~) key then the console has been turned off
	// (this is the only event that causes this key to ever get processed)
	// so clear the back buffer to get rid of any part of the console still showing
	if (key == VK_TILDE)
	{
        return true;
	}

	if( g_pChatInput )
	{
		if (g_pChatInput->IsVisible())
		{
			return true;
		}
	}
	if( g_pDebugInput )
	{
		if (g_pDebugInput->IsVisible())
		{
			return true;
		}
	}

	if( m_UserMenuMgr.HandleKeyUp(key))
		return true;
	if( m_SystemMenuMgr.HandleKeyUp(key))
		return true;

	switch (m_eGameState)
	{
	case GS_SCREEN:
		{
			GetScreenMgr( )->HandleKeyUp(key);
			return true;
		}
		break;
	case GS_LOADINGLEVEL :
		{
            return true;
		}
		break;
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::OnChar()
//
//	PURPOSE:	Handle OnChar messages
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::OnChar(wchar_t c)
{
	if (c < L' ') return;

	if (m_MessageBox.IsVisible())
	{
		m_MessageBox.HandleChar(c);
		return;
	}

	if( g_pChatInput )
	{
		if (g_pChatInput->IsVisible())
		{
			g_pChatInput->HandleChar(c);
			return;
		}
	}
	if( g_pDebugInput )
	{
		if (g_pDebugInput->IsVisible())
		{
			g_pDebugInput->HandleChar(c);
			return;
		}
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

bool CInterfaceMgr::Draw()
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	//render our overlays
	g_pGameClientShell->GetSimulationTimeClientFXMgr().RenderOverlays();
	g_pGameClientShell->GetRealTimeClientFXMgr().RenderOverlays();
	GetInterfaceFXMgr().RenderOverlays();

	// Tint the screen if necessary...

	// Check for interface drawing override...

	if (g_vtDrawInterface.GetFloat())
	{
		// Update letter box...

		UpdateLetterBox();

		if (GetGameState() == GS_PLAYING )
		{
			// Draw the player stats (health,armor,ammo) if appropriate...
			Credits::Instance().Render();
			GetHUDMgr()->Render( eHUDRenderLayer_Back );
		}


		if (!IsMultiplayerGameClient())
		{
			if (GetGameState() == GS_PLAYING || GetGameState() == GS_MENU)
			{
				g_pPaused->Render();
			}
		}


		// Update the screen fade alpha only if we are in the playing state and not paused...

		bool bUpdateAlpha = (GetGameState() == GS_PLAYING ? !g_pGameClientShell->IsGamePaused() : false);

		UpdateScreenFade(bUpdateAlpha);
	}

	// Render the systemmenumgr before the usermenumgr, so the usermenumgr is rendered on top.
	m_SystemMenuMgr.Render();
	m_UserMenuMgr.Render();

	GetHUDMgr()->Render( eHUDRenderLayer_Front );

	//this should be last so it is always on top.
	if (m_MessageBox.IsVisible())
	{
		if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
		{
			m_MessageBox.Draw();
			g_pLTClient->RenderConsoleToRenderTarget();
			g_pLTClient->GetRenderer()->End3D();
		}
	}

	m_CursorMgr.Update();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreScreenState()
//
//	PURPOSE:	Initialize the Screen state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PreScreenState(GameState eCurState, eScreenID screenID)
{
    if (eCurState == GS_SCREEN) return false;

	m_InterfaceResMgr.Setup();

	// Pause the game...
    g_pGameClientShell->PauseGame(true, true);

	// Disable light scaling when not in the playing state...
	g_pGameClientShell->GetLightScaleMgr()->Disable();
	g_pGameClientShell->GetScreenTintMgr()->ClearAll();

	CreateInterfaceBackground();

	// No fog in the menus...
	g_bInGameFogEnabled = !!GetConsoleInt("FogEnable", 1);
	WriteConsoleInt("FogEnable", 0);

	m_bUseInterfaceCamera = true;

	if (eCurState == GS_LOADINGLEVEL) 
	{
		if (!OverrideInitialFade())
		{
			AbortScreenFade();
		}
	}
	else
	{
		ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
	}

	// KLS 1/31/05 - if we're in the process of loading a level we don't want to play any
	// videos or music (which get created by the screen mgr when entering a screen)...
	// 
	// NOTE we may want to add checks for SCREEN_ID_FAILURE and SCREEN_ID_END_MISSION here 
	// as well...

	bool bCreateMedia = bool((SCREEN_ID_POSTLOAD != screenID) && (SCREEN_ID_PRELOAD != screenID));

	GetScreenMgr( )->EnterScreens(bCreateMedia);

    return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostScreenState()
//
//	PURPOSE:	Handle leaving the Screen state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostScreenState(GameState eNewState, eScreenID screenID)
{
    if (eNewState == GS_SCREEN) return false;

	GetScreenMgr( )->ExitScreens();

	if (eNewState != GS_LOADINGLEVEL && eNewState != GS_DEMOSCREEN)
	{
		int nGameMode = GAMEMODE_NONE;
        g_pLTClient->GetGameMode(&nGameMode);
        if (nGameMode == GAMEMODE_NONE) return false;
	}

	RemoveAllInterfaceFX();


	//see what state we need to restore the HUD into (in case we go into the menus
	//while we are dead)
	g_pHUDMgr->UpdateRenderLevel();


	CBindMgr::GetSingleton().ClearAllCommands();


	// Reset fog value...

	WriteConsoleInt("FogEnable", (int) g_bInGameFogEnabled);

	m_bSuppressNextFlip = true;

	// Match the pause from PreScreeState...

    g_pGameClientShell->PauseGame( false );

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePauseState()
//
//	PURPOSE:	Initialize the Pause state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PrePauseState(GameState eCurState)
{
    if (eCurState == GS_PAUSED) return false;

	CreateInterfaceBackground();

	// Create the "paused" surface...
	const char* pszImage = ClientDB::Instance().GetString( ClientDB::Instance().GetClientSharedRecord(), CDB_PauseScreenImage );
	m_hGamePausedTex.Load(pszImage);

	m_bUseInterfaceCamera = true;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPauseState()
//
//	PURPOSE:	Handle leaving the Pause state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostPauseState(GameState eNewState)
{
    if (eNewState == GS_PAUSED) return false;

	// Create the "paused" surface...
	m_hGamePausedTex.Free();

	RemoveAllInterfaceFX();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PrePlayingState()
//
//	PURPOSE:	Initialize the Playing state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PrePlayingState(GameState eCurState)
{
    if (eCurState == GS_PLAYING) return false;

	if (eCurState != GS_MENU)
	{
		// Unpause the game...

        g_pGameClientShell->PauseGame(false);
	}

	// Clear whatever input got us here...
	CBindMgr::GetSingleton().ClearAllCommands();

	// Eanble light scaling...

	g_pGameClientShell->GetLightScaleMgr()->Enable();
	g_pPlayerMgr->RestorePlayerModes();

	//HACK
	//g_pMissionText->Pause(false);

    m_bUseInterfaceCamera = false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostPlayingState()
//
//	PURPOSE:	Handle leaving the Playing state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostPlayingState(GameState eNewState)
{
    if (eNewState == GS_PLAYING) return false;

	g_pGameClientShell->GetScreenTintMgr()->ClearAll();
	g_pGameClientShell->GetLightScaleMgr()->ClearAllLightScales();

    //HACK
	//g_pMissionText->Pause(true);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreMenuState()
//
//	PURPOSE:	Initialize the menu state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PreMenuState(GameState eCurState)
{
    if (eCurState == GS_MENU) return false;

	if (!m_UserMenuMgr.GetCurrentMenu())
	{
		if (!m_UserMenuMgr.GetLastMenu())
			return false;

		m_UserMenuMgr.SetCurrentMenu(m_UserMenuMgr.GetLastMenu()->GetMenuID());
	}

	m_bUseInterfaceCamera = false;

	// [KLS 7/28/02] Shouldn't need to fade in for the menu, screen fade
	// is drawn behind the menu...
	// ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

	// Pause the game...
    g_pGameClientShell->PauseGame(true, true);
	g_pGameClientShell->SetInputState(true);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostMenuState()
//
//	PURPOSE:	Handle leaving the menu state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostMenuState(GameState eNewState)
{
    if (eNewState == GS_MENU) return false;

	m_UserMenuMgr.ExitMenus();

	// Unpause the game...
    g_pGameClientShell->PauseGame(false);
	CBindMgr::GetSingleton().ClearAllCommands();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreLoadingLevelState()
//
//	PURPOSE:	Initialize the LoadingLevel state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PreLoadingLevelState(GameState eCurState)
{
    if (eCurState == GS_LOADINGLEVEL) return false;

	// Disable light scaling when not in the playing state...
	g_pGameClientShell->GetLightScaleMgr()->ClearAllLightScales();

	RemoveInterfaceFX();

	m_bUseInterfaceCamera = true;

	// Don't bother showing loading screen for simple round changes.
	if( IsMultiplayerGameClient( ) && !g_pMissionMgr->IsNewMission( ))
	{
	}
	else
	{
		m_LoadingScreen.Show(true);
	}
		
	// Turn off sound
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetVolume(0);
	g_pGameClientShell->GetMixer( )->ProcessMixerByName( "Loading", -1, false );

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostLoadingLevelState()
//
//	PURPOSE:	Handle leaving the LoadingLevel state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostLoadingLevelState(GameState eNewState)
{
    if (eNewState == GS_LOADINGLEVEL) return false;

	//once we're done loading, force the video memory to reset.
	m_bOptimizeVideoMemory = true;

	// Don't allow the loading state to go away until the loading screen has been hidden
	if (m_LoadingScreen.IsVisible())
		return false;

	RemoveAllInterfaceFX();

	// Turn back on the sound
	// Turn off sound
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetVolume(SOUND_BASE_VOL);
	
	// NOTE: This could be turned on here, but only if the didn't work (or we're
	// returning to the menu). Since we can't tell that, we'll just have to not
	// do anything. 
	// The Loading mixer is shut off in FirstUpdate in GameClientShell.. -- Terry
	//g_pGameClientShell->GetMixer( )->ProcessMixerByName( "Loading", -1, true );

	m_bLoadFailed = false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreExitingLevelState()
//
//	PURPOSE:	Initialize the ExitingLevel state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PreExitingLevelState(GameState eCurState)
{
    if (eCurState == GS_EXITINGLEVEL) return false;

	if (eCurState == GS_SCREEN && GetScreenMgr( )->GetCurrentScreenID() == SCREEN_ID_MAIN )
	{
		return false;
	}

	StartScreenFadeOut( g_vtExitLevelScreenFadeTime.GetFloat() );

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostExitingLevelState()
//
//	PURPOSE:	Handle leaving the ExitingLevel state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostExitingLevelState(GameState eNewState)
{
    if (eNewState == GS_EXITINGLEVEL) return false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreSplashScreenState()
//
//	PURPOSE:	Initialize the SplashScreen state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PreSplashScreenState(GameState eCurState)
{
    if (eCurState == GS_SPLASHSCREEN) return false;

	// Since we're going to always go to the menu state next, load the surfaces here...
	m_InterfaceResMgr.Setup();

	// Get some database information
	ClientDB& ClientDatabase = ClientDB::Instance();
	uint32 nSplashImages = ClientDatabase.GetNumValues( ClientDatabase.GetClientSharedRecord(), CDB_SplashScreenImage );
	uint32 nSplashSounds = ClientDatabase.GetNumValues( ClientDatabase.GetClientSharedRecord(), CDB_SplashScreenSound );

	if( nSplashImages > 0 )
	{
		// Create the splash screen image
		const char* sImage = ClientDatabase.GetString( ClientDatabase.GetClientSharedRecord(), CDB_SplashScreenImage, GetRandom( 0, nSplashImages - 1 ) );
		g_hSplashTex.Load( sImage );
	}

	//overlay FEAR Combat logo on top of the normal splash screen
	if (LTGameCfg::IsMPFreeProduct())
	{
		// Create the splash screen image
		const char* sImage = ClientDatabase.GetString( ClientDatabase.GetClientSharedRecord(), CDB_SplashScreenMPFreeOverlay, 0 );
		g_hMPFreeSplashTex.Load(  sImage );
	}


    if( !g_hSplashTex )
	{
		// Return true so we're not put in an invalid state...
		g_fSplashSndDuration = 0.0;
		return true;
	}

	// Play splash screen sound...
	if( nSplashSounds > 0 )
	{
	    uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		const char* sSound = ClientDatabase.GetString( ClientDatabase.GetClientSharedRecord(), CDB_SplashScreenSound, GetRandom( 0, nSplashSounds - 1 ) );

		m_hSplashSound = g_pClientSoundMgr->PlayInterfaceSound( sSound, dwFlags );

		if( m_hSplashSound )
		{
			g_pLTClient->SoundMgr()->GetSoundDuration( m_hSplashSound, g_fSplashSndDuration );

			DebugCPrint( 1,"Splash sound duration: %.4f", g_fSplashSndDuration );
			DebugCPrint( 1,"Current Time: %.4f", RealTimeTimer::Instance().GetTimerAccumulatedS() );
		}
	}

// Must show the openning splash screen for minimum time so user
// is sure to read the ESRB rating.
#ifdef _FINAL

	// Be sure the splash screen is visible for at least some minimum time while in demo mode.
	const float fMinimumSplashScreenTime = 4.0f;
	g_fSplashSndMinEndTime = LTTimeUtils::GetTimeS( ) + fMinimumSplashScreenTime;
	if (g_fSplashSndDuration < fMinimumSplashScreenTime + g_vtSplashScreenFadeIn.GetFloat())
	{
		g_fSplashSndDuration = fMinimumSplashScreenTime + g_vtSplashScreenFadeIn.GetFloat();
	}
#endif

	// Fade into the splash screen...
    g_pLTClient->GetRenderer()->ClearRenderTarget( CLEARRTARGET_ALL, 0 );

	StartScreenFadeIn( g_vtSplashScreenFadeIn.GetFloat() );
	m_bUseInterfaceCamera = true;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostSplashScreenState()
//
//	PURPOSE:	Handle leaving the SplashScreen state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostSplashScreenState(GameState eNewState)
{
    if (eNewState == GS_SPLASHSCREEN) return false;

	// Stop splash screen sound (if playing)...

	if (m_hSplashSound)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hSplashSound);
        m_hSplashSound = NULL;
	}

	g_hSplashTex.Free();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreMovieState()
//
//	PURPOSE:	Initialize the movie state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PreMovieState(GameState eCurState)
{
    if (eCurState == GS_MOVIE) return false;

	g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_ALL, 0);
	m_bUseInterfaceCamera = true;

	m_nCurMovie = 0;
	NextMovie();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostMovieState()
//
//	PURPOSE:	Handle leaving the movie state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostMovieState(GameState eNewState)
{
	if (m_hMovie)
	{
		g_pLTClient->GetVideoTexture()->ReleaseVideoTexture(m_hMovie);
		m_hMovie = NULL;
	}

    if (eNewState == GS_MOVIE) 
		return false;

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PreDemoScreenState()
//
//	PURPOSE:	Initialize the DemoScreen state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PreDemoScreenState(GameState eCurState)
{
    if (eCurState == GS_DEMOSCREEN) return false;

	g_nDemoScreen = 0;
	NextDemoScreen();

	m_bUseInterfaceCamera = true;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::PostDemoScreenState()
//
//	PURPOSE:	Handle leaving the DemoScreen state
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::PostDemoScreenState(GameState eNewState)
{
    if (eNewState == GS_DEMOSCREEN) return false;

    return true;
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

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("DisableSound");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_SOUND;
	}

    hVar = g_pLTClient->GetConsoleVariable("DisableMovies");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_MOVIES;
	}

    hVar = g_pLTClient->GetConsoleVariable("DisableFog");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_FOG;
	}

    hVar = g_pLTClient->GetConsoleVariable("DisableLines");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_LINES;
	}

    hVar = g_pLTClient->GetConsoleVariable("DisableTripBuf");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_TRIPLEBUFFER;
	}

    hVar = g_pLTClient->GetConsoleVariable("DisableTJuncs");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_TJUNCTIONS;
	}

    hVar = g_pLTClient->GetConsoleVariable("DisableSoundFilters");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_SOUNDFILTERS;
	}

    hVar = g_pLTClient->GetConsoleVariable("DisableHardwareSound");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwAdvancedOptions &= ~AO_HARDWARESOUND;
	}

	// Record the original state of sound

    hVar = g_pLTClient->GetConsoleVariable("SoundEnable");
    if (hVar && g_pLTClient->GetConsoleVariableFloat(hVar))
	{
		m_dwOrignallyEnabled |= AO_SOUND;
	}

	// Implement any advanced options here (before renderer is started)
	if ( !(m_dwAdvancedOptions & AO_SOUNDFILTERS) )
	{
        g_pLTClient->SetConsoleVariableFloat("SoundFilters", 0.0f);
	}
	else
	{
		// only set this on, if there isn't a current setting
		hVar = g_pLTClient->GetConsoleVariable("SoundFilters");
		if ( !hVar )
	        g_pLTClient->SetConsoleVariableFloat("SoundFilters", 1.0f);
	}

    hVar = g_pLTClient->GetConsoleVariable("SoundEnable");
	if (!hVar && (m_dwAdvancedOptions & AO_SOUND))
	{
        g_pLTClient->SetConsoleVariableFloat("SoundEnable", 1.0f);
	}

	if (!(m_dwAdvancedOptions & AO_TRIPLEBUFFER))
	{
        g_pLTClient->SetConsoleVariableFloat("BackBufferCount", 1.0f);
	}

	if (!(m_dwAdvancedOptions & AO_FOG))
	{
        g_pLTClient->SetConsoleVariableFloat("FogEnable", 0.0f);
	}

	if (!(m_dwAdvancedOptions & AO_CURSOR))
	{
        g_pLTClient->SetConsoleVariableFloat("HardwareCursor", 0.0f);
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

bool CInterfaceMgr::ChangeState(GameState eNewState, eScreenID screenID/*=SCREEN_ID_UNASSIGNED*/)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return false;

	DebugChangeState(eNewState);

	GameState eCurState = m_eGameState;

	// First make sure we change change to the new state from the the
	// state we are currently in...

	if (PreChangeState(eCurState, eNewState, screenID))
	{
		m_eGameState = eNewState;

		// Since the state changed, update the cursor
		UpdateCursorState();

		m_eLastGameState = eCurState;

        return true;

	}

    return false;
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

bool CInterfaceMgr::PreChangeState(GameState eCurState, GameState eNewState, 
								   eScreenID screenID)
{
	// Do any clean up of the old (current) state...

	switch (eCurState)
	{
		case GS_PLAYING :
		{
            if (!PostPlayingState(eNewState)) return false;
		}
		break;

		case GS_MENU :
		{
            if (!PostMenuState(eNewState)) return false;
		}
		break;

		case GS_EXITINGLEVEL:
		{
            if (!PostExitingLevelState(eNewState)) return false;
		}
		break;

		case GS_LOADINGLEVEL:
		{
            if (!PostLoadingLevelState(eNewState)) return false;
		}
		break;

		case GS_SCREEN :
		{
            if (!PostScreenState(eNewState, screenID)) return false;
		}
		break;

		case GS_PAUSED :
		{
            if (!PostPauseState(eNewState)) return false;
		}
		break;

		case GS_SPLASHSCREEN :
		{
            if (!PostSplashScreenState(eNewState)) return false;
		}
		break;

		case GS_MOVIE :
		{
            if (!PostMovieState(eNewState)) return false;
		}
		break;

		case GS_DEMOSCREEN :
		{
            if (!PostDemoScreenState(eNewState)) return false;
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
			return PreScreenState(eCurState, screenID);
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

    return true;
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

bool CInterfaceMgr::SwitchToScreen(eScreenID screenID)
{
	if (m_eGameState != GS_SCREEN)
	{
		if (((m_eGameState == GS_SPLASHSCREEN) || (m_eGameState == GS_DEMOSCREEN)) && (screenID == SCREEN_ID_MAIN))
		{
			StartScreenFadeIn(g_vtMainScreenFadeIn.GetFloat());
		}

        if (!ChangeState(GS_SCREEN, screenID)) return false;
	}

	GetScreenMgr( )->SetCurrentScreen(screenID);

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::ForceScreenUpdate
//
//	PURPOSE:	Force the current screen to update
//
// --------------------------------------------------------------------------- //

bool CInterfaceMgr::ForceScreenUpdate(eScreenID screenID)
{
    if (m_eGameState != GS_SCREEN) return false;

	return GetScreenMgr( )->ForceScreenUpdate(screenID);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::MissionFailed
//
//	PURPOSE:	Go to the mission failure state
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::MissionFailed(const char* szFailStringId)
{
	// The player is no longer in the world...
	g_pGameClientShell->SetWorldNotLoaded();

	m_szFailStringId = szFailStringId;
	SwitchToScreen(SCREEN_ID_FAILURE);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetViewportRect
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

LTRect2f CInterfaceMgr::GetViewportRect()
{
	// Adjusts the viewport dimenstions for certain video modes.
	// GetDefaultFOVAspectRatioScale for the corresponding FOV aspect changes.

#ifdef PROJECT_DARK
#ifdef PLATFORM_XENON
	if( !(XGetVideoFlags() & XC_VIDEO_FLAGS_WIDESCREEN) )	
		return LTRect2f(0.0f, 0.125f, 1.0f, 0.875f);		// add letter-box to non-widescreen modes
	else
		return LTRect2f(0.0f, 0.0f, 1.0f, 1.0f);			// don't letterbox widescreen mode
#else
	return LTRect2f(0.0f, 0.125f, 1.0f, 0.875f);			// allows LDs to preview how it will look on xenon
#endif
#endif
	return LTRect2f(0.0f, 0.0f, 1.0f, 1.0f);				// normal PC viewport
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::GetDefaultFOVAspectRatioScale
//
//	PURPOSE:	Adjusts the FOV aspect ratio scale
//
// --------------------------------------------------------------------------- //

float CInterfaceMgr::GetDefaultFOVAspectRatioScale()
{
	// Adjusts the FOV aspect ratio when letterboxing certain video modes.
	// See GetViewportRect() for corresponding viewport settings
	return 1.0f;
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
	m_UserMenuMgr.ScreenDimsChanged();
	m_SystemMenuMgr.ScreenDimsChanged();
	GetHUDMgr()->ScreenDimsChanged();

    g_pLTClient->SetCameraRect (m_hInterfaceCamera, g_pInterfaceMgr->GetViewportRect());

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
	}

	m_UserMenuMgr.OnLButtonUp(x,y);
	m_SystemMenuMgr.OnLButtonUp(x,y);
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

	m_UserMenuMgr.OnLButtonDown(x,y);
	m_SystemMenuMgr.OnLButtonDown(x,y);

	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnLButtonDown(x,y);
		}	
		break;

		case GS_SPLASHSCREEN:
		{
			// They pressed a mouse button - end splash screen...
			double fCurTime = LTTimeUtils::GetTimeS();
			if ( fCurTime > g_fSplashSndMinEndTime )
			{
				EndSplashScreen( );
			}
		}	
		break;

		case GS_MOVIE:
		{
			// They pressed a mouse button - next movie
			NextMovie( LTProfileUtils::ReadUint32( "Game", "GameRuns", 0, g_pVersionMgr->GetGameSystemIniFile()) > 1 );
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

	m_UserMenuMgr.OnLButtonDblClick(x,y);
	m_SystemMenuMgr.OnLButtonDblClick(x,y);

	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnLButtonDblClick(x,y);
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

	m_UserMenuMgr.OnRButtonUp(x,y);
	m_SystemMenuMgr.OnRButtonUp(x,y);

	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonUp(x,y);
		} 
		break;
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

	m_UserMenuMgr.OnRButtonDown(x,y);
	m_SystemMenuMgr.OnRButtonDown(x,y);

	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonDown(x,y);
		}	
		break;
		
		case GS_SPLASHSCREEN:
		{
			// They pressed a button - end splash screen...
			double fCurTime = LTTimeUtils::GetTimeS();
			if ( fCurTime > g_fSplashSndMinEndTime )
			{
				EndSplashScreen( );
			}
		}	
		break;
	
		case GS_MOVIE:
		{
			bool bEndMovies = false;
			uint32 nGameRuns = LTProfileUtils::ReadUint32( "Game", "GameRuns", 0, g_pVersionMgr->GetGameSystemIniFile());
			if( nGameRuns > 1 )
				bEndMovies = true;

			// They pressed a button - next movie...
			NextMovie( bEndMovies );
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

	m_UserMenuMgr.OnRButtonDblClick(x,y);
	m_SystemMenuMgr.OnRButtonDblClick(x,y);

	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnRButtonDblClick(x,y);
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

	m_UserMenuMgr.OnMouseMove(x,y);
	m_SystemMenuMgr.OnMouseMove(x,y);

	switch (m_eGameState)
	{
		case GS_SCREEN:
		{
			GetScreenMgr( )->OnMouseMove(x,y);
		}
		break;
	}
}

void CInterfaceMgr::OnMouseWheel(int x, int y, int zDelta)
{
	// Make sure we're initialized.
	if( !IsInitialized( ))
		return;

	if (m_MessageBox.IsVisible())
	{
		return;
	}

	m_UserMenuMgr.OnMouseWheel(x,y,zDelta);
	m_SystemMenuMgr.OnMouseWheel(x,y,zDelta);

	switch (m_eGameState)
	{
	case GS_SCREEN:
		{
			GetScreenMgr( )->OnMouseWheel(x,y,zDelta);
		} 
		break;
	}
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
	if (!m_bScreenFade)
		return;

    LTRect2n rcSrc(0, 0, 2, 2);
	
	// Initialize the surface if necessary...

	if (!m_bFadeInitialized)
	{
		m_fScreenFadeAlpha = 1.0f;
		m_fCurFadeTime = m_fTotalFadeTime;
        m_bFadeInitialized = true;
	}


	// Fade the screen if the surface is around...

	if (m_bScreenFade)
	{
		float fTimeDelta;
		if (m_fTotalFadeTime < 0.1f)
		{
			fTimeDelta = m_bFadeIn ? 0.0f : LTMAX( m_fTotalFadeTime, 0.0f );
		}
		else
		{
			fTimeDelta = m_bFadeIn ? m_fCurFadeTime : (m_fTotalFadeTime - m_fCurFadeTime);

		}

		float fLinearAlpha = 1.0f;
		if (m_fTotalFadeTime > 0.0f)
		{
			fLinearAlpha = (fTimeDelta / m_fTotalFadeTime);
		}
		fLinearAlpha = fLinearAlpha < 0.0f ? 0.0f : (fLinearAlpha > 1.0f ? 1.0f : fLinearAlpha);

		m_fScreenFadeAlpha = 1.0f - WaveFn_SlowOn(1.0f - fLinearAlpha);
		m_fScreenFadeAlpha = m_fScreenFadeAlpha < 0.0f ? 0.0f : (m_fScreenFadeAlpha > 1.0f ? 1.0f : m_fScreenFadeAlpha);

		uint32 dwWidth = 640, dwHeight = 480;
		g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(dwWidth, dwHeight);

		//setup a black quad and render that on the screen
		LT_POLYG4 Quad;
		DrawPrimSetXYWH(Quad, -0.5f, -0.5f, (float)dwWidth, (float)dwHeight);
		DrawPrimSetRGBA(Quad, 0, 0, 0, (uint8)(m_fScreenFadeAlpha * 255.0f));

		if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
		{
			g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
			g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
			g_pLTClient->GetRenderer()->End3D();
		}


		// The alpha value is based on the fade time, only update it if
		// we want the alpha value to change...

		if (bUpdateAlpha)
		{
			m_fCurFadeTime -= RealTimeTimer::Instance( ).GetTimerElapsedS( );
		}
	}

	// See if we're done...

	if (ScreenFadeDone())
	{
		// If we faded in we're done...
		if (m_bFadeIn)
		{
            m_bFadeInitialized  = false;
            m_bScreenFade       = false;
			m_bOverrideInitialFade = false;
		}
		else
		{
			// We need to keep the screen black until it is faded
			// back in, so we'll just make sure the screen is clear
			// (and not set m_bScreenFade so we'll be called to
			// clear the screen every frame until we fade back in)...

	        g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_ALL, 0);
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
	m_bFadeInitialized		= false;
	m_bScreenFade			= false;
	m_bOverrideInitialFade  = false;
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
	bool bOn = (g_vtLetterBoxDisabled.GetFloat() ? false : (m_bLetterBox || g_vtLetterBox.GetFloat()));

	double fTime = RealTimeTimer::Instance( ).GetTimerAccumulatedS( );

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


	float fTimeLeft = (float)(m_fLetterBoxFadeEndTime - fTime);
	if (fTimeLeft < 0.0f) fTimeLeft = 0.0f;

	// Calculate the current alpha...

	if (bOn)
	{
		// Fading in...

		float fFadeTime = g_vtLetterBoxFadeInTime.GetFloat();

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

		float fFadeTime = g_vtLetterBoxFadeOutTime.GetFloat();

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
	float fLetterBoxPercent = ClientDB::Instance().GetFloat(ClientDB::Instance().GetClientSharedRecord(), CDB_Letterbox);

	if (fLetterBoxPercent > 0.01f && fLetterBoxPercent <= 1.0f)
	{
		fBorderPercent = (1.0f - fLetterBoxPercent) / 2.0f;
	}

    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetRenderer()->GetCurrentRenderTargetDims(dwWidth, dwHeight);

	//figure out how big our border is, making sure that it doesn't overlap
	uint32 nBorderSize = (uint32)LTMIN((float)dwHeight / 2.0f, ((float)dwHeight * fBorderPercent));

	//setup a black quad and render that on the screen
	LT_POLYG4 Quad;
	static const float fBuffer = 10.0f;
	DrawPrimSetXYWH(Quad, -fBuffer, -fBuffer, (float)dwWidth + fBuffer * 2.0f, (float)nBorderSize + fBuffer);
	DrawPrimSetRGBA(Quad, 0, 0, 0, (uint8)(m_fLetterBoxAlpha * 255.0f));

	g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
		
	//draw the top
	g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);

	//and now the bottom
	DrawPrimSetXYWH(Quad, -fBuffer, (float)(dwHeight - nBorderSize), (float)dwWidth + fBuffer * 2.0f, (float)nBorderSize + fBuffer);
	g_pLTClient->GetDrawPrim()->DrawPrim(&Quad, 1);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AllowCameraMovement
//
//	PURPOSE:	Can the camera move (while in this state)
//
// --------------------------------------------------------------------------- //

bool CInterfaceMgr::AllowCameraMovement()
{
	switch (m_eGameState)
	{
		case GS_MENU :
            return false;
		break;

		case GS_PLAYING:
			{
				if (g_pChatInput && g_pChatInput->IsVisible())
				{
					return false;
				}
				if (g_pDebugInput && g_pDebugInput->IsVisible())
				{
					return false;
				}
			}
			return true;
		break;


		default :
		break;
	}

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AllowCameraRotation
//
//	PURPOSE:	Can the camera rotate (while in this state)
//
// --------------------------------------------------------------------------- //

bool CInterfaceMgr::AllowCameraRotation()
{
	switch( m_eGameState )
	{
		case GS_MENU:
			return false;
		break;

		default:
		break;
	}

	return true;
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
//	m_BackSprite.Term();
}
void CInterfaceMgr::AddInterfaceFX(CClientFXLink* pLink, const char *pFXName, LTVector vPos, bool bLoop, bool bSmoothShutdown )
{
	uint32 dwFlags = 0;
	if (bLoop)
		dwFlags |= FXFLAG_LOOP;
	if (!bSmoothShutdown)
		dwFlags |= FXFLAG_NOSMOOTHSHUTDOWN;

	LTRigidTransform tTransform;
	tTransform.Init();
	tTransform.m_vPos = vPos;

	CLIENTFX_CREATESTRUCT	fxInit(pFXName , dwFlags, tTransform ); 
	CClientFXLink fxLink;
	if (!pLink)
		pLink = &fxLink;

	m_InterfaceFXMgr.CreateClientFX(pLink, fxInit, true );

	if (pLink->IsValid())
	{
		pLink->GetInstance()->Show();
	}
	else
	{
		DebugCPrint(1,"ERROR: Failed to create interface FX %s",pFXName);
	}

}
void CInterfaceMgr::RemoveInterfaceFX()
{
	m_InterfaceFXMgr.ShutdownAllFX();
}

void CInterfaceMgr::RemoveInterfaceFX(CClientFXLink	*pLink)
{
	if (pLink && pLink->IsValid())
	{
		m_InterfaceFXMgr.ShutdownClientFX(pLink);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RemoveAllInterfaceFX
//
//	PURPOSE:	Remove the 3D objects used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CInterfaceMgr::RemoveAllInterfaceFX()
{
	RemoveInterfaceBackground();

	m_InterfaceFXMgr.ShutdownAllFX();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdateInterfaceFX
//
//	PURPOSE:	Update the 3D Objects used as a background for the menu
//
// --------------------------------------------------------------------------- //

//callback function that is called for each object referenced by an FX in the FX manager
//when iterating through for rendering interface FX
static void CollectInterfaceFXObjectsCB(const CBaseFX* pBaseFx, HOBJECT hObject, void* pUserData)
{
	if(hObject)
	{
		FXRenderInfo NewInfo;
		NewInfo.hObj	= hObject;
		NewInfo.nLayer	= pBaseFx->GetMenuLayer();

		g_FxRenderList.push_back(NewInfo);
	}
}

void CInterfaceMgr::UpdateInterfaceFX()
{
	//make sure our render list is empty
	g_FxRenderList.resize(0);

    //collect all of our objects to render
	m_InterfaceFXMgr.EnumerateObjects(CollectInterfaceFXObjectsCB, NULL);

	//now sort all of these fx by layer
	std::sort(g_FxRenderList.begin(), g_FxRenderList.end());	

	//and now render each layer
	static const uint32 knMaxFxPerLayer = 256;
	HOBJECT hLayerObjects[knMaxFxPerLayer];

	//step through the render list
	FXRenderList::iterator itFx = g_FxRenderList.begin();
	while (itFx != g_FxRenderList.end())
	{
		uint32 nNumLayerObjects = 0;

		//get the menu layer of the first item
		uint32 nCurrLayer = itFx->nLayer;

		//add each item of that same layer to the list of objects to be rendered during this pass
		while ((itFx != g_FxRenderList.end()) && (nNumLayerObjects < knMaxFxPerLayer) && (itFx->nLayer == nCurrLayer))
		{
			hLayerObjects[nNumLayerObjects] = itFx->hObj;
			nNumLayerObjects++;
			itFx++;
		}

		g_pLTClient->GetRenderer()->RenderObjects(m_hInterfaceCamera, hLayerObjects, nNumLayerObjects);	
	}

	//the updating of the active effects comes at the end of the update so that way chained
	//effects will be able to setup prior to the next rendering and prevent popping issues
	m_InterfaceFXMgr.UpdateAllActiveFX();

	//we don't need this list anymore
	g_FxRenderList.resize(0);
}


/******************************************************************/
void CInterfaceMgr::RequestInterfaceSound(InterfaceSound eIS)
{
	if (m_eNextSound <= eIS)
	{
		m_eNextSound = eIS;
		UpdateInterfaceSound();
	}
}

void CInterfaceMgr::ClearInterfaceSound()
{
	m_eNextSound = IS_NONE;
}

HLTSOUND CInterfaceMgr::UpdateInterfaceSound()
{
	HLTSOUND hSnd = NULL;

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
	ILTVideoTexture* pVideoTex = g_pLTClient->GetVideoTexture();
	if (!pVideoTex || !(GetAdvancedOptions() & AO_MOVIES) || g_vtDisableMovies.GetFloat())
	{
		SwitchToScreen(SCREEN_ID_MAIN);
		return;
	}

	//free any existing movie
	if (m_hMovie)
	{
		pVideoTex->ReleaseVideoTexture(m_hMovie);
		m_hMovie = NULL;
	}

	ClientDB& ClientDatabase = ClientDB::Instance();
	int32 nSplashMovies = ClientDatabase.GetNumValues( ClientDatabase.GetClientSharedRecord(), CDB_SplashMovies );

	// Make sure we're not just stopping movies.
	if( !bEndMovies )
	{
		// Find the next valid movie.
		while( m_nCurMovie < nSplashMovies && !m_hMovie )
		{
			char const* pszCurMovie = ClientDatabase.GetString( ClientDatabase.GetClientSharedRecord(), CDB_SplashMovies, m_nCurMovie );
			m_hMovie = pVideoTex->CreateVideoTexture(pszCurMovie, eVTO_AllowStreaming | eVTO_PlayOnce | eVTO_PlaySound);
			m_nCurMovie++;
		}
	}

	// Check if we didn't find any movie.
	if( !m_hMovie )
	{
		m_nCurMovie = 0;
		m_hMovie = NULL;
		SwitchToScreen(SCREEN_ID_MAIN);
	}
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
	// Clear all the render surfaces...
	g_pLTClient->GetRenderer()->ClearRenderTarget( CLEARRTARGET_ALL, 0 );

	// Get rid of our current texture
	if( g_hDemoTexture )
	{
		g_hDemoTexture.Free();
	}

	ClientDB& ClientDatabase = ClientDB::Instance();
	uint32 nDemoScreens = ClientDatabase.GetNumValues( ClientDatabase.GetClientSharedRecord(), CDB_DemoEndScreenImage );

	// Make sure we haven't displayed them all yet...
	if( g_nDemoScreen < nDemoScreens )
	{
		g_hDemoTexture.Load( ClientDatabase.GetString( ClientDatabase.GetClientSharedRecord(), CDB_DemoEndScreenImage, g_nDemoScreen ) );
	}

	// Move on to the next demo screen...
	++g_nDemoScreen;

	// If we didn't create a new demo screen, we know we're done!
	if( !g_hDemoTexture )
	{
		m_bSeenDemoScreens = true;
	}
}


void CInterfaceMgr::ShowDemoScreens(bool bQuit)
{
	// Make sure we're disconnected from server.
	if(g_pLTClient->IsConnected())
	{
		g_pInterfaceMgr->SetIntentionalDisconnect( true );
		g_pClientConnectionMgr->ForceDisconnect();
	}

	m_bQuitAfterDemoScreens = bQuit;
	m_bSeenDemoScreens = false;
	ChangeState(GS_DEMOSCREEN);
}



// Find the filename in the missing filename result..
const char *FindFilename(const char *pFileName)
{
	// Jump to the end..
	const char *pResult = &pFileName[LTStrLen(pFileName)];
	// Look for a backslash
	while ((pResult != pFileName) && (pResult[-1] != '\\'))
		--pResult;
	// Return that..
	return pResult;
}

void CInterfaceMgr::Disconnected(uint32 nDisconnectFlag)
{
	eScreenID destination = IsMultiplayerGameClient( ) ? SCREEN_ID_MULTI : SCREEN_ID_MAIN;
	// Get the disconnect code.
	uint32 nDisconnectCode = g_pClientConnectionMgr->GetDisconnectCode();
	wchar_t wszMsg[256] = L"";

	switch (nDisconnectCode)
	{
		case eDisconnect_NotSameGUID:
			LTStrCpy( wszMsg, LoadString( "IDS_NETERR_NOTSAMEGUID" ), LTARRAYSIZE( wszMsg ));
			break;
		case eDisconnect_WrongPassword:
			LTStrCpy( wszMsg, LoadString( "IDS_NETERR_WRONGPASS" ), LTARRAYSIZE( wszMsg ));
			break;
		case eDisconnect_BadCdKey:
			LTStrCpy( wszMsg, LoadString( "IDS_NETERR_VERIFYCDKEYFAIL" ), LTARRAYSIZE( wszMsg ));
			break;
		case eDisconnect_InvalidAssets:
			LTStrCpy( wszMsg, LoadString( "IDS_NETERR_INVALIDASSETS" ), LTARRAYSIZE( wszMsg ));
			break;
		case eDisconnect_Banned:
			LTStrCpy( wszMsg, LoadString( "IDS_NETERR_BANNED" ), LTARRAYSIZE( wszMsg ));
			break;
		case eDisconnect_TimeOut:
			LTStrCpy( wszMsg, LoadString( "IDS_NETERR_TIMEOUT" ), LTARRAYSIZE( wszMsg ));
			break;
		case eDisconnect_PunkBuster:
			{
				// load the PunkBuster error string
				LTStrCpy( wszMsg, LoadString( "IDS_NETERR_PUNKBUSTER" ), LTARRAYSIZE( wszMsg ));

				// add newlines
				if( !LTStrEmpty( wszMsg ))
				{
					LTStrCat( wszMsg, L"\n\n", LTARRAYSIZE( wszMsg ));
				}

				// get the reason string
				const char* pszReason = g_pClientConnectionMgr->GetDisconnectMsg();

				// remove the error prefix
				if (LTSubStrCmp(pszReason, g_pszPunkBusterErrorPrefix, LTStrLen(g_pszPunkBusterErrorPrefix)) == 0)
				{
					pszReason += LTStrLen(g_pszPunkBusterErrorPrefix);
				}

				// append the string
				if( !LTStrEmpty( pszReason ))
				{
					LTStrCat(wszMsg, MPA2W(pszReason).c_str(), LTARRAYSIZE(wszMsg));
				}
			}
			break;
		case eDisconnect_ContentTransferFailed:
			{
				// Add the error message on the first line.
				char const* pszMsgId = g_pClientConnectionMgr->GetDisconnectMsg();
				if( !LTStrEmpty( pszMsgId ))
				{
					LTStrCpy( wszMsg, LoadString( pszMsgId ), LTARRAYSIZE( wszMsg ));
				}

				// Add the filename on the 2nd line.
				char const* pszFilename = g_pClientConnectionMgr->GetContentTransferErrorFilename();
				if( !LTStrEmpty( pszFilename ))
				{
					// Put it on the next line after the error message if there is one.
					if( !LTStrEmpty( wszMsg ))
					{
						LTStrCat( wszMsg, L"\n", LTARRAYSIZE( wszMsg ));
					}
					// Wrap the filename with parentheses.
					LTStrCat( wszMsg, L"(", LTARRAYSIZE( wszMsg ));
					LTStrCat( wszMsg, MPA2W( pszFilename ).c_str(), LTARRAYSIZE( wszMsg ));
					LTStrCat( wszMsg, L")", LTARRAYSIZE( wszMsg ));
				}

				// Add the servermessage on the 3rd line.
				wchar_t const* pszServerMessage = g_pClientConnectionMgr->GetContentTransferErrorServerMessage();
				if( !LTStrEmpty( pszServerMessage ))
				{
					// Need a carriage return if we had previous info.
					if( !LTStrEmpty( wszMsg ))
					{
						LTStrCat( wszMsg, L"\n\n", LTARRAYSIZE( wszMsg ));
					}
					LTStrCat( wszMsg, pszServerMessage, LTARRAYSIZE( wszMsg ));
				}
			}
			break;

		case 0 : // Avoid the warning..
		default :
		{
			if( nDisconnectFlag == LT_REJECTED )
			{
				LTStrCpy( wszMsg, LoadString( "IDS_KICKED" ), LTARRAYSIZE( wszMsg ));
			}
			else if ( nDisconnectFlag == LT_MISSINGWORLDFILE )
			{
				LTStrCpy( wszMsg, LoadString( "IDS_MISSING_WORLD" ), LTARRAYSIZE( wszMsg ));
			}
			else if ( nDisconnectFlag == LT_INVALIDWORLDFILE )
			{
				LTStrCpy( wszMsg, LoadString( "IDS_NETERR_INVALIDASSETS" ), LTARRAYSIZE( wszMsg ));
			}
			else
			{
				LTStrCpy( wszMsg, LoadString( "IDS_DISCONNECTED_FROM_SERVER" ), LTARRAYSIZE( wszMsg ));
			}

			DebugCPrint(0,"CInterfaceMgr::Disconnected(%d)", nDisconnectCode);
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


	if (GetGameState() == GS_LOADINGLEVEL)
	{
		LoadFailed( destination, wszMsg );
	}
	else
	{
		if (!m_bIntentionalDisconnect)
		{
			MBCreate mb;
			ShowMessageBox(wszMsg, &mb);
		}

		SwitchToScreen(destination);
	}

	GetScreenMgr()->ClearHistory();
	g_pClientConnectionMgr->ClearDisconnectCode();

}

void CInterfaceMgr::ConnectionFailed(uint32 nConnectionError)
{
	const char* szMsgId = "";
	eScreenID eScreenToShow = SCREEN_ID_NONE;
	if (IsMultiplayerGameClient())
	{
		bool bWasHosting = g_pClientConnectionMgr->GetStartGameRequest( ).m_Type == STARTGAME_HOST;
		switch( g_pClientConnectionMgr->GetNetGameInfo( ).m_eServerStartResult )
		{
			default:
				break;
			case eServerStartResult_Failed:
				szMsgId = bWasHosting ? "IDS_NETERR_HOSTSESSION" : "IDS_NETERR_JOINSESSION";
				break;
			case eServerStartResult_NetworkError:
				szMsgId = "IDS_NETERR_SELECTSERVICE";
				break;
		}

		if( GetCommandLineJoin( ))
		{
			eScreenToShow = SCREEN_ID_MAIN;
		}
		else
		{
			eScreenToShow = bWasHosting ? SCREEN_ID_HOST : SCREEN_ID_JOIN;

			//hack to rebuild our screen history
			CScreenMgr* pScreenMgr = GetScreenMgr( );
			pScreenMgr->ClearHistory();
			pScreenMgr->AddScreenToHistory( SCREEN_ID_MAIN );
			pScreenMgr->AddScreenToHistory( SCREEN_ID_MULTI );
		}
	}
	else
	{
		eScreenToShow = SCREEN_ID_MAIN;
	}

	MBCreate mb;
	if (nConnectionError == LT_REJECTED)
	{
		szMsgId = "IDS_SERVERFULL";
	}

	LoadFailed( eScreenToShow, LoadString( szMsgId ));
}

// hides or shows cursor based on current game state
void CInterfaceMgr::UpdateCursorState()
{
	if (m_MessageBox.IsVisible())
	{
		m_CursorMgr.UseCursor(true);
		return;
	}
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
		case GS_PAUSED:
		case GS_DEMOSCREEN:
		case GS_MOVIE:
		case GS_UNDEFINED:
		{
			m_CursorMgr.UseCursor(false);
		}
		break;
	
		case GS_MENU:
		case GS_SCREEN:
		default:
		{
			m_CursorMgr.UseCursor(true);
		}
	}
}



void CInterfaceMgr::UpdateClientList()
{
	if (IsMultiplayerGameClient())
	{
		// Don't send update requests more than once per second
		double fCurTime = RealTimeTimer::Instance( ).GetTimerAccumulatedS( );
		if (( fCurTime - m_fLastUpdateRequestTime) > 1.0f)
		{
			DebugCPrint(2,"Asking for clientlist update");
			SendEmptyServerMsg(MID_MULTIPLAYER_UPDATE, MESSAGE_GUARANTEED);
			m_fLastUpdateRequestTime = fCurTime;
		}
	}
}



LTVector CInterfaceMgr::GetWorldFromScreenPos(const LTVector2n& pos, float fDepth)
{
	return GetWorldFromScreenPos(LTVector2((float)pos.x, (float)pos.y), fDepth);
}

LTVector CInterfaceMgr::GetWorldFromScreenPos(const LTVector2& pos, float fDepth)
{
	//normalize the screen coordinates
	LTVector2 vUnitScreen(	pos.x / (float)g_pInterfaceResMgr->GetScreenWidth(),
							pos.y / (float)g_pInterfaceResMgr->GetScreenHeight());

	LTVector vPos;
	g_pLTClient->GetRenderer()->ScreenPosToWorldPos(m_hInterfaceCamera, vUnitScreen, fDepth, vPos);

	return vPos;
}

//transforms world space to camera space to screen coordinates
//return vector's x and y are screen coordinates
//return vector's z is distance from the camera

LTVector CInterfaceMgr::GetScreenFromWorldPos(const LTVector& vPos, HOBJECT hCamera, bool& bOnScreen)
{
	LTVector vScreenPos;
	LTRESULT res = g_pLTClient->GetRenderer()->WorldPosToScreenPos(hCamera, vPos, vScreenPos);

	bOnScreen = (LT_OK == res && vScreenPos.x >= 0.0f && vScreenPos.x <= 1.0f 
							&& vScreenPos.y >= 0.0f && vScreenPos.y <= 1.0f
							&& vScreenPos.z > 0.0f);

	//scale based upon the physical extents of the screen
	vScreenPos.x *= (float)g_pInterfaceResMgr->GetScreenWidth();
	vScreenPos.y *= (float)g_pInterfaceResMgr->GetScreenHeight();

	return vScreenPos;
}



void CInterfaceMgr::NextWeapon()
{
	CClientWeapon const *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	//no weapons or not enabled bail early
	if (pClientWeapon && !g_pClientWeaponMgr->WeaponsEnabled())
	{
		return;
	}

	//what do we have right now?
	HWEAPON hCurrent = g_pClientWeaponMgr->GetRequestedWeaponRecord();
	HWEAPON hNextWeapon = NULL;
	if (pClientWeapon && !hCurrent)
	{
		hCurrent = pClientWeapon->GetWeaponRecord();
	}

	//look at the next slot...
	uint8 nSlot = g_pPlayerStats->GetWeaponSlot(hCurrent);
	if (nSlot == WDB_INVALID_WEAPON_INDEX)
	{
		nSlot = 0;
	}
	else
	{
		nSlot++;
	}

	//if the slot is valid and not empty, switch to it...
	if (nSlot < g_pPlayerStats->GetWeaponCapacity() && (NULL != g_pPlayerStats->GetWeaponInSlot(nSlot)))
	{
		hNextWeapon = g_pPlayerStats->GetWeaponInSlot(nSlot);
	}
	else
	{
		hNextWeapon = g_pPlayerStats->GetWeaponInSlot(0);
	}

	//we're already trying to switch to this weapon...
	if (hNextWeapon == g_pClientWeaponMgr->GetRequestedWeaponRecord())
	{
//		DebugCPrint(0,"already switching to %s (%0.2f)",g_pWeaponDB->GetRecordName(hNextWeapon),RealTimeTimer::Instance().GetTimerAccumulatedS());
		return;
	}

	bool bDeselect = true;
	if (pClientWeapon && pClientWeapon->GetState() == WS_SELECT)
	{
//		bDeselect = false;
	}


	if (hCurrent && hNextWeapon)
	{
//		DebugCPrint(0,"next weapon %s >>> %s (%0.2f)",g_pWeaponDB->GetRecordName(hCurrent),g_pWeaponDB->GetRecordName(hNextWeapon),RealTimeTimer::Instance().GetTimerAccumulatedS());
	}

	//do the change
	g_pClientWeaponMgr->ChangeWeapon( hNextWeapon, NULL, -1, bDeselect );
	m_NextWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	m_PrevWeaponKeyDownTimer.Stop();

}

void CInterfaceMgr::PreviousWeapon()
{
	CClientWeapon const *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	//no weapons or not enabled bail early
	if (pClientWeapon && !g_pClientWeaponMgr->WeaponsEnabled())
	{
		return;
	}

	//what do we have right now?
	HWEAPON hCurrent = g_pClientWeaponMgr->GetRequestedWeaponRecord();
	HWEAPON hPrevWeapon = NULL;
	if (pClientWeapon && !hCurrent)
	{
		hCurrent = pClientWeapon->GetWeaponRecord();
	}

	//what slot is it in...
	uint8 nSlot = g_pPlayerStats->GetWeaponSlot(hCurrent);
	if (nSlot == 0 || nSlot == WDB_INVALID_WEAPON_INDEX)
	{
		if (g_pPlayerStats->HasEmptySlot())
		{
			nSlot = g_pPlayerStats->FindFirstEmptySlot(); 
		}
		else
		{
			nSlot = g_pPlayerStats->GetWeaponCapacity(); 
		}
		
	}

	//go to the previous slot
	nSlot--;

	//if the slot is valid and not empty, switch to it...
	if (nSlot < g_pPlayerStats->GetWeaponCapacity() && (NULL != g_pPlayerStats->GetWeaponInSlot(nSlot)))
	{
		hPrevWeapon = g_pPlayerStats->GetWeaponInSlot(nSlot);
	}
	else
	{
		hPrevWeapon = g_pPlayerStats->GetWeaponInSlot(0);
	}

	//we're already trying to switch to this weapon...
	if (hPrevWeapon == g_pClientWeaponMgr->GetRequestedWeaponRecord())
	{
//		DebugCPrint(0,"already switching to %s (%0.2f)",g_pWeaponDB->GetRecordName(hPrevWeapon),RealTimeTimer::Instance().GetTimerAccumulatedS());
		return;
	}

	bool bDeselect = true;
	if (pClientWeapon && pClientWeapon->GetState() == WS_SELECT)
	{
//		bDeselect = false;
	}

	if (hCurrent && hPrevWeapon)
	{
//		DebugCPrint(0,"prev weapon %s >>> %s (%0.2f)",g_pWeaponDB->GetRecordName(hCurrent),g_pWeaponDB->GetRecordName(hPrevWeapon),RealTimeTimer::Instance().GetTimerAccumulatedS());
	}

	//do the change
	g_pClientWeaponMgr->ChangeWeapon( hPrevWeapon, NULL, -1, bDeselect );
	m_PrevWeaponKeyDownTimer.Start(g_vtChooserAutoSwitchTime.GetFloat());
	m_NextWeaponKeyDownTimer.Stop();
}

void CInterfaceMgr::UpdateWeaponSwitch()
{
	if (m_NextWeaponKeyDownTimer.IsStarted( ) && m_NextWeaponKeyDownTimer.IsTimedOut( ))
	{
		// See if we should switch to the next weapon...
		if (m_AutoSwitchTimer.IsStarted( ))
		{
			if (m_AutoSwitchTimer.IsTimedOut( ))
			{
				NextWeapon();
				m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
			}
		}
		else
		{
			m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
		}
	}
	else if (m_PrevWeaponKeyDownTimer.IsStarted() && m_PrevWeaponKeyDownTimer.IsTimedOut())
	{
		if (m_AutoSwitchTimer.IsTimedOut( ))
		{
			PreviousWeapon();
			m_AutoSwitchTimer.Start(g_vtChooserAutoSwitchFreq.GetFloat());
		}
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

	if ( g_pDecision )
	{
		if ( g_pDecision->IsVisible() && g_pDecision->GetObject() == hObj )
		{
			g_pDecision->OnObjectRemove(hObj);
		}
	}
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
    double fGameTimeEnd = msg.Readdouble();
	bool bPaused = msg.Readbool();
	DisplayTimerMsgId eDisplayTimerMsgId = ( DisplayTimerMsgId )msg.Readuint8( );

	// Calculate time to the end.
	switch( eDisplayTimerMsgId )
	{
		case kDisplayTimerMsgId_Team0:
			g_pTeam0Timer->SetTime( fGameTimeEnd, bPaused );
			break;
		case kDisplayTimerMsgId_Team1:
			g_pTeam1Timer->SetTime( fGameTimeEnd, bPaused );
			break;
		case kDisplayTimerMsgId_Main:
			g_pMainTimer->SetTime( fGameTimeEnd, bPaused );
			break;
		case kDisplayTimerMsgId_SuddenDeath:
			g_pMainTimer->SetTime( fGameTimeEnd, bPaused );
			break;
	}
    return true;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::LoadFailed
//
//	PURPOSE:	Handle failed load attempt, specify what screen to go to
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::LoadFailed(eScreenID eScreenToShow /*= SCREEN_ID_NONE*/, const wchar_t* pwszLoadFailedMsg /* = NULL */, bool bShowMessage /* = true */)  
{	
	// if the flag is already set then we've handled this load failure and should not
	// attempt to handle it again
	if (m_bLoadFailed)
	{
		return;
	}

	m_bLoadFailed = true;
	m_bShowLoadFailedMessage = bShowMessage;

	if( LTStrEmpty( pwszLoadFailedMsg ) && g_pClientConnectionMgr->GetLastConnectionResult( ) == LT_REJECTED )
	{
		m_sLoadFailedMsg = LoadString( "IDS_SERVERFULL" );
	}
	else
	{
		m_sLoadFailedMsg = !LTStrEmpty( pwszLoadFailedMsg ) ? pwszLoadFailedMsg : LoadString( "IDS_DISCONNECTED_FROM_SERVER" );
	}

	if (eScreenToShow == SCREEN_ID_NONE)
	{
		if (IsMultiplayerGameClient())
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
	CSpecialFXList* pList = psfxMgr->GetFXList(SFX_NAVMARKER_ID);
	if (pList)
	{
		int nNumNav = pList->GetSize();

		for (int i=0; i < nNumNav; i++)
		{
			if ((*pList)[i])
			{
				CNavMarkerFX* pNMFX = dynamic_cast<CNavMarkerFX*>((*pList)[i]);
				pNMFX->UpdateClientFX();
			}
		}
	}

	pList = psfxMgr->GetFXList(SFX_PROJECTILE_ID);
	if (pList)
	{
		int nNumProj = pList->GetSize();

		for (int i=0; i < nNumProj; i++)
		{
			if ((*pList)[i])
			{
				CProjectileFX* pProjFX = dynamic_cast<CProjectileFX*>((*pList)[i]);
				pProjFX->UpdateClientFX();
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

	/*
	// Make sure we delete the browsers, since they can take up quite a bit of memory.
	CScreenJoin *pScreenJoin= dynamic_cast< CScreenJoin * >( GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN));
	if( pScreenJoin )
	{
	#pragma MESSAGE( "[BP] 11/25/03 - Re-enable this call when screenjoin is hooked up to gamespy." )
	//		pScreenJoin->TermServerLists( );
	}
	*/

// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// Don't need the patch info check at this point.
	if( m_pGameSpyPatchInfo )
	{
		g_pLTGameUtil->DestroyGameSpyPatchInfo( m_pGameSpyPatchInfo );
		m_pGameSpyPatchInfo = NULL;
	}

#endif // !PLATFORM_XENON

	GetPlayerStats()->SetObjective(NULL);

	// clear the HUD message queues
	g_pGameMsgs->ClearHistory();
	g_pChatMsgs->ClearHistory();

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandlePlayerEventMessage
//
//	PURPOSE:	Handle MID_PLAYER_EVENT message from server
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::HandlePlayerEventMessage(ILTMessage_Read *pMsg)
{
	PlayerEventMsgType eType = static_cast<PlayerEventMsgType>(pMsg->Readuint8());

	switch(eType) 
	{
	case kPEHealthTestMode:
		{
			g_pClientSoundMgr->PlayInterfaceDBSound("HealthTest");
			g_pGameMsgs->AddMessage(LoadString("IDS_IWASKILLED"),kMsgTransmission);
			const char *pszDeathFX  = ClientDB::Instance( ).GetString( ClientDB::Instance( ).GetClientSharedRecord( ), CDB_ClientShared_DeathClientFX );
			// Create the deathFX to display an overlay or other FX goodness...
			if( pszDeathFX && pszDeathFX[0] )
			{
				CLIENTFX_CREATESTRUCT fxInit( pszDeathFX, 0 );
				g_pGameClientShell->GetRealTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
			}

		} break;
	case kPEReward:
		{
			uint32 nLocalID = 0;
			g_pLTClient->GetLocalClientID(&nLocalID);

			uint32 nClientId = pMsg->Readuint32( );
			uint32 nId		= pMsg->Readuint32();

			wchar_t szTmp[128] = L"";
			CLIENT_INFO* pCI = m_ClientInfo.GetClientByID(nClientId);
			if (pCI)
			{
				FormatString(nId, szTmp,LTARRAYSIZE(szTmp), pCI->sName.c_str() );

				g_pTransmission->Show(szTmp);
				g_pGameMsgs->AddMessage(szTmp,kMsgTransmission);
				
				//removed from chat queue
				/*
				if (nClientId == nLocalID)
				{
					g_pChatMsgs->AddMessage(szTmp,kMsgTeam);
				}
				*/


			}

			
		} break;
	case kPETransmission:
		{
			// retrieve the string from the message, play the chat sound, and display the message
			uint32 nActivePlayerClientId = pMsg->Readuint32( );
			uint32 nNumMsgs = pMsg->Readuint8( );
			uint32 dwId[8];
			nNumMsgs = LTMIN( nNumMsgs, LTARRAYSIZE( dwId ));
			for( uint8 i = 0; i < nNumMsgs; i++ )
				dwId[i] = pMsg->Readuint32();
			uint32 nSound = pMsg->Readuint32();
			uint8 nTeam =  pMsg->Readuint8();

			if( GameModeMgr::Instance( ).m_grbUseTeams && nTeam != INVALID_TEAM )
			{
				CLIENT_INFO *pLocalCI = m_ClientInfo.GetLocalClient();
				if( !pLocalCI )
					return false;

				if( pLocalCI->nTeamID != nTeam )
					return false;
			}


			if (nSound)
			{
				const char* pszFilename = g_pClientSoundMgr->GetSoundFilenameFromId("Dialogue", StringIDFromIndex(nSound) );
				if( pszFilename && (pszFilename[0] != '\0') ) 
					g_pClientSoundMgr->PlaySoundLocal(pszFilename,SOUNDPRIORITY_PLAYER_HIGH);
			}
			else
			{
				g_pClientSoundMgr->PlayInterfaceDBSound("Transmission");
			}

			uint32 nLocalID = 0;
			g_pLTClient->GetLocalClientID (&nLocalID);

			// Check if there wasn't an active player for this transmission.
			if( nActivePlayerClientId == INVALID_CLIENT || nActivePlayerClientId == nLocalID )
			{
				// Copy all the messages into one string.
				wchar_t wszMsg[256] = L"";
				for( uint8 nMsg = 0; nMsg < nNumMsgs; nMsg++ )
				{
					// If there is already stuff in the message, then prepend a newline.
					if( wszMsg[0] )
					{
						LTStrCat( wszMsg, L"\n", LTARRAYSIZE( wszMsg ));
					}
					LTStrCpy( wszMsg, LoadString( dwId[nMsg] ), LTARRAYSIZE( wszMsg ));
				}

				g_pTransmission->Show(wszMsg);
				//removed from chat queue
				//g_pChatMsgs->AddMessage(wszMsg,kMsgTransmission);
			}
			// There was an active player with this transmission.  Format it like this:
			// "<PlayerName>: <Message>".
			else
			{
				wchar_t wszCharName[MAX_PLAYER_NAME+1];
				LTStrCpy( wszCharName, m_ClientInfo.GetPlayerName( nActivePlayerClientId ), LTARRAYSIZE( wszCharName ));

				// Copy all the messages into one string.
				wchar_t wszMsg[256] = L"";
				for( uint8 nMsg = 0; nMsg < nNumMsgs; nMsg++ )
				{
					// If there is already stuff in the message, then prepend a newline.
					if( wszMsg[0] )
					{
						LTStrCat( wszMsg, L"\n", LTARRAYSIZE( wszMsg ));
					}
					LTStrCat( wszMsg, wszCharName, LTARRAYSIZE( wszMsg ));
					LTStrCat( wszMsg, L": ", LTARRAYSIZE( wszMsg ));
					LTStrCat( wszMsg, LoadString( dwId[nMsg] ), LTARRAYSIZE( wszMsg ));
				}

				g_pTransmission->Show(wszMsg);
				//removed from chat queue
				//g_pChatMsgs->AddMessage(wszMsg,kMsgTransmission);
			}

			return true;
		}
		break;
	case kPESignal:
		{
			float fDuration = pMsg->Readfloat();
			g_pHUDMgr->StartFlicker( fDuration );
			g_pPlayerMgr->GetFlashLight()->PlayFlicker( fDuration,true );
			g_pClientSoundMgr->PlayDBSoundLocal( g_pSoundDB->GetSoundDBRecord( "Signal" ) );
			return true;
		}
		break;
	case kPEOverlay:
		{
			char szName[128] = "";
			pMsg->ReadString(szName,LTARRAYSIZE(szName));
			bool bOn = !!pMsg->Readuint8();

			if (!LTStrEmpty(szName))
			{
				if (bOn)
					g_pOverlay->Show(g_pOverlay->GetHUDOverlay( szName ));
				else
					g_pOverlay->Hide(g_pOverlay->GetHUDOverlay( szName ));
				return true;
			}
			LTASSERT(0,"Overlay message did not contain overlay name.");
		}
		break;
	case kPEEndRoundCondition:
		{
			GameModeMgr::EndRoundCondition eEndRoundCondition = ( GameModeMgr::EndRoundCondition )pMsg->Readuint8();
			uint8 nWinningTeam = pMsg->Readuint8( );
			uint8 nWinningPlayerClientId = pMsg->Readuint8( );

			uint32 nLocalID = INVALID_CLIENT;
			g_pLTClient->GetLocalClientID(&nLocalID);

			// Assume we're going to play the default transmission sound...
			const char* cpSound = "Transmission";

			// Copy all the messages into one string.
			wchar_t wszCondition[256] = L"";
			bool bCheckResult = false;
			switch( eEndRoundCondition )
			{
			case GameModeMgr::eEndRoundCondition_Restart:
				LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_RESTART" ), LTARRAYSIZE( wszCondition ));

				//if we're not in the game yet, we don't care that the round is restarting
				if (!g_pPlayerMgr->IsPlayerInWorld())
				{
					return true;
				}
				bCheckResult = false;
				break;
			case GameModeMgr::eEndRoundCondition_TimeLimit:
				LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_TIMELIMIT" ), LTARRAYSIZE( wszCondition ));;
				bCheckResult = true;
				break;
			case GameModeMgr::eEndRoundCondition_ScoreLimit:
				LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_SCORELIMIT" ), LTARRAYSIZE( wszCondition ));;
				bCheckResult = true;
				break;
			case GameModeMgr::eEndRoundCondition_SuddenDeathScore:
				LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_SUDDENDEATHSCORE" ), LTARRAYSIZE( wszCondition ));;
				bCheckResult = true;
				break;
			case GameModeMgr::eEndRoundCondition_Elimination:
				if( GameModeMgr::Instance().m_grbUseTeams )
				{
					if( GetClientInfoMgr()->GetLocalTeam( ) == nWinningTeam )
						LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_ELIMINATED_THEM" ), LTARRAYSIZE( wszCondition ));
					else
						LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_ELIMINATED_US" ), LTARRAYSIZE( wszCondition ));
				}
				else
				{
					if( nLocalID == nWinningPlayerClientId )
						LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_ELIMINATED_THEM" ), LTARRAYSIZE( wszCondition ));
					else
						LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_ELIMINATED_US" ), LTARRAYSIZE( wszCondition ));
				}
				bCheckResult = true;
				break;
			case GameModeMgr::eEndRoundCondition_Conquest:
				LTStrCpy( wszCondition, LoadString( "IDS_ENDROUND_CONDITION_CONQUEST" ), LTARRAYSIZE( wszCondition ));;
				bCheckResult = true;
				break;
			}
			

			wchar_t wszResult[256] = L"";
			if( bCheckResult )
			{
				if( GameModeMgr::Instance().m_grbUseTeams )
				{
					if( nWinningTeam == INVALID_TEAM )
					{
						LTStrCpy( wszResult, LoadString( "IDS_ENDROUND_CONDITION_DRAW" ), LTARRAYSIZE( wszResult ));
						cpSound = "MPTeamDraw";
					}
					else if( GetClientInfoMgr()->GetLocalTeam( ) == nWinningTeam )
					{
						LTStrCpy( wszResult, LoadString( "IDS_ENDROUND_CONDITION_WIN" ), LTARRAYSIZE( wszResult ));
						cpSound = "MPTeamWin";
					}
					else
					{
						LTStrCpy( wszResult, LoadString( "IDS_ENDROUND_CONDITION_LOSE" ), LTARRAYSIZE( wszResult ));
						cpSound = "MPTeamLose";
					}
				}
				else
				{
					if( nWinningPlayerClientId != INVALID_CLIENT )
					{
						FormatString( "IDS_ENDROUND_CONDITION_PLAYERWINS", wszResult, LTARRAYSIZE(wszResult), 
							m_ClientInfo.GetPlayerName(nWinningPlayerClientId));
						
						if (nLocalID == nWinningPlayerClientId)
						{
							cpSound = "MPPlayerWin";
						}
						else
						{
							cpSound = "MPPlayerLose";
						}
					}
				}
			}


			wchar_t wszMsg[256] = L"";
			LTSNPrintF( wszMsg, LTARRAYSIZE( wszMsg ), L"%s%s%s", wszCondition, wszResult[0] ? L"\n" : L"", wszResult );
			
			g_pEndRoundMessage->Show(wszMsg);
			//removed from chat queue
			//g_pChatMsgs->AddMessage(wszMsg,kMsgTransmission);

			g_pClientSoundMgr->PlayInterfaceDBSound(cpSound);
		}
		break;
	case kPEObjective:
		{
			HRECORD hRec = pMsg->ReadDatabaseRecord(g_pLTDatabase, DATABASE_CATEGORY( Objective ).GetCategory() );
			if (hRec)
			{
				HRECORD hText = DATABASE_CATEGORY( Objective ).GETRECORDATTRIB( hRec, TextDisplay );
				if (hText)
				{
					Credits::Instance().Start(hText);
				}
			}

			GetPlayerStats()->SetObjective(hRec);

			return true;
		}
		break;
	case kPEMission:
		{

			uint32 nID = pMsg->Readuint32();

			GetPlayerStats()->SetMission(StringIDFromIndex(nID));

			bool bShow = pMsg->Readbool();
			if (bShow && m_eGameState == GS_PLAYING && !IsMultiplayerGameClient() )
			{
				if (GetPlayerStats()->GetObjective() || !LTStrEmpty(GetPlayerStats()->GetMission()))
				{
					GetUserMenuMgr().SetCurrentMenu(MENU_ID_MISSION);
				}
			};


			return true;
		}
		break;
	case kPEAutobalance:
		{
			//since these events can occur between rounds, the display is delayed
			bool bScoreBalance = pMsg->Readbool();

			if (bScoreBalance)
			{
				m_bNotifyTeamScoreBalanced = true;
			}
			else
			{
				m_bNotifyTeamSizeBalanced = true;
			}
			

			return true;
		}
	}

	//we don't want it, rewind...
	pMsg->SeekTo(0);

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandlePlayerScoredMessage
//
//	PURPOSE:	Handle MID_PLAYER_SCORED message from server
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::HandlePlayerScoredMessage(ILTMessage_Read *pMsg)
{
	//Only do something if we're in multiplayer...
	if (!IsMultiplayerGameClient())
		return;

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID(&nLocalID);

	uint32 nVictim = pMsg->Readuint32();
	uint32 nScorer = pMsg->Readuint32();
	HAMMO  hAmmo = pMsg->ReadDatabaseRecord(g_pLTDatabase,g_pWeaponDB->GetAmmoCategory());
	bool   bHeadshot = pMsg->Readbool();

	eChatMsgType type = kMsgChat;
	bool bTeamKill = false;
	bool bLocalMsg = false;
	HRECORD hBroadcast = NULL;

	if (GameModeMgr::Instance( ).m_grbUseTeams)
	{
		uint8 nKillerTeam = m_ClientInfo.GetPlayerTeam(nScorer);
		uint8 nVictimTeam = m_ClientInfo.GetPlayerTeam(nVictim);
		if (nKillerTeam != INVALID_TEAM && nVictimTeam != INVALID_TEAM) 
		{
			if (nKillerTeam == nVictimTeam)
			{
				//team kill
				type = kMsgTransmission;
				bTeamKill = true;
			}
			else if (m_ClientInfo.IsLocalTeam(nKillerTeam))
			{
				//we got a kill
				type = kMsgTeam;
			}
			else
			{
				//they got a kill
				type = kMsgOtherTeam;
			}
		}
	}

	const char* szWpnID = NULL;
	if (hAmmo && nScorer != INVALID_CLIENT)
	{
		HWEAPON hWpn = g_pWeaponDB->GetWeaponFromAmmo(hAmmo, !USE_AI_DATA);
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWpn, !USE_AI_DATA);
		if (hWpnData)
		{
			szWpnID = g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_nShortNameId);
		}
	}

	wchar_t szTmp[128] = L"";
	CLIENT_INFO* pScorerCI = m_ClientInfo.GetClientByID(nScorer);
	if (nVictim == nLocalID)
	{
		bLocalMsg = true;
		type = kMsgTransmission;
		// Scored againsts self.
		if (nVictim == nScorer)
		{
			FormatString("IDS_KILLEDMYSELF",szTmp,LTARRAYSIZE(szTmp));
		}
		// Scored by ai or we don't know who did it
		else if( nScorer == INVALID_CLIENT || !pScorerCI)
		{
			FormatString("IDS_IWASKILLED",szTmp,LTARRAYSIZE(szTmp));
		}
		// Killed by player.
		else
		{
			if (bTeamKill)
			{
				FormatString("IDS_HEKILLEDME_TEAM", szTmp,LTARRAYSIZE(szTmp), m_ClientInfo.GetPlayerName (nScorer));
			}
			else if (szWpnID)
			{
				if (bHeadshot)
				{
					FormatString("IDS_HEKILLEDME_HS", szTmp,LTARRAYSIZE(szTmp),m_ClientInfo.GetPlayerName (nScorer),LoadString(szWpnID));
				}
				else
				{
					FormatString("IDS_HEKILLEDME_WPN", szTmp,LTARRAYSIZE(szTmp),m_ClientInfo.GetPlayerName (nScorer),LoadString(szWpnID));
				}						
			}
			else
			{
				FormatString("IDS_HEKILLEDME", szTmp,LTARRAYSIZE(szTmp), m_ClientInfo.GetPlayerName (nScorer));
			}


		}

		if (IsMultiplayerGameClient())
		{
			g_pPlayerMgr->SetKiller(nScorer);
		}

		g_pGameMsgs->AddMessage(szTmp,type);
		if (bLocalMsg)
		{
			//removed from chat queue
			//g_pChatMsgs->AddMessage(szTmp,kMsgOtherTeam);
		}

	}
	else if (nScorer == nLocalID)
	{
		bLocalMsg = true;
		if (bTeamKill)
		{
			FormatString("IDS_IKILLEDHIM_TEAM", szTmp,LTARRAYSIZE(szTmp),m_ClientInfo.GetPlayerName (nVictim));
			hBroadcast = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "TeamKill");
		}
		else if (szWpnID)
		{
			if (bHeadshot)
			{
				FormatString("IDS_IKILLEDHIM_HS", szTmp,LTARRAYSIZE(szTmp),m_ClientInfo.GetPlayerName (nVictim),LoadString(szWpnID));
				hBroadcast = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "HeadShot");

			}
			else
			{
				FormatString("IDS_IKILLEDHIM_WPN", szTmp,LTARRAYSIZE(szTmp),m_ClientInfo.GetPlayerName (nVictim),LoadString(szWpnID));
				hBroadcast = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "Kill");
			}						
		}
		else
		{
			FormatString("IDS_IKILLEDHIM", szTmp,LTARRAYSIZE(szTmp), m_ClientInfo.GetPlayerName (nVictim));
		}

		g_pLTClient->CPrint("%S",szTmp);
		g_pGameMsgs->AddMessage(szTmp,type);
		if (bLocalMsg)
		{
			if (bTeamKill)
			{
				type = kMsgOtherTeam;
			}
			else
			{
				type = kMsgTeam;
			}
			//removed from chat queue
			//g_pChatMsgs->AddMessage(szTmp,type);
		}
	}
	else
	{
		if (nVictim == nScorer)
		{
			FormatString("IDS_HEKILLEDHIMSELF", szTmp,LTARRAYSIZE(szTmp), m_ClientInfo.GetPlayerName(nVictim));
		}
		else if( nScorer == INVALID_CLIENT || !pScorerCI )
		{
			FormatString("IDS_HEWASKILLED", szTmp,LTARRAYSIZE(szTmp), m_ClientInfo.GetPlayerName(nVictim));
		}
		else
		{
			if (bTeamKill)
			{
				FormatString("IDS_HEKILLEDHIM_TEAM", szTmp,LTARRAYSIZE(szTmp), m_ClientInfo.GetPlayerName (nScorer), m_ClientInfo.GetPlayerName (nVictim));
			}
			else if (szWpnID)
			{
				if (bHeadshot)
				{
					FormatString("IDS_HEKILLEDHIM_HS", szTmp,LTARRAYSIZE(szTmp),m_ClientInfo.GetPlayerName (nScorer), m_ClientInfo.GetPlayerName (nVictim),LoadString(szWpnID));
				}
				else
				{
					FormatString("IDS_HEKILLEDHIM_WPN", szTmp,LTARRAYSIZE(szTmp),m_ClientInfo.GetPlayerName (nScorer), m_ClientInfo.GetPlayerName (nVictim),LoadString(szWpnID));
				}						
			}
			else
			{
				FormatString("IDS_HEKILLEDHIM", szTmp,LTARRAYSIZE(szTmp), m_ClientInfo.GetPlayerName (nScorer), m_ClientInfo.GetPlayerName (nVictim));
			}
		}

		g_pLTClient->CPrint("%S",szTmp);
		g_pGameMsgs->AddMessage(szTmp,type);
	}

	if (hBroadcast)
	{
		CCharacterFX *pFX = g_pGameClientShell->GetLocalCharacterFX();
		if (pFX)
		{
			uint32 nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hBroadcast );
			int8 nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hBroadcast, Priority);
			pFX->PlayBroadcast(nBroadcastID, true, nLocalID, nPriority,false);
		}

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UpdatePostLoad
//
//	PURPOSE:	update the game state when a world has finished loading
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::UpdatePostLoad()
{
	// Handle multiplayer.
	if( IsMultiplayerGameClient( ))
	{
		// If we're ready to go, but we're waiting for others, then post the waiting for others text.
		if( g_pClientConnectionMgr->GetSentClientConnectionState() != eClientConnectionState_InWorld )
		{
			g_pClientConnectionMgr->SendClientInWorldMessage();
		}

		// Don't do anything until the server has put the player in the world.
		if( g_pPlayerMgr->GetPlayerState() == ePlayerState_None )
		{
			return;
		}

		// Make sure we have our team selected.
		if (GameModeMgr::Instance( ).m_grbUseTeams && !g_pClientConnectionMgr->HasSelectedTeam())
		{
			SwitchToScreen(SCREEN_ID_PLAYER_TEAM);
		}
		// Make sure we have our loadout selected.
		else if ( GameModeMgr::Instance( ).m_grbUseLoadout && !g_pClientConnectionMgr->HasSelectedLoadout())
		{
			SwitchToScreen(SCREEN_ID_PLAYER_LOADOUT);
		}
		// Good to go, start playing.
		else
		{
			g_pClientConnectionMgr->SendMultiPlayerInfo();
			g_pInterfaceMgr->ChangeState( GS_PLAYING );
		}
	}
	// Always go to postload screen in singleplayer.
	else
	{
		SwitchToScreen(SCREEN_ID_POSTLOAD);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::HandleChat
//
//	PURPOSE:	handle displaying a chat message
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::HandleChat( const wchar_t* pwsMsg, uint32 nClientID, uint8 nTeam )
{
	if (IsPlayerMuted(nClientID))
	{
		return true;
	}

	if( GameModeMgr::Instance( ).m_grbUseTeams && nTeam != INVALID_TEAM )
	{
		CLIENT_INFO *pLocalCI = m_ClientInfo.GetLocalClient();
		if( !pLocalCI )
			return false;

		if( pLocalCI->nTeamID != nTeam )
			return false;
	}

	g_pClientSoundMgr->PlayInterfaceDBSound("Chat");

	if (IsMultiplayerGameClient())
	{

		wchar_t wszChatMessage[128];

		// The dedicated server will have a ClientID of -1...
		if( nClientID == -1 )
		{
//			FormatString( "IDS_CHATMESSAGEHEADER", wszChatMessage, LTARRAYSIZE( wszChatMessage ), LoadString( "IDS_HOST" ));
			g_pChatMsgs->AddMessage( L"", pwsMsg, kMsgChat, kMsgChat );
		}
		else
		{
			CLIENT_INFO *pClientInfo = m_ClientInfo.GetClientByID( nClientID );
			if( !pClientInfo )
				return false;

			CCharacterFX* pCharFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID( nClientID );
			if( !pCharFX )
				return false;

			char const* pszChatMessageFormat = pCharFX->IsSpectating() ? "IDS_CHATMESSAGEHEADER_SPECTATOR" : "IDS_CHATMESSAGEHEADER";
			FormatString( pszChatMessageFormat, wszChatMessage, LTARRAYSIZE( wszChatMessage ), pClientInfo->sName.c_str());

			if( GameModeMgr::Instance( ).m_grbUseTeams )
			{
				bool bTeamMsg = nTeam != INVALID_TEAM;
				if (bTeamMsg)
				{
					//can only be from a teammate
					g_pChatMsgs->AddMessage( wszChatMessage, pwsMsg, kMsgTeam, kMsgTeam );
				}
				else
				{
					CLIENT_INFO* pCI = m_ClientInfo.GetClientByID(nClientID);
					if (pCI && pCI->nTeamID != INVALID_TEAM)
					{
						if (m_ClientInfo.IsLocalTeam(pCI->nTeamID))
						{
							//if the chatter is on our team, use the team color for their name
							g_pChatMsgs->AddMessage( wszChatMessage, pwsMsg, kMsgTeam, kMsgChat );
						}
						else
						{
							//not on our team, use the other team color for their name
							g_pChatMsgs->AddMessage( wszChatMessage, pwsMsg, kMsgOtherTeam, kMsgChat );
						}
					}
					else
					{
						//the speaker isn't on a team
						g_pChatMsgs->AddMessage( wszChatMessage, pwsMsg, kMsgChat, kMsgChat );
					}
				}
			}
			else
			{
				g_pChatMsgs->AddMessage( wszChatMessage, pwsMsg, kMsgChat, kMsgChat );
			}
		}

		CCharacterFX* pCFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID(nClientID);
		if (pCFX)
		{
			pCFX->HandleChat();
		}

	}
	else
	{
		g_pChatMsgs->AddMessage(pwsMsg,kMsgChat);
	};

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::IsChatting
//
//	PURPOSE:	Test if client is chatting...
//
// ----------------------------------------------------------------------- //

bool CInterfaceMgr::IsChatting( ) const
{
	return (g_pChatInput && g_pChatInput->IsVisible( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::AskToPatch()
//
//	PURPOSE:	Ask user whether they wish to update to the latest
//
// ----------------------------------------------------------------------- //

void CInterfaceMgr::AskToPatch()
{
	//if we've already asked bail out
	if (m_bHasAskedToPatch)
	{
		return;
	}

	char const* pszPatchURL = g_pInterfaceMgr->GetPatchURL();
	
	//if we don't have a patch, bail and indicate we're good to go
	if( LTStrEmpty( pszPatchURL ))
	{
		m_bHasAskedToPatch = true;
		bHasAnsweredPatchRequest = true;
		return;		
	}

	//if we have a patch, but haven't gotten an answer yet, show the dialog
	if (!g_pInterfaceMgr->IsMessageBoxVisible( ))
	{
		m_bHasAskedToPatch = true;
		MBCreate mb;
		mb.eType = LTMB_YESNO;
		mb.pFn = GetPatchCallBack;
		mb.pUserData = NULL;
		mb.nFlags = eMBFlag_IgnoreESC;
		g_pInterfaceMgr->ShowMessageBox("IDS_NEW_VERSION",&mb);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::MutePlayer()
//
//	PURPOSE:	Mute the specified client
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::MutePlayer(uint32 nID)
{
	if (!IsMultiplayerGameClient())
	{
		return;
	}

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);
	if (nID == nLocalID)
	{
		return;
	}

	m_setMutedClients.insert(nID);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::UnmutePlayer()
//
//	PURPOSE:	Unmute the specified client
//
// ----------------------------------------------------------------------- //
void CInterfaceMgr::UnmutePlayer(uint32 nID)
{
	if (!IsMultiplayerGameClient())
	{
		return;
	}

	PlayerIDSet::iterator iter = m_setMutedClients.find(nID);
	if (iter != m_setMutedClients.end())
	{
		m_setMutedClients.erase(iter);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMgr::IsPlayerMuted()
//
//	PURPOSE:	is the specified client muted?
//
// ----------------------------------------------------------------------- //
bool CInterfaceMgr::IsPlayerMuted(uint32 nID) const
{
	if (!IsMultiplayerGameClient())
	{
		return false;
	}

	PlayerIDSet::const_iterator iter = m_setMutedClients.find(nID);
	if (iter != m_setMutedClients.end())
	{
		return true;
	}

	return false;
}
