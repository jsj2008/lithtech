//----------------------------------------------------------
//
// MODULE  : BloodClientShell.cpp
//
// PURPOSE : Blood 2 Client shell
//
// CREATED : 9/21/97
//
//----------------------------------------------------------


// includes...
#include <stdarg.h>
#include <stdio.h>
#include "BloodClientShell.h"
#include "ViewWeapon.h"
#include "Commands.h"
#include "VKDefs.h"
#include "ClientRes.h"
#include "SfxMsgIds.h"
#include "ClientUtilities.h"
#include "LoadSave.h"
#include "VolumeBrushFX.h"
#include "CameraFX.h"
#include "client_physics.h"
#include "NetStart.h"
#include "TheVoice.h"
#include "WinUtil.h"
#include "ClientRes.h"
#include "LoadScreenData.h"
#include "Splash.h"
#include "MoveMgr.h"
#include "WeaponDefs.h"
#include "GameWeapons.h"
#include "SoundTypes.h"
#include "physics_lt.h"

PhysicsState	  g_normalPhysicsState;
PhysicsState	  g_waterPhysicsState;

//#define FLAG_BASECHARACTER	1

// defines...

#define BOBV_WALKING	24.0f
#define BOBH_WALKING	16.0f
#define SWAYV_WALKING	32.0f
#define SWAYH_WALKING	80.0f

#define BOBV_CRAWLING	24.0f
#define BOBH_CRAWLING	16.0f
#define SWAYV_CRAWLING	16.0f
#define SWAYH_CRAWLING	40.0f

#define FOV_NORMAL		90.0f
#define FOV_SEEING_EYE	120.0f
#define FOV_ZOOMED		12.5f

#define MOVIE_STARTLITH		1
#define MOVIE_PLAYINGLITH	2
#define MOVIE_STARTGTI		3
#define MOVIE_PLAYINGGTI	4
#define MOVIE_STARTINTRO	5
#define MOVIE_PLAYINGINTRO	6
#define MOVIE_FINISHED		7

#define min(a,b)	(a < b ? a : b)
#define max(a,b)	(a > b ? a : b)

#define SOUND_REVERB_UPDATE_PERIOD		0.33f
#define PLAYER_MULTI_UPDATE_TIMER		0.14f

#define	MSG_SOUND_STRING	"sounds\\events\\woodclank2a.wav"


// Guids...

#ifdef _ADDON

DGUID BLOOD2GUID = { /* Add-On */ 
	0xc13e628, 0x418c, 0x11d2, 0x86, 0xa, 0x0, 0x60, 0x97, 0x18, 0xa9, 0x42
};

#else

DGUID BLOOD2GUID = { /* 0C13E629-419C-11D2-860A-00609719A842 */
	0xc13e629, 0x419c, 0x11d2, 0x86, 0xa, 0x0, 0x60, 0x97, 0x19, 0xa8, 0x42
};

#endif


// Misc...

char	msgbuf[513];

DBOOL	g_bAborting                = DFALSE;
DBOOL	g_bIsHost                  = DFALSE;
DBOOL	g_bLevelChange3rdPersonCam = DFALSE;
DBOOL	g_bLevelChangeNoUpdate     = DFALSE;
DBOOL	g_bPlayerInvReset          = DFALSE;

DFLOAT	g_fPlayerUpdateTimer       = 0.0f;

#ifdef _ADDON

static char *s_szCheerSounds[] =
{
	"sounds_ao\\events\\cheer1.wav",
	"sounds_ao\\events\\cheer2.wav"
};
#define NUMCHEERSOUNDS  ( sizeof( s_szCheerSounds ) / sizeof( s_szCheerSounds[0] ))

static char *s_szBooSounds[] =
{
	"sounds_ao\\events\\boo1.wav",
	"sounds_ao\\events\\boo2.wav",
	"sounds_ao\\events\\boo3.wav"
};
#define NUMBOOSOUNDS  ( sizeof( s_szBooSounds ) / sizeof( s_szBooSounds[0] ))

#endif // _ADDON



// Global pointer to the client shell..
CBloodClientShell	*g_pBloodClientShell = DNULL;

// Prototypes..
DBOOL ConnectToTcpIpAddress(CClientDE* pClientDE, char* sAddress);

// Setup..

SETUP_CLIENTSHELL();

ClientShellDE* CreateClientShell(ClientDE *pClientDE)
{
	g_pClientDE = pClientDE;
	return (ClientShellDE*)(new CBloodClientShell);
}

void DeleteClientShell(ClientShellDE *pInputShell)
{
	if (pInputShell)
	{
		delete ((CBloodClientShell*)pInputShell);
	}
}

void CShell_GetConsoleString(char* sKey, char* sDest, char* sDefault)
{
	if (g_pClientDE)
	{
		HCONSOLEVAR hVar = g_pClientDE->GetConsoleVar(sKey);
		if (hVar)
		{
			char* sValue = g_pClientDE->GetVarValueString(hVar);
			if (sValue)
			{
				strcpy(sDest, sValue);
				return;
			}
		}
	}

	strcpy(sDest, sDefault);
}

static DBOOL LoadLeakFile(ClientDE *pClientDE, char *pFilename);

void LeakFileFn(int argc, char **argv)
{
	if (argc < 0)
	{
		g_pBloodClientShell->GetClientDE()->CPrint("LeakFile <filename>");
		return;
	}

	if (LoadLeakFile(g_pBloodClientShell->GetClientDE(), argv[0]))
	{
		g_pBloodClientShell->GetClientDE()->CPrint("Leak file %s loaded successfully!", argv[0]);
	}
	else
	{
		g_pBloodClientShell->GetClientDE()->CPrint("Unable to load leak file %s", argv[0]);
	}
}

void ConnectFn(int argc, char **argv)
{
	if (argc <= 0)
	{
		g_pBloodClientShell->GetClientDE()->CPrint("Connect <tcpip address> (use '*' for local net)");
		return;
	}

	ConnectToTcpIpAddress(g_pBloodClientShell->GetClientDE(), argv[0]);
}

void NextLevelFn(int argc, char **argv)
{
	if (g_bIsHost)
	{
		ClientDE *pClientDE;
		HMESSAGEWRITE hWrite;

		pClientDE = g_pBloodClientShell->GetClientDE();
		hWrite = pClientDE->StartMessage(CMSG_NEXTLEVEL);
		pClientDE->EndMessage(hWrite);
	}
	else
	{
		g_pBloodClientShell->GetClientDE()->CPrint("Only the host can change levels");
	}
}

void LoadLevelFn(int argc, char **argv)
{
	if (argc >= 1 && argv[0])
	{
		g_pBloodClientShell->StartNewGame(argv[0]);
	}
	else
	{
		g_pBloodClientShell->GetClientDE()->CPrint("Usage: load <level-name>");
	}
}

void CBloodClientShell::StartNewGame(char *pszWorld)
{
	if (!pszWorld) return;

	if (m_bInWorld)
	{
		StartNewWorld(pszWorld, m_nGameType, LOADTYPE_NEW_LEVEL);
	}
	else
	{
		StartNewGame(pszWorld, STARTGAME_NORMAL, GAMETYPE_CUSTOM, LOADTYPE_NEW_GAME, DIFFICULTY_MEDIUM);
		m_bFirstWorld = DTRUE;
	}
}

void FragSelfFn(int argc, char **argv)
{
	ClientDE *pClientDE;
	HMESSAGEWRITE hWrite;

	pClientDE = g_pBloodClientShell->GetClientDE();
	hWrite = pClientDE->StartMessage(CMSG_FRAGSELF);
	pClientDE->EndMessage(hWrite);
}

void InitSoundFn(int argc, char **argv)
{
	if( g_pBloodClientShell )
		g_pBloodClientShell->InitSound( );
}


/*
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	AuraModelHook
//
//	PURPOSE:	Special Rendering Code for Aura spell
//
// --------------------------------------------------------------------------- //

void AuraModelHook (struct ModelHookData_t *pData, void *pUser)
{
	CClientDE* pClientDE = (CClientDE*) pUser;
	if (!pClientDE ) return;
	
	DDWORD nUserFlags = 0;
	pClientDE->GetObjectUserFlags (pData->m_hObject, &nUserFlags);
	if (nUserFlags & FLAG_BASECHARACTER)
	{
		pData->m_Flags &= ~MHF_USETEXTURE;
		if (pData->m_LightAdd)
		{
			VEC_SET (*pData->m_LightAdd, 255.0f, 255.0f, 255.0f);
		}
	}
}
*/

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DefaultModelHook
//
//	PURPOSE:	Default model hook function
//
// --------------------------------------------------------------------------- //

void DefaultModelHook (struct ModelHookData_t *pData, void *pUser)
{
	DDWORD nUserFlags;
	DFLOAT fGlowValue, fGlowFactor;

	g_pClientDE->GetObjectUserFlags( pData->m_hObject, &nUserFlags );

	if( g_pBloodClientShell->NightGogglesActive( ) && nUserFlags & USERFLG_NIGHTGOGGLESGLOW )
	{
		pData->m_Flags &= ~MHF_USETEXTURE;
		if (pData->m_LightAdd)
		{
			VEC_SET (*pData->m_LightAdd, 0.0f, 255.0f, 0.0f);
		}
	}
	else if( nUserFlags & USRFLG_GLOW )
	{
		if( pData->m_LightAdd )
		{
			// Cycle the glow with a period of 2 seconds...
			fGlowFactor = ( DFLOAT )sin( g_pClientDE->GetTime( ) * MATH_CIRCLE / 2.0f );
			fGlowValue = 128.0f + 128.0f * fGlowFactor;
			fGlowValue = DCLAMP( fGlowValue, 0.0f, 255.0f );
			VEC_SET( *pData->m_LightAdd, fGlowValue, fGlowValue, fGlowValue );
		}
	}
	// Glow red for anger
	else if (nUserFlags & USRFLG_ANGERGLOW)	
	{
		if( pData->m_LightAdd )
		{
			// Cycle the glow with a period of 0.5 seconds...
			fGlowFactor = ( DFLOAT )sin( g_pClientDE->GetTime( ) * MATH_CIRCLE * 6.0f );
			fGlowValue = 192.0f + 64.0f * fGlowFactor;
			fGlowValue = DCLAMP( fGlowValue, 0.0f, 255.0f );
			VEC_SET( *pData->m_LightAdd, fGlowValue, 255.0 - fGlowValue, 255.0 - fGlowValue );
		}
	}
	// Glow green for willpower
	else if (nUserFlags & USRFLG_WILLPOWERGLOW)
	{
		if( pData->m_LightAdd )
		{
			// Cycle the glow with a period of 0.5 seconds...
			fGlowFactor = ( DFLOAT )sin( g_pClientDE->GetTime( ) * MATH_CIRCLE * 6.0f );
			fGlowValue = 192.0f + 64.0f * fGlowFactor;
			fGlowValue = DCLAMP( fGlowValue, 0.0f, 255.0f );
			VEC_SET( *pData->m_LightAdd, 255.0 - fGlowValue, fGlowValue, 255.0 - fGlowValue );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::CBloodClientShell()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

CBloodClientShell::CBloodClientShell()
{
	m_pMoveMgr				= DNULL;
	m_hCamera				= NULL;
	m_fPitch = m_fYaw =	cdata.fPitch = cdata.fYaw = 0.0f;
	for (int i=0;i<CLIENTWEAPONSLOTS;i++)
	{
		m_abyWeaponID[i]	= WEAP_NONE;
		m_abyLWeaponID[i]	= WEAP_NONE;
	}
	m_pWeapon				= DNULL;
	m_pLWeapon				= DNULL;

	m_hWeapHandObj			= DNULL;
	m_hLWeapHandObj			= DNULL;

	m_nGameState			= GS_NONE;		

	m_bInWorld				= DFALSE;
	m_hCrosshair			= NULL;
	m_bShowCrosshair		= DFALSE;
	m_nCrosshair			= 0;
	m_nLastCrosshair		= 0;
	m_nCrosshairOriginalNum = 0;
	m_bCrosshairOriginallyOn = 0;
	m_cxCrosshair = m_cyCrosshair = 0;
	m_DefFovX				= FOV_NORMAL;
	m_fovYScale				= 0.80f;
	m_fovX					= 0;
	m_fovY					= 0;

	m_fCantIncrement		= 0.006f; // in radians (.25 degrees)
	m_fCantMaxDist			= 0.036f; // in radians (2.0 degrees)
	m_fCamCant				= 0.0f;

	cdata.byFlags			= 0;

	// Set up default attributes
	_mbscpy((unsigned char*)m_Config.szName, (const unsigned char*)"Player");
	m_Config.nCharacter		= CHARACTER_CALEB;
	m_Config.nSkin			= MULTIPLAY_SKIN_NORMAL;
	m_Config.nStrength		= 5;
	m_Config.nSpeed			= 3;
	m_Config.nResistance	= 3;
	m_Config.nMagic			= 1;
	m_bDrawStatusBar		= DTRUE;
	m_bDrawFragBar			= DFALSE;

    m_bRenderCamera			= DFALSE;
	m_pExternalCamera		= DNULL;
	m_hSeeingEyeObj			= DNULL;
	m_hOrbObj				= DNULL;
	m_hLastOrbObj			= DNULL;
 
	m_bBurn					= DFALSE;
    m_bBlind				= DFALSE;
	m_bFadeIn				= DFALSE;
	m_bFadeOut				= DFALSE;
	m_fFadeVal				= 0.0f;
    m_fBurnTime				= 0.0f;
    m_fBlindTime			= 0.0f;

	m_hstrTitle				= DNULL;
	m_hstrLoadScreen		= DNULL;
	m_nLoadScreenID			= 0;

	// Advanced options
	m_bAdvancedDisableMusic			= DFALSE;
	m_bAdvancedDisableSound			= DFALSE;
	m_bAdvancedDisableMovies		= DFALSE;
	m_bAdvancedDisableJoystick		= DFALSE;
	m_bAdvancedEnableOptSurf		= DFALSE;
	m_bAdvancedDisableLightMap		= DFALSE;
	m_bAdvancedEnableTripBuf		= DFALSE;
	m_bAdvancedDisableDx6Cmds		= DFALSE;
	m_bAdvancedEnableTJuncs			= DFALSE;
	m_bAdvancedDisableFog			= DFALSE;
	m_bAdvancedDisableLines			= DFALSE;
	m_bAdvancedDisableModelFB		= DFALSE;
	m_bAdvancedEnablePixelDoubling	= DFALSE;
	m_bAdvancedEnableMultiTexturing = DFALSE;

	// Container init
	m_hContainerSound		= DNULL;
	m_eCurContainerCode		= CC_NOTHING;

	m_fContainerStartTime	= -1.0f;
	m_fFovXFXDir			= 1.0f;
	m_fLastTime				= 0.0f;

	m_fRotDir				= 1.0f;
	m_fMaxRot				= 0.55f;

	m_bFirstUpdate			= DFALSE;

	m_hLoadingScreen		= DNULL;
	m_fLoadingFadeTime		= 0.0f;
	m_bLoadingFadeUp		= DTRUE;
	m_bPlayedWaitingSound	= DFALSE;

	m_bHandledStartup		= DFALSE;
	m_bFirstWorld			= DTRUE;

	m_SavedGameInfo.LoadInfo();

	g_pBloodClientShell		= this;
	m_bDemoPlayback			= DFALSE;

	m_bShiftState			= DFALSE;
	m_bRollCredits          = DFALSE;

	_mbscpy((unsigned char*)m_szFilename, (const unsigned char*)"");

	m_pCreature = DNULL;

#ifdef BRANDED
	m_fBrandCounter			= 45.0f;
	m_hBrandSurface			= DNULL;
#endif

	m_bIgnoreKeyboardMessage=DFALSE;
	g_bAborting = DFALSE;

	m_hCurrentItemName = DNULL;
	m_hCurrentItemIcon = DNULL;
	m_hCurrentItemIconH = DNULL;
	m_nCurrentItemCharge = 0;
	m_hPrevItemIcon = DNULL;
	m_nPrevItemCharge = 0;
	m_hNextItemIcon = DNULL;
	m_nNextItemCharge = 0;

	m_nGlobalDetail = DETAIL_LOW;

	m_bMouseInvertYAxis = DFALSE;
	m_bUseJoystick = DFALSE;
	m_bMouseLook = DTRUE;
	m_bLookSpring = DFALSE;

	m_bNightGogglesActive = DFALSE;
	m_hNightGogglesSound = DNULL;
	m_hTheEyeLoopingSound = DNULL;

	m_bBinocularsActive = DFALSE;

	m_bPaused = DFALSE;
	m_bInMessageBox = DFALSE;

	m_eMusicLevel			= CMusic::MUSICLEVEL_SILENCE;

	// Camera stuff...
	m_fOffsetY = 0.0f;
	m_fOffsetX = 0.0f;
	m_fOffsetRot = 0.0f;
	ROT_INIT(m_Rotation);

	m_hstrObjectivesText = DNULL;
	m_hstrObjectivesTitle = DNULL;

	m_dwAmmo = 0;
	m_dwAltAmmo = 0;

	m_hCtfCapturedString1 = DNULL;
	m_hCtfCapturedString2 = DNULL;

#ifdef _ADDON
	m_hSoccerGoalString1 = DNULL;
	m_hSoccerGoalString2 = DNULL;
#endif // _ADDON

	m_nLastLoadType = LOADTYPE_NEW_GAME;

	m_bUseReverb = DFALSE;
	m_fReverbLevel = 0.0f;
	VEC_INIT( m_vLastReverbPos );

	m_bNetFriendlyFire   = DFALSE;
	m_bNetNegTeamFrags   = DFALSE;
	m_bNetOnlyFlagScores = DFALSE;
	m_bNetOnlyGoalScores = DFALSE;

	m_nTrapped = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::~CBloodClientShell()
//
//	PURPOSE:	Destruction
//
// ----------------------------------------------------------------------- //
			
CBloodClientShell::~CBloodClientShell()
{
	CClientDE* pClientDE = GetClientDE();


	if (m_pMoveMgr)
	{
		delete m_pMoveMgr;
		m_pMoveMgr = DNULL;
	}

	if (m_pWeapon) delete m_pWeapon;
	if (m_pLWeapon) delete m_pLWeapon;

	g_pBloodClientShell = DNULL;
	if( m_hCurrentItemName )
		pClientDE->FreeString( m_hCurrentItemName );
	if( m_hCurrentItemIcon )
		pClientDE->FreeString( m_hCurrentItemIcon );
	if( m_hCurrentItemIconH )
		pClientDE->FreeString( m_hCurrentItemIconH );
	if( m_hPrevItemIcon )
		pClientDE->FreeString( m_hPrevItemIcon );
	if( m_hNextItemIcon )
		pClientDE->FreeString( m_hNextItemIcon );

	if( m_hNightGogglesSound )
	{
		g_pClientDE->KillSound( m_hNightGogglesSound );
	}

	if( m_hTheEyeLoopingSound )
	{
		g_pClientDE->KillSound( m_hTheEyeLoopingSound );
	}

	if( m_hContainerSound)
	{
		g_pClientDE->KillSound( m_hContainerSound );
	}

	if( m_hstrTitle )
		g_pClientDE->FreeString( m_hstrTitle );
	if( m_hstrLoadScreen )
		g_pClientDE->FreeString( m_hstrLoadScreen );
	if( m_hstrObjectivesText )
		g_pClientDE->FreeString( m_hstrObjectivesText );
	if( m_hstrObjectivesTitle )
		g_pClientDE->FreeString( m_hstrObjectivesTitle );

	if (m_hCtfCapturedString1)
		g_pClientDE->FreeString(m_hCtfCapturedString1);
	if (m_hCtfCapturedString2)
		g_pClientDE->FreeString(m_hCtfCapturedString2);

#ifdef _ADDON
	if (m_hSoccerGoalString1)
		g_pClientDE->FreeString(m_hSoccerGoalString1);
	if (m_hSoccerGoalString2)
		g_pClientDE->FreeString(m_hSoccerGoalString2);
#endif // _ADDON
}	



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::InitSound
//
//	PURPOSE:	Initialize the sounds
//
// ----------------------------------------------------------------------- //

DRESULT CBloodClientShell::InitSound()
{
	Sound3DProvider *pSound3DProviderList, *pSound3DProvider;
	InitSoundInfo soundInfo;
	ReverbProperties reverbProperties;
	HCONSOLEVAR hVar;
	DDWORD dwProviderID;
	char sz3dSoundProviderName[_MAX_PATH + 1];
	DRESULT res;

	CClientDE* pClientDE = GetClientDE();
	if( !pClientDE )
		return LT_ERROR;

	INITSOUNDINFO_INIT(soundInfo);

	// Reload the sounds if there are any
	soundInfo.m_dwFlags				= INITSOUNDINFOFLAG_RELOADSOUNDS;

	// Get the 3d sound provider id.
	hVar = pClientDE->GetConsoleVar( "3DSoundProvider" );
	if( hVar )
	{
		dwProviderID = ( DDWORD )pClientDE->GetVarValueFloat( hVar );
	}
	else
		dwProviderID = SOUND3DPROVIDERID_NONE;

	// Can also be set by provider name, in which case the id will be set to UNKNOWN.
	if( dwProviderID == SOUND3DPROVIDERID_NONE || dwProviderID == SOUND3DPROVIDERID_UNKNOWN )
	{
		sz3dSoundProviderName[0] = 0;
		hVar = pClientDE->GetConsoleVar( "3DSoundProviderName" );
		if( hVar )
		{
			SAFE_STRCPY( sz3dSoundProviderName, pClientDE->GetVarValueString( hVar ));
			dwProviderID = SOUND3DPROVIDERID_UNKNOWN;
		}
	}

	// See if the provider exists.
	if( dwProviderID != SOUND3DPROVIDERID_NONE )
	{
		pClientDE->GetSound3DProviderLists( pSound3DProviderList, DFALSE );
		if( !pSound3DProviderList )
			return LT_NO3DSOUNDPROVIDER;

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
			hVar = pClientDE->GetConsoleVar( "Max3DVoices" );
			if( hVar )
			{
				soundInfo.m_nNum3DVoices = ( DBYTE )g_pClientDE->GetVarValueFloat( hVar );
			}
			else
				soundInfo.m_nNum3DVoices = 16;
		}

		pClientDE->ReleaseSound3DProviderList( pSound3DProviderList );
	}

	// Get the maximum number of sw voices to use.
	hVar = pClientDE->GetConsoleVar( "MaxSWVoices" );
	if( hVar )
	{
		soundInfo.m_nNumSWVoices = ( DBYTE )g_pClientDE->GetVarValueFloat( hVar );
	}
	else
		soundInfo.m_nNumSWVoices = 32;

	soundInfo.m_nSampleRate			= 22050;
	soundInfo.m_nBitsPerSample		= 16;
	if ((hVar = pClientDE->GetConsoleVar("soundvolume")))
	{
		soundInfo.m_nVolume = (unsigned short)(pClientDE->GetVarValueFloat(hVar));
	}		
	
	DBOOL b8bitOnly = DFALSE;
	hVar = pClientDE->GetConsoleVar("sound16bit");
	if (hVar)
	{
		b8bitOnly = (pClientDE->GetVarValueFloat(hVar) == 0.0f) ? DTRUE : DFALSE;
	}		
	if( b8bitOnly )
		soundInfo.m_dwFlags |= INITSOUNDINFOFLAG_CONVERT16TO8;


	DFLOAT fMinStreamSoundTime = 0.5f;
	hVar = pClientDE->GetConsoleVar ("MinStreamTime");
	if (hVar)
	{
		fMinStreamSoundTime = pClientDE->GetVarValueFloat (hVar);
		fMinStreamSoundTime = (fMinStreamSoundTime < 0.2f ? 0.2f :
							   (fMinStreamSoundTime > 1.5f ? 1.5f : fMinStreamSoundTime));
	}
	soundInfo.m_fMinStreamTime = fMinStreamSoundTime;
	soundInfo.m_fDistanceFactor = 1.0f / 64.0f;

	// Go initialize the sounds.
	m_bUseReverb = DFALSE;
	if(( res = pClientDE->InitSound(&soundInfo)) == LT_OK )
	{
		if( soundInfo.m_dwResults & INITSOUNDINFORESULTS_REVERB )
		{
			m_bUseReverb = DTRUE;
		}

		hVar = pClientDE->GetConsoleVar( "ReverbLevel" );
		if( hVar )
		{
			m_fReverbLevel = g_pClientDE->GetVarValueFloat( hVar );
		}
		else
			m_fReverbLevel = 1.0f;

		reverbProperties.m_dwParams = REVERBPARAM_VOLUME;
		reverbProperties.m_fVolume = m_fReverbLevel;
		pClientDE->SetReverbProperties( &reverbProperties );
	}

	return res;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::CSPrint
//
//	PURPOSE:	Displays a line of text on the client
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::CSPrint (char* msg, ...)
{
	CClientDE* pClientDE = GetClientDE();

	// parse the message
	char pMsg[256];
	va_list marker;
	va_start (marker, msg);
	int nSuccess = vsprintf (pMsg, msg, marker);
	va_end (marker);

	if (nSuccess < 0) return;
	 
	// now display the message
	m_MessageMgr.AddLine(pMsg);
	pClientDE->CPrint(pMsg);
}

void CBloodClientShell::CSPrint(int nStringID)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	HSTRING hString = pClientDE->FormatString(nStringID);
	if (!hString) return;

	CSPrint(pClientDE->GetStringData(hString));

	pClientDE->FreeString(hString);
}

void CBloodClientShell::CSPrint2(char* msg)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	m_MessageMgr.AddLine(msg);
	pClientDE->CPrint(msg);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnEvent()
//
//	PURPOSE:	Engine event handler
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::OnEvent(DDWORD dwEventID, DDWORD dwParam)
{
	// Handle any events we want to deal with...

	switch (dwEventID)
	{
		case LTEVENT_DISCONNECT:
		{
			if (!g_bIsHost)
			{
				CSPrint(IDS_SERVERDISCONNECT);
				PlaySound(MSG_SOUND_STRING);
			}
			break;
		}
	}


	// Let the base class handle the even too...

	CClientShellDE::OnEvent(dwEventID, dwParam);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnEngineInitialized()
//
//	PURPOSE:	Called upon engine initialization.
//
// --------------------------------------------------------------------------- //
DRESULT CBloodClientShell::OnEngineInitialized(RMode *pMode, DGUID *pAppGuid)
{
	// Local variables...

	HCONSOLEVAR hVar;
	CClientDE* pClientDE = GetClientDE();
	DBOOL bPlayingMovies = DFALSE;
	DRESULT resInitSound;
	HSTRING hstrError;
	char *pszError;

	// Set the guid...

	*pAppGuid = BLOOD2GUID;

	// Init Move mgr...
	m_pMoveMgr = new CMoveMgr(this);
	if(!m_pMoveMgr)
		return LT_ERROR;

	m_pMoveMgr->Init(pClientDE);

	// Initialize The Voice data...
	TheVoice_Init(pClientDE);

	// Initialize the physics states...
	g_normalPhysicsState.m_pClientDE = pClientDE;
	VEC_SET(g_normalPhysicsState.m_GravityAccel, 0.0f, -1000.0f, 0.0f);
	g_normalPhysicsState.m_VelocityDampen = 0.35f;

	g_waterPhysicsState.m_pClientDE = pClientDE;
	VEC_SET(g_waterPhysicsState.m_GravityAccel, 0.0f, -500.0f, 0.0f);
	g_waterPhysicsState.m_VelocityDampen = 0.20f;

	// Initialize sound...
	resInitSound = InitSound();

	// Add to NumRuns count...
	float nNumRuns = 0.0f;
	hVar = pClientDE->GetConsoleVar ("NumRuns");
	if (hVar)
	{
		nNumRuns = pClientDE->GetVarValueFloat (hVar);
	}
	nNumRuns++;
	
	char strConsole[64];
	sprintf (strConsole, "+NumRuns %f", nNumRuns);
	pClientDE->RunConsoleString (strConsole);


	// Format some strings...

	m_hCtfCapturedString1 = pClientDE->FormatString(IDS_CTF_CAPTURED_1);
	m_hCtfCapturedString2 = pClientDE->FormatString(IDS_CTF_CAPTURED_2);

#ifdef _ADDON
	m_hSoccerGoalString1 = pClientDE->FormatString(IDS_SOCCER_GOAL_1);
	m_hSoccerGoalString2 = pClientDE->FormatString(IDS_SOCCER_GOAL_2);
#endif // _ADDON


	// check advanced options...

	hVar = pClientDE->GetConsoleVar ("DisableMusic");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableMusic = DTRUE;
	hVar = pClientDE->GetConsoleVar ("DisableSound");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableSound = DTRUE;
	hVar = pClientDE->GetConsoleVar ("DisableMovies");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableMovies = DTRUE;
	hVar = pClientDE->GetConsoleVar ("DisableJoystick");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableJoystick = DTRUE;
	hVar = pClientDE->GetConsoleVar ("EnableOptSurf");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedEnableOptSurf = DTRUE;
	hVar = pClientDE->GetConsoleVar ("DisableLightMap");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableLightMap = DTRUE;
	hVar = pClientDE->GetConsoleVar ("EnableTripBuf");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedEnableTripBuf = DTRUE;
	hVar = pClientDE->GetConsoleVar ("DisableDx6Cmds");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableDx6Cmds = DTRUE;
	hVar = pClientDE->GetConsoleVar ("EnableTJuncs");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedEnableTJuncs = DTRUE;
	hVar = pClientDE->GetConsoleVar ("DisableFog");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableFog = DTRUE;
	hVar = pClientDE->GetConsoleVar ("DisableLines");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableLines = DTRUE;
	hVar = pClientDE->GetConsoleVar ("DisableModelFB");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedDisableModelFB = DTRUE;
	hVar = pClientDE->GetConsoleVar ("EnablePixDub");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedEnablePixelDoubling = DTRUE;
	hVar = pClientDE->GetConsoleVar ("EnableMultiTex");
	if (hVar && pClientDE->GetVarValueFloat (hVar))		m_bAdvancedEnableMultiTexturing = DTRUE;

	// Set the detail console variables
	MenuSetDetail();

	// Set the player cursor display
	hVar = pClientDE->GetConsoleVar ("Crosshair");
	if(hVar && pClientDE->GetVarValueFloat(hVar))
	{
		m_bShowCrosshair = DTRUE;
		m_bCrosshairOriginallyOn = DTRUE;
	}
	else
	{
		m_bShowCrosshair = DFALSE;
		m_bCrosshairOriginallyOn = DFALSE;
	}

	hVar = pClientDE->GetConsoleVar ("CrosshairNum");
	m_nCrosshair = (DBYTE)pClientDE->GetVarValueFloat(hVar);
	if(m_nCrosshair > 41) m_nCrosshair = 0;
	m_nCrosshairOriginalNum = m_nCrosshair;

	// Set the player movement variables (mouselook)
	hVar = pClientDE->GetConsoleVar ("MouseLook");
	if(hVar && pClientDE->GetVarValueFloat(hVar))
	{
		m_bMouseLook = DTRUE;
//		cdata.byFlags |= CDATA_MOUSEAIMING;
	}
	else
	{
		m_bMouseLook = DFALSE;
//		cdata.byFlags &= ~CDATA_MOUSEAIMING;
	}

	// Set the player movement variables (lookspring)
	hVar = pClientDE->GetConsoleVar ("LookSpring");
	if(hVar && pClientDE->GetVarValueFloat(hVar))
	{
		m_bLookSpring = DTRUE;
	}
	else
	{
		m_bLookSpring = DFALSE;
	}

	// Set the player movement variables (runlock)
	hVar = pClientDE->GetConsoleVar ("RunLock");
	if(hVar && pClientDE->GetVarValueFloat(hVar))
		cdata.byFlags |= CDATA_RUNLOCK;
	else
		cdata.byFlags &= ~CDATA_RUNLOCK;

	// Turn on client side prediction
	pClientDE->RunConsoleString("Prediction 1");


	// Determine if we were lobby launched...

	DBOOL bNetworkGameStarted = DFALSE;
	DBOOL bLobbyLaunched      = DFALSE;

#ifndef _DEMO
	DRESULT dr = pClientDE->IsLobbyLaunched("dplay2");
	if (dr == LT_OK)
	{
		DBOOL bRet = NetStart_DoLobbyLaunchWizard(pClientDE);
		if (bRet)
		{
			bLobbyLaunched      = DTRUE;
			bNetworkGameStarted = DTRUE;
		}
		else
		{
			pClientDE->Shutdown();
			return LT_ERROR;
		}
	}
#endif


	// Check for console vars that say we should do networking setup stuff...
	DBOOL bDoNetStuff = DFALSE;

	hVar = pClientDE->GetConsoleVar("Multiplayer");
	if (hVar)
	{
		if ((DBOOL)pClientDE->GetVarValueFloat(hVar)) bDoNetStuff = DTRUE;
	}

	hVar = pClientDE->GetConsoleVar("ConnectPlr");
	if (hVar)
	{
		char* sVar = pClientDE->GetVarValueString(hVar);
		if (sVar && sVar[0] != '\0' && _mbscmp((const unsigned char*)sVar, (const unsigned char*)"0") != 0) bDoNetStuff = DTRUE;
	}

	hVar = pClientDE->GetConsoleVar("Connect");
	if (hVar)
	{
		char* sVar = pClientDE->GetVarValueString(hVar);
		if (sVar && sVar[0] != '\0' && _mbscmp((const unsigned char*)sVar, (const unsigned char*)"0") != 0) bDoNetStuff = DTRUE;
	}

	if (bLobbyLaunched) bDoNetStuff = DFALSE;

#ifdef _DEMO
	bDoNetStuff = DFALSE;
#endif

	// Determine if we should display the networking dialogs...
	if (bDoNetStuff)
	{
		bNetworkGameStarted = NetStart_DoWizard (pClientDE);

		if (!bNetworkGameStarted)
		{
			pClientDE->Shutdown();
			return LT_ERROR;
		}
	}

	// Pass RMode on to SetRenderMode...
	DRESULT hResult = pClientDE->SetRenderMode(pMode);
	if (hResult != LT_OK)
	{
		// If an error occurred, try 640x480x16...
		
		RMode rMode;

		rMode.m_Width		= 640;
		rMode.m_Height		= 480;
		rMode.m_BitDepth	= 16;
		rMode.m_bHardware	= pMode->m_bHardware;

		sprintf(rMode.m_RenderDLL, "%s", pMode->m_RenderDLL);
		sprintf(rMode.m_InternalName, "%s", pMode->m_InternalName);
		sprintf(rMode.m_Description, "%s", pMode->m_Description);

		if (pClientDE->SetRenderMode(&rMode) != LT_OK)
		{
			// Okay, that didn't work, looks like we're stuck with software...
		
			rMode.m_bHardware = DFALSE;

			sprintf(rMode.m_RenderDLL, "soft.ren");
			sprintf(rMode.m_InternalName, "");
			sprintf(rMode.m_Description, "");

			if (pClientDE->SetRenderMode(&rMode) != LT_OK)
			{
				Abort(IDS_ERR_SETVIDMODE, 6929314);
				return LT_ERROR;
			} 
		}
	}

	// Draw the loading screen if we are hosting a network game already...
	if (bNetworkGameStarted && g_bIsHost)
	{
		DrawLoadingScreen();
		pClientDE->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);
		DrawLoadingScreen();
	}

	// Register console commands
	pClientDE->RegisterConsoleProgram("LeakFile", LeakFileFn);
	pClientDE->RegisterConsoleProgram("Connect", ConnectFn);
	pClientDE->RegisterConsoleProgram("FragSelf", FragSelfFn);
	pClientDE->RegisterConsoleProgram("InitSound", InitSoundFn);
	pClientDE->RegisterConsoleProgram("NextLevel", NextLevelFn);
	pClientDE->RegisterConsoleProgram("Load", LoadLevelFn);

	// Try to initialize the level manager (including
//	if (!m_LevelMgr.Init(pClientDE))
//	{
//		PrintError("Could not init LevelMgr");
//		return LT_ERROR;
//	}

	// Setup the music stuff...
	if( !m_Music.IsInitialized( ))
	{
		HCONSOLEVAR hConsoleVar;
		hConsoleVar = pClientDE->GetConsoleVar( "musictype" );

		// Default to ima if music never set before...
		if( !hConsoleVar )
			m_Music.Init( pClientDE, DTRUE );
		else
		{
			// See if they want cdaudio or ima...
			if(_mbsicmp((const unsigned char*)pClientDE->GetVarValueString( hConsoleVar ), (const unsigned char*)"cdaudio" ) == 0 )
				m_Music.Init( pClientDE, DFALSE );
			else
				m_Music.Init( pClientDE, DTRUE );
		}
	}

	// Set the sound volume
	hVar = pClientDE->GetConsoleVar("SoundVolume");
	if (hVar)
	{
		pClientDE->SetSoundVolume((int)pClientDE->GetVarValueFloat(hVar));
	}
	else
	{
		pClientDE->SetSoundVolume(35);
	}
	
	// Set the music volume
	hVar = pClientDE->GetConsoleVar("MusicVolume");
	if (hVar)
	{
		pClientDE->SetMusicVolume((int)pClientDE->GetVarValueFloat(hVar));
	}
	else
	{
		pClientDE->SetMusicVolume(35);
	}

	// Set the mouse sensitivity
	m_fOriginalMouseSensitivity = GetMouseSensitivity();
//	SetMouseSensitivity(m_fOriginalMouseSensitivity);
	m_fOriginalKeyTurnRate = GetKeyboardTurnRate();
//	SetKeyboardTurnRate(m_fOriginalKeyTurnRate);

	// Set the invert y-axis variable
	hVar=pClientDE->GetConsoleVar("MouseInvertYAxis");
	if (hVar && (int)pClientDE->GetVarValueFloat(hVar) != 0)
	{
		m_bMouseInvertYAxis=DTRUE;
	}
	else
	{
		m_bMouseInvertYAxis=DFALSE;
	}

	// Set the use joystick variable
	hVar=pClientDE->GetConsoleVar("UseJoystick");
	if (hVar && (int)pClientDE->GetVarValueFloat(hVar) != 0)
	{
		m_bUseJoystick=DTRUE;
	}
	else
	{
		m_bUseJoystick=DFALSE;
	}

	// Set the keyboard turn rate	
	hVar=pClientDE->GetConsoleVar("KeyboardTurnRate");
	if (hVar)
	{
		m_fKeyboardTurnRate=pClientDE->GetVarValueFloat(hVar);
	}	
	else
	{
		m_fKeyboardTurnRate=1.0f;
	}

/*
	if (m_Music.IsInitialized())
	{
		// Only re-init if not already using ima...
		if (!m_Music.UsingIMA())
		{
			// preserve the music level, just in case we come back to ima...
			CMusic::EMusicLevel level;
			level = m_Music.GetMusicLevel();
			if (m_Music.Init (pClientDE, DTRUE))
			{
				// Only start the music if we are in a world...
				if (IsInWorld() && !pClientShell->IsFirstUpdate())
				{
					m_Music.InitPlayLists();
					m_Music.PlayMusicLevel (level);
				}
			}
		}
	}
*/
	// Set the model hook function to do special model rendering...
	pClientDE->SetModelHook((ModelHookFn)DefaultModelHook, this);

    // Initialize the Message Mgr (T option)
    m_MessageMgr.Init(pClientDE);
    m_MessageMgr.Enable(DTRUE);
    
    // Initialize the Cheat Manager
	m_CheatMgr.Init(pClientDE);

#ifndef _DEMO
	m_FragInfo.Init(pClientDE, GetTeamMgr());
#endif

	HSTRING		szErrorStr = 0;

    // Initialize the Status bar
    if (!m_NewStatusBar.Init(pClientDE))
	{
		szErrorStr = pClientDE->FormatString(IDS_INITERR_STATUS);
		CSPrint(pClientDE->GetStringData(szErrorStr));
		pClientDE->FreeString(szErrorStr);
		Abort(IDS_ERR_INITSTATUSBAR, 6929318);
        return LT_ERROR;
	}

    // Initialize the Status bar
    if (!m_CommLink.Init(pClientDE))
	{
		szErrorStr = pClientDE->FormatString(IDS_INITERR_COMMLINK);
		CSPrint(pClientDE->GetStringData(szErrorStr));
		pClientDE->FreeString(szErrorStr);
		Abort(IDS_ERR_INITSTATUSBAR, 6929317);
        return LT_ERROR;
	}

    // Initialize the Menu
	if (!m_Menu.Init (pClientDE))
	{
		szErrorStr = pClientDE->FormatString(IDS_INITERR_MENUS);
		CSPrint(pClientDE->GetStringData(szErrorStr));
		pClientDE->FreeString(szErrorStr);
		Abort(IDS_ERR_INITMENU, 6929316);
        return LT_ERROR;
	}

	// Init the special fx mgr...
	if (!m_sfxMgr.Init(pClientDE))
  	{
		szErrorStr = pClientDE->FormatString(IDS_INITERR_SFX);
		CSPrint(pClientDE->GetStringData(szErrorStr));
		pClientDE->FreeString(szErrorStr);
		Abort(IDS_ERR_INITSFXMGR, 6929315);
        return LT_ERROR;
	}

	// Init the voice manager...
	if (!m_VoiceMgr.Init(pClientDE))
	{
		szErrorStr = pClientDE->FormatString(IDS_INITERR_VOICE);
		CSPrint(pClientDE->GetStringData(szErrorStr));
		pClientDE->FreeString(szErrorStr);
		Abort(IDS_ERR_INITVOICEMGR, 61011701);
        return LT_ERROR;
	}

	if (!m_playerCamera.Init(pClientDE))
	{
		szErrorStr = pClientDE->FormatString(IDS_INITERR_CAMERA);
		CSPrint(pClientDE->GetStringData(szErrorStr));
		pClientDE->FreeString(szErrorStr);
		Abort(IDS_ERR_INITPLAYERCAMERA, 6929318);
        return LT_ERROR;
	}
	else
	{
		DVector vOffset;
		VEC_SET(vOffset, 0.0f, 30.0f, 0.0f);

//		m_playerCamera.SetDistUp(1000.0f);
//		m_playerCamera.SetPointAtOffset(vOffset);
//		m_playerCamera.SetChaseOffset(vOffset);
		m_playerCamera.SetDeathOffset(vOffset);
		m_playerCamera.SetFirstPerson();
		m_b3rdPerson = DFALSE;
	}

	if (!bNetworkGameStarted)
	{
		StartGameRequest request;
		memset( &request, 0, sizeof( StartGameRequest ));

		NetStart_ClearGameStruct();  // Start with clean slate
		request.m_pGameInfo   = NetStart_GetGameStruct();
		request.m_GameInfoLen = sizeof(NetGame_t);

		HCONSOLEVAR hVar;

		// Set the Console lines
		if (hVar = pClientDE->GetConsoleVar("NumConsoleLines"))
		{
			DFLOAT fLines = pClientDE->GetVarValueFloat(hVar);
	//		m_MessageMgr.SetMaxMessages((int)fLines);
			m_MessageMgr.SetMaxMessages((int)6);
			pClientDE->RunConsoleString("NumConsoleLines 0");
		}
		
		// Get the command line world, if found
		if (hVar = pClientDE->GetConsoleVar("runworld"))
		{
			char *pWorldName = pClientDE->GetVarValueString(hVar);

			StartNewGame(pWorldName, STARTGAME_NORMAL, GAMETYPE_CUSTOM, LOADTYPE_NEW_GAME, DIFFICULTY_MEDIUM);
			m_bFirstWorld = DTRUE;
		}
		else
		{
			if (!StartMovies(pClientDE))
			{
				bPlayingMovies = DFALSE;
				SetGameState(GS_MENU);
				m_Menu.SetCurrentMenu(MENU_ID_MAINMENU, MENU_ID_MAINMENU);
			}
			else
			{
				bPlayingMovies = DTRUE;
			}
		}
	}
	else
	{
		// Set the game state for a net game that is loading...
		SetGameState(GS_MPLOADINGLEVEL);
	}

	// Display the title screen if we're not playing movies...

	HSURFACE hTitleSurface = DNULL;
	DFLOAT	 fTitleTime    = CWinUtil::GetTime();

	if (!bPlayingMovies && !bNetworkGameStarted)
	{
#ifndef _DEBUG

#ifdef _ADDON
		hTitleSurface = Splash_Display(pClientDE, "\\screens_ao\\title.pcx", DTRUE);
#else
		hTitleSurface = Splash_Display(pClientDE, "\\screens\\title.pcx", DTRUE);
#endif

		fTitleTime    = CWinUtil::GetTime();
#endif
	}

	// Read the player.cfg file, or defaultplayer.cfg if you can't find it.
//	if (!LoadConfig("player.cfg", &m_Config))
//		LoadConfig("defaultplayer.cfg", &m_Config);

#ifdef BRANDED
	// Initialize brand stuff
	CreateBrandSurface();
#endif

	// Seed the random number generator
	if ( pClientDE )
	{
		int nSeed=(int)(CWinUtil::GetTime() * 1000.0f);
		srand(nSeed);
	}

	// Initialize the message box font
	m_messageFont.Init(pClientDE, "interface/fonts/MenuFont1.pcx", "interface/fonts/MenuFont1.fnt");
	
	// Initialize the message box
	m_messageBox.Create(pClientDE, "interface/mainmenus/dialog.pcx", &m_messageFont, DNULL, DNULL);
	m_messageBox.SetTextColor(SETRGB(255, 255, 255));

	// Make sure the title screen has been up long enough...
	if (hTitleSurface)
	{

#ifndef _DEBUG
		CWinUtil::TimedKeyWait(VK_ESCAPE, VK_RETURN, VK_SPACE, 6.0f);
#endif
		pClientDE->DeleteSurface(hTitleSurface);
		hTitleSurface = DNULL;

		CWinUtil::Sleep(50);
	}

	// Remove any stray keypresses...
	CWinUtil::RemoveKeyDownMessages();

	// Show error if sound didn't initialize properly
	if( resInitSound != LT_OK )
	{
		if( resInitSound == LT_NO3DSOUNDPROVIDER )
		{
			hstrError = pClientDE->FormatString( IDS_MESSAGE_INVALID3DSOUNDPROVIDER );
		}
		else
		{
			hstrError = pClientDE->FormatString( IDS_MESSAGE_SOUNDERROR );
		}

		pszError = pClientDE->GetStringData( hstrError );
		DoMessageBox( pszError );
		pClientDE->FreeString( hstrError );
	}

	// All done...
	return LT_OK;
}

	
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnEngineTerm()
//
//	PURPOSE:	Called upon engine termination.
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::OnEngineTerm()
{
	CClientDE* pClientDE = GetClientDE();

	m_Menu.Term();
	m_MessageMgr.Term();
	m_VoiceMgr.Term();
	m_NewStatusBar.Term();
	m_CommLink.Term();
	m_Music.Term( );
	if (m_Credits.IsInited())
		m_Credits.Term();

	if (m_hCrosshair)
		pClientDE->DeleteSurface(m_hCrosshair);

	// Terminate the message box and its font
	m_messageFont.Term();
	m_messageBox.Destroy();

	// Deal with some advanced options...

	if (m_bSoundOriginallyEnabled && AdvancedDisableSound())
	{
		pClientDE->RunConsoleString ("SoundEnable 1");
	}

	if (m_bMusicOriginallyEnabled && AdvancedDisableMusic())
	{
		pClientDE->RunConsoleString ("MusicEnable 1");
	}

	if (m_bModelFBOriginallyEnabled)
	{
		pClientDE->RunConsoleString ("ModelFullbrite 1");
	}

	if (m_bLightmappingOriginallyEnabled)
	{
		pClientDE->RunConsoleString ("LightMap 1");
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnEnterWorld()
//
//	PURPOSE:	Function called when the client first enters the world.
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::OnEnterWorld()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	m_bInWorld = DTRUE;
	m_bHandledStartup = DFALSE;

	m_bFadeIn = m_bFadeOut = DFALSE;

	if (!m_hCamera)
	{
		ObjectCreateStruct cocStruct;
		INIT_OBJECTCREATESTRUCT(cocStruct);
	
		cocStruct.m_ObjectType = OT_CAMERA;
		VEC_INIT(cocStruct.m_Pos)
		ROT_INIT(cocStruct.m_Rotation)
		cocStruct.m_Flags = 0;

		m_hCamera = pClientDE->CreateObject(&cocStruct);
	}
	pClientDE->GetSurfaceDims(pClientDE->GetScreenSurface(), &m_dwScreenWidth, &m_dwScreenHeight);
	m_fovX = DEG2RAD(m_DefFovX);
	m_fovY = m_fovX * m_fovYScale;
    
	pClientDE->SetCameraFOV(m_hCamera, m_fovX, m_fovY);

	// Initialize various variables
	m_SpinAmount = 0;
	m_bZeroXAngle = DFALSE;
	m_fBobPhase = m_fSwayPhase = 0;
	m_fBobHeight = m_fBobWidth = m_fBobCant = m_fShakeCant = 0;
	m_fViewKick = m_fKickTime = 0;

//	m_fVelY = 0.0f;
	m_fViewY = m_fViewYVel = 0.0f;
	m_fWeaponY = m_fWeaponYVel = 0.0f;

	m_fVelMagnitude		= 0;
	m_fEyeLevel			= 0;
	VEC_INIT(m_vMyLastPos);

	m_bDead				= DFALSE;
	m_bDeadNoHide       = DFALSE;
	m_bWonkyVision		= DFALSE;
	m_bWonkyNoMove      = DFALSE;
	m_fWonkyTime		= 0.0f;

	m_bShakeScreen		= DFALSE;
	m_fShakeTime		= 0.0f;
	m_fShakeStart		= 0.0f;
	VEC_INIT(m_vShakeMagnitude);

	m_bFlashScreen		= DFALSE;
	m_fFlashTime		= 0.0f;
	m_fFlashStart		= 0.0f;
	m_fFlashRampUp		= 0.0f;
	VEC_INIT(m_vFlashColor);

	ROT_INIT(m_Rotation)

	// Sky panning
	m_bPanSky			= DFALSE;
	m_fPanSkyOffsetX	= 1.0f;
	m_fPanSkyOffsetZ	= 1.0f;
	m_fPanSkyScaleX		= 1.0f;
	m_fPanSkyScaleZ		= 1.0f;
	m_fCurSkyXOffset	= 0.0f;
	m_fCurSkyZOffset	= 0.0f;

	// Send initial update message
//	SendPlayerUpdateMessage(DTRUE);

	m_fCameraZoom = 0;
	m_b3rdPerson = DFALSE;
	m_playerCamera.SetFirstPerson();

	m_bFirstUpdate = DTRUE;

	g_pClientDE->SetInputState(DTRUE);
	m_bDrawFragBar = DFALSE;
	g_bLevelChange3rdPersonCam = DFALSE;
	g_bLevelChangeNoUpdate     = DFALSE;

	cdataLast.byFlags = 0;
	// Stuff to initialize/send if this is the first world
	if (m_bFirstWorld)
	{
		m_nCurGun = 0;

		m_bSpectatorMode = DFALSE;
	
		m_bZoomView = DFALSE;
//		m_bStoneView = DFALSE;
//		m_bAuraView = DFALSE;

		if( m_hNightGogglesSound )
		{
			g_pClientDE->KillSound( m_hNightGogglesSound );
			m_hNightGogglesSound = DNULL;
		}

		if( m_hTheEyeLoopingSound )
		{
			g_pClientDE->KillSound( m_hTheEyeLoopingSound );
			m_hTheEyeLoopingSound = DNULL;
		}

		int mode = 0;
		if (pClientDE->GetGameMode(&mode) == DE_OK)
		{
			if (mode == STARTGAME_NORMAL)	// Single player game name irrelevant
				_mbscpy((unsigned char*)m_Config.szName, (const unsigned char*)"");
		}

		m_nLastExitType = -1;

		m_CheatMgr.Reset();
	}

	// Reset the status bar
	m_NewStatusBar.Reset();

	//SCHLEGZ 4/27/98 5:45:03 PM: automatically turn on vis lsit deactivation
	pClientDE->RunConsoleString("serv autodeactivate 1");

	m_Music.TermPlayLists();
	m_Music.InitPlayLists( );
	if ( !m_Music.UsingIMA( ))
		m_Music.PlayCDList( );
	else
		m_Music.PlayMusicLevel( m_eMusicLevel );

	VEC_INIT( m_vLastReverbPos );

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnExitWorld()
//
//	PURPOSE:	Called when the client leaves the world.  Perform cleanup here.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::OnExitWorld()
{
	CClientDE* pClientDE = GetClientDE();

	m_sfxMgr.RemoveAll();					// Remove all the sfx

	// Kill the music...
	pClientDE->StopMusic (MUSIC_IMMEDIATE);
	m_Music.TermPlayLists();
	m_eMusicLevel  = CMusic::MUSICLEVEL_SILENCE;

	// Pause sounds (this will only pause currently playing sounds, then they'll get removed)
	pClientDE->PauseSounds();

	m_playerCamera.AttachToObject(DNULL);	// Detatch camera


	if (m_hCamera)
	{
		pClientDE->DeleteObject(m_hCamera);
		m_hCamera = DNULL;
	}

	for (int i=0;i<CLIENTWEAPONSLOTS;i++)
	{
		m_abyWeaponID[i] = WEAP_NONE;
		m_abyLWeaponID[i] = WEAP_NONE;
	}
	if (m_pWeapon) 
	{
		delete m_pWeapon;
		m_pWeapon = DNULL;
	}
	if (m_pLWeapon) 
	{
		delete m_pLWeapon;
		m_pLWeapon = DNULL;
	}

	m_hWeapHandObj = DNULL;
	m_hLWeapHandObj = DNULL;

	if (m_hContainerSound)
	{
		pClientDE->KillSound(m_hContainerSound);
		m_hContainerSound = DNULL;
	}

	m_FragInfo.RemoveAllClients();

	m_bInWorld     = DFALSE;
	m_bRollCredits = DFALSE;

	m_Credits.Term();

	m_bNightGogglesActive = DFALSE;
	if( m_hNightGogglesSound )
	{
		g_pClientDE->KillSound( m_hNightGogglesSound );
		m_hNightGogglesSound = DNULL;
	}

	m_bBinocularsActive = DFALSE;
	m_hSeeingEyeObj = m_hOrbObj = m_hLastOrbObj = DNULL;

	if( m_hTheEyeLoopingSound )
	{
		g_pClientDE->KillSound( m_hTheEyeLoopingSound );
		m_hTheEyeLoopingSound = DNULL;
	}

	if( m_hstrObjectivesText )
	{
		g_pClientDE->FreeString( m_hstrObjectivesText );
		m_hstrObjectivesText = DNULL;
	}
	if( m_hstrObjectivesTitle )
	{
		g_pClientDE->FreeString( m_hstrObjectivesTitle );
		m_hstrObjectivesTitle = DNULL;
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SendPlayerUpdateMessage()
//
//	PURPOSE:	Sends player info to the server
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::SendPlayerUpdateMessage(DBOOL bSendAll)
{
	// Get our client pointer...

	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;


	// Determine if it's time to update if we're in multiplayer...

	if (!bSendAll && IsMultiplayerGame())
	{
		g_fPlayerUpdateTimer -= pClientDE->GetFrameTime();
		if (g_fPlayerUpdateTimer > 0.0f) return;
		g_fPlayerUpdateTimer = PLAYER_MULTI_UPDATE_TIMER;
	}


	// Determine the update flags we want...

	DBYTE byUpdateFlags = 0;

	// HACK - send container code to server that is just below our eye
	DVector vPos;
	pClientDE->GetObjectPos(m_hCamera, &vPos);
	vPos.y -= 15.0f;

	HLOCALOBJ objList[1];
	DDWORD dwNum = pClientDE->GetPointContainers(&vPos, objList, 1);

	if (dwNum > 0)
	{
		D_WORD wCode;
		pClientDE->GetContainerCode(objList[0], &wCode );
		cdata.eContainer = ( ContainerCode )wCode;
	}
	else
		cdata.eContainer = m_eCurContainerCode;

	// HACK - send container code to server that is at our feet
	HOBJECT hPlayerObj;
	DVector vDims;
	hPlayerObj = m_pMoveMgr->GetObject( );
	if( hPlayerObj && !m_bDead )
	{
		pClientDE->GetObjectPos( hPlayerObj, &vPos );
		pClientDE->Physics( )->GetObjectDims( hPlayerObj, &vDims );
		vPos.y -= vDims.y;

		HLOCALOBJ objList[1];
		DDWORD dwNum = pClientDE->GetPointContainers( &vPos, objList, 1 );

		if (dwNum > 0)
		{
			cdata.hContainerAtBottom = objList[0];
		}
		else
			cdata.hContainerAtBottom = DNULL;
	}
	else
		cdata.hContainerAtBottom = DNULL;

	// Set mouse aiming
	if (m_bMouseLook || pClientDE->IsCommandOn(COMMAND_MOUSEAIMTOGGLE))
		cdata.byFlags |= CDATA_MOUSEAIMING;
	else
		cdata.byFlags &= ~CDATA_MOUSEAIMING;


	// Determine what we want to send..
	if (!bSendAll)
	{
		if (cdata.byFlags != cdataLast.byFlags)
			byUpdateFlags |= PLAYERUPDATE_FLAGS;
		
		if (cdata.fPitch != cdataLast.fPitch)
			byUpdateFlags |= PLAYERUPDATE_PITCH;
		
		if (cdata.fYaw != cdataLast.fYaw)
			byUpdateFlags |= PLAYERUPDATE_YAW;
		
		if (!VEC_EQU(cdata.GunMuzzlePos, cdataLast.GunMuzzlePos))
			byUpdateFlags |= PLAYERUPDATE_MUZZLE;
		
		if (!VEC_EQU(cdata.lGunMuzzlePos, cdataLast.lGunMuzzlePos))
			byUpdateFlags |= PLAYERUPDATE_LMUZZLE;
		
		if (cdata.MouseAxis0 != cdataLast.MouseAxis0 || cdata.MouseAxis1 != cdataLast.MouseAxis1)
		{
			byUpdateFlags |= PLAYERUPDATE_MOUSE;
		}
		if (cdata.eContainer != cdataLast.eContainer)
			byUpdateFlags |= PLAYERUPDATE_CONTAINER;
		if (cdata.hContainerAtBottom != cdataLast.hContainerAtBottom)
			byUpdateFlags |= PLAYERUPDATE_CONTAINERATBOTTOM;
	}
	else	// Send all params.
	{
		byUpdateFlags = 0xff;
	}

	// Send the stuff.
	if (byUpdateFlags)
	{
		HMESSAGEWRITE hMsg = pClientDE->StartMessage(CMSG_PLAYERUPDATE);

		pClientDE->WriteToMessageByte(hMsg, byUpdateFlags);				// Tell what we are updating

		if (byUpdateFlags & PLAYERUPDATE_FLAGS)
			pClientDE->WriteToMessageByte(hMsg, cdata.byFlags);			// Flags
		
		if (byUpdateFlags & PLAYERUPDATE_PITCH)
			pClientDE->WriteToMessageFloat(hMsg, cdata.fPitch);			// Rotation
		
		if (byUpdateFlags & PLAYERUPDATE_YAW)
			pClientDE->WriteToMessageFloat(hMsg, cdata.fYaw);
		
		if (byUpdateFlags & PLAYERUPDATE_MUZZLE)
			pClientDE->WriteToMessageCompVector(hMsg, &cdata.GunMuzzlePos); // Muzzle pos
		
		if (byUpdateFlags & PLAYERUPDATE_LMUZZLE)
			pClientDE->WriteToMessageCompVector(hMsg, &cdata.lGunMuzzlePos); // Muzzle pos
		
		if (byUpdateFlags & PLAYERUPDATE_MOUSE)
		{
			DFLOAT fMouse0 = cdata.MouseAxis0 * 512;
			CLIPLOWHIGH(fMouse0, -128, 127);
			fMouse0 += 128;
			DFLOAT fMouse1 = cdata.MouseAxis1 * 512;
			CLIPLOWHIGH(fMouse1, -128, 127);
			fMouse1 += 128;

			pClientDE->WriteToMessageByte(hMsg, (DBYTE)fMouse0);			// Mouse axis
			pClientDE->WriteToMessageByte(hMsg, (DBYTE)fMouse1);
		}

		if (byUpdateFlags & PLAYERUPDATE_CONTAINER)
		{
			DBYTE nContainerCode;

			nContainerCode = (DBYTE) cdata.eContainer;
			if( m_pExternalCamera || m_hSeeingEyeObj || m_hOrbObj )
				nContainerCode |= 0x80;
			pClientDE->WriteToMessageByte(hMsg, nContainerCode );	// Container code
		}

		if (byUpdateFlags & PLAYERUPDATE_CONTAINERATBOTTOM )
		{
			pClientDE->WriteToMessageObject(hMsg, cdata.hContainerAtBottom );
		}

		pClientDE->EndMessage(hMsg);
	}
	// Save current cdata
	memcpy(&cdataLast, &cdata, sizeof(ClientData));

	// Send position packet
	HMESSAGEWRITE hMessage = pClientDE->StartMessage(CMSG_PLAYERPOSITION);

	// Write position info.
	m_pMoveMgr->WritePositionInfo(hMessage);

	pClientDE->EndMessage2(hMessage, 0); // Send unguaranteed.
	
//	m_fLastSentYaw	= m_fYaw;
//	m_fLastSentCamCant = m_fCamCant;
//	m_fPlayerInfoLastSendTime = pClientDE->GetTime();
//	m_nPlayerInfoChangeFlags = 0;

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SendPlayerInitMessage()
//
//	PURPOSE:	Sends a message to the server.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::SendPlayerInitMessage(DBYTE messageID)
{
	HMESSAGEWRITE hMsg;
	CClientDE* pClientDE = GetClientDE();

	if (messageID != CMSG_SINGLEPLAYER_INIT && messageID != CMSG_MULTIPLAYER_INIT)
		return;

	// Only send this if it's a new game.
	if (messageID == CMSG_SINGLEPLAYER_INIT && m_nLastLoadType != LOADTYPE_NEW_GAME)
		return;

	// Start the message
	hMsg = pClientDE->StartMessage(messageID);

	// Send the appropriate data for this message
	switch(messageID)
	{
		case CMSG_SINGLEPLAYER_INIT:
		case CMSG_MULTIPLAYER_INIT:
			{
				int i;
				HSTRING hstrName = DNULL;

				// Load in a B2C file if it's a multiplayer game
				if (IsMultiplayerGame())
				{
					NetPlayer* pPlayerInfo = NetStart_GetPlayerStruct();
					if (!pPlayerInfo) return;

					hstrName = pClientDE->CreateString(pPlayerInfo->m_sName);

					B2C		b2cFile;
					FILE	*file = 0;
					char	filename[128];

					if (strlen(pPlayerInfo->m_sConfig) <= 0)
					{
						CShell_GetConsoleString("NetConfigFile", pPlayerInfo->m_sConfig, "caleb.b2c");
					}

					sprintf(filename, "players\\%s", pPlayerInfo->m_sConfig);

					if(file = fopen(filename, "rb"))
					{
						fread(&b2cFile, sizeof(B2C), 1, file);
						fclose(file);
					}
					else	// just set default stuff
					{
						CSPrint("Warning - Unable to load .b2c file");

						b2cFile.dwCharacter	= CHARACTER_CALEB;
						b2cFile.dwColor		= 1;
						b2cFile.dwStrength	= 5;
						b2cFile.dwSpeed		= 3;
						b2cFile.dwResist	= 3;
						b2cFile.dwFocus		= 1;
						b2cFile.dwWeap1		= 1;
						b2cFile.dwWeap2		= 2;
						b2cFile.dwWeap3		= 3;
						b2cFile.dwWeap4		= 4;
						b2cFile.dwWeap5		= 5;
						b2cFile.dwWeap6		= 6;
						b2cFile.dwWeap7		= 7;
						b2cFile.dwWeap8		= 8;
						b2cFile.dwWeap9		= 9;
					}

					m_Config.nCharacter = (DBYTE)b2cFile.dwCharacter;
					m_Config.nSkin = (DBYTE)b2cFile.dwColor;
					m_Config.nStrength = (DBYTE)b2cFile.dwStrength;
					m_Config.nSpeed = (DBYTE)b2cFile.dwSpeed;
					m_Config.nResistance = (DBYTE)b2cFile.dwResist;
					m_Config.nMagic = (DBYTE)b2cFile.dwFocus;
					m_Config.nWeaponSlots[1] = (DBYTE)b2cFile.dwWeap1;
					m_Config.nWeaponSlots[2] = (DBYTE)b2cFile.dwWeap2;
					m_Config.nWeaponSlots[3] = (DBYTE)b2cFile.dwWeap3;
					m_Config.nWeaponSlots[4] = (DBYTE)b2cFile.dwWeap4;
					m_Config.nWeaponSlots[5] = (DBYTE)b2cFile.dwWeap5;
					m_Config.nWeaponSlots[6] = (DBYTE)b2cFile.dwWeap6;
					m_Config.nWeaponSlots[7] = (DBYTE)b2cFile.dwWeap7;
					m_Config.nWeaponSlots[8] = (DBYTE)b2cFile.dwWeap8;
					m_Config.nWeaponSlots[9] = (DBYTE)b2cFile.dwWeap9;
				}
				else
				{
					// first gun is the beretta
					m_Config.nWeaponSlots[1] = WEAP_BERETTA;
					m_Config.nWeaponSlots[2] = WEAP_NONE;
					m_Config.nWeaponSlots[3] = WEAP_NONE;
					m_Config.nWeaponSlots[4] = WEAP_NONE;
					m_Config.nWeaponSlots[5] = WEAP_NONE;
					m_Config.nWeaponSlots[6] = WEAP_NONE;
					m_Config.nWeaponSlots[7] = WEAP_NONE;
					m_Config.nWeaponSlots[8] = WEAP_NONE;
					m_Config.nWeaponSlots[9] = WEAP_NONE;
					hstrName = pClientDE->CreateString(m_Config.szName);
				}
				if (!hstrName) return;

				pClientDE->WriteToMessageHString(hMsg, hstrName);
				pClientDE->FreeString(hstrName);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nCharacter);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nSkin);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nStrength);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nSpeed);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nResistance);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nMagic);

//				for(i = 0; i < 3; i++)
//					pClientDE->WriteToMessageByte(hMsg, m_Config.nBindingSlots[i]);
				for (i=1;i<SLOTCOUNT_WEAPONS;i++)	// Skip slot 0, it's melee
					pClientDE->WriteToMessageByte(hMsg, m_Config.nWeaponSlots[i]);
/*				for (i=0;i<INV_LASTINVITEM;i++)
					pClientDE->WriteToMessageByte(hMsg, m_Config.nItemSlots[i]);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nProximityBombs);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nRemoteBombs);
				pClientDE->WriteToMessageByte(hMsg, m_Config.nTimeBombs);
//				for (i=0;i<6;i++)
//					pClientDE->WriteToMessageByte(hMsg, m_Config.nSpellSlots[i]);
*/
			}
			break;
	}

	// Finish it off..
	pClientDE->EndMessage(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateSoundReverb
//
//	PURPOSE:	Update the reverb info
//
// ----------------------------------------------------------------------- //
void CBloodClientShell::UpdateSoundReverb( )
{
	DVector vPlayerDims;
	ClientIntersectInfo info;
	ClientIntersectQuery query;
	DVector vPos[6], vSegs[6];
	int i;
	float fVolume, fReverbLevel;
	DBOOL bOpen;
	HLOCALOBJ hPlayerObj;
	ReverbProperties reverbProperties;
	HCONSOLEVAR hVar;

	if( !m_bUseReverb )
		return;

	hPlayerObj = g_pClientDE->GetClientObject();
	if( !hPlayerObj )
		return;

	hVar = g_pClientDE->GetConsoleVar( "ReverbLevel" );
	if( hVar )
	{
		fReverbLevel = g_pClientDE->GetVarValueFloat( hVar );
	}
	else
		fReverbLevel = 1.0f;

	// Check if reverb was off and is still off
	if( fReverbLevel < 0.001f && m_fReverbLevel < 0.001f )
		return;

	m_fReverbLevel = fReverbLevel;

	// Check if it's time yet
	if( g_pClientDE->GetTime( ) < m_fNextSoundReverbTime )
		return;

	// Update timer
	m_fNextSoundReverbTime = g_pClientDE->GetTime( ) + SOUND_REVERB_UPDATE_PERIOD;

	HOBJECT hFilterList[] = {hPlayerObj, m_pMoveMgr->GetObject(), DNULL};

	memset( &query, 0, sizeof( query ));
	query.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	query.m_FilterFn = ObjListFilterFn;
	query.m_pUserData = hFilterList;

	g_pClientDE->GetObjectPos( hPlayerObj, &query.m_From );
	g_pClientDE->Physics( )->GetObjectDims( hPlayerObj, &vPlayerDims );

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

	bOpen = DFALSE;
	for( i = 0; i < 6; i++ )
	{
		VEC_COPY( query.m_To, vSegs[i] );

		if( g_pClientDE->IntersectSegment( &query, &info ))
		{
			VEC_COPY( vPos[i], info.m_Point );
			if( info.m_SurfaceFlags == SURFTYPE_SKY )
			{
				bOpen = DTRUE;
			}
		}
		else
		{
			VEC_COPY( vPos[i], vSegs[i] );
			bOpen = DTRUE;
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
	g_pClientDE->SetReverbProperties( &reverbProperties );

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateCamera()
//
//	PURPOSE:	Updates the camera position and status
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::UpdateCamera()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	HLOCALOBJ hPlayerObj = pClientDE->GetClientObject();
	if (!hPlayerObj || !m_hCamera) return;

	if (!m_bHandledStartup)
	{
		m_bRenderCamera = DTRUE;
//		pClientDE->SetCameraZOrder(m_hCamera, 0);  // Okay, we can render now
		m_bHandledStartup = DTRUE;
	}

	// Update the player camera...

	if (!UpdatePlayerCamera()) return;


	m_pExternalCamera = DNULL;

	// See if we should use a cutscene camera...

	if (!m_hSeeingEyeObj && !m_hOrbObj)
	{
		CSpecialFXList* pCameraList = m_sfxMgr.GetCameraFXList();
		if (pCameraList)
		{
			int nNumCameras = pCameraList->GetNumItems();

			for (int i=0; i < nNumCameras; i++)
			{
				CCameraFX* pCameraFX = (CCameraFX*)(*pCameraList)[i];
				if (!pCameraFX) continue;

				if ( HOBJECT hObj = pCameraFX->GetServerObj() )
				{
					DDWORD dwUsrFlags;
					pClientDE->GetObjectUserFlags(hObj, &dwUsrFlags);
					
					// USRFLG_VISIBLE means the camera is active
					if (dwUsrFlags & USRFLG_VISIBLE)
					{
						m_pExternalCamera = pCameraFX;
						break;
					}
				}
			}
		}
	}

	if ((m_pExternalCamera && !m_pExternalCamera->GetHidePlayer()) || m_hSeeingEyeObj || m_hOrbObj)
	{
		ShowPlayer(DTRUE);
	}
	else
	{
		if (m_bDead && m_bDeadNoHide)
		{
			ShowPlayer(TRUE);
		}
		else
		{
			ShowPlayer((m_bDead) ? DFALSE : !m_playerCamera.IsFirstPerson());
		}
	}

	CalculateCameraRotation();
	UpdateCameraPosition();
	UpdateCameraRotation();

	// Adjust camera for current resolution
	SetCameraStuff(hPlayerObj, m_hCamera);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::FirstUpdate()
//
//	PURPOSE:	Special stuff to do on the first update after entering a world.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::FirstUpdate()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	char buf[100];
	char *pStrParam;

	ShowPlayer(m_b3rdPerson ? DTRUE : DFALSE);

	// Check for add-on special case stuff...

#ifdef _ADDON
	if (g_bPlayerInvReset)
	{
		ResetPlayerInventory();
		g_bPlayerInvReset = DFALSE;
	}
#endif
	
	// Display the world name if any
//	if (pStrParam = pClientDE->GetServerConVarValueString("WorldName"))
//	{
//		CSPrint("Entering %s", pStrParam);
//	}

	// Set the FarZ setting for the world...
	DFLOAT fVal = pClientDE->GetServerConVarValueFloat("FarZ");

	if (fVal > 0.0f)
	{
		sprintf(buf, "FarZ %d", (int)fVal);
		pClientDE->RunConsoleString(buf);
	}

	ResetGlobalFog();

	// Set up the environment map texture...

	pStrParam = pClientDE->GetServerConVarValueString("EnvironmentMap");
	if (pStrParam && (m_nGlobalDetail == DETAIL_HIGH))
	{
		sprintf(buf, "EnvMap %s", pStrParam);
		pClientDE->RunConsoleString(buf);
		pClientDE->RunConsoleString("EnvMapEnable 1");
	}
	else
	{
		pClientDE->RunConsoleString("EnvMapEnable 0");
	}

	pClientDE->RunConsoleString("ModelWarble 0");
	pClientDE->RunConsoleString("WarbleSpeed 15");
	pClientDE->RunConsoleString("WarbleScale .95");

	// Sky panning...

	m_bPanSky = (DBOOL) pClientDE->GetServerConVarValueFloat("PanSky");
	m_fPanSkyOffsetX = pClientDE->GetServerConVarValueFloat("PanSkyOffsetX");
	m_fPanSkyOffsetZ = pClientDE->GetServerConVarValueFloat("PanSkyOffsetZ");
	m_fPanSkyScaleX = pClientDE->GetServerConVarValueFloat("PanSkyScaleX");
	m_fPanSkyScaleZ = pClientDE->GetServerConVarValueFloat("PanSkyScaleZ");
	pStrParam  = pClientDE->GetServerConVarValueString("PanSkyTexture");

	if (m_bPanSky)
	{
		pClientDE->SetGlobalPanTexture(GLOBALPAN_SKYSHADOW, pStrParam);
	}

	// Initialize the music playlists...

	if ( m_Music.IsInitialized( ) )
	{
//		CMusic::EMusicLevel level;
//		level = m_Music.GetMusicLevel( );
/*		m_Music.TermPlayLists();
		m_Music.InitPlayLists( );
		if ( !m_Music.UsingIMA( ))
			m_Music.PlayCDList( );
		else
			m_Music.PlayMusicLevel( m_eMusicLevel );
*/
	}

	// Set up the soft renderer sky map...

	char* pSoftSky = pClientDE->GetServerConVarValueString("SoftSky");
	if (pSoftSky)
	{
		sprintf(buf, "SoftSky %s", pSoftSky);
		pClientDE->RunConsoleString(buf);
	}

	// Send initial update message
	SendPlayerUpdateMessage(DTRUE);

	m_bFirstUpdate = DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::ResetGlobalFog
//
//	PURPOSE:	Reset the global fog value...
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::ResetGlobalFog()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	DFLOAT fVal = pClientDE->GetServerConVarValueFloat("FogEnable");
	
	char buf[30];
	sprintf(buf, "FogEnable %d", (int)fVal);
	pClientDE->RunConsoleString(buf);
	if (fVal)
	{
		fVal = pClientDE->GetServerConVarValueFloat("FogNearZ");
		sprintf(buf, "FogNearZ %d", (int)fVal);
		pClientDE->RunConsoleString(buf);

		fVal = pClientDE->GetServerConVarValueFloat("FogFarZ");
		sprintf(buf, "FogFarZ %d", (int)fVal);
		pClientDE->RunConsoleString(buf);

		fVal = pClientDE->GetServerConVarValueFloat("FogR");
		sprintf(buf, "FogR %d", (int)fVal);
		pClientDE->RunConsoleString(buf);

		fVal = pClientDE->GetServerConVarValueFloat("FogG");
		sprintf(buf, "FogG %d", (int)fVal);
		pClientDE->RunConsoleString(buf);

		fVal = pClientDE->GetServerConVarValueFloat("FogB");
		sprintf(buf, "FogB %d", (int)fVal);
		pClientDE->RunConsoleString(buf);
	}

	// ModelAdd
	DFLOAT fR, fG, fB;
	fR = pClientDE->GetServerConVarValueFloat("ModelAddR");
	fG = pClientDE->GetServerConVarValueFloat("ModelAddG");
	fB = pClientDE->GetServerConVarValueFloat("ModelAddB");
	sprintf(buf, "ModelAdd %d %d %d", (int)fR, (int)fG, (int)fB);
	pClientDE->RunConsoleString(buf);

	// ModelDirAdd
	fR = pClientDE->GetServerConVarValueFloat("ModelDirAddR");
	fG = pClientDE->GetServerConVarValueFloat("ModelDirAddG");
	fB = pClientDE->GetServerConVarValueFloat("ModelDirAddB");
	sprintf(buf, "ModelDirAdd %d %d %d", (int)fR, (int)fG, (int)fB);
	pClientDE->RunConsoleString(buf);
	
	fVal = pClientDE->GetServerConVarValueFloat("SkyFog");
	sprintf(buf, "SkyFog %d", (int)fVal);
	pClientDE->RunConsoleString(buf);
	if (fVal)
	{
		fVal = pClientDE->GetServerConVarValueFloat("SkyFogNearZ");
		sprintf(buf, "SkyFogNearZ %d", (int)fVal);
		pClientDE->RunConsoleString(buf);

		fVal = pClientDE->GetServerConVarValueFloat("SkyFogFarZ");
		sprintf(buf, "SkyFogFarZ %d", (int)fVal);
		pClientDE->RunConsoleString(buf);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::Update()
//
//	PURPOSE:	the client shell update function.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::Update()
{
	HLOCALOBJ hPlayerObj;

	CClientDE* pClientDE = GetClientDE();

	m_pMoveMgr->Update();

	UpdateSoundReverb( );
	
	switch (GetGameState())
	{
		case GS_PLAYING:
		case GS_MENUANIM:
		{
			if (!m_bInWorld || !pClientDE) 
				return;

			if (g_bLevelChangeNoUpdate)
				return;

			// Make sure server is still running
			if (!pClientDE->IsConnected())
			{
				HSTRING szErrorStr = pClientDE->FormatString(IDS_GENERAL_SHUTDOWN);
				CSPrint(pClientDE->GetStringData(szErrorStr));
				pClientDE->FreeString(szErrorStr);
				m_bInWorld = DFALSE;
				return;
			}

			hPlayerObj = pClientDE->GetClientObject();

			if (!hPlayerObj)
				return;

			if (m_bFirstUpdate)
			{
				FirstUpdate();
			}

			// Update weapons firing..
			HandleFiring();

			// Initialize cameraadd and lightscale..

			VEC_SET(m_vLightScale, 1.0f, 1.0f, 1.0f);
			VEC_SET(m_vCameraAdd, 0.0f, 0.0f, 0.0f);

			// Update sky panning

			if (m_bPanSky)
			{
				DFLOAT fFrameTime = pClientDE->GetFrameTime();

				m_fCurSkyXOffset += fFrameTime * m_fPanSkyOffsetX;
				m_fCurSkyZOffset += fFrameTime * m_fPanSkyOffsetZ;

				pClientDE->SetGlobalPanInfo(GLOBALPAN_SKYSHADOW, m_fCurSkyXOffset, m_fCurSkyZOffset, m_fPanSkyScaleX, m_fPanSkyScaleZ);
			}

			// Update the camera
			UpdateCamera();

			// Update the screen shake
			if (m_bShakeScreen)
				UpdateScreenShake();

			// Update the screen flash
			if (m_bFlashScreen)
				UpdateScreenFlash();

			// Update container effects...
			UpdateContainerFX();

			if(m_pCreature && m_pCreature->GetType() == SMSG_THIEF_ATTACH || IsWonky())
			{
				// Update the wonkyvision timer.
				if (IsWonky())
				{
					m_fWonkyTime -= pClientDE->GetFrameTime();
					if (m_fWonkyTime <= 0)
					{
						EndWonkyVision();
					}
				}

				UpdateWonkyVision();
			}
			else
			{
				pClientDE->RunConsoleString("ModelWarble 0");

				m_fOffsetY = 0.0f;
				m_fOffsetX = 0.0f;
				m_fOffsetRot = 0.0f;
			}

			// Update night goggles sounds...
			if( m_bNightGogglesActive )
			{
				// Start the sound...
				if( !m_hNightGogglesSound )
				{
					m_hNightGogglesSound = PlaySoundLocal( "sounds\\powerups\\nightgogglesloop1.wav", 
						SOUNDPRIORITY_PLAYER_LOW, DTRUE, DTRUE );
				}
			}
			else
			{
				// Kill the sound...
				if( m_hNightGogglesSound )
				{
					g_pClientDE->KillSound( m_hNightGogglesSound );
					m_hNightGogglesSound = DNULL;
				}
			}


			SetPhysicsStateTimeStep(&g_normalPhysicsState, pClientDE->GetFrameTime());
			SetPhysicsStateTimeStep(&g_waterPhysicsState, pClientDE->GetFrameTime());


			// Update any client-side special effects...
			m_sfxMgr.UpdateSpecialFX();


			// Check for Burn


			// Set the light scale
			if (m_bBlind)
			{
				if (pClientDE->GetTime() > m_fBlindTime)
					m_bBlind = DFALSE;
			}

			// Set the light scale depending on the state
			DVector vLightScale;
			if (m_bBlind)
			{
				VEC_SET(vLightScale, 0.0f, 0.0f, 0.0f)
			}
			else if (m_bFadeIn)	// 4 second fade-in
			{
				m_fFadeVal += 100 * pClientDE->GetFrameTime();
				if (m_fFadeVal > 255) 
				{
					m_fFadeVal = 255;
					m_bFadeIn = DFALSE;
				}
				DFLOAT fScale = m_fFadeVal / 255.0f;
				VEC_SET(vLightScale, fScale, fScale, fScale);
			}
			else if (m_bFadeOut) // 4 second fade-out
			{
				m_fFadeVal -= 100 * pClientDE->GetFrameTime();
				if (m_fFadeVal < 0) 
				{
					m_fFadeVal = 0;
				}
				DFLOAT fScale = m_fFadeVal / 255.0f;
				VEC_SET(vLightScale, fScale, fScale, fScale);
			}
//			else if (m_bStoneView)
//			{
//				VEC_SET(vLightScale, 0.0f, 0.0f, 1.0f)
//			}
			else if (m_hSeeingEyeObj)
			{
				VEC_SET(vLightScale, 1.0f, 0.0f, 0.5f)
			}
			else if(m_pCreature)
			{
				VEC_COPY(vLightScale, m_pCreature->GetLightScale());
			}
			else if(m_bNightGogglesActive)
			{
				VEC_SET(vLightScale, 0.1f, 1.0f, 0.1f);
			}
			else	// Default
			{
				VEC_COPY(vLightScale, m_vLightScale);
			}

			pClientDE->SetGlobalLightScale(&vLightScale);
			pClientDE->SetCameraLightAdd(m_hCamera, &m_vCameraAdd);

			// Update the server with player info
			SendPlayerUpdateMessage(DFALSE);

			// Roll the credits if necessary
			if (m_bRollCredits)
			{
				if (m_Credits.IsInited())
				{
					m_Credits.Update();
				}
				else
				{
					m_Credits.Init(pClientDE, this, CM_CREDITS, DFALSE);
				}
			}

			// Zoom camera out
			if (m_CheatMgr.IsCheatActive(CHEAT_CHASEVIEW) || g_bLevelChange3rdPersonCam)
			{
				if (!m_b3rdPerson)
					Set3rdPersonCamera(DTRUE);
			}
			// Zoom camera in
			else
			{
				if (m_b3rdPerson)
					Set3rdPersonCamera(DFALSE);
			}

			break;
		}

		// Nothing to do if we're in menu mode..
		case GS_MENU:	
			break;

		case GS_LOADINGLEVEL:
		case GS_MPLOADINGLEVEL:
		case GS_WAITING:
			break;

		case GS_CREDITS :
		{
			UpdateCredits();
			return;
		}

		case GS_MOVIES:
		{
			UpdateMoviesState();
			return;
		}

		case GS_SPLASH:
		{
			Splash_Update();
			return;
		}

		break;
	}

	pClientDE->ClearScreen(DNULL, CLEARSCREEN_SCREEN | CLEARSCREEN_RENDER);
	if (m_bRenderCamera)
	{
		pClientDE->Start3D();
		if( pClientDE->GetClientObject())
			pClientDE->ProcessAttachments(pClientDE->GetClientObject());	// [blg]
		pClientDE->RenderCamera(m_hCamera);
		if (m_bRollCredits && m_Credits.IsInited()) m_Credits.Update();	// [blg]
		pClientDE->End3D();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateCredits()
//
//	PURPOSE:	Update the credits
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::UpdateCredits()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	pClientDE->Start3D();
	m_Credits.Update();
	pClientDE->End3D();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnObjectMove()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //
DRESULT CBloodClientShell::OnObjectMove(HOBJECT hObj, DBOOL bTeleport, DVector *pPos)
{
	m_pMoveMgr->OnObjectMove(hObj, bTeleport, pPos);
	return LT_OK;
}

DRESULT CBloodClientShell::OnObjectRotate(HOBJECT hObj, DBOOL bTeleport, DRotation *pNewRot)
{
	return m_pMoveMgr->OnObjectRotate(hObj, bTeleport, pNewRot);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::PreLoadWorld()
//
//	PURPOSE:	Display a loading screen right before loading the next world.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::PreLoadWorld(char *pWorldName)
{
	CClientDE* pClientDE = GetClientDE();

	DrawLoadingScreen();
	pClientDE->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);
	DrawLoadingScreen();

	g_bLevelChangeNoUpdate = DFALSE;


	// Re-Init the team manager...

	m_TeamMgr.Term();
	m_TeamMgr.Init();

	HSTRING hTeam1String = pClientDE->FormatString(IDS_TEAM1NAME);
	HSTRING hTeam2String = pClientDE->FormatString(IDS_TEAM2NAME);

	if (hTeam1String)
	{
		m_TeamMgr.AddTeam(TEAM_1, pClientDE->GetStringData(hTeam1String));
	}
	else
	{
		m_TeamMgr.AddTeam(TEAM_1, "Team Blue");
	}

	if (hTeam2String)
	{
		m_TeamMgr.AddTeam(TEAM_2, pClientDE->GetStringData(hTeam2String));
	}
	else
	{
		m_TeamMgr.AddTeam(TEAM_2, "Team Blue");
	}

	if (hTeam1String) pClientDE->FreeString(hTeam1String);
	if (hTeam2String) pClientDE->FreeString(hTeam2String);


	// Stop any voice sounds...

	GetVoiceMgr()->StopAll();


	// Check if we should roll the credits on top of this world...
	strupr(pWorldName);
	m_bRollCredits = DFALSE;

	if (pWorldName)
	{
		if (_mbsicmp((const unsigned char*)pWorldName, (const unsigned char*)"END") == 0 || _mbsicmp((const unsigned char*)pWorldName, (const unsigned char*)"WORLDS\\END") == 0)
		{
			m_bRollCredits = DTRUE;
		}

		if (_mbsicmp((const unsigned char*)pWorldName, (const unsigned char*)"CREDITS") == 0 || _mbsicmp((const unsigned char*)pWorldName, (const unsigned char*)"WORLDS\\CREDITS") == 0)
		{
			m_bRollCredits = DTRUE;
		}

#ifdef _ADDON
		if (_mbsicmp((const unsigned char*)pWorldName, (const unsigned char*)"WORLDS_AO\\END") == 0)
		{
			m_bRollCredits = DTRUE;
		}

		if (_mbsicmp((const unsigned char*)pWorldName, (const unsigned char*)"WORLDS_AO\\CREDITS") == 0)
		{
			m_bRollCredits = DTRUE;
		}
#endif

	}

	m_Credits.Term();


	// Check if this is an add-on level that wants a character change...

	g_bPlayerInvReset = DFALSE;

#ifdef _ADDON
	char sUpr[256];
	strncpy(sUpr, pWorldName, 255);
	strupr(sUpr);

	if (strstr(sUpr, "_AO"))			// is this an add-on level?
	{
		if (strstr(sUpr, "_CC_C"))		// switch to caleb?
		{
			SetCharacterInfo(CHARACTER_CALEB, MULTIPLAY_SKIN_NORMAL);
			g_bPlayerInvReset = DTRUE;
		}
		else if (strstr(sUpr, "_CC_I"))	// switch to ishmael?
		{
			SetCharacterInfo(CHARACTER_ISHMAEL, MULTIPLAY_SKIN_NORMAL);
			g_bPlayerInvReset = DTRUE;
		}
		else if (strstr(sUpr, "_CC_O"))	// switch to ophelia?
		{
			SetCharacterInfo(CHARACTER_OPHELIA, MULTIPLAY_SKIN_NORMAL);
			g_bPlayerInvReset = DTRUE;
		}
		else if (strstr(sUpr, "_CC_G"))	// switch to gabby?
		{
			SetCharacterInfo(CHARACTER_GABREILLA, MULTIPLAY_SKIN_NORMAL);
			g_bPlayerInvReset = DTRUE;
		}
	}

	if (m_nLastLoadType == LOADTYPE_RESTORESAVE || m_nLastLoadType == LOADTYPE_RESTOREAUTOSAVE)
	{
		g_bPlayerInvReset = DFALSE;
	}

	if (strstr(sUpr, "ENDBOSS_CC_C"))	// check for final caleb nightmare level
	{
		g_bPlayerInvReset = DFALSE;
	}
#endif

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::PostUpdate()
//
//	PURPOSE:	Called after client object update functions.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::PostUpdate()
{
	CClientDE* pClientDE = GetClientDE();

	HSURFACE hScreen = pClientDE->GetScreenSurface();

	// Delete the loading screen if there is one.
	if (GetGameState() == GS_LOADINGLEVEL || GetGameState() == GS_MPLOADINGLEVEL || GetGameState() == GS_WAITING)
	{
		DrawLoadingScreen();
		pClientDE->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);
		return;
	}
	else if (m_hLoadingScreen)
	{
		pClientDE->DeleteSurface(m_hLoadingScreen);
		m_hLoadingScreen = DNULL;
		m_bPlayedWaitingSound = DFALSE;
	}

	if (GetGameState() == GS_MENU)
	{
		m_Menu.Draw();
	}
	else if (GetGameState() == GS_PLAYING)
	{
		if (g_bLevelChangeNoUpdate)
		{
			return;
		}

		m_MessageMgr.Draw();
		DBOOL bDraw = m_bDrawStatusBar && !(m_pExternalCamera);
		m_NewStatusBar.Draw(bDraw);
		m_CommLink.Draw(DTRUE);
	
		if (m_bDrawFragBar && IsMultiplayerGame())
		{
			m_FragInfo.Draw (DTRUE, DTRUE);
		}

		if (m_bShowCrosshair && !m_pExternalCamera && !m_bDead && !m_hSeeingEyeObj && !m_hOrbObj && !g_bLevelChange3rdPersonCam && !m_bDrawFragBar)
		{
			if(m_nLastCrosshair != m_nCrosshair)
			{
				if(m_hCrosshair)	pClientDE->DeleteSurface(m_hCrosshair);
				m_hCrosshair = 0;
				m_nLastCrosshair = m_nCrosshair;
			}

			if (!m_hCrosshair)
			{
				char	file[128];

				if(m_nCrosshair < 10)
					sprintf(file, "Interface/Crosshairs/Crosshair0%d.pcx", m_nCrosshair);
				else
					sprintf(file, "Interface/Crosshairs/Crosshair%d.pcx", m_nCrosshair);

				m_hCrosshair = pClientDE->CreateSurfaceFromBitmap(file);
				pClientDE->GetSurfaceDims (m_hCrosshair, &m_cxCrosshair, &m_cyCrosshair);
			}

			DBOOL bFullScreen = DFALSE;
			int nLeft = 0;
			int nTop = 0;
			int nRight = 0;
			int nBottom = 0;
			pClientDE->GetCameraRect (m_hCamera, &bFullScreen, &nLeft, &nTop, &nRight, &nBottom);
			if (bFullScreen)
			{
				pClientDE->GetSurfaceDims (hScreen, &m_dwScreenWidth, &m_dwScreenHeight);
				nRight = (int)m_dwScreenWidth;
				nBottom = (int)m_dwScreenHeight;
			}

			int x = nLeft + (((nRight - nLeft) - m_cxCrosshair) >> 1);
			int y = nTop + (((nBottom - nTop) - m_cyCrosshair) >> 1);

			pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hCrosshair, NULL, x, y, NULL);
		}
	}

	// Draw the message box if it is up
	if (m_bInMessageBox)
	{
		m_messageBox.Render(hScreen);
	}

#ifdef BRANDED
	if (m_hBrandSurface && m_fBrandCounter > 0)
	{
		DrawBrandString(hScreen);
		m_fBrandCounter -= pClientDE->GetFrameTime();
	}
#endif

	pClientDE->FlipScreen(FLIPSCREEN_CANDRAWCONSOLE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateBob
//
//	PURPOSE:	Adjusts the head bobbing & swaying
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::UpdateBob()
{
	DFLOAT	bobH;
	DFLOAT  bobV;
	DFLOAT  swayH;
	DFLOAT  swayV;
	DFLOAT	pace;
	DBOOL   bRunning;
	DFLOAT  moveDist;

	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;
	HLOCALOBJ hPlayerObj = pClientDE->GetClientObject();
	if (!hPlayerObj) return;

	// Calculate velocities
	DVector vMyPos;
	pClientDE->GetObjectPos(hPlayerObj, &vMyPos);
	
	// Calculate Y velocity for camera dampening
	DFLOAT fVelY = (vMyPos.y - m_vMyLastPos.y) / (pClientDE->GetFrameTime() * 100.0f);
	CLIPLOWHIGH(fVelY, -20, 20);

	VEC_COPY(m_vMyLastPos, vMyPos);

	bRunning = (cdata.byFlags & CDATA_RUNLOCK || pClientDE->IsCommandOn(COMMAND_RUN));
	moveDist = (m_fVelMagnitude) * pClientDE->GetFrameTime();

	// Running

	if (bRunning)
	{
		pace = PIx2 * 0.5f;
		bobH = BOBH_WALKING / 500.0f;
		bobV = BOBV_WALKING / 500.0f;
		swayH = SWAYH_WALKING / 10000.0f;
		swayV = SWAYV_WALKING / 10000.0f;
	}
	else
	{
		pace = PIx2 * 0.25f;
		bobH = BOBH_WALKING / 500.0f;
		bobV = BOBV_WALKING / 500.0f;
		swayH = SWAYH_WALKING / 5000.0f;
		swayV = SWAYV_WALKING / 5000.0f;
	}
	
	m_fBobPhase += (pClientDE->GetFrameTime() * pace);
	if (m_fBobPhase > PIx2) m_fBobPhase -= PIx2;

	m_fSwayPhase += (pClientDE->GetFrameTime() * pace) / 2;
	if (m_fSwayPhase > PIx2) m_fSwayPhase -= PIx2;

	// decay the amplitude
	m_fBobAmp = m_fBobAmp - pClientDE->GetFrameTime() * 150;
	if (m_fBobAmp < 0) m_fBobAmp = 0;

	if (m_fBobAmp < 30)
	{
		m_fBobAmp += moveDist;
		if (m_fBobAmp > 30) m_fBobAmp = 30;
	}

	// Head bobbing
	m_fBobHeight = bobV * m_fBobAmp * (float)sin(m_fBobPhase * 2);
	m_fBobWidth  = bobH * m_fBobAmp * (float)sin(m_fBobPhase * 1 - (PI/4));
	m_fBobCant	 = m_fBobWidth / 200.0f;
	// Weapon swaying
	m_fSwayHeight = -swayV * m_fBobAmp * (float)sin(m_fSwayPhase * 4) / 3;
	m_fSwayWidth = swayH * m_fBobAmp * (float)sin(m_fSwayPhase * 2 - (PI/3)) / 3;

	// Adjust camera velocity
#define kViewDamping    0.43f
#define kViewStiffnessA 0.625f  // return force when above goal y
#define kViewStiffnessB 0.112f  // return force when below goal y

	DFLOAT fLast = m_fViewYVel;
	m_fViewYVel = INTERPOLATE(m_fViewYVel, fVelY, kViewDamping);
	DFLOAT dz = m_fEyeLevel - m_fViewY;
	if ( dz < 0 )
		m_fViewYVel += dz * kViewStiffnessA;
	else
		m_fViewYVel += dz * kViewStiffnessB;
	if (fabs(m_fViewYVel) < 0.1f) m_fViewYVel = 0;
	m_fViewY += m_fViewYVel;

#define kWeapDamping    0.312f
#define kWeapStiffnessA 0.5f  // return force when above goal y
#define kWeapStiffnessB 0.047f // return force when below goal y

	m_fWeaponYVel = INTERPOLATE(m_fWeaponYVel, -(fVelY/80.0f), kWeapDamping);
	dz = -m_fWeaponY;
	if ( dz < 0 )
		m_fWeaponYVel += dz * kWeapStiffnessA;
	else
		m_fWeaponYVel += dz * kWeapStiffnessB;
	m_fWeaponY += m_fWeaponYVel;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateHeadCant()
//
//	PURPOSE:	Update head tilt when strafing
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::UpdateHeadCant()
{
	CClientDE* pClientDE = GetClientDE();

	// If we are strafing, cant our head as we move..

		// Strafe.. Check for strafe modifier first
	if ((pClientDE->IsCommandOn(COMMAND_STRAFE) && pClientDE->IsCommandOn(COMMAND_LEFT)) ||
		pClientDE->IsCommandOn(COMMAND_STRAFELEFT))
	{
		m_fCamCant -= m_fCantIncrement;
		if(m_fCamCant < -m_fCantMaxDist)
			m_fCamCant = -m_fCantMaxDist;
	}
	else if ((pClientDE->IsCommandOn(COMMAND_STRAFE) && pClientDE->IsCommandOn(COMMAND_RIGHT)) ||
		pClientDE->IsCommandOn(COMMAND_STRAFERIGHT))
	{
		m_fCamCant += m_fCantIncrement;
		if(m_fCamCant > m_fCantMaxDist)
			m_fCamCant = m_fCantMaxDist;
	}
	else 
	{
		// We are not canting so move us toward zero...
		if(m_fCamCant != 0.0f)
		{
			if(m_fCamCant < 0.0f) 
			{
				m_fCamCant += m_fCantIncrement;
			} 
			else 
			{
				m_fCamCant -= m_fCantIncrement;
			}
			if(fabs(m_fCamCant) < m_fCantIncrement)
				m_fCamCant = 0.0f;
 		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdatePlayerCamera()
//
//	PURPOSE:	Update the player camera
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::UpdatePlayerCamera()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !m_hCamera || !m_pMoveMgr) return DFALSE;

	// Make sure our player camera is attached...

	m_playerCamera.AttachToObject(m_pMoveMgr->GetObject());

	// Update our camera position based on the player camera...

	m_playerCamera.CameraUpdate(pClientDE->GetFrameTime());

	return DTRUE;
}


void CBloodClientShell::GetCameraRotation(DRotation *pRot)
{
	GetClientDE()->SetupEuler(pRot, m_fPitch, m_fYaw, m_fCamCant);
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SetCameraStuff
//
//	PURPOSE:	Sets the camera field of view, rect & zoom mode.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::SetCameraStuff(HLOCALOBJ hPlayerObj, HLOCALOBJ hCamera)
{
	float oldFovX;
	float zoomSpeed;

	float fFOVx, fFOVy;
	int nLeft, nTop, nBottom, nRight;

	CClientDE* pClientDE = GetClientDE();

	pClientDE->GetSurfaceDims (pClientDE->GetScreenSurface(), &m_dwScreenWidth, &m_dwScreenHeight);

	cdata.byFlags |= CDATA_CANMOVE;

	if (m_hSeeingEyeObj)
	{
		nLeft = 0;
		nTop = 0;
		nRight = m_dwScreenWidth - 1;
		nBottom = m_dwScreenHeight - 1;

		fFOVx = DEG2RAD(m_DefFovX + 45);
		fFOVy = fFOVx * m_fovYScale;

		cdata.byFlags &= ~CDATA_CANMOVE;
	}
	else if (m_hOrbObj)
	{
		nLeft = 0;
		nTop = 0;
		nRight = m_dwScreenWidth - 1;
		nBottom = m_dwScreenHeight - 1;

		fFOVx = DEG2RAD(m_DefFovX + 45);
		fFOVy = fFOVx * m_fovYScale;

		cdata.byFlags &= ~CDATA_CANMOVE;
	}
	else if (m_pExternalCamera)
	{
		if (m_pExternalCamera->GetType() == CAMTYPE_LETTERBOX)
		{
			nLeft = 0;
			nTop = m_dwScreenHeight / 6;
			nRight = m_dwScreenWidth - 1;
			nBottom = m_dwScreenHeight - nTop - 1;

			fFOVx = DEG2RAD(m_DefFovX);
			fFOVy = fFOVx * 0.55f;
		}
		else
		{
			m_fovY = m_fovX * m_fovYScale;

			nLeft = 0;
			nTop = 0;
			nRight = m_dwScreenWidth - 1;
			nBottom = m_dwScreenHeight - 1;

			fFOVx = m_fovX;
			fFOVy = m_fovY;
		}
		if (!m_pExternalCamera->GetPlayerMovement())
			cdata.byFlags &= ~CDATA_CANMOVE;
	}
	else
	{
		zoomSpeed = 8 * pClientDE->GetFrameTime();

	//	pClientDE->GetSurfaceDims(pClientDE->GetScreenSurface(), &Width, &Height);
	//	pClientDE->GetCameraFOV(m_hCamera, &fovX, &fovY);

		oldFovX = m_fovX;

		if (!m_fovX)
		{
			m_fovX = DEG2RAD(m_DefFovX);
		}

		// Need to zoom camera
		if (m_bZoomView && m_fovX > DEG2RAD(FOV_ZOOMED))
		{
			m_fovX -= zoomSpeed;
			if (m_fovX < DEG2RAD(FOV_ZOOMED)) 
			{
				m_fovX = DEG2RAD(FOV_ZOOMED);
			}

			m_fovY = m_fovX * m_fovYScale;
		}
		else if (!m_bZoomView && m_fovX < DEG2RAD(m_DefFovX))
		{
			m_fovX += zoomSpeed;

			if (m_fovX > DEG2RAD(m_DefFovX))
				m_fovX = DEG2RAD(m_DefFovX);

			m_fovY = m_fovX * m_fovYScale;
		}
		else if (!m_bZoomView && m_fovX > DEG2RAD(m_DefFovX))
		{
			m_fovX -= zoomSpeed;

			if (m_fovX < DEG2RAD(m_DefFovX))
				m_fovX = DEG2RAD(m_DefFovX);

			m_fovY = m_fovX * m_fovYScale;
		}

		nLeft = 0;
		nTop = 0;
		nRight = m_dwScreenWidth - 1;
		nBottom = m_dwScreenHeight - 1;

		fFOVx = m_fovX;
		fFOVy = m_fovY;
	}

	pClientDE->SetCameraRect(m_hCamera, DFALSE, nLeft, nTop, nRight, nBottom);
	pClientDE->SetCameraFOV(m_hCamera, fFOVx, fFOVy);
}



	
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnCommandOn()
//
//	PURPOSE:	Called upon command activation.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::OnCommandOn(int command)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	switch(command)
	{
		// Auto Run
		case COMMAND_RUNLOCK:
			if (!m_MessageMgr.GetEditingState())
			{
				HSTRING szErrorStr = pClientDE->FormatString(IDS_GENERAL_AUTORUN);
				if(szErrorStr)
				{
					_mbscpy((unsigned char*)msgbuf, (const unsigned char*)pClientDE->GetStringData(szErrorStr));
					pClientDE->FreeString(szErrorStr);
				}

				// Toggle runlock flag
				if (cdata.byFlags & CDATA_RUNLOCK)
				{
					cdata.byFlags &= ~CDATA_RUNLOCK;

					szErrorStr = pClientDE->FormatString(IDS_GENERAL_OFF);
					if(szErrorStr)
					{
						_mbscat((unsigned char*)msgbuf, (const unsigned char*)pClientDE->GetStringData(szErrorStr));
						pClientDE->FreeString(szErrorStr);
					}

					pClientDE->RunConsoleString("+RunLock 0");
				}
				else
				{
					cdata.byFlags |= CDATA_RUNLOCK;

					szErrorStr = pClientDE->FormatString(IDS_GENERAL_ON);
					if(szErrorStr)
					{
						_mbscat((unsigned char*)msgbuf, (const unsigned char*)pClientDE->GetStringData(szErrorStr));
						pClientDE->FreeString(szErrorStr);
					}

					pClientDE->RunConsoleString("+RunLock 1");
				}

				CSPrint(msgbuf);
			}
			break;

		// Mouse aiming toggle
/*		case COMMAND_MOUSEAIMTOGGLE:
			if (!m_MessageMgr.GetEditingState())
			{
				_mbscpy(msgbuf, "MOUSE AIMING ");
				if (cdata.byFlags & CDATA_MOUSEAIMING)
				{
					cdata.byFlags &= ~CDATA_MOUSEAIMING;
					_mbscat((unsigned char*)msgbuf, (const unsigned char*)"OFF");
					m_bZeroXAngle = DTRUE;

					pClientDE->RunConsoleString("+MouseLook 0");
				}
				else
				{
					cdata.byFlags |= CDATA_MOUSEAIMING;
					_mbscat((unsigned char*)msgbuf, (const unsigned char*)"ON");

					pClientDE->RunConsoleString("+MouseLook 1");
				}

				CSPrint(msgbuf);
			}
			break;
*/
		case COMMAND_LOOKUP: 
			if (!m_MessageMgr.GetEditingState())
			{
				m_bZeroXAngle = DFALSE;
			}	
			break;

		case COMMAND_LOOKDOWN: 
			if (!m_MessageMgr.GetEditingState())
			{
				m_bZeroXAngle = DFALSE;
			}
			break;

		case COMMAND_CROSSHAIR:
			if (!m_MessageMgr.GetEditingState() && !m_bZoomView)
			{
				if(m_bShiftState == DTRUE)
				{
					m_nCrosshair++;
					if(m_nCrosshair > 41)	m_nCrosshair = 0;
					m_nCrosshairOriginalNum = m_nCrosshair;

					char str[64];
					sprintf(str, "+CrosshairNum %d", m_nCrosshair);
					pClientDE->RunConsoleString(str);
				}
				else
				{
					m_bShowCrosshair = !m_bShowCrosshair;
					m_bCrosshairOriginallyOn = m_bShowCrosshair;

					if(m_bShowCrosshair)
						pClientDE->RunConsoleString("+Crosshair 1");
					else
						pClientDE->RunConsoleString("+Crosshair 0");
				}
			}
			break;

		case COMMAND_SCREENSHRINK:
			//m_NewStatusBar.StatLevelUp();
			break;

		case COMMAND_SCREENENLARGE:
			//m_NewStatusBar.StatLevelDown();
			break;

		case COMMAND_USE:
			if(m_bDead && GetGameState() == GS_PLAYING)
			{
				if(IsMultiplayerGame())
				{
					m_pMoveMgr->ClearControlFlags();

					if (m_pWeapon)		m_pWeapon->CancelFiringState();
					if (m_pLWeapon)		m_pLWeapon->CancelFiringState();

					if (m_pWeapon) 
					{
						delete m_pWeapon;
						m_pWeapon = DNULL;
					}
					if (m_pLWeapon) 
					{
						delete m_pLWeapon;
						m_pLWeapon = DNULL;
					}

					if (m_abyWeaponID[m_nCurGun])
						m_pWeapon = CreateWeapon(m_abyWeaponID[m_nCurGun], DFALSE);
					if (m_abyLWeaponID[m_nCurGun])
						m_pLWeapon = CreateWeapon(m_abyLWeaponID[m_nCurGun], DTRUE);
				}
				else
				{
					pClientDE->RunConsoleString("NumConsoleLines 0");

					SetGameState(GS_MENU);
					PauseGame(DTRUE);
					SetGameState(GS_MENU);

#ifdef _DEMO
					m_Menu.SetCurrentMenu(MENU_ID_MAINMENU, MENU_ID_MAINMENU);
#else
					m_Menu.SetCurrentMenu(MENU_ID_LOAD_GAME, MENU_ID_LOAD_GAME);
#endif
				}
			}
			break;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnCommandOff()
//
//	PURPOSE:	Called upon command deactivation.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::OnCommandOff(int command)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	if (!m_MessageMgr.GetEditingState())
	{
		switch(command)
		{
			// Mouse aiming toggle
			case COMMAND_MOUSEAIMTOGGLE:
			{
				if (!m_MessageMgr.GetEditingState() && m_bLookSpring && !m_bMouseLook)
					m_bZeroXAngle = DTRUE;
			}
			break;

			case COMMAND_LOOKUP: 
			case COMMAND_LOOKDOWN: 
			{
				if (m_bLookSpring)
					m_bZeroXAngle = DTRUE;		
			}
			break;
			
			case COMMAND_USE:
			{
				if(m_pCreature)
					m_pCreature->UseKeyHit();
			}
			break;

			case COMMAND_MESSAGE:
			{
				if(GetGameState() == GS_PLAYING)
				{
					m_MessageMgr.SetEditingState(DTRUE);
					pClientDE->SetInputState(DFALSE);
				}
			}
			break;

			default: return;
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnKeyDown()
//
//	PURPOSE:	Called upon a key down message from Windows.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::OnKeyDown(int key, int rep)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	// This is used to ignore a keyboard message which is used while
	// configuring the keys on the keyboard.
	if (m_bIgnoreKeyboardMessage)
	{
		m_bIgnoreKeyboardMessage=DFALSE;
		return;
	}

	// Track shift keys.
	if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT)
		m_bShiftState = DTRUE;

	// The message box will receive all keyboard input if it is up
	if (m_bInMessageBox)
	{
		m_messageBox.HandleKeyDown(key, rep);
		return;
	}

	if (m_bRollCredits && GetGameState() != GS_CREDITS)
	{
		m_Credits.HandleInput(key);
	}

	if (m_MessageMgr.GetEditingState())
	{
		m_MessageMgr.HandleKeyDown(key, rep);
	}
	else if (GetGameState() == GS_SPLASH)
	{
		Splash_OnKeyDown(key);
		return;
	}
	else if (GetGameState() == GS_MOVIES)
	{
		if (key == VK_ESCAPE)
		{
			AdvanceMovies(pClientDE);
		}
		return;
	}
	else if (GetGameState() == GS_CREDITS)
	{
		if (key == VK_ESCAPE)
		{
			m_Credits.Term();

			if (m_bInWorld)
			{
				SetGameState(GS_PLAYING);
				PauseGame(DFALSE);
			}
			else
			{
				SetGameState(GS_MENU);
				m_Menu.SetCurrentMenu(MENU_ID_MAINMENU, MENU_ID_MAINMENU);
			}
		}
		else
		{
			m_Credits.HandleInput (key);
		}
	}
	else if (GetGameState() == GS_WAITING)
	{
		SetGameState(GS_PLAYING);
		PauseGame(DFALSE);
	}
	else if (GetGameState() == GS_MENU)
	{
		// Process the keyboard commands in the menu class
		m_Menu.HandleKeyDown(key, rep);
	}
	else if (key == VK_ESCAPE)
	{
		if (GetGameState() == GS_MENU && m_bInWorld)
		{
			SetGameState(GS_PLAYING);
			PauseGame(DFALSE);

			if (g_bLevelChange3rdPersonCam)
			{
				pClientDE->SetInputState(DFALSE);
				if (!m_b3rdPerson) Set3rdPersonCamera(DTRUE);
				m_bDrawFragBar = DTRUE;
			}
		}
		else if (GetGameState() == GS_PLAYING)
		{
			pClientDE->RunConsoleString("NumConsoleLines 0");
            
			SetGameState(GS_MENU);
            PauseGame(DTRUE);
			m_Menu.SetCurrentMenu(MENU_ID_MAINMENU, MENU_ID_MAINMENU);
		}
	}
	else if ((key == VK_F1) && !m_bShiftState)
	{
		m_NewStatusBar.ToggleWeapons();
	}
	else if (key == VK_TAB)
	{
		if(IsMultiplayerGame())
		{
			m_bDrawFragBar = !m_bDrawFragBar;
			if (m_bDrawFragBar) m_FragInfo.TurnOn();
			else m_FragInfo.TurnOff();
		}
		else
		{
			if (GetGameType() == GAMETYPE_SINGLE || GetGameType() == GAMETYPE_CUSTOM)
			{
				m_NewStatusBar.ToggleObjectives();
			}
		}
	}
	else if (m_bShiftState)		// Special functions for these keys..
	{
		if (key >= VK_F1 && key <= VK_F12)	// Text macros..
		{
			int n = key - VK_F1 + 1;
			sprintf(msgbuf, "macro%d", n);
			HCONSOLEVAR hVar;
			if (hVar = pClientDE->GetConsoleVar (msgbuf))
			{
				char* sVar = pClientDE->GetVarValueString(hVar);
				if (sVar && sVar[0] != '\0')
				{
					// Send the macro...
					HMESSAGEWRITE hMsg = pClientDE->StartMessage(CMSG_PLAYERMESSAGE);
					pClientDE->WriteToMessageString(hMsg, sVar);
					pClientDE->EndMessage(hMsg);
				}
			}
		}
/*		else if (key == VK_ADD || key == 0x00BB)
		{
			m_fovYScale += .01f;

			HSTRING szErrorStr = pClientDE->FormatString(IDS_GENERAL_FOVY, m_fovYScale);
			CSPrint(pClientDE->GetStringData(szErrorStr));
			pClientDE->FreeString(szErrorStr);
//			CSPrint("FOV Y Scale: %.2f", m_fovYScale);
		}
		else if (key == VK_SUBTRACT || key == 0x00BD)
		{
			m_fovYScale -= .01f;

			HSTRING szErrorStr = pClientDE->FormatString(IDS_GENERAL_FOVY, m_fovYScale);
			CSPrint(pClientDE->GetStringData(szErrorStr));
			pClientDE->FreeString(szErrorStr);
//			CSPrint("FOV Y Scale: %.2f", m_fovYScale);
		}
		else if (key == 0x00DB)
		{
			m_DefFovX -= 0.25f;

			HSTRING szErrorStr = pClientDE->FormatString(IDS_GENERAL_FOVX, m_DefFovX);
			CSPrint(pClientDE->GetStringData(szErrorStr));
			pClientDE->FreeString(szErrorStr);
//			CSPrint("FOV X: %.2f deg.", m_DefFovX);
		}
		else if (key == 0x00DD)
		{
			m_DefFovX += 0.25f;

			HSTRING szErrorStr = pClientDE->FormatString(IDS_GENERAL_FOVX, m_DefFovX);
			CSPrint(pClientDE->GetStringData(szErrorStr));
			pClientDE->FreeString(szErrorStr);
//			CSPrint("FOV X: %.2f deg.", m_DefFovX);
		}*/

#ifdef _DEBUG
		// 09/12/97 
		// Add option to move weapon in the game... viewing of the weapon
		// Move Weapon X,Y,Z
		if (m_pWeapon)
		{
			DVector offset = m_pWeapon->GetGunOffset();

			if (key == 'B')
			{
				offset.x += 0.1f;

				HSTRING szErrorStr = pClientDE->FormatString(IDS_GENERAL_FOVY, offset.x);
//				CSPrint(szErrorStr);
				pClientDE->FreeString(szErrorStr);

				sprintf(msgbuf, "Gun: m_GunOffset.x %.1f", offset.x);
				CSPrint(msgbuf);
			}

			// Move Weapon X,Y,Z
			if (key == 'N')
			{
				offset.y += 0.1f;
				sprintf(msgbuf, "Gun: m_GunOffset.y %.1f", offset.y);
				CSPrint(msgbuf);
			}

			// Move Weapon X,Y,Z
			if (key == 'M')
			{
				offset.z += 0.1f;
				sprintf(msgbuf, "Gun: m_GunOffset.z %.1f", offset.z);
				CSPrint(msgbuf);
			}

			// Move Weapon X,Y,Z
			if (key == 'H')
			{
				offset.x -= 0.1f;
				sprintf(msgbuf, "Gun: m_GunOffset.x %.1f", offset.x);
				CSPrint(msgbuf);
			}

			// Move Weapon X,Y,Z
			if (key == 'J')
			{
				offset.y -= 0.1f;
				sprintf(msgbuf, "Gun: m_GunOffset.y %.1f", offset.y);
				CSPrint(msgbuf);
			}

			// Move Weapon X,Y,Z
			if (key == 'K')
			{
				offset.z -= 0.1f;
				sprintf(msgbuf, "Gun: m_GunOffset.z %.1f", offset.z);
				CSPrint(msgbuf);
			}

			if (m_pWeapon)
			{
				m_pWeapon->SetGunOffset(offset);
			}

			if (m_pLWeapon)
			{
				offset.x = -offset.x;
				m_pLWeapon->SetGunOffset(offset);
			}
		}
#endif
	}
	else if(key == VK_F2)		// Save Game
	{
		if (GetGameState() == GS_PLAYING)
		{
#ifndef _DEMO
			PauseGame(DTRUE);
			SetGameState(GS_MENU);
			m_Menu.SetCurrentMenu(MENU_ID_SAVE_GAME, MENU_ID_SAVE_GAME);
#endif
		}
	}
	else if(key == VK_F3)		// Load Game
	{
		if (GetGameState() == GS_PLAYING)
		{
#ifndef _DEMO
			PauseGame(DTRUE);
			SetGameState(GS_MENU);
			m_Menu.SetCurrentMenu(MENU_ID_LOAD_GAME, MENU_ID_LOAD_GAME);
#endif
		}
	}
	else if(key == VK_F4)		// Options
	{
		if (GetGameState() == GS_PLAYING)
		{
			PauseGame(DTRUE);
			SetGameState(GS_MENU);
			m_Menu.SetCurrentMenu(MENU_ID_OPTIONS, MENU_ID_OPTIONS);
		}
	}
	else if(key == VK_F5)		// Quicksave
	{
		if (GetGameState() == GS_PLAYING && m_bInWorld && !m_bDead && !IsMultiplayerGame())
		{
#ifndef _DEMO
			MenuSaveGame(SLOT_QUICK);
			DoMessageBox("Quicksave...", DFALSE);
#endif
		}
	}
	else if (key == VK_F6)		// Quickload
	{
		if (GetGameState() == GS_PLAYING && !IsMultiplayerGame())
		{
#ifndef _DEMO
			if (MenuLoadGame(SLOT_QUICK))
			DoMessageBox("Quickload...", DFALSE);
#endif
		}
	}
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::StartNewGame()
//
//	PURPOSE:	Starts a new game
//
// --------------------------------------------------------------------------- //

DBOOL CBloodClientShell::StartNewGame(char *pszWorldName, DBYTE nStartType, DBYTE nGameType, DBYTE nLoadType, DBYTE nDifficulty)
{
	CClientDE* pClientDE = GetClientDE();
	HCONSOLEVAR hVar;

	// Initialize some stuff
	m_nCurGun		= 0;

	m_bSpectatorMode = DFALSE;

	m_fEyeLevel		= 0;
	m_bZoomView		= DFALSE;
	m_hSeeingEyeObj = DNULL;
	m_hOrbObj		= DNULL;
	m_hLastOrbObj	= DNULL;

	if( m_hNightGogglesSound )
	{
		g_pClientDE->KillSound( m_hNightGogglesSound );
		m_hNightGogglesSound = DNULL;
	}

	if( m_hTheEyeLoopingSound )
	{
		g_pClientDE->KillSound( m_hTheEyeLoopingSound );
		m_hTheEyeLoopingSound = DNULL;
	}

	m_bNightGogglesActive = DFALSE;

	m_bBinocularsActive = DFALSE;

	hVar = pClientDE->GetConsoleVar ("CrosshairNum");
	m_nCrosshair = (DBYTE)pClientDE->GetVarValueFloat(hVar);
	if(m_nCrosshair > 41) m_nCrosshair = 0;
	m_nCrosshairOriginalNum = m_nCrosshair;

	m_playerCamera.SetFirstPerson();
	m_b3rdPerson	= DFALSE;

	memset(&m_Request, 0, sizeof(StartGameRequest));
	m_Request.m_Type = nStartType;
	pClientDE->StartGame(&m_Request);

	// Reset the voice manager...
	GetVoiceMgr()->Reset();

	m_eMusicLevel = CMusic::MUSICLEVEL_SILENCE;

	// Send a message to the server to restore the autosave
	if (pszWorldName)
		_mbscpy((unsigned char*)m_szFilename, (const unsigned char*)pszWorldName);

	return StartNewWorld(m_szFilename, nGameType, nLoadType, nDifficulty);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnKeyUp()
//
//	PURPOSE:	Called upon a key up message from Windows.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::OnKeyUp(int key)
{
	// Track shift keys.
	if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT)
		m_bShiftState = DFALSE;

	m_MessageMgr.HandleKeyUp(key);

	if (GetGameState() == GS_MENU)
	{
		m_Menu.HandleKeyUp(key);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnMessage()
//
//	PURPOSE:	Called when a message is received from the server.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::OnMessage(DBYTE messageID, HMESSAGEREAD hMessage)
{
	DBYTE tbyte;

	CClientDE* pClientDE = GetClientDE();

	switch(messageID)
	{
		case SMSG_BOOTED:
		{
			CSPrint(IDS_BOOTED);
			PlaySound(MSG_SOUND_STRING);
			break;
		}

		case SMSG_SERVERSHUTDOWN:
		{
			CSPrint(IDS_SERVERSHUTDOWN);
			PlaySound(MSG_SOUND_STRING);
			break;
		}

		case SMSG_PINGTIMES:
		{
			while(1)
			{
				D_WORD id, ping;

				id = pClientDE->ReadFromMessageWord(hMessage);
				if(id == 0xFFFF) break;

				ping = pClientDE->ReadFromMessageWord(hMessage);

				m_FragInfo.UpdatePlayerPing(id, ping);
			}
		}
		break;

		case SMSG_PHYSICS_UPDATE:
		{
			if(m_pMoveMgr)
				m_pMoveMgr->OnPhysicsUpdate(hMessage);
		}
		break;

		case SMSG_FORCEPOS:
		{
			if(m_pMoveMgr)
				m_pMoveMgr->OnServerForcePos(hMessage);
		}
		break;

		case SMSG_PLAYERSTATSUPDATE:
			{
				DBYTE nStatsFlags = pClientDE->ReadFromMessageByte(hMessage);
				if( nStatsFlags & PLAYERSTATS_HEALTH )
				{
					DDWORD nHealth = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
					DDWORD nMaxHealth = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
					m_NewStatusBar.SetHealth(nHealth, nMaxHealth);
					m_nHealth = nHealth;

					if (m_bDead && m_nHealth >= 0)
					{
						m_playerCamera.SetFirstPerson();
						m_bDead        = DFALSE;
						m_bDeadNoHide  = DFALSE;
						m_bDrawFragBar = DFALSE;
						m_bBlind       = DFALSE;
						m_nTrapped     = 0;

						// Turn off wonkyvision if it was on.
						EndWonkyVision();
					}
				}
				if( nStatsFlags & PLAYERSTATS_AMMO )
				{
					DDWORD dwAmmo = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
					m_NewStatusBar.SetAmmo(dwAmmo);
					m_dwAmmo = dwAmmo;
				}
				if( nStatsFlags & PLAYERSTATS_ALTAMMO )
				{
					DDWORD dwAltAmmo = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
					m_dwAltAmmo = dwAltAmmo;

					if(m_pWeapon && (m_pWeapon->GetAmmoType(DTRUE) != m_pWeapon->GetAmmoType(DFALSE)))
						m_NewStatusBar.SetAltAmmo(dwAltAmmo);
					else
						m_NewStatusBar.SetAltAmmo(0);
				}
				if( nStatsFlags & PLAYERSTATS_ARMOR )
				{
					DDWORD nArmor = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
					DDWORD nMaxArmor = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
					m_NewStatusBar.SetArmor(nArmor, nMaxArmor);
				}
				if( nStatsFlags & PLAYERSTATS_FOCUS )
				{
					DDWORD nAmmo = pClientDE->ReadFromMessageWord(hMessage);
				}
				if( nStatsFlags & PLAYERSTATS_TOOKDAMAGE )
				{
					DFLOAT fDamage = (DFLOAT)pClientDE->ReadFromMessageByte(hMessage);
					DBYTE nDamageType = pClientDE->ReadFromMessageByte(hMessage);

					if (nDamageType != DAMAGE_TYPE_FREEFALL)
					{
						// Flash the screen up to 1 seconds depending on damage
						DFLOAT fTime = (fDamage/200.0f);
						// Ramp up = 0.2f
						// Ramp down = 0.2f
						DVector vFlashColor;
						DFLOAT fRed = fDamage / 200.0f;

						// minimum flash should be 0.2
						if (m_vFlashColor.x < 0.2f && fRed < 0.2f) fRed = 0.2f;
						VEC_SET(vFlashColor, fRed, 0.0f, 0.0f);

						FlashScreen(&vFlashColor, fTime, 0.1f);
					}
				}
				if( nStatsFlags & PLAYERSTATS_AIRLEVEL )
				{
					DFLOAT fPercent = pClientDE->ReadFromMessageFloat(hMessage);

					if((fPercent <= 0.0f) || (fPercent >= 1.0f))
						m_NewStatusBar.TurnOffPowerBar();
					else
						m_NewStatusBar.SetPowerBarLevel(BARTAB_AIR, fPercent);
				}
			}
			break;

		case SMSG_THROWDISTANCE:
			{
				DFLOAT fPercent = pClientDE->ReadFromMessageFloat(hMessage);

				if((fPercent <= 0.0f) || (fPercent >= 1.0f))
					m_NewStatusBar.TurnOffPowerBar();
				else
					m_NewStatusBar.SetPowerBarLevel(BARTAB_THROW, fPercent);
			}
			break;

		case SMSG_BOSSHEALTH:
			{
				DFLOAT fPercent = pClientDE->ReadFromMessageFloat(hMessage);

				if((fPercent <= 0.0f) || (fPercent >= 1.0f))
					m_NewStatusBar.TurnOffPowerBar();
				else
					m_NewStatusBar.SetPowerBarLevel(BARTAB_BOSS, fPercent);
			}
			break;

		case SMSG_HEALTH:
			{
				DDWORD nHealth = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
				DDWORD nMaxHealth = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
				m_NewStatusBar.SetHealth(nHealth, nMaxHealth);
				m_nHealth = nHealth;

				if (m_bDead && m_nHealth >= 0)
				{
					m_playerCamera.Reset();
					m_bDead        = DFALSE;
					m_bDrawFragBar = DFALSE;
					m_bBlind       = DFALSE;
					m_nTrapped     = 0;
				}
			}
			break;

		case SMSG_ARMOR:
			{
				DDWORD nArmor = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
				DDWORD nMaxArmor = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
				m_NewStatusBar.SetArmor(nArmor, nMaxArmor);
				break;
			}

		case SMSG_AMMO:
			{
				DDWORD nAmmo = (DDWORD)pClientDE->ReadFromMessageWord(hMessage);
				m_NewStatusBar.SetAmmo(nAmmo);
				break;
			}
		
		case SMSG_FOCUS:
			{
				DDWORD nAmmo = pClientDE->ReadFromMessageWord(hMessage);
				break;
			}

		case SMSG_EYELEVEL:
			m_fEyeLevel = pClientDE->ReadFromMessageFloat(hMessage);
			break;
		
#ifdef _DEBUG
		case STC_BPRINT:
#endif
		case SMSG_CONSOLEMESSAGE:
		{
			CSPrint2(pClientDE->ReadFromMessageString(hMessage));
			break;
		}

		case SMSG_CONSOLEMESSAGE_ALL:
		{
			CSPrint2(pClientDE->ReadFromMessageString(hMessage));
			PlaySound(MSG_SOUND_STRING);
			break;
		}

		case SMSG_FIREDWEAPON:
			{
				tbyte = pClientDE->ReadFromMessageByte(hMessage);
				if (m_pWeapon && !m_pExternalCamera)
				{
//					m_pWeapon->Fire(tbyte);
					if (m_pWeapon->IsCumulativeKick())
					{
						m_fViewKick += m_pWeapon->GetViewKick();
						if (m_fViewKick > 45.0f) m_fViewKick = 44.5f;
						m_fKickTime = __max(pClientDE->GetFrameTime() * 5, 0.3f);
					}
					else if (m_fViewKick < m_pWeapon->GetViewKick())
					{
						m_fViewKick = m_pWeapon->GetViewKick();
						m_fKickTime = 0.001f;
					}
				}

				// Also fire the left hand weapon if we've got one
				if (m_pLWeapon && !m_pExternalCamera)
				{
//					m_pLWeapon->Fire(tbyte);
					if (m_pLWeapon->IsCumulativeKick())
					{
						m_fViewKick += m_pLWeapon->GetViewKick();
						if (m_fViewKick > 45.0f) m_fViewKick = 44.5f;
						m_fKickTime = __max(pClientDE->GetFrameTime() * 5, 0.3f);
					}
					else if (m_fViewKick < m_pLWeapon->GetViewKick())
					{
						m_fViewKick = m_pLWeapon->GetViewKick();
						m_fKickTime = 0.001f;
					}
				}
			}
			break;

		case SMSG_TOOKDAMAGE:
			{
				DFLOAT fDamage = (DFLOAT)pClientDE->ReadFromMessageByte(hMessage);
				DBYTE nDamageType = pClientDE->ReadFromMessageByte(hMessage);

				if (nDamageType != DAMAGE_TYPE_FREEFALL)
				{
					// Flash the screen up to 1 seconds depending on damage
					DFLOAT fTime = (fDamage/200.0f);
					// Ramp up = 0.2f
					// Ramp down = 0.2f
					DVector vFlashColor;
					DFLOAT fRed = fDamage / 200.0f;

					// minimum flash should be 0.2
					if (m_vFlashColor.x < 0.2f && fRed < 0.2f) fRed = 0.2f;
					VEC_SET(vFlashColor, fRed, 0.0f, 0.0f);

					FlashScreen(&vFlashColor, fTime, 0.1f);
				}
			}
			break;


		case SMSG_SPECTATORMODE:
			m_bSpectatorMode = pClientDE->ReadFromMessageByte(hMessage);
/*			if (m_bSpectatorMode)
			{
				DVector vLS;
				VEC_SET(vLS, -.5f, -.5f, -.5f);
//				pClientDE->SetGlobalLightScale(&vLS);
				pClientDE->OffsetGlobalLightScale(&vLS);
			}
			else
			{
				DVector vLS;
				VEC_SET(vLS, 1.0f, 1.0f, 1.0f);
//				pClientDE->SetGlobalLightScale(&vLS);
				pClientDE->OffsetGlobalLightScale(&vLS);
			}
*/			break;

		case SMSG_NEWWEAPON:
			{
				HLOCALOBJ hWeapObj = DNULL;

				DBYTE bySlot          = pClientDE->ReadFromMessageByte(hMessage);
				DBYTE byWeaponID      = pClientDE->ReadFromMessageByte(hMessage);
				DBOOL bLeftHand		  = (DBOOL)pClientDE->ReadFromMessageByte(hMessage);
/*				hWeapObj			  = pClientDE->ReadFromMessageObject(hMessage);
				wd.m_szWeaponName     = pClientDE->ReadFromMessageString(hMessage);
				wd.m_szFlashSprite    = pClientDE->ReadFromMessageString(hMessage);
				wd.m_szAltFlashSprite = pClientDE->ReadFromMessageString(hMessage);
				wd.m_fFlashDuration   = pClientDE->ReadFromMessageFloat(hMessage);
				wd.m_fFlashScale	  = pClientDE->ReadFromMessageFloat(hMessage);
				pClientDE->ReadFromMessageVector(hMessage, &wd.m_GunOffset);
				pClientDE->ReadFromMessageVector(hMessage, &wd.m_MuzzleOffset);
				pClientDE->ReadFromMessageVector(hMessage, &wd.m_FlashPos);
				wd.m_EjectInterval    = pClientDE->ReadFromMessageFloat(hMessage);
				wd.m_fViewKick        = pClientDE->ReadFromMessageFloat(hMessage);
				wd.m_bCumulativeKick  = (DBOOL)pClientDE->ReadFromMessageByte(hMessage);
				wd.m_bLeftHand		  = (DBOOL)pClientDE->ReadFromMessageByte(hMessage);

				HSTRING weap		= pClientDE->ReadFromMessageHString(hMessage);
				HSTRING weapH		= pClientDE->ReadFromMessageHString(hMessage);
*/
				WeaponData *wd		= &g_WeaponDefaults[byWeaponID-1];
				HSTRING weap = DNULL, weapH = DNULL;

				if(wd->m_szStatusIcon)
					weap = pClientDE->CreateString(wd->m_szStatusIcon);
				if(wd->m_szStatusIconH)
					weapH = pClientDE->CreateString(wd->m_szStatusIconH);

				if(bySlot < 10)
				{
					m_NewStatusBar.UpdateWeap(weap, weapH, bySlot);
					m_NewStatusBar.UpdateCurrentWeap(m_nCurGun);
				}

				if (weap)		pClientDE->FreeString(weap);
				if (weapH)		pClientDE->FreeString(weapH);

				// New gun in the current slot? Get rid of what is there now..
					// See if we already have a weapon in this slot
				if (!bLeftHand)
				{
					if (m_nCurGun == bySlot)
					{
						if (m_pWeapon) delete m_pWeapon;
						m_pWeapon = CreateWeapon(byWeaponID, DFALSE);
					}
					m_abyWeaponID[bySlot] = byWeaponID;
				}
				else // bLeftHand == TRUE
				{
					if (m_nCurGun == bySlot)
					{
						if (m_pLWeapon) delete m_pLWeapon;
						m_pLWeapon = CreateWeapon(byWeaponID, DTRUE);
					}
					m_abyLWeaponID[bySlot] = byWeaponID;
				}
			}
			break;

		case SMSG_DELETESPECIFICWEAPON:
		case SMSG_DELETEWEAPON:
		{
			DBYTE slot = pClientDE->ReadFromMessageByte(hMessage);

			// First see if we have a left handed model, and it's the current gun..
			if (m_abyLWeaponID[slot] && slot == m_nCurGun)
			{
				if (m_pLWeapon)
				{
					m_pLWeapon->Hide();
					m_pLWeapon->Term();
					delete m_pLWeapon;
					m_pLWeapon = NULL;
				}
				m_abyLWeaponID[slot] = WEAP_NONE;
			}
			// Or else check for a right handed model
			else if (m_abyWeaponID[slot] && slot == m_nCurGun)
			{
				if(slot < 10)
				{
					m_NewStatusBar.UpdateWeap(0, 0, slot);
				}
				if (m_pWeapon)
				{
					m_pWeapon->Hide();
					m_pWeapon->Term();
					delete m_pWeapon;
					m_pWeapon = NULL;
				}
				m_abyWeaponID[slot] = WEAP_NONE;
			}
		}
		break;

		case SMSG_CHANGEWEAPON:
		{
			DBYTE slot = pClientDE->ReadFromMessageByte(hMessage);
			ChangeWeapon(slot);
		}
		break;

		case SMSG_ITEMCHANGED:
		{
			DBYTE nChangeFlags;
			DBOOL bShow;
			char *pCurName;

			nChangeFlags = pClientDE->ReadFromMessageByte(hMessage);
			bShow = ( nChangeFlags & ITEMF_SHOWITEMS ) ? DTRUE : DFALSE;

			if ( nChangeFlags & ITEMF_CURRENTITEM )
			{
				if( m_hCurrentItemName )
				{
					pClientDE->FreeString( m_hCurrentItemName );
					m_hCurrentItemName = DNULL;
				}
				if( m_hCurrentItemIcon )
				{
					pClientDE->FreeString( m_hCurrentItemIcon );
					m_hCurrentItemIcon = DNULL;
				}
				if( m_hCurrentItemIconH )
				{
					pClientDE->FreeString( m_hCurrentItemIconH );
					m_hCurrentItemIconH = DNULL;
				}

				pCurName = pClientDE->ReadFromMessageString(hMessage);
				if( pCurName[0] )
				{
					m_hCurrentItemName	= pClientDE->CreateString( pCurName );
					m_hCurrentItemIcon	= pClientDE->ReadFromMessageHString(hMessage);
					m_hCurrentItemIconH	= pClientDE->ReadFromMessageHString(hMessage);
				}
			}

			if ( nChangeFlags & ITEMF_PREVITEM )
			{
				if( m_hPrevItemIcon )
					pClientDE->FreeString( m_hPrevItemIcon );
				m_hPrevItemIcon		= pClientDE->ReadFromMessageHString(hMessage);
			}

			if ( nChangeFlags & ITEMF_NEXTITEM )
			{
				if( m_hNextItemIcon )
					pClientDE->FreeString( m_hNextItemIcon );
				m_hNextItemIcon		= pClientDE->ReadFromMessageHString(hMessage);
			}

			if ( nChangeFlags & ITEMF_CURRENTITEMCHARGE )
			{
				m_nCurrentItemCharge = pClientDE->ReadFromMessageByte(hMessage);
			}

			if ( nChangeFlags & ITEMF_PREVITEMCHARGE )
			{
				m_nPrevItemCharge = pClientDE->ReadFromMessageByte(hMessage);
			}

			if ( nChangeFlags & ITEMF_NEXTITEMCHARGE )
			{
				m_nNextItemCharge = pClientDE->ReadFromMessageByte(hMessage);
			}

			m_NewStatusBar.UpdateInv( bShow, m_hCurrentItemName, m_hCurrentItemIcon, m_hCurrentItemIconH, m_nCurrentItemCharge, m_hPrevItemIcon, m_nPrevItemCharge, m_hNextItemIcon, m_nNextItemCharge);
		}
		break;

		case SMSG_CONVERSATION:
		{
			GetVoiceMgr()->StopAll();
			HSTRING file = pClientDE->ReadFromMessageHString(hMessage);
			HSTRING text = pClientDE->ReadFromMessageHString(hMessage);
			DBYTE chosen = pClientDE->ReadFromMessageByte(hMessage);
			m_CommLink.StartCommunication(chosen, pClientDE->GetStringData(file), pClientDE->GetStringData(text));
			pClientDE->FreeString( file );
			pClientDE->FreeString( text );
		}
		break;

		case SMSG_OBJECTIVES:
		{
			if ((m_nGameType == GAMETYPE_SINGLE) || (m_nGameType == GAMETYPE_CUSTOM))
			{
				DDWORD	resNum = pClientDE->ReadFromMessageDWord(hMessage);

				if (m_hstrObjectivesText)
				{
					pClientDE->FreeString(m_hstrObjectivesText);
					m_hstrObjectivesText = DNULL;
				}

				if (m_hstrObjectivesTitle)
				{
					pClientDE->FreeString(m_hstrObjectivesTitle);
					m_hstrObjectivesTitle = DNULL;
				}

				if(resNum)
				{
					m_hstrObjectivesTitle = pClientDE->FormatString(IDS_OBJECTIVE_TITLE_1_1 + resNum - 1);
					m_hstrObjectivesText  = pClientDE->FormatString(IDS_OBJECTIVE_1_1 + resNum - 1);
				}
				else
				{

					m_hstrObjectivesTitle = pClientDE->ReadFromMessageHString(hMessage);
					m_hstrObjectivesText  = pClientDE->ReadFromMessageHString(hMessage);
				}

				m_NewStatusBar.UpdateObjectives(pClientDE->GetStringData(m_hstrObjectivesTitle), pClientDE->GetStringData(m_hstrObjectivesText));
			}
		}
		break;

		case SMSG_ZOOMVIEW:
		{
			m_bZoomView = pClientDE->ReadFromMessageByte(hMessage);
			DBYTE nType = pClientDE->ReadFromMessageByte(hMessage);

			if(m_bZoomView)
			{
//				m_fOriginalKeyTurnRate = GetKeyboardTurnRate();
//				SetKeyboardTurnRate(m_fOriginalKeyTurnRate / 4.0f);
//				m_fOriginalMouseSensitivity = GetMouseSensitivity();
//				SetMouseSensitivity(m_fOriginalMouseSensitivity / 4.0f, DTRUE);

				m_bCrosshairOriginallyOn = m_bShowCrosshair;
				m_nCrosshairOriginalNum = m_nCrosshair;

				if(nType)
				{
					m_bShowCrosshair = DTRUE;
					m_nCrosshair = nType;
				}
			}
			else
			{
//				SetKeyboardTurnRate(m_fOriginalKeyTurnRate);
//				SetMouseSensitivity(m_fOriginalMouseSensitivity);
				m_bShowCrosshair = m_bCrosshairOriginallyOn;
				m_nCrosshair = m_nCrosshairOriginalNum;
			}
		}
		break;

		case SMSG_ADDPLAYER:
		{
			if (IsMultiplayerGame())
			{
				HSTRING hstrName = pClientDE->ReadFromMessageHString (hMessage);
				DDWORD nID = pClientDE->ReadFromMessageDWord (hMessage);
				DBYTE byCharacter  = pClientDE->ReadFromMessageByte (hMessage);
				int nFrags = (int) pClientDE->ReadFromMessageFloat (hMessage);
				DBYTE byTeamID = pClientDE->ReadFromMessageByte (hMessage);

				if (m_FragInfo.GetClientInfo(nID))
				{
					pClientDE->FreeString(hstrName);
					return;
				}

				m_FragInfo.AddClient (hstrName, nID, nFrags, byCharacter);

				DDWORD dwLocalID = 0;
				pClientDE->GetLocalClientID(&dwLocalID);
				if (dwLocalID == nID)
				{
					m_NewStatusBar.SetTeamID((int)byTeamID);
				}

				m_TeamMgr.AddPlayer(byTeamID, nID);
				m_TeamMgr.AddPlayerFrags(byTeamID, nID, nFrags);
				CTeam* pTeam = m_TeamMgr.GetTeam(byTeamID);

				if (IsMultiplayerTeamBasedGame() && pTeam)
				{
					HSTRING hStr = pClientDE->FormatString(IDS_ADDPLAYERTEAM, pClientDE->GetStringData(hstrName), pTeam->GetName());
					CSPrint(pClientDE->GetStringData(hStr));
					pClientDE->FreeString (hStr);
				}
				else
				{
					HSTRING hStr = pClientDE->FormatString (IDS_ADDPLAYER, pClientDE->GetStringData (hstrName));
					CSPrint (pClientDE->GetStringData (hStr));
					pClientDE->FreeString (hStr);
				}

				pClientDE->FreeString(hstrName);
			}
		}
		break;

		case SMSG_REMOVEPLAYER:
		{
			if (IsMultiplayerGame())
			{
				DDWORD nID   = pClientDE->ReadFromMessageDWord(hMessage);
				char*  sName = m_FragInfo.GetPlayerName(nID);

				if (sName && strlen(sName) > 0)
				{
					DDWORD dwLocalID = 0xFFFFFFFF;
					pClientDE->GetLocalClientID(&dwLocalID);

					if (dwLocalID != nID)
					{
						HSTRING hStr = pClientDE->FormatString(IDS_REMOVEPLAYER, sName);
						CSPrint (pClientDE->GetStringData(hStr));
						pClientDE->FreeString(hStr);
					}
				}
				
				m_FragInfo.RemoveClient (nID);
				m_TeamMgr.RemovePlayer(nID);

				if (g_bLevelChange3rdPersonCam)
				{
					g_bLevelChangeNoUpdate = DTRUE;
				}
			}
		}
		break;

		case SMSG_PLAYER_FRAGGED:
		{
			if (IsMultiplayerGame())
			{
				DDWORD nLocalID = 0;
				_mbscpy((unsigned char*)msgbuf, (const unsigned char*)"");
				pClientDE->GetLocalClientID (&nLocalID);

				DDWORD nVictim = pClientDE->ReadFromMessageDWord (hMessage);
				DDWORD nKiller = pClientDE->ReadFromMessageDWord (hMessage);
				D_WORD wIndex = pClientDE->ReadFromMessageWord (hMessage);

				AssignFrags(nLocalID, nVictim, nKiller);

				VoiceEntry *pve = DNULL;

				char *pKiller = m_FragInfo.GetPlayerName(nKiller);
				char *pVictim = m_FragInfo.GetPlayerName(nVictim);

				if (pKiller && pVictim)
				{
					DBYTE byFlags = 0;
					switch(m_FragInfo.GetPlayerCharacter(nKiller))
					{
						case CHARACTER_GABREILLA:
						case CHARACTER_OPHELIA:
							byFlags |= VOICEFLAG_FEMALE_ATTACKER;
							break;
						default:
							byFlags |= VOICEFLAG_MALE_ATTACKER;
							break;
					}
					
					switch(m_FragInfo.GetPlayerCharacter(nVictim))
					{
						case CHARACTER_GABREILLA:
						case CHARACTER_OPHELIA:
							byFlags |= VOICEFLAG_FEMALE_VICTIM;
							break;
						default:
							byFlags |= VOICEFLAG_MALE_VICTIM;
							break;
					}
					
					// Display a message
					if (nVictim == nKiller)		// Suicide
					{
						pve = GetVoiceEntry(g_aVoiceSuicide, 
											NUM_VOICE_SUICIDE, 
											wIndex, 
											byFlags);
	//					pve = &g_aVoiceSuicide[wIndex % NUM_VOICE_SUICIDE]; 
						if (pve)
							sprintf(msgbuf, pve->m_szText, pKiller);
					}
					else						// Frag
					{
						pve = GetVoiceEntry(g_aVoiceKill, 
											NUM_VOICE_KILL, 
											wIndex, 
											byFlags);
	//					pve = &g_aVoiceKill[wIndex % NUM_VOICE_KILL]; 
						if (pve)
							sprintf(msgbuf, pve->m_szText, pKiller, pVictim);
					}

					if (nKiller == nLocalID)
					{
						char szSoundFile[MAX_CS_FILENAME_LEN];
						_mbscpy((unsigned char*)szSoundFile, (const unsigned char*)"Sounds\\Voice\\");
						_mbscat((unsigned char*)szSoundFile, (const unsigned char*)pve->m_szFile);
						m_CommLink.StartCommunication(4, szSoundFile, msgbuf);
					}
					else 
						CSPrint(msgbuf);
				}
			}
		}
		break;

		case SMSG_GRABBEDTHEFLAG:
		{
			DisplayStatusBarFlagIcon(DTRUE);
			PlaySound("sounds_multipatch\\CtfFlagGrab.wav");
			CSPrint(IDS_CTF_GRABBED);
			break;
		}

		case SMSG_RETURNEDTHEFLAG:
		{
			PlaySound("sounds_multipatch\\CtfFlagGrab.wav");
			CSPrint(IDS_CTF_RETURNED);
			break;
		}

		case SMSG_DROPPEDTHEFLAG:
		{
			DisplayStatusBarFlagIcon(DFALSE);
			break;
		}

		case SMSG_NEEDYOURFLAG:
		{
			static DFLOAT fLastTime = 0;
			DFLOAT fCurTime = g_pClientDE->GetTime();

			if ((fLastTime + 20) < fCurTime)
			{
				CSPrint(IDS_CTF_RETURNYOURFLAG);
				fLastTime = fCurTime;
			}
			break;
		}

		case SMSG_CAPTUREDTHEFLAG:
		{
			DisplayStatusBarFlagIcon(DFALSE);

			DDWORD dwLocalID = 0;
			pClientDE->GetLocalClientID(&dwLocalID);

			DDWORD dwFlagger = pClientDE->ReadFromMessageDWord(hMessage);
			int    nFrags    = (int)pClientDE->ReadFromMessageDWord(hMessage);

			m_FragInfo.AddFrags(dwLocalID, dwFlagger, nFrags);
			m_TeamMgr.AddPlayerFrags(dwFlagger, nFrags);

			char* sFlagger = m_FragInfo.GetPlayerName(dwFlagger);
			if (sFlagger)
			{
				char sMsg[256];
				CTeam* pTeam = m_TeamMgr.GetTeamFromPlayerID(dwFlagger);
				if (pTeam) sprintf(sMsg, pClientDE->GetStringData(m_hCtfCapturedString1), sFlagger, pTeam->GetName());
				else sprintf(sMsg, pClientDE->GetStringData(m_hCtfCapturedString2), sFlagger);

				if (dwLocalID == dwFlagger)
				{
					CSPrint(IDS_CTF_CAPTURED_3);
				}
				else
				{
					CSPrint(sMsg);
				}
			}
		}
		break;

#ifdef _ADDON
		case SMSG_SOCCERGOAL:
		{
			DDWORD dwLocalID, dwScorer;
			int nFrags;
			DBYTE nTeamID;
			char *sScorer;
			char sMsg[256];
			CTeam *pTeam;
			char *pszSound;

			DisplayStatusBarFlagIcon(DFALSE);

			dwLocalID = 0;
			pClientDE->GetLocalClientID(&dwLocalID);

			dwScorer = pClientDE->ReadFromMessageDWord(hMessage);
			nFrags = (int)pClientDE->ReadFromMessageDWord(hMessage);
			nTeamID = pClientDE->ReadFromMessageByte( hMessage );

			m_FragInfo.AddFrags( dwLocalID, dwScorer, nFrags );
			m_TeamMgr.AddPlayerFrags( dwScorer, nFrags );

			pszSound = DNULL;
			if( nFrags >= 0 )
			{
				pszSound = s_szCheerSounds[ GetRandom( 0, NUMCHEERSOUNDS - 1 ) ];
			}
			else
			{
				pszSound = s_szBooSounds[ GetRandom( 0, NUMBOOSOUNDS - 1 ) ];
			}
			if( pszSound )
				PlaySoundLocal( pszSound, SOUNDPRIORITY_MISC_MEDIUM );

			if( dwLocalID == dwScorer )
			{
				// Made a good goal
				if( nFrags >= 0 )
					CSPrint(IDS_SOCCER_GOAL_3);
				// Wrong goal!
				else
					CSPrint(IDS_SOCCER_GOAL_4);
			}
			else
			{
				sScorer = m_FragInfo.GetPlayerName( dwScorer );
				if( sScorer )
				{
					pTeam = m_TeamMgr.GetTeam(( nTeamID == TEAM_1 ) ? TEAM_2 : TEAM_1 );
					if( pTeam )
						sprintf( sMsg, pClientDE->GetStringData( m_hSoccerGoalString1 ), sScorer, pTeam->GetName( ));
					else
						sprintf( sMsg, pClientDE->GetStringData( m_hSoccerGoalString2 ), sScorer );
	
					CSPrint(sMsg);
				}
			}
		}
		break;

		case SMSG_TRAPPED:
		{
			m_nTrapped = (int)pClientDE->ReadFromMessageByte(hMessage);
			DBYTE byType = pClientDE->ReadFromMessageByte(hMessage);
		}
		break;

#endif // _ADDON

		case SMSG_THEVOICE:
		{
			if (IsMultiplayerGame())
			{
				TheVoice eVoice		= (TheVoice)pClientDE->ReadFromMessageByte(hMessage);
				D_WORD  wIndex		= pClientDE->ReadFromMessageWord(hMessage);
				DBYTE	byFlags		= pClientDE->ReadFromMessageByte(hMessage);
				
				VoiceEntry *pve = DNULL;
				switch(eVoice)
				{
					case VOICE_START_BB:
						pve = GetVoiceEntry(g_aVoiceStartBB, 
											NUM_VOICE_START_BB, 
											wIndex, 
											byFlags);
						break;

					case VOICE_START_CTF:
						pve = GetVoiceEntry(g_aVoiceStartCTF, 
											NUM_VOICE_START_CTF, 
											wIndex, 
											byFlags);
						break;

					case VOICE_FINISHHIM:
						pve = GetVoiceEntryWithExtraChecks(g_aVoiceHumiliation, 
											NUM_VOICE_HUMILIATION, 
											wIndex, 
											byFlags);
						break;
				}

				if (pve)
				{
					sprintf(msgbuf, "Sounds\\Voice\\%s", pve->m_szFile);
					m_CommLink.StartCommunication(4, msgbuf, pve->m_szText);
				}
			}
		}
		break;

		case SMSG_MP_CHANGING_LEVELS :
		{
			// we're about to change levels - we don't really care if we're not in a multiplayer game
			if (!IsMultiplayerGame()) return;

			pClientDE->SetInputState(DFALSE);

			if (!m_b3rdPerson)
				Set3rdPersonCamera(DTRUE);

			CSPrint(IDS_MP_CHANGINGLEVELS);
			PlaySound("sounds\\events\\bellding1.wav");

			m_bDrawFragBar = DTRUE;
			g_bLevelChange3rdPersonCam = DTRUE;

			m_bZoomView = DFALSE;
			m_bShowCrosshair = m_bCrosshairOriginallyOn;
			m_nCrosshair = m_nCrosshairOriginalNum;

			m_bBlind = DFALSE;
			m_nTrapped = 0;

			// Reset the inventory tabs
			m_NewStatusBar.UpdateInv(0, 0, 0, 0, 0, 0, 0, 0, 0);

			// Handle clearing out data for movement and weapon states
			m_pMoveMgr->ClearControlFlags();
			if (m_pWeapon)		m_pWeapon->CancelFiringState();
			if (m_pLWeapon)		m_pLWeapon->CancelFiringState();
		}
		break;

		case SMSG_EXITWORLD:
		{
			HandleExitWorld(hMessage);

			// Reset the inventory tabs
			m_NewStatusBar.UpdateInv(0, 0, 0, 0, 0, 0, 0, 0, 0);
		}
		break;
 /*           
		case SMSG_CUTSCENE_START :
		{
			GetVoiceMgr()->StopAll();
        	m_bDrawStatusBar = DFALSE;
		}
		break;
            
		case SMSG_CUTSCENE_END :
		{
			m_bSpectatorMode = DFALSE;
			m_pExternalCamera = DNULL;

			m_bRenderCamera = DFALSE;

			if (m_pWeapon[m_nCurGun])
				m_pWeapon[m_nCurGun]->Show();
			if (m_pLWeapon[m_nCurGun])
				m_pLWeapon[m_nCurGun]->Show();
            
            // Turn on the Status bar
        	m_bDrawStatusBar = DTRUE;
            
			m_bRenderCamera = DTRUE;
		}
		break;
*/
            
		case SMSG_BURN :
		{
			DFLOAT fTheTime = pClientDE->ReadFromMessageFloat(hMessage);
            if (fTheTime > 0.0f)
            {
                m_bBurn = DTRUE;
            	m_fBurnTime = pClientDE->GetTime() + fTheTime;
            }
            else
            {
                m_bBurn = DFALSE;
            }
		}
		break;
        
        
		case SMSG_BLIND:
		{
			if (m_bDead)	// [blg] don't go blind if we're already dead
			{
                m_bBlind = DFALSE;
				return;
			}

			DFLOAT fTheTime = pClientDE->ReadFromMessageFloat(hMessage);
            
            if (fTheTime > 0.0f)
            {
				HSTRING szErrorStr = pClientDE->FormatString(IDS_GENERAL_BLIND);
				CSPrint(pClientDE->GetStringData(szErrorStr));
				pClientDE->FreeString(szErrorStr);

                m_bBlind     = DTRUE;
            	m_fBlindTime = pClientDE->GetTime() + fTheTime;
            }
            else
            {
                m_bBlind = DFALSE;
            }
		}
		break;
            
            
		case SMSG_ALL_SEEING_EYE:
		{
			DBOOL bOn = (DBOOL)pClientDE->ReadFromMessageByte(hMessage);
            HLOCALOBJ hObj = pClientDE->ReadFromMessageObject(hMessage);
            
			// Kill any old instance of the sound...
			if( m_hTheEyeLoopingSound )
			{
				pClientDE->KillSound( m_hTheEyeLoopingSound );
				m_hTheEyeLoopingSound = DNULL;
			}

            if (bOn && hObj)
            {
                m_hSeeingEyeObj = hObj;
				pClientDE->SetObjectClientFlags(hObj, CF_NOTIFYREMOVE);
				m_hTheEyeLoopingSound = PlaySoundLocal("sounds\\powerups\\eyeloop1.wav", 
					SOUNDPRIORITY_MISC_HIGH, DTRUE, DTRUE );
            }
            else
            {
                m_hSeeingEyeObj = DNULL;
            }                
		}
		break;
          
		case SMSG_ORBCAM:
		{
			m_hOrbObj = DNULL;
		}
		break;
          
            
		case SMSG_DISPLAYTEXT:
		{   
			DFLOAT fTime = pClientDE->ReadFromMessageFloat(hMessage);		
			CSPrint(pClientDE->ReadFromMessageString(hMessage));
        
        // Need to send the Font, Height, Color, Position on the Screen x,y
        //    
        //    	m_hFont = pClientDE->CreateFont("Arial", 6, 16, DFALSE, DFALSE, DFALSE);
        //    	m_nFontHeight = 16;
        //    	HDECOLOR hForeground = pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, DFALSE);
        //    	hSurface = pClientDE->CreateSurfaceFromString (m_hFont, hMsg, hForeground, NULL, 10, 0);
        //    	if (hSurface)  // Error
        //    	{
        //    		return;
        //    	}
            
		}   
		break;
         


		case SMSG_SCREENFADE:
        {
			DBYTE m_nFadeMode = pClientDE->ReadFromMessageByte(hMessage);
            if (m_nFadeMode == 1)
            {
				m_bFadeIn = DTRUE;
				m_bFadeOut = DFALSE;
				m_fFadeVal = 0.0f;
			}
			else
			{
				m_bFadeOut = DTRUE;
				m_bFadeIn = DFALSE;
				m_fFadeVal = 255.0f;
			}
        }    
		break;

/*		case SMSG_STONEVIEW:
		{
			m_bStoneView = pClientDE->ReadFromMessageByte(hMessage);
			break;
		}
            
		case SMSG_AURAVIEW:
		{
			m_bAuraView = pClientDE->ReadFromMessageByte(hMessage);
			if (m_bAuraView)
				pClientDE->SetModelHook ((ModelHookFn)AuraModelHook, pClientDE);
			else
				pClientDE->SetModelHook (DNULL, DNULL);
			break;
		}
*/
		case SMSG_FORCEROTATION:
		{
			m_fPitch = pClientDE->ReadFromMessageFloat(hMessage);
			m_fYaw = pClientDE->ReadFromMessageFloat(hMessage);
			break;
		}


		case SMSG_MUSIC:
		{
			if( m_Music.IsInitialized( ))
			{
				m_Music.HandleMusicMessage( hMessage );
				m_eMusicLevel = m_Music.GetMusicLevel( );
			}
			break;
		}

		// Returned when the server has successfully saved a game
		// Copy save data from the Current slot into the 
		case SMSG_SAVEGAME_ACK:
		{
			if (GetGameState() == GS_SAVING)
			{
				// Save the current world filename for future restoration
				int nSlot = m_SavedGameInfo.GetReservedSlot();
				if (nSlot == SLOT_QUICK)
					_mbscpy((unsigned char*)m_SavedGameInfo.gQuickSaveInfo.szCurrentLevel, (const unsigned char*)m_szFilename);
				else
					_mbscpy((unsigned char*)m_SavedGameInfo.gSaveSlotInfo[nSlot].szCurrentLevel, (const unsigned char*)m_szFilename);

				if (nSlot != SLOT_NONE)
				{
					m_SavedGameInfo.CopySlot(SLOT_CURRENT, nSlot);
					m_SavedGameInfo.SetSlotName(nSlot, DNULL);
					m_SavedGameInfo.SaveInfo(nSlot, m_nGameType, GetCharacter());
				}
				m_SavedGameInfo.SetReservedSlot(SLOT_NONE);

				SetGameState(GS_PLAYING); 
	            PauseGame(DFALSE);
			}
			break;
		}

		// Successfully loaded a world, keep track of it for autosave
		case SMSG_LOADWORLD_ACK:
		{
			DBOOL bSuccess = (DBOOL)pClientDE->ReadFromMessageByte(hMessage);

			if (bSuccess)
			{
				// Set the world name
				if (m_hstrTitle)
					m_SavedGameInfo.SetSlotName(SLOT_CURRENT, pClientDE->GetStringData(m_hstrTitle));

				// Save the current world filename for future restoration
				_mbscpy((unsigned char*)m_SavedGameInfo.gCurrentSaveInfo.szCurrentLevel, (const unsigned char*)m_szFilename);
				m_SavedGameInfo.SaveInfo(SLOT_CURRENT, m_nGameType, GetCharacter());
			}
			else  // Abort! Abort!
			{
				SetGameState(GS_MENU);
//				pClientDE->Disconnect();
			}
			break;
		}

		case SMSG_CLIENTSAVEDATA:
		{
			LoadClientData( hMessage );
			break;
		}

		case SMSG_PLAYERINIT_SINGLEPLAYER:
		{
			InitSinglePlayer();
		}
		break;

		case SMSG_PLAYERINIT_MULTIPLAYER:
		{
			BYTE byGameType = pClientDE->ReadFromMessageByte(hMessage);
			m_nGameType = byGameType;

			m_bNetFriendlyFire   = pClientDE->ReadFromMessageByte(hMessage);
			m_bNetNegTeamFrags   = pClientDE->ReadFromMessageByte(hMessage);
			m_bNetOnlyFlagScores = pClientDE->ReadFromMessageByte(hMessage);
			m_bNetOnlyGoalScores = pClientDE->ReadFromMessageByte(hMessage);

			InitMultiPlayer();

			if (m_pWeapon)		m_pWeapon->CancelFiringState();
			if (m_pLWeapon)		m_pLWeapon->CancelFiringState();
		}
		break;

		case SMSG_THIEF_ATTACH:
		{
			HOBJECT hServObj = pClientDE->ReadFromMessageObject(hMessage);
			HOBJECT hEnemyObj = pClientDE->ReadFromMessageObject(hMessage);

			if(m_pCreature && hServObj && hEnemyObj)
			{
				HMESSAGEWRITE hMessage = pClientDE->StartMessage(CMSG_DETACH_AI);
				pClientDE->WriteToMessageObject(hMessage, hServObj);
				pClientDE->EndMessage(hMessage);
			}
			else
			{
				m_pCreature = new CViewCreature;
				m_pCreature->Create(pClientDE, SMSG_THIEF_ATTACH, hServObj, hEnemyObj);

				HMESSAGEWRITE hMessage = pClientDE->StartMessage(CMSG_ATTACH_ACK);
				pClientDE->WriteToMessageObject(hMessage, hServObj);
				pClientDE->EndMessage(hMessage);
			}

			break;
		}

		case SMSG_HAND_ATTACH:
		{
			HOBJECT hServObj = pClientDE->ReadFromMessageObject(hMessage);
			HOBJECT hEnemyObj = pClientDE->ReadFromMessageObject(hMessage);

			if(m_pCreature && hServObj && hEnemyObj)
			{
				HMESSAGEWRITE hMessage = pClientDE->StartMessage(CMSG_DETACH_AI);
				pClientDE->WriteToMessageObject(hMessage, hServObj);
				pClientDE->EndMessage(hMessage);
			}
			else
			{
				m_pCreature = new CViewCreature;
				m_pCreature->Create(pClientDE, SMSG_HAND_ATTACH, hServObj, hEnemyObj);

				HMESSAGEWRITE hMessage = pClientDE->StartMessage(CMSG_ATTACH_ACK);
				pClientDE->WriteToMessageObject(hMessage, hServObj);
				pClientDE->EndMessage(hMessage);
			}

			break;
		}

		case SMSG_BONELEECH_ATTACH:
		{
			HOBJECT hServObj = pClientDE->ReadFromMessageObject(hMessage);
			HOBJECT hEnemyObj = pClientDE->ReadFromMessageObject(hMessage);

			if(m_pCreature && hServObj && hEnemyObj)
			{
				HMESSAGEWRITE hMessage = pClientDE->StartMessage(CMSG_DETACH_AI);
				pClientDE->WriteToMessageObject(hMessage, hServObj);
				pClientDE->EndMessage(hMessage);
			}
			else
			{
				m_pCreature = new CViewCreature;
				m_pCreature->Create(pClientDE, SMSG_BONELEECH_ATTACH, hServObj, hEnemyObj);

				HMESSAGEWRITE hMessage = pClientDE->StartMessage(CMSG_ATTACH_ACK);
				pClientDE->WriteToMessageObject(hMessage, hServObj);
				pClientDE->EndMessage(hMessage);
			}

			break;
		}

		case SMSG_DETACH_AI:
		{
			if(m_pCreature)
			{
				m_pCreature->Detach();
			}

			break;
		}

		case SMSG_DEAD:
		{
			// Just died..
			m_bDead  = DTRUE;
			m_bBlind = DFALSE;
			m_nTrapped = 0;
			m_playerCamera.GoDeathMode();

			if (IsWonkyNoMove()) m_bDeadNoHide = DTRUE;
			EndWonkyVision();

			if(IsMultiplayerGame())
				m_bDrawFragBar = DTRUE;
		}
		break;

		case SMSG_WONKYVISION:		// Set to WonkyVision for a period of time.
		{
			DFLOAT fWonkyTime   = pClientDE->ReadFromMessageFloat(hMessage); 
			DBYTE  bWonkyNoMove = pClientDE->ReadFromMessageByte(hMessage); 

			if (m_bDead) fWonkyTime = 0;	// [blg] no wonky if we're dead!

			if (fWonkyTime > m_fWonkyTime)
			{
				if (bWonkyNoMove) CSPrint(IDS_EDGEOFDEATH);
				StartWonkyVision(fWonkyTime, bWonkyNoMove);
			}
			else if (fWonkyTime == 0)
			{
				EndWonkyVision();
			}

			break;
		}

		case SMSG_VOICEMGR:
		{
			DBOOL bEvent     = (DBOOL)pClientDE->ReadFromMessageByte(hMessage);
			int   iCharacter = (int)pClientDE->ReadFromMessageWord(hMessage);
			int   iEvent     = (int)pClientDE->ReadFromMessageWord(hMessage);

			if (bEvent)
			{
				GetVoiceMgr()->PlayEventSound(iCharacter, iEvent);
			}
			else
			{
				GetVoiceMgr()->PlayUniqueSound(iCharacter, iEvent);
			}

			break;
		}

		case SMSG_VOICEMGR_STOPALL:
		{
			GetVoiceMgr()->StopAll();
			break;
		}

		case SMSG_INVITEMACTION:
		{
			DBYTE nType;
			DBOOL bActivate;

			nType = g_pClientDE->ReadFromMessageByte( hMessage );
			bActivate = ( nType & 0x80 );
			nType &= 0x7F;

			HandleInvAction( nType, bActivate );
			break;
		}

/*		case SMSG_WEAPONHANDMODEL:
		{
			HLOCALOBJ hObj = g_pClientDE->ReadFromMessageObject(hMessage);
			DBOOL bLeft = g_pClientDE->ReadFromMessageByte(hMessage);
			DBOOL bVisible = g_pClientDE->ReadFromMessageByte(hMessage);

			if (!hObj) break;

			DDWORD dwFlags = pClientDE->GetObjectFlags(hObj);
			if (bVisible)
			{
				pClientDE->SetObjectFlags(hObj, dwFlags | FLAG_VISIBLE);
				pClientDE->SetObjectClientFlags(hObj, 0);
			}
			else
			{
				pClientDE->SetObjectFlags(hObj, dwFlags & ~FLAG_VISIBLE);
				pClientDE->SetObjectClientFlags(hObj, CF_NOTIFYREMOVE);
			}

			if (bLeft)		m_hLWeapHandObj	= (bVisible) ? NULL : hObj;
				else		m_hWeapHandObj	= (bVisible) ? NULL : hObj;

			break;
		}*/

		case SMSG_DOAUTOSAVE:
		{
			MenuSaveGame(SLOT_NONE, SAVETYPE_AUTO);
		}
		break;
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::InitSinglePlayer
//
//	PURPOSE:	Initializes a single player game
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::InitSinglePlayer()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	// Init player info only on the first world...
	if (m_bFirstWorld)
		SendPlayerInitMessage(CMSG_SINGLEPLAYER_INIT);

	// Send autosave message
	HMESSAGEWRITE hWrite = pClientDE->StartMessage(CMSG_AUTOSAVE);
	pClientDE->EndMessage(hWrite);

	if(m_bPaused)	SetGameState(GS_WAITING);
		else		SetGameState(GS_PLAYING);

	pClientDE->SetInputState(DTRUE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::InitMultiPlayer
//
//	PURPOSE:	Send the server the initial player info
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::InitMultiPlayer()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	// Init multiplayer info on server...
	SendPlayerInitMessage(CMSG_MULTIPLAYER_INIT);

	SetGameState(GS_PLAYING);
	pClientDE->SetInputState (DTRUE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateGun()
//
//	PURPOSE:	Updates the view gun info
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::UpdateGun(DRotation *rot, DVector *pos)
{
	CClientDE* pClientDE = GetClientDE();

	// Determine whether we should be showing the weapon or not.
	// Hide gun if spectator mode, or health is 0.
	if (m_bSpectatorMode || m_bDead || !m_playerCamera.IsFirstPerson() || m_pExternalCamera || m_hSeeingEyeObj || m_hOrbObj )
	{
		if (m_pWeapon) m_pWeapon->Hide();
		if (m_pLWeapon) m_pLWeapon->Hide();
	}
	else
	{
		if (m_pWeapon) m_pWeapon->Show();
		if (m_pLWeapon) m_pLWeapon->Show();
	}

	if (!m_bSpectatorMode)
	{
		DVector vGunMuzzlePos, vPos;

		if (m_pWeapon)
		{
			m_pWeapon->UpdateBob(m_fSwayHeight + m_fWeaponY, m_fSwayWidth);
			m_pWeapon->Update(cdata.fPitch, cdata.fYaw, pos);

			pClientDE->GetObjectPos(pClientDE->GetClientObject(), &vPos);
			VEC_COPY(vGunMuzzlePos, m_pWeapon->GetMuzzlePos());
			VEC_SUB(cdata.GunMuzzlePos, vGunMuzzlePos, vPos);
		}
		if (m_pLWeapon)
		{
			m_pLWeapon->UpdateBob(m_fSwayHeight + m_fWeaponY, m_fSwayWidth);
			m_pLWeapon->Update(cdata.fPitch, cdata.fYaw, pos);

			pClientDE->GetObjectPos(pClientDE->GetClientObject(), &vPos);
			VEC_COPY(vGunMuzzlePos, m_pLWeapon->GetMuzzlePos());
			VEC_SUB(cdata.lGunMuzzlePos, vGunMuzzlePos, vPos);
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::ChangeWeapon()
//
//	PURPOSE:	Selects a new weapon
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::ChangeWeapon(DBYTE slot)
{
	// Are we switching weapons?
	// If so, set our flags and get rid of the old one.
	if(slot != m_nCurGun) 
	{
		m_nCurGun = slot;

		if (m_pWeapon) 
		{
			delete m_pWeapon;
			m_pWeapon = DNULL;
		}
		if (m_pLWeapon) 
		{
			delete m_pLWeapon;
			m_pLWeapon = DNULL;
		}

		if (m_abyWeaponID[m_nCurGun])
		{
			m_pWeapon = CreateWeapon(m_abyWeaponID[m_nCurGun], DFALSE);
		}
		if (m_abyLWeaponID[m_nCurGun])
		{
			m_pLWeapon = CreateWeapon(m_abyLWeaponID[m_nCurGun], DTRUE);
		}
	}

	// Update the status bar stuff to reflect the new weapon
	m_NewStatusBar.UpdateCurrentWeap(m_nCurGun);

	if(m_pWeapon && (m_pWeapon->GetAmmoType(DTRUE) != m_pWeapon->GetAmmoType(DFALSE)))
		m_NewStatusBar.SetAltAmmo(m_dwAltAmmo);
	else
		m_NewStatusBar.SetAltAmmo(0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SpecialEffectNotify()
//
//	PURPOSE:	Handle creation of a special fx
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::SpecialEffectNotify(HLOCALOBJ hObj, HMESSAGEREAD hMessage)
{
	DDWORD dwClientFlags;
	CClientDE* pClientDE = GetClientDE();
	if(!pClientDE) return;

	if(hObj)
		pClientDE->SetObjectClientFlags(hObj, CF_NOTIFYREMOVE);

	DDWORD dwLocalID;
	pClientDE->GetLocalClientID(&dwLocalID);

	DBYTE nId = pClientDE->ReadFromMessageByte(hMessage);

	switch(nId)
	{
		case SFX_WEAPONHANDMODEL_ID:
		{
			DDWORD	cID = g_pClientDE->ReadFromMessageDWord(hMessage);
			DBYTE	byFlags = g_pClientDE->ReadFromMessageByte(hMessage);
			DBOOL	bLeft = byFlags >> 1;
			DBOOL	bVisible = byFlags & 0x01;

			if(cID != dwLocalID)
				break;

			DDWORD dwFlags = pClientDE->GetObjectFlags(hObj);

			if (bVisible)
			{
				pClientDE->SetObjectFlags(hObj, dwFlags | FLAG_VISIBLE);
				pClientDE->SetObjectClientFlags(hObj, 0);
			}
			else
				pClientDE->SetObjectFlags(hObj, dwFlags & ~FLAG_VISIBLE);

			if (bLeft)
				m_hLWeapHandObj	= (bVisible) ? NULL : hObj;
			else
				m_hWeapHandObj	= (bVisible) ? NULL : hObj;

			break;
		}

		case SFX_ORBCAM_ID:
		{
			DDWORD	cID = g_pClientDE->ReadFromMessageDWord(hMessage);
			DBYTE	bOn = g_pClientDE->ReadFromMessageByte(hMessage);

			if(cID != dwLocalID) break;

			if(m_hOrbObj == hObj)
			{
				if(!bOn)
				{
					m_hOrbObj = DNULL;
					break;
				}
			}
			else
			{
				if(bOn)
					m_hOrbObj = hObj;
			}

			break;
		}
		
		case SFX_DESTRUCTABLEMODEL:
		{
			DVector vDims;

			dwClientFlags = pClientDE->GetObjectClientFlags( hObj );
			dwClientFlags |= CF_DONTSETDIMS;
			pClientDE->SetObjectClientFlags( hObj, dwClientFlags );

			pClientDE->ReadFromMessageCompVector( hMessage, &vDims );

			pClientDE->Physics( )->SetObjectDims( hObj, &vDims, 0 );

		}
		break;

		default:
			m_sfxMgr.HandleSFXMsg(hObj, hMessage, nId);
			break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnObjectRemove()
//
//	PURPOSE:	Handle removal of a special fx
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::OnObjectRemove(HLOCALOBJ hObj)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	if (!hObj) return;

	if (hObj == m_hWeapHandObj)
		m_hWeapHandObj = DNULL;
	else if (hObj == m_hLWeapHandObj)
		m_hLWeapHandObj = DNULL;
	else if (hObj == m_hSeeingEyeObj)
		m_hSeeingEyeObj = DNULL;
	else if (hObj == m_hOrbObj)
		m_hOrbObj = DNULL;
	else if (m_pCreature)
	{
		if(hObj == m_pCreature->m_hEnemyObject || hObj == m_pCreature->m_hServerObject)
			m_pCreature->Detach();
	}
	else
		m_sfxMgr.RemoveSpecialFX(hObj);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::OnModelKey
//
//	PURPOSE:	Handle weapon model keys
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (m_pWeapon && m_pWeapon->GetWeapObj() == hObj)
		m_pWeapon->OnModelKey(hObj, pArgs);

	if (m_pLWeapon && m_pLWeapon->GetWeapObj() == hObj)
		m_pLWeapon->OnModelKey(hObj, pArgs);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::PauseGame()
//
//	PURPOSE:	Pauses/Unpauses the server
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::PauseGame (DBOOL bPause)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	m_bRenderCamera = !bPause;
	m_bPaused = bPause;

	if (!IsMultiplayerGame())
	{
		HMESSAGEWRITE hWrite = pClientDE->StartMessage(CMSG_GAME_PAUSE);
		pClientDE->WriteToMessageByte(hWrite, (DBYTE)bPause);
		pClientDE->EndMessage(hWrite);
	}

	pClientDE->SetInputState(!bPause);

	if (bPause)
		pClientDE->PauseMusic();
	else
		pClientDE->ResumeMusic();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateScreenShake
//
//	PURPOSE:	Update the screen shaking
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::UpdateScreenShake()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !m_hCamera) return;

	// Decay...
	DFLOAT fDecayAmount = 2.0f * pClientDE->GetFrameTime();

	m_vShakeMagnitude.x -= fDecayAmount;
	m_vShakeMagnitude.y -= fDecayAmount;
	m_vShakeMagnitude.z -= fDecayAmount;

	if (m_vShakeMagnitude.x < 0.0f) m_vShakeMagnitude.x = 0.0f;
	if (m_vShakeMagnitude.y < 0.0f) m_vShakeMagnitude.y = 0.0f;
	if (m_vShakeMagnitude.z < 0.0f) m_vShakeMagnitude.z = 0.0f;


	if (m_vShakeMagnitude.x <= 0.0f && m_vShakeMagnitude.y <= 0.0f && m_vShakeMagnitude.z <= 0.0f)
	{
		m_bShakeScreen = DFALSE;
		return;
	}


	// Apply...

	DFLOAT faddX = GetRandom(-1.0f, 1.0f) * m_vShakeMagnitude.x * 3.0f;
	DFLOAT faddY = GetRandom(-1.0f, 1.0f) * m_vShakeMagnitude.y * 3.0f;
	DFLOAT faddZ = GetRandom(-1.0f, 1.0f) * m_vShakeMagnitude.z * 3.0f;
	m_fShakeCant = DEG2RAD(GetRandom(-0.5f, 0.5f) * m_vShakeMagnitude.x);

	DVector vPos, vAdd;
	VEC_SET(vAdd, faddX, faddY, faddZ);

	pClientDE->GetObjectPos(m_hCamera, &vPos);
	VEC_ADD(vPos, vPos, vAdd);

	pClientDE->SetObjectPos(m_hCamera, &vPos);

	// Shake the guns too..
	if (m_pWeapon && m_pWeapon->GetWeapObj())
	{
		pClientDE->GetObjectPos(m_pWeapon->GetWeapObj(), &vPos);
		VEC_MULSCALAR(vAdd, vAdd, 0.95f);
		VEC_ADD(vPos, vPos, vAdd);
		pClientDE->SetObjectPos(m_pWeapon->GetWeapObj(), &vPos);
	}
	if (m_pLWeapon && m_pLWeapon->GetWeapObj())
	{
		pClientDE->GetObjectPos(m_pLWeapon->GetWeapObj(), &vPos);
		VEC_MULSCALAR(vAdd, vAdd, 0.95f);
		VEC_ADD(vPos, vPos, vAdd);
		pClientDE->SetObjectPos(m_pLWeapon->GetWeapObj(), &vPos);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateScreenFlash
//
//	PURPOSE:	Update the screen Flash
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::UpdateScreenFlash()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !m_bFlashScreen) return;

	DVector vLightAdd;
	VEC_INIT(vLightAdd);

	DFLOAT fTime  = pClientDE->GetTime();

	// ramp up
	if ((m_fFlashRampUp > 0.0f) && (fTime < m_fFlashStart + m_fFlashRampUp))
	{
		DFLOAT fDelta = (fTime - m_fFlashStart);
		vLightAdd.x = fDelta * (m_vFlashColor.x) / m_fFlashRampUp;
		vLightAdd.y = fDelta * (m_vFlashColor.y) / m_fFlashRampUp;
		vLightAdd.z = fDelta * (m_vFlashColor.z) / m_fFlashRampUp;
	}
	// full tint
	else if (fTime < m_fFlashStart + m_fFlashRampUp + m_fFlashTime)
	{
		VEC_COPY(vLightAdd, m_vFlashColor);
	}
	// ramp down
	else if (VEC_MAGSQR(m_vFlashColor) > 0)
	{
		DVector vSub;
		DFLOAT fScale = pClientDE->GetFrameTime() * 2;
		VEC_SET(vSub, fScale, fScale, fScale);
		VEC_SUB(m_vFlashColor, m_vFlashColor, vSub);
		VEC_CLAMP(m_vFlashColor, 0, 2.0f);

		VEC_COPY(vLightAdd, m_vFlashColor);

		// Add it to any additional camera light we're adding..
	}
	else
	{
		m_bFlashScreen = DFALSE;
	}

	VEC_CLAMP(vLightAdd, 0, 1.0f);

	VEC_ADD(m_vCameraAdd, m_vCameraAdd, vLightAdd);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateContainerFX
//
//	PURPOSE:	Update any client side container fx
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::UpdateContainerFX()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !m_hCamera) return;

	DVector vPos;
	pClientDE->GetObjectPos(m_hCamera, &vPos);

	HLOCALOBJ objList[1];
	DDWORD dwNum = pClientDE->GetPointContainers(&vPos, objList, 1);

	char* pCurSound		= DNULL;
	DBOOL bUpdateScale	= DFALSE;
	ContainerCode eCode = CC_NOTHING;
	DDWORD dwUserFlags	= FLAG_VISIBLE;
	DBOOL bUseWorldFog	= DTRUE;

	// Get the user flags associated with the container, and make sure that
	// the container isn't hidden...

	if (dwNum > 0 && objList[0])
	{
		pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);
	}


	if (dwNum > 0 && (dwUserFlags & FLAG_VISIBLE))
	{
		D_WORD code;
		if (pClientDE->GetContainerCode(objList[0], &code))
		{
			eCode = (ContainerCode)code;

			// See if we have entered/left a container...

			DFLOAT fTime = pClientDE->GetTime();

			if (m_eCurContainerCode != eCode)
			{
				m_fContainerStartTime = fTime;

				// See if this container has fog associated with it..

				CVolumeBrushFX* pFX = (CVolumeBrushFX*)m_sfxMgr.FindStaticSpecialFX(objList[0]);
				if (pFX)
				{
					bUseWorldFog = DFALSE;
					DBOOL bFog	 = pFX->IsFogEnable();

					char buf[30];
					sprintf(buf, "FogEnable %d", (int)bFog);
					pClientDE->RunConsoleString(buf);

					if (bFog)
					{
						sprintf(buf, "FogNearZ %d", (int)pFX->GetFogNearZ());
						pClientDE->RunConsoleString(buf);

						sprintf(buf, "FogFarZ %d", (int)pFX->GetFogFarZ());
						pClientDE->RunConsoleString(buf);

						DVector vFogColor = pFX->GetFogColor();

						sprintf(buf, "FogR %d", (int)vFogColor.x);
						pClientDE->RunConsoleString(buf);

						sprintf(buf, "FogG %d", (int)vFogColor.y);
						pClientDE->RunConsoleString(buf);

						sprintf(buf, "FogB %d", (int)vFogColor.z);
						pClientDE->RunConsoleString(buf);
					}
				}				
			}

			char *pWaterSound = "Sounds\\Player\\Underwaterloop1.wav";
			switch (eCode)
			{
				case CC_WATER:
				{
					VEC_SET(m_vLightScale, 0.3f, 0.8f, 0.8f);
					pCurSound = pWaterSound;
				}
				break;

				case CC_BLOOD:
				{
					VEC_SET(m_vLightScale, 1.0f, 0.0f, 0.0f);
					pCurSound = pWaterSound;
				}
				break;
				
				case CC_ACID:
				{
					VEC_SET(m_vLightScale, 0.0f, 0.8f, 0.4f);
					pCurSound = pWaterSound;
				}
				break;

				case CC_FREEFALL:
				{
					DFLOAT fFallTime = 1.5f;
					DFLOAT fScale = 0.0f;

					if (fTime < m_fContainerStartTime + fFallTime)
					{
						DFLOAT fTimeLeft = (m_fContainerStartTime + fFallTime) - fTime;
						fScale = fTimeLeft/fFallTime;
					}
					VEC_SET(m_vLightScale, fScale, fScale, fScale);
				}
				break;

				default : break;
			}

		}
	}


	// See if we have entered/left a container...

	if (m_eCurContainerCode != eCode || bUpdateScale)
	{
		// Adjust Fog as necessary...
		
		if (bUseWorldFog)
		{
			ResetGlobalFog();
		}

		// See if we are coming out of water...

		if (IsLiquid(m_eCurContainerCode) && !IsLiquid(eCode))
		{
			UpdateUnderWaterFX(DFALSE);
		}

		m_eCurContainerCode = eCode;

		if (m_hContainerSound)
		{
			pClientDE->KillSound(m_hContainerSound);
			m_hContainerSound = DNULL;
		}

		if (pCurSound)
		{
			PlaySoundInfo playSoundInfo;
			PLAYSOUNDINFO_INIT(playSoundInfo);

			playSoundInfo.m_dwFlags = PLAYSOUND_CLIENT | PLAYSOUND_LOCAL | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
			_mbscpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)pCurSound);
			playSoundInfo.m_nVolume = 20;
			if( pClientDE->PlaySound(&playSoundInfo) == LT_OK )
				m_hContainerSound = playSoundInfo.m_hSound;
		}

//		pClientDE->SetCameraLightAdd(m_hCamera, &vLightAdd);
	}


	// See if we are under water (under any liquid)...

	if (IsLiquid(m_eCurContainerCode)) 
	{
		UpdateUnderWaterFX();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateUnderWaterFX
//
//	PURPOSE:	Update under water fx
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::UpdateUnderWaterFX(DBOOL bUpdate)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !m_hCamera || m_bZoomView) return;

//	DDWORD dwWidth = 640, dwHeight = 480;
//	pClientDE->GetSurfaceDims(pClientDE->GetScreenSurface(), &dwWidth, &dwHeight);

	if (m_dwScreenWidth < 0 || m_dwScreenHeight < 0) return;

	// Initialize to default fov x and y...

	DFLOAT fFovX = m_fovX;
	DFLOAT fFovY = fFovX * m_fovYScale;
	
	if (bUpdate)
	{
		pClientDE->GetCameraFOV(m_hCamera, &fFovX, &fFovY);

		DFLOAT fSpeed = .02f * pClientDE->GetFrameTime();

		if (m_fFovXFXDir > 0)
		{
			fFovY += fSpeed;
			fFovX -= fSpeed;

			if (fFovY > m_fovX * m_fovYScale)
			{
				fFovY = m_fovX * m_fovYScale;
				m_fFovXFXDir = -m_fFovXFXDir;
			}
		}
		else
		{
			fFovY -= fSpeed;
			fFovX += fSpeed;

			if (fFovY < (m_fovX * m_dwScreenHeight - 40) / m_dwScreenWidth)
			{
				fFovY = (m_fovX * m_dwScreenHeight - 40) / m_dwScreenWidth;
				m_fFovXFXDir = -m_fFovXFXDir;
			}
		}
	}

	pClientDE->SetCameraFOV(m_hCamera, fFovX, fFovY);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateWonkyVision
//
//	PURPOSE:	Update WonkyVision (baddest LCD trip in da mofo)
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::UpdateWonkyVision(DBOOL bUpdate)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !m_hCamera || m_bZoomView) return;

	if (m_dwScreenWidth < 0 || m_dwScreenHeight < 0) return;

	DFLOAT fovX = m_fovX;
	DFLOAT fovY = m_fovY;

	DVector vU, vF, vR;
	pClientDE->GetRotationVectors(&m_Rotation, &vU, &vR, &vF);

	pClientDE->RunConsoleString("ModelWarble 1");
	pClientDE->RunConsoleString("WarbleSpeed 15");
	pClientDE->RunConsoleString("WarbleScale .90");

	DFLOAT fSpeed = 0.2f * pClientDE->GetFrameTime();
	fSpeed = DCLAMP( fSpeed, 0.0f, 0.010f );

	// FOV
	if (m_fFovXFXDir > 0)
	{
		m_fOffsetY += fSpeed;
		m_fOffsetX -= fSpeed;

		fovY += m_fOffsetY;
		fovX += m_fOffsetX;

		if (fovY > fovX * m_fovYScale)
		{
			fovY = fovX * m_fovYScale;
			m_fFovXFXDir = -m_fFovXFXDir;
		}
	}
	else
	{
		m_fOffsetY -= fSpeed;
		m_fOffsetX += fSpeed;

		fovY += m_fOffsetY;
		fovX += m_fOffsetX;

		if (fovY < (fovX * m_dwScreenHeight - 120) / m_dwScreenWidth)
		{
			fovY = (fovX * m_dwScreenHeight - 120) / m_dwScreenWidth;
			m_fFovXFXDir = -m_fFovXFXDir;
		}
	}

	// ROTATION
	if(m_fRotDir > 0)
	{
		m_fOffsetRot += fSpeed * 1.5f;

		if(m_fOffsetRot >= m_fMaxRot)
		{
			m_fRotDir = -m_fRotDir;
			m_fMaxRot = -0.55f * GetRandom(1.0f, 2.0f);
		}
	}
	else
	{
		m_fOffsetRot -= fSpeed * 1.5f;

		if(m_fOffsetRot <= m_fMaxRot)
		{
			m_fRotDir = -m_fRotDir;
			m_fMaxRot = 0.55f * GetRandom(1.0f, 2.0f);
		}
	}

	if (fovX < 1.3f) fovX = 1.3f;
	if (fovX > 1.8f) fovX = 1.8f;

	if (fovY < 1.0f) fovY = 1.0f;
	if (fovY > 1.3f) fovY = 1.3f;

	pClientDE->SetCameraFOV(m_hCamera, fovX, fovY);
	pClientDE->RotateAroundAxis(&m_Rotation, &vF, m_fOffsetRot);
	pClientDE->SetObjectRotation(m_hCamera, &m_Rotation);

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::ShakeScreen()
//
//	PURPOSE:	Shake screen
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::ShakeScreen(DVector *vShake, DFLOAT fTime)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	m_fShakeStart		= GetClientDE()->GetTime();
	m_fShakeTime		= fTime;

	VEC_ADD(m_vShakeMagnitude, m_vShakeMagnitude, *vShake);

	if (m_vShakeMagnitude.x > 3.5f) m_vShakeMagnitude.x = 3.5f;
	if (m_vShakeMagnitude.y > 3.5f) m_vShakeMagnitude.y = 3.5f;
	if (m_vShakeMagnitude.z > 3.5f) m_vShakeMagnitude.z = 3.5f;

	m_bShakeScreen = DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::FlashScreen()
//
//	PURPOSE:	Flash screen
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::FlashScreen(DVector *vFlashColor, DFLOAT fTime, DFLOAT fRampUp)
{
	if (!GetClientDE()) return;

	m_fFlashStart		= GetClientDE()->GetTime();
	m_fFlashTime		= fTime;

	m_vFlashColor.x += vFlashColor->x;
	m_vFlashColor.y += vFlashColor->y;
	m_vFlashColor.z += vFlashColor->z;

	// Scale lowest value to max of 1.0 (so we don't oversaturate)
	DFLOAT fMin = m_vFlashColor.x;
	fMin = (m_vFlashColor.y < fMin) ? m_vFlashColor.y : fMin;
	fMin = (m_vFlashColor.z < fMin) ? m_vFlashColor.z : fMin;

	if (fMin > 1.0f)
	{
		fMin = 1.0f/fMin;
		m_vFlashColor.x *= fMin;
		m_vFlashColor.y *= fMin;
		m_vFlashColor.z *= fMin;
	}


	// if screen is already flashing, don't ramp up..
	if (!m_bFlashScreen)
		m_fFlashRampUp	= fRampUp;
	else
		m_fFlashRampUp = 0.0f;

	m_bFlashScreen	= DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::HandleExitWorld()
//
//	PURPOSE:	The player has hit an exit trigger.
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::HandleExitWorld(HMESSAGEREAD hMessage)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	// don't go to the next level if you are dead
	if (m_bDead)
		return;

	// Stop the music
	m_Music.TermPlayLists();
	pClientDE->StopMusic (MUSIC_IMMEDIATE);	// Stop the music
	m_eMusicLevel = CMusic::MUSICLEVEL_SILENCE;

	// kill the sounds
	if (m_hContainerSound)
	{
		pClientDE->KillSound(m_hContainerSound);
		m_hContainerSound = DNULL;
	}

	if( m_hNightGogglesSound )
	{
		g_pClientDE->KillSound( m_hNightGogglesSound );
	}

	if( m_hTheEyeLoopingSound )
	{
		g_pClientDE->KillSound( m_hTheEyeLoopingSound );
	}


	// Get the new world
	HSTRING hstrNextWorld = pClientDE->ReadFromMessageHString(hMessage);
	m_nLastExitType = pClientDE->ReadFromMessageByte(hMessage);

	if (!hstrNextWorld)
	{
		PrintError("Invalid data in CBloodClientShell::HandleExitWorld()!");
		return;
	}

#ifdef _DEMO
	if (_mbsicmp((const unsigned char*)"EndGame", (const unsigned char*)pClientDE->GetStringData(hstrNextWorld)) == 0)
	{
		m_bInWorld = DFALSE;
		PauseGame(DTRUE);
		m_Menu.SetCurrentMenu(MENU_ID_MAINMENU, MENU_ID_MAINMENU);
		PlaySoundLocal("sounds\\interface\\mainmenus\\enter01.wav", SOUNDPRIORITY_MISC_HIGH, DFALSE, DFALSE, DFALSE, DFALSE, 100);
		Splash_SetState(pClientDE, "\\screens\\info.pcx");
		return;
	}
#endif

	_mbscpy((unsigned char*)m_szFilename, (const unsigned char*)pClientDE->GetStringData(hstrNextWorld));
	StartNewWorld(m_szFilename, m_nGameType, LOADTYPE_NEW_LEVEL);
	pClientDE->FreeString(hstrNextWorld);
	m_bFirstWorld = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::StartNewWorld()
//
//	PURPOSE:	Start the new world
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::StartNewWorld(char *pszWorld, DBYTE nGameType, DBYTE nLoadType, DBYTE nDifficulty)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return DFALSE;

	if (pszWorld)
	{
		// Set up the world title string
		if (m_hstrTitle) { pClientDE->FreeString(m_hstrTitle); m_hstrTitle = 0; }
		if (m_hstrLoadScreen) { pClientDE->FreeString(m_hstrLoadScreen); m_hstrLoadScreen = 0; }
		if (m_nLoadScreenID) m_nLoadScreenID = 0;

		GetWorldTitle(pszWorld);

		if(m_nLoadScreenID)		PauseGame(DTRUE);
			else				PauseGame(DFALSE);

		// Clear the current objective...
		m_NewStatusBar.InitObjectivesTab();

		// Send a message to the server to load a new world
		HMESSAGEWRITE hMsg;
		hMsg = pClientDE->StartMessage(CMSG_LOADWORLD);
		pClientDE->WriteToMessageString(hMsg, pszWorld);		// Filename
		pClientDE->WriteToMessageByte(hMsg, nGameType);			// Game type
		pClientDE->WriteToMessageByte(hMsg, nLoadType);			// Load type
		pClientDE->WriteToMessageByte(hMsg, nDifficulty);		// Difficulty level
		pClientDE->EndMessage(hMsg);

		SetGameState(GS_LOADINGLEVEL);

		m_nGameType = nGameType;
		m_nLastLoadType = nLoadType;

		m_bInWorld = DFALSE;

		m_bZoomView = DFALSE;
		m_bShowCrosshair = m_bCrosshairOriginallyOn;
		m_nCrosshair = m_nCrosshairOriginalNum;

		m_bBlind = DFALSE;
		m_nTrapped = 0;
	}
	else
	{
		PrintError("Error invalid world!");
		return DFALSE;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SaveClientData
//
//	PURPOSE:	Send client save data to server so it can be put into save file.
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::SaveClientData( HMESSAGEWRITE hMsg )
{
	HMESSAGEWRITE hSaveMsg;

	hSaveMsg = g_pClientDE->StartHMessageWrite( );

	m_eMusicLevel = m_Music.GetMusicLevel();

	// Start save data...

	g_pClientDE->WriteToMessageByte( hSaveMsg, m_bNightGogglesActive );
	g_pClientDE->WriteToMessageFloat( hSaveMsg, m_fWonkyTime );
	g_pClientDE->WriteToMessageByte( hSaveMsg, m_bWonkyVision );
	g_pClientDE->WriteToMessageByte( hSaveMsg, m_bWonkyNoMove );
	g_pClientDE->WriteToMessageFloat( hSaveMsg, m_fBlindTime );
	g_pClientDE->WriteToMessageByte( hSaveMsg, m_bBlind );
	g_pClientDE->WriteToMessageByte( hSaveMsg, m_bFadeIn );
	g_pClientDE->WriteToMessageByte( hSaveMsg, m_bFadeOut );
	g_pClientDE->WriteToMessageFloat( hSaveMsg, m_fFadeVal );
//	g_pClientDE->WriteToMessageByte( hSaveMsg, m_nCrosshair );
	g_pClientDE->WriteToMessageByte( hSaveMsg, m_eMusicLevel );
	g_pClientDE->WriteToMessageFloat( hSaveMsg, (float)m_nLastLoadType );
	g_pClientDE->WriteToMessageFloat( hSaveMsg, (float)m_nGameType );
	g_pClientDE->WriteToMessageHString( hSaveMsg, m_hstrObjectivesTitle );
	g_pClientDE->WriteToMessageHString( hSaveMsg, m_hstrObjectivesText );

	// End save data...

	g_pClientDE->WriteToMessageHMessageWrite( hMsg, hSaveMsg );
	g_pClientDE->EndHMessageWrite( hSaveMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::LoadClientData
//
//	PURPOSE:	Send client save data to server so it can be put into save file.
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::LoadClientData( HMESSAGEREAD hMsg )
{
	HMESSAGEREAD hLoadMsg;
	DBOOL bNightVisionActive;

	if (m_hstrObjectivesTitle)
	{
		g_pClientDE->FreeString(m_hstrObjectivesTitle);
		m_hstrObjectivesTitle = DNULL;
	}
	if (m_hstrObjectivesText)
	{
		g_pClientDE->FreeString(m_hstrObjectivesText);
		m_hstrObjectivesText = DNULL;
	}

	hLoadMsg = g_pClientDE->ReadFromMessageHMessageRead( hMsg );
	
	bNightVisionActive = g_pClientDE->ReadFromMessageByte( hLoadMsg );
	m_fWonkyTime	= g_pClientDE->ReadFromMessageFloat( hLoadMsg );
	m_bWonkyVision	= g_pClientDE->ReadFromMessageByte( hLoadMsg );
	m_bWonkyNoMove	= g_pClientDE->ReadFromMessageByte( hLoadMsg );
	m_fBlindTime	= g_pClientDE->ReadFromMessageFloat( hLoadMsg );
	m_bBlind		= g_pClientDE->ReadFromMessageByte( hLoadMsg );
	m_bFadeIn		= g_pClientDE->ReadFromMessageByte( hLoadMsg );
	m_bFadeOut		= g_pClientDE->ReadFromMessageByte( hLoadMsg );
	m_fFadeVal		= g_pClientDE->ReadFromMessageFloat( hLoadMsg );
//	m_nCrosshair	= g_pClientDE->ReadFromMessageByte( hLoadMsg );
	m_eMusicLevel	= (CMusic::EMusicLevel) g_pClientDE->ReadFromMessageByte( hLoadMsg );
	int nTmp		= (int)g_pClientDE->ReadFromMessageFloat( hLoadMsg );
	m_nGameType		= (int)g_pClientDE->ReadFromMessageFloat( hLoadMsg );
	m_hstrObjectivesTitle = g_pClientDE->ReadFromMessageHString( hLoadMsg );
	m_hstrObjectivesText = g_pClientDE->ReadFromMessageHString( hLoadMsg );

	if (m_hstrObjectivesTitle && m_hstrObjectivesText)
		m_NewStatusBar.UpdateObjectives(g_pClientDE->GetStringData(m_hstrObjectivesTitle), g_pClientDE->GetStringData(m_hstrObjectivesText));

	g_pClientDE->EndHMessageRead( hLoadMsg );

	g_pClientDE->RunConsoleString("maxfps 0");
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::ShowPlayer()
//
//	PURPOSE:	Show or hide the player object
//
// --------------------------------------------------------------------------- //

HLOCALOBJ m_hLastWeapHandObj = DNULL;
HLOCALOBJ m_hLastLWeapHandObj = DNULL;

void CBloodClientShell::ShowPlayer(DBOOL bShow)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	HLOCALOBJ hPlayerObj = pClientDE->GetClientObject();
	if (!hPlayerObj) return;

	DDWORD dwFlags = pClientDE->GetObjectFlags(hPlayerObj);
	if (bShow)
	{
		dwFlags |= FLAG_VISIBLE;
		cdata.byFlags &= ~CDATA_SHOWGUN;
	}
	else
	{
		dwFlags &= ~FLAG_VISIBLE;
		cdata.byFlags |= CDATA_SHOWGUN;
	}
	pClientDE->SetObjectFlags(hPlayerObj, dwFlags);

	if((m_hLastWeapHandObj != m_hWeapHandObj) || (m_hLastLWeapHandObj != m_hLWeapHandObj))
	{
		m_hLastWeapHandObj = m_hWeapHandObj;
		m_hLastLWeapHandObj = m_hLWeapHandObj;
	}

	if (m_hWeapHandObj)
	{
		DDWORD dwFlags = pClientDE->GetObjectFlags(m_hWeapHandObj);

		if(bShow)	dwFlags |= FLAG_VISIBLE;
			else	dwFlags &= ~FLAG_VISIBLE;

		pClientDE->SetObjectFlags(m_hWeapHandObj, dwFlags);
	}

	if (m_hLWeapHandObj)
	{
		DDWORD dwFlags = pClientDE->GetObjectFlags(m_hLWeapHandObj);

		if (bShow)	dwFlags |= FLAG_VISIBLE;
			else	dwFlags &= ~FLAG_VISIBLE;

		pClientDE->SetObjectFlags(m_hLWeapHandObj, dwFlags);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::DrawLoadingScreen()
//
//	PURPOSE:	Update the loading screen
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::DrawLoadingScreen()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	HSURFACE hScreen = pClientDE->GetScreenSurface();
	if (!hScreen) return;

	HSURFACE hTemp = DNULL;
	DDWORD	 screenWidth, screenHeight;

	pClientDE->GetSurfaceDims(hScreen, &screenWidth, &screenHeight);

	// If we don't have a loading screen, create one
	if(!m_hLoadingScreen)
	{
		// Create the loading screen surface the same size as the screen
		m_hLoadingScreen = pClientDE->CreateSurface(screenWidth, screenHeight);

		if(m_hstrLoadScreen)
			hTemp = pClientDE->CreateSurfaceFromBitmap(pClientDE->GetStringData(m_hstrLoadScreen));
		else
		{
			hTemp = pClientDE->CreateSurfaceFromBitmap("Interface\\LoadScreens\\LoadScreen.pcx");
			pClientDE->DrawBitmapToSurface(hTemp, "Interface\\LoadScreens\\LoadingText\\Loading_01.pcx", DNULL, 215, 321);
		}

		// Scale the bitmap to the loading screen and dump the temp surface
		pClientDE->ScaleSurfaceToSurface(m_hLoadingScreen, hTemp, DNULL, DNULL);
		pClientDE->DeleteSurface(hTemp);
		hTemp = DNULL;
	}

	// Init the fonts and cursor
	CoolFontCursor	*m_hLoadCursor = 0;
	CoolFont		*m_hLoadTitleFont = 0;
	CoolFont		*m_hLoadTextFont = 0;

	m_hLoadCursor = new CoolFontCursor();
	m_hLoadTitleFont = new CoolFont();
	m_hLoadTextFont = new CoolFont();

	m_hLoadTitleFont->Init(pClientDE, "interface/statusbar/fonts/stat_font_1.pcx");
	m_hLoadTitleFont->LoadXWidths("interface/statusbar/fonts/stat_font_1.fnt");

	m_hLoadTextFont->Init(pClientDE, "interface/objectives/obj_font_1.pcx");
	m_hLoadTextFont->LoadXWidths("interface/objectives/obj_font_1.fnt");

#ifdef _DEMO

	// Draw the title of the level
	m_hLoadCursor->SetFont(m_hLoadTitleFont);
	m_hLoadCursor->SetDest(m_hLoadingScreen);
	m_hLoadCursor->SetJustify(CF_JUSTIFY_CENTER);
	m_hLoadCursor->SetLoc((short)(screenWidth * 0.5f), (short)(m_hLoadTitleFont->height * 2));

	if(m_hstrTitle)
	{
		m_hLoadCursor->Draw(pClientDE->GetStringData(m_hstrTitle));
		m_hLoadCursor->NewLine();
	}

#else

	if(!m_nLoadScreenID)
	{
		// Draw the title of the level
		m_hLoadCursor->SetFont(m_hLoadTitleFont);
		m_hLoadCursor->SetDest(m_hLoadingScreen);
		m_hLoadCursor->SetJustify(CF_JUSTIFY_CENTER);
		m_hLoadCursor->SetLoc((short)(screenWidth * 0.5f), (short)(m_hLoadTitleFont->height * 2));

		if(m_hstrTitle)
		{
			m_hLoadCursor->Draw(pClientDE->GetStringData(m_hstrTitle));
			m_hLoadCursor->NewLine();
		}

		// Draw the loading screen...
		pClientDE->DrawSurfaceToSurface(hScreen, m_hLoadingScreen, DNULL, 0, 0);
	}
	else
	{
		HSTRING			hStr;
		LoadScreenData	*lsd = &(g_LoadScreenData[m_nLoadScreenID]);
		DFLOAT			ratioX = (float)screenWidth / 640.0f;
		DFLOAT			ratioY = (float)screenHeight / 480.0f;
		short			newX, newY;

		m_hLoadCursor->SetFont(m_hLoadTitleFont);
		m_hLoadCursor->SetDest(m_hLoadingScreen);
		m_hLoadCursor->SetJustify(CF_JUSTIFY_CENTER);

		// Find the scaled location for the title and draw it center justified
		newX = (short)(lsd->nTitleX * ratioX);
		newY = (short)(lsd->nTitleY * ratioY);

		// Draw the title of the level
		m_hLoadCursor->SetLoc(newX, (short)(m_hLoadTitleFont->height * 2));

		if(m_hstrTitle)
			m_hLoadCursor->Draw(pClientDE->GetStringData(m_hstrTitle));

		m_hLoadCursor->SetLoc(newX, (short)(m_hLoadTitleFont->height * 3));
		hStr = pClientDE->FormatString(lsd->nTitle);
		m_hLoadCursor->Draw(pClientDE->GetStringData(hStr));
		pClientDE->FreeString(hStr);

		if(m_nGameType != GAMETYPE_ACTION && screenWidth >= 512 && screenHeight >= 384)
		{
			// Find the scaled location for the text and draw it in formatted mode
			newX = (short)(lsd->nTextX * ratioX);
			newY = (short)(lsd->nTextY * ratioY);

			m_hLoadCursor->SetFont(m_hLoadTextFont);
			m_hLoadCursor->SetLoc(newX, (short)(m_hLoadTitleFont->height * 5));
			hStr = pClientDE->FormatString(lsd->nText);
			m_hLoadCursor->DrawFormat(pClientDE->GetStringData(hStr), (short)(lsd->nTextWrapWidth * ratioX));
			pClientDE->FreeString(hStr);
		}

		// Draw the loading screen...
		pClientDE->DrawSurfaceToSurface(hScreen, m_hLoadingScreen, DNULL, 0, 0);

		// Draw a loading message at the bottom of the screen
		m_hLoadCursor->SetDest(hScreen);
		m_hLoadCursor->SetFont(m_hLoadTitleFont);
		m_hLoadCursor->SetLoc((short)(screenWidth * 0.5f), (short)(screenHeight - m_hLoadTitleFont->height));

		if(GetGameState() == GS_WAITING)
		{
			DFLOAT		r, g, b, time = pClientDE->GetTime();
			DFLOAT		calc;

			if(!m_bPlayedWaitingSound)
			{
				PlaySoundLocal( "sounds\\interface\\mainmenus\\esc01.wav", SOUNDPRIORITY_MISC_HIGH);
				m_bPlayedWaitingSound = DTRUE;
			}

			if(time - m_fLoadingFadeTime >= 1.0f)
			{
				m_fLoadingFadeTime = time;
				m_bLoadingFadeUp = !m_bLoadingFadeUp;
			}

			calc = (time - m_fLoadingFadeTime) / 1.0f;

			if(m_bLoadingFadeUp)
			{
				r = 0.776f + (0.224f * calc);
				g = 0.647f - (0.647f * calc);
				b = 0.518f - (0.518f * calc);
			}
			else
			{
				r = 1.000f - (0.224f * calc);
				g = 0.647f * calc;
				b = 0.518f * calc;
			}

			m_hLoadCursor->DrawSolid("Press a key to continue...", pClientDE->SetupColor2(r, g, b, DFALSE));
		}
		else
		{
			hStr = pClientDE->FormatString(g_LoadScreenMessages[GetRandom(0,MAX_LOADING_MESSAGES-1)]);
			m_hLoadCursor->Draw(pClientDE->GetStringData(hStr));
			pClientDE->FreeString(hStr);
		}
	}
#endif

	// Free the fonts and cursor
	if(m_hLoadCursor)		{ delete m_hLoadCursor; m_hLoadCursor = 0; }
	if(m_hLoadTitleFont)	{ m_hLoadTitleFont->Free(); delete m_hLoadTitleFont; m_hLoadTitleFont = 0; }
	if(m_hLoadTextFont)		{ m_hLoadTextFont->Free(); delete m_hLoadTextFont; m_hLoadTextFont = 0; }
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::Set3rdPersonCamera()
//
//	PURPOSE:	Turn on/off 3rd person camera mode
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::Set3rdPersonCamera(DBOOL bExternal)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !m_hCamera) return;

	if (bExternal && m_playerCamera.IsFirstPerson())
	{
//		m_weaponModel.SetVisible(DFALSE);

		m_b3rdPerson = DTRUE;

		m_playerCamera.GoChaseMode();

		m_bZoomView = DFALSE;			 // Can't zoom in 3rd person...
	}
	else if (!bExternal && !m_playerCamera.IsFirstPerson()) // Go Internal
	{
//		m_weaponModel.SetVisible(DTRUE);

		m_b3rdPerson = DFALSE;
		m_playerCamera.GoFirstPerson();
	}
	ShowPlayer(m_b3rdPerson ? DTRUE : DFALSE);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateCameraPosition()
//
//	PURPOSE:	Update the camera position
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::UpdateCameraPosition()
{
	DBOOL bCameraIsListener;
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	HLOCALOBJ hPlayerObj = m_pMoveMgr->GetObject();
//	HLOCALOBJ hPlayerObj = pClientDE->GetClientObject();
	if (!hPlayerObj || !m_hCamera) return;

	DVector vPos;
	pClientDE->GetObjectPos(hPlayerObj, &vPos);

	// External camera
	if (m_pExternalCamera || m_hSeeingEyeObj || m_hOrbObj)
	{
		DRotation rRot;

		HLOCALOBJ hCameraObj = DNULL;

		bCameraIsListener = DTRUE;

		// Determine the object to use
		if (m_hSeeingEyeObj) 
			hCameraObj = m_hSeeingEyeObj;
		else if (m_hOrbObj) 
			hCameraObj = m_hOrbObj;
		else if (m_pExternalCamera) 
		{
			hCameraObj = m_pExternalCamera->GetServerObj();
			bCameraIsListener = m_pExternalCamera->IsListener( );
		}
		
		if (hCameraObj)
		{
			// Get position
			pClientDE->GetObjectPos(hCameraObj, &vPos);
			pClientDE->SetObjectPos(m_hCamera, &vPos);

			if (!m_hOrbObj && !m_hSeeingEyeObj)
			{
				// Get Rotation
				pClientDE->GetObjectRotation(hCameraObj, &rRot);
				pClientDE->SetObjectRotation(m_hCamera, &rRot);
			}                
			else
			{
				ROT_INIT( rRot );
			}

			if( bCameraIsListener )
			{
				// Set the listener for sounds
				pClientDE->SetListener( DFALSE, &vPos, &rRot );
			}
			else
				pClientDE->SetListener( DTRUE, DNULL, DNULL );
		}
	}
	else
	{
		// Player camera - first person
		if (m_playerCamera.IsFirstPerson())	
		{
			DVector up;
			DVector forward;
			DVector right;

			if (cdata.byFlags & CDATA_CANMOVE)
			{
				UpdateBob();
				UpdateHeadCant();
			}

			pClientDE->GetRotationVectors(&m_Rotation, &up, &right, &forward);

			VEC_MULSCALAR(right, right, m_fBobWidth)
			VEC_ADD(vPos, vPos, right)

			// HACK - remove eventually
	//#ifdef _DEBUG
	//		VEC_MULSCALAR(forward, forward, m_fCameraZoom);
	//		VEC_SUB(pos, pos, forward);
	//#endif

			vPos.y += m_fViewY + m_fBobHeight;

			pClientDE->SetObjectPos(m_hCamera, &vPos);

	//#ifdef _DEBUG
			// HACK - remove eventually
	//		VEC_ADD(pos, pos, forward);
	//#endif

			// Set the listener for sounds
			pClientDE->SetListener( DTRUE, DNULL, DNULL );
		}
		// Player camera - 3rd person
		else
		{
			VEC_COPY(vPos, m_playerCamera.GetPos());
			pClientDE->SetObjectPos(m_hCamera, &vPos);

			ROT_COPY(m_Rotation, m_playerCamera.GetRotation());

			// Set the listener for sounds
			pClientDE->SetListener( DFALSE, &vPos, &m_Rotation );
		}

		// Update the create that's on your face
		if(m_pCreature)
			m_pCreature->Update(cdata.fPitch, cdata.fYaw, &vPos);
	}

	UpdateGun(&m_Rotation, &vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::CalculateCameraRotation()
//
//	PURPOSE:	Calculate the new camera rotation
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::CalculateCameraRotation()
{
	DFLOAT offsets[3];
	DFLOAT rotateSpeed;
//	DRotation newRot;
	DFLOAT newPitch;
	DFLOAT newYaw;
	DFLOAT addPitch;
	DFLOAT addYaw;

	CClientDE* pClientDE = GetClientDE();

	newPitch = m_fPitch;
	newYaw = m_fYaw;
	addPitch = 0;
	addYaw = 0;

	rotateSpeed = 3 * pClientDE->GetFrameTime();
	pClientDE->GetAxisOffsets(offsets);

	HCONSOLEVAR hVar;
	float fAxisForwardBackward = 0.0f;
	float fAxisLeftRight = 0.0f;
	float fAxisYaw = 0.0f;
	float fAxisPitch = 0.0f;
	DBOOL bUseAxisForwardBackward = FALSE;
	DBOOL bUseAxisLeftRight = FALSE;
	DBOOL bFixedAxisPitch = FALSE;
	float fAxisYawDeadZone = 0.10f;
	float fAxisPitchDeadZone = 0.10f;
	float fAxisForwardBackwardDeadZone = 0.10f;
	float fAxisLeftRightDeadZone = 0.10f;

	if (m_bUseJoystick && !m_bAdvancedDisableJoystick)
	{
		hVar = pClientDE->GetConsoleVar( "AxisYawDeadZone");
		if (hVar != NULL) fAxisYawDeadZone = pClientDE->GetVarValueFloat(hVar);
		hVar = pClientDE->GetConsoleVar( "AxisPitchDeadZone");
		if (hVar != NULL) fAxisPitchDeadZone = pClientDE->GetVarValueFloat(hVar);
		hVar = pClientDE->GetConsoleVar( "AxisForwardBackwardDeadZone");
		if (hVar != NULL) fAxisForwardBackwardDeadZone = pClientDE->GetVarValueFloat(hVar);
		hVar = pClientDE->GetConsoleVar( "AxisLeftRightDeadZone");
		if (hVar != NULL) fAxisLeftRightDeadZone = pClientDE->GetVarValueFloat(hVar);

		hVar = pClientDE->GetConsoleVar( "AxisYaw");
		if (hVar != NULL)
		{
			fAxisYaw = pClientDE->GetVarValueFloat(hVar);
			if ((fAxisYaw > fAxisYawDeadZone) || ((fAxisYaw < -fAxisYawDeadZone))) offsets[0] += fAxisYaw;
		}
		hVar = pClientDE->GetConsoleVar( "AxisPitch");
		if (hVar != NULL) 
		{
			fAxisPitch = pClientDE->GetVarValueFloat(hVar);
			if ((fAxisPitch > fAxisPitchDeadZone) || (fAxisPitch < -fAxisPitchDeadZone)) offsets[1] += fAxisPitch;
		}
		hVar = pClientDE->GetConsoleVar( "AxisLeftRight");
		if (hVar != NULL) 
		{
			fAxisLeftRight = pClientDE->GetVarValueFloat(hVar);
			bUseAxisLeftRight = TRUE;
		}
		hVar = pClientDE->GetConsoleVar( "AxisForwardBackward");
		if (hVar != NULL)
		{
			fAxisForwardBackward = pClientDE->GetVarValueFloat(hVar);
			bUseAxisForwardBackward = TRUE;
		}

		hVar = pClientDE->GetConsoleVar( "FixedAxisPitch");
		if (hVar != NULL) if (pClientDE->GetVarValueFloat(hVar) == 1) 
		{
			newPitch = fAxisPitch;
			bFixedAxisPitch = TRUE;
//		pClientDE->CPrint("FixedAxisPitch==1 AxisPitch=%14.6f",(double)fAxisPitch); // BLB TEMP
		}
	}

	m_pMoveMgr->UpdateAxisMovement(bUseAxisForwardBackward, fAxisForwardBackward, fAxisForwardBackwardDeadZone, bUseAxisLeftRight, fAxisLeftRight, fAxisLeftRightDeadZone);

	cdata.MouseAxis0 = offsets[0];
	cdata.MouseAxis1 = offsets[1];

//	if (cdata.byFlags & CDATA_MOUSEAIMING)
	if (m_bMouseLook || pClientDE->IsCommandOn(COMMAND_MOUSEAIMTOGGLE))
	{
		DFLOAT addPitch = m_bZoomView ? (offsets[1] / 4.0f) : (offsets[1]);
		addPitch *= (IsMouseInvertYAxis() ? -1 : 1);
		newPitch += addPitch;
	}

	// Only allow input if not editing and movement is not blocked
	if (!m_MessageMgr.GetEditingState() && (cdata.byFlags & CDATA_CANMOVE || m_hOrbObj || m_hSeeingEyeObj))
	{
		// Rotate if not strafe state
		if (!pClientDE->IsCommandOn(COMMAND_STRAFE))
		{
			newYaw += (m_bZoomView) ? (offsets[0] / 4.0f) : offsets[0];

			if (pClientDE->IsCommandOn(COMMAND_LEFT))
				newYaw -= rotateSpeed * GetKeyboardTurnRate();
			else if (pClientDE->IsCommandOn(COMMAND_RIGHT))
				newYaw += rotateSpeed * GetKeyboardTurnRate();
		}

		// Turn around
		if (pClientDE->IsCommandOn(COMMAND_TURNAROUND) && m_SpinAmount == 0)
		{
			m_SpinAmount = - PI;
		}
		else if (m_SpinAmount != 0)
		{
			DFLOAT nSpin;

			nSpin = pClientDE->GetFrameTime() * 15;
			m_SpinAmount += nSpin;
			
			if (m_SpinAmount > 0)
			{
				nSpin -= m_SpinAmount;
				m_SpinAmount = 0;
			}

			newYaw += nSpin;
		}

		// Aim Up/Down
		if (pClientDE->IsCommandOn(COMMAND_AIMUP) && !m_bZeroXAngle)
			newPitch -= rotateSpeed;
		else if (pClientDE->IsCommandOn(COMMAND_AIMDOWN) && !m_bZeroXAngle)
			newPitch += rotateSpeed;

		// Look Up/Down
		if (pClientDE->IsCommandOn(COMMAND_LOOKUP))
			newPitch -= rotateSpeed;
		else if  (pClientDE->IsCommandOn(COMMAND_LOOKDOWN))
			newPitch += rotateSpeed;
	}

	// Zero X Angle (if we just let go of a look key)
	if (m_bZeroXAngle)
	{
		if (newPitch != 0)
		{
			DFLOAT nSpin;
			DFLOAT nNewX;

			if (newPitch <= 0)
				nSpin = rotateSpeed;
			else
				nSpin = -rotateSpeed;

			nNewX = newPitch + nSpin;

			if (newPitch > 0 && nNewX <= 0)
				newPitch = 0;
			else if (newPitch < 0 && nNewX >= 0)
				newPitch = 0;
			else
				newPitch = nNewX;

		}
		if (newPitch == 0)
			m_bZeroXAngle = DFALSE;
	}

	if (m_fViewKick)
	{
		addPitch -= DEG2RAD(m_fViewKick);
	}
	
	if (m_fKickTime)
	{
		m_fKickTime -= pClientDE->GetFrameTime();
		if (m_fKickTime <= 0)
		{
			m_fKickTime = 0.001f;
			m_fViewKick -= RAD2DEG(rotateSpeed);
			if (m_fViewKick <= 0)
			{
				m_fViewKick = 0;
				m_fKickTime = 0;
			}
		}
	}

	DFLOAT minY = (PI / 2) - 0.01f;

	if (newPitch < -minY) 
		newPitch = -minY;
	if (newPitch > minY) 
		newPitch = minY;

	if (newPitch + addPitch != cdata.fPitch || newYaw + addYaw != cdata.fYaw)
	{
		cdata.fPitch = newPitch + addPitch;
		m_fPitch = newPitch;
		cdata.fYaw = newYaw + addYaw;
		m_fYaw = newYaw;
//		pClientDE->CPrint("Pitch=%14.6f Yaw=%14.6f",(double)m_fPitch,(double)m_fYaw); // BLB TEMP
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::UpdateCameraRotation()
//
//	PURPOSE:	Set the new camera rotation
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::UpdateCameraRotation()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !m_hCamera) return;

	HLOCALOBJ hPlayerObj = pClientDE->GetClientObject();
	if (!hPlayerObj) return;

	if (m_bDemoPlayback)
	{
		// Just get camera angles from the object...

		pClientDE->GetObjectRotation(hPlayerObj, &m_Rotation);
		pClientDE->SetObjectRotation(m_hCamera, &m_Rotation);
		return;
	}


	// Setup the pitch and roll
	pClientDE->SetupEuler(&m_Rotation, cdata.fPitch, cdata.fYaw, 0);

	// Rotate the head canting amount around vF
	DVector vU, vF, vR;
	pClientDE->GetRotationVectors(&m_Rotation, &vU, &vR, &vF);
	pClientDE->RotateAroundAxis(&m_Rotation, &vF, m_fCamCant + m_fBobCant + m_fShakeCant);
	
	if (m_pExternalCamera)
	{
		// Just calculate the correct player rotation...
	}
	else if (m_playerCamera.IsFirstPerson())
	{
		pClientDE->SetObjectRotation(m_hCamera, &m_Rotation);
	}
	else
	{
		// Set the camera to use the rotation calculated by the player camera,
		// however we still need to calculate the correct rotation to be sent
		// to the player...

		DRotation rRot = m_playerCamera.GetRotation();
		pClientDE->SetObjectRotation(m_hCamera, &rRot);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::GetWorldTitle()
//
//	PURPOSE:	Parse the world title from the WorldInfo string
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::GetWorldTitle(char *pWorldFile)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE || !pWorldFile) return;

	HSTRING	hstrTitle = DNULL;
	char szFilename[MAX_CS_FILENAME_LEN+1];
	char szWorldInfo[255];

#ifdef _ADDON
	char sUpr[256];
	strncpy(sUpr, pWorldFile, 255);
	strupr(sUpr);
	if (strstr(sUpr, "WORLDS_AO"))
	{
		strncpy(szFilename, pWorldFile, MAX_CS_FILENAME_LEN);
		if (!strstr(sUpr, ".DAT"))
		{
			strcat(szFilename, ".dat");
		}
	}
	else
	{
		sprintf(szFilename, "Worlds\\%s.dat", pWorldFile);
	}
#else
	sprintf(szFilename, "Worlds\\%s.dat", pWorldFile);
#endif

	DDWORD dwInfoLen;
	if(pClientDE->GetWorldInfoString(szFilename, szWorldInfo, 255, &dwInfoLen) == LT_OK)
	{
		int nArgs;
    
		char tokenSpace[64*20];
		char *pTokens[64];
		char *pCommand = szWorldInfo;
		char *pCommandPos;

		DBOOL bDone = DFALSE;
		while (!bDone)
		{
			if (!pClientDE->Parse(pCommand, &pCommandPos, tokenSpace, pTokens, &nArgs))
				bDone = DTRUE;
			else
				pCommand = pCommandPos;

			for(int i = 0; i < nArgs; i++)
			{
				if(pTokens[i])
				{
					if((_mbsicmp((const unsigned char*)"worldname", (const unsigned char*)pTokens[i]) == 0) && pTokens[i+1])
					{
						m_hstrTitle = pClientDE->CreateString(pTokens[i+1]);
					}
					else if((_mbsicmp((const unsigned char*)"loadscreen", (const unsigned char*)pTokens[i]) == 0) && pTokens[i+1])
					{
#ifdef _DEMO
						strupr(szFilename);
						if (_mbsstr((const unsigned char*)szFilename, (const unsigned char*)"DEMO_02"))
							m_hstrLoadScreen = pClientDE->CreateString("\\screens\\loading2.pcx");
						else
							m_hstrLoadScreen = pClientDE->CreateString("\\screens\\loading1.pcx");
#else
						char szLoadScreen[128];
						sprintf(szLoadScreen, "Interface\\LoadScreens\\%s", pTokens[i+1]);
						m_hstrLoadScreen = pClientDE->CreateString(szLoadScreen);
#endif

					}
					else if((_mbsicmp((const unsigned char*)"loadscreenID", (const unsigned char*)pTokens[i]) == 0) && pTokens[i+1])
					{
						m_nLoadScreenID = (DDWORD)atoi(pTokens[i+1]);
					}
				}
			}
		}
	}


	// Make sure we have a title...

	if(!m_hstrTitle)
		m_hstrTitle = pClientDE->CreateString(pWorldFile);


	// Change the loading screen string if this is a nightmare level...

#ifdef _ADDON
	strupr(szFilename);
	if (strstr(szFilename, "WORLDS_AO"))
	{
		if (m_hstrLoadScreen) { pClientDE->FreeString(m_hstrLoadScreen); m_hstrLoadScreen = 0; }
		m_hstrLoadScreen = pClientDE->CreateString("Screens_ao\\Bumper.pcx");
	}
#endif

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	LoadLeakFile
//
//	PURPOSE:	Loads a leak file and creates a line system for it.
//
// --------------------------------------------------------------------------- //

DBOOL LoadLeakFile(ClientDE *pClientDE, char *pFilename)
{
	FILE *fp;
	char line[256];
	HLOCALOBJ hObj;
	ObjectCreateStruct cStruct;
	DELine theLine;
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
			return DFALSE;
		}

		while(fgets(line, 256, fp))
		{
			nRead = sscanf(line, "%f %f %f %f %f %f", 
				&theLine.m_Points[0].m_Pos.x, &theLine.m_Points[0].m_Pos.y, &theLine.m_Points[0].m_Pos.z, 
				&theLine.m_Points[1].m_Pos.x, &theLine.m_Points[1].m_Pos.y, &theLine.m_Points[1].m_Pos.z);

			theLine.m_Points[0].r = theLine.m_Points[0].g = theLine.m_Points[0].b = 1;
			theLine.m_Points[0].a = 1;
			
			theLine.m_Points[1].r = 1;
			theLine.m_Points[1].g = theLine.m_Points[1].b = 0;
			theLine.m_Points[1].a = 1;

			pClientDE->AddLine(hObj, &theLine);
		}

		fclose(fp);
		return DTRUE;
	}
	else
	{
		return DFALSE;
	}
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LoadConfig
//
//	PURPOSE:	Reads player config data
//
// ----------------------------------------------------------------------- //
DBOOL LoadConfig(char* pfilename, ConfigStruct *pConfig) 
{
	FILE *infile;
	char line[256];
	char *tokenname;
	char *tokenvalue;

	if (!(infile = fopen(pfilename, "r")))
		return DFALSE;

	while (fgets(line, 256, infile) != NULL)
	{
		// Get tokens, see if first one is a comment
		if (!(tokenname = strtok(line, " \t")) || tokenname[0] == ';')
			continue;
		if (!(tokenvalue = strtok(NULL, " \t\r\n")))
			continue;
		
		_strupr(tokenname);

		// Check for attributes
		if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"STRENGTH"))
			pConfig->nStrength = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"MAGIC"))
			pConfig->nMagic = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"SPEED"))
			pConfig->nSpeed = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"RESISTANCE"))
			pConfig->nResistance = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"NAME"))
			_mbscpy(pConfig->szName, tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"CHARACTER"))
			pConfig->nCharacter = atoi(tokenvalue);
		// Check for bindings
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"BINDING1"))
			pConfig->nBindingSlots[0] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"BINDING2"))
			pConfig->nBindingSlots[1] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"BINDING3"))
			pConfig->nBindingSlots[2] = atoi(tokenvalue);
		// Check for weapons
		// Slot 1 is always melee
//		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP1"))
//			pConfig->nWeaponSlots[0] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP2"))
			pConfig->nWeaponSlots[1] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP3"))
			pConfig->nWeaponSlots[2] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP4"))
			pConfig->nWeaponSlots[3] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP5"))
			pConfig->nWeaponSlots[4] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP6"))
			pConfig->nWeaponSlots[5] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP7"))
			pConfig->nWeaponSlots[6] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP8"))
			pConfig->nWeaponSlots[7] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP9"))
			pConfig->nWeaponSlots[8] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"WEAP0"))
			pConfig->nWeaponSlots[9] = atoi(tokenvalue);
		// Check for inventory items
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"INV1"))
			pConfig->nItemSlots[0] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"INV2"))
			pConfig->nItemSlots[1] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"INV3"))
			pConfig->nItemSlots[2] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"INV4"))
			pConfig->nItemSlots[3] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"PROXIMITYBOMBS"))
			pConfig->nProximityBombs = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"REMOTEBOMBS"))
			pConfig->nRemoteBombs = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"SMOKEBOMBS"))
			pConfig->nSmokeBombs = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"TIMEBOMBS"))
			pConfig->nTimeBombs = atoi(tokenvalue);
		// Check for spells
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"SPELL1"))
			pConfig->nSpellSlots[0] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"SPELL2"))
			pConfig->nSpellSlots[1] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"SPELL3"))
			pConfig->nSpellSlots[2] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"SPELL4"))
			pConfig->nSpellSlots[3] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"SPELL5"))
			pConfig->nSpellSlots[4] = atoi(tokenvalue);
		else if (!_mbscmp((const unsigned char*)tokenname, (const unsigned char*)"SPELL6"))
			pConfig->nSpellSlots[5] = atoi(tokenvalue);
	}
	fclose(infile);

	return DTRUE;
}
*/

/*
void CBloodClientShell::ImplementMusicSource (CClientDE* pClientDE)
{
	if (!pClientDE || !pClientShell) return;

	char str[64];
	switch (nMusicSource)
	{
		// Disable music...
		case 0:
		{
			_mbscpy (str, "musicenable 0");
			pClientDE->RunConsoleString (str);
		}
		break;

		// Use IMA...
		case 1:
		{
			_mbscpy (str, "musicenable 1");
			pClientDE->RunConsoleString (str);

			if (GetMusic()->IsInitialized())
			{
				// Only re-init if not already using ima...
				if (!GetMusic()->UsingIMA())
				{
					// preserve the music level, just in case we come back to ima...
					CMusic::EMusicLevel level;
					level = GetMusic()->GetMusicLevel();
					if (GetMusic()->Init (pClientDE, DTRUE))
					{
						// Only start the music if we are in a world...
						if (IsInWorld() && !pClientShell->IsFirstUpdate())
						{
							GetMusic()->InitPlayLists();
							GetMusic()->PlayMusicLevel (level);
						}
					}
				}
			}
			// music should already be inited, but let try again...
			else if (pClientShell->GetMusic()->Init (pClientDE, DTRUE))
			{
				// Only re-init the play lists if we have been in world for a while...
				if (pClientShell->IsInWorld() && !pClientShell->IsFirstUpdate())
					pClientShell->GetMusic()->InitPlayLists();
			}
		}
		break;

		// Use CD Audio...
		case 2:
		{
			_mbscpy (str, "musicenable 1");
			pClientDE->RunConsoleString (str);

			if (pClientShell)
			{
				if (pClientShell->GetMusic()->IsInitialized())
				{
					if (pClientShell->GetMusic()->UsingIMA())
					{
						// preserve the music level, just in case we come back to ima...
						CMusic::EMusicLevel level;
						level = pClientShell->GetMusic()->GetMusicLevel();
						if (pClientShell->GetMusic()->Init (pClientDE, DFALSE))
						{
							// Only start the music if we are in a world...
							if (pClientShell->IsInWorld() && !pClientShell->IsFirstUpdate())
							{
								pClientShell->GetMusic()->InitPlayLists();
								pClientShell->GetMusic()->SetMusicLevel (level);
								pClientShell->GetMusic()->PlayCDList();
							}
						}
					}
				}
				// music should already be inited, but let try again...
				else if (pClientShell->GetMusic()->Init (pClientDE, DTRUE))
				{
					// Only re-init the play lists if we have been in world for a while...
					if (pClientShell->IsInWorld() && !pClientShell->IsFirstUpdate())
					{
						pClientShell->GetMusic()->InitPlayLists();
						pClientShell->GetMusic()->PlayCDList();
					}
				}
			}
		}
		break;
	}
}

void CBloodClientShell::ImplementMusicVolume (CClientDE* pClientDE)
{
	if (!pClientDE) return;

	pClientDE->SetMusicVolume (nMusicVolume);

	char strConsole[64];
	sprintf (strConsole, "MusicVolume %d", nMusicVolume);
	pClientDE->RunConsoleString (strConsole);
}

void CBloodClientShell::ImplementSoundVolume (CClientDE* pClientDE)
{
	if (!pClientDE) return;

	pClientDE->SetSoundVolume (nSoundVolume);

	char strConsole[64];
	sprintf (strConsole, "SoundVolume %d", nSoundVolume);
	pClientDE->RunConsoleString (strConsole);
}

*/


#ifdef BRANDED
void CBloodClientShell::CreateBrandSurface()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	if (!m_hBrandSurface)
	{
		HSTRING hString = pClientDE->FormatString(IDS_BRAND, "Review Copy:");

		if (hString)
		{
			HDECOLOR hForeground = pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, DFALSE);
			HDECOLOR hTrans = pClientDE->SetupColor2 (0, 0, 0, DFALSE);

			HDEFONT hFont = pClientDE->CreateFont ("Arial", 6, 12, DFALSE, DFALSE, DTRUE);
			m_hBrandSurface = pClientDE->CreateSurfaceFromString (hFont, hString, hForeground, hTrans, 10, 0);

			pClientDE->FreeString(hString);
			pClientDE->DeleteFont (hFont);
		}
	}
}


void CBloodClientShell::DrawBrandString(HSURFACE hScreenSurface)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;

	if (m_hBrandSurface)
	{
		HDECOLOR hForeground = pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, DFALSE);
		HDECOLOR hTrans = pClientDE->SetupColor2 (0, 0, 0, DFALSE);

		DDWORD dwWidth, dwHeight;
		DDWORD dwScreenWidth, dwScreenHeight;
		pClientDE->GetSurfaceDims(m_hBrandSurface, &dwWidth, &dwHeight);
		pClientDE->GetSurfaceDims(hScreenSurface, &dwScreenWidth, &dwScreenHeight);

		pClientDE->DrawSurfaceToSurfaceTransparent (hScreenSurface, m_hBrandSurface, NULL, (dwScreenWidth - dwWidth - 1), (dwScreenHeight - dwHeight - 1), hTrans);
	}
}

#endif // BRANDED


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ConnectToTcpIpAddress
//
//	PURPOSE:	Connects (joins) to the given tcp/ip address
//
// --------------------------------------------------------------------------- //

DBOOL ConnectToTcpIpAddress(CClientDE* pClientDE, char* sAddress)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!sAddress) return(DFALSE);


	// Try to connect to the given address...

	DBOOL db = NetStart_DoConsoleConnect(pClientDE, sAddress);

	if (!db)
	{
		if (_mbstrlen(sAddress) <= 0) pClientDE->CPrint("Unable to connect");
		else pClientDE->CPrint("Unable to connect to %s", sAddress);
		return(DFALSE);
	}


	// All done...

	if (_mbstrlen(sAddress) > 0) pClientDE->CPrint("Connected to %s", sAddress);
	else pClientDE->CPrint("Connected");

	g_pBloodClientShell->SetGameState(GS_MPLOADINGLEVEL);

	return(DTRUE);
}

DBOOL CBloodClientShell::DoMultiplayer(DBOOL bMinimize, DBOOL bHost)
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return(DFALSE);

	pClientDE->PauseSounds();
		
	if (bMinimize)
	{
		NetStart_MinimizeMainWnd(pClientDE);
	}

	NetStart ns;
	DBOOL bRet = NetStart_DoWizard(pClientDE, &ns, bHost ? NET_HOST : NET_JOIN);

	pClientDE->ResumeSounds();

	if (bRet)
	{
		if (m_hstrTitle) { pClientDE->FreeString(m_hstrTitle); m_hstrTitle = 0; }
		if (m_hstrLoadScreen) { pClientDE->FreeString(m_hstrLoadScreen); m_hstrLoadScreen = 0; }
		if (m_nLoadScreenID) m_nLoadScreenID = 0;

		StartGameRequest req;
		memset(&req, 0, sizeof(req));

		if (ns.m_bHost)
		{
			req.m_Type = STARTGAME_HOST;

			_mbscpy((unsigned char*)req.m_WorldName, (const unsigned char*)ns.m_sLevel);
			memcpy(&req.m_HostInfo, &ns.m_NetHost, sizeof(NetHost));

			g_bIsHost = DTRUE;
		}
		else
		{
			req.m_Type        = STARTGAME_CLIENT;
			req.m_pNetSession = ns.m_pNetSession;

			if (ns.m_bHaveTcpIp)
			{
				req.m_Type = STARTGAME_CLIENTTCP;
				_mbscpy((unsigned char*)req.m_TCPAddress, (const unsigned char*)ns.m_sAddress);
			}

			g_bIsHost = DFALSE;
		}

		req.m_pGameInfo     = NetStart_GetGameStruct();
		req.m_GameInfoLen   = sizeof(NetGame_t);
		req.m_pClientData   = NetStart_GetClientDataStruct();
		req.m_ClientDataLen = sizeof(NetClientData_t);
	
		DRESULT dr = pClientDE->StartGame(&req);

		if (bMinimize)
		{
			NetStart_RestoreMainWnd();
		}

		if (dr != LT_OK)
		{
			return(DFALSE);
		}
	}
	else
	{
		if (bMinimize)
		{
			NetStart_RestoreMainWnd();
		}

		return(DFALSE);
	}

	NetStart_RunServerOptions(pClientDE);
	SetGameState(GS_MPLOADINGLEVEL);


	// All done...

	return(DTRUE);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::IsMultiplayerGame()
//
//	PURPOSE:	Are we playing a multiplayer game?
//
// --------------------------------------------------------------------------- //

DBOOL CBloodClientShell::IsMultiplayerGame()
{
	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE ) return DFALSE;

	int nGameMode = 0;
	pClientDE->GetGameMode(&nGameMode);
	if (nGameMode == STARTGAME_NORMAL || nGameMode == GAMEMODE_NONE) return DFALSE;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::PlayMovie()
//
//	PURPOSE:	Plays the given movie
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::PlayMovie(CClientDE* pClientDE, char* sMovie, DBOOL bCheckExistOnly)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!sMovie) return(DFALSE);


	// Check if movies are disabled...

	if (AdvancedDisableMovies())
	{
		return(DFALSE);
	}


	// Check if we got a full path...

	char sMoviePath[256];

	if (_mbsstr((const unsigned char*)sMovie, (const unsigned char*)"\\"))
	{
		_mbscpy((unsigned char*)sMoviePath, (const unsigned char*)sMovie);
	}
	else
	{
		sMoviePath[0] = '\0';
	}


	// Check if we need to look for this movie...

	if (sMoviePath[0] == '\0')
	{
		// Check if the movie file exists locally...

		if (CWinUtil::FileExist(sMovie))
		{
			_mbscpy((unsigned char*)sMoviePath, (const unsigned char*)sMovie);
		}
		else
		{
			// Look in a movies folder on the hard disk...

			char sTemp[256];
			sprintf(sTemp, "Movies\\%s", sMovie);
			if (CWinUtil::FileExist(sTemp))
			{
				_mbscpy((unsigned char*)sMoviePath, (const unsigned char*)sTemp);
			}
			else
			{
				// Look on the cd-rom...

				char chCdRom = CWinUtil::GetCdRomDriveWithGame();
				if (chCdRom == 0) return(DFALSE);

				sprintf(sMoviePath, "%c:\\Movies\\%s", chCdRom, sMovie);

				if (!CWinUtil::FileExist(sMoviePath)) return(DFALSE);
			}
		}
	}

	if (sMoviePath[0] == '\0') return(DFALSE);


	// If we are only checking for existence, return now...

	if (bCheckExistOnly)
	{
		return(DTRUE);
	}


	// Play the movie...

	DDWORD  nFlags  = PLAYBACK_FULLSCREEN;
	DRESULT nResult = pClientDE->StartVideo(sMoviePath, nFlags);

	if (nResult != DE_OK)
	{
		return(DFALSE);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::StartMovies()
//
//	PURPOSE:	Starts the movies and sets up the movie state for updating
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::StartMovies(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);

#ifdef _DEMO
	return(DFALSE);
#endif


	// Check if movies are disabled...

	if (AdvancedDisableMovies())
	{
		return(DFALSE);
	}


	// Make sure the movies actually exist...

	if (!PlayMovie(pClientDE, "Monolith.smk", DTRUE))
	{
		return(DFALSE);
	}


	// Set the movie state to the starting state...

	m_nMovieState = MOVIE_STARTLITH;


	// Set the game state...

	SetGameState(GS_MOVIES);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::EndMovies()
//
//	PURPOSE:	Ends the movies and returns the game state
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::EndMovies(CClientDE* pClientDE, DBOOL bShowTitle)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);


	// Stop the movies...

	pClientDE->StopVideo();

	m_nMovieState = MOVIE_FINISHED;


	// Show the title screen if requested...

#ifdef _DEBUG
	bShowTitle = DFALSE;
#endif

	if (bShowTitle)
	{

#ifdef _ADDON
		HSURFACE hTitleSurface = Splash_Display(pClientDE, "\\screens_ao\\title.pcx", DTRUE);
#else
		HSURFACE hTitleSurface = Splash_Display(pClientDE, "\\screens\\title.pcx", DTRUE);
#endif

		if (hTitleSurface)
		{

			CWinUtil::TimedKeyWait(VK_ESCAPE, VK_RETURN, VK_SPACE, 6.0f);
			pClientDE->DeleteSurface(hTitleSurface);
			hTitleSurface = DNULL;
			CWinUtil::Sleep(50);
		}
	}


	// Reset the menu game state...

	CWinUtil::RemoveKeyDownMessages();
	SetGameState(GS_MENU);
	m_Menu.SetCurrentMenu(MENU_ID_MAINMENU, MENU_ID_MAINMENU);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::AdvanceMovies()
//
//	PURPOSE:	Advanced to the next movie
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::AdvanceMovies(CClientDE* pClientDE)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);


	// Stop the current movies...

	pClientDE->StopVideo();


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRiotClientShell::UpdateMoviesState()
//
//	PURPOSE:	Update movies
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::UpdateMoviesState()
{
	// Sanity checks...

	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return;


	// Update based on our current movie state...

	switch (m_nMovieState)
	{
		case MOVIE_STARTLITH:
		{
			if (!PlayMovie(pClientDE, "Monolith.smk"))
			{
				EndMovies(pClientDE);
				return;
			}

			m_nMovieState = MOVIE_PLAYINGLITH;

			break;
		}

		case MOVIE_PLAYINGLITH:
		{
			pClientDE->UpdateVideo();

			if (pClientDE->IsVideoPlaying() != VIDEO_PLAYING)
			{
				m_nMovieState = MOVIE_STARTGTI;
			}

			break;
		}

		case MOVIE_STARTGTI:
		{
			if (!PlayMovie(pClientDE, "Gti.smk"))
			{
				EndMovies(pClientDE);
				return;
			}

			m_nMovieState = MOVIE_PLAYINGGTI;

			break;
		}

		case MOVIE_PLAYINGGTI:
		{
			pClientDE->UpdateVideo();

			if (pClientDE->IsVideoPlaying() != VIDEO_PLAYING)
			{
				m_nMovieState = MOVIE_STARTINTRO;

#ifdef _ADDON
				m_nMovieState = MOVIE_FINISHED;
#endif

			}

			break;
		}

		case MOVIE_STARTINTRO:
		{
			if (!PlayMovie(pClientDE, "Intro.smk"))
			{
				EndMovies(pClientDE);
				return;
			}

			m_nMovieState = MOVIE_PLAYINGINTRO;

			break;
		}

		case MOVIE_PLAYINGINTRO:
		{
			pClientDE->UpdateVideo();

			if (pClientDE->IsVideoPlaying() != VIDEO_PLAYING)
			{
				m_nMovieState = MOVIE_FINISHED;
			}

			break;
		}

		case MOVIE_FINISHED:
		default:
		{
			EndMovies(pClientDE, DTRUE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Abort
//
//	PURPOSE:	Aborts the game with a message and/or code
//
// ----------------------------------------------------------------------- //

void Abort(char* sErrorMsg, DDWORD dwCode)
{
	// Sanity checks...

	if (!g_pBloodClientShell) return;
	if (!g_pBloodClientShell->GetClientDE()) return;


	// Check if we've already aborted...

	if (g_bAborting)
	{
		return;
	}

	g_bAborting = DTRUE;


	// Build the error string...

	char sText[256];

	if (dwCode != 0)
	{
		sprintf(sText, "%s (%lu)", sErrorMsg, dwCode);
	}
	else
	{
		_mbscpy((unsigned char*)sText, (const unsigned char*)sErrorMsg);
	}


	// Shutdown the engine with an error message...

	g_pBloodClientShell->GetClientDE()->ShutdownWithMessage(sText);
}

void Abort(int nErrorID, DDWORD dwCode)
{
	// Sanity checks...

	if (!g_pBloodClientShell) return;
	if (!g_pBloodClientShell->GetClientDE()) return;


	// Load the string...

	char sText[256];

	HSTRING hString = g_pBloodClientShell->GetClientDE()->FormatString(nErrorID);
	if (!hString)
	{
		hString = g_pBloodClientShell->GetClientDE()->FormatString(IDS_ERR_GENERIC);
	}

	if (hString)
	{
		_mbscpy((unsigned char*)sText, (const unsigned char*)g_pBloodClientShell->GetClientDE()->GetStringData(hString));
		g_pBloodClientShell->GetClientDE()->FreeString(hString);
	}
	else
	{
		_mbscpy((unsigned char*)sText, (const unsigned char*)"ERROR - Unable to continue the game.");
	}


	// Let the main function do the real work...

	Abort(sText, dwCode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuQuit
//
//	PURPOSE:	Quits the game immediately
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuQuit()
{
	// Shutdown everything...

	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return(DFALSE);

	pClientDE->Shutdown();


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuNewGame
//
//	PURPOSE:	Starts new game at the given difficulty level
//
//	COMMENT:	nDifficulty must be one of: DIFFICULTY_EASY,
//				DIFFICULTY_MEDIUM, or DIFFICULTY_HARD.
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuNewGame(int nDifficulty, int nCharacter, int nGameType)
{
	// Remove the current save info...

	m_SavedGameInfo.ClearSaveSlot(SLOT_CURRENT);


	// Set the starting level...

#ifdef _DEMO
	char* sFirstLevel = "Demo_01";
#else
	char* sFirstLevel = "01_SubwayA";
#endif


	// Set some starting flags...

	m_bFirstWorld = DTRUE;

	g_pClientDE->RunConsoleString("maxfps 0");


	// Fill in the character info...

	if (!SetCharacterInfo(nCharacter, MULTIPLAY_SKIN_NORMAL))
	{
		return(DFALSE);
	}


	// Start the game at the given character and difficulty...

	return(StartNewGame(sFirstLevel, STARTGAME_NORMAL, nGameType, LOADTYPE_NEW_GAME, nDifficulty));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuNewNightmaresGame
//
//	PURPOSE:	Starts new nightmares game at the given difficulty level
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuNewNightmaresGame(int nDifficulty)
{
	// Remove the current save info...

	m_SavedGameInfo.ClearSaveSlot(SLOT_CURRENT);


	// Set the starting level...

	char* sFirstLevel = "worlds_ao\\caleb1_cc_c";


	// Set some starting flags...

	m_bFirstWorld = DTRUE;

	g_pClientDE->RunConsoleString("maxfps 0");


	// Fill in the character info...

	if (!SetCharacterInfo(CHARACTER_CALEB, MULTIPLAY_SKIN_NORMAL))
	{
		return(DFALSE);
	}


	// Start the game at the given character and difficulty...

	return(StartNewGame(sFirstLevel, STARTGAME_NORMAL, GAMETYPE_SINGLE, LOADTYPE_NEW_GAME, nDifficulty));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuHostGame
//
//	PURPOSE:	Hosts a multiplayer network game
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuHostGame()
{
	// Do the multipalyer wizard and host a net game...

	return(DoMultiplayer(DTRUE, DTRUE));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuJoinGame
//
//	PURPOSE:	Joins a multiplayer network game
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuJoinGame()
{
	// Do the multipalyer wizard and join a net game...

	return(DoMultiplayer(DTRUE, DFALSE));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuCredits
//
//	PURPOSE:	Displays the credits
//
//	COMMENT:	The "lame" credits will be used if we're currently
//				in the middle of a game.
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuCredits()
{
	// If we're currently in a game, do the lame credits...

	if (m_bInWorld)
	{
		m_Credits.Term();

		CClientDE* pClientDE = GetClientDE();
		if (!pClientDE) return(DFALSE);

		if (!m_Credits.Init(pClientDE, this))
		{
			return(DFALSE);
		}

		PauseGame(DTRUE);
		SetGameState(GS_CREDITS);

		return(DTRUE);
	}


	// If we're not in a game, do the cool credits...

	return(StartNewGame("Credits", STARTGAME_NORMAL, GAMETYPE_CUSTOM, LOADTYPE_NEW_GAME, DIFFICULTY_MEDIUM));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuHelp
//
//	PURPOSE:	Display the help info
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuHelp()
{
	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuLoadGame
//
//	PURPOSE:	Loads the given game slot
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuLoadGame(int nSlot)
{
	// As we go, figure out the gametype...

	int nGameType  = GAMETYPE_SINGLE;
	int nCharacter = GetCharacter();


	// Check for the quick slot...

	if (nSlot == SLOT_QUICK)
	{
		_mbscpy((unsigned char*)m_szFilename, (const unsigned char*)m_SavedGameInfo.gQuickSaveInfo.szCurrentLevel);
		nGameType  = m_SavedGameInfo.gQuickSaveInfo.nGameType;
		nCharacter = m_SavedGameInfo.gQuickSaveInfo.nCharacter;
	}

	if (nSlot == SLOT_CURRENT)
	{
		_mbscpy((unsigned char*)m_szFilename, (const unsigned char*)m_SavedGameInfo.gCurrentSaveInfo.szCurrentLevel);
		nGameType  = m_SavedGameInfo.gCurrentSaveInfo.nGameType;
		nCharacter = m_SavedGameInfo.gCurrentSaveInfo.nCharacter;
	}

		
	// Check for a valid regular slot...

	if (nSlot != SLOT_QUICK && nSlot != SLOT_CURRENT)
	{
		if (nSlot < 0) return(DFALSE);
		if (nSlot >= MAX_SAVESLOTS) return(DFALSE);

		_mbscpy((unsigned char*)m_szFilename, (const unsigned char*)m_SavedGameInfo.gSaveSlotInfo[nSlot].szCurrentLevel);
		nGameType  = m_SavedGameInfo.gSaveSlotInfo[nSlot].nGameType;
		nCharacter = m_SavedGameInfo.gSaveSlotInfo[nSlot].nCharacter;
	}


	// Make sure we have a level to load...

	if (_mbstrlen(m_szFilename) <= 0)
	{
		return(DFALSE);
	}

	// Copy slot to the current slot
	
	if (nSlot != SLOT_NONE && nSlot != SLOT_CURRENT)
	{
		m_SavedGameInfo.CopySlot(nSlot, SLOT_CURRENT);
	}


	// Special load type for current slot (autosave)

	DBYTE byLoadType = (nSlot == SLOT_CURRENT) ? LOADTYPE_RESTOREAUTOSAVE : LOADTYPE_RESTORESAVE;


	// Fill in the character info...

	if (!SetCharacterInfo(nCharacter, MULTIPLAY_SKIN_NORMAL))
	{
		return(DFALSE);
	}


	// Validate the game type...

	if (nGameType != GAMETYPE_SINGLE && nGameType != GAMETYPE_ACTION && nGameType != GAMETYPE_CUSTOM)
	{
		nGameType = GAMETYPE_SINGLE;
	}


	// Load the game...

	return(StartNewGame(m_szFilename, STARTGAME_NORMAL, nGameType, byLoadType, DIFFICULTY_MEDIUM));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuSaveGame
//
//	PURPOSE:	Saves the given game slot
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuSaveGame(int nSlot, DBYTE bySaveType)
{
	// Make sure we are actually in a game & still alive...

	if (!m_bInWorld || m_bDead) return(DFALSE);


	if (bySaveType == SAVETYPE_CURRENT)
	{
		// Make sure this is a valid slot...

		if (nSlot < SLOT_QUICK || nSlot >= MAX_SAVESLOTS) return(DFALSE);

		// Set the saved game info data so we can copy to a save slot once the server
		// tells us the save is complete (see SMSG_SAVEGAME_ACK).

		m_SavedGameInfo.SetReservedSlot(nSlot);


		// Change the game state to reflect the fact that we are saving the game...

		SetGameState(GS_SAVING);
	}


	// Tell the server to save our game...

	CClientDE* pClientDE = GetClientDE();
	if (!pClientDE) return(DFALSE);

	HMESSAGEWRITE hMsg = pClientDE->StartMessage(CMSG_SAVEGAME);

	pClientDE->WriteToMessageString(hMsg, m_szFilename);
	pClientDE->WriteToMessageByte(hMsg, bySaveType);
	SaveClientData( hMsg );
	pClientDE->EndMessage(hMsg);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuLoadCustomLevel
//
//	PURPOSE:	Loads the given custom level
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuLoadCustomLevel(char* sLevel, int nDifficulty)
{
	// Sanity checks...

	if (!sLevel) return(DFALSE);
	if (_mbstrlen(sLevel) <= 0) return(DFALSE);


	// Flag that we are starting a new game world...

	m_bFirstWorld = DTRUE;


	// Start a new game with the specified level and difficulty...

	return(StartNewGame(sLevel, STARTGAME_NORMAL, GAMETYPE_CUSTOM, LOADTYPE_NEW_GAME, nDifficulty));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuReturnToGame
//
//	PURPOSE:	Returns from the menus to the game
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuReturnToGame()
{
	if (IsInWorld())
	{
		SetGameState(GS_PLAYING);
		PauseGame(DFALSE);

		if (g_bLevelChange3rdPersonCam)
		{
			if (g_pClientDE) g_pClientDE->SetInputState(DFALSE);
			if (!m_b3rdPerson) Set3rdPersonCamera(DTRUE);
			m_bDrawFragBar = DTRUE;

		}

		return DTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::MenuSetDetail
//
//	PURPOSE:	Sets the detail file cfg and checks the launcher settings
//
// ----------------------------------------------------------------------- //

DBOOL CBloodClientShell::MenuSetDetail()
{
	CClientDE* pClientDE = GetClientDE();
	HCONSOLEVAR		hVar;

	// General detail settings
	hVar = pClientDE->GetConsoleVar("GlobalDetail");
	m_nGlobalDetail = (DBYTE)pClientDE->GetVarValueFloat (hVar);

	switch(m_nGlobalDetail)
	{
		case	DETAIL_LOW:
			pClientDE->ReadConfigFile("detaillo.cfg");
			break;
		case	DETAIL_MEDIUM:
			pClientDE->ReadConfigFile("detailmd.cfg");
			break;
		case	DETAIL_HIGH:
			pClientDE->ReadConfigFile("detailhi.cfg");
			break;
	}

	// record the original state of sound and music

	hVar = pClientDE->GetConsoleVar ("SoundEnable");
	m_bSoundOriginallyEnabled = (DBOOL) pClientDE->GetVarValueFloat (hVar);
	
	hVar = pClientDE->GetConsoleVar ("MusicEnable");
	m_bMusicOriginallyEnabled = (DBOOL) pClientDE->GetVarValueFloat (hVar);

	hVar = pClientDE->GetConsoleVar ("ModelFullbrite");
	m_bModelFBOriginallyEnabled	= (DBOOL) pClientDE->GetVarValueFloat (hVar);
	
	hVar = pClientDE->GetConsoleVar ("LightMap");
	m_bLightmappingOriginallyEnabled = (DBOOL) pClientDE->GetVarValueFloat (hVar);

	// implement any advanced options here (before renderer is started)

	hVar = pClientDE->GetConsoleVar ("SoundEnable");
	if (!hVar && !AdvancedDisableSound())
	{
		pClientDE->RunConsoleString ("SoundEnable 1");
	}
	else if (AdvancedDisableSound())
	{
		pClientDE->RunConsoleString ("SoundEnable 0");	// [blg] is this ok?
	}

	hVar = pClientDE->GetConsoleVar ("MusicEnable");
	if (!hVar && !AdvancedDisableMusic())
	{
		pClientDE->RunConsoleString ("MusicEnable 1");
	}
	else if (AdvancedDisableMusic())
	{
		pClientDE->RunConsoleString ("MusicEnable 0");	// [blg] is this ok?
	}

	if (AdvancedDisableLightMap())
	{
		pClientDE->RunConsoleString ("LightMap 0");
	}
	if (AdvancedEnableOptSurf())
	{
		pClientDE->RunConsoleString ("OptimizeSurfaces 1");
	}
	if (AdvancedEnableTripBuf())
	{
		pClientDE->RunConsoleString ("TripleBuffer 1");
	}
	if (AdvancedDisableDx6Cmds())
	{
		pClientDE->RunConsoleString ("UseDX6Commands 0");
	}
	if (AdvancedEnableTJuncs())
	{
		pClientDE->RunConsoleString ("FixTJunc 1");
	}
	if (AdvancedDisableFog())
	{
		pClientDE->RunConsoleString ("FogEnable 0");
	}
	if (AdvancedDisableLines())
	{
		pClientDE->RunConsoleString ("DrawLineSystems 0");
	}
	if (AdvancedDisableModelFB())
	{
		pClientDE->RunConsoleString ("ModelFullBrite 0");
	}
	if (AdvancedDisableJoystick())
	{
		pClientDE->RunConsoleString ("JoystickDisable 1");
	}
	if (AdvancedEnablePixelDoubling())
	{
		pClientDE->RunConsoleString ("PixelDouble 1");
	}
	if (AdvancedEnableMultiTexturing())
	{
		pClientDE->RunConsoleString ("Force1Pass 1");
	}

	return DTRUE;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::HandleInvAction()
//
//	PURPOSE:	The player has hit an exit trigger.
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::HandleInvAction( DBYTE nType, DBOOL bActivate )
{
	switch( nType )
	{
		case INV_BINOCULARS:
			PlaySoundLocal( "sounds\\powerups\\binoculars.wav", SOUNDPRIORITY_PLAYER_LOW );
			m_bBinocularsActive = bActivate;
			break;
		case INV_MEDKIT:
			PlaySoundLocal( "sounds\\powerups\\medikit.wav", SOUNDPRIORITY_PLAYER_LOW );
			break;
		case INV_FLASHLIGHT:
			PlaySoundLocal( "sounds\\powerups\\flashlight.wav", SOUNDPRIORITY_PLAYER_LOW );
			break;
		case INV_NIGHTGOGGLES:
			m_bNightGogglesActive = bActivate;
			break;
		default:
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SetMouseSensitivity()
//
//	PURPOSE:	Sets the mouse sensitivity
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::SetMouseSensitivity(float fSensitivity)
{
	CClientDE *pClientDE=GetClientDE();
	if (!pClientDE)
	{
		return;
	}

	char szDeviceName[256];
	char szXAxis[256];
	char szYAxis[256];

	memset(szDeviceName, 0, sizeof(szDeviceName));
	memset(szXAxis, 0, sizeof(szXAxis));
	memset(szYAxis, 0, sizeof(szYAxis));

	// Get the mouse device information
	GetMouseDeviceInfo(szDeviceName, sizeof(szDeviceName), szXAxis, sizeof(szXAxis), szYAxis, sizeof(szYAxis));

	if (_mbstrlen(szDeviceName) == 0)
	{
		return;
	}

	// Run the console strings
	char strConsole[256];

	if (_mbstrlen(szXAxis) > 0)
	{
		sprintf (strConsole, "scale \"%s\" \"%s\" %f", szDeviceName, szXAxis, 0.00125f + (fSensitivity * 0.001125f));
		pClientDE->RunConsoleString (strConsole);
	}

	if (_mbstrlen(szYAxis) > 0)
	{
		sprintf (strConsole, "scale \"%s\" \"%s\" %f", szDeviceName, szYAxis, 0.00125f + (fSensitivity * 0.001125f));
		pClientDE->RunConsoleString (strConsole);
	}

	sprintf(strConsole, "MouseSensitivity %f", fSensitivity);
	pClientDE->RunConsoleString(strConsole);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::GetMouseDeviceInfo()
//
//	PURPOSE:	Get the mouse device name and axis names
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::GetMouseDeviceInfo(char *lpszDeviceName, int nDeviceBufferSize,
										   char *lpszXAxis,		 int nXAxisBufferSize,
										   char *lpszYAxis,		 int nYAxisBufferSize)
{
	// Most of this code was taken from Shogo...
	CClientDE *pClientDE=GetClientDE();
	if (!pClientDE)
	{
		return;
	}

	// get the mouse device name	
	DRESULT result = pClientDE->GetDeviceName (DEVICETYPE_MOUSE, lpszDeviceName, nDeviceBufferSize);
	if (result == LT_OK)
	{		
		DeviceObject* pList = pClientDE->GetDeviceObjects (DEVICETYPE_MOUSE);
		DeviceObject* ptr = pList;
		while (ptr)
		{
			if (ptr->m_ObjectType == CONTROLTYPE_XAXIS)
			{
				_mbsncpy((unsigned char*)lpszXAxis, (const unsigned char*)ptr->m_ObjectName, nXAxisBufferSize);				
			}

			if (ptr->m_ObjectType == CONTROLTYPE_YAXIS)
			{
				_mbsncpy((unsigned char*)lpszYAxis, (const unsigned char*)ptr->m_ObjectName, nYAxisBufferSize);				
			}

			ptr = ptr->m_pNext;
		}
		if (pList)
		{
			pClientDE->FreeDeviceObjects (pList);		
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::GetMouseSensitivity()
//
//	PURPOSE:	Gets the mouse sensitivity
//
// ----------------------------------------------------------------------- //

float CBloodClientShell::GetMouseSensitivity()
{
	CClientDE *pClientDE=GetClientDE();
	if (!pClientDE)
	{
		return 1.5f;
	}

	// Load the mouse sensitivity
	HCONSOLEVAR hVar=pClientDE->GetConsoleVar("MouseSensitivity");
	if (hVar)
	{
		return pClientDE->GetVarValueFloat(hVar);
	}
	else
	{
		return 1.5f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SetUseJoystick()
//
//	PURPOSE:	Turns the "UseJoystick" console variable on and off
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::SetUseJoystick(DBOOL bUseJoystick)
{
	m_bUseJoystick=bUseJoystick;

	CClientDE *pClientDE=GetClientDE();
	if (!pClientDE)
	{
		return;
	}

	if (m_bUseJoystick)
	{
		pClientDE->RunConsoleString("+usejoystick 1");		// probably don't need this, should remove and just use joystickdisable everywhere!
		pClientDE->RunConsoleString("+joystickdisable 0");  // the engine looks at this one
	}
	else
	{
		pClientDE->RunConsoleString("+usejoystick 0");		// probably don't need this, should remove and just use joystickdisable everywhere!
		pClientDE->RunConsoleString("+joystickdisable 1");  // the engine looks at this one
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SetKeyboardTurnRate()
//
//	PURPOSE:	Sets the keyboard turn rate
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::SetKeyboardTurnRate(float fRate)
{
	m_fKeyboardTurnRate=fRate;

	CClientDE *pClientDE=GetClientDE();
	if (!pClientDE)
	{
		return;
	}
	
	char szTemp[256];
	sprintf(szTemp, "+KeyboardTurnRate %f", m_fKeyboardTurnRate);
	pClientDE->RunConsoleString(szTemp);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::DoMessageBox()
//
//	PURPOSE:	Puts a message box on the screen.  It is removed when ENTER, ESC,
//				or SPACE is pressed or on the next page flip if bAsync is FALSE.
//
// ----------------------------------------------------------------------- //
void CBloodClientShell::DoMessageBox(char *lpszMessage, DBOOL bAsync)
{	
	// TODO: Make this localizable
	ClientDE *pClientDE=GetClientDE();

	HSTRING hString=pClientDE->CreateString(lpszMessage);
	m_messageBox.SetText(hString);	
	pClientDE->FreeString(hString);

	m_messageBox.SetCommandHandler(&m_messageBoxHandler);

	m_messageBox.RemoveKeys();
	m_messageBox.AddKey(VK_ESCAPE, MESSAGE_BOX_ID_KILL);
	m_messageBox.AddKey(VK_RETURN, MESSAGE_BOX_ID_KILL);
	m_messageBox.AddKey(VK_SPACE, MESSAGE_BOX_ID_KILL);

	m_bInMessageBox=DTRUE;

	if (!bAsync)
	{
		// Render the message box to the screen
		PostUpdate();
		PostUpdate();
		m_bInMessageBox=DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::KillMessageBox()
//
//	PURPOSE:	Kills the message box
//
// ----------------------------------------------------------------------- //
void CBloodClientShell::KillMessageBox()
{
	m_bInMessageBox=DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::ShowInfoBox
//
//	PURPOSE:	Displays an informational message box.
//	Params:		lpszMessage:  The message to be displayed
//				bAddOkButton: TRUE if the box should have an OK button to close
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::ShowInfoBox(char *lpszMessage, DBOOL bAddOkButton)
{
	if (!lpszMessage) return;

	CSPrint(lpszMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::KillInfoBox
//
//	PURPOSE:	Stops the display of any info message box.
//
// ----------------------------------------------------------------------- //

void CBloodClientShell::KillInfoBox()
{
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::HandleFiring()
//
//	PURPOSE:	Checks for firing commands and updates the weapon object
//
// --------------------------------------------------------------------------- //
DBOOL CBloodClientShell::HandleFiring()
{
	ClientDE *pClientDE=GetClientDE();

	DRotation rotGun;
	DVector vRGunPos, vLGunPos, vMyPos;

	HLOCALOBJ hPlayerObj = pClientDE->GetClientObject();
	pClientDE->GetObjectPos(hPlayerObj, &vMyPos);

	VEC_ADD(vRGunPos, vMyPos, cdata.GunMuzzlePos);
	VEC_ADD(vLGunPos, vMyPos, cdata.lGunMuzzlePos);

	ROT_COPY(rotGun, m_Rotation);

	if (!m_bDead && !m_bSpectatorMode)
	{
		DBOOL bFiring = DFALSE;
		DBOOL bAltFiring = DFALSE;

		if (cdata.byFlags & CDATA_CANMOVE)
		{
			bFiring = (m_pMoveMgr->GetControlFlags() & CTRLFLAG_FIRE) != 0;
			bAltFiring = (!bFiring && (m_pMoveMgr->GetControlFlags() & CTRLFLAG_ALTFIRE));
		}

		// If we have duel weapons, make the altfire the same as the primary
		if (m_pWeapon && m_pLWeapon && bAltFiring)
			{ bFiring = DTRUE; bAltFiring = DFALSE; }

		if (m_pWeapon)
			m_pWeapon->UpdateFiringState(&vRGunPos, &rotGun, bFiring, bAltFiring);

		if (m_pLWeapon)
			m_pLWeapon->UpdateFiringState(&vLGunPos, &rotGun, bFiring, bAltFiring);
	}
	else if (m_bDead)
	{
		if (m_pWeapon)		m_pWeapon->CancelFiringState();
		if (m_pLWeapon)		m_pLWeapon->CancelFiringState();

/*		if (m_pWeapon)
			m_pWeapon->UpdateFiringState(&vRGunPos, &rotGun, DFALSE, DFALSE);

		if (m_pLWeapon)
			m_pLWeapon->UpdateFiringState(&vLGunPos, &rotGun, DFALSE, DFALSE);
*/	}

	return DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::CreateWeapon()
//
//	PURPOSE:	Creates a view weapon model.
//
// --------------------------------------------------------------------------- //
CViewWeapon* CBloodClientShell::CreateWeapon(DBYTE byType, DBOOL bLeftHand)
{
	ClientDE *pClientDE = GetClientDE();
	CViewWeapon *w = DNULL;

	switch (byType)
	{
		case WEAP_BERETTA:
			w = new CBeretta;
			break;

		case WEAP_SHOTGUN:
			w = new CShotgun;
			break;

		case WEAP_SNIPERRIFLE:
			w = new CSniperRifle;
			break;

		case WEAP_ASSAULTRIFLE:
			w = new CAssaultRifle;
			break;

		case WEAP_SUBMACHINEGUN:
			w = new CSubMachineGun;
			break;

		case WEAP_FLAREGUN:
			w = new CFlareGun;
			break;

		case WEAP_HOWITZER:
			w = new CHowitzer;
			break;

		case WEAP_BUGSPRAY:
			w = new CBugSpray;
			break;

		case WEAP_NAPALMCANNON:
			w = new CNapalmCannon;
			break;

		case WEAP_MINIGUN:
			w = new CMiniGun;
			break;

		case WEAP_VOODOO:
			w = new CVoodooDoll;
			break;

		case WEAP_ORB:
			w = new COrb;
			break;

		case WEAP_DEATHRAY:
			w = new CDeathRay;
			break;

		case WEAP_LIFELEECH:
			w = new CLifeLeech;
			break;

		case WEAP_TESLACANNON:
			w = new CTeslaCannon;
			break;

		case WEAP_SINGULARITY:
			w = new CSingularity;
			break;

		case WEAP_MELEE:
			w = new CMelee;
			break;

#ifdef _ADD_ON

		case WEAP_COMBATSHOTGUN:
			w = new CCombatShotgun;
			break;

		case WEAP_FLAYER:
			w = new CFlayer;
			break;
#endif

#ifndef _DEMO
		case WEAP_PROXIMITYBOMB:
			w = new CWeapProximityBomb;
			break;

		case WEAP_REMOTEBOMB:
			w = new CWeapRemoteBomb;
			break;

		case WEAP_TIMEBOMB:
			w = new CWeapTimeBomb;
			break;
#endif
	}

	if (w)
		w->Create(pClientDE, byType, bLeftHand);

	return w;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::CreateWeapon()
//
//	PURPOSE:	Creates a view weapon model.
//
// --------------------------------------------------------------------------- //
void CBloodClientShell::SetPowerBarLevel(DFLOAT fPercent)
{
	ClientDE *pClientDE = GetClientDE();

	if((fPercent <= 0.0f) || (fPercent >= 1.0f))
		m_NewStatusBar.TurnOffPowerBar();
	else
		m_NewStatusBar.SetPowerBarLevel(BARTAB_THROW, fPercent);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::PlaySound
//
//	PURPOSE:	Plays the given sound
//
// --------------------------------------------------------------------------- //

HSOUNDDE CBloodClientShell::PlaySound(char* sSound, DBOOL bStream, DBOOL bLoop, DBOOL bGetHandle)
{
	// Sanity checks...

	if (!g_pClientDE) return(NULL);


	// Fill out the structure...

	PlaySoundInfo playSoundInfo;

	PLAYSOUNDINFO_INIT(playSoundInfo);
	playSoundInfo.m_dwFlags = PLAYSOUND_LOCAL;

	if (bStream)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_FILESTREAM;
	}
	if (bLoop)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP;
	}
	if (bGetHandle)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_GETHANDLE;
	}

	_mbsncpy((unsigned char*)playSoundInfo.m_szSoundName, (const unsigned char*)sSound, sizeof(playSoundInfo.m_szSoundName) - 1);

	playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_MEDIUM;


	// Play the sound...

	if(g_pClientDE->PlaySound(&playSoundInfo) != LT_OK )
		return NULL;
	
	return playSoundInfo.m_hSound;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::DisplayStatusBarFlagIcon
//
//	PURPOSE:	Controls the display of the status bar flag icon
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::DisplayStatusBarFlagIcon(DBOOL bDisplay)
{
	// Check if we should turn if off...

	if (!bDisplay)
	{
		m_NewStatusBar.DisplayFlag(MULTIFLAG_NONE);
		return;
	}


	// Figure out which team we're on...

	if (!g_pClientDE) return;

	DDWORD dwLocalID = 0;
	g_pClientDE->GetLocalClientID(&dwLocalID);

	CTeam* pTeam = m_TeamMgr.GetTeamFromPlayerID(dwLocalID);
	if (!pTeam) return;

	BYTE byFlagType = MULTIFLAG_NONE;

	if (pTeam->GetID() == TEAM_1) byFlagType = MULTIFLAG_RED;	// show the other teams flag!
	if (pTeam->GetID() == TEAM_2) byFlagType = MULTIFLAG_BLUE;

	m_NewStatusBar.DisplayFlag(byFlagType);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::AssignFrags
//
//	PURPOSE:	Assigns frags as necessary based on the game options
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::AssignFrags(DDWORD dwLocalID, DDWORD dwVictim, DDWORD dwKiller)
{
	if (dwVictim == dwKiller)
	{
		m_FragInfo.RemoveFrag(dwLocalID, dwKiller);
		m_TeamMgr.AddPlayerFrags(dwKiller, -1);
	}
	else
	{
		if (IsMultiplayerTeamBasedGame())
		{
			if (!m_TeamMgr.IsOnSameTeam(dwKiller, dwVictim))	// not on same team
			{
				if (!IsNetOnlyFlagScores() && !IsNetOnlyGoalScores())
				{
					m_FragInfo.AddFrag(dwLocalID, dwKiller);
					m_TeamMgr.AddPlayerFrags(dwKiller, +1);
				}
			}
			else	// on the same team
			{
				if (IsNetFriendlyFire() && IsNetNegTeamFrags())
				{
					m_FragInfo.RemoveFrag(dwLocalID, dwKiller);
					m_TeamMgr.AddPlayerFrags(dwKiller, -1);
				}
			}
		}
		else
		{
			m_FragInfo.AddFrag(dwLocalID, dwKiller);
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::StartWonkyVision
//
//	PURPOSE:	Turns on wonky vision
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::StartWonkyVision(DFLOAT fWonkyTime, DBOOL bWonkyNoMove)
{
	if (fWonkyTime == 0) return;

	m_fWonkyTime   = fWonkyTime;
	m_bWonkyVision = DTRUE;
	m_bWonkyNoMove = bWonkyNoMove;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::EndWonkyVision
//
//	PURPOSE:	Turns off wonky vision
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::EndWonkyVision()
{
	m_fWonkyTime   = 0;
	m_bWonkyVision = DFALSE;
	m_bWonkyNoMove = DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::SetCharacterInfo
//
//	PURPOSE:	Sets the character info for the given character
//
// --------------------------------------------------------------------------- //

DBOOL CBloodClientShell::SetCharacterInfo(int nCharacter, int nSkin)
{
	m_Config.nCharacter = nCharacter;
	m_Config.nSkin      = nSkin;

	switch (m_Config.nCharacter)
	{
		case CHARACTER_CALEB:
		{
			m_Config.nStrength   = 5;
			m_Config.nSpeed      = 3;
			m_Config.nResistance = 3;
			m_Config.nMagic      = 1;
			break;
		}

		case CHARACTER_OPHELIA:
		{
			m_Config.nStrength   = 2;
			m_Config.nSpeed      = 5;
			m_Config.nResistance = 1;
			m_Config.nMagic      = 4;
			break;
		}

		case CHARACTER_ISHMAEL:
		{
			m_Config.nStrength   = 1;
			m_Config.nSpeed      = 3;
			m_Config.nResistance = 3;
			m_Config.nMagic      = 5;
			break;
		}

		case CHARACTER_GABREILLA:
		{
			m_Config.nStrength   = 5;
			m_Config.nSpeed      = 1;
			m_Config.nResistance = 5;
			m_Config.nMagic      = 1;
			break;
		}

#ifdef _ADDON
		case CHARACTER_M_CULTIST:
		{
			m_Config.nStrength   = 5;
			m_Config.nSpeed      = 3;
			m_Config.nResistance = 3;
			m_Config.nMagic      = 1;
			break;
		}

		case CHARACTER_F_CULTIST:
		{
			m_Config.nStrength   = 2;
			m_Config.nSpeed      = 5;
			m_Config.nResistance = 1;
			m_Config.nMagic      = 4;
			break;
		}

		case CHARACTER_SOULDRUDGE:
		{
			m_Config.nStrength   = 1;
			m_Config.nSpeed      = 3;
			m_Config.nResistance = 3;
			m_Config.nMagic      = 5;
			break;
		}

		case CHARACTER_PROPHET:
		{
			m_Config.nStrength   = 5;
			m_Config.nSpeed      = 1;
			m_Config.nResistance = 5;
			m_Config.nMagic      = 1;
			break;
		}
#endif

		default: return(DFALSE);
	}


	// All done...

	return(DTRUE);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CBloodClientShell::ResetPlayerInventory
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CBloodClientShell::ResetPlayerInventory()
{
	ClientDE *pClientDE=GetClientDE();

	// Clear the weapon info
	for (int i=0;i<CLIENTWEAPONSLOTS;i++)
	{
		m_abyWeaponID[i]	= WEAP_NONE;
		m_abyLWeaponID[i]	= WEAP_NONE;
	}

	// Delete the current viewmodels
	if(m_pWeapon)
	{
		delete m_pWeapon;
		m_pWeapon = DNULL;
	}

	if(m_pLWeapon)
	{
		delete m_pLWeapon;
		m_pLWeapon = DNULL;
	}

	// Reset the in game interface stuff
	m_NewStatusBar.Reset();

	// Send a message to the PlayerObj on the server to clear everything
	HMESSAGEWRITE hMessage = pClientDE->StartMessage(CMSG_RESETPLAYERINVENTORY);
	pClientDE->EndMessage(hMessage);
}