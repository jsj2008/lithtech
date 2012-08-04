// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.cpp
//
// PURPOSE : Implementation of class used to manage the client player
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "CameraFX.h"
#include "CameraOffsetMgr.h"
#include "ClientUtilities.h"
#include "ClientWeaponMgr.h"
#include "FlashLight.h"
#include "GameClientShell.h"	
#include "HeadBobMgr.h"
#include "LeanMgr.h"
#include "MsgIDs.h"
#include "PlayerCamera.h"
#include "VarTrack.h"
#include "VehicleMgr.h"
#include "VolumeBrushFX.h"
#include "TargetMgr.h"
#include "MissionMgr.h"
#include "ClientConnectionMgr.h"
#include "DynamicSectorVolumeFX.h"
#include "TriggerFX.h"
#include "PlayerLureFX.h"
#include "CharacterFX.h"
#include "PolyGridFX.h"
#include "PlayerViewAttachmentMgr.h"
#include "AutoTargetMgr.h"
#include "PlayerBodyMgr.h"
#include "AccuracyMgr.h"
#include "AimMgr.h"
#include "LadderMgr.h"
#include "SpecialMoveMgr.h"
#include "HUDMessageQueue.h"
#include "LTEulerAngles.h"
#include "HUDDistance.h"
#include "BindMgr.h"
#include "CommandIds.h"
#include "SoundFilterDB.h"
#include "ClientDB.h"
#include "SkillDefs.h"
#include "HUDOverlay.h"
#include "sys/win/mpstrconv.h"
#include "SlowMoDB.h"
#include "HUDCrosshair.h"
#include "objectdetector.h"
#include "PickupItemFX.h"
#include "ProjectileFX.h"
#include "LadderFX.h"
#include "TurretFX.h"
#include "SpecialMoveFX.h"
#include "ForensicObjectFX.h"
#include "EntryToolLockFX.h"
#include "PropsDB.h"
#include "GameModeMgr.h"
#include "HUDSlowMo.h"
#include "HUDRadio.h"
#include <algorithm>
#include "lttimeutils.h"
#include "AnimationPropStrings.h"
#include "SurfaceFunctions.h"
#include "PolyGridFX.h"
#include "AccuracyVisualization.h"

class CTFFlagSFX;

CPlayerMgr* g_pPlayerMgr = NULL;
extern ObjectDetector g_iFocusObjectDetector;
extern ObjectDetector g_iAttackPredictionObjectDetector;

#define DEFAULT_CSENDRATE				15.0f
#define DEFAULT_CSENDRATE_MIN			2.0f

extern bool	g_bScreenShotMode;

VarTrack			g_CV_CSendRate;		// The SendRate console variable.
VarTrack			g_CV_CSendRate_Min;	// The SendRate_Min console variable.
VarTrack			g_vtPlayerRotate;	// The PlayerRotate console variable


VarTrack			g_vtCamRotInterpTime;
VarTrack			g_vtScreenFadeInTime;


VarTrack			g_vtNormalTurnRate;
VarTrack			g_vtFastTurnRate;
VarTrack			g_vtLookUpRate;

VarTrack			g_vtModelGlowTime;
VarTrack			g_vtModelGlowMin;
VarTrack			g_vtModelGlowMax;
VarTrack			g_vtActivateOverride;
VarTrack			g_vtCamDamage;
VarTrack			g_vtCamDamagePitch;
VarTrack			g_vtCamDamageYaw;
VarTrack			g_vtCamDamageRoll;
VarTrack			g_vtCamDamageTime1;
VarTrack			g_vtCamDamageTime2;
VarTrack			g_vtCamDamageMinPitch;
VarTrack			g_vtCamDamageMaxPitch;
VarTrack			g_vtCamDamageMinYaw;
VarTrack			g_vtCamDamageMaxYaw;
VarTrack			g_vtCamDamageMinRoll;
VarTrack			g_vtCamDamageMaxRoll;

/**/
VarTrack			g_vtCameraMeleeDamageInfo;

VarTrack			g_vtCameraMeleeDamageTime1;
VarTrack			g_vtCameraMeleeDamageTime2;

VarTrack			g_vtCameraMeleeDamageWave1;
VarTrack			g_vtCameraMeleeDamageWave2;

VarTrack			g_vtCameraMeleeDamageMinPitch;
VarTrack			g_vtCameraMeleeDamageMaxPitch;
VarTrack			g_vtCameraMeleeDamageMinYaw;
VarTrack			g_vtCameraMeleeDamageMaxYaw;
VarTrack			g_vtCameraMeleeDamageMinRoll;
VarTrack			g_vtCameraMeleeDamageMaxRoll;

VarTrack			g_vtCameraMeleeDamageEffectTime;

VarTrack			g_vtCameraMeleeDamageDistortion;
VarTrack			g_vtCameraMeleeDamageDistortionIntensity;

VarTrack			g_vtCameraMeleeDamageOverlay;
VarTrack			g_vtCameraMeleeDamageOverlayBlur;
/**/

VarTrack			g_vtAlwaysHUD;
VarTrack			g_vtDamageFadeRate;
VarTrack			g_vtShowSoundFilterInfo;
VarTrack			g_vtMultiplayerDeathCamMoveTime;
VarTrack			g_vtMultiAttachDeathCamMaxTime;
VarTrack			g_vtWeaponShadow;
VarTrack			g_vtPlayerRotAccurate;

VarTrack			g_vtMouseSloMoDampen;
VarTrack			g_vtMouseMeleeDampen;
VarTrack			g_vtMouseMeleeSloMoFactor;
VarTrack			g_vtMouseDeathDampen;
VarTrack			g_vtMouseAimDampen;
extern VarTrack		g_vtMouseMinInputRate;
extern VarTrack		g_vtMouseMaxInputRate;

VarTrack			g_vtGPadAimEdgeThreshold;
VarTrack			g_vtGPadAimEdgeDelayTime;
VarTrack			g_vtGPadAimEdgeAccelTime;
VarTrack			g_vtGPadAimEdgeMultiplier;
VarTrack			g_vtGPadAimAspectRatio;
VarTrack			g_vtGPadAimSensitivity;
VarTrack			g_vtGPadAimBlurMultiplier;
VarTrack			g_vtGPadQuickTurnActive;
VarTrack			g_vtGPadQuickTurnThreshold;
VarTrack			g_vtGPadQuickTurnMultiplier;
VarTrack			g_vtGPadDoubleTapTime;
VarTrack			g_vtGPadAimPowFn;

extern VarTrack		g_vtUseSoundFilters;

VarTrack			g_vtCamRecoilDebug;
VarTrack			g_vtCamRecoilKick;
VarTrack			g_vtCamRecoilFactor;
VarTrack			g_vtCamRecoilScale;
VarTrack			g_vtCamRecoilMax;
VarTrack			g_vtCamRecoilAimDampen;
VarTrack			g_vtCamRecoilRecover;
VarTrack			g_vtCamRecoilRecoverFactor;

VarTrack			g_vtTrackCamOffset;

VarTrack			g_vtActivationDistance;
VarTrack			g_vtTargetDistance;
VarTrack			g_vtPickupDistance;

VarTrack			g_vtFlashlightWaverSpeedScale;
VarTrack			g_vtFlashlightBeamTransitionTime;
VarTrack			g_vtFlashlightTurnOffDelay;
VarTrack			g_vtFlashlightBattery;

VarTrack			g_vtWeaponLagEnabled;
VarTrack			g_vtWeaponLagFactor;
VarTrack			g_vtWeaponLagReversed;

VarTrack			g_vtContextBlockingEnabled;

VarTrack			g_vtHeadBobTransitionTime;

extern VarTrack		g_vtEnableSimulationLog;

#ifndef _FINAL
	AccuracyVisualization*			g_pPerturbLight;			// light to visualize perturb
#endif


// Utility function for comparing two messages that should probably go in a shared file somewhere.
bool AreMessagesEqual(ILTMessage_Read *pLHS, ILTMessage_Read *pRHS)
{
	if (!pLHS || !pRHS)
		return pLHS == pRHS;

	if (pLHS->Size() != pRHS->Size())
		return false;

	uint32 nLHSPos = pLHS->Tell();
	uint32 nRHSPos = pRHS->Tell();

	pLHS->SeekTo(0);
	pRHS->SeekTo(0);
	bool bEqual = true;
	while (bEqual && !pLHS->EOM())
		bEqual &= pLHS->Readuint32() == pRHS->Readuint32();

	pLHS->SeekTo(nLHSPos);
	pRHS->SeekTo(nRHSPos);

	return bEqual;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ApplySoundFilter()
//
//	PURPOSE:	Utility funciton to handle applying a sound filter.
//
// ----------------------------------------------------------------------- //

void ApplySoundFilter(HRECORD hSoundFilterRecord, bool bGlobal)
{
	if (g_vtUseSoundFilters.GetFloat())
	{
		ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();

		if (NULL == hSoundFilterRecord )
		{
			// If NULL, clear the filter
			pSoundMgr->EnableSoundFilter( false );
		}
		else
		{
			// If not NULL, clear the filter.
			const char* const pszFilterName = SoundFilterDB::Instance().GetFilterName(hSoundFilterRecord);
			const char* const pszFilterRecordName = SoundFilterDB::Instance().GetFilterRecordName(hSoundFilterRecord);

			bool bFilterOK = true;

			if ( !SoundFilterDB::Instance().IsFilterUnFiltered( hSoundFilterRecord ) )
			{
				if ( pSoundMgr->SetSoundFilter( pszFilterName ) == LT_OK )
				{
					pSoundMgr->SetSoundFilterName(pszFilterRecordName);

					for (int i=0; i < SoundFilterDB::Instance().GetFilterParmeterCount(); i++)
					{
						const char* const pszFilterParameterName = SoundFilterDB::Instance().GetFilterParmeterName(hSoundFilterRecord, i);
						float fFilterParameterValue = SoundFilterDB::Instance().GetFilterParmeterValue(hSoundFilterRecord, i);
						if ( pSoundMgr->SetSoundFilterParam(pszFilterParameterName, fFilterParameterValue) != LT_OK )
						{
							bFilterOK = false;
						}
					}
				}
				else
				{
					bFilterOK = false;
				}
			}
			else
			{
				// Not filtered.
				bFilterOK = false;
			}

			pSoundMgr->EnableSoundFilter( bFilterOK );

#ifndef _FINAL
			if (g_vtShowSoundFilterInfo.GetFloat())
			{
				g_pLTClient->CPrint("Entering %s sound filter '%s' (%s)",
					(bGlobal ? "(Global)" : ""),
					pszFilterName, 
					(bFilterOK ? "Enabled" : "Disabled"));

				// Display detailed filter info if necessary...
				if (g_vtShowSoundFilterInfo.GetFloat() > 1)
				{
					g_pLTClient->CPrint("  FilterName: '%s'", pszFilterName);
					for (int i=0; i < SoundFilterDB::Instance().GetFilterParmeterCount(); i++)
					{
						const char* const pszFilterParameterName = SoundFilterDB::Instance().GetFilterParmeterName(hSoundFilterRecord, i);
						float fFilterParameterValue = SoundFilterDB::Instance().GetFilterParmeterValue(hSoundFilterRecord, i);
						g_pLTClient->CPrint("  '%s' = '%f'", pszFilterParameterName, fFilterParameterValue);
					}
				}
			}
#endif // _FINAL
		}
	}
}

// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CPlayerMgr::CPlayerMgr()
:	m_bStrafing						( false ),
	m_bRestoreOrientation			( false ),
	m_bAllowPlayerMovement			( true ),
	m_bLastAllowPlayerMovement		( true ),
	m_bStartedPlaying				( false ),
	m_eSpectatorMode				( eSpectatorMode_None ),
	m_bInvisibleMode				( false ),
	m_fFireJitterPitch				( 0.0f ),
	m_fFireJitterYaw				( 0.0f ),
	m_fCurrentRecoil				( 0.0f ),
	m_dwPlayerFlags					( 0 ),
	m_ePlayerState					( ePlayerState_None ),
	m_eLastSonicType				( eSonicType_None ),
	m_eCurContainerCode				( CC_NO_CONTAINER ),
	m_hSoundFilterIdCurrentContainer( NULL ),
	m_hSoundFilterIdGlobal			( NULL ),
	m_hSoundFilterIDOverride		( NULL ),
	m_pSoundFilterIDCurrent			( NULL ),
	m_bSoundFilterOverrideOn		( false ),
	m_bInSafetyNet					( false ),
	m_hCurContainerObject			( NULL ),
	m_vCurModelGlow					( 127.0f, 127.0f, 127.0f ),
	m_vMaxModelGlow					( 255.0f, 255.0f, 255.0f ),
	m_vMinModelGlow					( 50.0f, 50.0f, 50.f ),
	m_fModelGlowCycleTime			( 0.0f ),
	m_bModelGlowCycleUp				( true ),
	m_bPlayerUpdated				( false ),
	m_nPlayerInfoChangeFlags		( 0 ),
	m_fPlayerInfoLastSendTime		( 0.0f ),
	m_bUseWorldFog					( true ),
	m_hContainerSound				( NULL ),
	m_bStartedDuckingDown			( false ),
	m_bStartedDuckingUp				( false ),
	m_fCamDuck						( 0.0f ),
	m_fDuckDownV					( -75.0f ),
	m_fDuckUpV						( 75.0f ),
	m_fMaxDuckDistance				( -20.0f ),
	m_fDistanceIndicatorPercent		( -1.0f ),
	m_hPreGrenadeWeapon				( NULL ),
	m_bChangingToGrenade			( false ),
	m_bSwitchingWeapons				( false ),
	m_bServerAccurateRotation		( false ),
	m_fPlayerInfoLastUniqueSendTime	( 0.0f ),
	m_bManualAim					( false ),
	m_bRespawnRequested				( false ),
	m_bRespawnWhenTimedOut			( false ),
	m_fRespawnPenalty				( 0.0f ),
	m_v2AxisOffsets					( 0.0f, 0.0f ),
	m_bShouldEnableBody				( false ),
	m_fMeleeDamageDistortionIntensity	( 0.0f ),
	m_fMeleeDamageDistortionI1		( 0.0f ),
	m_fMeleeDamageOverlayBlur		( 0.0f ),
	m_fMeleeDamageOverlayB1			( 0.0f ),
	m_hSlowMoRecord					( NULL ),
	m_eSlowMoState					( eEffectState_Out ),
	m_hDeathSound					( NULL ),
	m_hKiller						( ),
	m_bBlocked						( false ),
	m_bSemiAutoFire					( false ),
	m_bSemiAutoFireDuringReload		( false ),
	m_fSemiAutoFireTime				( 0.0f ),
	m_pTurretFX						( NULL ),
	m_pForensicObject				( NULL ),
	m_hCarryObject					( NULL ),
	m_fFlashlightButtonPressTime	( -1.0f ),
	m_fFlashlightTransitionTime		( -1.0f ),
	m_bFaceNodeForwardPitch			( true ),
	m_bFaceNodeForwardYaw			( true ),
	m_bAlignPitchWithNode			( true ),
	m_bAlignYawWithNode				( true ),
	m_bAtNodePosition				( false ),
	m_bAtNodeRotation				( false ),
	m_fFocusAccuracy				( 0.0f ),
	m_bStoryMode					( false ),
	m_bCanSkipStory					( true ),
	m_hSyncObject					( NULL )
{
	m_pHeadBobMgr		= debug_new( HeadBobMgr );					LTASSERT( m_pHeadBobMgr, "Failed to allocate HeadBobMgr!" );
	m_pCameraOffsetMgr	= debug_new( CCameraOffsetMgr );			LTASSERT( m_pCameraOffsetMgr, "Failed to allocate CCameraOffsetMgr!" );
	m_pFlashLight		= debug_new( Flashlight );					LTASSERT( m_pFlashLight, "Failed to allocate CFlashLightPlayer!" );
	m_pMoveMgr			= debug_new( CMoveMgr );					LTASSERT( m_pMoveMgr, "Failed to allocate CMoveMgr!" );
	m_pLeanMgr			= debug_new( CLeanMgr );					LTASSERT( m_pLeanMgr, "Failed to allocate CLeanMgr!" );
	m_pClientWeaponMgr	= debug_new( CClientWeaponMgr );			LTASSERT( m_pClientWeaponMgr, "Failed to allocate CClientWeaponMgr!" );
	m_pPlayerCamera		= debug_new( CPlayerCamera );				LTASSERT( m_pPlayerCamera, "Failed to allocate CPlayerCamera!" );
	m_pPVAttachmentMgr	= debug_new( CPlayerViewAttachmentMgr );	LTASSERT( m_pPVAttachmentMgr, "Failed to allocate CPlayerViewAttachmentMgr!" );

#ifndef _FINAL
	g_pPerturbLight		= debug_new( AccuracyVisualization );
#endif
}

CPlayerMgr::~CPlayerMgr()
{
	if ( m_pHeadBobMgr )
	{
		debug_delete( m_pHeadBobMgr );
		m_pHeadBobMgr = 0;
	}

	if ( m_pCameraOffsetMgr )
	{
		debug_delete( m_pCameraOffsetMgr );
		m_pCameraOffsetMgr = 0;
	}

	if ( m_pFlashLight )
	{
		debug_delete( m_pFlashLight );
		m_pFlashLight = 0;
	}

	if ( m_pMoveMgr )
	{
		debug_delete( m_pMoveMgr );
		m_pMoveMgr = 0;
	}

	if ( m_pLeanMgr )
	{
		debug_delete( m_pLeanMgr );
		m_pLeanMgr = 0;
	}

	if ( m_pClientWeaponMgr )
	{
		debug_delete( m_pClientWeaponMgr );
		m_pClientWeaponMgr = 0;
	}

	if ( m_pPlayerCamera )
	{
		debug_delete( m_pPlayerCamera );
		m_pPlayerCamera = 0;
	}

	if( m_pPVAttachmentMgr )
	{
		debug_delete( m_pPVAttachmentMgr );
		m_pPVAttachmentMgr = NULL;
	}

#ifndef _FINAL
	if ( g_pPerturbLight )
	{
		debug_delete( g_pPerturbLight );
		g_pPerturbLight = NULL;
	}
#endif

	SetCarryObject( NULL );

	g_pPlayerMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CPlayerMgr::Init()
{
	g_pPlayerMgr = this;

	InitTargetMgr();

	g_CV_CSendRate.Init(g_pLTClient, "CSendRate", NULL, DEFAULT_CSENDRATE);
	g_CV_CSendRate_Min.Init(g_pLTClient, "CSendRate_Min", NULL, DEFAULT_CSENDRATE_MIN);
	g_vtPlayerRotate.Init(g_pLTClient, "PlayerRotate", NULL, 1.0f);

	g_vtCamRotInterpTime.Init(g_pLTClient, "CamRotInterpTime", NULL, 0.15f);
	g_vtScreenFadeInTime.Init(g_pLTClient, "ScreenFadeInTime", NULL, 3.0f);

	g_vtFastTurnRate.Init(g_pLTClient, "FastTurnRate", NULL, 2.3f);
	g_vtNormalTurnRate.Init(g_pLTClient, "NormalTurnRate", NULL, 1.5f);
	g_vtLookUpRate.Init(g_pLTClient, "LookUpRate", NULL, 2.5f);
	g_vtModelGlowTime.Init(g_pLTClient, "ModelGlowTime", NULL, 1.5f);
	g_vtModelGlowMin.Init(g_pLTClient, "ModelGlowMin", NULL, -25.0f);
	g_vtModelGlowMax.Init(g_pLTClient, "ModelGlowMax", NULL, 75.0f);
	g_vtActivateOverride.Init(g_pLTClient, "ActivateOverride", " ", 0.0f);
	g_vtCamDamage.Init(g_pLTClient, "CamDamage", NULL, 1.0f);

	g_vtCamDamagePitch.Init(g_pLTClient, "CamDamagePitch", NULL, 1.0f);
	g_vtCamDamageYaw.Init( g_pLTClient, "CamDamageYaw", NULL, 1.0f );
	g_vtCamDamageRoll.Init(g_pLTClient, "CamDamageRoll", NULL, 1.0f);
	g_vtCamDamageTime1.Init(g_pLTClient, "CamDamageTime1", NULL, 0.1f);
	g_vtCamDamageTime2.Init(g_pLTClient, "CamDamageTime2", NULL, 0.25f);

	g_vtCamDamageMinPitch.Init(g_pLTClient, "CamDamageMinPitch", NULL, 2.0f);
	g_vtCamDamageMaxPitch.Init(g_pLTClient, "CamDamageMaxPitch", NULL, 3.0f);
	g_vtCamDamageMinYaw.Init(g_pLTClient, "CamDamageMinYaw", NULL, 2.0f);
	g_vtCamDamageMaxYaw.Init(g_pLTClient, "CamDamageMaxYaw", NULL, 3.0f);
	g_vtCamDamageMinRoll.Init(g_pLTClient, "CamDamageMinRoll", NULL, 2.0f);
	g_vtCamDamageMaxRoll.Init(g_pLTClient, "CamDamageMaxRoll", NULL, 3.0f);

	// Melee Damage...
	g_vtCameraMeleeDamageInfo.Init( g_pLTClient, "CameraMeleeDamageInfo", NULL, 0.0f );

	g_vtCameraMeleeDamageTime1.Init(g_pLTClient, "CameraMeleeDamageTime1", NULL, 0.05f);
	g_vtCameraMeleeDamageTime2.Init(g_pLTClient, "CameraMeleeDamageTime2", NULL, 0.5f);

	g_vtCameraMeleeDamageWave1.Init( g_pLTClient, "CameraMeleeDamageWave1", NULL, 2.0f ); // SlowOff
	g_vtCameraMeleeDamageWave2.Init( g_pLTClient, "CameraMeleeDamageWave2", NULL, 3.0f ); // SlowOn

	g_vtCameraMeleeDamageMinPitch.Init( g_pLTClient, "CameraMeleeDamageMinPitch", NULL, 20.0f );
	g_vtCameraMeleeDamageMaxPitch.Init( g_pLTClient, "CameraMeleeDamageMaxPitch", NULL, 35.0f );
	g_vtCameraMeleeDamageMinYaw.Init( g_pLTClient, "CameraMeleeDamageMinYaw", NULL, 30.0f );
	g_vtCameraMeleeDamageMaxYaw.Init( g_pLTClient, "CameraMeleeDamageMaxYaw", NULL, 45.0f );
	g_vtCameraMeleeDamageMinRoll.Init( g_pLTClient, "CameraMeleeDamageMinRoll", NULL, 20.0f );
	g_vtCameraMeleeDamageMaxRoll.Init( g_pLTClient, "CameraMeleeDamageMaxRoll", NULL, 35.0f );

	g_vtCameraMeleeDamageEffectTime.Init( g_pLTClient, "CameraMeleeDamageEffectTime", NULL, 1.5f );

	g_vtCameraMeleeDamageDistortion.Init( g_pLTClient, "CameraMeleeDamageDistortion", NULL, 1.0f );
	g_vtCameraMeleeDamageDistortionIntensity.Init( g_pLTClient, "CameraMeleeDamageDistortionIntensity", NULL, 0.5f );

	g_vtCameraMeleeDamageOverlay.Init( g_pLTClient, "CameraMeleeDamageOverlay", NULL, 1.0f );
	g_vtCameraMeleeDamageOverlayBlur.Init( g_pLTClient, "CameraMeleeDamageOverlayBlur", NULL, 0.5f );

	g_vtAlwaysHUD.Init(g_pLTClient, "AlwaysHUD", NULL, 0.0f);
	g_vtDamageFadeRate.Init(g_pLTClient, "DamageFadeRate", NULL, 0.5f);
	g_vtShowSoundFilterInfo.Init(g_pLTClient, "SoundFilterInfo", NULL, 0.0f);
	g_vtMultiplayerDeathCamMoveTime.Init( g_pLTClient, "MultiplayerDeathCamMoveTime", NULL, 0.5f );
	g_vtMultiAttachDeathCamMaxTime.Init( g_pLTClient, "MultiplayerDeathAttachCamMaxTime", NULL, 5.0f );
	g_vtWeaponShadow.Init( g_pLTClient, "WeaponShadow", NULL, 0.0f );
	g_vtPlayerRotAccurate.Init( g_pLTClient, "PlayerRotAccurate", NULL, 0.0f );

	g_vtMouseSloMoDampen.Init( g_pLTClient, "MouseSloMoDampen", NULL, 1.0f );
	g_vtMouseMeleeDampen.Init( g_pLTClient, "MouseMeleeDampen", NULL, 0.6f );
	g_vtMouseMeleeSloMoFactor.Init( g_pLTClient, "MouseMeleeSloMoFactor", NULL, 0.8f );
	g_vtMouseDeathDampen.Init( g_pLTClient, "MouseDeathDampen", NULL, 0.0f );
	g_vtMouseAimDampen.Init( g_pLTClient, "MouseAimDampen", NULL, 0.8f );
	
	g_vtCamRecoilDebug.Init( g_pLTClient, "CamRecoilDebug", NULL, 0.0f );

	/***Recoil settings for use with linear recoil curve*/
	g_vtCamRecoilKick.Init( g_pLTClient, "CamRecoilKick", NULL, -1.0f );
	g_vtCamRecoilFactor.Init( g_pLTClient, "CamRecoilFactor", NULL, 0.2f );
	g_vtCamRecoilScale.Init( g_pLTClient, "CamRecoilScale", NULL, 0.2f );
	g_vtCamRecoilMax.Init( g_pLTClient, "CamRecoilMax", NULL, 5.0f );
	g_vtCamRecoilAimDampen.Init( g_pLTClient, "CamRecoilAimDampen", NULL, 1.0f );
	g_vtCamRecoilRecover.Init( g_pLTClient, "CamRecoilRecover", NULL, -1.0f );
	g_vtCamRecoilRecoverFactor.Init( g_pLTClient, "CamRecoilRecoverFactor", NULL, 0.75f );
	/***/

	g_vtGPadAimEdgeThreshold.Init( g_pLTClient, "GPadAimEdgeThreshold", NULL, 0.75f);
	g_vtGPadAimEdgeAccelTime.Init( g_pLTClient, "GPadAimEdgeAccelTime", NULL, 1.0f);
	g_vtGPadAimEdgeDelayTime.Init( g_pLTClient, "GPadAimEdgeDelayTime", NULL, 0.25f);
	g_vtGPadAimEdgeMultiplier.Init( g_pLTClient, "GPadAimEdgeMultiplier", NULL, 1.6f);
	g_vtGPadAimAspectRatio.Init( g_pLTClient, "GPadAimAspectRatio", NULL, 1.0f);
	g_vtGPadAimSensitivity.Init( g_pLTClient, "GPadAimSensitivity", NULL, 2.0f);
	g_vtGPadAimBlurMultiplier.Init( g_pLTClient, "GPadAimBlurMultiplier", NULL, 0.25f);
	g_vtGPadQuickTurnActive.Init( g_pLTClient, "GPadQuickTurnActive", NULL, 0.0f); 
	g_vtGPadQuickTurnThreshold.Init( g_pLTClient, "GPadQuickTurnThreshold", NULL, 0.6f);
	g_vtGPadQuickTurnMultiplier.Init( g_pLTClient, "GPadQuickTurnMultiplier", NULL, 6.0f);
	g_vtGPadDoubleTapTime.Init( g_pLTClient, "GPadDoubleTapTime", NULL, 0.4f);
	g_vtGPadAimPowFn.Init( g_pLTClient, "GPadAimPowFn", NULL, 2.0f);

	g_vtTrackCamOffset.Init( g_pLTClient, "TrackCamOffset", NULL, 100.0f );

	if (!g_vtActivationDistance.IsInitted())
	{
		float f = ClientDB::Instance( ).GetActivationDistance();
		g_vtActivationDistance.Init(g_pLTClient, "ActivationDistance", NULL, f);
	}

	if( !g_vtTargetDistance.IsInitted() )
	{
		float f = ClientDB::Instance( ).GetTargetDistance();
		g_vtTargetDistance.Init(g_pLTClient, "TargetDistance", NULL, f);
	}

	if (!g_vtPickupDistance.IsInitted())
	{
		float f = ClientDB::Instance( ).GetPickupDistance();
		g_vtPickupDistance.Init(g_pLTClient, "PickupDistance", NULL, f);
	}

	g_vtFlashlightWaverSpeedScale.Init( g_pLTClient, "FlashlightWaverSpeedScale", NULL, 3.5f );
	g_vtFlashlightBeamTransitionTime.Init( g_pLTClient, "FlashlightBeamTransitionTime", NULL, 0.33f );
	g_vtFlashlightTurnOffDelay.Init( g_pLTClient, "FlashlightTurnOffDelay", NULL, 0.5f );
	g_vtFlashlightBattery.Init( g_pLTClient, "FlashlightBattery", NULL, 1.0f );

	g_vtWeaponLagEnabled.Init( g_pLTClient, "WeaponLagEnabled", NULL, 1.0f );
	g_vtWeaponLagFactor.Init( g_pLTClient, "WeaponLagFactor", NULL, 0.33f );
	g_vtWeaponLagReversed.Init( g_pLTClient, "WeaponLagReversed", NULL, 0.0f );

	g_vtContextBlockingEnabled.Init( g_pLTClient, "ContextBlockingEnabled", NULL, 1.0f );

	g_vtHeadBobTransitionTime.Init( g_pLTClient, "HeadBobTransitionTime", NULL, 0.2f );


	//
	// Initialize all the managers...
	//

	m_pHeadBobMgr->Initialize();
	m_pCameraOffsetMgr->Init();
	//m_pFlashLight no init
	m_pMoveMgr->Init();

    if( !m_pClientWeaponMgr->Init( ))
	{
		LTERROR( "Failed to initialize Client Weapon Mgr!" );
		return false;
	}

	m_pPVAttachmentMgr->Init();

	CPlayerBodyMgr::Instance( ).Init( );
	CAccuracyMgr::Instance( ).Init( );
	AimMgr::Instance( ).Init( );
	LadderMgr::Instance().Init();
	SpecialMoveMgr::Instance().Init();

	// Player camera...
	if( m_pPlayerCamera->Init( ))
	{
		m_pPlayerCamera->SetCameraMode( CPlayerCamera::kCM_FirstPerson );
	}
	else
	{
		LTERROR( "CPlayerMgr::Init() - Failed to init PlayerCamera" );
		return false;
	}

	ClearDamageSectors();

	// Setup the focus detector
	g_iFocusObjectDetector.SetBehaviorFlags( ObjectDetector::ODBF_ACQUIREFOV | ObjectDetector::ODBF_ACQUIRELINEOFSITE |
											ObjectDetector::ODBF_INCLUSIVEVERIFY | ObjectDetector::ODBF_VERIFYFOV | ObjectDetector::ODBF_VERIFYLINEOFSITE );
	g_iFocusObjectDetector.SetParamsFOV( 80.0f, 64.0f, 10.0f, 2000.0f, 0.25f, 0.5f, 0.25f );
	g_iFocusObjectDetector.SetVerifyFailureDelay( ObjectDetector::ODBF_VERIFYLINEOFSITE, 3.0f );
	g_iFocusObjectDetector.SetSpatialFn( FocusObjectDetectorSpatialCB, NULL );

	// Setup the attack prediction detector
	g_iAttackPredictionObjectDetector.SetBehaviorFlags( ObjectDetector::ODBF_ACQUIREFOV | ObjectDetector::ODBF_INCLUSIVEVERIFY | ObjectDetector::ODBF_VERIFYFOV );
	g_iAttackPredictionObjectDetector.SetParamsFOV( 80.0f, 180.0f, 10.0f, 150.0f, 0.25f, 0.5f, 0.25f );

	// Setup the pickup object detector
	m_PickupObjectDetector.SetBehaviorFlags( ObjectDetector::ODBF_ACQUIREFOV | ObjectDetector::ODBF_ACQUIRELINEOFSITE | ObjectDetector::ODBF_ACQUIRECUSTOM | ObjectDetector::ODBF_INCLUSIVEACQUIRE );
	m_PickupObjectDetector.SetParamsFOV( 30.0f, 120.0f, 0.0f, g_vtPickupDistance.GetFloat(), 1.0f, 1.0f, 1.0f );
	m_PickupObjectDetector.SetCustomTestFn( PickupObjectDetectorCustomTestCB, NULL );

	// Setup the forensic object detector
	m_ForensicObjectDetector.SetBehaviorFlags( ObjectDetector::ODBF_ACQUIRECUSTOM );
	m_ForensicObjectDetector.SetCustomTestFn( ForensicObjectDetectorCustomTestCB, NULL );

	m_HeartBeat.Init( "HeartBeat" );
	m_Breath.Init( "Breath" );

	m_tmrJoinGrace.SetEngineTimer( RealTimeTimer::Instance( ));

	m_SlowMoTimer.SetEngineTimer( GameTimeTimer::Instance( ));

	m_EarliestRespawnTime.SetEngineTimer( RealTimeTimer::Instance( ));

	m_fFlashlightMaxCharge = 0.0f;
	m_fFlashlightCharge = 0.0f;
	m_fFlashlightRecharge = 0.0f;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CPlayerMgr::Term()
{
	// terminate the weapon mgr
	m_pClientWeaponMgr->Term();

	m_pPVAttachmentMgr->Term();

	g_pPlayerMgr = NULL;

	m_cLastPlayerInfoMsg = NULL;
	m_cLastPlayerInfoMoveMsg = NULL;
	m_cLastPlayerInfoAnimMsg = NULL;

	// Get rid of any object being carried...
	SetCarryObject( NULL );


}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Save
//
//	PURPOSE:	Save the player info
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	// save the class information
	m_pMoveMgr->Save(pMsg, eSaveDataState);
	m_pClientWeaponMgr->Save(pMsg);
	m_pPlayerCamera->Save( pMsg );
	m_fxSlowMoSequence.Save( *pMsg );
	
	CPlayerBodyMgr::Instance( ).Save( pMsg );

	pMsg->Writeuint8(m_eSpectatorMode);
	pMsg->Writebool(m_bLastSent3rdPerson != false);
	pMsg->Writebool(m_bAllowPlayerMovement != false);
	pMsg->Writebool(m_bLastAllowPlayerMovement != false);
	pMsg->Writeuint8(( uint8 )m_ePlayerState);
	pMsg->WriteDatabaseRecord(g_pLTDatabase, m_hSoundFilterIdCurrentContainer);
	pMsg->WriteDatabaseRecord(g_pLTDatabase, m_hSoundFilterIdGlobal);
	pMsg->WriteDatabaseRecord(g_pLTDatabase, m_hSoundFilterIDOverride);
	pMsg->Writebool(m_bSoundFilterOverrideOn);

	pMsg->Writebool(m_pFlashLight->IsOn() != false);

	pMsg->Writebool(m_bBlocked);

    pMsg->Writefloat(m_fFireJitterPitch);
    pMsg->Writefloat(m_fFireJitterYaw);

	pMsg->Writebool( m_pMoveMgr->DuckLock() ? true : false );

	pMsg->Writebool( m_pMoveMgr->IsDucking() ? true : false );

	m_pFlashLight->Save(pMsg, eSaveDataState);

	pMsg->WriteObject( m_hPlayerNodeGoto );
	pMsg->Writebool( m_bFaceNodeForwardPitch );
	pMsg->Writebool( m_bFaceNodeForwardYaw );
	pMsg->Writebool( m_bAlignPitchWithNode );
	pMsg->Writebool( m_bAlignYawWithNode );

	pMsg->Writebool( m_bSlowMoPlayer );
	pMsg->Writedouble( m_fSlowMoCurCharge );
	pMsg->Writefloat( m_fSlowMoMinCharge );
	pMsg->Writefloat( m_fSlowMoMaxCharge );
	pMsg->Writefloat( m_fSlowMoRecharge );
	pMsg->Writeuint32( m_eSlowMoState );
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hSlowMoRecord );
	pMsg->Writedouble( m_SlowMoTimer.GetTimeLeft( ));
	
	pMsg->Writedouble( m_fFlashlightCharge );
	pMsg->Writefloat( m_fFlashlightMaxCharge );
	pMsg->Writefloat( m_fFlashlightRecharge );

	m_HeartBeat.Save(pMsg);
	m_Breath.Save(pMsg);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Load
//
//	PURPOSE:	Load the interface info
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState)
{
	// load the contained member classes
	m_pMoveMgr->Load(pMsg, eLoadDataState);
	m_pClientWeaponMgr->Load(pMsg);
	m_pPlayerCamera->Load( pMsg );
	m_fxSlowMoSequence.Load( *pMsg );

	CPlayerBodyMgr::Instance( ).Load( pMsg );

	m_eSpectatorMode			= ( SpectatorMode )pMsg->Readuint8();
	m_bLastSent3rdPerson		= pMsg->Readbool() ? true : false;
	m_bAllowPlayerMovement		= pMsg->Readbool() ? true : false;
	m_bLastAllowPlayerMovement	= pMsg->Readbool() ? true : false;
	m_ePlayerState              = (PlayerState) pMsg->Readuint8();

	m_hSoundFilterIdCurrentContainer = pMsg->ReadDatabaseRecord(g_pLTDatabase, SoundFilterDB::Instance().GetSoundFilterCategory());
	m_hSoundFilterIdGlobal = pMsg->ReadDatabaseRecord(g_pLTDatabase, SoundFilterDB::Instance().GetSoundFilterCategory());
	m_hSoundFilterIDOverride = pMsg->ReadDatabaseRecord(g_pLTDatabase, SoundFilterDB::Instance().GetSoundFilterCategory());
	m_bSoundFilterOverrideOn = pMsg->Readbool();

	if (pMsg->Readbool())
	{
		m_pFlashLight->TurnOn();
	}

	m_bBlocked = pMsg->Readbool();

    m_fFireJitterPitch          = pMsg->Readfloat();
    m_fFireJitterYaw	        = pMsg->Readfloat();

	m_pMoveMgr->SetDuckLock( pMsg->Readbool() ? true : false );

	m_pMoveMgr->SetDucking( pMsg->Readbool() ? true : false );

	m_pFlashLight->Load(pMsg, eLoadDataState);

	m_hPlayerNodeGoto		= pMsg->ReadObject( );
	m_bFaceNodeForwardPitch	= pMsg->Readbool( );
	m_bFaceNodeForwardYaw	= pMsg->Readbool( );
	m_bAlignPitchWithNode	= pMsg->Readbool( );
	m_bAlignYawWithNode		= pMsg->Readbool( );

	m_bSlowMoPlayer	= pMsg->Readbool( );
	m_fSlowMoCurCharge = pMsg->Readdouble();
	m_fSlowMoMinCharge = pMsg->Readfloat();
	m_fSlowMoMaxCharge = pMsg->Readfloat();
	m_fSlowMoRecharge = pMsg->Readfloat();
	m_eSlowMoState = static_cast<EffectState>(pMsg->Readuint32( ));
	m_hSlowMoRecord = pMsg->ReadDatabaseRecord( g_pLTDatabase, DATABASE_CATEGORY( SlowMo ).GetCategory( ));
	m_SlowMoTimer.Start( pMsg->Readdouble( ));

	m_fFlashlightCharge = pMsg->Readdouble();
	m_fFlashlightMaxCharge = pMsg->Readfloat();
	m_fFlashlightRecharge = pMsg->Readfloat();

	// Tell the clientfx about the slowmotion state.
	g_pGameClientShell->GetSimulationTimeClientFXMgr().SetInSlowMotion( (m_eSlowMoState != eEffectState_Out) );
	g_pGameClientShell->GetRealTimeClientFXMgr().SetInSlowMotion( (m_eSlowMoState != eEffectState_Out) );

	m_HeartBeat.Load(pMsg);
	m_Breath.Load(pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnEnterWorld()
//
//	PURPOSE:	Handle entering world
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::OnEnterWorld()
{
	m_ePlayerState			= ePlayerState_None;
	m_eSpectatorMode		= eSpectatorMode_None;
	m_bPlayerUpdated		= false;
	m_eCurContainerCode		= CC_NO_CONTAINER;
	m_bInSafetyNet			= false;
	m_hCurContainerObject	= NULL;
	m_bStoryMode			= false;
	m_bCanSkipStory			= true;

	// Check if the game has grace period on when we can join in.  If 
	// we miss it we'll be stuck in spectator mode.
	if( GameModeMgr::Instance( ).m_grnJoinGracePeriod > 0 )
	{
		// Get the time in the level so far.
		double fCurGameTime = g_pLTClient->GetGameTime();
		if( GameModeMgr::Instance( ).m_grnJoinGracePeriod > fCurGameTime )
		{
			m_tmrJoinGrace.Start(( float )GameModeMgr::Instance( ).m_grnJoinGracePeriod - fCurGameTime );
		}
	}

	m_pHeadBobMgr->OnEnterWorld();
	m_pPlayerCamera->OnEnterWorld();
	
	m_nPlayerInfoChangeFlags |= CLIENTUPDATE_CAMERAINFO | CLIENTUPDATE_ALLOWINPUT;

	m_pMoveMgr->OnEnterWorld();

	m_fDistanceIndicatorPercent = -1.0f;
	g_pHUDMgr->QueueUpdate( kHUDDistance );

	extern CHUDDistance* g_pDistance;
	if( g_pDistance )
	{
		g_pDistance->Clear();
	}

	m_pClientWeaponMgr->OnEnterWorld();

#ifdef PROJECT_DARK

	m_pFlashLight->Enable( true, 0.0f );
	m_pFlashLight->Initialize( m_pPlayerCamera->GetCamera(), "Player_Wide" );

#else//PROJECT_DARK

	m_pFlashLight->Enable( true, 0.0f );
	m_pFlashLight->Initialize( m_pPlayerCamera->GetCamera(), "Player_Narrow" );

#ifndef _FINAL
	g_pPerturbLight->Enable(true );
	g_pPerturbLight->Initialize( m_pPlayerCamera->GetCamera());
#endif

#endif//PROJECT_DARK
	m_fFlashlightMaxCharge = m_pFlashLight->GetMaxCharge();
	m_fFlashlightRecharge = m_pFlashLight->GetRechargeRate();
	m_fFlashlightCharge = m_fFlashlightMaxCharge;

	m_hSoundFilterIDOverride = NULL;
	m_pSoundFilterIDCurrent = NULL;
	m_bSoundFilterOverrideOn = false;

	m_hPlayerNodeGoto = NULL;
	m_hSpectatorFollowCharacter = NULL;
	m_hSpectatorTrackTarget = NULL;

	AllowPlayerMovement( true );

	m_EarliestRespawnTime.Stop();
	m_bRespawnWhenTimedOut = false;
	m_fRespawnPenalty = 0.0f;




}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::PreExitWorld()
//
//	PURPOSE:	Get ready to leave the world (
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::PreExitWorld()
{
	m_HeartBeat.Reset();
	m_Breath.Reset();
	ClearPlayerModes();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnExitWorld()
//
//	PURPOSE:	Handle leaving world
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::OnExitWorld()
{
	m_bStartedPlaying = false;

	m_ePlayerState = ePlayerState_None;
	m_eSpectatorMode = eSpectatorMode_None;

	// Detatch camera...
	m_pPlayerCamera->SetTargetObject( NULL );

	m_pFlashLight->TurnOff();
	m_pMoveMgr->OnExitWorld();
	CPlayerBodyMgr::Instance( ).OnExitWorld();
	CAccuracyMgr::Instance( ).Reset();
	
	if (m_hContainerSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hContainerSound);
		m_hContainerSound = NULL;
	}

	m_pClientWeaponMgr->OnExitWorld();

	OverrideSoundFilter(NULL, false);	 // cancel any filter override...
	g_pGameClientShell->GetMixer()->ClearMixers();

	// Make sure slowmo is off.
	ExitSlowMo( true );


	m_hSpectatorFollowCharacter = NULL;
	m_hSpectatorTrackTarget = NULL;

	m_EarliestRespawnTime.Stop( );
	m_bRespawnWhenTimedOut = false;
	m_fRespawnPenalty = 0.0f;
	m_tmrJoinGrace.Stop( );

	m_bStoryMode = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// Give the Client Weapon Mgr a chance to handle the message...
	if( g_pClientWeaponMgr->OnMessage( messageID, pMsg ))
	{
		return true;
	}

	switch(messageID)
	{
		case MID_PLAYER_STATE_CHANGE:		HandleMsgPlayerStateChange		(pMsg);	break;
		case MID_CLIENT_PLAYER_UPDATE:		HandleMsgClientPlayerUpdate		(pMsg);	break;
		case MID_PLAYER_DAMAGE:				HandleMsgPlayerDamage			(pMsg);	break;
		case MID_CHANGE_WORLDPROPERTIES:	HandleMsgChangeWorldProperties	(pMsg);	break;
		case MID_ADD_PUSHER:				HandleMsgAddPusher				(pMsg); break;
		case MID_SLOWMO:					HandleMsgSlowMo					(pMsg); break;
		case MID_PLAYER_EVENT:				return HandleMsgPlayerEvent		(pMsg); break;
		case MID_PLAYER_BODY:				HandleMsgPlayerBody				(pMsg); break;
		case MID_SONIC:						HandleMsgSonic					(pMsg); break;
		case MID_PLAYER_GOTO_NODE:			HandleMsgGoto					(pMsg); break;
		case MID_PLAYER_SPECTATORMODE:		HandleMsgSpectatorMode			(pMsg);	break;
		case MID_PLAYER_LEASH:				HandleMsgPlayerLeash			(pMsg); break;
		default:							return false;	break;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnKeyDown(int key, int rep)
//
//	PURPOSE:	Handle key down notification
//				Try to avoid using OnKeyDown and OnKeyUp as they
//				are not portable functions
//
// ----------------------------------------------------------------------- //
bool CPlayerMgr::OnKeyDown(int key, int /*rep*/)
{
	if (!IsPlayerInWorld()) 
		return false;

	// Death is a mission failure in sp.  Have to wait until we reach the dead state.
	if( !IsMultiplayerGameClient( ) && GetPlayerState( ) == ePlayerState_Dead )
	{
		// See if we can respawn.  For SP, this just makes us wait
		// a little bit before we can go to the mission failure screen.
		if( !m_EarliestRespawnTime.IsStarted( ) || m_EarliestRespawnTime.IsTimedOut( ))
		{
			g_pMissionMgr->HandleMissionFailed();
			return true;
		}
	}

	// Are we playing a cinematic...
	if( m_pPlayerCamera->GetCameraMode() == CPlayerCamera::kCM_Cinematic )
	{
// XENON: Always end the cinematic no matter what key is pressed
#if !defined(PLATFORM_XENON)
		if (key == VK_SPACE)
#endif // !PLATFORM_XENON
		{
			// Send an activate message to stop the cinemaitc...

			DoActivate();
			CBindMgr::GetSingleton().ClearAllCommands();
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnKeyUp(int key)
//
//	PURPOSE:	Handle key up notification
//
// ----------------------------------------------------------------------- //
bool CPlayerMgr::OnKeyUp(int /*key*/)
{
	return false;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::PreUpdate()
//
//	PURPOSE:	Handle client pre-updates
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::PreUpdate()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Update()
//
//	PURPOSE:	Handle client updates
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::Update()
{
	if( m_ePlayerState == ePlayerState_None )
		return;

	if (m_bStoryMode && !CPlayerBodyMgr::Instance( ).IsLocked(kAPG_Action, kAP_ACT_StoryMode)) 
	{
		CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_StoryMode, CPlayerBodyMgr::kLocked );
	}

	// Update models (powerups) glowing...

	UpdateModelGlow();

	UpdateDamage();

	UpdateSlowMo( );

	UpdateConcussionAudioEffect( );

	m_HeartBeat.Update( );
	m_Breath.Update( );

	if (!IsPlayerAlive( ))
	{
		if( !m_EarliestRespawnTime.IsStarted() || m_EarliestRespawnTime.IsTimedOut())
			g_pHUDMgr->QueueUpdate(kHUDSpectator);
		if( m_EarliestRespawnTime.IsStarted() && m_EarliestRespawnTime.IsTimedOut())
			g_pHUDMgr->QueueUpdate(kHUDRespawn);

		if (m_bRespawnWhenTimedOut && CanRespawn())
		{
			HandleRespawn();
		}
	}

	AimMgr::Instance().Update();
	CAccuracyMgr::Instance().Update();
#ifndef _FINAL
	if (CAccuracyMgr::Instance().s_vtDebugPerturb.GetFloat( ) > 0.0f)
	{
		if (!g_pPerturbLight->IsOn())
		{
			g_pPerturbLight->TurnOn();
		}
		g_pPerturbLight->Update( ObjectContextTimer( m_pMoveMgr->GetServerObject() ).GetTimerElapsedS() );
	}
	else
	{
		if (g_pPerturbLight->IsOn())
		{
			g_pPerturbLight->TurnOff();
		}
	}
#endif

	// See if we're in follow mode.
	if( GetSpectatorMode() == eSpectatorMode_Follow )
	{
		// Check if we need to switch to a new target since our old one may have gone away or died.
		bool bFindNewTarget = false;
		CCharacterFX* pCharFx = NULL;
		if( !GetSpectatorFollowTarget( ))
		{
			bFindNewTarget = true;
		}
		else
		{
			UpdateSpectatorFollowTarget( );

			pCharFx = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( GetSpectatorFollowTarget( ));
			if( pCharFx->IsDead( ))
				bFindNewTarget = true;
		}

		// Try to go to a new target.
		if( bFindNewTarget )
		{
			CCharacterFX* pNewCharFx = GetNextSpectatorFollow( pCharFx );
			SetSpectatorFollowCharacter( pNewCharFx );
		}
	}

#ifdef PROJECT_DARK

	// Handle the transition effect for switching beam types
	if( m_fFlashlightTransitionTime != -1.0f )
	{
		m_fFlashlightTransitionTime += g_pLTClient->GetFrameTime();

		if( m_fFlashlightTransitionTime >= g_vtFlashlightBeamTransitionTime.GetFloat() )
		{
			if( m_pFlashLight->IsRecordType( "Player_Wide" ) )
			{
				m_pFlashLight->Initialize( m_pPlayerCamera->GetCamera(), "Player_Narrow" );
			}
			else
			{
				m_pFlashLight->Initialize( m_pPlayerCamera->GetCamera(), "Player_Wide" );
			}

			m_fFlashlightTransitionTime = -1.0f;
		}
	}
	else
	{
		// Handle the button delay for turning off the flashlight...
		if( ( m_fFlashlightButtonPressTime != -1.0f ) && CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_FLASHLIGHT ) )
		{
			m_fFlashlightButtonPressTime += g_pLTClient->GetFrameTime();

			if( m_fFlashlightButtonPressTime >= g_vtFlashlightTurnOffDelay.GetFloat() )
			{
				m_pFlashLight->TurnOff();
				m_fFlashlightButtonPressTime = -1.0f;
			}
		}
	}

#endif//PROJECT_DARK

#ifdef PROJECT_FEAR
	if (m_fFlashlightMaxCharge > 0.0f && g_vtFlashlightBattery.GetFloat() > 0.0f && !IsMultiplayerGameClient())
	{
		float fTimeDilation = 1.0f; 		
		if( IsInSlowMo() )
		{
			uint32 dwNum, dwDenom;
			SimulationTimer::Instance( ).GetTimerTimeScale( dwNum, dwDenom );
			fTimeDilation = ((float)dwNum / (float)dwDenom);
		}

		if (m_pFlashLight->IsOn())
		{			
			m_fFlashlightCharge -= fTimeDilation * GameTimeTimer::Instance().GetTimerElapsedS();
			if (m_fFlashlightCharge <= 0.0f)
			{
				m_pFlashLight->TurnOff();
			}

		}
		else
		{
			m_fFlashlightCharge += fTimeDilation * GameTimeTimer::Instance().GetTimerElapsedS() * m_fFlashlightRecharge;
		}
		m_fFlashlightCharge = LTCLAMP(m_fFlashlightCharge,0.0f,m_fFlashlightMaxCharge);

	}

#endif

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::FirstUpdate
//
//	PURPOSE:	Handle first update (each level)
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::FirstUpdate()
{
	// Force the player camera to update...
	m_pPlayerCamera->Update( );

#ifdef USE_EAX20_HARDWARE_FILTERS

	m_hSoundFilterIdGlobal = NULL;

	char aGlobalSoundFilter[256] = {0};
	char* pGlobalSoundFilter = NULL;
	if (g_pLTClient->GetSConValueString("GlobalSoundFilter", aGlobalSoundFilter, sizeof(aGlobalSoundFilter)) == LT_OK)
	{
		pGlobalSoundFilter = aGlobalSoundFilter;
	}

	if (pGlobalSoundFilter && pGlobalSoundFilter[0])
	{
		HRECORD hSoundFilter = SoundFilterDB::Instance().GetFilterRecord(pGlobalSoundFilter);
		LTASSERT_PARAM1(hSoundFilter, "CPlayerMgr::FirstUpdate : Failed to find SoundFilter: %s", (pGlobalSoundFilter ? pGlobalSoundFilter : "<null>"));
		if (hSoundFilter)
		{
			ApplySoundFilter(hSoundFilter, true);
			m_hSoundFilterIdGlobal = hSoundFilter;
					}
				}
	if (m_bSoundFilterOverrideOn)
	{
		UpdateSoundFilters(m_hSoundFilterIDOverride, true);
	}

#endif  // USE_EAX20_HARDWARE_FILTERS

	// Set up primary mixer...
	char aGlobalSoundMixer[256] = {0};
	char* pGlobalSoundMixer = NULL;
	if (g_pLTClient->GetSConValueString("GlobalSoundMixer", aGlobalSoundMixer, sizeof(aGlobalSoundMixer)) == LT_OK)
	{
		pGlobalSoundMixer = aGlobalSoundMixer;
	}

	if (pGlobalSoundMixer && pGlobalSoundMixer[0])
	{
		g_pGameClientShell->GetMixer()->ProcessMixerByName(pGlobalSoundMixer, 0);
	}

	// Force us to re-evaluate what container we're in.  We call
	// UpdateContainers() first to make sure any container changes
	// have been accounted for, then we clear the container code
	// and force an update (this is done for underwater situations like
	// dying underwater and respawning, and also for picking up intelligence
	// items underwater)...
	UpdateContainers();
	ClearCurContainerCode();
	UpdateContainers();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::FirstPlayingUpdate()
//
//	PURPOSE:	Handle when the player first starts playing
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::FirstPlayingUpdate()
{
	g_pHUDSlowMo->ResetBar();

	// Set Initial cheats...
	if (g_pCheatMgr)
	{
		ClientDB& ClientDatabase = ClientDB::Instance();
		uint8 nNumCheats = ClientDatabase.GetNumValues(ClientDatabase.GetClientSharedRecord(), CDB_Cheats);

        for (uint8 i=0; i < nNumCheats; i++)
		{
			const char* pszCheat = ClientDatabase.GetString(ClientDatabase.GetClientSharedRecord(), CDB_Cheats);
			if (pszCheat && pszCheat[0])
			{
				ConParseW cParse( MPA2W( pszCheat ).c_str( ));
				if( LT_OK == g_pCommonLT->Parse( &cParse ))
				{
					CParsedMsgW parsedMsg( cParse.m_nArgs, cParse.m_Args );
					g_pCheatMgr->Check( parsedMsg );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlaying()
//
//	PURPOSE:	Handle updating playing (normal) game state
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlaying()
{
	// Just to make sure our flags don't get screwed up by
	// the server when we're paused (which they can)
	if (g_pGameClientShell->IsServerPaused())
	{
		UpdatePlayerVisibility( );
		return;
	}

	// First time UpdatePlaying is called...
	if (!m_bStartedPlaying)
	{
        m_bStartedPlaying = true;
		FirstPlayingUpdate( );
	}

	// Move and rotate the player...
	UpdatePlayerMovement( );

	// Hide or show the player...
	UpdatePlayerVisibility( );

	// Keep track of what the player is doing...
	UpdatePlayerFlags( );

	// Update duck camera offset...
	UpdateDuck( );

	LadderMgr::Instance().Update();
	SpecialMoveMgr::Instance().Update();

	// The PlayerBody state needs to be updated before the camera...
	if( !IsSpectating( ))
		CPlayerBodyMgr::Instance( ).Update( );

	// Once the animation of the PlayerBody is updated, update the camera...
	UpdateCamera( );

	// See if we are tracking distance to an object..
	UpdateDistanceIndicator( );

	// Update the targeting info...
	CAutoTargetMgr::Instance( ).Update( );
	m_pTargetMgr->Update( );

	// Always update the listener to be at the camera...
	((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetListener( false, const_cast< LTVector* >( &m_pPlayerCamera->GetCameraActivePos( )), 
		const_cast< LTRotation* >( &m_pPlayerCamera->GetCameraActiveRotation( )), true );

	ObjectContextTimer objectContextTimer( g_pMoveMgr->GetServerObject( ));

	// Update the detectors...
	g_iFocusObjectDetector.SetTransform( GetPlayerCamera()->GetCamera() );
	g_iFocusObjectDetector.Update( objectContextTimer.GetTimerElapsedS() );

	g_iAttackPredictionObjectDetector.SetTransform( GetPlayerCamera()->GetCamera() );
	g_iAttackPredictionObjectDetector.Update( objectContextTimer.GetTimerElapsedS() );
	g_iAttackPredictionObjectDetector.AcquireObject( false );

	m_PickupObjectDetector.SetTransform( GetPlayerCamera()->GetCamera() );
	m_PickupObjectDetector.Update( objectContextTimer.GetTimerElapsedS() );

	// Handle focus behavior
	if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_FOCUS ) )
	{
		// Melee weapons always have a focus accuracy of 1.0
		CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		bool bMelee = ( ( pWeapon && pWeapon->IsMeleeWeapon() ) ? true : false );

		if( !g_iFocusObjectDetector.GetObject() )
		{
			g_iFocusObjectDetector.AcquireObject();
			m_fFocusAccuracy = ( bMelee ? 1.0f : 0.0f );
		}
		else if( m_fFocusAccuracy < 1.0f )
		{
			m_fFocusAccuracy += ( g_pLTClient->GetFrameTime() / 4.0f );

			if( m_fFocusAccuracy > 1.0f )
			{
				m_fFocusAccuracy = 1.0f;
			}
		}
	}
	else
	{
		// Clear the object in a third of a second...
		if( !g_iFocusObjectDetector.ClearingObject() )
		{
			g_iFocusObjectDetector.ClearObject( 0.5f );
		}

		m_fFocusAccuracy = 0.0f;
	}

	// Try to acquire a pickup object.
	m_PickupObjectDetector.AcquireObject( false );
	g_pHUDMgr->QueueUpdate(kHUDSwap);

	if( m_hPlayerNodeGoto )
	{
		// Are we finished with the goto node...
		if( m_bAtNodePosition && (m_bAtNodeRotation || (!m_bFaceNodeForwardPitch && !m_bFaceNodeForwardYaw)) )
			HandleNodeArrival( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateNotPlaying()
//
//	PURPOSE:	Handle updating non-playing (screen) game state
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateNotPlaying()
{
	Update();
	UpdatePlaying();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::PostUpdate()
//
//	PURPOSE:	Handle post updates - after the scene is rendered
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::PostUpdate()
{
	// Update container effects...
	if(m_bStartedPlaying)
	{
		UpdateContainers();
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateDuck()
//
//	PURPOSE:	Update ducking camera offset
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateDuck()
{
	// Can't duck when free movement...

    if (g_pPlayerMgr->GetMoveMgr()->IsBodyInLiquid() || 
		LadderMgr::Instance().IsClimbing() ||
		SpecialMoveMgr::Instance().IsActive() ||
		!g_pPlayerMgr->IsPlayerAlive() ||
        IsFreeMovement(m_eCurContainerCode) ||
		m_pTurretFX ) 
	{
		// Reset ducking parameters...
		m_fCamDuck = 0.0f;
        m_bStartedDuckingDown = false;
		return;
	}

	float fBodyMult = 1.0f;

	if ( (m_dwPlayerFlags & BC_CFLG_DUCK) || m_bCameraDip)
	{
        m_bStartedDuckingUp = false;

		// Get the difference in  crouch height...
		
		float fCrouchDelta = g_pMoveMgr->GetCrouchHeightDifference();
		
		// If dipping don't use the crouch distance...

		if( m_bCameraDip )
			fCrouchDelta = 0.0f;

		// See if the duck just started...

		if (!m_bStartedDuckingDown)
		{
            m_bStartedDuckingDown = true;
			m_StartDuckTimer.Start( );
		}

		m_fCamDuck += m_fDuckDownV * ((float)m_StartDuckTimer.GetElapseTime());

		if (m_fCamDuck < (m_fMaxDuckDistance - fCrouchDelta) )
		{
			m_bCameraDip = false;
			m_fCamDuck = m_fMaxDuckDistance - fCrouchDelta;
		}
		else if( m_fCamDuck > 0.0f )
		{
			m_fCamDuck = 0.0f;
		}
	}
	else if (m_fCamDuck < 0.0) // Raise up
	{
        m_bStartedDuckingDown = false;

		if (!m_bStartedDuckingUp)
		{
			m_StartDuckTimer.Start( );
            m_bStartedDuckingUp = true;
		}

		m_fCamDuck += m_fDuckUpV * ((float)m_StartDuckTimer.GetElapseTime()) * fBodyMult;

		if (m_fCamDuck > 0.0f)
		{
			m_fCamDuck = 0.0f;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgClientPlayerUpdate()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgClientPlayerUpdate (ILTMessage_Read *pMsg)
{
    uint16 changeFlags = pMsg->ReadBits( PSTATE_NUMBITS );

	m_pMoveMgr->OnPhysicsUpdate(changeFlags, pMsg);

	if (changeFlags & PSTATE_CAMERA)
	{
		// For now just read in the head offset...
		LTVector vOffset = pMsg->ReadLTVector();
		vOffset.z = 30.0f;  // Temp
		
		m_pPlayerCamera->SetFirstPersonOffset( vOffset );
	}

	m_bPlayerUpdated = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgPlayerDamage()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //


void CPlayerMgr::HandleMsgPlayerDamage (ILTMessage_Read *pMsg)
{
	static const float kfSectorSize = MATH_PI * 2.0f / (float)kNumDamageSectors;
	if (!pMsg || g_bScreenShotMode) return;

	LTVector vDir = pMsg->ReadCompLTPolarCoord();
	float fPercent = (( float )pMsg->Readuint8() / 255.0f );
    DamageType eType = static_cast<DamageType>(pMsg->Readuint8());
	bool bTookHealth = pMsg->Readbool();

	g_pInterfaceMgr->GetPlayerStats( )->UpdateHealth(pMsg->Readuint16( ));
	g_pInterfaceMgr->GetPlayerStats( )->UpdateArmor(pMsg->Readuint16( ));

	bool bHeadShot = pMsg->Readbool();

	// No need to handle a damage message if the player is dead...
	
	if( !IsPlayerAlive())
		return;
	
	uint8 damageSector = kNumDamageSectors;

	// [KLS 3/5/02] - If we were damaged, determine the direction the damage came from...
	if (fPercent > 0.0f)
	{

		LTRotation const& rRot = m_pPlayerCamera->GetCameraRotation( );

		vDir.Normalize();
		float fDF = rRot.Forward().Dot(vDir);
		float fDR = rRot.Right().Dot(vDir);

		float damageAngle = MATH_PI+(float)atan2(fDF,fDR);
		damageSector = (uint8)(damageAngle / kfSectorSize); 

		m_fDamage[damageSector] += fPercent;

		if (m_fDamage[damageSector] > 1.0f)
			m_fDamage[damageSector] = 1.0f;

	}
	

/*******************************
/****** Damage Sector Map ******
	            3
		     4     2

           5         1

		  6           0

		   7         11

			 8     10
			    9
*******************************/

	
	// Do some camera FX for taking armor and taking health...
	// Tie this into the DamageSector at some point??

	DamageFlags dmgFlag = DamageTypeToFlag( eType );

	if( bTookHealth )
	{
		// Play the taking health fx for the DamageFX associated with the damage type...

		DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
		while( pDamageFX )
		{
			// Test the damage flags against the DamageFX...

			if( dmgFlag & pDamageFX->m_nDamageFlag || pDamageFX->m_vtTestFX.GetFloat() > 0.0f )
			{
				CLIENTFX_CREATESTRUCT fxInit( pDamageFX->m_szTakingHealthFXName, 0); 
				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
			}
			
			pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
		}

		//play head shot effect
		if (bHeadShot && IsMultiplayerGameClient())
		{
			const char *pszDeathFX = ClientDB::Instance( ).GetString( ClientDB::Instance( ).GetClientSharedRecord( ), CDB_ClientShared_HeadshotClientFX );
			//don't play the effect if the old last is still playing...
			if( !m_fxHeadShot.IsValid( ) || 
					(m_fxHeadShot.GetInstance()&& m_fxHeadShot.GetInstance()->IsDone())
				)
			{
				CLIENTFX_CREATESTRUCT fxInit( "Vision_Headshot", 0); 
				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_fxHeadShot, fxInit, true );
			}

		}
	}
	else
	{
		// Play the taking armor fx for the DamageFX associated with the damage type...

		DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
		while( pDamageFX )
		{
			// Test the damage flags against the DamageFX...

			if( dmgFlag & pDamageFX->m_nDamageFlag || pDamageFX->m_vtTestFX.GetFloat() > 0.0f )
			{
				CLIENTFX_CREATESTRUCT fxInit( pDamageFX->m_szTakingArmorFXName, 0 ); 
				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
			}
			
			pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
		}
	}

	// Tilt the camera based on the direction the damage came from...

	if (kNumDamageSectors != damageSector && IsJarCameraType(eType) && 
		(m_pPlayerCamera->GetCameraMode() == CPlayerCamera::kCM_FirstPerson) &&
		!IsOperatingTurret( ) && (g_vtCamDamage.GetFloat() > 0.0f) )
	{
		CameraDelta delta;

		if( eType == DT_MELEE )
		{
			if( g_vtCameraMeleeDamageDistortion.GetFloat() > 0.0f )
			{
				// Turn ScreenGlow on and add in more intensity...
				g_pLTClient->SetConsoleVariableFloat( "ScreenGlowEnable", 1.0f );
				m_fMeleeDamageDistortionI1 = m_fMeleeDamageDistortionIntensity + g_vtCameraMeleeDamageDistortionIntensity.GetFloat( );
			}

			if( g_vtCameraMeleeDamageOverlay.GetFloat() > 0.0f )
			{
				g_pOverlay->Show( g_pOverlay->GetHUDOverlay( OVM_DAMAGE ));
				m_fMeleeDamageOverlayB1 = m_fMeleeDamageOverlayBlur + g_vtCameraMeleeDamageOverlayBlur.GetFloat( );
			}

			m_MeleeDamageEffectTimer.Start( g_vtCameraMeleeDamageEffectTime.GetFloat( ));

		if (g_vtCamDamagePitch.GetFloat() > 0.0f)
		{
				float fPitchAngle = g_vtCameraMeleeDamageMinPitch.GetFloat() + ((g_vtCameraMeleeDamageMaxPitch.GetFloat() - g_vtCameraMeleeDamageMinPitch.GetFloat()) * fPercent);
				
				if (damageSector > 7 && damageSector <11)
				{
					delta.Pitch.fTime1	= g_vtCameraMeleeDamageTime1.GetFloat();
					delta.Pitch.fTime2	= g_vtCameraMeleeDamageTime2.GetFloat();
					delta.Pitch.eWave1	= (WaveType)(int)g_vtCameraMeleeDamageWave1.GetFloat();
					delta.Pitch.eWave2	= (WaveType)(int)g_vtCameraMeleeDamageWave2.GetFloat();
					delta.Pitch.fVar	= DEG2RAD(fPitchAngle);
				}
				else if (damageSector > 1 && damageSector < 5)
				{
					delta.Pitch.fTime1	= g_vtCameraMeleeDamageTime1.GetFloat();
					delta.Pitch.fTime2	= g_vtCameraMeleeDamageTime2.GetFloat();
					delta.Pitch.eWave1	= (WaveType)(int)g_vtCameraMeleeDamageWave1.GetFloat();
					delta.Pitch.eWave2	= (WaveType)(int)g_vtCameraMeleeDamageWave2.GetFloat();
					delta.Pitch.fVar	= -DEG2RAD(fPitchAngle);
				}
			}

			if( g_vtCamDamageYaw.GetFloat( ) > 0.0f )
			{
				float fYawAngle = g_vtCameraMeleeDamageMinYaw.GetFloat() + ((g_vtCameraMeleeDamageMaxYaw.GetFloat() - g_vtCameraMeleeDamageMinYaw.GetFloat()) * fPercent);
				
				if( damageSector > 3 && damageSector < 9 )
				{
					delta.Yaw.fTime1	= g_vtCameraMeleeDamageTime1.GetFloat();
					delta.Yaw.fTime2	= g_vtCameraMeleeDamageTime2.GetFloat();
					delta.Yaw.eWave1	= (WaveType)(int)g_vtCameraMeleeDamageWave1.GetFloat();
					delta.Yaw.eWave2	= (WaveType)(int)g_vtCameraMeleeDamageWave2.GetFloat();
					delta.Yaw.fVar		= DEG2RAD(fYawAngle);
				}
				else if( damageSector < 3 || damageSector > 9 )
				{
					delta.Yaw.fTime1	= g_vtCameraMeleeDamageTime1.GetFloat();
					delta.Yaw.fTime2	= g_vtCameraMeleeDamageTime2.GetFloat();
					delta.Yaw.eWave1	= (WaveType)(int)g_vtCameraMeleeDamageWave1.GetFloat();
					delta.Yaw.eWave2	= (WaveType)(int)g_vtCameraMeleeDamageWave2.GetFloat();
					delta.Yaw.fVar		= -DEG2RAD(fYawAngle);
				}
			}

			if( g_vtCamDamageRoll.GetFloat() > 0.0f )
			{
				float fRollAngle = g_vtCameraMeleeDamageMinRoll.GetFloat() + ((g_vtCameraMeleeDamageMaxRoll.GetFloat() - g_vtCameraMeleeDamageMinRoll.GetFloat()) * fPercent);

				if (damageSector > 3 && damageSector < 9)
				{
					delta.Roll.fTime1	= g_vtCameraMeleeDamageTime1.GetFloat();
					delta.Roll.fTime2	= g_vtCameraMeleeDamageTime2.GetFloat();
					delta.Roll.eWave1	= (WaveType)(int)g_vtCameraMeleeDamageWave1.GetFloat();
					delta.Roll.eWave2	= (WaveType)(int)g_vtCameraMeleeDamageWave2.GetFloat();
					delta.Roll.fVar		= -DEG2RAD(fRollAngle);
				}
				else if (damageSector < 3 || damageSector > 9 )
				{
					delta.Roll.fTime1	= g_vtCameraMeleeDamageTime1.GetFloat();
					delta.Roll.fTime2	= g_vtCameraMeleeDamageTime2.GetFloat();
					delta.Roll.eWave1	= (WaveType)(int)g_vtCameraMeleeDamageWave1.GetFloat();
					delta.Roll.eWave2	= (WaveType)(int)g_vtCameraMeleeDamageWave2.GetFloat();
					delta.Roll.fVar		= DEG2RAD(fRollAngle);
				}
			}
		}
		
		// Non Melee
		else
		{
			if (g_vtCamDamagePitch.GetFloat() > 0.0f)
			{
				float fPitchAngle = g_vtCamDamageMinPitch.GetFloat() + ((g_vtCamDamageMaxPitch.GetFloat() - g_vtCamDamageMinPitch.GetFloat()) * fPercent);
	
			if (damageSector > 7 && damageSector <11)
			{
				delta.Pitch.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Pitch.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Pitch.eWave1	= Wave_SlowOff;
					delta.Pitch.eWave2	= Wave_SlowOn;
				delta.Pitch.fVar	= DEG2RAD(fPitchAngle);
			}
			else if (damageSector > 1 && damageSector < 5)
			{
				delta.Pitch.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Pitch.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Pitch.eWave1	= Wave_SlowOff;
					delta.Pitch.eWave2	= Wave_SlowOn;
				delta.Pitch.fVar	= -DEG2RAD(fPitchAngle);
			}
		}

			if (g_vtCamDamageYaw.GetFloat() > 0.0f)
			{
				float fYawAngle = g_vtCamDamageMinYaw.GetFloat() + ((g_vtCamDamageMaxYaw.GetFloat() - g_vtCamDamageMinYaw.GetFloat()) * fPercent);

				if (damageSector > 3 && damageSector < 9)
				{
					delta.Yaw.fTime1	= g_vtCamDamageTime1.GetFloat();
					delta.Yaw.fTime2	= g_vtCamDamageTime2.GetFloat();
					delta.Yaw.eWave1	= Wave_SlowOff;
					delta.Yaw.eWave2	= Wave_SlowOn;
					delta.Yaw.fVar		= DEG2RAD(fYawAngle);
				}
				else if (damageSector < 3 || damageSector > 9)
				{
					delta.Yaw.fTime1	= g_vtCamDamageTime1.GetFloat();
					delta.Yaw.fTime2	= g_vtCamDamageTime2.GetFloat();
					delta.Yaw.eWave1	= Wave_SlowOff;
					delta.Yaw.eWave2	= Wave_SlowOn;
					delta.Yaw.fVar		= -DEG2RAD(fYawAngle);
				}
			}

		if (g_vtCamDamageRoll.GetFloat() > 0.0f)
		{
				float fRollAngle = g_vtCamDamageMinRoll.GetFloat() + ((g_vtCamDamageMaxRoll.GetFloat() - g_vtCamDamageMinRoll.GetFloat()) * fPercent);

			if (damageSector > 4 && damageSector < 8)
			{
				delta.Roll.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Roll.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Roll.eWave1	= Wave_SlowOff;
					delta.Roll.eWave2	= Wave_SlowOn;
				delta.Roll.fVar		= DEG2RAD(fRollAngle);
			}
			else if (damageSector < 2 || damageSector  == 11)
			{
				delta.Roll.fTime1	= g_vtCamDamageTime1.GetFloat();
				delta.Roll.fTime2	= g_vtCamDamageTime2.GetFloat();
				delta.Roll.eWave1	= Wave_SlowOff;
					delta.Roll.eWave2	= Wave_SlowOn;
				delta.Roll.fVar		= -DEG2RAD(fRollAngle);
			}
		}
		}

		m_pCameraOffsetMgr->AddDelta(delta);
	}

	// if this effect has an audio effect, start it up...
	if (kNumDamageSectors != damageSector && IsConcussionAudioEffectType(eType))
	{

		// determine the level of ring effect
		char* pMixerToUse;
		float fRingTime;

		if (fPercent < GetConcussionAudioRadius(eType, 1))
		{
			pMixerToUse = "EarRingLight";
			fRingTime = 3.0f;
		}
		else if (fPercent < GetConcussionAudioRadius(eType, 0))
		{
			pMixerToUse = "EarRingMed";
			fRingTime = 5.0f;
		}
		else
		{
			pMixerToUse = "EarRing";
			fRingTime = 7.0f;
		}
		// set the mixer..
		g_pGameClientShell->GetMixer( )->ProcessMixerByName( pMixerToUse );

		// call the ear-ringing tone...

		m_AudioTimer.Start(fRingTime);	// set for 5s for now...
		m_nAudioEffectState = 1;	// counting down..


		if (m_hRingSound == NULL)
		{
			m_hRingSound = g_pClientSoundMgr->PlayDBSoundFromObject( g_pMoveMgr->GetObject(),
				g_pSoundDB->GetSoundDBRecord("EarRingingBuzz"),
				SMGR_INVALID_RADIUS,
				SOUNDPRIORITY_PLAYER_MEDIUM, PLAYSOUND_LOCAL | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE, 
				80, 1.0f, SMGR_INVALID_RADIUS, DEFAULT_SOUND_CLASS,
				PLAYSOUND_MIX_WEAPONS_PLAYER);
		}
		else
		{
			g_pLTClient->SoundMgr()->SetSoundVolume(m_hRingSound, 80, 0.0f);
		}
	}


	float fSlow = SlowMovementDuration(eType);
	float fMult = SlowMovementMultiplier(eType);
	if (fSlow > 0.0f && fMult >= 0.0f)
	{
		m_pMoveMgr->AddDamagePenalty(fSlow,fMult);
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Handle()
//
//	PURPOSE:	This handles the message sent by the world properties
//				indicating that they have changed and that the player needs
//				to sync to the console variables on the server
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgChangeWorldProperties (ILTMessage_Read * /*pMsg*/)
{
	g_pGameClientShell->ResetDynamicWorldProperties(m_bUseWorldFog);
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::HandleMsgAddPusher
//
//  PURPOSE:	Add a pusher to the MoveMgr.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgAddPusher( ILTMessage_Read *pMsg )
{
    LTVector vPos		= pMsg->ReadCompLTVector();
	float fRadius		= pMsg->Readfloat();
	float fStartDelay = pMsg->Readfloat();
	float fDuration	= pMsg->Readfloat();
	float fStrength	= pMsg->Readfloat();

	g_pPlayerMgr->GetMoveMgr()->AddPusher( vPos, fRadius, fStartDelay, fDuration, fStrength );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::HandleMsgSlowMo
//
//  PURPOSE:	Change our slowmo state.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgSlowMo( ILTMessage_Read *pMsg )
{
	// Read in the whether we're going into or our of slowmo.
	SlowMoMsgType eType = (SlowMoMsgType)(pMsg->Readuint8());

	switch( eType )
	{
		case kSlowMoInit:
		{
			m_fSlowMoCurCharge = pMsg->Readfloat();
			m_fSlowMoMaxCharge = pMsg->Readfloat();
			m_fSlowMoMinCharge = pMsg->Readfloat();
			m_fSlowMoRecharge = pMsg->Readfloat();
		}
		break;
		case kSlowMoStart:
		{

			// Read in whether we should transition into new state or not.
			HRECORD hSlowMoRecord = pMsg->ReadDatabaseRecord( g_pLTDatabase, DATABASE_CATEGORY( SlowMo ).GetCategory( ));
			bool bTransition = pMsg->Readbool();
			double fSlowMoPeriod = pMsg->Readdouble( );
			bool bPlayer = pMsg->Readbool();
			uint32 nActivatorID = pMsg->Readuint32( );

			// Check if we're already in slowmo. If it is player started we can't override a scripted one
			if (( m_eSlowMoState == eEffectState_In || m_eSlowMoState == eEffectState_Entering) && bPlayer)
				return;


			
			if( IsMultiplayerGameClient( ) )
			{
				uint32 nLocalID = 0;
				g_pLTClient->GetLocalClientID( &nLocalID );
				CClientInfoMgr *pClientInfoMgr = g_pInterfaceMgr->GetClientInfoMgr( );

				uint8 nActivatorTeam = pClientInfoMgr->GetPlayerTeam( nActivatorID );

				CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
				while( iter != CCharacterFX::GetCharFXList( ).end( )) 
				{
					CCharacterFX* pChar = (*iter);

					if (pChar->m_cs.bIsPlayer && !pChar->m_cs.bIsDead && pChar->m_cs.nClientID != nLocalID)
					{
						if( GameModeMgr::Instance( ).m_grbUseTeams )
						{
							if (pClientInfoMgr->GetPlayerTeam( pChar->m_cs.nClientID ) == nActivatorTeam)
								pChar->CreateSlowMoFX( GETCATRECORDATTRIB( SlowMo, hSlowMoRecord, 3rdPersonFX ) );
						}
						else
						{
							if (pChar->m_cs.nClientID == nActivatorID)
								pChar->CreateSlowMoFX( GETCATRECORDATTRIB( SlowMo, hSlowMoRecord, 3rdPersonFX ) );
						}
					}


					++iter;
				}


				// I don't need a message telling me I activated slow-mo, I should already know that...
				if (nActivatorID != nLocalID)
				{
					

					eChatMsgType type = kMsgTransmission;
					if( GameModeMgr::Instance( ).m_grbUseTeams )
					{
						uint8 nLocalTeam = pClientInfoMgr->GetPlayerTeam( nLocalID );
						uint8 nActivatorTeam = pClientInfoMgr->GetPlayerTeam( nActivatorID );

						if( nLocalTeam != INVALID_TEAM && nActivatorTeam != INVALID_TEAM ) 
						{
							if( pClientInfoMgr->IsLocalTeam( nActivatorTeam ))
							{
								// My team activated slow-mo...
								type = kMsgTeam;
							}
							else
							{
								// The other team activated slow-mo...
								type = kMsgOtherTeam;
							}
						}
					}

					wchar_t szMsg[128] = L"";
					FormatString( "IDS_HEACTIVATEDSLOWMO", szMsg, LTARRAYSIZE(szMsg), pClientInfoMgr->GetPlayerName( nActivatorID ));
					g_pGameMsgs->AddMessage( szMsg, type );
				}
			}

			EnterSlowMo( hSlowMoRecord, bTransition, fSlowMoPeriod, bPlayer );
		}
		break;
		case kSlowMoEnd:
		{

			bool bDoTransition = pMsg->Readbool( );
			m_fSlowMoCurCharge = pMsg->Readfloat();

			if( IsMultiplayerGameClient( ) )
			{
				CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
				while( iter != CCharacterFX::GetCharFXList( ).end( )) 
				{
					(*iter)->RemoveSlowMoFX();
					++iter;
				}
			}

			// Check if we're already there.
			if( m_eSlowMoState == eEffectState_Out )
			{
				return;
			}
			// We're still in or starting our way out.
			else
			{
				// Check if we're supposed to do a transition out.
				
				if( !bDoTransition )
				{
					ExitSlowMo( true );
					return;
				}

				//get the transition time
				double fTransitionPeriod = fTransitionPeriod = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, TransitionPeriod );

				// If the timer was started with a duration and it is less than the transition time,
				// use the remaining duration as the transition time
				if( m_SlowMoTimer.IsStarted( ) && !m_SlowMoTimer.IsTimedOut( ) && m_SlowMoTimer.GetDuration() > 0.0f && m_SlowMoTimer.GetDuration() < fTransitionPeriod )
					fTransitionPeriod = m_SlowMoTimer.GetTimeLeft();
					
				if( fTransitionPeriod >= 0.0f )
				{
					m_SlowMoTimer.Start( fTransitionPeriod );
					ExitingSlowMo( );
					return;
				}
				else
				{
					ExitSlowMo( true );
					return;
				}
			}
		}
		break;
	}
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CPlayerMgr::HandleMsgPlayerBody()
//
//	PURPOSE:	Relay a message to the player body...
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::HandleMsgPlayerBody( ILTMessage_Read *pMsg )
{
	if( CPlayerBodyMgr::Instance( ).IsEnabled( ))
		CPlayerBodyMgr::Instance( ).OnMessage( pMsg );

	return true;
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CPlayerMgr::HandleMsgSonic()
//
//	PURPOSE:	Update sonic data
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::HandleMsgSonic( ILTMessage_Read *pMsg )
{
	m_iSonicData.HandleMessage( pMsg );
	return true;
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CPlayerMgr::HandleMsgGoto()
//
//	PURPOSE:	Handle a goto message...
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::HandleMsgGoto( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return false;

	// Retrieve the node we need to visit...
	m_hPlayerNodeGoto = pMsg->ReadObject( );
	m_bFaceNodeForwardPitch = pMsg->Readbool( );
	m_bFaceNodeForwardYaw = pMsg->Readbool( );
	m_bAlignPitchWithNode = pMsg->Readbool( );
	m_bAlignYawWithNode = pMsg->Readbool( );

	// Reset node data...
	m_bAtNodePosition = false;
	m_bAtNodeRotation = false;

	LTVector vPlayerPos;
	g_pLTClient->GetObjectPos( g_pMoveMgr->GetObject( ), &vPlayerPos );

	LTVector vNodePos;
	g_pLTClient->GetObjectPos( m_hPlayerNodeGoto, &vNodePos );

	// Ignore height...
	vNodePos.y = vPlayerPos.y;

	LTVector vDir = vNodePos - vPlayerPos;
	vDir.Normalize( );

	// Begin rotation in the direction of the node...
	LTRotation rRot = LTRotation( vDir, LTVector( 0.0f, 1.0f, 0.0f ));
	m_pPlayerCamera->RotateCamera( rRot, m_bAlignPitchWithNode, m_bAlignYawWithNode, false );

	return true;
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CPlayerMgr::UpdateSlowMo()
//
//	PURPOSE:	Update our slowmo state.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateSlowMo( )
{
	if ( (m_eSlowMoState == eEffectState_In || m_eSlowMoState == eEffectState_Entering) && m_bSlowMoPlayer) 
	{
		{
			m_fSlowMoCurCharge -= GameTimeTimer::Instance().GetTimerElapsedS();
		}
	}
	else
	{
		m_fSlowMoCurCharge += GameTimeTimer::Instance().GetTimerElapsedS() * m_fSlowMoRecharge;
	}
	m_fSlowMoCurCharge = LTCLAMP(m_fSlowMoCurCharge,0.0f,m_fSlowMoMaxCharge);

	switch( m_eSlowMoState )
	{
		case eEffectState_Out:
		{
			return;
		}
		break;
		case eEffectState_Entering:
		{
			// Check if we've made it all the way in.
			float fTransitionPeriod = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, TransitionPeriod );
			float fElapsedTime = (float)m_SlowMoTimer.GetElapseTime();
			if( fElapsedTime > fTransitionPeriod )
			{
				m_eSlowMoState = eEffectState_In;

				// Go straight to the looping state in the ClientFX sequence...
				m_fxSlowMoSequence.SetState( CClientFXSequence::eState_Loop );
			}
		}
		break;
		case eEffectState_In:
		{
			// Check if we're in slowmo indefinitely.
			if( m_SlowMoTimer.GetDuration() <= 0.0f )
			{
				return;
			}

			// Check if we ran out of time.
			float fTransitionPeriod = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, TransitionPeriod );
			if( m_SlowMoTimer.IsTimedOut() )//|| m_SlowMoTimer.GetTimeLeft( ) < fTransitionPeriod )
			{
				// Start exiting slowmo.
				ExitingSlowMo();		
			}
			else
			{
				return;
			}
		}
		break;
		case eEffectState_Exiting:
		{
			// Check if we have plenty of time left.
			if( m_SlowMoTimer.IsTimedOut( ))
			{
				ExitSlowMo( false );
				return;
			}
		}
		break;
		default:
		{
			LTERROR( "Invalid slowmo state." );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerMgr::EnterSlowMo
//
//  PURPOSE:	Enter the slowmo state.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::EnterSlowMo( HRECORD hSlowMoRecord, bool bTransition, double fSlowMoPeriod,	bool bPlayer )
{
	// Make sure the slowmo record is valid.
	if( !hSlowMoRecord )
		return;

	m_hSlowMoRecord = hSlowMoRecord;

	// Set when slowmo will wear off.
	if( fSlowMoPeriod )
		m_SlowMoTimer.Start( fSlowMoPeriod );
	else
		m_SlowMoTimer.Start( );

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile && pProfile->m_bSlowMoFX)
	{
		// Set the record used for the ClientFX sequence...
		HRECORD hSlowMoFXSequence = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, ClientFXSequence );
		m_fxSlowMoSequence.SetClientFXSequenceRecord( hSlowMoFXSequence );
	}
	else
	{
		m_fxSlowMoSequence.SetClientFXSequenceRecord( NULL );
	}
	

	// Parameterizes our position in the transition from 0 to 1.
	float fParameter = 0.0f;

	// Check if we're not supposed to transition.
	if( !bTransition )
	{
		// Set the timer for when we need to start transitioning out.
		m_eSlowMoState = eEffectState_In;

		// Go right to full.
		fParameter = 1.0f;

		// Go straight to the looping state in the ClientFX sequence...
		m_fxSlowMoSequence.SetState( CClientFXSequence::eState_Loop );
	}
	else
	{
		// We're entering the slowmo state.
		m_eSlowMoState = eEffectState_Entering;

		// Begin the ClientFX sequence...
		m_fxSlowMoSequence.SetState( CClientFXSequence::eState_Begin );

		fParameter = 0.0f;
	}

	// Set our sounds to use the slowmo mixer.
	HRECORD hMixer = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, Mixer );
	if( hMixer )
	{
		float fMixerTransitionPeriod = ( 1.0f - fParameter ) * GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, TransitionPeriod );
		g_pGameClientShell->GetMixer( )->ProcessMixerByName( hMixer, ( int32 )( fMixerTransitionPeriod * 1000.0f + 0.5f ));
	}

	// Tell the clientfx about the slowmotion state.
	g_pGameClientShell->GetSimulationTimeClientFXMgr().SetInSlowMotion( true );
	g_pGameClientShell->GetRealTimeClientFXMgr().SetInSlowMotion( true );

	m_bSlowMoPlayer = bPlayer;
	if (bPlayer)
	{
		m_fSlowMoCurCharge = fSlowMoPeriod;
	}

	DebugCPrint(0,"%s %0.2f (%0.2f)",__FUNCTION__,RealTimeTimer::Instance().GetTimerAccumulatedS(),fSlowMoPeriod);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ExitingSlowMo
//
//	PURPOSE:	Starts exiting slowmo.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::ExitingSlowMo( )
{
	m_eSlowMoState = eEffectState_Exiting;

	// Check if we're in slowmo indefinitely.
	if( m_SlowMoTimer.GetDuration() <= 0.0f )
	{
		ExitSlowMo( true );
		return;
	}

	// Kill the slowmo mixer over transition period.
	HRECORD hMixer = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, Mixer );
	if( hMixer )
	{
		// Check if we have plenty of time left.
		float fTimeLeft = (float)m_SlowMoTimer.GetTimeLeft( );
		g_pGameClientShell->GetMixer( )->ProcessMixerByName( hMixer, ( int32 )( fTimeLeft * 1000.0f + 0.5f ), true );
	}

	// Finish the ClientFX sequence...
	m_fxSlowMoSequence.SetState( CClientFXSequence::eState_End );


	DebugCPrint(0,"%s %0.2f (%0.2f)",__FUNCTION__,RealTimeTimer::Instance().GetTimerAccumulatedS(),m_SlowMoTimer.GetDuration());

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::EndPlayerSlowMo
//
//	PURPOSE:	End player initiated slowmo
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::EndPlayerSlowMo()
{
	//if we're not in slowmo, or we didn't start it, don't do anything...
	if (!IsInSlowMo() || !m_bSlowMoPlayer)
	{
		return;
	}

	ExitSlowMo(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ExitSlowMo
//
//	PURPOSE:	Exits slowmo.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::ExitSlowMo( bool bTermClientFx )
{
	if( bTermClientFx )
	{
		m_fxSlowMoSequence.SetState( CClientFXSequence::eInvalid_State );
	}

	// Stop the timer.
	m_SlowMoTimer.Stop( );

	// Consider us out of slowmo.
	m_eSlowMoState = eEffectState_Out;

	// Make sure the slowmo mixer is killed.
	if( m_hSlowMoRecord )
	{
		HRECORD hMixer = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, Mixer );
		if( hMixer )
			g_pGameClientShell->GetMixer( )->ProcessMixerByName( hMixer, 0, true );
		m_hSlowMoRecord = NULL;
	}

	// Tell the clientfx about the slowmotion state.
	g_pGameClientShell->GetSimulationTimeClientFXMgr().SetInSlowMotion( false );
	g_pGameClientShell->GetRealTimeClientFXMgr().SetInSlowMotion( false );

	DebugCPrint(0,"%s %0.2f",__FUNCTION__,RealTimeTimer::Instance().GetTimerAccumulatedS());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::InitHeadBobNodeControl()
//
//	PURPOSE:	Turn on/off head bob node control
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::InitHeadBobNodeControl()
{
	HMODELNODE hHeadBobNode;
	HOBJECT hObject = CPlayerBodyMgr::Instance().GetObject();

	if( hObject )
	{
		g_pLTClient->GetModelLT()->GetNode( hObject, "Torso", hHeadBobNode );
		g_pLTClient->GetModelLT()->AddNodeControlFn( hObject, hHeadBobNode, CPlayerMgr::HeadBobNodeControl, this );

		GetHeadBobMgr()->SetCycleNotifyFn( CPlayerMgr::HeadBobCycleDirection, this );
	}
}

// ----------------------------------------------------------------------- //

void CPlayerMgr::TermHeadBobNodeControl()
{
	HMODELNODE hHeadBobNode;
	HOBJECT hObject = CPlayerBodyMgr::Instance().GetObject();

	if( hObject )
	{
		g_pLTClient->GetModelLT()->GetNode( hObject, "Torso", hHeadBobNode );
		g_pLTClient->GetModelLT()->RemoveNodeControlFn( hObject, hHeadBobNode, CPlayerMgr::HeadBobNodeControl, this );

		GetHeadBobMgr()->SetCycleNotifyFn( NULL, NULL );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HeadBobCycleDirection()
//
//	PURPOSE:	Handle a change in cycle direction for the head bob
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HeadBobCycleDirection( bool bMaxExtent, void* pUserData, bool bApproachFromCenter )
{
	CPlayerMgr* pPM = ( CPlayerMgr* )pUserData;

	if( pPM )
	{
		CCharacterFX* pFX = g_pGameClientShell->GetLocalCharacterFX();

		// Play a footstep sound if the headbob record specifies to do so...
		HRECORD hHeadBob = pPM->GetHeadBobMgr( )->GetRecord( );
		if( pFX && hHeadBob && ClientDB::Instance( ).GetBool( hHeadBob, "PlayFootsteps" ))
		{
			SurfaceType eSurf = g_pMoveMgr->GetStandingOnSurface();
			eSurf = ( eSurf == ST_UNKNOWN ) ? pFX->GetLastSurface() : eSurf;

			// Get foot position.
			LTVector vPos;
			g_pLTClient->GetObjectPos( g_pMoveMgr->GetObject(), &vPos );
			char* pSocketName = (char *)(!bMaxExtent ? "LeftFoot" : "RightFoot");
			HMODELSOCKET hSocket;
			ILTModel* pModelLT = g_pLTClient->GetModelLT();
			if (pModelLT->GetSocket(g_pMoveMgr->GetObject(), pSocketName, hSocket) == LT_OK)
			{
				LTTransform transform;
				if (pModelLT->GetSocketTransform(g_pMoveMgr->GetObject(), hSocket, transform, true) == LT_OK)
				{
					vPos = transform.m_vPos;
				}
			}

			// Play footstep sound.
			pFX->PlayFootstepSound( vPos, eSurf, !bMaxExtent );

			// Play splash.
			CPolyGridFX* pPolyGridFX = CPolyGridFX::FindSplashInPolyGrid( g_pMoveMgr->GetObject(), vPos );
			if( pPolyGridFX )
			{
				pPolyGridFX->DoPolyGridSplash( g_pMoveMgr->GetObject(), vPos, g_pModelsDB->GetSplashesFootstepImpulse( pFX->GetModel( )));
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HeadBobCustomCycleDirection()
//
//	PURPOSE:	Handle a change in cycle direction for the head bob
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HeadBobCustomCycleDirection( bool bMaxExtent, void* pUserData, bool bApproachFromCenter )
{
	CPlayerMgr* pPM = ( CPlayerMgr* )pUserData;

	if( bApproachFromCenter )
	{
		if( pPM )
		{
			CCharacterFX* pFX = g_pGameClientShell->GetLocalCharacterFX();

			if( pFX )
			{
				pFX->PlayMovementSound();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HeadBobNodeControlFn()
//
//	PURPOSE:	Update the head bob node control
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HeadBobNodeControl( const NodeControlData& iData, void *pUserData )
{
	CPlayerMgr* pPM = ( CPlayerMgr* )pUserData;
	LTVector vWeaponOffsets = pPM->GetHeadBobMgr()->GetWeaponOffsets();
	LTVector vWeaponRotations = pPM->GetHeadBobMgr()->GetWeaponRotations();

	// Create a new rotation from the weapon rotations
	LTRotation rWeaponRotations( vWeaponRotations.x, vWeaponRotations.y, vWeaponRotations.z );

	// Get the camera vectors
	LTVector vR, vU, vF;
	pPM->GetPlayerCamera()->GetCameraRotation().GetVectors( vR, vU, vF );

	// Calculate a vector in camera space that represents the offset
	LTVector vCameraOffset = ( ( vR * vWeaponOffsets.x ) + ( vU * vWeaponOffsets.y ) + ( vF * vWeaponOffsets.z ) );

	// Now get at the player orientation and recalculate the offset in the space of the character
	LTRotation rPlayer;
	HOBJECT hPlayer = pPM->GetMoveMgr()->GetObject();
	g_pLTClient->GetObjectRotation( hPlayer, &rPlayer );

	rPlayer.GetVectors( vR, vU, vF );

	iData.m_pNodeTransform->m_vPos += LTVector( vR.Dot( vCameraOffset ), vU.Dot( vCameraOffset ), vF.Dot( vCameraOffset ) );
	iData.m_pNodeTransform->m_vPos.y += pPM->m_pPlayerCamera->GetCameraSmoothingOffset( );
	iData.m_pNodeTransform->m_rRot = ( rWeaponRotations * iData.m_pNodeTransform->m_rRot );

	// Now add on any weapon lead/drag
	if( g_vtWeaponLagEnabled.GetFloat() )
	{
		LTRotation rInit( 0.0f, 0.0f, 0.0f, 1.0f );
		LTRotation rLagDelta = pPM->m_pFlashLight->GetLagDelta();
		rLagDelta.Slerp( rInit, rLagDelta, g_vtWeaponLagFactor.GetFloat() );

		iData.m_pNodeTransform->m_rRot = ( ( g_vtWeaponLagReversed.GetFloat() ? rLagDelta.Conjugate() : rLagDelta ) * iData.m_pNodeTransform->m_rRot );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCamera()
//
//	PURPOSE:	Update the camera position/rotation
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateCamera()
{
	// Not really valid until we have a characterfx.
	if( !m_pMoveMgr->GetCharacterFX())
		return;

	//handle updating the camera, as long as we aren't paused
	if( !g_pGameClientShell->IsServerPaused() )
	{
		// Update our camera offset mgr...
		m_pCameraOffsetMgr->Update();

		// Update head-bob effect...
		ObjectContextTimer iPlayerTimer( g_pMoveMgr->GetServerObject() );
		LTVector vVelocity = g_pMoveMgr->GetVelocity();
		vVelocity.y = 0.0f;

		uint32 dwPlayerFlags = GetPlayerFlags();

#ifdef PROJECT_DARK

		static HRECORD hMovement = NULL;
		static bool bDucked = false;
		static bool bStairs = false;

		ClientDB& iClientDB = ClientDB::Instance();
		bool bPlayerOnStairs = WithinStairVolume();
		bool bUpdateMovementRecord = false;

		if( ( dwPlayerFlags & BC_CFLG_DUCK ) && !bDucked )		{ bUpdateMovementRecord = true; bDucked = true; }
		if( !( dwPlayerFlags & BC_CFLG_DUCK ) && bDucked )		{ bUpdateMovementRecord = true; bDucked = false; }
		if( bPlayerOnStairs && !bStairs )						{ bUpdateMovementRecord = true; bStairs = true; }
		if( !bPlayerOnStairs && bStairs )						{ bUpdateMovementRecord = true; bStairs = false; }

		// Get the proper movement record...
		if( bUpdateMovementRecord || !hMovement )
		{
			if( bDucked )
			{
				hMovement = iClientDB.GetRecord( iClientDB.GetMovementCategory(), ( bStairs ? "Player_DuckedStairs" : "Player_Ducked" ) );
			}
			else
			{
				hMovement = iClientDB.GetRecord( iClientDB.GetMovementCategory(), ( bStairs ? "Player_StandingStairs" : "Player_Standing" ) );
			}
		}

		if( hMovement )
		{
			// Find the right threshold for our current speed...
			HATTRIBUTE hAttribute;
			HATTRIBUTE hThresholds = iClientDB.GetAttribute( hMovement, "ThresholdData" );
			uint32 nThresholds = iClientDB.GetNumValues( hMovement, "ThresholdData" );

			hAttribute = iClientDB.GetAttribute( hMovement, "MovementSoundCycleRange" );
			float fMovementSoundCycleRange = iClientDB.GetFloat( hAttribute );

			GetHeadBobMgr()->SetCustomCycleNotifyFn( CPlayerMgr::HeadBobCustomCycleDirection, this, fMovementSoundCycleRange );

			float fVelocity = vVelocity.Mag();
			float fMaxVelocity = g_pMoveMgr->GetMaxVelMag();

			// Go through each threshold and find the one that we want to use
			for( uint32 i = 0; i < nThresholds; ++i )
			{
				hAttribute = iClientDB.GetStructAttribute( hThresholds, i, "InputRange" );
				LTVector2 vInputRange = iClientDB.GetVector2( hAttribute );
				vInputRange.x = LTCLAMP(vInputRange.x, 0.0f, fMaxVelocity);
				vInputRange.y = LTCLAMP(vInputRange.y, 0.0f, fMaxVelocity);

				if( ( fVelocity >= vInputRange.x ) && ( fVelocity <= vInputRange.y ) )
				{
					hAttribute = iClientDB.GetStructAttribute( hThresholds, i, "HeadBob" );
					HRECORD hHeadBob = iClientDB.GetRecordLink( hAttribute );

					hAttribute = iClientDB.GetStructAttribute( hThresholds, i, "HeadBobSpeedScaleOffset" );
					float fHeadBobSpeedScaleOffset = iClientDB.GetFloat( hAttribute );

					hAttribute = iClientDB.GetStructAttribute( hThresholds, i, "MovementSound" );
					HRECORD hMovementSound = iClientDB.GetRecordLink( hAttribute );

					hAttribute = iClientDB.GetStructAttribute( hThresholds, i, "FootstepVolume" );
					float fFootstepVolume = ( iClientDB.GetInt32( hAttribute ) / 100.0f );

					// Setup the head bob parameters...
					float fRangeDelta = ( vInputRange.y - vInputRange.x );

					m_pHeadBobMgr->SetRecord( hHeadBob, 0.2f );
					m_pHeadBobMgr->SetSpeed( fHeadBobSpeedScaleOffset + ( ( fRangeDelta != 0.0f ) ? ( ( fVelocity - vInputRange.x ) / fRangeDelta ) : 0.0f ) );

					// Setup the footstep parameters...
					CCharacterFX* pFX = g_pGameClientShell->GetLocalCharacterFX();

					if( pFX )
					{
						pFX->SetFootstepVolume( fFootstepVolume );
						pFX->SetMovementSound( hMovementSound );
					}

					break;
				}
			}
		}

		// Prototype stair height based footstep sounds
		if( bPlayerOnStairs )
		{
			LTVector vMoveObjectPos;
			g_pLTClient->GetObjectPos( g_pMoveMgr->GetServerObject(), &vMoveObjectPos );

			static float fOnStairHeight = 0.0f;
			static float fPrevTrackedHeight = 0.0f;
			static bool bOnStairLeftFoot = true;

			if( ( fabs( vMoveObjectPos.y - fPrevTrackedHeight ) < 1.0f ) && ( fabs( vMoveObjectPos.y - fOnStairHeight ) > 10.0f ) )
			{
				CCharacterFX* pFX = g_pGameClientShell->GetLocalCharacterFX();

				if( pFX )
				{
					SurfaceType eSurf = g_pMoveMgr->GetStandingOnSurface();
					eSurf = ( eSurf == ST_UNKNOWN ) ? pFX->GetLastSurface() : eSurf;

					LTVector vPos;
					g_pLTClient->GetObjectPos( g_pMoveMgr->GetObject(), &vPos );

					pFX->PlayFootstepSound( vPos, eSurf, bOnStairLeftFoot );
					bOnStairLeftFoot = !bOnStairLeftFoot;
				}

				fOnStairHeight = vMoveObjectPos.y;
			}

			fPrevTrackedHeight = vMoveObjectPos.y;
		}

#else//PROJECT_DARK

		float fTransitionTime = g_vtHeadBobTransitionTime.GetFloat( );

		// Don't use head bob when jumping or falling...
		if( m_pMoveMgr->IsFalling( ) || m_pMoveMgr->Jumped( ) || IsSpectating( ) ||
			LadderMgr::Instance( ).IsClimbing( ) || SpecialMoveMgr::Instance().IsActive() ||
			!CPlayerBodyMgr::Instance( ).CanUseHeadBob( ) || g_pPlayerMgr->IsOperatingTurret( ) ||
			IsUnderwater( ))
		{
			m_pHeadBobMgr->SetRecord( "None", fTransitionTime );
			m_pHeadBobMgr->SetSpeed( 0.0f );
		}
		else if( CPlayerBodyMgr::Instance().GetCurrentAnimProp( CPlayerBodyMgr::kLowerContext, kAPG_Movement ) == kAP_MOV_Idle )
		{
			// Play special Idle headbob to simulate breathing....
			m_pHeadBobMgr->SetRecord( "Idle", fTransitionTime );
		}
		else
		{
			if( dwPlayerFlags & BC_CFLG_RUN )
			{
				if( dwPlayerFlags & BC_CFLG_STRAFE_RIGHT )
				{
					m_pHeadBobMgr->SetRecord( "StrafeRight", fTransitionTime );
				}
				else if( dwPlayerFlags & BC_CFLG_STRAFE_LEFT )
				{
					m_pHeadBobMgr->SetRecord( "StrafeLeft", fTransitionTime );
				}
				else if( dwPlayerFlags & BC_CFLG_FORWARD )
				{
					m_pHeadBobMgr->SetRecord( "Run", fTransitionTime );
				}
				else if( dwPlayerFlags & BC_CFLG_REVERSE )
				{
					m_pHeadBobMgr->SetRecord( "RunBack", fTransitionTime );
				}
			}
			else
			{
				if( dwPlayerFlags & BC_CFLG_STRAFE_RIGHT )
				{
					m_pHeadBobMgr->SetRecord( "WalkStrafeRight", fTransitionTime );
				}
				else if( dwPlayerFlags & BC_CFLG_STRAFE_LEFT )
				{
					m_pHeadBobMgr->SetRecord( "WalkStrafeLeft", fTransitionTime );
				}
				else if( dwPlayerFlags & BC_CFLG_FORWARD )
				{
					m_pHeadBobMgr->SetRecord( "Walk", fTransitionTime );
				}
				else if( dwPlayerFlags & BC_CFLG_REVERSE )
				{
					m_pHeadBobMgr->SetRecord( "WalkBack", fTransitionTime );
				}
			}
			
			m_pHeadBobMgr->SetSpeed( 1.0f );
		}

#endif//PROJECT_DARK

		m_pHeadBobMgr->Update( iPlayerTimer.GetTimerElapsedS() );

		// Update the player camera...
		m_pPlayerCamera->Update();

		// Make sure the player gets updated
 		if (IsMultiplayerGameClient() || g_pInterfaceMgr->AllowCameraMovement())
		{
			UpdatePlayerInfo(true, false);
		}
	}
	// Remember to continue updating the server if we're in multiplayer
	else if (IsMultiplayerGameClient())
	{
		UpdatePlayerInfo(false, false);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateWeaponModel()
//
//	PURPOSE:	Update the weapon model
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateWeaponModel()
{
	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;


	// Decay weapon recoil...
	DecayWeaponRecoil();


	// If possible, get these values from the camera, because it
	// is more up-to-date...

	LTRotation rRot = m_pPlayerCamera->GetCameraRotation( );
	LTVector vPos = m_pPlayerCamera->GetCameraPos( );

	if( m_pPlayerCamera->GetCameraMode() != CPlayerCamera::kCM_FirstPerson )
	{
		// Use the gun's flash orientation...
		GetAttachmentSocketTransform(hPlayerObj, "Flash", vPos, rRot);
	}


	// GRENADE PROTOTYPE
	{
		static WeaponState eLastWeaponstate = W_INACTIVE;
		if ( GrenadeReady() &&
			!CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_THROW_GRENADE) &&
			( IsPlayerAlive() ) &&
			( !IsSpectating( )) &&
			( g_pInterfaceMgr->GetGameState() == GS_PLAYING ) )
		{
			//simulate fire key being held until we've been in the firing state for two updates
			if( eLastWeaponstate != W_FIRING )
			{
				m_dwPlayerFlags |= BC_CFLG_FIRING;
			}
			eLastWeaponstate = m_pClientWeaponMgr->GetCurrentWeaponState();
		}
		else
		{
			eLastWeaponstate = W_INACTIVE;
		}
	}

	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	//if we've just started firing, clear out our fire request
	if (pClientWeapon && pClientWeapon->IsSemiAuto() && m_bSemiAutoFire )
	{
		if (m_bSemiAutoFireDuringReload && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_FIRING))
		{
			m_bSemiAutoFireDuringReload = false;
		}

		//how long ago did we request this?
		double fElapsed = (RealTimeTimer::Instance().GetTimerAccumulatedS() - m_fSemiAutoFireTime);
		static float fBufferTime = g_pWeaponDB->GetSemiAutoBufferTime();
		if (fElapsed > fBufferTime && !m_bSemiAutoFireDuringReload) 
		{
			//the request is too old, toss it
			m_bSemiAutoFire = false;
		}

		static WeaponState eLastWeaponstate = W_INACTIVE;
		WeaponState eCurrent = pClientWeapon->GetState();
		if (eCurrent == W_FIRING && eLastWeaponstate != W_FIRING)
		{
			m_bSemiAutoFire = false;
		}
//		else if (eCurrent == W_RELOADING)
//		{
//			m_bSemiAutoFire = false;
//		}

		eLastWeaponstate = eCurrent;
	}

	// Update the model position and state...
	WeaponState eWeaponState = m_pClientWeaponMgr->Update( rRot, vPos );

	// Update the player body grenade...
	CPlayerBodyMgr::Instance().UpdateGrenadePosition( );

	// Do fire camera jitter...

	if (FiredWeapon(eWeaponState))
	{
		m_LastFireTimer.Start( );


		if( m_pPlayerCamera->GetCameraMode() == CPlayerCamera::kCM_FirstPerson )
		{
			StartWeaponRecoil();
		}

		// GRENADE PROTOTYPE
		if (FiredWeapon(eWeaponState) && m_hPreGrenadeWeapon)
		{
			g_pClientWeaponMgr->ChangeWeapon( m_hPreGrenadeWeapon );
		}

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::StartWeaponRecoil()
//
//	PURPOSE:	Start the weapon recoiling...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::StartWeaponRecoil()
{
	float fRecoilKick = g_vtCamRecoilKick.GetFloat();
	float fRecoilFactor = g_vtCamRecoilFactor.GetFloat();
	float fRecoilScale = g_vtCamRecoilScale.GetFloat();
	float fRecoilMax = g_vtCamRecoilMax.GetFloat();


	// Move view up a bit...(based on the current weapon/ammo type)

	float fPitchRatio = 1.0f;
	float fYawRatio	= 0.333f;
	HWEAPON hWeapon	= NULL;
	HAMMO hAmmo		= NULL;
	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		hWeapon	= pClientWeapon->GetWeaponRecord();
		hAmmo	= pClientWeapon->GetAmmoRecord();
	}

	if( hWeapon && hAmmo )
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo);
		float fFireRecoilKick	= g_pWeaponDB->GetFloat( hWpnData, WDB_WEAPON_fFireRecoilKick );
		float fFireRecoilMult	= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fFireRecoilMult );
		//if the console override variable is not set, use the DB value
		if (fRecoilKick < 0.0f)
		{
			fRecoilKick = fFireRecoilKick * fFireRecoilMult;
		}
		
		fYawRatio = g_pWeaponDB->GetFloat( hWpnData, WDB_WEAPON_fFireRecoilYawRatio );

	}

	if (fYawRatio > 1.0f) 
	{
		fPitchRatio = 1.0f / fYawRatio;
		fYawRatio = 1.0f;
	}
	else
	{
		fPitchRatio = 1.0f;
	}

	LTRotation rRot = m_pPlayerCamera->GetLocalCameraRotation();
	EulerAngles EA = Eul_FromQuat( rRot, EulOrdYXZr );
	float fPitch = RAD2DEG(EA.y);


	m_RecoilDecayTimer.Start( );

	/***Recoil with exponential recoil curve
	double fKickMult = 1.0f + (m_fCurrentRecoil * fRecoilFactor);
	float fDelta = (float)(fRecoilKick * fKickMult * fRecoilScale);
	m_fCurrentRecoil += fDelta;
	***/

	/***Recoil with linear recoil curve*/
	float fDelta = (fRecoilKick * fRecoilScale);
	m_fCurrentRecoil += fDelta * fRecoilFactor;
	/***/

	float fPitchDiff = (GetRandom(-fDelta,fDelta) - (fDelta * 0.5f)) * fPitchRatio;
	if (m_fCurrentRecoil > fRecoilMax)
		m_fCurrentRecoil = fRecoilMax;

	if (g_vtCamRecoilDebug.GetFloat() > 0.0f)
		g_pLTClient->CPrint("recoil up to: %0.2f",m_fCurrentRecoil);

	float fYawDiff = GetRandom(-fDelta,fDelta) * fYawRatio;

	//don't go side to side in debug
	if (g_vtCamRecoilDebug.GetFloat() > 0.0f)
		fYawDiff = 0.0f;

	if (AimMgr::Instance().IsAiming() && g_vtCamRecoilAimDampen.GetFloat() < 1.0f)
	{
		fPitchDiff *= g_vtCamRecoilAimDampen.GetFloat();
		fYawDiff *= g_vtCamRecoilAimDampen.GetFloat();
	}

	m_pPlayerCamera->ApplyLocalRotationOffset( LTVector( DEG2RAD(fPitchDiff), DEG2RAD(fYawDiff), 0.0f ));
		
	if (fDelta > 0.0f)
	{
		//create a clientFX that will be positioned at the fire position at the appropriate
		//orientation
		CLIENTFX_CREATESTRUCT fxCS( "WeaponRecoil", 0 );
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::DecayWeaponRecoil()
//
//	PURPOSE:	Decay the weapon's recoil...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::DecayWeaponRecoil()
{

	if (m_fCurrentRecoil <= 0.0f)
		return;

	float fElapsedTime = (float)m_RecoilDecayTimer.GetElapseTime();

	//don't decay immediately
	if (fElapsedTime < 0.03f)
		return;
	
	float fRecoilRecover = g_vtCamRecoilRecover.GetFloat();

	if (fRecoilRecover < 0.0f)
	{
		HWEAPON hWeapon	= NULL;
		CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		if ( pClientWeapon )
		{
			hWeapon	= pClientWeapon->GetWeaponRecord();
		}

		if( hWeapon )
		{
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
			fRecoilRecover = g_pWeaponDB->GetFloat( hWpnData, WDB_WEAPON_fFireRecoilDecay );
		}
	}

	/***Recoil with exponential recoil curve
	ObjectContextTimer objectContextTimer( g_pMoveMgr->GetServerObject( ));
	float fRecover = fRecoilRecover * fElapsedTime * g_vtCamRecoilRecoverFactor.GetFloat() * objectContextTimer.GetTimerElapsedS( ) * GetSkillValue(eAimCorrection);
	m_fCurrentRecoil -= fRecover;
	***/

	/***Recoil settings for use with linear recoil curve*/
	ObjectContextTimer objectContextTimer( g_pMoveMgr->GetServerObject( ));
	float fRecover = fRecoilRecover * g_vtCamRecoilRecoverFactor.GetFloat() * objectContextTimer.GetTimerElapsedS( ) * GetSkillValue(eAimCorrection);
	m_fCurrentRecoil -= fRecover;
	/***/

	if (m_fCurrentRecoil < 0.0f)
	{
		m_fCurrentRecoil = 0.0f;
		if (g_vtCamRecoilDebug.GetFloat() > 0.0f)
			g_pLTClient->CPrint("recovered in: %0.2f",fElapsedTime);

	}
	else
	{
		if (g_vtCamRecoilDebug.GetFloat() > 1.0f)
		g_pLTClient->CPrint("recoil down to: %0.2f",m_fCurrentRecoil);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::GetRecoilValue()
//
//	PURPOSE:	Get the current recoil value (0.0 to 1.0)
//
// ----------------------------------------------------------------------- //
float CPlayerMgr::GetRecoilValue() const
{
	float fRecoilMax = g_vtCamRecoilMax.GetFloat();

	float fPerturb = sqrtf(m_fCurrentRecoil / fRecoilMax);
	return min(fPerturb,1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CanRespawn()
//
//	PURPOSE:	Checks if the player can respawn.
//
// ----------------------------------------------------------------------- //
bool CPlayerMgr::CanRespawn( ) const
{
	if( IsPlayerAlive( ))
		return false;

	// Not allowed to respawn until we're fully dead.
	if( m_ePlayerState == ePlayerState_Dying_Stage1 || m_ePlayerState == ePlayerState_Dying_Stage2 )
		return false;

	// Don't allow respawning if we're waiting for a timeout.
	if( m_EarliestRespawnTime.IsStarted() && !m_EarliestRespawnTime.IsTimedOut())
		return false;

	// Check if the game mode doesn't allow it.
	if( !GameModeMgr::Instance( ).m_grbAllowRespawnFromDeath )
		return false;

	// If we have a grace period then the grace period will put us in automatically
	// when it ends.
	if( GameModeMgr::Instance( ).m_grnJoinGracePeriod > 0 && g_pMissionMgr->GetServerGameState() == EServerGameState_GracePeriod )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetSpectatorMode()
//
//	PURPOSE:	Turn spectator mode on/off
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetSpectatorMode( SpectatorMode eSpectatorMode )
{
/*
	switch( eSpectatorMode)
	{
	case eSpectatorMode_Clip:
		DebugCPrint(0,"CPlayerMgr::SetSpectatorMode () : eSpectatorMode_Clip");
		break;
	case eSpectatorMode_Fly:
		DebugCPrint(0,"CPlayerMgr::SetSpectatorMode () : eSpectatorMode_Fly");
		break;
	case eSpectatorMode_Follow:
		DebugCPrint(0,"CPlayerMgr::SetSpectatorMode () : eSpectatorMode_Follow");
		break;
	case eSpectatorMode_Fixed:
		DebugCPrint(0,"CPlayerMgr::SetSpectatorMode () : eSpectatorMode_Fixed");
		break;
	case eSpectatorMode_Tracking:
		DebugCPrint(0,"CPlayerMgr::SetSpectatorMode () : eSpectatorMode_Tracking");
		break;
	case eSpectatorMode_None:
		DebugCPrint(0,"CPlayerMgr::SetSpectatorMode () : eSpectatorMode_None");
		break;

	};
*/	

	// Check if we're already at the requested mode.
	if( m_eSpectatorMode == eSpectatorMode )
		return;

	g_pHUDMgr->QueueUpdate(kHUDSpectator);

	m_eSpectatorMode = eSpectatorMode;
	
	g_pHUDMgr->UpdateRenderLevel();

	switch( GetSpectatorMode())
	{
	case eSpectatorMode_Clip:
	case eSpectatorMode_Fly:
		{
			CPlayerBodyMgr::Instance( ).HidePlayerBody(true);
			m_pClientWeaponMgr->HideWeapons();
			m_pClientWeaponMgr->DisableWeapons();
			g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
			m_pMoveMgr->SetSpectatorMode(eSpectatorMode);
			m_pPlayerCamera->SetTargetObject(g_pMoveMgr->GetObject());
			m_pPlayerCamera->AttachToTarget(false);
			m_pPlayerCamera->SetCameraMode( CPlayerCamera::kCM_Fly );
			SetSpectatorFollowCharacter( NULL );
		}
		break;
	case eSpectatorMode_Follow:
		{
			CPlayerBodyMgr::Instance( ).HidePlayerBody(true);
			m_pClientWeaponMgr->HideWeapons();
			m_pClientWeaponMgr->DisableWeapons();
			g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
			m_pMoveMgr->SetSpectatorMode(eSpectatorMode);
			m_pPlayerCamera->SetCameraMode( CPlayerCamera::kCM_Follow );
			
			// Chose an initial character to follow.
			CCharacterFX* pFound = m_hSpectatorFollowCharacter ? g_pGameClientShell->GetSFXMgr()->GetCharacterFX( m_hSpectatorFollowCharacter ) : NULL;
			if( !pFound )
				pFound = GetNextSpectatorFollow( NULL );

			SetSpectatorFollowCharacter( pFound );
		}
		break;
	case eSpectatorMode_Fixed:
		{
			CPlayerBodyMgr::Instance( ).HidePlayerBody(true);
			m_pClientWeaponMgr->HideWeapons();
			m_pClientWeaponMgr->DisableWeapons();
			g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
			m_pMoveMgr->SetSpectatorMode(eSpectatorMode);
			SetSpectatorFollowCharacter( NULL );
			m_pPlayerCamera->SetCameraMode( CPlayerCamera::kCM_Fixed );
			m_pPlayerCamera->AttachToTarget( true );
		}
		break;
	case eSpectatorMode_Tracking:
		{
			CPlayerBodyMgr::Instance( ).HidePlayerBody(true);
			m_pClientWeaponMgr->HideWeapons();
			m_pClientWeaponMgr->DisableWeapons();
			g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
			m_pMoveMgr->SetSpectatorMode(eSpectatorMode);
			SetSpectatorFollowCharacter( NULL );
			if( GetSpectatorTrackTarget( ))
				m_pPlayerCamera->SetTargetObject( GetSpectatorTrackTarget()); 
			m_pPlayerCamera->SetCameraMode( CPlayerCamera::kCM_Track );
			m_pPlayerCamera->AttachToTarget( true );
		}
		break;
	default:
	case eSpectatorMode_None:
		{
			if( IsPlayerAlive( ))
			{
				g_pPlayerMgr->GetPlayerCamera()->ClearCameraClamping();
				SetSpectatorFollowCharacter( NULL );
				SetSpectatorTrackTarget( NULL );
				m_pPlayerCamera->SetTargetObject( m_pMoveMgr->GetObject( ));
				m_pPlayerCamera->AttachToTarget(true);
				CPlayerBodyMgr::Instance( ).Enable();
				CPlayerBodyMgr::Instance( ).HidePlayerBody(false);
				m_pClientWeaponMgr->ShowWeapons();
				m_pClientWeaponMgr->EnableWeapons();
				m_pPlayerCamera->SetCameraMode( CPlayerCamera::kCM_FirstPerson );
				m_pMoveMgr->SetSpectatorMode(eSpectatorMode);
			}
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::AllowSpectatorMode
//
//	PURPOSE:	Checks to see if specatator mode is allowed.
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::AllowSpectatorMode( SpectatorMode eSpectatorMode ) const
{
	// Check if we're allowed to use this mode.
	switch( eSpectatorMode ) 
	{
	case eSpectatorMode_Clip:
		{
			// No clipping in mp.
			if( !IsMultiplayerGameClient( ))
				return true;
		}
		break;
		// Only allow switching to tracking if we have a track target.
	case eSpectatorMode_Tracking:
		{
			if( !GetSpectatorTrackTarget())
				return false;

			// Check if gamemode doesn't allow this.
			if( !GameModeMgr::Instance().m_grbAllowKillerTrackSpectating )
				return false;

			// If they aren't visible anymore, then skip them.
			uint32 nFlags = 0;
			g_pCommonLT->GetObjectFlags( GetSpectatorTrackTarget(), OFT_Flags, nFlags );
			if( !( nFlags & FLAG_VISIBLE ))
				return false;

			return true;
		}
		break;
		// Only allow switching to follow if there is someone to follow.
	case eSpectatorMode_Follow:
		{
			if( GetNextSpectatorFollow( NULL ))
				return true;
		}
		break;
	case eSpectatorMode_Fixed:
	case eSpectatorMode_Fly:
		{
			return true;
		}
		break;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::GetNextSpectatorMode
//
//	PURPOSE:	Gets the next spectator mode based on a current one.
//
// ----------------------------------------------------------------------- //

SpectatorMode CPlayerMgr::GetNextSpectatorMode( SpectatorMode eSpectatorMode ) const
{
	SpectatorMode eNewSpectatorMode = eSpectatorMode;
	for( ;; )
	{
		// Advance to the next mode.
		eNewSpectatorMode = ( SpectatorMode )(( uint32 )eNewSpectatorMode + 1 );

		// Handle wrapping.
		if( eNewSpectatorMode == eSpectatorMode_Count )
			eNewSpectatorMode = ( SpectatorMode )0;

		// If we reached the old mode, then we didn't find a different one.
		if( eNewSpectatorMode == eSpectatorMode )
			return eNewSpectatorMode;

		// Don't stop on none or fixed.
		if( eNewSpectatorMode == eSpectatorMode_None || eNewSpectatorMode == eSpectatorMode_Fixed )
			continue;

		// Check if spectator mode is allowed.
		if( AllowSpectatorMode( eNewSpectatorMode ))
			return eNewSpectatorMode;

		// Still no valid mode.
		continue;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::GetNextSpectatorFollow
//
//	PURPOSE:	Gets the next character to follow.
//
// ----------------------------------------------------------------------- //

CCharacterFX* CPlayerMgr::GetNextSpectatorFollow( CCharacterFX* pCurCharFx ) const
{
	// No one to follow if there is less than 2 people.
	if( CCharacterFX::GetCharFXList( ).size( ) < 2 )
		return NULL;

	CCharacterFX* pNewCharFx = NULL;
	CCharacterFX::CharFXList::iterator iter;

	// If we don't have a starting place, start from the end.
	if( !pCurCharFx )
	{
		iter = CCharacterFX::GetCharFXList( ).begin( ) + ( CCharacterFX::GetCharFXList( ).size( ) - 1 );
	}
	else
	{
		// Find our current place in the list.
		iter = std::find( CCharacterFX::GetCharFXList( ).begin( ), CCharacterFX::GetCharFXList( ).end( ), pCurCharFx );
	}

	// Find the next available target.
	bool bWrapped = false;
	for( ;; )
	{
		// Advance to the next character.
		iter++;

		// Handle wrapping.
		if( iter == CCharacterFX::GetCharFXList( ).end( ))
		{
			// Never found anyone if we've already wrapped before.
			if( bWrapped )
				return NULL;

			iter = CCharacterFX::GetCharFXList( ).begin( );
			bWrapped = true;
		}

		pNewCharFx = *iter;

		// Don't stop on me.
		if( pNewCharFx == g_pMoveMgr->GetCharacterFX( ))
			continue;

		// Skip dead guys.
		if( pNewCharFx->m_cs.bIsDead )
			continue;

		// Skip no model guys.
		if( !pNewCharFx->m_cs.hModel )
			continue;

		// Skip players with no clientid.
		if( pNewCharFx->m_cs.bIsPlayer && pNewCharFx->m_cs.nClientID == INVALID_CLIENT )
			continue;

		// Skip invisible guys, like other spectators.
		uint32 nFlags = 0;
		g_pCommonLT->GetObjectFlags( pNewCharFx->GetServerObj(), OFT_Flags, nFlags );
		if( !( nFlags & FLAG_VISIBLE ))
			continue;

		return pNewCharFx;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetSpectatorFollowCharacter()
//
//	PURPOSE:	Set who we should follow.
//
// ----------------------------------------------------------------------- //
void CPlayerMgr::SetSpectatorFollowCharacter( CCharacterFX* pCharFx )
{
	// Make the current follow character visible again.
	if( m_hSpectatorFollowCharacter )
	{
		g_pLTBase->Common()->SetObjectFlags( m_hSpectatorFollowCharacter, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
		CCharacterFX* pCurCharFx = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( m_hSpectatorFollowCharacter );
		if( pCurCharFx )
			pCurCharFx->DoUpdateAttachments();
	}

	// Set it as our target.
	LTRigidTransform transform;
	if( pCharFx )
	{
		m_hSpectatorFollowCharacter = pCharFx->GetServerObj( );
		g_pLTBase->GetObjectTransform( pCharFx->GetServerObj( ), &transform );
		
		// Hide the new character.
		g_pLTBase->Common()->SetObjectFlags( m_hSpectatorFollowCharacter, OFT_Flags, 0, FLAG_VISIBLE );
		pCharFx->DoUpdateAttachments();
	}
	else
	{
		m_hSpectatorFollowCharacter = NULL;
		g_pLTBase->GetObjectTransform( GetMoveMgr()->GetObject(), &transform );
	}

	if( m_hSpectatorFollowCharacter )
	{
		m_pPlayerCamera->SetTargetObject( m_hSpectatorFollowCharacter );
		m_pPlayerCamera->AttachToTarget(true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateSpectatorFollowTarget()
//
//	PURPOSE:	Ensures the target being followed is hidden to the player.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateSpectatorFollowTarget( )
{
	// The character being followed, and it's attachments needs to remain be set invisible 
	// every update since the server may change the character flags at any time.
	if( m_hSpectatorFollowCharacter )
	{
		g_pCommonLT->SetObjectFlags( m_hSpectatorFollowCharacter, OFT_Flags, 0, FLAG_VISIBLE );

		CCharacterFX* pSpectatorCharFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( m_hSpectatorFollowCharacter );
		if( pSpectatorCharFX )
		{
			pSpectatorCharFX->DoUpdateAttachments( );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetInvisibleMode()
//
//	PURPOSE:	Turn invisible mode on/off
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetInvisibleMode(bool bOn)
{
	if( m_bInvisibleMode == bOn )
		return;

	m_bInvisibleMode = bOn;
	if( m_bInvisibleMode )
	{
		CPlayerBodyMgr::Instance( ).HidePlayerBody(true);
		m_pClientWeaponMgr->HideWeapons();
	}
	else
	{
		CPlayerBodyMgr::Instance( ).HidePlayerBody(false);
		m_pClientWeaponMgr->ShowWeapons();
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleWeaponChanged()
//
//	PURPOSE:	Handle updating necessary player data after a weapon
//				change occurs...(NOTE: this should only be called from the
//				CClientWeaponMgr::ChangeWeapon() after a successful 
//				weapon change)
//
// ----------------------------------------------------------------------- //

	// jrg 9/2/02 - this will get called twice most of the time when switching weapons:
	//			once when the weapon switch is started
	//			and again when it completes (if a switching animation was needed)
	//		bImmediateSwitch will be true on the second call (or the first if the switch is immediate)
	//		(I'm using the repeated call to track whether we are in mid switch)

void CPlayerMgr::HandleWeaponChanged( HWEAPON hWeapon, HAMMO hAmmo, bool bImmediateSwitch )
{
	// Turn off zooming...
	AimMgr::Instance().EndAim();
	m_bSemiAutoFire = false;

	g_pHUDMgr->QueueUpdate(kHUDWeapon);

	// Tell the server to change weapons...
	LTRESULT ltResult;
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_CHANGE );

	cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, hAmmo );

	ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	ASSERT( LT_OK == ltResult );

	// this isn't strictly necessary, the hide should have been
	// called when we entered the chase view
	if( m_pPlayerCamera->GetCameraMode() != CPlayerCamera::kCM_FirstPerson )
	{
		m_pClientWeaponMgr->HideWeapons();
	}

	//when the swicth is complete...
	if (bImmediateSwitch)
	{
//		const char* pszName = g_pLTDatabase->GetRecordName(hWeapon);
//		DebugCPrint(0,"%s - %s",__FUNCTION__,pszName);

		//force the end of the zoom state, in case it hasn't ended normally at this point
		m_pPlayerCamera->ExitZoom();


		//if we are changing to a Grenade, we're done switching
		if (m_bChangingToGrenade)
		{
			m_bChangingToGrenade = false;
		}
		else
		{
			//we are not changing to a Grenade, so forget about any Grenade related stuff...
			m_hPreGrenadeWeapon = NULL;
		}

		m_fCurrentRecoil = 0.0f;
    }

#ifndef _FINAL
	if (g_vtEnableSimulationLog.GetFloat())
	{
		WriteWeaponChangeToSimulationLog(hWeapon, hAmmo);
	}
#endif

	m_bSwitchingWeapons = !bImmediateSwitch;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnCommandOn()
//
//	PURPOSE:	Handle client commands
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::OnCommandOn(int command)
{
	// Make sure we're in the world and can do things.
	if (!IsPlayerInWorld() || 
		( g_pMissionMgr->GetServerGameState() != EServerGameState_Playing &&
		g_pMissionMgr->GetServerGameState() != EServerGameState_EndingRound && 
		g_pMissionMgr->GetServerGameState() != EServerGameState_PlayingSuddenDeath ))
		return false;

	// See if the vehiclemgr would like to trap this
	if(m_pMoveMgr->GetVehicleMgr()->OnCommandOn(command))
	{
		return true;
	}

	// Check for weapon change...

	if( g_pClientWeaponMgr->OnCommandOn( command ))
	{
		return true;
	}

	if (AimMgr::Instance().OnCommandOn(command))
	{
		return true;
	}

	// Take appropriate action

		switch (command)
		{
		case COMMAND_ID_SLOWMO :
			{
				if( GameModeMgr::Instance( ).m_grbUseSlowMo )
				{
					if (!IsPlayerAlive() || m_bStoryMode || GetPlayerCamera()->GetCameraMode() != CPlayerCamera::kCM_FirstPerson)
						return false;
					
					g_pHUDSlowMo->ResetFade();
					if (IsInSlowMo())
					{
						if (!m_bSlowMoPlayer)
						{
							DebugCPrint(0,"CPlayerMgr::OnCommandOn(COMMAND_ID_SLOWMO) - not in player slow mo");
							return false;
						}
						DebugCPrint(0,"CPlayerMgr::OnCommandOn(COMMAND_ID_SLOWMO) - kSlowMoEnd");
						CAutoMessage cMsg;
						cMsg.Writeuint8(MID_SLOWMO);
						cMsg.Writeuint8(kSlowMoEnd);
						return (LT_OK == g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED));						
					}
					else
					{
						if (m_fSlowMoCurCharge >= m_fSlowMoMinCharge)
						{
							DebugCPrint(0,"CPlayerMgr::OnCommandOn(COMMAND_ID_SLOWMO) - kSlowMoStart");
							CAutoMessage cMsg;
							cMsg.Writeuint8(MID_SLOWMO);
							cMsg.Writeuint8(kSlowMoStart);
							return (LT_OK == g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED));
						}
					}
				}
			}
			break;

		case COMMAND_ID_ACTIVATE :
		{
			// See if we're spectating.
			if( IsSpectating( ))
			{
				SpectatorMode eNextMode = GetNextSpectatorMode( GetSpectatorMode( ));
				if( eNextMode != GetSpectatorMode( ))
				{
					RequestSpectatorMode( eNextMode );
				}
			}
			else if (!IsPlayerAlive())
			{
				// Go to spectator if we can.
				SpectatorMode eNextMode = GetNextSpectatorMode( eSpectatorMode_None );
				if( eNextMode != eSpectatorMode_None )
				{
					RequestSpectatorMode( eNextMode );
				}
			}
			else
			{
				// If we're pointed at something activate it if we're not
				// riding a vehicle...
				if (m_pMoveMgr->GetVehicleMgr()->CanDismount())
				{
					// Vehicle Mgr now handles calling DoActivate(), just tell
					// the vehicle mgr to get off the vehicle...
					m_pMoveMgr->GetVehicleMgr()->SetPhysicsModel(PPM_NORMAL);
					return true;
				}
				else if( m_pTurretFX )
				{
					// Stop using the turret...
					m_pTurretFX->Deactivate( );
				}
				else if( m_PickupObjectDetector.GetObject( ))
				{
					HOBJECT hPickupObject = g_pPlayerMgr->GetPickupObjectDetector().GetObject( );
					if( !hPickupObject )
						return false;

					bool bFound = false;

					CPickupItemFX* pPickupItemFX = static_cast< CPickupItemFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_PICKUPITEM_ID, hPickupObject ));
					if( pPickupItemFX )
					{
#ifdef PROJECT_DARK
						CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
						if (pClientWeapon)
						{
							HOBJECT hWeaponModel = pClientWeapon->GetRightHandModel();

							LTRigidTransform tWeaponModel;
							g_pLTClient->GetObjectTransform(hWeaponModel, &tWeaponModel);

						CAutoMessage cMsg;
							cMsg.Writeuint8( MID_PICKUPITEM_ACTIVATE_EX );
							cMsg.WriteObject( hPickupObject );
							cMsg.Writebool( pPickupItemFX->IsMustSwap() );
							cMsg.WriteLTRigidTransform( tWeaponModel );
							return (LT_OK == g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED));
						}
#endif

						bFound = true;

						CAutoMessage cMsg;
						cMsg.Writeuint8( MID_PICKUPITEM_ACTIVATE );
						cMsg.WriteObject( hPickupObject );
						cMsg.Writebool( pPickupItemFX->IsMustSwap( ));
						return (LT_OK == g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED));
					}

					if( !bFound )
					{
						// See if it's a ctfflag.
						CTFFlagSFX* pCTFFlagSFX = reinterpret_cast< CTFFlagSFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_CTFFLAG_ID, hPickupObject ));
						if( pCTFFlagSFX )
						{
							bFound = true;
							CAutoMessage cMsg;
							cMsg.Writeuint8( MID_PICKUPITEM_ACTIVATE );
							cMsg.WriteObject( hPickupObject );
							cMsg.Writebool( false );
							return (LT_OK == g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED));
						}
					}

					if( !bFound )
					{
						// See if it's a recoverable projectile
						CProjectileFX* pProjSFX = reinterpret_cast< CProjectileFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_PROJECTILE_ID, hPickupObject ));
						if( pProjSFX )
						{
							bFound = true;
							CAutoMessage cMsg;
							cMsg.Writeuint8( MID_PICKUPITEM_ACTIVATE );
							cMsg.WriteObject( hPickupObject );
							cMsg.Writebool( false );
							return (LT_OK == g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED));
						}
					}

					if( !bFound )
						return false;
				}
				else if (GetTargetMgr()->IsTargetInRange())
				{
					//jrg 1/23/03 - if you're not allowed weapons, you can't activate stuff either
					if (g_pDamageFXMgr->AllowWeapons( ))
						DoActivate();
				}
			}
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if (IsPlayerAlive())
			{
				// Check for finishing move activation.
				CActivationData data = GetTargetMgr()->GetActivationData();
				if (data.m_nType == MID_ACTIVATE_SPECIALMOVE)
				{
					CSpecialMoveFX *pSpecialMove = g_pGameClientShell->GetSFXMgr()->GetSpecialMoveFX(data.m_hTarget);
					if (pSpecialMove && (pSpecialMove->GetSFXID() == SFX_FINISHINGMOVE_ID))
					{
						SpecialMoveMgr::Instance().Activate(pSpecialMove);
						break;
					}
				}

				CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();

				if( pClientWeapon )
				{
					// First check for activating forensics or entry tools...
					uint8 nActivateType = pClientWeapon->GetActivationType();	//!!ARL: Maybe get rid of crazy activate types and just check the ActionIcon?  (or just add bForensic, bEntry, etc)
					if (IS_ACTIVATE_FORENSIC(nActivateType) || IS_ACTIVATE_ENTRY(nActivateType))
					{
						CActivationData data = GetTargetMgr()->GetActivationData();
						if (data.m_nType == MID_ACTIVATE_SPECIALMOVE)
						{
							CSpecialMoveFX *pSpecialMove = g_pGameClientShell->GetSFXMgr()->GetSpecialMoveFX(data.m_hTarget);
							if (pSpecialMove)
							{
								if (pSpecialMove->GetSFXID() == SFX_FORENSICOBJECT_ID)
								{
									SpecialMoveMgr::Instance().Activate(pSpecialMove);
									break;
								}
								else if ((pSpecialMove->GetSFXID() == SFX_ENTRYTOOLLOCK_ID)
									&& (pClientWeapon->GetWeaponRecord() == ((CEntryToolLockFX*)pSpecialMove)->m_cs.m_rEntryTool))
								{
									SpecialMoveMgr::Instance().Activate(pSpecialMove);
									break;
								}
							}
						}

						// never let forensic tool's fire...
						if (IS_ACTIVATE_FORENSIC(nActivateType))
							break;
					}

					if( pClientWeapon->IsSemiAuto() )
					{
						//for semi-auto weapons register the fire request
						m_bSemiAutoFire = true;
						// mark down when we made the request
						m_fSemiAutoFireTime = RealTimeTimer::Instance().GetTimerAccumulatedS();

						m_bSemiAutoFireDuringReload = (pClientWeapon->GetState() == W_RELOADING);

					}

					pClientWeapon->QueueFireAnimation();
				}

				if (!g_vtContextBlockingEnabled.GetFloat())
					break;

				HOBJECT hObject = g_iAttackPredictionObjectDetector.GetObject();
				if( hObject )
				{
					// Get the character FX associated with this object
					CCharacterFX* pFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( hObject );
					if( pFX && pFX->IsBlockWindowOpen() )
					{
						pClientWeapon->Block( pFX->GetServerObj() );
					}
				}
			}
		}
		break;

		case COMMAND_ID_BLOCK :
		{
			if (!IsPlayerAlive())
				break;

			CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if (!pClientWeapon)
				break;

			HOBJECT hObject = g_iAttackPredictionObjectDetector.GetObject();

			// Get the character FX associated with this object
			CCharacterFX* pFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( hObject );

			// Now do the block (even if there's no one to actually block or their block window is open)
			pClientWeapon->Block( pFX ? pFX->GetServerObj() : NULL );
		}
		break;

		// GRENADE PROTOTYPE
		case COMMAND_ID_THROW_GRENADE :
		{
			//ignore prototype system when player body is enabled
			if (CPlayerBodyMgr::Instance( ).IsEnabled())
				return false;

			//if we've already started throwing, don't throw again
			if (m_hPreGrenadeWeapon)
				return false;

			//no grenades
			HWEAPON hGrenade = g_pPlayerStats->GetCurrentGrenadeRecord();
			if (!hGrenade || !g_pPlayerStats->GetCurrentGrenadeCount())
				return false;

			CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			HWEAPON hCurWeapon = NULL;
			if ( pClientWeapon )
				hCurWeapon = pClientWeapon->GetWeaponRecord();

			//if we found a weapon to use, and we aren't already using it
			if( hGrenade && (hCurWeapon != hGrenade) )
			{
				//found a match
				m_bChangingToGrenade = true;
				g_pClientWeaponMgr->ChangeWeapon( hGrenade );

				//switching to a Grenade, so remember what we had
				m_hPreGrenadeWeapon = hCurWeapon;

				return true;
			}
			return false;

		}
		break;

		case COMMAND_ID_DROPWEAPON :
		{
			if	(g_pClientWeaponMgr->GetCurrentWeaponState() == W_IDLE)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_PLAYER_EVENT );
				cMsg.Writeuint8(kPEDropWeapon);
				g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			}
		}
		break;

		case COMMAND_ID_RELOAD :
		{
			CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if ( pClientWeapon )
			{
				// Reload the clip and let the server know...
				pClientWeapon->ReloadClip( true, -1, false, true );
			}
		}
		break;

		case COMMAND_ID_FLASHLIGHT :
		{
			// Don't allow dead to do flashlight. or while running performance test level.
			if( !IsPlayerAlive( ) || g_pGameClientShell->IsRunningPerformanceTest( ))
				break;

#ifdef PROJECT_FEAR
			//no battery power
			if (g_vtFlashlightBattery.GetFloat() > 0.0f && GetFlashlightMaxCharge() > 0.0f && GetFlashlightCharge() <= 0.0f)
				break;
#endif

#ifdef PROJECT_DARK

			// Either turn the light on, or start a timer to see what to do on a button up...
			if( m_fFlashlightTransitionTime == -1.0f )
			{
				if( m_pFlashLight )
				{
					if( !m_pFlashLight->IsOn() )
					{
						m_pFlashLight->TurnOn();

						// If we're holding a tool, put it away.
						CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
						if (pWeapon && IS_ACTIVATE_FORENSIC(pWeapon->GetActivationType()))
						{
							g_pClientWeaponMgr->LastWeapon();
							g_pClientWeaponMgr->DeselectCustomWeapon();
						}
					}
					else
					{
						m_fFlashlightButtonPressTime = 0.0f;
					}
				}
			}

#else//PROJECT_DARK

			// Toggle flashlight on/off
			if( m_pFlashLight )
			{
				if( m_pFlashLight->IsOn() )
				{
					m_pFlashLight->TurnOff();
				}
				else if( !g_pPlayerMgr->IsOperatingTurret( ))
				{
					m_pFlashLight->TurnOn();
				}
			}

#endif//PROJECT_DARK
		}
		break;

		case COMMAND_ID_TOOLS :
		{
			//
			// Bring up the appropriate tool.
			//

			CClientWeapon* pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if (pClientWeapon)
			{
				// Don't select if currently in the middle of a swap (otherwise we'll get stuck).
				//!!ARL: Maybe queue up weapon selections?
				if (pClientWeapon->IsSwapping() ||
					CPlayerBodyMgr::Instance().GetAnimationContext(CPlayerBodyMgr::kCustomContext)->IsTransitioning())
				{
					break;
				}

				// Switch away if we've got a tool out already.
				if (IS_ACTIVATE_FORENSIC(pClientWeapon->GetActivationType()))
				{
					g_pClientWeaponMgr->LastWeapon();
					g_pClientWeaponMgr->DeselectCustomWeapon();
					break;
				}
			}

			// Now make sure we're in an instinct volume.
			if (m_pForensicObject)
			{
				// Set up the forensic object detector...
				g_pPlayerMgr->GetForensicObjectDetector().SetTransform( g_pPlayerMgr->GetPlayerCamera()->GetCamera() );
				g_pPlayerMgr->GetForensicObjectDetector().AcquireObject( false );

				// Check if we are within range of evidence...
				HOBJECT hForensicObject = GetForensicObjectDetector().GetObject();
				CForensicObjectFX* pFX = (CForensicObjectFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_FORENSICOBJECT_ID, hForensicObject);

				// Figure out the correct tool.
				HRECORD hTool = (pFX && pFX->m_bOn) ?
					pFX->m_cs.m_rCollectionTool : 
					GetClientWeaponMgr()->GetDefaultCollectionTool();
				if (hTool)
				{
					// Then select it.
					g_pClientWeaponMgr->ChangeWeapon(hTool);

					// Send activate command on server.
					m_pForensicObject->OnToolSelect();
				}
			}
		}
		break;

		case COMMAND_ID_CENTERVIEW :
		{
			EulerAngles EA = Eul_FromQuat( m_pPlayerCamera->GetCameraRotation( ), EulOrdYXZr );
			float fYaw		= EA.x;
			float fRoll		= EA.z;
			m_pPlayerCamera->SetCameraRotation( LTRotation( 0.0f, fYaw, fRoll ));
		}
		break;

		case COMMAND_ID_STRAFE:
		{
			m_bStrafing = true;
		}
		break;

		case COMMAND_ID_MANUALAIM:
		{
			m_bManualAim = !m_bManualAim;
		} 
		break;

		case COMMAND_ID_RUNLOCK :
		{
			if (!m_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
			{
				m_pMoveMgr->SetRunLock(!m_pMoveMgr->RunLock());
			}
		}
		break;

		case COMMAND_ID_DUCK :
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			if (pProfile && pProfile->m_bCrouchToggle)
			{
				if( !m_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
				{
					m_pMoveMgr->SetDuckLock(!m_pMoveMgr->DuckLock());
				}
			}

		}
		break;

		case COMMAND_ID_STAND:
		{
			g_pMoveMgr->SetDuckLock( false );
		}
		break;

		case COMMAND_ID_KNEEL:
		{
			g_pMoveMgr->SetDuckLock( true );
		}
		break;

		case COMMAND_ID_JUMP : 
		{
			// See if we're follow spectating.
			if( GetSpectatorMode( ) == eSpectatorMode_Follow )
			{
				// Go to the next character.
				CCharacterFX* pCurCharFx = m_hSpectatorFollowCharacter ? g_pGameClientShell->GetSFXMgr()->GetCharacterFX( m_hSpectatorFollowCharacter ) : NULL;
				pCurCharFx = GetNextSpectatorFollow( pCurCharFx );
				SetSpectatorFollowCharacter( pCurCharFx );
			}

			m_pMoveMgr->SetDuckLock(false); 
		}
		break;

		case COMMAND_ID_FOCUS :
		{
			g_iFocusObjectDetector.AcquireObject();

			// Melee weapons always have a focus accuracy of 1.0
			CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			m_fFocusAccuracy = ( ( pWeapon && pWeapon->IsMeleeWeapon() ) ? 1.0f : 0.0f );

			break;
		}

		case COMMAND_ID_TOGGLEMELEE :
		{
//			m_eLastSonicType = m_iSonicData.HasEnoughBreath( "Blast" ) ? eSonicType_Blast : eSonicType_Disabled;
			CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if (pWeapon)
			{
				pWeapon->SwitchToComplimentaryWeapon();
			}
			break;
		}

		case COMMAND_ID_AMMOCHECK :
		{
//			m_eLastSonicType = m_iSonicData.HasEnoughBreath( "Guide" ) ? eSonicType_Guide : eSonicType_Disabled;
			CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if (pWeapon)
			{
				pWeapon->CheckAmmo();
			}
			break;
		}

		case COMMAND_ID_STUNGUN :
		{
//			m_eLastSonicType = m_iSonicData.HasEnoughBreath( "Alter" ) ? eSonicType_Alter : eSonicType_Disabled;
			CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if (pWeapon)
			{
				pWeapon->Stun();
			}
			break;
		}

		default :
			return false;
		break;
	}

	return true;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off notification
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::OnCommandOff(int command)
{
	// Make sure we're in the world and can do things.
	if (!IsPlayerInWorld() || 
		( g_pMissionMgr->GetServerGameState() != EServerGameState_Playing &&
		g_pMissionMgr->GetServerGameState() != EServerGameState_EndingRound &&
		g_pMissionMgr->GetServerGameState() != EServerGameState_PlayingSuddenDeath ))
		return false;

	// See if the vehiclemgr would like to trap this
	if( m_pMoveMgr->GetVehicleMgr()->OnCommandOff( command ) )
	{
		return true;
	}

	if( AimMgr::Instance().OnCommandOff( command ) )
	{
		return true;
	}

	switch( command )
	{
		case COMMAND_ID_ALT_FIRING :
		{
			if(!IsPlayerAlive( ))
			{
				HandleRespawn( );
			}
		}
		break;

		case COMMAND_ID_FIRING :
		{
			if (!IsPlayerAlive())
			{
				HandleRespawn();
			}
		}
		break;

		case COMMAND_ID_STRAFE:
		{
            m_bStrafing = false;
			break;
		}

		case COMMAND_ID_FLASHLIGHT:
		{
#ifdef PROJECT_DARK

			// Make sure we're not transitioning to a new beam type...
			if( m_fFlashlightTransitionTime == -1.0f )
			{
				if( m_pFlashLight->IsOn() && ( m_fFlashlightButtonPressTime != -1.0f ) )
				{
					m_fFlashlightTransitionTime = 0.0f;
					m_pFlashLight->PlayFlicker( g_vtFlashlightBeamTransitionTime.GetFloat(), false );
					m_pFlashLight->PlaySpecialSound( 0 );
				}

				m_fFlashlightButtonPressTime = -1.0f;
			}

#endif//PROJECT_DARK

			break;
		}

		default: 
		{
			return false;
			break;
		}
	}

	return true;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnCommandOn()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //
void CPlayerMgr::OnClearAllCommands()
{
	// If you find yourself turning off commands in this
	// function, maybe you should instead look into
	// getting OnCommandOff called from within CBindMgr's
	// ClearAllCommands function.

	m_pMoveMgr->ClearAllRequests();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgPlayerStateChange()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgPlayerStateChange (ILTMessage_Read *pMsg)
{
	m_ePlayerState = (PlayerState) pMsg->Readuint8();
	

	switch (m_ePlayerState)
	{
		case ePlayerState_Dying_Stage1:
		{
			const char *pszDeathFX = NULL;
//			DebugCPrint(0,"CPlayerMgr::HandleMsgPlayerStateChange () : ePlayerState_Dying");
			bool bDamageTypeSent = pMsg->Readbool();
			if (bDamageTypeSent)
			{
				uint32 nDeathDamageFlag = pMsg->ReadBits(kNumDamageTypes);

				DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
				

				while( pDamageFX )
				{
					if( pDamageFX->m_nDamageFlag & nDeathDamageFlag )
					{
						pszDeathFX = pDamageFX->m_sz1stPersonDeathFXName;
					}
					pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
				}
			}
			bool bTeamKillsSent = pMsg->Readbool();
			uint8 nTK = 0;
			if (bTeamKillsSent)
			{
				nTK = pMsg->Readuint8();
			}


			// Do this first since we may be unattaching the camera from the head when clearing out a 
			// damage fx.
			LadderMgr::Instance().ReleaseLadder();
			SpecialMoveMgr::Instance().Release();
			m_pMoveMgr->SetDuckLock(false); 
			if( m_pTurretFX )
				m_pTurretFX->Deactivate( );
			
			// Clear out any damagefx...
			g_pDamageFXMgr->Clear();
					
			m_pPlayerCamera->AttachToTarget( true );

			if( CPlayerBodyMgr::Instance( ).IsEnabled() )
			{
				CPlayerBodyMgr::Instance( ).HidePlayerBody( true );
			}

			// Show respawn hud item if we can respawn.
			if( GameModeMgr::Instance().m_grbAllowRespawnFromDeath )
			{
				float fRespawnTime = ( float )GameModeMgr::Instance( ).m_grnRespawnWaitTime;
				if (GameModeMgr::Instance( ).m_grbUseRespawnWaves)
				{
					//time of the next wave
					double fWaveTime = ceil(g_pLTClient->GetGameTime() / fRespawnTime) * fRespawnTime;
					fRespawnTime = (float)(fWaveTime - g_pLTClient->GetGameTime());
					m_bRespawnWhenTimedOut = true;
				}

				//calculate any appropriate respawn penalties
				m_fRespawnPenalty = 0.0f;
				if (nTK > 0 && GameModeMgr::Instance( ).m_grnTeamKillRespawnPenalty > 0)
				{
					if (GameModeMgr::Instance( ).m_grbAccumulateRespawnPenalty)
					{
						m_fRespawnPenalty = (float)(GameModeMgr::Instance( ).m_grnTeamKillRespawnPenalty * nTK);
					}
					else
					{
						m_fRespawnPenalty = (float)(GameModeMgr::Instance( ).m_grnTeamKillRespawnPenalty);
					}
					if (GameModeMgr::Instance( ).m_grbUseRespawnWaves)
					{
						//penalize player by missing at least one wave respawn
						fRespawnTime += ( float )GameModeMgr::Instance( ).m_grnRespawnWaitTime;

						//extend the respawn time by wave periods until it is at least as long as the 
						// the penalty
						while (fRespawnTime < m_fRespawnPenalty)
						{
							fRespawnTime += ( float )GameModeMgr::Instance( ).m_grnRespawnWaitTime;
						}
					}
					else
					{
						fRespawnTime += m_fRespawnPenalty;
					}
					
				}


				if( fRespawnTime > 0.0f )
				{
					m_EarliestRespawnTime.Start( fRespawnTime );
					g_pHUDMgr->QueueUpdate(kHUDRespawn);
				}
			}

			if( g_pRadio && g_pRadio->IsVisible())
			{
				g_pRadio->Show(false);
			}

			g_pHUDMgr->UpdateRenderLevel();

			if( g_pMoveMgr->GetVehicleMgr()->GetPhysicsModel() == PPM_LURE )
			{
				// Get the playerlurefx.
				PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( g_pMoveMgr->GetVehicleMgr()->GetPlayerLureId() );
				if( pPlayerLureFX )
				{
					char const* pszDeathFX = pPlayerLureFX->GetDeathFX( );
					if( !pszDeathFX && pszDeathFX[0] != '\0' )
					{
						LTRigidTransform tTransform;
						tTransform.Init();
						g_pLTClient->GetObjectPos( g_pMoveMgr->GetObject(), &tTransform.m_vPos );

						CLIENTFX_CREATESTRUCT fxInit( pszDeathFX, 0, tTransform );
						g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
					}
				}
			}

			ClearPlayerModes();

			m_pClientWeaponMgr->OnPlayerDead();

			if (!IsMultiplayerGameClient( ))
			{
				// now display the message
				g_pGameMsgs->AddMessage(LoadString("IDS_YOUWEREKILLED"));
			}
			
			ClientDB& ClientDatabase = ClientDB::Instance(); 
			g_pGameClientShell->GetMixer( )->ProcessMixerByName( ClientDatabase.GetRecordLink( ClientDatabase.GetClientSharedRecord( ), CDB_ClientShared_DeathMixer ));
			m_hDeathSound = g_pClientSoundMgr->PlayDBSoundLocal( ClientDatabase.GetRecordLink( ClientDatabase.GetClientSharedRecord( ), CDB_ClientShared_DeathSound ), 
				SOUNDPRIORITY_INVALID, PLAYSOUND_GETHANDLE );
		

			

			// Start a timer that sets the minimum amount of time we must remain dead.
			if (!pszDeathFX || !pszDeathFX[0])
			{
				if (IsMultiplayerGameClient())
				{
					pszDeathFX = ClientDB::Instance( ).GetString( ClientDB::Instance( ).GetClientSharedRecord( ), CDB_ClientShared_MPDeathClientFX );
				}
				else
				{
					pszDeathFX = ClientDB::Instance( ).GetString( ClientDB::Instance( ).GetClientSharedRecord( ), CDB_ClientShared_DeathClientFX );
				}

			}

			// Create the deathFX to display an overlay or other FX goodness...
			if( pszDeathFX && pszDeathFX[0] )
			{
				CLIENTFX_CREATESTRUCT fxInit( pszDeathFX, FXFLAG_LOOP );
				g_pGameClientShell->GetRealTimeClientFXMgr().CreateClientFX( &m_fxDeathLoop, fxInit, true );
				if( m_fxDeathLoop.GetInstance( ))
					m_fxDeathLoop.GetInstance( )->Show( );
			}


			g_pPlayerStats->ResetInventory();
		}
		break;

		case ePlayerState_Dying_Stage2:
		{
//			DebugCPrint(0,"CPlayerMgr::HandleMsgPlayerStateChange () : ePlayerState_Dying_Stage2");
		}
		break;

		case ePlayerState_Alive:
		{
//			DebugCPrint(0,"CPlayerMgr::HandleMsgPlayerStateChange () : ePlayerState_Alive");
			uint32 dwFlags = FLAG_SOLID | FLAG_GRAVITY;
			uint32 dwFlags2 = FLAG2_PLAYERCOLLIDE | FLAG2_PLAYERSTAIRSTEP | FLAG2_SPECIALNONSOLID;
			uint32 dwClientFlags = CF_DONTSETDIMS;

			// Update the respawn bar.
			m_EarliestRespawnTime.Stop();
			g_pHUDMgr->QueueUpdate(kHUDRespawn);

			HOBJECT hObject = g_pMoveMgr->GetObject();
			g_pCommonLT->SetObjectFlags( hObject, OFT_Flags, dwFlags, dwFlags );
			g_pCommonLT->SetObjectFlags( hObject, OFT_Flags2, dwFlags2, dwFlags2 );
			g_pCommonLT->SetObjectFlags( hObject, OFT_Client, dwClientFlags, dwClientFlags );

			m_hKiller = NULL;
			if (m_hDeathSound)
			{
				g_pLTClient->SoundMgr()->KillSound(m_hDeathSound);
				m_hDeathSound = NULL;
			}

			g_pDamageFXMgr->Clear();					// Remove all the damage sfx
			ClearDamageSectors();

			ClearPlayerModes();

			// Wipe the death mixer..
			ClientDB& ClientDatabase = ClientDB::Instance(); 
			g_pGameClientShell->GetMixer( )->ProcessMixerByName( ClientDatabase.GetRecordLink( ClientDatabase.GetClientSharedRecord( ), CDB_ClientShared_DeathMixer ), 1, true);


			if( CPlayerBodyMgr::Instance( ).IsEnabled() )
			{
				CPlayerBodyMgr::Instance( ).HidePlayerBody( false );
			}

			if( !g_pInterfaceMgr->OverrideInitialFade() )
			{
				g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
			}

			SetSpectatorFollowCharacter( NULL );
			m_pMoveMgr->SetSpectatorMode(eSpectatorMode_None);
			g_pPlayerMgr->GetPlayerCamera()->ClearCameraClamping();
			m_pPlayerCamera->SetCameraMode( CPlayerCamera::kCM_FirstPerson );
			m_pPlayerCamera->SetTargetObject( m_pMoveMgr->GetObject( ));
			m_pPlayerCamera->AttachToTarget( true );

			g_pHUDMgr->UpdateRenderLevel();

			m_pClientWeaponMgr->OnPlayerAlive();

			if( IsMultiplayerGameClient() )
			{
				AllowPlayerMovement( true );
			}
	
			// Finished the respawn process...
			m_bRespawnRequested = false;
			m_bRespawnWhenTimedOut = false;
			m_fRespawnPenalty = 0.0f;

			// Kill the death ClientFX...
			if( m_fxDeathLoop.IsValid( ))
				g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_fxDeathLoop );

			if( m_fxHeadShot.IsValid( ))
				g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_fxHeadShot );

			g_pGameClientShell->GetMixer()->RestoreSavedMixers();

			if	(!IsMultiplayerGameClient())
			{
				//force characters to update attachment visibility here because before this point not all
				// of the attachments may have been created yet. This is intended to address save/load issues
				// with attachment visibility
				CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
				while( iter != CCharacterFX::GetCharFXList( ).end( )) 
				{
					CCharacterFX* pChar = (*iter);
					if (pChar)
					{
						UpdateAttachmentVisibility(pChar->GetServerObj());

					}
					iter++;

				}
			}

		}
		break;

		case ePlayerState_Dead:
		{
			if( !IsMultiplayerGameClient())
			{
				bool bHandleMissionFailed = true;

				if (g_pInterfaceMgr->FadingScreen() && g_pInterfaceMgr->FadingScreenIn())
				{
					bHandleMissionFailed = g_pInterfaceMgr->ScreenFadeDone();
				}

				if (bHandleMissionFailed)
				{
					g_pMissionMgr->HandleMissionFailed();
				}
			}

			// Make sure the camera isn't zoomed.
			if( m_pPlayerCamera->IsZoomed())
				m_pPlayerCamera->ExitZoom();

//			DebugCPrint(0,"CPlayerMgr::HandleMsgPlayerStateChange () : ePlayerState_Dead");

			// Remove all the damage sfx
			g_pDamageFXMgr->Clear();

			ClearDamageSectors();
		}
		break;

		case ePlayerState_Spectator:
		{
			// Kill the death ClientFX...
			if( m_fxDeathLoop.IsValid( ))
				g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_fxDeathLoop );

			if( m_fxHeadShot.IsValid( ))
				g_pGameClientShell->GetRealTimeClientFXMgr().ShutdownClientFX( &m_fxHeadShot );

//			DebugCPrint(0,"CPlayerMgr::HandleMsgPlayerStateChange () : ePlayerState_Spectator");
			SpectatorMode eSpectatorMode = ( SpectatorMode )pMsg->Readuint8( );
			SetSpectatorMode( eSpectatorMode );
		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgSpectatorMode()
//
//	PURPOSE:	Handles spectator mode change.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgSpectatorMode(ILTMessage_Read *pMsg)
{
	SpectatorMode eSpectatorMode = ( SpectatorMode )pMsg->Readuint8( );
	SetSpectatorMode( eSpectatorMode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgPlayerLeash()
//
//	PURPOSE:	Handles spectator mode change.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleMsgPlayerLeash( ILTMessage_Read* pMsg)
{
	float fOuterRadius = pMsg->Readfloat();
	if (fOuterRadius > 0.0f)
	{
		float fInnerRadius = pMsg->Readfloat();
		LTVector vPos = pMsg->ReadLTVector();
		g_pMoveMgr->SetPlayerLeash( fInnerRadius, fOuterRadius, &vPos );
	}
	else
	{
		g_pMoveMgr->SetPlayerLeash( 0.0f, 0.0f );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleRespawn
//
//	PURPOSE:	Handle player respawn
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::HandleRespawn()
{
	if ( !CanRespawn())
	{
		// we're waiting for a timeout record the fact that we have requested a respawn
		if( GameModeMgr::Instance( ).m_grbAllowRespawnFromDeath 
			&& m_EarliestRespawnTime.IsStarted() && !m_EarliestRespawnTime.IsTimedOut() )
		{
			if (!m_bRespawnWhenTimedOut)
			{
				g_pHUDMgr->QueueUpdate(kHUDRespawn);
			}
			m_bRespawnWhenTimedOut = true;
			
		}
			

		return;
	}


	// if we're in multiplayer send the respawn command...
	if (IsMultiplayerGameClient())
	{
		// When remotely connected there is a delay from this point to changing to the alive
		// state so set this flag so the player knows it's respawning.  This gets cleared
		// once the player has switched to the alive state and the respawn process is complete...
		m_bRespawnRequested = true;

		// send a message to the server telling it that it's ok to respawn us now...
		SendEmptyServerMsg(MID_PLAYER_RESPAWN);

		m_EarliestRespawnTime.Stop();

#ifndef _FINAL
		if (g_vtEnableSimulationLog.GetFloat())
		{
			WritePlayerRespawnToSimulationLog();
		}
#endif

		return;
	}
	else
	{
		g_pMissionMgr->HandleMissionFailed();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::RequestSpectatorMode
//
//	PURPOSE:	Request a spectator mode.
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::RequestSpectatorMode( SpectatorMode eSpectatorMode )
{
	// Don't allow switching to spectator mode if we're in the process of dying.
	// This will confuse the client what player should be spectated because the old
	// player will be replaced with the new player.
	if( m_ePlayerState == ePlayerState_Dying_Stage1 || m_ePlayerState == ePlayerState_Dying_Stage2 )
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_SPECTATORMODE);
	cMsg.Writeuint8(( uint8 )eSpectatorMode);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::PreRender()
//
//	PURPOSE:	Sets up the client for rendering
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::PreRender()
{
	if( !m_pPlayerCamera->CanRender() )
	{
		return false;
	}

	// Make sure the rendered player object is right where it should be.
	UpdateServerPlayerModel();

	// Make sure we process attachments before updating the weapon model
	// and special fx...(some fx are based on attachment positions/rotations)

	// Make sure the weapon is updated before we render the camera...
	UpdateWeaponModel();

	// The carry object is attached to the player model so it should update position right before rendering...
	UpdateCarryObject();

	// Make sure the move-mgr models are updated before we render...
	m_pMoveMgr->UpdateModels();

	// Update the flash light...
	LTVector vVelocity = g_pMoveMgr->GetVelocity();
	vVelocity.y = 0.0f;

	float fVelocity = vVelocity.Mag();
	float fMaxVelocity = g_pMoveMgr->GetMaxVelMag();
	float fSpeedRange = ( fVelocity / fMaxVelocity );

	m_pFlashLight->SetWaverSpeedScale( ( fMaxVelocity == 0.0f ) ? 0.0f : ( 1.0f + ( fSpeedRange * g_vtFlashlightWaverSpeedScale.GetFloat() ) ) );
	m_pFlashLight->Update( ObjectContextTimer( m_pMoveMgr->GetServerObject() ).GetTimerElapsedS() );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CalculateAxisOffsets()
//
//	PURPOSE:	Calculate the difference in X and Y axis movement (usually based off of mouse movement)...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::CalculateAxisOffsets()
{
	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if( !pSettings )
		return;

	// Get axis offsets...
	LTVector2 vOffsets;
	vOffsets.x = CBindMgr::GetSingleton().GetExtremalCommandValue(COMMAND_ID_YAW);
	vOffsets.y = CBindMgr::GetSingleton().GetExtremalCommandValue(COMMAND_ID_PITCH);

	if( m_bRestoreOrientation )
	{
		vOffsets.Init();
		m_bRestoreOrientation = false;
	}

	if( IsMouseStrafing() )
	{
		// Clear yaw and pitch offsets if we're using mouse strafing...
		vOffsets.Init();
	}

	// Zoom modifies how the offsets are determined...
	float fVal = m_pPlayerCamera->GetZoomMag();
	float fSloMoDampen = IsInSlowMo( ) ? g_vtMouseSloMoDampen.GetFloat( ) : 1.0f;
	float fDeathDampen = IsPlayerDead( ) ? g_vtMouseDeathDampen.GetFloat( ) : 1.0f;
	float fAimDampen = AimMgr::Instance().IsAiming() ? g_vtMouseAimDampen.GetFloat( ) : 1.0f;
	float fMeleeDampen = 1.0f;

	CPlayerBodyMgr& playerBodyMgr = CPlayerBodyMgr::Instance( );

	if( playerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_JumpIdleKick ) ||
		playerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_JumpRunKick ) ||
		playerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_CrouchIdleKick ) ||
		playerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_SlideKick ))
	{
		if( IsInSlowMo( ))
		{
			uint32 dwNum, dwDenom;
			SimulationTimer::Instance( ).GetTimerTimeScale( dwNum, dwDenom );
			float fTimeScale = ((float)dwNum / (float)dwDenom);

			uint32 dwPlayerNum, dwPlayerDenom;
			ObjectContextTimer PlayerTimer(g_pMoveMgr->GetServerObject());
			PlayerTimer.GetTimerTimeScale( dwPlayerNum, dwPlayerDenom );
			float fPlayerTimeScale = ((float)dwPlayerNum / (float)dwPlayerDenom) * fTimeScale;
			fMeleeDampen *= fPlayerTimeScale * g_vtMouseMeleeSloMoFactor.GetFloat( );
		}

		fMeleeDampen *= g_vtMouseMeleeDampen.GetFloat( );
	}


	// BEGIN: GAMEPAD CONTROLLER CODE...
	float fMouseYawDelta = ( vOffsets.x / fVal );
	float fMousePitchDelta = ( vOffsets.y / fVal );

	float fYawDelta = 0.0f;
	float fPitchDelta = 0.0f;

	static float fQuickTurnDirection = 0.0f; // need this here for scope below

	fYawDelta = ( CBindMgr::GetSingleton().GetExtremalCommandValue( COMMAND_ID_YAW_ACCEL ) / fVal );
	fPitchDelta = ( CBindMgr::GetSingleton().GetExtremalCommandValue( COMMAND_ID_PITCH_ACCEL ) / fVal );

	bool bRightThumbButtonPressed = CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_ACCEL_TURN );
	bool bLeftThumbButtonPressed = CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_RUN );

	// check for invert mouse combo (both thumbsticks pressed)
	bool bInvertMouse = (bRightThumbButtonPressed && bLeftThumbButtonPressed);
	static bool bLastInvertMouse = 0;

	if (bInvertMouse)
	{
		if (!bLastInvertMouse)
		{
			CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
			pSettings->SetMouseInvertY(!pSettings->MouseInvertY());
		}

		bRightThumbButtonPressed = false;
		bLeftThumbButtonPressed = false;
	}

	bLastInvertMouse = bInvertMouse;	// remember if we were holding last frame so this doesn't get called multiple times while we're holding down the buttons.


	float fAimPowFn = g_vtGPadAimPowFn.GetFloat();
	float fCenterDist = sqrt(powf(fYawDelta,fAimPowFn) + powf(fPitchDelta,fAimPowFn)); // store off center stick distance (already squared components above)

	static float fQuickTurnTarget = 0.0f;
	static bool bRightStickCentered = 0;

	bRightStickCentered = (fCenterDist > g_vtGPadQuickTurnThreshold.GetFloat()) ? false : true;

	if (!fQuickTurnDirection)
	{
		// square the offsets to provide more precision at center for forensics work
		fYawDelta = (fYawDelta < 0.0f) ? -(powf(fYawDelta,fAimPowFn)) : (powf(fYawDelta,fAimPowFn));
		fPitchDelta = (fPitchDelta < 0.0f) ? -(powf(fPitchDelta,fAimPowFn)) : (powf(fPitchDelta,fAimPowFn));

		// edge yaw acceleration: if we're pinned, set fEdgeAccelTime, else reset it to zero
		static double fEdgeAccelTime = 0.0f; // the time we first pin the controller extent

		if (fCenterDist > g_vtGPadAimEdgeThreshold.GetFloat())
		{
			if (!fEdgeAccelTime)
				fEdgeAccelTime = g_pLTClient->GetGameTime();
		}
		else
		{
			fEdgeAccelTime = 0.0f;
//			g_vtMotionBlur.WriteFloat(0.0f);
//			if/when we get a generic motion blur interface, re-enable this stuff so we get motion blur during quickturn
		}

		// if we're pinned to the edge, do some more stuff...

		if (fEdgeAccelTime || bRightThumbButtonPressed)
		{
			float fDelta = (float)(g_pLTClient->GetGameTime() - fEdgeAccelTime);

			// allow a delay between pinning and acceleration start (for "tapping" to edge)
			if (fDelta > g_vtGPadAimEdgeDelayTime.GetFloat() || bRightThumbButtonPressed) 
			{
				if (bRightThumbButtonPressed)	// if we're mashing the thumb button
				{
					fDelta = 2.0f;			// use full acceleration immediately
				}
				else						// otherwise check delay time
				{
					fDelta -= g_vtGPadAimEdgeDelayTime.GetFloat();

					float fMaxAccelTime = g_vtGPadAimEdgeAccelTime.GetFloat();

					if (fDelta > fMaxAccelTime)
						fDelta = fMaxAccelTime;

					fDelta /= fMaxAccelTime; // normalize to 1.0
					fDelta += 1.0f; // 1.0 -> 2.0 (so when we multiply through by yaw, no delta still gets our original rate)
				}

//				g_vtMotionBlur.WriteFloat(fDelta*g_vtGPadAimBlurMultiplier.GetFloat());

				fDelta *= g_vtGPadAimEdgeMultiplier.GetFloat(); // multiply by the edge accel multiplier
				fYawDelta *= fDelta; // multiply old yaw delta with our accelerated version
			}
		}

		fPitchDelta *= g_vtGPadAimAspectRatio.GetFloat(); // dampen pitch for GPad controller

		// normalize final values over framerate and multiply by console sensitivity
		fPitchDelta *= RealTimeTimer::Instance( ).GetTimerElapsedS( ) * g_vtGPadAimSensitivity.GetFloat();
		fYawDelta *= RealTimeTimer::Instance( ).GetTimerElapsedS( ) * g_vtGPadAimSensitivity.GetFloat();
	}

	fPitchDelta += fMousePitchDelta;
	fYawDelta += fMouseYawDelta;
	// END: GAMEPAD CONTROLLER CODE...


	// Check varying degrees of strafe and look...
	ObjectContextTimer objectContextTimer( g_pMoveMgr->GetServerObject( ));
	if( !(m_dwPlayerFlags & BC_CFLG_STRAFE) )
	{
		if( m_dwPlayerFlags & BC_CFLG_LEFT )
		{
			fYawDelta = -1.0f * objectContextTimer.GetTimerElapsedS() * ((m_dwPlayerFlags & BC_CFLG_RUN) ? g_vtFastTurnRate.GetFloat() : g_vtNormalTurnRate.GetFloat());
		}

		if( m_dwPlayerFlags & BC_CFLG_RIGHT )
		{
			fYawDelta = objectContextTimer.GetTimerElapsedS() * ((m_dwPlayerFlags & BC_CFLG_RUN) ? g_vtFastTurnRate.GetFloat() : g_vtNormalTurnRate.GetFloat());
		}
	}

	if( pSettings->MouseInvertY() )
		fPitchDelta *= -1;

	if( m_dwPlayerFlags & BC_CFLG_LOOKUP )
	{
		fPitchDelta -= objectContextTimer.GetTimerElapsedS() * g_vtLookUpRate.GetFloat();
	}

	if( m_dwPlayerFlags & BC_CFLG_LOOKDOWN )
	{
		fPitchDelta += objectContextTimer.GetTimerElapsedS() * g_vtLookUpRate.GetFloat();
	}

	fYawDelta *= fSloMoDampen * fMeleeDampen * fDeathDampen * fAimDampen;
	fPitchDelta *= fSloMoDampen * fMeleeDampen * fDeathDampen * fAimDampen;

	// Apply mouse smoothing
	LTVector2 vSmoothDelta( fPitchDelta, fYawDelta );
	
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile)
	{
		// Convert the user profile & wacky backwards compatible MouseMin/MaxInputRate variables
		// into a 0..1 smoothing value.
		float fMin = g_vtMouseMinInputRate.GetFloat();
		float fMax = g_vtMouseMaxInputRate.GetFloat();
		float fValue = (float)pProfile->m_nInputRate;
		float fSmoothing = (fValue - fMin) / (fMax - fMin);
		
		// Reduce smoothing in low framerate situations
		const float k_fMinTime = 0.02f; // Apply full smoothing until 10ms have passed
		const float k_fMaxTime = 0.1f;	// Reduce to no smoothing at 100ms.
		float fTime = g_pLTClient->GetFrameTime();
		float fTimeSmoothing = 1.0f - LTCLAMP((fTime - k_fMinTime) / (k_fMaxTime - k_fMinTime), 0.0f, 1.0f);

		// Take the average between the current delta and the previous axis value
		vSmoothDelta = vSmoothDelta.Lerp((vSmoothDelta + m_v2AxisOffsets) * 0.5f, fSmoothing * fTimeSmoothing);
	}
	
	m_v2AxisOffsets = vSmoothDelta;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayerFlags
//
//	PURPOSE:	Update our copy of the movement flags
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlayerFlags()
{
	// Update flags...

	m_dwPlayerFlags = m_pMoveMgr->GetControlFlags();

	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pClientWeapon && pClientWeapon->IsSemiAuto())
	{
		if (m_bSemiAutoFire)
		{
			m_dwPlayerFlags |= BC_CFLG_FIRING;
		}
		else
		{
			m_dwPlayerFlags &= ~BC_CFLG_FIRING;
		}

	}

    if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_PITCH_NEG))
	{
		m_dwPlayerFlags |= BC_CFLG_LOOKUP;
	}

    if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_PITCH_POS))
	{
		m_dwPlayerFlags |= BC_CFLG_LOOKDOWN;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayerInfo()
//
//	PURPOSE:	Tell the player about the new camera stuff
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlayerInfo(bool bPlaying, bool bForceSend)
{
	// track if we actually send a player update message
	bool bSentPlayerUpdate = false;

	// track the change flags we actually sent
	uint16 nSentChangeFlags = 0;


	if (m_bAllowPlayerMovement != m_bLastAllowPlayerMovement)
	{
		g_pGameClientShell->SetInputState( m_bAllowPlayerMovement );
	}

	if( (m_pPlayerCamera->GetCameraMode() == CPlayerCamera::kCM_Chase) != m_bLastSent3rdPerson )
	{
		m_nPlayerInfoChangeFlags |= CLIENTUPDATE_3RDPERSON;
		m_bLastSent3rdPerson = (m_pPlayerCamera->GetCameraMode() == CPlayerCamera::kCM_Chase);

		if( m_pPlayerCamera->GetCameraMode() == CPlayerCamera::kCM_Chase )
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
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_UPDATE);
		cMsg.Writeuint32(g_pGameClientShell->GetServerRealTimeMS());
		cMsg.Writeuint8(CLIENTUPDATE_ALLOWINPUT);
		cMsg.Writebool(m_bAllowPlayerMovement);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		m_nPlayerInfoChangeFlags &= ~CLIENTUPDATE_ALLOWINPUT;
	}

	double fCurTime   = RealTimeTimer::Instance( ).GetTimerAccumulatedS();
	float fSendRate  = 1.0f / g_CV_CSendRate.GetFloat(DEFAULT_CSENDRATE);
	float fSendDelta = (float)(fCurTime - m_fPlayerInfoLastSendTime);

	// If SP, then always send updates.
	if( !IsMultiplayerGameClient( ))
		bForceSend = true;

	bool bSendUnique = (fCurTime - m_fPlayerInfoLastUniqueSendTime) > (1.0f / g_CV_CSendRate_Min.GetFloat(DEFAULT_CSENDRATE_MIN));

	if ( m_pMoveMgr->IsInWorld() && ( bForceSend || ( !IsSpectating( ) && fSendDelta > fSendRate )))
	{
		CAutoMessage cMsg;

		cMsg.Writeuint8(MID_PLAYER_UPDATE);
		cMsg.Writeuint32(g_pGameClientShell->GetServerRealTimeMS());

		CLTMsgRef_Read	cCameraMsg_Read;
		if ( g_vtPlayerRotate.GetFloat(1.0) > 0.0)
		{
			CAutoMessage	cCameraMsg;

			bool bWroteCamInfo = m_pPlayerCamera->WriteCameraInfo( cCameraMsg, CLIENTUPDATE_CAMERAINFO );
			cCameraMsg_Read = cCameraMsg.Read( );
			
			if( bWroteCamInfo && (bSendUnique || !AreMessagesEqual( cCameraMsg_Read, m_cLastPlayerInfoCameraMsg )))
			{
				m_nPlayerInfoChangeFlags |= CLIENTUPDATE_CAMERAINFO;
			}
		}

		CLTMsgRef_Read cAnimMsg_Read;
		{
			// Try and write the animation message...
			CAutoMessage cAnimMsg;
			bool bWroteAnimInfo = CPlayerBodyMgr::Instance( ).WriteAnimationInfo( cAnimMsg );
			cAnimMsg_Read = cAnimMsg.Read();
			
			// If the write was valid and the message is different than the last (animations changed) send it...
			if( bWroteAnimInfo && !AreMessagesEqual( cAnimMsg_Read, m_cLastPlayerInfoAnimMsg ))
				m_nPlayerInfoChangeFlags |= CLIENTUPDATE_ANIMATION;
		}

		ASSERT((m_nPlayerInfoChangeFlags & ~0xFF) == 0);
        cMsg.Writeuint8((uint8)m_nPlayerInfoChangeFlags);
		
        // Write the PlayerCamera info...
		if( m_nPlayerInfoChangeFlags & CLIENTUPDATE_CAMERAINFO )
		{
			cMsg.WriteMessageRaw( cCameraMsg_Read );
			m_cLastPlayerInfoCameraMsg = cCameraMsg_Read;
		}

		// Write the PlayerBody's animation info..
		if( m_nPlayerInfoChangeFlags & CLIENTUPDATE_ANIMATION )
		{
			cMsg.WriteMessageRaw( cAnimMsg_Read );
			m_cLastPlayerInfoAnimMsg = cAnimMsg_Read;
		}

		cMsg.Writebool(m_pMoveMgr->IsDucking());


		// Write position info...
		// NOTE: The position info needs to happen last since it is handled separately on the server,
		//		 after the rest of the client update.
		{
			CAutoMessage cMoveMsg;
			m_pMoveMgr->WritePositionInfo(cMoveMsg);
			CLTMsgRef_Read cMoveMsg_Read = cMoveMsg.Read();
			if ( bSendUnique || !AreMessagesEqual(cMoveMsg_Read, m_cLastPlayerInfoMoveMsg))
			{
				cMsg.Writebool(1);
				cMsg.WriteMessageRaw(cMoveMsg_Read);

				m_cLastPlayerInfoMoveMsg = cMoveMsg_Read;
			}
			else
				cMsg.Writebool(0);
		}


		CLTMsgRef_Read cNewMsg = cMsg.Read();
		// Compare the messages...
		bool bMsgChange = bForceSend || bSendUnique || !AreMessagesEqual(cNewMsg, m_cLastPlayerInfoMsg);
		// Don't send the message if it's the same as last time or if we're spectating.  
		if ( bMsgChange )
		{
			m_cLastPlayerInfoMsg = cNewMsg;

			g_pLTClient->SendToServer(cNewMsg, ( bForceSend ) ? MESSAGE_GUARANTEED : 0 );

			m_fPlayerInfoLastUniqueSendTime = fCurTime;

			bSentPlayerUpdate = true;
			nSentChangeFlags  = m_nPlayerInfoChangeFlags;
		}

		m_fPlayerInfoLastSendTime = fCurTime;
		m_nPlayerInfoChangeFlags  = 0;
	}

#ifndef _FINAL

	// write a simulation log entry if necessary
	if (g_vtEnableSimulationLog.GetFloat())
	{
		if (bSentPlayerUpdate)
		{
			WritePlayerUpdateToSimulationLog(nSentChangeFlags);
		}
		else
		{
			WriteNullUpdateToSimulationLog();
		}
	}

#endif

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::IsPlayerInWorld()
//
//	PURPOSE:	See if the player is in the world
//
// --------------------------------------------------------------------------- //

bool CPlayerMgr::IsPlayerInWorld()
{
	return ( m_ePlayerState != ePlayerState_None );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::InitLocalPlayer()
//
//	PURPOSE:	Called when the player object is first added to the client.
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::InitLocalPlayer( CCharacterFX& characterFx )
{
	CCharacterFX* pOldCharFx = GetMoveMgr( )->GetCharacterFX( );
	bool bChangingServerObj = ( pOldCharFx && pOldCharFx != &characterFx );

	// Set up our move mgr...
	// The local CharacterFX should belong to PlayerMgr...
	GetMoveMgr()->SetCharacterFX( &characterFx );

	// Hook up timers that use the player timer.
	EngineTimer engineTimer( characterFx.GetServerObj( ));
	LTASSERT( engineTimer.IsValid( ), "Invalid timer for player." );
	m_StartDuckTimer.Stop( );
	m_StartDuckTimer.SetEngineTimer( engineTimer );
	m_ContainerStartTimer.Stop();
	m_ContainerStartTimer.SetEngineTimer( engineTimer );
	m_LastFireTimer.Stop( );
	m_LastFireTimer.SetEngineTimer( engineTimer );
	m_RecoilDecayTimer.Stop( );
	m_RecoilDecayTimer.SetEngineTimer( engineTimer );
	m_MeleeDamageEffectTimer.Stop( );
	m_MeleeDamageEffectTimer.SetEngineTimer( engineTimer );
	m_HeartBeat.InitTimer();
	m_Breath.InitTimer();

	// temp sound stuff (until we get it into ClientFX)
	m_hRingSound = NULL;
	m_nAudioEffectState = 0;
	m_AudioTimer.SetEngineTimer( engineTimer );

	m_pPlayerCamera->InitLocalPlayer( );

	engineTimer.ApplyTimerToObject( CPlayerBodyMgr::Instance( ).GetObject( ));

	CPlayerBodyMgr::Instance( ).ResetModel( characterFx.GetModel() );

	// Select the best weapon available if we don't already have one and haven't requested a weapon switch...
	if( !GetClientWeaponMgr( )->GetCurrentClientWeapon( ) &&
		(GetClientWeaponMgr( )->GetRequestedWeaponRecord( ) == NULL) )
	{
		GetClientWeaponMgr( )->AutoSelectWeapon( );
	}

	// Update the player-view weapon model so that it uses the correct
	// textures based on the model style...

	// this is called to update the skins on the playerview
	// model (to reflect the costume of the character),
	// figure out how to do this differently
	CClientWeapon *pClientWeapon = GetClientWeaponMgr()->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		pClientWeapon->ResetWeaponFilenames( );
	}

	if( bChangingServerObj )
	{
		// If I'm not the killer then track the killer.  Make sure tracking is allowed.
		bool bUseFixed = true;
		if (m_hKiller && m_hKiller != g_pMoveMgr->GetServerObject() && m_hKiller != pOldCharFx->GetServerObj())
		{
			SetSpectatorTrackTarget( m_hKiller );
			if( AllowSpectatorMode( eSpectatorMode_Tracking ))
			{
				RequestSpectatorMode( eSpectatorMode_Tracking );
				bUseFixed = false;
			}
		}

		// See if we should just use fixed mode.
		if( bUseFixed )
		{
			SetSpectatorTrackTarget( NULL );
			RequestSpectatorMode( eSpectatorMode_Fixed );
		}
	}

	// Initalize the sonic data
	m_iSonicData.SetBook( "Player" );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	UpdateModelGlow
//
//	PURPOSE:	Update the current model glow color
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateModelGlow()
{
    float fColor      = 0.0f;
	float fMin		  = g_vtModelGlowMin.GetFloat();
	float fMax		  = g_vtModelGlowMax.GetFloat();
    float fColorRange = fMax - fMin;
	float fTimeRange  = g_vtModelGlowTime.GetFloat();

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
			m_bModelGlowCycleUp = false;
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
            m_bModelGlowCycleUp = true;
			return;
		}
	}

	m_fModelGlowCycleTime += SimulationTimer::Instance().GetTimerElapsedS();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ClearCurContainerCode
//
//	PURPOSE:	Clear our current container info.
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::ClearCurContainerCode()
{
	//make sure we clear out any tinting that the container may have been doing
	g_pGameClientShell->GetLightScaleMgr()->ClearLightScale(CLightScaleMgr::eLightScaleEnvironment);

	m_eCurContainerCode = CC_NO_CONTAINER;
	m_hSoundFilterIdCurrentContainer = NULL;
	m_hCurContainerObject = NULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleWeaponDisable
//
//	PURPOSE:	Handle the weapon being disabled...
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::HandleWeaponDisable(bool bDisabled)
{
	if (bDisabled)
	{
		ClearPlayerModes(true);
	}
	else
	{
		m_pClientWeaponMgr->ShowWeapons();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ClearPlayerModes
//
//	PURPOSE:	Clear any special modes the player is in
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::ClearPlayerModes(bool bWeaponOnly)
{
	AimMgr::Instance().EndAim();
	m_bSemiAutoFire = false;

	// Make sure the camera isn't zoomed.
	if( m_pPlayerCamera->IsZoomed())
		m_pPlayerCamera->ExitZoom();

	m_pPlayerCamera->ResetHeightInterp();

	if (!bWeaponOnly)
	{
		if (m_bStoryMode) 
		{
			SetStoryMode(false);
		}

		ClearEarringEffect();
		g_pGameClientShell->GetMixer( )->ClearAllTemporaryMixers();

		LadderMgr::Instance().ReleaseLadder();
		SpecialMoveMgr::Instance().Release();

		// No longer playing special animation.
		CPlayerBodyMgr::Instance( ).ClearPlayingSpecial();

		if( m_pTurretFX )
			m_pTurretFX->Deactivate( );

		m_pFlashLight->TurnOff();

		// Clear any movement lock.
		AllowPlayerMovement( true );

		// Make sure all our HUD's get reset.
		g_pHUDMgr->Reset();

	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ClearEarringEffect
//
//	PURPOSE:	Clear the earring effect 
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::ClearEarringEffect()
{
	// Remove any soundmixers.
	ClientDB& ClientDatabase = ClientDB::Instance(); 
	g_pGameClientShell->GetMixer( )->ProcessMixerByName( "EarRingLight", 0, true );
	g_pGameClientShell->GetMixer( )->ProcessMixerByName( "EarRingMed", 0, true );
	g_pGameClientShell->GetMixer( )->ProcessMixerByName( "EarRing", 0, true );

	// Clean up the earring sound.
	if( m_hRingSound )
	{
		g_pLTClient->SoundMgr()->KillSound(m_hRingSound);
		m_hRingSound = NULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::RestorePlayerModes
//
//	PURPOSE:	Restore any special modes the player was in 
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::RestorePlayerModes()
{

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::DoActivate
//
//	PURPOSE:	Tell the server to do Activate
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::DoActivate()
{
	char const* pActivateOverride = g_vtActivateOverride.GetStr();
	if (pActivateOverride && pActivateOverride[0] != ' ')
	{
        g_pLTClient->RunConsoleCommand(pActivateOverride);
		return;
	}

	CActivationData data = GetTargetMgr()->GetActivationData();

	if (data.m_nType == MID_ACTIVATE_LADDER) 
	{
		CLadderFX *pLadder = g_pGameClientShell->GetSFXMgr()->GetLadderFX(data.m_hTarget);
		if (pLadder)
		{
			if (!LadderMgr::Instance().ActivateLadder(pLadder))
				return;
		}
	}
	else if( data.m_nType == MID_ACTIVATE_TURRET )
	{
		CTurretFX *pTurret = g_pGameClientShell->GetSFXMgr( )->GetTurretFX( data.m_hTarget );
		if( pTurret && !pTurret->IsInUse( ) )
		{
			pTurret->Activate( );
		}
	}
	else if( data.m_nType == MID_ACTIVATE_SPECIALMOVE )
	{
		CSpecialMoveFX *pSpecialMove = g_pGameClientShell->GetSFXMgr()->GetSpecialMoveFX( data.m_hTarget );
		if (pSpecialMove && (pSpecialMove->GetSFXID() == SFX_SPECIALMOVE_ID))	// don't activate subclasses (like forensics)
		{
			if (!SpecialMoveMgr::Instance().Activate(pSpecialMove))
				return;
		}
	}
	else if( data.m_nType == MID_ACTIVATE_SURFACESND )
	{
		// Check if there is just an activate sound on the main world.
		if( data.m_hActivateSnd )
		{
			g_pClientSoundMgr->PlayDBSoundLocal( data.m_hActivateSnd, SOUNDPRIORITY_PLAYER_MEDIUM, PLAYSOUND_CLIENT);
		}
		
		// Skip sending to server. Completely handled on client.
		return;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_ACTIVATE);
	data.Write(cMsg);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::Teleport
//
//	PURPOSE:	Tell the server to teleport to the specified point
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::Teleport(const LTVector & vPos)
{
#ifndef _FINAL // Don't allow player teleporing in the final version...

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_TELEPORT);
	cMsg.WriteLTVector(vPos);
	cMsg.WriteLTRotation(m_pPlayerCamera->GetCameraRotation( ));
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

#endif // _FINAL
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateServerPlayerModel()
//
//	PURPOSE:	Puts the server's player model where our invisible one is
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateServerPlayerModel()
{
	HOBJECT hClientObj, hRealObj;

    hClientObj = g_pLTClient->GetClientObject();
    if( !hClientObj )
		return;

	hRealObj = m_pMoveMgr->GetObject();
	if( !hRealObj )
		return;

    LTVector myPos;
	g_pLTClient->GetObjectPos(hRealObj, &myPos);
	g_pLTClient->SetObjectPos(hClientObj, myPos);

	// Don't need to update the server rotation unless we're in asscam.
	if( g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() != CPlayerCamera::kCM_Chase )
		return;

	if (g_vtPlayerRotate.GetFloat(1.0) > 0.0)
	{
		// Only allow yaw.
		EulerAngles eulerAngles = Eul_FromQuat( m_pPlayerCamera->GetCameraRotation( ), EulOrdYXZr );
		LTRotation rRot( 0.0f, eulerAngles.x, 0.0f );
		g_pLTClient->SetObjectRotation(hClientObj, rRot);
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateContainers
//
//	PURPOSE:	Update anything associated with being in a container
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateContainers()
{
	LTVector vCamPos = m_pPlayerCamera->GetCameraPos( );

    LTVector vScale(1.0f, 1.0f, 1.0f), vLightAdd(0.0f, 0.0f, 0.0f);

    HRECORD hCurSound      = NULL;
	HRECORD hSoundFilterRecord = SoundFilterDB::Instance().GetDynamicFilterRecord();

	bool bHasContainerChanged = false;

	// We'll update this below...
	m_bInSafetyNet	= false;
	m_bUseWorldFog	= true;

	// [KLS 4/16/02] Find container objects that we care about...

	HLOCALOBJ hContainerObj = NULL;
	ContainerCode eNewContainerCode = CC_NO_CONTAINER;
	const ContainerCode kePreviousContainerCode = m_eCurContainerCode;

	HLOCALOBJ objList[MAX_OVERLAPPING_CONTAINERS];
	uint32 nContainerFlags = (CC_ALL_FLAG & ~CC_PLAYER_IGNORE_FLAGS);
    uint32 dwNumContainers = ::GetPointContainers(vCamPos, objList, MAX_OVERLAPPING_CONTAINERS, nContainerFlags);

	for( uint32 dwContainer = 0; dwContainer < dwNumContainers; ++dwContainer )
	{
		uint16 code;
		if( g_pLTClient->GetContainerCode( objList[dwContainer], &code ))
		{
			ContainerCode eTempCode = static_cast<ContainerCode>(code);

			// Ignore dynamic sector volumes...
			if (CC_DYNAMIC_SECTOR_VOLUME != eTempCode)
			{
				// Look at all the containers on the list, and save off safety nets separately...
				if (CC_SAFTEY_NET == eTempCode)
				{
					// Don't count this as a normal container as it has one specific
					// purpose ONLY (protect the player)...
					m_bInSafetyNet = true;
				}
				else if (CC_NO_CONTAINER == eNewContainerCode || CC_FILTER == eNewContainerCode)
				{
					hContainerObj = objList[dwContainer];
					eNewContainerCode = eTempCode;
				}
			}
		}
	}

	// Update Dynamic sectors...
	UpdateDynamicSectors(objList, dwNumContainers);

	if (hContainerObj)
	{
		// Check for weather volume brush first (weather volume brushes
		// should never overlap normal volume brushes)

		CVolumeBrushFX* pFX = dynamic_cast<CVolumeBrushFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_VOLUMEBRUSH_ID, hContainerObj));
		pFX = UpdateVolumeBrushFX(pFX, eNewContainerCode);

		// See if we have entered/left a container...

		// if it's a sound filter, we need to compare the container
		// objects. Otherwise, if 2 different sound filter volumes
		// are touching, it will not change since it's moving
		// from one filter to another (even if they have different
		// data) -- Terry
		if ( eNewContainerCode == CC_FILTER )
		{
			if ( ( m_hCurContainerObject != hContainerObj ) ||
				 ( kePreviousContainerCode != eNewContainerCode ) )
			{
				bHasContainerChanged = true;
			}
		}
		else
		{
			if ( kePreviousContainerCode != eNewContainerCode )
			{
				bHasContainerChanged = true;
			}
		}
		m_hCurContainerObject = hContainerObj;

		if (bHasContainerChanged)
		{
			m_ContainerStartTimer.Start();

			if (pFX)
			{
				// Set the sound filter override...
				hSoundFilterRecord = pFX->GetSoundFilterRecord();

				// See if this container has fog associated with it..

                bool bFog = pFX->IsFogEnable();

				if (bFog)
				{
                    m_bUseWorldFog = false;

					g_pLTClient->SetConsoleVariableFloat("FogEnable", (float)bFog);
					g_pLTClient->SetConsoleVariableFloat("FogNearZ", pFX->GetFogNearZ());
					g_pLTClient->SetConsoleVariableFloat("FogFarZ", pFX->GetFogFarZ());

                    LTVector vFogColor = pFX->GetFogColor();

					g_pLTClient->SetConsoleVariableFloat("FogR", vFogColor.x / 255.0f);
					g_pLTClient->SetConsoleVariableFloat("FogG", vFogColor.y / 255.0f);
					g_pLTClient->SetConsoleVariableFloat("FogB", vFogColor.z / 255.0f);
				}

				// Get the tint color...

				vScale = pFX->GetTintColor();
				vScale /= 255.0f;

				vLightAdd = pFX->GetLightAdd();
				vLightAdd /= 255.0f;
			}
		}

		switch ( eNewContainerCode )
		{
			case CC_WATER:
			case CC_CORROSIVE_FLUID:
			case CC_FREEZING_WATER:
			{
				hCurSound = ClientDB::Instance().GetRecordLink(ClientDB::Instance().GetClientSharedRecord(), CDB_sClientShared_UnderwaterSound );
			}
			break;

			case CC_ENDLESS_FALL:
			{
                const double kfFallTime = 1.0f;

				if ( m_ContainerStartTimer.GetElapseTime() > kfFallTime)
				{
					vScale.Init(0.0f, 0.0f, 0.0f);
				}
				else
				{
                    float fScaleStart = .3f;
                    float fTimeLeft = (float)(kfFallTime - m_ContainerStartTimer.GetElapseTime());
                    float fScalePercent = (float)(fTimeLeft/kfFallTime);
                    float fScale = fScaleStart * fScalePercent;

					vScale.Init(fScale, fScale, fScale);
				}
			}
			break;

			default : break;
		}

	} // if (hContainerObj)
	else
	{
		// you've stepped out of a container entirely..
		if ( kePreviousContainerCode != eNewContainerCode )
		{
			bHasContainerChanged = true;
		}

		m_hCurContainerObject = NULL;
	}



	// See if we have entered/left a container...

	if (bHasContainerChanged)
	{
		// Change the code prior to calls that rely on the updated value.
		m_eCurContainerCode = eNewContainerCode;

		// Adjust world properties as necessary...
		g_pGameClientShell->ResetDynamicWorldProperties(m_bUseWorldFog);

		g_pGameClientShell->GetLightScaleMgr()->ClearLightScale(CLightScaleMgr::eLightScaleEnvironment);

		if (vScale.x != 1.0f || vScale.y != 1.0f || vScale.z != 1.0f)
		{
			g_pGameClientShell->GetLightScaleMgr()->SetLightScale(vScale, CLightScaleMgr::eLightScaleEnvironment);
		}

		// See if we are coming out of water...

		if ( IsLiquid( kePreviousContainerCode ) && !IsLiquid( eNewContainerCode ) )
		{
			UpdateUnderWaterFX(false);

			// Allow the player to use weapons again once they are out of water and not on a ladder...
			if( !LadderMgr::Instance( ).IsClimbing( ))
				g_pClientWeaponMgr->EnableWeapons( );
		}

		if ( !IsLiquid( kePreviousContainerCode ) && IsLiquid( eNewContainerCode ) && m_hCurContainerObject )
		{
			// entering water.. play the liquid splash sounds!

			HRECORD hSound;
			int32 fallpercent;

			// determine the sound from the liquid surface type..
			fallpercent = (int32)g_pMoveMgr->GetDistanceFallenPercent();

			SurfaceType eSurface = ::GetSurfaceType( m_hCurContainerObject );
			if( eSurface != ST_UNKNOWN )
			{
				hSound = GetLandingSound( eSurface, PPM_NORMAL, true, fallpercent );
				g_pClientSoundMgr->PlayDBSoundLocal(hSound, SOUNDPRIORITY_PLAYER_MEDIUM, PLAYSOUND_CLIENT);
			}

			// The player is not allowed to use weapons under water...
			if( IsSwimmingAllowed( ))
				g_pClientWeaponMgr->DisableWeapons( );

			LTVector vSplashPos;
			g_pLTClient->GetObjectPos( g_pMoveMgr->GetServerObject(), &vSplashPos );
			CPolyGridFX* pPolyGridFX = CPolyGridFX::FindSplashInPolyGrid( g_pMoveMgr->GetServerObject(), vSplashPos );
			CCharacterFX* pFX = g_pGameClientShell->GetLocalCharacterFX();
			if( pPolyGridFX && pFX )
			{
				pPolyGridFX->DoPolyGridSplash( g_pMoveMgr->GetServerObject(), vSplashPos, 
					g_pModelsDB->GetSplashesJumpingDeepImpulse( pFX->GetModel( )));
			}
		}

		UpdateSoundFilters(hSoundFilterRecord, false);

		if (m_hContainerSound)
		{
            g_pLTClient->SoundMgr()->KillSound(m_hContainerSound);
            m_hContainerSound = NULL;
		}

		if (hCurSound)
		{
            uint32 dwFlags = PLAYSOUND_CLIENT | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
			m_hContainerSound = g_pClientSoundMgr->PlayDBSoundLocal(hCurSound, SOUNDPRIORITY_PLAYER_MEDIUM, dwFlags);
		}

		g_pGameClientShell->GetScreenTintMgr()->Set(TINT_CONTAINER,&vLightAdd);
	}


	// See if we are under water (under any liquid)...

	if( IsUnderwater( ))
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
//	ROUTINE:	CPlayerMgr::UpdateVolumeBrushFX
//
//	PURPOSE:	Deterimine if we're really in a VolumeBrush that has
//				a polygrid surface (i.e., water)
//
//	NOTES:		Passed in eCode may be updated
//
// --------------------------------------------------------------------------- //

CVolumeBrushFX* CPlayerMgr::UpdateVolumeBrushFX(CVolumeBrushFX* pFX, 
												ContainerCode & eCode)
{
	if (!pFX) return NULL;

	// Get the PolyGridFX associated with the VolumeBrush...

/* 
KLS - COMMENTED OUT 4/12/04 - The GetVolumeBrush() call below would always return NULL because
volume brushes are not associated with polygrids anymore.  If we want the functionality
of this code again (which never actually worked correctly :) we'll probably need to search 
through all the polygrids and test against each one....


	// Get a list of all the poly grids...

	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList( SFX_POLYGRID_ID );
	if( pList )
	{
		int nNumPGrids = pList->GetSize();
		HOBJECT hObj = pFX->GetServerObj();

		// Try and find a polygrid that is the surface of our container...
		
		for( int i = 0; i < nNumPGrids; ++i )
		{
			if( (*pList)[i] )
			{
				CPolyGridFX* pPGrid= (CPolyGridFX*)(*pList)[i];

				if( pPGrid->GetVolumeBrush() == hObj )
				{
					float fDisplacement;
					LTVector vIntersection;

					LTRotation const& rRot = m_pPlayerCamera->GetCameraRotation( );

					LTVector const& vCamPos = m_pPlayerCamera->GetCameraPos( );

					// Develop a position out and down from the camera to test from...

					LTVector vPos = vCamPos + (rRot.Forward() * 5.0f);
					vPos.y -= 2.0f;
					
					// See if we interected the polygrid...

					if( pPGrid->GetOrientedIntersectionHeight( vPos, LTVector(0,1,0), fDisplacement, vIntersection ))
					{
						vIntersection.y += fDisplacement;

						// If we are above it clear our container and fx...
						
						if( vPos.y > vIntersection.y )
						{
							eCode = CC_NO_CONTAINER;
							pFX = NULL;
						}
					}

					break;
				}
			}
		}
	}
*/
	return pFX;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateSoundFilters
//
//	PURPOSE:	Update sound filters
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateSoundFilters(HRECORD hSoundFilterRecord, bool bOverride)
{
	// Verify the passed in sound filter is valid.
	if (NULL == hSoundFilterRecord)
{
		LTASSERT(0, "CPlayerMgr::UpdateSoundFilters : Passed in SoundFilterRecord is NULL.");
		return;
	}

#ifndef USE_EAX20_HARDWARE_FILTERS
	return;
#endif // USE_EAX20_HARDWARE_FILTERS

	if (g_vtUseSoundFilters.GetFloat())
	{
		// if we're not overriding, track the current container..
		if (!bOverride)
		{
			m_hSoundFilterIdCurrentContainer = hSoundFilterRecord;
		}

		// check if we're acting appropriately.. if the system is
		// set to override, but this call is coming as a non-override,
		// then we don't do anything..
		if ( (m_pSoundFilterIDCurrent != hSoundFilterRecord ) &&
			(bOverride == m_bSoundFilterOverrideOn) )
		{
			m_pSoundFilterIDCurrent = hSoundFilterRecord;

			//bool bFilterOK = true;

			ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();

			// tell the sound engine about new filter, if a dynamic filter, 
			// use the global sound filter

			bool bUsingDynamic = false;
			if ( SoundFilterDB::Instance().IsFilterDynamic( hSoundFilterRecord ) )
			{
				bUsingDynamic = true;
				m_hSoundFilterIdCurrentContainer = m_hSoundFilterIdGlobal;
			   	hSoundFilterRecord = m_hSoundFilterIdGlobal;
			}

			ApplySoundFilter(hSoundFilterRecord, (bUsingDynamic && m_eCurContainerCode == CC_NO_CONTAINER));
					}
				}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OverrideSoundFilter
//
//	PURPOSE:	Sets the override for the sound filter from containers.
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::OverrideSoundFilter(const char* pOverrideFilterName, bool bOverrideOn)
{
	m_bSoundFilterOverrideOn = bOverrideOn;

	if (m_bSoundFilterOverrideOn)
	{
		HRECORD nSoundFilterId = NULL;

		nSoundFilterId = SoundFilterDB::Instance().GetFilterRecord(pOverrideFilterName);

		m_hSoundFilterIDOverride = nSoundFilterId;
		UpdateSoundFilters(m_hSoundFilterIDOverride, true);
	}
	else
	{
		// null it out, just to be safe;
		m_hSoundFilterIDOverride = NULL;

		// put back the old one..
		if (m_hSoundFilterIdCurrentContainer == NULL)
		{
			UpdateSoundFilters(m_hSoundFilterIdGlobal, false);
		}
		else
		{
			UpdateSoundFilters(m_hSoundFilterIdCurrentContainer, false);
		}
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateUnderWaterFX
//
//	PURPOSE:	Update under water fx
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateUnderWaterFX(bool bUpdate)
{
	if( m_pPlayerCamera->IsZoomed() )
		return;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateBreathingFX
//
//	PURPOSE:	Update breathing fx
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateBreathingFX(bool /*bUpdate*/)
{
	//if (m_nZoomView) return;

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateDynamicSectors
//
//	PURPOSE:	Update dynamic sectors 
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::UpdateDynamicSectors(HLOCALOBJ* pContainerArray, uint32 nNumContainers)
{
	CDynamicSectorVolumeFX* enabledSectorFX[50];
	CDynamicSectorVolumeFX* disabledSectorFX[50];
	uint32 nNumEnabledSectorFX = 0;
	uint32 nNumDisabledSectorFX = 0;

	// Find the sector volumes fx that we wish to enable...

	for (uint32 i=0; i < nNumContainers; i++)
	{
		// We only care about "visible" containers...

		uint32 dwUserFlags = USRFLG_VISIBLE;
		g_pCommonLT->GetObjectFlags(pContainerArray[i], OFT_User, dwUserFlags);
	
		if (dwUserFlags & USRFLG_VISIBLE)
		{
			uint16 code;
			if (g_pLTClient->GetContainerCode(pContainerArray[i], &code))
			{
				ContainerCode eTempCode = (ContainerCode)code;

				if (CC_DYNAMIC_SECTOR_VOLUME == eTempCode)
				{
					disabledSectorFX[nNumDisabledSectorFX] = (CDynamicSectorVolumeFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_DYNAMIC_SECTOR_ID, pContainerArray[i]);
					nNumDisabledSectorFX++;
				}
			}
		}
	}

	// Find the sector volume FX that we wish to disable...

	CSpecialFXList* pDynSectorList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_DYNAMIC_SECTOR_ID);
	if (pDynSectorList)
	{
		uint32 nNum = pDynSectorList->GetSize();

		for (uint32 i=0; i < nNum; i++)
		{
			CDynamicSectorVolumeFX* pFX = (CDynamicSectorVolumeFX*)(*pDynSectorList)[i];
			if (!pFX) continue;

			// Check to see if this fx is on the enabled list...

			bool bDisabled = false;
			for (uint32 j=0; j < nNumDisabledSectorFX; j++)
			{
				if (pFX == disabledSectorFX[j])
				{
					bDisabled = true;
					break;
				}
			}

			// Add the fx to the disabled list...

			if (!bDisabled)
			{
				enabledSectorFX[nNumEnabledSectorFX] = pFX;
				nNumEnabledSectorFX++;
			}
		}
	}


	// Disable the necessary sector volumes...

	for (i=0; i < nNumDisabledSectorFX; i++)
	{
		if (disabledSectorFX[i])
		{
			(disabledSectorFX[i])->Enable(false);
		}
	}

	// Enable the necessary sector volumes...

	for (i=0; i < nNumEnabledSectorFX; i++)
	{
		if (enabledSectorFX[i])
		{
			(enabledSectorFX[i])->Enable(true);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetMouseInput()
//
//	PURPOSE:	Allows or disallows mouse input on the client
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetMouseInput( bool bAllowInput, bool bRestoreBackupAngles )
{
	if (bAllowInput)
	{
        m_bRestoreOrientation = true;
		
		if( bRestoreBackupAngles )
		{
			m_pPlayerCamera->RestoreBackupRotation( );
		}
	}
	else
	{
		m_pPlayerCamera->SaveBackupRotation( );
	}
}

void CPlayerMgr::AllowPlayerMovement(bool bAllowPlayerMovement)
{
	m_bLastAllowPlayerMovement	= m_bAllowPlayerMovement;
	m_bAllowPlayerMovement = bAllowPlayerMovement;

	SetMouseInput( bAllowPlayerMovement, false );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ShowPlayer()
//
//	PURPOSE:	Show/Hide the player object
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::ShowPlayer(bool bShow)
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

	g_pCommonLT->SetObjectFlags(hPlayerObj, OFT_Flags, (bShow ? FLAG_VISIBLE : 0), FLAG_VISIBLE);

	if( CPlayerBodyMgr::Instance( ).IsEnabled() )
	{
		g_pLTClient->SetObjectShadowLOD( hPlayerObj, eEngineLOD_Never );
	}
	else if (!IsSpectating( ))
	{
		// KLS - Always show the shadow even when the player is hidden
		//g_pLTClient->SetObjectShadowLOD(hPlayerObj, bShow ? eEngineLOD_Low : eEngineLOD_Never);
		g_pLTClient->SetObjectShadowLOD(hPlayerObj, eEngineLOD_Low);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayerVisibility()
//
//	PURPOSE:	Hide and show the player...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlayerVisibility( )
{
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if( !hPlayerObj || !IsPlayerAlive() )
		return;


	// This is pretty much a complete kludge, but I can't really think of
	// a better way to handle this...Okay, since the server can update the
	// player's flags at any time (and override anything that we set), we'll
	// make sure that the player's flags are always what we want them to be :)

    uint32 dwPlayerFlags;
	g_pCommonLT->GetObjectFlags(hPlayerObj, OFT_Flags, dwPlayerFlags);
	if( m_pPlayerCamera->GetCameraMode() == CPlayerCamera::kCM_FirstPerson )
	{
		if (dwPlayerFlags & FLAG_VISIBLE)
		{
// PLAYER_BODY
			if( !CPlayerBodyMgr::Instance( ).IsEnabled() )
				m_pPlayerCamera->AttachToTarget(false);

			ShowPlayer(false);

			// Show the normal player-view weapon model...NOTE: This shouldn't be
			// called all the time as we may want to hide the weapons some times, need
			// to come up with a better solution if we're going to support player-view
			// weapons in the future....
			m_pClientWeaponMgr->ShowWeapons();
		}

	}
	else  // Third person
	{
		if (!(dwPlayerFlags & FLAG_VISIBLE))
		{
			if( !CPlayerBodyMgr::Instance( ).IsEnabled() )
				ShowPlayer(true);
		}
	}

	// Hide/Show our attachments...
	HideShowAttachments( hPlayerObj );

	if  ( IsSpectating())
	{
		EEngineLOD eLOD;
		g_pLTClient->GetObjectShadowLOD(hPlayerObj,eLOD);
		if (eLOD != eEngineLOD_Never)
			DebugCPrint(0,"hPlayerObj shadow visible");
		g_pLTClient->GetObjectShadowLOD(CPlayerBodyMgr::Instance( ).GetObject(),eLOD);
		if (eLOD != eEngineLOD_Never)
			DebugCPrint(0,"PlayerBodyMgr shadow visible");
		g_pLTClient->GetObjectShadowLOD(g_pMoveMgr->GetObject(),eLOD);
		if (eLOD != eEngineLOD_Never)
			DebugCPrint(0,"MoveMgr shadow visible");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HideShowAttachments()
//
//	PURPOSE:	Recursively hide/show attachments...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HideShowAttachments(HOBJECT hObj)
{
	if (!hObj) return;

	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

    g_pCommonLT->GetAttachments(hObj, attachList, 20, dwListSize, dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	for (int i=0; i < nNum; i++)
	{
        uint32 dwUsrFlags;
        g_pCommonLT->GetObjectFlags(attachList[i], OFT_User, dwUsrFlags);

        if (dwUsrFlags & USRFLG_ATTACH_HIDE1SHOW3)
		{
			switch (m_pPlayerCamera->GetCameraMode())
			{
			case CPlayerCamera::kCM_FirstPerson:
			case CPlayerCamera::kCM_Cinematic:
			case CPlayerCamera::kCM_Track:
			case CPlayerCamera::kCM_Fly:
			case CPlayerCamera::kCM_Follow:
//			case CPlayerCamera::kCM_Fixed:	-- not sure where this belongs.
				{
// PLAYER_BODY prototype...
					if( CPlayerBodyMgr::Instance( ).IsEnabled() )
					{
						g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
						g_pLTClient->SetObjectShadowLOD(attachList[i], eEngineLOD_Never);
					}
					else
					{
						g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
						g_pLTClient->SetObjectShadowLOD(attachList[i], eEngineLOD_Never);

						// KLS - Even though the attachment is hidden, assume we want to see the
						// shadow - for things like hand-held weapons...
						bool bShadow = (!IsSpectating() && g_vtWeaponShadow.GetFloat() > 0.0f);

						g_pLTClient->SetObjectShadowLOD(attachList[i], (bShadow ? eEngineLOD_Low : eEngineLOD_Never) );
					}
				}
				break;

			case CPlayerCamera::kCM_Chase:
			default:
				{
					g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
					g_pLTClient->SetObjectShadowLOD(attachList[i], eEngineLOD_Low);
				}
				break;
			}
		}

		if( g_pVersionMgr->IsLowViolence() && (dwUsrFlags & USRFLG_GORE) )
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
			g_pLTClient->SetObjectShadowLOD(attachList[i], eEngineLOD_Never);
		}

		// Hide/Show this attachment's attachments...
		HideShowAttachments(attachList[i]);
	}
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CPlayerMgr::InitTargetMgr()
//
//	PURPOSE:	Init the TargetMgr...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::InitTargetMgr()
{
	m_pTargetMgr = debug_new( CTargetMgr );
	
	//force the initializtion
	CAutoTargetMgr::Instance();

	ASSERT( 0 != m_pTargetMgr );
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CPlayerMgr::ClearDamageSectors()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::ClearDamageSectors()
{
	memset(m_fDamage,0,sizeof(m_fDamage));
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CPlayerMgr::UpdateDamage()
//
//	PURPOSE:	Update the damage sectors...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateDamage()
{
	if (g_pGameClientShell->IsGamePaused()) return;

	float fDelta = ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS() * g_vtDamageFadeRate.GetFloat();
	for (int i = 0; i < kNumDamageSectors; i++)
	{
		if (m_fDamage[i] > fDelta)
			m_fDamage[i] -= fDelta;
		else
			m_fDamage[i] = 0.0f;
	}

	if( m_MeleeDamageEffectTimer.IsTimedOut( ) && m_MeleeDamageEffectTimer.IsStarted( ))
	{
		m_fMeleeDamageDistortionIntensity = 0.0f;
		m_fMeleeDamageOverlayBlur = 0.0f;

		g_pLTClient->SetConsoleVariableFloat("ScreenGlowIntensity", m_fMeleeDamageDistortionIntensity);
		g_pLTClient->SetConsoleVariableFloat("ScreenGlowEnable", 0.0f );

		if( g_vtCameraMeleeDamageOverlay.GetFloat( ) > 0.0f )
		{
			HMATERIAL hmatDamage = g_pOverlay->GetMaterialInstance( g_pOverlay->GetHUDOverlay( OVM_DAMAGE ));
			if( hmatDamage )
			{
				g_pLTClient->GetRenderer()->SetInstanceParamFloat( hmatDamage, "fBlurScale", m_fMeleeDamageOverlayBlur );
			}

			g_pOverlay->Hide( g_pOverlay->GetHUDOverlay( OVM_DAMAGE ));
		}

        m_MeleeDamageEffectTimer.Stop();
	}
	else if( m_MeleeDamageEffectTimer.GetTimeLeft() > 0.0f )
	{
		double fDuration = m_MeleeDamageEffectTimer.GetDuration();
		double fTimeLeft = m_MeleeDamageEffectTimer.GetTimeLeft();
		float fT = (float)((fDuration - fTimeLeft) / fDuration);

		if( fT <= 0.5f )
			// Double the speed up...
			fT *= 2.0f;
		else
			// Double the speed down...
			fT = 1 - ((fT * 2.0f) - 1);
		
		// Add in the intensity of the screen distortion...
		if( g_vtCameraMeleeDamageDistortion.GetFloat( ) > 0.0f )
		{
            m_fMeleeDamageDistortionIntensity = LTCLAMP( LTLERP( 0.0f, m_fMeleeDamageDistortionI1, fT ), 0.0f, 1.0f );
			
			if( g_vtCameraMeleeDamageInfo.GetFloat( ) > 0.0f )
				g_pLTClient->CPrint( "Intensity: %f - Duration: %f - TimeLeft: %f - T: %f - I1: %f", m_fMeleeDamageDistortionIntensity, fDuration, fTimeLeft, fT, m_fMeleeDamageDistortionI1 );

			g_pLTClient->SetConsoleVariableFloat("ScreenGlowIntensity", m_fMeleeDamageDistortionIntensity);
		}

		// Add in the blur...
		if( g_vtCameraMeleeDamageOverlay.GetFloat( ) > 0.0f )
		{
			m_fMeleeDamageOverlayBlur = LTCLAMP( LTLERP( 0.0f, m_fMeleeDamageOverlayB1, fT ), 0.0f, 0.95f );

			HMATERIAL hmatDamage = g_pOverlay->GetMaterialInstance( g_pOverlay->GetHUDOverlay( OVM_DAMAGE ));
			if( hmatDamage )
			{
				g_pLTClient->GetRenderer()->SetInstanceParamFloat( hmatDamage, "fBlurScale", m_fMeleeDamageOverlayBlur );
			}

			if( g_vtCameraMeleeDamageInfo.GetFloat( ) > 0.0f )
				g_pLTClient->CPrint( "Blur: %f - Duration: %f - TimeLeft: %f - T: %f - B1: %f", m_fMeleeDamageOverlayBlur, fDuration, fTimeLeft, fT, m_fMeleeDamageOverlayB1 );

		}
	}
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CPlayerMgr::UpdateConcussionAudioEffect()
//
//	PURPOSE:	Update the audio effect..
//
// ----------------------------------------------------------------------- //

// this will probably be temp (or at least greatly reworked) later..
// the goal is to make it a ClientFX in the effect editor..
void CPlayerMgr::UpdateConcussionAudioEffect()
{
	if (g_pGameClientShell->IsGamePaused()) return;

	switch (m_nAudioEffectState)
	{
		case (0):
		default:
			// nothin..
			break;
		case (1):	// just plain playin'..
			if (m_AudioTimer.GetTimeLeft() <= 0.0f)
			{
				// enter fade sequence
				m_AudioTimer.Start(6.0f);
				m_nAudioEffectState = 2;
				g_pLTClient->SoundMgr()->SetSoundVolume(m_hRingSound, 0, 6.0f);
			}
			break;
		case (2): //fading..
			if (m_AudioTimer.GetTimeLeft() <= 0.0f)
			{
				// enter fade sequence
				m_nAudioEffectState = 0;
				g_pLTClient->SoundMgr()->KillSound(m_hRingSound);
				m_hRingSound = NULL;
			}
			break;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateDistanceIndicator()
//
//	PURPOSE:	If we are close to a distance indicator object update the huditem...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateDistanceIndicator()
{
	float fLastDistPercent = m_fDistanceIndicatorPercent;
	m_fDistanceIndicatorPercent = -1.0f;

	// Currently the only distance indicator objects are triggers...

	CSpecialFXList* pTriggerList = g_pGameClientShell->GetSFXMgr()->GetFXList( SFX_TRIGGER_ID );
	if( pTriggerList )
	{
		uint32 nNum = pTriggerList->GetNumItems();

		for( uint32 i = 0; i < nNum; ++i )
		{
			CTriggerFX* pFX = (CTriggerFX*)(*pTriggerList)[i];
			if( !pFX || !pFX->WithinIndicatorRadius() ) continue;

			// Just grab the first distance being tracked.  The distance indicators
			// shouldn't overlap.  If they do it's an LD issue or this needs to change :)

			m_fDistanceIndicatorPercent	= pFX->GetDistancePercentage();
			m_hDistanceIndicatorIcon	= pFX->GetIcon();
			break;
		}
	}

	if( (m_fDistanceIndicatorPercent != fLastDistPercent) && m_hDistanceIndicatorIcon )
		g_pHUDMgr->QueueUpdate( kHUDDistance );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdatePlayerMovement
//
//	PURPOSE:	Move and rotate the player based on the input offsets...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdatePlayerMovement( )
{
	bool bInputLocked = (CPlayerBodyMgr::Instance().IsEnabled() && (CPlayerBodyMgr::Instance().GetInputDescriptor() == kAD_IN_Locked));
	if (!bInputLocked)
	{
		// The axis offsets determine rotation, so calculate them first...
		CalculateAxisOffsets();

		// Update the camera rotation based on the calculated offsets, this needs to be done beofe the player rotation...
		if( m_pPlayerCamera->GetCameraMode() != CPlayerCamera::kCM_Fixed )
			m_pPlayerCamera->ApplyLocalRotationOffset( LTVector( m_v2AxisOffsets.x, m_v2AxisOffsets.y, 0.0f ));
	}

	// Update the movement of the player, requires the above pre-calculation of player rotation...
	m_pMoveMgr->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CanFireWeapon
//
//	PURPOSE:	Tests if the player meets the requirements for firing a weapon...
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::CanFireWeapon( ) const
{
	bool bFire = false;

	// only fire if:
	//    fire key down AND
	//    player is not dead AND
	//    player is not in spectator mode AND
	//    player is not in story mode AND
	//    player is actually playing the game
	if ( ( m_dwPlayerFlags & BC_CFLG_FIRING ) &&
		( IsPlayerAlive() ) &&
		( !IsSpectating() ) &&
		( !m_bStoryMode ) &&
		( g_pInterfaceMgr->GetGameState() == GS_PLAYING ) )
	{
		bFire = true;
	}

	return bFire;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CanAltFireWeapon
//
//	PURPOSE:	Tests if the player meets the requirements for Alt-firing a weapon...
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::CanAltFireWeapon( ) const
{
	bool bFire = false;

	// only fire if:
	//    fire key down AND
	//    player is not dead AND
	//    player is not in spectator mode AND
	//    player is not in story mode AND
	//    player is actually playing the game
	if ( ( m_dwPlayerFlags & BC_CFLG_ALT_FIRING ) &&
		( IsPlayerAlive() ) &&
		( !IsSpectating() ) &&
		( !m_bStoryMode ) &&
		( g_pInterfaceMgr->GetGameState() == GS_PLAYING ) &&
		( !m_pPlayerCamera->IsZoomed( )))
	{
		bFire = true;
	}

	return bFire;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::GetPlayerTimerElapsedS
//
//	PURPOSE:	Returns the amount of time that has elapsed for the local 
//				player's update interval - Utility function that avoids
//				caller having to have knowledge of how player object is
//				implemented (i.e., server object in move mgr)
//
// ----------------------------------------------------------------------- //

float CPlayerMgr::GetPlayerTimerElapsedS( ) const
{
	ObjectContextTimer objectContextTimer(g_pMoveMgr->GetServerObject());
	return (objectContextTimer.GetTimerElapsedS());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CanThrowGrenade
//
//	PURPOSE:	Tests if the player meets the requirements for throwing a grenade...
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::CanThrowGrenade( ) const
{
	bool bFire = false;

	// only fire if:
	//    fire key down AND
	//    player is not dead AND
	//    player is not in spectator mode AND
	//    player is not in story mode AND
	//    player is actually playing the game
	if ( ( m_dwPlayerFlags & BC_CFLG_GRENADE ) &&
		( IsPlayerAlive() ) &&
		( !IsSpectating() ) &&
		( !m_bStoryMode ) &&
		( g_pInterfaceMgr->GetGameState() == GS_PLAYING )  &&
		( g_pPlayerStats->GetCurrentGrenadeRecord() != NULL) &&
		( g_pPlayerStats->GetCurrentGrenadeCount() > 0 )
		)
	{
		bFire = true;
	}
	return bFire;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleMsgPlayerEvent
//
//	PURPOSE:	Handle MID_PLAYER_EVENT message from server
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::HandleMsgPlayerEvent(ILTMessage_Read *pMsg)
{
	PlayerEventMsgType eType = static_cast<PlayerEventMsgType>(pMsg->Readuint8());

	switch(eType) 
	{
	case kPECrosshair:
		{
			bool bEnable = pMsg->Readbool();
			g_pCrosshair->Enable(bEnable);
			return true;
		}
		break;
	case kPEFlashlight:
		{
			bool bEnable = pMsg->Readbool();
			float fFlickerDuration = pMsg->Readfloat();
			m_pFlashLight->Enable(bEnable,fFlickerDuration);
			return true;
		}
		break;
	case kPEHeartBeat:
		{
			bool bEnable = pMsg->Readbool();

			if (bEnable)
			{
				float fDuration = pMsg->Readfloat();
				m_HeartBeat.Start(fDuration);
			}
			else
			{
				m_HeartBeat.Stop();
			}
			return true;
		}
		break;
	case kPEBreathing:
		{
			bool bEnable = pMsg->Readbool();

			if (bEnable)
			{
				float fDuration = pMsg->Readfloat();
				m_Breath.Start(fDuration);
			}
			else
			{
				m_Breath.Stop();
			}
			return true;
		}
		break;
	
	case kPECarry:
		{
			PropsDB::HPROP hProp = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pPropsDB->GetPropsCategory( ));
			if( !hProp )
			{
				// Drop any currently carried object...
				SetCarryObject( NULL );
			}
			else
			{
				// Create and carry object...
				SetCarryObject( hProp );
			}
		}
		break;

	case kPEBlocked:
		{
			m_bBlocked = true;
		}
		break;
	case kPEStoryMode:
		{
			const bool bOn = pMsg->Readbool();
			const bool bCanSkip = pMsg->Readbool();
			SetStoryMode(bOn, bCanSkip);
		}break;

	case kPEWeaponEffect:
		{
			char szFX[128];
			char szSocket[128];

			pMsg->ReadString(szFX, LTARRAYSIZE(szFX));
			pMsg->ReadString(szSocket, LTARRAYSIZE(szSocket));

			CreateWeaponEffect(szFX, szSocket);
		}
		break;

	case kPESyncAction:
		{
			HRECORD hSyncAction = pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pModelsDB->GetSyncActionCategory());
			HOBJECT hObject = pMsg->ReadObject();
			HandleSyncAction(hSyncAction, hObject);
		}
		break;
	};

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::FocusObjectDetectorSpatialCB
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::FocusObjectDetectorSpatialCB( ObjectDetectorLink* pLink, LTVector& vPos, LTVector& vDims, void* pUserData )
{
	HOBJECT hFocusObject = pLink->GetObject();
	HMODELNODE hInitialNode;
	HMODELNODE hFocusNode;

	LTTransform iInitialTrans;
	LTTransform iFocusTrans;

	// When using a ranged weapon...  do an accuracy adjustment from the pelvis to the head...
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();

	if( pWeapon && !pWeapon->IsMeleeWeapon() )
	{
		if( LT_OK != g_pLTClient->GetModelLT()->GetNode( hFocusObject, "Pelvis", hInitialNode ) )
		{
			return;
		}

		if( LT_OK != g_pLTClient->GetModelLT()->GetNode( hFocusObject, "Head", hFocusNode ) )
		{
			return;
		}

		g_pLTClient->GetModelLT()->GetNodeTransform( hFocusObject, hInitialNode, iInitialTrans, true );
		g_pLTClient->GetModelLT()->GetNodeTransform( hFocusObject, hFocusNode, iFocusTrans, true );

		vPos = ( iInitialTrans.m_vPos + ( ( iFocusTrans.m_vPos - iInitialTrans.m_vPos ) * g_pPlayerMgr->GetFocusAccuracy() ) );
		return;
	}

	// Otherwise, just do a simple check against the torso...
	if( LT_OK != g_pLTClient->GetModelLT()->GetNode( hFocusObject, "Upper_Torso", hFocusNode ) )
	{
		return;
	}

	g_pLTClient->GetModelLT()->GetNodeTransform( hFocusObject, hFocusNode, iFocusTrans, true );
	vPos = iFocusTrans.m_vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::PickupObjectDetectorCustomTestCB
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::PickupObjectDetectorCustomTestCB( ObjectDetectorLink* pLink, float& fMetRR, void* pUserData )
{
	// Set to lowest priority.
	fMetRR = 1.0f;

	bool bFound = false;

	// Get the pickupitem object.
	CPickupItemFX* pPickupItemFX = static_cast< CPickupItemFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_PICKUPITEM_ID, pLink->GetObject()));
	if( pPickupItemFX )
	{
		// Get the pickup type so we can get its name properly.
		HRECORD hRecord = pPickupItemFX->GetTypeRecord();
		if( !hRecord )
			return false;

		const char* pszName = g_pLTDatabase->GetRecordName(hRecord);

		bFound = true;

		// Check if we don't have the weapon and don't have room for it.
		if( pPickupItemFX->IsMustSwap( ))
		{
			// Set it to the highest priority.
			fMetRR = 0.0f;
		}
	}

	// Not a pickup item.
	if( !bFound )
	{
		// See if it's a ctfflag.
		CTFFlagSFX* pCTFFlagSFX = reinterpret_cast< CTFFlagSFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_CTFFLAG_ID, pLink->GetObject() ));
		if( pCTFFlagSFX )
		{
			bFound = true;

			// Set it to the highest priority.
			fMetRR = 0.0f;
		}
	}


	// Is it a recoverable projectile?
	if( !bFound )
	{
		// See if it's a projectile
		CProjectileFX* pProjSFX = reinterpret_cast< CProjectileFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_PROJECTILE_ID, pLink->GetObject() ));
		if( pProjSFX )
		{
			bFound = pProjSFX->IsRecoverable();

			// Set it to the highest priority.
			if (bFound)
			{
				fMetRR = 0.0f;
			}
			
		}
	}

	// Ignore if not a known item.
	if( !bFound )
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ForensicObjectDetectorCustomTestCB
//
// ----------------------------------------------------------------------- //

bool CPlayerMgr::ForensicObjectDetectorCustomTestCB( ObjectDetectorLink* pLink, float& fMetRR, void* pUserData )
{
	// Set to lowest priority.
	fMetRR = 1.0f;

	// Abort if we are not in an instinct volume.
	if(!g_pPlayerMgr->m_pForensicObject)
		return false;

	// Get the forensic object.
	CForensicObjectFX* pForensicObjectFX = static_cast< CForensicObjectFX* >( g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_FORENSICOBJECT_ID, pLink->GetObject()));
	if( !pForensicObjectFX )
		return false;
	if( !pForensicObjectFX->CanReach() )
		return false;

	// Set to highest priority (since there really should only ever be one anyway - no need to do any fancy calculations).
	fMetRR = 1.0f;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::IntoneSonic
//
//	PURPOSE:	do a sonic thing...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::IntoneSonic( const char* sSonic )
{
	m_iSonicData.IntoneSonic( sSonic, g_pLTClient->GetClientObject(), g_iFocusObjectDetector.GetObject() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetKiller
//
//	PURPOSE:	remember who killed us...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetKiller(uint32 nKiller)
{
	m_hKiller = NULL;

	// Ignore killing ourselves.  We only use this for 
	// tracking spectator mode.
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);
	if( nKiller == nLocalID )
	{
		return;
	}

	m_hKiller = NULL;
	if (nKiller >= 0) 
	{
		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
		if (!psfxMgr) return;

		CCharacterFX* pCFX = psfxMgr->GetCharacterFromClientID(nKiller);
		if (!pCFX) return;

		m_hKiller = pCFX->GetServerObj();
	}
}

// ----------------------------------------------------------------------- //

void CPlayerMgr::SetForensicObject(CForensicObjectFX* pFX)
{
	m_pForensicObject = pFX;

	const char* pszFX = ClientDB::Instance().GetString(ClientDB::Instance().GetClientSharedRecord(), pFX ? CDB_ClientShared_ForensicEnterClientFX : CDB_ClientShared_ForensicExitClientFX);
	if (pszFX && pszFX[0])
	{
		CLIENTFX_CREATESTRUCT fxInit(pszFX, 0, g_pMoveMgr->GetServerObject());
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxInit, true);
	}
}

// ----------------------------------------------------------------------- //

bool CPlayerMgr::WithinStairVolume()
{
	// Update based on the client position
	LTVector vClientPos;
	HOBJECT hClientObj = g_pLTClient->GetClientObject();
	g_pLTClient->GetObjectPos( hClientObj, &vClientPos );

	// Get a list of containers at this point
	HOBJECT aObjects[ 32 ];
	uint32 nContainers = g_pLTClient->GetPointContainers( &vClientPos, aObjects, 32 );

	// Go through the containers and see if any of them are instincts
	uint16 nCode;

	for( uint32 i = 0; i < nContainers; ++i )
	{
		g_pLTClient->GetContainerCode( aObjects[ i ], &nCode );

		if( nCode == CC_STAIRS )
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetOperatingTurret
//
//	PURPOSE:	Cache the turret the player is activating and handle anyother player setup for operating a turret...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetOperatingTurret( CTurretFX *pTurret )
{
	// Set/Clear the turret...
	m_pTurretFX = pTurret;

	g_pHUDMgr->UpdateRenderLevel();


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetCarryObject
//
//	PURPOSE:	Specifies a prop record to use as an object the player is carrying...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetCarryObject( PropsDB::HPROP hCarryProp )
{
	// [RP] 08/25/04 - This is first pass of this feature. In future iterations
	//		we will need to do something with the weapon. (disable, deselect/holster, hide...)

	if( !hCarryProp )
	{
		// Remove the object...
		if( m_hCarryObject )
		{
			g_pLTClient->RemoveObject( m_hCarryObject );
			m_hCarryObject = NULL;
		}

		return;
	}

	ObjectCreateStruct ocs;
	ocs.m_ObjectType = OT_MODEL;
	ocs.m_Flags = FLAG_VISIBLE;
    
	// Do we need to create the object or just change the model or material...
	if( !m_hCarryObject )
	{
		m_hCarryObject = g_pLTClient->CreateObject( &ocs );
		if( !m_hCarryObject )
			return;
	}

	ocs.SetFileName( g_pPropsDB->GetPropFilename( hCarryProp ));
	g_pPropsDB->CopyMaterialFilenames( hCarryProp, ocs.m_Materials[0], LTARRAYSIZE(ocs.m_Materials), LTARRAYSIZE(ocs.m_Materials[0]) );

	g_pCommonLT->SetObjectFilenames( m_hCarryObject, &ocs );

	HMODELANIM hCarryAnim = INVALID_MODEL_ANIM;
	if( g_pModelLT->GetAnimIndex( m_hCarryObject, "Carry", hCarryAnim ) == LT_OK )
	{
		g_pModelLT->SetCurAnim( m_hCarryObject, MAIN_TRACKER, hCarryAnim, true );
		g_pModelLT->SetLooping( m_hCarryObject, MAIN_TRACKER, false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::UpdateCarryObject
//
//	PURPOSE:	Update the position of the object being carried...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::UpdateCarryObject( )
{
	if( !IsCarryingObject( ))
		return;

	HOBJECT hPlayerBody = CPlayerBodyMgr::Instance( ).GetObject( );

	HMODELSOCKET hRightHand = INVALID_MODEL_SOCKET;
	if( g_pModelLT->GetSocket( hPlayerBody, "RightHand", hRightHand ) != LT_OK )
	{
		SetCarryObject( NULL );
		return;
	}

	LTTransform tRightHand;
	if( g_pModelLT->GetSocketTransform( hPlayerBody, hRightHand, tRightHand, true ) == LT_OK )
	{
		g_pLTClient->SetObjectTransform( m_hCarryObject, tRightHand );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::ReachedNodePosition
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::ReachedNodePosition( )
{
	m_bAtNodePosition = true;

	if( m_bFaceNodeForwardPitch || m_bFaceNodeForwardYaw )
	{
		// Interpolate rotation to nodes forward...
		LTRotation rNodeRot;
		g_pLTClient->GetObjectRotation( m_hPlayerNodeGoto, &rNodeRot );

		LTRotation rRot = LTRotation( rNodeRot.Forward( ), LTVector( 0.0f, 1.0f, 0.0f ));
		m_pPlayerCamera->RotateCamera( rRot, m_bFaceNodeForwardPitch, m_bFaceNodeForwardYaw, false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleNodeArrival
//
//	PURPOSE:	Notify server the player has arrived at its destination...
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::HandleNodeArrival( )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_ARRIVED_AT_NODE );
	cMsg.WriteObject( m_hPlayerNodeGoto );
	g_pLTClient->SendToServer( cMsg.Read( ), MESSAGE_GUARANTEED );

	m_hPlayerNodeGoto = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::SetStoryMode
//
//	PURPOSE:	turn story mode on/off
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::SetStoryMode(bool bOn, bool bCanSkip /*= true */)
{
	m_bStoryMode = bOn;
	m_bCanSkipStory = bCanSkip;
	if (bOn)
	{
		AimMgr::Instance().EndAim();
		m_bSemiAutoFire = false;

		// Remove any soundmixers.
		ClientDB& ClientDatabase = ClientDB::Instance(); 
		g_pGameClientShell->GetMixer( )->ProcessMixerByName( "EarRingLight", 0, true );
		g_pGameClientShell->GetMixer( )->ProcessMixerByName( "EarRingMed", 0, true );
		g_pGameClientShell->GetMixer( )->ProcessMixerByName( "EarRing", 0, true );

		// Clean up the earring sound.
		if( m_hRingSound )
		{
			g_pLTClient->SoundMgr()->KillSound(m_hRingSound);
			m_hRingSound = NULL;
		}

		if( m_pTurretFX )
			m_pTurretFX->Deactivate( );

//		m_pClientWeaponMgr->ChangeWeapon(g_pWeaponDB->GetUnarmedRecord(),NULL,-1,false);
		
		// Note that if the camera is in 1st person mode, the weapons
		// will be re-enabled by the camera.
		m_pClientWeaponMgr->DisableWeapons();

		m_pFlashLight->Enable(false,0.0f);

		AimMgr::Instance().SetCanAim(false);

		// Make sure slowmo is off.
		ExitSlowMo( true );

		m_HeartBeat.Stop();
		m_Breath.Stop();



	}
	else
	{
		m_pClientWeaponMgr->EnableWeapons();
//		m_pClientWeaponMgr->LastWeapon();
		m_pFlashLight->Enable(true,0.0f);
		AimMgr::Instance().SetCanAim(true);

	}

	g_pHUDMgr->UpdateRenderLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::CreateWeaponEffect
//
//	PURPOSE:	Create a ClientFX attached to our current weapon.
//
// ----------------------------------------------------------------------- //

void CPlayerMgr::CreateWeaponEffect(const char* pszFX, const char* pszSocket)
{
	static CParsedMsg::CToken s_cTok_WeaponSocketLeft("LEFT");
	static CParsedMsg::CToken s_cTok_WeaponSocketRight("RIGHT");

	CClientWeapon* pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (!pClientWeapon)
		return;

	HOBJECT hWeaponModel;
	HMODELSOCKET hSocket;

	if (s_cTok_WeaponSocketLeft == pszSocket)
	{
		hWeaponModel = pClientWeapon->GetLeftHandModel();
		hSocket = NULL;
	}
	else if (s_cTok_WeaponSocketRight == pszSocket)
	{
		hWeaponModel = pClientWeapon->GetRightHandModel();
		hSocket = NULL;
	}
	else
	{
		hWeaponModel = pClientWeapon->GetRightHandModel();
		if (!hWeaponModel ||
			(LT_OK != g_pModelLT->GetSocket(hWeaponModel, pszSocket, hSocket)))
		{
			hWeaponModel = pClientWeapon->GetLeftHandModel();
			if (!hWeaponModel ||
				(LT_OK != g_pModelLT->GetSocket(hWeaponModel, pszSocket, hSocket)))
			{
				return;
			}
		}
		LTASSERT(hWeaponModel,"UNEXPECTED: Weapon model not set!");
		LTASSERT(hSocket,"UNEXPECTED: Socket not set!");
	}

	CLIENTFX_CREATESTRUCT fxInit(pszFX, 0, hWeaponModel);
	fxInit.m_hSocket = hSocket;
	g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxInit, true);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::HandleSyncAction()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::HandleSyncAction( HRECORD hSyncAction, HOBJECT hObject )
{
	// Keep track of sync object for use later.  (usually for some sort of response like model key kick handling)
	m_hSyncObject = hObject;

	// Position correctly according to the way the animators animated the animations relative to each other...
	const char* pszSocket = g_pModelsDB->GetString(hSyncAction, "Socket");
	if (!LTStrEmpty(pszSocket))
	{
		HMODELSOCKET hSocket;
		if (g_pModelLT->GetSocket(hObject, pszSocket, hSocket) == LT_OK)
		{
			LTTransform tSocket;
			if (g_pModelLT->GetSocketTransform(hObject, hSocket, tSocket, true) == LT_OK)
			{
				g_pLTClient->SetObjectTransform(g_pMoveMgr->GetObject(), LTRigidTransform(tSocket.m_vPos, tSocket.m_rRot));
				GetPlayerCamera()->RotateCamera(tSocket.m_rRot, false, true, false);

				LTVector vAccel(0,0,0);
				g_pPhysicsLT->SetAcceleration(g_pMoveMgr->GetObject(), vAccel);
				g_pPhysicsLT->SetVelocity(g_pMoveMgr->GetObject(), vAccel);
			}
			else
			{
				char szError[256];
				LTSNPrintF( szError, LTARRAYSIZE( szError ), "Error getting SyncAction socket transform! Socket: '%s' Record: '%s'", pszSocket, g_pLTDatabase->GetRecordName(hSyncAction) );
				LTERROR( szError );
			}
		}
		else
		{
			char szError[256];
			LTSNPrintF( szError, LTARRAYSIZE( szError ), "SyncAction socket not found! ('%s') Record: '%s'", pszSocket, g_pLTDatabase->GetRecordName(hSyncAction) );
			LTERROR( szError );
		}
	}

	// Play a response animation (flailing about, etc)...
	
	const char* pszAction = g_pModelsDB->GetString(hSyncAction, "PlayerAction");
	if (!LTStrEmpty(pszAction))
	{
		EnumAnimProp eAction = AnimPropUtils::Enum(pszAction);
		if (eAction != kAP_Invalid)
		{
			if (eAction != kAP_None)
				CPlayerBodyMgr::Instance().SetAnimProp(kAPG_Action, eAction, CPlayerBodyMgr::kLocked);
		}
		else
		{
			char szError[256];
			LTSNPrintF( szError, LTARRAYSIZE( szError ), "Invalid SyncAction PlayerAction! ('%s') Record: '%s'", pszAction, g_pLTDatabase->GetRecordName(hSyncAction) );
			LTERROR( szError );
		}
	}

	const char* pszStimulus = g_pModelsDB->GetString(hSyncAction, "PlayerStimulus");
	if (!LTStrEmpty(pszStimulus))
	{
		CPlayerBodyMgr::Instance().HandleAnimationStimulus(pszStimulus);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::IsSwimmingAllowed()
//
//	PURPOSE:	Determine if the player is currently allowed to swim...
//
// --------------------------------------------------------------------------- //

bool CPlayerMgr::IsSwimmingAllowed( )
{
	if( !m_hCurContainerObject )
		return false;

	CVolumeBrushFX *pVolumeFX = dynamic_cast<CVolumeBrushFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_VOLUMEBRUSH_ID, m_hCurContainerObject ));
	if( pVolumeFX )
	{
		return pVolumeFX->IsSwimmingAllowed( );
	}

	return false;
}

#ifndef _FINAL

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::WritePlayerUpdateToSimulationLog()
//
//	PURPOSE:	Writes a player update to the simulation log.
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::WritePlayerUpdateToSimulationLog(uint16 nChangeFlags)
{
	CLTFileWrite& cFileWrite = g_pGameClientShell->GetSimulationLogFile();

	char dataBuffer[1024];

	// timestamp
	uint32 nTimeStamp = LTTimeUtils::GetTimeMS();
	cFileWrite.Write(&nTimeStamp, sizeof(uint32));

	// action code
	uint8 nAction = eSimulationActionPlayerUpdate;
	cFileWrite.Write(&nAction, sizeof(uint8));

	// update indicator
	uint8 nUpdateFlag = 1;
	cFileWrite.Write(&nUpdateFlag, sizeof(uint8));

	// change flags
	cFileWrite.Write(&nChangeFlags, sizeof(uint16)); 

	// camera info
	if (nChangeFlags & CLIENTUPDATE_CAMERAINFO)
	{
		m_cLastPlayerInfoCameraMsg->PeekData(&dataBuffer, m_cLastPlayerInfoCameraMsg->TellEnd());
		uint32 nSize = m_cLastPlayerInfoCameraMsg->TellEnd() + 8;
		cFileWrite.Write(&nSize, sizeof(uint32));
		cFileWrite.Write(&dataBuffer, nSize / 8); 
	}

	// animation info
	if (nChangeFlags & CLIENTUPDATE_ANIMATION)
	{
		m_cLastPlayerInfoAnimMsg->PeekData(&dataBuffer, m_cLastPlayerInfoAnimMsg->TellEnd());
		uint32 nSize = m_cLastPlayerInfoAnimMsg->TellEnd() + 8;
		cFileWrite.Write(&nSize, sizeof(uint32));
		cFileWrite.Write(&dataBuffer, nSize / 8);
	}

	// position and velocity
	LTVector vPos;
	g_pLTClient->GetObjectPos(m_pMoveMgr->GetObject(), &vPos);
	LTVector vVel;
	g_pPhysicsLT->GetVelocity(m_pMoveMgr->GetObject(), &vVel);

	cFileWrite.Write(&vPos.x, sizeof(float));
	cFileWrite.Write(&vPos.y, sizeof(float));
	cFileWrite.Write(&vPos.z, sizeof(float));

	cFileWrite.Write(&vVel.x, sizeof(float));
	cFileWrite.Write(&vVel.y, sizeof(float));
	cFileWrite.Write(&vVel.z, sizeof(float));

	// is on ground
	uint8 nIsOnGround = m_pMoveMgr->IsOnGround() ? 1 : 0;
	cFileWrite.Write(&nIsOnGround, sizeof(uint8));

	// surface type
	uint8 nSurfaceType = (uint8)m_pMoveMgr->GetStandingOnSurface();
	cFileWrite.Write(&nSurfaceType, sizeof(uint8));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::WriteNullUpdateToSimulationLog()
//
//	PURPOSE:	Writes a null update entry to the simulation log (for timing)
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::WriteNullUpdateToSimulationLog()
{
	CLTFileWrite& cFileWrite = g_pGameClientShell->GetSimulationLogFile();

	// timestamp
	uint32 nTimeStamp = LTTimeUtils::GetTimeMS();
	cFileWrite.Write(&nTimeStamp, sizeof(uint32));

	// action code
	uint8 nAction = eSimulationActionPlayerUpdate;
	cFileWrite.Write(&nAction, sizeof(uint8));

	// update indicator
	uint8 nUpdateFlag = 0;
	cFileWrite.Write(&nUpdateFlag, sizeof(uint8));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::WriteWeaponChangeToSimulationLog
//
//	PURPOSE:	Writes a weapon change entry to the simulation log
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::WriteWeaponChangeToSimulationLog(HWEAPON hWeapon, HAMMO hAmmo)
{
	CLTFileWrite& cFileWrite = g_pGameClientShell->GetSimulationLogFile();

	// timestamp
	uint32 nTimeStamp = LTTimeUtils::GetTimeMS();
	cFileWrite.Write(&nTimeStamp, sizeof(uint32));

	// action code
	uint8 nAction = eSimulationActionWeaponChange;
	cFileWrite.Write(&nAction, sizeof(uint8));

	// weapon
	const char* pName = g_pLTDatabase->GetCategoryName(g_pLTDatabase->GetRecordParent(hWeapon));
	uint32 nLength = LTStrLen(pName);
	cFileWrite.Write(&nLength, sizeof(uint32));
	cFileWrite.Write(pName, nLength);

	pName   = g_pLTDatabase->GetRecordName(hWeapon);
	nLength = LTStrLen(pName);
	cFileWrite.Write(&nLength, sizeof(uint32));
	cFileWrite.Write(pName, nLength);

	// ammo
	pName = g_pLTDatabase->GetCategoryName(g_pLTDatabase->GetRecordParent(hAmmo));
	nLength = LTStrLen(pName);
	cFileWrite.Write(&nLength, sizeof(uint32));
	cFileWrite.Write(pName, nLength);

	pName   = g_pLTDatabase->GetRecordName(hAmmo);
	nLength = LTStrLen(pName);
	cFileWrite.Write(&nLength, sizeof(uint32));
	cFileWrite.Write(pName, nLength);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::WritePlayerRespawnToSimulationLog
//
//	PURPOSE:	Writes a respawn entry to the simulation log
//
// --------------------------------------------------------------------------- //

void CPlayerMgr::WritePlayerRespawnToSimulationLog()
{
	CLTFileWrite& cFileWrite = g_pGameClientShell->GetSimulationLogFile();

	// timestamp
	uint32 nTimeStamp = LTTimeUtils::GetTimeMS();
	cFileWrite.Write(&nTimeStamp, sizeof(uint32));

	// action code
	uint8 nAction = eSimulationActionRespawn;
	cFileWrite.Write(&nAction, sizeof(uint8));
}

#endif // _FINAL


// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CHealthEffect::CHealthEffect()
:	m_eState				( eEffectState_Out ),
	m_hLoopSound			( NULL ),
	m_bStartedByHealth		( false ),
	m_bStartedByMsg			( false ),
	m_hSoundRec				( NULL )
{
}

CHealthEffect::~CHealthEffect() {}

const uint8	CHealthEffect::kPlayVolume = 127;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHealthEffect::Init
//
//	PURPOSE:	Init the effect
//
// ----------------------------------------------------------------------- //
bool CHealthEffect::Init(const char* szEffectName)
{
	char szTmp[64];
	ClientDB& ClientDatabase = ClientDB::Instance();
	HRECORD hShared = ClientDatabase.GetClientSharedRecord( );

	LTSNPrintF(szTmp,LTARRAYSIZE(szTmp),"%s.0.FadeTime",szEffectName);
	m_fTransitionTime = ClientDatabase.GetFloat(hShared, szTmp );

	LTSNPrintF(szTmp,LTARRAYSIZE(szTmp),"%s.0.HealthThreshold",szEffectName);
	m_nHealthThreshold = ClientDatabase.GetInt32(hShared, szTmp);

	LTSNPrintF(szTmp,LTARRAYSIZE(szTmp),"%s.0.Sound",szEffectName);
	m_hSoundRec = ClientDatabase.GetRecordLink( hShared, szTmp );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHealthEffect::InitTimer
//
//	PURPOSE:	Init the effect's timer
//
// ----------------------------------------------------------------------- //
void CHealthEffect::InitTimer()
{
	ObjectContextTimer objectContextTimer( g_pMoveMgr->GetServerObject( ));
	m_Timer.Stop();
	m_Timer.SetEngineTimer( objectContextTimer);
}

// ------------------------------------------------------------------------ //
//
//	ROUTINE:	CHealthEffect::Update()
//
//	PURPOSE:	Update our state.
//
// ----------------------------------------------------------------------- //

void CHealthEffect::Update( )
{

	//if we're hurt override our state and play the effect
	bool bHealthStart = (g_pPlayerStats->GetHealth() < m_nHealthThreshold && g_pPlayerStats->GetHealth() > 0);

	// Unless we are in story mode.
	if( g_pPlayerMgr->InStoryMode() )
		bHealthStart = false;
	
	// Unless we have have a cinematic camera playing.
	if( g_pPlayerMgr->GetPlayerCamera() && g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() != CPlayerCamera::kCM_FirstPerson )
		bHealthStart = false;

	if (bHealthStart != m_bStartedByHealth) 
	{
		if (bHealthStart)
		{
			Start(0.0f,false);
		}
		else
		{
			Stop(false);
		}
	}

	switch( m_eState )
	{
	case eEffectState_Out:
		{
			//we're not supposed to be playing the effect opt out early...	
			if (m_hLoopSound)
			{
				Play(false);
			}
			return;
		}
		break;
	case eEffectState_Entering:
		{
			// Check if we've made it all the way in.
			uint8 nCurVol = 0;
			float fTrans;
			g_pLTClient->SoundMgr()->GetSoundVolume(m_hLoopSound, nCurVol, fTrans);
			if( kPlayVolume == nCurVol )
			{
				m_eState = eEffectState_In;
			}
		}
		break;
	case eEffectState_In:
		{

			// Check if we have time left.
			if(!m_Timer.IsStarted() || m_Timer.GetTimeLeft( ) > m_fTransitionTime )
			{
				// Nothing to do.
				return;
			}
			else
			{
				Stop();
			}
		}
		break;
	case eEffectState_Exiting:
		{
			// Check if we have time left.
			uint8 nCurVol = 0;
			float fTrans;
			g_pLTClient->SoundMgr()->GetSoundVolume(m_hLoopSound, nCurVol, fTrans);
			if (0 == nCurVol)
			{
				// We're all the way out.  Clean up.
				Play(false);
				return;
			}
		}
		break;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHealthEffect::Save
//
//	PURPOSE:	Save the effect info
//
// --------------------------------------------------------------------------- //

void CHealthEffect::Save(ILTMessage_Write *pMsg)
{
	pMsg->Writebool(m_bStartedByMsg);
	pMsg->Writebool(m_bStartedByHealth);
	pMsg->Writedouble(m_Timer.GetTimeLeft());
	pMsg->Writeuint32( m_eState );


}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHealthEffect::Load
//
//	PURPOSE:	Load the effect info
//
// --------------------------------------------------------------------------- //

void CHealthEffect::Load(ILTMessage_Read *pMsg)
{
	Play(false);
	InitTimer();

	m_bStartedByMsg = pMsg->Readbool();
	m_bStartedByHealth = pMsg->Readbool();
	m_Timer.Start(pMsg->Readdouble());
	m_eState = static_cast<EffectState>(pMsg->Readuint32( ));

	if (m_eState != eEffectState_Out)
	{
		Play(true);
		switch(m_eState)
		{
		case eEffectState_In:
			g_pLTClient->SoundMgr()->SetSoundVolume(m_hLoopSound, kPlayVolume, 0.0f);
			break;
		case eEffectState_Entering:
			g_pLTClient->SoundMgr()->SetSoundVolume(m_hLoopSound, kPlayVolume, m_fTransitionTime);
			break;
		case eEffectState_Exiting:
			g_pLTClient->SoundMgr()->SetSoundVolume(m_hLoopSound, 0, m_fTransitionTime);
			break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHealthEffect::Start
//
//  PURPOSE:	Start the effect
//
// ----------------------------------------------------------------------- //

void CHealthEffect::Start(float fDuration /* = 0.0f */,bool bFromMsg /* = true */)
{

	bool bTempEffect = (fDuration > 0.0f);
	// Set when effect will wear off.
	if (bTempEffect)
	{
		m_Timer.Start( fDuration );		
	}
	else
	{
		if (bFromMsg)
		{
			m_Timer.Stop();
		}
	}

	if (bFromMsg) 
	{
		m_bStartedByMsg = true;
	}
	else
	{
		m_bStartedByHealth = true;
	}

	// Check if we're already there or entering.
	if( m_eState == eEffectState_In || m_eState == eEffectState_Entering)
		return;

	// call Play before setting the fade so that the sound
	// is guaranteed to be set up..
	if (!m_hLoopSound)
		Play(true);

	// We're entering the  state.
	m_eState = eEffectState_Entering;

	g_pLTClient->SoundMgr()->SetSoundVolume(m_hLoopSound, kPlayVolume, m_fTransitionTime);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHealthEffect::Stop
//
//	PURPOSE:	Stops .
//
// ----------------------------------------------------------------------- //

void CHealthEffect::Stop(bool bFromMsg /* = true */)
{
	if (m_eState == eEffectState_Exiting || m_eState == eEffectState_Out)
		return;

	if (bFromMsg)
	{
		m_bStartedByMsg = false;
	}
	else
	{
		m_bStartedByHealth = false;
	}
	
	if (!m_bStartedByHealth && !m_bStartedByMsg)
	{
		m_eState = eEffectState_Exiting;
		g_pLTClient->SoundMgr()->SetSoundVolume(m_hLoopSound, 0, m_fTransitionTime);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHealthEffect::Reset
//
//	PURPOSE:	Stops and resets
//
// ----------------------------------------------------------------------- //

void CHealthEffect::Reset()
{
	m_bStartedByMsg = false;
	m_bStartedByHealth = false;
	m_eState = eEffectState_Out;
	m_Timer.Stop();
	Play(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHealthEffect::PlaySound
//
//	PURPOSE:	Play/stop effect
//
// ----------------------------------------------------------------------- //

void CHealthEffect::Play(bool bOn)
{
	if (bOn)
	{
		uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP | PLAYSOUND_CLIENT;

		if (!m_hLoopSound)
		{
			m_hLoopSound  = g_pClientSoundMgr->PlayDBSoundLocal( m_hSoundRec, SOUNDPRIORITY_PLAYER_LOW, dwFlags );
			g_pLTClient->SoundMgr()->SetSoundVolume(m_hLoopSound, 0, 0.0f); // make sure 0 volume
		}
	}
	else
	{
		// kill the sound.
		if( m_hLoopSound )
		{
			g_pLTClient->SoundMgr()->KillSound( m_hLoopSound );
			m_hLoopSound = NULL;
		}

		// Stop the timer.
		m_Timer.Stop( );

		// Consider us out of the effect.
		m_eState = eEffectState_Out;
	}
}

