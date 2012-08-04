// ----------------------------------------------------------------------- //
//
// MODULE  : GameClientShell.cpp
//
// PURPOSE : Game Client Shell - Implementation
//
// CREATED : 9/18/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "PlayerMgr.h"
#include "AssertMgr.h"
#include "MsgIds.h"
#include "CommandIds.h"
#include "ClientUtilities.h"
#include "vkdefs.h"
#include "SoundTypes.h"
#include "VolumeBrushFX.h"
#include "client_physics.h"
#include "WinUtil.h"
#include "iltphysics.h"
#include "VarTrack.h"
#include "GameButes.h"
#include "SystemDependant.h"
#include "SurfaceFunctions.h"
#include "VehicleMgr.h"
#include "CharacterFX.h"
#include "iltsoundmgr.h"
#include "ClientWeaponMgr.h"
#include "CMoveMgr.h"
#include "PlayerCamera.h"
#include <stdarg.h>
#include <stdio.h>
#include "ClientConnectionMgr.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"
#include "ClientFXDB.h"
#include <sys/timeb.h>
#include "PerformanceTest.h"
#include "ScmdConsole.h"
#include "ScmdConsoleDriver_CShell.h"
#include "ClientConnectionMgr.h"
#include "iltrenderer.h"
#include "FrameStatConsoleInfo.h"
#include "PlayerBodyMgr.h"
#include "HUDMessageQueue.h"
#include "HUDMessageQueue.h"
#include "sys/win/mpstrconv.h"
#include "ShatterTypeDB.h"
#include "LTEulerAngles.h"
#include "bindmgr.h"
#include "ClientDB.h"
#include "ClientPhysicsCollisionMgr.h"
#include "SoundOcclusionDB.h"
#include "iltresourcemgr.h"
#include "iltresourceallocator.h"
#include "ModelDecalMgr.h"
#include "GameStreamingMgr.h"
#include "GameModeMgr.h"
#include "ModelDecalMgr.h"
#include <malloc.h>
#include "OrbitalScreenshotMgr.h"
#include "iltfilemgr.h"
#include "RenderTargetFX.h"
#include "ltprofileutils.h"
#include "WeaponFX.h"
#include "WeaponFXTypes.h"
#include "iperformancemonitor.h"
#include "performancestats.h"
#include "SoundMgr.h"
#include "SoundFilterDB.h"
#include "PerformanceLogMgr.h"
#include "GameRenderLayers.h"
#include "AnimationPropStrings.h"
#include "lttimeutils.h"
#include "PhysicsUtilities.h"
#include "PerformanceMgr.h"
#include "SpecialFXNotifyMessageHandler.h"
#include "LadderMgr.h"
#include "LadderFX.h"
#include "ClientVoteMgr.h"

#if !defined(PLATFORM_XENON)
#include "IGameSpy.h"
#endif // !PLATFORM_XENON

#if defined(PLATFORM_WIN32)
#include "mmsystem.h" // For timeGetTime (speed hacking prevention)
#endif // PLATFORM_WIN32

#include <algorithm>


#if !defined(PLATFORM_XENON)
#include "iltgameutil.h"
static ILTGameUtil *g_pLTGameUtil;
define_holder(ILTGameUtil, g_pLTGameUtil);
#endif // !defined(PLATFORM_XENON)

// Sample rate
unsigned short g_nSampleRate = 44100;

#if defined(PLATFORM_WIN32)

#ifdef STRICT
	WNDPROC g_pfnMainWndProc = NULL;
#else
	FARPROC	g_pfnMainWndProc = NULL;
#endif

#endif // PLATFORM_WIN32

#define WEAPON_MOVE_INC_VALUE_SLOW		0.025f
#define WEAPON_MOVE_INC_VALUE_FAST		0.05f

#define VK_WRITE_CAM_POS				VK_F4
#define VK_MAKE_SCREEN_SHOT				VK_F8
#define VK_MAKE_CUBIC_ENVMAP			VK_F12

//the name of the DLL that contains all of our clientFX information
#define CLIENT_FX_MODULE_NAME			"ClientFx.fxd"

//the location in our resource tree for all of our client fx files
#define CLIENT_FX_REL_RESOURCE_PATH		"ClientFx"

// convenience macro for executing the same function on all of the render target client fx mgrs.
#define FOREACH_RENDER_TARGET_MGR \
	for (RenderTargetClientFXMgrMap::iterator it=m_RenderTargetClientFXMgrs.begin(); it!=m_RenderTargetClientFXMgrs.end(); ++it) \
		it->second->

bool				g_bScreenShotMode   = false;
VarTrack			g_vtScreenShotMode;

HWND				g_hMainWnd = NULL;
RECT*				g_prcClip = NULL;

CGameClientShell*   g_pGameClientShell = NULL;

PhysicsState		g_normalPhysicsState;
PhysicsState		g_waterPhysicsState;
PhysicsState		g_playerNormalPhysicsState;
PhysicsState		g_playerWaterPhysicsState;

VarTrack			g_vtShowTimingTrack;
VarTrack			g_varStartLevelScreenFadeTime;
VarTrack			g_varStartLevelScreenFade;
VarTrack			g_vtUseSoundFilters;
VarTrack			g_vtMakeCubicEnvMapSize;
VarTrack			g_vtMakeCubicEnvMapName;
VarTrack			g_vtScreenShotPrefix;
VarTrack			g_vtScreenShotExtension;
VarTrack			g_vtApplyWorldOffset;
VarTrack			g_vtUserNotifiers;
VarTrack			g_vtUpdateClientFX;
VarTrack			g_vtStreamResources;
VarTrack			g_vtLoadUnprefetchedResources;

VarTrack			g_vtPTestMinFPS;
VarTrack			g_vtPTestMaxFPS;

extern CCheatMgr*	g_pCheatMgr;
extern VarTrack		g_vtFOVYNormal;
extern VarTrack		g_vtFOVAspectRatioScale;

VarTrack			g_vtDisplayImageScale;

VarTrack			g_vtEnableSimulationLog;

// couple required for XUI (even on non-Xenon builds)
VarTrack			g_vtXUIRender;
VarTrack			g_vtXUIUseOldHUD;

VarTrack			g_vtPhysicsSettleTime;
VarTrack			g_vtPhysicsSettleRate;

// Speed hack prevention variables
static _timeb g_StartTimeB;
static uint32 g_nStartClientTime = 0;
static uint32 g_nStartTicks = 0;

// keep alive update interval during world loading
static const uint32 k_nKeepAliveIntervalMS = 1000;

#if defined(PLATFORM_WIN32)

BOOL SetWindowSize(uint32 nWidth, uint32 nHeight);
BOOL HookWindow();
void UnhookWindow();

BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
LRESULT CALLBACK HookedWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#else // !PLATFORM_WIN32

BOOL SetWindowSize(uint32 nWidth, uint32 nHeight)
{
	LTUNREFERENCED_PARAMETER(nWidth);
	LTUNREFERENCED_PARAMETER(nHeight);
	return TRUE;
}
BOOL HookWindow() { return TRUE; }
void UnhookWindow() {}

BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	LTUNREFERENCED_PARAMETER(hwnd);
	LTUNREFERENCED_PARAMETER(hwndCursor);
	LTUNREFERENCED_PARAMETER(codeHitTest);
	LTUNREFERENCED_PARAMETER(msg);
	return TRUE;
}
LRESULT CALLBACK HookedWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LTUNREFERENCED_PARAMETER(hWnd);
	LTUNREFERENCED_PARAMETER(uMsg);
	LTUNREFERENCED_PARAMETER(wParam);
	LTUNREFERENCED_PARAMETER(lParam);
	return 0;
}

#endif // !PLATFORM_WIN32

void InitClientShell()
{
	// Hook up the AssertMgr

	CAssertMgr::Enable();

	// Get our ClientDE pointer

    _ASSERT(g_pLTClient);

	// Init our LT subsystems in non-SEM configurations
#if !defined(PLATFORM_SEM)
	g_pModelLT = g_pLTClient->GetModelLT();
	g_pPhysicsLT = g_pLTClient->Physics();
	g_pLTBase = static_cast<ILTCSBase*>(g_pLTClient);
	g_pCommonLT = g_pLTClient->Common();
#endif

	g_pLTClient->Physics()->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);

	g_pVersionMgr->Init( );
}

void TermClientShell()
{
	// Delete our client shell

	// Unhook the AssertMgr and let the CRT handle asserts once again

	CAssertMgr::Disable();
}


void FragSelfFn(int /*argc*/, char ** /*argv*/)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;
	SendEmptyServerMsg(MID_FRAG_SELF);
}

static void DoDamageFn(int argc, char** argv)
{
	if( argc == 2 )
	{
		const char* szTargetName = argv[0];
		const float fAmount = (float)atof(argv[1]);

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_DO_DAMAGE );
		cMsg.WriteString(szTargetName);
		cMsg.Writefloat(fAmount);
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}
	else
	{
		g_pLTClient->CPrint("DoDamage <target> <amount>");
	}
}

static void ClientFXFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if(argc == 1)
	{
		LTRigidTransform tTransform;
		tTransform.m_vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );
		tTransform.m_rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );
		CLIENTFX_CREATESTRUCT FxCs(argv[0], FXFLAG_LOOP, tTransform);

		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, FxCs, true);
	}
	else
	{
		g_pLTClient->CPrint("ClientFX <fxname>");
	}
}

//called to handle reloading of all client effect files
static void ReloadFXFn(int argc, char **argv)
{
	if(!g_pGameClientShell->ReloadClientFXFiles())
	{
		g_pLTClient->CPrint("Failed to reload all existing client effect files. Effects have been disabled.");
	}
}

static HTEXTURE g_hDisplayImageTexture = NULL;
static uint32 g_nDisplayImageWidth = 0;
static uint32 g_nDisplayImageHeight = 0;

static void DisplayImageFn(int argc, char **argv)
{
	// Release a texture if it exists already
	if( g_hDisplayImageTexture )
	{
		g_pLTClient->GetTextureMgr()->ReleaseTexture( g_hDisplayImageTexture );
		g_hDisplayImageTexture = NULL;
	}

	// Make sure we've got a correct number of parameters
	if( argc > 1 )
	{
		g_pLTClient->CPrint( "DisplayImage() -- Invalid number of parameters!" );
		return;
	}

	// Load the new texture!
	if( LT_OK != g_pLTClient->GetTextureMgr()->CreateTextureFromFile( g_hDisplayImageTexture, argv[ 0 ] ) )
	{
		g_pLTClient->CPrint( "DisplayImage() -- Failed to load texture '%s'!", argv[ 0 ] );
		return;
	}

	g_pLTClient->GetTextureMgr()->GetTextureDims( g_hDisplayImageTexture, g_nDisplayImageWidth, g_nDisplayImageHeight );
}

static void RenderDisplayImage()
{
	if( !g_hDisplayImageTexture )
	{
		return;
	}

	LT_POLYGT4 iPoly;

	iPoly.verts[ 0 ].pos.Init( 0.0f, 0.0f, 0.0f );
	iPoly.verts[ 1 ].pos.Init( ( g_nDisplayImageWidth * g_vtDisplayImageScale.GetFloat() ), 0.0f, 0.0f );
	iPoly.verts[ 2 ].pos.Init( ( g_nDisplayImageWidth * g_vtDisplayImageScale.GetFloat() ), ( g_nDisplayImageHeight * g_vtDisplayImageScale.GetFloat() ), 0.0f );
	iPoly.verts[ 3 ].pos.Init( 0.0f, ( g_nDisplayImageHeight * g_vtDisplayImageScale.GetFloat() ), 0.0f );

	iPoly.verts[ 0 ].uv.Init( 0.0f, 0.0f );
	iPoly.verts[ 1 ].uv.Init( 1.0f, 0.0f );
	iPoly.verts[ 2 ].uv.Init( 1.0f, 1.0f );
	iPoly.verts[ 3 ].uv.Init( 0.0f, 1.0f );

	iPoly.verts[ 0 ].rgba.Init( 255, 255, 255, 255 );
	iPoly.verts[ 1 ].rgba.Init( 255, 255, 255, 255 );
	iPoly.verts[ 2 ].rgba.Init( 255, 255, 255, 255 );
	iPoly.verts[ 3 ].rgba.Init( 255, 255, 255, 255 );

	g_pLTClient->GetDrawPrim()->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

	g_pLTClient->GetDrawPrim()->SetTexture( g_hDisplayImageTexture );
	g_pLTClient->GetDrawPrim()->DrawPrim( &iPoly, 1 );
}

void StimulusFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if( (argc == 1) || (argc == 2) )
	{
		// Message may contain a multiplier.
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_STIMULUS);
		if(argc == 1)
		{
			cMsg.Writeuint8(1);
			cMsg.WriteString(argv[0]);
		}
		else {
			cMsg.Writeuint8(2);
			cMsg.WriteString(argv[0]);
			cMsg.Writefloat(float(atof(argv[1])));	// multiplier.
		}
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);		
	}
	else {
		g_pLTClient->CPrint("Stimulus <stimulus type> [multiplier]");
	}
}

void RenderStimulusFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_RENDER_STIMULUS);
	if(argc == 1)
	{
		cMsg.Writeuint8( (uint8)atoi(argv[0]) );
	}
	else {
		cMsg.Writeuint8( 1 );
	}
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);		
}

void ObjectAlphaFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_OBJECT_ALPHA);
	if(argc == 2)
	{
		cMsg.WriteString(argv[0]);
		cMsg.Writefloat((float)atof(argv[1]));

		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);		
	}
}


void AddGoalFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_ADD_GOAL);
	if(argc >= 2)
	{
		cMsg.WriteString(argv[0]);
		cMsg.WriteString(argv[1]);
			
		cMsg.Writeuint32(argc - 2);
		int iArg = 2;
		while( iArg < argc )
		{
			cMsg.WriteString(argv[iArg++]);
		}

		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);		
	}
	else {
		g_pLTClient->CPrint("AddGoal <AI name> <goal> [name=val ...]");
	}
}

void RemoveGoalFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_REMOVE_GOAL);
	if(argc == 2)
	{
		cMsg.WriteString(argv[0]);
		cMsg.WriteString(argv[1]);
			
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);		
	}
	else 
	{
		g_pLTClient->CPrint("RemoveGoal <AI name> <goal>");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AIDebugFn
//
//	PURPOSE:	This console command enables routing of debug 
//			messages to AI subsystems such as the AIStimulusMgr
//			or AIWorkingMemoryCentral.
//
// ----------------------------------------------------------------------- //

void AIDebugFn( int argc, char **argv )
{
#ifndef _FINAL
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_AIDBUG);

	char buf[256];
	buf[0] = '\0';
	for (int i = 0; i < argc; i++)
	{
		LTStrCat(buf, "\"", LTARRAYSIZE(buf));
		LTStrCat(buf, argv[i], LTARRAYSIZE(buf));
		LTStrCat(buf, "\"", LTARRAYSIZE(buf));
		LTStrCat(buf, " ", LTARRAYSIZE(buf));
	}

	cMsg.WriteString(buf);
	
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);		

#endif// _FINAL
}

void ExitLevelFn(int /*argc*/, char ** /*argv*/)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (g_pCheatMgr)
	{
		CParsedMsgW cMsg( 0, NULL );
		g_pCheatMgr->Process( CHEAT_EXITLEVEL, cMsg );
	}
}

void NextSpawnPointFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_EVENT );
	cMsg.Writeuint8(kPENextSpawnPoint);
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

}
void PrevSpawnPointFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_EVENT );
	cMsg.Writeuint8(kPEPrevSpawnPoint);
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

void TeleportFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (argc < 3)
	{
        g_pLTClient->CPrint("Teleport x y z");
		return;
	}
	
	if (g_pPlayerMgr)
	{
		LTVector vPos;
		vPos.x = (float) atof(argv[0]);
		vPos.y = (float) atof(argv[1]);
		vPos.z = (float) atof(argv[2]);

		//handle the shift from the current world to the source world
		if((uint32)g_vtApplyWorldOffset.GetFloat(1.0f))
		{
			LTVector vOffset;
			g_pLTClient->GetSourceWorldOffset(vOffset);
			vPos -= vOffset;
		}


		g_pPlayerMgr->Teleport(vPos);
	}
}

void ChaseToggleFn(int /*argc*/, char ** /*argv*/)
{
	// Track the current execution shell scope for proper SEM behavior
 	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_CHASETOGGLE);
	}
}

void CmdFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (argc < 2)
	{
        g_pLTClient->CPrint("Cmd <command>");
		return;
	}

	// Send message to server...
	char buf[256];
	buf[0] = '\0';
	LTSNPrintF( buf, LTARRAYSIZE( buf ), "%s", argv[0]);
	for (int i=1; i < argc; i++)
	{
		LTStrCat(buf, " ", LTARRAYSIZE( buf ));
		LTStrCat(buf, "\"", LTARRAYSIZE( buf ));
		LTStrCat(buf, argv[i], LTARRAYSIZE( buf ));
		LTStrCat(buf, "\"", LTARRAYSIZE( buf ));
	}


	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CONSOLE_COMMAND);
	cMsg.WriteString(buf);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

}

void TriggerFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (argc < 2)
	{
        g_pLTClient->CPrint("Trigger <objectname> <message>");
		return;
	}

	// Send message to server...
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CONSOLE_TRIGGER);
    cMsg.WriteString(argv[0]);
    cMsg.WriteString(argv[1]);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

}

void ListFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (argc < 1 || !argv)
	{
        g_pLTClient->CPrint("List <classtype>");
		return;
	}

	// Send message to server...

	char buf[100];
	LTSNPrintF( buf, LTARRAYSIZE( buf ), "List %s", argv[0]);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CONSOLE_TRIGGER);
    cMsg.WriteString("");
    cMsg.WriteString(buf);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

}

void ExitGame(bool bResponse, uint32 /*nUserData*/)
{
	if (bResponse)
	{
        g_pLTClient->Shutdown();
	}
}

void InitSoundFn(int /*argc*/, char ** /*argv*/)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->InitSound();
	}
}

// __argc and __argv do not exist on xenon, but this function is suplied for mod
//  purposes, which also will not exist for xenon - JSC
#if !defined( PLATFORM_XENON )

// Gets mod from the command line.
static bool GetModFromCommandLine( char* pszMod, uint32 nModSize )
{
	if( !pszMod || !nModSize )
		return false;

	// Initialize out parameter.
	pszMod[0] = '\0';

	// look for an archcfg file specified on the command line
	if( __argc > 0 )
	{
		for (int32 nIndex = 0; nIndex < __argc - 1; ++nIndex) 
		{
			if (LTStrIEquals(__argv[nIndex], "-archcfg") && nIndex + 1 < __argc )
			{
				LTSNPrintF( pszMod, nModSize, __argv[nIndex + 1]);
				return true;
			}
		}
	}

	return false;
}

#endif

// Runs the given sp world.
bool ConsoleRunWorld( char const* pszWorldName )
{
	if( LTStrEmpty( pszWorldName ))
	{
		g_pLTBase->CPrint("No world string found after runworld token");
		return false;
	}
	g_pLTClient->CPrint("commandline runworld:\n %s",pszWorldName);

	// Initialize to the sp mission bute.
	if( !g_pMissionDB->Init( DB_Default_File ))
	{
		g_pLTBase->CPrint("Could not intialize mission information");
		return false;
	}

	g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );
	bool bOk = g_pClientConnectionMgr->SetupServerSinglePlayer( );
	bOk = bOk && g_pMissionMgr->StartGameFromLevel(pszWorldName);

	return bOk;
}

// Console command to run the world from console.
void ConsoleRunWorldFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (argc < 1 || !argv)
	{
		g_pLTClient->CPrint("ConsoleRunWorld <worldname>");
		return;
	}

	if( !ConsoleRunWorld( argv[0] ))
	{
		g_pLTBase->CPrint( "Error with ConsoleRunWorld." );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::CGameClientShell()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CGameClientShell::CGameClientShell()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	//AfxSetAllocStop(37803);

	m_pMissionMgr = NULL;
	m_pClientSaveLoadMgr = NULL;

	m_pModelDecalMgr = NULL;
	
	g_pGameClientShell = this;

    m_bMainWindowMinimized  = false;

	m_bResourceStreaming	= false;

    m_bGamePaused           = false;

    m_bTweakingWeapon       = false;
    m_bAdjust1stPersonCamera = false;

    m_bFirstUpdate          = true;
	m_bFirstPlayingUpdate	= false;
    m_bRestoringGame        = false;


	m_eDifficulty			= GD_NORMAL;

    m_bMainWindowFocus      = false;
	m_bRendererInit			= false;


	m_fNextSoundReverbTime	= 0.0f;
    m_bUseReverb            = false;
	m_fReverbLevel			= 0.0f;

	m_vLastReverbPos.Init();

	m_pClientMultiplayerMgr = NULL;

	m_bInWorld = false;

	m_bQuickSave = false;

	m_eSwitchingWorldsState = eSwitchingWorldsStateNone;


	m_fInitialServerTime = 0.0f;
	m_fInitialLocalTime = 0.0f;
	m_nSpeedCheatCounter = 0;

	m_bRunningPerfTest = false;
	m_pPerformanceTest = NULL;

	m_bLaunchedServerApp = false;

	m_nEntryCount = 0;

	m_fLastServerRealTime = 0.0f;

	m_nLastKeepAliveTime = 0;

	m_pPunkBusterClient	= NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::~CGameClientShell()
//
//	PURPOSE:	Destruction
//
// ----------------------------------------------------------------------- //

CGameClientShell::~CGameClientShell()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	GetInterfaceMgr( )->Term();

	if( m_pClientSaveLoadMgr )
	{
		debug_delete( m_pClientSaveLoadMgr );
		m_pClientSaveLoadMgr = NULL;
	}

	if( m_pMissionMgr )
	{
		m_pMissionMgr->Term();
		m_pMissionMgr = NULL;
	}

	GetPlayerMgr()->Term();
	
	m_LightScaleMgr.Term();
	m_UserNotificationMgr.Term();

	if( m_pClientMultiplayerMgr )
	{
		debug_delete( m_pClientMultiplayerMgr );
		m_pClientMultiplayerMgr = NULL;
	}

	if (m_pPerformanceTest)
	{
		debug_delete( m_pPerformanceTest );
		m_pPerformanceTest = NULL;
	}

	if (g_prcClip)
	{
		debug_delete(g_prcClip);
        g_prcClip = NULL;
	}

	if( m_pClientMultiplayerMgr )
	{
		debug_delete( m_pClientMultiplayerMgr );
		m_pClientMultiplayerMgr = NULL;
	}
	
	if( m_pModelDecalMgr )
	{
		debug_delete( m_pModelDecalMgr );
		m_pModelDecalMgr = NULL;
	}

	ShutdownAllRenderTargetClientFXMgrs();

	// Only show the endsplash if they didn't launch the serverapp.
	if( !m_bLaunchedServerApp )
	{
		LaunchApplication::LaunchEndSplashScreen();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::InitSound
//
//	PURPOSE:	Initialize the sounds
//
// ----------------------------------------------------------------------- //

void CGameClientShell::InitSound()
{

	InitSoundInfo soundInfo;

    uint32 dwAdvancedOptions = GetInterfaceMgr( )->GetAdvancedOptions();
	if (!(dwAdvancedOptions & AO_SOUND)) return;

    soundInfo.Init();

	// Reload the sounds if there are any...

	soundInfo.m_dwFlags	= INITSOUNDINFOFLAG_RELOADSOUNDS;


	// Get the maximum number of 3d voices to use.
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("Max3DVoices");
	if (hVar)
	{
	    soundInfo.m_nNum3DVoices = (uint8)g_pLTClient->GetConsoleVariableFloat(hVar);
	}
	else
	{
		soundInfo.m_nNum3DVoices = 32;
	}

#ifdef PLATFORM_WIN32
	Sound3DProvider *pSound3DProviderList, *pSound3DProvider;
	uint32 dwProviderID;
	char sz3dSoundProviderName[_MAX_PATH + 1];

	// Get the 3d sound provider id....
	if (!(dwAdvancedOptions & AO_HARDWARESOUND))
	{
		dwProviderID = SOUND3DPROVIDERID_NONE;
	}
	else
	{
		dwProviderID = SOUND3DPROVIDERID_UNKNOWN;
	}

	// if hardware isn't turned off, try and use hardware
	if ( dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
	{
		sz3dSoundProviderName[0] = 0;
        hVar = g_pLTClient->GetConsoleVariable("3DSoundProviderName");
		if ( hVar )
		{
            LTStrCpy( sz3dSoundProviderName, g_pLTClient->GetConsoleVariableString( hVar ), LTARRAYSIZE(sz3dSoundProviderName));
		}
		else
		{
			// try DX hardware
		    LTStrCpy( sz3dSoundProviderName, "DirectSound Hardware", LTARRAYSIZE(sz3dSoundProviderName) );
		}
	}

	// See if the provider exists....
	if ( dwProviderID != SOUND3DPROVIDERID_NONE )
	{
        ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->GetSound3DProviderLists( pSound3DProviderList, false, soundInfo.m_nNum3DVoices );
		if ( !pSound3DProviderList )
		{
			return;
		}

		pSound3DProvider = pSound3DProviderList;
		while ( pSound3DProvider )
		{
			// If the provider is selected by name, then compare the names.
			if (  dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
			{
				if ( LTStrCmp(( const char * )sz3dSoundProviderName, ( const char * )pSound3DProvider->m_szProvider ) == 0 )
					break;
			}
			// Or compare by the id's.
			else if ( pSound3DProvider->m_dwProviderID == dwProviderID )
				break;

			// Not this one, try next one.
			pSound3DProvider = pSound3DProvider->m_pNextProvider;
		}

		// Check if we found one.
		if (pSound3DProvider)
		{
			// Use this provider.
			LTStrCpy( soundInfo.m_sz3DProvider, pSound3DProvider->m_szProvider, LTARRAYSIZE(soundInfo.m_sz3DProvider));
		}

        ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->ReleaseSound3DProviderList(pSound3DProviderList);
	}
	else 
	{
		soundInfo.m_sz3DProvider[0] = 0;
	}
#endif // PLATFORM_WIN32

	// Get the maximum number of sw voices to use.
    hVar = g_pLTClient->GetConsoleVariable("MaxSWVoices");
	if (hVar)
	{
        soundInfo.m_nNumSWVoices = (uint8)g_pLTClient->GetConsoleVariableFloat(hVar);
	}
	else
	{
		soundInfo.m_nNumSWVoices = 104;	//32;  -- boosting the max # of SW channels
										// The engine will limit this with performance
										// so this is the highwater mark. -- terry
	}

	soundInfo.m_nSampleRate		= g_nSampleRate;
	soundInfo.m_nBitsPerSample	= 16;
	soundInfo.m_nVolume			= SOUND_BASE_VOL;

#pragma MESSAGE( "16bit sound is hardcoded on" )

	WriteConsoleBool("sound16bit",true);
	if ( !GetConsoleBool("sound16bit",true) )
	{
		soundInfo.m_dwFlags |= INITSOUNDINFOFLAG_CONVERT16TO8;
	}

	soundInfo.m_fDistanceFactor = 1.0f / 100.0f;
	soundInfo.m_fDopplerFactor = 1.0f;

	// Go initialize the sounds...

    m_bUseReverb = false;
    if (((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->InitSound(&soundInfo) == LT_OK)
	{
		if (soundInfo.m_dwResults & INITSOUNDINFORESULTS_REVERB)
		{
            m_bUseReverb = true;
		}

        hVar = g_pLTClient->GetConsoleVariable("ReverbLevel");
		if (hVar)
		{
            m_fReverbLevel = g_pLTClient->GetConsoleVariableFloat(hVar);
		}
		else
		{
			m_fReverbLevel = ClientDB::Instance().GetFloat( ClientDB::Instance().GetClientSharedRecord(), CDB_ClientShared_Reverb);
		}

		ReverbProperties reverbProperties;
		reverbProperties.m_dwParams = REVERBPARAM_VOLUME;
		reverbProperties.m_fVolume = m_fReverbLevel;
        ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetReverbProperties(&reverbProperties);

		// set up the channel limits
		int32 nChannelLimits[MAX_MIXER_CHANNELS];
		int32 i;

		for (i=0; i < MAX_MIXER_CHANNELS; i++)
		{
			nChannelLimits[i] = 0;
		}
		// normally, one would set the limit levels here, but since
		// we're only doing impacts, and it's overriden in the engine
		// so it can use a console variable for tuning right now, this
		// section is left blank. -- Terry
		//nChannelLimits[PLAYSOUND_MIX_WEAPON_IMPACTS] = g_CV_MaxImpactSounds;

		((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetMixerChannelLimits(nChannelLimits, MAX_MIXER_CHANNELS);
	}


	// [KLS 9/10/02] Since we just reinitialized the sound system we need to make sure
	// any lip-flap sounds that are currently playing are updated since their data
	// may have changed and is accessed directly by the game...

	ResetCharacterFXSoundData();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ResetCharacterFXSoundData
//
//	PURPOSE:	Tell all the CharacterFX to update their lip-flap
//				sound data...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ResetCharacterFXSoundData()
{
	CSpecialFXList* pList = m_sfxMgr.GetFXList(SFX_CHARACTER_ID);
	if (!pList) return;

	int nNumChars = pList->GetSize();

	for (int i=0; i < nNumChars; i++)
	{
		if ((*pList)[i])
		{
			CCharacterFX* pChar = (CCharacterFX*)(*pList)[i];
			pChar->ResetSoundBufferData();
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEngineInitialized
//
//	PURPOSE:	Called after engine is fully initialized
//				Handle object initialization here
//
// ----------------------------------------------------------------------- //

uint32 CGameClientShell::OnEngineInitialized(LTGUID *pAppGuid, RMode *pMode)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// check to see if we need to change the default user directory 
	HCONSOLEVAR hUserDirectoryVar = g_pLTClient->GetConsoleVariable("UserDirectory");
	const char* pszUserDirectory = g_pLTClient->GetConsoleVariableString(hUserDirectoryVar);
	if (pszUserDirectory)
	{
		LTFileOperations::SetUserDirectory(pszUserDirectory);
	}

	if(!InitResourceMgr())
	{
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Unable to initialize the main resource manager" ));
		return LT_ERROR;
	}

	InitClientShell();

	//CWinUtil::DebugBreak();

	*pAppGuid = GAMEGUID;


	float fStartTime = CWinUtil::GetTime();

	if (!g_hMainWnd)
	{
		HookWindow();
		SetWindowSize(pMode->m_Width, pMode->m_Height);
	}


	// Initialize all the global bute mgrs...

	if (!m_GlobalMgr.Init())
	{
		return LT_ERROR;
	}

	// Initialize global console variables...



	g_vtShowTimingTrack.Init(g_pLTClient, "ShowTiming", NULL, 0.0f);

	g_varStartLevelScreenFade.Init(g_pLTClient, "ScreenFadeAtLevelStart", NULL, 1.0f);
	g_varStartLevelScreenFadeTime.Init(g_pLTClient, "ScreenFadeInAtLevelStartTime", NULL, 1.5f);

	g_vtUseSoundFilters.Init(g_pLTClient, "SoundFilters", NULL, 1.0f);

	g_vtMakeCubicEnvMapSize.Init(g_pLTClient, "MakeCubicEnvMapSize", NULL, 256.0f);
	g_vtMakeCubicEnvMapName.Init(g_pLTClient, "MakeCubicEnvMapName", "CubicEnvMap." RESEXT_TEXTURE, 0.0f);
	g_vtScreenShotPrefix.Init(g_pLTClient, "ScreenShotPrefix", "Screenshot", 0.0f);
	g_vtScreenShotExtension.Init(g_pLTClient, "ScreenShotExtension", "bmp", 0.0f);
	g_vtApplyWorldOffset.Init(g_pLTClient, "ApplyWorldOffset", NULL, 1.0f);
	g_vtUserNotifiers.Init(g_pLTClient, "UserNotifiers", NULL, 1.0f);
	g_vtUpdateClientFX.Init(g_pLTClient, "UpdateClientFX", NULL, 1.0f);

	g_vtPTestMinFPS.Init(g_pLTClient, "PerformanceMinTestFPS", NULL, 25.0f);
	g_vtPTestMaxFPS.Init(g_pLTClient, "PerformanceMaxTestFPS", NULL, 40.0f);
	g_vtScreenShotMode.Init(g_pLTClient, "ScreenShotMode", NULL, 0.0f);

	HCONSOLEVAR hIsSet = g_pLTClient->GetConsoleVariable("UpdateRateInitted");
	if (!hIsSet || g_pLTClient->GetConsoleVariableFloat(hIsSet) != 1.0f)
	{
		// Initialize the update rate.
		g_pLTClient->SetConsoleVariableFloat("UpdateRateInitted", 1.0f);
		g_pLTClient->SetConsoleVariableFloat("UpdateRate", 60.0f);
	}

	m_cheatMgr.Init();
	m_LightScaleMgr.Init();

	// would be nice to move these to a centralized registry function

	g_pLTClient->RegisterConsoleProgram("Cmd", CmdFn);
	g_pLTClient->RegisterConsoleProgram("Trigger", TriggerFn);
	g_pLTClient->RegisterConsoleProgram("List", ListFn);
	g_pLTClient->RegisterConsoleProgram("FragSelf", FragSelfFn);
	g_pLTClient->RegisterConsoleProgram("DoDamage", DoDamageFn);
	g_pLTClient->RegisterConsoleProgram("InitSound", InitSoundFn);
	g_pLTClient->RegisterConsoleProgram("ExitLevel", ExitLevelFn);
	g_pLTClient->RegisterConsoleProgram("Teleport", TeleportFn);
	g_pLTClient->RegisterConsoleProgram("ChaseToggle", ChaseToggleFn);
	g_pLTClient->RegisterConsoleProgram("Stimulus", StimulusFn);
	g_pLTClient->RegisterConsoleProgram("RenderStimulus", RenderStimulusFn);
	g_pLTClient->RegisterConsoleProgram("AddGoal", AddGoalFn);
	g_pLTClient->RegisterConsoleProgram("RemoveGoal", RemoveGoalFn);
	g_pLTClient->RegisterConsoleProgram("AIDebug", AIDebugFn);
	g_pLTClient->RegisterConsoleProgram("Alpha", ObjectAlphaFn);
	g_pLTClient->RegisterConsoleProgram("ClientFX", ClientFXFn);
	g_pLTClient->RegisterConsoleProgram("ReloadFX", ReloadFXFn);
	g_pLTClient->RegisterConsoleProgram("ConsoleRunWorld", ConsoleRunWorldFn);
	g_pLTClient->RegisterConsoleProgram("NextSpawnPoint", NextSpawnPointFn);
	g_pLTClient->RegisterConsoleProgram("PrevSpawnPoint", PrevSpawnPointFn);

	g_pLTClient->RegisterConsoleProgram( "DisplayImage", DisplayImageFn );
	g_vtDisplayImageScale.Init( g_pLTClient, "DisplayImageScale", NULL, 1.0f );

	// The amount of time to spend settling physics at the beginning of the level.
	g_vtPhysicsSettleTime.Init( g_pLTClient, "PhysicsSettleTime", NULL, 0.0f );
	// The frame rate of the physics settling time.  This determines how fine a timeslice to give.
	g_vtPhysicsSettleRate.Init( g_pLTClient, "PhysicsSettleRate", NULL, 1.0f / 30.0f );


	//setup the orbital screenshot console programs
	COrbitalScreenshotMgr::Singleton().InitConsolePrograms();

	// Make sure the save directory exists...
	char strUserDirectory[MAX_PATH];
	g_pLTClient->FileMgr()->GetAbsoluteUserFileName( "", strUserDirectory, MAX_PATH );

	std::string strSaveDirectory = strUserDirectory;
	strSaveDirectory += "Save";

	if (!CWinUtil::DirExist(strSaveDirectory.c_str()))
	{
		CWinUtil::CreateDir(strSaveDirectory.c_str());
	}

	// Add to NumRuns count...
	uint32 nGameRuns = LTProfileUtils::ReadUint32( "Game", "GameRuns", 0, g_pVersionMgr->GetGameSystemIniFile());
	nGameRuns++;
	LTProfileUtils::WriteUint32( "Game", "GameRuns", nGameRuns, g_pVersionMgr->GetGameSystemIniFile());

	if( !ClientPhysicsCollisionMgr::Instance().Init( ))
	{
		return LT_ERROR;
	}

	// Make sure they'll be able to render

	if (!TestVideoCardCompatibility())
	{
		g_pLTClient->ShutdownWithMessage( LoadString( "IDS_VIDEO_CARD_UNSUPPORTED" ));
		return LT_ERROR;
	}

	//setup our render mode to only allow hardware TnL rendering modes at 32 bpp
	RMode rMode;

	rMode.m_Width		= pMode->m_Width;
	rMode.m_Height		= pMode->m_Height;
	rMode.m_BitDepth	= 32;
	rMode.m_bHWTnL		= true;
	rMode.m_pNext		= NULL;

	LTStrCpy(rMode.m_DeviceName, pMode->m_DeviceName, LTARRAYSIZE(rMode.m_DeviceName));

	// Initialize the renderer
	if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
	{
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Unable to initialize the renderer to a valid video mode" ));
		return LT_ERROR;
	}

	// init a couple XUI console vars (required for non-Xenon builds too)
	g_vtXUIRender.Init(g_pLTClient, "XUIRender", NULL, 0.0f);
	g_vtXUIUseOldHUD.Init(g_pLTClient, "XUIUseOldHUD", NULL, 1.0f);

	// Initialize the game streaming system
	CGameStreamingMgr::Singleton().Init();

	// Init the ClientFX Database
	if(!CClientFXDB::GetSingleton().LoadFxDll(CLIENT_FX_MODULE_NAME, true))
	{
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Unable to load the ClientFX DLL" ));
		return LT_ERROR;
	}

	//now load in the client fx files
	if(!CClientFXDB::GetSingleton().LoadFxFilesInDir(g_pLTClient, CLIENT_FX_REL_RESOURCE_PATH))
	{
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Unable to load the ClientFX effect files" ));
		return LT_ERROR;
	}

	// Init the ClientFX mgr... (this must become before most other classes)
	if( !m_SimulationTimeClientFXMgr.Init( g_pLTClient, SimulationTimer::Instance()))
	{
		// Make sure ClientFX.fxd is built and in the game dir
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Could not init SimulationClientFXMgr!" ));
		return LT_ERROR;
	}

	// Init the ClientFX mgr... (this must become before most other classes)
	if( !m_RealTimeClientFXMgr.Init( g_pLTClient, RealTimeTimer::Instance()))
	{
		// Make sure ClientFX.fxd is built and in the game dir
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Could not init RealTimeClientFXMgr!" ));
		return LT_ERROR;
	}

	for (RenderTargetClientFXMgrMap::iterator it=m_RenderTargetClientFXMgrs.begin(); it!=m_RenderTargetClientFXMgrs.end(); ++it)
	{
		bool bUseSimTimer = it->first.second;
		EngineTimer& Timer = bUseSimTimer ? (EngineTimer&)SimulationTimer::Instance() : (EngineTimer&)RealTimeTimer::Instance();

		if( !it->second->Init(g_pLTClient,Timer) )
		{
			// Make sure ClientFX.fxd is built and in the game dir
			g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Could not init RenderTargetClientFXMgrs!" ));
			return LT_ERROR;
		}
	}

	// Init the DamageFX mgr...
	if( !CDamageFXMgr::Instance().Init() )
	{
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Could not init DamageFXMgr!" ));
		return LT_ERROR;
	}

	// Assume sp for now until the user selects a mode.
	HRECORD hGameModeRecord = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( GameModes ).GetCategory( ), GameModeMgr::GetSinglePlayerRecordName( ));
	if( !GameModeMgr::Instance().ResetToMode( hGameModeRecord ))
	{
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Invalid single player game mode" ));
		return LT_ERROR;
	}

	// Mission stuff...
	m_pMissionMgr = debug_new(CMissionMgr);
	if (!m_pMissionMgr || !m_pMissionMgr->Init())
	{
		// Don't call ShutdownWithMessage since MissionMgr will have called
		// that, so calling it here will overwrite the message...
		return LT_ERROR;
	}

	m_pClientSaveLoadMgr = debug_new( CClientSaveLoadMgr );
	if( !m_pClientSaveLoadMgr )
	{
		// Don't call ShutdownWithMessage since MissionMgr will have called
		// that, so calling it here will overwrite the message...
		return LT_ERROR;
	}

	m_pModelDecalMgr = debug_new( CGameModelDecalMgr );
	if( !m_pModelDecalMgr )
	{
		return LT_ERROR;
	}


	// Interface stuff...
	if (!GetInterfaceMgr( ) || !GetInterfaceMgr( )->Init())
	{
		// Don't call ShutdownWithMessage since InterfaceMgr will have called
		// that, so calling it here will overwrite the message...
		return LT_ERROR;
	}

	// Initialize the user notification manager with the default notifications
#ifndef _FINAL
	InstallUserNotifiers(m_UserNotificationMgr);
#endif

	//Setup the console frame info manager
	CFrameStatConsoleInfo::Init();

	//setup our renderer frame statistics
	g_pLTClient->GetRenderer()->SetFrameStats(&m_RendererFrameStats);

	// Player stuff...
	if (!GetPlayerMgr() || !GetPlayerMgr()->Init())
	{
		// Don't call ShutdownWithMessage since PlayerMgr will have called
		// that, so calling it here will overwrite the message...
		return LT_ERROR;
	}

	//We need to make sure to setup the camera for the FX Mgr
	m_SimulationTimeClientFXMgr.SetCamera(g_pPlayerMgr->GetPlayerCamera()->GetCamera());
	m_RealTimeClientFXMgr.SetCamera(g_pPlayerMgr->GetPlayerCamera()->GetCamera());
	FOREACH_RENDER_TARGET_MGR SetCamera(g_pPlayerMgr->GetPlayerCamera()->GetCamera());

	if (!m_Mixer.IsInitialized())
	{
		m_Mixer.Init(g_pLTClient);
	}


	// Initialize the global physics states...
	g_normalPhysicsState.m_vGravityAccel.Init(0.0f, DEFAULT_WORLD_GRAVITY, 0.0f);
	g_normalPhysicsState.m_fVelocityDampen = 0.5f;
	g_playerNormalPhysicsState.m_vGravityAccel.Init(0.0f, DEFAULT_WORLD_GRAVITY, 0.0f);
	g_playerNormalPhysicsState.m_fVelocityDampen = 0.5f;

	// Underwater is just half the force of above water
	g_waterPhysicsState.m_vGravityAccel			= g_normalPhysicsState.m_vGravityAccel * 0.5f;
	g_waterPhysicsState.m_fVelocityDampen		= g_normalPhysicsState.m_fVelocityDampen * 0.5f;
	g_playerWaterPhysicsState.m_vGravityAccel	= g_waterPhysicsState.m_vGravityAccel;
	g_playerWaterPhysicsState.m_fVelocityDampen	= g_waterPhysicsState.m_fVelocityDampen;


	m_pClientMultiplayerMgr = debug_new( ClientConnectionMgr );

	ClientVoteMgr::Instance().Init();

	// Init the special fx mgr...
	if (!m_sfxMgr.Init(g_pLTClient))
	{
		g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("Could not initialize SFXMgr!"));
		return LT_ERROR;
	}

	m_bQuickSave = false;

#if !defined( PLATFORM_XENON ) // no mods on Xenon
	// Get the name of the mod we want to play.  If no mod specified then we consider that the 'Retail' mod...
	char szMod[MAX_PATH];

	// __argc and __argv do not exist on xenon, but this function is suplied for mod
	//  purposes, which also will not exist for xenon - JSC

	if( GetModFromCommandLine( szMod, LTARRAYSIZE( szMod )))
	{
		g_pClientConnectionMgr->SetModName( szMod );
	}
	else
	{
		g_pClientConnectionMgr->SetModName( RETAIL_MOD_NAME );
	}	

#else // !PLATFORM_XENON
	
	g_pClientConnectionMgr->SetModName( RETAIL_MOD_NAME );

#endif // !PLATFORM_XENON

	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("host");
	if ( hVar )
	{
		int nHost = atoi(g_pLTClient->GetConsoleVariableString( hVar ));

		// Use the settings from the current profile to determine what the game will be...

		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile( );
		if( nHost && pProfile )
		{
			bool bOk = true;

			// Initialize the mission mgr with the appropriate file based on game type...
			char szPath[MAX_PATH*2];
			LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
			LTStrCat( szPath, FILE_PATH_SEPARATOR, LTARRAYSIZE( szPath ));
			LTStrCat( szPath, MDB_MP_File, LTARRAYSIZE( szPath ));
			if( !g_pMissionDB->Init( szPath ))
			{
				g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("Could not load mission bute %s."), MPA2W(MDB_MP_File).c_str() );
				return LT_ERROR;
			}

			// Start the server..

			bOk = bOk && g_pClientConnectionMgr->SetupServerHost( );
			bOk = bOk && g_pMissionMgr->StartGameNew();

			if( !bOk )
			{
				// drop them into the host menu
				GetInterfaceMgr( )->LoadFailed();
				GetInterfaceMgr( )->SwitchToScreen(SCREEN_ID_HOST);

				MBCreate mb;
				GetInterfaceMgr( )->ShowMessageBox("IDS_NOLOADLEVEL",&mb);
			}
		}
		else
		{
			// Just treat like normal game...

			GetInterfaceMgr( )->ChangeState(GS_SPLASHSCREEN);
		}
	}

	if( !hVar )
	{
		hVar = g_pLTClient->GetConsoleVariable("runworld");
		if( hVar )
		{
			if( !ConsoleRunWorld( g_pLTClient->GetConsoleVariableString(hVar)))
			{
				g_pLTBase->CPrint( "Could not do runworld command line." );
			}
		}
	}

	if( !hVar )
	{
		if (GetConsoleInt("skiptitle",0))
		{
			GetInterfaceMgr( )->SwitchToScreen(SCREEN_ID_MAIN);
		}
		else
		{
			GetInterfaceMgr( )->ChangeState(GS_SPLASHSCREEN);
		}
	}

#if defined(_FINAL) && defined(_DEMO)
	WriteConsoleInt( "ConsoleEnable", 0 );
#endif // _DEMO


	m_pPerformanceTest = debug_new(CPerformanceTest);

	// Intialize the scmd handler.
	static ScmdConsoleDriver_CShell scmdConsoleDriver_CShell;
	if( !ScmdConsole::Instance( ).Init( scmdConsoleDriver_CShell ))
	{
		return LT_ERROR;
	}

	//force construction of DM mission file
	g_pMissionDB->Init(DB_Default_File);
	g_pMissionDB->CreateMPDB();

	m_VersionMgr.Update();

	//setup our performance logging system
	CPerformanceLogMgr::Init();

	//setup our rendering layers
	InitRenderingLayers();

	//with the update of the version manager, it might have changed the gore setting,
	//so make sure that the FX managers know about the change
	UpdateGoreSettings();

	// Make sure the specialfx handler is initialized.
	SpecialFXNotifyMessageHandler::Instance( ).Init();

#ifndef _FINAL
	// open simulation log file
	g_vtEnableSimulationLog.Init(g_pLTClient, "EnableSimulationLog", NULL, 0);
	if (g_vtEnableSimulationLog.GetFloat())
	{
		m_cSimulationLogFile.Open(g_pszSimulationLogFilename, false);
	}
#endif

	// Determine how long it took to initialize the game...
	char strTimeDiff[64];
	LTSNPrintF(strTimeDiff, LTARRAYSIZE(strTimeDiff), "Game initialized in %f seconds.\n", CWinUtil::GetTime() - fStartTime);
	CWinUtil::DebugOut(strTimeDiff);

	// initialize PunkBuster
	g_pGameClientShell->InitializePunkBuster();


	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEngineTerm()
//
//	PURPOSE:	Called before the engine terminates itself
//				Handle object destruction here
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnEngineTerm()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	UnhookWindow();

	//clean up our performance logging system
	CPerformanceLogMgr::Term();

	// Make sure the mixer is shut down..
	m_Mixer.Term();

	//Make sure that the FX Mgr isn't still holding on to our camera
	m_SimulationTimeClientFXMgr.SetCamera(NULL);
	m_RealTimeClientFXMgr.SetCamera(NULL);
	FOREACH_RENDER_TARGET_MGR SetCamera(NULL);

	//uninstall our renderer frame stats so there aren't any dangling pointers
	g_pLTClient->GetRenderer()->SetFrameStats(NULL);

	ClientPhysicsCollisionMgr::Instance().Term( );

	// Kill the playermgr.
	if( g_pPlayerMgr )
		g_pPlayerMgr->Term( );

	// Delete the movemgr.
	if( g_pMoveMgr )
		g_pMoveMgr->Term();

	// Kill the interfacemgr.  This prevents the interfacemgr from doing a bunch of
	// reset when the engine tells us about a possible disconnect.
	if( g_pInterfaceMgr )
		g_pInterfaceMgr->Term();

	TermClientShell();

	// shut down the game streaming system
	CGameStreamingMgr::Singleton().Term();

	//free our resource manager
	TermResourceMgr();

	// terminate PunkBuster
	TerminatePunkBuster();

	// shutdown the GameSpyBrowser if necessary
	if( m_pClientMultiplayerMgr )
		m_pClientMultiplayerMgr->TermBrowser();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnEvent(uint32 dwEventID, uint32 dwParam)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	switch(dwEventID)
	{
		// Client disconnected from server.  dwParam will
		// be a error flag found in de_codes.h.

		case LTEVENT_DISCONNECT :
		{
			m_pClientMultiplayerMgr->UnloadMultiplayerOverrides();
			SetWorldNotLoaded();

			SimulationTimer::Instance( ).PauseTimer( false );
			GameTimeTimer::Instance( ).PauseTimer( false );
			m_eSwitchingWorldsState = eSwitchingWorldsStateNone;

			m_pMissionMgr->ClearMapList();

		} break;

        case LTEVENT_LOSTFOCUS:
		{
			m_bMainWindowFocus = FALSE;
			
		}
		break;

		case LTEVENT_GAINEDFOCUS:
		{
			m_bMainWindowFocus = TRUE;

			// we need to restore the cursor
			if (g_pCursorMgr)
				g_pCursorMgr->SetDefaultCursor();
		}
		break;

        case LTEVENT_RENDERTERM:
		{
			m_bMainWindowFocus = FALSE;
			m_bRendererInit = false;
		}
		break;

		case LTEVENT_RENDERINIT:
		{
			m_bMainWindowFocus = TRUE;
			m_bRendererInit = true;

#if defined(PLATFORM_WIN32)

			// Clip the cursor if we're NOT in a window...

            HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("Windowed");
			BOOL bClip = TRUE;
			if (hVar)
			{
                float fVal = g_pLTClient->GetConsoleVariableFloat(hVar);
				if (fVal == 1.0f)
				{
					bClip = FALSE;
				}
			}

			if (bClip)
			{
				if (!g_prcClip)
				{
					g_prcClip = debug_new(RECT);
				}

				
				if (!g_hMainWnd)
				{
					HookWindow();
				}

				GetWindowRect(g_hMainWnd, g_prcClip);
				ClipCursor(g_prcClip);
			}

			if (g_pCursorMgr)
				g_pCursorMgr->Init();

#endif // PLATFORM_WIN32

		}
		break;
	}

	if (GetInterfaceMgr( ))
		GetInterfaceMgr( )->OnEvent(dwEventID, dwParam);
	if( g_pClientConnectionMgr )
		g_pClientConnectionMgr->OnEvent(dwEventID, dwParam);
}


LTRESULT CGameClientShell::OnObjectMove(HOBJECT hObj, bool bTeleport, LTVector *pPos)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;
	if( !g_pPlayerMgr )
		return LT_OK;
	return g_pPlayerMgr->GetMoveMgr()->OnObjectMove(hObj, bTeleport, pPos);
}


LTRESULT CGameClientShell::OnObjectRotate(HOBJECT hObj, bool bTeleport, LTRotation *pNewRot)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;
	m_sfxMgr.OnObjectRotate( hObj, bTeleport, pNewRot );
	if( !g_pPlayerMgr )
		return LT_OK;
	return g_pPlayerMgr->GetMoveMgr()->OnObjectRotate(hObj, bTeleport, pNewRot);
}

LTRESULT CGameClientShell::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;
	m_sfxMgr.OnTouchNotify(hMain, pInfo, forceMag);
	return LT_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEnterWorld()
//
//	PURPOSE:	Handle entering world
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnEnterWorld()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// The synchronized server timer needs to be based off the real time timer so
	// it won't be affected by changes to the simulation time.
	m_swtServerRealTimeAccumulator.SetEngineTimer( RealTimeTimer::Instance( ));
	m_fLastServerRealTime = 0.0f;

	// Reset our speed hack.
	g_nStartTicks = 0;

	// Make sure we think we're done with the previous level.
	g_pMissionMgr->FinishExitLevel();

	// In world now.
	m_bInWorld = true;

	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->ResumeSounds();

	ClientPhysicsCollisionMgr::Instance().PostStartWorld( );

	// Set the stair height gotten from the server...
	
	float fStairHeight = DEFAULT_STAIRSTEP_HEIGHT;
	g_pLTClient->GetSConValueFloat( STAIR_STEP_HEIGHT_CVAR, fStairHeight );
	g_pPhysicsLT->SetStairHeight( fStairHeight );


	m_bFirstPlayingUpdate = true;

	m_LightScaleMgr.Init();

	CBindMgr::GetSingleton().ClearAllCommands();

	GetInterfaceMgr( )->OnEnterWorld(m_bRestoringGame);
	GetPlayerMgr()->OnEnterWorld();

	//clear status for relevant cheats (i.e. god mode)
	g_pCheatMgr->OnEnterWorld();

    m_bRestoringGame = false;

	m_vLastReverbPos.Init();

	MirrorSConVar( "RespawnWaitTime", "RespawnWaitTime" );
	MirrorSConVar( "RespawnMultiWaitTime", "RespawnMultiWaitTime" );

	//update our performance log so that reports generated will be associated with this world
	CPerformanceLogMgr::SetCurrentLevel(g_pMissionMgr->GetCurrentWorldName());

	CPerformanceMgr::Instance( ).OnEnterWorld( );

#ifndef _FINAL
	//override for debugging purposes
	float fVal = 0.0f;
	if (!g_pLTClient) return;

	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("LowViolence");
	if(hVar)
	{
		fVal = g_pLTClient->GetConsoleVariableFloat(hVar);
		char buf[128] = "";
		LTSNPrintF(buf, LTARRAYSIZE(buf), "serv LowViolence %f",fVal);
		g_pLTClient->RunConsoleCommand(buf);
	}
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnExitWorld()
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnExitWorld()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	//clear out our current regions so we don't hold onto them beyond the level unloading
	CGameStreamingMgr::Singleton().ClearCurrentRegions();

	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->StopAllSounds(true);

	if (GetConsoleBool("MakeAssetList",false))
	{
		WriteConsoleBool("ShowFileAccess",false);
		g_pLTClient->CPrint("EndList");
	}

	// Set our simulation timer scale to real time.
	SimulationTimer::Instance().SetTimerTimeScale( 1, 1 );
	
	// Not in world any more.
    SetWorldNotLoaded();

	g_pDamageFXMgr->Clear();					// Remove all the sfx
	m_sfxMgr.RemoveAll();					// Remove all the sfx
	m_SimulationTimeClientFXMgr.ShutdownAllFX();			// Stop all the client FX
	m_RealTimeClientFXMgr.ShutdownAllFX();			// Stop all the client FX
	FOREACH_RENDER_TARGET_MGR ShutdownAllFX();
	m_ShatterEffectMgr.FreeShatterEffects(); //Stop all shattering effects

	// Clear out any buffered sector ID states
	m_aDynamicSectorStateBuffer.swap(TDynamicSectorList());

	if( g_pInterfaceMgr )
		g_pInterfaceMgr->OnExitWorld();

	if( g_pPlayerMgr )
		g_pPlayerMgr->OnExitWorld();

	ClientVoteMgr::Instance().OnExitWorld();

	ClientPhysicsCollisionMgr::Instance().PreStartWorld();

	// Make sure we clear out the specialfx messages from the old level.
	SpecialFXNotifyMessageHandler::Instance().Init( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PreUpdate()
//
//	PURPOSE:	Handle client pre-updates
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PreUpdate()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	GetPlayerMgr()->PreUpdate();
	GetInterfaceMgr( )->PreUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::Update()
//
//	PURPOSE:	Handle client updates
//
// ----------------------------------------------------------------------- //

void CGameClientShell::Update()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	if( m_bFirstUpdate )
	{
		FirstUpdate( );
	}

	// Synchronize our server timer with the actual time on the server...
	if( m_fLastServerRealTime < g_pLTClient->GetServerRealTime( ))
	{
		// Restart the accumulator so the calculated server time is accurate...
		m_swtServerRealTimeAccumulator.Start( );
		m_fLastServerRealTime = g_pLTClient->GetServerRealTime( );
	}

	// Update the input (Happens first to simulate the old way OnCommandOn/Off got called.)
	CBindMgr::GetSingleton().Update();

	// Allow multiplayermgr to update.
	g_pClientConnectionMgr->Update();

	// Update tint if applicable (always do this to make sure tinting
	// gets finished)...

	m_ScreenTintMgr.Update();

	ClientPhysicsCollisionMgr::Instance().Update( );

	// Update the dynamic sector states so they correctly handle order of operation issues
	UpdateDynamicSectorStates();

	// Update the specialfx message handler to handle interdependent objects.
	SpecialFXNotifyMessageHandler::Instance( ).Update( );
	
	// Update the model decals
	m_pModelDecalMgr->Update();

	// Update client-side physics structs...

	if (IsServerPaused())
	{
		SetPhysicsStateTimeStep(&g_normalPhysicsState, 0.0f);
		SetPhysicsStateTimeStep(&g_waterPhysicsState, 0.0f);
		SetPhysicsStateTimeStep(&g_playerNormalPhysicsState, 0.0f);
		SetPhysicsStateTimeStep(&g_playerWaterPhysicsState, 0.0f);
	}
	else
	{
		float fFrameTime = SimulationTimer::Instance().GetTimerElapsedS( );
		SetPhysicsStateTimeStep(&g_normalPhysicsState, fFrameTime);
		SetPhysicsStateTimeStep(&g_waterPhysicsState, fFrameTime);

		ObjectContextTimer objectContextTimer( g_pPlayerMgr->GetMoveMgr()->GetServerObject( ));
		fFrameTime = objectContextTimer.GetTimerElapsedS( );
		SetPhysicsStateTimeStep(&g_playerNormalPhysicsState, fFrameTime);
		SetPhysicsStateTimeStep(&g_playerWaterPhysicsState, fFrameTime);
	}

	if ( m_pPunkBusterClient )
		m_pPunkBusterClient->ProcessEvents();

	// Update the interface (don't do anything if the interface mgr
	// handles the update...)

	if (GetInterfaceMgr( )->Update())
	{
		// In multiplayer we want to continue updating the player to 
		// account for the player being in the air when the menu was
		// brought up...
		if( IsMultiplayerGameClient() && GetPlayerMgr()->IsPlayerInWorld() )
		{
			GetPlayerMgr()->UpdateNotPlaying();
			DontRenderCamera();
		}

		RenderDisplayImage();
		return;
	}

	GetPlayerMgr()->Update();

	// At this point we only want to proceed if the player is in the world...
	if (GetPlayerMgr()->IsPlayerInWorld() && g_pInterfaceMgr->GetGameState() != GS_UNDEFINED)
	{
		UpdatePlaying();
	}
	
	RenderDisplayImage();

	//setup our performance logging system
	CPerformanceLogMgr::Update(RealTimeTimer::Instance().GetTimerElapsedS());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateGoreSettings()
//
//	PURPOSE:	Looks at the gore settings and makes sure all objects reflect the setting
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateGoreSettings()
{
	//default to no gore
	bool bGore = false;

	if(g_pProfileMgr && g_pProfileMgr->GetCurrentProfile() && g_pVersionMgr)
	{
 		if (g_pVersionMgr->IsLowViolence())
 		{
 			//force the profile to be low violence here (to prevent people from hacking their profile to get around the limits)
 			g_pProfileMgr->GetCurrentProfile()->m_bGore = false;
 		}
 		else
 		{
 			bGore = g_pProfileMgr->GetCurrentProfile()->m_bGore;
 		}
   	}
	//setup the FX managers to handle the effects
	GetSimulationTimeClientFXMgr().SetGoreEnabled(bGore);
	GetRealTimeClientFXMgr().SetGoreEnabled(bGore);
	GetInterfaceMgr()->GetInterfaceFXMgr().SetGoreEnabled(bGore);

	// Send message to server about new gore settings...
	if( g_pLTClient->IsConnected( ))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_GORE_SETTING );
		cMsg.Writebool( bGore );
		g_pLTClient->SendToServer( cMsg.Read( ), MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SendPerformanceSettingsToServer()
//
//	PURPOSE:	Update the server with specific performance settings...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SendPerformanceSettingsToServer( )
{
	// Currently performance settings are ignored in mp games.
	if( IsMultiplayerGameClient( ))
		return;

	// Send message to server about current world detail setting...
    if( g_pLTClient->IsConnected( ))
	{
		EEngineLOD eWorldDetail = static_cast<EEngineLOD>(GetConsoleInt( "WorldDetail", static_cast<int>(eEngineLOD_Low) ));

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PERFORMANCE_SETTING );
		cMsg.WriteBits( eWorldDetail, FNumBitsInclusive<eEngineLOD_NumLODTypes>::k_nValue );
		cMsg.Writeuint32(( uint32 )GetConsoleFloat( "BodyCapRadius", -1.0f ));
		cMsg.Writeuint8(( uint8 )GetConsoleFloat( "BodyCapRadiusCount", -1.0f ));
		cMsg.Writeuint8(( uint8 )GetConsoleFloat( "BodyCapTotalCount", -1.0f ));
		g_pLTClient->SendToServer( cMsg.Read( ), MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdatePlaying()
//
//	PURPOSE:	Handle updating playing (normal) game state
//
// ----------------------------------------------------------------------- //

uint32 g_nSamplePeriod = 250;
uint32 g_nTolerance = 400;

void CGameClientShell::UpdatePlaying()
{
	// Handle first update...

	if (m_bFirstPlayingUpdate)
	{
		FirstPlayingUpdate();
	}

// Speed hacking prevention only available on Win32
#if defined(PLATFORM_WIN32)
	if(IsMultiplayerGameClient() && g_pPlayerMgr->IsPlayerInWorld())
	{
		// Only check the speed hack for remote clients.
		bool bIsLocalClient = false;
		g_pLTClient->IsLocalToServer(&bIsLocalClient);
		if( !bIsLocalClient )
		{
			uint32 nEndClientTime = timeGetTime( );
			uint32 nEndTicks = GetTickCount();
			_timeb nEndTimeB;
			_ftime(&nEndTimeB);

			// Get the client time since the last check.  Account for wrap.
			uint32 nDeltaClientTime = ( nEndClientTime > g_nStartClientTime ) ? ( nEndClientTime - g_nStartClientTime ) : 
				( nEndClientTime + ~g_nStartClientTime );

			// Check at a periodic rate.
			if( nDeltaClientTime > g_nSamplePeriod )
			{
				// Make sure we've gone through this at least once to make sure all the counters
				// are properly initialized.
				if( g_nStartTicks > 0 )
				{
					// Get the time between _ftime's.
					uint32 nDeltaTimeB = (nEndTimeB.time - g_StartTimeB.time)*1000 + (nEndTimeB.millitm - g_StartTimeB.millitm);

					// Get the time between GetTickCount's.  Account for wrapping.
					uint32 nDeltaTicks = ( nEndTicks > g_nStartTicks ) ? ( nEndTicks - g_nStartTicks ) : 
						( nEndTicks + ~g_nStartTicks );

					// Make sure all the counters match up.
					if((( uint32 )abs( (int)nDeltaTimeB - (int)nDeltaClientTime ) > g_nTolerance ) || 
						(( uint32 )abs( (int)nDeltaTicks - (int)nDeltaClientTime ) > g_nTolerance ) ||
						(( uint32 )abs( (int)nDeltaTimeB - (int)nDeltaTicks ) > g_nTolerance ))
					{
						g_pLTClient->CPrint( "Speedhack kick" );
						g_pLTClient->CPrint( "nDeltaTimeB %d", nDeltaTimeB );
						g_pLTClient->CPrint( "nDeltaTicks %d", nDeltaTicks );
						g_pLTClient->CPrint( "nDeltaClientTime %d", nDeltaClientTime );
					
						// You hAxOr!

						// Disconnect from the server.
						if(g_pLTClient->IsConnected())
						{
							m_pClientMultiplayerMgr->DisconnectFromServer();
						}
					}
				}

				// Reset the timers, the initial _ftime call must be before
				// the performance counter call!
				_ftime( &g_StartTimeB );
				g_nStartTicks = GetTickCount();
				g_nStartClientTime = timeGetTime( );
			}
		}
	}
#endif // PLATFORM_WIN32

	g_pPlayerMgr->UpdatePlaying();

	// Update cheats...(if a cheat is in effect, just return)...
	if( UpdateCheats( ))
		return;

	// Render the camera...

	RenderCamera();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PostUpdate()
//
//	PURPOSE:	Handle post updates - after the scene is rendered
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PostUpdate()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// See if we got a oncommandon for quicksave.
	if( m_bQuickSave )
	{
		m_bQuickSave = false;
		if( !g_pClientSaveLoadMgr->QuickSave() )
		{
			DebugCPrint(1, "ERROR - Quick Save Failed!" );
		}
	}

	GetPlayerMgr()->PostUpdate();

	// If there are conditions where we don't want to do a screen flip, we should not call GetInterfaceMgr( )->PostUpdate()...

	GetInterfaceMgr( )->PostUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateCheats()
//
//	PURPOSE:	Update cheats...
//
// ----------------------------------------------------------------------- //

bool CGameClientShell::UpdateCheats()
{

#ifndef _FINAL
	// Don't allow cheats in mp.
	if( IsMultiplayerGameClient( ))
		return false;
#endif

	if (m_bAdjust1stPersonCamera)
	{
		Adjust1stPersonCamera();
	}

	// Handle switching in/out of screen shot mode...
#ifndef _FINAL
	bool bScreenShot = (g_vtScreenShotMode.GetFloat() > 0.0f ? true : false);
	if (bScreenShot != g_bScreenShotMode)
	{
		g_bScreenShotMode = !g_bScreenShotMode;

		bool bDrawHUD = !g_bScreenShotMode;
		if (g_pHUDMgr)
		{
			g_pHUDMgr->UpdateRenderLevel();
		}

		g_pLTClient->CPrint("Screen shot mode: %s", g_bScreenShotMode ? "ON" : "OFF");
	}
#endif

	// If in spectator mode, just do the camera stuff...
	if (g_pPlayerMgr->IsSpectating() || !g_pPlayerMgr->IsPlayerAlive())
	{
		g_pPlayerMgr->UpdateCamera();
		RenderCamera();
        return true;
	}

	// Update weapon position if appropriated...

	if (m_bTweakingWeapon)
	{
		UpdateWeaponPosition();
		g_pPlayerMgr->UpdateCamera();
		RenderCamera();
        return true;
	}
	
    return false;
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ClearScreenTint
//
//	PURPOSE:	Clear any screen tinting
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ClearScreenTint()
{
	m_ScreenTintMgr.ClearAll();

	m_LightScaleMgr.Term();
	m_LightScaleMgr.Init();
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	QuickLoadCallBack
//
//  PURPOSE:	Callback for the QuickLoad messagebox...
//
// ----------------------------------------------------------------------- //

static void QuickLoadCallBack(bool bReturn, void* /*pData*/, void* /*pUserData*/)
{
	if (bReturn)
	{
		g_pMissionMgr->StartGameFromContinue( );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnCommandOn()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnCommandOn(int command)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// Make sure we're in the world...

	if (!GetPlayerMgr()->IsPlayerInWorld()) return;


	// Let the interface handle the command first...

	if (IsMultiplayerGameClient() || GetPlayerMgr()->IsPlayerAlive())
	{
		if (GetInterfaceMgr( )->OnCommandOn(command))
		{
			return;
		}
	}


	if (GetPlayerMgr()->OnCommandOn(command))
	{
		return;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnCommandOff()
//
//	PURPOSE:	Handle command off notification
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnCommandOff(int command)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// Let the interface handle the command first...
	if (GetInterfaceMgr( )->OnCommandOff(command))
	{
		return;
	}
	if (GetPlayerMgr()->OnCommandOff(command))
	{
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnClearAllCommands()
//
//	PURPOSE:	Handle command clearing notification
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnClearAllCommands()
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	GetInterfaceMgr( )->OnClearAllCommands();
	GetPlayerMgr()->OnClearAllCommands();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnKeyDown(int key, int rep)
//
//	PURPOSE:	Handle key down notification
//				Try to avoid using OnKeyDown and OnKeyUp as they
//				are not portable functions
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnKeyDown(int key, int rep)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// [RP] - 8/03/02: WinXP likes to add a second OnKeyDown() message with an invalid key of 255
	//		  for certain single key presses.  Just ignore invalid key codes 255 and larger.

	if( key >= 0xFF )
		return;

	//if we are performance testing...
	if (IsRunningPerformanceTest())
	{
		//allow abort...
		if (key == VK_ESCAPE)
			AbortPerformanceTest();

		//but don't process anything else
		return;
	}


	if (key == VK_WRITE_CAM_POS)
	{
#ifndef _FINAL
		DebugWriteCameraPosition();
#endif
		return;
	}

	if (key == VK_MAKE_CUBIC_ENVMAP)
	{
#ifndef _FINAL
		LTVector vPos;
		vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();
		g_pLTClient->GetRenderer()->MakeCubicEnvMap(vPos, (uint32)(g_vtMakeCubicEnvMapSize.GetFloat(256.0f) + 0.5f), g_vtMakeCubicEnvMapName.GetStr("CubicEnvMap." RESEXT_TEXTURE));
#endif
		return;
	}

	if(key == VK_MAKE_SCREEN_SHOT)
	{
		//determine a filename for this. This should start with the screenshot console variable and be
		//numbered sequentially

		//an upper cap to make sure that we don't attempt to create screenshots indefinitely, which could
		//potentially be caused by some file access issues
		static const uint32 knMaxScreenshotAttempts = 1000;

		for(uint32 nCurrScreenshotIndex = 0; nCurrScreenshotIndex < knMaxScreenshotAttempts; nCurrScreenshotIndex++)
		{
			//build up the filename to use for this screenshot
			char pszFilename[MAX_PATH];
			LTSNPrintF(pszFilename, LTARRAYSIZE(pszFilename), "%s%03d.%s",  g_vtScreenShotPrefix.GetStr("Screenshot"), nCurrScreenshotIndex, g_vtScreenShotExtension.GetStr("bmp"));

			// get the user path to the screenshot file, as we need this to check if the file exists.
			char pszUserFilename[MAX_PATH];
			g_pLTClient->FileMgr()->GetAbsoluteUserFileName( pszFilename, pszUserFilename, LTARRAYSIZE( pszUserFilename ) );

			//see if this file already exists - if it does, continue searching for an unused file
			if ( !LTFileOperations::FileExists( pszUserFilename ) )
			{
				//the filename doesn't exist, so go ahead and try to make the screenshot
				if(g_pLTClient->GetRenderer()->MakeScreenShot(pszUserFilename) == LT_OK)
				{
					g_pLTClient->CPrint("Successfully created screenshot %s", pszUserFilename);
				}
				else
				{
					g_pLTClient->CPrint("Failed to create screenshot %s", pszUserFilename);
				}
				break;
			}
		}

		//report overflow
		if(nCurrScreenshotIndex >= knMaxScreenshotAttempts)
		{
			g_pLTClient->CPrint("Unable to create screenshot. Please make sure that the directory is readable and that there are less than %d screenshots", knMaxScreenshotAttempts);
		}

		return;
	}

	// Let the interface mgr have a go at it...
	if (GetInterfaceMgr( )->OnKeyDown(key, rep))
	{
		return;
	}
	if (GetPlayerMgr()->OnKeyDown(key, rep))
	{
		return;
	}

#endif // !PLATFORM_XENON

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnKeyUp(int key, int rep)
//
//	PURPOSE:	Handle key up notification
//
// ----------------------------------------------------------------------- //
void CGameClientShell::OnKeyUp(int key)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// The engine handles the VK_F8 key for screenshots.
	if( key == VK_F8 )
		return;

	if (IsRunningPerformanceTest())
		return;


	// Allow quickload from anywhere, anytime.
	// jrg - 8/31/02 - well, almost anywhere and almost anytime, except...
	GameState eGameState = g_pInterfaceMgr->GetGameState();
	if	( key == VK_F9 && 
		(	GS_PLAYING == eGameState ||
			GS_SCREEN == eGameState ||
			GS_MENU == eGameState
		)	
		)
	{
		//handle special case of postload screen
		if (GS_SCREEN == eGameState && SCREEN_ID_POSTLOAD == g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID() )
			return;

		if (g_pInterfaceMgr->IsMessageBoxVisible())
		{
			g_pInterfaceMgr->GetMessageBox()->HandleKeyDown(key,1);
			return;
		}

		// Check if we can do a load.
		if( g_pClientSaveLoadMgr->CanLoadGame( ) && g_pClientSaveLoadMgr->CanContinueGame( ))
		{
			// If player is in the world, then give them confirmation.
			if( g_pPlayerMgr->IsPlayerInWorld() && g_pPlayerMgr->IsPlayerAlive( ))
			{
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = QuickLoadCallBack;
				mb.pUserData = this;
				mb.nHotKey = VK_F9;
				g_pInterfaceMgr->ShowMessageBox("IDS_ENDCURRENTGAME",&mb);
			}
			else
			{
				g_pMissionMgr->StartGameFromContinue( );
			}
		}
		return;
	}
	else if( key == VK_F5 )
	{
		// Only allow quicksave if we're in a world and not a remote client.
		if( GetPlayerMgr()->IsPlayerInWorld() && !g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
		{
			if( g_pClientSaveLoadMgr->CanSaveGame() && eGameState == GS_PLAYING )
			{
				m_bQuickSave = true;
			}
			else
			{
				// Can't quicksave now...
				g_pGameMsgs->AddMessage(LoadString("IDS_CANTQUICKSAVE"));
				g_pClientSoundMgr->PlayInterfaceDBSound("CantSave");
			}
		}
		return;
	}

	//handle voting
	if (ClientVoteMgr::Instance().IsVoteInProgress())
	{
		if (key == VK_F1)
		{
			ClientVoteMgr::Instance().CastVote(true);
			return;
		}
		if (key == VK_F2)
		{
			ClientVoteMgr::Instance().CastVote(false);
			return;
		}
	}


	if (GetInterfaceMgr( )->OnKeyUp(key))
		return;
	if (GetPlayerMgr()->OnKeyUp(key)) 
		return;

#endif // !PLATFORM_XENON

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnMessage(ILTMessage_Read *pMsg)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// Check inputs.
	if( !pMsg )
		return;

	// Store the server's address as where this message came from
	uint8 aAddrBuffer[4];
	uint16 nPort;
	g_pLTClient->GetServerIPAddress(aAddrBuffer, &nPort);

	// Read the message ID
	uint8 messageID = pMsg->Readuint8();

	// Act like the message ID wasn't in the message
	CLTMsgRef_Read cSubMsg(pMsg->SubMsg(pMsg->Tell()));

	// Let interface handle message first...

	if (GetInterfaceMgr( )->OnMessage(messageID, cSubMsg)) return;
	if (GetPlayerMgr()->OnMessage(messageID, cSubMsg)) return;

	if( g_pClientConnectionMgr->OnMessage(messageID, cSubMsg ))
		return;

	if( g_pMissionMgr->OnMessage( messageID, *cSubMsg ))
		return;

	if( g_pClientSaveLoadMgr->OnMessage( messageID, *cSubMsg ))
		return;

	// See if the scmd can handle this message.
	if( ScmdConsole::Instance( ).OnMessage( messageID, *cSubMsg ))
		return;

	/***************************************************************************/

	switch(messageID)
	{
		case MID_SFX_MESSAGE:			HandleMsgSFXMessage			(cSubMsg);	break;
		case MID_SFX_MESSAGE_OVERRIDE:	HandleMsgSFXMessageOverride	(cSubMsg);	break;
		case MID_APPLY_DECAL:			HandleMsgApplyDecal			(cSubMsg);	break;
		case MID_PLAYER_LOADCLIENT:		HandleMsgPlayerLoadClient	(cSubMsg);	break;
		case MID_GAME_PAUSE:			HandleMsgPauseGame			(cSubMsg);	break;
		case MID_SWITCHINGWORLDSSTATE:	HandleMsgSwitchingWorldState(cSubMsg);	break;
		case MID_MULTIPLAYER_OPTIONS:	HandleMsgMultiplayerOptions (cSubMsg);	break;
		case MID_DYNAMIC_SECTOR:		HandleMsgDynamicSector		(cSubMsg);  break;
		case MID_DYNANIMPROP:			HandleMsgDynAnimProp		(cSubMsg);	break;
		case MID_SHATTER_WORLD_MODEL:	HandleMsgShatterWorldModel	(cSubMsg);	break;
		case MID_SIMULATION_TIMER:		HandleMsgSimulationTimerScale(cSubMsg);	break;
		case MID_MIXER:					HandleMsgMixer				(cSubMsg);	break;	
		case MID_SOUND_MISC:			HandleMsgSoundMisc			(cSubMsg);	break;
		case MID_SOUND_FILTER:			HandleMsgSoundFilter		(cSubMsg);	break;
		case MID_WEAPON_FIRE_FX:		HandleMsgWeaponFireFX		(cSubMsg);	break;
		case MID_WEAPON_DAMAGE_PLAYER:	HandleMsgWeaponDamagePlayer	(cSubMsg);	break;
		case MID_SOUND_BROADCAST_DB:	HandleMsgSoundBroadcastDB	(cSubMsg);	break;
		case MID_PHYSICSCOLLISION:		ClientPhysicsCollisionMgr::Instance( ).HandlePhysicsMessage(cSubMsg);	break;	
		case MID_INSTANTNAVMARKER:		HandleInstantNavMarker(cSubMsg);	break;
		case MID_DO_DAMAGE:				HandleMsgDoDamage			(cSubMsg);	break;
		case MID_PLAYER_LADDER:			HandleMsgLadder				(cSubMsg);	break;
		case MID_PUNKBUSTER_MSG:		HandleMsgPunkBuster			(cSubMsg);  break;
		case MID_VOTE:			ClientVoteMgr::Instance().HandleMsgVote(cSubMsg); break;


		default: 					break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgSFXMessage()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgSFXMessage (ILTMessage_Read *pMsg)
{
	m_sfxMgr.OnSFXMessage(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgSFXMessageOverride()
//
//	PURPOSE:	Handle the sfx message override, which replaces
//			the old one that is now in a queue.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgSFXMessageOverride(ILTMessage_Read *pMsg)
{
	m_sfxMgr.OnSFXMessageOverride(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgApplyDecal()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgApplyDecal(ILTMessage_Read *pMsg)
{
	HOBJECT hModel = pMsg->ReadObject();
	HMODELNODE hNode = pMsg->Readuint32();
	HRECORD hDecalType = pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pModelsDB->GetModelDecalSetCategory());
	LTVector vPos = pMsg->ReadLTVector();
	LTVector vDir = pMsg->ReadLTVector();
	uint32 nDecalType = g_pModelDecalMgr->GetDecalType(hDecalType);
	g_pModelDecalMgr->AddDecal(hModel, hNode, nDecalType, vPos, vDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgPlayerLoadClient()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgPlayerLoadClient (ILTMessage_Read *pMsg)
{
	g_pVersionMgr->SetCurrentSaveVersion( pMsg->Readuint32( ));
	g_pVersionMgr->SetLGFlags(pMsg->Readuint8( ));
	CLTMsgRef_Read pSaveMessage = pMsg->ReadMessage();
	UnpackClientSaveMsg(pSaveMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgPauseGame()
//
//	PURPOSE:	Handle server telling us the game is paused.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgPauseGame( ILTMessage_Read* pMsg )
{
	bool bServerPaused = pMsg->Readbool( );
	SimulationTimer::Instance( ).PauseTimer( bServerPaused );
	GameTimeTimer::Instance( ).PauseTimer( bServerPaused );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgSwitchingWorldState()
//
//	PURPOSE:	Handle server telling us the switching world state.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgSwitchingWorldState( ILTMessage_Read* pMsg )
{
	m_eSwitchingWorldsState = ( SwitchingWorldsState )pMsg->Readuint8( );
	if (m_eSwitchingWorldsState == eSwitchingWorldsStateFinished)
	{
		if (GetConsoleBool("MakeAssetList",false))
		{
			WriteConsoleBool("ShowFileAccess",true);
			g_pLTClient->CPrint("StartList %s", g_pMissionMgr->GetCurrentWorldName());
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgMultiplayerOptions()
//
//	PURPOSE:	Handle server telling us the game options.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgMultiplayerOptions( ILTMessage_Read* pMsg )
{
	GameModeMgr::Instance().ReadFromMsg( *pMsg );

	// If we're the host, then save this
	bool bIsHost = false;
	if( g_pLTClient->IsLocalToServer( &bIsHost ) == LT_OK && bIsHost )
	{
		GameModeMgr::Instance().WriteToOptionsFile( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgShatterWorldModel()
//
//	PURPOSE:	Handle when the server has told us we need to create a
//				shattering effect for a world model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgShatterWorldModel( ILTMessage_Read* pMsg )
{
	//read in the blind object data
	uint32 nBlindObjectData = pMsg->Readuint32();
	HRECORD hShatterType = pMsg->ReadDatabaseRecord(g_pLTDatabase, CShatterTypeDB::Instance().GetShatterCategory());

	//read in the transform of the shattering world model
	LTRigidTransform tObjTrans;
	tObjTrans.m_vPos = pMsg->ReadLTVector();
	tObjTrans.m_rRot = pMsg->ReadLTRotation();

	//and now read in the information about the point of impact
	LTVector vImpactPt  = pMsg->ReadLTVector();
	LTVector vImpactDir = pMsg->ReadLTVector();

	//we now need to use this information to create a shattered effect
	m_ShatterEffectMgr.CreateShatterEffect(nBlindObjectData, tObjTrans, vImpactPt, vImpactDir, hShatterType);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgDynAnimProp()
//
//	PURPOSE:	Handle server telling us about a new dynamic animation propery
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgDynAnimProp( ILTMessage_Read* pMsg )
{
	char pszPropName[256];
	pMsg->ReadString(pszPropName,LTARRAYSIZE(pszPropName));
	EnumAnimProp eNewProp = (EnumAnimProp)pMsg->Readuint32();
	int iOffset = pMsg->Readuint32();
	AnimPropUtils::Sync(pszPropName,eNewProp,iOffset);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgDynamicSector()
//
//	PURPOSE:	Handle server telling us about a change to a visibility sector
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgDynamicSector( ILTMessage_Read* pMsg )
{
	// Read the sector ID
	uint32 nSectorID = pMsg->Readuint32();
	// Enable/Disable
	bool bEnable = pMsg->Readbool();

	// Tell the renderer
	LTRESULT nResult = g_pLTClient->SetSectorVisibility(nSectorID, bEnable);

	// Add it to the dynamic sector buffer for later use if we don't know about that sector yet
	if (nResult != LT_OK)
	{
		// Find out if it's already in the list
		TDynamicSectorList::iterator iSearchValue;
		iSearchValue = std::find(m_aDynamicSectorStateBuffer.begin(), m_aDynamicSectorStateBuffer.end(), TDynamicSectorList::value_type(nSectorID, bEnable));
		if (iSearchValue == m_aDynamicSectorStateBuffer.end())
		{
			// Find out if it's already in the list with the opposite value
			iSearchValue = std::find(m_aDynamicSectorStateBuffer.begin(), m_aDynamicSectorStateBuffer.end(), TDynamicSectorList::value_type(nSectorID, !bEnable));
			if (iSearchValue == m_aDynamicSectorStateBuffer.end())
			{
				// It wasn't found, so add it to the list
				m_aDynamicSectorStateBuffer.push_back(TDynamicSectorList::value_type(nSectorID, bEnable));
			}
			else
			{
				// It was found, so toggle the value
				iSearchValue->second = bEnable;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgMixer()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgMixer (ILTMessage_Read *pMsg)
{
	if (m_Mixer.IsInitialized())
	{
		m_Mixer.ProcessMixerMessage(pMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgSoundMisc()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgSoundMisc (ILTMessage_Read *pMsg)
{
	// get the sub-message

	uint8 nMiscMessage = pMsg->Readuint8();

	switch (nMiscMessage)
	{
	case (MID_SOUND_MISC_KILL_EARRING_EFFECT):
		GetPlayerMgr()->ClearEarringEffect();
		break;
	default:
		break;
	}
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgSoundBroadcastDB()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgSoundBroadcastDB (ILTMessage_Read *pMsg)
{
	HRECORD hSound;
	HOBJECT hObj;

	hSound = pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pSoundDB->GetSoundCategory());
	hObj = pMsg->ReadObject();

	if (hObj)
	{
		g_pClientSoundMgr->PlayDBSoundFromObject( hObj, hSound, SMGR_INVALID_RADIUS,
			SOUNDPRIORITY_PLAYER_MEDIUM, NULL, 
			SMGR_DEFAULT_VOLUME, 1.0f, SMGR_INVALID_RADIUS);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleInstantNavMarker()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleInstantNavMarker(ILTMessage_Read *pMsg)
{
	CAutoMessage cMsg;

	cMsg.Writeuint8( SFX_NAVMARKER_ID );
	NAVMARKERCREATESTRUCT navMarker;
	navMarker.m_bInstant = true;
	navMarker.Read( pMsg );
	navMarker.Write( cMsg );
	m_sfxMgr.HandleSFXMsg(NULL, cMsg.Read());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgSoundFilter()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgSoundFilter (ILTMessage_Read *pMsg)
{
	// get the data from the message
	char szMsg[MAX_MIXER_MSG];
	pMsg->ReadString(szMsg,LTARRAYSIZE(szMsg));

	ConParse parse;
	parse.Init(szMsg);

	while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 1)
		{
			if (parse.m_Args[1] != NULL)
			{
				if (LTStrICmp(parse.m_Args[1], "OFF") == 0)
				{
					GetPlayerMgr()->OverrideSoundFilter(NULL, false);
				}
				else
				{
					GetPlayerMgr()->OverrideSoundFilter(parse.m_Args[1], true);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgWeaponFireFX()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgWeaponFireFX( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	WCREATESTRUCT w;

	bool bShooterIDSent = pMsg->Readbool( );
	if( bShooterIDSent )
	{
		w.nShooterId    = pMsg->Readuint8( );

		// Search through the CharacterFX for the player with this ID...
		CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
		while( iter != CCharacterFX::GetCharFXList( ).end( )) 
		{
			if( (*iter)->m_cs.nClientID == w.nShooterId )
			{
				w.hFiredFrom = (*iter)->GetServerObj( );
			}

			++iter;
		}
	}
	else
	{
		w.hFiredFrom    = pMsg->ReadObject( );
	}

	w.hWeapon = NULL;
	w.hAmmo = NULL;

	// If the weapon record was written in the message it needs to be retrieved.
	// Otherwise the weapon can be obtained from the character that fired the weapon...
	bool bReadWeaponRecord = pMsg->Readbool( );
	if( bReadWeaponRecord )
	{
		w.hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	}
	else
	{
		// Get the current weapon of the firing character...
		CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
		while( iter != CCharacterFX::GetCharFXList( ).end( )) 
		{
			if( (*iter)->GetServerObj( ) == w.hFiredFrom )
			{
				w.hWeapon = (*iter)->m_cs.hCurWeaponRecord;
			}

			++iter;
		}
	}

	if( !w.hWeapon )
	{
		LTERROR( "Invalid weapon record, can't initialize WeaponFX!" );
		return;
	}

	// If the ammo record was written in the message it needs to be retrieved.
	// Otherwise the ammo used will be the weapons default...
	bool bReadAmmoRecord = pMsg->Readbool( );
	bool bIsAI = false;
	if( bReadAmmoRecord )
	{
		w.hAmmo	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
	}
	else
	{
		bIsAI = pMsg->Readbool();
		HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData(w.hWeapon, bIsAI);
		w.hAmmo = g_pWeaponDB->GetRecordLink(hWeaponData, WDB_WEAPON_rAmmoName);
	}

	if( !w.hAmmo )
	{
		LTERROR( "Invalid ammo record, can't initialize WeaponFX!" );
		return;
	}

	w.wIgnoreFX			= pMsg->Readuint16();

	w.bLeftHandWeapon	= pMsg->Readbool();

	uint32 nNumImpactPoints = pMsg->Readuint8( );
	if( nNumImpactPoints )
	{
		// Get the initial fire position...
		LTVector vFiredFrom( 0.0f, 0.0f, 0.0f );
		if( w.hFiredFrom )
		{
			LTRotation rRot;
			if( !GetAttachmentSocketTransform( w.hFiredFrom, (w.bLeftHandWeapon ? "Left_Flash" : "Flash"), vFiredFrom, rRot ) )
				GetAttachmentSocketTransform( w.hFiredFrom, (w.bLeftHandWeapon ? "LeftHand" : "RightHand"), vFiredFrom, rRot );
		}

		for( uint8 nPoint = 0; nPoint < nNumImpactPoints; ++nPoint )
		{
			LTVector vImpactPos = pMsg->ReadCompLTVector( );

			LTVector vDir = (vImpactPos - vFiredFrom);
			float fMag = vDir.Mag( );
			// Check if this is not a valid point.
			if( fMag < 0.0001f )
			{
				continue;	
			}

			vDir /= fMag;

			SurfaceType eSurfType = ST_UNKNOWN;

			IntersectInfo iTestInfo;
			IntersectQuery qTestInfo;

			qTestInfo.m_From = vImpactPos + (-vDir * 20.0f);
			qTestInfo.m_To   = vImpactPos + (vDir * 20.0f);

			qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

			if( g_pLTBase->IntersectSegment( qTestInfo, &iTestInfo ))
			{
				if( CWeaponPath::IsCharacterHitBox( iTestInfo.m_hObject ))
				{
					iTestInfo.m_hObject = CWeaponPath::GetCharacterFromHitBox( iTestInfo.m_hObject );
				}

				CCharacterFX *pCharFX = GetSFXMgr( )->GetCharacterFX( iTestInfo.m_hObject );
				if( pCharFX )
				{
					ModelsDB::HNODE hSkeletonNode = g_pModelsDB->GetSkeletonNodeAlongPath( iTestInfo.m_hObject,
														pCharFX->GetModelSkeleton( ), qTestInfo.m_From, vDir );
					
					// Retrieve the actual model node hit so WeaponFX can apply model decals...
					if( hSkeletonNode )
					{
						const char *pszNodeName = g_pModelsDB->GetNodeName( hSkeletonNode );
						if( !LTStrEmpty( pszNodeName ))
						{
							g_pModelLT->GetNode( iTestInfo.m_hObject, pszNodeName, w.hNodeHit );
						}
					}
				}

				eSurfType = GetSurfaceType( iTestInfo );

				w.hObjectHit		= iTestInfo.m_hObject;
				w.nSurfaceType		= eSurfType;
				w.vFirePos			= vFiredFrom;
				w.vPos				= iTestInfo.m_Point;
				w.vSurfaceNormal	= iTestInfo.m_Plane.m_Normal;
				w.bFXAtFlashSocket	= (nPoint == 0);
			}
			else
			{
				// No impact was found locally so just use the one from the server...
				w.vPos = vImpactPos;
			}

			

 			g_pGameClientShell->GetSFXMgr( )->CreateSFX( SFX_WEAPON_ID, &w );

			// If we are shooting multiple vectors ignore some special
			// fx after the first vector...
			w.wIgnoreFX |= WFX_FIRESOUND | WFX_ALTFIRESND | WFX_SHELL |
				WFX_SILENCED | WFX_MUZZLE | WFX_TRACER;

			w.hNodeHit = INVALID_MODEL_NODE;

			HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( w.hWeapon, bIsAI );
			if( g_pWeaponDB->GetInt32( hWeaponData, WDB_WEAPON_nVectorsPerRound ) <= 1 )
			{
				// Set the new from position to the old impact pos in order to properly carry the weapon path on...
				vFiredFrom = vImpactPos;
			}
		}
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgWeaponDamagePlayer()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgWeaponDamagePlayer( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	CClientInfoMgr *pCIMgr = g_pInterfaceMgr->GetClientInfoMgr( );
	if( !pCIMgr )
		return;
	
	uint8 nDamagedId = pMsg->Readuint8( );

	CLIENT_INFO* pCI = pCIMgr->GetClientByID( nDamagedId );
	if( pCI && !pCIMgr->IsLocalTeam( pCI->nTeamID ))
	{
		CCharacterFX* pCharFX = g_pGameClientShell->GetLocalCharacterFX();
		if( pCharFX )
		{
			pCharFX->PlayDingSound( );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgDoDamage()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgDoDamage( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	char szTargetName[256];
	pMsg->ReadString(szTargetName, ARRAY_LEN(szTargetName));

	const uint8 nNumObjects = pMsg->Readuint8();
	const float fAmount = pMsg->Readfloat();

	if( nNumObjects == 0 )
	{
		g_pLTClient->CPrint("DoDamage target \"%s\" could not be found.", szTargetName);
	}
	else if( nNumObjects == 1 )
	{
		g_pLTClient->CPrint("%.1f points of explosion damage done to \"%s\".", fAmount, szTargetName);
	}
	else
	{
		g_pLTClient->CPrint("%.1f points of explosion damage done to %d objects named \"%s\".", fAmount, nNumObjects, szTargetName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgLadder()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgLadder( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	HOBJECT hLadder = pMsg->ReadObject( );
	if( hLadder != INVALID_HOBJECT )
	{
		CLadderFX *pLadderFX = dynamic_cast< CLadderFX * >( GetSFXMgr( )->FindSpecialFX( SFX_LADDER_ID, hLadder ) );
		if( pLadderFX )
			LadderMgr::Instance( ).ActivateLadder( pLadderFX );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgPunkBuster()
//
//	PURPOSE:	Handle a PunkBuster message from the server.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgPunkBuster(ILTMessage_Read* pMsg)
{
	if (!pMsg)
	{
		return;
	}

	// get the server's address and port
	uint8  aServerAddress[4] = { 0 };
	uint16 nPort = 0;
	g_pLTClient->GetServerIPAddress(aServerAddress, &nPort);

	// extract the message data
	uint32 nDataLen = pMsg->Size() / 8;
	uint8* pData = ( uint8* )alloca( nDataLen );
	pMsg->ReadData( pData, nDataLen * 8 );

	// send it to PunkBuster
	if ( m_pPunkBusterClient )
		m_pPunkBusterClient->HandleNetMessage(pData, nDataLen, aServerAddress, nPort);
}

/***************************************************************************/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PauseGame()
//
//	PURPOSE:	Pauses/Unpauses the server
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PauseGame(bool bPause, bool bPauseSound)
{
	m_bGamePaused = bPause;

	g_pPaused->Show(!!bPause);

	if (!IsMultiplayerGameClient())
	{
		SendEmptyServerMsg(bPause ? MID_GAME_PAUSE : MID_GAME_UNPAUSE);
	}

	if (bPause && bPauseSound)
	{
        ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->PauseSounds();
	}
	else
	{
        ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->ResumeSounds();
	}

	SetInputState(!bPause && GetPlayerMgr()->IsPlayerMovementAllowed());
	GetPlayerMgr()->SetMouseInput(!bPause, true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetInputState()
//
//	PURPOSE:	Allows/disallows input
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SetInputState(bool bAllowInput)
{
	CBindMgr::GetSingleton().SetEnabled(bAllowInput);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateWeaponPosition()
//
//	PURPOSE:	Update the position of the current weapon
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateWeaponPosition()
{
	float fIncValue = WEAPON_MOVE_INC_VALUE_SLOW;
	bool bChanged = false;

	LTVector vOffset;
	vOffset.Init();

	uint32 dwPlayerFlags = GetPlayerMgr()->GetPlayerFlags();

	// Move weapon faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = WEAPON_MOVE_INC_VALUE_FAST;
	}

	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		vOffset = pClientWeapon->GetWeaponOffset();
	}

	// Move weapon forward or backwards...

	if ((dwPlayerFlags & BC_CFLG_FORWARD) || (dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vOffset.z += fIncValue;
		bChanged = true;
	}


	// Move the weapon to the player's right or left...

	if ((dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) ||
		(dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vOffset.x += fIncValue;
		bChanged = true;
	}


	// Move the weapon up or down relative to the player...

	if ((dwPlayerFlags & BC_CFLG_JUMP) || (dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		vOffset.y += fIncValue;
		bChanged = true;
	}


	// Okay, set the offset...

	if (bChanged)
	{
		if ( pClientWeapon )
		{
			pClientWeapon->SetWeaponOffset( vOffset );
			g_pLTClient->CPrint("Weapon offset = %f, %f, %f", vOffset.x, vOffset.y, vOffset.z);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::Adjust1stPersonCamera()
//
//	PURPOSE:	Update the 1st-person camera offset
//
// ----------------------------------------------------------------------- //

void CGameClientShell::Adjust1stPersonCamera()
{
    float fIncValue = 0.1f;
    bool bChanged = false;

	uint32 dwPlayerFlags = GetPlayerMgr()->GetPlayerFlags();

	LTVector vHeadOffset = g_pPlayerMgr->GetPlayerCamera()->GetFirstPersonOffset();

	// Move offset faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = fIncValue * 4.0f;
	}

	// Move 1st person offset.y up or down...

	if ((dwPlayerFlags & BC_CFLG_FORWARD) || (dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vHeadOffset.y += fIncValue;
        bChanged = true;
	}

	// Move 1st person offset.x in / out...

	if ((dwPlayerFlags & BC_CFLG_RIGHT) || (dwPlayerFlags & BC_CFLG_LEFT))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_RIGHT ? fIncValue : -fIncValue;
		vHeadOffset.z += fIncValue;
        bChanged = true;
	}

	if (bChanged)
	{
		// Okay, set the offset...

		g_pPlayerMgr->GetPlayerCamera()->SetFirstPersonOffset( vHeadOffset );

		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(g_pLTClient->GetClientObject(), &vDims);
		float fEyePos = vDims.y + vHeadOffset.y;

		g_pLTClient->CPrint("1st-Person Eye Position:           %.2f", fEyePos);
		g_pLTClient->CPrint("1st-Person Eye Offset:	            %.2f", vHeadOffset.y);
		g_pLTClient->CPrint("1st-Person Eye Offset (forward):   %.2f", vHeadOffset.x);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SpecialEffectNotify()
//
//	PURPOSE:	Handle creation of a special fx
//
//				This function is called by the engine directly. 
//				When an effect takes place the server notifies all clients
//				who are able to witness the effect.  NOT ALL EFFECTS will
//				necesarily be recieves through this routine.  If the server
//				needs to gaurantee that all clients get a particular effect
//				it will use the MID_SFX_MESSAGE identifier which will be
//				recieved in the OnMessage routine and the OnSFXMessage 
//				method will be envoked to handle the effect instead. 
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SpecialEffectNotify(HLOCALOBJ hObj, ILTMessage_Read *pMsg)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	if (hObj)
	{
        g_pCommonLT->SetObjectFlags(hObj, OFT_Client, CF_NOTIFYREMOVE, CF_NOTIFYREMOVE);
	}
	
	m_sfxMgr.HandleSFXMsg(hObj, pMsg);

	// Reset the message and send to the other FX mgr
	pMsg->SeekTo(0);
	m_SimulationTimeClientFXMgr.OnSpecialEffectNotify( hObj, pMsg );
	m_RealTimeClientFXMgr.OnSpecialEffectNotify( hObj, pMsg );
	FOREACH_RENDER_TARGET_MGR OnSpecialEffectNotify( hObj, pMsg );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnObjectRemove()
//
//	PURPOSE:	Handle removal of a server created object...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnObjectRemove(HLOCALOBJ hObj)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	if (!hObj) return;

	m_sfxMgr.RemoveSpecialFX(hObj);

	GetInterfaceMgr()->OnObjectRemove(hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnObjectAdd()
//
//	PURPOSE:	Handle an addition of a new object to the client world...
//				This is called for both server and client created objects.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnObjectAdd( HLOCALOBJ hObject )
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	if( !hObject )
		return;

	uint32 dwUserFlags = 0;
	g_pCommonLT->GetObjectFlags( hObject, OFT_User, dwUserFlags );

	// Set the object's rigidbodies to be simulated in the client physics simulation...
	if( dwUserFlags & USRFLG_CLIENT_RIGIDBODY_ONLY )
	{
		ILTPhysicsSim *pLTPhysicsSim = g_pLTClient->PhysicsSim( );
		if( !pLTPhysicsSim )
			return;

		uint32 dwType = 0;
		g_pCommonLT->GetObjectType( hObject, &dwType );

		switch( dwType )
		{
			case OT_WORLDMODEL:
			{
				HPHYSICSRIGIDBODY hRigidBody = INVALID_PHYSICS_RIGID_BODY;
				if( pLTPhysicsSim->GetWorldModelRigidBody( hObject, hRigidBody ) == LT_OK )
				{
					pLTPhysicsSim->SetRigidBodySolid( hRigidBody, true );
					pLTPhysicsSim->SetRigidBodyCollisionGroup( hRigidBody, PhysicsUtilities::ePhysicsGroup_UserMultiplayer );
					pLTPhysicsSim->ReleaseRigidBody( hRigidBody );
				}
			}
			break;

			case OT_MODEL:
			{
				// Set all of the models rigidbodies to solid...
				uint32 nNumBodies = 0;
				if( pLTPhysicsSim->GetNumModelRigidBodies( hObject, nNumBodies ) == LT_OK)
				{
					for( uint32 nCurrBody = 0; nCurrBody < nNumBodies; ++nCurrBody )
					{
						HPHYSICSRIGIDBODY hRigidBody = INVALID_PHYSICS_RIGID_BODY;
						if( pLTPhysicsSim->GetModelRigidBody( hObject, nCurrBody, hRigidBody ) == LT_OK )
						{
							pLTPhysicsSim->SetRigidBodySolid( hRigidBody, true );
							pLTPhysicsSim->SetRigidBodyCollisionGroup( hRigidBody, PhysicsUtilities::ePhysicsGroup_UserMultiplayer );
							pLTPhysicsSim->ReleaseRigidBody( hRigidBody );
						}
					}
				}
			}
			break;

			default:
			break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PreLoadWorld()
//
//	PURPOSE:	Called before world loads
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PreLoadWorld(const char *pWorldName)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// Pause the physics simulation so objects aren't moving as the player is entering the world...
	g_pLTClient->RunConsoleCommand( "PausePhysics 1" );
	
	if (IsMainWindowMinimized())
	{
		RestoreMainWindow();
	}

	g_pMissionMgr->PreLoadWorld( pWorldName );

	// Make sure the specialfx handler is initialized.
	SpecialFXNotifyMessageHandler::Instance( ).Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StartPerformanceTest()
//
//	PURPOSE:	Start the perfomance test
//
// ----------------------------------------------------------------------- //

void CGameClientShell::StartPerformanceTest()
{
	if (m_pPerformanceTest)
	{
		m_bRunningPerfTest = true;

		m_pPerformanceTest->Start((uint32)g_vtPTestMinFPS.GetFloat(), 
			(uint32)g_vtPTestMaxFPS.GetFloat());

		// Seed Random number generator...
		SRand();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StopPerformanceTest()
//
//	PURPOSE:	Stop the perfomance test
//
// ----------------------------------------------------------------------- //

void CGameClientShell::StopPerformanceTest()
{
	m_bRunningPerfTest = false;

	if (m_pPerformanceTest)
	{
		m_pPerformanceTest->Stop();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::AbortPerformanceTest()
//
//	PURPOSE:	Cancel out of the perfomance test
//
// ----------------------------------------------------------------------- //

void CGameClientShell::AbortPerformanceTest()
{
	StopPerformanceTest();

	// Make sure we're disconnected from server.
	if(g_pLTClient->IsConnected())
	{
		g_pInterfaceMgr->SetIntentionalDisconnect( true );
		g_pClientConnectionMgr->ForceDisconnect();
	}

	//since our history was cleared by loading the level... rebuild it
	g_pInterfaceMgr->GetScreenMgr()->AddScreenToHistory( SCREEN_ID_MAIN );
	g_pInterfaceMgr->GetScreenMgr()->AddScreenToHistory( SCREEN_ID_OPTIONS );

	if ( GetInterfaceMgr( )->GetGameState() == GS_LOADINGLEVEL)
	{
		GetInterfaceMgr( )->LoadFailed(SCREEN_ID_PERFORMANCE, LoadString("IDS_PERFORMANCE_TEST_ABORTED"));
	}
	else
	{

		MBCreate mb;
		GetInterfaceMgr( )->ShowMessageBox("IDS_PERFORMANCE_TEST_ABORTED", &mb);

	//we aborted performance testing go back to performance screen
	g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PERFORMANCE);
}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ToggleDebugCheat
//
//	PURPOSE:	Handle debug cheat toggles
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ToggleDebugCheat(CheatCode eCheat)
{
	switch (eCheat)
	{
		case CHEAT_POSWEAPON :
		{
			if (!g_pPlayerMgr->IsSpectating())
			{
				m_bTweakingWeapon = !m_bTweakingWeapon;

				g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bTweakingWeapon);
			}
		}
		break;

		case CHEAT_POS1STCAM :
		{
			if (!g_pPlayerMgr->IsSpectating())
			{
				m_bAdjust1stPersonCamera = !m_bAdjust1stPersonCamera;

				g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bAdjust1stPersonCamera);
			}
		}
		break;

		case CHEAT_CHASETOGGLE :
		{
			if (g_pPlayerMgr->GetPlayerState() == ePlayerState_Alive && !g_pPlayerMgr->GetPlayerCamera()->IsZoomed())
			{
				if( g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Chase )
				{
					g_pPlayerMgr->GetPlayerCamera()->SetCameraMode( CPlayerCamera::kCM_FirstPerson );
				}
				else
				{
					g_pPlayerMgr->GetPlayerCamera()->SetCameraMode( CPlayerCamera::kCM_Chase );
				}
			}
		}
		break;


		default : break;
	}
}

void CGameClientShell::DebugWriteCameraPosition()
{
	static char		s_szDebugCamName[64] = "";
	static uint32	s_nDebugCamCount = 0;

	char szTmp[128] = "";
	LTStrCpy( szTmp, GetConsoleTempString("DebugCameraName", "CameraPoint" ), LTARRAYSIZE( szTmp ));
	if (LTStrICmp(s_szDebugCamName,szTmp) != 0)
	{
		LTStrCpy(s_szDebugCamName, szTmp, LTARRAYSIZE(s_szDebugCamName));
		s_nDebugCamCount = 0;
	}
	char szFileName[64];
	sprintf(szFileName,"Game\\%s%02d.txt", s_szDebugCamName, s_nDebugCamCount);

	FILE* pFile = fopen (szFileName, "wt");
	if (!pFile) return;

	LTVector vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

	//handle the shift from the current world to the source world
	if((uint32)g_vtApplyWorldOffset.GetFloat(1.0f))
	{
		LTVector vOffset;
		g_pLTClient->GetSourceWorldOffset(vOffset);
		vPos += vOffset;
	}

	// Convert pitch and yaw to user friendly units...
	// Convert pitch and yaw to the same units used by WorldEdit...
	EulerAngles EA = Eul_FromQuat( g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( ), EulOrdYXZr );
	float fYaw		= EA.x;
	float fPitch	= EA.y;
	float fRoll		= EA.z;

	float fYawDeg = fmodf( RAD2DEG(fYaw), 360.0f );
	float fPitchDeg = fmodf( RAD2DEG(fPitch), 360.0f );
	float fRollDeg = fmodf( RAD2DEG(fRoll), 360.0f );

	LTSNPrintF(szTmp, LTARRAYSIZE(szTmp),"\n[CameraPoint___%s%02d]\n", s_szDebugCamName, s_nDebugCamCount);
	fwrite (szTmp, LTStrLen(szTmp), 1, pFile);

	LTSNPrintF(szTmp, LTARRAYSIZE(szTmp),"Pos      = <%6.0f,%6.0f,%6.0f>\n", vPos.x, vPos.y, vPos.z);
	fwrite (szTmp, LTStrLen(szTmp), 1, pFile);

	LTSNPrintF(szTmp, LTARRAYSIZE(szTmp),"Rotation = <%0.5f, %0.5f, 0.00000>\n", fPitch, fYaw);
	fwrite (szTmp, LTStrLen(szTmp), 1, pFile);

	fwrite ("\n", 1, 1, pFile);

	fclose (pFile);

	g_pLTClient->CPrint("%s%02d:Pos=<%6.0f,%6.0f,%6.0f>;Rot=<%6.0f,%6.0f,     0>", 
						s_szDebugCamName, s_nDebugCamCount, vPos.x, vPos.y, vPos.z, fPitchDeg, fYawDeg);

	//update our camera count
	s_nDebugCamCount++;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::FirstUpdate
//
//	PURPOSE:	Handle first CGameClientShell::Update ever called.
//
// --------------------------------------------------------------------------- //

void CGameClientShell::FirstUpdate()
{
	m_bFirstUpdate = false;

	// Process command line.
	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("join");
	if( hVar )
	{
		// Check if we're running multiplayer.
		if( IsMultiplayerGameClient( ))
		{
			g_pInterfaceMgr->SetCommandLineJoin( true );

			// Change to the splash screen.  We'll start the game after that.
			GetInterfaceMgr()->ChangeState( GS_SPLASHSCREEN );
		}
		// Not running multiplayer, switch to the mp exe.
		else
		{
			LaunchApplication::LaunchMultiPlayerExe( LaunchApplication::kSwitchToScreen_None );
		}
	}

	// Check if they specified a menu to switch to.
	hVar = g_pLTClient->GetConsoleVariable("screen");
	if( hVar )
	{
		// Look through the list of avaiable menus that can be switched to.
		const char* pszMenu = g_pLTClient->GetConsoleVariableString( hVar );
		if( LTStrEquals( pszMenu, "single" ))
		{
			GetInterfaceMgr( )->SwitchToScreen(SCREEN_ID_SINGLE);
		}
		else if( LTStrEquals( pszMenu, "multi" ))
		{
			GetInterfaceMgr( )->SwitchToScreen(SCREEN_ID_MULTI);
		}
		else if( LTStrEquals( pszMenu, "performance" ))
		{
			GetInterfaceMgr( )->SwitchToScreen(SCREEN_ID_PERFORMANCE);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::FirstPlayingUpdate
//
//	PURPOSE:	Handle first update (each level)
//
// --------------------------------------------------------------------------- //

void CGameClientShell::FirstPlayingUpdate()
{
	GetPlayerMgr()->FirstUpdate();

    m_bFirstPlayingUpdate = false;


	// Set up the level-start screen fade...

	if (g_varStartLevelScreenFade.GetFloat() && !GetInterfaceMgr()->OverrideInitialFade())
	{
		GetInterfaceMgr( )->StartScreenFadeIn(g_varStartLevelScreenFadeTime.GetFloat());
	}

	// Start fading in the sound (sound was silenced by the loading screen) -- Terry
	g_pGameClientShell->GetMixer( )->ProcessMixerByName( "Loading", -1, true );

	// Unpause the physics now that the player is actually in the game...
	g_pLTClient->RunConsoleCommand( "PausePhysics 0" );

	// Step the physics simulation a few times to let objects in the level settle.
	TLTPrecisionTime start = LTTimeUtils::GetPrecisionTime();
	float fPhysicsSettleTime = g_vtPhysicsSettleTime.GetFloat();
	float fPhysicsSettleRate = g_vtPhysicsSettleRate.GetFloat();
	g_pLTClient->PhysicsSim()->StepSimulationForward( fPhysicsSettleTime, fPhysicsSettleRate, 
		( uint32 )( fPhysicsSettleTime / fPhysicsSettleRate + 0.5f ));
	double fPreSteppingTime = LTTimeUtils::GetPrecisionTimeIntervalS( start, LTTimeUtils::GetPrecisionTime());
	DebugCPrint( 1, "Pre-stepping physics took %.3f seconds", fPreSteppingTime );


	// Set misc console vars...

	MirrorSConVar("AllSkyPortals", "AllSkyPortals");


	// Set up the global (per level) fog values...

	ResetDynamicWorldProperties();


	// Start with a clean slate...

	CBindMgr::GetSingleton().ClearAllCommands();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::MirrorSConVar
//
//	PURPOSE:	Takes the value of the server-side variable specified by
//				pSVarName and sets its value into the client-sdie variable
///				specified by pCVarName.
//
// --------------------------------------------------------------------------- //
void CGameClientShell::MirrorSConVar(char *pSVarName, char *pCVarName)
{
	float fVal = 0.0f;

    g_pLTClient->GetSConValueFloat(pSVarName, fVal);

	// Special case, make all farz calls go through this function...
	if (LTStrICmp(pCVarName, "FarZ") == 0)
	{
		SetFarZ((int)fVal);
	}
	else
	{
		g_pLTClient->SetConsoleVariableFloat(pCVarName, fVal);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ResetGlobalFog
//
//	PURPOSE:	Reset the global fog values based on the saved values...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ResetDynamicWorldProperties(bool bUseWorldFog)
{
	// Set the FarZ for the level...

	MirrorSConVar("FarZ", "FarZ");
	MirrorSConVar("WorldNorth","WorldNorth");

	//Setup the fog to reflect the server's fog if applicable
	if(bUseWorldFog)
	{
		// See if fog should be disabled
		uint32 dwAdvancedOptions = GetInterfaceMgr( )->GetAdvancedOptions();

		if (!(dwAdvancedOptions & AO_FOG))
		{
			g_pLTClient->SetConsoleVariableFloat("FogEnable", 0.0f);
			return;
		}

		MirrorSConVar("FogEnable", "FogEnable");
		MirrorSConVar("FogNearZ", "FogNearZ");
		MirrorSConVar("FogFarZ", "FogFarZ");
		MirrorSConVar("FogR", "FogR");
		MirrorSConVar("FogG", "FogG");
		MirrorSConVar("FogB", "FogB");

		MirrorSConVar("SkyFogEnable", "SkyFogEnable");
		MirrorSConVar("SkyFogNearZ", "SkyFogNearZ");
		MirrorSConVar("SkyFogFarZ", "SkyFogFarZ");
	}

	// Update the ambient light settings
	MirrorSConVar("Light_AmbientR", "Server_Light_AmbientR");
	MirrorSConVar("Light_AmbientG", "Server_Light_AmbientG");
	MirrorSConVar("Light_AmbientB", "Server_Light_AmbientB");

	MirrorSConVar("AddAmbientLightLow", "AddAmbientLightLow");
	MirrorSConVar("AddAmbientLightMed", "AddAmbientLightMed");
	MirrorSConVar("AddAmbientLightHigh", "AddAmbientLightHigh");

	// Update the ambient LOD values.
	CPerformanceMgr::Instance().ApplyAmbientLOD( );

	MirrorSConVar("Light_SkyAmbientR", "Light_SkyAmbientR");
	MirrorSConVar("Light_SkyAmbientG", "Light_SkyAmbientG");
	MirrorSConVar("Light_SkyAmbientB", "Light_SkyAmbientB");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::RenderCamera()
//
//	PURPOSE:	Sets up the client and renders the camera
//
// ----------------------------------------------------------------------- //

void CGameClientShell::RenderCamera(bool bDrawInterface)
{
	if (!GetPlayerMgr()->PreRender()) return;

	// Important to update this after the weapon model has been updated
	// (some fx depend on the position of the weapon model)...

	m_sfxMgr.UpdateSpecialFX();

	// Update any client-side special effects...

	g_pDamageFXMgr->Update();

	// Update any shattering effects
	m_ShatterEffectMgr.UpdateShatterEffects( SimulationTimer::Instance().GetTimerElapsedS( ));

	HLOCALOBJ hCamera = g_pPlayerMgr->GetPlayerCamera()->GetCamera();

	//update all the effects status as well as any that might effect the camera
	if(g_vtUpdateClientFX.GetFloat() != 0.0f)
	{
		m_SimulationTimeClientFXMgr.UpdateAllActiveFX();
		m_RealTimeClientFXMgr.UpdateAllActiveFX();
		FOREACH_RENDER_TARGET_MGR UpdateAllActiveFX();
	}

	//update our streaming information
	if(m_bResourceStreaming)
	{
		LTVector vCameraPos;
		g_pLTClient->GetObjectPos(hCamera, &vCameraPos);
		CGameStreamingMgr::Singleton().UpdateRegions(vCameraPos);
	}

	//reset our renderer stats
	ResetRendererFrameStats();

	if (!m_bMainWindowFocus) return;

	if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
	{
		//render the full scene and time how long it takes
		LARGE_INTEGER pcStart, pcEnd, pcFreq;
		QueryPerformanceFrequency( &pcFreq );
		QueryPerformanceCounter( &pcStart );

		//update our render targets
		LTRigidTransform tCamera;
		g_pLTClient->GetObjectTransform(hCamera, &tCamera);
		LTVector2 vCameraFOV;
		g_pLTClient->GetCameraFOV(hCamera, &vCameraFOV.x, &vCameraFOV.y);

		CRenderTargetFX::IncrementCurrentFrame();
		m_sfxMgr.UpdateRenderTargets(tCamera, vCameraFOV);

		// Render the camera...
		g_pPlayerMgr->GetPlayerCamera( )->Render( );

		//determine how long it took to render
		QueryPerformanceCounter( &pcEnd );
		float fRenderTimeMS = ((float)(pcEnd.QuadPart - pcStart.QuadPart) * 1000.0f / (float)pcFreq.QuadPart);

		// Render the the dynamic FX.
		m_sfxMgr.RenderFX(hCamera);

		GetInterfaceMgr( )->Draw();

		// Display any necessary debugging info...
		UpdateAndRenderUserNotifiers(RealTimeTimer::Instance().GetTimerElapsedS( ));

		//display our streaming HUD
		if(m_bResourceStreaming)
		{
			CGameStreamingMgr::Singleton().DisplayStreamingHUD(RealTimeTimer::Instance().GetTimerElapsedS());	
		}

		//write out the console information for the frame stats
		DisplayFrameStatConsoleInfo(fRenderTimeMS);

		g_pLTClient->RenderConsoleToRenderTarget();
		g_pLTClient->GetRenderer()->End3D();
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DontRenderCamera()
//
//	PURPOSE:	Sets up the client but doesn't render the camera
//
// ----------------------------------------------------------------------- //

void CGameClientShell::DontRenderCamera()
{
 	GetPlayerMgr()->PreRender();
 
 	m_sfxMgr.UpdateSpecialFX();

 	//update all the effects status as well as any that might effect the camera
	if(g_vtUpdateClientFX.GetFloat() != 0.0f)
	{
		m_SimulationTimeClientFXMgr.UpdateAllActiveFX();
		m_RealTimeClientFXMgr.UpdateAllActiveFX();
		FOREACH_RENDER_TARGET_MGR UpdateAllActiveFX();
	}
 }

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnWorldLoadingProgress()
//
//	PURPOSE:	Performs any processing necessary during world loading 
//				progress notifications.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnWorldLoadingProgress()
{
	if (g_pClientConnectionMgr->IsConnectedToRemoteServer())
	{
		// make sure the server holds our connection while we're loading
		uint32 nCurrentTime = LTTimeUtils::GetTimeMS();
		if ((nCurrentTime - m_nLastKeepAliveTime) > k_nKeepAliveIntervalMS)
		{
			g_pClientConnectionMgr->SendKeepAliveMessage();
			m_nLastKeepAliveTime = nCurrentTime;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::InitializePunkBuster()
//
//	PURPOSE:	Initialize the PunkBuster client.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::InitializePunkBuster()
{
	if (!m_pPunkBusterClient)
	{
		IPunkBusterClient::StartupInfo startupInfo;
		startupInfo.m_PBClientCallBacks.GetVersion = PunkBusterGetGameVersionCallback ;
		startupInfo.m_PBClientCallBacks.GetServerAddr = PunkBusterGetServerAddressCallback ;
		startupInfo.m_PBClientCallBacks.GetSocket = PunkBusterGetSocketCallback ;
		startupInfo.m_PBClientCallBacks.DisplayMessage = PunkBusterDisplayMessageCallback ;
		startupInfo.m_PBClientCallBacks.SendGameMessage = PunkBusterSendGameMessageCallback ;
		startupInfo.m_PBClientCallBacks.GetMapName = PunkBusterGetMapNameCallback ;
		startupInfo.m_PBClientCallBacks.GetServerHostName = PunkBusterGetServerHostNameCallback ;
		startupInfo.m_PBClientCallBacks.GetGameName = PunkBusterGetGameNameCallback ;
		startupInfo.m_PBClientCallBacks.GetPlayerName = PunkBusterGetPlayerNameCallback ;
		startupInfo.m_PBClientCallBacks.GetCvarValue = PunkBusterGetConsoleVariableCallback ;
		m_pPunkBusterClient = g_pLTGameUtil->CreatePunkBusterClient( startupInfo );

		// initialize PunkBuster client
		m_pPunkBusterClient->Initialize();

		// Make sure to use the current profile's desired setting.  The profile might have been 
		// loaded before pb got initialized here, so we need to apply the setting now.
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile( );
		if( pProfile )
		{
			if( pProfile->GetUsePunkbuster() && !m_pPunkBusterClient->IsEnabled( ))
			{
				m_pPunkBusterClient->Enable( );
			}
			else if( !pProfile->GetUsePunkbuster() && m_pPunkBusterClient->IsEnabled( ))
			{
				m_pPunkBusterClient->Disable( );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::TerminatePunkBuster()
//
//	PURPOSE:	Terminate the PunkBuster client;
//
// ----------------------------------------------------------------------- //

void CGameClientShell::TerminatePunkBuster()
{
	if (m_pPunkBusterClient)
	{
		g_pLTGameUtil->DestroyPunkBusterClient(m_pPunkBusterClient);
		m_pPunkBusterClient = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::CheckForCancel()
//
//	PURPOSE:	Determines if the current operation should be canceled.
//
// ----------------------------------------------------------------------- //

LTRESULT CGameClientShell::CheckForCancel()
{
	LTRESULT nResult = LT_NO;

	if (GetInterfaceMgr()->GetLoadFailed())
	{
		nResult = LT_YES;
	}

	return nResult;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::BuildClientSaveMsg
//
//	PURPOSE:	Save all the necessary client-side info
//
// --------------------------------------------------------------------------- //

void CGameClientShell::BuildClientSaveMsg(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	if (!pMsg) return;

	// Save the state of the save data...

	pMsg->Writeuint8( eSaveDataState );

	// Save complex data members...

	GetInterfaceMgr( )->Save(pMsg);
	GetPlayerMgr()->Save(pMsg, eSaveDataState);

	m_Mixer.Save(pMsg);
	
	m_pModelDecalMgr->Save(pMsg, eSaveDataState);

	// Save all necessary data members...

	// No data members to save for now.
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UnpackClientSaveMsg
//
//	PURPOSE:	Load all the necessary client-side info
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UnpackClientSaveMsg(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    m_bRestoringGame = true;

	// Get the state of the data we are about to load...

	SaveDataState eLoadDataState = static_cast<SaveDataState>(pMsg->Readuint8());

	// Load complex data members...

	GetInterfaceMgr( )->Load(pMsg);
	GetPlayerMgr()->Load(pMsg, eLoadDataState);
	
	m_Mixer.Load(pMsg);

	m_pModelDecalMgr->Load(pMsg, eLoadDataState);

	// Load data members...

	// No data members to load for now.
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnModelKey
//
//	PURPOSE:	Handle weapon model keys
//
// --------------------------------------------------------------------------- //

void CGameClientShell::OnModelKey(HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList *pArgs)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// give the client weapon mgr first shot at it
	if( g_pPlayerMgr->GetClientWeaponMgr()->OnModelKey( hObj, hTrackerID, pArgs ))
		return;

	// Check if its the player body...
	if( CPlayerBodyMgr::Instance( ).OnModelKey( hObj, hTrackerID, pArgs ))
		return;

	// try the sfx mgr
	m_sfxMgr.OnModelKey(hObj, pArgs, hTrackerID);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	OnGetOcclusionFromPoly
//
//	PURPOSE:	Gets the occlusion information from an HPOLY provided by sound..
//
// --------------------------------------------------------------------------- //

void CGameClientShell::OnGetOcclusionFromPoly(HPOLY *pHPoly, float *occlusionDampening, float *occlusionLFRatio)
{

	CClientShellScopeTracker cScopeTracker;

	// make sure we don't get a memory error if the pointers are invalid...
	if (occlusionDampening && occlusionLFRatio)	
	{
		uint8 nOccIndex;

		g_pLTClient->Common()->GetPolyOcclusionIndex(*pHPoly, &nOccIndex);

		if (nOccIndex == OCCLUSION_USE_MATERIAL)
		{
			// simply get the surface info from the HPOLY and..
			SurfaceType eType = GetSurfaceType( *pHPoly );
			HSURFACE hSurf = g_pSurfaceDB->GetSurface(eType);

			// .. copy the occlusion data...
			*occlusionDampening = g_pSurfaceDB->GetFloat(hSurf,SrfDB_Srf_fOcclusionDampening);
			*occlusionLFRatio = g_pSurfaceDB->GetFloat(hSurf,SrfDB_Srf_fOcclusionLFRatio);
		}
		else
		{
			// get the occlusion from the occlusion DB using the index..
			HSOUNDOCCLUSION hOcc;

			hOcc = g_pSoundOcclusionDB->GetSoundOcclusion(nOccIndex);

			// .. copy the occlusion data...
			*occlusionDampening = g_pSoundOcclusionDB->GetSoundOcclusionDampening(hOcc);
			*occlusionLFRatio = g_pSoundOcclusionDB->GetSoundOcclusionLFRatio(hOcc);

		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	OnGetSoundFilterFromName
//
//	PURPOSE:	Gets the sound filter info from the name
//
// --------------------------------------------------------------------------- //
void CGameClientShell::OnGetSoundFilterParamCountFromName(char* pszSoundFilter, int32* piNumParams)
{
	*piNumParams = SoundFilterDB::Instance().GetFilterParmeterCount();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	OnGetSoundFilterFromName
//
//	PURPOSE:	Gets the sound filter info from the name
//
// --------------------------------------------------------------------------- //
void CGameClientShell::OnGetSoundFilterParamFromName(char* pszSoundFilter, int32 nIndex, char* pszParamName, int32 nParamLength, float* pfParamValue)
{
	HRECORD hSoundFilter = SoundFilterDB::Instance().GetFilterRecord(pszSoundFilter);

	const char* const pszFilterParameterName = SoundFilterDB::Instance().GetFilterParmeterName(hSoundFilter, nIndex);

	LTStrCpy(pszParamName, pszFilterParameterName, nParamLength);

	*pfParamValue = SoundFilterDB::Instance().GetFilterParmeterValue(hSoundFilter, nIndex);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	OnGetSoundFilterFromPoint
//
//	PURPOSE:	Gets the sound filter name based on a position.
//
// --------------------------------------------------------------------------- //

void CGameClientShell::OnGetSoundFilterFromPoint(LTVector *pVector, char* pszSoundFilter, int32 nMaxChars)
{
	HRECORD hSoundFilterRecord = SoundFilterDB::Instance().GetDynamicFilterRecord();

	// [KLS 4/16/02] Find container objects that we care about...

	HLOCALOBJ hContainerObj = NULL;
	ContainerCode eCode = CC_NO_CONTAINER;

	HLOCALOBJ objList[MAX_OVERLAPPING_CONTAINERS];
	uint32 nContainerFlags = (CC_ALL_FLAG & ~CC_PLAYER_IGNORE_FLAGS);
	uint32 dwNum = ::GetPointContainers(*pVector, objList, MAX_OVERLAPPING_CONTAINERS, nContainerFlags);

	for (uint32 i=0; i < dwNum; i++)
	{
		uint16 code;
		if (g_pLTClient->GetContainerCode(objList[i], &code))
		{
			ContainerCode eTempCode = (ContainerCode)code;

			// Ignore dynamic sector volumes...
			if (CC_DYNAMIC_SECTOR_VOLUME != eTempCode)
			{
				if (CC_NO_CONTAINER == eCode || CC_FILTER == eCode)
				{
					hContainerObj = objList[i];
					eCode = eTempCode;
				}
			}
		}
	}

	if (hContainerObj)
	{
		CVolumeBrushFX* pFX = dynamic_cast<CVolumeBrushFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_VOLUMEBRUSH_ID, hContainerObj));

		if (pFX)
		{
			// Set the sound filter override...
			hSoundFilterRecord = pFX->GetSoundFilterRecord();
		}
	} 

	const char* pszName;
	pszName = SoundFilterDB::Instance().GetFilterRecordName(hSoundFilterRecord);

	LTStrCpy(pszSoundFilter, pszName, nMaxChars);
}

#if defined(PLATFORM_WIN32)

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HookedWindowProc
//
//	PURPOSE:	Hook it real good
//
// --------------------------------------------------------------------------- //

LRESULT CALLBACK HookedWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	switch(uMsg)
	{
		HANDLE_MSG(hWnd, WM_LBUTTONUP, CGameClientShell::OnLButtonUp);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, CGameClientShell::OnLButtonDown);
		HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, CGameClientShell::OnLButtonDblClick);
		HANDLE_MSG(hWnd, WM_RBUTTONUP, CGameClientShell::OnRButtonUp);
		HANDLE_MSG(hWnd, WM_RBUTTONDOWN, CGameClientShell::OnRButtonDown);
		HANDLE_MSG(hWnd, WM_RBUTTONDBLCLK, CGameClientShell::OnRButtonDblClick);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, CGameClientShell::OnMouseMove);
		HANDLE_MSG(hWnd, WM_MOUSEWHEEL, CGameClientShell::OnMouseWheel);
		HANDLE_MSG(hWnd, WM_SETCURSOR, OnSetCursor);
 /*
  *	
  	case WM_KEYDOWN:
		{
			UINT vk = wParam;
			UINT scancode = HIWORD(lParam);
			scancode = LOBYTE(scancode);

			static BYTE keystate[256];

			if (GetKeyboardState(keystate))
			{
				static wchar_t szTmp[8];
				int num = ToUnicodeEx(vk,scancode,keystate,szTmp,LTARRAYSIZE(szTmp),0,GetKeyboardLayout(0));
				wchar_t* pC = szTmp;
				while (num > 0)
				{
					pC++;
					num--;
				}
			}

	}
		break;
	case WM_KEYUP:
		{
			UINT vk = wParam;
			UINT scancode = HIWORD(lParam);
			scancode = LOBYTE(scancode);

			static BYTE keystate[256];

			if (GetKeyboardState(keystate))
			{
				static wchar_t szTmp[8];
				int num = ToUnicodeEx(vk,scancode,keystate,szTmp,LTARRAYSIZE(szTmp),0,GetKeyboardLayout(0));
				wchar_t* pC = szTmp;
				while (num > 0)
				{
					CGameClientShell::OnChar(hWnd,*pC);
					pC++;
					num--;
				}
			}

	}
		break;
		*/

		case WM_CHAR: 
			CGameClientShell::OnChar(hWnd,wParam);
			break;
	}

	_ASSERT(g_pfnMainWndProc);
	return(CallWindowProcW(g_pfnMainWndProc,hWnd,uMsg,wParam,lParam));
}

void CGameClientShell::OnChar(HWND hWnd, WPARAM c)
{
	g_pInterfaceMgr->OnChar( (wchar_t)c);
}


void CGameClientShell::OnLButtonUp(HWND hWnd, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnLButtonUp(x,y);
}

void CGameClientShell::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	/*if (!g_tmrDblClick.Stopped() &&
		(g_mouseMgr.GetClickPosX() == x) && (g_mouseMgr.GetClickPosY() == y))
	{
		g_tmrDblClick.Stop();
		OnLButtonDblClick(hwnd,fDoubleClick,x,y,keyFlags);
	}
	else
	{
		g_mouseMgr.SetClickPos(x,y);
		g_tmrDblClick.Start(.5);
	}*/

	g_pInterfaceMgr->OnLButtonDown(x,y);
}

void CGameClientShell::OnLButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnLButtonDblClick(x,y);
}

void CGameClientShell::OnRButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnRButtonUp(x,y);
}

void CGameClientShell::OnRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnRButtonDown(x,y);
}

void CGameClientShell::OnRButtonDblClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	g_pInterfaceMgr->OnRButtonDblClick(x,y);
}

void CGameClientShell::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	//g_mouseMgr.SetMousePos(x,y);
	g_pInterfaceMgr->OnMouseMove(x,y);
}

void CGameClientShell::OnMouseWheel(HWND hwnd, int xPos, int yPos, int zDelta, UINT fwKeys)
{
	// the coordinates are in screen space, convert to client
	POINT ptCursor = { xPos, yPos };
	::ScreenToClient( hwnd, &ptCursor );
	g_pInterfaceMgr->OnMouseWheel(ptCursor.x,ptCursor.y,zDelta);
}

BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	return TRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	SetWindowSize
//
//	PURPOSE:	This will resize the window to the specified dimensions and clip the cursor if necessary
//
// --------------------------------------------------------------------------- //

BOOL SetWindowSize(uint32 nWidth, uint32 nHeight)
{
	// Clip the cursor if we're NOT in a window
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVariable("Windowed");
	BOOL bClip = TRUE;
	if(hVar)
	{
        float fVal = g_pLTClient->GetConsoleVariableFloat(hVar);
		if(fVal == 1.0f)
			bClip = FALSE;
	}

	RECT screenRect;
	GetWindowRect(GetDesktopWindow(), &screenRect);


	if(bClip)
	{
  		SetWindowLongW(g_hMainWnd,GWL_STYLE,WS_VISIBLE);
		SetWindowPos(g_hMainWnd,HWND_TOPMOST,0,0,screenRect.right - screenRect.left,screenRect.bottom - screenRect.top,SWP_FRAMECHANGED);

		RECT wndRect;
		GetWindowRect(g_hMainWnd, &wndRect);
		ClipCursor(&wndRect);
	}
	else
	{
		SetWindowPos(g_hMainWnd,HWND_NOTOPMOST,
					((screenRect.right - screenRect.left) - (int32)nWidth) / 2,
					((screenRect.bottom - screenRect.top) - (int32)nHeight) / 2, 
					nWidth, nHeight,SWP_FRAMECHANGED);
		ShowWindow(g_hMainWnd, SW_NORMAL);
	}

	return TRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HookWindow
//
//	PURPOSE:	HOOK IT!
//
// --------------------------------------------------------------------------- //

BOOL HookWindow()
{
	// Hook the window
    if(g_pLTClient->GetEngineHook("HWND",(void **)&g_hMainWnd) != LT_OK)
	{
		TRACE("HookWindow - ERROR - could not get the engine window!\n");
		return FALSE;
	}

	// Get the window procedure
#ifdef STRICT
	g_pfnMainWndProc = (WNDPROC)GetWindowLongW(g_hMainWnd,GWL_WNDPROC);
#else
	g_pfnMainWndProc = (FARPROC)GetWindowLongW(g_hMainWnd,GWL_WNDPROC);
#endif

	if(!g_pfnMainWndProc)
	{
		TRACE("HookWindow - ERROR - could not get the window procedure from the engine window!\n");
		return FALSE;
	}

	// Replace it with ours
	if(!SetWindowLongW(g_hMainWnd,GWL_WNDPROC,(LONG)HookedWindowProc))
	{
		TRACE("HookWindow - ERROR - could not set the window procedure!\n");
		return FALSE;
	}

	return TRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	UnhookWindow
//
//	PURPOSE:	Unhook the window
//
// --------------------------------------------------------------------------- //

void UnhookWindow()
{
	if(g_pfnMainWndProc && g_hMainWnd)
	{
		SetWindowLongW(g_hMainWnd, GWL_WNDPROC, (LONG)g_pfnMainWndProc);
		g_hMainWnd = 0;
		g_pfnMainWndProc = NULL;
	}
}

#endif // PLATFORM_WIN32

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetDifficulty
//
//	PURPOSE:	Dynamically change our difficulty level
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetDifficulty(GameDifficulty e)
{
	m_eDifficulty = e;
	
	//check to see if we've got a server
	if (g_pLTClient->IsConnected())
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_DIFFICULTY);
		cMsg.Writeuint8(m_eDifficulty);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetFarZ
//
//	PURPOSE:	Localize setting the far z (cause it can screw things up
//				when set to 0)
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetFarZ(int nFarZ)
{
	// Don't EVER set the farZ really close!
	if (nFarZ > 50)
	{
		WriteConsoleInt("FarZ", nFarZ);
	}
}

void CGameClientShell::EnterClientShell()
{
#if defined(PLATFORM_SEM)
	if (m_nEntryCount == 0)
	{
		// We shouldn't already have an external scope base interface value if we're entering this scope
		LTASSERT(m_pExternalScopeBaseInterface == NULL, "Scope entry with non-NULL external interface pointer encountered");
		// Save the external scope
		m_pExternalScopeBaseInterface = g_pLTBase;
		// Switch the interfaces over to the client
	    g_pModelLT = g_pLTClient->GetModelLT();
		g_pPhysicsLT = g_pLTClient->Physics();
		g_pLTBase = static_cast<ILTCSBase*>(g_pLTClient);
		g_pCommonLT = g_pLTClient->Common();
	}
	// Track our entry level for proper re-entrant behavior
	++m_nEntryCount;
	LTASSERT(g_pLTBase == g_pLTClient, "Scope entry encountered with invalid interface");
#endif // PLATFORM_SEM
}

void CGameClientShell::ExitClientShell()
{
#if defined(PLATFORM_SEM)
	// If we're exiting the context, fix up the interfaces
	if (m_nEntryCount == 1)
	{
		// If we previosly had an external scope, restore the interfaces
		if (m_pExternalScopeBaseInterface != NULL)
		{
			g_pModelLT = m_pExternalScopeBaseInterface->GetModelLT();
			g_pPhysicsLT = m_pExternalScopeBaseInterface->Physics();
			g_pLTBase = static_cast<ILTCSBase*>(m_pExternalScopeBaseInterface);
			g_pCommonLT = m_pExternalScopeBaseInterface->Common();
			m_pExternalScopeBaseInterface = NULL;
		}
		// Otherwise set them to null
		else
		{
			g_pModelLT = NULL;
			g_pPhysicsLT = NULL;
			g_pLTBase = NULL;
			g_pCommonLT = NULL;
		}
	}
	// Track the entry count for proper re-entrancy handling
	if (m_nEntryCount != 0)
		--m_nEntryCount;
#endif // PLATFORM_SEM
}

void CGameClientShell::PostLevelLoadFirstUpdate()
{
	//allow the streaming system to handle the level switch
	if(m_bResourceStreaming)
		CGameStreamingMgr::Singleton().OnLevelLoaded();

	// Draw the screen once
	g_pPlayerMgr->UpdateCamera();
	RenderCamera(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateSpeedMonitor()
//
//	PURPOSE:	Handle updating the server speed monitor...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateSpeedMonitor()
{
	double fLocalTime = g_pLTClient->GetTime();
	double fServerTime = g_pLTClient->GetGameTime();
	if(fServerTime - m_fInitialServerTime > 5.0f)
	{
		// time to do our check
		double fServerDelta	= fServerTime - m_fInitialServerTime;
		double fLocalDelta	= fLocalTime - m_fInitialLocalTime;

		if(fServerDelta / fLocalDelta < 0.98)
		{
			// possible cheater, increment cheat counter
			m_nSpeedCheatCounter++;

			if(m_nSpeedCheatCounter > 24)
			{
				// Disconnect from the server.
				if(g_pLTClient->IsConnected())
				{
					m_pClientMultiplayerMgr->DisconnectFromServer();
				}
			}
		}
		else
		{
			// reset the instance counter
			m_nSpeedCheatCounter = 0;
		}

		// reset the time counters
		m_fInitialServerTime	= fServerTime;
		m_fInitialLocalTime		= fLocalTime;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterGetServerAddressCallback
//
//	PURPOSE:	PunkBuster callback function for retrieving the server's
//				address.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterGetServerAddressCallback(char* pszServerAddressBuffer, int nServerAddressBufferLength)
{
	*pszServerAddressBuffer = NULL;

	// check for the game client shell and LTClient
	if (!g_pGameClientShell || !g_pLTClient)
	{
		return;
	}

	// do not return a server address unless we're in the world
	if (!g_pGameClientShell->m_bInWorld)
	{
		return;
	}

	// do not return a server address if we're in singleplayer
	if (!IsMultiplayerGameClient())
	{
		return;
	}

	// if we're the local client in a hosted game, we need to return "localhost",
	// otherwise return the server's IP address
	bool bIsHost;
	if (g_pLTClient->IsLocalToServer(&bIsHost) == LT_OK && bIsHost)
	{
		LTStrCpy(pszServerAddressBuffer, "localhost", nServerAddressBufferLength);
	}
	else
	{
		// get the server's IP address
		uint8  aAddressBuffer[4] = { 0 };
		uint16 nPort = 0;
		if (g_pLTClient->GetServerIPAddress(aAddressBuffer, &nPort) != LT_OK)
		{
			LTERROR("Failed to retrieve server IP address");
		}
		else
		{
			// build the string
			LTSNPrintF(pszServerAddressBuffer, nServerAddressBufferLength, "%d.%d.%d.%d:%d",
					   aAddressBuffer[0], aAddressBuffer[1], aAddressBuffer[2], aAddressBuffer[3], nPort);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterGetGameVersionCallback
//
//	PURPOSE:	PunkBuster callback function for retrieving the game
//				version string.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterGetGameVersionCallback(char* pszGameVersionBuffer, int nGameVersionBufferLength)
{
	LTStrCpy(pszGameVersionBuffer, g_pVersionMgr->GetNetVersion(), nGameVersionBufferLength);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterGetSocketCallback
//
//	PURPOSE:	PunkBuster callback function for retrieving client socket.
//				
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterGetSocketCallback(SOCKET *sock)
{
	// initialize to INVALID_SOCKET
	*sock = INVALID_SOCKET;

	if (!g_pLTClient)
	{
		return;
	}

	// if we're the local client in a hosted game the use the server's socket,
	// otherwise use the client socket
	bool bIsHost = false;
	if (g_pLTClient->IsLocalToServer(&bIsHost) == LT_OK && bIsHost)
	{
		g_pLTClient->GetHostUDPSocket(*sock);
	}
	else 
	{
		g_pLTClient->GetUDPSocket(*sock);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterSendGameMessageCallback
//
//	PURPOSE:	PunkBuster callback function for sending a game-stream network
//              message.
//
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterSendGameMessageCallback(char *data, int datalen)
{
	// build the message
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PUNKBUSTER_MSG);
	cMsg.WriteData(data, datalen*8);

	// send it to the server (unguaranteed, since PunkBuster manages retries internally)
	g_pLTClient->SendToServer(cMsg.Read(), 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterDisplayMessageCallback
//
//	PURPOSE:	PunkBuster callback function for displaying a HUD message.
//
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterDisplayMessageCallback(char *data, int datalen, int skipnotify)
{
	StartGameRequest& cStartGameRequest = g_pClientConnectionMgr->GetStartGameRequest();
	bool bMultiplayerClient = (cStartGameRequest.m_Type == STARTGAME_CLIENT || cStartGameRequest.m_Type == STARTGAME_HOST);

	// copy the string
	uint32 nDataCopyLength = LTStrLen(data);
	char* pDataCopy = (char*)alloca(nDataCopyLength + 1);
	LTStrCpy(pDataCopy, data, nDataCopyLength + 1);

	// convert CR characters to spaces
	for (uint32 nIndex = 0; nIndex < nDataCopyLength; ++nIndex)
	{
		if (pDataCopy[nIndex] == '\r')
		{
			pDataCopy[nIndex] = ' ';
		}
	}

	// only display PunkBuster messages on the HUD if we're a multiplayer client
	if (bMultiplayerClient && !skipnotify && g_pGameMsgs)
	{	
		g_pGameMsgs->AddMessage(MPA2W(pDataCopy).c_str());
	}

	g_pLTClient->CPrint(data);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterGetMapNameCallback
//
//	PURPOSE:	PunkBuster callback function for retrieving the current map.
//
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterGetMapNameCallback(char *data, int datalen)
{

	LTFileOperations::SplitPath( g_pMissionMgr->GetCurrentWorldName(), NULL, data, NULL );

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameServerShell::PunkBusterGetServerHostNameCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving the server hostname.
//
//
// --------------------------------------------------------------------------- //

void CGameClientShell::PunkBusterGetServerHostNameCallback(char *data, int datlen)
{

	strncpy ( data, MPW2A( g_pClientConnectionMgr->GetServerName() ).c_str( ), datlen ) ;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterGetGameNameCallback()
//
//	PURPOSE:	PunkBuster callback function for retrieving the game/mod name.
//
//
// --------------------------------------------------------------------------- //

void CGameClientShell::PunkBusterGetGameNameCallback(char *data, int datlen)
{
	*data = 0;	
	LTStrCpy(data, g_pClientConnectionMgr->GetModName(), datlen);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterGetPlayerNameCallback
//
//	PURPOSE:	PunkBuster callback function for retrieving the player name.
//
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterGetPlayerNameCallback(char *data, int datalen)
{
	
	strncpy ( data, MPW2A ( g_pProfileMgr->GetCurrentProfile( )->m_sPlayerName.c_str() ), datalen );
  
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterProcessCommand
//
//	PURPOSE:	PunkBuster callback function for sending PB a command.
//
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterProcessCommand(char *cmd)
{
	if ( m_pPunkBusterClient )
		m_pPunkBusterClient->ProcessCmd( cmd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PunkBusterGetConsoleVariableCallback
//
//	PURPOSE:	PunkBuster callback function for retrieving the value
//				of a console variable.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PunkBusterGetConsoleVariableCallback(const char* pszName, char* pszValueBuffer, int nValueBufferLength)
{
	*pszValueBuffer = NULL;

	if (!pszName)
	{
		return;
	}

	HCONSOLEVAR hVariable = g_pLTClient->GetConsoleVariable(pszName);
	if (hVariable)
	{	
		LTStrCpy(pszValueBuffer, g_pLTClient->GetConsoleVariableString(hVariable), nValueBufferLength);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::LauncherServerApp()
//
//	PURPOSE:	Launches the serverapp.
//
// --------------------------------------------------------------------------- //

bool CGameClientShell::LauncherServerApp( char const* pszOptionsFile )
{
	if( !LaunchApplication::LaunchServerApp( pszOptionsFile ))
		return false;

	// Flag that we launched the server app so that other code knows we
	// are exiting due to it.
	m_bLaunchedServerApp = true;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ProcessPacket
//
//	PURPOSE:	Handling for connectionless networking packets
//
// ----------------------------------------------------------------------- //

LTRESULT CGameClientShell::ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort)
{
	// Track the current execution shell scope for proper SEM behavior
	CClientShellScopeTracker cScopeTracker;

	// Gotta have some data...
	if (pMsg->Size() < 16)
		return LT_OK;

// Gamespy and PunkBuster not available in Xenon builds
#if !defined(PLATFORM_XENON)
	
		// Read the data out of the message.
		uint32 nDataLen = pMsg->Size() / 8;
		uint8* pData = ( uint8* )alloca( nDataLen );
		pMsg->ReadData( pData, nDataLen * 8 );

	// check if the message is for PunkBuster
	if (m_pPunkBusterClient)
	{
		if (m_pPunkBusterClient->HandleNetMessage(pData, nDataLen, senderAddr, senderPort))
		{
			// handled by PunkBuster
			return LT_OK;
		}
	}

	// if it's not a PunkBuster message, check if we have a gamespy server browser
	if (g_pClientConnectionMgr->GetServerBrowser())
	{
		// See if the gamespybrowser wants it.
		if (g_pClientConnectionMgr->GetServerBrowser( )->HandleNetMessage( pData, nDataLen, senderAddr, senderPort ))
		{
			// handled by GameSpy
			return LT_OK;
		}
	}

#endif // !PLATFORM_XENON

	// Reset to beginning
		pMsg->Seek( 0 );

	// Skip the engine-side portion of the message header
	CLTMsgRef_Read cSubMsg(pMsg->SubMsg(8));

	// Hand it off as if it were a message
	OnMessage(cSubMsg);

	return LT_OK;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ResetRendererFrameStats
//
//	PURPOSE:	Called to reset the frame stats associated with the renderer
//
// ----------------------------------------------------------------------- //
void CGameClientShell::ResetRendererFrameStats()
{
	m_RendererFrameStats.ClearFrameStats();
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DisplayFrameStatConsoleInfo
//
//	PURPOSE:	called to display all the relevant console information for the frame statistics
//
// ----------------------------------------------------------------------- //
void CGameClientShell::DisplayFrameStatConsoleInfo(float fRenderTimeMS)
{
	CFrameStatConsoleInfo::DisplayConsoleInfo(m_RendererFrameStats, fRenderTimeMS);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateAndRenderUserNotifiers
//
//	PURPOSE:	called to update and render the user notifiers. This will use the specified time
//				value for the update. This must be called within a begin/end 3d block.
//
// ----------------------------------------------------------------------- //
void CGameClientShell::UpdateAndRenderUserNotifiers(float fFrameTime)
{
#ifndef _FINAL
	//avoid this if the user has disabled it
	if(g_vtUserNotifiers.GetFloat() == 0.0f)
		return;

	m_UserNotificationMgr.UpdateNotifications(fFrameTime);

	//only render if we aren't in screenshot mode though
	if(!g_bScreenShotMode)
		m_UserNotificationMgr.RenderActiveNotifications();
#endif
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateDynamicSectorStates
//
//	PURPOSE:	Updates the state of the dynamic sector state list
//
// ----------------------------------------------------------------------- //
void CGameClientShell::UpdateDynamicSectorStates()
{
	// Try to update the visibility state for each of the buffered sector records
	for(uint32 nCurrSector = 0; nCurrSector < m_aDynamicSectorStateBuffer.size(); )
	{
		LTRESULT nResult = g_pLTClient->SetSectorVisibility(m_aDynamicSectorStateBuffer[nCurrSector].first, 
															m_aDynamicSectorStateBuffer[nCurrSector].second);

		if (nResult == LT_OK)
		{
			// If we succeeded, remove it from the list, and don't update our count so we hit
			//the item we just moved into its spot
			m_aDynamicSectorStateBuffer[nCurrSector] = m_aDynamicSectorStateBuffer[m_aDynamicSectorStateBuffer.size() - 1];
			m_aDynamicSectorStateBuffer.pop_back();
		}
		else
		{
			// If we didn't succeed, try again next frame
			nCurrSector++;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::TestVideoCardCompatibility
//
//	PURPOSE:	Returns true if the user's video card is compatible with the game
//
// ----------------------------------------------------------------------- //

bool CGameClientShell::TestVideoCardCompatibility()
{
	SPerformanceStats sStats;
	// Do a quick-test, and assume everything's ok if something bad happens
	if (g_pLTClient->QueryPerformanceStats(&sStats, true) != LT_OK)
		return true;
	return (sStats.m_fGPUMaxPS >= 1.1f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgSimulationTimerScale()
//
//	PURPOSE:	Handle server telling us the simulation timer scale.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgSimulationTimerScale( ILTMessage_Read* pMsg )
{
	// Read the timer scale.
	uint32 nNumerator = 1;
	uint32 nDenominator = 1;
	bool bRealTime = pMsg->Readbool();
	if( !bRealTime )
	{
		nNumerator = pMsg->Readuint16();
		nDenominator = pMsg->Readuint16();
	}

	// Set our simulation timer scale.
	SimulationTimer::Instance().SetTimerTimeScale( nNumerator, nDenominator );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ReloadClientFXFiles()
//
//	PURPOSE:	Called to reload the effect files and restart all effects. 
//				This does nothing during final builds
//
// ----------------------------------------------------------------------- //
bool CGameClientShell::ReloadClientFXFiles()
{
#ifndef _FINAL

	//first tell all effect managers to unbind from the database
	m_SimulationTimeClientFXMgr.ReleaseEffectDatabase();
	m_RealTimeClientFXMgr.ReleaseEffectDatabase();
	FOREACH_RENDER_TARGET_MGR ReleaseEffectDatabase();
	GetInterfaceMgr()->GetInterfaceFXMgr().ReleaseEffectDatabase();

	//now tell the database to unload and reload
	CClientFXDB::GetSingleton().UnloadFxFiles();
	if(!CClientFXDB::GetSingleton().LoadFxFilesInDir(g_pLTClient, CLIENT_FX_REL_RESOURCE_PATH))
	{
		//failure, terminate all effects
		m_SimulationTimeClientFXMgr.ShutdownAllFX();
		m_RealTimeClientFXMgr.ShutdownAllFX();
		FOREACH_RENDER_TARGET_MGR ShutdownAllFX();
		GetInterfaceMgr()->GetInterfaceFXMgr().ShutdownAllFX();

		return false;
	}

	//alright, we reloaded the effect files, now restart all of the effects
	m_SimulationTimeClientFXMgr.RestartEffects();
	m_RealTimeClientFXMgr.RestartEffects();
	FOREACH_RENDER_TARGET_MGR RestartEffects();
	GetInterfaceMgr()->GetInterfaceFXMgr().RestartEffects();
#endif	

	return true;
}

//called to handle game side intiialization of the resource manager
bool CGameClientShell::InitResourceMgr()
{
	//initialize our resource console variables prior to using them
	g_vtStreamResources.Init(g_pLTClient, "StreamResources", NULL, 0.0f);
	g_vtLoadUnprefetchedResources.Init(g_pLTClient, "LoadUnprefetchedResources", NULL, 1.0f);

	//are we streaming?
	m_bResourceStreaming = (g_vtStreamResources.GetFloat() != 0.0f);

	//initialize our resource system
	if(g_pLTClient->ResourceMgr()->InitResourceSystem(m_bResourceStreaming) != LT_OK)
		return false;

	//create the allocator that we will use for prefetching of resources
	ILTResourceAllocator* pPrefetchAllocator = NULL;
	g_pLTClient->ResourceMgr()->CreateRandomAccessAllocator(pPrefetchAllocator);
	if(!pPrefetchAllocator)
		return false;

	//assign this as our main allocator, and then release our reference since we no longer need it
	g_pLTClient->ResourceMgr()->SetPrefetchAllocator(pPrefetchAllocator);
	LTSafeRelease(pPrefetchAllocator);

	if(g_vtLoadUnprefetchedResources.GetFloat() != 0.0f)
	{
		if(m_bResourceStreaming)
		{
			//now enable prefetching on lock so we will start loading files when they are used
			g_pLTClient->ResourceMgr()->EnablePrefetchOnLock(true);
		}
		else
		{
			//now enable prefetching on request so we will always have files ready when needed
			g_pLTClient->ResourceMgr()->EnablePrefetchOnRequest(true);
		}
	}

	return true;
}

//called to handle game side cleanup of the resource manager
void CGameClientShell::TermResourceMgr()
{
	//disable auto prefetching
	g_pLTClient->ResourceMgr()->EnablePrefetchOnLock(false);
	g_pLTClient->ResourceMgr()->EnablePrefetchOnRequest(false);
}


CCharacterFX* CGameClientShell::GetLocalCharacterFX()
{
	return GetPlayerMgr( )->GetMoveMgr()->GetCharacterFX();
}

//called to initialize the rendering layers used by the game code
void CGameClientShell::InitRenderingLayers()
{
	//setup the player depth bias to shift itself by about half of the Z buffer
	g_pLTClient->GetRenderer()->SetDepthBiasTableIndex(eRenderLayer_Player, 0.0f, -0.5f);

	//setup the decal depth biases
	static const float kfDecalSlopeScale = -3.0f;

	//for the depth bias, do 3 depth values for a depth buffer with 24 bit resolution
	static const float kfDecalDepthScale = -3.0f / (float)0xFFFFFF;
	for(uint32 nLayer = 0; nLayer < eRenderLayer_NumDecalLayers; nLayer++)
	{
		g_pLTClient->GetRenderer()->SetDepthBiasTableIndex(	eRenderLayer_Decal0 + nLayer, 
															(float)(nLayer + 1) * kfDecalSlopeScale, 
															(float)(nLayer + 1) * kfDecalDepthScale);
	}
}

CClientFXMgr* CGameClientShell::GetRenderTargetClientFXMgr(CRenderTarget* pRenderTarget, bool bUseSimTime)
{
	// Search for a pre-existing manager with the passed in specs...
	RenderTargetClientFXKey key(pRenderTarget,bUseSimTime);
	RenderTargetClientFXMgrMap::iterator it = m_RenderTargetClientFXMgrs.find(key);
	if(it!=m_RenderTargetClientFXMgrs.end())
		return it->second;

	// Create a new one if needed...
	CClientFXMgr* pNewMgr = debug_new(CClientFXMgr);
	if(!pNewMgr)
	{
		LTERROR("Unable to allocate memory for a render target client fx manager");
		return NULL;
	}

	// Get the appropriate timer...
	EngineTimer& Timer = bUseSimTime ? (EngineTimer&)SimulationTimer::Instance() : (EngineTimer&)RealTimeTimer::Instance();

	// Init the ClientFX mgr...
	if (!pNewMgr->Init( g_pLTClient, Timer))
	{
		debug_delete(pNewMgr);
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Could not init SimulationClientFXMgr!" ));
		return NULL;
	}

	// Set up the manager's camera
	pNewMgr->SetCamera(g_pPlayerMgr->GetPlayerCamera()->GetCamera());

	// Add it to the list so we can delete it later on demand...
	m_RenderTargetClientFXMgrs.insert(std::make_pair(key,pNewMgr));

	return pNewMgr;
}

void CGameClientShell::ShutdownRenderTargetClientFXMgr(CRenderTarget* pRenderTarget)
{
	RenderTargetClientFXMgrMap::iterator it;

	it = m_RenderTargetClientFXMgrs.find(std::make_pair(pRenderTarget,true));
	if(it!=m_RenderTargetClientFXMgrs.end())
	{
		debug_delete(it->second);
		m_RenderTargetClientFXMgrs.erase(it);
	}

	it = m_RenderTargetClientFXMgrs.find(std::make_pair(pRenderTarget,false));
	if(it!=m_RenderTargetClientFXMgrs.end())
	{
		debug_delete(it->second);
		m_RenderTargetClientFXMgrs.erase(it);
	}
}

void CGameClientShell::ShutdownAllRenderTargetClientFXMgrs()
{
	RenderTargetClientFXMgrMap::iterator it;
	for(it=m_RenderTargetClientFXMgrs.begin(); it!=m_RenderTargetClientFXMgrs.end(); ++it)
	{
		debug_delete(it->second);
	}
	m_RenderTargetClientFXMgrs.clear();
}
