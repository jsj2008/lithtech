// ----------------------------------------------------------------------- //
//
// MODULE  : GameClientShell.cpp
//
// PURPOSE : Game Client Shell - Implementation
//
// CREATED : 9/18/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "CommandIds.h"
#include "WeaponModel.h"
#include "ClientUtilities.h"
#include "vkdefs.h"
#include "ClientRes.h"
#include "SoundTypes.h"
#include "Music.h"
#include "VolumeBrushFX.h"
#include "client_physics.h"
#include "CameraFX.h"
#include "WinUtil.h"
#include "WeaponStringDefs.h"
#include "CMoveMgr.h"
#include "iltmath.h"
#include "iltphysics.h"
#include "VarTrack.h"
#include "GameButes.h"
#include "LTWnd.h"
#include "LTMaskedWnd.h"
#include "AssertMgr.h"
#include "SystemDependant.h"
#include "SurfaceFunctions.h"
#include "VehicleMgr.h"
#include "BodyFX.h"
#include "PlayerShared.h"
#include "CharacterFX.h"

#include "CRC32.h"

#include <stdarg.h>
#include <stdio.h>

#ifdef STRICT
	WNDPROC g_pfnMainWndProc = NULL;
#else
	FARPROC	g_pfnMainWndProc = NULL;
#endif

#define min(a,b)	((a) < (b) ? (a) : (b))
#define max(a,b)	((a) > (b) ? (a) : (b))

#define FOVX_ZOOMED		20.0f
#define FOVX_ZOOMED1	7.0f
#define FOVX_ZOOMED2	2.0f
#define ZOOM_TIME		0.5f

#define DEFAULT_LOD_OFFSET				0.0f
#define LOD_ZOOMADJUST					-5.0f

#define MAX_SHAKE_AMOUNT				10.0f
#define MAX_FRAME_DELTA					0.1f

#define WEAPON_MOVE_INC_VALUE_SLOW		0.005f
#define WEAPON_MOVE_INC_VALUE_FAST		0.02f

#define DEFAULT_CSENDRATE				7.0f

#define VK_TOGGLE_EDITMODE				VK_F2
#define VK_TOGGLE_SCREENSHOTMODE		VK_F3

uint32              g_dwSpecial         = 2342349;
LTBOOL              g_bScreenShotMode   = LTFALSE;
HLTCOLOR            g_hColorTransparent = LTNULL;

HWND				g_hMainWnd = NULL;
RECT*				g_prcClip = NULL;

CGameClientShell*   g_pGameClientShell = LTNULL;

LTVector            g_vWorldWindVel(0.0f, 0.0f, 0.0f);
LTVector            g_vNVModelColor(0.0f, 0.0f, 0.0f);      // model color modifier for night vision
LTVector            g_vIRModelColor(0.0f, 0.0f, 0.0f);      // model color modifier for infrared

PhysicsState		g_normalPhysicsState;
PhysicsState		g_waterPhysicsState;

VarTrack			g_vtFOVXNormal;
VarTrack			g_vtFOVYNormal;
VarTrack			g_vtInterfceFOVX;
VarTrack			g_vtInterfceFOVY;
VarTrack			g_CV_CSendRate;		// The SendRate console variable.
VarTrack			g_vtPlayerRotate;	// The PlayerRotate console variable
VarTrack			g_vtShowTimingTrack;
VarTrack			g_vtNormalTurnRate;
VarTrack			g_vtFastTurnRate;
VarTrack			g_vtLookUpRate;
VarTrack			g_vtCameraSwayXFreq;
VarTrack			g_vtCameraSwayYFreq;
VarTrack			g_vtCameraSwayXSpeed;
VarTrack			g_vtCameraSwayYSpeed;
VarTrack			g_vtCameraSwayDuckMult;
VarTrack			g_vtChaseCamPitchAdjust;
VarTrack			g_vtChaseCamOffset;
VarTrack			g_vtChaseCamDistUp;
VarTrack			g_vtChaseCamDistBack;
VarTrack			g_vtActivateOverride;
VarTrack			g_vtCamDamage;
VarTrack			g_vtCamDamagePitch;
VarTrack			g_vtCamDamageRoll;
VarTrack			g_vtCamDamageTime1;
VarTrack			g_vtCamDamageTime2;
VarTrack			g_vtCamDamageVal;
VarTrack			g_vtCamDamagePitchMin;
VarTrack			g_vtCamDamageRollMin;
VarTrack			g_varStartLevelScreenFadeTime;
VarTrack			g_varStartLevelScreenFade;
VarTrack			g_vtUseCamRecoil;
VarTrack			g_vtCamRecoilRecover;
VarTrack			g_vtBaseCamRecoilPitch;
VarTrack			g_vtMaxCamRecoilPitch;
VarTrack			g_vtBaseCamRecoilYaw;
VarTrack			g_vtMaxCamRecoilYaw;
VarTrack			g_vtFireJitterDecayTime;
VarTrack			g_vtFireJitterMaxPitchDelta;
VarTrack            g_vtCamRotInterpTime;
VarTrack			g_vtRespawnWaitTime;
VarTrack			g_vtMultiplayerRespawnWaitTime;
VarTrack			g_vtUseSoundFilters;
VarTrack			g_vtScreenFadeInTime;
VarTrack			g_vtScreenFadeOutTime;
VarTrack			g_vtSpecial;
VarTrack			g_vtModelGlowTime;
VarTrack			g_vtModelGlowMin;
VarTrack			g_vtModelGlowMax;

VarTrack			g_vtSunZoomLevel1MaxDist;
VarTrack			g_vtSunZoomLevel2MaxDist;

VarTrack			g_vtFOVYMaxUW;
VarTrack			g_vtFOVYMinUW;
VarTrack			g_vtUWFOVRate;
VarTrack			g_vtPlayerName;

LTFLOAT             s_fDemoTime     = 0.0f;
LTFLOAT             s_fDeadTimer    = 0.0f;
LTFLOAT             s_fDeathDelay   = 0.0f;

LTVector            g_vPlayerCameraOffset = g_kvPlayerCameraOffset;

int					g_nCinSaveModelShadows = 0;

static uint8		s_nLastCamType = CT_FULLSCREEN;

extern CCheatMgr*	g_pCheatMgr;

void IRModelHook(ModelHookData *pData, void *pUser);
void NVModelHook(ModelHookData *pData, void *pUser);
void DefaultModelHook(ModelHookData *pData, void *pUser);
BOOL HookWindow();
void UnhookWindow();
void QuickLoadCallBack(LTBOOL bReturn, void *pData);

BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg);
LRESULT CALLBACK HookedWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern void CalcNonClipPos(LTVector & vPos, LTRotation & rRot);

// Setup..
SETUP_CLIENTSHELL();


IClientShell* CreateClientShell(ILTClient *pClientDE)
{
	// Hook up the AssertMgr

	CAssertMgr::Enable();

	// Get our ClientDE pointer

    g_pLTClient  = pClientDE;
    _ASSERT(g_pLTClient);

	CGameClientShell* pShell = debug_new(CGameClientShell);
	_ASSERT(pShell);

	// Init our LT subsystems

    g_pMathLT = g_pLTClient->GetMathLT();
    g_pModelLT = g_pLTClient->GetModelLT();
    g_pTransLT = g_pLTClient->GetTransformLT();
    g_pPhysicsLT = g_pLTClient->Physics();
	g_pBaseLT = static_cast<ILTCSBase*>(g_pLTClient);

	g_pPhysicsLT->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);

    return ((IClientShell*)pShell);
}

void DeleteClientShell(IClientShell *pInputShell)
{
	// Delete our client shell

	if (pInputShell)
	{
		debug_delete(((CGameClientShell*)pInputShell));
	}

	// Unhook the AssertMgr and let the CRT handle asserts once again

	CAssertMgr::Disable();
}

static LTBOOL LoadLeakFile(ILTClient *g_pLTClient, char *pFilename);


void LeakFileFn(int argc, char **argv)
{
	if (argc < 1)
	{
        g_pLTClient->CPrint("LeakFile <filename>");
		return;
	}

    if (LoadLeakFile(g_pLTClient, argv[0]))
	{
        g_pLTClient->CPrint("Leak file %s loaded successfully!", argv[0]);
	}
	else
	{
        g_pLTClient->CPrint("Unable to load leak file %s", argv[0]);
	}
}

LTBOOL ConnectToTcpIpAddress(ILTClient* pClientDE, char* sAddress);

void ConnectFn(int argc, char **argv)
{
	if (argc <= 0)
	{
        g_pLTClient->CPrint("Connect <tcpip address> (use '*' for local net)");
		return;
	}

    ConnectToTcpIpAddress(g_pLTClient, argv[0]);
}

void FragSelfFn(int argc, char **argv)
{
    HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_FRAG_SELF);
    g_pLTClient->EndMessage(hWrite);
}

void CheatFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->HandleCheat(argc, argv);
	}
}

void SunglassFn(int argc, char **argv)
{
	if (g_pInterfaceMgr)
	{
		int mode = (int)g_pInterfaceMgr->GetSunglassMode();
		mode++;
		if (mode == NUM_SUN_MODES)
		{
			g_pInterfaceMgr->SetSunglassMode(SUN_NONE);
            g_pLTClient->CPrint("sunglasses off");
		}
		else
		{
			g_pInterfaceMgr->SetSunglassMode((eSunglassMode)mode);
            g_pLTClient->CPrint("sunglasses %d",mode);
		}
	}
}

void ReloadWeaponAttributesFn(int argc, char **argv)
{
    g_pWeaponMgr->Reload(g_pLTClient);
    g_pLTClient->CPrint("Reloaded weapons attributes file...");
}

void ReloadSurfacesAttributesFn(int argc, char **argv)
{
    g_pSurfaceMgr->Reload(g_pLTClient);
    g_pLTClient->CPrint("Reloaded surface attributes file...");
}

void ReloadFXAttributesFn(int argc, char **argv)
{
    g_pFXButeMgr->Reload(g_pLTClient);
    g_pLTClient->CPrint("Reloaded fx attributes file...");

	// Make sure we re-load the weapons and surface data, it has probably
	// changed...
	ReloadWeaponAttributesFn(0, 0);
	ReloadSurfacesAttributesFn(0, 0);
}

void RecordFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->HandleRecord(argc, argv);
	}
}

void PlayDemoFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->HandlePlaydemo(argc, argv);
	}
}

void ExitLevelFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->HandleExitLevel(argc, argv);
	}
}

void ChaseToggleFn(int argc, char **argv)
{
	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_CHASETOGGLE);
	}
}

void ChangeTeamFn(int argc, char **argv)
{
    uint8 team = 0;
	if (argc >= 1)
	{
        team = (uint8)atoi(argv[0]);
	}

    HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_PLAYER_CHANGETEAM);
    g_pLTClient->WriteToMessageByte(hWrite, team);
    g_pLTClient->EndMessage(hWrite);

}

void ExitGame(LTBOOL bResponse, uint32 nUserData)
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
        g_pLTClient->CPrint("    I  <intensity number to set> [when to enact change]");
        g_pLTClient->CPrint("    PS <segment name> [when to begin playing]");
        g_pLTClient->CPrint("    PM <motif style> <motif name> [when to begin playing]");
        g_pLTClient->CPrint("    V  <volume adjustment in db>");
        g_pLTClient->CPrint("    SS <segment name> [when to stop]");
        g_pLTClient->CPrint("    SM <motif name> [when to stop]");
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

	g_pGameClientShell = this;

	m_fFrameTime			= 0.0f;

	g_vWorldWindVel.Init();

    m_hCamera               = LTNULL;
    m_hInterfaceCamera      = LTNULL;

    m_bUseWorldFog          = LTTRUE;
    m_bMainWindowMinimized  = LTFALSE;

    m_bStrafing             = LTFALSE;
    m_bHoldingMouseLook     = LTFALSE;

    m_bGamePaused           = LTFALSE;
	m_resSoundInit			= LT_ERROR;

	m_fYawBackup				= 0.0f;
	m_fPitchBackup				= 0.0f;
    m_bRestoreOrientation       = LTFALSE;
    m_bAllowPlayerMovement      = LTTRUE;
    m_bLastAllowPlayerMovement  = LTTRUE;
    m_bWasUsingExternalCamera   = LTFALSE;
    m_bUsingExternalCamera      = LTFALSE;
	m_bCamIsListener			= LTFALSE;

    m_bNightVision  = LTFALSE;
	m_vNVScreenTint.Init(0.0f, 0.0f, 0.0f);

	m_vDefaultLightScale.Init(1.0f, 1.0f, 1.0f);

	m_nPlayerInfoChangeFlags	= 0;
	m_fPlayerInfoLastSendTime	= 0.0f;

    m_rRotation.Init();

	m_fPitch			= 0.0f;
	m_fYaw				= 0.0f;
	m_fRoll				= 0.0f;

	m_fPlayerPitch		= 0.0f;
	m_fPlayerYaw		= 0.0f;
	m_fPlayerRoll		= 0.0f;

	m_fFireJitterPitch	= 0.0f;
	m_fFireJitterYaw	= 0.0f;

	m_dwPlayerFlags			= 0;
	m_ePlayerState			= PS_UNKNOWN;
    m_bSpectatorMode        = LTFALSE;
    m_bTweakingWeapon       = LTFALSE;
    m_bTweakingWeaponMuzzle = LTFALSE;
    m_bAdjustWeaponBreach   = LTFALSE;
    m_bAdjust1stPersonCamera = LTFALSE;

	m_vShakeAmount.Init();

    m_bFlashScreen      = LTFALSE;
	m_fFlashTime		= 0.0f;
	m_fFlashStart		= 0.0f;
	m_fFlashRampUp		= 0.0f;
	m_fFlashRampDown	= 0.0f;
	m_vFlashColor.Init();

	memset(m_strCurrentWorldName, 0, 256);

	m_vIRLightScale.Init(1.0f,1.0,1.0f);
	m_vCurContainerLightScale.Init(-1.0f, -1.0f, -1.0f);

	m_nZoomView				= 0;
    m_bZooming              = LTFALSE;
    m_bZoomingIn            = LTFALSE;
	m_fSaveLODScale			= DEFAULT_LOD_OFFSET;
    m_bInWorld              = LTFALSE;
    m_bStartedLevel         = LTFALSE;
	m_nCurrentLevel			= 0;
	m_nCurrentMission		= 0;
	m_nMPNameId				= 0;
	m_nMPBriefingId			= 0;


    m_bStartedDuckingDown   = LTFALSE;
    m_bStartedDuckingUp     = LTFALSE;
	m_fCamDuck				= 0.0f;
	m_fDuckDownV			= -75.0f;
	m_fDuckUpV				= 75.0f;
	m_fMaxDuckDistance		= -20.0f;
	m_fStartDuckTime		= 0.0f;

    m_h3rdPersonCrosshair   = LTNULL;
    m_hContainerSound       = LTNULL;
	m_eCurContainerCode		= CC_NO_CONTAINER;
	m_nSoundFilterId		= 0;
	m_nGlobalSoundFilterId	= 0;

    m_bShowPlayerPos        = LTFALSE;
	m_bShowCamPosRot		= LTFALSE;
    m_hDebugInfo            = LTNULL;

    m_bAdjustLightScale     = LTFALSE;
    m_bAdjustLightAdd       = LTFALSE;
    m_bAdjustFOV            = LTFALSE;

	m_fContainerStartTime	= -1.0f;
	m_fFovXFXDir			= 1.0f;

    m_bPanSky               = LTFALSE;
	m_fPanSkyOffsetX		= 1.0f;
	m_fPanSkyOffsetZ		= 1.0f;
	m_fPanSkyScaleX			= 1.0f;
	m_fPanSkyScaleZ			= 1.0f;
	m_fCurSkyXOffset		= 0.0f;
	m_fCurSkyZOffset		= 0.0f;

	m_vCurModelGlow.Init(127.0f, 127.0f, 127.0f);
	m_vMaxModelGlow.Init(255.0f, 255.0f, 255.0f);
	m_vMinModelGlow.Init(50.0f, 50.0f, 50.f);
	m_fModelGlowCycleTime	= 0.0f;
    m_bModelGlowCycleUp     = LTTRUE;

    m_bFirstUpdate          = LTFALSE;
    m_bRestoringGame        = LTFALSE;
    m_bQuickSave            = LTFALSE;

    m_bCameraPosInited      = LTFALSE;

	m_eDifficulty			= GD_NORMAL;
	m_bFadeBodies			= LTFALSE;
	m_eGameType				= SINGLE;
	m_eLevelEnd				= LE_UNKNOWN;
	m_nEndString			= 0;

	m_fEarliestRespawnTime	= 0.0f;
    m_hBoundingBox          = LTNULL;

    m_bMainWindowFocus      = LTFALSE;

    m_bPlayerPosSet         = LTFALSE;

	m_fNextSoundReverbTime	= 0.0f;
    m_bUseReverb            = LTFALSE;
	m_fReverbLevel			= 0.0f;
	m_bCameraAttachedToHead	= LTFALSE;

	m_vLastReverbPos.Init();

	m_szServerAddress[0]	= LTNULL;
	m_nServerPort			= -1;
	m_szServerName[0]		= LTNULL;
	memset(m_fServerOptions, 0, sizeof(m_fServerOptions));

	m_bForceDisconnect		= LTFALSE;

	m_nDisconnectCode = 0;
	m_nDisconnectSubCode = 0;
	m_pDisconnectMsg = LTNULL;

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
	if (m_hDebugInfo)
	{
        g_pLTClient->DeleteSurface(m_hDebugInfo);
		m_hDebugInfo = NULL;
	}

	if (m_hBoundingBox)
	{
        g_pLTClient->DeleteObject(m_hBoundingBox);
	}

	if (g_prcClip)
	{
		debug_delete(g_prcClip);
        g_prcClip = LTNULL;
	}

	if (m_pDisconnectMsg)
	{
		debug_deletea(m_pDisconnectMsg);
		m_pDisconnectMsg = LTNULL;
	}

    g_pGameClientShell = LTNULL;
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
	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (!pSettings) return;

	Sound3DProvider *pSound3DProviderList, *pSound3DProvider;
	InitSoundInfo soundInfo;
	ReverbProperties reverbProperties;
	HCONSOLEVAR hVar;
    uint32 dwProviderID;
	char sz3dSoundProviderName[_MAX_PATH + 1];
	int nError;

	m_resSoundInit = LT_ERROR;

    uint32 dwAdvancedOptions = m_InterfaceMgr.GetAdvancedOptions();
	if (!(dwAdvancedOptions & AO_SOUND)) return;

    soundInfo.Init();

	// Reload the sounds if there are any...

	soundInfo.m_dwFlags	= INITSOUNDINFOFLAG_RELOADSOUNDS;

	// Get the 3d sound provider id....

    hVar = g_pLTClient->GetConsoleVar("3DSoundProvider");
	if (hVar)
	{
        dwProviderID = ( uint32 )g_pLTClient->GetVarValueFloat( hVar );
	}
	else
	{
		dwProviderID = SOUND3DPROVIDERID_NONE;
	}

	// Can also be set by provider name, in which case the id will be set to
	// UNKNOWN...

	if ( dwProviderID == SOUND3DPROVIDERID_NONE ||
		 dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
	{
		sz3dSoundProviderName[0] = 0;
        hVar = g_pLTClient->GetConsoleVar("3DSoundProviderName");
		if ( hVar )
		{
            SAFE_STRCPY( sz3dSoundProviderName, g_pLTClient->GetVarValueString( hVar ));
			dwProviderID = SOUND3DPROVIDERID_UNKNOWN;
		}
	}

	// See if the provider exists....

	if ( dwProviderID != SOUND3DPROVIDERID_NONE )
	{
        g_pLTClient->GetSound3DProviderLists( pSound3DProviderList, LTFALSE );
		if ( !pSound3DProviderList )
		{
			m_resSoundInit = LT_NO3DSOUNDPROVIDER;
			return;
		}

		pSound3DProvider = pSound3DProviderList;
		while ( pSound3DProvider )
		{
			// If the provider is selected by name, then compare the names.
			if (  dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
			{
				if ( _mbscmp(( const unsigned char * )sz3dSoundProviderName, ( const unsigned char * )pSound3DProvider->m_szProvider ) == 0 )
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

			// Get the maximum number of 3d voices to use.
            hVar = g_pLTClient->GetConsoleVar("Max3DVoices");
			if (hVar)
			{
                soundInfo.m_nNum3DVoices = (uint8)g_pLTClient->GetVarValueFloat(hVar);
			}
			else
			{
				soundInfo.m_nNum3DVoices = 16;
			}
		}

        g_pLTClient->ReleaseSound3DProviderList(pSound3DProviderList);
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

	soundInfo.m_nSampleRate		= 22050;
	soundInfo.m_nBitsPerSample	= 16;
	soundInfo.m_nVolume			= (unsigned short)pSettings->SoundVolume();

	if ( !pSettings->Sound16Bit( ) )
	{
		soundInfo.m_dwFlags |= INITSOUNDINFOFLAG_CONVERT16TO8;
	}

	soundInfo.m_fDistanceFactor = 1.0f / 64.0f;

	// Go initialize the sounds...

    m_bUseReverb = LTFALSE;
    if ((m_resSoundInit = g_pLTClient->InitSound(&soundInfo)) == LT_OK)
	{
		if (soundInfo.m_dwResults & INITSOUNDINFORESULTS_REVERB)
		{
            m_bUseReverb = LTTRUE;
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

		reverbProperties.m_dwParams = REVERBPARAM_VOLUME;
		reverbProperties.m_fVolume = m_fReverbLevel;
        g_pLTClient->SetReverbProperties(&reverbProperties);
	}
	else
	{
		if (m_resSoundInit == LT_NO3DSOUNDPROVIDER)
		{
			nError = IDS_INVALID3DSOUNDPROVIDER;
		}
		else
		{
			nError = IDS_SOUNDNOTINITED;
		}

		// DoMessageBox( nError, TH_ALIGN_CENTER );
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PreChangeGameState
//
//	PURPOSE:	Handle pre setting of game state
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::PreChangeGameState(GameState eNewState)
{
	switch (eNewState)
	{
		case GS_FOLDER :
		{
			if (m_bUsingExternalCamera)
			{
				TurnOffAlternativeCamera(CT_FULLSCREEN);
                m_bCameraPosInited = LTFALSE;

				// Special case, we want to still be using the external camera
				// when we return to the game...

                m_bUsingExternalCamera = LTTRUE;
			}
		}
		break;

		default : break;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PostChangeGameState
//
//	PURPOSE:	Handle post setting of game state
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::PostChangeGameState(GameState eOldState)
{
    return LTTRUE;
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
	m_InterfaceMgr.GetMessageMgr()->AddLine(pMsg);
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
	//CWinUtil::DebugBreak();

	*pAppGuid = NOLFGUID;

    char strTimeDiff[64];
	float fStartTime = CWinUtil::GetTime();


	if (!g_hMainWnd)
	{
		HookWindow();
	}

	// Initialize all the global bute mgrs...

	if (!m_GlobalMgr.Init())
	{
		return LT_ERROR;
	}

	// Initialize global console variables...

    g_vtFOVXNormal.Init(g_pLTClient, "FovX", NULL, 90.0f);
    g_vtFOVYNormal.Init(g_pLTClient, "FovY", NULL, 78.0f);

    g_vtInterfceFOVX.Init(g_pLTClient, "FovXInterface", NULL, 90.0f);
    g_vtInterfceFOVY.Init(g_pLTClient, "FovYInterface", NULL, 75.0f);

    g_CV_CSendRate.Init(g_pLTClient, "CSendRate", NULL, DEFAULT_CSENDRATE);
    g_vtPlayerRotate.Init(g_pLTClient, "PlayerRotate", NULL, 1.0f);

    g_vtShowTimingTrack.Init(g_pLTClient, "ShowTiming", NULL, 0.0f);

    g_vtFastTurnRate.Init(g_pLTClient, "FastTurnRate", NULL, 2.3f);
    g_vtNormalTurnRate.Init(g_pLTClient, "NormalTurnRate", NULL, 1.5f);
    g_vtLookUpRate.Init(g_pLTClient, "LookUpRate", NULL, 2.5f);

    g_vtCameraSwayXFreq.Init(g_pLTClient, "CameraSwayXFreq", NULL, 13.0f);
    g_vtCameraSwayYFreq.Init(g_pLTClient, "CameraSwayYFreq", NULL, 5.0f);
    g_vtCameraSwayXSpeed.Init(g_pLTClient, "CameraSwayXSpeed", NULL, 12.0f);
    g_vtCameraSwayYSpeed.Init(g_pLTClient, "CameraSwayYSpeed", NULL, 1.5f);
    g_vtCameraSwayDuckMult.Init(g_pLTClient, "CameraSwayCrouchMultiplier", NULL, 0.5f);

    g_vtChaseCamOffset.Init(g_pLTClient, "ChaseCamOffset", NULL, 50.0f);
    g_vtChaseCamPitchAdjust.Init(g_pLTClient, "ChaseCamPitchAdjust", NULL, 0.0f);
    g_vtChaseCamDistUp.Init(g_pLTClient, "ChaseCamDistUp", NULL, 10.0f);
    g_vtChaseCamDistBack.Init(g_pLTClient, "ChaseCamDistBack", NULL, 100.0f);

    g_vtActivateOverride.Init(g_pLTClient, "ActivateOverride", " ", 0.0f);

    g_vtCamDamage.Init(g_pLTClient, "CamDamage", NULL, 1.0f);
    g_vtCamDamagePitch.Init(g_pLTClient, "CamDamagePitch", NULL, 1.0f);
    g_vtCamDamageRoll.Init(g_pLTClient, "CamDamageRoll", NULL, 1.0f);
    g_vtCamDamageTime1.Init(g_pLTClient, "CamDamageTime1", NULL, 0.1f);
    g_vtCamDamageTime2.Init(g_pLTClient, "CamDamageTime2", NULL, 0.25f);
    g_vtCamDamageVal.Init(g_pLTClient, "CamDamageVal", NULL, 5.0f);
    g_vtCamDamagePitchMin.Init(g_pLTClient, "CamDamagePitchMin", NULL, 0.7f);
    g_vtCamDamageRollMin.Init(g_pLTClient, "CamDamageRollMin", NULL, 0.7f);

    g_varStartLevelScreenFade.Init(g_pLTClient, "ScreenFadeAtLevelStart", NULL, 1.0f);
    g_varStartLevelScreenFadeTime.Init(g_pLTClient, "ScreenFadeInAtLevelStartTime", NULL, 3.0f);
	g_vtScreenFadeInTime.Init(g_pLTClient, "ScreenFadeInTime", LTNULL, 3.0f);
	g_vtScreenFadeOutTime.Init(g_pLTClient, "ScreenFadeOutTime", LTNULL, 5.0f);

    g_vtUseCamRecoil.Init(g_pLTClient, "CamRecoil", NULL, 0.0f);
    g_vtCamRecoilRecover.Init(g_pLTClient, "CamRecoilRecover", NULL, 0.3f);
    g_vtBaseCamRecoilPitch.Init(g_pLTClient, "CamRecoilBasePitch", NULL, 5.0f);
    g_vtMaxCamRecoilPitch.Init(g_pLTClient, "CamRecoilMaxPitch", NULL, 75.0f);
    g_vtBaseCamRecoilYaw.Init(g_pLTClient, "CamRecoilBaseYaw", NULL, 3.0f);
    g_vtMaxCamRecoilYaw.Init(g_pLTClient, "CamRecoilMaxYaw", NULL, 35.0f);
    g_vtCamRotInterpTime.Init(g_pLTClient, "CamRotInterpTime", NULL, 0.15f);

	g_vtRespawnWaitTime.Init(g_pLTClient, "RespawnWaitTime", NULL, 1.0f);
	g_vtMultiplayerRespawnWaitTime.Init(g_pLTClient, "RespawnMultiWaitTime", NULL, 0.5f);

	g_vtUseSoundFilters.Init(g_pLTClient, "SoundFilters", LTNULL, 0.0f);

	g_vtSpecial.Init(g_pLTClient, "ShowSpecial", LTNULL, 0.0f);

	g_vtModelGlowTime.Init(g_pLTClient, "ModelGlowTime", LTNULL, 1.5f);
	g_vtModelGlowMin.Init(g_pLTClient, "ModelGlowMin", LTNULL, -25.0f);
	g_vtModelGlowMax.Init(g_pLTClient, "ModelGlowMax", LTNULL, 75.0f);

	g_vtSunZoomLevel1MaxDist.Init(g_pLTClient, "SunZoomLevel1MaxDist", LTNULL, 100.0f);
	g_vtSunZoomLevel2MaxDist.Init(g_pLTClient, "SunZoomLevel2MaxDist", LTNULL, 700.0f);

	// Default these to use values specified in the weapon...(just here for
	// tweaking values...)

    g_vtFireJitterDecayTime.Init(g_pLTClient, "FireJitterDecayTime", NULL, -1.0f);
    g_vtFireJitterMaxPitchDelta.Init(g_pLTClient, "FireJitterMaxPitchDelta", NULL, -1.0f);

    HSTRING hStr = g_pLTClient->FormatString(IDS_PLAYER);
    g_vtPlayerName.Init(g_pLTClient, "NetPlayerName", g_pLTClient->GetStringData(hStr), 0.0f);
	g_pLTClient->FreeString (hStr);


	g_vtFOVYMaxUW.Init(g_pLTClient, "FOVYUWMax", NULL, 78.0f);
    g_vtFOVYMinUW.Init(g_pLTClient, "FOVYUWMin", NULL, 77.0f);
    g_vtUWFOVRate.Init(g_pLTClient, "FOVUWRate", NULL, 0.3f);

    HCONSOLEVAR hIsSet = g_pLTClient->GetConsoleVar("UpdateRateInitted");
    if (!hIsSet || g_pLTClient->GetVarValueFloat(hIsSet) != 1.0f)
	{
		// Initialize the update rate.
        g_pLTClient->RunConsoleString("+UpdateRateInitted 1");
        g_pLTClient->RunConsoleString("+UpdateRate 6");
	}

    m_MoveMgr.Init();
	m_editMgr.Init();
	m_cheatMgr.Init();
	m_LightScaleMgr.Init();

    m_AttachButeMgr.Init(g_pLTClient);

	m_CameraOffsetMgr.Init();
	m_HeadBobMgr.Init();

    g_pLTClient->RegisterConsoleProgram("Cheat", CheatFn);
    g_pLTClient->RegisterConsoleProgram("Sunglass", SunglassFn);
    g_pLTClient->RegisterConsoleProgram("LeakFile", LeakFileFn);
//  g_pLTClient->RegisterConsoleProgram("Connect", ConnectFn);
    g_pLTClient->RegisterConsoleProgram("FragSelf", FragSelfFn);
    g_pLTClient->RegisterConsoleProgram("ReloadWeapons", ReloadWeaponAttributesFn);
    g_pLTClient->RegisterConsoleProgram("ReloadSurfaces", ReloadSurfacesAttributesFn);
    g_pLTClient->RegisterConsoleProgram("ReloadFX", ReloadFXAttributesFn);
    g_pLTClient->RegisterConsoleProgram("Record", RecordFn);
    g_pLTClient->RegisterConsoleProgram("PlayDemo", PlayDemoFn);
    g_pLTClient->RegisterConsoleProgram("InitSound", InitSoundFn);
    g_pLTClient->RegisterConsoleProgram("ExitLevel", ExitLevelFn);
    g_pLTClient->RegisterConsoleProgram("ChaseToggle", ChaseToggleFn);
    g_pLTClient->RegisterConsoleProgram("ChangeTeam", ChangeTeamFn);
    g_pLTClient->RegisterConsoleProgram("Music", MusicFn);

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

    LTBOOL bNetworkGameStarted = LTFALSE;

	// Initialize the renderer
    LTRESULT hResult = g_pLTClient->SetRenderMode(pMode);
	if (hResult != LT_OK)
	{
        g_pLTClient->DebugOut("%s Error: Couldn't set render mode!\n", GAME_NAME);

		RMode rMode;

		// If an error occurred, try 640x480x16...

		rMode.m_Width		= 640;
		rMode.m_Height		= 480;
		rMode.m_BitDepth	= 16;
		rMode.m_bHardware	= pMode->m_bHardware;

		sprintf(rMode.m_RenderDLL, "%s", pMode->m_RenderDLL);
		sprintf(rMode.m_InternalName, "%s", pMode->m_InternalName);
		sprintf(rMode.m_Description, "%s", pMode->m_Description);

        g_pLTClient->DebugOut("Setting render mode to 640x480x16...\n");

        if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
		{
			// Okay, that didn't work, looks like we're stuck with software...

            rMode.m_bHardware = LTFALSE;

			sprintf(rMode.m_RenderDLL, "d3d.ren");
			sprintf(rMode.m_InternalName, "");
			sprintf(rMode.m_Description, "");

            g_pLTClient->DebugOut("Setting render mode to D3D Emulation mode...\n");

            if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
			{
                g_pLTClient->DebugOut("%s Error: Couldn't set to D3D Emulation mode.\nShutting down %s...\n", GAME_NAME, GAME_NAME);
                g_pLTClient->ShutdownWithMessage("%s Error: Couldn't set D3D Emulation mode.\nShutting down %s...\n", GAME_NAME, GAME_NAME);
				return LT_ERROR;
			}
		}
	}

	// Setup the global transparency color

	g_hColorTransparent = SETRGB_T(255,0,255);


	// Setup the music stuff...(before we setup the interface!)

    uint32 dwAdvancedOptions = m_InterfaceMgr.GetAdvancedOptions();

	if (!m_Music.IsInitialized() && (dwAdvancedOptions & AO_MUSIC))
	{
        m_Music.Init(g_pLTClient);
	}


	// Interface stuff...

	if (!m_InterfaceMgr.Init())
	{
		// Don't call ShutdownWithMessage since InterfaceMgr will have called
		// that, so calling it here will overwrite the message...
 		return LT_ERROR;
	}

    if (!m_PlayerSummary.Init(g_pLTClient))
	{
        char errorBuf[256];
		sprintf(errorBuf, "ERROR in CGameClientShell::OnEngineInitialized()\n\nCouldn't initialize PlayerSummaryMgr!");
        g_pLTClient->ShutdownWithMessage(errorBuf);
		return LT_ERROR;
	}

    if (!m_IntelItemMgr.Init())
	{
        char errorBuf[256];
		sprintf(errorBuf, "ERROR in CGameClientShell::OnEngineInitialized()\n\nCouldn't initialize IntelItemMgr!");
        g_pLTClient->ShutdownWithMessage(errorBuf);
		return LT_ERROR;
	}


	//init these here because it needs to access layout mgr through interface mgr
	s_fDeathDelay = g_pLayoutMgr->GetDeathDelay();
	g_vNVModelColor = g_pLayoutMgr->GetNightVisionModelColor();
	m_vNVScreenTint = g_pLayoutMgr->GetNightVisionScreenTint();
	m_vIRLightScale = g_pLayoutMgr->GetInfraredLightScale();
	g_vIRModelColor = g_pLayoutMgr->GetInfraredModelColor();

	m_weaponModel.Init();


	// Create the camera...

    uint32 dwWidth = 640;
    uint32 dwHeight = 480;

    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_ObjectType = OT_CAMERA;

    m_hCamera = g_pLTClient->CreateObject(&theStruct);
	_ASSERT(m_hCamera);

    g_pLTClient->SetCameraRect(m_hCamera, LTFALSE, 0, 0, dwWidth, dwHeight);
	SetCameraFOV(DEG2RAD(g_vtFOVXNormal.GetFloat()), DEG2RAD(g_vtFOVYNormal.GetFloat()));

	// Create the Interface camera...

    m_hInterfaceCamera = g_pLTClient->CreateObject(&theStruct);
	_ASSERT(m_hInterfaceCamera);

    g_pLTClient->SetCameraRect(m_hInterfaceCamera, LTFALSE, 0, 0, dwWidth, dwHeight);
    g_pLTClient->SetCameraFOV(m_hInterfaceCamera, DEG2RAD(g_vtInterfceFOVX.GetFloat()), DEG2RAD(g_vtInterfceFOVY.GetFloat()));

	// Initialize the global physics states...

	g_normalPhysicsState.m_vGravityAccel.Init(0.0f, -1000.0f, 0.0f);
	g_normalPhysicsState.m_fVelocityDampen = 0.5f;
	g_waterPhysicsState.m_vGravityAccel.Init(0.0f, -500.0f, 0.0f);
	g_waterPhysicsState.m_fVelocityDampen = 0.25f;

	// Player camera (non-1st person) stuff...

    if (m_PlayerCamera.Init(g_pLTClient))
	{
		InitPlayerCamera();
		m_PlayerCamera.GoFirstPerson();
	}
	else
	{
		CSPrint ("Could not init player camera!");
	}


	// Init the special fx mgr...
    if (!m_sfxMgr.Init(g_pLTClient))
	{
        g_pLTClient->ShutdownWithMessage("Could not initialize SFXMgr!");
		return LT_ERROR;
	}

	// Init the damage fx mgr...

    if (!m_DamageFXMgr.Init(g_pLTClient))
	{
        g_pLTClient->ShutdownWithMessage("Could not initialize DamageFXMgr!");
		return LT_ERROR;
	}


	// [blg] Check for a multiplayer launch...

	LTBOOL bMultiConnect = LTFALSE;
	LTBOOL bMultiLaunch  = LTFALSE;

	char* sConnect = NULL;

    if (hVar = g_pLTClient->GetConsoleVar("connect"))
	{
		sConnect = g_pLTClient->GetVarValueString(hVar);
		if (sConnect)
		{
			bMultiConnect = LTTRUE;
		}
	}

    if (hVar = g_pLTClient->GetConsoleVar("multiplayer"))
	{
		LTFLOAT fMulti = g_pLTClient->GetVarValueFloat(hVar);
		if (fMulti != 0.0f)
		{
			bMultiLaunch = LTTRUE;
		}
	}

	if (bMultiConnect || bMultiLaunch)
	{
		bNetworkGameStarted = LTTRUE;
	}


	// Okay, start the game...

	if (!bNetworkGameStarted)
	{
		HCONSOLEVAR hVar;

        if (hVar = g_pLTClient->GetConsoleVar("NumConsoleLines"))
		{
			// UNCOMMENT this for final builds...Show console output
			// for debugging...
            // g_pLTClient->RunConsoleString("+NumConsoleLines 0");
		}

        if (hVar = g_pLTClient->GetConsoleVar("runworld"))
		{
            if (!LoadWorld(g_pLTClient->GetVarValueString(hVar)))
			{
                HSTRING hString = g_pLTClient->FormatString(IDS_NOLOADLEVEL);
                g_pLTClient->ShutdownWithMessage(g_pLTClient->GetStringData(hString));
                g_pLTClient->FreeString(hString);
				return LT_ERROR;
			}
		}
		else if (GetConsoleInt("SkipTitle",0))
		{
			m_InterfaceMgr.SwitchToFolder(FOLDER_ID_MAIN);
		}
		else
		{
			m_InterfaceMgr.ChangeState(GS_SPLASHSCREEN);
		}
	}
	else
	{
		if (bMultiConnect && sConnect)
		{
			if (!DoJoinGame(sConnect))
			{
				m_InterfaceMgr.LoadFailed();
				m_InterfaceMgr.SwitchToFolder(FOLDER_ID_MAIN);
				HSTRING hString = g_pLTClient->FormatString(IDS_CANT_CONNECT_TO_SERVER);
				if (hString)
				{
					m_InterfaceMgr.ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
					g_pLTClient->FreeString(hString);
				}
			}
		}
		else if (bMultiLaunch)
		{
			m_InterfaceMgr.SwitchToFolder(FOLDER_ID_MAIN);
		}
		else
		{
			m_InterfaceMgr.ChangeState(GS_SPLASHSCREEN);
		}
	}



	// Determine how long it took to initialize the game...
	sprintf(strTimeDiff, "Game initialized in %f seconds.\n", CWinUtil::GetTime() - fStartTime);
	CWinUtil::DebugOut(strTimeDiff);

	int nDiff = GetConsoleInt("Difficulty",1);
	if (nDiff < GD_EASY)
		nDiff = GD_EASY;
	else if (nDiff > GD_VERYHARD)
		nDiff = GD_VERYHARD;

	m_eDifficulty = (GameDifficulty)nDiff;
	m_bFadeBodies = (LTBOOL)GetConsoleInt("FadeBodies",0);

	// Check for playdemo....

    if (hVar = g_pLTClient->GetConsoleVar("playdemo"))
	{
        DoLoadWorld("", NULL, NULL, LOAD_NEW_GAME, NULL, g_pLTClient->GetVarValueString(hVar));
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

	if (m_hCamera)
	{
        g_pLTClient->DeleteObject(m_hCamera);
        m_hCamera = LTNULL;
	}

	if (m_hInterfaceCamera)
	{
        g_pLTClient->DeleteObject(m_hInterfaceCamera);
        m_hInterfaceCamera = LTNULL;
	}

	m_InterfaceMgr.Term();

	m_Music.Term();
	m_LightScaleMgr.Term();
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
			if (g_pGameClientShell->IsMultiplayerGame())
			{
				m_szServerAddress[0]	= LTNULL;
				m_nServerPort			= -1;
				m_szServerName[0]		= LTNULL;
				memset(m_fServerOptions, 0, sizeof(m_fServerOptions));
			}
			m_bInWorld = LTFALSE;

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
		}
		break;

		case LTEVENT_RENDERINIT:
		{
			m_bMainWindowFocus = TRUE;

			if (!g_hMainWnd)
			{
				HookWindow();
			}

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

				GetWindowRect(g_hMainWnd, g_prcClip);
				ClipCursor(g_prcClip);
			}

			g_pInterfaceMgr->InitCursor();
		}
		break;
	}

	m_InterfaceMgr.OnEvent(dwEventID, dwParam);
}


LTRESULT CGameClientShell::OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector *pPos)
{
	return m_MoveMgr.OnObjectMove(hObj, bTeleport, pPos);
}


LTRESULT CGameClientShell::OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation *pNewRot)
{
	return m_MoveMgr.OnObjectRotate(hObj, bTeleport, pNewRot);
}


LTRESULT CGameClientShell::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, float forceMag)
{
	m_sfxMgr.OnTouchNotify(hMain, pInfo, forceMag);
	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PreLoadWorld()
//
//	PURPOSE:	Called before world loads
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PreLoadWorld(char *pWorldName)
{
	if (IsMainWindowMinimized())
	{
//		NetStart_RestoreMainWnd();
		RestoreMainWindow();
	}

	SAFE_STRCPY(m_strCurrentWorldName, pWorldName);
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
	g_pLTClient->ResumeSounds();

	g_pPhysicsLT->SetStairHeight(DEFAULT_STAIRSTEP_HEIGHT);

	m_bFirstUpdate = LTTRUE;
	m_ePlayerState = PS_UNKNOWN;

	m_vShakeAmount.Init();

    m_bPlayerPosSet = LTFALSE;
    m_bInWorld      = LTTRUE;
	m_nZoomView		= 0;
    m_bZooming      = LTFALSE;
    m_bZoomingIn    = LTFALSE;

	m_eCurContainerCode		= CC_NO_CONTAINER;

	m_bCameraAttachedToHead = LTFALSE;

	m_LightScaleMgr.Init();
	m_InterfaceMgr.AddToClearScreenCount();

    g_pLTClient->ClearInput();

	m_HeadBobMgr.OnEnterWorld();
	m_InterfaceMgr.OnEnterWorld(m_bRestoringGame);

    SetExternalCamera(LTFALSE);

    m_bRestoringGame        = LTFALSE;
    m_bCameraPosInited      = LTFALSE;
	m_nPlayerInfoChangeFlags |= CLIENTUPDATE_PLAYERROT | CLIENTUPDATE_ALLOWINPUT;

	m_vLastReverbPos.Init();

	m_MoveMgr.OnEnterWorld();
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
    g_pLTClient->PauseSounds();

    m_bInWorld      = LTFALSE;
    m_bStartedLevel = LTFALSE;

	m_LightScaleMgr.Term();
	ClearScreenTint();
    HandleZoomChange(m_weaponModel.GetWeaponId(), LTTRUE);
	EndZoom();
	m_InterfaceMgr.EndUnderwater();

	memset(m_strCurrentWorldName, 0, 256);

	if (m_h3rdPersonCrosshair)
	{
        g_pLTClient->DeleteObject(m_h3rdPersonCrosshair);
        m_h3rdPersonCrosshair = LTNULL;
	}

	m_DamageFXMgr.Clear();					// Remove all the sfx
	m_sfxMgr.RemoveAll();					// Remove all the sfx
    m_PlayerCamera.AttachToObject(LTNULL);   // Detatch camera

	m_weaponModel.Reset();
	m_FlashLight.TurnOff();

	if (m_hContainerSound)
	{
        g_pLTClient->KillSound(m_hContainerSound);
        m_hContainerSound = LTNULL;
	}

	m_InterfaceMgr.OnExitWorld();

	m_MoveMgr.OnExitWorld();
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
	// Conditions in which we don't want to clear the screen

	if ((m_ePlayerState == PS_UNKNOWN && m_bInWorld))
	{
		return;
	}

	// See if we're using an external camera now - if so, clear the screen
	// immediately, and add to the clearscreen count
	if (m_bUsingExternalCamera && !m_bWasUsingExternalCamera)
	{
        m_bWasUsingExternalCamera = LTTRUE;
		m_InterfaceMgr.AddToClearScreenCount();
	}
	else if (m_bWasUsingExternalCamera && !m_bUsingExternalCamera)
	{
        m_bWasUsingExternalCamera = LTFALSE;
		m_InterfaceMgr.AddToClearScreenCount();
	}

	m_InterfaceMgr.PreUpdate();
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
	// This will happen when something wanted to disconnect, but wasn't
	// in a valid location to do so.  (e.g. when processing packets..)
	if (m_bForceDisconnect)
	{
		g_pLTClient->Disconnect();
		m_bForceDisconnect = LTFALSE;
		return;
	}

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


	// Update models (powerups) glowing...

	UpdateModelGlow();


	// Update client-side physics structs...

	SetPhysicsStateTimeStep(&g_normalPhysicsState, m_fFrameTime);
	SetPhysicsStateTimeStep(&g_waterPhysicsState, m_fFrameTime);



	// Update the interface (don't do anything if the interface mgr
	// handles the update...)

	if (m_InterfaceMgr.Update())
	{
		return;
	}

	if (IsPlayerDead())
	{
		if (!IsMultiplayerGame())
		{
			if (s_fDeadTimer < s_fDeathDelay)
			{
				s_fDeadTimer += m_fFrameTime;
			}
			else
			{
				LTBOOL bHandleMissionFailed = LTTRUE;

				if (m_InterfaceMgr.FadingScreen() && m_InterfaceMgr.FadingScreenIn())
				{
					bHandleMissionFailed = m_InterfaceMgr.ScreenFadeDone();
				}

				if (bHandleMissionFailed)
				{
					HandleMissionFailed();
				}
			}
		}

	}

	// At this point we only want to proceed if the player is in the world...

	if (IsPlayerInWorld() && m_InterfaceMgr.GetGameState() != GS_UNDEFINED)
	{
		UpdatePlaying();
	}


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdatePlaying()
//
//	PURPOSE:	Handle updating playing (normal) game state
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdatePlaying()
{
	// Handle first update...

	if (m_bFirstUpdate)
	{
		FirstUpdate();
	}


	// Update player movement...

	m_MoveMgr.Update();


	// Update our camera offset mgr...

	m_CameraOffsetMgr.Update();



	// Update reverb...

	UpdateSoundReverb();


	// Tell the player to "start" the level...

	if (!m_bStartedLevel)
	{
        m_bStartedLevel = LTTRUE;
		StartLevel();
	}


	// Update Player...

	UpdatePlayer();


	// Update sky-texture panning...

	if (m_bPanSky && !m_bGamePaused)
	{
		m_fCurSkyXOffset += m_fFrameTime * m_fPanSkyOffsetX;
		m_fCurSkyZOffset += m_fFrameTime * m_fPanSkyOffsetZ;

        g_pLTClient->SetGlobalPanInfo(GLOBALPAN_SKYSHADOW, m_fCurSkyXOffset, m_fCurSkyZOffset, m_fPanSkyScaleX, m_fPanSkyScaleZ);
	}


	// Keep track of what the player is doing...

	UpdatePlayerFlags();

	// Update any debugging information...

	UpdateDebugInfo();

#ifndef _FINAL
	// Update cheats...(if a cheat is in effect, just return)...

	if (!IsMultiplayerGame())
	{
		if (UpdateCheats()) return;
	}
#endif

	// Update head-bob/head-cant camera offsets...

	m_HeadBobMgr.Update();


	// Update duck camera offset...

	UpdateDuck();


	// Update the camera's position...

	UpdateCamera();


	// Update any overlays...

	m_InterfaceMgr.UpdateOverlays();


	// Render the camera...

	RenderCamera();

	// Update container effects...

	UpdateContainerFX();
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
	if (m_bQuickSave && GetGameType() == SINGLE)
	{
		SaveGame(QUICKSAVE_FILENAME);
        m_bQuickSave = LTFALSE;
	}

	// Conditions where we don't want to flip...

	if (m_ePlayerState == PS_UNKNOWN && m_bInWorld && m_InterfaceMgr.GetGameState() != GS_FOLDER)
	{
		return;
	}


	m_InterfaceMgr.PostUpdate();
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

	SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter(m_nSoundFilterId);
	if (!pFilter) return LTNULL;


	if (!g_pSoundFilterMgr->IsDynamic(pFilter))
	{
		// Found it...
		return pFilter;
	}
	else  // Calculate the filter based on the listener...
	{
		// For now just return global default (from WorldProperties)

		return g_pSoundFilterMgr->GetFilter(m_nGlobalSoundFilterId);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateSoundReverb()
//
//	PURPOSE:	Update the sound reverb
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateSoundReverb( )
{
	if (!m_bUseReverb) return;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	float fReverbLevel;
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("ReverbLevel");
	if (hVar)
	{
        fReverbLevel = g_pLTClient->GetVarValueFloat(hVar);
	}
	else
	{
		fReverbLevel = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_DEFAULTLEVEL);
	}

	// Check if reverb was off and is still off...

	if (fReverbLevel < 0.001f && m_fReverbLevel < 0.001f) return;

	m_fReverbLevel = fReverbLevel;

	// Check if it's time yet...

    if (g_pLTClient->GetTime() < m_fNextSoundReverbTime) return;

	// Update timer...

	float fUpdatePeriod = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_UPDATEPERIOD);
    m_fNextSoundReverbTime = g_pLTClient->GetTime() + fUpdatePeriod;

    HOBJECT hFilterList[] = {hPlayerObj, m_MoveMgr.GetObject(), LTNULL};

	ClientIntersectInfo info;
	ClientIntersectQuery query;
	query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	query.m_FilterFn = ObjListFilterFn;
	query.m_pUserData = hFilterList;

    g_pLTClient->GetObjectPos(hPlayerObj, &query.m_From);

	// Make sure the player moved far enough to check reverb again...

	float fPlayerMoveDist = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAYERMOVEDIST);
	if (VEC_DIST(query.m_From, m_vLastReverbPos) < fPlayerMoveDist) return;

	float fReverbSegmentLen = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_INTERSECTSEGMENTLEN);
	VEC_COPY( m_vLastReverbPos, query.m_From );

    LTVector vPos[6], vSegs[6];
	VEC_SET( vSegs[0], query.m_From.x + fReverbSegmentLen, query.m_From.y, query.m_From.z );
	VEC_SET( vSegs[1], query.m_From.x - fReverbSegmentLen, query.m_From.y, query.m_From.z );
	VEC_SET( vSegs[2], query.m_From.x, query.m_From.y + fReverbSegmentLen, query.m_From.z );
	VEC_SET( vSegs[3], query.m_From.x, query.m_From.y - fReverbSegmentLen, query.m_From.z );
	VEC_SET( vSegs[4], query.m_From.x, query.m_From.y, query.m_From.z + fReverbSegmentLen );
	VEC_SET( vSegs[5], query.m_From.x, query.m_From.y, query.m_From.z - fReverbSegmentLen );

    LTBOOL bOpen = LTFALSE;
	for (int i = 0; i < 6; i++)
	{
		VEC_COPY( query.m_To, vSegs[i] );

        if ( g_pLTClient->IntersectSegment( &query, &info ))
		{
			VEC_COPY( vPos[i], info.m_Point );

			SurfaceType eSurfType = GetSurfaceType(info);

			if (eSurfType == ST_AIR || eSurfType == ST_SKY ||
				eSurfType == ST_INVISIBLE)
			{
                bOpen = LTTRUE;
			}
		}
		else
		{
			VEC_COPY( vPos[i], vSegs[i] );
            bOpen = LTTRUE;
		}
	}

	float fVolume = VEC_DIST( vPos[0], vPos[1] );
	fVolume *= VEC_DIST( vPos[2], vPos[3] );
	fVolume *= VEC_DIST( vPos[4], vPos[5] );


	ReverbProperties reverbProperties;

	// Use room types that are not completely enclosed rooms...

	if ( bOpen )
	{
		float fPipeSpace  = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPESPACE);
		float fPlainSpace = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINSPACE);
		float fArenaSpace = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENASPACE);

		if ( fVolume < fPipeSpace*fPipeSpace*fPipeSpace )
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_SEWERPIPE;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEDAMPING);
		}
		else if ( fVolume < fPlainSpace*fPlainSpace*fPlainSpace )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_PLAIN;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PLAINDAMPING);
		}
		else if ( fVolume < fArenaSpace*fArenaSpace*fArenaSpace )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_ARENA;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENAREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENADECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENAVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_ARENADAMPING);
		}
		else
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_MOUNTAINS;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_MOUNTAINSREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_MOUNTAINSDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_MOUNTAINSVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_MOUNTAINSDAMPING);
		}
	}
	else  // Use room types that are enclosed rooms
	{
		float fStoneRoomSpace	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMSPACE);
		float fHallwaySpace		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYSPACE);
		float fConcertHallSpace = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLSPACE);

		if ( fVolume < fStoneRoomSpace*fStoneRoomSpace*fStoneRoomSpace)
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_STONEROOM;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMDAMPING);
		}
		else if ( fVolume < fHallwaySpace*fHallwaySpace*fHallwaySpace )
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_HALLWAY;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_HALLWAYDAMPING);
		}
		else if ( fVolume < fConcertHallSpace*fConcertHallSpace*fConcertHallSpace )
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_CONCERTHALL;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_CONCERTHALLDAMPING);
		}
		else
		{
			reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_AUDITORIUM;
			reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_AUDITORIUMREFLECTTIME);
			reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_AUDITORIUMDECAYTIME);
			reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_AUDITORIUMVOLUME) * m_fReverbLevel;
			reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_AUDITORIUMDAMPING);
		}
	}

	if (m_DamageFXMgr.IsStunned())
	{
		reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_DIZZY;
//		reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMREFLECTTIME);
//		reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMDECAYTIME);
//		reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMVOLUME) * m_fReverbLevel;
//		reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_STONEROOMDAMPING);
	}
	else if (m_DamageFXMgr.IsPoisoned())
	{
		reverbProperties.m_dwAcoustics  = REVERB_ACOUSTICS_DRUGGED;
//		reverbProperties.m_fReflectTime = g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEREFLECTTIME);
//		reverbProperties.m_fDecayTime	= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEDECAYTIME);
//		reverbProperties.m_fVolume		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEVOLUME) * m_fReverbLevel;
//		reverbProperties.m_fDamping		= g_pClientButeMgr->GetReverbAttributeFloat(REVERB_BUTE_PIPEDAMPING);
	}

	// Override to water if in it...

	if (IsLiquid(m_eCurContainerCode))
	{
		reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_UNDERWATER;
	}

	reverbProperties.m_dwParams = REVERBPARAM_ALL;
    g_pLTClient->SetReverbProperties(&reverbProperties);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateCheats()
//
//	PURPOSE:	Update cheats...
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::UpdateCheats()
{
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

	if (m_bAdjustWeaponBreach)
	{
		AdjustWeaponBreach();
	}

	if (m_bAdjust1stPersonCamera)
	{
		Adjust1stPersonCamera();
	}

	// If in spectator mode, just do the camera stuff...

	if (m_bSpectatorMode || IsPlayerDead())
	{
		UpdateCamera();
		RenderCamera();
        return LTTRUE;
	}

	// Update weapon position if appropriated...

	if (m_bTweakingWeapon)
	{
		UpdateWeaponPosition();
		UpdateCamera();
		RenderCamera();
        return LTTRUE;
	}
	else if (m_bTweakingWeaponMuzzle)
	{
		UpdateWeaponMuzzlePosition();
		UpdateCamera();
		RenderCamera();
        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateCamera()
//
//	PURPOSE:	Update the camera position/rotation
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateCamera()
{
	// Update the sway...

	if (IsZoomed())
	{
		UpdateCameraSway();
	}

	// Update the camera's position and rotation..

	UpdateAlternativeCamera();


	// Update the player camera...

	UpdatePlayerCamera();


	if (!m_bUsingExternalCamera)
	{
		if (IsMultiplayerGame() || m_InterfaceMgr.AllowCameraMovement())
		{
			UpdateCameraPosition();
		}
	}

	if (m_InterfaceMgr.AllowCameraMovement())
	{
		CalculateCameraRotation();
		UpdateCameraRotation();
	}

	if (m_bUsingExternalCamera)
	{
        HandleZoomChange(m_weaponModel.GetWeaponId(), LTTRUE);
	}

	// Update zoom if applicable...

	if (m_bZooming)
	{
		UpdateCameraZoom();
	}

	// Update shake if applicable...

	UpdateCameraShake();


	// Make sure the player gets updated

 	if (IsMultiplayerGame() || m_InterfaceMgr.AllowCameraMovement())
	{
		UpdatePlayerInfo();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdatePlayerInfo()
//
//	PURPOSE:	Tell the player about the new camera stuff
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdatePlayerInfo()
{
	if (m_bAllowPlayerMovement != m_bLastAllowPlayerMovement)
	{
		SetInputState(m_bAllowPlayerMovement);
	}

	if (m_PlayerCamera.IsChaseView() != m_bLastSent3rdPerson)
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_3RDPERSON;
		m_bLastSent3rdPerson = m_PlayerCamera.IsChaseView();

		if (m_PlayerCamera.IsChaseView())
		{
			m_nPlayerInfoChangeFlags |= CLIENTUPDATE_3RDPERVAL;
		}
	}

	if (m_bAllowPlayerMovement != m_bLastAllowPlayerMovement)
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_ALLOWINPUT;
	}

	// Always send CLIENTUPDATE_ALLOWINPUT changes guaranteed...

	if (m_nPlayerInfoChangeFlags & CLIENTUPDATE_ALLOWINPUT)
	{
        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_UPDATE);
        g_pLTClient->WriteToMessageWord(hMessage, CLIENTUPDATE_ALLOWINPUT);
        g_pLTClient->WriteToMessageByte(hMessage, (uint8)m_bAllowPlayerMovement);
        g_pLTClient->EndMessage(hMessage);
		m_nPlayerInfoChangeFlags &= ~CLIENTUPDATE_ALLOWINPUT;
	}

    float fCurTime   = g_pLTClient->GetTime();
	float fSendRate  = 1.0f / g_CV_CSendRate.GetFloat(DEFAULT_CSENDRATE);
	float fSendDelta = (fCurTime - m_fPlayerInfoLastSendTime);

	if (!IsMultiplayerGame() || fSendDelta > fSendRate)
	{
        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_UPDATE);

		if (g_vtPlayerRotate.GetFloat(1.0) > 0.0)
		{
			m_nPlayerInfoChangeFlags |= CLIENTUPDATE_PLAYERROT;
		}

        g_pLTClient->WriteToMessageWord(hMessage, m_nPlayerInfoChangeFlags);

		if (m_nPlayerInfoChangeFlags & CLIENTUPDATE_PLAYERROT)
		{
			// Set the player's rotation (don't allow model to rotate up/down).

            LTRotation rPlayerRot;
            //g_pLTClient->SetupEuler(&rPlayerRot, 0.0f, m_fYaw, m_fRoll);
            g_pLTClient->SetupEuler(&rPlayerRot, m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
            g_pLTClient->WriteToMessageByte(hMessage, CompressRotationByte(g_pLTClient->Common(), &rPlayerRot));

			{ // BL 10/05/00 - character waist pitching
//				CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
//				int8 byPitch = (int8)(10.0f*(pSettings->MouseInvertY() ? m_fPitch : -m_fPitch));
				int8 byPitch = (int8)(10.0f * -m_fPitch);
				//g_pLTClient->CPrint("Client pitch = %d", byPitch);
				g_pLTClient->WriteToMessageByte(hMessage, byPitch);
			}
		}

		// Write position info...

		m_MoveMgr.WritePositionInfo(hMessage);

        g_pLTClient->EndMessage2(hMessage, 0);

		m_fPlayerInfoLastSendTime = fCurTime;
		m_nPlayerInfoChangeFlags  = 0;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StartLevel()
//
//	PURPOSE:	Tell the player to start the level
//
// ----------------------------------------------------------------------- //

void CGameClientShell::StartLevel()
{
	if (IsMultiplayerGame()) return;

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SINGLEPLAYER_START);
    g_pLTClient->EndMessage(hMessage);

	// Set Initial cheats...

	if (g_pCheatMgr)
	{
        uint8 nNumCheats = g_pClientButeMgr->GetNumCheatAttributes();
		CString strCheat;

        for (uint8 i=0; i < nNumCheats; i++)
		{
			strCheat = g_pClientButeMgr->GetCheat(i);
			if (strCheat.GetLength() > 1)
			{
				g_pCheatMgr->Check(strCheat.GetBuffer(0));
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdatePlayerCamera()
//
//	PURPOSE:	Update the player camera
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::UpdatePlayerCamera()
{
	// Make sure our player camera is attached...

	if (m_PlayerCamera.GetAttachedObject() != m_MoveMgr.GetObject())
	{
		m_PlayerCamera.AttachToObject(m_MoveMgr.GetObject());
	}

	if (m_PlayerCamera.IsChaseView())
	{
		// Init the camera so they can be adjusted via the console...

		InitPlayerCamera();

		Update3rdPersonInfo();
	}


	// Update our camera position based on the player camera...

	m_PlayerCamera.CameraUpdate(m_fFrameTime);


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::InitPlayerCamera()
//
//	PURPOSE:	Update the player camera
//
// ----------------------------------------------------------------------- //

void CGameClientShell::InitPlayerCamera()
{
    LTVector vOffset(0.0f, 0.0f, 0.0f);
	vOffset.y = g_vtChaseCamOffset.GetFloat();

	m_PlayerCamera.SetDistUp(g_vtChaseCamDistUp.GetFloat());
	m_PlayerCamera.SetDistBack(g_vtChaseCamDistBack.GetFloat());
	m_PlayerCamera.SetPointAtOffset(vOffset);
	m_PlayerCamera.SetChaseOffset(vOffset);
	m_PlayerCamera.SetCameraState(CPlayerCamera::SOUTH);

	// Determine the first person offset...

	vOffset = g_vPlayerCameraOffset;

	CCharacterFX* pChar = m_MoveMgr.GetCharacterFX();
	if (pChar)
	{
		vOffset = GetPlayerHeadOffset(g_pModelButeMgr, pChar->GetModelStyle());
	}

	m_PlayerCamera.SetFirstPersonOffset(vOffset);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetExternalCamera()
//
//	PURPOSE:	Turn on/off external camera mode
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SetExternalCamera(LTBOOL bExternal)
{
	if (bExternal && m_PlayerCamera.IsFirstPerson())
	{
		m_DamageFXMgr.Clear();
        m_weaponModel.SetVisible(LTFALSE);

        ShowPlayer(LTTRUE);
		m_PlayerCamera.GoChaseMode();
		m_PlayerCamera.CameraUpdate(0.0f);

        m_InterfaceMgr.EnableCrosshair(LTFALSE); // Disable cross hair in 3rd person...
	}
	else if (!bExternal && !m_PlayerCamera.IsFirstPerson()) // Go Internal
	{
        m_weaponModel.SetVisible(LTTRUE);
        ShowPlayer(LTFALSE);

		m_PlayerCamera.GoFirstPerson();
		m_PlayerCamera.CameraUpdate(0.0f);

        m_InterfaceMgr.EnableCrosshair(LTTRUE);

		if (m_h3rdPersonCrosshair)
		{
            g_pLTClient->DeleteObject(m_h3rdPersonCrosshair);
            m_h3rdPersonCrosshair = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateAlternativeCamera()
//
//	PURPOSE:	Update the camera using an alternative camera
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::UpdateAlternativeCamera()
{
	m_bLastAllowPlayerMovement = m_bAllowPlayerMovement;
    m_bAllowPlayerMovement     = LTTRUE;

    HOBJECT hObj = LTNULL;

	// See if we should use an alternative camera position...

	CSpecialFXList* pCameraList = m_sfxMgr.GetCameraList();
	if (pCameraList)
	{
		int nNum = pCameraList->GetSize();

		for (int i=0; i < nNum; i++)
		{
			CCameraFX* pCamFX = (CCameraFX*)(*pCameraList)[i];
			if (!pCamFX) continue;

			hObj = pCamFX->GetServerObj();

			if (hObj)
			{
                uint32 dwUsrFlags;
                g_pLTClient->GetObjectUserFlags(hObj, &dwUsrFlags);

				if (dwUsrFlags & USRFLG_CAMERA_LIVE)
				{
                    m_InterfaceMgr.SetDrawInterface(LTFALSE);

                    SetExternalCamera(LTTRUE);

					m_bAllowPlayerMovement = pCamFX->AllowPlayerMovement();

                    LTVector vPos;
                    g_pLTClient->GetObjectPos(hObj, &vPos);
                    g_pLTClient->SetObjectPos(m_hCamera, &vPos);
                    m_bCameraPosInited = LTTRUE;

                    LTRotation rRot;
                    g_pLTClient->GetObjectRotation(hObj, &rRot);
                    g_pLTClient->SetObjectRotation(m_hCamera, &rRot);

					m_bCamIsListener = pCamFX->IsListener();

					// Always set the camera as the listener, the
					// is listener flag tells the dialogue where
					// to play ;)
					g_pLTClient->SetListener(LTFALSE, &vPos, &rRot);

					// Initialize the cinematic camera

					s_nLastCamType = pCamFX->GetType();

					TurnOnAlternativeCamera(s_nLastCamType);

                    return LTTRUE;
				}
			}
		}
	}


	// Okay, we're no longer using an external camera...

	if (m_bUsingExternalCamera)
	{
		TurnOffAlternativeCamera(s_nLastCamType);
	}


    return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::TurnOnAlternativeCamera()
//
//	PURPOSE:	Set up using an alternative camera
//
// ----------------------------------------------------------------------- //

void CGameClientShell::TurnOnAlternativeCamera(uint8 nCamType)
{
	m_InterfaceMgr.SetLetterBox((nCamType == CT_CINEMATIC));

	if (!m_bUsingExternalCamera)
	{
	    //g_pLTClient->CPrint("TURNING ALTERNATIVE CAMERA: ON");

		// Make sure we clear whatever was on the screen before
		// we switch to this camera...

		m_InterfaceMgr.ClearAllScreenBuffers();

        m_InterfaceMgr.ClosePopup();
		m_weaponModel.Disable(LTTRUE);
	}

    m_bUsingExternalCamera = LTTRUE;

	// Turn off model shadows in cinematics...

	g_nCinSaveModelShadows = GetConsoleInt("MaxModelShadows", 0);
	WriteConsoleInt("MaxModelShadows", 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::TurnOffAlternativeCamera()
//
//	PURPOSE:	Turn off the alternative camera mode
//
// ----------------------------------------------------------------------- //

void CGameClientShell::TurnOffAlternativeCamera(uint8 nCamType)
{
    //g_pLTClient->CPrint("TURNING ALTERNATIVE CAMERA: OFF");

	m_InterfaceMgr.SetDrawInterface(LTTRUE);
    m_bUsingExternalCamera = LTFALSE;

	// Set the listener back to the client...

    g_pLTClient->SetListener(LTTRUE, LTNULL, LTNULL);

	// Force 1st person...

    SetExternalCamera(LTFALSE);

	m_weaponModel.Disable(LTFALSE);

	m_InterfaceMgr.SetLetterBox(LTFALSE);

	// Set shadows back to whatever they were set to before...

	WriteConsoleInt("MaxModelShadows", g_nCinSaveModelShadows);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateCameraPosition()
//
//	PURPOSE:	Update the camera position
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateCameraPosition()
{
    LTVector vPos = m_PlayerCamera.GetPos();

	if (m_PlayerCamera.IsFirstPerson())
	{
		m_HeadBobMgr.AdjustCameraPos(vPos);

		vPos.y	+= m_fCamDuck;
		vPos	+= m_CameraOffsetMgr.GetPosDelta();

		// Special case of camera being attached to the player's head
		// (i.e., death)...

		if (m_bCameraAttachedToHead)
		{
			GetPlayerHeadPosRot(vPos, m_rRotation);
		}
	}
	else
	{
        m_rRotation = m_PlayerCamera.GetRotation();
	}

 	g_pLTClient->SetObjectPos(m_hCamera, &vPos);


    m_bCameraPosInited = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::CalculateCameraRotation()
//
//	PURPOSE:	Calculate the new camera rotation
//
// ----------------------------------------------------------------------- //

void CGameClientShell::CalculateCameraRotation()
{
	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (!pSettings) return;

    LTBOOL bIsVehicle = m_MoveMgr.GetVehicleMgr()->IsVehiclePhysics();

    LTFLOAT fVal = 1.0f + (LTFLOAT)(3 * m_nZoomView);

	// Get axis offsets...

	float offsets[3];
    g_pLTClient->GetAxisOffsets(offsets);

	if (m_bRestoreOrientation)
	{
		memset(offsets, 0, sizeof(float) * 3);
        m_bRestoreOrientation = LTFALSE;
	}

	if (m_bStrafing)
	{
		m_MoveMgr.UpdateMouseStrafeFlags(offsets);
	}

    LTFLOAT fYawDelta    = offsets[0] / fVal;
    LTFLOAT fPitchDelta  = offsets[1] / fVal;

	m_fYaw += fYawDelta;

	// [kml] 12/26/00 Check varying degrees of strage and look.
	if(!(m_dwPlayerFlags & BC_CFLG_STRAFE))
	{
		if(m_dwPlayerFlags & BC_CFLG_LEFT)
		{
			m_fYaw -= m_fFrameTime * ((m_dwPlayerFlags & BC_CFLG_RUN) ? g_vtFastTurnRate.GetFloat() : g_vtNormalTurnRate.GetFloat());
		}

		if(m_dwPlayerFlags & BC_CFLG_RIGHT)
		{
			m_fYaw += m_fFrameTime * ((m_dwPlayerFlags & BC_CFLG_RUN) ? g_vtFastTurnRate.GetFloat() : g_vtNormalTurnRate.GetFloat());
		}
	}

	if (pSettings->MouseLook() || (m_dwPlayerFlags & BC_CFLG_LOOKUP) || (m_dwPlayerFlags & BC_CFLG_LOOKDOWN)
		|| m_bHoldingMouseLook)
    {
        if (pSettings->MouseLook() || m_bHoldingMouseLook)
		{
			if (pSettings->MouseInvertY())
			{
				m_fPitch -= fPitchDelta;
			}
			else
			{
				m_fPitch += fPitchDelta;
			}
		}

		if(m_dwPlayerFlags & BC_CFLG_LOOKUP)
		{
			m_fPitch -= m_fFrameTime * g_vtLookUpRate.GetFloat();
		}

		if(m_dwPlayerFlags & BC_CFLG_LOOKDOWN)
		{
			m_fPitch += m_fFrameTime * g_vtLookUpRate.GetFloat();
		}

		// Don't allow much movement up/down if 3rd person...

		if (!m_PlayerCamera.IsFirstPerson())
		{
            LTFLOAT fMinY = DEG2RAD(45.0f) - 0.1f;

			if (m_fPitch < -fMinY) m_fPitch = -fMinY;
			if (m_fPitch > fMinY)  m_fPitch = fMinY;
		}
	}
	else if (m_fPitch != 0.0f && pSettings->Lookspring())
	{
        LTFLOAT fPitchDelta = (m_fFrameTime * g_vtLookUpRate.GetFloat());
		if (m_fPitch > 0.0f) m_fPitch -= min(fPitchDelta, m_fPitch);
		if (m_fPitch < 0.0f) m_fPitch += min(fPitchDelta, -m_fPitch);
	}

    LTFLOAT fMinY = MATH_HALFPI - 0.1f;

	if (m_fPitch < -fMinY) m_fPitch = -fMinY;
	if (m_fPitch > fMinY)  m_fPitch = fMinY;


	// Set camera and player variables...

	// Only use mouse values for yaw if the player isn't on a vehicle...

	if (bIsVehicle)
	{
		// Can't look up/down on vehicles...

		m_fPitch = 0.0f;

		LTVector vPlayerPYR(m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
		LTVector vPYR(m_fPitch, m_fYaw, m_fRoll);

		m_MoveMgr.GetVehicleMgr()->CalculateVehicleRotation(vPlayerPYR, vPYR, fYawDelta);

		m_fPlayerPitch	= vPlayerPYR.x;
		m_fPlayerYaw	= vPlayerPYR.y;
		m_fPlayerRoll	= vPlayerPYR.z;

		m_fPitch		= vPYR.x;
		m_fYaw			= vPYR.y;
		m_fRoll			= vPYR.z;
	}
	else  // Not vehicle...
	{
		m_fPlayerPitch	= 0.0f;
		m_fPlayerYaw	= m_fYaw;
		m_fPlayerRoll	= m_fRoll;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateCameraRotation()
//
//	PURPOSE:	Set the new camera rotation
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::UpdateCameraRotation()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
    if (!hPlayerObj) return LTFALSE;

	// Update camera orientation vars...

    LTVector vPitchYawRollDelta = m_CameraOffsetMgr.GetPitchYawRollDelta();

	if (m_bUsingExternalCamera)
	{
		// Just calculate the correct player rotation...

        g_pLTClient->SetupEuler(&m_rRotation, m_fPitch, m_fYaw, m_fRoll);
	}
	else if (m_PlayerCamera.IsFirstPerson())
	{
		if (!m_bCameraAttachedToHead)
		{
			float fPitch = m_fPitch + vPitchYawRollDelta.x;
			float fYaw   = m_fYaw	+ vPitchYawRollDelta.y;
			float fRoll  = m_fRoll  + vPitchYawRollDelta.z;

			LTRotation rNewRot, rOldRot;
			g_pLTClient->SetupEuler(&rNewRot, fPitch, fYaw, fRoll);
			g_pLTClient->GetObjectRotation(m_hCamera, &rOldRot);

			m_rRotation = rNewRot;
		}

	    g_pLTClient->SetObjectRotation(m_hCamera, &m_rRotation);
	}
	else
	{
		// Set the camera to use the rotation calculated by the player camera,
		// however we still need to calculate the correct rotation to be sent
		// to the player...

        LTFLOAT fAdjust = DEG2RAD(g_vtChaseCamPitchAdjust.GetFloat());
        g_pLTClient->EulerRotateX(&m_rRotation, m_fPitch + fAdjust);
 		g_pLTClient->SetObjectRotation(m_hCamera, &m_rRotation);

		// Okay, now calculate the correct player rotation...

        g_pLTClient->SetupEuler(&m_rRotation, m_fPitch, m_fYaw, m_fRoll);
	}

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetCameraFOV
//
//	PURPOSE:	Set the camera's FOV
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetCameraFOV(LTFLOAT fFovX, LTFLOAT fFovY)
{
	if (!m_hCamera) return;

    g_pLTClient->SetCameraFOV(m_hCamera, fFovX, fFovY);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateCameraZoom
//
//	PURPOSE:	Update the camera's field of view
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateCameraZoom()
{
	char strConsole[30];

    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

    LTFLOAT fovX, fovY;
    g_pLTClient->GetCameraFOV(m_hCamera, &fovX, &fovY);

    LTFLOAT fOldFovX = fovX;

	if (!fovX)
	{
		fovX = DEG2RAD(g_vtFOVXNormal.GetFloat());
	}

    m_bZooming = LTTRUE;


    LTFLOAT fFovXZoomed, fZoomDist;

	if (m_bZoomingIn)
	{
		if (m_nZoomView == 1)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED);
			fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - fFovXZoomed;
		}
		else if (m_nZoomView == 2)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED1);
			fZoomDist	= DEG2RAD(FOVX_ZOOMED) - fFovXZoomed;
		}
		else if (m_nZoomView == 3)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED2);
			fZoomDist	= DEG2RAD(FOVX_ZOOMED1) - fFovXZoomed;
		}
	}
	else
	{
		if (m_nZoomView == 0)
		{
			fFovXZoomed	= DEG2RAD(g_vtFOVXNormal.GetFloat());
			fZoomDist	= DEG2RAD(g_vtFOVXNormal.GetFloat()) - DEG2RAD(FOVX_ZOOMED);
		}
		else if (m_nZoomView == 1)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED);
			fZoomDist	= fFovXZoomed - DEG2RAD(FOVX_ZOOMED1);
		}
		else if (m_nZoomView == 2)
		{
			fFovXZoomed	= DEG2RAD(FOVX_ZOOMED1);
			fZoomDist	= fFovXZoomed - DEG2RAD(FOVX_ZOOMED2);
		}
	}

    LTFLOAT fZoomVel = fZoomDist / ZOOM_TIME;
    LTFLOAT fZoomAmount = fZoomVel * m_fFrameTime;

	// Zoom camera in or out...

	if (m_bZoomingIn)
	{
		if (fovX > fFovXZoomed)
		{
			// Zoom camera in...

			fovX -= fZoomAmount;
		}

		if (fovX <= fFovXZoomed)
		{
			fovX = fFovXZoomed;
            m_bZooming = LTFALSE;
			m_InterfaceMgr.EndZoom();
		}
	}
	else  // Zoom camera out...
	{
		if (fovX < fFovXZoomed)
		{
			// Zoom camera out...

			fovX += fZoomAmount;
		}

		if (fovX >= fFovXZoomed)
		{
			fovX = fFovXZoomed;
            m_bZooming = LTFALSE;
			m_InterfaceMgr.EndZoom();
			if (m_nZoomView == 0)
			{
				EndZoom();
			}
		}
	}

	if (fOldFovX != fovX && dwWidth && dwHeight)
	{
		fovY = (fovX * DEG2RAD(g_vtFOVYNormal.GetFloat())) / DEG2RAD(g_vtFOVXNormal.GetFloat());

		SetCameraFOV(fovX, fovY);

		// Update the lod adjustment for models...
		LTFLOAT fZoomAmount = (DEG2RAD(g_vtFOVXNormal.GetFloat()) - fovX) / (DEG2RAD(g_vtFOVXNormal.GetFloat()) - DEG2RAD(FOVX_ZOOMED2));
		LTFLOAT fNewLODOffset = m_fSaveLODScale + (LOD_ZOOMADJUST * fZoomAmount);

		sprintf(strConsole, "+ModelLODOffset %f", fNewLODOffset);
        g_pLTClient->RunConsoleString(strConsole);

        //g_pLTClient->CPrint("Current FOV (%f, %f)", fovX, fovY);
        //g_pLTClient->CPrint("Current Zoom LODOffset: %f", fNewLODOffset);
	}

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::InCameraGadgetRange
//
//	PURPOSE:	See if the given object is in the camera gadget's range
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::InCameraGadgetRange(HOBJECT hObj)
{
	if (!hObj || !m_hCamera) return LTFALSE;

	if (m_weaponModel.IsDisabled()) return LTFALSE;

	LTVector vCamPos, vObjPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);
	g_pLTClient->GetObjectPos(hObj, &vObjPos);

	LTVector vDist = vCamPos - vObjPos;
	LTFLOAT fDist = vDist.Mag();

	// g_pLTClient->CPrint("fDist = %.2f, Zoom View = %d", fDist, m_nZoomView);

	if (fDist < g_vtSunZoomLevel1MaxDist.GetFloat())
	{
		return LTTRUE;
	}
	else if (fDist < g_vtSunZoomLevel2MaxDist.GetFloat())
	{
		if (m_nZoomView > 0) return LTTRUE;
	}
	else
	{
		if (m_nZoomView > 1) return LTTRUE;
	}

	// Not zoomed in enough...
	return LTFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateCameraShake
//
//	PURPOSE:	Update the camera's shake
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateCameraShake()
{
	// Decay...

    LTFLOAT fDecayAmount = 2.0f * m_fFrameTime;

	m_vShakeAmount.x -= fDecayAmount;
	m_vShakeAmount.y -= fDecayAmount;
	m_vShakeAmount.z -= fDecayAmount;

	if (m_vShakeAmount.x < 0.0f) m_vShakeAmount.x = 0.0f;
	if (m_vShakeAmount.y < 0.0f) m_vShakeAmount.y = 0.0f;
	if (m_vShakeAmount.z < 0.0f) m_vShakeAmount.z = 0.0f;


	if (m_vShakeAmount.x <= 0.0f && m_vShakeAmount.y <= 0.0f && m_vShakeAmount.z <= 0.0f) return;


	// Apply...

    LTFLOAT faddX = GetRandom(-1.0f, 1.0f) * m_vShakeAmount.x * 3.0f;
    LTFLOAT faddY = GetRandom(-1.0f, 1.0f) * m_vShakeAmount.y * 3.0f;
    LTFLOAT faddZ = GetRandom(-1.0f, 1.0f) * m_vShakeAmount.z * 3.0f;

	// Don't update the camera's position if the interface
	// doesn't want us to...

 	if (!m_InterfaceMgr.AllowCameraMovement()) return;

    LTVector vPos, vAdd;
	vAdd.Init(faddX, faddY, faddZ);

    g_pLTClient->GetObjectPos(m_hCamera, &vPos);
	vPos += vAdd;

    g_pLTClient->SetObjectPos(m_hCamera, &vPos);

	HLOCALOBJ hWeapon = m_weaponModel.GetHandle();
	if (!hWeapon) return;

    g_pLTClient->GetObjectPos(hWeapon, &vPos);

	vAdd += 0.95f;
	vPos += vAdd;
    g_pLTClient->SetObjectPos(hWeapon, &vPos);
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
	if (!m_bFlashScreen) return;

    LTVector vLightAdd;
	VEC_SET(vLightAdd, 0.0f, 0.0f, 0.0f);

    LTFLOAT fTime  = g_pLTClient->GetTime();
	if ((m_fFlashRampUp > 0.0f) && (fTime < m_fFlashStart + m_fFlashRampUp))
	{
        LTFLOAT fDelta = (fTime - m_fFlashStart);
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
        LTFLOAT fDelta = (fTime - (m_fFlashStart + m_fFlashRampUp + m_fFlashTime));

		vLightAdd.x = m_vFlashColor.x - (fDelta * (m_vFlashColor.x) / m_fFlashRampUp);
		vLightAdd.y = m_vFlashColor.y - (fDelta * (m_vFlashColor.y) / m_fFlashRampUp);
		vLightAdd.z = m_vFlashColor.z - (fDelta * (m_vFlashColor.z) / m_fFlashRampUp);
	}
	else
	{
        m_bFlashScreen = LTFALSE;
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
        m_bFlashScreen = LTFALSE;
	}

	m_LightScaleMgr.Term();
	m_LightScaleMgr.Init();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateWeaponModel()
//
//	PURPOSE:	Update the weapon model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateWeaponModel()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;


	// Decay weapon recoil...

	DecayWeaponRecoil();


	// If possible, get these values from the camera, because it
	// is more up-to-date...

    LTRotation rRot;
	rRot.Init();
    LTVector vPos(0, 0, 0);

	// Weapon model pos/rot is relative to camera now...
    g_pLTClient->GetObjectPos(m_hCamera, &vPos);
    g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

	if (!m_PlayerCamera.IsFirstPerson() || m_bUsingExternalCamera)
	{
		// Use the gun's flash orientation...

		GetAttachmentSocketTransform(hPlayerObj, "Flash", vPos, rRot);
	}


	// If we aren't dead, and we aren't in the middle of changing weapons,
	// let us fire.

	FireType eFireType = FT_NORMAL_FIRE;
    LTBOOL bFire = LTFALSE;

	// Only check if we're firing if we aren't choosing ammo/weapons...
	if (!m_InterfaceMgr.IsChoosingAmmo() && !m_InterfaceMgr.IsChoosingWeapon())
	{
		if ((m_dwPlayerFlags & BC_CFLG_FIRING) || (m_dwPlayerFlags & BC_CFLG_ALT_FIRING) &&
			!IsPlayerDead() && !m_bSpectatorMode)
		{
            bFire = LTTRUE;
			// Disable alt-fire
			//eFireType = (m_dwPlayerFlags & BC_CFLG_ALT_FIRING) ? FT_ALT_FIRE : FT_NORMAL_FIRE;

			if (m_dwPlayerFlags & BC_CFLG_ALT_FIRING)
			{
                bFire = LTFALSE;
			}
		}
	}


	// Can't fire when asleep...

	//if (m_DamageFXMgr.IsSleeping())
	//{
	//	bFire = LTFALSE;
	//}


	// Update the model position and state...
	WeaponState eWeaponState = m_weaponModel.UpdateWeaponModel(rRot, vPos, bFire, eFireType);


	// Do fire camera jitter...

	if (FiredWeapon(eWeaponState) && m_PlayerCamera.IsFirstPerson())
	{
		StartWeaponRecoil();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StartWeaponRecoil()
//
//	PURPOSE:	Start the weapon recoiling...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::StartWeaponRecoil()
{
	// Shake the screen if it isn't shaking...

	if (m_vShakeAmount.x < 0.1f &&
		m_vShakeAmount.y < 0.1f &&
		m_vShakeAmount.z < 0.1f)
	{
        LTVector vShake(0.1f, 0.1f, 0.1f);
		ShakeScreen(vShake);
	}

	if (g_vtUseCamRecoil.GetFloat() > 0.0f)
	{
		// Move view up a bit...

        LTFLOAT fPCorrect = 1.0f - (m_fFireJitterPitch / DEG2RAD(g_vtMaxCamRecoilPitch.GetFloat()));
        LTFLOAT fPVal = GetRandom(0.5f,2.0f) * DEG2RAD(g_vtBaseCamRecoilPitch.GetFloat()) * fPCorrect;
		m_fFireJitterPitch += fPVal;
		m_fPitch -= fPVal;

		// Move view left/right a bit...

        LTFLOAT fYawDiff = (LTFLOAT)fabs(m_fFireJitterYaw);
        LTFLOAT fYCorrect = 1.0f - (fYawDiff / DEG2RAD(g_vtMaxCamRecoilYaw.GetFloat()));
        LTFLOAT fYVal = GetRandom(-2.0f,2.0f) * DEG2RAD(g_vtBaseCamRecoilYaw.GetFloat()) * fYCorrect;
		m_fFireJitterYaw += fYVal;
		m_fYaw -= fYVal;
	}
	else
	{
		// Move view up a bit...(based on the current weapon/ammo type)

		WEAPON* pWeaponData = m_weaponModel.GetWeapon();
		AMMO*	pAmmoData	= m_weaponModel.GetAmmo();

        LTFLOAT fMaxPitch = g_vtFireJitterMaxPitchDelta.GetFloat();

		if (pWeaponData && pAmmoData)
		{
			if (fMaxPitch < 0.0f)
			{
				fMaxPitch = pWeaponData->fFireRecoilPitch * pAmmoData->fFireRecoilMult;
			}
		}

        LTFLOAT fVal = (LTFLOAT)DEG2RAD(fMaxPitch) - m_fFireJitterPitch;
		m_fFireJitterPitch += fVal;
		m_fPitch -= fVal;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DecayWeaponRecoil()
//
//	PURPOSE:	Decay the weapon's recoil...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::DecayWeaponRecoil()
{
	// Decay firing jitter if necessary...

	if (g_vtUseCamRecoil.GetFloat() > 0.0f)
	{
		if (m_fFireJitterPitch > 0.0f)
		{
            LTFLOAT fCorrect = 1.0f + m_fFireJitterPitch / DEG2RAD(g_vtMaxCamRecoilPitch.GetFloat());
            LTFLOAT fVal = (m_fFrameTime * g_vtCamRecoilRecover.GetFloat()) * fCorrect;

			if (m_fFireJitterPitch < fVal)
			{
				fVal = m_fFireJitterPitch;
			}

			m_fFireJitterPitch -= fVal;
			m_fPitch += fVal;
		}

        LTFLOAT fYawDiff = (LTFLOAT)fabs(m_fFireJitterYaw);
		if (fYawDiff > 0.0f)
		{
            LTFLOAT fCorrect = 1.0f + fYawDiff / DEG2RAD(g_vtMaxCamRecoilYaw.GetFloat());
            LTFLOAT fVal = (m_fFrameTime * g_vtCamRecoilRecover.GetFloat()) * fCorrect;

			if (fYawDiff < fVal)
			{
				fVal = fYawDiff;
			}
			if (m_fFireJitterYaw < 0.0f)
				fVal *= -1.0f;

			m_fFireJitterYaw -= fVal;
			m_fYaw += fVal;
		}
	}
	else
	{
		if (m_fFireJitterPitch > 0.0f)
		{
            LTFLOAT fVal = m_fFireJitterPitch;

			WEAPON* pWeaponData = m_weaponModel.GetWeapon();
			AMMO*	pAmmoData	= m_weaponModel.GetAmmo();

            LTFLOAT fMaxPitch  = g_vtFireJitterMaxPitchDelta.GetFloat();
            LTFLOAT fTotalTime = g_vtFireJitterDecayTime.GetFloat();

			if (pWeaponData && pAmmoData)
			{
				if (fMaxPitch < 0.0f)
				{
					fMaxPitch = pWeaponData->fFireRecoilPitch * pAmmoData->fFireRecoilMult;
				}

				if (fTotalTime < 0.0f)
				{
					fTotalTime = pWeaponData->fFireRecoilDecay;
				}
			}

			if (fTotalTime > 0.01)
			{
				fVal = m_fFrameTime * (DEG2RAD(fMaxPitch) / fTotalTime);
			}

			if (m_fFireJitterPitch - fVal < 0.0f)
			{
				fVal = m_fFireJitterPitch;
			}

			m_fFireJitterPitch -= fVal;
			m_fPitch += fVal;
		}

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::Update3rdPersonCrossHair()
//
//	PURPOSE:	Update the 3rd person crosshair pos
//
// ----------------------------------------------------------------------- //

void CGameClientShell::Update3rdPersonCrossHair(LTFLOAT fDistance)
{
	if (!m_PlayerCamera.IsChaseView()) return;

	// Don't do 3rd person cross hair...
	return;


	HLOCALOBJ hPlayerObj = m_MoveMgr.GetObject();
	if (!hPlayerObj) return;

	if (!m_h3rdPersonCrosshair)
	{
		// Create the 3rd person crosshair sprite...

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		theStruct.m_ObjectType = OT_SPRITE;
		SAFE_STRCPY(theStruct.m_Filename, "Spr\\Crosshair.spr");
		theStruct.m_Flags = FLAG_VISIBLE | FLAG_GLOWSPRITE | FLAG_NOLIGHT;
        m_h3rdPersonCrosshair = g_pLTClient->CreateObject(&theStruct);

        LTVector vScale;
		VEC_SET(vScale, .5f, .5f, 1.0f);
        g_pLTClient->SetObjectScale(m_h3rdPersonCrosshair, &vScale);
	}

	if (fDistance < 1.0f || m_bUsingExternalCamera || !m_InterfaceMgr.IsCrosshairOn())
	{
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_h3rdPersonCrosshair);
        g_pLTClient->SetObjectFlags(m_h3rdPersonCrosshair, dwFlags & ~FLAG_VISIBLE);
		return;
	}
	else
	{
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_h3rdPersonCrosshair);
        g_pLTClient->SetObjectFlags(m_h3rdPersonCrosshair, dwFlags | FLAG_VISIBLE);
	}

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&m_rRotation, &vU, &vR, &vF);
	VEC_NORM(vF);

    LTVector vPos;
    g_pLTClient->GetObjectPos(hPlayerObj, &vPos);

    LTFLOAT fDist = fDistance > 100.0f ? fDistance - 20.0f : fDistance;

    LTVector vTemp;
	VEC_MULSCALAR(vTemp, vF, fDist);
	VEC_ADD(vPos, vPos, vTemp);

	// Set the 3rd person crosshair to the correct position...

    g_pLTClient->SetObjectPos(m_h3rdPersonCrosshair, &vPos);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateContainerFX
//
//	PURPOSE:	Update any client side container fx
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateContainerFX()
{
    LTVector vPos;
    g_pLTClient->GetObjectPos(m_hCamera, &vPos);

	HLOCALOBJ objList[1];
    uint32 dwNum = g_pLTClient->GetPointContainers(&vPos, objList, 1);

    LTVector vScale, vLightAdd;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_SET(vLightAdd, 0.0f, 0.0f, 0.0f);
    LTBOOL bClearCurrentLightScale = LTFALSE;

    char* pCurSound      = LTNULL;
	ContainerCode eCode  = CC_NO_CONTAINER;
	uint8 nSoundFilterId = 0;
    uint32 dwUserFlags   = USRFLG_VISIBLE;
    m_bUseWorldFog       = LTTRUE;

	// Get the user flags associated with the container, and make sure that
	// the container isn't hidden...

	if (dwNum > 0 && objList[0])
	{
        g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);
	}


	if (dwNum > 0 && (dwUserFlags & USRFLG_VISIBLE))
	{
        uint16 code;
        if (g_pLTClient->GetContainerCode(objList[0], &code))
		{
			eCode = (ContainerCode)code;

			// See if we have entered/left a container...

            LTFLOAT fTime = g_pLTClient->GetTime();

			if (m_eCurContainerCode != eCode)
			{
				m_fContainerStartTime = fTime;

				// Check for weather volume brush first (weather volume brushes
				// should never overlap normal volume brushes)

				CVolumeBrushFX* pFX = (CVolumeBrushFX*)m_sfxMgr.FindSpecialFX(SFX_WEATHER_ID, objList[0]);
				if (!pFX)
				{
					pFX = (CVolumeBrushFX*)m_sfxMgr.FindSpecialFX(SFX_VOLUMEBRUSH_ID, objList[0]);
				}

				if (pFX)
				{
					// Set the sound filter override...

					nSoundFilterId = pFX->GetSoundFilterId();

					// See if this container has fog associated with it..

                    LTBOOL bFog = pFX->IsFogEnable();

					if (bFog)
					{
                        m_bUseWorldFog = LTFALSE;

						char buf[30];
						sprintf(buf, "FogEnable %d", (int)bFog);
                        g_pLTClient->RunConsoleString(buf);

						sprintf(buf, "FogNearZ %d", (int)pFX->GetFogNearZ());
                        g_pLTClient->RunConsoleString(buf);

						sprintf(buf, "FogFarZ %d", (int)pFX->GetFogFarZ());
                        g_pLTClient->RunConsoleString(buf);

                        LTVector vFogColor = pFX->GetFogColor();

						sprintf(buf, "FogR %d", (int)vFogColor.x);
                        g_pLTClient->RunConsoleString(buf);

						sprintf(buf, "FogG %d", (int)vFogColor.y);
                        g_pLTClient->RunConsoleString(buf);

						sprintf(buf, "FogB %d", (int)vFogColor.z);
                        g_pLTClient->RunConsoleString(buf);
					}

					// Get the tint color...

					vScale = pFX->GetTintColor();

					if (eCode == CC_WATER)
					{
						vScale = LTVector(0.0f, 127.0f, 178.5f);
					}

					vScale /= 255.0f;

					vLightAdd = pFX->GetLightAdd();
					vLightAdd /= 255.0f;
				}
			}


			switch (eCode)
			{
				case CC_WATER:
				case CC_CORROSIVE_FLUID:
				case CC_FREEZING_WATER:
				{
					pCurSound = "Chars\\Snd\\Player\\unwater.wav";
				}
				break;

				case CC_ENDLESS_FALL:
				{
                    LTFLOAT fFallTime = 1.0f;

					if (fTime > m_fContainerStartTime + fFallTime)
					{
						VEC_SET(vScale, 0.0f, 0.0f, 0.0f);
					}
					else
					{
                        LTFLOAT fScaleStart = .3f;
                        LTFLOAT fTimeLeft = (m_fContainerStartTime + fFallTime) - fTime;
                        LTFLOAT fScalePercent = fTimeLeft/fFallTime;
                        LTFLOAT fScale = fScaleStart * fScalePercent;

						VEC_SET(vScale, fScale, fScale, fScale);
					}

					// special-case the light scale stuff for endless fall
					// if this is our first time in this case, don't clear the effect - otherwise clear it
					if (m_eCurContainerCode == CC_ENDLESS_FALL)
					{
                        bClearCurrentLightScale = LTTRUE;
					}
				}
				break;

				default : break;
			}

		}
	}


	// See if we have entered/left a container...

	if (m_eCurContainerCode != eCode)
	{
		// See if the old container (if any) modified the light scale

        bClearCurrentLightScale = LTTRUE;

		// Adjust Fog as necessary...

		if (m_bUseWorldFog)
		{
			ResetGlobalFog();
		}

		// See if we need to reset the current lightscale
		// If we entered a new container (or modified the light scale in the case of endless fall), then vScale won't be (1,1,1)
        // If we left a container that modified the light scale, then bClearCurrentLightScale will be LTTRUE

		if (bClearCurrentLightScale)
		{
			m_LightScaleMgr.ClearLightScale(&m_vCurContainerLightScale, LightEffectEnvironment);
			VEC_SET(m_vCurContainerLightScale, -1.0f, -1.0f, -1.0f);
		}

		if (vScale.x != 1.0f || vScale.y != 1.0f || vScale.z != 1.0f)
		{
			m_LightScaleMgr.SetLightScale(&vScale, LightEffectEnvironment);
			m_vCurContainerLightScale = vScale;
		}

		// See if we are coming out of water...

		if (IsLiquid(m_eCurContainerCode) && !IsLiquid(eCode))
		{
            UpdateUnderWaterFX(LTFALSE);
            g_pLTClient->RunConsoleString("+ModelWarble 0");
			m_InterfaceMgr.EndUnderwater();
		}

		if (!IsLiquid(m_eCurContainerCode) && IsLiquid(eCode))
		{
            g_pLTClient->RunConsoleString("ModelWarble 1");
			m_InterfaceMgr.BeginUnderwater();
		}

		m_eCurContainerCode = eCode;
		m_nSoundFilterId	= nSoundFilterId;

		if (m_hContainerSound)
		{
            g_pLTClient->KillSound(m_hContainerSound);
            m_hContainerSound = LTNULL;
		}

		if (pCurSound)
		{
            uint32 dwFlags = PLAYSOUND_CLIENT | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
			m_hContainerSound = g_pClientSoundMgr->PlaySoundLocal(pCurSound, SOUNDPRIORITY_PLAYER_MEDIUM, dwFlags);
		}

		m_ScreenTintMgr.Set(TINT_CONTAINER,&vLightAdd);
	}


	// See if we are under water (under any liquid)...

	if (IsLiquid(m_eCurContainerCode))
	{
		UpdateUnderWaterFX();
	}
	else
	{
		UpdateBreathingFX();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateUnderWaterFX
//
//	PURPOSE:	Update under water fx
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateUnderWaterFX(LTBOOL bUpdate)
{
	if (m_nZoomView) return;

    uint32 dwWidth = 640, dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	if (dwWidth < 0 || dwHeight < 0) return;


	// Initialize to default fov x and y...

    LTFLOAT fFovX = g_vtFOVXNormal.GetFloat();
    LTFLOAT fFovY = g_vtFOVYNormal.GetFloat();

	if (bUpdate)
	{
        g_pLTClient->GetCameraFOV(m_hCamera, &fFovX, &fFovY);

		fFovX = RAD2DEG(fFovX);
		fFovY = RAD2DEG(fFovY);

        LTFLOAT fSpeed = g_vtUWFOVRate.GetFloat() * m_fFrameTime;

		if (m_fFovXFXDir > 0)
		{
			fFovX -= fSpeed;
			fFovY += fSpeed;

			if (fFovY > g_vtFOVYMaxUW.GetFloat())
			{
				fFovY = g_vtFOVYMaxUW.GetFloat();
				m_fFovXFXDir = -m_fFovXFXDir;
			}
		}
		else
		{
			fFovX += fSpeed;
			fFovY -= fSpeed;

			if (fFovY < g_vtFOVYMinUW.GetFloat())
			{
				fFovY = g_vtFOVYMinUW.GetFloat();
				m_fFovXFXDir = -m_fFovXFXDir;
			}
		}
	}

	SetCameraFOV(DEG2RAD(fFovX), DEG2RAD(fFovY));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateBreathingFX
//
//	PURPOSE:	Update breathing fx
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateBreathingFX(LTBOOL bUpdate)
{
	//if (m_nZoomView) return;


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ChangeWeapon(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

    uint8 nCommandId = g_pLTClient->ReadFromMessageByte(hMessage);
    uint8 bAuto      = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);
    LTFLOAT fAmmoId  = g_pLTClient->ReadFromMessageFloat(hMessage);


    uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
	if (!pWeapon) return;

    LTBOOL bChange = LTTRUE;

	// See what ammo the weapon should start with...

	uint8 nAmmoId = pWeapon->nDefaultAmmoType;
	if (fAmmoId >= 0)
	{
		nAmmoId = (uint8)fAmmoId;
	}

	// If this is an auto weapon change and this is a multiplayer game, see
	// if the user really wants us to switch or not (we'll always switch in
	// single player games)...

	if (bAuto && IsMultiplayerGame())
	{
        bChange = (LTBOOL)GetConsoleInt("AutoWeaponSwitch",1);

		// See if the weapon we're chaning to is really a weapon...

		if (bChange)
		{
			// Don't autoswitch to gadgets in multiplayer...

			AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
			if (pAmmo && ::IsGadgetType(pAmmo->eInstDamageType))
			{
				bChange = LTFALSE;
			}
		}
	}

	if (bChange)
	{
        // Force a change to the approprite weapon...
		CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();
		if (pStats)
		{
			ChangeWeapon(nWeaponId, nAmmoId, pStats->GetAmmoCount(nAmmoId));
		}
    }

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ChangeWeapon(uint8 nWeaponId, uint8 nAmmoId, uint32 dwAmmo)
{
    LTBOOL bSameWeapon = (nWeaponId == m_weaponModel.GetWeaponId());
	// Turn off zooming...

    uint8 nOldWeaponId = m_weaponModel.GetWeaponId();
	if (!bSameWeapon && g_pWeaponMgr->IsValidWeapon(nOldWeaponId))
	{
        HandleZoomChange(nOldWeaponId, LTTRUE);
	}

	if (!m_weaponModel.GetHandle() || !bSameWeapon)
	{
        m_weaponModel.Create(g_pLTClient, nWeaponId, nAmmoId, dwAmmo);
	}

	if (m_PlayerCamera.IsChaseView())
	{
        m_weaponModel.SetVisible(LTFALSE);
	}

	// Tell the server to change weapons...

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_WEAPON_CHANGE);
    g_pLTClient->WriteToMessageByte(hMessage, nWeaponId);
    g_pLTClient->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::BeginInfrared()
//
//	PURPOSE:	setup infrared viewing mode
//
// ----------------------------------------------------------------------- //

void CGameClientShell::BeginInfrared()
{
	m_LightScaleMgr.SetLightScale(&m_vIRLightScale, LightEffectPowerup);
    g_pLTClient->SetModelHook((ModelHookFn)IRModelHook, this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::EndInfrared()
//
//	PURPOSE:	end infrared viewing mode
//
// ----------------------------------------------------------------------- //

void CGameClientShell::EndInfrared()
{
	m_LightScaleMgr.ClearLightScale(&m_vIRLightScale, LightEffectPowerup);
    g_pLTClient->SetModelHook((ModelHookFn)DefaultModelHook, this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::BeginMineMode()
//
//	PURPOSE:	setup mine viewing mode
//
// ----------------------------------------------------------------------- //

void CGameClientShell::BeginMineMode()
{
    LTVector vCol = g_pLayoutMgr->GetMineDetectScreenTint();
	m_LightScaleMgr.SetLightScale(&vCol, LightEffectPowerup);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::EndMineMode()
//
//	PURPOSE:	end mine viewing mode
//
// ----------------------------------------------------------------------- //

void CGameClientShell::EndMineMode()
{
    LTVector vCol = g_pLayoutMgr->GetMineDetectScreenTint();
	m_LightScaleMgr.ClearLightScale(&vCol, LightEffectPowerup);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::BeginZoom()
//
//	PURPOSE:	prepare for zooming
//
// ----------------------------------------------------------------------- //

void CGameClientShell::BeginZoom()
{
    m_bNightVision = LTFALSE;
	CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();
    uint8 nScopeId = pStats->GetScope();
	if (nScopeId != WMGR_INVALID_ID)
	{
		MOD* pMod = g_pWeaponMgr->GetMod(nScopeId);
		if (pMod)
		{
			m_bNightVision = pMod->bNightVision;
		}
	}

	m_InterfaceMgr.BeginScope(m_bNightVision);
	if (m_bNightVision)
	{
		//m_ScreenTintMgr.Set(TINT_NIGHTVISION,&m_vNVScreenTint);
		m_LightScaleMgr.SetLightScale(&m_vNVScreenTint, LightEffectPowerup);
        g_pLTClient->SetModelHook((ModelHookFn)NVModelHook, this);
	}
    m_weaponModel.SetVisible(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::EndZoom()
//
//	PURPOSE:	done zooming
//
// ----------------------------------------------------------------------- //

void CGameClientShell::EndZoom()
{
	m_InterfaceMgr.EndScope();
	if (m_bNightVision)
	{
		// m_ScreenTintMgr.Clear(TINT_NIGHTVISION);
		m_LightScaleMgr.ClearLightScale(&m_vNVScreenTint, LightEffectPowerup);
        g_pLTClient->SetModelHook((ModelHookFn)DefaultModelHook, this);
        m_bNightVision = LTFALSE;
	}
    m_weaponModel.SetVisible(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleZoomChange()
//
//	PURPOSE:	Handle a potential zoom change
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleZoomChange(uint8 nWeaponId, LTBOOL bReset)
{
	// Reset to normal FOV...

	if (bReset)
	{
		if (m_nZoomView == 0) return;

		m_nZoomView  = 0;
        m_bZooming   = LTFALSE;
        m_bZoomingIn = LTFALSE;
		m_InterfaceMgr.EndZoom();
		EndZoom();

		SetCameraFOV(DEG2RAD(g_vtFOVXNormal.GetFloat()), DEG2RAD(g_vtFOVYNormal.GetFloat()));

		char strConsole[40];
		sprintf(strConsole, "+ModelLODOffset %f", m_fSaveLODScale);
        g_pLTClient->RunConsoleString(strConsole);
	}


	CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();
    uint8 nScopeId = pStats->GetScope();
	if (nScopeId == WMGR_INVALID_ID) return;

	MOD* pMod = g_pWeaponMgr->GetMod(nScopeId);
	if (!pMod) return;

	// Play zoom in/out sounds...

	if (m_bZoomingIn)
	{
		if (pMod->szZoomInSound[0])
		{
			g_pClientSoundMgr->PlaySoundLocal(pMod->szZoomInSound, SOUNDPRIORITY_MISC_MEDIUM);
		}
	}
	else
	{
		if (pMod->szZoomOutSound[0])
		{
			g_pClientSoundMgr->PlaySoundLocal(pMod->szZoomOutSound, SOUNDPRIORITY_MISC_MEDIUM);
		}
	}
}


void CGameClientShell::ProcessHandshake(HMESSAGEREAD hMessage)
{
	int nHandshakeSub = (int)g_pLTClient->ReadFromMessageByte(hMessage);
	switch (nHandshakeSub)
	{
		case MID_HANDSHAKE_HELLO :
		{
			int nHandshakeVer = (int)g_pLTClient->ReadFromMessageWord(hMessage);
			if (nHandshakeVer != GAME_HANDSHAKE_VER)
			{
				// Disconnect
				m_bForceDisconnect = LTTRUE;

				// Show a mis-matched version message to the user
				g_pInterfaceMgr->ChangeState(GS_FOLDER);  // Note : Don't take this out or the cursor goes away!
				HSTRING hString = g_pLTClient->FormatString(IDS_NETERR_NOTSAMEGUID);
				g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL,g_pInterfaceResMgr->IsEnglish());
				g_pLTClient->FreeString(hString);

				return;
			}

			// Send back a hello response
			HMESSAGEWRITE hResponse = g_pLTClient->StartMessage(MID_HANDSHAKE);
		    g_pLTClient->WriteToMessageByte(hResponse, MID_HANDSHAKE_HELLO);
		    g_pLTClient->WriteToMessageWord(hResponse, GAME_HANDSHAKE_VER);
			// Send them our secret key
			g_pLTClient->WriteToMessageDWord(hResponse, GAME_HANDSHAKE_PASSWORD);
			g_pLTClient->EndMessage(hResponse);
		}
		break;
		case MID_HANDSHAKE_PASSWORD:
		{
			// Read in their key
			uint32 nServerKey = g_pLTClient->ReadFromMessageDWord(hMessage);

			uint32 nPassword = GAME_HANDSHAKE_PASSWORD;
			uint32 nXORMask = GAME_HANDSHAKE_MASK;

			nPassword ^= nXORMask;

			// Get the weapons file CRC
			uint32 nWeaponCRC = g_pWeaponMgr->GetFileCRC();
			// Mask that up too
			nWeaponCRC ^= nXORMask;

			// Get the client shell file CRC
			char aClientShellName[MAX_PATH + 1];
			// Just in case getting the file name fails
			aClientShellName[0] = 0; 
			// Get the client shell handle from the engine
			HMODULE hClientShell;
			g_pLTClient->GetEngineHook("cshell_hinstance", (void**)&hClientShell);
			DWORD nResult = GetModuleFileName(hClientShell, aClientShellName, sizeof(aClientShellName));
			uint32 nClientCRC = CRC32::CalcFileCRC(aClientShellName);
			
			// Mask that up too
			nClientCRC ^= nXORMask;

			// Send it back their direction
			HMESSAGEWRITE hResponse = g_pLTClient->StartMessage(MID_HANDSHAKE);
			g_pLTClient->WriteToMessageByte(hResponse, MID_HANDSHAKE_LETMEIN);
			g_pLTClient->WriteToMessageDWord(hResponse, nPassword);
			g_pLTClient->WriteToMessageDWord(hResponse, nWeaponCRC);
			g_pLTClient->WriteToMessageDWord(hResponse, nClientCRC);
			g_pLTClient->EndMessage(hResponse);
		}
		break;
		case MID_HANDSHAKE_DONE:
		{
			// This just means the server validated us...
		}
		break;
		default :
		{
			// Disconnect
			m_bForceDisconnect = LTTRUE;

			// Show a mis-matched version message to the user
			g_pInterfaceMgr->ChangeState(GS_FOLDER);  // Note : Don't take this out or the cursor goes away!
			HSTRING hString = g_pLTClient->FormatString(IDS_NETERR_NOTSAMEGUID);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL,g_pInterfaceResMgr->IsEnglish());
			g_pLTClient->FreeString(hString);
		}
		break;
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
	// Let the interface handle the command first...

	if (IsPlayerInWorld())
	{
		if (IsMultiplayerGame() || !IsPlayerDead())
		{
			if (m_InterfaceMgr.OnCommandOn(command))
			{
				return;
			}
		}
	}

	// Check for weapon change...

	if (g_pWeaponMgr->GetFirstWeaponCommandId() <= command &&
		command <= g_pWeaponMgr->GetLastWeaponCommandId())
	{
		m_weaponModel.ChangeWeapon(command);
		return;
	}


	// Make sure we're in the world...

	if (!IsPlayerInWorld()) return;


	// Take appropriate action

	switch (command)
	{
		case COMMAND_ID_ACTIVATE :
		{
			DoActivate(LTFALSE);
		}
		break;

		case COMMAND_ID_RELOAD :
		{
			m_weaponModel.ReloadClip();
		}
		break;

		case COMMAND_ID_FLASHLIGHT :
		{
			m_FlashLight.Toggle();
		}
		break;

		case COMMAND_ID_ZOOM_IN :
		{
			if (m_weaponModel.IsDisabled()) break;

			CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();
            uint8 nScopeId = pStats->GetScope();
			if (nScopeId == WMGR_INVALID_ID) break;

			MOD* pMod = g_pWeaponMgr->GetMod(nScopeId);
			if (!pMod) break;

			int nZoomLevel = pMod->nZoomLevel;

			// Figure out if our current weapon has a scope...
			if (!m_bZooming && nZoomLevel > 0)
			{
				int nOldZoom = m_nZoomView;

				m_nZoomView++;
				m_nZoomView = m_nZoomView > nZoomLevel ? nZoomLevel : m_nZoomView;

				if (m_nZoomView != nOldZoom)
				{
                    m_bZooming   = LTTRUE;
                    m_bZoomingIn = LTTRUE;

					if (nOldZoom == 0)
					{
						BeginZoom();
                    }

					m_InterfaceMgr.BeginZoom(LTTRUE);
					HandleZoomChange(pStats->GetCurWeapon());
				}
			}
		}
		break;

		case COMMAND_ID_ZOOM_OUT :
		{
			if (m_weaponModel.IsDisabled()) break;

			CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();
            uint8 nScopeId = pStats->GetScope();
			if (nScopeId == WMGR_INVALID_ID) break;

			MOD* pMod = g_pWeaponMgr->GetMod(nScopeId);
			if (!pMod) break;

			int nZoomLevel = pMod->nZoomLevel;

			if (!m_bZooming && nZoomLevel > 0)
			{
				int nOldZoom = m_nZoomView;
				m_nZoomView = 0;

				if (m_nZoomView != nOldZoom)
				{
                    m_bZooming   = LTTRUE;
                    m_bZoomingIn = LTFALSE;
                    m_InterfaceMgr.BeginZoom(LTFALSE);
					HandleZoomChange(pStats->GetCurWeapon());
				}
			}
		}
		break;

		case COMMAND_ID_TURNAROUND :
		{
			m_fYaw += MATH_PI;
		}
		break;

		case COMMAND_ID_MOUSEAIMTOGGLE :
		{
			CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
			if (!pSettings) return;

			if (m_PlayerCamera.IsFirstPerson())
			{
				if (!pSettings->MouseLook())
				{
                    m_bHoldingMouseLook = LTTRUE;
				}
			}
		}
		break;

		case COMMAND_ID_CENTERVIEW :
		{
			m_fPitch = 0.0f;
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if (IsPlayerDead())
			{
				if (IsMultiplayerGame())
				{
                    if (g_pLTClient->GetTime() > m_fEarliestRespawnTime)
					{
						HandleRespawn();
					}
				}
				else if (g_pLTClient->GetTime() > m_fEarliestRespawnTime)
				{
					HandleMissionFailed();
				}
			}
		}
		break;

		case COMMAND_ID_STRAFE:
		{
            m_bStrafing = LTTRUE;
		}
		break;

		case COMMAND_ID_QUICKSAVE :
		{
			if (!IsUsingExternalCamera() && GetGameType() == SINGLE &&
				!IsPlayerDead() && !m_MoveMgr.IsZipCordOn())
			{
				QuickSave();
			}
			else if (GetGameType() == SINGLE)
			{
				// Can't quicksave now...
			    HSTRING hStr = g_pLTClient->FormatString(IDS_CANTQUICKSAVE);
			    char* pStr = g_pLTClient->GetStringData(hStr);
				CSPrint(pStr);
				g_pLTClient->FreeString(hStr);
			}
		}
		break;

		case COMMAND_ID_QUICKLOAD :
		{
            if (GetGameType() != SINGLE && IsPlayerInWorld())
			{
			    HSTRING hString = g_pLTClient->FormatString(IDS_ENDCURRENTGAME);
			    g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,QuickLoadCallBack,this);
				g_pLTClient->FreeString(hString);
			}
			else

			QuickLoad();
		}
		break;



		default :
		break;
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
	// Let the interface handle the command first...
	if (m_InterfaceMgr.OnCommandOff(command))
	{
		return;
	}

	switch (command)
	{
		case COMMAND_ID_STRAFE :
		{
            m_bStrafing = LTFALSE;
		}
		break;

		case COMMAND_ID_MOUSEAIMTOGGLE :
		{
            m_bHoldingMouseLook = LTFALSE;
		}
		break;

        default : break;
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

#define __ALLOW_EDIT_MODE__
#ifdef __ALLOW_EDIT_MODE__

	if (key == VK_TOGGLE_SCREENSHOTMODE)
	{
#ifndef _DEMO
#ifndef _FINAL
		g_bScreenShotMode = !g_bScreenShotMode;
		m_InterfaceMgr.DrawPlayerStats((!g_bScreenShotMode && !m_editMgr.IsEditMode()));
        g_pLTClient->CPrint("Screen shot mode: %s", g_bScreenShotMode ? "ON" : "OFF");
#endif
#endif
		return;
	}

	if (key == VK_TOGGLE_EDITMODE)
	{
#ifndef _DEMO
#ifndef _FINAL
		m_editMgr.ToggleEditMode();
#endif
#endif
		return;
	}
	else if (m_editMgr.IsEditMode())
	{
		m_editMgr.OnKeyDown(key);
		return;
	}

#endif

	// Let the interface mgr have a go at it...
	if (m_InterfaceMgr.OnKeyDown(key, rep))
	{
		return;
	}

	if (IsPlayerDead())
	{
		if (g_pLTClient->GetTime() > m_fEarliestRespawnTime)
		{
			if (!IsMultiplayerGame())
			{
				HandleMissionFailed();
			}
		}
	}

	// Are we playing a cinematic...

	if (m_bUsingExternalCamera)
	{
		if (key == VK_SPACE && m_InterfaceMgr.GetGameState() != GS_DIALOGUE)
		{
			// Send an activate message to stop the cinemaitc...

			DoActivate(LTFALSE);
			g_pLTClient->ClearInput();
			return;
		}
		else if	(key == VK_F9)
		{
			QuickLoad();
			return;
		}
	}

	if (IsMultiplayerGame())
	{
		uint8 nTauntNum = 255;
		switch (key)
		{
		case VK_F4:
			nTauntNum = 0;
			break;
		case VK_F5:
			nTauntNum = 1;
			break;
		case VK_F6:
			nTauntNum = 2;
			break;
		case VK_F7:
			nTauntNum = 3;
			break;
		}

		if (nTauntNum != 255)
		{
			uint32 nLocalID;
			g_pLTClient->GetLocalClientID(&nLocalID);
			DoTaunt(nLocalID,nTauntNum);
		}
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
	m_InterfaceMgr.OnKeyUp(key);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdatePlayerFlags
//
//	PURPOSE:	Update our copy of the movement flags
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdatePlayerFlags()
{
	// Update flags...

	m_dwPlayerFlags = m_MoveMgr.GetControlFlags();

    if (g_pLTClient->IsCommandOn(COMMAND_ID_LOOKUP))
	{
		m_dwPlayerFlags |= BC_CFLG_LOOKUP;
	}

    if (g_pLTClient->IsCommandOn(COMMAND_ID_LOOKDOWN))
	{
		m_dwPlayerFlags |= BC_CFLG_LOOKDOWN;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

void CGameClientShell::OnMessage(uint8 messageID, HMESSAGEREAD hMessage)
{
	// Let interface handle message first...

	if (m_InterfaceMgr.OnMessage(messageID, hMessage)) return;

	switch(messageID)
	{
		case MID_TIMEOFDAYCOLOR:
		{
            LTVector vNewColor;
            vNewColor.x = (float)g_pLTClient->ReadFromMessageByte(hMessage) / MAX_WORLDTIME_COLOR;
            vNewColor.y = (float)g_pLTClient->ReadFromMessageByte(hMessage) / MAX_WORLDTIME_COLOR;
            vNewColor.z = (float)g_pLTClient->ReadFromMessageByte(hMessage) / MAX_WORLDTIME_COLOR;
			m_LightScaleMgr.SetTimeOfDayScale(vNewColor);

            LTVector vNewDir;
            vNewDir.x = (float)(char)g_pLTClient->ReadFromMessageByte(hMessage) / 127.0f;
            vNewDir.y = (float)(char)g_pLTClient->ReadFromMessageByte(hMessage) / 127.0f;
            vNewDir.z = (float)(char)g_pLTClient->ReadFromMessageByte(hMessage) / 127.0f;
            g_pLTClient->SetGlobalLightDir(vNewDir);

            LTFLOAT fTodHours = g_pLTClient->ReadFromMessageFloat(hMessage);
			// Hack: ignores it currently...
            g_pLTClient->SetAmbientLight(0.4f);

			if (m_bUseWorldFog)
			{
				ResetGlobalFog();
			}
		}
		break;

		case MID_SERVERFORCEPOS:
		{
			m_MoveMgr.OnServerForcePos(hMessage);

            m_bPlayerPosSet = LTTRUE;
		}
		break;

		case MID_PHYSICS_UPDATE:
		{
			m_MoveMgr.OnPhysicsUpdate(hMessage);
		}
		break;

		case MID_WEAPON_CHANGE :
		{
			ChangeWeapon(hMessage);
		}
		break;

		case MID_CHANGING_LEVELS :
		{
		}
		break;

		case STC_BPRINT :
		{
			char msg[50];
			hMessage->ReadStringFL(msg, sizeof(msg));
			CSPrint(msg);
		}
		break;

		case MID_SHAKE_SCREEN :
		{
            LTVector vAmount;
            g_pLTClient->ReadFromMessageVector(hMessage, &vAmount);
			ShakeScreen(vAmount);
		}
		break;

		case MID_SFX_MESSAGE :
		{
			m_sfxMgr.OnSFXMessage(hMessage);
		}
		break;

		case MID_PLAYER_EXITLEVEL :
		{
			HandleExitLevel(hMessage);
		}
		break;

		case MID_PLAYER_SUMMARY :
		{
			m_PlayerSummary.ReadClientData(hMessage);
			m_InterfaceMgr.ForceFolderUpdate(m_InterfaceMgr.GetCurrentFolder());
		}
		break;

		case MID_PLAYER_STATE_CHANGE :
		{
			HandlePlayerStateChange(hMessage);
		}
		break;

		case MID_PLAYER_AUTOSAVE :
		{
			AutoSave(hMessage);
		}
		break;

		case MID_PLAYER_DAMAGE :
		{
			HandlePlayerDamage(hMessage);
		}
		break;

		case MID_PLAYER_ORIENTATION :
		{
			// Set our pitch, yaw, and roll according to the players...

            LTVector vVec;
            g_pLTClient->ReadFromMessageVector(hMessage, &vVec);

			m_fPitch		= vVec.x;
			m_fYaw			= vVec.y;
			m_fRoll			= vVec.z;
			m_fPlayerPitch	= vVec.x;
			m_fPlayerYaw	= vVec.y;
			m_fPlayerRoll	= vVec.z;
			m_fYawBackup	= m_fYaw;
			m_fPitchBackup	= m_fPitch;
		}
		break;

		case MID_COMMAND_TOGGLE :
		{
            uint8 nId = g_pLTClient->ReadFromMessageByte(hMessage);

			switch(nId)
			{
				case COMMAND_ID_RUNLOCK :
				{
					CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
					if (pSettings)
					{
                        pSettings->SetRunLock((LTBOOL)g_pLTClient->ReadFromMessageByte(hMessage));
					}
				}
				break;

				default : break;
			}
		}
		break;

		case MID_MUSIC:
		{
			if (m_Music.IsInitialized())
			{
				m_Music.ProcessMusicMessage(hMessage);
			}
		}
		break;

		case MID_PLAYER_LOADCLIENT :
		{
			UnpackClientSaveMsg(hMessage);
		}
		break;

		case MID_PLAYER_MULTIPLAYER_INIT :
		{
			m_nMPNameId = (int)g_pLTClient->ReadFromMessageDWord(hMessage);;
			m_nMPBriefingId = (int)g_pLTClient->ReadFromMessageDWord(hMessage);;

			if (m_nMPNameId || m_nMPBriefingId)
			{
				// Hide the loading screen since we're not in the playing state yet
				m_InterfaceMgr.HideLoadScreen();
				m_InterfaceMgr.SwitchToFolder(FOLDER_ID_MP_BRIEFING);
				m_InterfaceMgr.StartScreenFadeIn(0.5f);
			}
			else
			{
				InitMultiPlayer();
			}
		}
		break;

		case MID_HANDSHAKE :
		{
			ProcessHandshake(hMessage);
		}
		break;

		case MID_PLAYER_SINGLEPLAYER_INIT :
		{
			InitSinglePlayer();
		}
		break;

		case MID_SERVER_ERROR :
		{
			HandleServerError(hMessage);
		}
		break;

		case MID_MULTIPLAYER_DATA :
		{
			HandleMultiplayerGameData(hMessage);
		}
		break;

		case MID_UPDATE_OPTIONS :
		{
			HandleServerOptions(hMessage);
		}
		break;

		case MID_EDIT_OBJECTINFO :
		{
			m_editMgr.HandleEditObjectInfo(hMessage);
		}
		break;

		case MID_CHANGE_FOG :
		{
			if (m_bUseWorldFog)
			{
				ResetGlobalFog();
			}
		}
		break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ShakeScreen()
//
//	PURPOSE:	Shanke, rattle, and roll
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ShakeScreen(LTVector vShake)
{
	// Add...

	VEC_ADD(m_vShakeAmount, m_vShakeAmount, vShake);

	if (m_vShakeAmount.x > MAX_SHAKE_AMOUNT) m_vShakeAmount.x = MAX_SHAKE_AMOUNT;
	if (m_vShakeAmount.y > MAX_SHAKE_AMOUNT) m_vShakeAmount.y = MAX_SHAKE_AMOUNT;
	if (m_vShakeAmount.z > MAX_SHAKE_AMOUNT) m_vShakeAmount.z = MAX_SHAKE_AMOUNT;
}


// ----------------------------------------------------------------------- //
// Console command handlers for recording and playing demos.
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleRecord(int argc, char **argv)
{
	if(argc < 2)
	{
        g_pLTClient->CPrint("Record <world name> <filename>");
		return;
	}

	if(!DoLoadWorld(argv[0], NULL, NULL, LOAD_NEW_GAME, argv[1], NULL))
	{
        g_pLTClient->CPrint("Error starting world");
	}
}

void CGameClientShell::HandlePlaydemo(int argc, char **argv)
{
	if(argc < 1)
	{
        g_pLTClient->CPrint("Playdemo <filename>");
		return;
	}

	if(!DoLoadWorld("asdf", NULL, NULL, LOAD_NEW_GAME, NULL, argv[0]))
	{
        g_pLTClient->CPrint("Error starting world");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleCheat()
//
//	PURPOSE:	Handle cheat console command
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleCheat(int argc, char **argv)
{
	if (argc < 1 || !g_pCheatMgr) return;

	if (g_pCheatMgr->Check(argv[0]))
	{
		g_pClientSoundMgr->PlayInterfaceSound("Menu\\Snd\\Cheat.wav");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::FlashScreen()
//
//	PURPOSE:	Tint screen
//
// ----------------------------------------------------------------------- //

void CGameClientShell::FlashScreen(LTVector vFlashColor, LTVector vPos, LTFLOAT fFlashRange,
                                  LTFLOAT fTime, LTFLOAT fRampUp, LTFLOAT fRampDown, LTBOOL bForce)
{
	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (!pSettings) return;

	if (!bForce && !pSettings->ScreenFlash()) return;

    LTVector vCamPos;
    g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

	// Determine if we can see this...

    LTVector vDir;
	vDir =  vPos - vCamPos;
    LTFLOAT fDirMag = vDir.Mag();
	if (fDirMag > fFlashRange) return;

	// Okay, not adjust the tint based on the camera's angle to the tint pos.

    LTRotation rRot;
    LTVector vU, vR, vF;
    g_pLTClient->GetObjectRotation(m_hCamera, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_NORM(vDir);
	VEC_NORM(vF);
    LTFLOAT fMul = VEC_DOT(vDir, vF);
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

    LTFLOAT fVal = 1.0f - (fDirMag/fFlashRange);
	fMul *= (fVal <= 1.0f ? fVal : 1.0f);

    m_bFlashScreen  = LTTRUE;
    m_fFlashStart   = g_pLTClient->GetTime();
	m_fFlashTime		= fTime;
	m_fFlashRampUp	= fRampUp;
	m_fFlashRampDown	= fRampDown;
	VEC_COPY(m_vFlashColor, vFlashColor);
	VEC_MULSCALAR(m_vFlashColor, m_vFlashColor, fMul);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateDuck()
//
//	PURPOSE:	Update ducking camera offset
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateDuck()
{
	// Can't duck when free movement...

    if (m_MoveMgr.IsBodyInLiquid() || m_MoveMgr.IsBodyOnLadder() ||
        IsFreeMovement(m_eCurContainerCode)) return;


    LTFLOAT fTime = g_pLTClient->GetTime();

	if (m_dwPlayerFlags & BC_CFLG_DUCK)
	{
        m_bStartedDuckingUp = LTFALSE;

		// See if the duck just started...

		if (!m_bStartedDuckingDown)
		{
            m_bStartedDuckingDown = LTTRUE;
			m_fStartDuckTime = fTime;
		}

		m_fCamDuck = m_fDuckDownV * (fTime - m_fStartDuckTime);

		if (m_fCamDuck < m_fMaxDuckDistance)
		{
			m_fCamDuck = m_fMaxDuckDistance;
		}

	}
	else if (m_fCamDuck < 0.0) // Raise up
	{
        m_bStartedDuckingDown = LTFALSE;

		if (!m_bStartedDuckingUp)
		{
			m_fStartDuckTime = fTime;
            m_bStartedDuckingUp = LTTRUE;
		}

		m_fCamDuck += m_fDuckUpV * (fTime - m_fStartDuckTime);

		if (m_fCamDuck > 0.0f)
		{
			m_fCamDuck = 0.0f;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::PauseGame()
//
//	PURPOSE:	Pauses/Unpauses the server
//
// ----------------------------------------------------------------------- //

void CGameClientShell::PauseGame(LTBOOL bPause, LTBOOL bPauseSound)
{
	m_bGamePaused = bPause;

	if (!IsMultiplayerGame())
	{
        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(bPause ? MID_GAME_PAUSE : MID_GAME_UNPAUSE);
        g_pLTClient->EndMessage(hMessage);
	}

	if (bPause && bPauseSound)
	{
        g_pLTClient->PauseSounds();
	}
	else
	{
        g_pLTClient->ResumeSounds();
	}

	SetInputState(!bPause && m_bAllowPlayerMovement);
	SetMouseInput(!bPause);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetInputState()
//
//	PURPOSE:	Allows/disallows input
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SetInputState(LTBOOL bAllowInput)
{
    g_pLTClient->SetInputState(bAllowInput);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetMouseInput()
//
//	PURPOSE:	Allows or disallows mouse input on the client
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SetMouseInput(LTBOOL bAllowInput)
{
	if (bAllowInput)
	{
        m_bRestoreOrientation = LTTRUE;
		m_fYaw   = m_fYawBackup;
		m_fPitch = m_fPitchBackup;
	}
	else
	{
		m_fYawBackup = m_fYaw;
		m_fPitchBackup = m_fPitch;
	}
}

void CGameClientShell::AllowPlayerMovement(LTBOOL bAllowPlayerMovement)
{
	m_bAllowPlayerMovement = bAllowPlayerMovement;
	SetMouseInput(bAllowPlayerMovement);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandlePlayerStateChange()
//
//	PURPOSE:	Update player state change
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandlePlayerStateChange(HMESSAGEREAD hMessage)
{
    m_ePlayerState = (PlayerState) g_pLTClient->ReadFromMessageByte(hMessage);

	switch (m_ePlayerState)
	{
		case PS_DYING:
		{
			AttachCameraToHead(LTTRUE);

			if (IsMultiplayerGame())
			{
				m_InterfaceMgr.GetPlayerStats()->ResetInventory();
                m_fEarliestRespawnTime = g_pLTClient->GetTime() + g_vtMultiplayerRespawnWaitTime.GetFloat();
			}
			else
			{
                m_fEarliestRespawnTime = g_pLTClient->GetTime() + g_vtRespawnWaitTime.GetFloat();
			}

			HandleZoomChange(m_weaponModel.GetWeaponId(), LTTRUE);
			EndZoom();
			m_InterfaceMgr.EndUnderwater();

            m_weaponModel.SetVisible(LTFALSE);
			m_weaponModel.Reset();

            m_InterfaceMgr.SetDrawInterface(LTFALSE);
			m_InterfaceMgr.AddToClearScreenCount();

			if (GetGameType() == SINGLE)
			{
				HSTRING hStr = g_pLTClient->FormatString(IDS_YOUWEREKILLED);
				CSPrint(g_pLTClient->GetStringData(hStr));
			    g_pLTClient->FreeString(hStr);
				m_InterfaceMgr.StartScreenFadeOut(g_vtScreenFadeOutTime.GetFloat());
			}
		}
		break;

		case PS_ALIVE:
		{
			m_DamageFXMgr.Clear();					// Remove all the damage sfx
			m_InterfaceMgr.ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

			AttachCameraToHead(LTFALSE);

            m_InterfaceMgr.SetDrawInterface(LTTRUE);
            m_InterfaceMgr.DrawPlayerStats(LTTRUE);

            SetExternalCamera(LTFALSE);

            m_weaponModel.Disable(LTFALSE);
        }
		break;

		case PS_DEAD:
		{
			m_DamageFXMgr.Clear();					// Remove all the damage sfx
			s_fDeadTimer = 0.0f;
		}
		break;

		case PS_GHOST:
		{
			AttachCameraToHead(LTFALSE);
            m_weaponModel.SetVisible(LTFALSE);

            m_InterfaceMgr.SetDrawInterface(LTFALSE);
            m_InterfaceMgr.DrawPlayerStats(LTFALSE);

			m_InterfaceMgr.ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

			m_InterfaceMgr.GetMessageMgr()->AddLine(IDS_NO_RESPAWN);

/*
            SetExternalCamera(LTTRUE);
*/
            m_weaponModel.Disable(LTTRUE);

        }
		break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::AutoSave()
//
//	PURPOSE:	Autosave the game
//
// ----------------------------------------------------------------------- //

void CGameClientShell::AutoSave(HMESSAGEREAD hMessage)
{
	if (m_ePlayerState != PS_ALIVE) return;

	// Save the game for reloading if the player just changed levels
	// or if the player just started a game...
	int missionNum = -1;
	int sceneNum = -1;

	if (!g_pGameClientShell->IsCustomLevel())
	{
		missionNum = g_pGameClientShell->GetCurrentMission();
		sceneNum = g_pGameClientShell->GetCurrentLevel();
	}




	time_t seconds;
	time (&seconds);

	char strSaveGame[256];
	sprintf (strSaveGame, "%s|%d,%d|%ld",m_strCurrentWorldName, missionNum, sceneNum, (long)seconds);


	CWinUtil::WinWritePrivateProfileString(GAME_NAME, "Reload", strSaveGame, SAVEGAMEINI_FILENAME);
	SaveGame(RELOADLEVEL_FILENAME);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandlePlayerDamage
//
//	PURPOSE:	Handle the player getting damaged
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandlePlayerDamage(HMESSAGEREAD hMessage)
{
	if (!hMessage || g_bScreenShotMode) return;

    LTVector vDir;
    g_pLTClient->ReadFromMessageVector(hMessage, &vDir);
    DamageType eType = (DamageType) g_pLTClient->ReadFromMessageByte(hMessage);
    LTBOOL bUsingDamage = (LTBOOL) g_pLTClient->ReadFromMessageByte(hMessage);

    LTFLOAT fPercent = VEC_MAG(vDir);

	LTFLOAT fLowValue = bUsingDamage ? 0.5f : 0.0f;
    LTFLOAT fColor = fLowValue + fPercent;
	fColor = fColor > 1.0f ? 1.0f : fColor;

    LTFLOAT fRampDown = fLowValue + fPercent * 2.0f;

	LTVector vFlashColor(fColor, 0.0f, 0.0f);
 	if (!bUsingDamage)
	{
		if (fColor > 0.7f) fColor = 0.7f;
	    vFlashColor.Init(0.2f, 0.2f, fColor);
    }

	LTFLOAT fRampUp = 0.2f, fFlashTime = 0.1f;

    LTVector vCamPos;
    g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

    FlashScreen(vFlashColor, vCamPos, 1000.0f, fRampUp, fFlashTime, fRampDown, LTTRUE);

	// Tilt the camera based on the direction the damage came from...

	if (bUsingDamage && IsJarCameraType(eType) && m_PlayerCamera.IsFirstPerson() &&
		g_vtCamDamage.GetFloat() > 0.0f && fPercent > 0.0f)
	{
        LTRotation rRot;
        LTVector vU, vR, vF;
		GetPlayerRotation(&rRot);
        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

		vDir.Norm();
        LTFLOAT fMul = VEC_DOT(vDir, vF);

		CameraDelta delta;

		if (g_vtCamDamagePitch.GetFloat() > 0.0f)
		{
            // g_pLTClient->CPrint("Damage Pitch Val: %.2f", fMul);

			// Hit from the back...

			if (fMul > g_vtCamDamagePitchMin.GetFloat())
			{
				delta.Pitch.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Pitch.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Pitch.eWave1	= Wave_SlowOff;
				delta.Pitch.eWave2	= Wave_SlowOff;
				delta.Pitch.fVar	= DEG2RAD(g_vtCamDamageVal.GetFloat());
			}
			else if (fMul < - g_vtCamDamagePitchMin.GetFloat()) // Hit from the front...
			{
				delta.Pitch.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Pitch.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Pitch.eWave1	= Wave_SlowOff;
				delta.Pitch.eWave2	= Wave_SlowOff;
				delta.Pitch.fVar	= -DEG2RAD(g_vtCamDamageVal.GetFloat());
			}
		}

		if (g_vtCamDamageRoll.GetFloat() > 0.0f)
		{
			fMul = VEC_DOT(vDir, vR);
            //g_pLTClient->CPrint("Damage Roll Val: %.2f", fMul);

			// Hit from the left...

			if (fMul > g_vtCamDamageRollMin.GetFloat())
			{
				delta.Roll.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Roll.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Roll.eWave1	= Wave_SlowOff;
				delta.Roll.eWave2	= Wave_SlowOff;
				delta.Roll.fVar		= DEG2RAD(g_vtCamDamageVal.GetFloat());
			}
			else if (fMul < - g_vtCamDamageRollMin.GetFloat()) // Hit from the right...
			{
				delta.Roll.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Roll.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Roll.eWave1	= Wave_SlowOff;
				delta.Roll.eWave2	= Wave_SlowOff;
				delta.Roll.fVar		= -DEG2RAD(g_vtCamDamageVal.GetFloat());
			}
		}

		m_CameraOffsetMgr.AddDelta(delta);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleServerError()
//
//	PURPOSE:	Handle any error messages sent from the server
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleServerError(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

    uint8 nError = g_pLTClient->ReadFromMessageByte(hMessage);
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

			m_InterfaceMgr.ChangeState(GS_FOLDER);
		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMultiplayerGameData()
//
//	PURPOSE:	Handle global game data sent from the server
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleMultiplayerGameData(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

    uint8 byGameType = g_pLTClient->ReadFromMessageByte(hMessage);
	m_eGameType = (GameType)byGameType;

	if (m_eGameType != SINGLE)
	{
		hMessage->ReadStringFL(m_szServerAddress, sizeof(m_szServerAddress));
		uint32 tmp = g_pLTClient->ReadFromMessageDWord(hMessage);
		m_nServerPort = (int)tmp;

	}

	if (m_eGameType == COOPERATIVE_ASSAULT)
	{
		m_InterfaceMgr.GetPlayerStats()->SetMultiplayerObjectives(hMessage);

		LTBOOL bFragScore = g_pLTClient->ReadFromMessageByte(hMessage);

	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleServerOptions()
//
//	PURPOSE:	Handle game option data sent from the server
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleServerOptions(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;
	if (m_eGameType == SINGLE) return;

	hMessage->ReadStringFL(m_szServerName, sizeof(m_szServerName));

	int	nNumOptions = (int)g_pServerOptionMgr->GetNumOptions();
	if (nNumOptions > MAX_GAME_OPTIONS)
		nNumOptions = MAX_GAME_OPTIONS;
	for (int i = 0; i < nNumOptions; i++)
	{
		OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
		if (GetGameType() == pOpt->eGameType || pOpt->eGameType == SINGLE)
		{
			m_fServerOptions[i] =  g_pLTClient->ReadFromMessageFloat(hMessage);

		}
		else
			m_fServerOptions[i] = 0.0f;
	}
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
    LTFLOAT fIncValue = WEAPON_MOVE_INC_VALUE_SLOW;
    LTBOOL bChanged = LTFALSE;

    LTVector vOffset;
	VEC_INIT(vOffset);


	// Move weapon faster if running...

	if (m_dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = WEAPON_MOVE_INC_VALUE_FAST;
	}


	vOffset = m_weaponModel.GetMuzzleOffset();


	// Move weapon forward or backwards...

	if ((m_dwPlayerFlags & BC_CFLG_FORWARD) || (m_dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vOffset.z += fIncValue;
        bChanged = LTTRUE;
	}


	// Move the weapon to the player's right or left...

	if ((m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) ||
		(m_dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vOffset.x += fIncValue;
        bChanged = LTTRUE;
	}


	// Move the weapon up or down relative to the player...

	if ((m_dwPlayerFlags & BC_CFLG_JUMP) || (m_dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		vOffset.y += fIncValue;
        bChanged = LTTRUE;
	}


	// Okay, set the offset...

	if (bChanged)
	{
		m_weaponModel.SetMuzzleOffset(vOffset);
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
    LTFLOAT fIncValue = WEAPON_MOVE_INC_VALUE_SLOW;
    LTBOOL bChanged = LTFALSE;

    LTVector vOffset;
	VEC_INIT(vOffset);


	// Move weapon faster if running...

	if (m_dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = WEAPON_MOVE_INC_VALUE_FAST;
	}


	vOffset = m_weaponModel.GetWeaponOffset();


	// Move weapon forward or backwards...

	if ((m_dwPlayerFlags & BC_CFLG_FORWARD) || (m_dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vOffset.z += fIncValue;
        bChanged = LTTRUE;
	}


	// Move the weapon to the player's right or left...

	if ((m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) ||
		(m_dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vOffset.x += fIncValue;
        bChanged = LTTRUE;
	}


	// Move the weapon up or down relative to the player...

	if ((m_dwPlayerFlags & BC_CFLG_JUMP) || (m_dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		vOffset.y += fIncValue;
        bChanged = LTTRUE;
	}


	// Okay, set the offset...

	if (bChanged)
	{
		m_weaponModel.SetWeaponOffset(vOffset);
	}

	//if (bChanged)
	//{
	//	CSPrint ("Weapon offset = %f, %f, %f", vOffset.x, vOffset.y, vOffset.z);
	//}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::AdjustWeaponBreach()
//
//	PURPOSE:	Update the position of the current hand-held weapon breach offset
//
// ----------------------------------------------------------------------- //

void CGameClientShell::AdjustWeaponBreach()
{
    LTFLOAT fIncValue = WEAPON_MOVE_INC_VALUE_SLOW;
    LTBOOL bChanged = LTFALSE;

	// Move breach offset faster if running...

	if (m_dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = WEAPON_MOVE_INC_VALUE_FAST;
	}

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_weaponModel.GetWeaponId());
	if (!pWeapon) return;

    LTFLOAT fBreach = pWeapon->fHHBreachOffset;


	// Move weapon breach offset forward or backwards...

	if ((m_dwPlayerFlags & BC_CFLG_FORWARD) || (m_dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		fBreach += fIncValue;
        bChanged = LTTRUE;
	}

	if (bChanged)
	{
		// Okay, set the offset...

		WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(m_weaponModel.GetWeaponId());
		if (pWeaponData)
		{
			pWeaponData->fHHBreachOffset = fBreach;
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
    LTFLOAT fIncValue = 0.1f;
    LTBOOL bChanged = LTFALSE;

	// Move breach offset faster if running...

	if (m_dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = fIncValue * 2.0f;
	}

	// Move 1st person offset.x forward or backwards...

	if ((m_dwPlayerFlags & BC_CFLG_FORWARD) || (m_dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		g_vPlayerCameraOffset.x += fIncValue;
        bChanged = LTTRUE;
	}

	// Move 1st person offset.y up or down...

	if ((m_dwPlayerFlags & BC_CFLG_JUMP) || (m_dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_JUMP ? fIncValue : -fIncValue;
		g_vPlayerCameraOffset.y += fIncValue;
        bChanged = LTTRUE;
	}


	if (bChanged)
	{
		// Okay, set the offset...

		m_PlayerCamera.SetFirstPersonOffset(g_vPlayerCameraOffset);
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
    LTFLOAT fIncValue = 0.01f;
    LTBOOL bChanged = LTFALSE;

    LTVector vScale;
	VEC_INIT(vScale);

    g_pLTClient->GetGlobalLightScale(&vScale);

	// Move faster if running...

	if (m_dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .5f;
	}


	// Move Red up/down...

	if ((m_dwPlayerFlags & BC_CFLG_FORWARD) || (m_dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vScale.x += fIncValue;
		vScale.x = vScale.x < 0.0f ? 0.0f : (vScale.x > 1.0f ? 1.0f : vScale.x);

        bChanged = LTTRUE;
	}


	// Move Green up/down...

	if ((m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) ||
		(m_dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vScale.y += fIncValue;
		vScale.y = vScale.y < 0.0f ? 0.0f : (vScale.y > 1.0f ? 1.0f : vScale.y);

        bChanged = LTTRUE;
	}


	// Move Blue up/down...

	if ((m_dwPlayerFlags & BC_CFLG_JUMP) || (m_dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		vScale.z += fIncValue;
		vScale.z = vScale.z < 0.0f ? 0.0f : (vScale.z > 1.0f ? 1.0f : vScale.z);

        bChanged = LTTRUE;
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
    LTFLOAT fIncValue = 0.01f;
    LTBOOL bChanged = LTFALSE;

    LTVector vScale;
	VEC_INIT(vScale);

    g_pLTClient->GetCameraLightAdd(m_hCamera, &vScale);

	// Move faster if running...

	if (m_dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .5f;
	}


	// Move Red up/down...

	if ((m_dwPlayerFlags & BC_CFLG_FORWARD) || (m_dwPlayerFlags & BC_CFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_FORWARD ? fIncValue : -fIncValue;
		vScale.x += fIncValue;
		vScale.x = vScale.x < 0.0f ? 0.0f : (vScale.x > 1.0f ? 1.0f : vScale.x);

        bChanged = LTTRUE;
	}


	// Move Green up/down...

	if ((m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) ||
		(m_dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vScale.y += fIncValue;
		vScale.y = vScale.y < 0.0f ? 0.0f : (vScale.y > 1.0f ? 1.0f : vScale.y);

        bChanged = LTTRUE;
	}


	// Move Blue up/down...

	if ((m_dwPlayerFlags & BC_CFLG_JUMP) || (m_dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		vScale.z += fIncValue;
		vScale.z = vScale.z < 0.0f ? 0.0f : (vScale.z > 1.0f ? 1.0f : vScale.z);

        bChanged = LTTRUE;
	}


	// Okay, set the light add.

    g_pLTClient->SetCameraLightAdd(m_hCamera, &vScale);

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
    LTFLOAT fIncValue = 0.001f;
    LTBOOL bChanged = LTFALSE;

    LTFLOAT fCurFOVx, fCurFOVy;

	// Save the current camera fov...

    g_pLTClient->GetCameraFOV(m_hCamera, &fCurFOVx, &fCurFOVy);


	// Move faster if running...

	if (m_dwPlayerFlags & BC_CFLG_RUN)
	{
		fIncValue = .01f;
	}


	// Adjust X

	if ((m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT) || (m_dwPlayerFlags & BC_CFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		fCurFOVx += fIncValue;

        bChanged = LTTRUE;
	}


	// Adjust Y

	if ((m_dwPlayerFlags & BC_CFLG_JUMP) || (m_dwPlayerFlags & BC_CFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & BC_CFLG_DUCK ? -fIncValue : fIncValue;
		fCurFOVy += fIncValue;

        bChanged = LTTRUE;
	}


	// Okay, set the FOV..

	// Adjust the fov...

	SetCameraFOV(fCurFOVx, fCurFOVy);

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
// ----------------------------------------------------------------------- //

void CGameClientShell::SpecialEffectNotify(HLOCALOBJ hObj, HMESSAGEREAD hMessage)
{
	if (hObj)
	{
        uint32 dwCFlags = g_pLTClient->GetObjectClientFlags(hObj);
        g_pLTClient->SetObjectClientFlags(hObj, dwCFlags | CF_NOTIFYREMOVE);
	}

	m_sfxMgr.HandleSFXMsg(hObj, hMessage);
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
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleExitLevel()
//
//	PURPOSE:	Handle ExitLevel console command
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleExitLevel(int argc, char **argv)
{
	// Tell the server to exit the level...
	if (GetGameType() == SINGLE)
	{
	    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_EXITLEVEL);
		g_pLTClient->EndMessage(hMessage);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleExitLevel()
//
//	PURPOSE:	Update player state change
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleExitLevel(HMESSAGEREAD hMessage)
{
	if (GetGameType() != SINGLE)
	{
		m_eLevelEnd = (LevelEnd)g_pLTClient->ReadFromMessageByte(hMessage);
		m_nEndString = (int)g_pLTClient->ReadFromMessageDWord(hMessage);
		m_InterfaceMgr.SwitchToFolder(FOLDER_ID_MP_SUMMARY);
		m_InterfaceMgr.StartScreenFadeIn(0.5f);
	}
	else
	{
		// Nothing in the message...currently...
		ExitLevel();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ExitLevel()
//
//	PURPOSE:	Exit this level and go to the next level
//
// ----------------------------------------------------------------------- //

void CGameClientShell::ExitLevel()
{
    g_pLTClient->ClearInput(); // Start next level with a clean slate

	TurnOffAlternativeCamera(CT_FULLSCREEN);


	// We are officially no longer in a world...

    m_bInWorld = LTFALSE;


	// Go to the the next level...

	m_nCurrentLevel++;

	MISSION* pMission = g_pMissionMgr->GetMission(m_nCurrentMission);
	if (!pMission)
	{
        g_pLTClient->CPrint("ERROR in CGameClientShell::ExitLevel():");
        g_pLTClient->CPrint("      Invalid mission %d!", m_nCurrentMission);
		return;
	}

	// See if we finished a mission...

	if (m_nCurrentLevel >= pMission->nNumLevels)
	{
		//do end of mission clean up
		GetPlayerSummary()->CompleteMission(m_nCurrentMission);

		m_nCurrentLevel = 0;
		m_nCurrentMission++;


		// Check to see if the game is over...

		if (m_nCurrentMission >= g_pMissionMgr->GetNumMissions())
		{
			// TODO: Do game over...

			// For now just start over at the beginning...

			m_nCurrentMission = 0;

			// Show the mission summary...

			m_InterfaceMgr.SwitchToFolder(FOLDER_ID_MISSION);
		}
		else
		{
			// Show the mission summary...

			m_InterfaceMgr.SwitchToFolder(FOLDER_ID_MISSION);
		}
	}
	else
	{
		// Load the next level...

		if (!LoadCurrentLevel())
		{
            g_pLTClient->CPrint("ERROR in CGameClientShell::ExitLevel():");
            g_pLTClient->CPrint("      Couldn't start level %d!", m_nCurrentLevel);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetSpectatorMode()
//
//	PURPOSE:	Turn spectator mode on/off
//
// ----------------------------------------------------------------------- //

void CGameClientShell::SetSpectatorMode(LTBOOL bOn)
{
	m_bSpectatorMode = bOn;

	// Don't show stats in spectator mode...

	m_InterfaceMgr.DrawPlayerStats(!bOn);

	if (m_PlayerCamera.IsFirstPerson())
	{
        ShowPlayer(LTFALSE);

		m_MoveMgr.SetSpectatorMode(bOn);
	}

	m_weaponModel.SetVisible(!m_bSpectatorMode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::LoadWorld()
//
//	PURPOSE:	Handles loading a world (with AutoSave)
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::LoadWorld(char* pWorldFile, char* pCurWorldSaveFile,
                                  char* pRestoreObjectsFile, uint8 nFlags)
{
	// Auto save the newly loaded level...
	return DoLoadWorld(pWorldFile, pCurWorldSaveFile, pRestoreObjectsFile, nFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DoLoadWorld()
//
//	PURPOSE:	Does actual work of loading a world
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::DoLoadWorld(char* pWorldFile, char* pCurWorldSaveFile,
                                    char* pRestoreObjectsFile, uint8 nFlags,
									char *pRecordFile, char *pPlaydemoFile)
{
    if (!pWorldFile) return LTFALSE;


	CMissionData* pMissionData = m_InterfaceMgr.GetMissionData();
	_ASSERT(pMissionData);
    if (!pMissionData) return LTFALSE;


	// Make sure the FOV is set correctly...

    uint32 dwWidth = 640;
    uint32 dwHeight = 480;
    g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

    g_pLTClient->SetCameraRect(m_hCamera, LTFALSE, 0, 0, dwWidth, dwHeight);
	SetCameraFOV(DEG2RAD(g_vtFOVXNormal.GetFloat()), DEG2RAD(g_vtFOVYNormal.GetFloat()));

	// See if the loaded world is a custom level or not...

	int nMissionId, nLevel;
	if (g_pMissionMgr->IsMissionLevel(pWorldFile, nMissionId, nLevel))
	{
		m_nCurrentMission	= nMissionId;
		m_nCurrentLevel		= nLevel;
        m_bIsCustomLevel    = LTFALSE;

		// Set the level in the mission data...
		if (nMissionId != pMissionData->GetMissionNum())
		{
			pMissionData->NewMission(nMissionId);

			CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();
			pStats->ClearMissionDamage();
		}

		MISSION* pMission = g_pMissionMgr->GetMission(m_nCurrentMission);
		if (pMission)
		{
			int missionId = pMission->nNameId;
			HSTRING hTxt=g_pLTClient->FormatString(missionId);
			if (pMission->nNumLevels == 1)
			{
				g_pInterfaceMgr->SetLoadLevelString(hTxt);
			}
			else
			{
				char tmp[128];
				HSTRING hTxt2=g_pLTClient->FormatString(IDS_SCENENUMBER,m_nCurrentLevel + 1);
				sprintf(tmp,"%s, %s",g_pLTClient->GetStringData(hTxt),g_pLTClient->GetStringData(hTxt2));
				HSTRING hWorld =g_pLTClient->CreateString(tmp);
				g_pInterfaceMgr->SetLoadLevelString(hWorld);
				g_pLTClient->FreeString(hWorld);
				g_pLTClient->FreeString(hTxt2);
			}
			if (pMission->szPhoto)
			{
				g_pInterfaceMgr->SetLoadLevelPhoto(pMission->szPhoto);
			}
			else
			{
				g_pInterfaceMgr->SetLoadLevelPhoto("interface\\photo\\missions\\default.pcx");
			}
			g_pLTClient->FreeString(hTxt);

		}

		pMissionData->SetLevelNum(m_nCurrentLevel);
	}
	else
	{
        m_bIsCustomLevel = LTTRUE;

		// No mission data in custom levels...

		pMissionData->Clear();
		char *pWorld = strrchr(pWorldFile,'\\');
        if(!pWorld)
        {
            pWorld = strrchr(pWorldFile, '/');
        }
        if(pWorld)
        {
            pWorld++;
            HSTRING hWorld = g_pLTClient->CreateString(pWorld);
            g_pInterfaceMgr->SetLoadLevelString(hWorld);
            g_pLTClient->FreeString(hWorld);
        }

		char szPhoto[512];
		SAFE_STRCPY(szPhoto,pWorldFile);
		strtok(szPhoto,".");
		strcat(szPhoto,".pcx");
		g_pInterfaceMgr->SetLoadLevelPhoto(szPhoto);

	}

	// Change to the loading level state...

	m_InterfaceMgr.ChangeState(GS_LOADINGLEVEL);

	// Check for special case of not being connected to a server or going to
	// single player mode from multiplayer...

	int nGameMode = 0;
    g_pLTClient->GetGameMode(&nGameMode);
    if (pRecordFile || pPlaydemoFile || !g_pLTClient->IsConnected() ||
		(nGameMode != STARTGAME_NORMAL && nGameMode != GAMEMODE_NONE))
	{
		StartGameRequest request;
		memset(&request, 0, sizeof(StartGameRequest));

		// Start with clean slate
		NetGame sNetGame;
		NetClientData sNetClientData;
		memset(&sNetGame, 0, sizeof(NetGame));
		memset(&sNetClientData, 0, sizeof(NetClientData));
		request.m_pGameInfo = &sNetGame;
		request.m_GameInfoLen = sizeof(NetGame_t);
		request.m_pClientData   = &sNetClientData;
		request.m_ClientDataLen = sizeof(NetClientData_t);
		request.m_Type = STARTGAME_NORMAL;

		if(pRecordFile)
		{
			SAFE_STRCPY(request.m_RecordFilename, pRecordFile);
			SAFE_STRCPY(request.m_WorldName, pWorldFile);
		}

		if(pPlaydemoFile)
		{
			SAFE_STRCPY(request.m_PlaybackFilename, pPlaydemoFile);
		}

        LTRESULT dr = g_pLTClient->StartGame(&request);
		if (dr != LT_OK)
		{
            return LTFALSE;
		}

		if(pPlaydemoFile)
		{
			// If StartGameRequest::m_PlaybackFilename is filled in the engine fills in m_WorldName.
			pWorldFile = request.m_WorldName;
		}
	}



	// Send the mission data to the server so the server can update
	// the player as necessary...

	m_InterfaceMgr.SendMissionDataToServer();



	// Send a message to the server shell with the needed info...

    HSTRING hWorldFile          = g_pLTClient->CreateString(pWorldFile);
    HSTRING hCurWorldSaveFile   = g_pLTClient->CreateString(pCurWorldSaveFile ? pCurWorldSaveFile : (char *)" ");
    HSTRING hRestoreObjectsFile = g_pLTClient->CreateString(pRestoreObjectsFile ? pRestoreObjectsFile : (char *)" ");

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_LOAD_GAME);
    g_pLTClient->WriteToMessageByte(hMessage, nFlags);
    g_pLTClient->WriteToMessageByte(hMessage, m_eDifficulty);
    g_pLTClient->WriteToMessageByte(hMessage, m_bFadeBodies);
    g_pLTClient->WriteToMessageHString(hMessage, hWorldFile);
    g_pLTClient->WriteToMessageHString(hMessage, hCurWorldSaveFile);
    g_pLTClient->WriteToMessageHString(hMessage, hRestoreObjectsFile);

	BuildClientSaveMsg(hMessage);

    g_pLTClient->EndMessage(hMessage);

    g_pLTClient->FreeString(hWorldFile);
    g_pLTClient->FreeString(hCurWorldSaveFile);
    g_pLTClient->FreeString(hRestoreObjectsFile);


	//if we've loaded a game, we're now in a single player game!
	SetGameType(SINGLE);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::LoadGame()
//
//	PURPOSE:	Handles loading a saved game
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::LoadGame(char* pWorld, char* pObjectsFile)
{
    if (!pWorld || !pObjectsFile) return LTFALSE;

	TurnOffAlternativeCamera(CT_FULLSCREEN);

	// if we're playing multiplayer, I know I'm about to disconnect so don't warn me
	if (IsInWorld() && GetGameType() != SINGLE)
		g_pInterfaceMgr->StartingNewGame();

    return DoLoadWorld(pWorld, LTNULL, pObjectsFile, LOAD_RESTORE_GAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SaveGame()
//
//	PURPOSE:	Handles saving a game...
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::SaveGame(char* pObjectsFile)
{
    if (!pObjectsFile) return LTFALSE;

    uint8 nFlags = 0;

	// Save the level objects...

    HSTRING hSaveObjectsName = g_pLTClient->CreateString(pObjectsFile);

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_SAVE_GAME);
    g_pLTClient->WriteToMessageByte(hMessage, nFlags);
    g_pLTClient->WriteToMessageHString(hMessage, hSaveObjectsName);

	BuildClientSaveMsg(hMessage);

    g_pLTClient->EndMessage(hMessage);

    g_pLTClient->FreeString(hSaveObjectsName);

	char strSaveGame[256];
	sprintf (strSaveGame, "%s|%s",m_strCurrentWorldName, pObjectsFile);
	CWinUtil::WinWritePrivateProfileString (GAME_NAME, "Continue", strSaveGame, SAVEGAMEINI_FILENAME);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsJoystickEnabled()
//
//	PURPOSE:	Determines whether or not there is a joystick device
//				enabled
//
// ----------------------------------------------------------------------- //

LTBOOL CGameClientShell::IsJoystickEnabled()
{
	// first attempt to find a joystick device

	char strJoystick[128];
	memset (strJoystick, 0, 128);
    LTRESULT result = g_pLTClient->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
    if (result != LT_OK) return LTFALSE;

	// ok - we found the device and have a name...see if it's enabled

    LTBOOL bEnabled = LTFALSE;
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

LTBOOL CGameClientShell::EnableJoystick()
{
	// first attempt to find a joystick device

	char strJoystick[128];
	memset(strJoystick, 0, 128);
    LTRESULT result = g_pLTClient->GetDeviceName(DEVICETYPE_JOYSTICK, strJoystick, 127);
    if (result != LT_OK) return LTFALSE;

	// ok, now try to enable the device

	char strConsole[256];
	sprintf(strConsole, "EnableDevice \"%s\"", strJoystick);
    g_pLTClient->RunConsoleString(strConsole);

    LTBOOL bEnabled = LTFALSE;
    g_pLTClient->IsDeviceEnabled(strJoystick, &bEnabled);

	return bEnabled;
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

	if (m_hDebugInfo)
	{
        g_pLTClient->DeleteSurface(m_hDebugInfo);
		m_hDebugInfo = NULL;
	}


	// Check to see if we should show the player position...

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_bShowPlayerPos && hPlayerObj)
	{
        LTVector vPos;
        g_pLTClient->GetObjectPos(hPlayerObj, &vPos);

		sprintf(buf, "Pos(%.0f,%.0f,%.0f)", vPos.x, vPos.y, vPos.z);

		CreateDebugSurface(buf);
	}

	if (m_bShowCamPosRot)
	{
        LTVector vPos;
        g_pLTClient->GetObjectPos(m_hCamera, &vPos);

		// Convert pitch and yaw to the same units used by DEdit...

		LTFLOAT fYawDeg = RAD2DEG(m_fYaw);
		while (fYawDeg < 0.0f)
		{
			fYawDeg += 360.0f;
		}
		while (fYawDeg > 360.0f)
		{
			fYawDeg -= 360.0f;
		}

		LTFLOAT fPitchDeg = RAD2DEG(m_fPitch);
		while (fPitchDeg < 0.0f)
		{
			fPitchDeg += 360.0f;
		}
		while (fPitchDeg > 360.0f)
		{
			fPitchDeg -= 360.0f;
		}

		sprintf(buf, "CamPos(%.0f,%.0f,%.0f)|Pitch(%.0f)|Yaw(%.0f)",
			vPos.x, vPos.y, vPos.z, fPitchDeg, fYawDeg);

		CreateDebugSurface(buf);
	}


	// See if the FOV has changed...

	if (!m_bUsingExternalCamera && !m_bZooming && !m_nZoomView && !IsLiquid(m_eCurContainerCode))
	{
		LTFLOAT fovX, fovY;
		g_pLTClient->GetCameraFOV(m_hCamera, &fovX, &fovY);

		if (fovX != g_vtFOVXNormal.GetFloat() || fovY != g_vtFOVYNormal.GetFloat())
		{
			SetCameraFOV(DEG2RAD(g_vtFOVXNormal.GetFloat()), DEG2RAD(g_vtFOVYNormal.GetFloat()));
		}
	}


	// Check to see if we are in object edit mode...

	if (m_editMgr.IsEditMode() && !g_bScreenShotMode)
	{
		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (hPlayerObj)
		{
			LTVector vPos;
			g_pLTClient->GetObjectPos(hPlayerObj, &vPos);
			sprintf(buf, "Pos(%.0f,%.0f,%.0f)", vPos.x, vPos.y, vPos.z);
			CreateDebugSurface(buf);
		}

		sprintf(buf, "EDIT MODE (Object: '%s')", m_editMgr.GetCurObjectName());
		CreateDebugSurface(buf);
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
            g_pLTClient->DeleteObject(m_hBoundingBox);
            m_hBoundingBox = LTNULL;
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
//	ROUTINE:	CGameClientShell::CreateDebugSurface
//
//	PURPOSE:	Create a surface with debug info on it.
//
// --------------------------------------------------------------------------- //

void CGameClientShell::CreateDebugSurface(char* strMessage)
{
	if (!strMessage || strMessage[0] == '\0' || m_hDebugInfo) return;

	m_hDebugInfo = g_pInterfaceResMgr->CreateSurfaceFromString(
        g_pInterfaceResMgr->GetLargeFont(), strMessage, g_hColorTransparent);
    g_pLTClient->OptimizeSurface(m_hDebugInfo, g_hColorTransparent);

    uint32 cx, cy;
    g_pLTClient->GetSurfaceDims(m_hDebugInfo, &cx, &cy);
	m_rcDebugInfo.left   = 0;
	m_rcDebugInfo.top    = 0;
	m_rcDebugInfo.right  = (int)cx;
	m_rcDebugInfo.bottom = (int)cy;
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
			if (!m_bSpectatorMode)
			{
				m_bTweakingWeaponMuzzle = !m_bTweakingWeaponMuzzle;

				m_MoveMgr.AllowMovement(!m_bTweakingWeaponMuzzle);

				// Save tweaks...

				if (!m_bTweakingWeaponMuzzle)
				{
                    g_pWeaponMgr->WriteFile(g_pLTClient);
				}
			}
		}
		break;

		case CHEAT_POSWEAPON :
		{
			if (!m_bSpectatorMode)
			{
				m_bTweakingWeapon = !m_bTweakingWeapon;

				m_MoveMgr.AllowMovement(!m_bTweakingWeapon);

				// Save tweaks...

				if (!m_bTweakingWeapon)
				{
                    g_pWeaponMgr->WriteFile(g_pLTClient);
				}
			}
		}
		break;

		case CHEAT_POSBREACH :
		{
			if (!m_bSpectatorMode)
			{
				m_bAdjustWeaponBreach	= !m_bAdjustWeaponBreach;

				m_MoveMgr.AllowMovement(!m_bAdjustWeaponBreach);

				// Save tweaks...

				if (!m_bAdjustWeaponBreach)
				{
                    g_pWeaponMgr->WriteFile(g_pLTClient);
				}
			}
		}
		break;

		case CHEAT_POS1STCAM :
		{
			if (!m_bSpectatorMode)
			{
				m_bAdjust1stPersonCamera = !m_bAdjust1stPersonCamera;

				m_MoveMgr.AllowMovement(!m_bAdjust1stPersonCamera);
			}
		}
		break;

		case CHEAT_LIGHTSCALE :
		{
			m_bAdjustLightScale = !m_bAdjustLightScale;

			m_MoveMgr.AllowMovement(!m_bAdjustLightScale);
		}
		break;

		case CHEAT_LIGHTADD :
		{
			m_bAdjustLightAdd = !m_bAdjustLightAdd;

			m_MoveMgr.AllowMovement(!m_bAdjustLightAdd);
		}
		break;

		case CHEAT_FOV :
		{
			m_bAdjustFOV = !m_bAdjustFOV;

			m_MoveMgr.AllowMovement(!m_bAdjustFOV);
		}
		break;

		case CHEAT_INTERFACEADJUST :
		{
//			m_InterfaceMgr.ToggleInterfaceAdjust();
		}
		break;

		case CHEAT_CHASETOGGLE :
		{
			if (m_ePlayerState == PS_ALIVE && !m_nZoomView)
			{
				SetExternalCamera(m_PlayerCamera.IsFirstPerson());
			}
		}
		break;


		default : break;
	}
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

	char buf[200];
    m_bFirstUpdate = LTFALSE;


	// Force the player camera to update...

	UpdatePlayerCamera();


	// Set up the level-start screen fade...

	if (g_varStartLevelScreenFade.GetFloat())
	{
		m_InterfaceMgr.StartScreenFadeIn(g_varStartLevelScreenFadeTime.GetFloat());
	}


	// Initialize model warble sheeyot...

    g_pLTClient->RunConsoleString("+ModelWarble 0");
    g_pLTClient->RunConsoleString("+WarbleSpeed 15");
    g_pLTClient->RunConsoleString("+WarbleScale .95");


	// Set prediction if we are playing multiplayer...We turn this
	// off for single player because projectiles look MUCH better...

	if (IsMultiplayerGame())
	{
	   g_pLTClient->RunConsoleString("Prediction 1");
	}
	else
	{
	   g_pLTClient->RunConsoleString("Prediction 0");
	}


	// Set up the panning sky values

    m_bPanSky = (LTBOOL) g_pLTClient->GetServerConVarValueFloat("PanSky");
    m_fPanSkyOffsetX = g_pLTClient->GetServerConVarValueFloat("PanSkyOffsetX");
    m_fPanSkyOffsetZ = g_pLTClient->GetServerConVarValueFloat("PanSkyOffsetX");
    m_fPanSkyScaleX = g_pLTClient->GetServerConVarValueFloat("PanSkyScaleX");
    m_fPanSkyScaleZ = g_pLTClient->GetServerConVarValueFloat("PanSkyScaleZ");

	char* pTexture = LTNULL;
	if (m_bPanSky)
	{
		pTexture = g_pLTClient->GetServerConVarValueString("PanSkyTexture");
	}

    g_pLTClient->SetGlobalPanTexture(GLOBALPAN_SKYSHADOW, pTexture);


	// Set misc console vars...

	MirrorSConVar("AllSkyPortals", "AllSkyPortals");


	// Set up the environment map (chrome) texture...

    char* pEnvMap = g_pLTClient->GetServerConVarValueString("EnvironmentMap");
    const char* pVal = ((!pEnvMap || !pEnvMap[0]) ? "Tex\\Chrome.dtx" : pEnvMap);
	sprintf(buf, "EnvMap %s", pVal);
    g_pLTClient->RunConsoleString(buf);

 	m_nGlobalSoundFilterId = 0;

	char* pGlobalSoundFilter = g_pLTClient->GetServerConVarValueString("GlobalSoundFilter");
	if (pGlobalSoundFilter && pGlobalSoundFilter[0])
	{
		SOUNDFILTER* pFilter = g_pSoundFilterMgr->GetFilter(pGlobalSoundFilter);
		if (pFilter)
		{
			m_nGlobalSoundFilterId = pFilter->nId;
		}
	}

	// Set up the global (per level) wind values...

    g_vWorldWindVel.x = g_pLTClient->GetServerConVarValueFloat("WindX");
    g_vWorldWindVel.y = g_pLTClient->GetServerConVarValueFloat("WindY");
    g_vWorldWindVel.z = g_pLTClient->GetServerConVarValueFloat("WindZ");


	// Set up the global (per level) fog values...

	ResetGlobalFog();


	// Initialize the music playlists...

	if (m_Music.IsInitialized() && (SINGLE == m_eGameType) )
	{
		// Initialize music for the current level...

		char* pMusicDirectory = g_pLTClient->GetServerConVarValueString("MusicDirectory");
		char* pMusicControlFile = g_pLTClient->GetServerConVarValueString("MusicControlFile");

		if (pMusicDirectory && pMusicControlFile)
		{
			CMusicState MusicState;
			strcpy(MusicState.szDirectory, pMusicDirectory);
			strcpy(MusicState.szControlFile, pMusicControlFile);

			m_Music.RestoreMusicState(MusicState);
		}

		//m_InterfaceMgr.RestoreGameMusic();
		//m_Music.Play();
	}


	// Force us to re-evaluate what container we're in.  We call
	// UpdateContainerFX() first to make sure any container changes
	// have been accounted for, then we clear the container code
	// and force an update (this is done for underwater situations like
	// dying underwater and respawning, and also for picking up intelligence
	// items underwater)...
	UpdateContainerFX();
	ClearCurContainerCode();
	UpdateContainerFX();
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
	float fVal;

    fVal = g_pLTClient->GetServerConVarValueFloat(pSVarName);

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

void CGameClientShell::ResetGlobalFog()
{
	// Set the FarZ for the level...

	MirrorSConVar("FarZ", "FarZ");


	// See if fog should be disabled

    uint32 dwAdvancedOptions = m_InterfaceMgr.GetAdvancedOptions();

	if (!(dwAdvancedOptions & AO_FOG))
	{
        g_pLTClient->RunConsoleString("FogEnable 0");
		return;
	}

	MirrorSConVar("FogEnable", "FogEnable");
	MirrorSConVar("FogNearZ", "FogNearZ");
	MirrorSConVar("FogFarZ", "FogFarZ");
	MirrorSConVar("LMAnimStatic", "LMAnimStatic");

    LTVector todScale = m_LightScaleMgr.GetTimeOfDayScale();

	char buf[255];
    LTFLOAT fVal = g_pLTClient->GetServerConVarValueFloat("FogR") * todScale.x;
	sprintf(buf, "FogR %d", (int)fVal);
    g_pLTClient->RunConsoleString(buf);

    fVal = g_pLTClient->GetServerConVarValueFloat("FogG") * todScale.y;
	sprintf(buf, "FogG %d", (int)fVal);
    g_pLTClient->RunConsoleString(buf);

    fVal = g_pLTClient->GetServerConVarValueFloat("FogB") * todScale.z;
	sprintf(buf, "FogB %d", (int)fVal);
    g_pLTClient->RunConsoleString(buf);

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


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ShowPlayer()
//
//	PURPOSE:	Show/Hide the player object
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ShowPlayer(LTBOOL bShow)
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	// Comment this out to show player model in 1st person view...
#define DO_NORMAL_PLAYER_HIDE_SHOW

    uint32 dwFlags = g_pLTClient->GetObjectFlags(hPlayerObj);
	if (bShow)
	{
		dwFlags |= FLAG_VISIBLE;
        g_pLTClient->SetObjectFlags(hPlayerObj, dwFlags);

#ifndef DO_NORMAL_PLAYER_HIDE_SHOW
		HMODELPIECE hPiece;
        g_pLTClient->GetModelLT()->GetPiece(hPlayerObj, "Torso", hPiece);
        g_pLTClient->GetModelLT()->SetPieceHideStatus(hPlayerObj, hPiece, LTFALSE);
        g_pLTClient->GetModelLT()->GetPiece(hPlayerObj, "Head_zTex1", hPiece);
        g_pLTClient->GetModelLT()->SetPieceHideStatus(hPlayerObj, hPiece, LTFALSE);
#endif

	}
	else if (!bShow)
	{

#ifdef DO_NORMAL_PLAYER_HIDE_SHOW
		dwFlags &= ~FLAG_VISIBLE;
#else
		dwFlags |= FLAG_VISIBLE; // | FLAG_REALLYCLOSE;

		HMODELPIECE hPiece;
        g_pLTClient->GetModelLT()->GetPiece(hPlayerObj, "Torso", hPiece);
        g_pLTClient->GetModelLT()->SetPieceHideStatus(hPlayerObj, hPiece, LTTRUE);
        g_pLTClient->GetModelLT()->GetPiece(hPlayerObj, "Head_zTex1", hPiece);
        g_pLTClient->GetModelLT()->SetPieceHideStatus(hPlayerObj, hPiece, LTTRUE);
#endif

        g_pLTClient->SetObjectFlags(hPlayerObj, dwFlags);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateServerPlayerModel()
//
//	PURPOSE:	Puts the server's player model where our invisible one is
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdateServerPlayerModel()
{
	HOBJECT hClientObj, hRealObj;
    LTRotation myRot;
    LTVector myPos;

    if (!(hClientObj = g_pLTClient->GetClientObject())) return;

	if (!(hRealObj = m_MoveMgr.GetObject())) return;

    g_pLTClient->GetObjectPos(hRealObj, &myPos);
    g_pLTClient->SetObjectPos(hClientObj, &myPos);

	if (g_vtPlayerRotate.GetFloat(1.0) > 0.0)
	{
        g_pLTClient->SetupEuler(&myRot, m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
        g_pLTClient->SetObjectRotation(hClientObj, &myRot);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::RenderCamera()
//
//	PURPOSE:	Sets up the client and renders the camera
//
// ----------------------------------------------------------------------- //

void CGameClientShell::RenderCamera(LTBOOL bDrawInterface)
{
	if (!m_bCameraPosInited) return;

	// Make sure the rendered player object is right where it should be.

	UpdateServerPlayerModel();

	// Make sure we process attachments before updating the weapon model
	// and special fx...(some fx are based on attachment positions/rotations)

    g_pLTClient->ProcessAttachments(g_pLTClient->GetClientObject());

	// Make sure the weapon is updated before we render the camera...

	UpdateWeaponModel();

	// Make sure the move-mgr models are updated before we render...

	m_MoveMgr.UpdateModels();

	// Update any client-side special effects...

	m_DamageFXMgr.Update();

	// Important to update this after the weapon model has been updated
	// (some fx depend on the position of the weapon model)...

	m_sfxMgr.UpdateSpecialFX();

	// Update the flash light...

	m_FlashLight.Update();

    g_pLTClient->Start3D();
    g_pLTClient->RenderCamera(m_hCamera);
    g_pLTClient->StartOptimized2D();

	m_InterfaceMgr.Draw();

	// Display any necessary debugging info...

	if (m_hDebugInfo)
	{
		// Get the screen width and height...

        HSURFACE hScreen = g_pLTClient->GetScreenSurface();
        uint32 nScreenWidth, nScreenHeight;
        g_pLTClient->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);

		int x = nScreenWidth  - (m_rcDebugInfo.right - m_rcDebugInfo.left);
		int y = nScreenHeight - (m_rcDebugInfo.bottom - m_rcDebugInfo.top);

        g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hDebugInfo,
            &m_rcDebugInfo, x, y, g_hColorTransparent);
	}

    g_pLTClient->EndOptimized2D();
    g_pLTClient->End3D();

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdatePlayer()
//
//	PURPOSE:	Update the player
//
// ----------------------------------------------------------------------- //

void CGameClientShell::UpdatePlayer()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj || IsPlayerDead()) return;


	// This is pretty much a complete kludge, but I can't really think of
	// a better way to handle this...Okay, since the server can update the
	// player's flags at any time (and override anything that we set), we'll
	// make sure that the player's flags are always what we want them to be :)

    uint32 dwPlayerFlags = g_pLTClient->GetObjectFlags(hPlayerObj);
	if (m_PlayerCamera.IsFirstPerson())
	{
		if (dwPlayerFlags & FLAG_VISIBLE)
		{
            ShowPlayer(LTFALSE);
		}
	}
	else  // Third person
	{
		if (!(dwPlayerFlags & FLAG_VISIBLE))
		{
            ShowPlayer(LTTRUE);
		}
	}


	// Hide/Show our attachments...

	HideShowAttachments(hPlayerObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HideShowAttachments()
//
//	PURPOSE:	Recursively hide/show attachments...
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HideShowAttachments(HOBJECT hObj)
{
	if (!hObj) return;

	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

    g_pLTClient->GetAttachments(hObj, attachList, 20, &dwListSize, &dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	for (int i=0; i < nNum; i++)
	{
        uint32 dwUsrFlags;
        g_pLTClient->GetObjectUserFlags(attachList[i], &dwUsrFlags);

        if (dwUsrFlags & USRFLG_ATTACH_HIDE1SHOW3)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(attachList[i]);

			if (m_PlayerCamera.IsFirstPerson())
			{
				if (dwFlags & FLAG_VISIBLE)
				{
					dwFlags &= ~FLAG_VISIBLE;
                    g_pLTClient->SetObjectFlags(attachList[i], dwFlags);
				}

				if (!(dwFlags & FLAG_PORTALVISIBLE))
				{
					dwFlags |= FLAG_PORTALVISIBLE;
					g_pLTClient->SetObjectFlags(attachList[i], dwFlags);
				}
			}
			else
			{
				if (!(dwFlags & FLAG_VISIBLE))
				{
					dwFlags |= FLAG_VISIBLE;
                    g_pLTClient->SetObjectFlags(attachList[i], dwFlags);
				}
			}
		}
		else if (dwUsrFlags & USRFLG_ATTACH_HIDE1)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(attachList[i]);

			if (m_PlayerCamera.IsFirstPerson())
			{
				if (dwFlags & FLAG_VISIBLE)
				{
					dwFlags &= ~FLAG_VISIBLE;
                    g_pLTClient->SetObjectFlags(attachList[i], dwFlags);
				}
			}
		}

		// Hide/Show this attachment's attachments...
		HideShowAttachments(attachList[i]);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::Update3rdPersonInfo
//
//	PURPOSE:	Update the 3rd person cross hair / camera info
//
// ----------------------------------------------------------------------- //

void CGameClientShell::Update3rdPersonInfo()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj || IsPlayerDead()) return;

    HOBJECT hFilterList[] = {hPlayerObj, m_MoveMgr.GetObject(), LTNULL};


	ClientIntersectInfo info;
	ClientIntersectQuery query;
    LTVector vPlayerPos, vUp, vRight, vForward;

    g_pLTClient->GetObjectPos(hPlayerObj, &vPlayerPos);

    LTFLOAT fCrosshairDist = -1.0f;
    LTFLOAT fCameraOptZ = g_vtChaseCamDistBack.GetFloat();

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_weaponModel.GetWeaponId());
	if (!pWeapon) return;

	// Figure out crosshair distance...

	if (m_InterfaceMgr.IsCrosshairOn() && m_weaponModel.GetHandle())
	{
        fCrosshairDist = (LTFLOAT) pWeapon->nRange;

        g_pLTClient->GetRotationVectors(&m_rRotation, &vUp, &vRight, &vForward);

		// Determine where the cross hair should be...

        LTVector vStart, vEnd, vPos;
		VEC_COPY(vStart, vPlayerPos);
		VEC_MULSCALAR(vEnd, vForward, fCrosshairDist);
		VEC_ADD(vEnd, vEnd, vStart);

		VEC_COPY(query.m_From, vStart);
		VEC_COPY(query.m_To, vEnd);

		query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		query.m_FilterFn = ObjListFilterFn;
		query.m_pUserData = hFilterList;

        if (g_pLTClient->IntersectSegment (&query, &info))
		{
			VEC_COPY(vPos, info.m_Point);
		}
		else
		{
			VEC_COPY(vPos, vEnd);
		}

        LTVector vTemp;
		VEC_SUB(vTemp, vPos, vStart);

		fCrosshairDist = VEC_MAG(vTemp);
	}


	// Figure out optinal camera distance...

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(hPlayerObj, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);
	VEC_NORM(vForward);

	// Determine how far behind the player the camera can go...

    LTVector vEnd;
	VEC_MULSCALAR(vEnd, vForward, -fCameraOptZ);
	VEC_ADD(vEnd, vEnd, vPlayerPos);

	VEC_COPY(query.m_From, vPlayerPos);
	VEC_COPY(query.m_To, vEnd);

	query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	query.m_FilterFn = ObjListFilterFn;
	query.m_pUserData = hFilterList;

    if (g_pLTClient->IntersectSegment (&query, &info))
	{
        LTVector vTemp;
		VEC_SUB(vTemp, info.m_Point, vPlayerPos);
        LTFLOAT fDist = VEC_MAG(vTemp);

		fCameraOptZ = fDist < fCameraOptZ ? -(fDist - 5.0f) : -fCameraOptZ;
	}
	else
	{
		fCameraOptZ = -fCameraOptZ;
	}


	Update3rdPersonCrossHair(fCrosshairDist);
	m_PlayerCamera.SetOptZ(fCameraOptZ);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	UpdateModelGlow
//
//	PURPOSE:	Update the current model glow color
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateModelGlow()
{
    LTFLOAT fColor      = 0.0f;
	LTFLOAT	fMin		= g_vtModelGlowMin.GetFloat();
	LTFLOAT	fMax		= g_vtModelGlowMax.GetFloat();
    LTFLOAT fColorRange = fMax - fMin;
	LTFLOAT fTimeRange  = g_vtModelGlowTime.GetFloat();

	if (m_bModelGlowCycleUp)
	{
		if (m_fModelGlowCycleTime < fTimeRange)
		{
			fColor = fMin + (fColorRange * (m_fModelGlowCycleTime / fTimeRange));
			m_vCurModelGlow.Init(fColor, fColor, fColor);
		}
		else
		{
			m_fModelGlowCycleTime = 0.0f;
			m_vCurModelGlow.Init(fMax, fMax, fMax);
			m_bModelGlowCycleUp = LTFALSE;
			return;
		}
	}
	else
	{
		if (m_fModelGlowCycleTime < fTimeRange)
		{
			fColor = fMax - (fColorRange * (m_fModelGlowCycleTime / fTimeRange));
			m_vCurModelGlow.Init(fColor, fColor, fColor);
		}
		else
		{
			m_fModelGlowCycleTime = 0.0f;
			m_vCurModelGlow.Init(fMin, fMin, fMin);
            m_bModelGlowCycleUp = LTTRUE;
			return;
		}
	}

	m_fModelGlowCycleTime += m_fFrameTime;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::InitSinglePlayer
//
//	PURPOSE:	Send the server the initial single player info
//
// --------------------------------------------------------------------------- //

void CGameClientShell::InitSinglePlayer()
{
	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (!pSettings) return;

	m_eGameType = SINGLE;

	// Init player variables on server...

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_INITVARS);
    g_pLTClient->WriteToMessageByte(hMessage, (uint8)pSettings->RunLock());
    g_pLTClient->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::InitMultiPlayer
//
//	PURPOSE:	Send the server the initial multiplayer info
//
// --------------------------------------------------------------------------- //
void CGameClientShell::InitMultiPlayer()
{
	if (!IsMultiplayerGame()) return;

	char	sName[255];
	char	sMod[255];
	char	sSkin[255];
	char	sHead[255];
    int		nTeam;

	SAFE_STRCPY(sName,g_vtPlayerName.GetStr());
	GetConsoleString("NetPlayerModel",sMod,"Hero,action");
	GetConsoleString("NetPlayerSkin",sSkin,"");
	GetConsoleString("NetPlayerHead",sHead,"");
	nTeam = GetConsoleInt("NetPlayerTeam",0);

	if ( !sSkin[0] || !sHead[0] )
	{
		char szTemp[512];
		strcpy(szTemp, sMod);
		if ( strchr(szTemp, ',') )
		{
			*strchr(szTemp, ',') = '_';
			_strlwr(szTemp);
			strcpy(sSkin, g_pModelButeMgr->GetButeMgr()->GetString(szTemp, "Skin0"));
			strcpy(sHead, g_pModelButeMgr->GetButeMgr()->GetString(szTemp, "Head0"));
		} 
	}

    HSTRING hstrName = g_pLTClient->CreateString(sName);
	if (!hstrName) return;
    HSTRING hstrMod = g_pLTClient->CreateString(sMod);
	if (!hstrMod) return;
    HSTRING hstrSkin = g_pLTClient->CreateString(strchr(sSkin, ',')+1);
	if (!hstrSkin) return;
    HSTRING hstrHead = g_pLTClient->CreateString(strchr(sHead, ',')+1);
	if (!hstrHead) return;

	// Init multiplayer info on server...

    HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_PLAYER_MULTIPLAYER_INIT);
    g_pLTClient->WriteToMessageHString(hWrite, hstrName);
    g_pLTClient->WriteToMessageHString(hWrite, hstrMod);
    g_pLTClient->WriteToMessageHString(hWrite, hstrSkin);
    g_pLTClient->WriteToMessageHString(hWrite, hstrHead);
    g_pLTClient->EndMessage(hWrite);

	g_pLTClient->FreeString(hstrName);
	g_pLTClient->FreeString(hstrMod);
	g_pLTClient->FreeString(hstrSkin);
	g_pLTClient->FreeString(hstrHead);

	// Init player settings...

	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (!pSettings) return;

    hWrite = g_pLTClient->StartMessage(MID_PLAYER_INITVARS);
    g_pLTClient->WriteToMessageByte(hWrite, (uint8)pSettings->RunLock());
    g_pLTClient->EndMessage(hWrite);
}






// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::BuildClientSaveMsg
//
//	PURPOSE:	Save all the necessary client-side info
//
// --------------------------------------------------------------------------- //

void CGameClientShell::BuildClientSaveMsg(HMESSAGEWRITE hMessage)
{
	if (!hMessage) return;

	{ // BL 09/30/00 - HACK to save ammo type
		g_pLTClient->WriteToMessageFloat(hMessage, m_weaponModel.GetAmmo() ? m_weaponModel.GetAmmo()->nId : -1.0f);
		g_pLTClient->WriteToMessageDWord(hMessage, m_Music.GetMusicState()->nIntensity);
	}

    HMESSAGEWRITE hData = g_pLTClient->StartHMessageWrite();

	// Save complex data members...

	m_InterfaceMgr.Save(hData);
	m_MoveMgr.Save(hData);


	// Save all necessary data members...

    g_pLTClient->WriteToMessageRotation(hData, &m_rRotation);
    g_pLTClient->WriteToMessageVector(hData, &m_vFlashColor);

    g_pLTClient->WriteToMessageByte(hData, m_eDifficulty);
    g_pLTClient->WriteToMessageByte(hData, m_bFlashScreen);
    g_pLTClient->WriteToMessageByte(hData, m_bSpectatorMode);
    g_pLTClient->WriteToMessageByte(hData, m_bLastSent3rdPerson);
//    g_pLTClient->WriteToMessageByte(hData, m_bStartedDuckingDown);
//    g_pLTClient->WriteToMessageByte(hData, m_bStartedDuckingUp);
    g_pLTClient->WriteToMessageByte(hData, m_bAllowPlayerMovement);
    g_pLTClient->WriteToMessageByte(hData, m_bLastAllowPlayerMovement);
    g_pLTClient->WriteToMessageByte(hData, m_bWasUsingExternalCamera);
    g_pLTClient->WriteToMessageByte(hData, m_bUsingExternalCamera);
    g_pLTClient->WriteToMessageByte(hData, m_ePlayerState);
    g_pLTClient->WriteToMessageByte(hData, m_nCurrentLevel);
    g_pLTClient->WriteToMessageByte(hData, m_nCurrentMission);
    g_pLTClient->WriteToMessageByte(hData, m_nSoundFilterId);
    g_pLTClient->WriteToMessageByte(hData, m_nGlobalSoundFilterId);
    g_pLTClient->WriteToMessageByte(hData, m_weaponModel.GetHolster());
    g_pLTClient->WriteToMessageByte(hData, m_FlashLight.IsOn());

    g_pLTClient->WriteToMessageFloat(hData, m_fPitch);
    g_pLTClient->WriteToMessageFloat(hData, m_fYaw);
    g_pLTClient->WriteToMessageFloat(hData, m_fRoll);
    g_pLTClient->WriteToMessageFloat(hData, m_fPlayerPitch);
    g_pLTClient->WriteToMessageFloat(hData, m_fPlayerYaw);
    g_pLTClient->WriteToMessageFloat(hData, m_fPlayerRoll);

	m_fPitchBackup = m_fPitch;
	m_fYawBackup = m_fYaw;

    g_pLTClient->WriteToMessageFloat(hData, m_fPitchBackup);
    g_pLTClient->WriteToMessageFloat(hData, m_fYawBackup);

    g_pLTClient->WriteToMessageFloat(hData, m_fFlashTime);
    g_pLTClient->WriteToMessageFloat(hData, m_fFlashStart);
    g_pLTClient->WriteToMessageFloat(hData, m_fFlashRampUp);
    g_pLTClient->WriteToMessageFloat(hData, m_fFlashRampDown);
    g_pLTClient->WriteToMessageFloat(hData, m_fFireJitterPitch);
    g_pLTClient->WriteToMessageFloat(hData, m_fFireJitterYaw);
    g_pLTClient->WriteToMessageFloat(hData, m_fContainerStartTime);
    g_pLTClient->WriteToMessageFloat(hData, m_fFovXFXDir);
    g_pLTClient->WriteToMessageFloat(hData, m_fSaveLODScale);
//    g_pLTClient->WriteToMessageFloat(hData, m_fCamDuck);
//    g_pLTClient->WriteToMessageFloat(hData, m_fDuckDownV);
//    g_pLTClient->WriteToMessageFloat(hData, m_fDuckUpV);
//    g_pLTClient->WriteToMessageFloat(hData, m_fMaxDuckDistance);
//    g_pLTClient->WriteToMessageFloat(hData, m_fStartDuckTime);

    g_pLTClient->WriteToMessageHMessageWrite(hMessage, hData);
    g_pLTClient->EndHMessageWrite(hData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UnpackClientSaveMsg
//
//	PURPOSE:	Load all the necessary client-side info
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UnpackClientSaveMsg(HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

    m_bRestoringGame = LTTRUE;

    HMESSAGEREAD hData = g_pLTClient->ReadFromMessageHMessageRead(hMessage);


	// Load complex data members...

	m_InterfaceMgr.Load(hData);
	m_MoveMgr.Load(hData);

	// Load data members...

    g_pLTClient->ReadFromMessageRotation(hData, &m_rRotation);
    g_pLTClient->ReadFromMessageVector(hData, &m_vFlashColor);

    m_eDifficulty               = (GameDifficulty) g_pLTClient->ReadFromMessageByte(hData);
    m_bFlashScreen              = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
    m_bSpectatorMode            = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
    m_bLastSent3rdPerson        = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
//    m_bStartedDuckingDown       = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
//    m_bStartedDuckingUp         = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
    m_bAllowPlayerMovement      = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
    m_bLastAllowPlayerMovement  = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
    m_bWasUsingExternalCamera   = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
    m_bUsingExternalCamera      = (LTBOOL) g_pLTClient->ReadFromMessageByte(hData);
    m_ePlayerState              = (PlayerState) g_pLTClient->ReadFromMessageByte(hData);
    m_nCurrentLevel             = g_pLTClient->ReadFromMessageByte(hData);
    m_nCurrentMission           = g_pLTClient->ReadFromMessageByte(hData);
    m_nSoundFilterId            = g_pLTClient->ReadFromMessageByte(hData);
    m_nGlobalSoundFilterId      = g_pLTClient->ReadFromMessageByte(hData);

	m_weaponModel.SetHolster(g_pLTClient->ReadFromMessageByte(hData));

	if (g_pLTClient->ReadFromMessageByte(hData))
	{
		m_FlashLight.TurnOn();
	}

    m_fPitch                    = g_pLTClient->ReadFromMessageFloat(hData);
    m_fYaw                      = g_pLTClient->ReadFromMessageFloat(hData);
    m_fRoll                     = g_pLTClient->ReadFromMessageFloat(hData);
    m_fPlayerPitch              = g_pLTClient->ReadFromMessageFloat(hData);
    m_fPlayerYaw                = g_pLTClient->ReadFromMessageFloat(hData);
    m_fPlayerRoll               = g_pLTClient->ReadFromMessageFloat(hData);
    m_fPitchBackup              = g_pLTClient->ReadFromMessageFloat(hData);
    m_fYawBackup                = g_pLTClient->ReadFromMessageFloat(hData);
    m_fFlashTime                = g_pLTClient->ReadFromMessageFloat(hData);
    m_fFlashStart               = g_pLTClient->ReadFromMessageFloat(hData);
    m_fFlashRampUp              = g_pLTClient->ReadFromMessageFloat(hData);
    m_fFlashRampDown            = g_pLTClient->ReadFromMessageFloat(hData);
    m_fFireJitterPitch          = g_pLTClient->ReadFromMessageFloat(hData);
    m_fFireJitterYaw	        = g_pLTClient->ReadFromMessageFloat(hData);
    m_fContainerStartTime       = g_pLTClient->ReadFromMessageFloat(hData);
    m_fFovXFXDir                = g_pLTClient->ReadFromMessageFloat(hData);
    m_fSaveLODScale             = g_pLTClient->ReadFromMessageFloat(hData);
//    m_fCamDuck                  = g_pLTClient->ReadFromMessageFloat(hData);
//    m_fDuckDownV                = g_pLTClient->ReadFromMessageFloat(hData);
//    m_fDuckUpV                  = g_pLTClient->ReadFromMessageFloat(hData);
//    m_fMaxDuckDistance          = g_pLTClient->ReadFromMessageFloat(hData);
//    m_fStartDuckTime            = g_pLTClient->ReadFromMessageFloat(hData);

    g_pLTClient->EndHMessageRead(hData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ProcessCheat
//
//	PURPOSE:	process a cheat.
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ProcessCheat(CheatCode nCode)
{
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleRespawn
//
//	PURPOSE:	Handle player respawn
//
// --------------------------------------------------------------------------- //

void CGameClientShell::HandleRespawn()
{
	if (!IsPlayerDead()) return;

	// if we're in multiplayer send the respawn command...

	if (IsMultiplayerGame())
	{
		// Don't send the respawn command if we are clicking around in
		// a menu...
		if (m_InterfaceMgr.GetGameState() != GS_MENU)
		{
			// send a message to the server telling it that it's ok to respawn us now...
	        HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_RESPAWN);
		    g_pLTClient->EndMessage(hMsg);
		}
	}
	else  // Bring up load game menu...
	{
		m_InterfaceMgr.SetLoadGameMenu();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::QuickSave
//
//	PURPOSE:	Quick save the game
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::QuickSave()
{
	if (IsPlayerDead() || m_bUsingExternalCamera || GetGameType() != SINGLE ||
        m_InterfaceMgr.GetGameState() != GS_PLAYING) return LTFALSE;

	// Do quick save...

    HSTRING hStr = g_pLTClient->FormatString(IDS_QUICKSAVING);
    char* pStr = g_pLTClient->GetStringData(hStr);
    CSPrint(pStr);
    g_pLTClient->FreeString(hStr);

	time_t seconds;
	time (&seconds);
	struct tm* timedate = localtime (&seconds);
	if (!timedate) return 0;

	int missionNum = -1;
	int sceneNum = -1;

	if (!g_pGameClientShell->IsCustomLevel())
	{
		missionNum = g_pGameClientShell->GetCurrentMission();
		sceneNum = g_pGameClientShell->GetCurrentLevel();
	}

	char strSaveGame[256];
	sprintf (strSaveGame, "%s|%d,%d|%ld",m_strCurrentWorldName, missionNum, sceneNum, (long)seconds);

	CWinUtil::WinWritePrivateProfileString(GAME_NAME, "SaveGame00", strSaveGame, SAVEGAMEINI_FILENAME);

    m_bQuickSave = LTTRUE;

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::QuickLoad
//
//	PURPOSE:	Quick load the game
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::QuickLoad()
{
    if (m_InterfaceMgr.GetGameState() != GS_PLAYING &&
		m_InterfaceMgr.GetGameState() != GS_FAILURE &&
		m_InterfaceMgr.GetCurrentFolder() != FOLDER_ID_FAILURE &&
		m_InterfaceMgr.GetCurrentFolder() != FOLDER_ID_SUMMARY) return LTFALSE;

	ClearScreenTint();

	char strSaveGameSetting[256];
	memset(strSaveGameSetting, 0, 256);
	CWinUtil::WinGetPrivateProfileString(GAME_NAME, "SaveGame00", "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);

	if (!*strSaveGameSetting)
	{
        HSTRING hString = g_pLTClient->FormatString(IDS_NOQUICKSAVEGAME);
        m_InterfaceMgr.ShowMessageBox(hString,LTMB_OK,LTNULL);
		g_pLTClient->FreeString(hString);

        return LTFALSE;
	}

	char* strWorldName = strtok(strSaveGameSetting,"|");

	if (!LoadGame(strWorldName, QUICKSAVE_FILENAME))
	{
        HSTRING hString = g_pLTClient->FormatString(IDS_LOADGAMEFAILED);
        m_InterfaceMgr.ShowMessageBox(hString,LTMB_OK,LTNULL);
		g_pLTClient->FreeString(hString);

        return LTFALSE;
	}

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsMultiplayerGame()
//
//	PURPOSE:	See if we are playing a multiplayer game
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::IsMultiplayerGame()
{
	int nGameMode = 0;
    g_pLTClient->GetGameMode(&nGameMode);
    if (nGameMode == STARTGAME_NORMAL || nGameMode == GAMEMODE_NONE) return LTFALSE;

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsHosting()
//
//	PURPOSE:	See if we are playing a multiplayer game
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::IsHosting()
{
	int nGameMode = 0;
    g_pLTClient->GetGameMode(&nGameMode);
    if (nGameMode == STARTGAME_HOST || nGameMode == STARTGAME_HOSTTCP) return LTTRUE;

    return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::IsPlayerInWorld()
//
//	PURPOSE:	See if the player is in the world
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::IsPlayerInWorld()
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();

    if (!m_bPlayerPosSet || !m_bInWorld || m_ePlayerState == PS_UNKNOWN || !hPlayerObj) return LTFALSE;

    return LTTRUE;
}


void CGameClientShell::GetCameraRotation(LTRotation *pRot)
{
    g_pLTClient->SetupEuler(pRot, m_fPitch, m_fYaw, m_fRoll);
}

void CGameClientShell::GetPlayerRotation(LTRotation *pRot)
{
    g_pLTClient->SetupEuler(pRot, m_fPlayerPitch, m_fPlayerYaw, m_fPlayerRoll);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StartGame
//
//	PURPOSE:	Start a new game
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::StartGame(GameDifficulty eDifficulty)
{
	SetDifficulty(eDifficulty);
	SetFadeBodies(m_bFadeBodies);
	DoStartGame();
    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DoStartGame
//
//	PURPOSE:	*Really* Start a new game
//
// --------------------------------------------------------------------------- //

void CGameClientShell::DoStartGame()
{
	m_nCurrentMission = 0;
	m_nCurrentLevel	  = 0;

	if (!LoadCurrentLevel())
	{
        g_pLTClient->CPrint("ERROR in CGameClientShell::DoStartGame():");
        g_pLTClient->CPrint("      Couldn't load the first mission!");
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::StartMission
//
//	PURPOSE:	Start a new mission
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::StartMission(int nMissionId)
{

	// if we're playing multiplayer, I know I'm about to disconnect so don't warn me
	if (IsInWorld() && GetGameType() != SINGLE)
		g_pInterfaceMgr->StartingNewGame();

	MISSION* pMission = g_pMissionMgr->GetMission(nMissionId);
    if (!pMission) return LTFALSE;

	m_nCurrentMission = nMissionId;
	m_nCurrentLevel	  = 0;

	CPlayerStats* pStats = m_InterfaceMgr.GetPlayerStats();

	pStats->ClearMissionDamage();

	ObjectivesList* pObjList = pStats->GetObjectives();
	pObjList->Clear();

	pObjList = pStats->GetCompletedObjectives();
	pObjList->Clear();

	m_eGameType = SINGLE;
	m_InterfaceMgr.ChangeState(GS_LOADINGLEVEL);

	if (!LoadCurrentLevel())
	{
        g_pLTClient->CPrint("ERROR in CGameClientShell::StartMission():");
        g_pLTClient->CPrint("      Couldn't start mission %d!", m_nCurrentMission);
        m_InterfaceMgr.SwitchToFolder(FOLDER_ID_MAIN);

        return LTFALSE;
	}

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::LoadCurrentLevel
//
//	PURPOSE:	Handle loading the current level
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::LoadCurrentLevel()
{
	MISSION* pMission = g_pMissionMgr->GetMission(m_nCurrentMission);
    if (!pMission) return LTFALSE;

    if (m_nCurrentLevel < 0 || m_nCurrentLevel > pMission->nNumLevels) return LTFALSE;

    uint8 nFlags = m_nCurrentLevel == 0 ? LOAD_NEW_GAME : LOAD_NEW_LEVEL;

    return LoadWorld(pMission->aLevels[m_nCurrentLevel].szLevel, LTNULL, LTNULL, nFlags);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::GetNiceWorldName
//
//	PURPOSE:	Get the nice (level designer set) world name...
//
// --------------------------------------------------------------------------- //

LTBOOL CGameClientShell::GetNiceWorldName(char* pWorldFile, char* pRetName, int nRetLen)
{
    if (!pWorldFile || !pRetName || nRetLen < 2) return LTFALSE;

	char buf[_MAX_PATH];
	buf[0] = '\0';
	uint32 len;

	char buf2[_MAX_PATH];
	sprintf(buf2, "%s.dat", pWorldFile);

    LTRESULT dRes = g_pLTClient->GetWorldInfoString(buf2, buf, _MAX_PATH, &len);

	if (dRes != LT_OK || !buf[0] || len < 1)
	{
		// try pre-pending "worlds\" to the filename to see if it will find it then...
		// sprintf (buf2, "worlds\\%s.dat", pWorldFile);
        // dRes = g_pLTClient->GetWorldInfoString(buf2, buf, _MAX_PATH, &len);

		if (dRes != LT_OK || !buf[0] || len < 1)
		{
            return LTFALSE;
		}
	}


	char tokenSpace[5*(PARSE_MAXTOKENSIZE + 1)];
	char *pTokens[5];
	int nArgs;

	char* pCurPos = buf;
	char* pNextPos;

    LTBOOL bMore = LTTRUE;
	while (bMore)
	{
        bMore = g_pLTClient->Parse(pCurPos, &pNextPos, tokenSpace, pTokens, &nArgs);
		if (nArgs < 2) break;

		if (_stricmp(pTokens[0], "WORLDNAME") == 0)
		{
			strncpy(pRetName, pTokens[1], nRetLen);
            return LTTRUE;
		}

		pCurPos = pNextPos;
	}

    return LTFALSE;
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
	if (m_weaponModel.GetHandle() == hObj)
	{
		m_weaponModel.OnModelKey(hObj, pArgs);
	}
	else
	{
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
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DoActivate
//
//	PURPOSE:	Tell the server to do Activate
//
// --------------------------------------------------------------------------- //

void CGameClientShell::DoActivate(LTBOOL bEditMode)
{
	// Don't allow activation when the zip cord is on...
	if (m_MoveMgr.IsZipCordOn()) return;

	char* pActivateOverride = g_vtActivateOverride.GetStr();
	if (pActivateOverride && pActivateOverride[0] != ' ')
	{
        g_pLTClient->RunConsoleString(pActivateOverride);
		return;
	}

    LTRotation rRot;
    LTVector vU, vR, vF, vPos;

	if (m_PlayerCamera.IsFirstPerson())
	{
		g_pLTClient->GetObjectPos(m_hCamera, &vPos);
		g_pLTClient->GetObjectRotation(m_hCamera, &rRot);
		g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
	}
	else  // Use player pos/rot
	{
		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (!hPlayerObj) return;

		g_pLTClient->GetObjectPos(hPlayerObj, &vPos);
		g_pLTClient->GetObjectRotation(hPlayerObj, &rRot);
		g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
	}

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_ACTIVATE);
    g_pLTClient->WriteToMessageVector(hMessage, &vPos);
    g_pLTClient->WriteToMessageVector(hMessage, &vF);
    g_pLTClient->WriteToMessageByte(hMessage, bEditMode);
    g_pLTClient->EndMessage(hMessage);
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

	HLOCALOBJ hMoveMgrObj = m_MoveMgr.GetObject();
	if (!hMoveMgrObj) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

    LTVector vPos;
    g_pLTClient->GetObjectPos(hMoveMgrObj, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	SAFE_STRCPY(theStruct.m_Filename, "Models\\1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "SpecialFX\\smoke.dtx");
	theStruct.m_ObjectType = OT_MODEL;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_MODELWIREFRAME;

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

	HLOCALOBJ hMoveMgrObj = m_MoveMgr.GetObject();
	if (!hMoveMgrObj) return;

    LTVector vPos;
    g_pLTClient->GetObjectPos(hMoveMgrObj, &vPos);
    g_pLTClient->SetObjectPos(m_hBoundingBox, &vPos);

    LTVector vDims;
    g_pLTClient->Physics()->GetObjectDims(hMoveMgrObj, &vDims);

    LTVector vScale;
	VEC_DIVSCALAR(vScale, vDims, 0.5f);
    g_pLTClient->SetObjectScale(m_hBoundingBox, &vScale);
}

// --------------------------------------------------------------------------- //
// Called by the engine, saves all variables (console and member variables)
// related to demo playback.
// --------------------------------------------------------------------------- //
void LoadConVar(ILTClient *g_pLTClient, ILTStream *pStream, char *pVarName)
{
	float val;
	char cString[512];

	(*pStream) >> val;
	sprintf(cString, "%s %f", pVarName, val);
    g_pLTClient->RunConsoleString(cString);
}

void SaveConVar(ILTClient *g_pLTClient, ILTStream *pStream, char *pVarName, float defaultVal)
{
	HCONSOLEVAR hVar;
	float val;

	val = defaultVal;
    if(hVar = g_pLTClient->GetConsoleVar (pVarName))
	{
        val = g_pLTClient->GetVarValueFloat (hVar);
	}

	(*pStream) << val;
}

void CGameClientShell::DemoSerialize(ILTStream *pStream, LTBOOL bLoad)
{
	CGameSettings* pSettings = m_InterfaceMgr.GetSettings();
	if (!pSettings) return;

	if (bLoad)
	{
		g_vtNormalTurnRate.Load(pStream);
		g_vtFastTurnRate.Load(pStream);
		g_vtLookUpRate.Load(pStream);

		m_HeadBobMgr.DemoLoad(pStream);

		pSettings->LoadDemoSettings(pStream);
	}
	else
	{
		g_vtNormalTurnRate.Save(pStream);
		g_vtFastTurnRate.Save(pStream);
		g_vtLookUpRate.Save(pStream);

		m_HeadBobMgr.DemoSave(pStream);

		pSettings->SaveDemoSettings(pStream);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	LoadLeakFile
//
//	PURPOSE:	Loads a leak file and creates a line system for it.
//
// --------------------------------------------------------------------------- //

LTBOOL LoadLeakFile(ILTClient *g_pLTClient, char *pFilename)
{
	FILE *fp;
	char line[256];
	HLOCALOBJ hObj;
	ObjectCreateStruct cStruct;
    LTLine theLine;
	int nRead;

	fp = fopen(pFilename, "rt");
	if(fp)
	{
		INIT_OBJECTCREATESTRUCT(cStruct);
		cStruct.m_ObjectType = OT_LINESYSTEM;
		cStruct.m_Flags = FLAG_VISIBLE;
        hObj = g_pLTClient->CreateObject(&cStruct);
		if(!hObj)
		{
			fclose(fp);
            return LTFALSE;
		}

		while(fgets(line, 256, fp))
		{
			nRead = sscanf(line, "%f %f %f %f %f %f",
				&theLine.m_Points[0].m_Pos.x, &theLine.m_Points[0].m_Pos.y, &theLine.m_Points[0].m_Pos.z,
				&theLine.m_Points[1].m_Pos.x, &theLine.m_Points[1].m_Pos.y, &theLine.m_Points[1].m_Pos.z);

			// White
			theLine.m_Points[0].r = theLine.m_Points[0].g = theLine.m_Points[0].b = 1;
			theLine.m_Points[0].a = 1;

			// Read
			theLine.m_Points[1].r = 1;
			theLine.m_Points[1].g = theLine.m_Points[1].b = 0;
			theLine.m_Points[1].a = 1;

            g_pLTClient->AddLine(hObj, &theLine);
		}

		fclose(fp);
        return LTTRUE;
	}
	else
	{
        return LTFALSE;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ConnectToTcpIpAddress
//
//	PURPOSE:	Connects (joins) to the given tcp/ip address
//
// --------------------------------------------------------------------------- //

LTBOOL ConnectToTcpIpAddress(ILTClient* pClientDE, char* sAddress)
{
	// Sanity checks...

    if (!g_pLTClient) return(LTFALSE);
    if (!sAddress) return(LTFALSE);

/*
	// Try to connect to the given address...

    LTBOOL db = NetStart_DoConsoleConnect(g_pLTClient, sAddress);

	if (!db)
	{
        if (strlen(sAddress) <= 0) g_pLTClient->CPrint("Unable to connect");
        else g_pLTClient->CPrint("Unable to connect to %s", sAddress);
        return(LTFALSE);
	}


	// All done...

    if (strlen(sAddress) > 0) g_pLTClient->CPrint("Connected to %s", sAddress);
    else g_pLTClient->CPrint("Connected");

    return(LTTRUE);
	*/
    return LTFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	NVModelHook
//
//	PURPOSE:	Special Rendering Code for NightVision Powerup
//
// --------------------------------------------------------------------------- //

void NVModelHook (ModelHookData *pData, void *pUser)
{
	CGameClientShell* pShell = (CGameClientShell*) pUser;
	if (!pShell) return;

    uint32 nUserFlags = 0;
    g_pLTClient->GetObjectUserFlags (pData->m_hObject, &nUserFlags);
	if (nUserFlags & USRFLG_NIGHT_INFRARED)
	{
		pData->m_Flags &= ~MHF_USETEXTURE;
		if (pData->m_LightAdd)
		{
			VEC_COPY(*pData->m_LightAdd , g_vNVModelColor);
		}
	}
	else
	{

		DefaultModelHook(pData, pUser);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	IRModelHook
//
//	PURPOSE:	Special Rendering Code for Infrared Powerup
//
// --------------------------------------------------------------------------- //

void IRModelHook (ModelHookData *pData, void *pUser)
{
	CGameClientShell* pShell = (CGameClientShell*) pUser;
	if (!pShell) return;

    uint32 nUserFlags = 0;
    g_pLTClient->GetObjectUserFlags (pData->m_hObject, &nUserFlags);
	if (nUserFlags & USRFLG_NIGHT_INFRARED)
	{
		pData->m_Flags &= ~MHF_USETEXTURE;
		if (pData->m_LightAdd)
		{
			// *(pData->m_LightAdd) = g_vIRModelColor;
			*(pData->m_ObjectColor) = g_vIRModelColor;
			pData->m_ObjectFlags |= FLAG_NOLIGHT;
		}
	}
	else
	{
		DefaultModelHook(pData, pUser);
	}
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
    g_pLTClient->GetObjectUserFlags (pData->m_hObject, &nUserFlags);

	if (nUserFlags & USRFLG_GLOW)
	{
		if (pData->m_LightAdd)
		{
			*pData->m_LightAdd = pShell->GetModelGlow();
			VEC_CLAMP((*pData->m_LightAdd), 0.0f, 255.0f);
		}
	}
	else if (nUserFlags & USRFLG_MODELADD)
	{
		// Get the new color out of the upper 3 bytes of the
		// user flags...

        LTFLOAT r = (LTFLOAT)(nUserFlags>>24);
        LTFLOAT g = (LTFLOAT)(nUserFlags>>16);
        LTFLOAT b = (LTFLOAT)(nUserFlags>>8);

		if (pData->m_LightAdd)
		{
			VEC_SET (*pData->m_LightAdd, r, g, b);
			VEC_CLAMP((*pData->m_LightAdd), 0.0f, 255.0f);
		}
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
	g_pInterfaceMgr->OnChar(c);
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
//	ROUTINE:	HookWindow
//
//	PURPOSE:	HOOK IT!
//
// --------------------------------------------------------------------------- //

BOOL HookWindow()
{
	// Get the screen dimz
    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	if(!hScreen)
		return FALSE;

	// Hook the window
    if(g_pLTClient->GetEngineHook("HWND",(void **)&g_hMainWnd) != LT_OK)
	{
		TRACE("HookWindow - ERROR - could not get the engine window!\n");
		return FALSE;
	}

	// Get the window procedure
#ifdef STRICT
	g_pfnMainWndProc = (WNDPROC)GetWindowLong(g_hMainWnd,GWL_WNDPROC);
#else
	g_pfnMainWndProc = (FARPROC)GetWindowLong(g_hMainWnd,GWL_WNDPROC);
#endif

	if(!g_pfnMainWndProc)
	{
		TRACE("HookWindow - ERROR - could not get the window procedure from the engine window!\n");
		return FALSE;
	}

	// Replace it with ours
	if(!SetWindowLong(g_hMainWnd,GWL_WNDPROC,(LONG)HookedWindowProc))
	{
		TRACE("HookWindow - ERROR - could not set the window procedure!\n");
		return FALSE;
	}

	// Clip the cursor if we're NOT in a window
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("Windowed");
	BOOL bClip = TRUE;
	if(hVar)
	{
        float fVal = g_pLTClient->GetVarValueFloat(hVar);
		if(fVal == 1.0f)
			bClip = FALSE;
	}

	if(bClip)
	{
        uint32 dwScreenWidth = g_pGameClientShell->GetScreenWidth();
        uint32 dwScreenHeight = g_pGameClientShell->GetScreenHeight();

		SetWindowLong(g_hMainWnd,GWL_STYLE,WS_VISIBLE);
		SetWindowPos(g_hMainWnd,HWND_TOPMOST,0,0,dwScreenWidth,dwScreenHeight,SWP_FRAMECHANGED);
		RECT wndRect;
		GetWindowRect(g_hMainWnd, &wndRect);
		ClipCursor(&wndRect);
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
		SetWindowLong(g_hMainWnd, GWL_WNDPROC, (LONG)g_pfnMainWndProc);
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
	WriteConsoleInt("Difficulty",(int)e);

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_DIFFICULTY);
    g_pLTClient->WriteToMessageByte(hMessage, m_eDifficulty);
    g_pLTClient->EndMessage(hMessage);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetFadeBodies
//
//	PURPOSE:	Dynamically change our difficulty level
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetFadeBodies(LTBOOL bFade)
{
	m_bFadeBodies = bFade;

    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_FADEBODIES);
    g_pLTClient->WriteToMessageByte(hMessage, (uint8)m_bFadeBodies);
    g_pLTClient->EndMessage(hMessage);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleWeaponPickup()
//
//	PURPOSE:	Handle picking up weapon
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleWeaponPickup(uint8 nWeaponID)
{
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
	if (!pWeapon) return;

	// Don't display information about picking up melee weapons...

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->nDefaultAmmoType);
	if (!pAmmo) return;

	if (pAmmo->eInstDamageType == DT_MELEE) return;


	int nNameId = pWeapon->nNameId;
	if (!nNameId) return;

    HSTRING hStr = g_pLTClient->FormatString(nNameId);
	if (!hStr) return;

    char* pStr = g_pLTClient->GetStringData(hStr);
	if (!pStr)
	{
        g_pLTClient->FreeString(hStr);
		return;
	}

    HSURFACE hSurf = LTNULL;
	if (strlen(pWeapon->szSmallIcon))
		hSurf = g_pLTClient->CreateSurfaceFromBitmap(pWeapon->szSmallIcon);
	else
		hSurf = g_pLTClient->CreateSurfaceFromBitmap(pWeapon->szIcon);
	if (!hSurf)
        hSurf = g_pLTClient->CreateSurfaceFromBitmap("interface\\missingslot.pcx");

    HSTRING hMsg = g_pLTClient->FormatString(IDS_GUNPICKUP,pStr);
	m_InterfaceMgr.GetMessageMgr()->AddLine(hMsg,MMGR_PICKUP,hSurf);
    g_pLTClient->FreeString(hStr);

    LTVector vTintColor = g_pLayoutMgr->GetWeaponPickupColor();
    LTFLOAT  fTotalTime = g_pLayoutMgr->GetTintTime();

    LTFLOAT fRampDown = fTotalTime * 0.85f;
    LTFLOAT fRampUp   = fTotalTime * 0.10f;
    LTFLOAT fTintTime = fTotalTime * 0.05f;

    LTVector vCamPos;
    g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

    g_pGameClientShell->FlashScreen(vTintColor, vCamPos, 1000.0f, fRampUp, fTintTime, fRampDown, LTTRUE);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleGearPickup()
//
//	PURPOSE:	Handle picking up gear
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleGearPickup(uint8 nGearId)
{
	GEAR* pGear = g_pWeaponMgr->GetGear(nGearId);
	if (!pGear) return;

	int nNameId = pGear->nNameId;
	if (!nNameId) return;

    HSTRING hStr = g_pLTClient->FormatString(nNameId);
	if (!hStr) return;

    char* pStr = g_pLTClient->GetStringData(hStr);
	if (!pStr)
	{
        g_pLTClient->FreeString(hStr);
		return;
	}

    HSURFACE hSurf = LTNULL;
	if (strlen(pGear->szSmallIcon))
		hSurf = g_pLTClient->CreateSurfaceFromBitmap(pGear->szSmallIcon);
	else
		hSurf = g_pLTClient->CreateSurfaceFromBitmap(pGear->szIcon);

	if (!hSurf)
        hSurf = g_pLTClient->CreateSurfaceFromBitmap("interface\\missingslot.pcx");


    HSTRING hMsg = g_pLTClient->FormatString(IDS_GEARPICKUP,pStr);
	m_InterfaceMgr.GetMessageMgr()->AddLine(hMsg,MMGR_PICKUP, hSurf);
    g_pLTClient->FreeString(hStr);

    LTVector vTintColor = pGear->vScreenTintColor;
    LTFLOAT  fTotalTime = pGear->fScreenTintTime;

    LTFLOAT fRampDown = fTotalTime * 0.85f;
    LTFLOAT fRampUp   = fTotalTime * 0.10f;
    LTFLOAT fTintTime = fTotalTime * 0.05f;


    LTVector vCamPos;
    g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

    g_pGameClientShell->FlashScreen(vTintColor, vCamPos, 1000.0f, fRampUp, fTintTime, fRampDown, LTTRUE);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleModPickup()
//
//	PURPOSE:	Handle picking up mod
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleModPickup(uint8 nModId)
{
	MOD* pMod = g_pWeaponMgr->GetMod(nModId);
	if (!pMod) return;

	int nNameId = pMod->nNameId;
	if (!nNameId) return;

    HSTRING hStr = g_pLTClient->FormatString(nNameId);
	if (!hStr) return;

    char* pStr = g_pLTClient->GetStringData(hStr);
	if (!pStr)
	{
        g_pLTClient->FreeString(hStr);
		return;
	}

    HSURFACE hSurf = LTNULL;
	if (strlen(pMod->szSmallIcon))
		hSurf = g_pLTClient->CreateSurfaceFromBitmap(pMod->szSmallIcon);
    else
		hSurf = g_pLTClient->CreateSurfaceFromBitmap(pMod->szIcon);
	if (!hSurf)
        hSurf = g_pLTClient->CreateSurfaceFromBitmap("interface\\missingslot.pcx");

    HSTRING hMsg = g_pLTClient->FormatString(IDS_MODPICKUP,pStr);
	m_InterfaceMgr.GetMessageMgr()->AddLine(hMsg, MMGR_PICKUP, hSurf);
    g_pLTClient->FreeString(hStr);

    LTVector vTintColor = pMod->vScreenTintColor;
    LTFLOAT  fTotalTime = pMod->fScreenTintTime;

    LTFLOAT fRampDown = fTotalTime * 0.85f;
    LTFLOAT fRampUp   = fTotalTime * 0.10f;
    LTFLOAT fTintTime = fTotalTime * 0.05f;


    LTVector vCamPos;
    g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

    g_pGameClientShell->FlashScreen(vTintColor, vCamPos, 1000.0f, fRampUp, fTintTime, fRampDown, LTTRUE);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleAmmoPickup()
//
//	PURPOSE:	Handle picking up ammo
//
// ----------------------------------------------------------------------- //

void CGameClientShell::HandleAmmoPickup(uint8 nAmmoId, int nAmmoCount)
{
	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pAmmo) return;

	int nNameId = pAmmo->nNameId;
	if (!nNameId) return;

    HSTRING hStr = g_pLTClient->FormatString(nNameId);
	if (!hStr) return;

    char* pStr = g_pLTClient->GetStringData(hStr);
	if (!pStr)
	{
        g_pLTClient->FreeString(hStr);
		return;
	}

    HSURFACE hSurf = g_pLTClient->CreateSurfaceFromBitmap(pAmmo->szSmallIcon);
	if (!hSurf)
        hSurf = g_pLTClient->CreateSurfaceFromBitmap("interface\\missingslot.pcx");

    HSTRING hMsg = g_pLTClient->FormatString(IDS_AMMOPICKUP, nAmmoCount, pStr);
	m_InterfaceMgr.GetMessageMgr()->AddLine(hMsg,MMGR_PICKUP, hSurf);
    g_pLTClient->FreeString(hStr);

    LTVector vTintColor = g_pLayoutMgr->GetAmmoPickupColor();
    LTFLOAT  fTotalTime = g_pLayoutMgr->GetTintTime();

    LTFLOAT fRampDown = fTotalTime * 0.85f;
    LTFLOAT fRampUp   = fTotalTime * 0.10f;
    LTFLOAT fTintTime = fTotalTime * 0.05f;


    LTVector vCamPos;
    g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

    g_pGameClientShell->FlashScreen(vTintColor, vCamPos, 1000.0f, fRampUp, fTintTime, fRampDown, LTTRUE);
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::UpdateCameraSway
//
//	PURPOSE:	Update the camera's sway
//
// --------------------------------------------------------------------------- //

void CGameClientShell::UpdateCameraSway()
{
	if (g_pInterfaceMgr->GetSunglassMode() != SUN_NONE) return;

	// Apply...
    LTFLOAT swayAmount = m_fFrameTime / 1000.0f;

    LTFLOAT tm  = g_pLTClient->GetTime()/10.0f;

	// Adjust if ducking...
	LTFLOAT fMult = (m_dwPlayerFlags & BC_CFLG_DUCK) ? g_vtCameraSwayDuckMult.GetFloat() : 1.0f;

    LTFLOAT faddP = fMult * g_vtCameraSwayYSpeed.GetFloat() * (float)sin(tm*g_vtCameraSwayYFreq.GetFloat()) * swayAmount;
    LTFLOAT faddY = fMult * g_vtCameraSwayXSpeed.GetFloat() * (float)sin(tm*g_vtCameraSwayXFreq.GetFloat()) * swayAmount;

	m_fPitch += faddP;
	m_fYaw	 += faddY;
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleWeaponDisable
//
//	PURPOSE:	Handle the weapon being disabled...
//
// --------------------------------------------------------------------------- //

void CGameClientShell::HandleWeaponDisable(LTBOOL bDisabled)
{
	if (bDisabled)
	{
		ClearScreenTint();
        HandleZoomChange(m_weaponModel.GetWeaponId(), LTTRUE);
		EndZoom();
	}
	else
	{
		// Force us to re-evaluate what container we're in.  We call
		// UpdateContainerFX() first to make sure any container changes
		// have been accounted for, then we clear the container code
		// and force an update (this is done for underwater situations like
		// dying underwater and respawning, and also for picking up intelligence
		// items underwater)...

		if (IsPlayerInWorld())
		{
			UpdateContainerFX();
			ClearCurContainerCode();
			UpdateContainerFX();
		}

        m_weaponModel.SetVisible(LTTRUE);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::HandleMissionFailed
//
//	PURPOSE:	Handle mission failure
//
// --------------------------------------------------------------------------- //

void CGameClientShell::HandleMissionFailed()
{
	ClearScreenTint();
    HandleZoomChange(m_weaponModel.GetWeaponId(), LTTRUE);
	EndZoom();
	m_InterfaceMgr.EndUnderwater();

	m_InterfaceMgr.ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());

	m_InterfaceMgr.MissionFailed(IDS_YOUWEREKILLED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::AttachCameraToHead
//
//	PURPOSE:	Attach the camera to a socket in the player's head
//
// --------------------------------------------------------------------------- //

void CGameClientShell::AttachCameraToHead(LTBOOL bAttach)
{
	m_bCameraAttachedToHead = bAttach;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::GetPlayerHeadPos
//
//	PURPOSE:	Get the player's head position
//
// --------------------------------------------------------------------------- //

void CGameClientShell::GetPlayerHeadPosRot(LTVector & vPos, LTRotation & rRot)
{
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	HOBJECT hBody = LTNULL; // g_pLTClient->GetClientObject();

	// We actually want to use the body prop, so...

	CSpecialFXList* pList = m_sfxMgr.GetFXList(SFX_BODY_ID);
	if (!pList) return;

	int nNumBodies = pList->GetSize();

    uint32 dwId;
    g_pLTClient->GetLocalClientID(&dwId);

	for (int i=0; i < nNumBodies; i++)
	{
		if ((*pList)[i])
		{
			CBodyFX* pBody = (CBodyFX*)(*pList)[i];

			if (pBody->GetClientId() == dwId)
			{
				hBody = (*pList)[i]->GetServerObj();
				break;
			}
		}
	}

	if (hBody)
	{
		if (g_pModelLT->GetSocket(hBody, "Eyes", hSocket) == LT_OK)
		{
			LTransform transform;
            if (g_pModelLT->GetSocketTransform(hBody, hSocket, transform, LTTRUE) == LT_OK)
			{
				g_pTransLT->Get(transform, vPos, rRot);
				CalcNonClipPos(vPos, rRot);
			}
		}
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

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ClearCurContainerCode
//
//	PURPOSE:	Clear our current container info.
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ClearCurContainerCode()
{
	m_eCurContainerCode = CC_NO_CONTAINER;
	m_nSoundFilterId = 0;
}

// returns LTTRUE if the passed in address matches the current server address
LTBOOL CGameClientShell::CheckServerAddress(char *pszTestAddress, int nPort)
{
	if (!pszTestAddress) return LTFALSE;
	if (nPort != m_nServerPort) return LTFALSE;
	return (stricmp(pszTestAddress,m_szServerAddress) == 0);
}


void QuickLoadCallBack(LTBOOL bReturn, void *pData)
{
	if (bReturn)
	{
		g_pGameClientShell->QuickLoad();
	}
}


void CGameClientShell::DoTaunt(uint32 nClientID,uint8 nTaunt)
{
	if (m_ePlayerState != PS_ALIVE) return;

	//if you're not listening to taunts, you're not allowed to send them
	if (GetConsoleInt("IgnoreTaunts",0) > 0) return;

	CClientInfoMgr *pCIMgr = m_InterfaceMgr.GetClientInfoMgr();
	if (!pCIMgr) return;

	CLIENT_INFO *pInfo = pCIMgr->GetLocalClient();
	if (!pInfo) return;

	char szVar[16] = "";

	switch (pInfo->team)
	{
	case 0:
		sprintf(szVar,"TauntDM%d",nTaunt);
		break;
	case 1:
		sprintf(szVar,"TauntUnity%d",nTaunt);
		break;
	case 2:
		sprintf(szVar,"TauntHARM%d",nTaunt);
		break;
	}

	uint32 nTauntID = (uint32)GetConsoleInt(szVar,0);

	if (!nTauntID) return;

	// Don't allow the client to flood the server with taunts...

	CCharacterFX *pFX = m_MoveMgr.GetCharacterFX();
	if (pFX && !pFX->IsPlayingTaunt())
	{
		pFX->PlayTaunt(nTauntID);

		HMESSAGEWRITE hWrite = g_pLTClient->StartMessage(MID_PLAYER_TAUNT);
		g_pLTClient->WriteToMessageDWord(hWrite, nTauntID);
		g_pLTClient->EndMessage(hWrite);
	}
}

LTBOOL CGameClientShell::DoJoinGame(char* sIp)
{
	// Sanity checks...

	if (!sIp) return(LTFALSE);
	if (!g_pLTClient) return(LTFALSE);


	// Start the game...

	StartGameRequest req;
	NetClientData clientData;

	memset( &req, 0, sizeof( req ));


	// Setup our client...

	clientData.m_dwTeam = (uint32)GetConsoleInt("NetPlayerTeam",0);
	SAFE_STRCPY(clientData.m_sName,g_vtPlayerName.GetStr());

	req.m_pClientData = &clientData;
	req.m_ClientDataLen = sizeof( clientData );
	req.m_Type = STARTGAME_CLIENTTCP;
	strncpy(req.m_TCPAddress, sIp, MAX_SGR_STRINGLEN);


	// Try to join a game...

    LTRESULT dr = g_pLTClient->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
		g_pLTClient->CPrint("InitNetworking() : error %d", dr);
        return(LTFALSE);
	}


	// [blg] we don't know it yet. g_pGameClientShell->SetGameType((GameType)nType);
	g_pInterfaceMgr->DrawFragCount(LTFALSE);
	g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);

	int nRetries = GetConsoleInt("NetJoinRetry", 0);
	while (nRetries >= 0)
	{
		// If successful, then we're done.
        if( g_pLTClient->StartGame( &req ) == LT_OK )
		{
            return(LTTRUE);
		}

		// Wait a sec and try again.
		Sleep(1000);
		nRetries--;
	}

	// All done...
    return(LTFALSE);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SetDisconnectCode
//
//	PURPOSE:	Sets the disconnection code and message
//
// --------------------------------------------------------------------------- //

void CGameClientShell::SetDisconnectCode(uint32 nCode, const char *pMsg, uint32 nSubCode)
{
	// Don't override what someone already told us
	if (m_nDisconnectCode)
		return;

	m_nDisconnectCode = nCode;
	m_nDisconnectSubCode = nSubCode;
	if (m_pDisconnectMsg)
		debug_deletea(m_pDisconnectMsg);
	m_pDisconnectMsg = debug_newa(char, strlen(pMsg) + 1);
	strcpy(m_pDisconnectMsg, pMsg);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::ClearDisconnectCode
//
//	PURPOSE:	Clears the disconnection code and message
//
// --------------------------------------------------------------------------- //

void CGameClientShell::ClearDisconnectCode()
{
	m_nDisconnectCode = 0;
	m_nDisconnectSubCode = 0;
	if (m_pDisconnectMsg)
		debug_deletea(m_pDisconnectMsg);
	m_pDisconnectMsg = LTNULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::GetDisconnectCode
//
//	PURPOSE:	Retrieves the disconnection code
//
// --------------------------------------------------------------------------- //

uint32 CGameClientShell::GetDisconnectCode()
{
	return m_nDisconnectCode;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::GetDisconnectSubCode
//
//	PURPOSE:	Retrieves the disconnection sub-code
//
// --------------------------------------------------------------------------- //

uint32 CGameClientShell::GetDisconnectSubCode()
{
	return m_nDisconnectSubCode;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::GetDisconnectMsg
//
//	PURPOSE:	Retrieves the disconnection message
//
// --------------------------------------------------------------------------- //

const char *CGameClientShell::GetDisconnectMsg()
{
	if (m_pDisconnectMsg)
		return m_pDisconnectMsg;
	else
		return LTNULL;
}


