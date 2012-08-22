// ----------------------------------------------------------------------- //
//
// MODULE  : GameClientShell.cpp
//
// PURPOSE : Game Client Shell - Implementation
//
// CREATED : 9/18/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "ClientResShared.h"
#include "SoundTypes.h"
#include "Music.h"
#include "VolumeBrushFX.h"
#include "client_physics.h"
#include "WinUtil.h"
#include "WeaponStringDefs.h"
#include "iltmath.h"
#include "iltphysics.h"
#include "VarTrack.h"
#include "GameButes.h"
#include "SystemDependant.h"
#include "SurfaceFunctions.h"
#include "VehicleMgr.h"
#include "BodyFX.h"
#include "PlayerShared.h"
#include "CharacterFX.h"
#include "iltsoundmgr.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "ClientTrackedNodeMgr.h"
#include "CMoveMgr.h"
#include "PlayerCamera.h"
#include <stdarg.h>
#include <stdio.h>
#include "ClientMultiplayerMgr.h"
#include "ClientButeMgr.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"
#include "FXButeMgr.h"
#include "ClientFXDB.h"
#include <sys/timeb.h>
#include "PerformanceTest.h"
#include "ScmdConsole.h"
#include "ScmdConsoleDriver_CShell.h"
#include "mmsystem.h"

#ifdef STRICT
	WNDPROC g_pfnMainWndProc = NULL;
#else
	FARPROC	g_pfnMainWndProc = NULL;
#endif

#define MAX_FRAME_DELTA					0.1f

#define WEAPON_MOVE_INC_VALUE_SLOW		0.0025f
#define WEAPON_MOVE_INC_VALUE_FAST		0.005f

#define VK_TOGGLE_GHOST_MODE			VK_F1
#define VK_TOGGLE_SPECTATOR_MODE		VK_F2
#define VK_TOGGLE_SCREENSHOTMODE		VK_F3
#define VK_WRITE_CAM_POS				VK_F4
#define VK_MAKE_CUBIC_ENVMAP			VK_F12

	
uint32              g_dwSpecial         = 2342349;
bool				g_bScreenShotMode   = false;

HWND				g_hMainWnd = NULL;
RECT*				g_prcClip = NULL;

CGameClientShell*   g_pGameClientShell = NULL;

LTVector            g_vWorldWindVel(0.0f, 0.0f, 0.0f);

PhysicsState		g_normalPhysicsState;
PhysicsState		g_waterPhysicsState;

VarTrack			g_vtShowTimingTrack;
VarTrack			g_varStartLevelScreenFadeTime;
VarTrack			g_varStartLevelScreenFade;
VarTrack			g_vtUseSoundFilters;
VarTrack			g_vtSpecial;
VarTrack			g_vtMakeCubicEnvMapSize;
VarTrack			g_vtMakeCubicEnvMapName;
VarTrack			g_vtApplyWorldOffset;

VarTrack			g_vtPTestMinFPS;
VarTrack			g_vtPTestMaxFPS;

#ifdef _TRON_E3_DEMO
	#define TRON_E3_DEMO_IDLE_TIME		120.0f
	float             s_fDemoTime		= 0.0f;
#endif

extern CCheatMgr*	g_pCheatMgr;
extern LTVector		g_vPlayerCameraOffset;
extern VarTrack		g_vtFOVXNormal;
extern VarTrack		g_vtFOVYNormal;

// Sample rate
extern int g_nSampleRate;

// Speed hack prevention variables
static _timeb g_StartTimeB;
static uint32 g_nStartClientTime = 0;
static uint32 g_nStartTicks = 0;

BOOL SetWindowSize(uint32 nWidth, uint32 nHeight);
BOOL HookWindow();
void UnhookWindow();

BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
LRESULT CALLBACK HookedWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);








void InitClientShell()
{
	// Hook up the AssertMgr

	CAssertMgr::Enable();

	// Get our ClientDE pointer

    _ASSERT(g_pLTClient);

	// Init our LT subsystems

    g_pModelLT = g_pLTClient->GetModelLT();
    g_pTransLT = g_pLTClient->GetTransformLT();
    g_pPhysicsLT = g_pLTClient->Physics();
	g_pLTBase = static_cast<ILTCSBase*>(g_pLTClient);

	g_pPhysicsLT->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);
}

void TermClientShell()
{
	// Delete our client shell

	// Unhook the AssertMgr and let the CRT handle asserts once again

	CAssertMgr::Disable();
}


void FragSelfFn(int argc, char **argv)
{
	SendEmptyServerMsg(MID_FRAG_SELF);
}

void ClientFXFn(int argc, char **argv)
{
	if(argc == 4)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_CONSOLE_CLIENTFX);
		cMsg.Writeuint8(0); // 0 = No object
		cMsg.WriteString(argv[0]);
		LTVector vPos;
		vPos.x = (float)atof(argv[1]);
		vPos.y = (float)atof(argv[2]);
		vPos.z = (float)atof(argv[3]);
		cMsg.WriteLTVector(vPos);
		g_pLTClient->SendToServer(cMsg.Read(),MESSAGE_GUARANTEED);
	}
	else if(argc == 2)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_CONSOLE_CLIENTFX);
		cMsg.Writeuint8(1); // 1 = Object
		cMsg.WriteString(argv[0]);
		cMsg.WriteString(argv[1]);
		g_pLTClient->SendToServer(cMsg.Read(),MESSAGE_GUARANTEED);
	}
	else
	{
		g_pLTClient->CPrint("ClientFX <fxname> [<objectname> OR <xPos> <yPos> <zPos>]\n");
	}
}

void StimulusFn(int argc, char **argv)
{
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

void ReloadWeaponAttributesFn(int argc, char **argv)
{
    g_pWeaponMgr->Reload();
    g_pLTClient->CPrint("Reloaded weapons attributes file...");
}

void ReloadSurfacesAttributesFn(int argc, char **argv)
{
    g_pSurfaceMgr->Reload();
    g_pLTClient->CPrint("Reloaded surface attributes file...");
}

void ReloadFXAttributesFn(int argc, char **argv)
{
    g_pFXButeMgr->Reload();
    g_pLTClient->CPrint("Reloaded fx attributes file...");

	// Make sure we re-load the weapons and surface data, it has probably
	// changed...
	ReloadWeaponAttributesFn(0, 0);
	ReloadSurfacesAttributesFn(0, 0);
}

void ExitLevelFn(int argc, char **argv)
{
	if (g_pCheatMgr)
	{
		CParsedMsg cMsg( 0, LTNULL );
		g_pCheatMgr->Process( CHEAT_EXITLEVEL, cMsg );
	}
}

void TeleportFn(int argc, char **argv)
{
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

void ChaseToggleFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_CHASETOGGLE);
	}
}

void CmdFn(int argc, char **argv)
{
	if (argc < 2)
	{
        g_pLTClient->CPrint("Cmd <command>");
		return;
	}

	// Send message to server...
	char buf[256];
	buf[0] = '\0';
	sprintf(buf, "%s", argv[0]);
	for (int i=1; i < argc; i++)
	{
		strcat(buf, " ");
		strcat(buf, "\"");
		strcat(buf, argv[i]);
		strcat(buf, "\"");
	}

    HSTRING hstrCmd = g_pLTClient->CreateString(buf);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CONSOLE_COMMAND);
	cMsg.WriteHString(hstrCmd);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

    g_pLTClient->FreeString(hstrCmd);
}

void TriggerFn(int argc, char **argv)
{
	if (argc < 2)
	{
        g_pLTClient->CPrint("Trigger <objectname> <message>");
		return;
	}

	// Send message to server...

    HSTRING hstrObjName = g_pLTClient->CreateString(argv[0]);
    HSTRING hstrMsg     = g_pLTClient->CreateString(argv[1]);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CONSOLE_TRIGGER);
    cMsg.WriteHString(hstrObjName);
    cMsg.WriteHString(hstrMsg);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

    g_pLTClient->FreeString(hstrObjName);
    g_pLTClient->FreeString(hstrMsg);
}

void ListFn(int argc, char **argv)
{
	if (argc < 1 || !argv)
	{
        g_pLTClient->CPrint("List <classtype>");
		return;
	}

	// Send message to server...

	char buf[100];
	sprintf(buf, "List %s", argv[0]);

    HSTRING hstrMsg = g_pLTClient->CreateString(buf);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CONSOLE_TRIGGER);
    cMsg.WriteHString(LTNULL);
    cMsg.WriteHString(hstrMsg);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

    g_pLTClient->FreeString(hstrMsg);
}


void ExitGame(bool bResponse, uint32 nUserData)
{
	if (bResponse)
	{
        g_pLTClient->Shutdown();
	}
}

void InitSoundFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->InitSound();
	}
}

void MusicFn(int argc, char **argv)
{
	if (!g_pGameClientShell->GetMusic()->IsInitialized())
	{
        g_pLTClient->CPrint("Direct Music hasn't been initialized!");
	}

	if (argc < 2)
	{
        g_pLTClient->CPrint("Music <command>");
        g_pLTClient->CPrint("  Commands: (syntax -> Command <Required> [Optional])");
        g_pLTClient->CPrint(" ");
        g_pLTClient->CPrint("    I  <intensity number to set>  [Enact]  - Change Intensity");
        g_pLTClient->CPrint("    PS <segment name> [Enact]              - Play secondary segment");
        g_pLTClient->CPrint("    PM <motif style> <motif name> [Enact]  - Play motif");
        g_pLTClient->CPrint("    V  <volume adjustment in db>           - Change volume");
        g_pLTClient->CPrint("    SS <segment name> [Enact]              - Stop secondary segment");
        g_pLTClient->CPrint("    SM <motif name> [Enact]                - Stop motif");
        g_pLTClient->CPrint("    S  [Enact]                             - Stop music");
        g_pLTClient->CPrint("    P  <intensity> [Enact]                 - Play music");
        g_pLTClient->CPrint(" ");
        g_pLTClient->CPrint("  Enact Change Values:");
        g_pLTClient->CPrint("    Default     - Will use the default value that is defined in the Control File or by DirectMusic");
        g_pLTClient->CPrint("    Immediately - Will happen immediately");
        g_pLTClient->CPrint("    Beat        - Will happen on the next beat");
        g_pLTClient->CPrint("    Measure     - Will happen on the next measure");
        g_pLTClient->CPrint("    Grid        - Will happen on the next grid");
        g_pLTClient->CPrint("    Segment     - Will happen on the next segment transition");

		return;
	}

	// Build the command...

	char buf[512];
	buf[0] = '\0';
	sprintf(buf, "Music");
	for (int i=0; i < argc; i++)
	{
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}

	g_pGameClientShell->GetMusic()->ProcessMusicMessage(buf);
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
	//AfxSetAllocStop(37803);

	m_pMissionMgr = NULL;
	m_pClientSaveLoadMgr = NULL;

	
	g_pGameClientShell = this;

	m_fFrameTime			= 0.0f;

	g_vWorldWindVel.Init();


    m_bMainWindowMinimized  = false;


    m_bGamePaused           = false;

    m_bTweakingWeapon       = false;
    m_bTweakingWeaponMuzzle = false;
	m_bTweakingWeaponBreachOffset = false;
    m_bAdjust1stPersonCamera = false;


    m_bFlashScreen      = false;
	m_fFlashTime		= 0.0f;
	m_fFlashStart		= 0.0f;
	m_fFlashRampUp		= 0.0f;
	m_fFlashRampDown	= 0.0f;
	m_vFlashColor.Init();

    m_bShowPlayerPos        = false;
	m_bShowCamPosRot		= false;

	for (int i=0; i < kMaxDebugStrings; i++)
	{
		m_pLeftDebugString[i] = NULL;
		m_pRightDebugString[i] = NULL;
	}

    m_bAdjustLightScale     = false;
    m_bAdjustLightAdd       = false;
    m_bAdjustFOV            = false;

    m_bFirstUpdate          = false;
    m_bRestoringGame        = false;


	m_eDifficulty			= GD_NORMAL;
	m_eGameType				= eGameTypeSingle;

    m_hBoundingBox          = NULL;

    m_bMainWindowFocus      = false;
	m_bRendererInit			= false;


	m_fNextSoundReverbTime	= 0.0f;
    m_bUseReverb            = false;
	m_fReverbLevel			= 0.0f;

	m_vLastReverbPos.Init();

	m_pClientTrackedNodeMgr = NULL;

	m_pClientMultiplayerMgr = NULL;

	m_bInWorld = false;

	m_bQuickSave = false;

	m_bServerPaused = false;
	m_eSwitchingWorldsState = eSwitchingWorldsStateNone;


	m_fInitialServerTime = 0.0f;
	m_fInitialLocalTime = 0.0f;
	m_nSpeedCheatCounter = 0;
 
	m_bRunningPerfTest = false;
	m_pPerformanceTest = LTNULL;
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
	for (int i=0; i < kMaxDebugStrings; i++)
	{
		if (m_pLeftDebugString[i])
		{
			g_pFontManager->DestroyPolyString(m_pLeftDebugString[i]);
			m_pLeftDebugString[i] = NULL;
		}

		if (m_pRightDebugString[i])
		{
			g_pFontManager->DestroyPolyString(m_pRightDebugString[i]);
			m_pRightDebugString[i] = NULL;
		}
	}
	
	if (m_hBoundingBox)
	{
        g_pLTClient->RemoveObject(m_hBoundingBox);
	}

	if (g_prcClip)
	{
		debug_delete(g_prcClip);
        g_prcClip = NULL;
	}

	if ( m_pClientTrackedNodeMgr != 0 )
	{
		debug_delete( m_pClientTrackedNodeMgr );
		m_pClientTrackedNodeMgr = 0;
	}

	if( m_pClientMultiplayerMgr )
	{
		debug_delete( m_pClientMultiplayerMgr );
		m_pClientMultiplayerMgr = NULL;
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
	CGameSettings* pSettings = GetInterfaceMgr( )->GetSettings();
	if (!pSettings) return;

	Sound3DProvider *pSound3DProviderList, *pSound3DProvider;
	InitSoundInfo soundInfo;
    uint32 dwProviderID;
	char sz3dSoundProviderName[_MAX_PATH + 1];

    uint32 dwAdvancedOptions = GetInterfaceMgr( )->GetAdvancedOptions();
	if (!(dwAdvancedOptions & AO_SOUND)) return;

    soundInfo.Init();

	// Reload the sounds if there are any...

	soundInfo.m_dwFlags	= INITSOUNDINFOFLAG_RELOADSOUNDS;


	// Get the maximum number of 3d voices to use.
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("Max3DVoices");
	if (hVar)
	{
	    soundInfo.m_nNum3DVoices = (uint8)g_pLTClient->GetVarValueFloat(hVar);
	}
	else
	{
		soundInfo.m_nNum3DVoices = 32;
	}

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
        hVar = g_pLTClient->GetConsoleVar("3DSoundProviderName");
		if ( hVar )
		{
            SAFE_STRCPY( sz3dSoundProviderName, g_pLTClient->GetVarValueString( hVar ));
		}
		else
		{
			// try DX hardware
		    SAFE_STRCPY( sz3dSoundProviderName, "DirectSound Hardware" );
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
				if ( strcmp(( const char * )sz3dSoundProviderName, ( const char * )pSound3DProvider->m_szProvider ) == 0 )
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
			SAFE_STRCPY( soundInfo.m_sz3DProvider, pSound3DProvider->m_szProvider);
		}

        ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->ReleaseSound3DProviderList(pSound3DProviderList);
	}
	else 
	{
		soundInfo.m_sz3DProvider[0] = 0;
	}

	// Get the maximum number of sw voices to use.
    hVar = g_pLTClient->GetConsoleVar("MaxSWVoices");
	if (hVar)
	{
        soundInfo.m_nNumSWVoices = (uint8)g_pLTClient->GetVarValueFloat(hVar);
	}
	else
	{
		soundInfo.m_nNumSWVoices = 32;
	}

	soundInfo.m_nSampleRate		= g_nSampleRate;
	soundInfo.m_nBitsPerSample	= 16;
	soundInfo.m_nVolume			= (unsigned short)pSettings->SoundVolume();

	if ( !pSettings->Sound16Bit( ) )
	{
		soundInfo.m_dwFlags |= INITSOUNDINFOFLAG_CONVERT16TO8;
	}

	soundInfo.m_fDistanceFactor = 1.0f / 64.0f;
	soundInfo.m_fDopplerFactor = 1.0f;

	// Go initialize the sounds...

    m_bUseReverb = false;
    if (((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->InitSound(&soundInfo) == LT_OK)
	{
		if (soundInfo.m_dwResults & INITSOUNDINFORESULTS_REVERB)
		{
            m_bUseReverb = true;
		}

        hVar = g_pLTClient->GetConsoleVar("ReverbLevel");
		if (hVar)
		{
            m_fReverbLevel = g_pLTClient->GetVarValueFloat(hVar);
		}
		else
		{
			m_fReverbLevel = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_DEFAULTLEVEL);
		}

		ReverbProperties reverbProperties;
		reverbProperties.m_dwParams = REVERBPARAM_VOLUME;
		reverbProperties.m_fVolume = m_fReverbLevel;
        ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetReverbProperties(&reverbProperties);
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
//	ROUTINE:	CGameClientShell::CSPrint
//
//	PURPOSE:	Displays a line of text on the client
//
// ----------------------------------------------------------------------- //

void CGameClientShell::CSPrint(char* msg, ...)
{
	// parse the message

	char pMsg[256];
	va_list marker;
	va_start (marker, msg);
	int nSuccess = vsprintf (pMsg, msg, marker);
	va_end (marker);

	if (nSuccess < 0) return;

	// now display the message
	g_pChatMsgs->AddMessage(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnEngineInitialized
//
//	PURPOSE:	Called after engine is fully initialized
//				Handle object initialization here
//
// ----------------------------------------------------------------------- //

uint32 CGameClientShell::OnEngineInitialized(RMode *pMode, LTGUID *pAppGuid)
{
	InitClientShell();

	//CWinUtil::DebugBreak();

	*pAppGuid = GAMEGUID;


    char strTimeDiff[64];
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

	g_vtUseSoundFilters.Init(g_pLTClient, "SoundFilters", NULL, 0.0f);

	g_vtSpecial.Init(g_pLTClient, "ShowSpecial", NULL, 0.0f);

	g_vtMakeCubicEnvMapSize.Init(g_pLTClient, "MakeCubicEnvMapSize", NULL, 256.0f);
	g_vtMakeCubicEnvMapName.Init(g_pLTClient, "MakeCubicEnvMapName", "CubicEnvMap", 0.0f);
	g_vtApplyWorldOffset.Init(g_pLTClient, "ApplyWorldOffset", NULL, 1.0f);

	g_vtPTestMinFPS.Init(g_pLTClient, "PerformanceMinTestFPS", NULL, 25.0f);
	g_vtPTestMaxFPS.Init(g_pLTClient, "PerformanceMaxTestFPS", NULL, 40.0f);

    HCONSOLEVAR hIsSet = g_pLTClient->GetConsoleVar("UpdateRateInitted");
    if (!hIsSet || g_pLTClient->GetVarValueFloat(hIsSet) != 1.0f)
	{
		// Initialize the update rate.
        g_pLTClient->RunConsoleString("+UpdateRateInitted 1");
        g_pLTClient->RunConsoleString("+UpdateRate 60");
	}

	m_cheatMgr.Init();
	m_LightScaleMgr.Init();

	// would be nice to move these to a centralized registry function

    g_pLTClient->RegisterConsoleProgram("Cmd", CmdFn);
    g_pLTClient->RegisterConsoleProgram("Trigger", TriggerFn);
    g_pLTClient->RegisterConsoleProgram("List", ListFn);
    g_pLTClient->RegisterConsoleProgram("FragSelf", FragSelfFn);
    g_pLTClient->RegisterConsoleProgram("ReloadWeapons", ReloadWeaponAttributesFn);
    g_pLTClient->RegisterConsoleProgram("ReloadSurfaces", ReloadSurfacesAttributesFn);
    g_pLTClient->RegisterConsoleProgram("ReloadFX", ReloadFXAttributesFn);
    g_pLTClient->RegisterConsoleProgram("InitSound", InitSoundFn);
    g_pLTClient->RegisterConsoleProgram("ExitLevel", ExitLevelFn);
    g_pLTClient->RegisterConsoleProgram("Teleport", TeleportFn);
    g_pLTClient->RegisterConsoleProgram("ChaseToggle", ChaseToggleFn);
    g_pLTClient->RegisterConsoleProgram("Music", MusicFn);
    g_pLTClient->RegisterConsoleProgram("Stimulus", StimulusFn);
    g_pLTClient->RegisterConsoleProgram("RenderStimulus", RenderStimulusFn);
    g_pLTClient->RegisterConsoleProgram("AddGoal", AddGoalFn);
    g_pLTClient->RegisterConsoleProgram("RemoveGoal", RemoveGoalFn);
    g_pLTClient->RegisterConsoleProgram("Alpha", ObjectAlphaFn);
	g_pLTClient->RegisterConsoleProgram("ClientFX", ClientFXFn);

    g_pLTClient->SetModelHook((ModelHookFn)DefaultModelHook, this);

	// Make sure the save directory exists...
	if (!CWinUtil::DirExist("Save"))
	{
		CWinUtil::CreateDir("Save");
	}

	// Add to NumRuns count...

	float nNumRuns = 0.0f;
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("NumRuns");
	if (hVar)
	{
        nNumRuns = g_pLTClient->GetVarValueFloat(hVar);
	}
	nNumRuns++;

	char strConsole[64];
	sprintf (strConsole, "+NumRuns %f", nNumRuns);
    g_pLTClient->RunConsoleString(strConsole);

    bool bNetworkGameStarted = false;

	//setup our render mode to only allow hardware TnL rendering modes at 32 bpp
	RMode rMode;

	rMode.m_Width		= pMode->m_Width;
	rMode.m_Height		= pMode->m_Height;
	rMode.m_BitDepth	= 32;
	rMode.m_bHWTnL		= true;
	rMode.m_pNext		= NULL;

	sprintf(rMode.m_InternalName, "%s", pMode->m_InternalName);
	sprintf(rMode.m_Description, "%s", pMode->m_Description);

	// Initialize the renderer
    LTRESULT hResult = g_pLTClient->SetRenderMode(&rMode);
	if (hResult != LT_OK)
	{
		// If an error occurred, try 640x480x32...
		rMode.m_Width		= 640;
		rMode.m_Height		= 480;

		g_pLTClient->DebugOut("%s Error: Couldn't set render mode!\n", GAME_NAME);
        g_pLTClient->DebugOut("Setting render mode to 640x480x32...\n");

        if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
		{
			//alright, both of the above failed, so now we need to inform the user that we are unable
			//to create a HWTnL device. This can be caused by them not having a TnL device, or by
			//them not having DX8.1. We will let them choose if they want to exit or attempt to 
			//create a SWTnL device
			char pszNoTnLWarningBuff[512];
			FormatString(IDS_APP_UNABLE_TO_CREATE_HW_TNL_DEVICE, pszNoTnLWarningBuff, sizeof(pszNoTnLWarningBuff), GAME_NAME);

			if(MessageBox(g_hMainWnd, pszNoTnLWarningBuff, GAME_NAME, MB_OKCANCEL) == IDCANCEL)
			{
				//alright, they canceled, so we need to just exit
				g_pLTClient->DebugOut("User chose to not create software TnL device. Exiting.");
				g_pLTClient->Shutdown();
				return LT_ERROR;
			}

			g_pLTClient->DebugOut("Attempting to create software TnL device.");

			//they want to continue, so create a software version
			rMode.m_Width	= pMode->m_Width;
			rMode.m_Height	= pMode->m_Height;
			rMode.m_bHWTnL	= false;

			if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
			{
				g_pLTClient->DebugOut("Failed to create default resoltution software TnL, falling back to 640x480");

				rMode.m_Width	= 640;
				rMode.m_Height	= 480;

				if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
				{
					char pszErrorBuffer[256];
					FormatString(IDS_APP_SHUTDOWN_1, pszErrorBuffer, sizeof(pszErrorBuffer), GAME_NAME);

					// Okay, that didn't work, looks like we're stuck with software...
					g_pLTClient->DebugOut(pszErrorBuffer);
					g_pLTClient->ShutdownWithMessage(pszErrorBuffer);
					return LT_ERROR;
				}
			}
			WriteConsoleInt("HWTnLDisabled",1);
		}
	}

	// Init the ClientFX Database
	if(!CClientFXDB::GetSingleton().Init(g_pLTClient))
	{
		g_pLTClient->ShutdownWithMessage( "Could not init ClientFXDB!" );
		return LT_ERROR;
	}

	// Init the ClientFX mgr... (this must become before most other classes)
	if( !m_ClientFXMgr.Init( g_pLTClient ) )
	{
		// Make sure ClientFX.fxd is built and in the game dir
		g_pLTClient->ShutdownWithMessage( "Could not init ClientFXMgr!" );
		return LT_ERROR;
	}

	// Init the DamageFX mgr...
	if( !m_DamageFXMgr.Init() )
	{
		g_pLTClient->ShutdownWithMessage( "Could not init DamageFXMgr!" );
		return false;
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

	// Interface stuff...
	if (!GetInterfaceMgr( ) || !GetInterfaceMgr( )->Init())
	{
		// Don't call ShutdownWithMessage since InterfaceMgr will have called
		// that, so calling it here will overwrite the message...
 		return LT_ERROR;
	}

	// Player stuff...
	if (!GetPlayerMgr() || !GetPlayerMgr()->Init())
	{
		// Don't call ShutdownWithMessage since PlayerMgr will have called
		// that, so calling it here will overwrite the message...
		return LT_ERROR;
	}

	//We need to make sure to setup the camera for the FX Mgr
	m_ClientFXMgr.SetCamera(GetPlayerMgr()->GetCamera());

	// Tracked Node stuff...
	if( m_pClientTrackedNodeMgr == 0 )
	{
		m_pClientTrackedNodeMgr = debug_new( CClientTrackedNodeMgr );
		ASSERT( 0 != m_pClientTrackedNodeMgr );
	}

	// Setup the music stuff...(before we setup the interface!)

    uint32 dwAdvancedOptions = GetInterfaceMgr( )->GetAdvancedOptions();

	if (!m_Music.IsInitialized() && (dwAdvancedOptions & AO_MUSIC))
	{
        m_Music.Init(g_pLTClient);
	}



	// Initialize the global physics states...

	g_normalPhysicsState.m_vGravityAccel.Init(0.0f, -1000.0f, 0.0f);
	g_normalPhysicsState.m_fVelocityDampen = 0.5f;
	g_waterPhysicsState.m_vGravityAccel.Init(0.0f, -500.0f, 0.0f);
	g_waterPhysicsState.m_fVelocityDampen = 0.25f;


	m_pClientMultiplayerMgr = debug_new( ClientMultiplayerMgr );

	// Init the special fx mgr...
	if (!m_sfxMgr.Init(g_pLTClient))
	{
		g_pLTClient->ShutdownWithMessage("Could not initialize SFXMgr!");
		return LT_ERROR;
	}

	m_bQuickSave = false;

	// Get the name of the mod we want to play.  If no mod specified then we consider that the 'Retail' mod...
	// Set this when the game initializes and use g_pClientMultiplayerMgr->GetModName() in case the console var changes.

	HCONSOLEVAR	hModVar = g_pLTClient->GetConsoleVar( "Mod" );
	if( hModVar )
	{
        g_pClientMultiplayerMgr->SetModName( g_pLTClient->GetVarValueString( hModVar) );
	}
	else
	{
		g_pClientMultiplayerMgr->SetModName( LoadTempString( IDS_RETAIL ) );
	}	

	// Process command line.

	if (hVar = g_pLTClient->GetConsoleVar("join")) // looking for join [ip] format
	{
		g_pInterfaceMgr->SetCommandLineJoin( true );

		// Change to the splash screen.  We'll start the game after that.
		GetInterfaceMgr( )->ChangeState(GS_SPLASHSCREEN);
	}
	else if (hVar = g_pLTClient->GetConsoleVar("host")) // looking for host [1] format
	{
		int nHost = atoi(g_pLTClient->GetVarValueString( hVar ));
		
		// Use the settings from the current profile to determine what the game will be...

		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile( );
		if( nHost && pProfile )
		{
			bool bOk = true;

			// Initialize the mission mgr with the appropriate file based on game type...

			std::string sMissionFile;
			switch( pProfile->m_ServerGameOptions.m_eGameType )
			{
				case eGameTypeCooperative:
				{
					sMissionFile = MISSION_COOP_FILE;
				}
				break;

				case eGameTypeDeathmatch:
				case eGameTypeTeamDeathmatch:
				{
					sMissionFile = MISSION_DM_FILE;
				}
				break;

				case eGameTypeDoomsDay:
				{
					sMissionFile = MISSION_DD_FILE;
				}
				break;

				case eGameTypeSingle:
				default:
				{
					sMissionFile = MISSION_DEFAULT_FILE;
					bOk = false;
				}
				break;
			}

			if( !g_pMissionButeMgr->Init( sMissionFile.c_str() ))
			{
				g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", sMissionFile.c_str() );
				return LT_ERROR;
			}

			// Start the server..

			bOk = bOk && g_pClientMultiplayerMgr->SetupServerHost( pProfile->m_ServerGameOptions.m_nPort , pProfile->m_ServerGameOptions.m_bLANOnly );
			bOk = bOk && g_pMissionMgr->StartGameNew();
			
			if( !bOk )
			{
				// drop them into the host menu
				GetInterfaceMgr( )->LoadFailed();
				GetInterfaceMgr( )->SwitchToScreen(SCREEN_ID_HOST);

				MBCreate mb;
				GetInterfaceMgr( )->ShowMessageBox(IDS_NOLOADLEVEL,&mb);
			}
		}
		else
		{
			// Just treat like normal game...
			
			GetInterfaceMgr( )->ChangeState(GS_SPLASHSCREEN);
		}
	}
	else if (hVar = g_pLTClient->GetConsoleVar("runworld"))
	{
		const char* pMap = g_pLTClient->GetVarValueString(hVar);
		if (!pMap)
		{
			g_pLTClient->ShutdownWithMessage("No world string found after runworld tolken, SUCKA'!");
			return LT_ERROR;
		}

		// Initialize to the sp mission bute.
		if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
			return LT_ERROR;
  		}

		g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );
		bool bOk = g_pClientMultiplayerMgr->SetupServerSinglePlayer( );
		bOk = bOk && g_pMissionMgr->StartGameFromLevel(pMap);
		if( !bOk )
		{
			g_pLTClient->ShutdownWithMessage(LoadTempString(IDS_NOLOADLEVEL));
			return LT_ERROR;
		}
	}
	else if (GetConsoleInt("skiptitle",0))
	{
		GetInterfaceMgr( )->SwitchToScreen(SCREEN_ID_MAIN);
	}
	else
	{
		GetInterfaceMgr( )->ChangeState(GS_SPLASHSCREEN);
	}

#ifdef _TO2DEMO
	WriteConsoleInt( "ConsoleEnable", 0 );
#endif // _TO2DEMO


	// Determine how long it took to initialize the game...
	sprintf(strTimeDiff, "Game initialized in %f seconds.\n", CWinUtil::GetTime() - fStartTime);
	CWinUtil::DebugOut(strTimeDiff);

	m_pPerformanceTest = debug_new(CPerformanceTest);

	// Intialize the scmd handler.
	static ScmdConsoleDriver_CShell scmdConsoleDriver_CShell;
	if( !ScmdConsole::Instance( ).Init( scmdConsoleDriver_CShell ))
	{
		return LT_ERROR;
	}

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
    UnhookWindow();

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

	//Make sure that the FX Mgr isn't still holding on to our camera
	m_ClientFXMgr.SetCamera(NULL);

	GetPlayerMgr()->Term();

	m_Music.Term();
	m_LightScaleMgr.Term();

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
	switch(dwEventID)
	{
		// Client disconnected from server.  dwParam will
		// be a error flag found in de_codes.h.

		case LTEVENT_DISCONNECT :
		{
			SetWorldNotLoaded();
			m_bServerPaused = false;
			m_eSwitchingWorldsState = eSwitchingWorldsStateNone;

		} break;

        case LTEVENT_LOSTFOCUS:
		{
			m_bMainWindowFocus = FALSE;
			
		}
		break;

		case LTEVENT_GAINEDFOCUS:
		{
			m_bMainWindowFocus = TRUE;
		}
		break;

        case LTEVENT_RENDERTERM:
		{
			m_bMainWindowFocus = FALSE;
			m_bRendererInit = false;

			// Let the ClientFx mgr know the renderer is shutting down
			m_ClientFXMgr.OnRendererShutdown();
		}
		break;

		case LTEVENT_RENDERINIT:
		{
			m_bMainWindowFocus = TRUE;
			m_bRendererInit = true;

			// Clip the cursor if we're NOT in a window...

            HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("Windowed");
			BOOL bClip = TRUE;
			if (hVar)
			{
                float fVal = g_pLTClient->GetVarValueFloat(hVar);
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

			//setup the glow table
			char pszRS[2][_MAX_PATH];
			g_pClientButeMgr->GetDefaultGlowRS(pszRS[0], _MAX_PATH);
			g_pLTClient->SetGlowDefaultRenderStyle(pszRS[0]);

			g_pClientButeMgr->GetNoGlowRS(pszRS[0], _MAX_PATH);
			g_pLTClient->SetNoGlowRenderStyle(pszRS[0]);

			for(uint32 nCurrMap = 0; nCurrMap < g_pClientButeMgr->GetNumGlowMappings(); nCurrMap++)
			{
				g_pClientButeMgr->GetGlowMappingRS(nCurrMap, pszRS[0], _MAX_PATH, pszRS[1], _MAX_PATH);
				g_pLTClient->AddGlowRenderStyleMapping(pszRS[0], pszRS[1]);
			}
		}
		break;
	}

	if (GetInterfaceMgr( ))
		GetInterfaceMgr( )->OnEvent(dwEventID, dwParam);
	if( g_pClientMultiplayerMgr )
		g_pClientMultiplayerMgr->OnEvent(dwEventID, dwParam);
}


LTRESULT CGameClientShell::OnObjectMove(HOBJECT hObj, bool bTeleport, LTVector *pPos)
{
	return g_pPlayerMgr->GetMoveMgr()->OnObjectMove(hObj, bTeleport, pPos);
}


LTRESULT CGameClientShell::OnObjectRotate(HOBJECT hObj, bool bTeleport, LTRotation *pNewRot)
{
	m_sfxMgr.OnObjectRotate( hObj, bTeleport, pNewRot );
	return g_pPlayerMgr->GetMoveMgr()->OnObjectRotate(hObj, bTeleport, pNewRot);
}

LTRESULT CGameClientShell::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag)
{
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
	// Reset our speed hack.
	g_nStartTicks = 0;

	// In world now.
	m_bInWorld = true;

	// Make sure we enable all object render groups so we don't have any weird state
	// left over from previous levels
	g_pLTClient->SetAllObjectRenderGroupEnabled();

	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->ResumeSounds();

	// Set the stair height gotten from the server...
	
	float fStairHeight = DEFAULT_STAIRSTEP_HEIGHT;
	g_pLTClient->GetSConValueFloat( STAIR_STEP_HEIGHT_CVAR, fStairHeight );
	g_pPhysicsLT->SetStairHeight( fStairHeight );

	m_bFirstUpdate = true;

	m_LightScaleMgr.Init();
	GetInterfaceMgr( )->AddToClearScreenCount();

    g_pLTClient->ClearInput();

	GetInterfaceMgr( )->OnEnterWorld(m_bRestoringGame);
	GetPlayerMgr()->OnEnterWorld();

    m_bRestoringGame = false;

	m_vLastReverbPos.Init();

	MirrorSConVar( "RespawnWaitTime", "RespawnWaitTime" );
	MirrorSConVar( "RespawnMultiWaitTime", "RespawnMultiWaitTime" );
	MirrorSConVar( "RespawnDoomsdayWaitTime", "RespawnDoomsdayWaitTime" );
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
    ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->PauseSounds();

	// Not in world any more.
    m_bInWorld = false;

	m_DamageFXMgr.Clear();					// Remove all the sfx
	m_sfxMgr.RemoveAll();					// Remove all the sfx
	m_ClientFXMgr.ShutdownAllFX();			// Stop all the client FX

	GetInterfaceMgr( )->OnExitWorld();

	if( GetPlayerMgr() )
		GetPlayerMgr()->OnExitWorld();
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
	// Allow multiplayermgr to update.
	g_pClientMultiplayerMgr->Update( );

	// Set up the time for this frame...
    m_fFrameTime = g_pLTClient->GetFrameTime();
	if (m_fFrameTime > MAX_FRAME_DELTA)
	{
		m_fFrameTime = MAX_FRAME_DELTA;
	}

	// Update tint if applicable (always do this to make sure tinting
	// gets finished)...

	UpdateScreenFlash();
	m_ScreenTintMgr.Update();



	// Update client-side physics structs...

	if (IsServerPaused())
	{
		SetPhysicsStateTimeStep(&g_normalPhysicsState, 0.0f);
		SetPhysicsStateTimeStep(&g_waterPhysicsState, 0.0f);
	}
	else
	{
		SetPhysicsStateTimeStep(&g_normalPhysicsState, m_fFrameTime);
		SetPhysicsStateTimeStep(&g_waterPhysicsState, m_fFrameTime);
	}

#ifdef _TRON_E3_DEMO
	// Check to see if they've left the game idle for too long
	// If so, take them to the demo screen state
	s_fDemoTime += g_pLTClient->GetFrameTime();
	if(s_fDemoTime > TRON_E3_DEMO_IDLE_TIME)
	{
		GetInterfaceMgr()->ShowDemoScreens(LTFALSE);
	}
#endif

	// Update the interface (don't do anything if the interface mgr
	// handles the update...)

	if (GetInterfaceMgr( )->Update())
	{
		return;
	}

	GetPlayerMgr()->Update();

	// At this point we only want to proceed if the player is in the world...
	if (GetPlayerMgr()->IsPlayerInWorld() && g_pInterfaceMgr->GetGameState() != GS_UNDEFINED)
	{
		UpdatePlaying();
	}
	else
	{
		// we should not be here, since we think we should rendering the world, but we are not
		bool bBlackScreen = true;
	}

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
	GetClientFXMgr()->SetGoreEnabled(bGore);
	GetInterfaceMgr()->GetInterfaceFXMgr().SetGoreEnabled(bGore);
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

	if (m_bFirstUpdate)
	{
		FirstUpdate();
	}

	if(IsMultiplayerGame() && g_pPlayerMgr->IsPlayerInWorld())
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
					if( (( uint32 ) abs( (long)(nDeltaTimeB - nDeltaClientTime) ) > g_nTolerance ) || 
						(( uint32 ) abs( (long)(nDeltaTicks - nDeltaClientTime) ) > g_nTolerance ) ||
						(( uint32 ) abs( (long)(nDeltaTimeB - nDeltaTicks) ) > g_nTolerance ) )
					{
						g_pLTClient->CPrint( "Speedhack kick" );
						g_pLTClient->CPrint( "nDeltaTimeB %d", nDeltaTimeB );
						g_pLTClient->CPrint( "nDeltaTicks %d", nDeltaTicks );
						g_pLTClient->CPrint( "nDeltaClientTime %d", nDeltaClientTime );
					
						// You hAxOr!

						// Disconnect from the server.
						if(g_pLTClient->IsConnected())
						{
							g_pLTClient->Disconnect();
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

	g_pPlayerMgr->UpdatePlaying();


	// Update any debugging information...

	UpdateDebugInfo();

	// Update cheats...(if a cheat is in effect, just return)...
	if( UpdateCheats( ))
		return;

	// Update any overlays...

	GetInterfaceMgr( )->UpdateOverlays();


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
	// See if we got a oncommandon for quicksave.
	if( m_bQuickSave )
	{
		m_bQuickSave = false;
		if( !g_pClientSaveLoadMgr->QuickSave() )
		{
			g_pLTClient->CPrint( "ERROR - Quick Save Failed!" );
		}
	}

	GetPlayerMgr()->PostUpdate();

	// Conditions where we don't want to flip...

	if (GetPlayerMgr()->GetPlayerState() == PS_UNKNOWN && g_pGameClientShell->IsWorldLoaded() && GetInterfaceMgr( )->GetGameState() != GS_SCREEN)
	{
		return;
	}


	GetInterfaceMgr( )->PostUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::GetDynamicSoundFilter()
//
//	PURPOSE:	Get the dynamic sound filter
//
// ----------------------------------------------------------------------- //

SOUNDFILTER* CGameClientShell::GetDynamicSoundFilter()
{
	// See if we have a current container filter override...

	SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter(g_pPlayerMgr->GetSoundFilter());
	if (!pFilter) return NULL;


	if (!g_pSoundFilterMgr->IsDynamic(pFilter))
	{
		// Found it...
		return pFilter;
	}
	else  // Calculate the filter based on the listener...
	{
		// For now just return global default (from WorldProperties)

		return g_pSoundFilterMgr->GetFilter(g_pPlayerMgr->GetGlobalSoundFilter());
	}
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
	if( IsMultiplayerGame( ))
		return false;
#endif

	if (m_bAdjustLightScale)
	{
		AdjustLightScale();
	}

	if (m_bAdjustLightAdd)
	{
		AdjustLightAdd();
	}

	if (m_bAdjustFOV)
	{
		AdjustFOV();
	}

	if (m_bAdjust1stPersonCamera)
	{
		Adjust1stPersonCamera();
	}

	// If in spectator mode, just do the camera stuff...

	if (g_pPlayerMgr->IsSpectatorMode() || g_pPlayerMgr->IsPlayerDead())
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
	else if (m_bTweakingWeaponMuzzle)
	{
		UpdateWeaponMuzzlePosition();
		g_pPlayerMgr->UpdateCamera();
		RenderCamera();
        return true;
	}
	else if (m_bTweakingWeaponBreachOffset)
	{
		UpdateWeaponBreachOffset();
		g_pPlayerMgr->UpdateCamera();
		RenderCamera();
        return true;
	}

    return false;
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateScreenFlash
//
//	PURPOSE:	Update the screen flash
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateScreenFlash()
{
	// [KLS 2/27/02] - Removed ALL screen flashing...
	return;

	if (!m_bFlashScreen) return;

    LTVector vLightAdd;
	VEC_SET(vLightAdd, 0.0f, 0.0f, 0.0f);

    float fTime  = g_pLTClient->GetTime();
	if ((m_fFlashRampUp > 0.0f) && (fTime < m_fFlashStart + m_fFlashRampUp))
	{
        float fDelta = (fTime - m_fFlashStart);
		vLightAdd.x = fDelta * (m_vFlashColor.x) / m_fFlashRampUp;
		vLightAdd.y = fDelta * (m_vFlashColor.y) / m_fFlashRampUp;
		vLightAdd.z = fDelta * (m_vFlashColor.z) / m_fFlashRampUp;
	}
	else if (fTime < m_fFlashStart + m_fFlashRampUp + m_fFlashTime)
	{
		VEC_COPY(vLightAdd, m_vFlashColor);
	}
	else if ((m_fFlashRampDown > 0.0f) && (fTime < m_fFlashStart + m_fFlashRampUp + m_fFlashTime + m_fFlashRampDown))
	{
        float fDelta = (fTime - (m_fFlashStart + m_fFlashRampUp + m_fFlashTime));

		vLightAdd.x = m_vFlashColor.x - (fDelta * (m_vFlashColor.x) / m_fFlashRampUp);
		vLightAdd.y = m_vFlashColor.y - (fDelta * (m_vFlashColor.y) / m_fFlashRampUp);
		vLightAdd.z = m_vFlashColor.z - (fDelta * (m_vFlashColor.z) / m_fFlashRampUp);
	}
	else
	{
        m_bFlashScreen = false;
	}

	// Make sure values are in range...

	vLightAdd.x = (vLightAdd.x < 0.0f ? 0.0f : (vLightAdd.x > 1.0f ? 1.0f : vLightAdd.x));
	vLightAdd.y = (vLightAdd.y < 0.0f ? 0.0f : (vLightAdd.y > 1.0f ? 1.0f : vLightAdd.y));
	vLightAdd.z = (vLightAdd.z < 0.0f ? 0.0f : (vLightAdd.z > 1.0f ? 1.0f : vLightAdd.z));

	m_ScreenTintMgr.Set(TINT_SCREEN_FLASH,&vLightAdd);
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
	if (m_bFlashScreen)
	{
        m_bFlashScreen = false;
	}

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

static void QuickLoadCallBack(LTBOOL bReturn, void *pData)
{
	if (bReturn)
	{
		g_pMissionMgr->StartGameFromQuickSave( );
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

#ifdef _TRON_E3_DEMO
	// Reset the demo timer
	s_fDemoTime = 0.0f;
#endif

	// Make sure we're in the world...

	if (!GetPlayerMgr()->IsPlayerInWorld()) return;


	// Let the interface handle the command first...

	if (IsMultiplayerGame() || !GetPlayerMgr()->IsPlayerDead())
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
#ifdef _TRON_E3_DEMO
	// Reset the demo timer
	s_fDemoTime = 0.0f;
#endif

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
//	ROUTINE:	CGameClientShell::OnKeyDown(int key, int rep)
//
//	PURPOSE:	Handle key down notification
//				Try to avoid using OnKeyDown and OnKeyUp as they
//				are not portable functions
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnKeyDown(int key, int rep)
{
	// [RP] - 8/03/02: WinXP likes to add a second OnKeyDown() message with an invalid key of 255
	//		  for certain single key presses.  Just ignore invalid key codes 255 and larger.

	if( key >= 0xFF )
		return;

	// The engine handles the VK_F8 key for screenshots.
	if( key == VK_F8 )
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

	if (key == VK_TOGGLE_SCREENSHOTMODE)
	{
#ifndef _FINAL
		g_bScreenShotMode = !g_bScreenShotMode;

		bool bDrawHUD = !g_bScreenShotMode; // && GetPlayerMgr()->IsSpectatorMode());
		if (bDrawHUD)
		{
			GetInterfaceMgr( )->SetHUDRenderLevel(kHUDRenderFull);
		}
		else
		{
			GetInterfaceMgr( )->SetHUDRenderLevel(kHUDRenderNone);
		}

        g_pLTClient->CPrint("Screen shot mode: %s", g_bScreenShotMode ? "ON" : "OFF");
#endif
		return;
	}

	if (key == VK_TOGGLE_SPECTATOR_MODE)
	{
#ifndef _FINAL
		char *pCheat = "mpclip";
		g_pCheatMgr->Check(CParsedMsg( 1, &pCheat ));
#endif
		return;
	}

	if (key == VK_TOGGLE_GHOST_MODE)
	{
#ifndef _FINAL
		char *pCheat = "mppoltergeist";
		g_pCheatMgr->Check(CParsedMsg( 1, &pCheat ));
#endif
		return;
	}

#ifndef _DEMO
#ifndef _TRON_E3_DEMO
#ifndef _FINAL
	if (key == VK_MULTIPLY)
	{
		static bool g_bCycleOn = false;
		g_bCycleOn = !g_bCycleOn;
		if(g_bCycleOn)
		{
			g_pLTClient->RunConsoleString("cmd msg player StartLightCycle");
		}
		else
		{
			g_pLTClient->RunConsoleString("cmd msg player EndLightCycle");
		}
	}
#endif
#endif
#endif

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
		g_pLTClient->MakeCubicEnvMap(GetPlayerMgr()->GetCamera(), (uint32)(g_vtMakeCubicEnvMapSize.GetFloat(256.0f) + 0.5f), g_vtMakeCubicEnvMapName.GetStr("CubicEnvMap"));
#endif
		return;
	}

	// Allow quickload from anywhere, anytime.
	// jrg - 8/31/02 - well, almost anywhere and almost anytime, except...
	GameState eGameState = g_pInterfaceMgr->GetGameState();
	if	( key == VK_F9 && 
			(	GS_PLAYING == eGameState ||
				GS_SCREEN == eGameState ||
				GS_POPUP == eGameState ||
				GS_MENU == eGameState
			)	
		)
	{
		//handle special case of postload screen
		if (GS_SCREEN == eGameState && SCREEN_ID_POSTLOAD == g_pInterfaceMgr->GetScreenMgr()->GetCurrentScreenID() )
			return;


		// Check if we can do a load.
		if( g_pClientSaveLoadMgr->CanLoadGame( ) && g_pClientSaveLoadMgr->QuickSaveExists( ))
		{
			// If player is in the world, then give them confirmation.
			if( g_pPlayerMgr->IsPlayerInWorld() && !g_pPlayerMgr->IsPlayerDead( ))
			{
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = QuickLoadCallBack;
				mb.pData = this;
				g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
			}
			else
			{
				//if mission bute mgr is not initialized
				if (!g_pMissionButeMgr->GetAttributeFile( ) || !strlen(g_pMissionButeMgr->GetAttributeFile( )))
				{
					// Initialize to the sp mission bute.
					if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
					{
						g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
						return;
  					}
				}

				g_pMissionMgr->StartGameFromQuickSave( );
			}
		}
		return;
	}
	else if( key == VK_F6 )
	{
		// Only allow quicksave if we're in a world and not a remote client.
		if( GetPlayerMgr()->IsPlayerInWorld() && !g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
		{
			if( g_pClientSaveLoadMgr->CanSaveGame() && eGameState == GS_PLAYING )
			{
				m_bQuickSave = true;
			}
			else
			{
				// Can't quicksave now...
				g_pGameClientShell->CSPrint(LoadTempString(IDS_CANTQUICKSAVE));
				g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\Nosave.wav");
			}
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
	// The engine handles the VK_F8 key for screenshots.
	if( key == VK_F8 )
		return;

	if (IsRunningPerformanceTest())
		return;

	if (GetInterfaceMgr( )->OnKeyUp(key))
		return;
	if (GetPlayerMgr()->OnKeyUp(key)) 
		return;
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
	// Check inputs.
	if( !pMsg )
		return;

	// Read the message ID
	uint8 messageID = pMsg->Readuint8();

	// Act like the message ID wasn't in the message
	CLTMsgRef_Read cSubMsg(pMsg->SubMsg(pMsg->Tell()));

	// Let interface handle message first...

	if (GetInterfaceMgr( )->OnMessage(messageID, cSubMsg)) return;
	if (GetPlayerMgr()->OnMessage(messageID, cSubMsg)) return;

	if( g_pClientMultiplayerMgr->OnMessage(messageID, cSubMsg ))
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
		case MID_SFX_MESSAGE:		HandleMsgSFXMessage			(cSubMsg);	break;
		case MID_MUSIC:				HandleMsgMusic				(cSubMsg);	break;
		case MID_PLAYER_LOADCLIENT:	HandleMsgPlayerLoadClient	(cSubMsg);	break;
		case MID_SERVER_ERROR:		HandleMsgServerError		(cSubMsg);	break;
		case MID_PROJECTILE:		HandleMsgProjectile			(messageID, cSubMsg); break;
		case MID_GAME_PAUSE:		HandleMsgPauseGame			(cSubMsg); break;
		case MID_SWITCHINGWORLDSSTATE:	HandleMsgSwitchingWorldState(cSubMsg); break;
		case MID_MULTIPLAYER_OPTIONS:	HandleMsgMultiplayerOptions(cSubMsg); break;
		
		case STC_BPRINT:			HandleMsgBPrint				(cSubMsg);	break;
		default: 					HandleMsgDefault			(cSubMsg);	break;
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
//	ROUTINE:	CGameClientShell::HandleMsgMusic()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgMusic (ILTMessage_Read *pMsg)
{
	if (m_Music.IsInitialized())
	{
		m_Music.ProcessMusicMessage(pMsg);
	}
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
	CLTMsgRef_Read pSaveMessage = pMsg->ReadMessage();
	UnpackClientSaveMsg(pSaveMessage);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgServerError()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgServerError (ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    uint8 nError = pMsg->Readuint8();
	switch (nError)
	{
		case SERROR_SAVEGAME :
		{
			//DoMessageBox(IDS_SAVEGAMEFAILED, TH_ALIGN_CENTER);
		}
		break;

		case SERROR_LOADGAME :
		{
			//DoMessageBox(IDS_NOLOADLEVEL, TH_ALIGN_CENTER);

			GetInterfaceMgr( )->ChangeState(GS_SCREEN);
		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgBPrint()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgBPrint (ILTMessage_Read *pMsg)
{
	char msg[50];
	pMsg->ReadString(msg, sizeof(msg));
	CSPrint(msg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgProjectile()
//
//	PURPOSE:	Handle messages from the server regarding projectiles
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgProjectile( uint8 messageID, ILTMessage_Read *pMsg )
{
	// pass the message to the client weapon manager
	ASSERT( 0 != g_pPlayerMgr );
	CClientWeaponMgr *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	pClientWeaponMgr->OnMessage( messageID, pMsg );
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
	m_bServerPaused = pMsg->Readbool( );
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
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
		return;

	

	uint8 nRunSpeed = pMsg->Readuint8( );
	uint8 nScoreLimit = pMsg->Readuint8( );
	uint8 nTimeLimit = pMsg->Readuint8( );
	uint8 nRounds = pMsg->Readuint8();
	bool bFriendlyFire = pMsg->Readbool( );
	uint8 nDifficulty = pMsg->Readuint8( );
	float fPlayerDiffFactor = pMsg->Readfloat( );

	switch (m_eGameType)
	{
	case eGameTypeCooperative:
		pProfile->m_ServerGameOptions.GetCoop().m_bFriendlyFire = bFriendlyFire;
		pProfile->m_ServerGameOptions.GetCoop().m_nDifficulty = nDifficulty;
		break;
	case eGameTypeDeathmatch:
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nRunSpeed = nRunSpeed;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nScoreLimit = nScoreLimit;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nTimeLimit = nTimeLimit;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_nRounds = nRounds;
		break;
	case eGameTypeTeamDeathmatch:
		pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_nRunSpeed = nRunSpeed;
		pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_nScoreLimit = nScoreLimit;
		pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_nTimeLimit = nTimeLimit;
		pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_nRounds = nRounds;
		pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_bFriendlyFire = bFriendlyFire;
		break;
	case eGameTypeDoomsDay:
		pProfile->m_ServerGameOptions.GetDoomsday().m_nRunSpeed = nRunSpeed;
		pProfile->m_ServerGameOptions.GetDoomsday().m_nTimeLimit = nTimeLimit;
		pProfile->m_ServerGameOptions.GetDoomsday().m_nRounds = nRounds;
		pProfile->m_ServerGameOptions.GetDoomsday().m_bFriendlyFire = bFriendlyFire;
		break;
	}

	pProfile->Save( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMsgDefault()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMsgDefault (ILTMessage_Read *pMsg)
{

}

/***************************************************************************/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::FlashScreen()
//
//	PURPOSE:	Tint screen
//
// ----------------------------------------------------------------------- //

void CGameClientShell::FlashScreen(const LTVector &vFlashColor, const LTVector &vPos, float fFlashRange,
                                  float fTime, float fRampUp, float fRampDown, bool bForce)
{
	// [KLS 2/27/02] - Removed ALL screen flashing...
	return;


	CGameSettings* pSettings = GetInterfaceMgr( )->GetSettings();
	if (!pSettings) return;

	if (!bForce && !pSettings->ScreenFlash()) return;

    LTVector vCamPos;
	g_pLTClient->GetObjectPos(GetPlayerMgr()->GetCamera(), &vCamPos);

	// Determine if we can see this...

    LTVector vDir;
	vDir =  vPos - vCamPos;
    float fDirMag = vDir.Length();
	if (fDirMag > fFlashRange) return;

	// Okay, not adjust the tint based on the camera's angle to the tint pos.

    LTRotation rRot;
	g_pLTClient->GetObjectRotation(GetPlayerMgr()->GetCamera(), &rRot);

	vDir.Normalize();
    float fMul = vDir.Dot(rRot.Forward());
	if (fMul <= 0.0f) return;

	// {MD} See if we can even see this point.
	ClientIntersectQuery iQuery;
	ClientIntersectInfo iInfo;
	iQuery.m_From = vPos;
	iQuery.m_To = vCamPos;
    if(g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		// Something is in the way.
		return;
	}

	// Tint less if the pos was far away from the camera...

    float fVal = 1.0f - (fDirMag/fFlashRange);
	fMul *= (fVal <= 1.0f ? fVal : 1.0f);

    m_bFlashScreen  = true;
    m_fFlashStart   = g_pLTClient->GetTime();
	m_fFlashTime		= fTime;
	m_fFlashRampUp	= fRampUp;
	m_fFlashRampDown	= fRampDown;
	VEC_COPY(m_vFlashColor, vFlashColor);
	VEC_MULSCALAR(m_vFlashColor, m_vFlashColor, fMul);
}


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

	GetInterfaceMgr()->GetFullScreenTint()->TurnOn(m_bGamePaused);

	if (!IsMultiplayerGame())
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
	GetPlayerMgr()->SetMouseInput(!bPause, LTTRUE);
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
    g_pLTClient->SetInputState(bAllowInput);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateWeaponMuzzlePosition()
//
//	PURPOSE:	Update the current weapon muzzle pos
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateWeaponMuzzlePosition()
{
    float fIncValue = WEAPON_MOVE_INC_VALUE_SLOW;
    bool bChanged = false;

    LTVector vOffset(0, 0, 0);

	uint32 dwPlayerFlags = GetPlayerMgr()->GetPlayerFlags();

	// Move weapon faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = WEAPON_MOVE_INC_VALUE_FAST;
	}

  	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		vOffset = pClientWeapon->GetMuzzleOffset();
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
			pClientWeapon->SetMuzzleOffset( vOffset );
		}
	}

	//if (bChanged)
	//{
	//	CSPrint ("Muzzle offset = %f, %f, %f", vOffset.x, vOffset.y, vOffset.z);
	//}
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
	VEC_INIT(vOffset);

	uint32 dwPlayerFlags = GetPlayerMgr()->GetPlayerFlags();

	// Move weapon faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = WEAPON_MOVE_INC_VALUE_FAST;
	}

	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
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
		}
	}

	//if (bChanged)
	//{
	//	CSPrint ("Weapon offset = %f, %f, %f", vOffset.x, vOffset.y, vOffset.z);
	//}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateWeaponBreachOffset()
//
//	PURPOSE:	Update the current weapon breach offset
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateWeaponBreachOffset()
{
    float fIncValue = WEAPON_MOVE_INC_VALUE_SLOW;
    bool bChanged = false;

    LTVector vOffset(0, 0, 0);

	uint32 dwPlayerFlags = GetPlayerMgr()->GetPlayerFlags();

	// Move weapon faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = WEAPON_MOVE_INC_VALUE_FAST;
	}

  	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		vOffset = pClientWeapon->GetBreachOffset();
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
			pClientWeapon->SetBreachOffset( vOffset );
		}
	}

	if (bChanged)
	{
		CSPrint ("Breach offset = %f, %f, %f", vOffset.x, vOffset.y, vOffset.z);
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

	// Move offset faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = fIncValue * 2.0f;
	}

	// Move 1st person offset.x forward or backwards...

	if ((dwPlayerFlags & BC_CFLG_FORWARD) || (dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		g_vPlayerCameraOffset.x += fIncValue;
        bChanged = true;
	}

	// Move 1st person offset.y up or down...

	if ((dwPlayerFlags & BC_CFLG_JUMP) || (dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_JUMP ? fIncValue : -fIncValue;
		g_vPlayerCameraOffset.y += fIncValue;
        bChanged = true;
	}


	if (bChanged)
	{
		// Okay, set the offset...

		GetPlayerMgr()->GetPlayerCamera()->SetFirstPersonOffset(g_vPlayerCameraOffset);
        g_pLTClient->CPrint("1st person camera offset: %.2f, %.2f, %.2f", g_vPlayerCameraOffset.x, g_vPlayerCameraOffset.y, g_vPlayerCameraOffset.z);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::AdjustLightScale()
//
//	PURPOSE:	Update the current global light scale
//
// ----------------------------------------------------------------------- //

void CGameClientShell::AdjustLightScale()
{
    float fIncValue = 0.01f;
    bool bChanged = false;

    LTVector vScale;
	VEC_INIT(vScale);

    g_pLTClient->GetGlobalLightScale(&vScale);

	uint32 dwPlayerFlags = GetPlayerMgr()->GetPlayerFlags();

	// Move faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .5f;
	}


	// Move Red up/down...

	if ((dwPlayerFlags & BC_CFLG_FORWARD) || (dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vScale.x += fIncValue;
		vScale.x = vScale.x < 0.0f ? 0.0f : (vScale.x > 1.0f ? 1.0f : vScale.x);

        bChanged = true;
	}


	// Move Green up/down...

	if ((dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) ||
		(dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vScale.y += fIncValue;
		vScale.y = vScale.y < 0.0f ? 0.0f : (vScale.y > 1.0f ? 1.0f : vScale.y);

        bChanged = true;
	}


	// Move Blue up/down...

	if ((dwPlayerFlags & BC_CFLG_JUMP) || (dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		vScale.z += fIncValue;
		vScale.z = vScale.z < 0.0f ? 0.0f : (vScale.z > 1.0f ? 1.0f : vScale.z);

        bChanged = true;
	}


	// Okay, set the light scale.

    g_pLTClient->SetGlobalLightScale(&vScale);

	if (bChanged)
	{
		CSPrint ("Light Scale = %f, %f, %f", vScale.x, vScale.y, vScale.z);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::AdjustLightAdd()
//
//	PURPOSE:	Update the current global light add
//
// ----------------------------------------------------------------------- //

void CGameClientShell::AdjustLightAdd()
{
    float fIncValue = 0.01f;
    bool bChanged = false;

    LTVector vScale;
	VEC_INIT(vScale);

    g_pLTClient->GetCameraLightAdd(GetPlayerMgr()->GetCamera(), &vScale);

	uint32 dwPlayerFlags = GetPlayerMgr()->GetPlayerFlags();

	// Move faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .5f;
	}


	// Move Red up/down...

	if ((dwPlayerFlags & BC_CFLG_FORWARD) || (dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vScale.x += fIncValue;
		vScale.x = vScale.x < 0.0f ? 0.0f : (vScale.x > 1.0f ? 1.0f : vScale.x);

        bChanged = true;
	}


	// Move Green up/down...

	if ((dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) ||
		(dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vScale.y += fIncValue;
		vScale.y = vScale.y < 0.0f ? 0.0f : (vScale.y > 1.0f ? 1.0f : vScale.y);

        bChanged = true;
	}


	// Move Blue up/down...

	if ((dwPlayerFlags & BC_CFLG_JUMP) || (dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		vScale.z += fIncValue;
		vScale.z = vScale.z < 0.0f ? 0.0f : (vScale.z > 1.0f ? 1.0f : vScale.z);

        bChanged = true;
	}


	// Okay, set the light add.

    g_pLTClient->SetCameraLightAdd(GetPlayerMgr()->GetCamera(), &vScale);

	if (bChanged)
	{
		CSPrint ("Light Add = %f, %f, %f", vScale.x, vScale.y, vScale.z);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::AdjustFOV()
//
//	PURPOSE:	Update the current FOV
//
// ----------------------------------------------------------------------- //

void CGameClientShell::AdjustFOV()
{
    float fIncValue = 0.001f;
    bool bChanged = false;

    float fCurFOVx, fCurFOVy;

	// Save the current camera fov...

    g_pLTClient->GetCameraFOV(GetPlayerMgr()->GetCamera(), &fCurFOVx, &fCurFOVy);

	uint32 dwPlayerFlags = GetPlayerMgr()->GetPlayerFlags();

	// Move faster if running...

	if (dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .01f;
	}


	// Adjust X

	if ((dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) || (dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		fCurFOVx += fIncValue;

        bChanged = true;
	}


	// Adjust Y

	if ((dwPlayerFlags & BC_CFLG_JUMP) || (dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		fCurFOVy += fIncValue;

        bChanged = true;
	}


	// Okay, set the FOV..

	// Adjust the fov...

	GetPlayerMgr()->SetCameraFOV(fCurFOVx, fCurFOVy);

	if (bChanged)
	{
		CSPrint ("FOV X = %.2f, FOV Y = %.2f", RAD2DEG(fCurFOVx), RAD2DEG(fCurFOVy));
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
	if (hObj)
	{
        g_pCommonLT->SetObjectFlags(hObj, OFT_Client, CF_NOTIFYREMOVE, CF_NOTIFYREMOVE);
	}
	
	m_sfxMgr.HandleSFXMsg(hObj, pMsg);

	// Reset the message and send to the other FX mgr
	pMsg->SeekTo(0);
	m_ClientFXMgr.OnSpecialEffectNotify( hObj, pMsg );
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
	if (!hObj) return;

	m_sfxMgr.RemoveSpecialFX(hObj);

	m_ClientFXMgr.OnObjectRemove( hObj );

	GetInterfaceMgr()->OnObjectRemove(hObj);
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
	if (IsMainWindowMinimized())
	{
		RestoreMainWindow();
	}

	g_pMissionMgr->PreLoadWorld( pWorldName );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HasJoystick()
//
//	PURPOSE:	Determines whether or not there is a joystick device
//				present
//
// ----------------------------------------------------------------------- //

bool CGameClientShell::HasJoystick()
{
	// Ask the engine if we really have a joystick...
	DeviceObject *pJoysticks = g_pLTClient->GetDeviceObjects(DEVICETYPE_JOYSTICK);    
	if (pJoysticks)
	{
		g_pLTClient->FreeDeviceObjects(pJoysticks);
		return true;
	}

	return false;
	
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsJoystickEnabled()
//
//	PURPOSE:	Determines whether or not there is a joystick device
//				enabled
//
// ----------------------------------------------------------------------- //

bool CGameClientShell::IsJoystickEnabled()
{
	// first attempt to find a joystick device

	char strJoystick[128];
	memset (strJoystick, 0, 128);
    LTRESULT result = g_pLTClient->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
    if (result != LT_OK) return false;

	// ok - we found the device and have a name...see if it's enabled

    bool bEnabled = false;
    g_pLTClient->IsDeviceEnabled (strJoystick, &bEnabled);

	return bEnabled;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::EnableJoystick()
//
//	PURPOSE:	Attempts to find and enable a joystick device
//
// ----------------------------------------------------------------------- //

bool CGameClientShell::EnableJoystick()
{
	// first attempt to find a joystick device

	char strJoystick[128];
	memset(strJoystick, 0, 128);
    LTRESULT result = g_pLTClient->GetDeviceName(DEVICETYPE_JOYSTICK, strJoystick, 127);
    if (result != LT_OK) return false;

	// ok, now try to enable the device

	char strConsole[256];
	sprintf(strConsole, "EnableDevice \"%s\"", strJoystick);
    g_pLTClient->RunConsoleString(strConsole);

    bool bEnabled = false;
    g_pLTClient->IsDeviceEnabled(strJoystick, &bEnabled);

	return bEnabled;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HasGamepad()
//
//	PURPOSE:	Determines whether or not there is a Gamepad device
//				present
//
// ----------------------------------------------------------------------- //

bool CGameClientShell::HasGamepad()
{
	// Ask the engine if we really have a Gamepad...
	DeviceObject *pGamepads = g_pLTClient->GetDeviceObjects(DEVICETYPE_GAMEPAD);    
	if (pGamepads)
	{
		g_pLTClient->FreeDeviceObjects(pGamepads);
		return true;
	}

	return false;

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsGamepadEnabled()
//
//	PURPOSE:	Determines whether or not there is a Gamepad device
//				enabled
//
// ----------------------------------------------------------------------- //

bool CGameClientShell::IsGamepadEnabled()
{
	// first attempt to find a Gamepad device

	char strGamepad[128];
	memset (strGamepad, 0, 128);
    LTRESULT result = g_pLTClient->GetDeviceName (DEVICETYPE_GAMEPAD, strGamepad, 127);
    if (result != LT_OK) return false;

	// ok - we found the device and have a name...see if it's enabled

    bool bEnabled = false;
    g_pLTClient->IsDeviceEnabled (strGamepad, &bEnabled);

	return bEnabled;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::EnableGamepad()
//
//	PURPOSE:	Attempts to find and enable a Gamepad device
//
// ----------------------------------------------------------------------- //

bool CGameClientShell::EnableGamepad()
{
	// first attempt to find a Gamepad device

	char strGamepad[128];
	memset(strGamepad, 0, 128);
    LTRESULT result = g_pLTClient->GetDeviceName(DEVICETYPE_GAMEPAD, strGamepad, 127);
    if (result != LT_OK) return false;

	// ok, now try to enable the device

	char strConsole[256];
	sprintf(strConsole, "EnableDevice \"%s\"", strGamepad);
    g_pLTClient->RunConsoleString(strConsole);

    bool bEnabled = false;
    g_pLTClient->IsDeviceEnabled(strGamepad, &bEnabled);

	return bEnabled;
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
		g_pClientMultiplayerMgr->ForceDisconnect();
	}

	//since our history was cleared by loading the level... rebuild it
	g_pInterfaceMgr->GetScreenMgr()->AddScreenToHistory( SCREEN_ID_MAIN );
	g_pInterfaceMgr->GetScreenMgr()->AddScreenToHistory( SCREEN_ID_OPTIONS );

	if ( GetInterfaceMgr( )->GetGameState() == GS_LOADINGLEVEL)
	{
		GetInterfaceMgr( )->LoadFailed(SCREEN_ID_PERFORMANCE,IDS_PERFORMANCE_TEST_ABORTED);
	}
	else
	{

		MBCreate mb;
		GetInterfaceMgr( )->ShowMessageBox(IDS_PERFORMANCE_TEST_ABORTED, &mb);

		//we aborted performance testing go back to performance screen
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PERFORMANCE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateDebugInfo()
//
//	PURPOSE:	Update debugging info.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateDebugInfo()
{
	char buf[100];

	ClearDebugStrings();

	//determine the offset we should use on the positions
	LTVector vOffset(0, 0, 0);
	const char* pszOffsetDescription = "(Actual)";

	if((uint32)g_vtApplyWorldOffset.GetFloat(1.0f))
	{
		g_pLTClient->GetSourceWorldOffset(vOffset);
		pszOffsetDescription = "(Level)";
	}

	// Check to see if we should show the player position...

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_bShowPlayerPos && hPlayerObj)
	{
        LTVector vPos;
		g_pLTClient->GetObjectPos(hPlayerObj, &vPos);

		//handle the shift from the current world to the source world
		vPos += vOffset;

		sprintf(buf, "Player Position %s: %6.0f, %6.0f, %6.0f  ", pszOffsetDescription, vPos.x, vPos.y, vPos.z);

		SetDebugString(buf);
	}

	if (m_bShowCamPosRot)
	{
        LTVector vPos;
		g_pLTClient->GetObjectPos(GetPlayerMgr()->GetCamera(), &vPos);

		//handle the shift from the current world to the source world
		vPos += vOffset;

		// Convert pitch and yaw to the same units used by DEdit...

		float fYawDeg = RAD2DEG(GetPlayerMgr()->GetYaw());
		while (fYawDeg < 0.0f)
		{
			fYawDeg += 360.0f;
		}
		while (fYawDeg > 360.0f)
		{
			fYawDeg -= 360.0f;
		}

		float fPitchDeg = RAD2DEG(GetPlayerMgr()->GetPitch());
		while (fPitchDeg < 0.0f)
		{
			fPitchDeg += 360.0f;
		}
		while (fPitchDeg > 360.0f)
		{
			fPitchDeg -= 360.0f;
		}

		sprintf(buf, "Camera Position %s: %6.0f %6.0f %6.0f  ", pszOffsetDescription, vPos.x, vPos.y, vPos.z);
		SetDebugString(buf, eDSBottomRight, 0);

		sprintf(buf, "Camera Yaw: %6.0f  ", fYawDeg);
		SetDebugString(buf, eDSBottomRight, 2);

		sprintf(buf, "Camera Pitch: %6.0f  ", fPitchDeg);
		SetDebugString(buf, eDSBottomRight, 1);
	}



	// See if the FOV has changed...

	if (GetPlayerMgr()->CanChangeFOV())
	{
		float fovX, fovY;
		g_pLTClient->GetCameraFOV(GetPlayerMgr()->GetCamera(), &fovX, &fovY);

		if (fovX != g_vtFOVXNormal.GetFloat() || fovY != g_vtFOVYNormal.GetFloat())
		{
			GetPlayerMgr()->SetCameraFOV(DEG2RAD(g_vtFOVXNormal.GetFloat()), DEG2RAD(g_vtFOVYNormal.GetFloat()));
		}
	}


	// Check to see if we are in spectator or invisible mode...

	LTBOOL bSpectator = GetPlayerMgr()->IsSpectatorMode();
	if ((bSpectator || GetPlayerMgr()->IsInvisibleMode()) && !g_bScreenShotMode)
	{
		SetDebugString(bSpectator ? "SPECTATOR MODE" : "GHOST MODE", eDSBottomLeft);

#ifndef _TO2DEMO
        LTVector vPos;
		g_pLTClient->GetObjectPos(GetPlayerMgr()->GetCamera(), &vPos);

		//handle the shift from the current world to the source world
		vPos += vOffset;

		sprintf(buf, "Camera Position %s: %6.0f %6.0f %6.0f", pszOffsetDescription, vPos.x, vPos.y, vPos.z);
		SetDebugString(buf);
#endif // _TO2DEMO
	}


	// Check to see if we are in performance test mode...

	if (m_bRunningPerfTest && m_pPerformanceTest)
	{
		SetDebugString(" TESTING PERFORMANCE SETTINGS", eDSBottomLeft);
	
		m_pPerformanceTest->Update(g_pLTClient->GetFrameTime());

		/* Don't show framerate...
		uint32 nFPS = m_pPerformanceTest->GetCurFPS();
		if (nFPS > 0)
		{
			bool bVSync = (GetConsoleInt("VSyncOnFlip", 0) ? true : false);

			sprintf(buf, " %4d FPS %s", nFPS, (bVSync ? "(VSYNC ENABLED) " : ""));
			SetDebugString(buf);
		}
		*/
	}

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("PlayerDims");
	if (hVar)
	{
        if (g_pLTClient->GetVarValueFloat(hVar) > 0.0f)
		{
			CreateBoundingBox();
			UpdateBoundingBox();
		}
		else if (m_hBoundingBox)
		{
            g_pLTClient->RemoveObject(m_hBoundingBox);
            m_hBoundingBox = NULL;
		}
	}


	if (g_vtSpecial.GetFloat())
	{
		g_vtSpecial.SetFloat(0.0f);
		g_pLTClient->CPrint("%d", g_dwSpecial);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetDebugString
//
//	PURPOSE:	Set the debug string (create it if necessary).
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetDebugString(char* strMessage, DSSL eLoc, uint8 nLine)
{
	if (!strMessage || strMessage[0] == '\0') return;
	if (nLine < 0 || nLine >= kMaxDebugStrings) return;

	// Set the size each time since the screen resolution may have changed...

	uint8 nSize = (uint8)((float)g_pLayoutMgr->GetHelpSize() * g_pInterfaceResMgr->GetXRatio());
	uint8 nFont = g_pLayoutMgr->GetHelpFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	
	CUIPolyString* pPolyString = NULL;

	switch (eLoc)
	{
		case eDSBottomLeft :
		{
			if (!m_pLeftDebugString[nLine])
			{
				pPolyString = g_pFontManager->CreatePolyString(pFont, "", 0.0f, 0.0f);
				m_pLeftDebugString[nLine] = pPolyString;
			}
			else
			{
				pPolyString = m_pLeftDebugString[nLine];
			}
		}
		break;

		case eDSBottomRight :
		default :
		{
			if (!m_pRightDebugString[nLine])
			{
				pPolyString = g_pFontManager->CreatePolyString(pFont, "", 0.0f, 0.0f);
				m_pRightDebugString[nLine] = pPolyString;
			}
			else
			{
				pPolyString = m_pRightDebugString[nLine];
			}
		}
		break;
	}

	if (pPolyString)
	{
		pPolyString->SetColor(argbWhite);
		pPolyString->SetCharScreenHeight(nSize);
		pPolyString->SetText(strMessage);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ClearDebugStrings
//
//	PURPOSE:	Clear all the debug strings
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ClearDebugStrings()
{
	for (int i=0; i < kMaxDebugStrings; i++)
	{
		if (m_pLeftDebugString[i])
		{
			m_pLeftDebugString[i]->SetText("");
		}

		if (m_pRightDebugString[i])
		{
			m_pRightDebugString[i]->SetText("");
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::RenderDebugStrings
//
//	PURPOSE:	Render all the debug strings
//
// --------------------------------------------------------------------------- //

void CGameClientShell::RenderDebugStrings()
{
    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nScreenWidth, nScreenHeight;
    g_pLTClient->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);

	float fx = 0.0f, fy = 0.0f;
	float fYLeftOffset = 0.0f, fYRightOffset = 0.0f;
	float fScreenWidth  = float(nScreenWidth);
	float fScreenHeight = float(nScreenHeight);

	for (int i=0; i < kMaxDebugStrings; i++)
	{
		if (m_pLeftDebugString[i])
		{
			fYLeftOffset += m_pLeftDebugString[i]->GetHeight();

			fy = fScreenHeight - fYLeftOffset;
	
			m_pLeftDebugString[i]->SetPosition(0.0f, fy);
			m_pLeftDebugString[i]->Render();
		}

		if (m_pRightDebugString[i])
		{
			fYRightOffset += m_pRightDebugString[i]->GetHeight();

			fx = fScreenWidth  - m_pRightDebugString[i]->GetWidth();
			fy = fScreenHeight - fYRightOffset;
	
			m_pRightDebugString[i]->SetPosition(fx, fy);
			m_pRightDebugString[i]->Render();
		}
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
		case CHEAT_POSWEAPON_MUZZLE :
		{
			if (!g_pPlayerMgr->IsSpectatorMode())
			{
				m_bTweakingWeaponMuzzle = !m_bTweakingWeaponMuzzle;

				g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bTweakingWeaponMuzzle);

				// Save tweaks...

				if (!m_bTweakingWeaponMuzzle)
				{
                    g_pWeaponMgr->WriteFile();
				}
			}
		}
		break;

		case CHEAT_WEAPON_BREACHOFFSET :
		{
			if (!g_pPlayerMgr->IsSpectatorMode())
			{
				m_bTweakingWeaponBreachOffset = !m_bTweakingWeaponBreachOffset;

				g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bTweakingWeaponBreachOffset);

				// Save tweaks...

				if (!m_bTweakingWeaponBreachOffset)
				{
                    g_pWeaponMgr->WriteFile();
				}
			}
		}
		break;

		case CHEAT_POSWEAPON :
		{
			if (!g_pPlayerMgr->IsSpectatorMode())
			{
				m_bTweakingWeapon = !m_bTweakingWeapon;

				g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bTweakingWeapon);

				// Save tweaks...

				if (!m_bTweakingWeapon)
				{
                    g_pWeaponMgr->WriteFile();
				}
			}
		}
		break;

		case CHEAT_POS1STCAM :
		{
			if (!g_pPlayerMgr->IsSpectatorMode())
			{
				m_bAdjust1stPersonCamera = !m_bAdjust1stPersonCamera;

				g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bAdjust1stPersonCamera);
			}
		}
		break;

		case CHEAT_LIGHTSCALE :
		{
			m_bAdjustLightScale = !m_bAdjustLightScale;

			g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bAdjustLightScale);
		}
		break;

		case CHEAT_LIGHTADD :
		{
			m_bAdjustLightAdd = !m_bAdjustLightAdd;

			g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bAdjustLightAdd);
		}
		break;

		case CHEAT_FOV :
		{
			m_bAdjustFOV = !m_bAdjustFOV;

			g_pPlayerMgr->GetMoveMgr()->AllowMovement(!m_bAdjustFOV);
		}
		break;

		case CHEAT_CHASETOGGLE :
		{
			if (GetPlayerMgr()->GetPlayerState() == PS_ALIVE && !GetPlayerMgr()->IsZoomed())
			{
				GetPlayerMgr()->SetExternalCamera(GetPlayerMgr()->IsFirstPerson());
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
	GetConsoleString("DebugCameraName",szTmp,"CameraPoint");
	if (stricmp(s_szDebugCamName,szTmp) != 0)
	{
		SAFE_STRCPY(s_szDebugCamName,szTmp);
		s_nDebugCamCount = 0;
	}
	char szFileName[64];
	sprintf(szFileName,"Game\\%s%02d.txt", s_szDebugCamName, s_nDebugCamCount);

	FILE* pFile = fopen (szFileName, "wt");
	if (!pFile) return;

    LTVector vPos;
	g_pLTClient->GetObjectPos(GetPlayerMgr()->GetCamera(), &vPos);

	//handle the shift from the current world to the source world
	if((uint32)g_vtApplyWorldOffset.GetFloat(1.0f))
	{
		LTVector vOffset;
		g_pLTClient->GetSourceWorldOffset(vOffset);
		vPos += vOffset;
	}

	// Convert pitch and yaw to user friendly units...

	float fYawDeg = RAD2DEG(GetPlayerMgr()->GetYaw());
	while (fYawDeg < 0.0f)
	{
		fYawDeg += 360.0f;
	}
	while (fYawDeg > 360.0f)
	{
		fYawDeg -= 360.0f;
	}

	float fPitchDeg = RAD2DEG(GetPlayerMgr()->GetPitch());
	while (fPitchDeg < 0.0f)
	{
		fPitchDeg += 360.0f;
	}
	while (fPitchDeg > 360.0f)
	{
		fPitchDeg -= 360.0f;
	}


	sprintf(szTmp,"\n[CameraPoint]\n");
	fwrite (szTmp, strlen(szTmp), 1, pFile);

	s_nDebugCamCount++;
	sprintf(szTmp,"Name     = \"%s%02d\"\n", s_szDebugCamName, s_nDebugCamCount);
	fwrite (szTmp, strlen(szTmp), 1, pFile);

	sprintf(szTmp,"Pos      = <%6.0f,%6.0f,%6.0f>\n", vPos.x, vPos.y, vPos.z);
	fwrite (szTmp, strlen(szTmp), 1, pFile);

	sprintf(szTmp,"Rotation = <%0.5f, %0.5f, 0.00000>\n", GetPlayerMgr()->GetPitch(), GetPlayerMgr()->GetYaw());
	fwrite (szTmp, strlen(szTmp), 1, pFile);


	fwrite ("\n", 1, 1, pFile);

	fclose (pFile);

	g_pLTClient->CPrint("%s%02d:Pos=<%6.0f,%6.0f,%6.0f>;Rot=<%6.0f,%6.0f,     0>", 
						s_szDebugCamName, s_nDebugCamCount, vPos.x, vPos.y, vPos.z, fPitchDeg, fYawDeg);


}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::FirstUpdate
//
//	PURPOSE:	Handle first update (each level)
//
// --------------------------------------------------------------------------- //

void CGameClientShell::FirstUpdate()
{
	if (!m_bFirstUpdate) return; 

#ifdef _TRON_E3_DEMO
	s_fDemoTime = 0.0f;
#endif

	GetPlayerMgr()->FirstUpdate();

    m_bFirstUpdate = false;


	// Set up the level-start screen fade...

	if (g_varStartLevelScreenFade.GetFloat())
	{
		GetInterfaceMgr( )->StartScreenFadeIn(g_varStartLevelScreenFadeTime.GetFloat());
	}


	// Set prediction if we are playing multiplayer...We turn this
	// off for single player because projectiles look MUCH better...

	if ( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
	{
	   g_pLTClient->RunConsoleString("Prediction 1");
	}
	else
	{
	   g_pLTClient->RunConsoleString("Prediction 0");
	}


	// Set misc console vars...

	MirrorSConVar("AllSkyPortals", "AllSkyPortals");


	// Set up the global (per level) wind values...

    g_pLTClient->GetSConValueFloat("WindX", g_vWorldWindVel.x);
    g_pLTClient->GetSConValueFloat("WindY", g_vWorldWindVel.y);
    g_pLTClient->GetSConValueFloat("WindZ", g_vWorldWindVel.z);


	// Set up the global (per level) fog values...

	ResetDynamicWorldProperties();


	// Initialize the music playlists...

	RestoreMusic();


	// Start with a clean slate...

	g_pLTClient->ClearInput();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::RestoreMusic
//
//	PURPOSE:	Restore the music state
//
// --------------------------------------------------------------------------- //

void CGameClientShell::RestoreMusic()
{
	if (m_Music.IsInitialized())
	{
		// Initialize music for the current level...

		char aMusicDirectory[256];
		char* pMusicDirectory = NULL;
		if (g_pLTClient->GetSConValueString("MusicDirectory", aMusicDirectory, sizeof(aMusicDirectory)) == LT_OK)
		{
			pMusicDirectory = aMusicDirectory;
		}

		char aMusicControlFile[256];
		char* pMusicControlFile = NULL;
		if (g_pLTClient->GetSConValueString("MusicControlFile", aMusicControlFile, sizeof(aMusicControlFile)) == LT_OK)
		{
			pMusicControlFile = aMusicControlFile;
		}

		if (pMusicDirectory && pMusicControlFile)
		{
			CMusicState* pMS = m_Music.GetMusicState();

			CMusicState musicState;
			musicState.nIntensity = pMS->nIntensity;
			strcpy(musicState.szDirectory, pMusicDirectory);
			strcpy(musicState.szControlFile, pMusicControlFile);

			m_Music.RestoreMusicState(musicState);
		}
	}
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
	char buf[512];
	float fVal = 0.0f;

    g_pLTClient->GetSConValueFloat(pSVarName, fVal);

	// Special case, make all farz calls go through this function...
	if (stricmp(pCVarName, "FarZ") == 0)
	{
		SetFarZ((int)fVal);
	}
	else
	{
		sprintf(buf, "%s %f", pCVarName, fVal);
		g_pLTClient->RunConsoleString(buf);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ResetGlobalFog
//
//	PURPOSE:	Reset the global fog values based on the saved values...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ResetDynamicWorldProperties(LTBOOL bUseWorldFog)
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
			g_pLTClient->RunConsoleString("FogEnable 0");
			return;
		}

		MirrorSConVar("FogEnable", "FogEnable");
		MirrorSConVar("FogNearZ", "FogNearZ");
		MirrorSConVar("FogFarZ", "FogFarZ");
		MirrorSConVar("LMAnimStatic", "LMAnimStatic");
		MirrorSConVar("FogR", "FogR");
		MirrorSConVar("FogG", "FogG");
		MirrorSConVar("FogB", "FogB");

		MirrorSConVar("SkyFogEnable", "SkyFogEnable");
		MirrorSConVar("SkyFogNearZ", "SkyFogNearZ");
		MirrorSConVar("SkyFogFarZ", "SkyFogFarZ");

		// VFog....

		MirrorSConVar("SC_VFog", "VFog");
		MirrorSConVar("SC_VFogMinY", "VFogMinY");
		MirrorSConVar("SC_VFogMaxY", "VFogMaxY");
		MirrorSConVar("SC_VFogDensity", "VFogDensity");
		MirrorSConVar("SC_VFogMax", "VFogMax");
		MirrorSConVar("SC_VFogMaxYVal", "VFogMaxYVal");
		MirrorSConVar("SC_VFogMinYVal", "VFogMinYVal");
	}

	//update the shadow information
	MirrorSConVar("ModelShadow_Proj_Alpha", "ModelShadow_Proj_Alpha");
	MirrorSConVar("ModelShadow_Proj_MinColorComponent", "ModelShadow_Proj_MinColorComponent");
	MirrorSConVar("ModelShadow_Proj_MaxProjDist", "ModelShadow_Proj_MaxProjDist");
	MirrorSConVar("DrawAllModelShadows", "DrawAllModelShadows");
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
	if (!m_bMainWindowFocus) return;
	
	if (!GetPlayerMgr()->PreRender()) return;


	// Important to update this after the weapon model has been updated
	// (some fx depend on the position of the weapon model)...

	m_sfxMgr.UpdateSpecialFX();

	// Update any client-side special effects...

	m_DamageFXMgr.Update();

	//make sure to update the gore settings in case they changed
	UpdateGoreSettings();

	HLOCALOBJ hCamera = GetPlayerMgr()->GetCamera();

	//handle updating the paused status
	m_ClientFXMgr.Pause(IsServerPaused());

	//update all the effects status as well as any that might effect the camera
	m_ClientFXMgr.UpdateAllActiveFX( m_bMainWindowFocus );

    g_pLTClient->Start3D();

	float fFrameTime = (IsServerPaused()) ? 0.0f : GetFrameTime();

    g_pLTClient->RenderCamera(hCamera, fFrameTime);

	// Render the the dynamic FX.
	m_sfxMgr.RenderFX(hCamera);

	// Render the effects
	m_ClientFXMgr.RenderAllActiveFX(m_bMainWindowFocus );

    g_pLTClient->StartOptimized2D();

	GetInterfaceMgr( )->Draw();

	// Display any necessary debugging info...

	RenderDebugStrings();

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D(END3D_CANDRAWCONSOLE);

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

	m_Music.Save(pMsg);

	// Save all necessary data members...

	pMsg->WriteLTVector(m_vFlashColor);

	pMsg->Writeuint8(m_eDifficulty);
	pMsg->Writebool(m_bFlashScreen);

	pMsg->Writefloat(m_fFlashTime);
	pMsg->Writefloat(m_fFlashStart);
	pMsg->Writefloat(m_fFlashRampUp);
	pMsg->Writefloat(m_fFlashRampDown);
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
	
	m_Music.Load(pMsg);

	// Load data members...

    m_vFlashColor				= pMsg->ReadLTVector();

    m_eDifficulty               = (GameDifficulty) pMsg->Readuint8();
    m_bFlashScreen              = pMsg->Readbool();
    m_fFlashTime                = pMsg->Readfloat();
    m_fFlashStart               = pMsg->Readfloat();
    m_fFlashRampUp              = pMsg->Readfloat();
    m_fFlashRampDown            = pMsg->Readfloat();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnModelKey
//
//	PURPOSE:	Handle weapon model keys
//
// --------------------------------------------------------------------------- //

void CGameClientShell::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	bool bResult;

	// give the client weapon mgr first shot at it
	bResult = g_pPlayerMgr->GetClientWeaponMgr()->OnModelKey( hObj, pArgs );

	// if it hasn't been handled...
	if ( !bResult )
	{
		// try the sfx mgr
		m_sfxMgr.OnModelKey(hObj, pArgs);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnPlaySound
//
//	PURPOSE:	Handle a sound being played...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::OnPlaySound(PlaySoundInfo* pPSI)
{
#ifdef USE_DX8_SOFTWARE_FILTERS
	if (!pPSI || !g_vtUseSoundFilters.GetFloat()) return;

	SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter((uint8)pPSI->m_UserData);
	if (!pFilter)
	{
		g_pLTClient->CPrint("ERROR in CGameClientShell::OnPlaySound()!");
		g_pLTClient->CPrint("  couldn't find filter (%d)", pPSI->m_UserData);
		return;
	}

	if (pFilter && pPSI->m_hSound)
	{
		// See if we need to calculate the filter dynamically...

		if (g_pSoundFilterMgr->IsDynamic(pFilter))
		{
			pFilter = GetDynamicSoundFilter();

			if (!pFilter)
			{
				g_pLTClient->CPrint("ERROR in CGameClientShell::OnPlaySound()!");
				g_pLTClient->CPrint("  couldn't find a dynamic filter (%s)", pFilter->szName);
				return;
			}
		}

		// Some sounds are unfiltered...

		if (g_pSoundFilterMgr->IsUnFiltered(pFilter)) return;


		// Set up the filter

		ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();

		pSoundMgr->SetSoundFilter(pPSI->m_hSound, pFilter->szFilterName);
		for (int i=0; i < pFilter->nNumVars; i++)
		{
			pSoundMgr->SetSoundFilterParam(pPSI->m_hSound, pFilter->szVars[i], pFilter->fValues[i]);
		}

		// TEMP, let us test what filter is being used...
		// g_pLTClient->CPrint("Using Filter: %s", pFilter->szName);
	}
	else
	{
		g_pLTClient->CPrint("ERROR in CGameClientShell::OnPlaySound()!");
		if (pFilter)
		{
			g_pLTClient->CPrint("  Invalid sound associated with Filter: %s", pFilter->szName);
		}
		else
		{
			g_pLTClient->CPrint("  Invalid filter associated with FilterId: %d", pPSI->m_UserData);
		}
	}
#endif
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::CreateBoundingBox
//
//	PURPOSE:	Create a box around the MoveMgr object
//
// --------------------------------------------------------------------------- //

void CGameClientShell::CreateBoundingBox()
{
	if (m_hBoundingBox) return;

	HLOCALOBJ hMoveMgrObj = g_pPlayerMgr->GetMoveMgr()->GetObject();
	if (!hMoveMgrObj) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    LTVector vPos;
	g_pLTClient->GetObjectPos(hMoveMgrObj, &vPos);
	theStruct.m_Pos = vPos;

	SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.ltb");
	SAFE_STRCPY(theStruct.m_SkinName, "SpecialFX\\smoke.dtx");
	theStruct.m_ObjectType = OT_MODEL;
	theStruct.m_Flags = FLAG_VISIBLE;

    m_hBoundingBox = g_pLTClient->CreateObject(&theStruct);

	UpdateBoundingBox();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateBoundingBox()
//
//	PURPOSE:	Update the bounding box
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateBoundingBox()
{
	if (!m_hBoundingBox) return;

	HLOCALOBJ hMoveMgrObj = g_pPlayerMgr->GetMoveMgr()->GetObject();
	if (!hMoveMgrObj) return;

    LTVector vPos;
	g_pLTClient->GetObjectPos(hMoveMgrObj, &vPos);
	g_pLTClient->SetObjectPos(m_hBoundingBox, &vPos);

    LTVector vDims;
    g_pPhysicsLT->GetObjectDims(hMoveMgrObj, &vDims);

    LTVector vScale;
	VEC_DIVSCALAR(vScale, vDims, 0.5f);
    g_pLTClient->SetObjectScale(m_hBoundingBox, &vScale);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DefaultModelHook
//
//	PURPOSE:	Default model hook function
//
// --------------------------------------------------------------------------- //

void DefaultModelHook (ModelHookData *pData, void *pUser)
{

	CGameClientShell* pShell = (CGameClientShell*) pUser;
	if (!pShell) return;

    uint32 nUserFlags = 0;
    g_pCommonLT->GetObjectFlags(pData->m_hObject, OFT_User, nUserFlags);

	// If we're using spy vision, turn off the glow on all models
	// that use the default model hook function...

	if (g_pPlayerMgr->UsingSpyVision())
	{
		pData->m_HookFlags |= MHF_NOGLOW;
	}

	if (nUserFlags & USRFLG_GLOW)
	{
		pData->m_LightAdd = g_pPlayerMgr->GetModelGlow();
		VEC_CLAMP((pData->m_LightAdd), 0.0f, 255.0f);
	}
	else if (nUserFlags & USRFLG_MODELADD)
	{
		// Get the new color out of the upper 3 bytes of the
		// user flags...

        float r = (float)(nUserFlags>>24);
        float g = (float)(nUserFlags>>16);
        float b = (float)(nUserFlags>>8);

		VEC_SET (pData->m_LightAdd, r, g, b);
		VEC_CLAMP((pData->m_LightAdd), 0.0f, 255.0f);
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	HookedWindowProc
//
//	PURPOSE:	Hook it real good
//
// --------------------------------------------------------------------------- //

LRESULT CALLBACK HookedWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		HANDLE_MSG(hWnd, WM_LBUTTONUP, CGameClientShell::OnLButtonUp);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, CGameClientShell::OnLButtonDown);
		HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, CGameClientShell::OnLButtonDblClick);
		HANDLE_MSG(hWnd, WM_RBUTTONUP, CGameClientShell::OnRButtonUp);
		HANDLE_MSG(hWnd, WM_RBUTTONDOWN, CGameClientShell::OnRButtonDown);
		HANDLE_MSG(hWnd, WM_RBUTTONDBLCLK, CGameClientShell::OnRButtonDblClick);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, CGameClientShell::OnMouseMove);
		HANDLE_MSG(hWnd, WM_CHAR, CGameClientShell::OnChar);
		HANDLE_MSG(hWnd, WM_SETCURSOR, OnSetCursor);
	}
	_ASSERT(g_pfnMainWndProc);
	return(CallWindowProc(g_pfnMainWndProc,hWnd,uMsg,wParam,lParam));
}

void CGameClientShell::OnChar(HWND hWnd, char c, int rep)
{
	g_pInterfaceMgr->OnChar((unsigned char)c);
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
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("Windowed");
	BOOL bClip = TRUE;
	if(hVar)
	{
        float fVal = g_pLTClient->GetVarValueFloat(hVar);
		if(fVal == 1.0f)
			bClip = FALSE;
	}

	RECT screenRect;
	GetWindowRect(GetDesktopWindow(), &screenRect);


	if(bClip)
	{
  		SetWindowLong(g_hMainWnd,GWL_STYLE,WS_VISIBLE);
		SetWindowPos(g_hMainWnd,HWND_TOPMOST,0,0,screenRect.right - screenRect.left,screenRect.bottom - screenRect.top,SWP_FRAMECHANGED);

		RECT wndRect;
		GetWindowRect(g_hMainWnd, &wndRect);
		ClipCursor(&wndRect);
	}
	else
	{
		SetWindowPos(g_hMainWnd,HWND_NOTOPMOST,
					((screenRect.right - screenRect.left) - nWidth) / 2,
					((screenRect.bottom - screenRect.top) - nHeight) / 2, 
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
	g_pfnMainWndProc = (WNDPROC)GetWindowLongPtr(g_hMainWnd,GWLP_WNDPROC);
#else
	g_pfnMainWndProc = (FARPROC)GetWindowLongPtr(g_hMainWnd,GWLP_WNDPROC);
#endif

	if(!g_pfnMainWndProc)
	{
		TRACE("HookWindow - ERROR - could not get the window procedure from the engine window!\n");
		return FALSE;
	}

	// Replace it with ours
	if(!SetWindowLongPtr(g_hMainWnd,GWLP_WNDPROC, (LONG_PTR)HookedWindowProc))
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
		SetWindowLongPtr(g_hMainWnd, GWLP_WNDPROC, (LONG_PTR)g_pfnMainWndProc);
		g_hMainWnd = 0;
		g_pfnMainWndProc = NULL;
	}
}

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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleWeaponPickup()
//
//	PURPOSE:	Handle picking up weapon
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleWeaponPickup(uint8 nWeaponID, bool bSuccess /* = true */)
{
	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if (!pWeapon) return;

	char szMsg[128] = "";
	if (bSuccess)
	{
		if (g_pPlayerStats->HaveWeapon(nWeaponID) && g_pPlayerMgr->IsSearching())
		{
			g_pPickupMsgs->AddMessage(LoadTempString(IDS_DISARMED),"");
			return;
		}
		else
		{
			FormatString(IDS_GUNPICKUP,szMsg,sizeof(szMsg),pWeapon->szShortName);
		}
	}
	else if( IsCoopMultiplayerGameType( ))
	{
		if (pWeapon->bIsAmmo && pWeapon->nIsAmmoNoPickupId)
		{
			FormatString(pWeapon->nIsAmmoNoPickupId, szMsg,sizeof(szMsg));
		}
		else if (pWeapon->bInfiniteAmmo)
		{
			FormatString(IDS_GUN_NOPICKUP_INF,szMsg,sizeof(szMsg),pWeapon->szShortName);
		}
		else
		{
			FormatString(IDS_GUN_NOPICKUP,szMsg,sizeof(szMsg),pWeapon->szShortName);
		}

		// Play the sound bute NoPickupSound
		g_pClientSoundMgr->PlaySoundLocal("NoPickupSound", SOUNDPRIORITY_PLAYER_HIGH);
	}

	// If the weapon is really ammo (e.g., grenade), don't display a pickup message, only 
	// display a message if we didn't pick up the weapon...

	if (szMsg[0] && (!pWeapon->bIsAmmo || !bSuccess))
	{
		std::string icon = pWeapon->GetSilhouetteIcon();
		g_pPickupMsgs->AddMessage(szMsg,icon.c_str());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleGearPickup()
//
//	PURPOSE:	Handle picking up gear
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleGearPickup(uint8 nGearId, bool bSuccess /* = true */)
{
	GEAR const *pGear = g_pWeaponMgr->GetGear(nGearId);
	if (!pGear) return;

	int nNameId = pGear->nNameId;
	if (!nNameId) return;

	char szName[64];
	LoadString(nNameId,szName,sizeof(szName));
	if (!strlen(szName)) return;

	char szMsg[128] = "";
	if (bSuccess)
	{
		FormatString(IDS_GEARPICKUP,szMsg,sizeof(szMsg),szName);
	}
	else if( IsCoopMultiplayerGameType( ))
	{
		FormatString(IDS_GEAR_NOPICKUP,szMsg,sizeof(szMsg),szName);

		// Play the sound bute NoPickupSound
		g_pClientSoundMgr->PlaySoundLocal("NoPickupSound", SOUNDPRIORITY_PLAYER_HIGH);
	}

	if (szMsg[0])
	{
		g_pPickupMsgs->AddMessage(szMsg,pGear->szIcon);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleModPickup()
//
//	PURPOSE:	Handle picking up mod
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleModPickup(uint8 nModId, bool bSuccess /* = true */)
{
	MOD const *pMod = g_pWeaponMgr->GetMod(nModId);
	if (!pMod) return;

	int nNameId = pMod->nNameId;
	if (!nNameId) return;

	char szName[64];
	LoadString(nNameId,szName,sizeof(szName));
	if (!strlen(szName)) return;
    
	char szMsg[128] = "";
	if (bSuccess)
	{
		FormatString(IDS_MODPICKUP,szMsg,sizeof(szMsg),szName);
	}
	else if( IsCoopMultiplayerGameType( ))
	{
		FormatString(IDS_MOD_NOPICKUP,szMsg,sizeof(szMsg),szName);

		// Play the sound bute NoPickupSound
		g_pClientSoundMgr->PlaySoundLocal("NoPickupSound", SOUNDPRIORITY_PLAYER_HIGH);
	}

	if (szMsg[0])
	{
		g_pPickupMsgs->AddMessage(szMsg,pMod->szIcon);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleAmmoPickup()
//
//	PURPOSE:	Handle picking up ammo
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleAmmoPickup(uint8 nAmmoId, int nAmmoCount, 
										bool bSuccess /* = true */,
										uint8 nWeaponId /* = WMGR_INVALID_ID */)
{
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo) return;

	if (pAmmo->nSelectionAmount >= 1000)
		return;

	char szMsg[1280] = "";
	if (bSuccess)
	{
		//check for "infinite" ammo types
		FormatString(IDS_AMMOPICKUP,szMsg,sizeof(szMsg),nAmmoCount,pAmmo->szShortName);
	}
	else if( IsCoopMultiplayerGameType( ))
	{
		FormatString(IDS_AMMO_NOPICKUP,szMsg,sizeof(szMsg),pAmmo->szShortName);

		// Play the sound bute NoPickupSound
		g_pClientSoundMgr->PlaySoundLocal("NoPickupSound", SOUNDPRIORITY_PLAYER_HIGH);
	}

	if (szMsg[0])
	{
		std::string icon = pAmmo->GetNormalIcon();

		// Use the weapon's icon if necessary...
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
		if (pWeapon && pWeapon->bIsAmmo)
		{
			icon = pWeapon->GetSilhouetteIcon();
		}

		g_pPickupMsgs->AddMessage(szMsg, icon.c_str());
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


void CGameClientShell::OnLockRenderer()
{
	if (GetInterfaceMgr()->IsLoadScreenVisible())
	{
		GetInterfaceMgr()->PauseLoadScreen();
	}
}

void CGameClientShell::OnUnLockRenderer()
{
	if (GetInterfaceMgr()->IsLoadScreenVisible())
	{
		GetInterfaceMgr()->ResumeLoadScreen();
	}
}

void CGameClientShell::PostLevelLoadFirstUpdate()
{
	// Draw the screen once
	g_pPlayerMgr->UpdateCamera();
	RenderCamera(false);
	
	// Restore the game's music state.  (This can take a while...)
	RestoreMusic();
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
	LTFLOAT fLocalTime = g_pLTClient->GetTime();
	LTFLOAT fServerTime = g_pLTClient->GetGameTime();
	if(fServerTime - m_fInitialServerTime > 5.0f)
	{
		// time to do our check
		LTFLOAT fServerDelta	= fServerTime - m_fInitialServerTime;
		LTFLOAT fLocalDelta		= fLocalTime - m_fInitialLocalTime;

		if(fServerDelta / fLocalDelta < 0.98)
		{
			// possible cheater, increment cheat counter
			m_nSpeedCheatCounter++;

			if(m_nSpeedCheatCounter > 24)
			{
				// Disconnect from the server.
				if(g_pLTClient->IsConnected())
				{
					g_pLTClient->Disconnect();
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
//	ROUTINE:	CGameClientShell::SendClientLoadedMessage
//
//	PURPOSE:	Sent when client done with loading and postloading screens.
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SendClientLoadedMessage( )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENT_LOADED);
	g_pLTClient->SendToServer(cMsg.Read(),MESSAGE_GUARANTEED);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::LauncherServerApp()
//
//	PURPOSE:	Launches the serverapp.
//
// --------------------------------------------------------------------------- //

bool CGameClientShell::LauncherServerApp( char const* pszProfileFile )
{
	PROCESS_INFORMATION procInfo;
	STARTUPINFO startInfo;
	RMode rMode;

	// Save the current render mode.  We'll need to restore it if the serverapp
	// launching fails.
	g_pLTClient->GetRenderMode( &rMode );

	// Shutdown the renderer, minimize it, and hide it...
	g_pLTClient->ShutdownRender( 0 ); // RSHUTDOWN_MINIMIZEWINDOW | RSHUTDOWN_HIDEWINDOW );

	// Initialize the startup info.
	memset( &startInfo, 0, sizeof( STARTUPINFO ));
	startInfo.cb = sizeof( STARTUPINFO );

	// Setup the command line.
	std::string sCmdLine = "NOLF2Srv.exe -profile ";
	
	// Enclose the profile name in quotes since we allow spaces in the name...

	sCmdLine += "\"";
	sCmdLine += pszProfileFile;
	sCmdLine += "\"";

	// Start the server app.
	if( !CreateProcess( "NOLF2Srv.exe", ( char * )sCmdLine.c_str( ), NULL, NULL, FALSE, 0, NULL, NULL, 
		&startInfo, &procInfo ))
	{
		// Serverapp failed.  Restore the render mode.
		g_pLTClient->SetRenderMode( &rMode );
		return false;
	}

	// We're done with this process.
	g_pLTClient->Shutdown();

	return true;
}

void CGameClientShell::SetGameType(GameType eGameType)	
{
	if (m_eGameType == eGameType) return;

	switch( eGameType )
	{
		// Single player and coop use the same weapon files...

		case eGameTypeSingle:
		case eGameTypeCooperative:
		{
			g_pWeaponMgr->LoadOverrideButes( WEAPON_DEFAULT_FILE );
		}
		break;

		case eGameTypeDeathmatch:
		case eGameTypeTeamDeathmatch:
		case eGameTypeDoomsDay:
		{
			g_pWeaponMgr->LoadOverrideButes( WEAPON_DEFAULT_MULTI_FILE );
		}
		break;
		
	}
	m_eGameType = eGameType;
}