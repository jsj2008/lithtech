// ----------------------------------------------------------------------- //
//
// MODULE  : VehicleMgr.cpp
//
// PURPOSE : Client side vehicle mgr - Implementation
//
// CREATED : 6/12/00
//
// (c) 2000-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VehicleMgr.h"
#include "VarTrack.h"
#include "MsgIDs.h"
#include "CharacterFX.h"
#include "CMoveMgr.h"
#include "PlayerMgr.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientSoundMgr.h"
#include "ClientWeaponMgr.h"
#include "LTEulerAngles.h"
#include "JumpVolumeFX.h"
#include "PlayerLureFX.h"
#include "SurfaceFunctions.h"
#include "PlayerViewAttachmentMgr.h"
#include "resourceextensions.h"
#include "PlayerCamera.h"
#include "PlayerBodyMgr.h"

#define ROUNDFLOAT(f) ((int)((f) + ((f) > 0 ? 0.5 : -0.5)))

#define VEHICLE_SLIDE_TO_STOP_TIME	5.0f

#define VEHICLE_GEAR_0					0
#define VEHICLE_GEAR_1					1
#define VEHICLE_GEAR_2					2

extern CMoveMgr* g_pMoveMgr;

// [KLS 6/29/02] - Added more vehicle debugging info...
#define		VEHICLE_TRACK_GENERAL	1.0f
#define		VEHICLE_TRACK_ANIMATION	2.0f
#define		VEHICLE_TRACK_VERBOSE	3.0f


// Vehicle tweaking vars...

static VarTrack	s_vtVehicleInfoTrack;
static VarTrack	s_vtVehicleVel;
static VarTrack	s_vtVehicleAccel;
static VarTrack	s_vtVehicleAccelTime;
static VarTrack	s_vtVehicleMaxDecel;
static VarTrack	s_vtVehicleDecelTime;
static VarTrack	s_vtVehicleMaxBrake;
static VarTrack	s_vtVehicleBrakeTime;
static VarTrack	s_vtVehicleStoppedPercent;
static VarTrack	s_vtVehicleMaxSpeedPercent;
static VarTrack	s_vtVehicleAccelGearChange;
static VarTrack	s_vtVehicleMinTurnAccel;
static VarTrack	s_vtVehicleTurnRate;
static VarTrack	s_vtVehicleTurnRateScale;
static VarTrack	s_vtVehicleInitialTurnRate;
static VarTrack	s_vtVehicleSurfaceSpeedUpPitch;
static VarTrack s_vtVehicleSurfaceSlowDownPitch;
static VarTrack	s_vtVehicleStartMotionPitch;
static VarTrack s_vtVehicleStopMotionPitch;
static VarTrack s_vtVehicleBrakePitch;
static VarTrack s_vtVehicleDecelPitch;


// Bicycle...

VarTrack	g_vtBicycleInterBumpTimeRange;
VarTrack	g_vtBicycleIntraBumpTimeRange;
VarTrack	g_vtBicycleBumpHeightRange;
VarTrack	g_vtBicycleBobRate;
VarTrack	g_vtBicycleBobSwayAmplitude;
VarTrack	g_vtBicycleBobRollAmplitude;
VarTrack	g_vtBicycleModelOffset;


VarTrack	g_vtVehicleImpactDamage;
VarTrack	g_vtVehicleImpactDamageMin;
VarTrack	g_vtVehicleImpactDamageMax;
VarTrack	g_vtVehicleImpactDamageVelPercent;
VarTrack	g_vtVehicleImpactPushVelPercent;
VarTrack	g_vtVehicleImpactMinAngle;
VarTrack	g_vtVehicleImpactMaxAdjustAngle;
VarTrack	g_vtVehicleImpactPushAmount;
VarTrack	g_vtVehicleImpactPushRadius;
VarTrack	g_vtVehicleImpactPushDuration;
VarTrack	g_vtVehicleImpactFXOffsetForward;
VarTrack	g_vtVehicleImpactFXOffsetUp;
VarTrack	g_vtVehicleImpactThreshold;

VarTrack	g_vtVehicleImpactAIDamageVelPercent;
VarTrack	g_vtVehicleImpactAIMinDamage;
VarTrack	g_vtVehicleImpactAIMaxDamage;
VarTrack	g_vtVehicleImpactAIDoNormalCollision;

VarTrack	g_vtVehicleFallDamageMinHeight;
VarTrack	g_vtVehicleFallDamageMaxHeight;

VarTrack	g_vtVehicleMouseMinYawBuffer;
VarTrack	g_vtVehicleMouseMaxYawBuffer;

VarTrack	g_vtVehicleMaxHeadOffsetX;
VarTrack	g_vtVehicleMaxHeadOffsetY;
VarTrack	g_vtVehicleMaxHeadOffsetZ;

VarTrack	g_vtVehicleMaxHeadYaw;

VarTrack	g_vtVehicleSlideToStopTime;

VarTrack	g_vtVehicleCollisionInfo;

VarTrack	g_vtJumpVolumeWaveType;
VarTrack	g_vtJumpVolumeMaxDirectionPercent;	// Give 100% if at or above this max
VarTrack	g_vtJumpVolumeMinDirectionPercent;	// Give 0% if at or below this min
VarTrack	g_vtJumpVolumeInfo;

VarTrack	g_vtVehicleContourPlayerViewModel;
VarTrack	g_vtVehicleContourPoints;
VarTrack	g_vtVehicleContourExtraDimsX;
VarTrack	g_vtVehicleContourExtraDimsZ;
VarTrack	g_vtVehicleCamContourExtraDimsX;
VarTrack	g_vtVehicleCamContourExtraDimsZ;
VarTrack	g_vtVehicleContourMaxRotation;
VarTrack	g_vtVehicleCamContourMaxRotation;
VarTrack	g_vtVehicleContourRate;
VarTrack	g_vtVehicleContour;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelNodeControlFn
//
//	PURPOSE:	Node control callback
//
// ----------------------------------------------------------------------- //

void ModelNodeControlFn(const NodeControlData& Data, void *pUserData)
{
	CVehicleMgr* pMgr = (CVehicleMgr*)pUserData;
	LTASSERT(pMgr,"");
	if (!pMgr) return;

		//build up the current transform
	LTTransform tWS = (*Data.m_pModelTransform) * (*Data.m_pNodeTransform);
    
	//build up a matrix that they can modify
	LTMatrix mResult;
	tWS.ToMatrix(mResult);

	pMgr->HandleNodeControl(Data.m_hModel, Data.m_hNode, &mResult);

	//and now convert back to the node transform
	LTTransform tResult;
	tResult.FromMatrixWithScale(mResult);

	//move it into object space
	tResult = Data.m_pModelTransform->GetInverse() * tResult;

	//and store this back into the node transform
	Data.m_pNodeTransform->m_vPos = tResult.m_vPos;
	Data.m_pNodeTransform->m_rRot = tResult.m_rRot;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CVehicleMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CVehicleMgr::CVehicleMgr()
{
	m_hVehicleModel			= NULL;
	m_hVehicleAttachSocket	= INVALID_MODEL_SOCKET;

	m_hVehicleStartSnd  = NULL;
	m_hVehicleAccelSnd  = NULL;
	m_hVehicleDecelSnd  = NULL;
	m_hVehicleImpactSnd = NULL;
	m_hIdleSnd			= NULL;
	m_hBrakeSnd			= NULL;

	m_vVehicleOffset.Init();
	m_vHeadOffset.Init();
	m_fHeadYaw = 0.0f;

	m_bVehicleTurning       = false;
	m_nVehicleTurnDirection	= 0;

	m_bVehicleStopped    = true;
	m_bVehicleAtMaxSpeed = false;

	m_fHandlebarRoll = 0.0f;
	m_fContourRoll = 0.0f;
	m_bTurned = false;
	m_bHolsteredWeapon = false;

	m_vLastPlayerAngles.Init();
	m_vLastCameraAngles.Init();
	m_vLastVehiclePYR.Init();
	m_vVehiclePYR.Init();

	m_rtSavedEncodedTransform.Init( );
	m_bHasSavedEncodedTransform = false;

	InitWorldData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::~CVehicleMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CVehicleMgr::~CVehicleMgr()
{
	TermLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::Init
//
//	PURPOSE:	Initialize mgr
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::Init()
{
    // Vehicle tweaking...

	s_vtVehicleInfoTrack.Init( g_pLTClient, "VehicleInfo", NULL, 0.0f );
    s_vtVehicleVel.Init( g_pLTClient, "VehicleVel", NULL, 0.0f );
	s_vtVehicleAccel.Init( g_pLTClient, "VehicleAccel", NULL, 0.0f );
	s_vtVehicleAccelTime.Init( g_pLTClient, "VehicleAccelTime", NULL, 0.0f );
	s_vtVehicleMaxDecel.Init( g_pLTClient, "VehicleDecelMax", NULL, 0.0f );
    s_vtVehicleDecelTime.Init( g_pLTClient, "VehicleDecelTime", NULL, 0.0f );
	s_vtVehicleMaxBrake.Init( g_pLTClient, "VehicleBrakeMax", NULL, 0.0 );
    s_vtVehicleBrakeTime.Init( g_pLTClient, "VehicleBrakeTime", NULL, 0.0 );
    s_vtVehicleMinTurnAccel.Init( g_pLTClient, "VehicleMinTurnAccel", NULL, 0.0f );
	s_vtVehicleStoppedPercent.Init( g_pLTClient, "VehicleStoppedPercent", NULL, 0.0 );
	s_vtVehicleMaxSpeedPercent.Init( g_pLTClient, "VehicleMaxSpeedPercent", NULL, 0.0 );
    s_vtVehicleAccelGearChange.Init( g_pLTClient, "VehicleAccelGearChange", NULL, 1.0 );
	s_vtVehicleTurnRate.Init( g_pLTClient, "VehicleTurnRate", NULL, 0.0f );
	s_vtVehicleTurnRateScale.Init( g_pLTClient, "VehicleTurnRateScale", NULL, 1.0f );
	s_vtVehicleInitialTurnRate.Init( g_pLTClient, "VehicleInitialTurnRate", NULL, 0.0f );
	s_vtVehicleSurfaceSpeedUpPitch.Init( g_pLTClient, "VehicleSurfaceSpeedUpPitch", NULL, 0.0f );
	s_vtVehicleSurfaceSlowDownPitch.Init( g_pLTClient, "VehicleSurfaceSlowDownPitch", NULL, 0.0f);
	s_vtVehicleStartMotionPitch.Init( g_pLTClient, "VehicleStartMotionPitch", NULL, 0.0f );
	s_vtVehicleStopMotionPitch.Init( g_pLTClient, "VehicleStopMotionPitch", NULL, 0.0f);
	s_vtVehicleBrakePitch.Init( g_pLTClient, "VehicleBrakePitch", NULL, 0.0f );
	s_vtVehicleDecelPitch.Init( g_pLTClient, "VehicleDecelPitch", NULL, 0.0f );
	
	// Bicycle...

	g_vtBicycleInterBumpTimeRange.Init(g_pLTClient, "BicycleInterBumpTimeRange", "0.75 1.25", 0.0f );
	g_vtBicycleIntraBumpTimeRange.Init(g_pLTClient, "BicycleIntraBumpTimeRange", "0.05 0.15", 0.0f );
	g_vtBicycleBumpHeightRange.Init(g_pLTClient, "BicycleBumpHeightRange", "0 0", 0.0f );
	g_vtBicycleBobRate.Init(g_pLTClient, "BicycleBobRate", NULL, 2.5f );
	g_vtBicycleBobSwayAmplitude.Init(g_pLTClient, "BicycleBobSwayAmplitude", NULL, 2.0f );
	g_vtBicycleBobRollAmplitude.Init(g_pLTClient, "BicycleBobRollAmplitude", NULL, -0.25f );
	g_vtBicycleModelOffset.Init(g_pLTClient, "BicycleModelOffset", "0.0 -20.0 5.0", 0.0f );


	g_vtVehicleImpactDamage.Init(g_pLTClient, "VehicleImpactDamage", NULL, 0.0f);
	g_vtVehicleImpactDamageMin.Init(g_pLTClient, "VehicleImpactDamageMin", NULL, 5.0f);
	g_vtVehicleImpactDamageMax.Init(g_pLTClient, "VehicleImpactDamageMax", NULL, 100.0f);
	g_vtVehicleImpactDamageVelPercent.Init(g_pLTClient, "VehicleImpactDamageVelPerent", NULL, 0.9f);
	g_vtVehicleImpactPushVelPercent.Init(g_pLTClient, "VehicleImpactPushVelPerent", NULL, 0.0f);
	g_vtVehicleImpactMinAngle.Init(g_pLTClient, "VehicleImpactMinAngle", NULL, 20.0f);
	g_vtVehicleImpactMaxAdjustAngle.Init(g_pLTClient, "VehicleImpactMinAdjustAngle", NULL, 10.0f);
	g_vtVehicleImpactPushAmount.Init(g_pLTClient, "VehicleImpactPushAmount", NULL, 400.0f);
	g_vtVehicleImpactPushRadius.Init(g_pLTClient, "VehicleImpactPushRadius", NULL, 100.0f);
	g_vtVehicleImpactPushDuration.Init(g_pLTClient, "VehicleImpactPushDuration", NULL, 0.25f);

	g_vtVehicleImpactFXOffsetForward.Init(g_pLTClient, "VehicleImpactFXOffsetForward", NULL, 0.0f);
	g_vtVehicleImpactFXOffsetUp.Init(g_pLTClient, "VehicleImpactFXOffsetUp", NULL, 0.0f);

	g_vtVehicleImpactThreshold.Init(g_pLTClient, "VehicleImpactThreshold", NULL, 0.8f);

	g_vtVehicleImpactAIDamageVelPercent.Init(g_pLTClient, "VehicleImpactAIDamageVelPerent", NULL, 0.1f);
 	g_vtVehicleImpactAIMinDamage.Init(g_pLTClient, "VehicleImpactAIMinDamage", NULL, 1.0f);
 	g_vtVehicleImpactAIMaxDamage.Init(g_pLTClient, "VehicleImpactAIMaxDamage", NULL, 100.0f);
 	g_vtVehicleImpactAIDoNormalCollision.Init(g_pLTClient, "VehicleImpactAIDoNormalCollision", NULL, 1.0f);

	g_vtVehicleFallDamageMinHeight.Init(g_pLTClient, "VehicleFallDamageMinHeight", NULL, 400.0f);
 	g_vtVehicleFallDamageMaxHeight.Init(g_pLTClient, "VehicleFallDamageMaxHeight", NULL, 800.0f);

    
	g_vtVehicleMouseMinYawBuffer.Init(g_pLTClient, "VehicleMouseMinYawBuffer", NULL, 2.0f);
	g_vtVehicleMouseMaxYawBuffer.Init(g_pLTClient, "VehicleMouseMaxYawBuffer", NULL, 35.0f);

	g_vtVehicleMaxHeadOffsetX.Init(g_pLTClient, "VehicleHeadMaxOffsetX", NULL, 0.0f);
	g_vtVehicleMaxHeadOffsetY.Init(g_pLTClient, "VehicleHeadMaxOffsetY", NULL, 0.0f);
	g_vtVehicleMaxHeadOffsetZ.Init(g_pLTClient, "VehicleHeadMaxOffsetZ", NULL, 0.0f);
	g_vtVehicleMaxHeadYaw.Init(g_pLTClient, "VehicleHeadMaxYaw", NULL, -1.0f);

	g_vtVehicleSlideToStopTime.Init( g_pLTClient, "VehicleSlideToStopTime", NULL, 5.0f );

    g_vtVehicleCollisionInfo.Init(g_pLTClient, "VehicleCollisionInfo", NULL, 0.0f);

	g_vtJumpVolumeWaveType.Init( g_pLTClient, "JumpVolumeWaveType", NULL, 3.0f );
	g_vtJumpVolumeMaxDirectionPercent.Init( g_pLTClient, "JumpVolumeMaxDirectionPercent", NULL, 0.97f );
	g_vtJumpVolumeMinDirectionPercent.Init( g_pLTClient, "JumpVolumeMinDirectionPercent", NULL, 0.35f );
	g_vtJumpVolumeInfo.Init( g_pLTClient, "JumpVolumeInfo", NULL, 0.0f );

	g_vtVehicleContourPlayerViewModel.Init( g_pLTClient, "VehicleContourPlayerViewModel", NULL, 1.0f );
	g_vtVehicleContourPoints.Init( g_pLTClient, "VehicleContourPoints", NULL, 2.0f );
	g_vtVehicleContourExtraDimsX.Init( g_pLTClient, "VehicleContourExtraDimsX", NULL, 24.0f );
	g_vtVehicleContourExtraDimsZ.Init( g_pLTClient, "VehicleContourExtraDimsZ", NULL, 32.0f );
	g_vtVehicleCamContourExtraDimsX.Init( g_pLTClient, "VehicleCamCountourExtraDimsX", NULL, 48.0f );
	g_vtVehicleCamContourExtraDimsZ.Init( g_pLTClient, "VehicleCamCountourExtraDimsZ", NULL, 64.0f );
	g_vtVehicleContourMaxRotation.Init( g_pLTClient, "VehicleContourMaxRotation", NULL, 65.0f );
	g_vtVehicleCamContourMaxRotation.Init( g_pLTClient, "VehicleCamContourMaxRotation", NULL, 3.5f );
	g_vtVehicleContourRate.Init( g_pLTClient, "VehicleContourRate", NULL, 0.4f );
	g_vtVehicleContour.Init( g_pLTClient, "VehicleContour", NULL, 1.0f );

	m_VehicleModelOffsetMgr.Init();

	// Init world specific data members...

	InitWorldData();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::InitWorldData
//
//	PURPOSE:	Initialize our data members that are specific to the
//				current world.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::InitWorldData()
{
	m_dwControlFlags		= 0;
	m_dwLastControlFlags	= 0;

	m_fVehicleBaseMoveAccel = 0.0f;

	m_bVehicleStopped       = true;
    m_bVehicleAtMaxSpeed    = false;
    m_bVehicleTurning       = false;
	m_nVehicleTurnDirection	= 0;

	m_ePPhysicsModel		= PPM_NORMAL;

	m_nCurGear				= VEHICLE_GEAR_0;
	m_nLastGear				= VEHICLE_GEAR_0;

	m_hVehicleAttachSocket	= INVALID_MODEL_SOCKET;

	m_bSetLastAngles		= false;

	m_vVehiclePYR.Init();
	m_vLastVehiclePYR.Init();
	m_vLastPlayerAngles.Init();
	m_vLastCameraAngles.Init();

	m_eSurface				= ST_UNKNOWN;
	m_eLastSurface			= ST_UNKNOWN;

	m_fYawDiff				= 0.0f;
	m_fYawDelta				= 0.0f;
	m_eMouseTurnDirection	= TD_CENTER;
	m_bKeyboardTurning		= false;

	m_vHeadOffset.Init();
	m_fHandlebarRoll		= 0.0f;
	m_fHeadYaw				= 0.0f;

	m_nPlayerLureId			= 0;
	m_bResetLure			= false;
	m_bResetLureFromSave	= false;
	m_hCurJumpVolume		= NULL;

	m_bHaveJumpVolumeVel	= false;
	m_fJumpVolumeVel		= 0.0f;
	m_bTurned				= false;
	m_bHolsteredWeapon		= false;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnEnterWorld
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnEnterWorld()
{
	InitWorldData();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnExitWorld
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnExitWorld()
{
	// Turn off vehicle...

	SetNormalPhysicsModel();

	// Clean up any level specific shiznit...

	TermLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::TermLevel
//
//	PURPOSE:	Terminate any level specific stuff
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::TermLevel()
{
	if (m_hVehicleModel)
	{
        g_pLTClient->RemoveObject(m_hVehicleModel);
		m_hVehicleModel = NULL;
	}

	KillAllVehicleSounds();

	if (m_hVehicleImpactSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleImpactSnd);
        m_hVehicleImpactSnd = NULL;
	}

	m_hVehicleAttachSocket = INVALID_MODEL_SOCKET;

	// Remove any player-view attachments...

	if (g_pPVAttachmentMgr)
		g_pPVAttachmentMgr->RemovePVAttachments();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateControlFlags
//
//	PURPOSE:	Update the control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateControlFlags()
{
	// Clear control flags...

	m_dwLastControlFlags = m_dwControlFlags;
	m_dwControlFlags = 0;

	if (!g_pMoveMgr->GetObject() ||
		!g_pInterfaceMgr->AllowCameraMovement()) return;

	switch (m_ePPhysicsModel)
	{
		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdatePlayerLure
//
//	PURPOSE:	Update the playerlure.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdatePlayerLure()
{
	// Get the playerlurefx object.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		// It may not be on the client yet.  Keep polling for it.
		return;
	}

	// See if we need to reset the lure info.
	if( m_bResetLure )
	{
		if( m_bResetLureFromSave )
		{
			pPlayerLureFX->Reset( m_ResetLureTransform );
		}
		else
		{
			// Reset the lure.
			pPlayerLureFX->Reset( );
		}

		// Make sure the weapon is set correctly.  Need to keep calling this since
		// we poll for the playerlurefx object.
		EnableWeapon( pPlayerLureFX->GetAllowWeapon( ));

		// Don't need to reset anymore.
		m_bResetLure = false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateMotion
//
//	PURPOSE:	Update our motion
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateMotion()
{
	g_pMoveMgr->UpdateOnGround();

	m_eLastSurface = m_eSurface;
	m_eSurface	   = g_pMoveMgr->GetStandingOnSurface();

	// See if we moved over a new surface...

	if (m_eLastSurface != m_eSurface)
	{
		float fLastMult = GetSurfaceVelMult(m_eLastSurface);
		float fNewMult = GetSurfaceVelMult(m_eSurface);

		if (fLastMult < fNewMult)
		{
			// Speeding up...

			CameraDelta delta;
			delta.Pitch.fVar	= DEG2RAD( s_vtVehicleSurfaceSpeedUpPitch.GetFloat( ));
			delta.Pitch.fTime1	= 0.25f;
			delta.Pitch.fTime2	= 0.5;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;
			m_VehicleModelOffsetMgr.AddDelta(delta);
		}
		else if (fNewMult < fLastMult)
		{
			// Slowing down...

			CameraDelta delta;
			delta.Pitch.fVar	= DEG2RAD( s_vtVehicleSurfaceSlowDownPitch.GetFloat( ));
			delta.Pitch.fTime1	= 0.25f;
			delta.Pitch.fTime2	= 0.5;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;
			m_VehicleModelOffsetMgr.AddDelta(delta);
		}
	}

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
			UpdateLureMotion( );
			break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateLureMotion
//
//	PURPOSE:	Update our lure motion
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateLureMotion()
{
	HOBJECT hObj = g_pMoveMgr->GetObject();
	if( !hObj )
	{
		ASSERT( !"CVehicleMgr::UpdateLureMotion:  Invalid client object." );
		return;
	}

	// Don't allow any acceleration or velocity.
	g_pPhysicsLT->SetAcceleration( g_pMoveMgr->GetObject(), LTVector::GetIdentity( ));
	g_pPhysicsLT->SetVelocity( g_pMoveMgr->GetObject(), LTVector::GetIdentity( ));

	// BJL - Copy the rotation from the camera to the body if the 
	// current playerlure allows body rotation.

	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if ( pPlayerLureFX && pPlayerLureFX->GetAllowBodyRotation() )
	{
		LTRotation rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation();
		float fCamYaw = Eul_FromQuat( rRot, EulOrdYXZr ).x;
		rRot = LTRotation( 0.0f, fCamYaw, 0.0f );
		g_pLTClient->SetObjectRotation( g_pMoveMgr->GetObject(), rRot );
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateGear
//
//	PURPOSE:	Update the appropriate vehicle gear
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateGear()
{
 	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSound
//
//	PURPOSE:	Update our sounds
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSound()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		case PPM_NORMAL:
		default :
		break;
	}

	// See if the impact snd is done playing...

	if (m_hVehicleImpactSnd)
	{
        if (g_pLTClient->IsDone(m_hVehicleImpactSnd))
		{
            g_pLTClient->SoundMgr()->KillSound(m_hVehicleImpactSnd);
            m_hVehicleImpactSnd = NULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetAccelTime
//
//	PURPOSE:	Get the current vehicle's accerlation time
//
// ----------------------------------------------------------------------- //

float	CVehicleMgr::GetAccelTime() const
{
	float fTime = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fTime;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMinTurnAccel
//
//	PURPOSE:	Get the current vehicle's minimum turn acceleration
//
// ----------------------------------------------------------------------- //

float	CVehicleMgr::GetMinTurnAccel() const
{
	float fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxDecel
//
//	PURPOSE:	Get the current vehicle's maximum deceleration
//
// ----------------------------------------------------------------------- //

float	CVehicleMgr::GetMaxDecel() const
{
	float fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetDecelTime
//
//	PURPOSE:	Get the current vehicle's deceleration time
//
// ----------------------------------------------------------------------- //

float	CVehicleMgr::GetDecelTime() const
{
	float fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxBrake
//
//	PURPOSE:	Get the current vehicle's max break value
//
// ----------------------------------------------------------------------- //

float	CVehicleMgr::GetMaxBrake() const
{
	float fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetBrakeTime
//
//	PURPOSE:	Get the current vehicle's break time
//
// ----------------------------------------------------------------------- //

float	CVehicleMgr::GetBrakeTime() const
{
	float fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateFriction
//
//	PURPOSE:	Update fricton
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateFriction()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::IsVehiclePhysics
//
//	PURPOSE:	Are we doing vehicle physics
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::IsVehiclePhysics()
{
	return IsVehicleModel(m_ePPhysicsModel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CanDismount
//
//	PURPOSE:	Are we allowed do get off the vehicle
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::CanDismount()
{
	if( !IsVehiclePhysics() )
		return false;

	return (m_ePPhysicsModel != PPM_LURE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::EnableWeapon
//
//	PURPOSE:	Enable/disables the weapon.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::EnableWeapon( bool bEnable, bool bToggleHolster /*= true*/ )
{
#ifdef PROJECT_DARK

	if (m_bHolsteredWeapon)
	{
		g_pClientWeaponMgr->LastWeapon();
		m_bHolsteredWeapon = false;
	}
	else if (!bEnable)	// AllowWeapon == false
	{
		CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		if (pWeapon)
		{
			// Temporarily switch to the hands if needed...
			HWEAPON hDefault = g_pClientWeaponMgr->GetDefaultWeapon();
			if (pWeapon->GetWeaponRecord() != hDefault)
			{
				pWeapon->ClearFiring();		// We don't want to accidently fire off a round or two coming out of a PlayerLure...
				g_pClientWeaponMgr->ChangeWeapon(hDefault);
				m_bHolsteredWeapon = true;
			}
		}
	}

#else

	// Get our weapon.
	CClientWeaponMgr *pClientWeaponMgr = g_pPlayerMgr->GetClientWeaponMgr();
	CClientWeapon* pClientWeapon = pClientWeaponMgr->GetCurrentClientWeapon();
	if( !pClientWeapon )
		return;

	// Enable the weapon...
	if( bEnable )
	{
		// We must enable the weapon before holstering it...
		
		if (!pClientWeaponMgr->WeaponsEnabled())
		{
			pClientWeaponMgr->EnableWeapons();
		}
		
		
		// Check if we have a holstered weapon.
		if( m_bHolsteredWeapon && bToggleHolster )
		{
			g_pClientWeaponMgr->ToggleHolster( false );
			m_bHolsteredWeapon = false;
		}
	}
	// Disable the weapon.
	else
	{
		if( !m_bHolsteredWeapon && bToggleHolster )
		{
			// Can't holster melee weapon. (why i don't know BEP).
			if (!pClientWeapon->IsMeleeWeapon())
			{
				g_pClientWeaponMgr->ToggleHolster( false );
				m_bHolsteredWeapon = true;
			}
		}

		if (pClientWeaponMgr->WeaponsEnabled())
		{
			pClientWeaponMgr->DisableWeapons();
		}
	}

#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::PreSetPhysicsModel
//
//	PURPOSE:	Do any clean up from old model
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::PreSetPhysicsModel(PlayerPhysicsModel eModel)
{
	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;
	m_fContourRoll = 0.0f;

	HOBJECT hObj = g_pMoveMgr->GetObject();

	bool bRet = true;

	bool bTouchNotify;

	if (IsVehicleModel(eModel))
	{
		switch (eModel)
		{
			case PPM_LURE :
			case PPM_NORMAL :
			default :
			break;
		}

		// Needed for collision physics...

		bTouchNotify = true;
		g_pPhysicsLT->SetForceIgnoreLimit(hObj, 0.0f);
	}
	else
	{
		bTouchNotify = false;
		g_pPhysicsLT->SetForceIgnoreLimit(hObj, 1000.0f);

		// Turn off the vehicle...

		KillAllVehicleSounds();

 		switch (m_ePPhysicsModel)
		{
			case PPM_LURE :
			{
				// Enable the weapon.
				EnableWeapon( true );
			}
			break;

			default :
			case PPM_NORMAL :
			break;
		}

	}

	g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, bTouchNotify ? FLAG_TOUCH_NOTIFY : 0, FLAG_TOUCH_NOTIFY);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetPhysicsModel
//
//	PURPOSE:	Set the physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPhysicsModel( PlayerPhysicsModel eModel, bool bDoPreSet /*= true*/ )
{
	if (m_ePPhysicsModel == eModel) return;

	// Do clean up...

	if (bDoPreSet && !PreSetPhysicsModel(eModel)) return;

	m_ePPhysicsModel = eModel;

 	switch (eModel)
	{
		case PPM_LURE :
		{
			SetPlayerLurePhysicsModel( );
		}
		break;

		default :
		case PPM_NORMAL :
		{
			SetNormalPhysicsModel();

			if( s_vtVehicleInfoTrack.GetFloat() )
			{
				g_pLTClient->CPrint("Normal Physics Mode: ON");
			}
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::IsCurVehicleAni
//
//	PURPOSE:	See if this is the current vehicle animation
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::IsCurVehicleAni(const char* pAniName, bool & bIsDone)
{
	if (!m_hVehicleModel || !pAniName) return false;

	// See if the current ani is done...

	uint32 dwState = g_pLTClient->GetModelPlaybackState(m_hVehicleModel);
 	bIsDone = (dwState & MS_PLAYDONE);

	uint32 dwTestAni = g_pLTClient->GetAnimIndex(m_hVehicleModel, pAniName);
	uint32 dwAni	 = g_pLTClient->GetModelAnimation(m_hVehicleModel);

	return (dwTestAni == dwAni);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetPlayerLurePhysicsModel
//
//	PURPOSE:	Set the playerlure physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPlayerLurePhysicsModel()
{
	// Get the playerlurefx for info.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		return;
	}

	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

	// Make sure our lure is updated.
	if( m_bResetLure )
	{
		UpdatePlayerLure();
	}

	g_pPlayerMgr->GetPlayerCamera()->ResetCameraRotations();
	
	if( CPlayerBodyMgr::Instance( ).IsEnabled( ))
		g_pPlayerMgr->GetPlayerCamera( )->AttachToTarget( true );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetNormalPhysicsModel
//
//	PURPOSE:	Set the normal physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetNormalPhysicsModel()
{
	if (m_hVehicleModel)
	{
        g_pLTClient->RemoveObject(m_hVehicleModel);
		m_hVehicleModel = NULL;
	}

	KillAllVehicleSounds();

	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

	// Remove player-view attachments

	g_pPVAttachmentMgr->RemovePVAttachments();

	if( CPlayerBodyMgr::Instance( ).IsEnabled( ))
		g_pPlayerMgr->GetPlayerCamera( )->AttachToTarget( true );

	// Clear any accumulated movement encoding from being attached to a PlayerLure...
	g_pModelLT->ResetMovementEncodingTransform( g_pMoveMgr->GetObject( ));

	// Remove any movement encoded transform from a saved game...
	m_bHasSavedEncodedTransform = false;
	m_rtSavedEncodedTransform.Init( );

	// Reset the move objects rotation if the PlayerBody is not being scripted...
	if( !CPlayerBodyMgr::Instance( ).IsPlayingSpecial( ))
	{
		// Get the current camera rotation.
		LTRotation rRot = g_pPlayerMgr->GetPlayerCamera( )->GetCameraRotation( );

		// Set our current rotation.  Only take the yaw for the mover object.
		EulerAngles EA = Eul_FromQuat( rRot, EulOrdYXZr );
		g_pLTClient->SetObjectRotation( g_pMoveMgr->GetObject( ), LTRotation( 0.0f, EA.x, 0.0f ));
	}
	
	// Make sure our lure is updated.
	if( m_bResetLure )
	{
		UpdatePlayerLure();
	}

	g_pPlayerMgr->GetPlayerCamera()->ResetCameraRotations();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CreateVehicleModel
//
//	PURPOSE:	Create the player-view vehicle model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CreateVehicleModel()
{
	ObjectCreateStruct ocs;
	
	switch (m_ePPhysicsModel)
	{
		case PPM_LURE:
		{
			// NEED TO MOVE THESE INTO A BUTE FILE!!!

			ocs.m_ObjectType	= OT_MODEL;
			ocs.m_Flags			= FLAG_VISIBLE;

			ocs.SetFileName("chars\\Models\\armtricycle." RESEXT_MODEL_PACKED);
			ocs.SetMaterial(0, "chars\\skins\\armstrong." RESEXT_MATERIAL);
			ocs.SetMaterial(1, "chars\\skins\\armstronghead." RESEXT_MATERIAL);
		}
		break;

		default :
			return;
		break;
	}

	// Remove any old vehicle model.
	if( m_hVehicleModel )
	{
		g_pLTClient->RemoveObject( m_hVehicleModel );
		m_hVehicleModel = NULL;
	}

    m_hVehicleModel = g_pLTClient->CreateObject(&ocs);
	if (!m_hVehicleModel) return;
	
	// Set up node control...
	// Do this AFTER we set the filename of the model since the node control is now per node.
	// First remove any we may have previously set otherwise the matrix will be applied multiple times.

	g_pModelLT->RemoveNodeControlFn( m_hVehicleModel, ModelNodeControlFn, this );
	g_pModelLT->AddNodeControlFn(m_hVehicleModel, ModelNodeControlFn, this);
    

	// Map all the nodes in our skeleton in the bute file to the nodes in the actual RESEXT_MODEL_PCAKED model file

	m_VehicleNode1.hNode = NULL;
	m_VehicleNode2.hNode = NULL;
	m_VehicleNode3.hNode = NULL;

	// Do post creation stuff.
	switch( m_ePPhysicsModel )
	{
		case PPM_LURE:
		{
			g_pLTClient->SetModelLooping( m_hVehicleModel, false );
		}
		break;

		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateModels
//
//	PURPOSE:	Update the player-view models
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateModels()
{
	if (m_ePPhysicsModel != PPM_NORMAL)
	{

	}
	}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::ShowVehicleModel
//
//	PURPOSE:	Show/Hide the vehicle model (and attachments)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::ShowVehicleModel(bool bShow)
{
	if (!m_hVehicleModel) return;

	if (bShow)
	{
		g_pCommonLT->SetObjectFlags(m_hVehicleModel, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	}
	else
	{
		g_pCommonLT->SetObjectFlags(m_hVehicleModel, OFT_Flags, 0, FLAG_VISIBLE);
	}

	g_pPVAttachmentMgr->ShowPVAttachments( !!(bShow) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxVelMag
//
//	PURPOSE:	Get the max velocity for our current mode
//
// ----------------------------------------------------------------------- //

float CVehicleMgr::GetMaxVelMag(float fMult) const
{
    float fMaxVel = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	fMaxVel *= fMult;

	float fMoveMulti = g_pMoveMgr->GetMoveMultiplier();
	fMoveMulti = (fMoveMulti > 1.0f ? 1.0f : fMoveMulti);

	fMaxVel *= (g_pMoveMgr->IsBodyInLiquid() ? fMoveMulti/2.0f : fMoveMulti);

	return fMaxVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxAccelMag
//
//	PURPOSE:	Get the max acceleration for our current mode
//
// ----------------------------------------------------------------------- //

float CVehicleMgr::GetMaxAccelMag(float fMult) const
{
    float fMaxVel = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	fMaxVel *= fMult;

	return fMaxVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetSurfaceVelMult
//
//	PURPOSE:	Get the velocity multiplier for the current surface
//
// ----------------------------------------------------------------------- //

float CVehicleMgr::GetSurfaceVelMult(SurfaceType eSurf) const
{
	float fVelMultiplier = 1.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	return fVelMultiplier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetIdleSnd
//
//	PURPOSE:	Get the appropriate idle sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetIdleSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetAccelSnd
//
//	PURPOSE:	Get the appropriate acceleration sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetAccelSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
			return "Acceleration";	// -- DB Record name
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetDecelSnd
//
//	PURPOSE:	Get the appropriate deceleration sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetDecelSnd()
{
 	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetBrakeSnd
//
//	PURPOSE:	Get the appropriate brake sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetBrakeSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_LURE :
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetImpactSnd
//
//	PURPOSE:	Get the appropriate impact sound...
//
// ----------------------------------------------------------------------- //

const char* CVehicleMgr::GetImpactSnd(float /*fCurVelocityPercent*/, HSURFACE hSurface)
{
	if (!hSurface) return NULL;

	if (g_pSurfaceDB->GetSurfaceType(hSurface) == ST_FLESH)
	{
		return "VehicleHit";	// DB Record
	}
	else
	{
		switch (m_ePPhysicsModel)
		{
			case PPM_LURE :
			break;

			case PPM_NORMAL:
			default :
			break;
		}
	}

	return NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::KillAllVehicleSounds
//
//	PURPOSE:	Kill all the vehicle sounds
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::KillAllVehicleSounds(HLTSOUND hException)
{
	if (m_hVehicleStartSnd && hException != m_hVehicleStartSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleStartSnd);
        m_hVehicleStartSnd = NULL;
	}

	if (m_hVehicleAccelSnd && hException != m_hVehicleAccelSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleAccelSnd);
        m_hVehicleAccelSnd = NULL;
	}

	if (m_hVehicleDecelSnd && hException != m_hVehicleDecelSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hVehicleDecelSnd);
        m_hVehicleDecelSnd = NULL;
	}

	if (m_hIdleSnd && hException != m_hIdleSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hIdleSnd);
        m_hIdleSnd = NULL;
	}

	if (m_hBrakeSnd && hException != m_hBrakeSnd)
	{
        g_pLTClient->SoundMgr()->KillSound(m_hBrakeSnd);
        m_hBrakeSnd = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnTouchNotify
//
//	PURPOSE:	Handle our object touching something...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnTouchNotify(CollisionInfo *pInfo, float /*forceMag*/)
{
	if (!pInfo->m_hObject) return;

	// Cancel out the engine's velocity adjustment on touch.  We'll handle that, thank you.
	LTVector vOldVel = g_pMoveMgr->GetVelocity();
	g_pMoveMgr->SetVelocity(vOldVel - pInfo->m_vStopVel);

	// Filter out the player object
	if (pInfo->m_hObject == g_pLTClient->GetClientObject()) return;

	// Test for a JumpVolume...
	
	uint16 nCode;
	if( g_pLTClient->GetContainerCode( pInfo->m_hObject, &nCode) )
	{
		if( nCode == CC_JUMP_VOLUME )
		{
			if( HandleJumpVolumeTouch( pInfo->m_hObject ))
			{
				return;			
			}
		}
	}
	
	// All other objects must be solid for us to collide with them...
    
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_Flags, dwFlags);
	if (!(dwFlags & FLAG_SOLID)) 
	{
		return;
	}

	LTPlane cPolyPlane;
	if (pInfo->m_hPoly != INVALID_HPOLY)
		g_pLTClient->Common()->GetPolyPlane(pInfo->m_hPoly, &cPolyPlane);
	else
	{
		cPolyPlane.m_Normal.Init();
	}

	LTVector vNormal = pInfo->m_Plane.m_Normal;

	// If flat enough, we can drive on it...

	if ((vNormal.y > 0.76f) || (cPolyPlane.m_Normal.y > 0.76f)) return;

	// Check initial stuff so we handle object collisions properly

	SurfaceType eType = ::GetSurfaceType(*pInfo);
	HSURFACE hSurface = g_pSurfaceDB->GetSurface(eType);

	bool bIsWorld = IsMainWorld(pInfo->m_hObject);

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
        g_pLTClient->CPrint("***********************************");
        g_pLTClient->CPrint("MoveMgr Object Touch:");
        g_pLTClient->CPrint("  Stop Vel: %.2f, %.2f, %.2f", VEC_EXPAND(pInfo->m_vStopVel));
        g_pLTClient->CPrint("  (%s)", bIsWorld ? "WORLD" : "OBJECT");
        g_pLTClient->CPrint("  Normal: %.2f, %.2f, %.2f", VEC_EXPAND(vNormal));
        g_pLTClient->CPrint("  Object Type: %d", GetObjectType(pInfo->m_hObject));
		g_pLTClient->CPrint("  Surface Type: %s", hSurface ? g_pLTDatabase->GetRecordName(hSurface) : "No surface");
	}

	vNormal.y = 0.0f; // Don't care about this anymore...


	// Determine the current percentage of our max velocity we are
	// currently moving...

	LTVector vVel = g_pMoveMgr->GetVelocity();
	vVel.y = 0.0f;

	float fVelPercent = GetVehicleMovementPercent();

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
		g_pLTClient->CPrint("Calculated Velocity Percent: %.2f", fVelPercent);
	}


	// Hit objects

	if (!bIsWorld && pInfo->m_hObject)
	{
		uint32 dwUserFlags = 0;
		g_pCommonLT->GetObjectFlags(pInfo->m_hObject, OFT_User, dwUserFlags);

		if (dwUserFlags & USRFLG_CHARACTER)
		{
			if (fVelPercent > g_vtVehicleImpactAIDamageVelPercent.GetFloat())
			{
				// Make sure we're pointing at the correct surface type...

				hSurface = g_pSurfaceDB->GetSurface(ST_FLESH);

				if ( HandleCharacterTouch( pInfo->m_hObject ) )
				{
					return;
				}
			}
		}
	}

	m_bWasTouched = true;
	m_cLastTouchInfo = *pInfo;
}
	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleLightPosRot
//
//	PURPOSE:	Get the vehicle light pos/rot
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::GetVehicleLightPosRot(LTVector & vPos, LTRotation & rRot)
{
	vPos.Init();
	rRot.Init();

	if (!m_hVehicleModel) return;

	// Get the vehicle light position/rotation

	HMODELSOCKET hSocket;

	if (g_pModelLT->GetSocket(m_hVehicleModel, "Light", hSocket) == LT_OK)
	{
		LTTransform transform;
        if (g_pModelLT->GetSocketTransform(m_hVehicleModel, hSocket, transform, false) == LT_OK)
		{
			vPos = transform.m_vPos;
			rRot = transform.m_rRot;

			HOBJECT hCamera = g_pPlayerMgr->GetPlayerCamera()->GetCamera();
			if (!hCamera) return;

			LTVector vCamPos;
			g_pLTClient->GetObjectPos(hCamera, &vCamPos);
			g_pLTClient->GetObjectRotation(hCamera, &rRot);

			// Rotate down a bit...
			rRot.Rotate(rRot.Right(), DEG2RAD(10.0f));

			vPos += vCamPos;
		}
	}
	else
	{
		HOBJECT hCamera = g_pPlayerMgr->GetPlayerCamera()->GetCamera();
		if (!hCamera) return;

		g_pLTClient->GetObjectRotation(hCamera, &rRot);
		g_pLTClient->GetObjectPos(hCamera, &vPos);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::HandleNodeControl
//
//	PURPOSE:	Handle node control
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::HandleNodeControl(HOBJECT /*hObj*/, HMODELNODE hNode, LTMatrix *pGlobalMat)
{
	if (m_VehicleNode1.hNode && hNode == m_VehicleNode1.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode1.matTransform;
	}
	else if (m_VehicleNode2.hNode && hNode == m_VehicleNode2.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode2.matTransform;
	}
	else if (m_VehicleNode3.hNode && hNode == m_VehicleNode3.hNode)
	{
		*pGlobalMat = *pGlobalMat * m_VehicleNode3.matTransform;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleHandleBars
//
//	PURPOSE:	Update the vehicle handlebars
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleHandleBars()
{
	if (!m_hVehicleModel) return;

	// Reset all our nodes matrices

	m_VehicleNode1.matTransform.Identity();
	m_VehicleNode2.matTransform.Identity();
	m_VehicleNode3.matTransform.Identity();

	// Do the rotation on the handle bar node
	//
    LTRotation rRot;
    rRot.Init();
    LTVector vAxis(0.0f, 1.0f, 0.0f);

	// Negate the roll for some reason
	rRot.Rotate(vAxis, -m_fHandlebarRoll);

	// Create a rotation matrix and apply it to the current offset matrix

	LTMatrix m1;
	rRot.ConvertToMatrix(m1);
	m_VehicleNode1.matTransform = m_VehicleNode1.matTransform * m1;


	// Do the rotation on the speedometer node...
	
    float fMaxRot = MATH_DEGREES_TO_RADIANS(270.0f);

	// Calculate the speed dial rotation...

	LTVector vVel = g_pMoveMgr->GetVelocity();

	float fMaxVel = GetMaxVelMag();
	float fCurVel = vVel.Mag() > fMaxVel ? fMaxVel : vVel.Mag();

	float fPercent = 1.0f - ((fMaxVel - fCurVel) / fMaxVel);

	float fCurRot = fPercent*fMaxRot;
	
	rRot.Init();
	vAxis.Init(0.0f, 0.0f, 1.0f);
	rRot.Rotate(vAxis, -fCurRot);

	// Create a rotation matrix and apply it to the current offset matrix

	rRot.ConvertToMatrix(m1);
	m_VehicleNode2.matTransform = m_VehicleNode2.matTransform * m1;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::AdjustRoll
//
//	PURPOSE:	Adjust the camera roll if necessary
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::AdjustCameraRoll(float & fRoll)
{
	// Only adjust if we are on a vehicle...

	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		// Don't adjust camera when selecting/deselecting the vehicle...

		bool bIsDone = false;
		if (IsCurVehicleAni("Deselect", bIsDone) || IsCurVehicleAni("Select", bIsDone))
		{
			return;
		}

		float fMaxCant = 0.0f;//DEG2RAD(g_vtMaxVehicleHeadCant.GetFloat());

		float fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());
		float fYawDiff = (float)fabs(m_fYawDiff);
		float fPercent = fYawDiff / fMaxOffset;

		if (m_fYawDiff > 0.0f)
		{
			fPercent = -fPercent;
		}

		// Save off roll value for updating handlebars...

		m_fHandlebarRoll = fMaxCant * fPercent;


		// Update head can't only when moving...

		if (!m_bVehicleStopped)
		{
			fRoll = m_fHandlebarRoll;

			// Move head left or right depending on amount we are turning...

			m_vHeadOffset.x = g_vtVehicleMaxHeadOffsetX.GetFloat() * fPercent;
			m_vHeadOffset.y = g_vtVehicleMaxHeadOffsetY.GetFloat() * fPercent;
			m_vHeadOffset.z = g_vtVehicleMaxHeadOffsetZ.GetFloat() * fPercent;
			m_fHeadYaw = DEG2RAD(g_vtVehicleMaxHeadYaw.GetFloat() * fPercent);
		}
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveLocalSolidObject
//
//	PURPOSE:	Handle the moving of the object if we need to.
//
// ----------------------------------------------------------------------- //
bool CVehicleMgr::MoveLocalSolidObject( )
{
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		// These guys don't handle movement.
		case PPM_NORMAL :
			return false;
			break;

		case PPM_LURE:
			return MoveToLure( );
			break;

		default:
			ASSERT( !"CVehicleMgr::MoveLocalSolidObject: Invalid physics model." );
			return false;
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveToLure
//
//	PURPOSE:	Handle the moving of the object if we need to.
//
// ----------------------------------------------------------------------- //
bool CVehicleMgr::MoveToLure( )
{
	// Get the playerlurefx.
	PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
	if( !pPlayerLureFX )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing PlayerLureFX." );
		return false;
	}

	// Get the server lure object.
	HOBJECT hServerObj = pPlayerLureFX->GetServerObj( );
	if( !hServerObj )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing server-side player object." );
		return false;
	}

	// Get the client player object.
	HOBJECT hObj = g_pMoveMgr->GetObject( );
	if( !hObj )
	{
		ASSERT( !"CVehicleMgr::MoveToLure:  Missing client-side player object." );
		return false;
	}

	// Our resulting pos and rotation.
	LTVector vPos;
	LTRotation rRot;

	// Apply offset if told to.
	if( pPlayerLureFX->GetRetainOffsets( ))
	{
		// Get the playerlure's transform.
		LTRigidTransform playerLureTransform;
		g_pLTClient->GetObjectPos( hServerObj, &playerLureTransform.m_vPos );
		g_pLTClient->GetObjectRotation( hServerObj, &playerLureTransform.m_rRot );

		// Get the offset transform.
		LTRigidTransform offsetTransform = pPlayerLureFX->GetOffsetTransform( );

		// Find the offset position and rotation.
		LTRigidTransform playerTransform = playerLureTransform * offsetTransform;

		// Stuff the result.
		vPos = playerTransform.m_vPos;
		rRot = playerTransform.m_rRot;
		rRot.Normalize( );
	}
	// Not retaining offsets.
	else
	{
		// Get the playerlure's position.
		g_pLTClient->GetObjectPos( hServerObj, &vPos );

		// Get the playerlure's rotation.
		g_pLTClient->GetObjectRotation( hServerObj, &rRot );
	}

	if( CPlayerBodyMgr::Instance( ).IsMovementEncoded( ))
	{
		// Get the current movement encoding transformation...
		LTRigidTransform tMovementEncoding;
		g_pModelLT->GetMovementEncodingTransform( hObj, tMovementEncoding );

		// Get the current object transformation...
		LTRigidTransform tObject;
		g_pLTClient->GetObjectTransform( hServerObj, &tObject );

		// The movement encoding transform does not get reset while on a Lure,
		// so any saved transform must always be factored in.
		if( m_bHasSavedEncodedTransform )
		{
			tObject *= m_rtSavedEncodedTransform;
		}

		// Caclulate the world space movement encoding transform...
		LTRigidTransform tMovementWS = tObject * tMovementEncoding;
		vPos = tMovementWS.m_vPos;
	}

	g_pPhysicsLT->MoveObject( hObj, vPos, MOVEOBJECT_TELEPORT );

	// BJL - Some playerlure do not want to stomp the body rotation.
	// Only overwrite the rotation with the lures rotation if 
	// AllowBodyRotation is disabled.

	if ( !pPlayerLureFX->GetAllowBodyRotation() )
	{
		g_pLTClient->SetObjectRotation( hObj, rRot );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::GetVehicleMovementPercent
//
//  PURPOSE:	Calculate the percentage from our max velocity...
//
// ----------------------------------------------------------------------- //

float CVehicleMgr::GetVehicleMovementPercent() const
{
	LTVector vVel = g_pMoveMgr->GetVelocity();
	vVel.y = 0.0f;

	float fMaxVel = GetMaxVelMag();
	float fCurVel = vVel.Mag() > fMaxVel ? fMaxVel : vVel.Mag();

	if (fMaxVel > 0.0f)
	{
		return 1.0f - ((fMaxVel - fCurVel) / fMaxVel);
	}

	return 0.0f;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::HandleJumpVolumeTouch
//
//  PURPOSE:	Handle hitting a JumpVolume...
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::HandleJumpVolumeTouch( HOBJECT hJumpVolume )
{
	if( !hJumpVolume ) return false;

	// We only want one JumpVolume to effect us at a time...
	
	if( m_hCurJumpVolume )
		return true;
	
	// Grab the SpecialFX associated with this object...
	
	CJumpVolumeFX *pFX = (CJumpVolumeFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_JUMPVOLUME_ID, hJumpVolume );
	if( pFX )
	{
			
		// Dont add the full velocity amount if we aren't at the max...
				
		LTRotation	rRot;
		g_pLTClient->GetObjectRotation( g_pMoveMgr->GetObject(), &rRot );
		
		LTVector	vF = rRot.Forward();
		vF.y = 0.0f;
		vF.Normalize();

		LTVector	vDir = pFX->GetVelocity();
		vDir.y = 0.0f;
		vDir.Normalize();
		
		float	fDot = rRot.Forward().Dot( vDir );

		// Don't let the JumpVolume effct us if we're heading in the opposite direction...

		if( fDot <= 0.001f )
			return true;

		// Adjust the percentage based on the min and max values...

		float	fDirMax = g_vtJumpVolumeMaxDirectionPercent.GetFloat();
		float	fDirMin	= g_vtJumpVolumeMinDirectionPercent.GetFloat();
		float	fT = 0.0f;
		
		if( fDot >= fDirMax )
		{
			// Give us full amount
			fT = 1.0f;
		}
		else if( fDot <= fDirMin )
		{
			// Don't let it affect us at all
			return true;
		}
		else
		{
			fT = (fDot - fDirMin) / (1.0f - fDirMin);
		}

		WaveType	eType = (WaveType)(int)g_vtJumpVolumeWaveType.GetFloat();
		float		fPercent = GetVehicleMovementPercent() * GetWaveFn(eType)(fT); 

		if( g_vtJumpVolumeInfo.GetFloat() > 0.0f )
		{
			g_pLTClient->CPrint( "Movement: %.4f Dot: %.4f T: %.4f Wave: %.3f", GetVehicleMovementPercent(), fDot, fT, GetWaveFn(eType)(fT) );
			g_pLTClient->CPrint( "Percent: %.4f", fPercent );
		}
		
		LTVector vJumpVolumeVel = g_pMoveMgr->GetVelocity() + (pFX->GetVelocity() * fPercent);
		g_pMoveMgr->SetVelocity( vJumpVolumeVel );

		m_fJumpVolumeVel = vJumpVolumeVel.Mag();
		
		// The vehicle is now being affected by a JumpVolume...

		m_hCurJumpVolume		= hJumpVolume;
		m_bHaveJumpVolumeVel	= true;

		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::UpdateInJumpVolume
//
//  PURPOSE:	Detrimines if we are still within a JumpVolume...
//				This is really only used so a JumpVolume doesn't keep 
//				adding it's velocity if we intersect it on multiple frames.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateInJumpVolume()
{
	// Make sure we are in a JumpVolume...

	if( !m_hCurJumpVolume )
		return;

	HOBJECT hObj = g_pMoveMgr->GetObject();

	LTVector	vJVPos, vObjPos, vJVDims, vObjDims;
	LTVector	vJVMin, vJVMax, vObjMin, vObjMax;

	g_pLTClient->GetObjectPos( m_hCurJumpVolume, &vJVPos );
	g_pLTClient->GetObjectPos( hObj, &vObjPos );

	g_pPhysicsLT->GetObjectDims( m_hCurJumpVolume, &vJVDims );
	g_pPhysicsLT->GetObjectDims( hObj, &vObjDims );

	// Setup the extents...
	
	vJVMin	= vJVPos - vJVDims;
	vJVMax	= vJVPos + vJVDims;
	vObjMin	= vObjPos - vObjDims;
	vObjMax = vObjPos + vObjDims;

	// If the boxes arent intersecting then we are no longer in the JumpVolume...

	if( !BoxesIntersect( vJVMin, vJVMax, vObjMin, vObjMax ))
		m_hCurJumpVolume = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CVehicleMgr::HandleCharacterTouch
//
//  PURPOSE:	Handle hitting a characer...
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::HandleCharacterTouch( HOBJECT hCharacter )
{
	if( !hCharacter ) return false;

	float fVelPercent = GetVehicleMovementPercent();
	float fDamage		= g_vtVehicleImpactAIMinDamage.GetFloat();
	float fMaxDamage	= g_vtVehicleImpactAIMaxDamage.GetFloat();
	float fDamRange	= fMaxDamage - fDamage;

	fDamage += (fDamRange * fVelPercent);

	LTVector vPos;
	g_pLTClient->GetObjectPos(hCharacter, &vPos);

	// Get our forward vector...

	HOBJECT hObj = g_pMoveMgr->GetObject();
	LTRotation rRot;
	g_pLTClient->GetObjectRotation(hObj, &rRot);
	
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
	cMsg.Writeuint8(CP_DAMAGE_VEHICLE_IMPACT);
	cMsg.Writeuint8(DT_EXPLODE);
	cMsg.Writefloat(fDamage);
	cMsg.WriteLTVector(rRot.Forward());
	cMsg.WriteLTVector(vPos);
	cMsg.Writeuint8(0);
	cMsg.WriteObject(hCharacter);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	if (fDamage > (g_vtVehicleImpactAIMaxDamage.GetFloat() * 0.75f))
	{
		// Pop a little wheely...

		CameraDelta delta;
		delta.Pitch.fVar	= -DEG2RAD(5.0f);
		delta.Pitch.fTime1	= 0.1f;
		delta.Pitch.fTime2	= 0.3f;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		g_pPlayerMgr->GetCameraOffsetMgr()->AddDelta(delta);
	}

	// Should we continue processing the collision?

	if (!g_vtVehicleImpactAIDoNormalCollision.GetFloat())
	{
		return true;
	}
	
	return false;
}


bool CVehicleMgr::OnCommandOn(int /*command*/)
{
	return false;
}

bool CVehicleMgr::OnCommandOff(int /*command*/)
{
	return false;
}

void CVehicleMgr::AlignAngleToAxis(float &fAngle)
{
	// First make sure the yaw is in the 0 - 2pi range
	if(fAngle < 0)
	{
		fAngle = fAngle + (((float)((int)(fAngle/MATH_CIRCLE))+1)*MATH_CIRCLE);
	}
	else if(fAngle > MATH_CIRCLE)
	{
		fAngle = fAngle - (((float)((int)(fAngle/MATH_CIRCLE))+1)*MATH_CIRCLE);
	}

	// Now axis align it
	if(fAngle <= (MATH_PI/4.0f)) // PI/4
	{
		fAngle = 0;
	}
	else if(fAngle <= (MATH_PI*0.75f)) // 3PI/4
	{
		fAngle = MATH_HALFPI;
	}
	else if(fAngle <= (MATH_PI*1.25)) // 5PI/4
	{
		fAngle = MATH_PI;
	}
	else if(fAngle <= (MATH_PI*1.75)) // 7PI/4
	{
		fAngle = MATH_HALFPI*3.0f;
	}
	else
	{
		fAngle = 0;
	}

	/***********************
	Just in case, this code does the same thing as above but uses a forward 
	vector instead of an angle

		vF.y = 0;
		LTVector vAxis(0.0f,0.0f,0.0f);
		float fTheta;

		// Figure out which quadrant it's in
		if(vF.x >= 0.0f && vF.z >= 0.0f)
		{
			vAxis.z = 1.0f; // 12 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.x = 1.0f; // 3 o'clock
			}
			else
			{
				vMoveVel.z = 1.0f; // 12 o'clock
			}
		}
		else if(vF.x >= 0.0f && vF.z <= 0.0f)
		{
			vAxis.x = 1.0f; // 3 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.z = -1.0f; // 6 o'clock
			}
			else
			{
				vMoveVel.x = 1.0f; // 3 o'clock
			}
		}
		else if(vF.x <= 0.0f && vF.z >= 0.0f)
		{
			vAxis.z = -1.0f; // 6 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.x = -1.0f; // 9 o'clock
			}
			else
			{
				vMoveVel.z = -1.0f; // 6 o'clock
			}
		}
		else // (vF.x <= 0.0f && vF.z <= 0.0f)
		{
			vAxis.x = -1.0f; // 9 o'clock
			fTheta = ltacosf(vF.Dot(vAxis));
			if(fTheta >= (MATH_HALFPI/2.0f))
			{
				vMoveVel.z = 1.0f; // 12 o'clock
			}
			else
			{
				vMoveVel.x = -1.0f; // 9 o'clock
			}
		}
	***********************/
}

void CVehicleMgr::UpdateContainerMotion()
{
	// Get the containerinfo from movemgr update the gravity and viscosity...

	g_pMoveMgr->UpdateContainerList();

	uint32 nNumContainers = g_pMoveMgr->GetNumContainers();
	CContainerInfo *pInfo = NULL;

	for( uint32 i = 0; i < nNumContainers; ++i )
	{
		pInfo = g_pMoveMgr->GetContainerInfo( i );
		if( !pInfo )
			continue;

		g_pMoveMgr->UpdateContainerViscosity( pInfo );
		g_pMoveMgr->UpdateContainerGravity( pInfo );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveVehicleObject
//
//	PURPOSE:	Allows the vehicle to do some extra handeling of moving the object...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::MoveVehicleObject( LTVector &vPos )
{ 
	// Do movement based on physics model.
	switch( m_ePPhysicsModel )
	{
		case PPM_NORMAL:
		case PPM_LURE:
		{
			// Simple movement call...

			g_pPhysicsLT->MoveObject( g_pMoveMgr->GetObject(), vPos, 0 );
		}
		break;

		default:
			ASSERT( !"CVehicleMgr::MoveLocalSolidObject: Invalid physics model." );
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::MoveVehicleObject
//
//	PURPOSE:	Allows the vehicle to do some extra handeling of moving the object...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPlayerLureId( uint32 nPlayerLureId )
{
	m_nPlayerLureId = nPlayerLureId; 

	if( m_nPlayerLureId != PlayerLureId_Invalid )
	{
		m_bResetLure = true; 
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::Save
//
//	PURPOSE:	Serialization
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::Save(ILTMessage_Write *pMsg, SaveDataState /*eSaveDataState*/)
{
	// Saving this flag is fine since it doesn't deal with any actualy physics info
	// and is needed so we know if we should holster the current weapon when the 
	// physics model gets reset...

	pMsg->Writebool( m_bHolsteredWeapon );

	// If we are following a player lure we need to save the original offset transform...
	if( m_ePPhysicsModel == PPM_LURE )
	{
		// Get the playerlurefx object.
		PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_nPlayerLureId );
		if( pPlayerLureFX )
		{
			const LTRigidTransform &offset = pPlayerLureFX->GetOffsetTransform();

			pMsg->Writebool( true );
			pMsg->WriteLTVector( offset.m_vPos );
			pMsg->WriteLTRotation( offset.m_rRot );
		}
		else
		{
			pMsg->Writebool( false );
		}
	}
	else
	{
		pMsg->Writebool( false );
	}

	bool bSaveEncodingTransform = CPlayerBodyMgr::Instance( ).IsMovementEncoded( );
	pMsg->Writebool ( bSaveEncodingTransform );
	if( bSaveEncodingTransform )
	{
		LTRigidTransform rtSavedEncodingTransform;
		g_pModelLT->GetMovementEncodingTransform( g_pMoveMgr->GetObject( ), rtSavedEncodingTransform );
		
		// Factor in any previously saved transform so multiple save/loads while on a lure work.
		rtSavedEncodingTransform *= m_rtSavedEncodedTransform;
		pMsg->WriteLTRigidTransform( rtSavedEncodingTransform );
	}
	
// IMPORTANT NOTE!!! 
// Serialization of VehicleMgr is very bad, since the rest of the code assumes it's going
// to have to reset the state of the player physics model
/*
	pMsg->Writeuint32(m_ePPhysicsModel);

	pMsg->Writefloat(m_fAccelStart);
	pMsg->Writefloat(m_fDecelStart);
	pMsg->Writeuint32(m_dwControlFlags);
	pMsg->Writeuint32(m_dwLastControlFlags);
	pMsg->Writefloat(m_fVehicleBaseMoveAccel);

	pMsg->Writeint32(m_nCurGear);
	pMsg->Writeint32(m_nLastGear);
	pMsg->Writebool(m_bTurned);

	pMsg->Writebool(m_bVehicleStopped != false);
	pMsg->Writebool(m_bVehicleAtMaxSpeed != false);
	pMsg->Writebool(m_bVehicleTurning != false);
	pMsg->Writeint32(m_nVehicleTurnDirection);

	pMsg->WriteLTVector(m_vVehiclePYR);

	pMsg->Writefloat(m_fContourRoll);

	pMsg->Writeuint32(m_eSurface);
	pMsg->Writeuint32(m_eLastSurface);

	pMsg->Writefloat(m_fYawDiff);
	pMsg->Writefloat(m_fYawDelta);
	pMsg->Writefloat(m_fHandlebarRoll);
	pMsg->Writefloat(m_fHeadYaw);

	pMsg->Writeuint32(m_nPlayerLureId);

	pMsg->Writebool(m_bHaveJumpVolumeVel != false);
	pMsg->Writefloat(m_fJumpVolumeVel);

	pMsg->Writebool(m_bSetLastAngles);
	pMsg->WriteLTVector(m_vLastVehiclePYR);
	pMsg->WriteLTVector(m_vLastPlayerAngles);
	pMsg->WriteLTVector(m_vLastCameraAngles);
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::Load
//
//	PURPOSE:	Serialization
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::Load(ILTMessage_Read *pMsg, SaveDataState /*eLoadDataState*/)
{
	m_bHolsteredWeapon = pMsg->Readbool();

	m_bResetLureFromSave = pMsg->Readbool();
	if( m_bResetLureFromSave )
	{
		m_ResetLureTransform.m_vPos		= pMsg->ReadLTVector();
		m_ResetLureTransform.m_rRot		= pMsg->ReadLTRotation();
	}

	m_bHasSavedEncodedTransform = pMsg->Readbool( );
	if( m_bHasSavedEncodedTransform )
	{
		m_rtSavedEncodedTransform = pMsg->ReadLTRigidTransform( );
	}

// IMPORTANT NOTE!!! 
// Serialization of VehicleMgr is very bad, since the rest of the code assumes it's going
// to have to reset the state of the player physics model
/*
	SetPhysicsModel((PlayerPhysicsModel)pMsg->Readuint32());

	m_fAccelStart = pMsg->Readfloat();
	m_fDecelStart = pMsg->Readfloat();
	m_dwControlFlags = pMsg->Readuint32();
	m_dwLastControlFlags = pMsg->Readuint32();
	m_fVehicleBaseMoveAccel = pMsg->Readfloat();

	m_nCurGear = pMsg->Readuint32();
	m_nLastGear = pMsg->Readuint32();
	m_bTurned = pMsg->Readbool();

	m_bVehicleStopped = pMsg->Readbool() ? true : false;
	m_bVehicleAtMaxSpeed = pMsg->Readbool() ? true : false;
	m_bVehicleTurning = pMsg->Readbool() ? true : false;
	m_nVehicleTurnDirection = pMsg->Readuint32();

	m_vVehiclePYR = pMsg->ReadLTVector();

	m_fContourRoll = pMsg->Readfloat();

	m_eSurface = (SurfaceType)pMsg->Readuint32();
	m_eLastSurface = (SurfaceType)pMsg->Readuint32();

	m_fYawDiff = pMsg->Readfloat();
	m_fYawDelta = pMsg->Readfloat();
	m_fHandlebarRoll = pMsg->Readfloat();
	m_fHeadYaw = pMsg->Readfloat();

	m_nPlayerLureId = pMsg->Readuint32();

	m_bHaveJumpVolumeVel = pMsg->Readbool() ? true : false;
	m_fJumpVolumeVel = pMsg->Readfloat();

	m_bSetLastAngles = pMsg->Readbool();
	m_vLastVehiclePYR = pMsg->ReadLTVector();
	m_vLastPlayerAngles = pMsg->ReadLTVector();
	m_vLastCameraAngles = pMsg->ReadLTVector();
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SaveVehicleInfo
//
//	PURPOSE:	Write the console variables to the vehicle bute file.
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SaveVehicleInfo()
{
	switch( m_ePPhysicsModel )
	{
		case PPM_LURE :
		case PPM_NORMAL:
		default:
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleMgr::CanShowCrosshair()
//
//	PURPOSE:	Decide if the vehicle should display the crosshair...
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::CanShowCrosshair() const
{
	switch( m_ePPhysicsModel )
	{
		case PPM_LURE :
		case PPM_NORMAL :
		default :
			return true;

	}
}
// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CVehicleMgr::AllowHeadBobCant()
//
//	PURPOSE:	Allow headbobmgr to adjust head bob and canting.
//
// ----------------------------------------------------------------------- //

bool CVehicleMgr::AllowHeadBobCant() const
{
	switch( m_ePPhysicsModel )
	{
		case PPM_LURE :
			return false;

		case PPM_NORMAL :
		default :
			return true;

	}
}
