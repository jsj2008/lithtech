// ----------------------------------------------------------------------- //
//
// MODULE  : RiotClientShell.cpp
//
// PURPOSE : Riot's Client Shell - Implementation
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //


#include "RiotClientShell.h"
#include "RiotMsgIds.h"
#include "RiotCommandIds.h"
#include "WeaponModel.h"
#include "WeaponDefs.h"
#include "ClientUtilities.h"
#include "vkdefs.h"
#include "ClientRes.h"
#include "RiotSoundTypes.h"
#include "Music.h"
#include "TextHelper.h"
#include "PopupMenu.h"
#include "VolumeBrushFX.h"
#include "client_physics.h"
#include "CameraFX.h"
#include "WinUtil.h"
#include "Font28.h"
#include "Font08.h"
#include "NetStart.h"
#include "WeaponStringDefs.h"
#include "CMoveMgr.h"
#include "iltphysics.h"
#include "VarTrack.h"
#include "ClientWeaponUtils.h"
#include "SFXReg.h"
#include <mbstring.h>
#include "ltobjectcreate.h"
#include "iltsoundmgr.h"

#include <stdarg.h>
#include <stdio.h>

#define min(a,b)	(a < b ? a : b)
#define max(a,b)	(a > b ? a : b)


#define BOBV_WALKING	24.0f
#define BOBH_WALKING	16.0f
#define SWAYV_WALKING	32.0f
#define SWAYH_WALKING	80.0f

#define BOBV_CRAWLING	24.0f
#define BOBH_CRAWLING	16.0f
#define SWAYV_CRAWLING	16.0f
#define SWAYH_CRAWLING	40.0f

#define FOV_NORMAL		90
#define FOV_ZOOMED		10

#define DEG2RAD(x)		(((x)*MATH_PI)/180.0f)
#define RAD2DEG(x)		(((x)*180.0f)/MATH_PI)

#define LETTERBOX_ADJUST	20

#define TRANSMISSION_IMAGE_ANIM_RATE	1000
#define TRANSMISSION_TEXT_ANIM_RATE		400

#define	FIRE_JITTER_DECAY_DELTA			0.1f
#define FIRE_JITTER_MAX_PITCH_DELTA		1.0f

#define DEFAULT_LOD_SCALE				1.1f
#define LODSCALE_MULTIPLIER				5.0f

#define MODELGLOW_HALFCYCLE				1.0f

#define MAX_PLAYER_INFO_SEND_PER_SECOND	20.0f

#define RESPAWN_WAIT_TIME					1.0f
#define MULTIPLAYER_RESPAWN_WAIT_TIME		0.2f

#define PICKUPITEM_TINT_UNKNOWN			LTVector(0.0f, 0.0f, 0.0f)
#define PICKUPITEM_TINT_ARMOR			LTVector(0.25f, 0.25f, 0.25f)
#define PICKUPITEM_TINT_HEALTH			LTVector(0.25f, 0.25f, 0.25f)
#define PICKUPITEM_TINT_WEAPON			LTVector(0.25f, 0.25f, 0.4f)

#define DEFAULT_CSENDRATE	7.0f

#define SOUND_REVERB_UPDATE_PERIOD		0.33f


#define DEFAULT_NORMAL_TURN_SPEED 1.5f
#define DEFAULT_FAST_TURN_SPEED 2.3f
#define NORMAL_TURN_RATE_VAR "NormalTurnRate"
#define FAST_TURN_RATE_VAR "FastTurnRate"


CRiotClientShell* g_pRiotClientShell = LTNULL;
LTVector			  g_vWorldWindVel;
PhysicsState	  g_normalPhysicsState;
PhysicsState	  g_waterPhysicsState;

VarTrack g_CV_CSendRate; // The SendRate console variable.

LTFLOAT s_fDemoTime   = 0.0f;

void IRModelHook (struct ModelHookData_t *pData, void *pUser);
void NVModelHook (struct ModelHookData_t *pData, void *pUser);
void DefaultModelHook (struct ModelHookData_t *pData, void *pUser);

// Guids...

#ifdef _DEMO

LTGUID SHOGOGUID = { 
	0x67aade60,	0xab3, 0x22d4, 0xcd, 0x69, 0x0, 0x40, 0x5, 0x92, 0x34, 0x31
};

#else

LTGUID SHOGOGUID = { /* 87EEDE80-0ED4-11d2-BA96-006008904776 */
	0x87eede80,	0xed4, 0x11d2, 0xba, 0x96, 0x0, 0x60, 0x8, 0x90, 0x47, 0x76
};

#endif


// Setup..

SETUP_CLIENTSHELL();

ILTClient* CreateClientShell(ILTClient *pClientDE)
{
	g_pLTClient = pClientDE;
	return (ILTClient*)(new CRiotClientShell);
}

void DeleteClientShell(ILTClient *pInputShell)
{
	if (pInputShell)
	{
		delete ((CRiotClientShell*)pInputShell);
	}
}

static LTBOOL LoadLeakFile(ILTClient *pClientDE, char *pFilename);

void LeakFileFn(int argc, char **argv)
{
	if (argc < 0)
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

//#ifdef _DEMO
//	g_pRiotClientShell->GetILTClient()->CPrint("Multiplayer is not available in the demo");
//	return;
//#endif

	if (argc <= 0)
	{
		g_pLTClient->CPrint("Connect <tcpip address> (use '*' for local net)");
		return;
	}

	ConnectToTcpIpAddress(g_pLTClient, argv[0]);
}

void FragSelfFn(int argc, char **argv)
{
	ILTClient *pClientDE;
	ILTMessage_Write* hWrite;

	hWrite = g_pLTClient->StartMessage(MID_FRAG_SELF);
	pClientDE->EndMessage(hWrite);
}


void RecordFn(int argc, char **argv)
{
	if(g_pRiotClientShell)
		g_pRiotClientShell->HandleRecord(argc, argv);
}

void PlayDemoFn(int argc, char **argv)
{
	if(g_pRiotClientShell)
		g_pRiotClientShell->HandlePlaydemo(argc, argv);
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
	if( g_pRiotClientShell )
		g_pRiotClientShell->InitSound( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::CRiotClientShell()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //
			
CRiotClientShell::CRiotClientShell()
{
	g_pRiotClientShell = this;

	m_MoveMgr = LTNULL;

	VEC_INIT(g_vWorldWindVel);
	memset(&m_rcMenuRestoreCamera, 0, sizeof(m_rcMenuRestoreCamera));

	m_hCamera		= LTNULL;
	m_nGameState	= GS_PLAYING;

	m_bUseWorldFog = LTTRUE;
	m_bMainWindowMinimized	= LTFALSE;

	m_bStrafing			= LTFALSE;
	m_bHoldingMouseLook	= LTFALSE;
	
	m_bMusicOriginallyEnabled	= LTFALSE;
	m_bSoundOriginallyEnabled	= LTFALSE;
	m_bLightmappingOriginallyEnabled = LTTRUE;
	m_bModelFBOriginallyEnabled		 = LTTRUE;

	m_bGamePaused		= LTFALSE;
	m_bRecordingDemo	= LTFALSE;

	m_bSwitchingModes = LTFALSE;
	m_nClearScreenCount		= 0;
	m_bClearScreenAlways	= LTFALSE;
	m_bOldDrawInterface		= LTTRUE;
	m_bDrawHud				= LTTRUE;
	m_bDrawInterface		= LTTRUE;
	m_bDrawMissionLog		= LTFALSE;
	m_bDrawOrdinance		= LTFALSE;
	m_bDrawFragCount		= LTFALSE;
	m_bStatsSizedOff		= LTFALSE;
	m_bWaitingForMLClosure	= LTFALSE;
	m_bNewObjective			= LTFALSE;
	m_hNewObjective			= LTNULL;
	m_bUpdateStats			= LTFALSE;
	m_bMissionLogKeyStillDown = LTFALSE;
	m_bCrosshairOn			= LTTRUE;
	
	m_hGameMessage			 = NULL;
	m_nGameMessageRemoveTime = 0.0f;
	m_pMenuPolygrid			 = LTNULL;
	m_fMenuSaveFOVx			 = 0.0f;
	m_fMenuSaveFOVy			 = 0.0f;

	m_pIngameDialog		= LTNULL;
	m_pMessageBox		= LTNULL;
	m_pYesNoProc		= LTNULL;
	m_nYesNoUserData	= LTNULL;

	m_hBumperText		= LTNULL;
	m_pPressAnyKeySound = "Sounds\\Interface\\pak.wav";

	m_fYawBackup		= 0.0f;
	m_fPitchBackup		= 0.0f;
	m_bRestoreOrientation		= LTFALSE;
	m_bAllowPlayerMovement		= LTTRUE;
	m_bLastAllowPlayerMovement	= LTTRUE;
	m_bWasUsingExternalCamera	= LTFALSE;
	m_bUsingExternalCamera		= LTFALSE;
	m_bMovieCameraRect			= LTFALSE;

	m_bHaveNightVision	= LTFALSE;
	m_bHaveInfrared		= LTFALSE;
	m_bHaveSilencer		= LTFALSE;
	m_bHaveStealth		= LTFALSE;
	m_fNormalWeaponAlpha = 0.0f;

	m_hGamePausedSurface	= LTNULL;
	VEC_SET(m_vDefaultLightScale, 1.0f, 1.0f, 1.0f);

	m_hMenuMusic		= LTNULL;
	
	m_nPlayerInfoChangeFlags	= 0;
	m_fPlayerInfoLastSendTime	= 0.0f;

	m_fTransmissionTimeLeft = 0.0f;
	m_hTransmissionImage = LTNULL;
	m_hTransmissionText = LTNULL;
	m_hTransmissionSound = LTNULL;
	m_bAnimatingTransmissionOn = LTFALSE;
	m_bAnimatingTransmissionOff = LTFALSE;
	m_xTransmissionImage = 0;
	m_yTransmissionImage = 0;
	m_cxTransmissionImage = 0;
	m_cyTransmissionImage = 0;
	m_xTransmissionText = 0;
	m_yTransmissionText = 0;
	m_cxTransmissionText = 0;
	m_cyTransmissionText = 0;
	m_hPressAnyKey = LTNULL;
	m_hLoadingWorld = LTNULL;
	m_hWorldName	= LTNULL;
	m_cxPressAnyKey = 0;
	m_cyPressAnyKey = 0;
	m_hLoading = LTNULL;
	m_cxLoading = 0;
	m_cyLoading = 0;

	VEC_INIT(m_vCameraOffset);
	m_rRotation.Init();

	m_fPitch			= 0.0f;
	m_fYaw				= 0.0f;

	m_fFireJitterPitch	= 0.0f;

	m_dwPlayerFlags		= 0;
	m_nPlayerMode		= PM_MODE_FOOT;
	m_ePlayerState		= PS_UNKNOWN;
	m_bSpectatorMode	= LTFALSE;
	m_bMoving			= LTFALSE;
	m_bMovingSide		= LTFALSE;
	m_bOnGround			= LTTRUE;
	m_bTweakingWeapon	= LTFALSE;
	m_bTweakingWeaponMuzzle	= LTFALSE;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fBobAmp			= 0.0f;
	m_fBobPhase			= 0.0f;
	m_fSwayPhase		= 0.0f;
	m_fVelMagnitude		= 0.0f;

	VEC_INIT(m_vShakeAmount);

	m_bTintScreen		= LTFALSE;
	m_fTintTime			= 0.0f;
	m_fTintStart		= 0.0f;
	m_fTintRampUp		= 0.0f;
	m_fTintRampDown		= 0.0f;
	VEC_INIT(m_vTintColor);

	memset (m_strCurrentWorldName, 0, 256);
	memset (m_strMoviesDir, 0, 256);

	VEC_SET (m_vLightScaleInfrared, 0.8f, 0.1f, 0.1f);
	VEC_SET (m_vLightScaleNightVision, 0.1f, 1.0f, 0.1f);
	VEC_SET (m_vLightScaleObjectives, 0.3f, 0.3f, 0.3f);
	VEC_SET (m_vCurContainerLightScale, -1.0f, -1.0f, -1.0f);
	
	m_fCantIncrement	= 0.009f; // in radians (.5 degrees)
	m_fCantMaxDist		= 0.035f; // in radians (2.0 degrees)
	m_fCamCant			= 0.0f;
	
	m_fCurrentFovX			= DEG2RAD(FOV_NORMAL);
	m_bZoomView				= LTFALSE;
	m_bOldZoomView			= LTFALSE;
	m_bZooming				= LTFALSE;
	m_fSaveLODScale			= DEFAULT_LOD_SCALE;
	m_bHandledStartup		= LTFALSE;
	m_bInWorld				= LTFALSE;
	m_bLoadingWorld			= LTFALSE;
	m_bStartedLevel			= LTFALSE;

	m_bStartedDuckingDown	= LTFALSE;
	m_bStartedDuckingUp		= LTFALSE;
	m_fCamDuck				= 0.0f;
	m_fDuckDownV			= -75.0f;		
	m_fDuckUpV				= 75.0f;
	m_fMaxDuckDistance		= -20.0f;
	m_fStartDuckTime		= 0.0f;

	m_h3rdPersonCrosshair	= LTNULL;
	m_hContainerSound		= LTNULL;
	m_eCurContainerCode		= CC_NONE;
	m_bUnderwater			= LTFALSE;

	m_bShowPlayerPos		= LTFALSE;
	m_hDebugInfo			= LTNULL;
	m_bDebugInfoChanged		= LTFALSE;
	m_hLeakLines			= LTNULL;

	m_bAdjustLightScale		= LTFALSE;
	m_fContainerStartTime	= -1.0f;
	m_fFovXFXDir			= 1.0f;
	m_fLastTime				= 0.0f;

	m_bAdvancedDisableMusic			= LTFALSE;
	m_bAdvancedDisableSound			= LTFALSE;
	m_bAdvancedDisableMovies		= LTFALSE;
	m_bAdvancedEnableOptSurf		= LTFALSE;
	m_bAdvancedDisableLightMap		= LTFALSE;
	m_bAdvancedEnableTripBuf		= LTFALSE;
	m_bAdvancedDisableDx6Cmds		= LTFALSE;
	m_bAdvancedEnableTJuncs			= LTFALSE;
	m_bAdvancedDisableFog			= LTFALSE;
	m_bAdvancedDisableLines			= LTFALSE;
	m_bAdvancedDisableModelFB		= LTFALSE;
	m_bAdvancedEnablePixelDoubling	= LTFALSE;
	m_bAdvancedEnableMultiTexturing = LTFALSE;
	m_bAdvancedDisableJoystick		= LTFALSE;
	
	m_bPanSky				= LTFALSE;
	m_fPanSkyOffsetX		= 1.0f;
	m_fPanSkyOffsetZ		= 1.0f;
	m_fPanSkyScaleX			= 1.0f;
	m_fPanSkyScaleZ			= 1.0f;
	m_fCurSkyXOffset		= 0.0f;
	m_fCurSkyZOffset		= 0.0f;

	VEC_SET(m_vCurModelGlow, 127.0f, 127.0f, 127.0f);
	VEC_SET(m_vMaxModelGlow, 255.0f, 255.0f, 255.0f);
	VEC_SET(m_vMinModelGlow, 50.0f, 50.0f, 50.f);
	m_fModelGlowCycleTime	= 0.0f;
	m_bModelGlowCycleUp		= LTTRUE;

	m_bFirstUpdate			= LTFALSE;
	m_bRestoringGame		= LTFALSE;
	m_bQuickSave			= LTFALSE;
	m_bGameMusicPaused		= LTFALSE;

	m_bCameraPosInited		= LTFALSE;

	m_resSoundInit			= LT_ERROR;
	m_eDifficulty			= GD_NORMAL;

	m_eMusicLevel			= CMusic::MUSICLEVEL_SILENCE;
	m_bAnime				= LTFALSE;
	m_bGameOver				= LTFALSE;

	m_fEarliestRespawnTime	= 0.0f;
	m_hBoundingBox			= LTNULL;

	m_bPlayerPosSet			= LTFALSE;

	m_fNextSoundReverbTime	= 0.0f;
	m_bUseReverb			= LTFALSE;
	m_fReverbLevel			= 0.0f;
	VEC_INIT( m_vLastReverbPos );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::~CRiotClientShell()
//
//	PURPOSE:	Destruction
//
// ----------------------------------------------------------------------- //
			
CRiotClientShell::~CRiotClientShell()
{
	g_pRiotClientShell = LTNULL;

	if (!g_pLTClient) return;

	if(m_MoveMgr)
	{
		delete m_MoveMgr;
		m_MoveMgr = LTNULL;
	}

	if (m_hGameMessage)
	{
		g_pLTClient->DeleteSurface(m_hGameMessage);
		m_hGameMessage = NULL;
	}

	RemoveBumperScreen();
	SetMenuMusic(LTFALSE);

	if (m_hDebugInfo)
	{
		g_pLTClient->DeleteSurface(m_hDebugInfo);
		m_hDebugInfo = NULL;
	}

	if (m_hBoundingBox)
	{
		g_pLTClient->RemoveObject(m_hBoundingBox);
	}

	if (m_hTransmissionImage) 
	{
		g_pLTClient->DeleteSurface (m_hTransmissionImage);
	}

	if (m_hTransmissionText) 
	{
		g_pLTClient->DeleteSurface (m_hTransmissionText);
	}

	if (m_pMenuPolygrid)
	{
		HLOCALOBJ hObj = m_pMenuPolygrid->GetServerObj();
		if (hObj)
		{
			g_pLTClient->RemoveObject(hObj);
		}

		delete m_pMenuPolygrid;
		m_pMenuPolygrid = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::InitSound
//
//	PURPOSE:	Initialize the sounds
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::InitSound()
{
	Sound3DProvider *pSound3DProviderList, *pSound3DProvider;
	InitSoundInfo soundInfo;
	ReverbProperties reverbProperties;
	HCONSOLEVAR hVar;
	uint32 dwProviderID;
	char sz3dSoundProviderName[_MAX_PATH + 1];
	int nError;

	m_resSoundInit = LT_ERROR;

	if (!g_pLTClient) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;

	if (AdvancedDisableSound())
		return;

	soundInfo.Init();

	// Reload the sounds if there are any
	soundInfo.m_dwFlags				= INITSOUNDINFOFLAG_RELOADSOUNDS;

	// Get the 3d sound provider id.
	hVar = g_pLTClient->GetConsoleVar( "3DSoundProvider" );
	if( hVar )
	{
		dwProviderID = ( uint32 )g_pLTClient->GetVarValueFloat( hVar );
	}
	else
		dwProviderID = SOUND3DPROVIDERID_NONE;

	// Can also be set by provider name, in which case the id will be set to UNKNOWN.
	if( dwProviderID == SOUND3DPROVIDERID_NONE || dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
	{
		sz3dSoundProviderName[0] = 0;
		hVar = g_pLTClient->GetConsoleVar( "3DSoundProviderName" );
		if( hVar )
		{
			SAFE_STRCPY( sz3dSoundProviderName, g_pLTClient->GetVarValueString( hVar ));
			dwProviderID = SOUND3DPROVIDERID_UNKNOWN;
		}
	}

	// See if the provider exists.
	if( dwProviderID != SOUND3DPROVIDERID_NONE )
	{
		((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->GetSound3DProviderLists( pSound3DProviderList, LTFALSE );
		if( !pSound3DProviderList )
		{
			m_resSoundInit = LT_NO3DSOUNDPROVIDER;
			return;
		}

		pSound3DProvider = pSound3DProviderList;
		while( pSound3DProvider )
		{
			// If the provider is selected by name, then compare the names.
			if( dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
			{
				if( _mbscmp(( const unsigned char * )sz3dSoundProviderName, ( const unsigned char * )pSound3DProvider->m_szProvider ) == 0 )
					break;
			}
			// Or compare by the id's.
			else if( pSound3DProvider->m_dwProviderID == dwProviderID )
				break;

			// Not this one, try next one.
			pSound3DProvider = pSound3DProvider->m_pNextProvider;
		}

		// Check if we found one.
		if( pSound3DProvider )
		{
			// Use this provider.
			SAFE_STRCPY( soundInfo.m_sz3DProvider, pSound3DProvider->m_szProvider );

			// Get the maximum number of 3d voices to use.
			hVar = g_pLTClient->GetConsoleVar( "Max3DVoices" );
			if( hVar )
			{
				soundInfo.m_nNum3DVoices = ( uint8 )g_pLTClient->GetVarValueFloat( hVar );
			}
			else
				soundInfo.m_nNum3DVoices = 16;
		}

		((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->ReleaseSound3DProviderList( pSound3DProviderList );
	}

	// Get the maximum number of sw voices to use.
	hVar = g_pLTClient->GetConsoleVar( "MaxSWVoices" );
	if( hVar )
	{
		soundInfo.m_nNumSWVoices = ( uint8 )g_pLTClient->GetVarValueFloat( hVar );
	}
	else
		soundInfo.m_nNumSWVoices = 32;

	soundInfo.m_nSampleRate			= 22050;
	soundInfo.m_nBitsPerSample		= 16;
	soundInfo.m_nVolume				= (unsigned short)pSettings->SoundVolume();
	
	if( !pSettings->Sound16Bit( ))
		soundInfo.m_dwFlags |= INITSOUNDINFOFLAG_CONVERT16TO8;

	LTFLOAT fMinStreamSoundTime = 0.5f;
	hVar = g_pLTClient->GetConsoleVar ("MinStreamTime");
	if (hVar)
	{
		fMinStreamSoundTime = g_pLTClient->GetVarValueFloat (hVar);
		fMinStreamSoundTime = (fMinStreamSoundTime < 0.2f ? 0.2f :
							   (fMinStreamSoundTime > 1.5f ? 1.5f : fMinStreamSoundTime));
	}
	soundInfo.m_fDistanceFactor = 1.0f / 64.0f;
	soundInfo.m_fDopplerFactor = 1.0f;

	// Go initialize the sounds.
	m_bUseReverb = LTFALSE;
	if(( m_resSoundInit = ((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->InitSound(&soundInfo)) == LT_OK )
	{
		if( soundInfo.m_dwResults & INITSOUNDINFORESULTS_REVERB )
		{
			m_bUseReverb = LTTRUE;
		}

		hVar = g_pLTClient->GetConsoleVar( "ReverbLevel" );
		if( hVar )
		{
			m_fReverbLevel = g_pLTClient->GetVarValueFloat( hVar );
		}
		else
			m_fReverbLevel = 1.0f;

		reverbProperties.m_dwParams = REVERBPARAM_VOLUME;
		reverbProperties.m_fVolume = m_fReverbLevel;
		((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetReverbProperties( &reverbProperties );
	}
	else
	{
		if( m_resSoundInit == LT_NO3DSOUNDPROVIDER )
		{
			nError = IDS_MESSAGE_INVALID3DSOUNDPROVIDER;
		}
		else
		{
			nError = IDS_SOUNDNOTINITED;
		}

		DoMessageBox( nError, TH_ALIGN_CENTER );
	}

	return;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::CSPrint
//
//	PURPOSE:	Displays a line of text on the client
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::CSPrint (char* msg, ...)
{
	// parse the message

	char pMsg[256];
	va_list marker;
	va_start (marker, msg);
	int nSuccess = vsprintf (pMsg, msg, marker);
	va_end (marker);

	if (nSuccess < 0) return;
	
	// now display the message

	m_messageMgr.AddLine(pMsg);

	g_pLTClient->CPrint(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ShowSplash
//
//	PURPOSE:	Called after engine is fully initialized
//				to show a splash screen
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::ShowSplash()
{
	if (!g_pLTClient) return;	
	
	HSURFACE hSplash = g_pLTClient->CreateSurfaceFromBitmap ("interface/splash.pcx");
	if (!hSplash) return;

	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nWidth = 0;
	uint32 nHeight = 0;
	
	g_pLTClient->GetSurfaceDims (hScreen, &nWidth, &nHeight);
	
	LTRect rcDst;
	rcDst.left = rcDst.top = 0;
	rcDst.right = nWidth;
	rcDst.bottom = nHeight;

	g_pLTClient->GetSurfaceDims (hSplash, &nWidth, &nHeight);
	LTRect rcSrc;
	rcSrc.left = rcSrc.top = 0;
	rcSrc.right = nWidth;
	rcSrc.bottom = nHeight;

	g_pLTClient->ClearScreen (LTNULL, CLEARSCREEN_SCREEN);
	g_pLTClient->Start3D();
	g_pLTClient->RenderCamera (m_hCamera);
	g_pLTClient->StartOptimized2D();
	g_pLTClient->ScaleSurfaceToSurface (hScreen, hSplash, &rcDst, &rcSrc);
	g_pLTClient->EndOptimized2D();
	g_pLTClient->End3D();
	g_pLTClient->FlipScreen (FLIPSCREEN_CANDRAWCONSOLE);
	AddToClearScreenCount();

	g_pLTClient->DeleteSurface (hSplash);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnEngineInitialized
//
//	PURPOSE:	Called after engine is fully initialized
//				Handle object initialization here
//
// ----------------------------------------------------------------------- //

uint32 CRiotClientShell::OnEngineInitialized(struct RMode *pMode, LTGUID *pAppGuid)
{
	//CWinUtil::DebugBreak();

	if (!g_pLTClient) return LT_OK;	

	*pAppGuid = SHOGOGUID;

	LTFLOAT nStartTime = CWinUtil::GetTime();

	m_MoveMgr = new CMoveMgr(this);
	if(!m_MoveMgr)
		return LT_ERROR;

	// Init our client send rate var.
	g_CV_CSendRate.Init(g_pLTClient, "CSendRate", NULL, DEFAULT_CSENDRATE);
	

	HCONSOLEVAR hIsSet = g_pLTClient->GetConsoleVar("UpdateRateInitted");
	if(hIsSet && g_pLTClient->GetVarValueFloat(hIsSet) == 1.0f)
	{
		// Ok it's already initialized.
	}
	else
	{
		// Initialize the update rate.
		g_pLTClient->RunConsoleString("+UpdateRateInitted 1");
		g_pLTClient->RunConsoleString("+UpdateRate 6");
	}


	m_MoveMgr->Init(g_pLTClient);

	g_pLTClient->RegisterConsoleProgram("LeakFile", LeakFileFn);
	g_pLTClient->RegisterConsoleProgram("Connect", ConnectFn);
	g_pLTClient->RegisterConsoleProgram("FragSelf", FragSelfFn);
	g_pLTClient->RegisterConsoleProgram("Record", RecordFn);
	g_pLTClient->RegisterConsoleProgram("PlayDemo", PlayDemoFn);
	g_pLTClient->RegisterConsoleProgram("InitSound", InitSoundFn);

	g_pLTClient->SetModelHook((ModelHookFn)DefaultModelHook, this);

	// Make sure the save directory exists...

	if (!CWinUtil::DirExist ("Save"))
	{
		CWinUtil::CreateDir ("Save");
	}


	// Add to NumRuns count...
	
	LTFLOAT nNumRuns = 0.0f;
	HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar ("NumRuns");
	if (hVar)
	{
		nNumRuns = g_pLTClient->GetVarValueFloat (hVar);
	}
	nNumRuns++;
	
	char strConsole[64];
	sprintf (strConsole, "+NumRuns %f", nNumRuns);
	g_pLTClient->RunConsoleString (strConsole);

	// check advanced options...

	hVar = g_pLTClient->GetConsoleVar ("DisableMusic");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableMusic = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("DisableSound");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableSound = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("DisableMovies");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableMovies = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("EnableOptSurf");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedEnableOptSurf = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("DisableLightMap");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableLightMap = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("EnableTripBuf");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedEnableTripBuf = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("DisableDx6Cmds");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableDx6Cmds = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("EnableTJuncs");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedEnableTJuncs = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("DisableFog");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableFog = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("DisableLines");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableLines = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("DisableModelFB");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableModelFB = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("EnablePixDub");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedEnablePixelDoubling = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("EnableMultiTex");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedEnableMultiTexturing = LTTRUE;
	hVar = g_pLTClient->GetConsoleVar ("DisableJoystick");
	if (hVar && g_pLTClient->GetVarValueFloat (hVar))		m_bAdvancedDisableJoystick = LTTRUE;

	// record the original state of sound and music

	hVar = g_pLTClient->GetConsoleVar ("SoundEnable");
	m_bSoundOriginallyEnabled = (LTBOOL) g_pLTClient->GetVarValueFloat (hVar);
	
	hVar = g_pLTClient->GetConsoleVar ("MusicEnable");
	m_bMusicOriginallyEnabled = (LTBOOL) g_pLTClient->GetVarValueFloat (hVar);

	hVar = g_pLTClient->GetConsoleVar ("ModelFullbrite");
	m_bModelFBOriginallyEnabled	= (LTBOOL) g_pLTClient->GetVarValueFloat (hVar);
	
	hVar = g_pLTClient->GetConsoleVar ("LightMap");
	m_bLightmappingOriginallyEnabled = (LTBOOL) g_pLTClient->GetVarValueFloat (hVar);

	// implement any advanced options here (before renderer is started)

	hVar = g_pLTClient->GetConsoleVar ("SoundEnable");
	if (!hVar && !AdvancedDisableSound())
	{
		g_pLTClient->RunConsoleString ("SoundEnable 1");
	}
	hVar = g_pLTClient->GetConsoleVar ("MusicEnable");
	if (!hVar && !AdvancedDisableMusic())
	{
		g_pLTClient->RunConsoleString ("MusicEnable 1");
	}

	if (AdvancedEnableOptSurf())
	{
		g_pLTClient->RunConsoleString ("OptimizeSurfaces 1");
	}
	if (AdvancedEnableTripBuf())
	{
		g_pLTClient->RunConsoleString ("TripleBuffer 1");
	}
	if (AdvancedDisableDx6Cmds())
	{
		g_pLTClient->RunConsoleString ("UseDX6Commands 0");
	}
	if (AdvancedEnableTJuncs())
	{
		g_pLTClient->RunConsoleString ("FixTJunc 1");
	}
	if (AdvancedDisableFog())
	{
		g_pLTClient->RunConsoleString ("FogEnable 0");
	}
	if (AdvancedDisableLines())
	{
		g_pLTClient->RunConsoleString ("DrawLineSystems 0");
	}
	if (AdvancedDisableModelFB())
	{
		g_pLTClient->RunConsoleString ("ModelFullBrite 0");
	}
	if (AdvancedEnablePixelDoubling())
	{
		g_pLTClient->RunConsoleString ("PixelDouble 1");
	}
	if (AdvancedEnableMultiTexturing())
	{
		g_pLTClient->RunConsoleString ("Force1Pass 1");
	}
	if (AdvancedDisableJoystick())
	{
		g_pLTClient->RunConsoleString ("JoystickDisable 1");
	}

	// Determine if we were lobby launched...

	LTBOOL bNetworkGameStarted = LTFALSE;
	LTBOOL bLobbyLaunched      = LTFALSE;

	LTRESULT dr = g_pLTClient->IsLobbyLaunched("dplay2");
	if (dr == LT_OK)
	{
		BOOL bRet = NetStart_DoLobbyLaunchWizard(g_pLTClient);
		if (bRet)
		{
			bLobbyLaunched      = LTTRUE;
			bNetworkGameStarted = LTTRUE;
		}
		else
		{
			g_pLTClient->Shutdown();
			return LT_ERROR;
		}
	}


	// Check for console vars that say we should do networking setup stuff...

	BOOL bDoNetStuff = LTFALSE;

	hVar = g_pLTClient->GetConsoleVar ("Multiplayer");
	if (hVar)
	{
		if ((LTBOOL)g_pLTClient->GetVarValueFloat(hVar)) bDoNetStuff = LTTRUE;
	}
	g_pLTClient->RunConsoleString ("+Multiplayer 0");

	hVar = g_pLTClient->GetConsoleVar ("Connect");
	if (hVar)
	{
		const char* sVar = g_pLTClient->GetVarValueString(hVar);
		if (sVar && sVar[0] != '\0' && strcmp(sVar, "0") != 0) bDoNetStuff = LTTRUE;
	}

	hVar = g_pLTClient->GetConsoleVar ("ConnectPlr");
	if (hVar)
	{
		const char* sVar = g_pLTClient->GetVarValueString(hVar);
		if (sVar && sVar[0] != '\0' && strcmp(sVar, "0") != 0) bDoNetStuff = LTTRUE;
	}

	if (bLobbyLaunched) bDoNetStuff = LTFALSE;

//#ifdef _DEMO
//	bDoNetStuff = LTFALSE;
//#endif


	// Determine if we should display the networking dialogs...

	if (bDoNetStuff)
	{
		bNetworkGameStarted = NetStart_DoWizard (g_pLTClient);

		if (!bNetworkGameStarted)
		{
			g_pLTClient->Shutdown();
			return LT_ERROR;
		}
	}

	// Initialize the renderer

	LTRESULT hResult = g_pLTClient->SetRenderMode(pMode);
	if (hResult != LT_OK)
	{
		g_pLTClient->DebugOut("Shogo Error: Couldn't set render mode!\n");

		RMode rMode;

		// If an error occurred, try 640x480x16...
		
		rMode.m_Width		= 640;
		rMode.m_Height		= 480;
		rMode.m_BitDepth	= 16;
		//rMode.m_bHardware	= pMode->m_bHardware;

		//sprintf(rMode.m_RenderDLL, "%s", pMode->m_RenderDLL);
		sprintf(rMode.m_InternalName, "%s", pMode->m_InternalName);
		sprintf(rMode.m_Description, "%s", pMode->m_Description);

		g_pLTClient->DebugOut("Setting render mode to 640x480x16...\n");
		
		if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
		{
			// Okay, that didn't work, looks like we're stuck with software...
			/*
			rMode.m_bHardware = LTFALSE;

			sprintf(rMode.m_RenderDLL, "soft.ren");
			sprintf(rMode.m_InternalName, "");
			sprintf(rMode.m_Description, "");

			g_pLTClient->DebugOut("Setting render mode to software...\n");

			if (g_pLTClient->SetRenderMode(&rMode) != LT_OK)
			{
				g_pLTClient->DebugOut("Shogo Error: Couldn't set software render mode.\nShutting down Shogo...\n");
				g_pLTClient->ShutdownWithMessage("Shogo Error: Couldn't set software render mode.\nShutting down Shogo...\n");
				return LT_OK;
			} 
			*/

			g_pLTClient->DebugOut("Shogo Error: Couldn't set render mode.\nShutting down Shogo...\n");
			g_pLTClient->ShutdownWithMessage("Shogo Error: Couldn't set render mode.\nShutting down Shogo...\n");
			return LT_OK;
		}
	}

	// show the splash screen

	ShowSplash();


	// Create the camera...

	uint32 dwWidth = 640, dwHeight = 480;
	g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_ObjectType = OT_CAMERA;

	m_hCamera = g_pLTClient->CreateObject(&theStruct);
	g_pLTClient->SetCameraRect(m_hCamera, LTFALSE, 0, 0, dwWidth, dwHeight);
	
	LTFLOAT y = (m_fCurrentFovX * dwHeight) / dwWidth;
	g_pLTClient->SetCameraFOV(m_hCamera, m_fCurrentFovX, y);

	
	// Attempt to find the movies path

	CWinUtil::GetMoviesPath (m_strMoviesDir);

	// Initialize the physics states...

	g_normalPhysicsState.m_pClientDE = g_pLTClient;
	VEC_SET(g_normalPhysicsState.m_GravityAccel, 0.0f, -1000.0f, 0.0f);
	g_normalPhysicsState.m_VelocityDampen = 0.5f;

	g_waterPhysicsState.m_pClientDE = g_pLTClient;
	VEC_SET(g_waterPhysicsState.m_GravityAccel, 0.0f, -500.0f, 0.0f);
	g_waterPhysicsState.m_VelocityDampen = 0.25f;


	// Interface stuff...

	m_messageMgr.Init (g_pLTClient, this);
	m_messageMgr.Enable (LTTRUE);
	m_cheatMgr.Init (g_pLTClient);
	m_ClientInfo.Init (g_pLTClient);
	m_inventory.Init (g_pLTClient, this);
	m_infoDisplay.Init (g_pLTClient);
	m_LightScaleMgr.Init (g_pLTClient);

	// Setup the music stuff...
	if (!m_Music.IsInitialized( ) && !AdvancedDisableMusic())
	{
		HCONSOLEVAR hConsoleVar;
		hConsoleVar = g_pLTClient->GetConsoleVar("musictype");

		// Default to ima if music never set before...
		if( !hConsoleVar )
		{
			m_Music.Init( g_pLTClient, LTTRUE );
		}
		else
		{
			// See if they want cdaudio or ima...
			if( stricmp( g_pLTClient->GetVarValueString(hConsoleVar), "cdaudio") == 0 )
				m_Music.Init( g_pLTClient, LTFALSE );
			else
				m_Music.Init( g_pLTClient, LTTRUE );
		}
	}

	// back to interface stuff...

	if (!m_menu.Init (g_pLTClient, this))
	{
		CSPrint ("Could not init menu");
		
		// if we couldn't init, something critical must have happened (like no render dlls)
		g_pLTClient->ShutdownWithMessage("Could not initialize menu");
		return LT_OK;
	}

	if (!m_stats.Init (g_pLTClient, this))
	{
		CSPrint ("Could not init player stats");
	}

	m_objectives.Init (g_pLTClient, this);

	// player camera (non-1st person) stuff...

	if (!m_playerCamera.Init(g_pLTClient))
	{
		CSPrint ("Could not init player camera!");
	}
	else
	{
		LTVector vOffset;
		VEC_SET(vOffset, 0.0f, 30.0, 0.0f);

		m_playerCamera.SetDistUp(10.0f);
		m_playerCamera.SetPointAtOffset(vOffset);
		m_playerCamera.SetChaseOffset(vOffset);
		m_playerCamera.SetCameraState(CPlayerCamera::SOUTH);
		m_playerCamera.GoFirstPerson();
	}


	// init the "press any key" surface

	m_hPressAnyKey = CTextHelper::CreateSurfaceFromString (g_pLTClient, m_menu.GetFont12s(), IDS_PRESSANYKEY);
	g_pLTClient->GetSurfaceDims (m_hPressAnyKey, &m_cxPressAnyKey, &m_cyPressAnyKey);
	
	// init the "loading" surface

	m_hLoading = g_pLTClient->CreateSurfaceFromBitmap ("interface/loading.pcx");
	g_pLTClient->GetSurfaceDims (m_hLoading, &m_cxLoading, &m_cyLoading);


	// Init the special fx mgr...

	if (!m_sfxMgr.Init(g_pLTClient)) return LT_OK;


	// Post an error if the sounds didn't init correctly

	// Okay, start the game...

	if (!bNetworkGameStarted)
	{
		StartGameRequest request;
		memset( &request, 0, sizeof( StartGameRequest ));

		NetStart_ClearGameStruct();  // Start with clean slate
		request.m_pGameInfo   = NetStart_GetGameStruct();
		request.m_GameInfoLen = sizeof(NetGame_t);

		HCONSOLEVAR hVar;

		if (hVar = g_pLTClient->GetConsoleVar("NumConsoleLines"))
		{
			LTFLOAT fLines = g_pLTClient->GetVarValueFloat(hVar);
			//m_messageMgr.SetMaxMessages((int)fLines);
			g_pLTClient->RunConsoleString("+NumConsoleLines 0");
		}
		
		if (hVar = g_pLTClient->GetConsoleVar("runworld"))
		{
			strncpy (request.m_WorldName, g_pLTClient->GetVarValueString(hVar), MAX_SGR_STRINGLEN - 1);
			request.m_Type = STARTGAME_NORMAL;
			LTRESULT dr = g_pLTClient->StartGame(&request);
			if (dr != LT_OK)
			{
				HSTRING hString = g_pLTClient->FormatString (IDS_NOLOADLEVEL);
				g_pLTClient->ShutdownWithMessage (g_pLTClient->GetStringData (hString));
				g_pLTClient->FreeString (hString);
				return LT_ERROR;
			}
		}
		else
		{
			// play the movies

			PlayIntroMovies (g_pLTClient);
		}
	}
	else
	{
		m_nGameState = GS_LOADINGLEVEL;
	}

	LTFLOAT nEndTime = CWinUtil::GetTime();
	char strTimeDiff[64];
	sprintf (strTimeDiff, "Game initialized in %f seconds.\n", nEndTime - nStartTime);
	CWinUtil::DebugOut (strTimeDiff);

	
	// Check for playdemo.
	if (hVar = g_pLTClient->GetConsoleVar("playdemo"))
	{
		DoLoadWorld("", NULL, NULL, LOAD_NEW_GAME, NULL, const_cast<char *>(g_pLTClient->GetVarValueString(hVar)));
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnEngineTerm()
//
//	PURPOSE:	Called before the engine terminates itself
//				Handle object destruction here
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnEngineTerm()
{
	if (!g_pLTClient) return;
	
	if (m_hCamera)
	{
		g_pLTClient->RemoveObject(m_hCamera);
		m_hCamera = LTNULL;
	}

	if (m_hPressAnyKey) 
	{
		g_pLTClient->DeleteSurface (m_hPressAnyKey);
		m_hPressAnyKey = LTNULL;
	}

	if (m_hLoadingWorld) 
	{
		g_pLTClient->DeleteSurface (m_hLoadingWorld);
		m_hLoadingWorld = LTNULL;
	}

	if (m_hWorldName) 
	{
		g_pLTClient->DeleteSurface (m_hWorldName);
		m_hWorldName = LTNULL;
	}

	if (m_hLoading) 
	{
		g_pLTClient->DeleteSurface (m_hLoading);
		m_hLoading = LTNULL;
	}

	m_messageMgr.Term();
	m_menu.Term();
	m_stats.Term();
	m_Music.Term( );
	m_infoDisplay.Term();
	m_LightScaleMgr.Term();
	
	if (m_bSoundOriginallyEnabled && AdvancedDisableSound())
	{
		g_pLTClient->RunConsoleString ("SoundEnable 1");
	}

	if (m_bMusicOriginallyEnabled && AdvancedDisableMusic())
	{
		g_pLTClient->RunConsoleString ("MusicEnable 1");
	}

	if (m_bModelFBOriginallyEnabled)
	{
		g_pLTClient->RunConsoleString ("ModelFullbrite 1");
	}

	if (m_bLightmappingOriginallyEnabled)
	{
		g_pLTClient->RunConsoleString ("LightMap 1");
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnEvent(uint32 dwEventID, uint32 dwParam)
{
	ILTClient *pClientDE;
	uint32 cxLoading, cyLoading, cxScreen, cyScreen;
	HSURFACE hLoading;
	HSURFACE hScreen;
	CBitmapFont *pFont;

	if(!g_pLTClient)
		return;

	switch (dwEventID)
	{
		// Called when the renderer has switched into
		// the new mode but before it reloads all the textures
		// so you can display a loading screen.
		
		case LTEVENT_RENDERALMOSTINITTED :
			if(m_bSwitchingModes)
			{
				ClearAllScreenBuffers();
				
				hScreen = pClientDE->GetScreenSurface();
				g_pLTClient->GetSurfaceDims (hScreen, &cxScreen, &cyScreen);

				g_pLTClient->Start3D();
				g_pLTClient->StartOptimized2D();

				hLoading = g_pLTClient->CreateSurfaceFromBitmap ("interface/blanktag.pcx");
				if(hLoading)
				{
					g_pLTClient->GetSurfaceDims (hLoading, &cxLoading, &cyLoading);
					g_pLTClient->DrawSurfaceToSurface(hScreen, hLoading, LTNULL, ((int)cxScreen - (int)cxLoading) / 2, ((int)cyScreen - (int)cyLoading) / 2);
					g_pLTClient->DeleteSurface(hLoading);
					
					pFont = m_menu.GetFont08n();
					if(pFont)
					{
						hLoading = CTextHelper::CreateSurfaceFromString(pClientDE,
							pFont, IDS_REINITIALIZING_RENDERER);
						if(hLoading)
						{
							g_pLTClient->GetSurfaceDims (hLoading, &cxLoading, &cyLoading);
							g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, hLoading, 
								LTNULL, ((int)cxScreen - (int)cxLoading) / 2, ((int)cyScreen - (int)cyLoading) / 2,
								SETRGB_T(0,0,0));
							g_pLTClient->DeleteSurface(hLoading);
						}
					}
				}

				g_pLTClient->EndOptimized2D();
				g_pLTClient->End3D();
				g_pLTClient->FlipScreen(0);
			}
		break;

		// Client disconnected from server.  dwParam will 
		// be a error flag found in de_codes.h.
		
		case LTEVENT_DISCONNECT :

			if((dwParam & ~ERROR_DISCONNECT) == LT_INVALIDNETVERSION)
			{
				if(IsMultiplayerGame())
				{
					ClearAllScreenBuffers();
					SetMenuMode(LTTRUE, LTFALSE);
					DoMessageBox(IDS_DISCONNECTED_WRONG_VERSION, TH_ALIGN_CENTER);
				}
			}
			else
			{
				if(IsMultiplayerGame())
				{
					ClearAllScreenBuffers();
					SetMenuMode(LTTRUE, LTFALSE);
	//#ifdef _DEMO
					//DoMessageBox(IDS_DEMODISCONNECT, TH_ALIGN_CENTER);
	//#else
					DoMessageBox(IDS_DISCONNECTED_FROM_SERVER, TH_ALIGN_CENTER);
	//#endif
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
		break;

		// The renderer is being shutdown.  This happens when
		// ShutdownRender is called or if the app loses focus.

		case  LTEVENT_RENDERTERM :
		break;

		default :
		{
			uint32 nStringID = IDS_UNSPECIFIEDERROR;
			SetMenuMode (LTTRUE, LTFALSE);
			DoMessageBox (nStringID, TH_ALIGN_CENTER);
		}
		break;
	}
}


LTRESULT CRiotClientShell::OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector *pPos)
{
	return m_MoveMgr->OnObjectMove(hObj, bTeleport, pPos);
}


LTRESULT CRiotClientShell::OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation *pNewRot)
{
	return m_MoveMgr->OnObjectRotate(hObj, bTeleport, pNewRot);
}


LTRESULT	CRiotClientShell::OnTouchNotify(HOBJECT hMain, CollisionInfo *pInfo, LTFLOAT forceMag)
{
	m_sfxMgr.OnTouchNotify(hMain, pInfo, forceMag);
	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::PreLoadWorld()
//
//	PURPOSE:	Called before world loads
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::PreLoadWorld(char *pWorldName)
{
	if (!g_pLTClient) return;
	
	if (IsMainWindowMinimized())
	{
		NetStart_RestoreMainWnd();
		MainWindowRestored();
	}

	char* pStrWorldOnly = &pWorldName[strlen(pWorldName) - 1];
	while (*pStrWorldOnly != '\\' && pStrWorldOnly != pWorldName) pStrWorldOnly--;
	if (pStrWorldOnly != pWorldName) pStrWorldOnly++;

	SAFE_STRCPY(m_strCurrentWorldName, pStrWorldOnly);
	m_objectives.SetLevelName (m_strCurrentWorldName);

	if (m_nGameState == GS_BUMPER || m_nGameState == GS_LOADINGLEVEL)
	{
		UpdateLoadingLevel();
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnEnterWorld()
//
//	PURPOSE:	Handle entering world
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnEnterWorld()
{
	if (!g_pLTClient) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;


	// if we are in a multiplayer game, set the correct game state...

	if (m_nGameState == GS_MPLOADINGLEVEL)
	{
		m_nGameState = GS_PLAYING;
	}

	m_bFirstUpdate = LTTRUE;
	m_ePlayerState = PS_UNKNOWN;

	m_bPlayerPosSet = LTFALSE;

	m_bNewObjective	= LTFALSE;
	m_bDrawHud		= LTTRUE;

	AddToClearScreenCount();

	pClientDE->ClearInput();

	m_bHandledStartup = LTFALSE;
	m_bInWorld		  = LTTRUE;

	m_fVelMagnitude	= 0.0f;

	m_stats.OnEnterWorld(m_bRestoringGame);
	m_menu.OnEnterWorld();
	m_menu.ExitMenu(LTTRUE);


	// If we're in loading level mode switch to bumper mode

	if (m_nGameState == GS_LOADINGLEVEL)
	{
		if (m_hBumperText)
		{
			PauseGame(LTTRUE);
			m_nGameState = GS_BUMPER;
		}
	}

	SetExternalCamera(LTFALSE);

	m_bRestoringGame		= LTFALSE;
	m_bCameraPosInited		= LTFALSE;
	m_nPlayerInfoChangeFlags |= CLIENTUPDATE_PLAYERROT;
	VEC_INIT( m_vLastReverbPos );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnExitWorld()
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnExitWorld()
{
	if (!g_pLTClient) return;

	// See if we're changing levels if a multiplayer game (if OnExitWorld()
	// was called before we got the change level message)...

	if (IsMultiplayerGame() && m_nGameState != GS_MPLOADINGLEVEL)
	{
		HandleMPChangeLevel();
	}


	// Make sure credits are reset if playing...

	m_credits.Term();

	m_bInWorld		= LTFALSE;
	m_bStartedLevel	= LTFALSE;

	m_LightScaleMgr.ClearLightScale (&m_vDefaultLightScale, LightEffectWorld);

	memset (m_strCurrentWorldName, 0, 256);

	// Kill the music...
	g_pLTClient->StopMusic (MUSIC_IMMEDIATE);
	m_Music.TermPlayLists();
	m_eMusicLevel  = CMusic::MUSICLEVEL_SILENCE;

	if (m_h3rdPersonCrosshair)
	{
		g_pLTClient->RemoveObject(m_h3rdPersonCrosshair);
		m_h3rdPersonCrosshair = LTNULL;
	}

	m_sfxMgr.RemoveAll();					// Remove all the sfx
	m_playerCamera.AttachToObject(LTNULL);	// Detatch camera

	m_weaponModel.Reset();

	if (m_hContainerSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hContainerSound);
		m_hContainerSound = LTNULL;
	}

	if (m_hTransmissionSound)
	{
		g_pLTClient->SoundMgr()->KillSound (m_hTransmissionSound);
		m_hTransmissionSound = LTNULL;
	}

	m_ClientInfo.RemoveAllClients();
	m_inventory.ShogoPowerupClear();

	m_stats.OnExitWorld();
	m_menu.OnExitWorld();

	if (m_hTransmissionImage)
	{
		g_pLTClient->DeleteSurface (m_hTransmissionImage);
		m_hTransmissionImage = LTNULL;
	}

	if (m_hTransmissionText)
	{
		g_pLTClient->DeleteSurface (m_hTransmissionText);
		m_hTransmissionText = LTNULL;
	}

	m_fTransmissionTimeLeft = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::PreUpdate()
//
//	PURPOSE:	Handle client pre-updates
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::PreUpdate()
{
	if (!g_pLTClient) return;

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
		AddToClearScreenCount();
	}
	else if (m_bWasUsingExternalCamera && !m_bUsingExternalCamera)
	{
		m_bWasUsingExternalCamera = LTFALSE;
		AddToClearScreenCount();
	}

	if (m_bClearScreenAlways)
	{
		g_pLTClient->ClearScreen (LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	}
	else if (m_nClearScreenCount)
	{
		g_pLTClient->ClearScreen (LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
		m_nClearScreenCount--;
	}
	else
	{
		g_pLTClient->ClearScreen (LTNULL, CLEARSCREEN_RENDER);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::Update()
//
//	PURPOSE:	Handle client updates
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::Update()
{
	if (!g_pLTClient) return;

	// Handle first update...

	if (m_bFirstUpdate)
	{
		FirstUpdate();
	}


	// Update tint if applicable (always do this to make sure tinting
	// gets finished)...

	UpdateScreenTint();

	UpdateModelGlow();
	

	// Update any client-side special effects...

	m_sfxMgr.UpdateSpecialFX();

	
	// If there's a messagebox, draw it...

	if (m_pMessageBox)
	{
		m_pMessageBox->Draw();
		return;
	}


	// Make sure menu music is only playing in the menus (or loading a level)...

	if (m_nGameState != GS_MENU && 
		m_nGameState != GS_LOADINGLEVEL && 
		m_nGameState != GS_BUMPER)
	{
		SetMenuMusic(LTFALSE);
	}


	// Make sure menu polygrid is/isn't around...

	if (m_nGameState == GS_MENU || m_nGameState == GS_PAUSED ||
		m_nGameState == GS_LOADINGLEVEL || m_nGameState == GS_BUMPER ||
		m_nGameState == GS_CREDITS || m_nGameState == GS_INTRO ||
		m_nGameState == GS_DEMO_MULTIPLAYER)
	{
		CreateMenuPolygrid();
	}
	else if (m_nGameState != GS_MENU && m_nGameState != GS_PAUSED && 
			 m_nGameState != GS_LOADINGLEVEL && m_nGameState != GS_BUMPER &&
			 m_nGameState != GS_CREDITS && m_nGameState != GS_INTRO &&
			 m_nGameState != GS_DEMO_MULTIPLAYER)
	{
		RemoveMenuPolygrid();
	}


	// Update based on the game state...

	switch (m_nGameState)
	{
		case GS_PLAYING :
		break;

		case GS_UNDEFINED:
		case GS_MPLOADINGLEVEL:
		{
			// Don't do anything
			return;
		}
		break;

		case GS_MOVIES:
		{
			UpdateMoviesState();
			return;
		}
		break;
		
		case GS_CREDITS :
		{
			UpdateCreditsState();
			return;
		}
		break;

		case GS_INTRO :
		{
			UpdateIntroState();
			return;
		}
		break;

		case GS_MENU :
		{
			UpdateMenuState();
			return;
		}
		break;

		case GS_BUMPER :
		{
			UpdateBumperState();
			return;
		}
		break;

		case GS_LOADINGLEVEL :
		{
			UpdateLoadingLevelState();
			return;
		}
		break;

		case GS_PAUSED:
		{
			UpdatePausedState();
			return;
		}
		break;

		case GS_DEMO_MULTIPLAYER :
		{
			UpdateDemoMultiplayerState();
			return;
		}
		break;

		default : break;
	}


	// Update client-side physics structs...

	SetPhysicsStateTimeStep(&g_normalPhysicsState, g_pLTClient->GetFrameTime());
	SetPhysicsStateTimeStep(&g_waterPhysicsState, g_pLTClient->GetFrameTime());
	
	

	// At this point we only want to proceed if the player is in the world...

	if (!IsPlayerInWorld()) return;


	m_MoveMgr->Update();

	UpdateSoundReverb( );
	
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
		LTFLOAT fFrameTime = g_pLTClient->GetFrameTime();

		m_fCurSkyXOffset += fFrameTime * m_fPanSkyOffsetX;
		m_fCurSkyZOffset += fFrameTime * m_fPanSkyOffsetZ;

		g_pLTClient->SetGlobalPanInfo(GLOBALPAN_SKYSHADOW, m_fCurSkyXOffset, m_fCurSkyZOffset, m_fPanSkyScaleX, m_fPanSkyScaleZ);
	}


	// update the player stats and inventory
	if (!m_bStatsSizedOff)
	{
		m_stats.Update();
	}

	// Keep track of what the player is doing...

	UpdatePlayerFlags();


	if (m_bAdjustLightScale)
	{
		AdjustLightScale();
	}


	// If in spectator mode, just do the camera stuff...
	if (m_bSpectatorMode || IsPlayerDead())
	{
		UpdateCamera();
		RenderCamera();
		return;
	}

	// Update weapon position if appropriated (Should probably remove this
	// at some point...although, it might be cool for end users???)...
	if (m_bTweakingWeapon)
	{
		UpdateWeaponPosition();
		UpdateCamera();
		RenderCamera();
		return;
	}
	else if (m_bTweakingWeaponMuzzle)
	{
		UpdateWeaponMuzzlePosition();
		UpdateCamera();
		RenderCamera();
		return;
	}

	// Update head-bob/head-cant camera offsets...
	if (m_bOnGround && !m_bZoomView) 
	{
		UpdateHeadBob();
		UpdateHeadCant();
	}				

	// Update duck camera offset...
	UpdateDuck();

	// Update the camera's position...
	UpdateCamera();

	// Render the camera and draw the interface first...then process everything
	RenderCamera();

	// Update container effects...
	UpdateContainerFX();

	// Update any debugging information...

	UpdateDebugInfo();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::PostUpdate()
//
//	PURPOSE:	Handle post updates - after the scene is rendered
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::PostUpdate()
{
	if (!g_pLTClient) return;

	if (m_bQuickSave)
	{
		SaveGame(QUICKSAVE_FILENAME);
		m_bQuickSave = LTFALSE;
	}

	// conditions where we don't want to flip...

	if (m_nGameState == GS_MPLOADINGLEVEL)
	{
		return;
	}

	if (m_ePlayerState == PS_UNKNOWN && m_bInWorld)
	{
		return;
	}
	
	// See if the game is over...

	if (m_bGameOver && m_nGameState == GS_PLAYING)
	{
		UpdateGameOver();
	}

	g_pLTClient->FlipScreen (FLIPSCREEN_CANDRAWCONSOLE);

	// Check to see if we should start the world...(do after flip)...

	if (m_nGameState == GS_INTRO)
	{		
		if (m_credits.IsDone())
		{
			m_credits.Term();
			DoStartGame();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateCamera()
//
//	PURPOSE:	Update the camera position/rotation
//
// ----------------------------------------------------------------------- //
void CRiotClientShell::UpdateSoundReverb( )
{
	LTVector vPlayerDims;
	ClientIntersectInfo info;
	ClientIntersectQuery query;
	LTVector vPos[6], vSegs[6];
	int i;
	LTFLOAT fVolume, fReverbLevel;
	LTBOOL bOpen;
	HLOCALOBJ hPlayerObj;
	ReverbProperties reverbProperties;
	HCONSOLEVAR hVar;

	if( !m_bUseReverb )
		return;

	hPlayerObj = g_pLTClient->GetClientObject();
	if( !hPlayerObj )
		return;

	hVar = g_pLTClient->GetConsoleVar( "ReverbLevel" );
	if( hVar )
	{
		fReverbLevel = g_pLTClient->GetVarValueFloat( hVar );
	}
	else
		fReverbLevel = 1.0f;

	// Check if reverb was off and is still off
	if( fReverbLevel < 0.001f && m_fReverbLevel < 0.001f )
		return;

	m_fReverbLevel = fReverbLevel;

	// Check if it's time yet
	if( g_pLTClient->GetTime( ) < m_fNextSoundReverbTime )
		return;

	// Update timer
	m_fNextSoundReverbTime = g_pLTClient->GetTime( ) + SOUND_REVERB_UPDATE_PERIOD;

	HOBJECT hFilterList[] = {hPlayerObj, m_MoveMgr->GetObject(), LTNULL};

	memset( &query, 0, sizeof( query ));
	query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	query.m_FilterFn = ObjListFilterFn;
	query.m_pUserData = hFilterList;

	g_pLTClient->GetObjectPos( hPlayerObj, &query.m_From );
	g_pLTClient->Physics( )->GetObjectDims( hPlayerObj, &vPlayerDims );

	// Player must move at least 2x his dims mag before reverb is checked again.
	if( VEC_DISTSQR( query.m_From, m_vLastReverbPos ) < 4 * VEC_MAGSQR( vPlayerDims ))
		return;

	VEC_COPY( m_vLastReverbPos, query.m_From );

	VEC_SET( vSegs[0], query.m_From.x + 2000.0f,	query.m_From.y,				query.m_From.z );
	VEC_SET( vSegs[1], query.m_From.x - 2000.0f,	query.m_From.y,				query.m_From.z );
	VEC_SET( vSegs[2], query.m_From.x,				query.m_From.y + 2000.0f,	query.m_From.z );
	VEC_SET( vSegs[3], query.m_From.x,				query.m_From.y - 2000.0f,	query.m_From.z );
	VEC_SET( vSegs[4], query.m_From.x,				query.m_From.y,				query.m_From.z + 2000.0f );
	VEC_SET( vSegs[5], query.m_From.x,				query.m_From.y,				query.m_From.z - 2000.0f );

	bOpen = LTFALSE;
	for( i = 0; i < 6; i++ )
	{
		VEC_COPY( query.m_To, vSegs[i] );

		if( g_pLTClient->IntersectSegment( &query, &info ))
		{
			VEC_COPY( vPos[i], info.m_Point );
			if( info.m_SurfaceFlags == ST_AIR || info.m_SurfaceFlags == ST_SKY )
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

	fVolume = VEC_DIST( vPos[0], vPos[1] );
	fVolume *= VEC_DIST( vPos[2], vPos[3] );
	fVolume *= VEC_DIST( vPos[4], vPos[5] );

	// Use room types that are not completely enclosed rooms
	if( bOpen )
	{
		if( fVolume < 100.0f*100.0f*100.0f )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_SEWERPIPE;
			reverbProperties.m_fReflectTime		= 0.005f;
			reverbProperties.m_fDecayTime		= 1.493f;
			reverbProperties.m_fVolume			= 0.1f * m_fReverbLevel;
		}
		else if( fVolume < 500.0f*500.0f*500.0f )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_PLAIN;
			reverbProperties.m_fReflectTime		= 0.005f;
			reverbProperties.m_fDecayTime		= 1.493f;
			reverbProperties.m_fVolume			= 0.2f * m_fReverbLevel;
		}
		else if( fVolume < 1000.0f*1000.0f*1000.0f )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_ARENA;
			reverbProperties.m_fReflectTime		= 0.01f;
			reverbProperties.m_fDecayTime		= 4.236f;
			reverbProperties.m_fVolume			= 0.1f * m_fReverbLevel;
		}
		else
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_MOUNTAINS;
			reverbProperties.m_fReflectTime		= 0.0f;
			reverbProperties.m_fDecayTime		= 3.0f;
			reverbProperties.m_fVolume			= 0.1f * m_fReverbLevel;
		}
	} 
	// Use room types that are enclosed rooms
	else
	{
		if( fVolume < 100.0f*100.0f*100.0f )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_STONEROOM;
			reverbProperties.m_fReflectTime		= 0.005f;
			reverbProperties.m_fDecayTime		= 0.6f;
			reverbProperties.m_fVolume			= 0.3f * m_fReverbLevel;
		}
		else if( fVolume < 500.0f*500.0f*500.0f )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_HALLWAY;
			reverbProperties.m_fReflectTime		= 0.01f;
			reverbProperties.m_fDecayTime		= 1.3f;
			reverbProperties.m_fVolume			= 0.1f * m_fReverbLevel;
		}
		else if( fVolume < 1500.0f*1500.0f*1500.0f )
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_CONCERTHALL;
			reverbProperties.m_fReflectTime		= 0.02f;
			reverbProperties.m_fDecayTime		= 2.2f;
			reverbProperties.m_fVolume			= 0.1f * m_fReverbLevel;
		}
		else
		{
			reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_AUDITORIUM;
			reverbProperties.m_fReflectTime		= 0.02f;
			reverbProperties.m_fDecayTime		= 3.5f;
			reverbProperties.m_fVolume			= 0.1f * m_fReverbLevel;
		}
	}

	// Override to water if in it.
	if( IsLiquid( m_eCurContainerCode ))
		reverbProperties.m_dwAcoustics = REVERB_ACOUSTICS_UNDERWATER;

	reverbProperties.m_dwParams = REVERBPARAM_VOLUME | REVERBPARAM_ACOUSTICS | REVERBPARAM_REFLECTTIME | REVERBPARAM_DECAYTIME;
	// If in mech, then lengthen the decaytime
	if( !((m_nPlayerMode == PM_MODE_FOOT) || (m_nPlayerMode == PM_MODE_KID)))
		reverbProperties.m_fDecayTime *= 2;
	g_pLTClient->SetReverbProperties( &reverbProperties );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateCamera()
//
//	PURPOSE:	Update the camera position/rotation
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateCamera()
{
	if (!g_pLTClient || !m_hCamera) return;


	// Update the camera's position and rotation..

	UpdateAlternativeCamera();


	// Update the player camera...

	if (!UpdatePlayerCamera()) return;


	// This is sort of a kludge.  Basically if this is the first update(s), make
	// sure the camera is positioned correctly before actually rendering the
	// scene.
	
	if (!m_bHandledStartup)
	{
		ShowPlayer(LTFALSE);
	}


	if (!m_bUsingExternalCamera)
	{
		UpdateCameraPosition();
	}

	CalculateCameraRotation();
	UpdateCameraRotation();


	// If we're dead or dying, stop zooming...

	if (m_bUsingExternalCamera || IsPlayerDead())
	{
		m_bZoomView = LTFALSE;
	}

	// Update zoom if applicable...

	if (m_bZooming || m_bZoomView != m_bOldZoomView)
	{
		UpdateCameraZoom();
	}

	m_bOldZoomView = m_bZoomView;


	// Update shake if applicable...

	UpdateCameraShake();



	
	// Make sure the player gets updated

	UpdatePlayerInfo();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateMoviesState()
//
//	PURPOSE:	Update movies
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateMoviesState()
{
	if (!g_pLTClient) return;

	// If we're playing movies, see if the current one is finished...
	
	g_pLTClient->UpdateVideo();
	
	if (g_pLTClient->IsVideoPlaying() != VIDEO_PLAYING)
	{
		PlayIntroMovies (g_pLTClient);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateCreditsState()
//
//	PURPOSE:	Update credits
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateCreditsState()
{
	if (!g_pLTClient) return;

	int nMode = CM_CREDITS;

#ifdef _DEMO
	nMode = CM_DEMO_INFO;
#endif

	if (!m_credits.IsInited() && !m_credits.Init (g_pLTClient, this, nMode, LTFALSE))
	{
		m_nGameState = GS_MENU;
		SetMenuMusic(LTTRUE);
	}
	else
	{
		g_pLTClient->Start3D();
		UpdateMenuPolygrid();
		m_credits.Update();
		g_pLTClient->End3D();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateGameOver()
//
//	PURPOSE:	Update game over
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateGameOver()
{
	if (!g_pLTClient) return;	

	int nMode = CM_CREDITS;
#ifdef _DEMO
	nMode = CM_DEMO_INFO;
#endif

	if (!m_credits.IsInited() && !m_credits.Init(g_pLTClient, this, nMode, LTFALSE))
	{
		return;
	}
	else
	{
		m_credits.Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateIntroState()
//
//	PURPOSE:	Update intro
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateIntroState()
{
	if (!g_pLTClient) return;

	int nMode = CM_INTRO;
#ifdef _DEMO
	nMode = CM_DEMO_INTRO;
#endif

	if (!m_credits.IsInited() && !m_credits.Init(g_pLTClient, this, nMode, LTFALSE))
	{
		m_nGameState = GS_MENU;
		SetMenuMusic(LTTRUE);
	}
	else
	{
		g_pLTClient->Start3D();
		UpdateMenuPolygrid();
		m_credits.Update();
		g_pLTClient->End3D();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateMenuState()
//
//	PURPOSE:	Update menu state
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateMenuState()
{
	if (!g_pLTClient) return;

	g_pLTClient->Start3D();

	UpdateMenuPolygrid();

	g_pLTClient->StartOptimized2D();
	m_menu.Draw();
	g_pLTClient->EndOptimized2D();

	g_pLTClient->End3D();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateBumperState()
//
//	PURPOSE:	Update bumper screen state
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateBumperState()
{
	if (!g_pLTClient) return;

	// Remove the loading text and play a sound so that they know they can
	// start the level...

	if (m_bLoadingWorld)
	{
		g_pLTClient->ClearInput();

		if (m_pPressAnyKeySound)
		{
			PlaySoundLocal(m_pPressAnyKeySound, SOUNDPRIORITY_PLAYER_HIGH);
		}
	}

	// World is done loading if we got called...

	m_bLoadingWorld = LTFALSE;

	g_pLTClient->Start3D();

	UpdateMenuPolygrid();

	g_pLTClient->StartOptimized2D();

	UpdateLoadingLevel();

	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nWidth, nHeight;
	g_pLTClient->GetSurfaceDims (hScreen, &nWidth, &nHeight);
	g_pLTClient->DrawSurfaceToSurfaceTransparent (hScreen, m_hPressAnyKey, LTNULL, ((int)nWidth - (int)m_cxPressAnyKey) / 2, (int)nHeight - (int)m_cyPressAnyKey, LTNULL);

	g_pLTClient->EndOptimized2D();
	g_pLTClient->End3D();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateLoadingLevelState()
//
//	PURPOSE:	Update loading level state
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateLoadingLevelState()
{
	if (!g_pLTClient) return;

	g_pLTClient->Start3D();

	UpdateMenuPolygrid();

	g_pLTClient->StartOptimized2D();

	UpdateLoadingLevel();

	g_pLTClient->EndOptimized2D();
	g_pLTClient->End3D();

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_bInWorld && hPlayerObj)
	{
		m_nGameState = GS_PLAYING;
		PauseGame(LTFALSE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdatePausedState()
//
//	PURPOSE:	Update paused state
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdatePausedState()
{
	if (!g_pLTClient) return;

	g_pLTClient->Start3D();

	UpdateMenuPolygrid();

	g_pLTClient->StartOptimized2D();
	if (m_hGamePausedSurface)
	{
		uint32 nSurfaceWidth = 0, nSurfaceHeight = 0;
		uint32 nScreenWidth, nScreenHeight;

		HSURFACE hScreen = g_pLTClient->GetScreenSurface();
		g_pLTClient->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);
		g_pLTClient->GetSurfaceDims(m_hGamePausedSurface, &nSurfaceWidth, &nSurfaceHeight);

		g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hGamePausedSurface, LTNULL, 
												   ((int)(nScreenWidth - nSurfaceWidth)) / 2, 
												   (((int)(nScreenHeight - nSurfaceHeight)) / 2) + 70, LTNULL);
	}
	g_pLTClient->EndOptimized2D();

	g_pLTClient->End3D();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateDemoMultiplayerState()
//
//	PURPOSE:	Update demo multiplayer state
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateDemoMultiplayerState()
{
	if (!g_pLTClient) return;

	if (!m_credits.IsInited() && !m_credits.Init (g_pLTClient, this, CM_DEMO_MULTI, LTFALSE))
	{
		m_nGameState = GS_MENU;
		SetMenuMusic(LTTRUE);
	}
	else
	{
		g_pLTClient->Start3D();
		UpdateMenuPolygrid();
		m_credits.Update();
		g_pLTClient->End3D();
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdatePlayerInfo()
//
//	PURPOSE:	Tell the player about the new camera stuff
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdatePlayerInfo()
{
	if (!g_pLTClient || !m_hCamera) return;

	LTFLOAT sendRate;

	if (m_bAllowPlayerMovement != m_bLastAllowPlayerMovement)
	{
		SetInputState(m_bAllowPlayerMovement);
	}

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;
	
	// Send the rotation and the current weapon flash/model position to the 
	// server to update the player.  The flash and model position are 
	// relative to the player position...

	LTVector vFlashPos, vModelPos, vPlayerPos, vCameraPos;
	VEC_INIT(vFlashPos);
	VEC_INIT(vModelPos);
	VEC_INIT(vCameraPos);

	g_pLTClient->GetObjectPos(hPlayerObj, &vPlayerPos);

	VEC_SUB(vFlashPos, m_weaponModel.GetFlashPos(), vPlayerPos);
	VEC_SUB(vModelPos, m_weaponModel.GetModelPos(), vPlayerPos);


	LTRotation rPlayerRot;
	uint8 nCode	= (uint8)m_eCurContainerCode;

	m_nPlayerInfoChangeFlags |= CLIENTUPDATE_PLAYERROT;
	m_nPlayerInfoChangeFlags |= CLIENTUPDATE_WEAPONROT;

	// Set the player's rotation (don't allow model to rotate up/down).

	g_pLTClient->Math()->SetupEuler(rPlayerRot, 0.0f, m_fYaw, m_fCamCant);

	if ( m_playerCamera.IsChaseView() != m_bLastSent3rdPerson )
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_3RDPERSON;
		m_bLastSent3rdPerson = m_playerCamera.IsChaseView();
		if ( m_playerCamera.IsChaseView() )
			m_nPlayerInfoChangeFlags |= CLIENTUPDATE_3RDPERVAL;
	}
	if ( !m_bHandledStartup || m_bAllowPlayerMovement != m_bLastAllowPlayerMovement )
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_ALLOWINPUT;
	}

	if (m_bUsingExternalCamera)
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_EXTERNALCAMERA;
		g_pLTClient->GetObjectPos(m_hCamera, &vCameraPos);
	}

	if ( m_nPlayerInfoChangeFlags )
	{
		// Always send CLIENTUPDATE_ALLOWINPUT changes guaranteed.
		if(m_nPlayerInfoChangeFlags & CLIENTUPDATE_ALLOWINPUT)
		{
			ILTMessage_Write* hMessage = pClientDE->StartMessage(MID_PLAYER_UPDATE);
			pClientDE->WriteToMessageWord(hMessage, CLIENTUPDATE_ALLOWINPUT);
			pClientDE->WriteToMessageByte(hMessage, (uint8)m_bAllowPlayerMovement);
			pClientDE->EndMessage(hMessage);
			m_nPlayerInfoChangeFlags &= ~CLIENTUPDATE_ALLOWINPUT;
		}
		
		sendRate = 1.0f / g_CV_CSendRate.GetLTFLOAT(DEFAULT_CSENDRATE);

		if(!IsMultiplayerGame() || 
			(g_pLTClient->GetTime() - m_fPlayerInfoLastSendTime) > sendRate)
		{
			ILTMessage_Write* hMessage = g_pLTClient->StartMessage(MID_PLAYER_UPDATE);

			// Write rotation info.			
			g_pLTClient->WriteToMessageWord(hMessage, m_nPlayerInfoChangeFlags);
			if ( m_nPlayerInfoChangeFlags & CLIENTUPDATE_PLAYERROT )
			{
				//pClientDE->WriteToMessageRotation(hMessage, &rPlayerRot);
				g_pLTClient->WriteToMessageByte(hMessage, CompressRotationByte(&rPlayerRot));
			}
			
			if ( m_nPlayerInfoChangeFlags & CLIENTUPDATE_WEAPONROT )
				g_pLTClient->WriteToMessageRotation(hMessage, &m_rRotation);
			
			if ( m_nPlayerInfoChangeFlags & CLIENTUPDATE_EXTERNALCAMERA )
				g_pLTClient->WriteToMessageVector(hMessage, &vCameraPos);
			

			// Write position info.
			m_MoveMgr->WritePositionInfo(hMessage);

			g_pLTClient->EndMessage2(hMessage, 0); // Send unguaranteed.
			
			m_fLastSentYaw	= m_fYaw;
			m_fLastSentCamCant = m_fCamCant;
			m_fPlayerInfoLastSendTime = pClientDE->GetTime();
			m_nPlayerInfoChangeFlags = 0;
		}
	}

	m_bHandledStartup = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::StartLevel()
//
//	PURPOSE:	Tell the player to start the level
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::StartLevel()
{
	if (!g_pLTClient || IsMultiplayerGame()) return;

	ILTMessage_Write* hMessage = g_pLTClient->StartMessage(MID_SINGLEPLAYER_START);
	g_pLTClient->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdatePlayerCamera()
//
//	PURPOSE:	Update the player camera
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::UpdatePlayerCamera()
{
	if (!g_pLTClient || !m_hCamera) return LTFALSE;

	// Make sure our player camera is attached...

	m_playerCamera.AttachToObject(m_MoveMgr->GetObject());


	if (m_playerCamera.IsChaseView())
	{
		Update3rdPersonInfo();
	}

	
	// Update our camera position based on the player camera...

	m_playerCamera.CameraUpdate(g_pLTClient->GetFrameTime());


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::SetExternalCamera()
//
//	PURPOSE:	Turn on/off external camera mode
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::SetExternalCamera(LTBOOL bExternal)
{
	if (!g_pLTClient || !m_hCamera) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;
	
	if (bExternal && m_playerCamera.IsFirstPerson())
	{
		m_weaponModel.SetVisible(LTFALSE);

		ShowPlayer(LTTRUE);
		m_playerCamera.GoChaseMode();
		m_playerCamera.CameraUpdate(0.0f);

		m_bZoomView = LTFALSE;			 // Can't zoom in 3rd person...
		m_stats.EnableCrosshair(LTFALSE); // Disable cross hair in 3rd person...
	}
	else if (!bExternal && !m_playerCamera.IsFirstPerson()) // Go Internal
	{
		if (!pSettings->VehicleMode())
		{
			m_weaponModel.SetVisible(LTTRUE);
			ShowPlayer(LTFALSE);
		}

		m_playerCamera.GoFirstPerson();
		m_playerCamera.CameraUpdate(0.0f);

		if (m_nPlayerMode != PM_MODE_KID)
		{
			m_stats.EnableCrosshair(LTTRUE); // Enable cross hair in 1st person...
		}

		if (m_h3rdPersonCrosshair)
		{
			g_pLTClient->RemoveObject(m_h3rdPersonCrosshair);
			m_h3rdPersonCrosshair = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateAlternativeCamera()
//
//	PURPOSE:	Update the camera using an alternative camera
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::UpdateAlternativeCamera()
{
	if (!g_pLTClient || !m_hCamera) return LTFALSE;

	m_bLastAllowPlayerMovement = m_bAllowPlayerMovement;
	m_bAllowPlayerMovement	   = LTTRUE;

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
				g_pLTClient->Common()->GetObjectFlags(hObj, OFT_User, dwUsrFlags);
				
				if (dwUsrFlags & USRFLG_CAMERA_LIVE)
				{
					m_bUsingExternalCamera = LTTRUE;
					m_bDrawInterface	   = LTFALSE;

					SetExternalCamera(LTTRUE);

					m_bAllowPlayerMovement = pCamFX->AllowPlayerMovement();

					LTVector vPos;
					g_pLTClient->GetObjectPos(hObj, &vPos);
					g_pLTClient->SetObjectPos(m_hCamera, &vPos);
					m_bCameraPosInited = LTTRUE;

					LTRotation rRot;
					g_pLTClient->GetObjectRotation(hObj, &rRot);
					g_pLTClient->SetObjectRotation(m_hCamera, &rRot);	
					
					if (pCamFX->IsListener())
					{
						g_pLTClient->SetListener(LTFALSE, &vPos, &rRot);
					}

					// Set to movie camera rect, if not already set...

					if (!m_bMovieCameraRect)
					{
						// Make sure we clear whatever was on the screen before
						// we switch to this camera...

						ClearAllScreenBuffers();

						m_bMovieCameraRect = LTTRUE;

						uint32 dwWidth = 640, dwHeight = 480;
						g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

						bool bFullScreen;
						g_pLTClient->GetCameraRect(m_hCamera, &bFullScreen, &m_nOldCameraLeft, &m_nOldCameraTop, 
												 &m_nOldCameraRight, &m_nOldCameraBottom);

						// Determine how to adjust camera rect/fov...

						uint8 nCamType = pCamFX->GetType();
						LTFLOAT fVal1 = (nCamType == CT_CINEMATIC) ? 1.0f/6.0f : 0.0f;
						LTFLOAT fVal2 = (nCamType == CT_CINEMATIC) ? 2.0f/3.0f : 1.0f;

						int nOffset = int(dwHeight * fVal1);
						int nBottom = dwHeight - nOffset;
						
						// {MD 9/11/98} Force fullscreen to false otherwise the movie
						// won't be letterbox.
						bFullScreen = false;
						
						g_pLTClient->SetCameraRect(m_hCamera, bFullScreen, 0, nOffset, dwWidth, nBottom);

						LTFLOAT y, x;
						g_pLTClient->GetCameraFOV(m_hCamera, &x, &y);
						y = (x * dwHeight * fVal2) / dwWidth;

						g_pLTClient->SetCameraFOV(m_hCamera, x, y);
					}

					return LTTRUE;
				}
			}
		}
	}


	// Okay, we're no longer using an external camera...

	if (m_bUsingExternalCamera)
	{
		TurnOffAlternativeCamera();
	}


	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::TurnOffAlternativeCamera()
//
//	PURPOSE:	Turn off the alternative camera mode
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::TurnOffAlternativeCamera()
{
	if (!g_pLTClient || !m_hCamera) return;

	m_bUsingExternalCamera = LTFALSE;
	m_bDrawInterface	   = LTTRUE;

	// Set the listener back to the client...

	g_pLTClient->SetListener(LTTRUE, LTNULL, LTNULL);
	
	
	// Force 1st person...

	SetExternalCamera(LTFALSE); 


	// Set the camera back like it was...

	g_pLTClient->SetCameraRect(m_hCamera, LTFALSE, m_nOldCameraLeft, m_nOldCameraTop, 
							 m_nOldCameraRight, m_nOldCameraBottom);
	m_bMovieCameraRect = LTFALSE;

	uint32 dwHeight = m_nOldCameraBottom - m_nOldCameraTop;
	uint32 dwWidth  = m_nOldCameraRight - m_nOldCameraLeft;
	LTFLOAT y, x;

	pClientDE->GetCameraFOV(m_hCamera, &x, &y);
	y = (x * dwHeight) / dwWidth;

	pClientDE->SetCameraFOV(m_hCamera, x, y);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateCameraPosition()
//
//	PURPOSE:	Update the camera position
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateCameraPosition()
{
	if (!g_pLTClient || !m_hCamera) return;

	LTVector vPos;
	VEC_COPY(vPos, m_playerCamera.GetPos());

	if (m_playerCamera.IsFirstPerson())
	{
		vPos.y += m_fBobHeight + m_fCamDuck;

		LTVector vU, vR, vF;
		g_pLTClient->Math()->GetRotationVectors(m_rRotation, vU, vR, vF);

		VEC_MULSCALAR(vR, vR, m_fBobWidth)
		VEC_ADD(vPos, vPos, vR)
	}
	else
	{
		m_rRotation = m_playerCamera.GetRotation();
	}

	g_pLTClient->SetObjectPos(m_hCamera, &vPos);
	m_bCameraPosInited = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::CalculateCameraRotation()
//
//	PURPOSE:	Calculate the new camera rotation
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::CalculateCameraRotation()
{
	if (!g_pLTClient || !m_hCamera) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;

	LTFLOAT fVal = m_bZoomView ? 10.0f : 1.0f;

	// Get axis offsets...

	LTFLOAT offsets[3];
	g_pLTClient->GetAxisOffsets(offsets);

	// update offsets with values from console variables

	HCONSOLEVAR hVar;
	LTFLOAT fAxisForwardBackward = 0.0f;
	LTFLOAT fAxisLeftRight = 0.0f;
	LTFLOAT fAxisYaw = 0.0f;
	LTFLOAT fAxisPitch = 0.0f;
	LTBOOL bUseAxisForwardBackward = FALSE;
	LTBOOL bUseAxisLeftRight = FALSE;
	LTBOOL bFixedAxisPitch = FALSE;
	LTFLOAT fAxisYawDeadZone = 0.10f;
	LTFLOAT fAxisPitchDeadZone = 0.10f;
	LTFLOAT fAxisForwardBackwardDeadZone = 0.10f;
	LTFLOAT fAxisLeftRightDeadZone = 0.10f;

	if (IsJoystickEnabled())
	{
		hVar = g_pLTClient->GetConsoleVar( "AxisYawDeadZone");
		if (hVar != NULL) fAxisYawDeadZone = g_pLTClient->GetVarValueFloat(hVar);
		hVar = g_pLTClient->GetConsoleVar( "AxisPitchDeadZone");
		if (hVar != NULL) fAxisPitchDeadZone = g_pLTClient->GetVarValueFloat(hVar);
		hVar = g_pLTClient->GetConsoleVar( "AxisForwardBackwardDeadZone");
		if (hVar != NULL) fAxisForwardBackwardDeadZone = g_pLTClient->GetVarValueFloat(hVar);
		hVar = g_pLTClient->GetConsoleVar( "AxisLeftRightDeadZone");
		if (hVar != NULL) fAxisLeftRightDeadZone = g_pLTClient->GetVarValueFloat(hVar);

		hVar = g_pLTClient->GetConsoleVar( "AxisYaw");
		if (hVar != NULL)
		{
			fAxisYaw = g_pLTClient->GetVarValueFloat(hVar);
			if ((fAxisYaw > fAxisYawDeadZone) || ((fAxisYaw < -fAxisYawDeadZone))) offsets[0] += fAxisYaw;
		}
		hVar = g_pLTClient->GetConsoleVar( "AxisPitch");
		if (hVar != NULL) 
		{
			fAxisPitch = g_pLTClient->GetVarValueFloat(hVar);
			if ((fAxisPitch > fAxisPitchDeadZone) || (fAxisPitch < -fAxisPitchDeadZone)) offsets[1] += fAxisPitch;
		}
		hVar = g_pLTClient->GetConsoleVar( "AxisLeftRight");
		if (hVar != NULL) 
		{
			fAxisLeftRight = g_pLTClient->GetVarValueFloat(hVar);
			bUseAxisLeftRight = TRUE;
		}
		hVar = g_pLTClient->GetConsoleVar( "AxisForwardBackward");
		if (hVar != NULL)
		{
			fAxisForwardBackward = g_pLTClient->GetVarValueFloat(hVar);
			bUseAxisForwardBackward = TRUE;
		}

		// force pitch axis if we are using a joystick and the FixedAxisPitch variable is 1
		hVar = g_pLTClient->GetConsoleVar( "FixedAxisPitch");
		if (hVar != NULL) if (g_pLTClient->GetVarValueFloat(hVar) == 1) 
		{
			m_fPitch = fAxisPitch;
			bFixedAxisPitch = TRUE;
		}
	}

	m_MoveMgr->UpdateAxisMovement(bUseAxisForwardBackward, fAxisForwardBackward, fAxisForwardBackwardDeadZone, bUseAxisLeftRight, fAxisLeftRight, fAxisLeftRightDeadZone);


	if (m_bRestoreOrientation)
	{
		m_fYaw   = m_fYawBackup;
		m_fPitch = m_fPitchBackup;
		memset (offsets, 0, sizeof(LTFLOAT) * 3);

		m_bRestoreOrientation = LTFALSE;
	}

	if (m_bStrafing)
	{
		m_MoveMgr->UpdateMouseStrafeFlags(offsets);
	}

	m_fYaw += offsets[0] / fVal;

	// get the turning speed
	
	LTFLOAT nNormalTurnSpeed = DEFAULT_NORMAL_TURN_SPEED;
	LTFLOAT nFastTurnSpeed = DEFAULT_FAST_TURN_SPEED;
	hVar = g_pLTClient->GetConsoleVar (NORMAL_TURN_RATE_VAR);
	if (hVar)
	{
		nNormalTurnSpeed = g_pLTClient->GetVarValueFloat (hVar);
	}
	hVar = g_pLTClient->GetConsoleVar (FAST_TURN_RATE_VAR);
	if (hVar)
	{
		nFastTurnSpeed = g_pLTClient->GetVarValueFloat (hVar);
	}

	if (!(m_dwPlayerFlags & CS_MFLG_STRAFE) && (m_dwPlayerFlags & CS_MFLG_LEFT))
	{
		m_fYaw -= g_pLTClient->GetFrameTime() * ((m_dwPlayerFlags & CS_MFLG_RUN) ? nFastTurnSpeed : nNormalTurnSpeed);
	}

	if (!(m_dwPlayerFlags & CS_MFLG_STRAFE) && (m_dwPlayerFlags & CS_MFLG_RIGHT))
	{
		m_fYaw += g_pLTClient->GetFrameTime() * ((m_dwPlayerFlags & CS_MFLG_RUN) ? nFastTurnSpeed : nNormalTurnSpeed);
	}

	
	if (pSettings->MouseLook() || (m_dwPlayerFlags & CS_MFLG_LOOKUP) || (m_dwPlayerFlags & CS_MFLG_LOOKDOWN) || m_bHoldingMouseLook)
	{
		if (pSettings->MouseLook() || m_bHoldingMouseLook)
		{
			if (pSettings->MouseInvertY())
			{
				m_fPitch -= offsets[1] / fVal;
			}
			else
			{
				m_fPitch += offsets[1] / fVal;
			}
		}

		if (m_dwPlayerFlags & CS_MFLG_LOOKUP)
		{
			m_fPitch -= 0.075f;
		}

		if (m_dwPlayerFlags & CS_MFLG_LOOKDOWN)
		{
			m_fPitch += 0.075f;
		}

		// Don't allow much movement up/down if 3rd person...

		if (!m_playerCamera.IsFirstPerson())
		{
			LTFLOAT fMinY = DEG2RAD(45.0f) - 0.1f;

			if (m_fPitch < -fMinY) m_fPitch = -fMinY;
			if (m_fPitch > fMinY)  m_fPitch = fMinY;
		}
	}
	else if (m_fPitch != 0.0f && pSettings->Lookspring())
	{
		if (m_fPitch > 0.0f) m_fPitch -= min (0.075f, m_fPitch);
		if (m_fPitch < 0.0f) m_fPitch += min (0.075f, -m_fPitch);
	}

	LTFLOAT fMinY = MATH_HALFPI - 0.1f;

	if (m_fPitch < -fMinY) m_fPitch = -fMinY;
	if (m_fPitch > fMinY)  m_fPitch = fMinY;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateCameraRotation()
//
//	PURPOSE:	Set the new camera rotation
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::UpdateCameraRotation()
{
	if (!g_pLTClient || !m_hCamera) return LTFALSE;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return LTFALSE;

	if (m_bUsingExternalCamera)
	{
		// Just calculate the correct player rotation...

		g_pLTClient->Math()->SetupEuler(m_rRotation, m_fPitch, m_fYaw, m_fCamCant);
	}
	else if (m_playerCamera.IsFirstPerson())
	{
		g_pLTClient->Math()->SetupEuler(m_rRotation, m_fPitch, m_fYaw, m_fCamCant);
		g_pLTClient->SetObjectRotation(m_hCamera, &m_rRotation);
	}
	else
	{
		// Set the camera to use the rotation calculated by the player camera,
		// however we still need to calculate the correct rotation to be sent
		// to the player...

		g_pLTClient->Math()->EulerRotateX(m_rRotation, m_fPitch);
		g_pLTClient->SetObjectRotation(m_hCamera, &m_rRotation);

		// Okay, now calculate the correct player rotation...

		g_pLTClient->Math()->SetupEuler(m_rRotation, m_fPitch, m_fYaw, m_fCamCant);
	}

	return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateCameraZoom
//
//	PURPOSE:	Update the camera's field of view
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateCameraZoom()
{
	if (!g_pLTClient || !m_hCamera) return;

	LTFLOAT zoomSpeed = 4 * pClientDE->GetFrameTime();

	char strConsole[30];
	LTFLOAT fovX, fovY, oldFovX;
	uint32 dwWidth = 640, dwHeight = 480;
	g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);
	g_pLTClient->GetCameraFOV(m_hCamera, &fovX, &fovY);

	oldFovX = fovX;

	if (!fovX)
	{
		fovX = m_fCurrentFovX;
	}

	m_bZooming = LTTRUE;
	
	LTBOOL bZoomingIn = LTTRUE;

	// Need to zoom camera
	if (m_bZoomView && fovX > DEG2RAD(FOV_ZOOMED))
	{
		fovX -= zoomSpeed;
		
		if (fovX < DEG2RAD(FOV_ZOOMED)) 
		{
			fovX = DEG2RAD(FOV_ZOOMED);
			CSPrint ("Zoom mode on");
			m_bZooming = LTFALSE;

			// Set the lod scale to max value

			sprintf(strConsole, "+LODScale %f", m_fSaveLODScale * LODSCALE_MULTIPLIER);
			g_pLTClient->RunConsoleString(strConsole);
		}
	}
	else if (!m_bZoomView && fovX < m_fCurrentFovX)
	{
		bZoomingIn = LTFALSE;

		fovX += zoomSpeed;

		if (fovX > m_fCurrentFovX) 
		{
			fovX = m_fCurrentFovX;
			CSPrint ("Zoom mode off");
			m_bZooming = LTFALSE;

			// Set the lod scale for models back to saved value...

			sprintf(strConsole, "+LODScale %f", m_fSaveLODScale);
			g_pLTClient->RunConsoleString(strConsole);
		}
	}

	if (oldFovX != fovX && dwWidth && dwHeight)
	{
		fovY = (fovX * dwHeight) / dwWidth;

		g_pLTClient->SetCameraFOV(m_hCamera, fovX, fovY);

		// Update the lod scale for models...

		LTFLOAT fVal1 = m_fCurrentFovX - fovX;
		LTFLOAT fVal2 = (m_fSaveLODScale * LODSCALE_MULTIPLIER) - m_fSaveLODScale;
		LTFLOAT fVal3 = m_fCurrentFovX - DEG2RAD(FOV_ZOOMED);

		LTFLOAT fNewLODScale = fVal3 > 0.0f ? (m_fSaveLODScale + (fVal1 * fVal2) / fVal3) : (m_fSaveLODScale * LODSCALE_MULTIPLIER);

		sprintf(strConsole, "+LODScale %f", fNewLODScale);
		g_pLTClient->RunConsoleString(strConsole);

		//pClientDE->CPrint("Current FOV (%f, %f)", fovX, fovY);
		//pClientDE->CPrint("Current Zoom LODScale: %f", fNewLODScale);
	}

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateCameraShake
//
//	PURPOSE:	Update the camera's shake
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateCameraShake()
{
	if (!g_pLTClient || !m_hCamera) return;

	// Decay...

	LTFLOAT fDecayAmount = 2.0f * pClientDE->GetFrameTime();

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

	LTVector vPos, vAdd;
	VEC_SET(vAdd, faddX, faddY, faddZ);

	g_pLTClient->GetObjectPos(m_hCamera, &vPos);
	VEC_ADD(vPos, vPos, vAdd);

	g_pLTClient->SetObjectPos(m_hCamera, &vPos);

	HLOCALOBJ hWeapon = m_weaponModel.GetHandle();
	if (!hWeapon) return;

	g_pLTClient->GetObjectPos(hWeapon, &vPos);

	VEC_MULSCALAR(vAdd, vAdd, 0.95f);
	VEC_ADD(vPos, vPos, vAdd);
	g_pLTClient->SetObjectPos(hWeapon, &vPos);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateScreenTint
//
//	PURPOSE:	Update the screen tint
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateScreenTint()
{
	if (!g_pLTClient || !m_hCamera || !m_bTintScreen) return;

	LTVector vLightAdd, vCurLightAdd;
	VEC_SET(vLightAdd, 0.0f, 0.0f, 0.0f);

	g_pLTClient->GetCameraLightAdd(m_hCamera, &vCurLightAdd);

	LTFLOAT fTime  = g_pLTClient->GetTime();
	if ((m_fTintRampUp > 0.0f) && (fTime < m_fTintStart + m_fTintRampUp))
	{
		LTFLOAT fDelta = (fTime - m_fTintStart);
		vLightAdd.x = fDelta * (m_vTintColor.x) / m_fTintRampUp;
		vLightAdd.y = fDelta * (m_vTintColor.y) / m_fTintRampUp;
		vLightAdd.z = fDelta * (m_vTintColor.z) / m_fTintRampUp;
	}
	else if (fTime < m_fTintStart + m_fTintRampUp + m_fTintTime)
	{
		VEC_COPY(vLightAdd, m_vTintColor);
	}
	else if ((m_fTintRampDown > 0.0f) && (fTime < m_fTintStart + m_fTintRampUp + m_fTintTime + m_fTintRampDown))
	{
		LTFLOAT fDelta = (fTime - (m_fTintStart + m_fTintRampUp + m_fTintTime));

		vLightAdd.x = m_vTintColor.x - (fDelta * (m_vTintColor.x) / m_fTintRampUp);
		vLightAdd.y = m_vTintColor.y - (fDelta * (m_vTintColor.y) / m_fTintRampUp);
		vLightAdd.z = m_vTintColor.z - (fDelta * (m_vTintColor.z) / m_fTintRampUp);
	}
	else
	{
		m_bTintScreen = LTFALSE;
	}

	// Make sure values are in range...

	vLightAdd.x = (vLightAdd.x < 0.0f ? 0.0f : (vLightAdd.x > 1.0f ? 1.0f : vLightAdd.x));
	vLightAdd.y = (vLightAdd.y < 0.0f ? 0.0f : (vLightAdd.y > 1.0f ? 1.0f : vLightAdd.y));
	vLightAdd.z = (vLightAdd.z < 0.0f ? 0.0f : (vLightAdd.z > 1.0f ? 1.0f : vLightAdd.z));

	g_pLTClient->SetCameraLightAdd(m_hCamera, &vLightAdd);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ClearScreenTint
//
//	PURPOSE:	Clear any tint on the screen
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::ClearScreenTint()
{
	if (!g_pLTClient) return;

	if (m_bTintScreen)
	{
		m_bTintScreen = LTFALSE;
		LTVector vLightAdd;
		VEC_INIT(vLightAdd);
		g_pLTClient->SetCameraLightAdd(m_hCamera, &vLightAdd);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateWeaponModel()
//
//	PURPOSE:	Update the weapon model
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateWeaponModel()
{
	if (!g_pLTClient || !m_MoveMgr) return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings || pSettings->VehicleMode()) return;


	// Decay firing jitter if necessary...

	if (m_fFireJitterPitch > 0.0f)
	{
		LTFLOAT fVal = (pClientDE->GetFrameTime() * FIRE_JITTER_DECAY_DELTA);

		if (m_fFireJitterPitch - fVal < 0.0f)
		{
			fVal = m_fFireJitterPitch;
		}
		
		m_fFireJitterPitch -= fVal;
		m_fPitch += fVal;
	}


	// If possible, get these values from the camera, because it 
	// is more up-to-date...
	
	HLOCALOBJ hObj = hPlayerObj; // m_MoveMgr->GetObject();
	if (m_playerCamera.IsFirstPerson() && !m_bUsingExternalCamera)
	{
		hObj = m_hCamera;
	}

	LTRotation rRot;
	LTVector vPos;
	g_pLTClient->GetObjectPos(hObj, &vPos);
	g_pLTClient->GetObjectRotation(m_hCamera, &rRot);


	// If we aren't dead, and we aren't in the middle of changing weapons,
	// let us fire.
	
	LTBOOL bFire = LTFALSE;

	if ((m_dwPlayerFlags & CS_MFLG_FIRING) && (m_ePlayerState != PS_DEAD) && !m_bSpectatorMode)
	{
		bFire = LTTRUE;
	}


	// Babies don't get to use their weapon...

	uint32 dwUsrFlags;
	g_pLTClient->Common()->GetObjectFlags(hPlayerObj, OFT_User, dwUsrFlags);
	if (dwUsrFlags & USRFLG_PLAYER_TEARS)
	{
		bFire = LTFALSE;
	}


	// Update the model position and state...

	WeaponState eWeaponState = m_weaponModel.UpdateWeaponModel(rRot, vPos, bFire);


	// Set the correct flags...

	HLOCALOBJ hWeaponModel = m_weaponModel.GetHandle();
	if (m_bHaveStealth && !g_pLTClient->IsCommandOn(COMMAND_ID_FIRING))
	{
		LTFLOAT r, g, b, a;
		g_pLTClient->GetObjectColor(hWeaponModel, &r, &g, &b, &a);
		g_pLTClient->SetObjectColor(hWeaponModel, r, g, b, 0.6f);
	}
	else
	{
		LTFLOAT r, g, b, a;
		g_pLTClient->GetObjectColor(hWeaponModel, &r, &g, &b, &a);
		g_pLTClient->SetObjectColor(hWeaponModel, r, g, b, 1.0f);
	}


	// Do fire camera jitter...

	if (FiredWeapon(eWeaponState) && m_playerCamera.IsFirstPerson())
	{
		LTVector vShake;
		VEC_SET(vShake, 0.1f, 0.1f, 0.1f);
		ShakeScreen(vShake);

		// Move view up a bit...

		LTFLOAT fVal = (LTFLOAT)DEG2RAD(FIRE_JITTER_MAX_PITCH_DELTA) - m_fFireJitterPitch;
		m_fFireJitterPitch += fVal;
		m_fPitch -= fVal;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::Update3rdPersonCrossHair()
//
//	PURPOSE:	Update the 3rd person crosshair pos
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::Update3rdPersonCrossHair(LTFLOAT fDistance)
{
	if (!m_MoveMgr || !g_pLTClient || !m_playerCamera.IsChaseView()) return;

	//HLOCALOBJ hPlayerObj = pClientDE->GetClientObject();
	HLOCALOBJ hPlayerObj = m_MoveMgr->GetObject();
	if (!hPlayerObj) return;

	if (!m_h3rdPersonCrosshair)
	{
		// Create the 3rd person crosshair sprite...

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		theStruct.m_ObjectType = OT_SPRITE;
		SAFE_STRCPY(theStruct.m_Filename, "Sprites\\Crosshair.spr");
		theStruct.m_Flags = FLAG_VISIBLE | FLAG_GLOWSPRITE | FLAG_NOLIGHT;  
		m_h3rdPersonCrosshair = g_pLTClient->CreateObject(&theStruct);

		LTVector vScale;
		VEC_SET(vScale, .5f, .5f, 1.0f);
		g_pLTClient->SetObjectScale(m_h3rdPersonCrosshair, &vScale);		
	}

	if (fDistance < 1.0f || m_bUsingExternalCamera || !m_bCrosshairOn)
	{
		uint32 dwFlags;
		g_pLTClient->Common()->GetObjectFlags(m_h3rdPersonCrosshair, OFT_Flags, dwFlags);
		dwFlags &= ~FLAG_VISIBLE;
		g_pLTClient->Common()->SetObjectFlags(m_h3rdPersonCrosshair, OFT_Flags, dwFlags, FLAGMASK_ALL);
		return;
	}
	else
	{
		uint32 dwFlags;
		g_pLTClient->Common()->GetObjectFlags(m_h3rdPersonCrosshair, OFT_Flags, dwFlags);
		dwFlags |= FLAG_VISIBLE;
		g_pLTClient->Common()->SetObjectFlags(m_h3rdPersonCrosshair, OFT_Flags, dwFlags, FLAGMASK_ALL);
	}

	LTVector vU, vR, vF;
	g_pLTClient->Math()->GetRotationVectors(m_rRotation, vU, vR, vF);
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
//	ROUTINE:	CRiotClientShell::UpdateContainerFX
//
//	PURPOSE:	Update any client side container fx
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateContainerFX()
{
	if (!g_pLTClient || !m_hCamera) return;

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vPos);

	HLOCALOBJ objList[1];
	uint32 dwNum = g_pLTClient->GetPointContainers(&vPos, objList, 1);

	LTVector vScale, vLightAdd;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_SET(vLightAdd, 0.0f, 0.0f, 0.0f);
	LTBOOL bClearCurrentLightScale = LTFALSE;

	char* pCurSound      = LTNULL;
	ContainerCode eCode  = CC_NONE;
	uint32 dwUserFlags	 = FLAG_VISIBLE;
	m_bUseWorldFog = LTTRUE;

	// Get the user flags associated with the container, and make sure that
	// the container isn't hidden...

	if (dwNum > 0 && objList[0])
	{
		g_pLTClient->Common()->GetObjectFlags(objList[0], OFT_User, dwUserFlags);
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

				// See if this container has fog associated with it..

				CVolumeBrushFX* pFX = (CVolumeBrushFX*)m_sfxMgr.FindSpecialFX(SFX_VOLUMEBRUSH_ID, objList[0]);
				if (pFX)
				{
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
						pClientDE->RunConsoleString(buf);
					}
				}				
			}

			
			switch (eCode)
			{
				case CC_BLUE_WATER:
				{
					VEC_SET(vScale, 0.0f, 0.5f, 0.7f);
					pCurSound = "Sounds\\Player\\unwater2.wav";
				}
				break;
				case CC_DIRTY_WATER:
				{
					VEC_SET(vScale, 0.25f, 0.3f, 0.05f);
					pCurSound = "Sounds\\Player\\unwater2.wav";
				}
				break;
				case CC_CLEAR_WATER:
				{
					VEC_SET(vScale, 0.3f, 0.8f, 0.8f);
					pCurSound = "Sounds\\Player\\unwater2.wav";
				}
				break;
				case CC_CORROSIVE_FLUID:
				{
					VEC_SET(vScale, 0.3f, 0.4f, 0.0f);
					pCurSound = "Sounds\\Player\\unwater2.wav";
				}
				break;
				case CC_KATO:
				{
					VEC_SET(vScale, 0.5f, 0.15f, 0.0f);
					pCurSound = "Sounds\\Player\\unwater2.wav";
				}
				break;
				case CC_LIQUID_NITROGEN:
				{
					VEC_SET(vScale, 0.0f, 0.5f, 0.7f);
					VEC_SET(vLightAdd, .1f, .1f, .1f);
				}
				break;
				case CC_POISON_GAS:
				{
					VEC_SET(vScale, 1.0f, 1.0f, 0.3f);
				}
				break;
				case CC_SMOKE:
				{
					VEC_SET(vScale, 0.2f, 0.2f, 0.2f);
				}
				break;
				case CC_ELECTRICITY:
				{
					VEC_SET(vLightAdd, .1f, .1f, .1f);
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
				case CC_WIND:
				{
				}
				break;
				case CC_ZERO_GRAVITY:
				{
				}
				break;
				case CC_VACUUM:
				{
					VEC_SET(vScale, 0.8f, 0.8f, 0.8f);
				}
				break;
				case CC_LADDER:
				{
				}
				break;
				case CC_TOTAL_RED:
				{
					VEC_SET(vLightAdd, 1.0f, 0.0f, 0.0f);
					VEC_SET(vScale, 1.0f, 0.0f, 0.0f);
					pCurSound = "Sounds\\Voice\\totalred.wav";
				}
				break;
				case CC_TINT_SCREEN:
				{
					// Get the tint color out of the upper 3 bytes of the 
					// user flags...

					uint8 r = (uint8)(dwUserFlags>>24);
					uint8 g = (uint8)(dwUserFlags>>16);
					uint8 b = (uint8)(dwUserFlags>>8);

					VEC_SET(vScale, (LTFLOAT)r/255.0f, (LTFLOAT)g/255.0f, (LTFLOAT)b/255.0f);
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

		if (m_eCurContainerCode == CC_BLUE_WATER ||
			m_eCurContainerCode == CC_DIRTY_WATER ||
			m_eCurContainerCode == CC_CLEAR_WATER ||
			m_eCurContainerCode == CC_CORROSIVE_FLUID ||
			m_eCurContainerCode == CC_KATO ||
			m_eCurContainerCode == CC_LIQUID_NITROGEN ||
			m_eCurContainerCode == CC_POISON_GAS ||
			m_eCurContainerCode == CC_SMOKE ||
			m_eCurContainerCode == CC_ENDLESS_FALL ||
			m_eCurContainerCode == CC_VACUUM ||
			m_eCurContainerCode == CC_TOTAL_RED ||
			m_eCurContainerCode == CC_TINT_SCREEN)
		{
			bClearCurrentLightScale = LTTRUE;
		}

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
			m_LightScaleMgr.ClearLightScale (&m_vCurContainerLightScale, LightEffectEnvironment);
			VEC_SET (m_vCurContainerLightScale, -1.0f, -1.0f, -1.0f);
		}
		if (vScale.x != 1.0f || vScale.y != 1.0f || vScale.z != 1.0f)
		{
			m_LightScaleMgr.SetLightScale (&vScale, LightEffectEnvironment);
			VEC_COPY (m_vCurContainerLightScale, vScale);
		}

		// See if we are coming out of water...

		if (IsLiquid(m_eCurContainerCode) && !IsLiquid(eCode))
		{
			UpdateUnderWaterFX(LTFALSE);
			g_pLTClient->RunConsoleString("+ModelWarble 0");
		}

		m_eCurContainerCode = eCode;

		if (m_hContainerSound)
		{
			g_pLTClient->SoundMgr()->KillSound(m_hContainerSound);
			m_hContainerSound = LTNULL;
		}

		if (pCurSound)
		{
			PlaySoundInfo playSoundInfo;
			PLAYSOUNDINFO_INIT(playSoundInfo);

			playSoundInfo.m_dwFlags = PLAYSOUND_CLIENT | PLAYSOUND_LOCAL | PLAYSOUND_REVERB | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
			SAFE_STRCPY(playSoundInfo.m_szSoundName, pCurSound);
			HLTSOUND hSnd = LTNULL;
			g_pLTClient->SoundMgr()->PlaySound(&playSoundInfo, hSnd);
			m_hContainerSound = playSoundInfo.m_hSound;
		}

		if (!m_bHaveNightVision && !m_bHaveInfrared)
		{
			if (!m_bTintScreen)
			{
				g_pLTClient->SetCameraLightAdd(m_hCamera, &vLightAdd);
			}
		}
	}


	// See if we are under water (under any liquid)...

	if (IsLiquid(m_eCurContainerCode)) 
	{
		g_pLTClient->RunConsoleString("ModelWarble 1");
		UpdateUnderWaterFX();
	}
	else
	{
		// UpdateBreathingFX();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateUnderWaterFX
//
//	PURPOSE:	Update under water fx
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateUnderWaterFX(LTBOOL bUpdate)
{
	if (!g_pLTClient || !m_hCamera || m_bZoomView) return;

	uint32 dwWidth = 640, dwHeight = 480;
	g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	if (dwWidth < 0 || dwHeight < 0) return;


	// set under water flag...
	
	m_bUnderwater = bUpdate;

	// Initialize to default fov x and y...

	LTFLOAT fFovX = m_fCurrentFovX;
	LTFLOAT fFovY = (fFovX * dwHeight) / dwWidth;
	
	if (bUpdate)
	{
		g_pLTClient->GetCameraFOV(m_hCamera, &fFovX, &fFovY);

		LTFLOAT fSpeed = .02f * g_pLTClient->GetFrameTime();

		if (m_fFovXFXDir > 0)
		{
			fFovX -= fSpeed;
			fFovY += fSpeed;

			if (fFovY > (m_fCurrentFovX * dwHeight) / dwWidth)
			{
				fFovY = (m_fCurrentFovX * dwHeight) / dwWidth;
				m_fFovXFXDir = -m_fFovXFXDir;
			}
		}
		else
		{
			fFovX += fSpeed;
			fFovY -= fSpeed;

			if (fFovY < (m_fCurrentFovX * dwHeight - 40) / dwWidth)
			{
				fFovY = (m_fCurrentFovX * dwHeight - 40) / dwWidth;
				m_fFovXFXDir = -m_fFovXFXDir;
			}
		}
	}

	g_pLTClient->SetCameraFOV(m_hCamera, fFovX, fFovY);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateBreathingFX
//
//	PURPOSE:	Update breathing fx
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateBreathingFX(LTBOOL bUpdate)
{
	if (!g_pLTClient || !m_hCamera || m_bZoomView) return;

	uint32 dwWidth = 640, dwHeight = 480;
	g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	if (dwWidth < 0 || dwHeight < 0) return;

	// Initialize to default fov x and y...

	LTFLOAT fFovX = m_fCurrentFovX;
	LTFLOAT fFovY = (fFovX * dwHeight) / dwWidth;
	
	if (bUpdate)
	{
		g_pLTClient->GetCameraFOV(m_hCamera, &fFovX, &fFovY);

		LTFLOAT fSpeed = .005f * g_pLTClient->GetFrameTime();

		if (m_fFovXFXDir > 0)
		{
			fFovX -= fSpeed;
			fFovY -= fSpeed;

			if (fFovY < (m_fCurrentFovX * dwHeight - 10) / dwWidth)
			{
				fFovY = (m_fCurrentFovX * dwHeight - 10) / dwWidth;
				m_fFovXFXDir = -m_fFovXFXDir;
			}

		}
		else
		{
			fFovX += fSpeed;
			fFovY += fSpeed;

			if (fFovY > (m_fCurrentFovX * dwHeight) / dwWidth)
			{
				fFovY = (m_fCurrentFovX * dwHeight) / dwWidth;
				m_fFovXFXDir = -m_fFovXFXDir;
			}

		}
	}

	g_pLTClient->SetCameraFOV(m_hCamera, fFovX, fFovY);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::ChangeWeapon(ILTMessage_Read* hMessage)
{
	if (!g_pLTClient || !hMessage) return;

	uint8 nCommandId = hMessage->Readuint8();
	uint8 bAuto      = (LTBOOL) hMessage->Readuint8();

	uint8 nWeaponId = GetWeaponId(nCommandId, GetPlayerMode());

#ifdef _DEMO
	if (!m_weaponModel.IsDemoWeapon(nCommandId))
	{
		// Tell the user the weapon isn't available...

		UpdatePlayerStats(IC_OUTOFAMMO_ID, nWeaponId, 1.0f);		
		return;
	}
#endif

	LTBOOL bChange = LTTRUE;

	// If this is an auto weapon change and this is a multiplayer game, see 
	// if the user really wants us to switch or not (we'll always switch in
	// single player games)...

	if (bAuto && IsMultiplayerGame())
	{
		bChange = LTFALSE;

		HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("AutoWeaponSwitch");
		if (hVar)
		{
			bChange = (LTBOOL) pClientDE->GetVarValueFloat(hVar);
		}
	}

	if (bChange)
	{
		// Force a change to the approprite weapon... 

		ChangeWeapon(nWeaponId, LTFALSE, m_stats.GetAmmoCount(nWeaponId));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ChangeWeapon()
//
//	PURPOSE:	Change the weapon model
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::ChangeWeapon(uint8 nWeaponId, LTBOOL bZoom, uint32 dwAmmo)
{
	if (!g_pLTClient) return;

//	CSPrint("ChangeWeapon nWeaponId = %u", (DWORD) nWeaponId); // BLB Temp

	m_bZoomView = bZoom;

	HandleZoomChange(nWeaponId);

	if (!m_weaponModel.GetHandle() || nWeaponId != m_weaponModel.GetId())
	{
		m_weaponModel.Create(g_pLTClient, nWeaponId);
	}

	// Update the ammo display...

	m_stats.UpdateAmmo((uint32)nWeaponId, dwAmmo);
	m_stats.UpdatePlayerWeapon(nWeaponId);
		
	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;

	if (pSettings->VehicleMode() || m_playerCamera.IsChaseView())
	{
		m_weaponModel.SetVisible(LTFALSE);
	}

	// Tell the server to change weapons...

	ILTMessage_Write* hMessage = g_pLTClient->StartMessage(MID_WEAPON_CHANGE);
	pClientDE->WriteToMessageByte(hMessage, nWeaponId);
	pClientDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleZoomChange()
//
//	PURPOSE:	Handle a potential zoom change
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::HandleZoomChange(uint8 nWeaponId)
{
	if (!g_pLTClient) return;
	
	if (m_bOldZoomView != m_bZoomView)
	{
		// Play zoom in/out sounds...

		char* pSound = (nWeaponId == GUN_SNIPERRIFLE_ID ? "Sounds\\Weapons\\SniperRifle\\zoomout.wav"
													    : "Sounds\\Weapons\\AssaultRifle\\zoomout.wav");
		if (m_bZoomView)
		{
			m_bDrawHud = LTFALSE;

			pSound = (nWeaponId == GUN_SNIPERRIFLE_ID ? "Sounds\\Weapons\\SniperRifle\\zoomin.wav"
													  : "Sounds\\Weapons\\AssaultRifle\\zoomin.wav");
		}
		else
		{
			m_bDrawHud = LTTRUE;
		}

		PlaySoundLocal(pSound, SOUNDPRIORITY_MISC_MEDIUM, LTFALSE, LTFALSE, 100, LTTRUE );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnCommandOn()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnCommandOn(int command)
{
	if (!pClientDE) return;

	// only allow input if not editing and not drawing the mission log

	if (m_messageMgr.GetEditingState())
	{
		return;
	}
	
	// take appropriate action

	switch (command)
	{
		case COMMAND_ID_PREV_WEAPON : 
		{
			m_weaponModel.ChangeToPrevWeapon(); 
		}
		break;
		
		case COMMAND_ID_NEXT_WEAPON : 
		{
			m_weaponModel.ChangeToNextWeapon();
		}
		break;

		case COMMAND_ID_WEAPON_1 :
		case COMMAND_ID_WEAPON_2 :
		case COMMAND_ID_WEAPON_3 :
		case COMMAND_ID_WEAPON_4 :
		case COMMAND_ID_WEAPON_5 :
		case COMMAND_ID_WEAPON_6 :
		case COMMAND_ID_WEAPON_7 :
		case COMMAND_ID_WEAPON_8 :
		case COMMAND_ID_WEAPON_9 :	
		case COMMAND_ID_WEAPON_10 :
		{
			m_weaponModel.ChangeWeapon(command);
		}
		break;

		case COMMAND_ID_SHOWORDINANCE :
		{
			m_bDrawOrdinance = (m_nPlayerMode != PM_MODE_KID) ? LTTRUE : LTFALSE;
		}
		break;
		
		case COMMAND_ID_DECSCREENRECT :
		case COMMAND_ID_INCSCREENRECT :
		{
			if (m_bUsingExternalCamera) break;

			uint32 nWidth = 0;
			uint32 nHeight = 0;
			HSURFACE hScreen = pClientDE->GetScreenSurface();
			g_pLTClient->GetSurfaceDims (hScreen, &nWidth, &nHeight);

			int dx = (int)(((LTFLOAT)nWidth * 0.1f) / 2.0f);		// 5% of screen width on either side
			int dy = (int)(((LTFLOAT)nHeight * 0.1f) / 2.0f);		// 5% of screen height on either side
			int nMinWidth = (int)nWidth >> 1;
			int nMinHeight = (int)nHeight >> 1;

			int nLeft = 0;
			int nTop = 0;
			int nRight = 0;
			int nBottom = 0;
			bool bFullScreen = false;
			g_pLTClient->GetCameraRect (m_hCamera, &bFullScreen, &nLeft, &nTop, &nRight, &nBottom);

			// compute the actual resizing...
			
			if (command == COMMAND_ID_DECSCREENRECT)
			{
				// special case for letterbox format
				if (nTop == 0 && nBottom == (int)nHeight && !m_bStatsSizedOff)
				{
					dy = LETTERBOX_ADJUST;
					dx = 0;
				}
				else if (nTop == 0 && nBottom == (int)nHeight && m_bStatsSizedOff)
				{
					m_bStatsSizedOff = LTFALSE;
					dx = 0;
					dy = 0;
				}
				else if (nTop == LETTERBOX_ADJUST && nLeft == 0)
				{
					nTop = 0;
					nBottom = nHeight;
				}
				
				if (((nRight - dx) - (nLeft + dx)) >= nMinWidth && ((nBottom - dy) - (nTop + dy)) >= nMinHeight)
				{
					nLeft += dx;
					nTop += dy;
					nRight -= dx;
					nBottom -= dy;
				}

				if (nLeft != 0 || nTop != 0 || nRight != (int)nWidth || nBottom != (int)nHeight) bFullScreen = false;
				g_pLTClient->SetCameraRect (m_hCamera, bFullScreen, nLeft, nTop, nRight, nBottom);
			}
			else
			{
				// special case for letterbox format
				if (nTop == LETTERBOX_ADJUST && nLeft == 0)
				{
					nTop = dy;
					nBottom = nHeight - dy;
				}
				else if (nLeft == 0 && nTop == 0 && !m_bStatsSizedOff)
				{
					m_bStatsSizedOff = LTTRUE;
				}
				else if (nLeft - dx == 0 && nTop - dy == 0)
				{
					nTop += LETTERBOX_ADJUST;
					nBottom -= LETTERBOX_ADJUST;
				}

				nLeft -= dx;
				nTop -= dy;
				nRight += dx;
				nBottom += dy;

				if (nLeft < 0) nLeft = 0;
				if (nTop < 0) nTop = 0;
				if (nRight > (int)nWidth) nRight = (int)nWidth;
				if (nBottom > (int)nHeight) nBottom = (int)nHeight;

				if (nLeft == 0 && nTop == 0 && nRight == (int)nWidth && nBottom == (int)nHeight) bFullScreen = true;
				g_pLTClient->SetCameraRect (m_hCamera, bFullScreen, nLeft, nTop, nRight, nBottom);
			}

			AddToClearScreenCount();
		}
		break;

		case COMMAND_ID_FRAGCOUNT :
		{
			// make sure we're in multiplayer first
			
			if (!IsMultiplayerGame()) return;

			m_bDrawFragCount = LTTRUE;
		}
		break;

		case COMMAND_ID_TURNAROUND :
		{
			m_fYaw += MATH_PI;
		}
		break;

		case COMMAND_ID_CAMERACIRCLE :
		{
			//if (!m_bSpectatorMode && !m_messageMgr.GetEditingState())
			//{
			//	m_playerCamera.StartCircle(0.0f, 75.0f, 0.0f, 3.0f);
			//}
		}
		break;
	
		case COMMAND_ID_CHASEVIEWTOGGLE :
		{
			if (m_ePlayerState == PS_ALIVE && !m_bZoomView)
			{
				SetExternalCamera(m_playerCamera.IsFirstPerson());
			}
		}
		break;

		case COMMAND_ID_CROSSHAIRTOGGLE :
		{
			m_stats.ToggleCrosshair();
			m_bCrosshairOn = m_stats.CrosshairOn();
		}
		break;

		case COMMAND_ID_MOUSEAIMTOGGLE :
		{
			CRiotSettings* pSettings = m_menu.GetSettings();
			if (!pSettings) return;
			
			if (m_playerCamera.IsFirstPerson())
			{
				if (!pSettings->MouseLook())
				{
					m_bHoldingMouseLook = LTTRUE;
				}
			}
		}
		break;

		case COMMAND_ID_INTERFACETOGGLE:
		{
			// m_bDrawInterface = !m_bDrawInterface;
		}
		break;

		case COMMAND_ID_CENTERVIEW :
		{
			m_fPitch = 0.0f;
		}
		break;

		case COMMAND_ID_MESSAGE :
		{
			if (m_nGameState == GS_PLAYING)
			{
				m_messageMgr.SetEditingState(LTTRUE);
				SetInputState(LTFALSE);
			}
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if (m_ePlayerState == PS_DEAD)
			{
				if (g_pLTClient->GetTime() > m_fEarliestRespawnTime)
				{
					HandleRespawn();
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
			QuickSave();
		}
		break;

		case COMMAND_ID_QUICKLOAD :
		{
			QuickLoad();
		}
		break;

		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnCommandOff()
//
//	PURPOSE:	Handle command off notification
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnCommandOff(int command)
{
	if (!g_pLTClient) return;

	// only process if not editing a message
	if (!m_messageMgr.GetEditingState())
	{
		switch (command)
		{
			case COMMAND_ID_SHOWORDINANCE :
			{
				m_bDrawOrdinance = LTFALSE;
			}
			break;

			case COMMAND_ID_FRAGCOUNT : 
			{
				m_bDrawFragCount = LTFALSE;
			}
			break;

			case COMMAND_ID_STRAFE :
			{
				m_bStrafing = LTFALSE;
			}
			break;

			case COMMAND_ID_MOUSEAIMTOGGLE :
			{
				CRiotSettings* pSettings = m_menu.GetSettings();
				if (!pSettings) return;
				
				m_bHoldingMouseLook = LTFALSE;
			}
			break;

			default : break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnKeyDown(int key, int rep)
//
//	PURPOSE:	Handle key down notification
//				Try to avoid using OnKeyDown and OnKeyUp as they
//				are not portable functions
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnKeyDown(int key, int rep)
{
	if (!g_pLTClient) return;

	// get the vk codes for yes and no from cres.dll
	int nYesVKCode = TextHelperGetIntValFromStringID(pClientDE, IDS_YES_VK_CODE, VK_Y);
	int nNoVKCode = TextHelperGetIntValFromStringID(pClientDE, IDS_NO_VK_CODE, VK_N);


	//********  Are We Showing a MessageBox  ********//
	
	if (m_pMessageBox)
	{
		if ((key == VK_RETURN && !m_pMessageBox->IsYesNo()) || ((key == nYesVKCode || key == nNoVKCode) && m_pMessageBox->IsYesNo()))
		{
			LTBOOL bYesNo = m_pMessageBox->IsYesNo();
			
			delete m_pMessageBox;
			m_pMessageBox = LTNULL;

			if (bYesNo && m_pYesNoProc)
			{
				m_pYesNoProc (key == nYesVKCode, m_nYesNoUserData);
				m_pYesNoProc = LTNULL;
				m_nYesNoUserData = 0;
			}

			if (m_nGameState == GS_MENU)
			{
				m_bClearScreenAlways = LTTRUE;
			}
			else
			{
				PauseGame(LTFALSE);
				AddToClearScreenCount();
			}
		}
		return;
	}


	//********  We are playing the credits  ********//

	if (m_bGameOver && m_nGameState == GS_PLAYING)
	{
		if (key != VK_ESCAPE)
		{
			m_credits.HandleInput (key);
			return;
		}
	}
	
	
	//********  Are We Broadcasting a Message  ********//
	
	if (m_messageMgr.GetEditingState())
	{
		m_messageMgr.HandleKeyDown (key, rep);
		return;
	}
	
	//********  Stopping Movies  ********//
	
	if (m_nGameState == GS_MOVIES)
	{
		if (key == VK_ESCAPE && pClientDE->IsVideoPlaying() == VIDEO_PLAYING)
		{
			if (pClientDE->StopVideo() == LT_OK)
			{
				PlayIntroMovies (pClientDE);
			}
		}
		return;
	}
	
	//********  Credits Mode Input  ********//

	if (m_nGameState == GS_CREDITS)
	{
		if (key == VK_ESCAPE)
		{
			m_nGameState = GS_MENU;
			SetMenuMusic(LTTRUE);
			m_credits.Term();
		}
		else
		{
			m_credits.HandleInput (key);
		}

		return;
	}
	
	//********  Intro Mode Input  ********//

	if (m_nGameState == GS_INTRO)
	{
		if (key == VK_ESCAPE)
		{
			m_credits.Term();
			DoStartGame();
		}
		else
		{
			m_credits.HandleInput (key);
		}

		return;
	}

	//********  Stopping Bumper Screen Mode  ********//

	if (m_nGameState == GS_BUMPER)
	{
		// change our state to playing...

		m_nGameState = GS_PLAYING;
		PauseGame (LTFALSE);

		// get rid of the bumper screen and text if we had one (there better be one if we got here!)
	
		RemoveBumperScreen();
		
		g_pLTClient->ClearInput(); // Don't process the key they hit...
		return;
	}
	
	//********  Menu Input  ********//
	
	if (m_nGameState == GS_MENU)
	{
		m_menu.HandleInput (key);
		return;
	}
	

	//********  Paused Input  ********//
	
	if (m_nGameState == GS_PAUSED)
	{
		// They pressed a key - unpause the game

		if (m_hGamePausedSurface) 
		{
			g_pLTClient->DeleteSurface (m_hGamePausedSurface);
			m_hGamePausedSurface = LTNULL;
		}

		PauseGame (LTFALSE);
		m_nGameState = GS_PLAYING;
		return;
	}

	
	//********  Demo Multiplayer Mode Input  ********//

	if (m_nGameState == GS_DEMO_MULTIPLAYER)
	{
		if (key == VK_ESCAPE)
		{
			m_nGameState = GS_MENU;
			SetMenuMusic(LTTRUE);
			m_credits.Term();
		}
		else
		{
			m_credits.HandleInput (key);
		}

		return;
	}

	//********  Ingame Dialog Handling  ********//
	
	if (m_pIngameDialog)
	{
		switch (key)
		{
			case VK_DOWN:
			{
				m_pIngameDialog->ScrollDown();
				break;
			}

			case VK_UP:
			{
				m_pIngameDialog->ScrollUp();
				break;
			}

			case VK_NEXT:
			{
				m_pIngameDialog->PageDown();
				break;
			}

			case VK_PRIOR:
			{
				m_pIngameDialog->PageUp();
				break;
			}

			case VK_RETURN:
			{
				// get the current selection's string and target

				LTFLOAT fSelection =  (LTFLOAT)m_pIngameDialog->GetMenuSelection();
				POPUPMENUITEM* pItem = m_pIngameDialog->GetMenuItem ((int)fSelection);
				uint32 nDlgObjHandle = pItem->nData;
				pItem->nData = 0;

				// delete the popup menu and send a message to the server

				delete m_pIngameDialog;
				m_pIngameDialog = LTNULL;

				ILTMessage_Write* hMessage = g_pLTClient->StartMessage(MID_DIALOG_CLOSE);
				pClientDE->WriteToMessageLTFLOAT (hMessage, fSelection);
				pClientDE->WriteToMessageByte (hMessage, (uint8) (nDlgObjHandle));
				pClientDE->WriteToMessageByte (hMessage, (uint8) (nDlgObjHandle >> 8));
				pClientDE->WriteToMessageByte (hMessage, (uint8) (nDlgObjHandle >> 16));
				pClientDE->WriteToMessageByte (hMessage, (uint8) (nDlgObjHandle >> 24));
				pClientDE->EndMessage(hMessage);

				// re-enable client-side input

				PauseGame (LTFALSE);

				break;
			}
		}

		return;
	}
	
	//********  Mission Log Drawing  ********//
	
	if (m_bDrawMissionLog && !m_objectives.IsClosing() && !m_bMissionLogKeyStillDown)// && rep == 1)
	{
		if (key == VK_ESCAPE || key == VK_F1)
		{
			PlaySoundLocal ("Sounds\\Interface\\LogClose.wav", SOUNDPRIORITY_MISC_MEDIUM);
			
			if (key == VK_F1) m_bMissionLogKeyStillDown = LTTRUE;
			m_objectives.StartCloseAnimation();
			m_bWaitingForMLClosure = LTTRUE;
			
			m_objectives.ResetTop();
		}
		else if (key == VK_UP)
		{
			m_objectives.ScrollUp();
		}
		else if (key == VK_DOWN)
		{
			m_objectives.ScrollDown();
		}

		return;
	}


	if (key == VK_PAUSE)
	{
		if (IsMultiplayerGame() || m_nGameState != GS_PLAYING) return;
		
		if (!m_bGamePaused)
		{
			m_nGameState = GS_PAUSED;

			m_hGamePausedSurface = CTextHelper::CreateSurfaceFromString(pClientDE, m_menu.GetFont28n(), IDS_PAUSED);
		}

		PauseGame (!m_bGamePaused, LTTRUE);
		return;
	}
	
	if (key == VK_F1 && !m_bDrawMissionLog && m_nPlayerMode != PM_MODE_KID && 
		!m_bMissionLogKeyStillDown && !m_bUsingExternalCamera)
	{
		// if we're in multiplayer just return
		
		if (IsMultiplayerGame()) return;

		PlaySoundLocal ("Sounds\\Interface\\LogOpen.wav", SOUNDPRIORITY_MISC_MEDIUM);
		
		m_objectives.ResetTop();

		m_bDrawMissionLog = LTTRUE;
		m_bMissionLogKeyStillDown = LTTRUE;
		m_bNewObjective = LTFALSE;
		if (m_hNewObjective) g_pLTClient->DeleteSurface (m_hNewObjective);
		m_hNewObjective = LTNULL;
		AddToClearScreenCount();

		PauseGame(LTTRUE);

		m_LightScaleMgr.SetLightScale (&m_vLightScaleObjectives, LightEffectInterface);

		m_objectives.StartOpenAnimation();

		return;
	}

	//********  Escape Key Handling  ********//
	
	if (key == VK_ESCAPE)
	{
		if (m_nGameState == GS_PLAYING) //  && !m_bUsingExternalCamera)
		{
			SetMenuMode (LTTRUE);
		}
		return;
	}

	//********  Show Player Stats  ********//
	
	if (key == VK_TAB)
	{
//		m_stats.ShowAllStats (LTTRUE);
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnKeyUp(int key, int rep)
//
//	PURPOSE:	Handle key down notification
//				Try to avoid using OnKeyDown and OnKeyUp as they
//				are not portable functions
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnKeyUp(int key)
{
	if (!g_pLTClient) return;

	// if it's the tilde (~) key then the console has been turned off
	// (this is the only event that causes this key to ever get processed)
	// so clear the back buffer to get rid of any part of the console still showing
	if (key == VK_TILDE)
	{
		AddToClearScreenCount();
	}

	if (m_messageMgr.GetEditingState())
	{
		m_messageMgr.HandleKeyUp (key);
	}

	if (key == VK_F1 && m_bMissionLogKeyStillDown)
	{
		m_bMissionLogKeyStillDown = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdatePlayerFlags
//
//	PURPOSE:	Update our copy of the movement flags
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdatePlayerFlags()
{
	// Clear movement flags...

	m_dwPlayerFlags = 0; 

	if (!g_pLTClient) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;

	// only allow input if not editing and not drawing the mission log

	if (m_messageMgr.GetEditingState())
	{
		return;
	}

	// Determine what commands are currently on...

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RUN) || pSettings->RunLock())
	{
		m_dwPlayerFlags |= CS_MFLG_RUN;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))
	{
		m_dwPlayerFlags |= CS_MFLG_JUMP;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_DOUBLEJUMP))
	{
		m_dwPlayerFlags |= CS_MFLG_DOUBLEJUMP;
	}

	// Use the player's user flags instead of checking this flag directly
	// (this insures that the camera will be moved accurately...)
	//if (pClientDE->IsCommandOn(COMMAND_ID_DUCK))

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (hPlayerObj)
	{
		uint32 dwUsrFlags;
		g_pLTClient->Common()->GetObjectFlags(hPlayerObj, OFT_User, dwUsrFlags);
		if (dwUsrFlags & USRFLG_PLAYER_DUCK)
		{
			m_dwPlayerFlags |= CS_MFLG_DUCK;
		}
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))
	{
		m_dwPlayerFlags |= CS_MFLG_FORWARD;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_REVERSE))
	{
		m_dwPlayerFlags |= CS_MFLG_REVERSE;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE))
	{
		m_dwPlayerFlags |= CS_MFLG_STRAFE;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RIGHT))
	{
		m_dwPlayerFlags |= CS_MFLG_RIGHT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_LEFT))
	{
		m_dwPlayerFlags |= CS_MFLG_LEFT;
	}

	if ( ((m_dwPlayerFlags & CS_MFLG_RIGHT) && 
		  (m_dwPlayerFlags & CS_MFLG_STRAFE)) ||
		  g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT) )
	{
		m_dwPlayerFlags |= CS_MFLG_STRAFE_RIGHT;
	}

	if ( ((m_dwPlayerFlags & CS_MFLG_LEFT) && 
		  (m_dwPlayerFlags & CS_MFLG_STRAFE)) ||
		  g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT) )
	{
		m_dwPlayerFlags |= CS_MFLG_STRAFE_LEFT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FIRING))
	{
		m_dwPlayerFlags |= CS_MFLG_FIRING;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_LOOKUP))
	{
		m_dwPlayerFlags |= CS_MFLG_LOOKUP;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_LOOKDOWN))
	{
		m_dwPlayerFlags |= CS_MFLG_LOOKDOWN;
	}

	// Check to see if the player is moving...

	m_bMoving = m_bMovingSide = LTFALSE;

	if (m_dwPlayerFlags & CS_MFLG_FORWARD ||
		m_dwPlayerFlags & CS_MFLG_REVERSE || 
		m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT ||
		m_dwPlayerFlags & CS_MFLG_STRAFE_LEFT)
	{
		m_bMoving = LTTRUE;
	}

	if (m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT || 
		m_dwPlayerFlags & CS_MFLG_STRAFE_LEFT)
	{
		m_bMovingSide = LTTRUE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnMessage()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnMessage(uint8 messageID, ILTMessage_Read* hMessage)
{
	if (!g_pLTClient) return;

	LTVector newColor;

	switch(messageID)
	{
		case MID_TIMEOFDAYCOLOR:
		{
			newColor.x = (LTFLOAT)hMessage->Readuint8() / MAX_WORLDTIME_COLOR;
			newColor.y = (LTFLOAT)hMessage->Readuint8() / MAX_WORLDTIME_COLOR;
			newColor.z = (LTFLOAT)hMessage->Readuint8()/ MAX_WORLDTIME_COLOR;
			m_LightScaleMgr.SetTimeOfDayScale(newColor);

			if(m_bUseWorldFog)
			{
				ResetGlobalFog();
			}
		}
		break;

		case MID_SERVERFORCEPOS:
		{
			if(m_MoveMgr)
				m_MoveMgr->OnServerForcePos(hMessage);

			m_bPlayerPosSet = LTTRUE;
		}
		break;

		case MID_TRACTORBEAM_POS:
		{
			if(m_MoveMgr)
				m_MoveMgr->OnTractorBeamPos(hMessage);
		}
		break;

		case MID_PHYSICS_UPDATE:
		{
			if(m_MoveMgr)
				m_MoveMgr->OnPhysicsUpdate(hMessage);
		}
		break;

		case MID_PLAYER_ONGROUND :
		{
			m_bOnGround = (LTBOOL)hMessage->Readuint8();
		}
		break;


		case MID_PLAYER_INFOCHANGE :
		{
			int nThing = -1, nType = -1;
			LTFLOAT fNewAmount = 0.0f;

			nThing = hMessage->Readuint8();
			nType  = hMessage->Readuint8();
			fNewAmount = hMessage->Readfloat();

			UpdatePlayerStats(nThing, nType, fNewAmount);
		}
		break;

		case MID_WEAPON_CHANGE :
		{
			ChangeWeapon(hMessage);
		}
		break;

		case MID_PLAYER_ADDED:
		{
			// only do something if we're in multiplayer
			if (!IsMultiplayerGame()) break;
			
			HSTRING hstrName = hMessage->ReadHString();
			uint32 nID = (uint32) hMessage->Readfloat();
			int nFrags = (int) hMessage->Readfloat();
			uint8 r, g, b;
			r = hMessage->Readuint8();
			g = hMessage->Readuint8();
			b = hMessage->Readuint8();
			m_ClientInfo.AddClient (hstrName, nID, nFrags, r, g, b);

			HSTRING hStr = g_pLTClient->FormatString (IDS_JOINEDGAME, g_pLTClient->GetStringData (hstrName));
			CSPrint (const_cast<char *>(g_pLTClient->GetStringData (hStr)));
			g_pLTClient->FreeString (hStr);
		}
		break;

		case MID_PLAYER_REMOVED:
		{
			// only do something if we're in multiplayer
			if (!IsMultiplayerGame()) break;
			
			uint32 nID = (uint32) hMessage->Readfloat();
			
			HSTRING hStr = g_pLTClient->FormatString (IDS_LEFTGAME, m_ClientInfo.GetPlayerName (nID));
			CSPrint (const_cast<char *>(g_pLTClient->GetStringData (hStr)));
			g_pLTClient->FreeString (hStr);
			
			m_ClientInfo.RemoveClient (nID);
		}
		break;

		case MID_PINGTIMES:
		{
			while(1)
			{
				uint16 id, ping;
				CLIENT_INFO *pClient;

				id = hMessage->Readuint16();
				if(id == 0xFFFF)
					break;

				ping = hMessage->Readuint16();
				if(pClient = m_ClientInfo.GetClientByID(id))
					pClient->m_Ping = ping / 1000.0f;
			}

			m_ClientInfo.ClearUpToDate();
		}
		break;

		case MID_PLAYER_FRAGGED:
		{
			// only do something if we're in multiplayer
			if (!IsMultiplayerGame()) break;

			uint32 nLocalID = 0;
			g_pLTClient->GetLocalClientID (&nLocalID);

			uint32 nVictim = (uint32) hMessage->Readfloat();
			uint32 nKiller = (uint32) hMessage->Readfloat();

			if (nVictim == nKiller)
			{
				m_ClientInfo.RemoveFrag (nLocalID, nKiller);
			}
			else
			{
				m_ClientInfo.AddFrag (nLocalID, nKiller);
			}

			if (nVictim == nLocalID)
			{
				HSTRING hStr = LTNULL;
				if (nVictim == nKiller)
				{
					hStr = g_pLTClient->FormatString (IDS_KILLEDMYSELF);
				}
				else
				{
					hStr = g_pLTClient->FormatString (IDS_HEKILLEDME, m_ClientInfo.GetPlayerName (nKiller));
				}
				CSPrint (const_cast<char *>(g_pLTClient->GetStringData (hStr)));
				g_pLTClient->FreeString (hStr);
			}
			else if (nKiller == nLocalID)
			{
				HSTRING hStr = LTNULL;
				hStr = g_pLTClient->FormatString (IDS_IKILLEDHIM, m_ClientInfo.GetPlayerName (nVictim));
				CSPrint (const_cast<char *>(g_pLTClient->GetStringData (hStr)));
				g_pLTClient->FreeString (hStr);
			}
			else
			{
				HSTRING hStr = LTNULL;

				if (nVictim == nKiller)
				{
					hStr = g_pLTClient->FormatString (IDS_HEKILLEDHIMSELF, m_ClientInfo.GetPlayerName(nKiller));
				}
				else
				{
					hStr = g_pLTClient->FormatString (IDS_HEKILLEDHIM, m_ClientInfo.GetPlayerName (nKiller), m_ClientInfo.GetPlayerName (nVictim));
				}
				CSPrint (const_cast<char *>(g_pLTClient->GetStringData (hStr)));
				g_pLTClient->FreeString (hStr);
			}
		}
		break;

		case MID_CHANGING_LEVELS :
		{
			HandleMPChangeLevel();
		}
		break;

		case MID_POWERUP_PICKEDUP :
		{
			PickupItemType eType = (PickupItemType)hMessage->Readuint8();
			HandleItemPickedup(eType);
		}
		break;

		case MID_POWERUP_EXPIRED :
		{
			PickupItemType eType = (PickupItemType)hMessage->Readuint8();
			HandleItemExpired(eType);
		}
		break;

		case MID_WEAPON_STATE :
		{
			m_weaponModel.HandleStateChange(hMessage);
		}
		break;

		case STC_BPRINT :
		{
			char msg[50];
			hMessage->ReadString(msg, 50);
			CSPrint (msg);
		}
		break;

		case MID_CAMERA_FOV :
		{
			LTFLOAT x, y;
			x = hMessage->Readfloat();
			y = hMessage->Readfloat();

			g_pLTClient->SetCameraFOV(m_hCamera, x, y);

			// Save the current fovX...(used for zooming view)

			m_fCurrentFovX = x;
		}
		break;

		case MID_CAMERA_RECT :
		{
			int l, t, r, b;
			l = (int)hMessage->Readfloat();
			t = (int)hMessage->Readfloat();
			r = (int)hMessage->Readfloat();
			b = (int)hMessage->Readfloat();

			if (m_hCamera)
			{
				g_pLTClient->SetCameraRect(m_hCamera, LTFALSE, l, t, r, b);
			}
		}
		break;

		case MID_PLAYER_MODECHANGE :
		{
			CRiotSettings* pSettings = m_menu.GetSettings();
			if (!pSettings) return;
			
			LTBOOL  bOldVehicleMode = pSettings->VehicleMode();
			uint32 nOldPlayerMode = m_nPlayerMode;
			LTFLOAT x, y;

			m_vCameraOffset = hMessage->ReadLTVector();
			m_nPlayerMode  = hMessage->Readuint8();
			pSettings->Misc[RS_MISC_VEHICLEMODE].nValue = (LTBOOL)hMessage->Readuint8();
			x			   = hMessage->Readfloat();
			y			   = hMessage->Readfloat();

			if (pSettings->VehicleMode() != bOldVehicleMode) 
			{
				// Going to vehicle mode...

				if (pSettings->VehicleMode())
				{
					// Assume we have the sniper rifle, so un-zoom (won't do
					// anything if we're not zoomed in)...

					m_bZoomView = LTFALSE;
					HandleZoomChange(GUN_SNIPERRIFLE_ID);

					m_weaponModel.SetVisible(LTFALSE);
					m_weaponModel.SetZoom(LTFALSE);

					m_stats.SetDrawAmmo (LTFALSE);
				}
				else 
				{
					if (m_playerCamera.IsFirstPerson())
					{
						m_weaponModel.SetVisible(LTTRUE);
					}

					uint8 nWeaponId = m_stats.GetCurWeapon();

					m_stats.SetDrawAmmo(LTTRUE);
					m_stats.UpdatePlayerWeapon(nWeaponId, LTTRUE);

					ChangeWeapon(nWeaponId, LTFALSE, m_stats.GetAmmoCount(nWeaponId));
				}

				if (m_playerCamera.IsFirstPerson())
				{
					if (pSettings->VehicleMode()) ShowPlayer(LTTRUE);
					else ShowPlayer(LTFALSE);
				}
			}

			if (m_hCamera)
			{
				g_pLTClient->SetCameraFOV(m_hCamera, x, y);
			}

			m_playerCamera.SetFirstPersonOffset(m_vCameraOffset);

			if (nOldPlayerMode == PM_MODE_KID && m_nPlayerMode != PM_MODE_KID)
			{
				m_bDrawInterface = LTTRUE;
			}
			else if (m_nPlayerMode == PM_MODE_KID)
			{
				m_bDrawInterface = LTFALSE;
			}

			m_stats.UpdatePlayerMode (m_nPlayerMode);
		}
		break;

		case MID_PLAYER_EXITLEVEL :
		{	
			HandleExitLevel(hMessage);
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

		case MID_PLAYER_DAMAGE:
		{
			HandlePlayerDamage(hMessage);
		}
		break;

		case MID_PLAYER_MESSAGE :
		{
			// retrieve the string from the message, play the chat sound, and display the message
			
			char *pMessage = pClientDE->ReadFromMessageString (hMessage);

			HSTRING hstrChatSound = pClientDE->FormatString (IDS_CHATSOUND);
			PlaySoundInfo playSoundInfo;
			PLAYSOUNDINFO_INIT(playSoundInfo);
			playSoundInfo.m_dwFlags = PLAYSOUND_CLIENT | PLAYSOUND_LOCAL;
			SAFE_STRCPY(playSoundInfo.m_szSoundName, pClientDE->GetStringData (hstrChatSound));
			pClientDE->PlaySound(&playSoundInfo);

			pClientDE->FreeString(hstrChatSound);

			CSPrint (pMessage);
		}
		break;

		case MID_PLAYER_ORIENTATION :
		{
			// Set our pitch, yaw, and roll according to the players...

			LTVector vVec;
			vVec = hMessage->ReadLTVector();

			m_fPitch		= vVec.x;
			m_fYaw			= vVec.y;
			m_fCamCant		= vVec.z;
			m_fYawBackup	= m_fYaw;
			m_fPitchBackup	= m_fPitch;
		}
		break;

		case MID_COMMAND_TOGGLE :
		{
			uint8 nId = hMessage->Readuint8();

			switch(nId)
			{
				case COMMAND_ID_RUNLOCK :
				{
					CRiotSettings* pSettings = m_menu.GetSettings();
					if (pSettings)
					{
						pSettings->SetRunLock((LTBOOL)hMessage->Readuint8());
					}
				}
				break;

				default : break;
			}
		}
		break;

		case MID_COMMAND_SHOWGAMEMSG :
		{
			char* strMessage = pClientDE->ReadFromMessageString (hMessage);
			ShowGameMessage (strMessage);
		}
		break;

		case MID_MUSIC:
		{
			if( m_Music.IsInitialized( ))
				m_Music.HandleMusicMessage( hMessage );
			break;
		}

		case MID_COMMAND_SHOWDLG:
		{
			m_pIngameDialog = CreateIngameDialog (hMessage);
			if (m_pIngameDialog)
			{
				// disable client input
				PauseGame (LTTRUE);
			}
		}
		break;

		case MID_COMMAND_OBJECTIVE:
		{
			HandleObjectives (hMessage);
		}
		break;

		case MID_COMMAND_TRANSMISSION:
		{
			HandleTransmission(hMessage);
		}
		break;

		case MID_PLAYER_LOADCLIENT :
		{
			UnpackClientSaveMsg(hMessage);
		}
		break;

		case MID_PLAYER_MULTIPLAYER_INIT :
		{
			InitMultiPlayer();
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

		case MID_SFX_REG:
		{
			SFXReg *pReg;
			char *pClassName;
			uint32 id;

			pClassName = pClientDE->ReadFromMessageString(hMessage);
			id = hMessage->Readuint8(); 

			if(pReg = FindSFXRegByName(pClassName))
			{
				pReg->m_ID = id;
			}
		}
		break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ShakeScreen()
//
//	PURPOSE:	Shake, rattle, and roll
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::ShakeScreen(LTVector vShake)
{
	if (!g_pLTClient) return;

	// Add...

	VEC_ADD(m_vShakeAmount, m_vShakeAmount, vShake);

	if (m_vShakeAmount.x > 3.0f) m_vShakeAmount.x = 3.0f;
	if (m_vShakeAmount.y > 3.0f) m_vShakeAmount.y = 3.0f;
	if (m_vShakeAmount.z > 3.0f) m_vShakeAmount.z = 3.0f;
}


// ----------------------------------------------------------------------- //
// Console command handlers for recording and playing demos.
// ----------------------------------------------------------------------- //

void CRiotClientShell::HandleRecord(int argc, char **argv)
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

void CRiotClientShell::HandlePlaydemo(int argc, char **argv)
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
//	ROUTINE:	CRiotClientShell::TintScreen()
//
//	PURPOSE:	Tint screen
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::TintScreen(LTVector vTintColor, LTVector vPos, LTFLOAT fTintRange,
								  LTFLOAT fTime, LTFLOAT fRampUp, LTFLOAT fRampDown, LTBOOL bForce)
{
	if (!g_pLTClient || !m_hCamera) return;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return;
	
	if (!bForce && !pSettings->ScreenFlash()) return;

	LTVector vCamPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

	// Determine if we can see this...

	LTVector vDir;
	VEC_SUB(vDir, vPos, vCamPos);
	LTFLOAT fDirMag = VEC_MAG(vDir);
	if (fDirMag > fTintRange) return;

	// Okay, not adjust the tint based on the camera's angle to the tint pos.

	LTRotation rRot;
	LTVector vU, vR, vF;
	g_pLTClient->GetObjectRotation(m_hCamera, &rRot);
	g_pLTClient->Math()->GetRotationVectors(rRot, vU, vR, vF);

	VEC_NORM(vDir);
	VEC_NORM(vF);
	LTFLOAT fMul = VEC_DOT(vDir, vF);
	if (fMul <= 0.0f) return;

	// {MD} See if we can even see this point.
	ClientIntersectQuery iQuery;
	ClientIntersectInfo iInfo;
	memset(&iQuery, 0, sizeof(iQuery));
	iQuery.m_From = vPos;
	iQuery.m_To = vCamPos;
	if(g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		// Something is in the way.
		return;
	}

	// Tint less if the pos was far away from the camera...

	LTFLOAT fVal = 1.0f - (fDirMag/fTintRange);
	fMul *= (fVal <= 1.0f ? fVal : 1.0f);

	m_bTintScreen	= LTTRUE;
	m_fTintStart	= pClientDE->GetTime();
	m_fTintTime		= fTime;
	m_fTintRampUp	= fRampUp;
	m_fTintRampDown	= fRampDown;
	VEC_COPY(m_vTintColor, vTintColor);
	VEC_MULSCALAR(m_vTintColor, m_vTintColor, fMul);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ShowGameMessage
//
//	PURPOSE:	Displays a message from the game in the center of the screen
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::ShowGameMessage(char* strMessage)
{
	if (!g_pLTClient) return;

	if (m_hGameMessage)
	{
		g_pLTClient->DeleteSurface (m_hGameMessage);
		m_hGameMessage = NULL;
	}

	HLTCOLOR hForeColor = g_pLTClient->SetupColor1 (1.0f, 1.0f, 1.0f, LTFALSE);
	HLTCOLOR hTrans = g_pLTClient->SetupColor2 (0.0f, 0.0f, 0.0f, LTTRUE);
	
	HSTRING hstrFont = g_pLTClient->FormatString (IDS_INGAMEFONT);
	FONT fontdesc (g_pLTClient->GetStringData(hstrFont), 
					TextHelperGetIntValFromStringID(g_pLTClient, IDS_GAMEMESSAGETEXTWIDTH, 9),
					TextHelperGetIntValFromStringID(g_pLTClient, IDS_GAMEMESSAGETEXTHEIGHT, 18),
					LTFALSE, LTFALSE, LTFALSE);
	g_pLTClient->FreeString (hstrFont);

	m_hGameMessage = CTextHelper::CreateSurfaceFromString (g_pLTClient, &fontdesc, strMessage, hForeColor);
	g_pLTClient->OptimizeSurface (m_hGameMessage, hTrans);

	uint32 cx, cy;
	g_pLTClient->GetSurfaceDims (m_hGameMessage, &cx, &cy);
	m_rcGameMessage.left = 0;
	m_rcGameMessage.top = 0;
	m_rcGameMessage.right = (int)cx;
	m_rcGameMessage.bottom = (int)cy;
	
	m_nGameMessageRemoveTime = g_pLTClient->GetTime() + 2.0f;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateHeadBob
//
//	PURPOSE:	Adjusts the head bobbing & swaying
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateHeadBob()
{
	// don't update the head bob if we have a dialog up

	if (m_pIngameDialog || m_bGamePaused) return;

	// on with bobbing...

	if (!g_pLTClient) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;
	
	const LTFLOAT c_fMaxBobAmp  = 10.0f;
	const LTFLOAT c_fDecayValue = 100.0f;

	LTBOOL bMecha = !((m_nPlayerMode == PM_MODE_FOOT) || (m_nPlayerMode == PM_MODE_KID));

	LTFLOAT	bobH  = BOBH_WALKING  / (bMecha ? (pSettings->VehicleMode() ? 500.0f : 165.0f) : 500.0f);
	LTFLOAT  bobV  = BOBV_WALKING  / (bMecha ? (pSettings->VehicleMode() ? 500.0f : 50.0f)  : 100.0f); // 500.0f;
	LTFLOAT  swayH = SWAYH_WALKING / 4000.0f;
	LTFLOAT  swayV = SWAYV_WALKING / 4000.0f;

	LTBOOL bRunning = (LTBOOL)(m_dwPlayerFlags & CS_MFLG_RUN);
	LTFLOAT moveDist = m_fVelMagnitude * g_pLTClient->GetFrameTime();

	// Running

	LTFLOAT pace;
	if (bMecha && !pSettings->VehicleMode())
	{
		if (bRunning) pace = MATH_CIRCLE * 0.8f;
		else pace = MATH_CIRCLE * 0.6;
	}
	else
	{
		if (bRunning) pace = MATH_CIRCLE * 0.7f;
		else pace = MATH_CIRCLE * 0.5;
	}

	m_fBobPhase += (g_pLTClient->GetFrameTime() * pace);
	if (m_fBobPhase > MATH_CIRCLE) m_fBobPhase -= MATH_CIRCLE;

	if (bRunning) pace = MATH_CIRCLE * 0.8f;
	else pace = 5*MATH_CIRCLE/6;

	m_fSwayPhase += (g_pLTClient->GetFrameTime() * pace) / 2;
	if (m_fSwayPhase > MATH_CIRCLE) m_fSwayPhase -= MATH_CIRCLE;


	// decay the amplitude
	m_fBobAmp = m_fBobAmp - g_pLTClient->GetFrameTime() * c_fDecayValue;
	if (m_fBobAmp < 0) m_fBobAmp = 0;

	if (m_fBobAmp < c_fMaxBobAmp)
	{
		m_fBobAmp += moveDist;
	}
	if (m_fBobAmp > c_fMaxBobAmp) m_fBobAmp = c_fMaxBobAmp;

	m_fBobHeight = bobV * m_fBobAmp * (LTFLOAT)sin(m_fBobPhase * 2);
	m_fBobWidth  = bobH * m_fBobAmp * (LTFLOAT)sin(m_fBobPhase - (MATH_PI/4));

	LTFLOAT fSwayHeight = -swayV * m_fBobAmp * (LTFLOAT)sin(m_fSwayPhase * 2);
	LTFLOAT fSwayWidth  = swayH * m_fBobAmp * (LTFLOAT)sin(m_fSwayPhase - (MATH_PI/3));


	// Update the weapon model bobbing...

	m_weaponModel.UpdateBob(fSwayWidth, fSwayHeight);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateHeadCant()
//
//	PURPOSE:	Update head tilt when strafing
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateHeadCant()
{
	LTFLOAT fMaxCantDist = m_fCantMaxDist;
	
	if (IsVehicleMode())
	{
		fMaxCantDist *= 1.5f;
	}
	else if (IsOnFoot())
	{
		fMaxCantDist *= 0.5f;
	}

		// If we are strafing, cant our head as we move..

	if (m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT)
	{
		m_fCamCant -= m_fCantIncrement;
		if (m_fCamCant < -fMaxCantDist)
			m_fCamCant = -fMaxCantDist;
	}
	else if (m_dwPlayerFlags & CS_MFLG_STRAFE_LEFT)
	{
		m_fCamCant += m_fCantIncrement;
		if (m_fCamCant > fMaxCantDist)
			m_fCamCant = fMaxCantDist;
	}
	else 
	{
		// We are not canting so move us toward zero...
		if (m_fCamCant != 0.0f)
		{
			if (m_fCamCant < 0.0f) 
			{
				m_fCamCant += m_fCantIncrement;
			} 
			else 
			{
				m_fCamCant -= m_fCantIncrement;
			}
			if (fabs(m_fCamCant) < m_fCantIncrement)
				m_fCamCant = 0.0f;
 		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateDuck()
//
//	PURPOSE:	Update ducking camera offset
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateDuck()
{
	if (!g_pLTClient) return;


	// Can't duck when in liquid...

	if (IsLiquid(m_eCurContainerCode)) return;


	LTFLOAT fTime = g_pLTClient->GetTime();

	if (m_dwPlayerFlags & CS_MFLG_DUCK)
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
//	ROUTINE:	CRiotClientShell::ProcessMenuCommand()
//
//	PURPOSE:	Process a menu command
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::ProcessMenuCommand (int nID)
{
	if (!g_pLTClient) return LTTRUE;
	
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdatePlayerStats()
//
//	PURPOSE:	Update the player's stats
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdatePlayerStats(uint8 nThing, uint8 nType, LTFLOAT fNewAmount)
{
	if (!g_pLTClient) return;
	
	switch (nThing)
	{
		case IC_WEAPON_PICKUP_ID :
		{
			m_inventory.GunPickup (nType);
			m_stats.UpdateAmmo ((uint32) nType, (uint32) fNewAmount);
			m_inventory.UpdateAmmo ((uint32) nType, (uint32) fNewAmount);
		}
		break;

		case IC_WEAPON_ID :
		{
			m_inventory.GunPickup (nType, LTFALSE);
			m_stats.UpdateAmmo ((uint32) nType, (uint32) fNewAmount);
			m_inventory.UpdateAmmo ((uint32) nType, (uint32) fNewAmount);
		}
		break;

		case IC_AMMO_ID :
		{		
			m_stats.UpdateAmmo ((uint32) nType, (uint32) fNewAmount);
			m_inventory.UpdateAmmo ((uint32) nType, (uint32) fNewAmount);
		}
		break;

		case IC_HEALTH_ID :
		{		
			m_stats.UpdateHealth ((uint32) fNewAmount);
		}
		break;

		case IC_ARMOR_ID :
		{		
			m_stats.UpdateArmor ((uint32) fNewAmount);
		}
		break;

		case IC_AIRLEVEL_ID :
		{	
			m_stats.UpdateAir (fNewAmount);
		}
		break;

		case IC_ROCKETLOCK_ID :
		{
			// Play targeting sound..

			char* pSound = (nType == 1 ? "Sounds\\Weapons\\lockedon.wav"
									   : "Sounds\\Weapons\\lockingon.wav");
	
			HSTRING hStr = g_pLTClient->FormatString (nType == 1 ? IDS_ROCKETLOCKON : IDS_ROCKETLOCKDETECTED);

			g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));

			m_infoDisplay.AddInfo(g_pLTClient->GetStringData (hStr), m_menu.GetFont12s(), 1.5f, DI_CENTER | DI_TOP);

			PlaySoundLocal(pSound, SOUNDPRIORITY_MISC_MEDIUM );

			g_pLTClient->FreeString (hStr);
		}
		break;

		case IC_OUTOFAMMO_ID :
		{

#ifdef _DEMO
			// If the newamount is 1.0 and this is a demo, let user know this
			// weapon is not available...

			if (fNewAmount == 1.0f)
			{
				char buf[50];
				sprintf(buf, "Not Available in Demo");
				pClientDE->CPrint(buf);
				m_infoDisplay.AddInfo(buf, m_menu.GetFont12s(), 1.5f, DI_CENTER | DI_BOTTOM);
				break;
			}
#endif

			HSTRING hStr = g_pLTClient->FormatString (IDS_OUTOFAMMO, GetWeaponString((RiotWeaponId)nType));
			g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));
			m_infoDisplay.AddInfo(g_pLTClient->GetStringData (hStr), m_menu.GetFont12s(), 1.5f, DI_CENTER | DI_BOTTOM);
			g_pLTClient->FreeString (hStr);
		}
		break;

		default : break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::CreateIngameDialog()
//
//	PURPOSE:	Create an ingame dialog
//
// ----------------------------------------------------------------------- //

CPopupMenu* CRiotClientShell::CreateIngameDialog (ILTMessage_Read* hMessage)
{
	if (!g_pLTClient) return LTNULL;

	// get the strings we need for the dialog

	uint32 nByte1 = (uint32) hMessage->Readuint8();
	uint32 nByte2 = (uint32) hMessage->Readuint8();
	uint32 nByte3 = (uint32) hMessage->Readuint8();
	uint32 nByte4 = (uint32) hMessage->Readuint8();

	int nItems = (int)hMessage->Readfloat();
	if (nItems == 0) return LTNULL;

	uint32 nStringID[3];
	for (int i = 0; i < nItems; i++)
	{
		nStringID[i] = hMessage->Readuint32();
	}

	// create the string surfaces

	HSURFACE hSurf[3];
	HSURFACE hSurfSelected[3];	
	for (int i = 0; i < nItems; i++)
	{
		hSurf[i] = CTextHelper::CreateSurfaceFromString (g_pLTClient, m_menu.GetFont08n(), nStringID[i]);
		hSurfSelected[i] = CTextHelper::CreateSurfaceFromString (g_pLTClient, m_menu.GetFont08s(), nStringID[i]);
	}
	
	// find the string with the largest width and largest height

	uint32 nMaxHeight = 0;
	uint32 nMaxWidth = 0;
	uint32 nWidth, nHeight;
	for (int i = 0; i < nItems; i++)
	{
		g_pLTClient->GetSurfaceDims (hSurf[i], &nWidth, &nHeight);
		if (nWidth > nMaxWidth) nMaxWidth = nWidth;
		if (nHeight > nMaxHeight) nMaxHeight = nHeight;
		
		g_pLTClient->GetSurfaceDims (hSurfSelected[i], &nWidth, &nHeight);
		if (nWidth > nMaxWidth) nMaxWidth = nWidth;
		if (nHeight > nMaxHeight) nMaxHeight = nHeight;
	}
	
	// make sure we're not wider than the screen width or 600, whichever is smaller

	uint32 nScreenWidth, nScreenHeight;
	g_pLTClient->GetSurfaceDims (g_pLTClient->GetScreenSurface(), &nScreenWidth, &nScreenHeight);
	if (nScreenWidth <= 16)
	{
		// possible infinite loop below if nScreenWidth comes back as fucked-up value
		for (int i = 0; i < nItems; i++)
		{
			g_pLTClient->DeleteSurface (hSurf[i]);
			g_pLTClient->DeleteSurface (hSurfSelected[i]);
		}
	}

	if (nMaxWidth + 16 > (min (nScreenWidth, 600)))
	{
		uint32 nShortenedWidth = nMaxWidth / 2;
		while (nShortenedWidth + 16 > (min (nScreenWidth, 600)))
		{
			nShortenedWidth /= 2;
		}
		
		for (int i = 0; i < nItems; i++)
		{
			g_pLTClient->DeleteSurface (hSurf[i]);
			g_pLTClient->DeleteSurface (hSurfSelected[i]);

			hSurf[i] = CTextHelper::CreateWrappedStringSurface (g_pLTClient, (int)nShortenedWidth, m_menu.GetFont08n(), nStringID[i], TH_ALIGN_CENTER, LTFALSE);
			hSurfSelected[i] = CTextHelper::CreateWrappedStringSurface (g_pLTClient, (int)nShortenedWidth, m_menu.GetFont08s(), nStringID[i], TH_ALIGN_CENTER, LTFALSE);
		}
	}

	nMaxWidth = 0;
	nMaxHeight = 0;
	int nTotalHeight = 0;
	for (int i = 0; i < nItems; i++)
	{
		g_pLTClient->GetSurfaceDims (hSurf[i], &nWidth, &nHeight);
		if (nWidth > nMaxWidth) nMaxWidth = nWidth;
		if (nHeight > nMaxHeight) nMaxHeight = nHeight;

		nTotalHeight += nHeight;
		
		g_pLTClient->GetSurfaceDims (hSurfSelected[i], &nWidth, &nHeight);
		if (nWidth > nMaxWidth) nMaxWidth = nWidth;
		if (nHeight > nMaxHeight) nMaxHeight = nHeight;
	}
	
	// now determine the best location (center screen?)

	g_pLTClient->GetSurfaceDims (g_pLTClient->GetScreenSurface(), &nWidth, &nHeight);
	int x = ((int)nWidth - ((int)nMaxWidth + 16)) / 2;
	int y = ((int)nHeight - ((int)nMaxHeight + 16)) / 2;

	// create the popup menu and start adding the menu items
	
	if (m_pIngameDialog)
	{
		delete m_pIngameDialog;
	}

	CPopupMenu* pIngameDialog = new CPopupMenu (x, y, (int)nMaxWidth + 16, nTotalHeight + 16);
	if (!pIngameDialog) return LTNULL;

	pIngameDialog->Init (g_pLTClient, LTNULL);

	for (int i = 0; i < nItems; i++)
	{
		POPUPMENUITEM* pItem = new POPUPMENUITEM;
		if (!pItem)
		{
			delete pIngameDialog;
			return LTNULL;
		}

		g_pLTClient->GetSurfaceDims (hSurf[i], &nWidth, &nHeight);

		pItem->nHeight = (int)nHeight;
		pItem->nSelectionPoint = (int)nHeight / 2;
		pItem->hSurface = hSurf[i];
		pItem->hSelected = hSurfSelected[i];
		pItem->nType = List;
		pItem->nData = nByte1 | (nByte2 << 8) | (nByte3 << 16) | (nByte4 << 24);

		pIngameDialog->AddItem (pItem);
		pItem = LTNULL;
	}

	return pIngameDialog;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleObjectives()
//
//	PURPOSE:	Handles the mission objective message
//
// ----------------------------------------------------------------------- //
		
void CRiotClientShell::HandleObjectives (ILTMessage_Read* hMessage)
{
	if (!g_pLTClient) return;

	// get the strings we need 

	HSTRING hstrAdd = hMessage->ReadHString();
	const char* strAdd = g_pLTClient->GetStringData (hstrAdd);
	
	HSTRING hstrRemove = hMessage->ReadHString();
	const char* strRemove = g_pLTClient->GetStringData (hstrRemove);
	
	HSTRING hstrComplete = hMessage->ReadHString();
	const char* strComplete = g_pLTClient->GetStringData (hstrComplete);

	// play log updated sound if the log has been updated

	if (strAdd)
	{
		PlaySoundLocal ("Sounds\\Interface\\Update.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}

	// parse out the string ids

	const char* ptr = strtok (const_cast<char *>(strAdd), " ");
	HLTCOLOR hTrans = g_pLTClient->SetupColor2 (0.0f, 0.0f, 0.0f, LTTRUE);
	while (ptr)
	{
		uint32 nID = atoi (ptr);
		m_objectives.AddObjective (nID);
		m_bNewObjective = LTTRUE;
		m_hNewObjective = g_pLTClient->CreateSurfaceFromBitmap ("interface/Log_Updated.pcx");
		g_pLTClient->OptimizeSurface (m_hNewObjective, hTrans);

		ptr = strtok (NULL, " ");
	}

	ptr = strtok (const_cast<char *>(strRemove), " ");
	while (ptr)
	{
		uint32 nID = atoi (ptr);
		m_objectives.RemoveObjective (nID);

		ptr = strtok (NULL, " ");
	}

	ptr = strtok (const_cast<char *>(strComplete), " ");
	while (ptr)
	{
		uint32 nID = atoi (ptr);
		m_objectives.CompleteObjective (nID);

		ptr = strtok (NULL, " ");
	}

	g_pLTClient->FreeString(hstrAdd);
	g_pLTClient->FreeString(hstrRemove);
	g_pLTClient->FreeString(hstrComplete);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleTransmission()
//
//	PURPOSE:	Handles the transmission message
//
// ----------------------------------------------------------------------- //
		
void CRiotClientShell::HandleTransmission (ILTMessage_Read* hMessage)
{
	ILTMessage_Write* hMsg;
	if (!g_pLTClient) return;
	char *pszImage;

	uint32	nStringID = hMessage->Readuint32();
	pszImage = pClientDE->ReadFromMessageString (hMessage);
	
	uint32 nScreenWidth, nScreenHeight;
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	g_pLTClient->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);

	if (m_hTransmissionImage) g_pLTClient->DeleteSurface (m_hTransmissionImage);
	if (m_hTransmissionText) g_pLTClient->DeleteSurface (m_hTransmissionText);

	char strImageFilename[256];
	SAFE_STRCPY(strImageFilename, "Interface\\TranPix\\");
	strcat (strImageFilename, pszImage );

	m_hTransmissionImage = g_pLTClient->CreateSurfaceFromBitmap (strImageFilename);
	HLTCOLOR hTrans = g_pLTClient->SetupColor1 (0.0f, 0.0f, 1.0f, LTTRUE);
	g_pLTClient->OptimizeSurface (m_hTransmissionImage, hTrans);

	HSTRING hstrFont = g_pLTClient->FormatString (IDS_INGAMEFONT);
	FONT fontdef (g_pLTClient->GetStringData (hstrFont), 
					TextHelperGetIntValFromStringID(g_pLTClient, IDS_TRANSMISSIONTEXTWIDTH, 6),
					TextHelperGetIntValFromStringID(g_pLTClient, IDS_TRANSMISSIONTEXTHEIGHT, 12));
	g_pLTClient->FreeString (hstrFont);

	HLTCOLOR foreColor = g_pLTClient->SetupColor1 (1.0f, 1.0f, 1.0f, LTFALSE);
	m_hTransmissionText = CTextHelper::CreateWrappedStringSurface (g_pLTClient, min (256, nScreenWidth - 90), &fontdef, nStringID, foreColor);
	hTrans = g_pLTClient->SetupColor1(0.0f, 0.0f, 0.0f, LTTRUE);
	g_pLTClient->OptimizeSurface (m_hTransmissionText, hTrans);

	if (!m_hTransmissionImage || !m_hTransmissionText)
	{
		if (m_hTransmissionImage) g_pLTClient->DeleteSurface (m_hTransmissionImage);
		if (m_hTransmissionText) g_pLTClient->DeleteSurface (m_hTransmissionText);
		m_hTransmissionImage = LTNULL;
		m_hTransmissionText = LTNULL;
		m_fTransmissionTimeLeft = 0.0f;
		return;;
	}

	// Make sure we're not already playing a transmission sound...

	if (m_hTransmissionSound)
	{
		g_pLTClient->KillSound(m_hTransmissionSound);
		m_hTransmissionSound = LTNULL;

		// Tell server the transmission ended.
		hMsg = g_pLTClient->StartMessage( MID_TRANSMISSIONENDED );
		g_pLTClient->EndMessage( hMsg );
	}

	// Play the sound streamed...

	char strSoundFilename[256];
	memset (strSoundFilename, 0, 256);
	SAFE_STRCPY(strSoundFilename, "Sounds\\Voice\\");
	ltoa ((long)nStringID, &strSoundFilename[13], 10);
	strcat (strSoundFilename, ".wav");

	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT( playSoundInfo );

	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL | PLAYSOUND_FILESTREAM | PLAYSOUND_GETHANDLE;
	strncpy(playSoundInfo.m_szSoundName, strSoundFilename, _MAX_PATH);
	playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_HIGH;
	pClientDE->PlaySound(&playSoundInfo);
	m_hTransmissionSound = playSoundInfo.m_hSound;

	// Make sure the sound played.
	if( !m_hTransmissionSound )
	{
		// Tell server the transmission ended.
		hMsg = g_pLTClient->StartMessage( MID_TRANSMISSIONENDED );
		g_pLTClient->EndMessage( hMsg );
	}

	m_fTransmissionTimeLeft = 5.0f;
	m_bAnimatingTransmissionOn = LTTRUE;
	m_bAnimatingTransmissionOff = LTFALSE;

	uint32 nWidth = 0;
	uint32 nHeight = 0;

	g_pLTClient->GetSurfaceDims (m_hTransmissionImage, &nWidth, &nHeight);
	m_xTransmissionImage = -(LTFLOAT)nWidth;
	m_yTransmissionImage = 6;
	m_cxTransmissionImage = (LTFLOAT)nWidth;
	m_cyTransmissionImage = (LTFLOAT)nHeight;
	
	g_pLTClient->GetSurfaceDims (m_hTransmissionText, &nWidth, &nHeight);
	m_xTransmissionText = 80;
	m_yTransmissionText = -(LTFLOAT)nHeight;
	m_cxTransmissionText = (LTFLOAT)nWidth;
	m_cyTransmissionText = (LTFLOAT)nHeight;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::PlayIntroMovies()
//
//	PURPOSE:	Plays the intro movies in the correct order
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::PlayIntroMovies (ILTClient* pClientDE)
{
	if (!g_pLTClient) return;

	static int nMovie = 0;
	
	LTRESULT nResult = LT_ERROR;
	uint32 nFlags = PLAYBACK_FULLSCREEN;

	// Changed to look in current directory if the movie path is invalid...

	if (/*m_strMoviesDir[0] == '\0' ||*/ AdvancedDisableMovies())
	{
		nMovie = 2;	// fall through into default handler below
	}

#ifdef _DEMO
	nMovie = 2;
#endif

	switch (nMovie)
	{
		case 0:
		{
			nMovie++;

			// see if the movie exists
			char strPath[256];
			SAFE_STRCPY(strPath, m_strMoviesDir);
			strcat (strPath, "Logo.smk");
			if (CWinUtil::FileExist (strPath))
			{
				// attempt to play the movie
				nResult = g_pLTClient->StartVideo (strPath, nFlags);
			}

			if (nResult != LT_OK)
			{
				PlayIntroMovies (pClientDE);
				return;
			}
			
			m_nGameState = GS_MOVIES;
			return;
		}
		break;

		case 1:
		{
			nMovie++;

			// see if the movie exists
			char strPath[256];
			SAFE_STRCPY(strPath, m_strMoviesDir);
			strcat (strPath, "Intro.smk");
			if (CWinUtil::FileExist (strPath))
			{
				// attempt to play the movie
				nResult = g_pLTClient->StartVideo (strPath, nFlags);
			}
			
			if (nResult != LT_OK)
			{
				PlayIntroMovies (pClientDE);
				return;
			}
			
			m_nGameState = GS_MOVIES;
			return;
		}
		break;
		
		default:
		{
			SetMenuMode (LTTRUE, LTFALSE);
			return;
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::PauseGame()
//
//	PURPOSE:	Pauses/Unpauses the server
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::PauseGame (LTBOOL bPause, LTBOOL bPauseSound)
{
	if (!g_pLTClient) return;

	m_bGamePaused = bPause;

	if (!IsMultiplayerGame())
	{
		ILTMessage_Write* hMessage = pClientDE->StartMessage(bPause ? MID_GAME_PAUSE : MID_GAME_UNPAUSE);
		pClientDE->EndMessage(hMessage);
	}

	if (bPause && bPauseSound)
	{
		g_pLTClient->PauseSounds();
	}
	else
	{
		g_pLTClient->ResumeSounds();
	}

	SetInputState (!bPause);
	SetMouseInput (!bPause);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::SetInputState()
//
//	PURPOSE:	Allows/disallows input
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::SetInputState(LTBOOL bAllowInput)
{
	if (!g_pLTClient) return;

	g_pLTClient->SetInputState(bAllowInput);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::SetMouseInput()
//
//	PURPOSE:	Allows or disallows mouse input on the client
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::SetMouseInput (LTBOOL bAllowInput)
{
	if (!g_pLTClient) return;

	if (bAllowInput)
	{
		m_bRestoreOrientation = LTTRUE;
	}
	else
	{
		m_fYawBackup = m_fYaw;
		m_fPitchBackup = m_fPitch;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandlePlayerStateChange()
//
//	PURPOSE:	Update player state change
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::HandlePlayerStateChange(ILTMessage_Read* hMessage)
{
	if (!g_pLTClient) return;
	
	m_ePlayerState = (PlayerState) hMessage->Readuint8();

	if (m_ePlayerState == PS_DYING)
	{
		ClearScreenTint();

		if (IsMultiplayerGame())
		{
			m_fEarliestRespawnTime = g_pLTClient->GetTime() + MULTIPLAYER_RESPAWN_WAIT_TIME;
		}
		else
		{
			m_fEarliestRespawnTime = g_pLTClient->GetTime() + RESPAWN_WAIT_TIME;
		}

		HLOCALOBJ hWeapon = m_weaponModel.GetHandle();
		if (hWeapon)
		{
			uint32 dwFlags;
			g_pLTClient->Common()->GetObjectFlags(hWeapon, OFT_Flags, dwFlags);
			dwFlags &= ~FLAG_VISIBLE;
			g_pLTClient->Common()->SetObjectFlags(hWeapon, OFT_Flags, dwFlags, FLAGMASK_ALL);
		}

		m_weaponModel.Reset();
		
		m_bDrawInterface = LTFALSE;
		AddToClearScreenCount();
	
		if (m_playerCamera.IsFirstPerson())
		{
			SetExternalCamera(LTTRUE);
		}

		HSTRING hStr = g_pLTClient->FormatString (IDS_YOUWEREKILLED);
		g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));

		m_infoDisplay.AddInfo(g_pLTClient->GetStringData (hStr), m_menu.GetFont12s(), 5.0f, DI_CENTER | DI_TOP);

		g_pLTClient->FreeString(hStr);
	}
	else if (m_ePlayerState == PS_ALIVE)
	{
		if (m_nPlayerMode == PM_MODE_KID)
		{
			m_bDrawInterface = LTFALSE;
		}
		else
		{
			m_bDrawInterface = LTTRUE;
		}

		m_bDrawHud = LTTRUE;

		SetExternalCamera(LTFALSE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::AutoSave()
//
//	PURPOSE:	Autosave the game
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::AutoSave(ILTMessage_Read* hMessage)
{
	if (m_ePlayerState != PS_ALIVE || IsMultiplayerGame()) return;

	// Save the game for reloading if the player just changed levels
	// or if the player just started a game...

	CWinUtil::WinWritePrivateProfileString("Shogo", "Reload", m_strCurrentWorldName, SAVEGAMEINI_FILENAME);
	SaveGame(RELOADLEVEL_FILENAME);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandlePlayerDamage
//
//	PURPOSE:	Handle the player getting damaged
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::HandlePlayerDamage(ILTMessage_Read* hMessage)
{
	if (!g_pLTClient || !m_hCamera || !hMessage) return;

	LTVector vDir;
	vDir = hMessage->ReadLTVector();

	LTFLOAT fPercent = VEC_MAG(vDir);

	LTFLOAT fRed = 0.5f + fPercent;
	fRed = fRed > 1.0f ? 1.0f : fRed;

	LTFLOAT fRampDown = 0.5f + fPercent * 2.0f;

	LTVector vTintColor;
	VEC_SET(vTintColor, fRed, 0.0f, 0.0f);
	LTFLOAT fRampUp = 0.2f, fTintTime = 0.1f;
	
	LTVector vCamPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

	LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

	LTVector vU, vR, vF;
	g_pLTClient->Math()->GetRotationVectors(rRot, vU, vR, vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

	TintScreen(vTintColor, vCamPos, 1000.0f, fRampUp, fTintTime, fRampDown, LTTRUE);


	// Shake the camera based on the direction the damage came from...
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleServerError()
//
//	PURPOSE:	Handle any error messages sent from the server
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::HandleServerError(ILTMessage_Read* hMessage)
{
	if (!g_pLTClient || !hMessage) return;

	uint8 nError = hMessage->Readuint8();
	switch (nError)
	{
		case SERROR_SAVEGAME :
		{
			DoMessageBox(IDS_SAVEGAMEFAILED, TH_ALIGN_CENTER);
		}
		break;

		case SERROR_LOADGAME :
		{
			DoMessageBox(IDS_NOLOADLEVEL, TH_ALIGN_CENTER);
			m_menu.LoadAllSurfaces();
			m_nGameState = GS_MENU;  // Force the menu up
		}
		break;
		
		default : break;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateWeaponMuzzlePosition()
//
//	PURPOSE:	Update the current weapon muzzle pos
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateWeaponMuzzlePosition()
{
	if (!g_pLTClient) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;
	
	LTFLOAT fIncValue = 0.1f;
	LTBOOL bChanged = LTFALSE;

	LTVector vOffset;
	VEC_INIT(vOffset);


	// Move weapon faster if running...

	if (m_dwPlayerFlags &  CS_MFLG_RUN)
	{
		fIncValue = fIncValue * 2.0f;
	}


	vOffset = m_weaponModel.GetMuzzleOffset();


	// Move weapon forward or backwards...

	if ((m_dwPlayerFlags & CS_MFLG_FORWARD) || (m_dwPlayerFlags & CS_MFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_FORWARD ? fIncValue : -fIncValue;
		vOffset.z += fIncValue;
		bChanged = LTTRUE;
	}


	// Move the weapon to the player's right or left...

	if ((m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT) || 
		(m_dwPlayerFlags & CS_MFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vOffset.x += fIncValue;
		bChanged = LTTRUE;
	}


	// Move the weapon up or down relative to the player...

	if ((m_dwPlayerFlags & CS_MFLG_JUMP) || (m_dwPlayerFlags & CS_MFLG_DOUBLEJUMP) || (m_dwPlayerFlags & CS_MFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_DUCK ? -fIncValue : fIncValue;
		vOffset.y += fIncValue;
		bChanged = LTTRUE;
	}


	// Okay, set the offset...

	m_weaponModel.SetMuzzleOffset(vOffset);

	if (bChanged)
	{
		CSPrint ("Muzzle offset = %f, %f, %f", vOffset.x, vOffset.y, vOffset.z);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateWeaponPosition()
//
//	PURPOSE:	Update the position of the current weapon
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateWeaponPosition()
{
	if (!g_pLTClient) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;
	
	LTFLOAT fIncValue = 0.1f;
	LTBOOL bChanged = LTFALSE;

	LTVector vOffset;
	VEC_INIT(vOffset);


	// Move weapon faster if running...

	if (m_dwPlayerFlags &  CS_MFLG_RUN)
	{
		fIncValue = fIncValue * 2.0f;
	}


	vOffset = m_weaponModel.GetOffset();


	// Move weapon forward or backwards...

	if ((m_dwPlayerFlags & CS_MFLG_FORWARD) || (m_dwPlayerFlags & CS_MFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_FORWARD ? fIncValue : -fIncValue;
		vOffset.z += fIncValue;
		bChanged = LTTRUE;
	}


	// Move the weapon to the player's right or left...

	if ((m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT) || 
		(m_dwPlayerFlags & CS_MFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vOffset.x += fIncValue;
		bChanged = LTTRUE;
	}


	// Move the weapon up or down relative to the player...

	if ((m_dwPlayerFlags & CS_MFLG_JUMP) || (m_dwPlayerFlags & CS_MFLG_DOUBLEJUMP) || (m_dwPlayerFlags & CS_MFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_DUCK ? -fIncValue : fIncValue;
		vOffset.y += fIncValue;
		bChanged = LTTRUE;
	}


	// Okay, set the offset...

	m_weaponModel.SetOffset(vOffset);

	if (bChanged)
	{
		CSPrint ("Weapon offset = %f, %f, %f", vOffset.x, vOffset.y, vOffset.z);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::AdjustLightScale()
//
//	PURPOSE:	Update the current global light scale
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::AdjustLightScale()
{
	if (!g_pLTClient) return;

	LTFLOAT fIncValue = 0.01f;
	LTBOOL bChanged = LTFALSE;

	LTVector vScale;
	VEC_INIT(vScale);

	g_pLTClient->GetGlobalLightScale(&vScale);

	// Move faster if running...

	if (m_dwPlayerFlags & CS_MFLG_RUN)
	{
		fIncValue = .5f;
	}


	// Move Red up/down...

	if ((m_dwPlayerFlags & CS_MFLG_FORWARD) || (m_dwPlayerFlags & CS_MFLG_REVERSE))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_FORWARD ? fIncValue : -fIncValue;
		vScale.x += fIncValue;
		vScale.x = vScale.x < 0.0f ? 0.0f : (vScale.x > 1.0f ? 1.0f : vScale.x);

		bChanged = LTTRUE;
	}


	// Move Green up/down...

	if ((m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT) || 
		(m_dwPlayerFlags & CS_MFLG_STRAFE_LEFT))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_STRAFE_RIGHT ? fIncValue : -fIncValue;
		vScale.y += fIncValue;
		vScale.y = vScale.y < 0.0f ? 0.0f : (vScale.y > 1.0f ? 1.0f : vScale.y);

		bChanged = LTTRUE;
	}


	// Move Blue up/down...

	if ((m_dwPlayerFlags & CS_MFLG_JUMP) || (m_dwPlayerFlags & CS_MFLG_DOUBLEJUMP) || (m_dwPlayerFlags & CS_MFLG_DUCK))
	{
		fIncValue = m_dwPlayerFlags & CS_MFLG_DUCK ? -fIncValue : fIncValue;
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
//	ROUTINE:	CRiotClientShell::SpecialEffectNotify()
//
//	PURPOSE:	Handle creation of a special fx
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::SpecialEffectNotify(HLOCALOBJ hObj, ILTMessage_Read* hMessage)
{
	if (!g_pLTClient) return;

	if (hObj)
	{
		g_pLTClient->Common()->SetObjectFlags(hObj, OFT_Client, CF_NOTIFYREMOVE, FLAGMASK_ALL);
	}

	m_sfxMgr.HandleSFXMsg(hObj, hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnObjectRemove()
//
//	PURPOSE:	Handle removal of an object...
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::OnObjectRemove(HLOCALOBJ hObj)
{
	if (!hObj) return;

	m_sfxMgr.RemoveSpecialFX(hObj);
	
	if(m_MoveMgr)
	{
		m_MoveMgr->OnObjectRemove(hObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleExitLevel()
//
//	PURPOSE:	Update player state change
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::HandleExitLevel(ILTMessage_Read* hMessage)
{
	if (!g_pLTClient) return;
	
	// Get the new world

	HSTRING	hstrWorld		 = hMessage->ReadHString();
	HSTRING hstrBumperScreen = hMessage->ReadHString();
	uint32  nBumperTextID	 = hMessage->Readuint32();
	LTBOOL	bEndOfScenario	 = (LTBOOL)hMessage->Readuint8();

	if (!hstrWorld || !hstrBumperScreen)
	{
		PrintError("Invalid data in CRiotClientShell::HandleExitLevel()!");
		return;
	}

	g_pLTClient->ClearInput(); // Start next level with a clean slate

	const char* pStr = g_pLTClient->GetStringData(hstrBumperScreen);

	TurnOffAlternativeCamera();

	CreateBumperScreen(const_cast<char *>(pStr), nBumperTextID);
	StartNewWorld(hstrWorld);

	m_bInWorld = LTFALSE;

	g_pLTClient->FreeString(hstrWorld);
	g_pLTClient->FreeString(hstrBumperScreen);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::CreateBumperScreen()
//
//	PURPOSE:	Create the bumper screen image
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::CreateBumperScreen(char* pPCXName, uint32 dwBumperTextID)
{
	if (!g_pLTClient || !pPCXName) return;

	// Make sure we remove any old bumper screen stuff...

	RemoveBumperScreen();
	m_bLoadingWorld = LTTRUE;


	// Get the screen width and height...

	if (dwBumperTextID)
	{
		uint32 nWidth, nHeight;
		HSURFACE hScreen = g_pLTClient->GetScreenSurface();
		pClientDE->GetSurfaceDims(hScreen, &nWidth, &nHeight);
	
		if (nWidth < 640)
		{
			m_hBumperText = CTextHelper::CreateWrappedStringSurface(pClientDE, (min ((int)nWidth - 20, 600)), m_menu.GetFont08s(), dwBumperTextID);
		}
		else
		{
			m_hBumperText = CTextHelper::CreateWrappedStringSurface(pClientDE, (min ((int)nWidth - 40, 600)), m_menu.GetFont12s(), dwBumperTextID);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::RemoveBumperScreen()
//
//	PURPOSE:	Remove the bumper screen image / text
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::RemoveBumperScreen()
{
	if (!g_pLTClient) return;

	if (m_hBumperText) 
	{
		g_pLTClient->DeleteSurface (m_hBumperText);
		m_hBumperText = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::StartNewWorld()
//
//	PURPOSE:	Start the new world
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::StartNewWorld(HSTRING hstrWorld)
{
	if (!g_pLTClient || !hstrWorld) return;

	const char* pWorld = g_pLTClient->GetStringData(hstrWorld);
	if (!pWorld)
	{
		PrintError("Error invalid world!");
		return;
	}


	// Load the new level...

	char newWorld[100];
	sprintf(newWorld, "worlds\\%s", pWorld);

	if (!LoadWorld(newWorld, LTNULL, LTNULL, LOAD_NEW_LEVEL))
	{
		SetMenuMode (LTTRUE, LTFALSE);
		DoMessageBox (IDS_NOLOADLEVEL, TH_ALIGN_CENTER);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateLoadingLevel()
//
//	PURPOSE:	Update the level loading screen
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateLoadingLevel()
{
	if (!g_pLTClient || (!m_hBumperText && !m_hLoading)) return;

	// Start the loading level music...

	SetMenuMusic(LTTRUE);


	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nScreenWidth = 0;
	uint32 nScreenHeight = 0;
	g_pLTClient->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);

	uint32 nWidth = 0;
	uint32 nHeight = 0;
	
	if (m_hBumperText)
	{
		if (m_hBumperText)
		{
			g_pLTClient->GetSurfaceDims(m_hBumperText, &nWidth, &nHeight);
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hBumperText, LTNULL, ((int)(nScreenWidth - nWidth)) / 2, ((int)nScreenHeight - (int)nHeight) / 2, LTNULL);
		}

		if (m_bLoadingWorld)
		{
			if (m_hWorldName)
			{
				g_pLTClient->GetSurfaceDims(m_hWorldName, &nWidth, &nHeight);
				g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hWorldName, LTNULL, ((int)(nScreenWidth - nWidth)) / 2, (int)m_cyPressAnyKey, LTNULL);
			}

			if (m_hLoadingWorld)
			{
				g_pLTClient->GetSurfaceDims(m_hLoadingWorld, &nWidth, &nHeight);
				g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hLoadingWorld, LTNULL, ((int)(nScreenWidth - nWidth)) / 2, (int)nScreenHeight - (int)m_cyPressAnyKey, LTNULL);
			}
		}
	}
	else if (m_hLoading)
	{
		g_pLTClient->DrawSurfaceToSurfaceTransparent (hScreen, m_hLoading, LTNULL, ((int)(nScreenWidth - m_cxLoading)) / 2, ((int)(nScreenHeight - m_cyLoading)) / 2, LTNULL);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::PrintError()
//
//	PURPOSE:	Display an error to the end-user
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::PrintError(char* pError)
{
	if (!g_pLTClient || !pError) return;

	CSPrint (pError);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::SetSpectatorMode()
//
//	PURPOSE:	Turn spectator mode on/off
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::SetSpectatorMode(LTBOOL bOn)
{
	m_bSpectatorMode = bOn;

	if (m_playerCamera.IsFirstPerson())
	{
		m_weaponModel.SetVisible(!m_bSpectatorMode);
		ShowPlayer(LTFALSE);
		
		if(m_MoveMgr)
			m_MoveMgr->SetSpectatorMode(bOn);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::LoadWorld()
//
//	PURPOSE:	Handles loading a world (with AutoSave)
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::LoadWorld(char* pWorldFile, char* pCurWorldSaveFile,
								  char* pRestoreObjectsFile, uint8 nFlags)
{
	// Auto save the newly loaded level...

	return DoLoadWorld(pWorldFile, pCurWorldSaveFile, pRestoreObjectsFile, nFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::DoLoadWorld()
//
//	PURPOSE:	Does actual work of loading a world
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::DoLoadWorld(char* pWorldFile, char* pCurWorldSaveFile,
								    char* pRestoreObjectsFile, uint8 nFlags, 
									char *pRecordFile, char *pPlaydemoFile)
{
	if (!g_pLTClient || !pWorldFile) return LTFALSE;
	

	// Make sure the FOV is set correctly...

	if (m_hCamera)
	{
		uint32 dwWidth = 640, dwHeight = 480;
		g_pLTClient->GetSurfaceDims(pClientDE->GetScreenSurface(), &dwWidth, &dwHeight);

		g_pLTClient->SetCameraRect(m_hCamera, LTFALSE, 0, 0, dwWidth, dwHeight);
		
		m_fCurrentFovX = DEG2RAD(FOV_NORMAL);

		LTFLOAT y = (m_fCurrentFovX * dwHeight) / dwWidth;
		g_pLTClient->SetCameraFOV(m_hCamera, m_fCurrentFovX, y);
	}
	

	// See if the game is over...

	if (pWorldFile && (strcmpi(pWorldFile, "end") == 0 || strcmpi(pWorldFile, "worlds\\end") == 0))
	{
		m_bGameOver = LTTRUE;
	}
	else
	{
		m_bGameOver = LTFALSE;
	}


	// Set us to the loading state...

	m_nGameState = GS_LOADINGLEVEL;

	
	// Create the "loading" surface...

	if (m_hLoadingWorld)
	{
		g_pLTClient->DeleteSurface (m_hLoadingWorld);
		m_hLoadingWorld = LTNULL;
	}

	if (m_hWorldName)
	{
		g_pLTClient->DeleteSurface (m_hWorldName);
		m_hWorldName = LTNULL;
	}

	char worldname[51];
	worldname[0] = '\0';
	GetNiceWorldName(pWorldFile, worldname, 50);

	if (worldname[0])
	{
		m_hWorldName = CTextHelper::CreateSurfaceFromString(g_pLTClient, m_menu.GetFont12s(), worldname);
	}

	m_hLoadingWorld = CTextHelper::CreateSurfaceFromString(g_pLTClient, m_menu.GetFont12s(), IDS_BUMPER_LOADING);
	
	
	// Bring up the loading screen...

	g_pLTClient->ClearScreen (LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	g_pLTClient->Start3D();

	CreateMenuPolygrid();
	UpdateMenuPolygrid();

	g_pLTClient->StartOptimized2D();
	UpdateLoadingLevel();
	g_pLTClient->EndOptimized2D();

	g_pLTClient->End3D();
	g_pLTClient->FlipScreen (FLIPSCREEN_CANDRAWCONSOLE);

	
	// Check for special case of not being connected to a server or going to 
	// single player mode from multiplayer...

	int nGameMode = 0;
	g_pLTClient->GetGameMode(&nGameMode);
	if (pPlaydemoFile || 
		!g_pLTClient->IsConnected() || (nGameMode != STARTGAME_NORMAL && nGameMode != GAMEMODE_NONE))
	{
		StartGameRequest request;
		memset(&request, 0, sizeof(StartGameRequest));
	
		NetStart_ClearGameStruct();  // Start with clean slate
		request.m_pGameInfo   = NetStart_GetGameStruct();
		request.m_GameInfoLen = sizeof(NetGame_t);
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

		LTRESULT dr = pClientDE->StartGame(&request);
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

	// Since we're loading a game, reset the music level to silence...
	// (m_eMusicLevel is used in BuildClientSaveMsg())...

	m_eMusicLevel = CMusic::MUSICLEVEL_SILENCE;

	
	// Get rid of any mission objectives we currently have

	m_objectives.Reset();
	
	// Send a message to the server shell with the needed info...

	HSTRING hWorldFile			= g_pLTClient->CreateString(pWorldFile);
	HSTRING hCurWorldSaveFile	= g_pLTClient->CreateString(pCurWorldSaveFile ? pCurWorldSaveFile : " ");
	HSTRING hRestoreObjectsFile	= g_pLTClient->CreateString(pRestoreObjectsFile ? pRestoreObjectsFile : " ");

	ILTMessage_Write* hMessage = g_pLTClient->StartMessage(MID_LOAD_GAME);
	g_pLTClient->WriteToMessageByte(hMessage, nFlags);
	g_pLTClient->WriteToMessageByte(hMessage, m_eDifficulty);
	g_pLTClient->WriteToMessageHString(hMessage, hWorldFile);
	g_pLTClient->WriteToMessageHString(hMessage, hCurWorldSaveFile);
	g_pLTClient->WriteToMessageHString(hMessage, hRestoreObjectsFile);

	BuildClientSaveMsg(hMessage);

	g_pLTClient->EndMessage(hMessage);

	g_pLTClient->FreeString(hWorldFile);
	g_pLTClient->FreeString(hCurWorldSaveFile);
	g_pLTClient->FreeString(hRestoreObjectsFile);


	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::LoadGame()
//
//	PURPOSE:	Handles loading a saved game
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::LoadGame(char* pWorld, char* pObjectsFile)
{
	if (!g_pLTClient || !pWorld || !pObjectsFile) return LTFALSE;
		
	char fullWorldPath[100];
	sprintf(fullWorldPath,"worlds\\%s", pWorld);

	return DoLoadWorld(fullWorldPath, LTNULL, pObjectsFile, LOAD_RESTORE_GAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::SaveGame()
//
//	PURPOSE:	Handles saving a game...
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::SaveGame(char* pObjectsFile)
{
	if (!g_pLTClient || !pObjectsFile || IsMultiplayerGame()) return LTFALSE;
	
	uint8 nFlags = 0;

	// Since we're saving the game, save the music level...
	// (m_eMusicLevel is used in BuildClientSaveMsg())...

	m_eMusicLevel = m_Music.GetMusicLevel();

	
	// Save the level objects...

	HSTRING hSaveObjectsName = g_pLTClient->CreateString(pObjectsFile);

	ILTMessage_Write* hMessage = g_pLTClient->StartMessage(MID_SAVE_GAME);
	g_pLTClient->WriteToMessageByte(hMessage, nFlags);
	g_pLTClient->WriteToMessageHString(hMessage, hSaveObjectsName);

	BuildClientSaveMsg(hMessage);

	g_pLTClient->EndMessage(hMessage);

	g_pLTClient->FreeString(hSaveObjectsName);

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
// Clears the screen a few times so the backbuffer(s) get cleared.
// ----------------------------------------------------------------------- //
void CRiotClientShell::ClearAllScreenBuffers()
{
	int i;

	for(i=0; i < 4; i++)
	{
		g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN);
		g_pLTClient->FlipScreen(0);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::SetMenuMode()
//
//	PURPOSE:	Turns menu mode on and off
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::SetMenuMode (LTBOOL bMenuUp, LTBOOL bLoadingLevel)
{
	if (!g_pLTClient) return LTFALSE;

	uint32 nWidth, nHeight;
	g_pLTClient->GetSurfaceDims (g_pLTClient->GetScreenSurface(), &nWidth, &nHeight);

	if (bMenuUp && m_nGameState != GS_MENU)
	{
		if (m_bUsingExternalCamera)
		{
			TurnOffAlternativeCamera();
			m_bCameraPosInited = LTFALSE;
		}	

		// Make sure menus are full screen...

		memset (&m_rcMenuRestoreCamera, 0, sizeof (LTRect));
		if (m_hCamera && !m_bMovieCameraRect)
		{
			g_pLTClient->GetCameraRect (m_hCamera, &m_bMenuRestoreFullScreen, &m_rcMenuRestoreCamera.left, &m_rcMenuRestoreCamera.top, &m_rcMenuRestoreCamera.right, &m_rcMenuRestoreCamera.bottom);
			g_pLTClient->SetCameraRect (m_hCamera, LTTRUE, 0, 0, (int)nWidth, (int)nHeight);
		}
		
		m_nGameState = GS_MENU;
		PauseGame (LTTRUE, LTTRUE);
		SetMenuMusic (LTTRUE);
		ClearScreenAlways();
		
		m_menu.LoadAllSurfaces();
	}
	else if (!bMenuUp)
	{
		if (m_hCamera && (m_rcMenuRestoreCamera.left != 0 || m_rcMenuRestoreCamera.top != 0 || m_rcMenuRestoreCamera.right != 0 || m_rcMenuRestoreCamera.bottom != 0))
		{
			g_pLTClient->SetCameraRect (m_hCamera, m_bMenuRestoreFullScreen, m_rcMenuRestoreCamera.left, m_rcMenuRestoreCamera.top, m_rcMenuRestoreCamera.right, m_rcMenuRestoreCamera.bottom);
		}

		int nGameMode = GAMEMODE_NONE;
		g_pLTClient->GetGameMode (&nGameMode);
		if (nGameMode == GAMEMODE_NONE) return LTFALSE;

		ClearScreenAlways (LTFALSE);
		AddToClearScreenCount();
		
		if (!bLoadingLevel)
		{
			m_nGameState = GS_PLAYING;
			PauseGame (LTFALSE);
			SetMenuMusic (LTFALSE);
		}
		
		m_menu.UnloadAllSurfaces();
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::SetLoadGameMenu()
//
//	PURPOSE:	Turn the load game menu on
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::SetLoadGameMenu()
{
	if (!g_pLTClient) return;

	SetMenuMode(LTTRUE);

	// Make sure we're on the load menu...

	CMainMenu* pMain = m_menu.GetMainMenu();
	if (pMain)
	{
		CSinglePlayerMenu* pSingle = pMain->GetSinglePlayerMenu();
		if (pSingle)
		{
			CLoadSavedLevelMenu* pLoad = pSingle->GetLoadSavedLevelMenu();
			if (pLoad)
			{
				m_menu.SetCurrentMenu(pLoad);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::SetMenuMusic()
//
//	PURPOSE:	Turns menu / load level music on or off
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::SetMenuMusic(LTBOOL bMusicOn)
{
	if (!g_pLTClient) return LTFALSE;
	
	if (bMusicOn)
	{
		if (!m_bGameMusicPaused)
		{
			g_pLTClient->PauseMusic();
			m_bGameMusicPaused = LTTRUE;
		}

		if (!m_hMenuMusic)
		{
			// Set up the music...

			PlaySoundInfo psi;
			PLAYSOUNDINFO_INIT (psi);
			psi.m_dwFlags = PLAYSOUND_LOCAL | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT | PLAYSOUND_FILESTREAM;

			char* s_pLoadingMusic[] = 
			{
				"Sounds\\Interface\\loop1.wav",
				"Sounds\\Interface\\loop2.wav",
				"Sounds\\Interface\\loop3.wav",
				"Sounds\\Interface\\loop4.wav",
				"Sounds\\Interface\\loop5.wav",
				"Sounds\\Interface\\loop6.wav"
			};

#ifdef _DEMO
			char* pMusic = s_pLoadingMusic[0];
#else
			char* pMusic = s_pLoadingMusic[GetRandom(0, 5)];
#endif
			SAFE_STRCPY(psi.m_szSoundName, pMusic);

			psi.m_nPriority  = SOUNDPRIORITY_MISC_HIGH;

			g_pLTClient->PlaySound (&psi);
			m_hMenuMusic = psi.m_hSound;
		}

		return !!m_hMenuMusic;
	}
	else
	{
		if (m_bGameMusicPaused)
		{
			g_pLTClient->ResumeMusic();
			m_bGameMusicPaused = LTFALSE;
		}

		if (m_hMenuMusic) 
		{
			g_pLTClient->KillSound(m_hMenuMusic);
			m_hMenuMusic = LTNULL;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::DoMessageBox()
//
//	PURPOSE:	Displays a message box to the user in the center of the
//				screen
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::DoMessageBox (int nStringID, int nAlignment, LTBOOL bCrop)
{
	if (!g_pLTClient) return LTFALSE;

	if (m_pMessageBox) return LTFALSE;

	m_pMessageBox = new CMessageBox;
	if (!m_pMessageBox->Init (g_pLTClient, nStringID, LTFALSE, nAlignment, bCrop))
	{
		delete m_pMessageBox;
		m_pMessageBox = LTNULL;
		return LTFALSE;
	}

	if (m_nGameState != GS_MENU)
	{
		PauseGame(LTTRUE, LTTRUE);
	}
	m_bClearScreenAlways = LTFALSE;
	ZeroClearScreenCount();
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::DoYesNoMessageBox()
//
//	PURPOSE:	Displays a message box to the user in the center of the
//				screen
//
// ----------------------------------------------------------------------- //

LTBOOL CRiotClientShell::DoYesNoMessageBox (int nStringID, YESNOPROC pYesNoProc, uint32 nUserData, int nAlignment, LTBOOL bCrop)
{
	if (!g_pLTClient) return LTFALSE;

	if (m_pMessageBox) return LTFALSE;

	m_pMessageBox = new CMessageBox;
	if (!m_pMessageBox->Init (g_pLTClient, nStringID, LTTRUE, nAlignment, bCrop))
	{
		delete m_pMessageBox;
		m_pMessageBox = LTNULL;
		return LTFALSE;
	}

	m_pYesNoProc = pYesNoProc;
	m_nYesNoUserData = nUserData;

	if (m_nGameState != GS_MENU)
	{
		PauseGame(LTTRUE, LTTRUE);
	}
	m_bClearScreenAlways = LTFALSE;
	ZeroClearScreenCount();
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::IsJoystickEnabled()
//
//	PURPOSE:	Determines whether or not there is a joystick device 
//				enabled
//
// ----------------------------------------------------------------------- //

bool CRiotClientShell::IsJoystickEnabled()
{
	if (!g_pLTClient) return LTFALSE;

	// first attempt to find a joystick device

	char strJoystick[128];
	memset (strJoystick, 0, 128);
	LTRESULT result = g_pLTClient->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
	if (result != LT_OK) return LTFALSE;

	// ok - we found the device and have a name...see if it's enabled

	bool bEnabled = false;
	g_pLTClient->IsDeviceEnabled (strJoystick, &bEnabled);

	return bEnabled;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::EnableJoystick()
//
//	PURPOSE:	Attempts to find and enable a joystick device
//
// ----------------------------------------------------------------------- //

bool CRiotClientShell::EnableJoystick()
{
	if (!g_pLTClient) return LTFALSE;

	// first attempt to find a joystick device

	char strJoystick[128];
	memset (strJoystick, 0, 128);
	LTRESULT result = g_pLTClient->GetDeviceName (DEVICETYPE_JOYSTICK, strJoystick, 127);
	if (result != LT_OK) return LTFALSE;

	// ok, now try to enable the device

	char strConsole[256];
	sprintf (strConsole, "EnableDevice \"%s\"", strJoystick);
	g_pLTClient->RunConsoleString (strConsole);

	bool bEnabled = false;
	g_pLTClient->IsDeviceEnabled (strJoystick, &bEnabled);

	return bEnabled;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateDebugInfo()
//
//	PURPOSE:	Update debugging info.
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdateDebugInfo()
{
	if (!g_pLTClient || !m_hCamera) return;

	if (m_hDebugInfo)
	{
		g_pLTClient->DeleteSurface(m_hDebugInfo);
		m_hDebugInfo = NULL;
	}

	char buf[100];
	buf[0] = '\0';
	
	// Check to see if we should show the player position...

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_bShowPlayerPos && hPlayerObj)
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos(hPlayerObj, &vPos);

		sprintf(buf, "Pos (%.0f, %.0f, %.0f)", vPos.x, vPos.y, vPos.z);
	}

	if (buf[0])
	{
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
			g_pLTClient->RemoveObject(m_hBoundingBox);
			m_hBoundingBox = LTNULL;
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::CreateDebugSurface
//
//	PURPOSE:	Create a surface with debug info on it.
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::CreateDebugSurface(char* strMessage)
{
	if (!g_pLTClient || !strMessage || strMessage[0] == '\0') return;

	HSTRING hstrFont = g_pLTClient->FormatString (IDS_INGAMEFONT);
	HLTFONT hFont = g_pLTClient->CreateFont (g_pLTClient->GetStringData(hstrFont), 9, 18, LTFALSE, LTFALSE, LTFALSE);
	g_pLTClient->FreeString (hstrFont);

	HLTCOLOR hFColor = g_pLTClient->SetupColor1(1.0f, 1.0f, 1.0f, LTFALSE);
	HLTCOLOR hTrans  = g_pLTClient->SetupColor2 (0.0f, 0.0f, 0.0f, LTTRUE);
	m_hDebugInfo     = CTextHelper::CreateSurfaceFromString(g_pLTClient, hFont, strMessage, hFColor);
	g_pLTClient->OptimizeSurface (m_hDebugInfo, hTrans);

	g_pLTClient->DeleteFont(hFont);

	uint32 cx, cy;
	g_pLTClient->GetSurfaceDims(m_hDebugInfo, &cx, &cy);
	m_rcDebugInfo.left   = 0;
	m_rcDebugInfo.top    = 0;
	m_rcDebugInfo.right  = (int)cx;
	m_rcDebugInfo.bottom = (int)cy;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ToggleDebugCheat
//
//	PURPOSE:	Handle debug cheat toggles
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::ToggleDebugCheat(CheatCode eCheat)
{
	if (!g_pLTClient ) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;
	
	switch (eCheat)
	{
		case CHEAT_POSWEAPON_MUZZLE :
		{
			if (!m_bSpectatorMode)
			{
				m_bTweakingWeaponMuzzle = !m_bTweakingWeaponMuzzle;
				m_bAllowPlayerMovement  = !m_bTweakingWeaponMuzzle;
			}
		}
		break;

		case CHEAT_POSWEAPON :
		{
			if (!m_bSpectatorMode)
			{
				m_bTweakingWeapon		= !m_bTweakingWeapon;
				m_bAllowPlayerMovement	= !m_bTweakingWeapon;
			}
		}
		break;

		case CHEAT_LIGHTSCALE :
		{
			m_bAdjustLightScale = !m_bAdjustLightScale;

			SetInputState(!m_bAdjustLightScale);
		}
		break;

		default : break;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::FirstUpdate
//
//	PURPOSE:	Handle first update (each level)
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::FirstUpdate()
{
	if (!g_pLTClient || !m_bFirstUpdate) return;

	char buf[200];
	m_bFirstUpdate = LTFALSE;


	// Initialize model warble sheeyot...

	g_pLTClient->RunConsoleString("+ModelWarble 0");
	g_pLTClient->RunConsoleString("+WarbleSpeed 15");
	g_pLTClient->RunConsoleString("+WarbleScale .95");

	
	// Set up the panning sky values

	m_bPanSky = (LTBOOL) g_pLTClient->GetServerConVarValueFloat("PanSky");
	m_fPanSkyOffsetX = g_pLTClient->GetServerConVarValueFloat("PanSkyOffsetX");
	m_fPanSkyOffsetZ = g_pLTClient->GetServerConVarValueFloat("PanSkyOffsetX");
	m_fPanSkyScaleX = g_pLTClient->GetServerConVarValueFloat("PanSkyScaleX");
	m_fPanSkyScaleZ = g_pLTClient->GetServerConVarValueFloat("PanSkyScaleZ");
	char* pTexture  = g_pLTClient->GetServerConVarValueFloat("PanSkyTexture");

	if (m_bPanSky)
	{
		g_pLTClient->SetGlobalPanTexture(GLOBALPAN_SKYSHADOW, pTexture);
	}


	// Set up the environment map (chrome) texture...

	char* pEnvMap = g_pLTClient->GetServerConVarValueString("EnvironmentMap");
	if (pEnvMap)
	{
		char* pVal = pEnvMap[0] == '0' ? "Textures\\Chrome.dtx" : pEnvMap;
		sprintf(buf, "EnvMap %s", pVal);
		g_pLTClient->RunConsoleString(buf);
	}


	// Set up the global (per level) wind values...

	g_vWorldWindVel.x = g_pLTClient->GetServerConVarValueFloat("WindX");
	g_vWorldWindVel.y = g_pLTClient->GetServerConVarValueFloat("WindY");
	g_vWorldWindVel.z = g_pLTClient->GetServerConVarValueFloat("WindZ");


	// Set up the global (per level) light scale values...

	m_vDefaultLightScale.x = g_pLTClient->GetServerConVarValueFloat("LightScaleR") / 255.0f;
	m_vDefaultLightScale.y = g_pLTClient->GetServerConVarValueFloat("LightScaleG") / 255.0f;
	m_vDefaultLightScale.z = g_pLTClient->GetServerConVarValueFloat("LightScaleB") / 255.0f;

	m_LightScaleMgr.SetLightScale (&m_vDefaultLightScale, LightEffectWorld);

	
	// Set up the global (per level) far z value.

	//LTFLOAT fVal = pClientDE->GetServerConVarValueLTFLOAT("FarZ");

	//if (fVal > 0.0f)
	//{
		// All levels should be vised, so make sure the far z is out there! :)

		sprintf(buf, "+FarZ %d", 100000 /*(int)fVal*/);
		g_pLTClient->RunConsoleString(buf);
	//}


	// Set up the global (per level) fog values...

	ResetGlobalFog();


	// Initialize the music playlists...

	if (m_Music.IsInitialized())
	{
//		CMusic::EMusicLevel level = m_Music.GetMusicLevel();
		m_Music.TermPlayLists();
		m_Music.InitPlayLists();

		if (!m_Music.UsingIMA())
		{
			m_Music.PlayCDList();
		}
		else
		{
			m_Music.PlayMusicLevel(m_eMusicLevel);
		}
	}

	// Set up the soft renderer sky map...

	char* pSoftSky = g_pLTClient->GetServerConVarValueString("SoftSky");
	if (pSoftSky)
	{
		sprintf(buf, "SoftSky %s", pSoftSky);
		g_pLTClient->RunConsoleString(buf);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ResetGlobalFog
//
//	PURPOSE:	Reset the global fog value...
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::ResetGlobalFog()
{
	if (!g_pLTClient) return;

	// see if fog should be disabled

	if (AdvancedDisableFog())
	{
		g_pLTClient->RunConsoleString("FogEnable 0");
		return;
	}

	LTFLOAT fVal = g_pLTClient->GetServerConVarValueFloat("EnableFog");
	
	char buf[30];
	LTVector todScale;

	sprintf(buf, "FogEnable %d", (int)fVal);
	g_pLTClient->RunConsoleString(buf);
	
	todScale = m_LightScaleMgr.GetTimeOfDayScale();
	if (fVal)
	{
		fVal = g_pLTClient->GetServerConVarValueFloat("FogNearZ");
		sprintf(buf, "FogNearZ %d", (int)fVal);
		g_pLTClient->RunConsoleString(buf);

		fVal = g_pLTClient->GetServerConVarValueFloat("FogFarZ");
		sprintf(buf, "FogFarZ %d", (int)fVal);
		g_pLTClient->RunConsoleString(buf);

		fVal = g_pLTClient->GetServerConVarValueFloat("FogR") * todScale.x;
		sprintf(buf, "FogR %d", (int)fVal);
		g_pLTClient->RunConsoleString(buf);

		fVal = g_pLTClient->GetServerConVarValueFloat("FogG") * todScale.y;
		sprintf(buf, "FogG %d", (int)fVal);
		g_pLTClient->RunConsoleString(buf);

		fVal = g_pLTClient->GetServerConVarValueFloat("FogB") * todScale.z;
		sprintf(buf, "FogB %d", (int)fVal);
		g_pLTClient->RunConsoleString(buf);

		fVal = g_pLTClient->GetServerConVarValueFloat("SkyFog");
		sprintf(buf, "SkyFog %d", (int)fVal);
		g_pLTClient->RunConsoleString(buf);

		if (fVal)
		{
			fVal = g_pLTClient->GetServerConVarValueFloat("SkyFogNearZ");
			sprintf(buf, "SkyFogNearZ %d", (int)fVal);
			g_pLTClient->RunConsoleString(buf);

			fVal = g_pLTClient->GetServerConVarValueFloat("SkyFogFarZ");
			sprintf(buf, "SkyFogFarZ %d", (int)fVal);
			g_pLTClient->RunConsoleString(buf);
		}
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ShowPlayer()
//
//	PURPOSE:	Show/Hide the player object
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::ShowPlayer(LTBOOL bShow)
{
	if (!g_pLTClient) return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	uint32 dwFlags;
	g_pLTClient->Common()->GetObjectFlags(hPlayerObj, OFT_Flags, dwFlags);
	if (bShow /*&& !(dwFlags & FLAG_VISIBLE)*/)
	{
		dwFlags |= FLAG_VISIBLE;
		g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, dwFlags, FLAGMASK_ALL);
	}
	else if (!bShow /*&& (dwFlags & FLAG_VISIBLE)*/)
	{
		dwFlags &= ~FLAG_VISIBLE;
		g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, dwFlags, FLAGMASK_ALL);
	}
}


// ----------------------------------------------------------------------- //
// Puts the server's player model where our invisible one is.
// ----------------------------------------------------------------------- //
void CRiotClientShell::UpdateServerPlayerModel()
{
	HOBJECT hClientObj, hRealObj;
	LTRotation myRot;
	LTVector myPos;
	
	if(!g_pLTClient || !m_MoveMgr)
		return;
	
	if(!(hClientObj = g_pLTClient->GetClientObject()))
		return;

	if(!(hRealObj = m_MoveMgr->GetObject()))
		return;

	g_pLTClient->GetObjectPos(hRealObj, &myPos);
	g_pLTClient->SetObjectPos(hClientObj, &myPos);

	g_pLTClient->Math()->SetupEuler(myRot, m_fPitch*0.1f, m_fYaw, m_fCamCant);
	g_pLTClient->SetObjectRotation(hClientObj, &myRot);	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::RenderCamera()
//
//	PURPOSE:	Sets up the client and renders the camera
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::RenderCamera (LTBOOL bDrawInterface)
{
	if (!g_pLTClient || !m_bCameraPosInited) return;
	
	// Make sure the rendered player object is right where it should be.

	UpdateServerPlayerModel();

	// Update anything attached to the player object if it's not going to be rendered.
	if(m_playerCamera.IsFirstPerson())
	{
		g_pLTClient->ProcessAttachments(g_pLTClient->GetClientObject());
	}

	// Make sure the weapon is updated before we render the camera...

	UpdateWeaponModel();


	g_pLTClient->Start3D();
	g_pLTClient->RenderCamera (m_hCamera);
	if (bDrawInterface) 
	{
		g_pLTClient->StartOptimized2D();
		DrawInterface();
		g_pLTClient->EndOptimized2D();
	}
	g_pLTClient->End3D();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::DrawInterface()
//
//	PURPOSE:	Draws any interface stuff that may need to be drawn
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::DrawInterface()
{
	if (!g_pLTClient) return;

	// find out if we're in multiplayer
	LTBOOL bMultiplayer = IsMultiplayerGame();
	
	// get the screen width and height
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	uint32 nScreenWidth, nScreenHeight;
	g_pLTClient->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);

	// draw any temporary information stuff...
	m_infoDisplay.Draw();

	// draw the player stats (health,armor,ammo) if appropriate
	if (m_bDrawInterface) 
	{
		m_stats.Draw (m_bStatsSizedOff, m_bDrawHud);
	}

	// draw any frag counts (ours only and maybe all) if appropriate
	m_ClientInfo.Draw (m_bDrawInterface, m_bDrawFragCount || (m_ePlayerState == PS_DEAD && bMultiplayer));
	
	// draw any inventory pickup messages, and the ordinance screen if appropriate
	m_inventory.Draw (m_bDrawOrdinance);

	// draw mission log if we need to
	if (m_bDrawMissionLog)
	{
		m_objectives.Draw();
		if (m_bWaitingForMLClosure && !m_objectives.IsClosing())
		{
			m_bWaitingForMLClosure = LTFALSE;
			m_bDrawMissionLog = LTFALSE;
			PauseGame(LTFALSE);
			m_LightScaleMgr.ClearLightScale (&m_vLightScaleObjectives, LightEffectInterface);
		}
	}

	// if we have a new objective that we haven't checked yet, draw the new objective notice
	if (m_bNewObjective && m_hNewObjective && !m_bUsingExternalCamera)
	{
		// draw the notice
		uint32 nNewObjectiveWidth = 0;
		uint32 nNewObjectiveHeight = 0;
		g_pLTClient->GetSurfaceDims (m_hNewObjective, &nNewObjectiveWidth, &nNewObjectiveHeight);

		g_pLTClient->DrawSurfaceToSurface (hScreen, m_hNewObjective, LTNULL, nScreenWidth - nNewObjectiveWidth - 10, 25);
	}

	// draw transmission if needed
	if (m_fTransmissionTimeLeft > 0.0f) DrawTransmission();

	// if there's an ingame dialog, draw that
	if (m_pIngameDialog) m_pIngameDialog->Draw (g_pLTClient->GetScreenSurface());

	// handle message editing...
	m_messageMgr.Draw();


	///////////////////////////////////////////////////////////////////////////////
	//
	// WARNING: everything from here on down relies on the camera being created!
	//
	///////////////////////////////////////////////////////////////////////////////

	if (!m_hCamera)
	{
		return;
	}
	
	// get the camera dims
	LTBOOL bFullScreen = LTFALSE;
	int nCameraLeft = 0;
	int nCameraTop = 0;
	int nCameraRight = 0;
	int nCameraBottom = 0;
	g_pLTClient->GetCameraRect (m_hCamera, &bFullScreen, &nCameraLeft, &nCameraTop, &nCameraRight, &nCameraBottom);
	if (bFullScreen)
	{
		nCameraRight = (int)nScreenWidth;
		nCameraBottom = (int)nScreenHeight;
	}

	// if there is a game message (sent by a trigger) to display, display it
	if (m_hGameMessage)
	{
		LTFLOAT nCurrentTime = g_pLTClient->GetTime();
		if (nCurrentTime < m_nGameMessageRemoveTime)
		{
			int x = nCameraLeft + (((nCameraRight - nCameraLeft) - m_rcGameMessage.right) >> 1);
			int y = nCameraBottom - (m_rcGameMessage.bottom << 1);

			HLTCOLOR hBlack = g_pLTClient->SetupColor1 (0.1f, 0.1f, 0.1f, LTFALSE);
			g_pLTClient->DrawSurfaceSolidColor (hScreen, m_hGameMessage, &m_rcGameMessage, x+3, y+3, NULL, hBlack);
			g_pLTClient->DrawSurfaceToSurfaceTransparent (hScreen, m_hGameMessage, &m_rcGameMessage, x, y, NULL);
		}
		else
		{
			g_pLTClient->DeleteSurface (m_hGameMessage);
			m_hGameMessage = NULL;
		}
	}
	
	// Display any necessary debugging info...
	if (m_hDebugInfo)
	{
		int x = nScreenWidth  - (m_rcDebugInfo.right - m_rcDebugInfo.left);
		int y = nScreenHeight - 18;

		g_pLTClient->DrawSurfaceToSurfaceTransparent (hScreen, m_hDebugInfo, &m_rcDebugInfo, x, y, NULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::DrawTransmission()
//
//	PURPOSE:	Update the transmission if there is one
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::DrawTransmission()
{
	ILTMessage_Write* hMsg;
	if (!g_pLTClient) return;
	
	if (m_fTransmissionTimeLeft > 0.0f)
	{
		LTFLOAT nFrameTime = g_pLTClient->GetFrameTime();
		if (nFrameTime > 1.0f) nFrameTime = 1.0f;
		m_fTransmissionTimeLeft -= nFrameTime;

		if (m_bAnimatingTransmissionOn)
		{
			m_xTransmissionImage += nFrameTime * TRANSMISSION_IMAGE_ANIM_RATE;
			m_yTransmissionText += nFrameTime * TRANSMISSION_TEXT_ANIM_RATE;

			if (m_xTransmissionImage > 6.0f || m_yTransmissionText > 32.0f)
			{
				m_xTransmissionImage = 6.0f;
				m_yTransmissionText = 32.0f;
				m_bAnimatingTransmissionOn = LTFALSE;
			}
		}
		else if (m_bAnimatingTransmissionOff)
		{
			m_xTransmissionImage -= nFrameTime * TRANSMISSION_IMAGE_ANIM_RATE;
			m_yTransmissionText -= nFrameTime * TRANSMISSION_TEXT_ANIM_RATE;

			if (m_xTransmissionImage < -m_cxTransmissionImage && m_yTransmissionText < -m_cyTransmissionText)
			{
				m_bAnimatingTransmissionOff = LTFALSE;
				m_fTransmissionTimeLeft = 0.0f;
			
				g_pLTClient->DeleteSurface (m_hTransmissionImage);
				g_pLTClient->DeleteSurface (m_hTransmissionText);
				m_hTransmissionImage = LTNULL;
				m_hTransmissionText = LTNULL;
			}
		}

		// Check if sound is done.
		if( m_hTransmissionSound )
		{
			if( g_pLTClient->IsDone( m_hTransmissionSound ))
			{
				g_pLTClient->KillSound( m_hTransmissionSound );
				m_hTransmissionSound = LTNULL;
				
				// Tell server the transmission ended.
				hMsg = g_pLTClient->StartMessage( MID_TRANSMISSIONENDED );
				g_pLTClient->EndMessage( hMsg );
			}
		}

		if (m_hTransmissionImage)
		{
			HLTCOLOR hTrans = g_pLTClient->SetupColor1 (0.0f, 0.0f, 1.0f, LTFALSE);
			g_pLTClient->DrawSurfaceToSurfaceTransparent (g_pLTClient->GetScreenSurface(), m_hTransmissionText, LTNULL, (int)m_xTransmissionText, (int)m_yTransmissionText, LTNULL);
			g_pLTClient->DrawSurfaceToSurfaceTransparent (g_pLTClient->GetScreenSurface(), m_hTransmissionImage, LTNULL, (int)m_xTransmissionImage, (int)m_yTransmissionImage, hTrans);

			if (m_fTransmissionTimeLeft <= 0.0f)
			{
				if( m_hTransmissionSound && !pClientDE->IsDone( m_hTransmissionSound ))
				{
					m_fTransmissionTimeLeft += nFrameTime;
				}
				else
				{
					m_fTransmissionTimeLeft = 100.0f;
					m_bAnimatingTransmissionOff = LTTRUE;
				}
			}

			AddToClearScreenCount();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::DoRenderLoop()
//
//	PURPOSE:	Forces exactly one update to occur
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::DoRenderLoop (LTBOOL bDrawInterface)
{
	if (!g_pLTClient) return;
	
	PreUpdate();
	g_pLTClient->Start3D();
	g_pLTClient->RenderCamera (m_hCamera);
	if (bDrawInterface) 
	{
		g_pLTClient->StartOptimized2D();
		DrawInterface();
		g_pLTClient->EndOptimized2D();
	}
	g_pLTClient->End3D();
	g_pLTClient->FlipScreen (FLIPSCREEN_CANDRAWCONSOLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdatePlayer()
//
//	PURPOSE:	Update the player
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::UpdatePlayer()
{
	if (!g_pLTClient) return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj || m_ePlayerState == PS_DEAD) return;
	

	// This is pretty much a complete kludge, but I can't really think of
	// a better way to handle this...Okay, since the server can update the
	// player's flags at any time (and override anything that we set), we'll 
	// make sure that the player's flags are always what we want them to be :)

	uint32 dwPlayerFlags;
	g_pLTClient->Common()->GetObjectFlags(hPlayerObj, OFT_Flags, dwPlayerFlags);
	if (m_playerCamera.IsFirstPerson())
	{
		if (dwPlayerFlags & FLAG_VISIBLE)
		{
			dwPlayerFlags &= ~FLAG_VISIBLE;
			g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, dwPlayerFlags, FLAGMASK_ALL);
		}
	}
	else  // Third person
	{
		if (!(dwPlayerFlags & FLAG_VISIBLE))
		{
			dwPlayerFlags |= FLAG_VISIBLE;
			g_pLTClient->Common()->SetObjectFlags(hPlayerObj, OFT_Flags, dwPlayerFlags, FLAGMASK_ALL);
		}
	}


	// Hide/Show our attachments...

	HLOCALOBJ attachList[20];
	uint32 dwListSize = 0;
	uint32 dwNumAttach = 0;

	g_pLTClient->Common()->GetAttachments(hPlayerObj, attachList, 20, dwListSize, dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	for (int i=0; i < nNum; i++)
	{
		uint32 dwUsrFlags;
		g_pLTClient->Common()->GetObjectFlags(attachList[i], OFT_User, dwUsrFlags);
		
		if (dwUsrFlags & USRFLG_ATTACH_HIDE1SHOW3)
		{
			uint32 dwFlags;
			g_pLTClient->Common()->GetObjectFlags(attachList[i], OFT_Flags, dwFlags);

			if (m_playerCamera.IsFirstPerson())
			{
				if (dwFlags & FLAG_VISIBLE)
				{
					dwFlags &= ~FLAG_VISIBLE;
					g_pLTClient->Common()->SetObjectFlags(attachList[i], OFT_Flags, dwFlags, FLAGMASK_ALL);
				}
			}
			else
			{
				if (!(dwFlags & FLAG_VISIBLE))
				{
					dwFlags |= FLAG_VISIBLE;
					g_pLTClient->Common()->SetObjectFlags(attachList[i], OFT_Flags, dwFlags, FLAGMASK_ALL);
				}
			}
		}
		else if (dwUsrFlags & USRFLG_ATTACH_HIDE1)
		{
			uint32 dwFlags;
			g_pLTClient->Common()->GetObjectFlags(attachList[i], OFT_Flags, dwFlags);

			if (m_playerCamera.IsFirstPerson())
			{
				if (dwFlags & FLAG_VISIBLE)
				{
					dwFlags &= ~FLAG_VISIBLE;
					g_pLTClient->Common()->SetObjectFlags(attachList[i], OFT_Flags, dwFlags, FLAGMASK_ALL);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::Update3rdPersonInfo
//
//	PURPOSE:	Update the 3rd person cross hair / camera info
//
// ----------------------------------------------------------------------- //

void CRiotClientShell::Update3rdPersonInfo()
{
	if (!g_pLTClient) return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj || m_ePlayerState == PS_DEAD || !m_MoveMgr) return;

	HOBJECT hFilterList[] = {hPlayerObj, m_MoveMgr->GetObject(), LTNULL};


	ClientIntersectInfo info;
	ClientIntersectQuery query;
	memset(&query, 0, sizeof(query));
	LTVector vPlayerPos, vUp, vRight, vForward;

	g_pLTClient->GetObjectPos(hPlayerObj, &vPlayerPos);

	LTFLOAT fCrosshairDist = -1.0f;
	LTFLOAT fCameraOptZ = 110.0f;

	// Figure out crosshair distance...

	if (m_bCrosshairOn && m_weaponModel.GetHandle())
	{
		fCrosshairDist = GetWeaponRange(m_weaponModel.GetId());

		g_pLTClient->Math()->GetRotationVectors(m_rRotation, vUp, vRight, vForward);

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
	g_pLTClient->Math()->GetRotationVectors(rRot, vUp, vRight, vForward);
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
	m_playerCamera.SetOptZ(fCameraOptZ);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CreateMenuPolygrid
//
//	PURPOSE:	Create the polygrid used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::CreateMenuPolygrid()
{
	if (!g_pLTClient || m_pMenuPolygrid || !m_hCamera) return;

	// Hide our hand-held weapon...

	HLOCALOBJ hWeapon = m_weaponModel.GetHandle();
	if (hWeapon)
	{
		uint32 dwFlags;
		g_pLTClient->Common()->GetObjectFlags(hWeapon, OFT_Flags, dwFlags);
		dwFlags &= ~FLAG_VISIBLE;
		g_pLTClient->Common()->SetObjectFlags(hWeapon, OFT_Flags, dwFlags, FLAGMASK_ALL);
	}


	// Create an object to serve as the "server" object for the polygrid.
	// The polygrid uses this object to determine its position, rotation,
	// and visibility...

	LTVector vPos, vU, vR, vF, vTemp;
	LTRotation rRot;

	g_pLTClient->GetObjectPos(m_hCamera, &vPos);
	g_pLTClient->GetObjectRotation(m_hCamera, &rRot);
	g_pLTClient->Math()->GetRotationVectors(rRot, vU, vR, vF);

	// Put the polygrid a little in front of the camera...

	VEC_MULSCALAR(vTemp, vF, /*25.0f*/10.0f);
	VEC_ADD(vPos, vPos, vTemp);

	// Need to orient the polygrid correctly...

	g_pLTClient->Math()->EulerRotateX(rRot, MATH_HALFPI);

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);
	theStruct.m_ObjectType = OT_NORMAL;
	VEC_COPY(theStruct.m_Pos, vPos);
	theStruct.m_Rotation = rRot;

	HLOCALOBJ hObj = g_pLTClient->CreateObject(&theStruct);
	if (!hObj) return;


	// Need to set this to visible, or polygrid won't be drawn...

	g_pLTClient->Common()->SetObjectFlags(hObj, OFT_User, USRFLG_VISIBLE, FLAGMASK_ALL);


	// Save the current camera fov...

	g_pLTClient->GetCameraFOV(m_hCamera, &m_fMenuSaveFOVx, &m_fMenuSaveFOVy);

	// Set the menu fov...

	uint32 dwWidth = 640, dwHeight = 480;
	g_pLTClient->GetSurfaceDims(g_pLTClient->GetScreenSurface(), &dwWidth, &dwHeight);

	LTFLOAT y = (DEG2RAD(FOV_NORMAL) * dwHeight) / dwWidth;
	g_pLTClient->SetCameraFOV(m_hCamera, DEG2RAD(FOV_NORMAL), y);


	// Okay, since we now have a "server" object to associate with the
	// polygrid, create the polygrid...

	m_pMenuPolygrid = new CPolyGridFX();
	if (!m_pMenuPolygrid) return;

	PGCREATESTRUCT pg;

	pg.hServerObj = hObj;
	VEC_SET(pg.vDims, 10.6f, 0.5f, 10.6f);
	VEC_SET(pg.vColor1, 0.0f, 0.0f, 0.0f);
	VEC_SET(pg.vColor2, 255.0f, 255.0f, 255.0f);
	pg.fXScaleMin = .085f;
	pg.fXScaleMax = .085f;
	pg.fYScaleMin = .09f;
	pg.fYScaleMax = .09f;
	pg.fXScaleDuration = 0.0f;
	pg.fYScaleDuration = 0.0f;
	pg.fXPan = 0.0f;
	pg.fYPan = 0.0f;
	pg.fAlpha = 1.0f;
	pg.hstrSurfaceSprite = g_pLTClient->CreateString("Sprites\\Shogo.spr");
	pg.dwNumPolies = 500;
	pg.nPlasmaType = 1;
	pg.nRingRate[0] = 50;
	pg.nRingRate[1] = 10;
	pg.nRingRate[2] = 30;
	pg.nRingRate[3] = 20;

	m_pMenuPolygrid->Init(&pg);
	m_pMenuPolygrid->SetAlwaysUpdate(LTTRUE);
	m_pMenuPolygrid->SetUseGlobalSettings(LTFALSE);
	m_pMenuPolygrid->CreateObject(g_pLTClient);

	//HOBJECT hPolyObj = m_pMenuPolygrid->GetObject();
	//pClientDE->SetPolyGridEnvMap(hPolyObj, "Textures\\Chrome.dtx");
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RemoveMenuPolygrid
//
//	PURPOSE:	Remove the polygrid used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::RemoveMenuPolygrid()
{
	if (!g_pLTClient || !m_pMenuPolygrid) return;

	// Set the FOV back to what it was...

	if (m_hCamera)
	{
		g_pLTClient->SetCameraFOV(m_hCamera, m_fMenuSaveFOVx, m_fMenuSaveFOVy);
	}


	// Hide our hand-held weapon...

	HLOCALOBJ hWeapon = m_weaponModel.GetHandle();
	if (hWeapon && m_playerCamera.IsFirstPerson())
	{
		uint32 dwFlags; 
		g_pLTClient->Common()->GetObjectFlags(hWeapon, OFT_Flags, dwFlags);
		dwFlags |= FLAG_VISIBLE;
		g_pLTClient->Common()->SetObjectFlags(hWeapon, OFT_Flags, dwFlags, FLAGMASK_ALL);
	}

	HLOCALOBJ hObj = m_pMenuPolygrid->GetServerObj();
	g_pLTClient->RemoveObject(hObj);

	delete m_pMenuPolygrid;
	m_pMenuPolygrid = LTNULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	UpdateMenuPolygrid
//
//	PURPOSE:	Update the polygrid used as a background for the menu
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateMenuPolygrid()
{
	if (!g_pLTClient || !m_pMenuPolygrid || !m_hCamera) return;

	m_pMenuPolygrid->Update();

	HLOCALOBJ objs[1];
	objs[0] = m_pMenuPolygrid->GetObject();
	LTRESULT dRes = g_pLTClient->RenderObjects(m_hCamera, objs, 1);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	UpdateModelGlow
//
//	PURPOSE:	Update the current model glow color
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UpdateModelGlow()
{
	if (!g_pLTClient) return;

	LTFLOAT fColor      = 0.0f;
	LTFLOAT fColorRange = m_vMaxModelGlow.x - m_vMinModelGlow.x;

	m_fModelGlowCycleTime += pClientDE->GetFrameTime();

	if (m_bModelGlowCycleUp)
	{
		if (m_fModelGlowCycleTime < MODELGLOW_HALFCYCLE)
		{
			fColor = m_vMinModelGlow.x + (m_fModelGlowCycleTime * (fColorRange / MODELGLOW_HALFCYCLE));
			VEC_SET(m_vCurModelGlow, fColor, fColor, fColor);
		}
		else
		{
			m_fModelGlowCycleTime = 0.0f;
			VEC_COPY(m_vCurModelGlow, m_vMaxModelGlow);
			m_bModelGlowCycleUp = LTFALSE;
		}
	}
	else 
	{
		if (m_fModelGlowCycleTime < MODELGLOW_HALFCYCLE)
		{
			fColor = m_vMaxModelGlow.x - (m_fModelGlowCycleTime * (fColorRange / MODELGLOW_HALFCYCLE));
			VEC_SET(m_vCurModelGlow, fColor, fColor, fColor);
		}
		else
		{
			m_fModelGlowCycleTime = 0.0f;
			VEC_COPY(m_vCurModelGlow, m_vMinModelGlow);
			m_bModelGlowCycleUp = LTTRUE;
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::InitSinglePlayer
//
//	PURPOSE:	Send the server the initial single player info
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::InitSinglePlayer()
{
	if (!g_pLTClient) return;

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;

	// init player variables on server...

	ILTMessage_Write* hMessage = g_pLTClient->StartMessage(MID_PLAYER_INITVARS);
	g_pLTClient->WriteToMessageByte(hMessage, (uint8)pSettings->RunLock());
	g_pLTClient->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::InitMultiPlayer
//
//	PURPOSE:	Send the server the initial multiplayer info
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::InitMultiPlayer()
{
	if (!g_pLTClient) return;

	if (!IsMultiplayerGame()) return;

	NetPlayer* pPlayerInfo = NetStart_GetPlayerStruct();
	if (!pPlayerInfo) return;

	HSTRING hstrName = g_pLTClient->CreateString(pPlayerInfo->m_sName);
	if (!hstrName) return;

	// Init multiplayer info on server...

	ILTMessage_Write* hMessage = g_pLTClient->StartMessage(MID_PLAYER_MULTIPLAYER_INIT);
	g_pLTClient->WriteToMessageByte(hMessage, pPlayerInfo->m_byMech);
	g_pLTClient->WriteToMessageByte(hMessage, pPlayerInfo->m_byColor);
	g_pLTClient->WriteToMessageHString(hMessage, hstrName);
	g_pLTClient->EndMessage(hMessage);

	// Init player settings...

	CRiotSettings* pSettings = m_menu.GetSettings();
	if (!pSettings) return;

	hMessage = g_pLTClient->StartMessage(MID_PLAYER_INITVARS);
	g_pLTClient->WriteToMessageByte(hMessage, (uint8)pSettings->RunLock());
	g_pLTClient->EndMessage(hMessage);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleItemPickedup
//
//	PURPOSE:	Handle an item being pickedup
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::HandleItemPickedup(PickupItemType eType)
{
	if (!g_pLTClient) return;
 
	DoPickupItemScreenTint(eType);

	if (eType == PIT_ULTRA_NIGHTVISION)
	{
		m_bHaveNightVision = LTTRUE;
		
		g_pLTClient->SetModelHook ((ModelHookFn)NVModelHook, this);
		m_LightScaleMgr.SetLightScale (&m_vLightScaleNightVision, LightEffectPowerup);
	}
	else if (eType == PIT_ULTRA_INFRARED)
	{
		m_bHaveInfrared = LTTRUE;
		
		g_pLTClient->SetModelHook ((ModelHookFn)IRModelHook, this);
		m_LightScaleMgr.SetLightScale (&m_vLightScaleInfrared, LightEffectPowerup);
	}
	else if (eType == PIT_ULTRA_SILENCER)
	{
		m_bHaveSilencer = LTTRUE;
	}
	else if (eType == PIT_ULTRA_STEALTH)
	{
		m_bHaveStealth = LTTRUE;
	}
	else if (eType == PIT_SHOGO_S || eType == PIT_SHOGO_H || eType == PIT_SHOGO_O || eType == PIT_SHOGO_G)
	{
		m_inventory.ShogoPowerupPickup (eType);
	}
	else
	{
		DisplayGenericPickupMessage(eType);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleItemExpired
//
//	PURPOSE:	Handle an item expiring
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::HandleItemExpired(PickupItemType eType)
{
	if (!g_pLTClient) return;

	if (eType == PIT_ULTRA_NIGHTVISION)
	{
		m_bHaveNightVision = LTFALSE;

		g_pLTClient->SetModelHook ((ModelHookFn)DefaultModelHook, this);
		m_LightScaleMgr.ClearLightScale (&m_vLightScaleNightVision, LightEffectPowerup);
	}
	else if (eType == PIT_ULTRA_INFRARED)
	{
		m_bHaveInfrared = LTFALSE;
		
		g_pLTClient->SetModelHook ((ModelHookFn)DefaultModelHook, this);
		m_LightScaleMgr.ClearLightScale (&m_vLightScaleInfrared, LightEffectPowerup);
	}
	else if (eType == PIT_ULTRA_SILENCER)
	{
		m_bHaveSilencer = LTFALSE;
	}
	else if (eType == PIT_ULTRA_STEALTH)
	{
		m_bHaveStealth = LTFALSE;
	}
}





// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::BuildClientSaveMsg
//
//	PURPOSE:	Save all the necessary client-side info
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::BuildClientSaveMsg(ILTMessage_Write* hMessage)
{
	if (!g_pLTClient || !hMessage) return;

	ILTMessage_Write* hData = pClientDE->StartILTMessage_Write();
	
	// Save complex data members...

	m_stats.Save(hData);
	m_inventory.Save(hData);
	m_objectives.Save(hData);


	// Save all necessary data members...
	LTRotation dummyRotation;
	dummyRotation.Init();

	hData->WriteLTRotation(m_rRotation);
	hData->WriteLTRotation(dummyRotation);
	hData->WriteLTVector(m_vTintColor);
	hData->WriteLTVector(m_vLastSentFlashPos);
	hData->WriteLTVector(m_vLastSentModelPos);
	hData->WriteLTVector(m_vCameraOffset);

	hData->Writeuint8(m_eDifficulty);
	hData->Writeuint8(m_nPlayerMode);
	hData->Writeuint8(m_nLastSentCode);
	hData->Writeuint8(m_bTintScreen);
	hData->Writeuint8(m_bSpectatorMode);
	hData->Writeuint8(m_bMoving);
	hData->Writeuint8(m_bMovingSide);
	hData->Writeuint8(m_bOnGround);
	hData->Writeuint8(m_bLastSent3rdPerson);
	hData->Writeuint8(m_bZoomView);
	hData->Writeuint8(m_bOldZoomView);
	hData->Writeuint8(m_bZooming);
	hData->Writeuint8(m_bStartedDuckingDown);
	hData->Writeuint8(m_bStartedDuckingUp);
	hData->Writeuint8(m_bCenterView);
	hData->Writeuint8(m_bAllowPlayerMovement);
	hData->Writeuint8(m_bLastAllowPlayerMovement);
	hData->Writeuint8(m_bWasUsingExternalCamera);
	hData->Writeuint8(m_bUsingExternalCamera);
	hData->Writeuint8(m_bUnderwater);
	//hData->Writeuint8(m_bGameOver);
	hData->Writeuint8(m_ePlayerState);
	hData->Writeuint8(m_eMusicLevel);

	hData->Writeuint32(m_dwPlayerFlags);
	hData->Writeuint32(m_nOldCameraLeft);
	hData->Writeuint32(m_nOldCameraTop);
	hData->Writeuint32(m_nOldCameraRight);
	hData->Writeuint32(m_nOldCameraBottom);

	hData->Writefloat(m_fTintTime);
	hData->Writefloat(m_fTintStart);
	hData->Writefloat(m_fTintRampUp);
	hData->Writefloat(m_fTintRampDown);
	hData->Writefloat(m_fYaw);
	hData->Writefloat(m_fPitch);
	hData->Writefloat(m_fLastSentYaw);
	hData->Writefloat(m_fLastSentCamCant);
	hData->Writefloat(m_fPitch);
	hData->Writefloat(m_fYaw);
	hData->Writefloat(m_fFireJitterPitch);
	hData->Writefloat(m_fContainerStartTime);
	hData->Writefloat(m_fFovXFXDir);
	hData->Writefloat(m_fLastTime);
	hData->Writefloat(m_fBobHeight);
	hData->Writefloat(m_fBobWidth);
	hData->Writefloat(m_fBobAmp);
	hData->Writefloat(m_fBobPhase);
	hData->Writefloat(m_fSwayPhase);
	hData->Writefloat(m_fVelMagnitude);
	hData->Writefloat(m_fCantIncrement);
	hData->Writefloat(m_fCantMaxDist);
	hData->Writefloat(m_fCamCant);
	hData->Writefloat(m_fCurrentFovX);
	hData->Writefloat(m_fSaveLODScale);
	hData->Writefloat(m_fCamDuck);
	hData->Writefloat(m_fDuckDownV);
	hData->Writefloat(m_fDuckUpV);
	hData->Writefloat(m_fMaxDuckDistance);
	hData->Writefloat(m_fStartDuckTime);

	hMessage->WriteMessage(hData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UnpackClientSaveMsg
//
//	PURPOSE:	Load all the necessary client-side info
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::UnpackClientSaveMsg(ILTMessage_Read* hMessage)
{
	if (!g_pLTClient || !hMessage) return;

	m_bRestoringGame = LTTRUE;


	ILTMessage_Read* hData = hMessage->ReadMessage();


	// Load complex data members...

	// DO NOT CHANGE THE LOADING ORDER OF STATS AND INVENTORY
	// INVENTORY LOAD DEPENDS ON BEING LOADED AFTER STATS
	m_stats.Load(hData);
	m_inventory.Load(hData);
	m_objectives.Load(hData);


	// Load data members...
	LTRotation dummyRotation;

	m_rRotation = hData->ReadLTRotation();
	dummyRotation = hData->ReadLTRotation();
	m_vTintColor = hData->ReadLTVector();
	m_vLastSentFlashPos = hData->ReadLTVector();
	m_vLastSentModelPos = hData->ReadLTVector();
	m_vCameraOffset = hData->ReadLTVector();

	// Reset our first-person camera offset...

	m_playerCamera.SetFirstPersonOffset(m_vCameraOffset);

	m_eDifficulty				= (GameDifficulty) hData->Readuint8();
	m_nPlayerMode				= hData->Readuint8();
	m_nLastSentCode				= hData->Readuint8();
	m_bTintScreen 				= (LTBOOL) hData->Readuint8();
	m_bSpectatorMode			= (LTBOOL) hData->Readuint8();
	m_bMoving					= (LTBOOL) hData->Readuint8();
	m_bMovingSide				= (LTBOOL) hData->Readuint8();
	m_bOnGround					= (LTBOOL) hData->Readuint8();
	m_bLastSent3rdPerson		= (LTBOOL) hData->Readuint8();
	m_bZoomView					= (LTBOOL) hData->Readuint8();
	m_bOldZoomView				= (LTBOOL) hData->Readuint8();
	m_bZooming					= (LTBOOL) hData->Readuint8();
	m_bStartedDuckingDown		= (LTBOOL) hData->Readuint8();
	m_bStartedDuckingUp			= (LTBOOL) hData->Readuint8();
	m_bCenterView				= (LTBOOL) hData->Readuint8();
	m_bAllowPlayerMovement		= (LTBOOL) hData->Readuint8();
	m_bLastAllowPlayerMovement	= (LTBOOL) hData->Readuint8();
	m_bWasUsingExternalCamera	= (LTBOOL) hData->Readuint8();
	m_bUsingExternalCamera		= (LTBOOL) hData->Readuint8();
	m_bMovieCameraRect			= (LTBOOL) hData->Readuint8();
	m_bUnderwater				= (LTBOOL) hData->Readuint8();
	//m_bGameOver					= (LTBOOL) hData->Readuint8();
	m_ePlayerState				= (PlayerState) hData->Readuint8();
	m_eMusicLevel				= (CMusic::EMusicLevel) hData->Readuint8();

	m_dwPlayerFlags				= hData->Readuint32();
	m_nOldCameraLeft			= (int) hData->Readuint32();
	m_nOldCameraTop				= (int) hData->Readuint32();
	m_nOldCameraRight			= (int) hData->Readuint32();
	m_nOldCameraBottom			= (int) hData->Readuint32();

	m_fTintTime					= hData->Readfloat();
	m_fTintStart				= hData->Readfloat();
	m_fTintRampUp				= hData->Readfloat();
	m_fTintRampDown				= hData->Readfloat();
	m_fYawBackup				= hData->Readfloat();
	m_fPitchBackup				= hData->Readfloat();
	m_fLastSentYaw				= hData->Readfloat();
	m_fLastSentCamCant			= hData->Readfloat();
	m_fPitch					= hData->Readfloat();
	m_fYaw						= hData->Readfloat();
	m_fFireJitterPitch			= hData->Readfloat();
	m_fContainerStartTime		= hData->Readfloat();
	m_fFovXFXDir				= hData->Readfloat();
	m_fLastTime					= hData->Readfloat();
	m_fBobHeight				= hData->Readfloat();
	m_fBobWidth					= hData->Readfloat();
	m_fBobAmp					= hData->Readfloat();
	m_fBobPhase					= hData->Readfloat();
	m_fSwayPhase				= hData->Readfloat();
	m_fVelMagnitude				= hData->Readfloat();
	m_fCantIncrement			= hData->Readfloat();
	m_fCantMaxDist				= hData->Readfloat();
	m_fCamCant					= hData->Readfloat();
	m_fCurrentFovX				= hData->Readfloat();
	m_fSaveLODScale				= hData->Readfloat();
	m_fCamDuck					= hData->Readfloat();
	m_fDuckDownV				= hData->Readfloat();
	m_fDuckUpV					= hData->Readfloat();
	m_fMaxDuckDistance			= hData->Readfloat();
	m_fStartDuckTime			= hData->Readfloat();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::ProcessCheat
//
//	PURPOSE:	process a cheat.
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::ProcessCheat(CheatCode nCode)
{
	switch (nCode)
	{
		case CHEAT_ANIME:
			m_bAnime = !m_bAnime;
		break;

		default : break;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleRespawn
//
//	PURPOSE:	Handle player respawn
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::HandleRespawn()
{
	if (!g_pLTClient || m_ePlayerState != PS_DEAD) return;


	// if we're in multiplayer send the respawn command...

	if (IsMultiplayerGame())
	{
		// send a message to the server telling it that it's ok to respawn us now...

		ILTMessage_Write* hMsg = pClientDE->StartMessage(MID_PLAYER_RESPAWN);
		pClientDE->EndMessage(hMsg);
		return;
	}
	else  // Bring up load game menu...
	{
		SetLoadGameMenu();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::HandleMPChangeLevel
//
//	PURPOSE:	Handle changing levels in a multiplayer game
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::HandleMPChangeLevel()
{
	if (!g_pLTClient || !IsMultiplayerGame() || m_nGameState == GS_MPLOADINGLEVEL) return;

	// Clear screen tint...

	if (m_bTintScreen)
	{
		m_bTintScreen = LTFALSE;
		LTVector vLightAdd;
		VEC_INIT(vLightAdd);
		g_pLTClient->SetCameraLightAdd(m_hCamera, &vLightAdd);
	}

	// Update the screen here with the current frag counts and no interface, and tell
	// the game we want to ignore any future updates until OnEnterWorld() is called...

	g_pLTClient->ClearScreen(LTNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	g_pLTClient->Start3D();

	CreateMenuPolygrid();
	UpdateMenuPolygrid();

	g_pLTClient->StartOptimized2D();

	HSURFACE hLoadingWorld = CTextHelper::CreateSurfaceFromString(g_pLTClient, m_menu.GetFont12s(), IDS_BUMPER_LOADING);
	if (hLoadingWorld)
	{
		HSURFACE hScreen = g_pLTClient->GetScreenSurface();
		uint32 nScreenWidth = 0, nScreenHeight = 0;
		g_pLTClient->GetSurfaceDims(hScreen, &nScreenWidth, &nScreenHeight);

		uint32 nWidth = 0, nHeight = 0;
		g_pLTClient->GetSurfaceDims(hLoadingWorld, &nWidth, &nHeight);
		g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, hLoadingWorld, LTNULL, ((int)(nScreenWidth - nWidth)) / 2, 
												   (int)nScreenHeight - (int)m_cyPressAnyKey, LTNULL);

		g_pLTClient->DeleteSurface(hLoadingWorld);
	}
	
	m_ClientInfo.Draw(LTFALSE, LTTRUE);
	g_pLTClient->EndOptimized2D();

	g_pLTClient->End3D();
	g_pLTClient->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);

	m_nGameState = GS_MPLOADINGLEVEL;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::QuickSave
//
//	PURPOSE:	Quick save the game
//
// --------------------------------------------------------------------------- //

LTBOOL CRiotClientShell::QuickSave()
{
	if (!g_pLTClient || IsMultiplayerGame()) return LTFALSE;

	if (m_ePlayerState == PS_DEAD || m_nGameState != GS_PLAYING || 
		m_bUsingExternalCamera) return LTFALSE;

	// Do quick save...
	
	HSTRING hStr = g_pLTClient->FormatString (IDS_QUICKSAVING);
	g_pLTClient->CPrint(g_pLTClient->GetStringData (hStr));
	m_infoDisplay.AddInfo(g_pLTClient->GetStringData (hStr), m_menu.GetFont12s(), 0.5f, DI_CENTER | DI_TOP);
	g_pLTClient->FreeString (hStr);

	char strKey[32];
	SAFE_STRCPY(strKey, "SaveGame00");
	char strSaveGame[256];
	SAFE_STRCPY(strSaveGame, m_strCurrentWorldName);
	CWinUtil::WinWritePrivateProfileString ("Shogo", strKey, strSaveGame, SAVEGAMEINI_FILENAME);
	
	m_bQuickSave = LTTRUE;

	return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::QuickLoad
//
//	PURPOSE:	Quick load the game
//
// --------------------------------------------------------------------------- //

LTBOOL CRiotClientShell::QuickLoad()
{
	if (!g_pLTClient || IsMultiplayerGame()) return LTFALSE;

	if (m_nGameState != GS_PLAYING || m_bUsingExternalCamera) return LTFALSE;

	ClearScreenTint();

	char strSaveGameSetting[256];
	memset (strSaveGameSetting, 0, 256);
	char strKey[32];
	SAFE_STRCPY(strKey, "SaveGame00");
	CWinUtil::WinGetPrivateProfileString ("Shogo", strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
	
	if (!*strSaveGameSetting)
	{
		DoMessageBox (IDS_NOQUICKSAVEGAME, TH_ALIGN_CENTER);
		return LTFALSE;
	}

	char* strWorldName = strSaveGameSetting;
	if (!LoadGame(strWorldName, QUICKSAVE_FILENAME))
	{
		DoMessageBox(IDS_LOADGAMEFAILED, TH_ALIGN_CENTER);
		return LTFALSE;
	}

	return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::IsMultiplayerGame()
//
//	PURPOSE:	See if we are playing a multiplayer game
//
// --------------------------------------------------------------------------- //

LTBOOL CRiotClientShell::IsMultiplayerGame()
{
	if (!g_pLTClient ) return LTFALSE;

	int nGameMode = 0;
	g_pLTClient->GetGameMode(&nGameMode);
	if (nGameMode == STARTGAME_NORMAL || nGameMode == GAMEMODE_NONE) return LTFALSE;

	return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::IsPlayerInWorld()
//
//	PURPOSE:	See if the player is in the world
//
// --------------------------------------------------------------------------- //

LTBOOL CRiotClientShell::IsPlayerInWorld()
{
	if (!g_pLTClient ) return LTFALSE;

	HLOCALOBJ hPlayerObj = pClientDE->GetClientObject();

	if (!m_bPlayerPosSet || !m_bInWorld || m_ePlayerState == PS_UNKNOWN || !hPlayerObj) return LTFALSE;

	return LTTRUE;
}


void CRiotClientShell::GetCameraRotation(LTRotation *pRot)
{
	g_pLTClient->Math()->SetupEuler(*pRot, m_fPitch, m_fYaw, m_fCamCant);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::StartGame
//
//	PURPOSE:	Start a new game
//
// --------------------------------------------------------------------------- //

LTBOOL CRiotClientShell::StartGame(GameDifficulty eDifficulty)
{
	SetDifficulty(eDifficulty);

	// Play the intro first...

	SetGameState(GS_INTRO);
	SetMenuMusic(LTFALSE);

	return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::DoStartGame
//
//	PURPOSE:	*Really* Start a new game
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::DoStartGame()
{
#ifdef _DEMO
	CreateBumperScreen("Demo", 1606);
	LoadWorld("Worlds\\demo_mca");
#else
	CreateBumperScreen("Ambush", 1600);
	LoadWorld("Worlds\\01_Ambush");
#endif
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::GetNiceWorldName
//
//	PURPOSE:	Get the nice (level designer set) world name...
//
// --------------------------------------------------------------------------- //

LTBOOL CRiotClientShell::GetNiceWorldName(char* pWorldFile, char* pRetName, int nRetLen)
{
	if (!g_pLTClient || !pWorldFile || !pRetName || nRetLen < 2) return LTFALSE;

	char buf[_MAX_PATH];
	buf[0] = '\0';
	DWORD len;

	char buf2[_MAX_PATH];
	sprintf(buf2, "%s.dat", pWorldFile);

	LTRESULT dRes = g_pLTClient->GetWorldInfoString(buf2, buf, _MAX_PATH, &len);

	if (dRes != LT_OK || !buf[0] || len < 1)
	{
		// try pre-pending "worlds\" to the filename to see if it will find it then...

		sprintf (buf2, "worlds\\%s.dat", pWorldFile);
		dRes = g_pLTClient->GetWorldInfoString(buf2, buf, _MAX_PATH, &len);

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
//	ROUTINE:	CRiotClientShell::DisplayGenericPickupMessage
//
//	PURPOSE:	Display a message when an item is picked up
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::DisplayGenericPickupMessage(PickupItemType eType)
{
	if (!g_pLTClient || eType < 0 || eType >= PIT_MAX) return;

	static uint32 s_pickupItemToStringIdMap[PIT_MAX] =
	{
		IDS_PIT_UNKNOWN, 
		
		// Armor...

		IDS_PIT_ARMOR_REPAIR100,
		IDS_PIT_ARMOR_REPAIR250,
		IDS_PIT_ARMOR_REPAIR500,
		IDS_PIT_ARMOR_BODY50,
		IDS_PIT_ARMOR_BODY100,
		IDS_PIT_ARMOR_BODY200,

		// Enhancements...

		IDS_PIT_ENHANCEMENT_DAMAGE,
		IDS_PIT_ENHANCEMENT_MELEEDAMAGE,
		IDS_PIT_ENHANCEMENT_PROTECTION,
		IDS_PIT_ENHANCEMENT_ENERGYPROTECTION,
		IDS_PIT_ENHANCEMENT_PROJECTILEPROTECTION,
		IDS_PIT_ENHANCEMENT_EXPLOSIVEPROTECTION,
		IDS_PIT_ENHANCEMENT_REGEN,
		IDS_PIT_ENHANCEMENT_HEALTH,
		IDS_PIT_ENHANCEMENT_ARMOR,

		// First aid...

		IDS_PIT_FIRSTAID_10,
		IDS_PIT_FIRSTAID_15,
		IDS_PIT_FIRSTAID_25,
		IDS_PIT_FIRSTAID_50,

		IDS_PIT_POWERSURGE_50,
		IDS_PIT_POWERSURGE_100,
		IDS_PIT_POWERSURGE_150,
		IDS_PIT_POWERSURGE_250,

		// Ultra Powerups...

		IDS_PIT_ULTRA_DAMAGE, 
		IDS_PIT_ULTRA_HEALTH, 
		IDS_PIT_ULTRA_POWERSURGE, 
		IDS_PIT_ULTRA_SHIELD, 
		IDS_PIT_ULTRA_STEALTH, 
		IDS_PIT_ULTRA_REFLECT, 
		IDS_PIT_ULTRA_NIGHTVISION, 
		IDS_PIT_ULTRA_INFRARED, 
		IDS_PIT_ULTRA_SILENCER, 
		IDS_PIT_ULTRA_RESTORE,

		// Uprades...

		IDS_PIT_UPGRADE_DAMAGE,
		IDS_PIT_UPGRADE_PROTECTION,
		IDS_PIT_UPGRADE_REGEN,
		IDS_PIT_UPGRADE_HEALTH,
		IDS_PIT_UPGRADE_ARMOR,
		IDS_PIT_UPGRADE_TARGETING,

		// Misc...

		IDS_PIT_CAT,
		IDS_PIT_SHOGO_S,
		IDS_PIT_SHOGO_H,
		IDS_PIT_SHOGO_O,
		IDS_PIT_SHOGO_G
	};

	uint32 dwItemId = s_pickupItemToStringIdMap[eType];
	if (dwItemId == IDS_PIT_UNKNOWN) return;

	HSTRING hStr = g_pLTClient->FormatString(dwItemId);
	if (!hStr) return;

	const char* pStr = g_pLTClient->GetStringData(hStr);
	if (!pStr) return;

	uint32 nWidth, nHeight;
	HSURFACE hScreen = g_pLTClient->GetScreenSurface();
	g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);
	
	CBitmapFont* pFont = m_menu.GetFont12s();
	if (nWidth < 640)
	{
		pFont = m_menu.GetFont08s();
	}
	
	g_pLTClient->CPrint(pStr);
	m_infoDisplay.AddInfo(pStr, pFont, 2.0f, DI_CENTER | DI_BOTTOM);
	g_pLTClient->FreeString(hStr);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::DoPickupItemScreenTint
//
//	PURPOSE:	Tint the screen when an item is picked up
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::DoPickupItemScreenTint(PickupItemType eType)
{
	if (!g_pLTClient || eType < 0 || eType >= PIT_MAX || !m_hCamera) return;

	static LTVector s_vPickupItemTintColor[PIT_MAX] =
	{
		PICKUPITEM_TINT_UNKNOWN,		// Unknown 

		// Armor...

		PICKUPITEM_TINT_ARMOR,			// Armor repair 100
		PICKUPITEM_TINT_ARMOR,			// Armor repair 250
		PICKUPITEM_TINT_ARMOR,			// Armor repair 500
		PICKUPITEM_TINT_ARMOR,			// Body armor	50
		PICKUPITEM_TINT_ARMOR,			// Body armor	100
		PICKUPITEM_TINT_ARMOR,			// Body armor	200
	
		// Enhancements...

		PICKUPITEM_TINT_WEAPON,			// Damage Enhancement
		PICKUPITEM_TINT_WEAPON,			// Melee Damage Enhancement
		PICKUPITEM_TINT_ARMOR,			// Protection Enhancement
		PICKUPITEM_TINT_ARMOR,			// Energy Protection Enhancement
		PICKUPITEM_TINT_ARMOR,			// Projectile Protection Enhancement
		PICKUPITEM_TINT_ARMOR,			// Explosive Protection Enhancement
		PICKUPITEM_TINT_ARMOR,			// Regeneration Enhancement
		PICKUPITEM_TINT_HEALTH,			// Health Enhancement
		PICKUPITEM_TINT_ARMOR,			// Armor Enhancement

		// First aid...

		PICKUPITEM_TINT_HEALTH,			// First Aid 10
		PICKUPITEM_TINT_HEALTH,			// First Aid 15
		PICKUPITEM_TINT_HEALTH,			// First Aid 25
		PICKUPITEM_TINT_HEALTH,			// First Aid 50

		PICKUPITEM_TINT_HEALTH,			// Power Surge 50
		PICKUPITEM_TINT_HEALTH,			// Power Surge 100
		PICKUPITEM_TINT_HEALTH,			// Power Surge 150
		PICKUPITEM_TINT_HEALTH,			// Power Surge 250


		// Ultra Powerups...

		PICKUPITEM_TINT_WEAPON,			// Ultra Damage
		PICKUPITEM_TINT_HEALTH,			// Ultra Health
		PICKUPITEM_TINT_HEALTH,			// Ultra Power Surge
		PICKUPITEM_TINT_ARMOR,			// Ultra Shield
		PICKUPITEM_TINT_ARMOR,			// Ultra Stealth
		PICKUPITEM_TINT_ARMOR,			// Ultra Reflect
		PICKUPITEM_TINT_UNKNOWN,		// Ultra Night Vision
		PICKUPITEM_TINT_UNKNOWN,		// Ultra Infrared
		PICKUPITEM_TINT_WEAPON,			// Ultra Silencer
		PICKUPITEM_TINT_HEALTH,			// Ultra Restore

		// Uprades...

		PICKUPITEM_TINT_WEAPON,			// Damage Upgrade
		PICKUPITEM_TINT_ARMOR,			// Protection Upgrade
		PICKUPITEM_TINT_HEALTH,			// Regeneration Upgrade
		PICKUPITEM_TINT_HEALTH,			// Health Upgrade
		PICKUPITEM_TINT_ARMOR,			// Armor Upgrade
		PICKUPITEM_TINT_WEAPON,			// Targeting Upgrade


		// Misc...

		PICKUPITEM_TINT_UNKNOWN,		// Cat
		PICKUPITEM_TINT_UNKNOWN,		// S
		PICKUPITEM_TINT_UNKNOWN,		// H
		PICKUPITEM_TINT_UNKNOWN,		// O
		PICKUPITEM_TINT_UNKNOWN			// G
	};


	LTVector vTintColor;
	VEC_COPY(vTintColor, s_vPickupItemTintColor[eType]);
	LTFLOAT fRampDown = 2.0f;
	LTFLOAT fRampUp = 0.2f, fTintTime = 0.1f;
	
	LTVector vCamPos;
	g_pLTClient->GetObjectPos(m_hCamera, &vCamPos);

	LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hCamera, &rRot);

	LTVector vU, vR, vF;
	g_pLTClient->Math()->GetRotationVectors(rRot, vU, vR, vF);

	VEC_MULSCALAR(vF, vF, 10.0f);
	VEC_ADD(vCamPos, vCamPos, vF);

	TintScreen(vTintColor, vCamPos, 1000.0f, fRampUp, fTintTime, fRampDown, LTTRUE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::OnModelKey
//
//	PURPOSE:	Handle weapon model keys
//
// --------------------------------------------------------------------------- //

void CRiotClientShell::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	m_weaponModel.OnModelKey(hObj, pArgs);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::CreateBoundingBox
//
//	PURPOSE:	Create a box around the MoveMgr object
//
// --------------------------------------------------------------------------- //
	
void CRiotClientShell::CreateBoundingBox()
{
	if (!g_pLTClient || !m_MoveMgr || m_hBoundingBox) return;

	HLOCALOBJ hMoveMgrObj = m_MoveMgr->GetObject();
	if (!hMoveMgrObj) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	LTVector vPos;
	g_pLTClient->GetObjectPos(hMoveMgrObj, &vPos);
	VEC_COPY(theStruct.m_Pos, vPos);

	SAFE_STRCPY(theStruct.m_Filename, "Models\\Props\\1x1_square.abc");
	SAFE_STRCPY(theStruct.m_SkinName, "SpecialFX\\smoke.dtx");
	theStruct.m_ObjectType = OT_MODEL;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_MODELWIREFRAME;

	m_hBoundingBox = g_pLTClient->CreateObject(&theStruct);

	UpdateBoundingBox();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateBoundingBox()
//
//	PURPOSE:	Update the bounding box
//
// ----------------------------------------------------------------------- //
	
void CRiotClientShell::UpdateBoundingBox()
{
	if (!g_pLTClient || !m_hBoundingBox) return;

	HLOCALOBJ hMoveMgrObj = m_MoveMgr->GetObject();
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
void LoadConVar(ILTClient *pClientDE, ILTStream *pStream, char *pVarName)
{
	LTFLOAT val;
	char cString[512];

	(*pStream) >> val;
	sprintf(cString, "%s %f", pVarName, val);
	pClientDE->RunConsoleString(cString);
}

void SaveConVar(ILTClient *pClientDE, ILTStream *pStream, char *pVarName, LTFLOAT defaultVal)
{
	HCONSOLEVAR hVar;
	LTFLOAT val;

	val = defaultVal;
	if(hVar = pClientDE->GetConsoleVar (pVarName))
	{
		val = pClientDE->GetVarValueFloat (hVar);
	}

	(*pStream) << val;
}

void CRiotClientShell::DemoSerialize(ILTStream *pStream, LTBOOL bLoad)
{
	CRiotSettings* pSettings = m_menu.GetSettings();
	uint32 i;

	if(bLoad)
	{
		LoadConVar(g_pLTClient, pStream, NORMAL_TURN_RATE_VAR);
		LoadConVar(g_pLTClient, pStream, FAST_TURN_RATE_VAR);
		
		for(i=RS_CTRL_FIRST; i <= RS_CTRL_LAST; i++)
		{		
			(*pStream) >> pSettings->ControlSetting(i);
		}
	
		for(i=RS_DET_FIRST; i <= RS_DET_LAST; i++)
		{		
			(*pStream) >> pSettings->DetailSetting(i);
		}

		for(i=RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
		{		
			(*pStream) >> pSettings->SubDetailSetting(i);
		}
	}
	else
	{
		SaveConVar(g_pLTClient, pStream, NORMAL_TURN_RATE_VAR, DEFAULT_NORMAL_TURN_SPEED);
		SaveConVar(g_pLTClient, pStream, FAST_TURN_RATE_VAR, DEFAULT_FAST_TURN_SPEED);

		for(i=RS_CTRL_FIRST; i <= RS_CTRL_LAST; i++)
		{		
			(*pStream) << pSettings->ControlSetting(i);
		}
	
		for(i=RS_DET_FIRST; i <= RS_DET_LAST; i++)
		{		
			(*pStream) << pSettings->DetailSetting(i);
		}

		for(i=RS_SUBDET_FIRST; i <= RS_SUBDET_LAST; i++)
		{		
			(*pStream) << pSettings->SubDetailSetting(i);
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	LoadLeakFile
//
//	PURPOSE:	Loads a leak file and creates a line system for it.
//
// --------------------------------------------------------------------------- //

LTBOOL LoadLeakFile(ILTClient *pClientDE, char *pFilename)
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
		hObj = pClientDE->CreateObject(&cStruct);
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

			pClientDE->AddLine(hObj, &theLine);
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

	if (!pClientDE) return(LTFALSE);
	if (!sAddress) return(LTFALSE);


	// Try to connect to the given address...

	LTBOOL db = NetStart_DoConsoleConnect(pClientDE, sAddress);

	if (!db)
	{
		if (strlen(sAddress) <= 0) pClientDE->CPrint("Unable to connect");
		else pClientDE->CPrint("Unable to connect to %s", sAddress);
		return(LTFALSE);
	}


	// All done...

	if (strlen(sAddress) > 0) pClientDE->CPrint("Connected to %s", sAddress);
	else pClientDE->CPrint("Connected");

	return(LTTRUE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	NVModelHook
//
//	PURPOSE:	Special Rendering Code for NightVision Powerup
//
// --------------------------------------------------------------------------- //

void NVModelHook (struct ModelHookData_t *pData, void *pUser)
{
	CRiotClientShell* pShell = (CRiotClientShell*) pUser;
	if (!pShell) return;
	
	if (!g_pLTClient) return;
	
	uint32 nUserFlags = 0;
	g_pLTClient->Common()->GetObjectUserFlags (pData->m_hObject, OFT_User, nUserFlags);
	if (nUserFlags & USRFLG_NIGHT_INFRARED)
	{
		pData->m_Flags &= ~MHF_USETEXTURE;
		if (pData->m_LightAdd)
		{
			VEC_SET (*pData->m_LightAdd, 0.0f, 255.0f, 0.0f);
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

void IRModelHook (struct ModelHookData_t *pData, void *pUser)
{
	CRiotClientShell* pShell = (CRiotClientShell*) pUser;
	if (!pShell) return;
	
	if (!g_pLTClient) return;

	uint32 nUserFlags = 0;
	g_pLTClient->Common()->GetObjectUserFlags (pData->m_hObject, OFT_User, nUserFlags);
	if (nUserFlags & USRFLG_NIGHT_INFRARED)
	{
		pData->m_Flags &= ~MHF_USETEXTURE;
		if (pData->m_LightAdd)
		{
			VEC_SET (*pData->m_LightAdd, 255.0f, 64.0f, 64.0f);
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

void DefaultModelHook (struct ModelHookData_t *pData, void *pUser)
{
	CRiotClientShell* pShell = (CRiotClientShell*) pUser;
	if (!pShell) return;
	
	if (!g_pLTClient) return;

	uint32 nUserFlags = 0;
	g_pLTClient->Common()->GetObjectUserFlags (pData->m_hObject, OFT_User, nUserFlags);

	if (nUserFlags & USRFLG_GLOW)
	{
		// MD {Updates model glow in Update}
		//pShell->UpdateModelGlow(vColor);

		if (pData->m_LightAdd)
		{
			*pData->m_LightAdd = pShell->GetModelGlow();
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
		}
	}
}



