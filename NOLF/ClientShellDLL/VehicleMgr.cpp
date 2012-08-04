// ----------------------------------------------------------------------- //
//
// MODULE  : VehicleMgr.cpp
//
// PURPOSE : Client side vehicle mgr - Implementation
//
// CREATED : 6/12/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VehicleMgr.h"
#include "VarTrack.h"
#include "MsgIDs.h"
#include "CharacterFX.h"

#define MOTORCYCLE_SLIDE_TO_STOP_TIME	3.0f
#define SNOWMOBILE_SLIDE_TO_STOP_TIME	5.0f

#define VEHICLE_GEAR_0					0
#define VEHICLE_GEAR_1					1
#define VEHICLE_GEAR_2					2

#define DEFAULT_WORLD_GRAVITY			-2000.0f

extern CMoveMgr* g_pMoveMgr;
extern VarTrack g_vtMaxVehicleHeadCant;

static LTBOOL s_bHolsteredWeapon = LTFALSE;

VarTrack	g_vtMotorcycleInfoTrack;
VarTrack	g_vtMotorcycleVel;

VarTrack	g_vtMotorcycleAccel;
VarTrack	g_vtMotorcycleAccelTime;
VarTrack	g_vtMotorcycleMaxDecel;
VarTrack	g_vtMotorcycleDecelTime;
VarTrack	g_vtMotorcycleMaxBrake;
VarTrack	g_vtMotorcycleBrakeTime;

VarTrack	g_vtMotorcycleOffsetX;
VarTrack	g_vtMotorcycleOffsetY;
VarTrack	g_vtMotorcycleOffsetZ;

VarTrack	g_vtMotorcycleShift0Percent;
VarTrack	g_vtMotorcycleShift1Percent;
VarTrack	g_vtMotorcycleStoppedPercent;
VarTrack	g_vtMotorcycleMaxSpeedPercent;

VarTrack	g_vtMotorcycleAccelGearChange;

VarTrack	g_vtMotorcycleDialGear0Percent;
VarTrack	g_vtMotorcycleDialGear1Percent;

VarTrack	g_vtMotorcycleMinTurnAccel;

VarTrack	g_vtSnowmobileInfoTrack;
VarTrack	g_vtSnowmobileVel;

VarTrack	g_vtSnowmobileAccel;
VarTrack	g_vtSnowmobileAccelTime;
VarTrack	g_vtSnowmobileMaxDecel;
VarTrack	g_vtSnowmobileDecel;
VarTrack	g_vtSnowmobileDecelTime;
VarTrack	g_vtSnowmobileMaxBrake;
VarTrack	g_vtSnowmobileBrakeTime;

VarTrack	g_vtSnowmobileOffsetX;
VarTrack	g_vtSnowmobileOffsetY;
VarTrack	g_vtSnowmobileOffsetZ;

VarTrack	g_vtSnowmobileStoppedPercent;
VarTrack	g_vtSnowmobileStoppedTurnPercent;
VarTrack	g_vtSnowmobileMaxSpeedPercent;
VarTrack	g_vtSnowmobileAccelGearChange;

VarTrack	g_vtSnowmobileMinTurnAccel;

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

VarTrack	g_vtVehicleImpactAIDamageVelPercent;
VarTrack	g_vtVehicleImpactAIMinDamage;
VarTrack	g_vtVehicleImpactAIMaxDamage;
VarTrack	g_vtVehicleImpactAIDoNormalCollision;

VarTrack	g_vtVehicleFallDamageMinHeight;
VarTrack	g_vtVehicleFallDamageMaxHeight;

VarTrack	g_vtVehicleTurnRateScale;
VarTrack	g_vtVehicleTurnRate;
VarTrack	g_vtVehicleMaxTurnRate;
VarTrack	g_vtVehicleInitialTurnRate;
VarTrack	g_vtVehicleTurnScaleVelPercent;
VarTrack	g_vtVehicleTurnScaleStart;
VarTrack	g_vtVehicleTurnScaleEnd;

VarTrack	g_vtVehicleMouseMinYawBuffer;
VarTrack	g_vtVehicleMouseMaxYawBuffer;

VarTrack	g_vtVehicleMaxHeadOffsetX;
VarTrack	g_vtVehicleMaxHeadOffsetY;
VarTrack	g_vtVehicleMaxHeadOffsetZ;

VarTrack	g_vtVehicleMaxHeadYaw;

VarTrack	g_vtVehicleCollisionInfo;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModelNodeControlFn
//
//	PURPOSE:	Node control callback
//
// ----------------------------------------------------------------------- //

void ModelNodeControlFn(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat, void *pUserData)
{
	CVehicleMgr* pMgr = (CVehicleMgr*)pUserData;
	_ASSERT(pMgr);
	if (!pMgr) return;

	pMgr->HandleNodeControl(hObj, hNode, pGlobalMat);
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
	m_hVehicleModel			= LTNULL;
	m_hVehicleAttachModel	= LTNULL;
	m_hVehicleAttachSocket	= INVALID_MODEL_SOCKET;

    m_hVehicleStartSnd  = LTNULL;
    m_hVehicleAccelSnd  = LTNULL;
    m_hVehicleDecelSnd  = LTNULL;
	m_hVehicleImpactSnd = LTNULL;
	m_hIdleSnd			= LTNULL;
	m_hBrakeSnd			= LTNULL;

	m_vVehicleOffset.Init();
	m_vHeadOffset.Init();
	m_fHeadYaw = 0.0f;

    m_bVehicleTurning       = LTFALSE;
	m_nVehicleTurnDirection	= 0;

    m_bVehicleStopped    = LTTRUE;
    m_bVehicleAtMaxSpeed = LTFALSE;

	m_fHandlebarRoll = 0.0f;

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

LTBOOL CVehicleMgr::Init()
{
    g_vtMotorcycleInfoTrack.Init(g_pLTClient, "MotorcycleInfo", NULL, 0.0);
    g_vtMotorcycleVel.Init(g_pLTClient, "MotorcycleVel", NULL, 800.0);

	g_vtMotorcycleAccel.Init(g_pLTClient, "MotorcycleAccel", NULL, 4000.0);
	g_vtMotorcycleAccelTime.Init(g_pLTClient, "MotorcycleAccelTime", NULL, 3.0);
	g_vtMotorcycleMaxDecel.Init(g_pLTClient, "MotorcycleDecelMax", LTNULL, 30.0f);
    g_vtMotorcycleDecelTime.Init(g_pLTClient, "MotorcycleDecelTime", LTNULL, 1.0f);
	g_vtMotorcycleMaxBrake.Init(g_pLTClient, "MotorcycleBrakeMax", LTNULL, 150.0f);
    g_vtMotorcycleBrakeTime.Init(g_pLTClient, "MotorcycleBrakeTime", LTNULL, 1.0f);

	g_vtMotorcycleOffsetX.Init(g_pLTClient, "MotorcycleOffsetX", LTNULL, 0.0f);
    g_vtMotorcycleOffsetY.Init(g_pLTClient, "MotorcycleOffsetY", LTNULL, -1.2f);
    g_vtMotorcycleOffsetZ.Init(g_pLTClient, "MotorcycleOffsetZ", LTNULL, 2.5f);

    g_vtMotorcycleShift0Percent.Init(g_pLTClient, "MotorcycleShift0Percent", LTNULL, 0.05f);
    g_vtMotorcycleShift1Percent.Init(g_pLTClient, "MotorcycleShift1Percent", LTNULL, 0.5f);
    g_vtMotorcycleStoppedPercent.Init(g_pLTClient, "MotorcycleStoppedPercent", LTNULL, 0.05f);
    g_vtMotorcycleMaxSpeedPercent.Init(g_pLTClient, "MotorcycleMaxSpeedPercent", LTNULL, 0.95f);

    g_vtMotorcycleDialGear0Percent.Init(g_pLTClient, "MotorcycleDialGear0Percent", LTNULL, 1.0f);
    g_vtMotorcycleDialGear1Percent.Init(g_pLTClient, "MotorcycleDialGear1Percent", LTNULL, 0.8f);

    g_vtMotorcycleAccelGearChange.Init(g_pLTClient, "MotorcycleAccelGearChange", LTNULL, 1.0f);

    g_vtMotorcycleMinTurnAccel.Init(g_pLTClient, "MotorcycleMinTurnAccel", LTNULL, 0.0f);

    g_vtSnowmobileInfoTrack.Init(g_pLTClient, "SnowmobileInfo", NULL, 0.0);
    g_vtSnowmobileVel.Init(g_pLTClient, "SnowmobileVel", NULL, 800.0);

	g_vtSnowmobileAccel.Init(g_pLTClient, "SnowmobileAccel", NULL, 4000.0);
	g_vtSnowmobileAccelTime.Init(g_pLTClient, "SnowmobileAccelTime", NULL, 3.0);
	g_vtSnowmobileMaxDecel.Init(g_pLTClient, "SnowmobileDecelMax", LTNULL, 30.0f);
	g_vtSnowmobileDecel.Init(g_pLTClient, "SnowmobileDecel", LTNULL, 50.0f);
    g_vtSnowmobileDecelTime.Init(g_pLTClient, "SnowmobileDecelTime", LTNULL, 0.5f);
	g_vtSnowmobileMaxBrake.Init(g_pLTClient, "SnowmobileBrakeMax", LTNULL, 150.0f);
    g_vtSnowmobileBrakeTime.Init(g_pLTClient, "SnowmobileBrakeTime", LTNULL, 1.0f);

    g_vtSnowmobileMinTurnAccel.Init(g_pLTClient, "SnowmobileMinTurnAccel", LTNULL, 0.0f);

	g_vtSnowmobileOffsetX.Init(g_pLTClient, "SnowmobileOffsetX", LTNULL, 0.0f);
    g_vtSnowmobileOffsetY.Init(g_pLTClient, "SnowmobileOffsetY", LTNULL, -1.2f);
    g_vtSnowmobileOffsetZ.Init(g_pLTClient, "SnowmobileOffsetZ", LTNULL, 2.5f);

    g_vtSnowmobileStoppedPercent.Init(g_pLTClient, "SnowmobileStoppedPercent", LTNULL, 0.05f);
    g_vtSnowmobileStoppedTurnPercent.Init(g_pLTClient, "SnowmobileStoppedTurnPercent", LTNULL, 0.2f);
    g_vtSnowmobileMaxSpeedPercent.Init(g_pLTClient, "SnowmobileMaxSpeedPercent", LTNULL, 0.95f);

    g_vtSnowmobileAccelGearChange.Init(g_pLTClient, "SnowmobileAccelGearChange", LTNULL, 1.0f);


	g_vtVehicleImpactDamage.Init(g_pLTClient, "VehicleImpactDamage", LTNULL, 0.0f);
	g_vtVehicleImpactDamageMin.Init(g_pLTClient, "VehicleImpactDamageMin", LTNULL, 5.0f);
	g_vtVehicleImpactDamageMax.Init(g_pLTClient, "VehicleImpactDamageMax", LTNULL, 100.0f);
	g_vtVehicleImpactDamageVelPercent.Init(g_pLTClient, "VehicleImpactDamageVelPerent", LTNULL, 0.9f);
	g_vtVehicleImpactPushVelPercent.Init(g_pLTClient, "VehicleImpactPushVelPerent", LTNULL, 0.0f);
	g_vtVehicleImpactMinAngle.Init(g_pLTClient, "VehicleImpactMinAngle", LTNULL, 20.0f);
	g_vtVehicleImpactMaxAdjustAngle.Init(g_pLTClient, "VehicleImpactMinAdjustAngle", LTNULL, 10.0f);
	g_vtVehicleImpactPushAmount.Init(g_pLTClient, "VehicleImpactPushAmount", LTNULL, 800.0f);
	g_vtVehicleImpactPushRadius.Init(g_pLTClient, "VehicleImpactPushRadius", LTNULL, 100.0f);
	g_vtVehicleImpactPushDuration.Init(g_pLTClient, "VehicleImpactPushDuration", LTNULL, 0.25f);

	g_vtVehicleImpactFXOffsetForward.Init(g_pLTClient, "VehicleImpactFXOffsetForward", LTNULL, 0.0f);
	g_vtVehicleImpactFXOffsetUp.Init(g_pLTClient, "VehicleImpactFXOffsetUp", LTNULL, 0.0f);

	g_vtVehicleImpactAIDamageVelPercent.Init(g_pLTClient, "VehicleImpactAIDamageVelPerent", LTNULL, 0.1f);
 	g_vtVehicleImpactAIMinDamage.Init(g_pLTClient, "VehicleImpactAIMinDamage", LTNULL, 1.0f);
 	g_vtVehicleImpactAIMaxDamage.Init(g_pLTClient, "VehicleImpactAIMaxDamage", LTNULL, 100.0f);
 	g_vtVehicleImpactAIDoNormalCollision.Init(g_pLTClient, "VehicleImpactAIDoNormalCollision", LTNULL, 1.0f);

	g_vtVehicleFallDamageMinHeight.Init(g_pLTClient, "VehicleFallDamageMinHeight", LTNULL, 400.0f);
 	g_vtVehicleFallDamageMaxHeight.Init(g_pLTClient, "VehicleFallDamageMaxHeight", LTNULL, 600.0f);

    g_vtVehicleTurnRateScale.Init(g_pLTClient, "VehicleTurnRateScale", NULL, 1.0f);
    g_vtVehicleTurnScaleVelPercent.Init(g_pLTClient, "VehicleTurnScaleVelPercent", NULL, 0.5f);
    g_vtVehicleTurnScaleStart.Init(g_pLTClient, "VehicleTurnScaleStart", NULL, 1.0f);
    g_vtVehicleTurnScaleEnd.Init(g_pLTClient, "VehicleTurnScaleEnd", NULL, 0.5f);

	g_vtVehicleTurnRate.Init(g_pLTClient, "VehicleTurnRate", NULL, 3.5f);
    g_vtVehicleMaxTurnRate.Init(g_pLTClient, "VehicleMaxTurnRate", NULL, 3.5f);
	g_vtVehicleInitialTurnRate.Init(g_pLTClient, "VehicleInitialTurnRate", NULL, 1.0f);

	g_vtVehicleMouseMinYawBuffer.Init(g_pLTClient, "VehicleMouseMinYawBuffer", NULL, 5.0f);
	g_vtVehicleMouseMaxYawBuffer.Init(g_pLTClient, "VehicleMouseMaxYawBuffer", NULL, 45.0f);

	g_vtVehicleMaxHeadOffsetX.Init(g_pLTClient, "VehicleHeadMaxOffsetX", NULL, 0.0f);
	g_vtVehicleMaxHeadOffsetY.Init(g_pLTClient, "VehicleHeadMaxOffsetY", NULL, 0.0f);
	g_vtVehicleMaxHeadOffsetZ.Init(g_pLTClient, "VehicleHeadMaxOffsetZ", NULL, 0.0f);
	g_vtVehicleMaxHeadYaw.Init(g_pLTClient, "VehicleHeadMaxYaw", NULL, -1.0f);

    g_vtVehicleCollisionInfo.Init(g_pLTClient, "VehicleCollisionInfo", LTNULL, 0.0f);

	m_VehicleModelOffsetMgr.Init();

	// Init world specific data members...

	InitWorldData();

    return LTTRUE;
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

	m_bVehicleStopped       = LTTRUE;
    m_bVehicleAtMaxSpeed    = LTFALSE;
    m_bVehicleTurning       = LTFALSE;
	m_nVehicleTurnDirection	= 0;

	m_ePPhysicsModel		= PPM_NORMAL;

	m_nCurGear				= VEHICLE_GEAR_0;
	m_nLastGear				= VEHICLE_GEAR_0;

	m_hVehicleAttachSocket	= INVALID_MODEL_SOCKET;

	m_fVehiclePitch			= 0.0f;
	m_fVehicleYaw			= 0.0f;
	m_fVehicleRoll			= 0.0f;

	m_eSurface				= ST_UNKNOWN;
	m_eLastSurface			= ST_UNKNOWN;

	m_fYawDiff				= 0.0f;
	m_fYawDelta				= 0.0f;
	m_eMouseTurnDirection	= TD_CENTER;
	m_bKeyboardTurning		= LTFALSE;

	m_vHeadOffset.Init();
	m_fHandlebarRoll		= 0.0f;
	m_fHeadYaw				= 0.0f;
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
        g_pLTClient->DeleteObject(m_hVehicleModel);
		m_hVehicleModel = LTNULL;
	}

	if (m_hVehicleAttachModel)
	{
        g_pLTClient->DeleteObject(m_hVehicleAttachModel);
		m_hVehicleAttachModel = LTNULL;
	}

	KillAllVehicleSounds();

	if (m_hVehicleImpactSnd)
	{
        g_pLTClient->KillSound(m_hVehicleImpactSnd);
        m_hVehicleImpactSnd = LTNULL;
	}

	m_hVehicleAttachSocket = INVALID_MODEL_SOCKET;
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
		case PPM_MOTORCYCLE :
			UpdateMotorcycleControlFlags();
		break;

		case PPM_SNOWMOBILE :
			UpdateSnowmobileControlFlags();
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateMotorcycleControlFlags
//
//	PURPOSE:	Update the motorcycle physics model control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateMotorcycleControlFlags()
{
	// Determine what commands are currently on...

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RUN) ^ g_pInterfaceMgr->GetSettings()->RunLock())
	{
		m_dwControlFlags |= BC_CFLG_RUN;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))
	{
		m_dwControlFlags |= BC_CFLG_JUMP;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))
	{
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_REVERSE))
	{
		m_dwControlFlags |= BC_CFLG_REVERSE;
		m_dwControlFlags &= ~BC_CFLG_FORWARD;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_LEFT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_RIGHT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_STRAFE_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_FIRING;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_ALT_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_ALT_FIRING;
	}

	// Check for strafe left and strafe right.
	if ((m_dwControlFlags & BC_CFLG_RIGHT) && (m_dwControlFlags & BC_CFLG_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if ((m_dwControlFlags & BC_CFLG_LEFT) && (m_dwControlFlags & BC_CFLG_STRAFE))
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if ( (m_dwControlFlags & BC_CFLG_FORWARD) ||
		 (m_dwControlFlags & BC_CFLG_REVERSE) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_LEFT) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) ||
		 (m_dwControlFlags & BC_CFLG_JUMP))
	{
		m_dwControlFlags |= BC_CFLG_MOVING;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSnowmobileControlFlags
//
//	PURPOSE:	Update the snowmobile physics model control flags
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSnowmobileControlFlags()
{
	// Same as motorcycle
	UpdateMotorcycleControlFlags();
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
	// Make sure we are NOT using the playercollide physics if we are
	// on a vehicle (won't get correct collision information)...We need
	// to check this every frame since it might be changed someplace
	// else (like in CMoveMgr ;)...

	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		uint32 dwFlags2;
		HOBJECT hObj = g_pMoveMgr->GetObject();
		g_pLTClient->Common()->GetObjectFlags(hObj, OFT_Flags2, dwFlags2);
		dwFlags2 &= ~FLAG2_PLAYERCOLLIDE;
		g_pLTClient->Common()->SetObjectFlags(hObj, OFT_Flags2, dwFlags2);
	}


	m_eLastSurface = m_eSurface;
	m_eSurface	   = g_pMoveMgr->GetStandingOnSurface();

	// See if we moved over a new surface...

	if (m_eLastSurface != m_eSurface)
	{
		LTFLOAT fLastMult = GetSurfaceVelMult(m_eLastSurface);
		LTFLOAT fNewMult = GetSurfaceVelMult(m_eSurface);

		if (fLastMult < fNewMult)
		{
			// Speeding up...

			CameraDelta delta;
			delta.Pitch.fVar	= DEG2RAD(-0.5f);
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
			delta.Pitch.fVar	= DEG2RAD(0.75f);
			delta.Pitch.fTime1	= 0.25f;
			delta.Pitch.fTime2	= 0.5;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;
			m_VehicleModelOffsetMgr.AddDelta(delta);
		}
	}

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
		case PPM_SNOWMOBILE :
			UpdateVehicleMotion();
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleGear
//
//	PURPOSE:	Update the appropriate vehicle gear
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleGear()
{
 	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			UpdateMotorcycleGear();
		break;

		case PPM_SNOWMOBILE :
			UpdateSnowmobileGear();
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
		case PPM_MOTORCYCLE :
		case PPM_SNOWMOBILE :
			UpdateVehicleSounds();
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	// See if the impact snd is done playing...

	if (m_hVehicleImpactSnd)
	{
        if (g_pLTClient->IsDone(m_hVehicleImpactSnd))
		{
            g_pLTClient->KillSound(m_hVehicleImpactSnd);
            m_hVehicleImpactSnd = LTNULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleMotion
//
//	PURPOSE:	Update our motion when on a vehicle
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleMotion()
{
	HOBJECT hObj = g_pMoveMgr->GetObject();

    LTFLOAT fTime = g_pLTClient->GetTime();

	LTBOOL bHeadInLiquid = g_pMoveMgr->IsHeadInLiquid();
    LTBOOL bInLiquid     = (bHeadInLiquid || g_pMoveMgr->IsBodyInLiquid());
    LTBOOL bFreeMovement = (g_pMoveMgr->IsFreeMovement() |
		g_pMoveMgr->IsBodyOnLadder() || g_pGameClientShell->IsSpectatorMode());

	// Can't ride vehicles underwater, in spectator mode, or when dead...

	if (bHeadInLiquid || g_pGameClientShell->IsSpectatorMode() || g_pGameClientShell->IsPlayerDead())
	{
		HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
		g_pLTClient->WriteToMessageByte(hMessage, CP_PHYSICSMODEL);
		g_pLTClient->WriteToMessageByte(hMessage, PPM_NORMAL);
		g_pLTClient->EndMessage(hMessage);

		// The server won't notify us, so change models...

		SetPhysicsModel(PPM_NORMAL);
		return;
	}

	// Zero out the acceleration to start with...

    LTVector zeroVec;
	zeroVec.Init();
	g_pPhysicsLT->SetAcceleration(hObj, &zeroVec);


	// Set our current rotation...

    LTRotation rRot;
	g_pGameClientShell->GetPlayerRotation(&rRot);
	g_pLTClient->SetObjectRotation(hObj, &rRot);


    LTVector myVel = g_pMoveMgr->GetVelocity();

    LTVector vAccel(0, 0, 0);
    LTVector moveDir = myVel;
	moveDir.Norm();

    LTVector moveVel = myVel;
	moveVel.y = 0;

    LTBOOL bForward = LTFALSE;
	if ((m_dwControlFlags & BC_CFLG_FORWARD))
	{
        bForward = LTTRUE;
	}

	// This case happens when we jump if the user stops pressing forward
	// and we're in mid air...don't slow down!
	if (!bForward && !g_pMoveMgr->CanDoFootstep())
	{
		bForward = LTTRUE;
	}

	LTBOOL bBrake = LTFALSE;
	if ((m_dwControlFlags & BC_CFLG_REVERSE))
	{
		// Only brake if we are on the ground...
        bBrake = g_pMoveMgr->CanDoFootstep();
	}

    LTBOOL bReverse = !bForward;

	// Determine the acceleration to use...

    LTFLOAT fMaxAccel  = GetMaxAccelMag(GetSurfaceVelMult(m_eSurface));
    LTFLOAT fMaxVel    = GetMaxVelMag(GetSurfaceVelMult(m_eSurface));


	if (bForward)
	{
		LTFLOAT fAccelTime = GetVehicleAccelTime();

		if (m_fAccelStart < 0.0f)
		{
			m_fAccelStart = fTime - (m_fVehicleBaseMoveAccel * fAccelTime / fMaxAccel);
		}

		m_fVehicleBaseMoveAccel = fMaxAccel * (fTime - m_fAccelStart) / fAccelTime;
	}
	else
	{
		m_fAccelStart = -1.0f;
	}

	if (m_bVehicleTurning && !bBrake)
	{
		if (m_fVehicleBaseMoveAccel < GetVehicleMinTurnAccel())
		{
			m_fVehicleBaseMoveAccel = GetVehicleMinTurnAccel();
			bReverse = LTFALSE;
		}
	}

	m_fVehicleBaseMoveAccel = m_fVehicleBaseMoveAccel > fMaxAccel ? fMaxAccel : m_fVehicleBaseMoveAccel;


	// Limit velocity...

	if (g_pMoveMgr->IsOnGround() && !g_pGameClientShell->IsSpectatorMode())
	{
		float fCurLen = moveVel.Mag();
		if (fCurLen > fMaxVel)
		{
			myVel *= (fMaxVel/fCurLen);

			g_pPhysicsLT->SetVelocity(hObj, &myVel);
		}
	}
	else if (moveVel.Mag() > fMaxVel)
	{
		// Don't cap velocity in the y direction...

		moveVel.Norm();
		moveVel *= fMaxVel;

		myVel.x = moveVel.x;
		myVel.z = moveVel.z;

		g_pPhysicsLT->SetVelocity(hObj, &myVel);
	}

	// Update our "on ground" status...

	g_pMoveMgr->UpdateOnGround();


	g_pLTClient->GetObjectRotation(hObj, &rRot);

    LTVector vUp, vRight, vForward;
	g_pLTClient->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);
	vRight.y = 0.0f;


	LTFLOAT fAccelMulti = g_pMoveMgr->GetMoveAccelMultiplier();
	fAccelMulti = (fAccelMulti > 1.0f ? 1.0f : fAccelMulti);

    LTFLOAT fMoveAccel = (m_fVehicleBaseMoveAccel * fAccelMulti);

	if (!bInLiquid && !bFreeMovement)  // Can only move forward in x and z directions
	{
		vForward.y = 0.0;
		vForward.Norm();
	}


	// If we aren't dead we can drive around

	if (!g_pGameClientShell->IsPlayerDead())
	{
		vAccel += (vForward * fMoveAccel);

		if (bReverse || bBrake)
		{
			// Don't let us go backwards, slow us down...

            LTVector vAccelDir = vAccel;
			vAccelDir.Norm();
            LTFLOAT fAccelMag = vAccel.Mag();

			// Determine the acceleration to use...

            LTFLOAT fMaxDecel  = GetVehicleMaxDecel();
            LTFLOAT fDecelTime = GetVehicleDecelTime();

			if (bBrake)
			{
				fMaxDecel  = GetVehicleMaxBrake();
				fDecelTime = GetVehicleBrakeTime();
			}

			if (m_fDecelStart < 0.0f)
			{
				m_fDecelStart = fTime;
			}

            LTFLOAT fAccelAdjust = fMaxDecel * (fTime - m_fDecelStart) / fDecelTime;
			fAccelAdjust = fAccelAdjust > fAccelMag ? fAccelMag : fAccelAdjust;

			vAccel -= vAccelDir * fAccelAdjust;

			if (vAccel.Mag() < 10.0f)
			{
				vAccel.Init();
			}

			m_fVehicleBaseMoveAccel = vAccel.Mag();
		}
		else
		{
			m_fDecelStart = -1.0f;
		}
	}

	// Reset the mouse strafe flags in case they are set

	g_pMoveMgr->SetMouseStrafeFlags(0);


	// Cap the acceleration...

	if (vAccel.Mag() > fMoveAccel)
	{
		vAccel.Norm();
		vAccel *= fMoveAccel;
	}

	g_pPhysicsLT->SetAcceleration(hObj, &vAccel);


	// Add any container currents to my velocity...

    LTVector vel = g_pMoveMgr->GetVelocity();
	vel += g_pMoveMgr->GetTotalCurrent();

	g_pPhysicsLT->SetVelocity(hObj, &vel);


	// If we're dead, we can't move...

	if (g_pGameClientShell->IsPlayerDead() || !g_pGameClientShell->IsPlayerMovementAllowed())
	{
		g_pPhysicsLT->SetAcceleration(hObj, &zeroVec);
		g_pPhysicsLT->SetVelocity(hObj, &zeroVec);
	}


	// Handle case when we just start moving...

    g_pMoveMgr->UpdateStartMotion(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleAccelTime
//
//	PURPOSE:	Get the current vehicle's accerlation time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleAccelTime()
{
	LTFLOAT fTime = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			fTime = g_vtMotorcycleAccelTime.GetFloat();
		break;

		case PPM_SNOWMOBILE :
			fTime = g_vtSnowmobileAccelTime.GetFloat();
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fTime;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleMinTurnAccel
//
//	PURPOSE:	Get the current vehicle's minimum turn acceleration
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleMinTurnAccel()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			fValue = g_vtMotorcycleMinTurnAccel.GetFloat();
		break;

		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileMinTurnAccel.GetFloat();
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleMaxDecel
//
//	PURPOSE:	Get the current vehicle's maximum deceleration
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleMaxDecel()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			fValue = g_vtMotorcycleMaxDecel.GetFloat();
		break;

		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileMaxDecel.GetFloat();
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleDecelTime
//
//	PURPOSE:	Get the current vehicle's deceleration time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleDecelTime()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			fValue = g_vtMotorcycleDecelTime.GetFloat();
		break;

		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileDecelTime.GetFloat();
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleMaxBrake
//
//	PURPOSE:	Get the current vehicle's max break value
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleMaxBrake()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			fValue = g_vtMotorcycleMaxBrake.GetFloat();
		break;

		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileMaxBrake.GetFloat();
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return fValue;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetVehicleBrakeTime
//
//	PURPOSE:	Get the current vehicle's break time
//
// ----------------------------------------------------------------------- //

LTFLOAT	CVehicleMgr::GetVehicleBrakeTime()
{
	LTFLOAT fValue = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			fValue = g_vtMotorcycleBrakeTime.GetFloat();
		break;

		case PPM_SNOWMOBILE :
			fValue = g_vtSnowmobileBrakeTime.GetFloat();
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
		case PPM_MOTORCYCLE :
			UpdateVehicleFriction(MOTORCYCLE_SLIDE_TO_STOP_TIME);
		break;

		case PPM_SNOWMOBILE :
			UpdateVehicleFriction(SNOWMOBILE_SLIDE_TO_STOP_TIME);
		break;

		case PPM_NORMAL:
		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleFriction
//
//	PURPOSE:	Update vehicle fricton
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleFriction(LTFLOAT fSlideToStopTime)
{
	// Dampen our velocity so we don't slide around too much...

    uint32 dwTestFlags = BC_CFLG_MOVING | BC_CFLG_JUMP;

    LTVector vZero(0, 0, 0);

	if ( !(m_dwControlFlags & dwTestFlags) && g_pMoveMgr->IsOnGround() &&
		g_pMoveMgr->GetTotalCurrent() == vZero &&
		g_pMoveMgr->GetTotalViscosity() == 0.0f)
	{
        LTVector vCurVel = g_pMoveMgr->GetVelocity();

        LTFLOAT fYVal = vCurVel.y;
		vCurVel.y = 0.0f;

        LTVector vVel;

		if (vCurVel.Mag() > 5.0f)
		{
            LTVector vDir = vCurVel;
			vDir.Norm();

			if (fSlideToStopTime < 0.0f) fSlideToStopTime = 0.1f;

            LTFLOAT fAdjust = g_pGameClientShell->GetFrameTime() * (GetMaxVelMag()/fSlideToStopTime);

			vVel = (vDir * fAdjust);

			if (vVel.MagSqr() < vCurVel.MagSqr())
			{
				vVel = vCurVel - vVel;
			}
			else
			{
				vVel.Init();
			}

			vVel.y = fYVal;
		}
		else
		{
			vVel.Init();
		}

		g_pMoveMgr->SetVelocity(vVel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::IsVehiclePhysics
//
//	PURPOSE:	Are we doing vehicle physics
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::IsVehiclePhysics()
{
	return IsVehicleModel(m_ePPhysicsModel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::PreSetPhysicsModel
//
//	PURPOSE:	Do any clean up from old model
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::PreSetPhysicsModel(PlayerPhysicsModel eModel)
{
	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

	HOBJECT hObj = g_pMoveMgr->GetObject();
	uint32 dwFlags = g_pLTClient->GetObjectFlags(hObj);

	LTBOOL bRet = LTTRUE;

	// Enable/disable the weapon based on the physics model...

	CWeaponModel* pWeapon = g_pGameClientShell->GetWeaponModel();
	if (pWeapon)
	{
		if (IsVehicleModel(eModel))
		{
            uint32 dwFlags2;
            g_pLTClient->Common()->GetObjectFlags(hObj, OFT_Flags2, dwFlags2);
			dwFlags2 &= ~FLAG2_PLAYERCOLLIDE;
            g_pLTClient->Common()->SetObjectFlags(hObj, OFT_Flags2, dwFlags2);

			s_bHolsteredWeapon = LTFALSE;
			if (!pWeapon->IsMeleeWeapon())
			{
				pWeapon->ToggleHolster(LTFALSE);
				s_bHolsteredWeapon = LTTRUE;
			}
            pWeapon->Disable(LTTRUE);

			// Needed for collision physics...

			dwFlags |= FLAG_TOUCH_NOTIFY;
			g_pPhysicsLT->SetForceIgnoreLimit(hObj, 0.0f);
		}
		else
		{
            uint32 dwFlags2;
            g_pLTClient->Common()->GetObjectFlags(hObj, OFT_Flags2, dwFlags2);
			dwFlags2 |= FLAG2_PLAYERCOLLIDE;
            g_pLTClient->Common()->SetObjectFlags(hObj, OFT_Flags2, dwFlags2);

			dwFlags &= ~FLAG_TOUCH_NOTIFY;
			g_pPhysicsLT->SetForceIgnoreLimit(hObj, 1000.0f);

			// Turn off the vehicle...

			KillAllVehicleSounds();

 			switch (m_ePPhysicsModel)
			{
				case PPM_MOTORCYCLE :
				{
					g_pClientSoundMgr->PlaySoundLocal("Snd\\Vehicle\\Motorcycle\\turnoff.wav", SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_CLIENT);
				}
				break;

				case PPM_SNOWMOBILE :
				{
					g_pClientSoundMgr->PlaySoundLocal("Snd\\Vehicle\\Snowmobile\\turnoff.wav", SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_CLIENT);
				}
				break;

				default :
				case PPM_NORMAL :
				break;
			}

			PlayVehicleAni("Deselect");

			// Can't move until deselect is done...
			g_pMoveMgr->AllowMovement(LTFALSE);

			// Wait to change modes...
			bRet = LTFALSE;
		}
	}

    g_pLTClient->SetObjectFlags(hObj, dwFlags);

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetPhysicsModel
//
//	PURPOSE:	Set the physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetPhysicsModel(PlayerPhysicsModel eModel, LTBOOL bDoPreSet)
{
	if (m_ePPhysicsModel == eModel) return;

	// Do clean up...

	if (bDoPreSet && !PreSetPhysicsModel(eModel)) return;

	m_ePPhysicsModel = eModel;

 	switch (eModel)
	{
		case PPM_MOTORCYCLE :
		{
			SetMotorcyclePhysicsModel();

			if (g_vtMotorcycleInfoTrack.GetFloat())
			{
				g_pLTClient->CPrint("Motorcycle Physics Mode: ON");
			}
		}
		break;

		case PPM_SNOWMOBILE :
		{
			SetSnowmobilePhysicsModel();

			if (g_vtMotorcycleInfoTrack.GetFloat())
			{
				g_pLTClient->CPrint("Snowmobile Physics Mode: ON");
			}
		}
		break;

		default :
		case PPM_NORMAL :
		{
			SetNormalPhysicsModel();

			if (g_vtMotorcycleInfoTrack.GetFloat())
			{
				g_pLTClient->CPrint("Normal Physics Mode: ON");
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetMotorcyclePhysicsModel
//
//	PURPOSE:	Set the motorcycle physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetMotorcyclePhysicsModel()
{
	if (!m_hVehicleStartSnd)
	{
        uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		m_hVehicleStartSnd = g_pClientSoundMgr->PlaySoundLocal("Snd\\Vehicle\\Motorcycle\\startup.wav", SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
	}

	CreateVehicleModel();

	m_vVehicleOffset.x = g_vtMotorcycleOffsetX.GetFloat();
	m_vVehicleOffset.y = g_vtMotorcycleOffsetY.GetFloat();
	m_vVehicleOffset.z = g_vtMotorcycleOffsetZ.GetFloat();

	// Reset acceleration so we don't start off moving...

	m_fVehicleBaseMoveAccel = 0.0f;

	ShowVehicleModel(LTTRUE);

	PlayVehicleAni("Select");

	// Can't move until select ani is done...

	g_pMoveMgr->AllowMovement(LTFALSE);

	// Use motorcycle gravity (much less ;)

	g_pMoveMgr->SetGravity(DEFAULT_WORLD_GRAVITY/2.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::PlayVehicleAni
//
//	PURPOSE:	Change the vehicle's animation...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::PlayVehicleAni(char* pAniName, LTBOOL bReset, LTBOOL bLoop)
{
	if (!m_hVehicleModel || !pAniName) return;

	LTBOOL bIsDone = LTFALSE;
	if (!bReset && IsCurVehicleAni(pAniName, bIsDone))
	{
		return;
	}

	uint32 dwAnim = g_pLTClient->GetAnimIndex(m_hVehicleModel, pAniName);

    g_pLTClient->SetModelLooping(m_hVehicleModel, bLoop);
	g_pLTClient->SetModelAnimation(m_hVehicleModel, dwAnim);

	if (g_vtMotorcycleInfoTrack.GetFloat() == 2.0f)
	{
       g_pLTClient->CPrint("Playing Ani(%s)", pAniName);
	}

	if (bReset)
	{
		g_pLTClient->ResetModelAnimation(m_hVehicleModel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::IsCurVehicleAni
//
//	PURPOSE:	See if this is the current vehicle animation
//
// ----------------------------------------------------------------------- //

LTBOOL CVehicleMgr::IsCurVehicleAni(char* pAniName, LTBOOL & bIsDone)
{
	if (!m_hVehicleModel || !pAniName) return LTFALSE;

	// See if the current ani is done...

	uint32 dwState = g_pLTClient->GetModelPlaybackState(m_hVehicleModel);
 	bIsDone = (dwState & MS_PLAYDONE);

	uint32 dwTestAni = g_pLTClient->GetAnimIndex(m_hVehicleModel, pAniName);
	uint32 dwAni	 = g_pLTClient->GetModelAnimation(m_hVehicleModel);

	return (dwTestAni == dwAni);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::SetSnowmobilePhysicsModel
//
//	PURPOSE:	Set the snowmobile physics model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::SetSnowmobilePhysicsModel()
{
	if (!m_hVehicleStartSnd)
	{
        uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
		m_hVehicleStartSnd = g_pClientSoundMgr->PlaySoundLocal("Snd\\Vehicle\\Snowmobile\\startup.wav", SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
	}

	CreateVehicleModel();

	m_vVehicleOffset.x = g_vtSnowmobileOffsetX.GetFloat();
	m_vVehicleOffset.y = g_vtSnowmobileOffsetY.GetFloat();
	m_vVehicleOffset.z = g_vtSnowmobileOffsetZ.GetFloat();

	// Reset acceleration so we don't start off moving...

	m_fVehicleBaseMoveAccel = 0.0f;

 	ShowVehicleModel(LTTRUE);

	PlayVehicleAni("Select");

	// Can't move until select ani is done...

	g_pMoveMgr->AllowMovement(LTFALSE);

	// Use snowmobile gravity (much less ;)

	g_pMoveMgr->SetGravity(DEFAULT_WORLD_GRAVITY/2.0f);
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
	KillAllVehicleSounds();

	// Clear turning values...

	m_vHeadOffset.Init();
	m_fHandlebarRoll = 0.0f;
	m_fHeadYaw = 0.0f;
	m_fYawDiff = 0.0f;
	m_fYawDelta	= 0.0f;

	// Set gravity back to normal...

	g_pMoveMgr->SetGravity(DEFAULT_WORLD_GRAVITY);
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
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	// NEED TO MOVE THESE INTO A BUTE FILE!!!
	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
		{
			SAFE_STRCPY(createStruct.m_Filename, "Guns\\Models_PV\\moto_pv.abc");
			SAFE_STRCPY(createStruct.m_SkinNames[0], "Guns\\Skins_PV\\moto_pv.dtx");
		}
		break;

		case PPM_SNOWMOBILE :
		{
			SAFE_STRCPY(createStruct.m_Filename, "Guns\\Models_PV\\snow_pv.abc");
			SAFE_STRCPY(createStruct.m_SkinNames[0], "Guns\\Skins_PV\\snow_pv.dtx");
		}
		break;

		default :
			return;
		break;
	}
	// NEED TO MOVE THESE INTO A BUTE FILE!!!

	ModelStyle eModelStyle = eModelStyleDefault;
	CCharacterFX* pCharFX = g_pMoveMgr->GetCharacterFX();
	if (pCharFX)
	{
		eModelStyle = pCharFX->GetModelStyle();
	}

	if (g_pGameClientShell->GetGameType() == SINGLE || g_pMoveMgr->IsPlayerModel())
	{
		SAFE_STRCPY(createStruct.m_SkinNames[1], g_pModelButeMgr->GetHandsSkinFilename(eModelStyle));
	}
	else
	{
		SAFE_STRCPY(createStruct.m_SkinNames[1], "guns\\skins_pv\\MultiHands_pv.dtx");
	}

	if (!m_hVehicleModel)
	{
		createStruct.m_ObjectType = OT_MODEL;
		createStruct.m_Flags	  = FLAG_VISIBLE | FLAG_REALLYCLOSE | FLAG_ENVIRONMENTMAP;
		createStruct.m_Flags2	  = FLAG2_PORTALINVISIBLE | FLAG2_DYNAMICDIRLIGHT;

        m_hVehicleModel = g_pLTClient->CreateObject(&createStruct);
		if (!m_hVehicleModel) return;

		// Set up node control...

		g_pLTClient->ModelNodeControl(m_hVehicleModel, ModelNodeControlFn, this);
	}

    g_pLTClient->Common()->SetObjectFilenames(m_hVehicleModel, &createStruct);
    g_pLTClient->SetModelLooping(m_hVehicleModel, LTFALSE);


	// Map all the nodes in our skeleton in the bute file to the nodes in the actual .abc model file

	m_VehicleNode1.hNode = LTNULL;
	m_VehicleNode2.hNode = LTNULL;
	m_VehicleNode3.hNode = LTNULL;

	if (m_ePPhysicsModel == PPM_MOTORCYCLE)
	{
		int iNode = 0;
		HMODELNODE hCurNode = INVALID_MODEL_NODE;
		while (g_pLTClient->GetNextModelNode(m_hVehicleModel, hCurNode, &hCurNode) == LT_OK)
		{
			char szName[64];
			*szName = 0;
			g_pLTClient->GetModelNodeName(m_hVehicleModel, hCurNode, szName, sizeof(szName));

			if (stricmp(szName, "Dail") == 0)
			{
				m_VehicleNode1.hNode = hCurNode;
			}
			else if (stricmp(szName, "Dail2") == 0)
			{
				m_VehicleNode2.hNode = hCurNode;
			}
			else if (stricmp(szName, "brace") == 0)
			{
				m_VehicleNode3.hNode = hCurNode;
			}
		}
	}
	else if (m_ePPhysicsModel == PPM_SNOWMOBILE)
	{
		int iNode = 0;
		HMODELNODE hCurNode = INVALID_MODEL_NODE;
		while (g_pLTClient->GetNextModelNode(m_hVehicleModel, hCurNode, &hCurNode) == LT_OK)
		{
			char szName[64];
			*szName = 0;
			g_pLTClient->GetModelNodeName(m_hVehicleModel, hCurNode, szName, sizeof(szName));

			if (stricmp(szName, "Post") == 0)
			{
				m_VehicleNode1.hNode = hCurNode;
			}
		}
	}

	CreateVehicleAttachModel();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::CreateVehicleAttachModel
//
//	PURPOSE:	Create the player-view vehicle attachment model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CreateVehicleAttachModel()
{
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	m_hVehicleAttachSocket = INVALID_MODEL_SOCKET;

	char* pSocketName = LTNULL;

	// NEED TO MOVE THESE INTO A BUTE FILE!!!
	switch (m_ePPhysicsModel)
	{
		case PPM_SNOWMOBILE :
		{
			SAFE_STRCPY(createStruct.m_Filename, "Guns\\Models_PV\\windshield.abc");
			SAFE_STRCPY(createStruct.m_SkinNames[0], "Guns\\Skins_PV\\snow_pv.dtx");

			pSocketName = "windshield";
		}
		break;

		case PPM_MOTORCYCLE :
		default :
		{
			if (m_hVehicleAttachModel)
			{
				g_pLTClient->DeleteObject(m_hVehicleAttachModel);
				m_hVehicleAttachModel = LTNULL;
			}
		}
		break;
	}
	// NEED TO MOVE THESE INTO A BUTE FILE!!!


	if (!m_hVehicleAttachModel)
	{
		createStruct.m_ObjectType = OT_MODEL;
		createStruct.m_Flags	  = FLAG_VISIBLE | FLAG_REALLYCLOSE;
		createStruct.m_Flags2	  = FLAG2_PORTALINVISIBLE | FLAG2_DYNAMICDIRLIGHT;

        m_hVehicleAttachModel = g_pLTClient->CreateObject(&createStruct);
		if (!m_hVehicleAttachModel) return;

        g_pLTClient->SetObjectColor(m_hVehicleAttachModel, 1.0f, 1.0f, 1.0f, 0.99f);
	}

    g_pLTClient->Common()->SetObjectFilenames(m_hVehicleAttachModel, &createStruct);
    g_pLTClient->SetModelLooping(m_hVehicleAttachModel, LTFALSE);

	if (pSocketName)
	{
		g_pModelLT->GetSocket(m_hVehicleModel, pSocketName, m_hVehicleAttachSocket);
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
		UpdateVehicleModel();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleModel
//
//	PURPOSE:	Update the player-view vehicle model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleModel()
{
	if (!m_hVehicleModel) return;

 	// Update our camera offset mgr...

	m_VehicleModelOffsetMgr.Update();

	uint32 dwVehicleFlags = g_pLTClient->GetObjectFlags(m_hVehicleModel);

	// See if we're just waiting for he deselect animation to finish...

	LTBOOL bIsDone = LTFALSE;
	if (IsCurVehicleAni("Deselect", bIsDone))
	{
		if (bIsDone)
		{
			// Okay to move now...
			g_pMoveMgr->AllowMovement(LTTRUE);

			ShowVehicleModel(LTFALSE);
			SetPhysicsModel(PPM_NORMAL, LTFALSE);

			// Enable the weapon...

			CWeaponModel* pWeapon = g_pGameClientShell->GetWeaponModel();
			if (pWeapon)
			{
				pWeapon->Disable(LTFALSE);
				if (s_bHolsteredWeapon)
				{
					pWeapon->ToggleHolster(LTFALSE);
				}
			}

			return;
		}
	}

	// If we're in specatator mode, or 3rd-person don't show the player-view
	// vehicle model...

	if (g_pGameClientShell->IsSpectatorMode() || !g_pGameClientShell->IsFirstPerson())
	{
		ShowVehicleModel(LTFALSE);
		return;
	}

	ShowVehicleModel(LTTRUE);

    LTRotation rRot;
	rRot.Init();

    LTVector vPitchYawRollDelta = m_VehicleModelOffsetMgr.GetPitchYawRollDelta();

	m_fVehiclePitch = vPitchYawRollDelta.x;
	m_fVehicleYaw	= vPitchYawRollDelta.y + m_fHeadYaw;
	m_fVehicleRoll  = vPitchYawRollDelta.z;

    g_pLTClient->SetupEuler(&rRot, m_fVehiclePitch, m_fVehicleYaw, m_fVehicleRoll);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

    LTVector vNewPos(0, 0, 0), vScale;

	vNewPos	+= m_VehicleModelOffsetMgr.GetPosDelta();

	vNewPos += vR * (m_vVehicleOffset.x + m_vHeadOffset.x);
	vNewPos += vU * (m_vVehicleOffset.y + m_vHeadOffset.y);
	vNewPos += vF * (m_vVehicleOffset.z + m_vHeadOffset.z);

    g_pLTClient->SetObjectPos(m_hVehicleModel, &vNewPos, LTTRUE);
    g_pLTClient->SetObjectRotation(m_hVehicleModel, &rRot);

	UpdateVehicleAttachModel();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleAttachModel
//
//	PURPOSE:	Update the player-view vehicle attachment model
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleAttachModel()
{
	if (!m_hVehicleModel || !m_hVehicleAttachModel) return;
	if (m_hVehicleAttachSocket == INVALID_MODEL_SOCKET) return;

	LTransform transform;
    if (g_pModelLT->GetSocketTransform(m_hVehicleModel, m_hVehicleAttachSocket, transform, LTTRUE) == LT_OK)
	{
		LTVector vPos;
		LTRotation rRot;
		g_pTransLT->Get(transform, vPos, rRot);

        g_pLTClient->SetObjectPos(m_hVehicleAttachModel, &vPos, LTTRUE);
        g_pLTClient->SetObjectRotation(m_hVehicleAttachModel, &rRot);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::ShowVehicleModel
//
//	PURPOSE:	Show/Hide the vehicle model (and attachments)
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::ShowVehicleModel(LTBOOL bShow)
{
	if (!m_hVehicleModel) return;

	int32 dwFlags = g_pLTClient->GetObjectFlags(m_hVehicleModel);

	if (bShow)
	{
		g_pLTClient->SetObjectFlags(m_hVehicleModel, dwFlags | FLAG_VISIBLE);

		if (m_hVehicleAttachModel)
		{
			uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hVehicleAttachModel);
			g_pLTClient->SetObjectFlags(m_hVehicleAttachModel, dwFlags | FLAG_VISIBLE);
		}
	}
	else
	{
		g_pLTClient->SetObjectFlags(m_hVehicleModel, dwFlags & ~FLAG_VISIBLE);

		if (m_hVehicleAttachModel)
		{
			uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hVehicleAttachModel);
			g_pLTClient->SetObjectFlags(m_hVehicleAttachModel, dwFlags & ~FLAG_VISIBLE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetMaxVelMag
//
//	PURPOSE:	Get the max velocity for our current mode
//
// ----------------------------------------------------------------------- //

LTFLOAT CVehicleMgr::GetMaxVelMag(LTFLOAT fMult) const
{
    LTFLOAT fMaxVel = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
		{
			fMaxVel = g_vtMotorcycleVel.GetFloat();
		}
		break;

		case PPM_SNOWMOBILE :
		{
			fMaxVel = g_vtSnowmobileVel.GetFloat();
		}
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	fMaxVel *= fMult;

	LTFLOAT fMoveMulti = g_pMoveMgr->GetMoveMultiplier();
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

LTFLOAT CVehicleMgr::GetMaxAccelMag(LTFLOAT fMult) const
{
    LTFLOAT fMaxVel = 0.0f;

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
		{
			fMaxVel = g_vtMotorcycleAccel.GetFloat();
		}
		break;

		case PPM_SNOWMOBILE :
		{
			fMaxVel = g_vtSnowmobileAccel.GetFloat();
		}
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

LTFLOAT CVehicleMgr::GetSurfaceVelMult(SurfaceType eSurf) const
{
	LTFLOAT fVelMultiplier = 1.0f;

	SURFACE* pSurface = g_pSurfaceMgr->GetSurface(eSurf);

	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
		{
			if (pSurface)
			{
				fVelMultiplier = pSurface->fMotoVelMult;
			}
		}
		break;

		case PPM_SNOWMOBILE :
		{
			if (pSurface)
			{
				fVelMultiplier = pSurface->fSnowVelMult;
			}
		}
		break;

		case PPM_NORMAL:
		default:
		break;
	}

	return fVelMultiplier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateVehicleSounds
//
//	PURPOSE:	Update sounds while on the vehicle
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateVehicleSounds()
{
	// Update what gear the vehicle is in...

	UpdateVehicleGear();

	// See if the start snd is done playing...

	if (m_hVehicleStartSnd)
	{
        if (g_pLTClient->IsDone(m_hVehicleStartSnd))
		{
            g_pLTClient->KillSound(m_hVehicleStartSnd);
            m_hVehicleStartSnd = LTNULL;
		}
	}


	// See if we are going forward or backward...

    LTBOOL bForward = LTFALSE;
	if ((m_dwControlFlags & BC_CFLG_FORWARD))
	{
        bForward = LTTRUE;
	}

	LTBOOL bBrake = LTFALSE;
	if ((m_dwControlFlags & BC_CFLG_REVERSE))
	{
        bBrake = g_pMoveMgr->CanDoFootstep();
	}

	LTBOOL bMotorcycle = (m_ePPhysicsModel == PPM_MOTORCYCLE);
    LTBOOL bWheely = LTFALSE;

	if (bMotorcycle)
	{
		if ((m_dwControlFlags & BC_CFLG_JUMP))
		{
			bWheely = LTTRUE;
		}
	}

	LTBOOL bIsDone = LTFALSE;
	LTBOOL bPlayAni = LTTRUE;

	if (IsCurVehicleAni("Select", bIsDone))
	{
		if (bIsDone)
		{
			// Okay, we can move now...
			g_pMoveMgr->AllowMovement(LTTRUE);
		}
		else // Can't do anything until this is done...
		{
			return;
		}
	}
	else if (IsCurVehicleAni("Deselect", bIsDone))
	{
		return;
	}

	bIsDone = LTFALSE;

	// Play the appropriate sound, based on acceleration...

	if (!bForward)
	{
		if (m_hVehicleAccelSnd)
		{
            g_pLTClient->KillSound(m_hVehicleAccelSnd);
            m_hVehicleAccelSnd = LTNULL;
		}

		// Play braking ani if necessary...

		if (bBrake && !m_bVehicleStopped)
		{
			PlayVehicleAni("BrakeOn", LTFALSE);

			if (!m_hBrakeSnd)
			{
				char* pSound = GetBrakeSnd();

				if (pSound)
				{
					uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
					m_hBrakeSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
				}
			}

		}
		else
		{
			if (m_hBrakeSnd)
			{
                g_pLTClient->KillSound(m_hBrakeSnd);
                m_hBrakeSnd = LTNULL;
			}
		}

		if (m_bVehicleStopped)
		{
			if (m_hVehicleDecelSnd)
			{
                g_pLTClient->KillSound(m_hVehicleDecelSnd);
                m_hVehicleDecelSnd = LTNULL;
			}
		}
		else if (!m_hVehicleDecelSnd)
		{
			char* pSound = GetDecelSnd();

			if (pSound)
			{
                uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
				m_hVehicleDecelSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}
	}
	else // Forward
	{
		// Make sure we're playing the accelerate sound...

		if (!m_hVehicleAccelSnd)
		{
			KillAllVehicleSounds();

			char* pSound = GetAccelSnd();

			if (pSound)
			{
                uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT | PLAYSOUND_LOOP;
				m_hVehicleAccelSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
			}
		}

		if (!m_bVehicleStopped && bWheely)
		{
			// Pop a little wheely...

			CameraDelta delta;
			delta.Pitch.fVar	= -DEG2RAD(5.0f);
			delta.Pitch.fTime1	= 0.1f;
			delta.Pitch.fTime2	= 0.3f;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;

			g_pGameClientShell->GetCameraOffsetMgr()->AddDelta(delta);

			PlayVehicleAni("Fire", LTFALSE);
			bPlayAni = LTFALSE;
		}

		if (bForward && !m_bVehicleAtMaxSpeed)
		{
			// Play accelleration ani if necessary...

			if (bPlayAni)
			{
				if (bMotorcycle)
				{
					bIsDone = LTFALSE;
					if (IsCurVehicleAni("Clutch", bIsDone))
					{
						if (bIsDone)
						{
							PlayVehicleAni("Accellerate", LTFALSE);
						}
					}
					else if (!IsCurVehicleAni("Accellerate", bIsDone))
					{
						PlayVehicleAni("Clutch", LTFALSE);
					}
				}
				else
				{
					if (bPlayAni)
					{
						PlayVehicleAni("Accellerate", LTFALSE);
					}
				}
			}
		}
	}


	// See if we are idle...

	if (!bForward)
	{
		if (m_bVehicleStopped || !bBrake)
		{
			PlayVehicleAni("Idle_0", LTFALSE, LTTRUE);

			if (!m_hIdleSnd)
			{
				KillAllVehicleSounds(LTNULL);

				char* pSound = GetIdleSnd();

				if (pSound)
				{
					uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
					m_hIdleSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
				}
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetIdleSnd
//
//	PURPOSE:	Get the appropriate idle sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetIdleSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			return "Snd\\Vehicle\\Motorcycle\\idle.wav";
		break;

		case PPM_SNOWMOBILE :
			return "Snd\\Vehicle\\Snowmobile\\idle.wav";
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetAccelSnd
//
//	PURPOSE:	Get the appropriate acceleration sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetAccelSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			return "Snd\\Vehicle\\Motorcycle\\accel.wav";
		break;

		case PPM_SNOWMOBILE :
			return "Snd\\Vehicle\\Snowmobile\\accel.wav";
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetDecelSnd
//
//	PURPOSE:	Get the appropriate deceleration sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetDecelSnd()
{
 	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			return "Snd\\Vehicle\\Motorcycle\\decel.wav";
		break;

		case PPM_SNOWMOBILE :
			return "Snd\\Vehicle\\Snowmobile\\decel.wav";
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetBrakeSnd
//
//	PURPOSE:	Get the appropriate brake sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetBrakeSnd()
{
	switch (m_ePPhysicsModel)
	{
		case PPM_MOTORCYCLE :
			return "Snd\\Vehicle\\Motorcycle\\brake.wav";
		break;

		case PPM_SNOWMOBILE :
			return "Snd\\Vehicle\\Snowmobile\\brake.wav";
		break;

		case PPM_NORMAL:
		default :
		break;
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::GetImpactSnd
//
//	PURPOSE:	Get the appropriate impact sound...
//
// ----------------------------------------------------------------------- //

char* CVehicleMgr::GetImpactSnd(LTFLOAT fCurVelocityPercent, LTBOOL bHitCharacter)
{
    char* pSound = LTNULL;

	if (bHitCharacter)
	{
		if (GetRandom(0, 1) == 1)
		{
			pSound = "Snd\\Vehicle\\vehiclehit1.wav";
		}
		else
		{
			pSound = "Snd\\Vehicle\\vehiclehit2.wav";
		}
	}
	else
	{
		switch (m_ePPhysicsModel)
		{
			case PPM_MOTORCYCLE :
			{
				if (fCurVelocityPercent > 0.1f && fCurVelocityPercent < 0.4f)
				{
					pSound = "Snd\\Vehicle\\Motorcycle\\slowimpact.wav";
				}
				else if (fCurVelocityPercent < 0.7f)
				{
					pSound = "Snd\\Vehicle\\Motorcycle\\medimpact.wav";
				}
				else
				{
					pSound = "Snd\\Vehicle\\Motorcycle\\fastimpact.wav";
				}
			}
			break;

			case PPM_SNOWMOBILE :
			{
				if (fCurVelocityPercent > 0.1f && fCurVelocityPercent < 0.4f)
				{
					pSound = "Snd\\Vehicle\\Snowmobile\\slowimpact.wav";
				}
				else if (fCurVelocityPercent < 0.7f)
				{
					pSound = "Snd\\Vehicle\\Snowmobile\\medimpact.wav";
				}
				else
				{
					pSound = "Snd\\Vehicle\\Snowmobile\\fastimpact.wav";
				}
			}
			break;

			case PPM_NORMAL:
			default :
			break;
		}
	}

	return pSound;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateMotorcycleGear
//
//	PURPOSE:	Update the mototcycle's current gear...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateMotorcycleGear()
{
   LTVector myVel = g_pMoveMgr->GetVelocity();

   if (g_vtMotorcycleInfoTrack.GetFloat() == 1.0f)
	{
		// How fast are we going anyway...
		LTVector vAccel;
		g_pPhysicsLT->GetAcceleration(g_pMoveMgr->GetObject(), &vAccel);

		g_pLTClient->CPrint("Vel        = %.2f", myVel.Mag());
		g_pLTClient->CPrint("Accel      = %.2f", vAccel.Mag());
		g_pLTClient->CPrint("Base Accel = %.2f", m_fVehicleBaseMoveAccel);
	}

	myVel.y = 0.0f;

	LTBOOL bUsingAccel = (LTBOOL) g_vtMotorcycleAccelGearChange.GetFloat();

	LTFLOAT fMaxVal  = bUsingAccel ? GetMaxAccelMag() : GetMaxVelMag();
	LTFLOAT fVal	 = bUsingAccel ? m_fVehicleBaseMoveAccel : myVel.Mag();

	LTBOOL bWasStopped = m_bVehicleStopped;

	m_bVehicleStopped = LTFALSE;
	if (fVal < fMaxVal * g_vtMotorcycleStoppedPercent.GetFloat())
	{
        m_bVehicleStopped = LTTRUE;

		myVel.Init();
		g_pMoveMgr->SetVelocity(myVel);
	}

	// Add a little movement to show we stopped...

	if (!bWasStopped && m_bVehicleStopped)
	{
		CameraDelta delta;
		delta.Pitch.fVar	= DEG2RAD(0.25f);
		delta.Pitch.fTime1	= 0.4f;
		delta.Pitch.fTime2	= 0.5;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);
	}
	else if (bWasStopped && !m_bVehicleStopped)
	{
		// Show a little movement to show we started moving...

		CameraDelta delta;

		delta.Pitch.fVar	= DEG2RAD(-0.5f);
		delta.Pitch.fTime1	= 0.25f;
		delta.Pitch.fTime2	= 0.5;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);
	}


    m_bVehicleAtMaxSpeed = LTFALSE;
	if (fVal > fMaxVal * g_vtMotorcycleMaxSpeedPercent.GetFloat())
	{
        m_bVehicleAtMaxSpeed = LTTRUE;
	}

	m_nLastGear = m_nCurGear;

	if (fVal < fMaxVal * g_vtMotorcycleShift0Percent.GetFloat())
	{
		m_nCurGear = VEHICLE_GEAR_0;
	}
	else if (fVal < fMaxVal * g_vtMotorcycleShift1Percent.GetFloat())
	{
		m_nCurGear = VEHICLE_GEAR_1;
	}
	else
	{
		m_nCurGear = VEHICLE_GEAR_2;
	}


	// Update the motorcycle dials...

	UpdateMotorcycleDials();
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
        g_pLTClient->KillSound(m_hVehicleStartSnd);
        m_hVehicleStartSnd = LTNULL;
	}

	if (m_hVehicleAccelSnd && hException != m_hVehicleAccelSnd)
	{
        g_pLTClient->KillSound(m_hVehicleAccelSnd);
        m_hVehicleAccelSnd = LTNULL;
	}

	if (m_hVehicleDecelSnd && hException != m_hVehicleDecelSnd)
	{
        g_pLTClient->KillSound(m_hVehicleDecelSnd);
        m_hVehicleDecelSnd = LTNULL;
	}

	if (m_hIdleSnd && hException != m_hIdleSnd)
	{
        g_pLTClient->KillSound(m_hIdleSnd);
        m_hIdleSnd = LTNULL;
	}

	if (m_hBrakeSnd && hException != m_hBrakeSnd)
	{
        g_pLTClient->KillSound(m_hBrakeSnd);
        m_hBrakeSnd = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::OnTouchNotify
//
//	PURPOSE:	Handle our object touching something...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::OnTouchNotify(CollisionInfo *pInfo, float forceMag)
{
	if (!pInfo->m_hObject) return;

	// Filter out the player object
	if (pInfo->m_hObject == g_pLTClient->GetClientObject())
		return;

    uint32 dwFlags = g_pLTClient->GetObjectFlags(pInfo->m_hObject);
	if (!(dwFlags & FLAG_SOLID)) return;

    LTBOOL bIsWorld = IsMainWorld(pInfo->m_hObject);

    LTVector vNormal = pInfo->m_Plane.m_Normal;

	// If flat enough, we can drive on it...

	if (vNormal.y > 0.76f) return;

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
        g_pLTClient->CPrint("***********************************");
        g_pLTClient->CPrint("MoveMgr Object Touch:");
        g_pLTClient->CPrint("  Force: %.2f", forceMag);
        g_pLTClient->CPrint("  Stop Vel: %.2f, %.2f, %.2f", VEC_EXPAND(pInfo->m_vStopVel));
        g_pLTClient->CPrint("  (%s)", bIsWorld ? "WORLD" : "OBJECT");
        g_pLTClient->CPrint("  Normal: %.2f, %.2f, %.2f", VEC_EXPAND(vNormal));
        g_pLTClient->CPrint("  Object Type: %d", g_pLTClient->GetObjectType(pInfo->m_hObject));
	}

	vNormal.y = 0.0f; // Don't care about this anymore...


	// Determine the current percentage of our max velocity we are
	// currently moving...

	LTVector vVel = g_pMoveMgr->GetVelocity();
	vVel.y = 0.0f;

	LTFLOAT fMaxVel = GetMaxVelMag();
	LTFLOAT fCurVel = vVel.Mag() > fMaxVel ? fMaxVel : vVel.Mag();

	LTFLOAT fVelPercent = 1.0f - ((fMaxVel - fCurVel) / fMaxVel);

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
		g_pLTClient->CPrint("Calculated Velocity Percent: %.2f", fVelPercent);
	}


	// Adjust our yaw based on the object we hit...

	// Get our forward vector...

	HOBJECT hObj = g_pMoveMgr->GetObject();

    LTVector vU, vR, vF;
    LTRotation rRot;
    g_pLTClient->GetObjectRotation(hObj, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	LTBOOL bHitCharacter = LTFALSE;

	if (!bIsWorld && pInfo->m_hObject)
	{
		uint32 dwUserFlags = 0;
		g_pLTClient->GetObjectUserFlags(pInfo->m_hObject, &dwUserFlags);

		if (dwUserFlags & USRFLG_CHARACTER)
		{
			if (fVelPercent > g_vtVehicleImpactAIDamageVelPercent.GetFloat())
			{
				bHitCharacter = LTTRUE;

				LTFLOAT fDamage = g_vtVehicleImpactAIMinDamage.GetFloat();
				LTFLOAT fMaxDamage = g_vtVehicleImpactAIMaxDamage.GetFloat();
				LTFLOAT fDamRange = fMaxDamage - fDamage;

				fDamage += (fDamRange * fVelPercent);

				LTVector vPos;
				g_pLTClient->GetObjectPos(pInfo->m_hObject, &vPos);

				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
				g_pLTClient->WriteToMessageByte(hMessage, CP_DAMAGE_VEHICLE_IMPACT);
				g_pLTClient->WriteToMessageByte(hMessage, DT_EXPLODE);
				g_pLTClient->WriteToMessageFloat(hMessage, fDamage);
				g_pLTClient->WriteToMessageVector(hMessage, &vF);
				g_pLTClient->WriteToMessageVector(hMessage, &vPos);
				g_pLTClient->WriteToMessageByte(hMessage, 0);
				g_pLTClient->WriteToMessageObject(hMessage, pInfo->m_hObject);
				g_pLTClient->EndMessage(hMessage);

				if (fDamage > (g_vtVehicleImpactAIMaxDamage.GetFloat() * 0.75f))
				{
					// Pop a little wheely...

					CameraDelta delta;
					delta.Pitch.fVar	= -DEG2RAD(5.0f);
					delta.Pitch.fTime1	= 0.1f;
					delta.Pitch.fTime2	= 0.3f;
					delta.Pitch.eWave1	= Wave_SlowOff;
					delta.Pitch.eWave2	= Wave_SlowOff;
					g_pGameClientShell->GetCameraOffsetMgr()->AddDelta(delta);
				}

				// Should we continue processing the collision?

				if (!g_vtVehicleImpactAIDoNormalCollision.GetFloat())
				{
					return;
				}
			}
		}
	}

	vF.y = 0.0f; // Don't care about this...

	// Determine the direction to change our forward to...
	// Determine the angle between our forward and the new direction...

	vNormal.Norm();
	vF.Norm();

	// fAngle will be a value between -1 (left) and 1 (right)...

	LTFLOAT fAngle = VEC_DOT(vNormal, vR);

	if (fAngle > 0.0f)
	{
		fAngle = (MATH_PI/2.0f) * (1.0f - fAngle);
	}
	else
	{
		fAngle = (-MATH_PI/2.0f) * (1.0f + fAngle);
	}

	if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
	{
	   g_pLTClient->CPrint("Impact Angle: %.2f", RAD2DEG(fAngle));
	}

	LTFLOAT fImpactAngle = (LTFLOAT) fabs(fAngle);

	float fBiggestAngle = DEG2RAD(g_vtVehicleImpactMaxAdjustAngle.GetFloat());

	if (fImpactAngle > fBiggestAngle)
	{
		fAngle = fAngle > 0.0f ? fBiggestAngle : -fBiggestAngle;
	}

	// Add this angle to the yaw...

    LTFLOAT fYaw         = g_pGameClientShell->GetYaw();
    LTFLOAT fPlayerYaw   = g_pGameClientShell->GetPlayerYaw();

	// Change our yaw...
	// g_pGameClientShell->SetYaw(fYaw + fAngle);
	// g_pGameClientShell->SetPlayerYaw(fPlayerYaw + fAngle);

	// Adjust the min impact angle based on our current velocity (the
	// faster you go the more hitting stuff matters ;)

	float fMinImpactAngle = DEG2RAD(g_vtVehicleImpactMinAngle.GetFloat());

	fMinImpactAngle = fMinImpactAngle + (
		(1.0f - fVelPercent) * (DEG2RAD(90.0f) - fMinImpactAngle));

	// See if we hit hard enough to play a sound and do damage...

	if (fImpactAngle > fMinImpactAngle)
	{
		LTFLOAT fPushPercent = g_vtVehicleImpactPushVelPercent.GetFloat();
		if (fVelPercent < fPushPercent) return;

		// Create some sparks ;)
		IFXCS cs;

        g_pLTClient->AlignRotation(&(cs.rSurfRot), &(pInfo->m_Plane.m_Normal), LTNULL);

		LTVector vPos;
		g_pLTClient->GetObjectPos(hObj, &vPos);

		if (g_pGameClientShell->IsFirstPerson())
		{
			HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();
			if (!hCamera) return;

 			g_pLTClient->GetObjectPos(hCamera, &(cs.vPos));

			LTRotation rRot;
			g_pGameClientShell->GetCameraRotation(&rRot);

			LTVector vU, vR, vF;
			g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

			cs.vPos += (vF * g_vtVehicleImpactFXOffsetForward.GetFloat());
			cs.vPos += (vU * g_vtVehicleImpactFXOffsetUp.GetFloat());
		}
		else
		{
			cs.vPos = vPos;
		}

		cs.vSurfNormal = pInfo->m_Plane.m_Normal;

        const char* pImpactFXName = (fVelPercent < 0.4f ? "VehicleSlow" :
			(fVelPercent < 0.7f ? "VehicleMedium" : "VehicleFast"));

		if (!bHitCharacter)
		{
            IMPACTFX* pImpactFX = g_pFXButeMgr->GetImpactFX((char *)pImpactFXName);
			g_pFXButeMgr->CreateImpactFX(pImpactFX, cs);
		}

		// Push us away from the impact...(keep y value the same so we
		// don't get pushed up into the air)

		LTVector vDir = vVel;
		vDir.Norm();

		float dot = VEC_DOT(vDir, pInfo->m_Plane.m_Normal);
		dot *= -2.0f;

		VEC_ADDSCALED(vDir, vDir, pInfo->m_Plane.m_Normal, dot);

		float fOldY = vPos.y;
		vPos += (vDir * -1.0f);
		vPos.y = fOldY;

		float fPushRadius = g_vtVehicleImpactPushRadius.GetFloat();
		float fPushAmount = g_vtVehicleImpactPushAmount.GetFloat() * fVelPercent;
		float fStartDelay = 0.0f;
		float fDuration   = g_vtVehicleImpactPushDuration.GetFloat();
		g_pMoveMgr->AddPusher(vPos, fPushRadius, fStartDelay, fDuration, fPushAmount);

		if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
		{
			g_pLTClient->CPrint("Pusher Pos: %.2f, %.2f, %.2f", VEC_EXPAND(vPos));
		}

		// Move handle bars down a bit...(head over)...

		CameraDelta delta;
		delta.Pitch.fVar	= DEG2RAD(3.0f);
		delta.Pitch.fTime1	= 0.1f;
		delta.Pitch.fTime2	= 0.3f;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);

		// Tilt steering wheel based on the surface we hit...

		LTFLOAT fOffset		= DEG2RAD(g_vtVehicleMouseMinYawBuffer.GetFloat());
		LTFLOAT fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());
		LTFLOAT fYawPercent = (fImpactAngle / DEG2RAD(90.0f));
		m_fYawDiff = fYawPercent * fMaxOffset;
		m_fYawDiff *= (fAngle > 0.0f ? 1.0f : -1.0f);

		if (m_fYawDiff < -fOffset)
		{
			m_eMouseTurnDirection = TD_LEFT;
		}
		else if (m_fYawDiff > fOffset)
		{
			m_eMouseTurnDirection = TD_RIGHT;
		}

		// Clear velocity...

		LTVector vNewVel(0, 0, 0);

		LTFLOAT fAdjust = (1.0f - fYawPercent);
		vNewVel = vVel * fAdjust;
		g_pMoveMgr->SetVelocity(vNewVel);
		m_fVehicleBaseMoveAccel *= fAdjust;
		m_fAccelStart = -1.0f;

		// Reset the ani/sounds...

		PlayVehicleAni("Idle_0", LTFALSE, LTTRUE);
		KillAllVehicleSounds(LTNULL);


		char* pSound = GetImpactSnd(fVelPercent, bHitCharacter);
		if (pSound && !m_hVehicleImpactSnd)
		{
			uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
			m_hVehicleImpactSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
		}

		// See if we hit hard enough to actually damage us...

		LTFLOAT fImpactVelPercent = g_vtVehicleImpactDamageVelPercent.GetFloat();
		if (fVelPercent > fImpactVelPercent)
		{
			// Okay, since we were going pretty damn fast, just determine
			// the damage based on the angle we hit...

			float fRange = (MATH_PI/2.0f) - fMinImpactAngle;
			if (fRange <= 0.0f) fRange = 0.01f;

			LTFLOAT fDamagePercent = 1.0f - (((MATH_PI/2.0f) - fImpactAngle) / fRange);

 			// Send damage message to server...

			float fVehicleImpactDamageMin = g_vtVehicleImpactDamageMin.GetFloat();
			float fVehicleImpactDamageMax = g_vtVehicleImpactDamageMax.GetFloat();

			float fDamageRange = fVehicleImpactDamageMax - fVehicleImpactDamageMin;
			float fDamage = fVehicleImpactDamageMin + fDamageRange * fDamagePercent;

			if (g_vtVehicleImpactDamage.GetFloat())
			{
				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
				g_pLTClient->WriteToMessageByte(hMessage, CP_DAMAGE);
				g_pLTClient->WriteToMessageByte(hMessage, DT_STUN);
				g_pLTClient->WriteToMessageFloat(hMessage, 0.0f);
				g_pLTClient->WriteToMessageVector(hMessage, &vNormal);
				g_pLTClient->WriteToMessageByte(hMessage, 1);
				g_pLTClient->WriteToMessageFloat(hMessage, 1.0f);
				g_pLTClient->WriteToMessageObject(hMessage, g_pLTClient->GetClientObject());
				g_pLTClient->EndMessage(hMessage);
			}

			if (g_vtVehicleCollisionInfo.GetFloat() > 0.0f)
			{
				g_pLTClient->CPrint("Impact Damage: %.2f", fDamage);
			}
		}
	}
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
		LTransform transform;
        if (g_pModelLT->GetSocketTransform(m_hVehicleModel, hSocket, transform, LTFALSE) == LT_OK)
		{
			g_pTransLT->Get(transform, vPos, rRot);

			HOBJECT hCamera = g_pGameClientShell->GetCamera();
			if (!hCamera) return;

			LTVector vCamPos;
			g_pLTClient->GetObjectPos(hCamera, &vCamPos);
			g_pLTClient->GetObjectRotation(hCamera, &rRot);

			// Rotate down a bit...
			LTFLOAT fAmount = DEG2RAD(10.0f);
			LTVector vU, vR, vF;
			g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);
			g_pLTClient->RotateAroundAxis(&rRot, &vR, fAmount);

			vPos += vCamPos;
		}
	}
	else
	{
		HOBJECT hCamera = g_pGameClientShell->GetCamera();
		if (!hCamera) return;

		g_pLTClient->GetObjectRotation(hCamera, &rRot);
		g_pLTClient->GetObjectPos(hCamera, &vPos);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateMotorcycleDials
//
//	PURPOSE:	Update the motorcycle dials
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateMotorcycleDials()
{
	if (!m_hVehicleModel) return;

	// Reset all our nodes matrices

	m_VehicleNode1.matTransform.Identity();
	m_VehicleNode2.matTransform.Identity();
	m_VehicleNode3.matTransform.Identity();

	// Do the rotation on each dial
	//
    ILTMath *pMathLT = g_pLTClient->GetMathLT();

    LTRotation rRot;
    rRot.Init();
    LTVector vAxis(0.0f, 1.0f, 0.0f);
    LTFLOAT fMaxRot = MATH_DEGREES_TO_RADIANS(270.0f);

	// Calculate the speed dial rotation...

	LTVector vVel = g_pMoveMgr->GetVelocity();;

	LTFLOAT fMaxVel = GetMaxVelMag();
	LTFLOAT fCurVel = vVel.Mag() > fMaxVel ? fMaxVel : vVel.Mag();

	LTFLOAT fPercent = 1.0f - ((fMaxVel - fCurVel) / fMaxVel);

	LTFLOAT fCurRot = fPercent*fMaxRot;
	pMathLT->RotateAroundAxis(rRot, vAxis, -fCurRot);

	// Create a rotation matrix and apply it to the current offset matrix

	LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rRot);
	m_VehicleNode1.matTransform = m_VehicleNode1.matTransform * m1;

	// Tac...

	LTFLOAT fMaxAccel = GetMaxAccelMag();

	switch (m_nCurGear)
	{
		case VEHICLE_GEAR_0 :
		{
			fMaxAccel *= g_vtMotorcycleDialGear0Percent.GetFloat();
		}
		break;

		case VEHICLE_GEAR_1 :
		{
			fMaxAccel *= g_vtMotorcycleDialGear1Percent.GetFloat();
		}
		break;

		case VEHICLE_GEAR_2 :
		default :
		{
		}
		break;
	}

	LTFLOAT fCurAccel = m_fVehicleBaseMoveAccel;

	fPercent = 1.0f - ((fMaxAccel - fCurAccel) / fMaxAccel);
	fPercent = fPercent < 0.07f ? 0.07f : fPercent;
	fPercent += GetRandom(-0.05f, 0.0f);

    fMaxRot = MATH_DEGREES_TO_RADIANS(225.0f);

	fCurRot = fPercent*fMaxRot;
    rRot.Init();
	pMathLT->RotateAroundAxis(rRot, vAxis, -fCurRot);

	// Create a rotation matrix and apply it to the current offset matrix

	pMathLT->SetupRotationMatrix(m1, rRot);
	m_VehicleNode2.matTransform = m_VehicleNode2.matTransform * m1;


	// Handle bars...

    rRot.Init();
    vAxis.Init(0.0f, 1.0f, 0.0f);

	pMathLT->RotateAroundAxis(rRot, vAxis, m_fHandlebarRoll/2.0f);

	// Create a rotation matrix and apply it to the current offset matrix

	pMathLT->SetupRotationMatrix(m1, rRot);
	m_VehicleNode3.matTransform = m_VehicleNode3.matTransform * m1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::HandleNodeControl
//
//	PURPOSE:	Handle node control
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::HandleNodeControl(HOBJECT hObj, HMODELNODE hNode, LTMatrix *pGlobalMat)
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
//	ROUTINE:	CVehicleMgr::CalculateVehicleRotation()
//
//	PURPOSE:	Calculate the new player/camera rotation values...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::CalculateVehicleRotation(LTVector & vPlayerPYR,
										   LTVector & vPYR, LTFLOAT fYawDelta)
{
	vPlayerPYR.x = 0.0f;
	vPlayerPYR.z = vPYR.z;

	LTFLOAT fOffset		= DEG2RAD(g_vtVehicleMouseMinYawBuffer.GetFloat());
	LTFLOAT fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());

	LTFLOAT fAdjustedYawDelta = g_vtVehicleInitialTurnRate.GetFloat() * g_pGameClientShell->GetFrameTime();;

	// If we're playing the select or deselect anis, don't allow us to turn...

	LTBOOL bIsDone = LTFALSE;
	if (IsCurVehicleAni("Deselect", bIsDone) || IsCurVehicleAni("Select", bIsDone))
	{
		fYawDelta = 0.0f;
		m_bKeyboardTurning = LTFALSE;
		m_dwControlFlags &= ~BC_CFLG_STRAFE_LEFT;
		m_dwControlFlags &= ~BC_CFLG_LEFT;
		m_dwControlFlags &= ~BC_CFLG_STRAFE_RIGHT;
		m_dwControlFlags &= ~BC_CFLG_RIGHT;
	}

	if (fYawDelta)
	{
		m_bKeyboardTurning = LTFALSE;
	}

	// See if we are turning (using the keyboard)...

	if ((m_dwControlFlags & BC_CFLG_STRAFE_LEFT) || (m_dwControlFlags & BC_CFLG_LEFT))
	{
		m_bKeyboardTurning = LTTRUE;
		fYawDelta = -fAdjustedYawDelta;
	}
	else if ((m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) || (m_dwControlFlags & BC_CFLG_RIGHT))
	{
 		m_bKeyboardTurning = LTTRUE;
		fYawDelta = fAdjustedYawDelta;
	}
	else
	{
		// If we were turning with the keyboard automatically adjust back to
		// center...

		if (m_bKeyboardTurning)
		{
			switch (m_eMouseTurnDirection)
			{
				case TD_LEFT :
				{
 					fYawDelta = fAdjustedYawDelta;
				}
				break;

				case TD_RIGHT :
				{
					fYawDelta = -fAdjustedYawDelta;
				}
				break;

				default :
				case TD_CENTER :
				{
					if (m_fYawDiff > 0.0f)
					{
						fYawDelta = -fAdjustedYawDelta;
					}
					else if (m_fYawDiff < -fOffset)
					{
						fYawDelta = fAdjustedYawDelta;
					}
				}
				break;
			}
		}
	}

	// Use our last calculated yaw delta for the amount to turn
	// (if it is greater than our min turn rate)...

	if (fYawDelta)
	{
		if (m_fYawDelta > fAdjustedYawDelta)
		{
			fAdjustedYawDelta = m_fYawDelta;
		}

		fAdjustedYawDelta = (fYawDelta > 0.0f ? fAdjustedYawDelta : -fAdjustedYawDelta);
		m_fYawDiff += fAdjustedYawDelta;
	}

	// Depending on the direction we are turning, cap our yaw difference (from center)
	// and determine if the direciton we're turning changed...

	switch (m_eMouseTurnDirection)
	{
		case TD_LEFT :
		{
			if (m_fYawDiff < -fMaxOffset)
			{
				m_fYawDiff = -fMaxOffset;
			}
			else if (m_fYawDiff > -fOffset)
			{
				m_eMouseTurnDirection = TD_CENTER;
			}
		}
		break;

		case TD_RIGHT :
		{
			if (m_fYawDiff > fMaxOffset)
			{
				m_fYawDiff = fMaxOffset;
			}
			else if (m_fYawDiff < fOffset)
			{
				m_eMouseTurnDirection = TD_CENTER;
			}
		}
		break;

		default :
		case TD_CENTER :
		{
			if (m_fYawDiff > fOffset)
			{
				m_eMouseTurnDirection = TD_RIGHT;
			}
			else if (m_fYawDiff < -fOffset)
			{
				m_eMouseTurnDirection = TD_LEFT;
			}
		}
		break;
	}

	// For now assume we aren't turning...

	m_nVehicleTurnDirection = 0;
    m_bVehicleTurning = LTFALSE;


    LTFLOAT fInitialYawDiff = (LTFLOAT) fabs(vPYR.y - vPlayerPYR.y);

	if (fYawDelta != 0.0f || m_eMouseTurnDirection != TD_CENTER)
	{
		m_bVehicleTurning = LTTRUE;

		if (m_eMouseTurnDirection != TD_CENTER)
		{
			m_nVehicleTurnDirection = (m_eMouseTurnDirection == TD_LEFT ? -1 : 1);
		}
		else if (fYawDelta != 0.0f)
		{
			m_nVehicleTurnDirection = (fYawDelta < 0.0f ? -1 : 1);
		}
	}


	// Calculate the player's yaw...

	if (m_bVehicleTurning)
	{
		// Calculate the turn rate (may be scaled based on velocity)...

        LTFLOAT fMaxRate = g_vtVehicleMaxTurnRate.GetFloat();
        LTFLOAT fMinRate = g_vtVehicleTurnRate.GetFloat();

        LTFLOAT fRateRange = fMaxRate - fMinRate;

        LTVector vVel = g_pMoveMgr->GetVelocity();
		vVel.y = 0.0f;

        LTFLOAT fVelMag = vVel.Mag();
        LTFLOAT fMaxVelMag = GetMaxVelMag();
        LTFLOAT fMinVelMag = (fMaxVelMag * g_vtVehicleTurnScaleVelPercent.GetFloat());

		// If necessary scale the turning based on how fast we are moving...

		if (fVelMag > fMinVelMag)
		{
            LTFLOAT fScaleStart  = g_vtVehicleTurnScaleStart.GetFloat();
            LTFLOAT fScaleEnd    = g_vtVehicleTurnScaleEnd.GetFloat();
            LTFLOAT fScaleRange  = fScaleStart - fScaleEnd; // Assumes slow down
            LTFLOAT fVelRange    = fMaxVelMag - fMinVelMag;

            LTFLOAT fScaleOffset = fScaleRange * (fVelMag - fMinVelMag) / fVelRange;
            LTFLOAT fScaleVal = fScaleStart - fScaleOffset;

			// Update scale value...

			fRateRange *= fScaleVal;
		}

 		LTFLOAT fNewRate = fMinRate + (fRateRange * fVelMag / fMaxVelMag);
        m_fYawDelta = fNewRate * g_pGameClientShell->GetFrameTime();


		// Scale how fast we turn based on how close to center the
		// "steering wheel" is...

		LTFLOAT fScale = (LTFLOAT)fabs(m_fYawDiff) / fMaxOffset;
		fScale = WaveFn_SlowOn(fScale);
		m_fYawDelta *= fScale;


		// Scale the rate based on use input...

		m_fYawDelta *= g_vtVehicleTurnRateScale.GetFloat();


		// Set the player's new yaw...

		if (!m_bVehicleStopped)
		{
			vPlayerPYR.y += (m_fYawDelta * m_nVehicleTurnDirection);
		}
	}

	// Keep the camera and player pitch/yaw/roll in sync...

	vPYR.y = vPlayerPYR.y;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSnowmobileGear
//
//	PURPOSE:	Update the snowmobile's current gear...
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSnowmobileGear()
{
	LTVector myVel = g_pMoveMgr->GetVelocity();

	// Print out info...

	if (g_vtSnowmobileInfoTrack.GetFloat() == 1.0f)
	{
		// How fast are we going anyway...

		LTVector vAccel;
		g_pPhysicsLT->GetAcceleration(g_pMoveMgr->GetObject(), &vAccel);

		g_pLTClient->CPrint("Vel = %.2f", myVel.Mag());
		g_pLTClient->CPrint("Accel = %.2f", vAccel.Mag());
	}

	myVel.y = 0.0f;

	LTBOOL bUsingAccel = (LTBOOL) g_vtSnowmobileAccelGearChange.GetFloat();

	LTFLOAT fMaxVal  = bUsingAccel ? GetMaxAccelMag() : GetMaxVelMag();
	LTFLOAT fVal	 = bUsingAccel ? m_fVehicleBaseMoveAccel : myVel.Mag();

	LTBOOL bWasStopped = m_bVehicleStopped;

	m_bVehicleStopped = LTFALSE;
	if (fVal < fMaxVal * g_vtSnowmobileStoppedPercent.GetFloat())
	{
        m_bVehicleStopped = LTTRUE;

		myVel.Init();
		g_pMoveMgr->SetVelocity(myVel);
	}

	// Add a little movement to show we stopped...

	if (!bWasStopped && m_bVehicleStopped)
	{
		CameraDelta delta;
		delta.Pitch.fVar	= DEG2RAD(0.25f);
		delta.Pitch.fTime1	= 0.4f;
		delta.Pitch.fTime2	= 0.5;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);
	}
	else if (bWasStopped && !m_bVehicleStopped)
	{
		// Show a little movement to show we started moving...

		CameraDelta delta;

		delta.Pitch.fVar	= DEG2RAD(-0.5f);
		delta.Pitch.fTime1	= 0.25f;
		delta.Pitch.fTime2	= 0.5;
		delta.Pitch.eWave1	= Wave_SlowOff;
		delta.Pitch.eWave2	= Wave_SlowOff;
		m_VehicleModelOffsetMgr.AddDelta(delta);
	}

    m_bVehicleAtMaxSpeed = LTFALSE;
	if (fVal > fMaxVal * g_vtSnowmobileMaxSpeedPercent.GetFloat())
	{
        m_bVehicleAtMaxSpeed = LTTRUE;
	}

	m_nLastGear = m_nCurGear;

	m_nCurGear = VEHICLE_GEAR_0;

	// Update the handlebars...

	UpdateSnowmobileHandleBars();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::UpdateSnowmobileHandleBars
//
//	PURPOSE:	Update the snowmobile handlebars
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::UpdateSnowmobileHandleBars()
{
	if (!m_hVehicleModel) return;

	// Reset all our nodes matrices

	m_VehicleNode1.matTransform.Identity();
	m_VehicleNode2.matTransform.Identity();
	m_VehicleNode3.matTransform.Identity();

	// Do the rotation on the handle bar node
	//
    ILTMath *pMathLT = g_pLTClient->GetMathLT();

    LTRotation rRot;
    rRot.Init();
    LTVector vAxis(0.0f, 1.0f, 0.0f);

	// LTFLOAT fRoll = g_pGameClientShell->GetRoll();
	pMathLT->RotateAroundAxis(rRot, vAxis, m_fHandlebarRoll);

	// Create a rotation matrix and apply it to the current offset matrix

	LTMatrix m1;
	pMathLT->SetupRotationMatrix(m1, rRot);
	m_VehicleNode1.matTransform = m_VehicleNode1.matTransform * m1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVehicleMgr::AdjustRoll
//
//	PURPOSE:	Adjust the camera roll if necessary
//
// ----------------------------------------------------------------------- //

void CVehicleMgr::AdjustCameraRoll(LTFLOAT & fRoll)
{
	// Only adjust if we are on a vehicle...

	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		// Don't adjust camera when selecting/deselecting the vehicle...

		LTBOOL bIsDone = LTFALSE;
		if (IsCurVehicleAni("Deselect", bIsDone) || IsCurVehicleAni("Select", bIsDone))
		{
			return;
		}

		LTFLOAT fMaxCant = DEG2RAD(g_vtMaxVehicleHeadCant.GetFloat());

		LTFLOAT fMaxOffset	= DEG2RAD(g_vtVehicleMouseMaxYawBuffer.GetFloat());
		LTFLOAT fYawDiff = (LTFLOAT)fabs(m_fYawDiff);
		LTFLOAT fPercent = fYawDiff / fMaxOffset;

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