// ----------------------------------------------------------------------- //
//
// MODULE  : CMoveMgr.cpp
//
// PURPOSE : Client side player movement mgr - Implementation
//
// CREATED : 10/2/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "iltclient.h"
#include "iltphysics.h"
#include "CMoveMgr.h"
#include "GameClientShell.h"
#include "SharedMovement.h"
#include "CommandIDs.h"
#include "ClientServerShared.h"
#include "ClientWeaponUtils.h"
#include "MsgIDs.h"
#include "SurfaceFunctions.h"
#include "SFXMgr.h"
#include "CharacterFX.h"
#include "VarTrack.h"
#include "VehicleMgr.h"
#include "BankedList.h"
#include "PlayerStats.h"
#include "LeanMgr.h"
#include "ClientConnectionMgr.h"
#include "ProfileMgr.h"
#include "TargetMgr.h"
#include "VolumeBrushFX.h"
#include "PlayerBodyMgr.h"
#include "PlayerCamera.h"
#include "LTEulerAngles.h"
#include "PlayerLureFX.h"
#include "ClientWeaponMgr.h"
#include "bindmgr.h"
#include "SkillDefs.h"
#include "AimMgr.h"
#include "LadderMgr.h"
#include "LadderFX.h"
#include "SpecialMoveMgr.h"
#include "ClientDB.h"
#include "GameModeMgr.h"
#include "PolyGridFX.h"

//!!ARL: Move to ltmath.h
//this is the inverse of LTLERP. given two values and result, this will return the interpolant used 
//to generate given result. use LTALERP<float> with ints, etc. This uses the - and / operators
template<typename T>
inline T LTALERP_I(T min, T max, T result)			{ return ((result - min) / (max - min)); }
template<typename T, typename TMAX, typename TRESULT>
inline T LTALERP(T min, TMAX max, TRESULT result)	{ LTASSERT((min != max), "Min == Max causes divide by zero!"); return LTALERP_I<T>(min, max, result); }

StopWatchTimer TestFireTimer;
StopWatchTimer TestFireRate;
double fLastTestFireTime;
uint32 nTestFireShotsFired;
float fTestFireDuration;
float fTestFireRate;
uint32 nTestGrenadesThrown;

static void TestFireFn(int argc, char **argv)
{
	fTestFireDuration = 2.0f;
	if(argc >= 1)
	{
		uint32 nfps = atol(argv[0]);
		if (nfps > 0)
		{
			g_pLTClient->SetConsoleVariableFloat("MaxFPS", (float)nfps);
		}
		if(argc >= 2)
		{
			fTestFireDuration = float(atof(argv[1]));
 		}
	}


	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		// Reload the clip and let the server know...
		pClientWeapon->ReloadClip( false, -1, true, true );
	}

	nTestFireShotsFired = 0;

	WriteConsoleBool("TestFiring",true);
}
static void TestSAFireFn(int argc, char **argv)
{
	fTestFireRate = 0.05f;
	fTestFireDuration = 2.0f;
	if(argc >= 1)
	{
		fTestFireRate = float(atof(argv[0]));
		if(argc >= 2)
		{
			fTestFireDuration = float(atof(argv[1]));
		}
	}


	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		// Reload the clip and let the server know...
		pClientWeapon->ReloadClip( false, -1, true, true );
	}

	nTestFireShotsFired = 0;
	
//	WriteConsoleBool("ClientWeaponStateTracking",true);

	WriteConsoleBool("TestSAFiring",true);
}

static void TestFire2Fn(int argc, char **argv)
{
	fTestFireDuration = 2.0f;
	if(argc >= 1)
	{
		fTestFireDuration = float(atof(argv[0]));
	}

	nTestFireShotsFired = 0;

	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if ( pClientWeapon )
	{
		if	(pClientWeapon->IsSemiAuto())
		{
			WriteConsoleBool("TestSAFiring",true);
		}
		else
		{
			WriteConsoleBool("TestFiring",true);
		}
	}

}
static void TestGrenadeFn(int argc, char **argv)
{
	fTestFireRate = 0.05f;
	fTestFireDuration = 2.0f;
	if(argc >= 1)
	{
		fTestFireRate = float(atof(argv[0]));
		if(argc >= 2)
		{
			fTestFireDuration = float(atof(argv[1]));
		}
	}



	nTestGrenadesThrown = 0;

	//	WriteConsoleBool("ClientWeaponStateTracking",true);

	WriteConsoleBool("TestGrenadeThrow",true);
}



#define SPECTATOR_ACCELERATION			3000.0f
#define MIN_ONGROUND_Y					-10000000.0f

// Note : Duplicated from PlayerObj.cpp on the server
#define DEFAULT_FRICTION					5.0f

// Animation information for forceduck
#define PLAYERANIM_STAND	"LSt"
#define PLAYERANIM_CROUCH	"LC"

class Pusher
{
public:
	Pusher()
	{
		m_Link.SetData(this);
	}

	~Pusher( )
	{
		m_Link.Remove();
	}

	// Banked list for keeping track of the Pusher objects statically
	static CBankedList<Pusher> *GetBank() {
		static CBankedList<Pusher> theBank;
		return &theBank;
	}

    LTVector m_Pos;
	float	m_Radius;
	float	m_Delay;	// How long before it starts actually pushing.
	float	m_TimeLeft; // Time left for this sphere.
	float	m_Strength;
    LTLink<Pusher*>   m_Link;
};

VarTrack	g_vtPlayerGravity;
const char * const g_pszPlayerGravity = "PlayerGravity";

VarTrack	g_vtInAirAccelMultiplier;
const char * const g_pszInAirAccelMultiplier = "InAirAccelMultiplier";

VarTrack	g_vtCamLandMinHeight;
const char * const g_pszCamLandMinHeight = "CamLandMinHeight";

VarTrack	g_vtFallDamageMinHeight;
const char * const g_pszFallDamageMinHeight = "FallDamageMinHeight";

VarTrack	g_vtFallDamageMaxHeight;
const char * const g_pszFallDamageMaxHeight = "FallDamageMaxHeight";

VarTrack	g_vtFallDamageMin;
const char * const g_pszFallDamageMin = "FallDamageMin";

VarTrack	g_vtFallDamageMax;
const char * const g_pszFallDamageMax = "FallDamageMax";

VarTrack	g_vtFallDamageDebug;
const char * const g_pszFallDamageDebug = "FallDamageDebug";

VarTrack	g_vtCamLandMoveDist;
const char * const g_pszCamLandMoveDist = "CamLandMoveDist";

VarTrack	g_vtCamLandDownTime;
const char * const g_pszCamLandDownTime = "CamLandDownTime";

VarTrack	g_vtCamLandUpTime;
const char * const g_pszCamLandUpTime = "CamLandUpTime";

VarTrack	g_vtCamLandRollVal;
const char * const g_pszCamLandRollVal = "CamLandRollVal";

VarTrack	g_vtCamLandRollTime1;
const char * const g_pszCamLandRollTime1 = "CamLandRollTime1";

VarTrack	g_vtCamLandRollTime2;
const char * const g_pszCamLandRollTime2 = "CamLandRollTime2";

VarTrack	g_vtCamLandPitchVal;
const char * const g_pszCamLandPitchVal = "CamLandPitchVal";

VarTrack	g_vtCamLandPitchTime1;
const char * const g_pszCamLandPitchTime1 = "CamLandPitchTime1";

VarTrack	g_vtCamLandPitchTime2;
const char * const g_pszCamLandPitchTime2 = "CamLandPitchTime2";

VarTrack	g_vtSlideToStopTime;
const char * const g_pszSlideToStopTime = "SlideToStopTime";

VarTrack	g_vtMaxPushYVelocity;
const char * const g_pszPusherMaxYVelocity = "PusherMaxYVelocity";

VarTrack	g_vtPlayerYawShuffleEnabled;
const char * const g_pszPlayerYawShuffleEnabled = "PlayerYawShuffleEnabled";

VarTrack	g_vtPlayerYawInterpolateTime;
const char * const g_pszPlayerYawInterpolateTime = "PlayerYawInterpolateTime";

VarTrack	g_vtPlayerYawFreeRangeAngle;
const char * const g_pszPlayerYawFreeRangeAngle = "PlayerYawFreeRangeAngle";

VarTrack	g_vtPlayerYawRotateAngle;
const char * const g_pszPlayerYawRotateAngle = "PlayerYawRotateAngle";

VarTrack	g_vtSprintWalkSpeedDelta;
const char * const g_pszSprintWalkSpeedDelta = "SprintWalkSpeedDelta";

VarTrack	g_vtSprintDrainSpeed;
const char * const g_pszSprintDrainSpeed = "SprintDrainSpeed";

VarTrack	g_vtSprintRecoverSpeed;
const char * const g_pszSprintRecoverSpeed = "SprintRecoverSpeed";

VarTrack	g_vtSprintValidStartRange;
const char * const g_pszSprintValidStartRange = "SprintValidStartRange";

VarTrack	g_vtPersonalSlipMax;
const char * const g_pszPersonalSlipMax = "PersonalSlipMax";

VarTrack	g_vtPersonalSlipMin;
const char * const g_pszPersonalSlipMin = "PersonalSlipMin";

VarTrack	g_vtPlayerLeashSlipMax;
const char * const g_pszPlayerLeashSlipMax = "PlayerLeashSlipMax";

VarTrack	g_vtPlayerLeashSlipMin;
const char * const g_pszPlayerLeashSlipMin = "PlayerLeashSlipMin";

VarTrack	g_vtPostureDownTime;
const char * const g_pszPostureDownTime = "PostureDownTime";

VarTrack	g_vtPostureUpTime;
const char * const g_pszPostureUpTime = "PostureUpTime";

VarTrack	g_vtTestFireRecoil;
const char * const g_pszTestFireRecoil = "TestFireRecoil";

static VarTrack g_vtSurfaceSwimHeightOffset;
static const char * const g_pszSurfaceSwimHeight = "SurfaceSwimHeight";

bool g_bJumpRequested  = false;
bool g_bJumpFromLadder  = false;

extern VarTrack g_vtVehicleFallDamageMinHeight;
extern VarTrack g_vtVehicleFallDamageMaxHeight;

extern ObjectDetector g_iFocusObjectDetector;

CMoveMgr* g_pMoveMgr = NULL;

// ----------------------------------------------------------------------- //

void CMoveMgr::PlayerLeashFn( int argc, char** argv )
{
	// Make sure we've got enough parameters for this
	if( argc != 2 )
	{
		g_pMoveMgr->m_vLeashParameters.x = 0.0f;
		g_pMoveMgr->m_vLeashParameters.y = 0.0f;

		g_pLTClient->CPrint( "Player leash disabled!" );
		return;
	}

	// Get the position of the leash
	HOBJECT hMoveObject = g_pMoveMgr->GetObject();
	g_pLTClient->GetObjectPos( hMoveObject, &( g_pMoveMgr->m_vLeashPosition ) );

	// Setup the leash parameters
	g_pMoveMgr->m_vLeashParameters.x = ( float )atof( argv[ 0 ] );
	g_pMoveMgr->m_vLeashParameters.y = ( float )atof( argv[ 1 ] );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::CMoveMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMoveMgr::CMoveMgr()
{
	g_pMoveMgr = this;

	m_vWantedDims.Init(1, 1, 1);

    m_hObject = NULL;

	m_bLoading = false;
	m_vSavedVel.Init();

	m_vTotalCurrent.Init();
	m_fTotalViscosity = 0.0f;

	m_vGroundNormal.Init();

	// Always have this...
	m_pVehicleMgr = debug_new(CVehicleMgr);

	InitWorldData();

	m_bJumped	= false;
	m_bSwimJumped = false;
    m_bRunLock	= false;

	m_bGravityOverride = false;

	m_bWasMovementEncoded = false;

	m_vLastPos.Init();
	m_vLastVel.Init();

	m_vLeashPosition.Init();
	m_vLeashParameters.Init();

	m_fYawSnapShot = 0.0f;
	m_bInterpolateYaw = false;
	m_bSeperateCameraFromBody = false;

	m_fSprintBoostPercent = 1.0f;
	m_bSprintBoostEnabled = false;

	m_bWaterAffectsSpeed = true;

	m_bHasSavedEncodedTransform = false;
	m_rtSavedEncodedTransform.Init( );

	m_bWasSlideKicking = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::~CMoveMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CMoveMgr::~CMoveMgr()
{
	Term( );
}

void CMoveMgr::Term()
{
	g_pMoveMgr = NULL;

	TermLevel();

	if (m_pVehicleMgr)
	{
		debug_delete(m_pVehicleMgr);
		m_pVehicleMgr = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::Init
//
//	PURPOSE:	Initialize mgr
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::Init()
{
	ClientDB& ClientDatabase = ClientDB::Instance();
	HRECORD hSharedRecord = ClientDatabase.GetClientSharedRecord();
	HRECORD hPlayerMovementRecord = ClientDatabase.GetPlayerMovementRecord( );

	TestFireTimer.SetEngineTimer( SimulationTimer::Instance());
	TestFireRate.SetEngineTimer( SimulationTimer::Instance());

	m_PostureDownTime.SetEngineTimer( RealTimeTimer::Instance( ));
	m_PostureUpTime.SetEngineTimer( RealTimeTimer::Instance( ));

	m_CV_SpectatorSpeedMul.Init(g_pLTClient, "SpectatorSpeedMul", NULL, 2.0f);
	g_vtFallDamageDebug.Init(g_pLTClient, g_pszFallDamageDebug, NULL, 0.0f);

	g_vtPlayerGravity.Init(g_pLTClient, g_pszPlayerGravity, NULL, DEFAULT_PLAYER_GRAVITY);

	g_vtSlideToStopTime.Init(g_pLTClient, g_pszSlideToStopTime, NULL, ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszSlideToStopTime ) * 0.001f );

	g_vtMaxPushYVelocity.Init(g_pLTClient, g_pszPusherMaxYVelocity, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszPusherMaxYVelocity ));

	g_vtPlayerYawShuffleEnabled.Init( g_pLTClient, g_pszPlayerYawShuffleEnabled, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszPlayerYawShuffleEnabled ));
	g_vtPlayerYawInterpolateTime.Init( g_pLTClient, g_pszPlayerYawInterpolateTime, NULL, ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszPlayerYawInterpolateTime ) * 0.001f );
	g_vtPlayerYawFreeRangeAngle.Init( g_pLTClient, g_pszPlayerYawFreeRangeAngle, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszPlayerYawFreeRangeAngle ));
	g_vtPlayerYawRotateAngle.Init( g_pLTClient, g_pszPlayerYawRotateAngle, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszPlayerYawRotateAngle ));

	g_vtSprintWalkSpeedDelta.Init( g_pLTClient, g_pszSprintWalkSpeedDelta, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszSprintWalkSpeedDelta ));
	g_vtSprintDrainSpeed.Init( g_pLTClient, g_pszSprintDrainSpeed, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszSprintDrainSpeed ));
	g_vtSprintRecoverSpeed.Init( g_pLTClient, g_pszSprintRecoverSpeed, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszSprintRecoverSpeed ));
	g_vtSprintValidStartRange.Init( g_pLTClient, g_pszSprintValidStartRange, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszSprintValidStartRange ));

	g_vtPersonalSlipMax.Init( g_pLTClient, g_pszPersonalSlipMax, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszPersonalSlipMax ));
	g_vtPersonalSlipMin.Init( g_pLTClient, g_pszPersonalSlipMin, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszPersonalSlipMin ));

	g_vtPlayerLeashSlipMax.Init( g_pLTClient, g_pszPlayerLeashSlipMax, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszPlayerLeashSlipMax ));
	g_vtPlayerLeashSlipMin.Init( g_pLTClient, g_pszPlayerLeashSlipMin, NULL, ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszPlayerLeashSlipMin ));

	g_vtPostureDownTime.Init( g_pLTClient, g_pszPostureDownTime, NULL, ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszPostureDownTime ) * 0.001f );
	g_vtPostureUpTime.Init( g_pLTClient, g_pszPostureUpTime, NULL, ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszPostureUpTime ) * 0.001f );

	g_vtTestFireRecoil.Init( g_pLTClient, g_pszTestFireRecoil, NULL, 0.0f );

	g_vtSurfaceSwimHeightOffset.Init( g_pLTClient, g_pszSurfaceSwimHeight, NULL, 0.5f );

	g_pLTClient->RegisterConsoleProgram( "PlayerLeash", PlayerLeashFn );

	m_bWaterAffectsSpeed = ClientDatabase.GetBool( hSharedRecord, "WaterAffectsSpeed", m_bWaterAffectsSpeed );

	m_DamageTimer.SetEngineTimer( ObjectContextTimer( g_pMoveMgr->GetServerObject( )));
	m_fDamageMoveMultiplier = 1.0f;

 	// Init some defaults.  These should NEVER get used because we don't
	// have our object until the server sends the physics update.

	m_fSwimVel			= 0.0f;
	m_fWalkVel			= 280.0f;
	m_fRunVel			= 400.0f;
	m_fJumpVel			= 550.0f;
	m_fCrawlVel			= 200.0f;
	m_fMoveMultiplier	= 1.0f;

	//force idle at 1/4 of crawl vel
	m_fForceIdleVel = (m_fMoveMultiplier * m_fCrawlVel * 0.25f); 

	m_pVehicleMgr->Init();

	// Init world specific data members...

	InitWorldData();

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile)
	{
		SetRunLock(pProfile->m_bAlwaysRun);
	}

	g_pLTClient->RegisterConsoleProgram("TestFire2", TestFire2Fn);

	g_pLTClient->RegisterConsoleProgram("TestFire", TestFireFn);
	g_pLTClient->RegisterConsoleProgram("TestSAFire", TestSAFireFn);
	g_pLTClient->RegisterConsoleProgram("TestGrenade", TestGrenadeFn);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::InitWorldData
//
//	PURPOSE:	Initialize our data members that are specific to the
//				current world.
//
// ----------------------------------------------------------------------- //

void CMoveMgr::InitWorldData()
{
	m_pVehicleMgr->InitWorldData();

	m_dwControlFlags		= 0;
	m_dwLastControlFlags	= 0;

	m_fGravity				= DEFAULT_PLAYER_GRAVITY;

	m_Pushers.Clear();
    
	m_eBodyContainerCode	= CC_NO_CONTAINER;
    m_bBodyInLiquid         = false;
    m_bOnGround             = true;
    m_bOnLift               = false;
    m_bFalling              = false;
    m_bAllowMovement        = true;
	m_eStandingOnSurface	= ST_UNKNOWN;
	m_eBodyContainerSurface	= ST_UNKNOWN;
    m_bSwimmingOnSurface    = false;
    m_bSwimmingJump         = false;
    m_bCanSwimJump          = false;
	m_hStandingOnPoly		= INVALID_HPOLY;

    m_bDuckLock             = false;


	m_fBaseMoveAccel		= 0.0f;
	m_fMoveAccelMultiplier	= 1.0f;
	m_fJumpMultiplier		= 1.0f;

    m_eCurContainerCode		= CC_NO_CONTAINER;
	m_eLastContainerCode	= CC_NO_CONTAINER;
	m_nContainers			= 0;

    m_bFirstAniUpdate       = true;

    m_pCharFX               = NULL;

	m_bForceToServerPos		= true;

	// Don't overwrite our last ground pos if loading a game...
	
	if( !m_bLoading )
	{
		m_fLastOnGroundY	= MIN_ONGROUND_Y;
	}

	m_hStandingOnRigidBody = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnEnterWorld
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CMoveMgr::OnEnterWorld()
{
	InitWorldData();

	if( m_pVehicleMgr )
		m_pVehicleMgr->OnEnterWorld();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnExitWorld
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CMoveMgr::OnExitWorld()
{
	if( m_pVehicleMgr )
		m_pVehicleMgr->OnExitWorld();

	// Clean up any level specific shiznit...

	TermLevel();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::TermLevel
//
//	PURPOSE:	Terminate any level specific stuff
//
// ----------------------------------------------------------------------- //

void CMoveMgr::TermLevel()
{
	for (LTListIter<Pusher*> itCur=m_Pushers.Begin(); itCur != m_Pushers.End(); )
	{
		LTListIter<Pusher*> itNext = itCur.GetNext();
		Pusher::GetBank()->Delete(*itCur);
		itCur = itNext;
	}
	m_Pushers.Clear();

	if( m_pVehicleMgr )
		m_pVehicleMgr->TermLevel();

	m_DamageTimer.Stop();
	m_fDamageMoveMultiplier = 1.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateMouseStrafeFlags
//
//	PURPOSE:	Update the mouse strafe flags
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateMouseStrafeFlags(const LTVector2 &vAxisOffsets)
{
	if (vAxisOffsets.x < 0.0f)
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}
	else if (vAxisOffsets.x > 0.0f)
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if (vAxisOffsets.y < 0.0f)
	{
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}
	else if (vAxisOffsets.y > 0.0f)
	{
		m_dwControlFlags |= BC_CFLG_REVERSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateControlFlags
//
//	PURPOSE:	Update the control flags
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateControlFlags()
{
	// Clear control flags...

	m_dwLastControlFlags = m_dwControlFlags;
	m_dwControlFlags = 0;

	if( !m_hObject )
		return;

	UpdateNormalControlFlags();
	
	// Set the lean flags...

	m_dwControlFlags |= g_pPlayerMgr->GetLeanMgr()->GetControlFlags();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateNormalControlFlags
//
//	PURPOSE:	Update the normal physics model control flags
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateNormalControlFlags()
{
	// Set the idle movement properties first, these will get overriden if the player is actually moving...
	EnumAnimProp eLastMovement = CPlayerBodyMgr::Instance( ).GetAnimProp(kAPG_Movement);

	CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, kAP_MOV_Idle );
	CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_None );

	EnumAnimProp eLastPosture = CPlayerBodyMgr::Instance( ).GetLastAnimProp( kAPG_Posture );
	CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Posture, eLastPosture );

	if(     !g_pInterfaceMgr->AllowCameraMovement() 
		|| 	g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Cinematic )
	{
		// Don't clear the duck flag...
		m_dwControlFlags |= (m_dwLastControlFlags & BC_CFLG_DUCK);
		return;
	}

	// Tells us our feet are in a fixed position.
	bool bFeetFixedPosition = false;
	bool bCanCrouch = true;
	bool bForceForward = false;
	bool bAllowStrafe = true;
	bool bAllowJump = true;

	// Disable certain movement when we have a goto node...
	if( g_pPlayerMgr->GetPlayerNodeGoto( ))
	{
		// Force forward movement until the node position is reached...
		bForceForward = !g_pPlayerMgr->IsAtNodePosition( );
		bAllowStrafe = bAllowJump = !g_pPlayerMgr->GetPlayerNodeGoto( );

		// Hold player in place once they reach the node...
		bFeetFixedPosition = g_pPlayerMgr->IsAtNodePosition( );
	}
		
	if( m_pVehicleMgr->IsVehiclePhysics())
	{
		// Get the playerlurefx object.
		PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( m_pVehicleMgr->GetPlayerLureId( ));
		if( !pPlayerLureFX )
		{
			// It may not be on the client yet.  Keep polling for it.
			return;
		}

		bFeetFixedPosition = true;
		bCanCrouch = pPlayerLureFX->GetAllowCrouch( );
	}

	// Check if the PlayBody will allow crouching...
	bCanCrouch &= CPlayerBodyMgr::Instance( ).CanCrouch( );

	//if spectating first person, you can't do any movement.
	if( g_pPlayerMgr->GetSpectatorMode( ) == eSpectatorMode_Follow || 
		g_pPlayerMgr->GetSpectatorMode( ) == eSpectatorMode_Tracking ||
		g_pPlayerMgr->GetSpectatorMode( ) == eSpectatorMode_Fixed )
		bFeetFixedPosition = true;

	if( LadderMgr::Instance().IsClimbing() || SpecialMoveMgr::Instance().IsActive() || g_pPlayerMgr->IsOperatingTurret( ))
	{
		bFeetFixedPosition = true;
		bCanCrouch = false;
		CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Posture, kAP_POS_Stand );
	}

	// If we're doing mouse strafing, hook us up...

	if ( !bFeetFixedPosition && g_pPlayerMgr->IsMouseStrafing())
	{
		LTVector2 vAxisOffsets;
		vAxisOffsets.x = CBindMgr::GetSingleton().GetExtremalCommandValue(COMMAND_ID_YAW);
		vAxisOffsets.y = CBindMgr::GetSingleton().GetExtremalCommandValue(COMMAND_ID_PITCH);

		UpdateMouseStrafeFlags(vAxisOffsets);
	}

	// Get the axis movement offsets
	LTVector2 vAxisMovement(
		CBindMgr::GetSingleton().GetExtremalCommandValue(COMMAND_ID_STRAFE_AXIS),
		CBindMgr::GetSingleton().GetExtremalCommandValue(COMMAND_ID_FORWARD_AXIS));

	// Determine what commands are currently on...

    if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_YAW_NEG) && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_YAW_POS))
	{
		m_dwControlFlags |= BC_CFLG_LEFT;
		CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_Left );
	}
	else if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_YAW_POS) && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_YAW_NEG))
	{
		m_dwControlFlags |= BC_CFLG_RIGHT;
		CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_Right );
	}

	//if you are climbing, also check for forward/backward movement, but nothing else
	if (LadderMgr::Instance().IsClimbing())
	{
		if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_JUMP)) 
		{
			if (!LadderMgr::Instance( ).IsDismounting( ))
			{
				g_bJumpFromLadder = true;
			}
			
		}
		else if ((CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_FORWARD) || (vAxisMovement.y > 0.0f)) && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_REVERSE))
		{
			m_dwControlFlags |= BC_CFLG_FORWARD;
		}
		else if ((CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_REVERSE) || (vAxisMovement.y < 0.0f)) && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_FORWARD))
		{
			m_dwControlFlags |= BC_CFLG_REVERSE;
		}
		return;
	}


	// Use the player's user flags instead of checking this flag directly
	// (this insures that the camera will be moved accurately...)

	if ( bCanCrouch && ( CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_DUCK) || m_bDuckLock))
	{
		m_dwControlFlags |= BC_CFLG_DUCK;
	}

	// Force the duck flag back on if we can't stand up in our current position
	if ( bCanCrouch && (((m_dwControlFlags & BC_CFLG_DUCK) == 0) && ((m_dwLastControlFlags & BC_CFLG_DUCK) != 0)))
	{
		if (!CanStandUp())
			m_dwControlFlags |= BC_CFLG_DUCK;
	}

	// Force the duck flag back on if we just finished a slide kick but can't stand up here
	bool bSlideKick = CPlayerBodyMgr::Instance().IsLocked(kAPG_Action,kAP_ACT_SlideKick);
	if ( bCanCrouch && !bSlideKick && m_bWasSlideKicking)
	{
		if (!CanStandUp())
			m_dwControlFlags |= BC_CFLG_DUCK;
	}
	//save off whether we were slidekicking last update
	m_bWasSlideKicking = bSlideKick;


	// Determine the posture...
	if( bCanCrouch && ( m_dwControlFlags & BC_CFLG_DUCK ) )
	{
		CPlayerBodyMgr::Instance().SetAnimProp( kAPG_Posture, kAP_POS_Crouch );
	}
	else
	{
		// See if we have a target... and use that anim prop instead if we do...
		CPlayerBodyMgr::Instance().SetAnimProp( kAPG_Posture, g_iFocusObjectDetector.GetObject() ? kAP_POS_StandAimed : kAP_POS_Stand );
	}

	if( !bFeetFixedPosition )
	{
        // Only process jump and run if we aren't ducking and aren't leaning...
		if (!(m_dwControlFlags & BC_CFLG_DUCK) && !g_pPlayerMgr->GetLeanMgr()->IsLeaning() && !AimMgr::Instance().IsAiming())
		{
#if defined(PROJECT_DARK)
			EnableSprintBoost( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_RUN ) );

			if( m_bRunLock )
			{
				m_dwControlFlags |= BC_CFLG_RUN;
			}
#elif defined(PROJECT_FEAR)
			// on FEAR the run command is actually a walk, instead of speeding 
			// up the player it needs to slow him down
			if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_RUN ) )
			{
				m_dwControlFlags &= ~BC_CFLG_RUN;
			}
			else if( m_bRunLock )
			{
				m_dwControlFlags |= BC_CFLG_RUN;
			}
#else
			if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_RUN ) )
			{
				m_dwControlFlags |= BC_CFLG_RUN;
			}

			if( m_bRunLock )
			{
				m_dwControlFlags |= BC_CFLG_RUN;
			}
#endif

			if( bAllowJump && CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_JUMP) && g_pPlayerMgr->IsFinishedDucking( ) )
			{
				m_dwControlFlags |= BC_CFLG_JUMP;
			}
		}

		// Process the directions...
        if( bForceForward || (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_FORWARD) || (vAxisMovement.y > 0.0f)) && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_REVERSE))
		{
			m_dwControlFlags |= BC_CFLG_FORWARD;
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_Forward );
		}
		else if ((CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_REVERSE) || (vAxisMovement.y < 0.0f)) && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_FORWARD))
		{
			m_dwControlFlags |= BC_CFLG_REVERSE;
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_Backward );
		}
		
		if( bAllowStrafe )
		{
			if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_STRAFE))
			{
				m_dwControlFlags |= BC_CFLG_STRAFE;
			}

			if ((CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_STRAFE_RIGHT) || (vAxisMovement.x > 0.0f)) && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_STRAFE_LEFT))
			{
				m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_Right );
			}
			else if ((CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_STRAFE_LEFT)  || (vAxisMovement.x < 0.0f)) && !CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_STRAFE_RIGHT))
			{
				m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_Left );
			}

			// Check for the diagonal directions...
			if( m_dwControlFlags & BC_CFLG_FORWARD )
			{
				if( m_dwControlFlags & (BC_CFLG_STRAFE_LEFT | BC_CFLG_LEFT) )
				{
					CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_ForeLeft );
				}
				else if( m_dwControlFlags & (BC_CFLG_STRAFE_RIGHT | BC_CFLG_RIGHT) )
				{
					CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_ForeRight );
				}
			}
			else if( m_dwControlFlags & BC_CFLG_REVERSE )
			{
				if( m_dwControlFlags & (BC_CFLG_STRAFE_LEFT | BC_CFLG_LEFT) )
				{
					CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_BackLeft );
				}
				else if( m_dwControlFlags & (BC_CFLG_STRAFE_RIGHT | BC_CFLG_RIGHT) )
				{
					CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_MDIR_BackRight );
				}
			}
		}

		// See if the player is trying to go in opposite directions...
		if( ((m_dwControlFlags & BC_CFLG_REVERSE) && (m_dwControlFlags & BC_CFLG_FORWARD)) ||
			((m_dwControlFlags & (BC_CFLG_STRAFE_RIGHT | BC_CFLG_RIGHT)) && m_dwControlFlags & (BC_CFLG_STRAFE_LEFT | BC_CFLG_LEFT)) )
		{
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_None );
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, kAP_MOV_Idle );
		}

		// Only set running or walking if actually moving in a direction...
		if( CPlayerBodyMgr::Instance( ).GetAnimProp( kAPG_MovementDir ) != kAP_None )
		{
			EnumAnimProp eMovement = ((m_dwControlFlags & BC_CFLG_RUN) ? kAP_MOV_Run : kAP_MOV_Walk);

			//if we've accelerated up to normal speed, yet we didn't move very far, force us into an idle anim
			if (GetMovementPercent() > 0.9f && (m_vLastVel.Mag() < m_fForceIdleVel))
			{
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_MovementDir, kAP_None );
				eMovement = kAP_MOV_Idle;
			}

			// If underwater set swimming movement...
			if( g_pPlayerMgr->IsUnderwater( ) && g_pPlayerMgr->IsSwimmingAllowed( ))
			{
				eMovement = kAP_MOV_Swim;
			}

			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, eMovement );
		}
		else if( g_pPlayerMgr->IsUnderwater( ) && g_pPlayerMgr->IsSwimmingAllowed( ))
		{
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, kAP_MOV_Swim );
		}
	}

	static float fFireTime = 0.0f;
	if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_FIRING))
	{
//		if (fFireTime == 0.0f)
//		{
//			fFireTime = RealTimeTimer::Instance( ).GetTimerAccumulatedS();
//			DebugCPrint(0,"Fire key down: %0.3f", fFireTime);
//		}
		m_dwControlFlags |= BC_CFLG_FIRING;
	}
	else
	{
//		if (fFireTime > 0.0f)
//		{
//			DebugCPrint(0,"Fire key up: %0.3f", RealTimeTimer::Instance( ).GetTimerAccumulatedS());
//			fFireTime = 0.0f;
//		}
	}

	// Nothing more to check for turrets...
	if( g_pPlayerMgr->IsOperatingTurret( ))
		return;

	if (GetConsoleBool("TestFiring",false)) 
	{
		if (!TestFireTimer.IsStarted())
		{
			if (g_vtTestFireRecoil.GetFloat() < 1.0f)
			{
				WriteConsoleInt("CamRecoilKick",0);
			}
			TestFireTimer.Start(fTestFireDuration);
			m_dwControlFlags |= BC_CFLG_FIRING;
			DebugCPrint(0,"======================\nStart Test Firing");
			fLastTestFireTime = 0.0f;
		}
		else if (TestFireTimer.IsTimedOut())
		{
			if (g_vtTestFireRecoil.GetFloat() < 1.0f)
			{
				WriteConsoleInt("CamRecoilKick",-1);
			}
			TestFireTimer.Stop();
			WriteConsoleBool("TestFiring",false);

			float fROF = float(nTestFireShotsFired) / fTestFireDuration;
			DebugCPrint(0,"End Test Firing : %d shots, %0.2f RPS\n======================",nTestFireShotsFired,fROF);
		}
		else
		{
			m_dwControlFlags |= BC_CFLG_FIRING;
		}
	}


	if (GetConsoleBool("TestSAFiring",false)) 
	{
		static bool bFired = false;
		if (!TestFireTimer.IsStarted())
		{
			if (g_vtTestFireRecoil.GetFloat() < 1.0f)
			{
				WriteConsoleInt("CamRecoilKick",0);
			}

			TestFireTimer.Start(fTestFireDuration);
			TestFireRate.Start(fTestFireRate);
			m_dwControlFlags |= BC_CFLG_FIRING;
			g_pPlayerMgr->OnCommandOn(COMMAND_ID_FIRING);
			
			uint32 n = uint32(1.0f/fTestFireRate);
			DebugCPrint(0,"======================\nStart Test Firing: %d clicks per second", n);
			fLastTestFireTime = 0.0f;
		}
		else if (TestFireTimer.IsTimedOut())
		{
			if (g_vtTestFireRecoil.GetFloat() < 1.0f)
			{
				WriteConsoleInt("CamRecoilKick",-1);
			}

			TestFireTimer.Stop();
			WriteConsoleBool("TestSAFiring",false);
			WriteConsoleBool("ClientWeaponStateTracking",false);
			float fROF = float(nTestFireShotsFired) / fTestFireDuration;
			DebugCPrint(0,"End Test Firing : %d shots, %0.2f RPS\n======================",nTestFireShotsFired,fROF);
		}
		else if (TestFireRate.IsStarted() && TestFireRate.IsTimedOut())
		{
			TestFireRate.Start(fTestFireRate);
			g_pPlayerMgr->OnCommandOn(COMMAND_ID_FIRING);
			bFired = true;
			m_dwControlFlags |= BC_CFLG_FIRING;
		}
		else if (bFired)
		{
			g_pPlayerMgr->OnCommandOff(COMMAND_ID_FIRING);
			bFired = false;
		}
	}

	if (GetConsoleBool("TestGrenadeThrow",false)) 
	{
		static bool bFired = false;
		if (!TestFireTimer.IsStarted())
		{
			if (g_vtTestFireRecoil.GetFloat() < 1.0f)
			{
				WriteConsoleInt("CamRecoilKick",0);
			}

			TestFireTimer.Start(fTestFireDuration);
			TestFireRate.Start(fTestFireRate);
			m_dwControlFlags |= BC_CFLG_GRENADE;
			g_pPlayerMgr->OnCommandOn(COMMAND_ID_THROW_GRENADE);

			uint32 n = uint32(1.0f/fTestFireRate);
			DebugCPrint(0,"======================\nStart Test Grenade Throw: %d clicks per second", n);
			fLastTestFireTime = 0.0f;
		}
		else if (TestFireTimer.IsTimedOut())
		{
			if (g_vtTestFireRecoil.GetFloat() < 1.0f)
			{
				WriteConsoleInt("CamRecoilKick",-1);
			}

			TestFireTimer.Stop();
			WriteConsoleBool("TestGrenadeThrow",false);
			WriteConsoleBool("ClientWeaponStateTracking",false);
			float fROF = float(nTestGrenadesThrown	) / fTestFireDuration;
			DebugCPrint(0,"End Test Firing : %d shots, %0.2f RPS\n======================",nTestGrenadesThrown,fROF);
		}
		else if (TestFireRate.IsStarted() && TestFireRate.IsTimedOut())
		{
			TestFireRate.Start(fTestFireRate);
			g_pPlayerMgr->OnCommandOn(COMMAND_ID_THROW_GRENADE);
			bFired = true;
			m_dwControlFlags |= BC_CFLG_GRENADE;
		}
		else if (bFired)
		{
			g_pPlayerMgr->OnCommandOff(BC_CFLG_GRENADE);
			bFired = false;
		}
	}


	if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_ALT_FIRING))
	{
		m_dwControlFlags |= BC_CFLG_ALT_FIRING;
	}

	if( CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_JumpIdleKick ) ||
		CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_JumpRunKick ))
	{
		if( m_dwControlFlags & BC_CFLG_JUMP )
		{
			// Don't allow jumping at any point during alt-fire animation...
			m_dwControlFlags &= ~BC_CFLG_JUMP;

			// Register the jump key was actually pressed with the PlaeyrBody...
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, kAP_MOV_Jump );
		}

		// Clear any previously requested jump....
		g_bJumpRequested = false;
	}

	if (CPlayerBodyMgr::Instance( ).IsEnabled())
	{
		if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_THROW_GRENADE))
		{
			m_dwControlFlags |= BC_CFLG_GRENADE;
		}
	}
	else
	{
		// GRENADE PROTOTYPE
		if (CBindMgr::GetSingleton().IsCommandOn(COMMAND_ID_THROW_GRENADE) && g_pPlayerMgr->GrenadeReady())
		{
			m_dwControlFlags |= BC_CFLG_FIRING;
		}
	}



	if( !bFeetFixedPosition )
	{
		// Check for strafe left and strafe right.
		if ((m_dwControlFlags & BC_CFLG_RIGHT) && (m_dwControlFlags & BC_CFLG_STRAFE))
		{
			m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
		}

		if ((m_dwControlFlags & BC_CFLG_LEFT) && (m_dwControlFlags & BC_CFLG_STRAFE))
		{
			m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
		}
	}

	if ( (m_dwControlFlags & BC_CFLG_FORWARD) ||
		 (m_dwControlFlags & BC_CFLG_REVERSE) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_LEFT) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) ||
		 (g_bJumpFromLadder || m_bJumped))
	{
		m_dwControlFlags |= BC_CFLG_MOVING;
	}

	if ((m_dwControlFlags & BC_CFLG_JUMP) &&
		!(m_dwLastControlFlags & BC_CFLG_JUMP) && 
		!g_bJumpFromLadder && !m_bJumped )
	{
        g_bJumpRequested = true;
	}

	if( (m_dwControlFlags & BC_CFLG_DUCK) && !(m_dwLastControlFlags & BC_CFLG_DUCK) )
	{
		m_PostureDownTime.Start( g_vtPostureDownTime.GetFloat( ));
	}

	if( !(m_dwControlFlags & BC_CFLG_DUCK) && (m_dwLastControlFlags & BC_CFLG_DUCK) )
	{
		m_PostureUpTime.Start( g_vtPostureUpTime.GetFloat( ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateContainerViscosity
//
//	PURPOSE:	Update our friction based on the containers we're in
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateContainerViscosity(CContainerInfo *pInfo)
{
	if (!pInfo || pInfo->m_bHidden) return;

	// Only allow the container to affect players in the correct physics model....

	if( pInfo->m_ePPhysicsModel != m_pVehicleMgr->GetPhysicsModel() )
		return;

	// Don't do viscosity dampening, if we're jumping out of water...

	if (m_bJumped && !IsHeadInLiquid()) return;


	// Update the total viscosity for the frame...

	m_fTotalViscosity += pInfo->m_fViscosity;


	// Do REAL viscosity dampening (i.e., actually change our velocity)...

    LTVector vVel = GetVelocity();
    LTVector vCurVel = vVel;

	if (pInfo->m_fViscosity > 0.0f && vCurVel.Mag() > 1.0f)
	{
        LTVector vDir = vCurVel.GetUnit();

        float fAdjust = MAX_CONTAINER_VISCOSITY * pInfo->m_fViscosity;
		fAdjust *= ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );

		vVel = (vDir * fAdjust);

		if (vVel.MagSqr() < vCurVel.MagSqr())
		{
			vVel = vCurVel - vVel;
		}
		else
		{
			vVel.Init();
		}

		g_pPhysicsLT->SetVelocity(m_hObject, vVel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateContainerGravity
//
//	PURPOSE:	Update our gravity based on the containers we're in
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateContainerGravity( CContainerInfo *pInfo )
{
	// Only do this for Gavity volumes for now....

	if( !pInfo || pInfo->m_bHidden || pInfo->m_ContainerCode != CC_GRAVITY ) return;

	// Only allow the container to affect players in the correct physics model....

	if( pInfo->m_ePPhysicsModel != m_pVehicleMgr->GetPhysicsModel() )
		return;

	m_bGravityOverride = true;
	m_fTotalContainerGravity += pInfo->m_fGravity;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::CanSave
//
//	PURPOSE:	Determines whether or not it's safe to save this object
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::CanSave() const
{ 
	return ( !g_pPlayerMgr->GetPlayerNodeGoto( ) && m_pVehicleMgr->CanSave( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateInLiquid
//
//	PURPOSE:	Update being in liquid
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateInLiquid(CContainerInfo *pInfo)
{
    m_bBodyInLiquid = true;

	// Ladder physics takes priority over liquid...

	if (LadderMgr::Instance().IsClimbing()) return;
	if (SpecialMoveMgr::Instance().IsActive()) return;

    bool bHeadInLiquid = IsHeadInLiquid();

    LTVector vVel = GetVelocity();

    LTVector curAccel;
	g_pPhysicsLT->GetAcceleration(m_hObject, &curAccel);

	// Handle floating around on the surface...

	if (m_bSwimmingOnSurface)
	{
		bool bMoving = ((curAccel.Mag() > 0.01f) || (vVel.Mag() > 0.01f));

		// Disable gravity.
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_GRAVITY);

		if (bMoving)  // Turn off y acceleration and velocity
		{
			if (vVel.y > 0.0f || curAccel.y > 0.0f)
			{
				vVel.y = 0.0f;
				curAccel.y = 0.0f;
			}
		}
		else // Pull us down if we're not moving (fast enough)
		{
			curAccel.y += pInfo->m_fGravity;
		}

		// Find the surface height so that we will stay at that height.
		if( pInfo->m_hObject != NULL )
		{
			LTVector vContainerCenter;
			g_pLTClient->GetObjectPos(pInfo->m_hObject, &vContainerCenter);

			LTVector vContainerDims;
			g_pPhysicsLT->GetObjectDims(pInfo->m_hObject, &vContainerDims);

			LTVector vPlayerDims;
			g_pPhysicsLT->GetObjectDims(m_hObject, &vPlayerDims);

			const float fSurfaceSwimHeight = vContainerCenter.y + vContainerDims.y - g_vtSurfaceSwimHeightOffset.GetFloat()*vPlayerDims.y;

			LTVector vPlayerPos;
			g_pLTClient->GetObjectPos( m_hObject, &vPlayerPos );

			if( vPlayerPos.y > fSurfaceSwimHeight )
			{
				vPlayerPos.y = fSurfaceSwimHeight; 
				g_pPhysicsLT->MoveObject(m_hObject, vPlayerPos, 0);
			}
		}
	}
	else if (bHeadInLiquid)
	{
		// Disable gravity.
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_GRAVITY);

		curAccel.y += pInfo->m_fGravity;
	}

	g_pPhysicsLT->SetVelocity(m_hObject, vVel);
	g_pPhysicsLT->SetAcceleration(m_hObject, curAccel);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateOnGround
//
//	PURPOSE:	Update our "on ground" status
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateOnGround()
{
	// Lets see if we are in the ground or in the air.
	CollisionInfo Info;
	g_pPhysicsLT->GetStandingOn(m_hObject, &Info);

	// Check if we're standing on a rigid body object that has moved.  If so, we need to detach ourselves
	// from it.  We have to do it here rather than when it actually moved because we want to use 
	// the standing on information.
	if( m_hStandingOnRigidBody && Info.m_hObject == m_hStandingOnRigidBody )
	{
		// Detach what we're standing on so we'll fall correctly off of physically
		// simulated objects...
		g_pLTClient->DetachObjectStanding(m_hObject);

		// Clear standing on object now that we are detached.  If we re-attach and it moves, this
		// will get set again.
		m_hStandingOnRigidBody = NULL;
	}

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hObject, &vPos);

	// Clear surface we're standing on...

    m_bOnLift            = false;
	m_hStandingOnPoly    = INVALID_HPOLY;
    m_bOnGround          = false;

	if (Info.m_hObject)
	{
		m_bOnGround = true;

		m_eStandingOnSurface = ST_UNKNOWN;
		if (Info.m_hPoly != INVALID_HPOLY)
		{
			m_hStandingOnPoly    = Info.m_hPoly;
			m_eStandingOnSurface = GetSurfaceType(Info.m_hPoly);
		}
		else // Get the texture flags from the object...
		{
			m_eStandingOnSurface = GetSurfaceType(Info.m_hObject);
		}


		// See if we are standing on a lift (i.e., on an object that
		// may move)...

        uint32 dwUserFlags;
        g_pCommonLT->GetObjectFlags(Info.m_hObject, OFT_User, dwUserFlags);

		if (dwUserFlags & USRFLG_MOVEABLE)
		{
            m_bOnLift = true;
		}


		// See if we fell...
        float fDistFell = m_fLastOnGroundY - vPos.y;
		m_fLastOnGroundY = vPos.y;

		if (fDistFell > 1.0f || m_bFalling)
		{
			HandleFallLand(fDistFell, m_eStandingOnSurface);
		}

		m_bJumped = m_bSwimJumped = false;
		m_bFalling = false;

		// Save the normal of the surface we are standing on...

		m_vGroundNormal = Info.m_Plane.m_Normal;

		// Make sure we fall down step slopes...

		if (m_vGroundNormal.y < 0.707)
		{
		   m_bOnGround = false;
		}

		// Don't allow standing on characters
		if (dwUserFlags & USRFLG_CHARACTER)
		{
			m_bOnGround = false;
		}
	}


	// Cases when we can't be on the ground...

	if (m_bJumped || IsHeadInLiquid())
	{
		m_bOnGround = false;
	}

	if (!m_bOnGround)
	{
		if (g_pPlayerMgr->IsSpectating())
		{
			m_fLastOnGroundY = MIN_ONGROUND_Y;
		}
		else
		{
			// If we're moving up (i.e., jumping, on a lift, etc.), update
			// our last on ground y pos...

			if (vPos.y > m_fLastOnGroundY)
			{
				m_fLastOnGroundY = vPos.y;
			}
		}

		// See if we're done jumping...

		if (m_bJumped)
		{
			if (IsHeadInLiquid())
			{
                m_bJumped = m_bSwimJumped = false;
                UpdateStartMotion(true, NULL, NULL);
			}

			if( !CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Movement, kAP_MOV_Jump ))
			{
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, kAP_MOV_Fall );
			}
			else
			{
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, kAP_MOV_Jump );
			}
		}
	}

	// See if we just started falling (i.e., we're not in liquid, we didn't
	// jump, and there is a bit of distance between us and the ground)...

    bool bFreeMovement = (IsFreeMovement() || g_pPlayerMgr->IsSpectating());

	if (!bFreeMovement && !m_bJumped && !CanDoFootstep() && !m_bFalling && !(LadderMgr::Instance().IsClimbing()) && !(SpecialMoveMgr::Instance().IsActive()))
	{
        m_bFalling = true;

		// Tell the server we're falling...

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
		cMsg.Writeuint8(CP_MOTION_STATUS);
        cMsg.Writeuint8(MS_FALLING);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::CanDoFootstep
//
//	PURPOSE:	Can we make a footstep sound or footprint?
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::CanDoFootstep()
{
	// This is basically just the same as m_bOnGround, however, when going
	// up/down stairs often m_bOnGround is false...So...

	// No footsteps if standing on sky/invisible texture (this should
	// only happen in really special case levels)...

	if (m_eStandingOnSurface == ST_INVISIBLE || m_eStandingOnSurface == ST_SKY)
	{
        return false;
	}

	// NOTE the order of these trivial case returns is important...

    if (LadderMgr::Instance().IsClimbing())
	{
		if (LadderMgr::Instance().IsClimbing())
		{
			m_eStandingOnSurface = ST_LADDER;
		}
		return true;     // Even if underwater
	}
    if (IsHeadInLiquid()) return false;    // Even if on the ground
    if (m_bOnGround) return true;          // Even if we just jumped
    if (m_bJumped) return false;


	// This is sort of lame...but can't think of a better approach.  Cast
	// a ray down and see if we hit something...

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hObject, &vPos);

	IntersectQuery iQuery;
	IntersectInfo  iInfo;

	iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	iQuery.m_From  = vPos;
	iQuery.m_To	   = iQuery.m_From;
	iQuery.m_To.y -= 150.0f;

	// Don't hit ourself...

    HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), m_hObject, NULL};

	iQuery.m_FilterFn  = ObjListFilterFn;
	iQuery.m_pUserData = hFilterList;

	if (!g_pLTClient->IntersectSegment(iQuery, &iInfo))
	{
		m_eStandingOnSurface = ST_UNKNOWN;
        return false;
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::HandleFallLand
//
//	PURPOSE:	Handle landing
//
// ----------------------------------------------------------------------- //

void CMoveMgr::HandleFallLand(float fDistFell, SurfaceType eLandSurface)
{
	if (IsHeadInLiquid( ))
	{
		// Handle landing after jumping.
		if( m_bJumped )
		{
			m_bJumped = m_bSwimJumped = false;
			UpdateStartMotion(true, NULL, NULL );
		}
		return;
	}

	ClientDB& ClientDatabase = ClientDB::Instance();
	HRECORD hPlayerMovementRecord = ClientDatabase.GetPlayerMovementRecord( );

	float fMinLandHeight   = ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszCamLandMinHeight );
	float fDamageMinHeight = m_pVehicleMgr->IsVehiclePhysics() ? g_vtVehicleFallDamageMinHeight.GetFloat() : ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszFallDamageMinHeight );
	float fDamageMaxHeight = m_pVehicleMgr->IsVehiclePhysics() ? g_vtVehicleFallDamageMaxHeight.GetFloat() : ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszFallDamageMaxHeight );
	float fFallDamageMin   = ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszFallDamageMin );
	float fFallDamageMax   = ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszFallDamageMax );

	// Will be filled in with landing sound if needed.
	HRECORD hSound = NULL;

	if (fDistFell >= fMinLandHeight)
	{
		// Adjust camera...

		CameraDelta delta;
	
		delta.PosY.fVar		= ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszCamLandMoveDist );
		delta.PosY.fTime1	= ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszCamLandDownTime ) * 0.001f;
		delta.PosY.fTime2	= ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszCamLandUpTime ) * 0.001f;
		delta.PosY.eWave1	= Wave_SlowOff;
		delta.PosY.eWave2	= Wave_SlowOff;

		if (fDistFell >= fDamageMinHeight)
		{
			float fHeightRange = fDamageMaxHeight - fDamageMinHeight;
			float fDamageRange = fFallDamageMax - fFallDamageMin;
			float fDamage = 0.0f;
			if (fHeightRange > 0.0f)
			{
				fDamage = fFallDamageMin + fDamageRange * (fDistFell - fDamageMinHeight) / fHeightRange;
			}

			if (g_vtFallDamageDebug.GetFloat() > 0.0f)
			{
				DebugCPrint(0,"Fall Distance: %0.2f, Fall Damage: %0.2f",fDistFell,fDamage);
			}

			// Send damage message to server...

			// See if we're in a SafteyNet, if so, no damage...

			if (!g_pPlayerMgr->InSafetyNet())
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
				cMsg.Writeuint8(CP_DAMAGE);
				cMsg.Writeuint8(DT_CRUSH);
				cMsg.Writefloat(fDamage);
				cMsg.Writeuint8(0);
				cMsg.WriteObject(g_pLTClient->GetClientObject());
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			}

			// Tweak camera...
			delta.Roll.fVar		= DEG2RAD(ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszCamLandRollVal ));
			delta.Roll.fVar		= GetRandom(0, 1) == 1 ? -delta.Roll.fVar : delta.Roll.fVar;
			delta.Roll.fTime1	= ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszCamLandRollTime1 ) * 0.001f;
			delta.Roll.fTime2	= ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszCamLandRollTime2 ) * 0.001f;
			delta.Roll.eWave1	= Wave_SlowOff;
			delta.Roll.eWave2	= Wave_SlowOff;

			delta.Pitch.fVar	= DEG2RAD(ClientDatabase.GetFloat( hPlayerMovementRecord, g_pszCamLandPitchVal ));
			delta.Pitch.fTime1	= ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszCamLandPitchTime1 ) * 0.001f;
			delta.Pitch.fTime2	= ClientDatabase.GetInt32( hPlayerMovementRecord, g_pszCamLandPitchTime2 ) * 0.001f;
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;

		}
		else
		{
			if (g_vtFallDamageDebug.GetFloat() > 0.0f)
			{
				DebugCPrint(0,"Fall Distance: %0.2f, below minimum fall distance",fDistFell);
			}
		}

		g_pPlayerMgr->GetCameraOffsetMgr()->AddDelta(delta);

		// If we're on a vehicle, adjust the vehicle model...

		if (m_pVehicleMgr->IsVehiclePhysics())
		{
			CameraDelta delta;

			delta.PosY.fVar		= 0.1f;
			delta.PosY.fTime1	= 0.2f;
			delta.PosY.fTime2	= 0.5f;
			delta.PosY.eWave1	= Wave_SlowOff;
			delta.PosY.eWave2	= Wave_SlowOff;

			m_pVehicleMgr->GetModelOffsetMgr()->AddDelta(delta);
		}

		// Play land sound...
		// i've moved this outside of the   fDamageMinHeight because
		// we don't want the landing sound tied simply to damage
		// anymore. -- Terry
		bool bPlaySound = false;
		if (!IsHeadInLiquid())
		{
			bPlaySound = (m_bBodyInLiquid ? m_bOnGround : (m_eStandingOnSurface != ST_INVISIBLE));
		}

		if (bPlaySound)
		{
			int32 fallpercent;

			fallpercent = (int32)(fDistFell*100.0f/fDamageMaxHeight);
			hSound = GetLandingSound(eLandSurface, PPM_NORMAL, true, fallpercent );
			if (hSound)
			{
				g_pClientSoundMgr->PlayDBSoundLocal(hSound,SOUNDPRIORITY_PLAYER_HIGH );
			}
		}

		// See if we should play footsplash.  Only do this if we're not in watervolume or if
		// we're not head deep in water volume.  If we were head deep, then we wouldn't 
		// really be stepping on the water surface.
		CPolyGridFX* pPolyGridFX = NULL;
		if( !m_bBodyInLiquid || !IsHeadInLiquid())
		{
			LTVector vSplashPos;
			g_pLTClient->GetObjectPos( g_pMoveMgr->GetServerObject(), &vSplashPos );
			pPolyGridFX = CPolyGridFX::FindSplashInPolyGrid( GetServerObject(), vSplashPos );
			if( pPolyGridFX )
			{
				pPolyGridFX->DoPolyGridSplash( GetServerObject(), vSplashPos, 
					g_pModelsDB->GetSplashesJumpingShallowImpulse( m_pCharFX->GetModel( )));
			}
		}

		m_bJumped = m_bSwimJumped = false;
		UpdateStartMotion(true, hSound, pPolyGridFX );

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateMotion
//
//	PURPOSE:	Update our motion
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateMotion()
{
	// Reset to the server position if we didn't know where to go at creation time
	if (m_bForceToServerPos)
	{
		HOBJECT hClientObj = g_pLTClient->GetClientObject();
		if (hClientObj)
		{
			LTVector vServerPos;
			g_pLTClient->GetObjectPos(hClientObj, &vServerPos);
			g_pLTClient->SetObjectPos(m_hObject, vServerPos);
			m_bForceToServerPos = false;
		}
	}

	if (!m_bAllowMovement || !g_pDamageFXMgr->AllowMovement() )
	{
		// Clear accel/velocity...
	    LTVector vVec;
		g_pPhysicsLT->GetAcceleration(m_hObject, &vVec);

		vVec.x = vVec.z = 0.0f;
		if (vVec.y > 0.0f) vVec.y = 0.0f;

		g_pPhysicsLT->SetAcceleration(m_hObject, vVec);

		// Dampen velocity...
		g_pPhysicsLT->GetVelocity(m_hObject, &vVec);
		vVec.x *= 0.5f;
		vVec.z *= 0.5f;
		if (vVec.y > 0.0f) vVec.y = 0.0f;

		g_pPhysicsLT->SetVelocity(m_hObject, vVec);
		return;
	}

	// Make sure this gets reset...
	m_fTotalContainerGravity = 0.0f;
	m_bGravityOverride = false;

	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		m_pVehicleMgr->UpdateMotion();
	}
	else if( CPlayerBodyMgr::Instance( ).IsEnabled( ) && CPlayerBodyMgr::Instance( ).IsMovementEncoded( ) &&
			 (g_pPlayerMgr->GetPlayerNodeGoto( ) == NULL) )
	{
		UpdateMovementEncoding( );
		m_bWasMovementEncoded = true;
	}
	else
	{
		UpdateNormalMotion();
		m_bWasMovementEncoded = false;
	}

	if( !m_pVehicleMgr->IsVehiclePhysics( ))
	{
		// Clear the movement encoding every frame to prevent accidental accumulation...
		g_pModelLT->ResetMovementEncodingTransform( m_hObject );
	}

	// Cache the last known position...
	LTVector vPlayerPos;
	g_pLTClient->GetObjectPos( m_hObject, &vPlayerPos );
	float fTimeDelta = SimulationTimer::Instance().GetTimerElapsedS();
	m_vLastVel = (fTimeDelta > 0) ? ((vPlayerPos - m_vLastPos) / fTimeDelta) : LTVector(0.0f, 0.0f, 0.0f);
	m_vLastPos = vPlayerPos;

	m_PlayerRigidBody.Update( vPlayerPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateSound
//
//	PURPOSE:	Update our sounds
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateSound()
{
	if( m_pVehicleMgr->IsVehiclePhysics( ))
		m_pVehicleMgr->UpdateSound();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateNormalMotion
//
//	PURPOSE:	Update our normal motion
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateNormalMotion()
{
	// Zero out the acceleration to start with.
	g_pPhysicsLT->SetAcceleration( m_hObject, LTVector::GetIdentity() );

	if( LadderMgr::Instance().IsClimbing() || g_pPlayerMgr->IsOperatingTurret() )
	{
		return;
	}

	// Update motion due to any containers
	UpdateContainerMotion();

	// Update m_bOnGround data member...
	UpdateOnGround();

	// Determine and set rotation...
	UpdateRotation();

	// Determine and set the acceleration...
	UpdateAcceleration();

	// Determine and set the velocity...
	UpdateVelocity();

	// Check if we reached our desination...
	UpdateGotoNodeMovement();

	// Update the speed boost variables...
	UpdateSprintBoost();

	// If we're dead, we can't move....
	if( g_pPlayerMgr->IsPlayerDead() || !g_pPlayerMgr->IsPlayerMovementAllowed() )
	{
        LTVector vZero( 0.0f, 0.0f, 0.0f );
		g_pPhysicsLT->SetVelocity( m_hObject, vZero );
		g_pPhysicsLT->SetAcceleration( m_hObject, vZero );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateAcceleration
//
//	PURPOSE:	Determine and set the acceleration...
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateAcceleration( )
{
	bool bHeadInLiquid = IsHeadInLiquid();
	bool bInLiquid     = (bHeadInLiquid || m_bBodyInLiquid);
	bool bFreeMovement = (IsFreeMovement() || g_pPlayerMgr->IsSpectating());

	LTVector vAccel(0.0f, 0.0f, 0.0f);

	LTRotation rRot;
	if (g_pPlayerMgr->IsSpectating() || m_bSwimmingOnSurface  || IsFreeMovement())
	{
		// Get full rotation...
		rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );
	}
	else
	{
		// Only care about yaw...
		g_pLTClient->GetObjectRotation( m_hObject, &rRot );
	}

	LTVector vRight, vForward;
	vRight = rRot.Right();
	vForward = rRot.Forward();
	vRight.y = 0.0f;

	float fMoveAccelMulti = m_fMoveAccelMultiplier;
	if (m_bBodyInLiquid)
	{
		fMoveAccelMulti = fMoveAccelMulti < 1.0f ? 1.0f : fMoveAccelMulti;
	}

	float fMoveAccel = (m_fBaseMoveAccel * fMoveAccelMulti);

	if (g_pPlayerMgr->IsSpectating())
	{
		// The spectating acceleration will only be used to determine direction.  We don't actuall integration the
		// acceleration into velocity, we just go full speed.
		fMoveAccel = (m_dwControlFlags & BC_CFLG_RUN) ? 2.0f * SPECTATOR_ACCELERATION : SPECTATOR_ACCELERATION;
		fMoveAccel *= m_fMoveAccelMultiplier;
		fMoveAccel *= m_CV_SpectatorSpeedMul.GetFloat();
	}
	else if (!bInLiquid && !bFreeMovement)
	{
		// Can only move forward in x and z directions...

		vForward.y = 0.0;
		vForward.Normalize();
	}
	else if (m_bBodyInLiquid && !bHeadInLiquid)
	{
		// No up acceleration...

		vForward.y = vForward.y > 0.0 ? 0.0f : vForward.y;
		vForward.Normalize();
	}

	// Scale the right and forward vectors by the analog movement factors, if in use
	LTVector2 vMoveAxis(
		CBindMgr::GetSingleton().GetExtremalCommandValue(COMMAND_ID_STRAFE_AXIS),
		CBindMgr::GetSingleton().GetExtremalCommandValue(COMMAND_ID_FORWARD_AXIS));

	if (vMoveAxis.x != 0.0f)
		vRight *= fabsf(vMoveAxis.x);
	if (vMoveAxis.y != 0.0f)
		vForward *= fabsf(vMoveAxis.y);

	// If we're ducking make us move slower....

	if( g_pPlayerMgr->GetLeanMgr()->IsLeaning() )
	{
		fMoveAccel /= 2.0f;
	}

	// If we're in the air during a normal jump, adjust how much we can move...

	if (m_bJumped && !bFreeMovement)
	{
		fMoveAccel *= ClientDB::Instance().GetFloat( ClientDB::Instance().GetPlayerMovementRecord( ), g_pszInAirAccelMultiplier );
	}


	// We can walk around if we're alive.
	if ( !g_pPlayerMgr->IsPlayerDead( ))
	{
		HOBJECT hGotoNode = g_pPlayerMgr->GetPlayerNodeGoto( );
		if( hGotoNode && !g_pPlayerMgr->IsAtNodePosition( ))
		{
			LTVector vPlayerPos;
			g_pLTClient->GetObjectPos( m_hObject, &vPlayerPos );

			LTVector vNodePos;
			g_pLTClient->GetObjectPos( hGotoNode, &vNodePos );

			// Ignore height...
			vNodePos.y = vPlayerPos.y;

			// Accelerate towards the node...
			LTVector vDir = vNodePos - vPlayerPos;
			float fDirMag = vDir.Mag( );
			if( fDirMag > 0.000001f )
			{
				// Normalize it.
				vDir /= fDirMag;

				vAccel += (vDir * fMoveAccel);

				LTVector vVel = GetVelocity( );
				float fVelMag = vVel.Mag();
				if( fVelMag > 0.00001f )
				{
					// Normalize it.
					vVel /= fVelMag;

					if( vDir.Dot( vVel ) < 0.0f )
					{
						vVel = vDir * fVelMag;
						g_pPhysicsLT->SetVelocity( m_hObject, vVel );
					}
				}
			}
		}
		else
		{
			// Just base acceleration on what the player wants...

			if (m_dwControlFlags & BC_CFLG_FORWARD)
			{
				vAccel += (vForward * fMoveAccel);
			}

			if (m_dwControlFlags & BC_CFLG_REVERSE)
			{
				vAccel -= (vForward * fMoveAccel);
			}
		}

		// If we are in a container that supports free movement, see if we are
		// moving up or down...

		if (bInLiquid || bFreeMovement)
		{
			if( m_dwControlFlags & BC_CFLG_JUMP )
			{
				if (bInLiquid)
				{
					if( bHeadInLiquid )
					{
						vAccel.y = fMoveAccel;
					}
				}
				else
				{
					vAccel.y += fMoveAccel;
				}
			}
			if (m_dwControlFlags & BC_CFLG_DUCK)
			{
				vAccel.y = -fMoveAccel;
			}
		}


		if (m_dwControlFlags & BC_CFLG_STRAFE_LEFT)
		{
			vAccel -= (vRight * fMoveAccel);
		}

		if (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT)
		{
			vAccel += (vRight * fMoveAccel);
		}
	}

	g_pPhysicsLT->SetAcceleration(m_hObject, vAccel);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateVelocity
//
//	PURPOSE:	Determine and set the velocity...
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateVelocity( )
{
	LTVector vVelocity = GetVelocity();
	LTVector vMoveVelocity = vVelocity;
	vMoveVelocity.y = 0;

	bool bHeadInLiquid = IsHeadInLiquid( );
	bool bFreeMovement = (IsFreeMovement() || g_pPlayerMgr->IsSpectating());
	bool bJumping = false;
	
	if (bFreeMovement)
	{
		if (m_dwControlFlags & BC_CFLG_JUMP)
		{
			bJumping = true;
		}

		g_bJumpRequested = false;
	}
	else // normal case
	{
		bJumping = g_bJumpRequested && !m_bJumped;
	}

	float fMaxVelocityMag = GetMaxVelMag();

	// Limit velocity...
	if (m_bBodyInLiquid )
	{
		bool bCapY = true;

		// Cap the velocity in all directions, unless we're falling into,
		// or jumping out of, liquid...

		if (m_bBodyInLiquid)
		{
			if (!bHeadInLiquid && !m_bSwimmingOnSurface)
			{
				// Don't cap velocity in the y direction...

				bCapY = false;
			}
		}

		if (bCapY)
		{
			vMoveVelocity = vVelocity;
		}

		if (vMoveVelocity.Mag() > fMaxVelocityMag)
		{
			vMoveVelocity.Normalize();
			vMoveVelocity *= fMaxVelocityMag;

			vVelocity.x = vMoveVelocity.x;
			vVelocity.z = vMoveVelocity.z;
			if (bCapY)
			{
				vVelocity.y = vMoveVelocity.y;
			}

			g_pPhysicsLT->SetVelocity( m_hObject, vVelocity );
		}
	}
	else if (m_bOnGround && !g_pPlayerMgr->IsSpectating() && !bJumping)
	{
		float fCurLen = LTSqrt(vVelocity.x*vVelocity.x + vVelocity.z*vVelocity.z);
		if (fCurLen > fMaxVelocityMag)
		{
			vVelocity *= (fMaxVelocityMag/fCurLen);

			g_pPhysicsLT->SetVelocity( m_hObject, vVelocity );
		}
		// If we're on the ground, we should never be moving down, since we're just going to
		// collide with it on the next frame anyway.  (And the velocity will be fully cleared
		// when the player lets go of the controls.)
		if (vVelocity.y < 0.0f)
		{
			vVelocity.y = 0.0f;
			g_pPhysicsLT->SetVelocity( m_hObject, vVelocity );
		}
	}
	else if (vMoveVelocity.Mag() > fMaxVelocityMag)
	{
		// Don't cap velocity in the y direction...

		vMoveVelocity.Normalize();
		vMoveVelocity *= fMaxVelocityMag;

		vVelocity.x = vMoveVelocity.x;
		vVelocity.z = vMoveVelocity.z;

		g_pPhysicsLT->SetVelocity( m_hObject, vVelocity );
	}

	// See if we just broke the surface of water...
	if ((IsLiquid(m_eLastContainerCode) && !bHeadInLiquid) && !m_bOnGround )
	{
		m_bSwimmingOnSurface = true;
	}
	else if (bHeadInLiquid)  // See if we went back under...
	{
		m_bSwimmingOnSurface = false;
		m_bCanSwimJump       = true;
	}
	// body only in water.
	else if( m_bBodyInLiquid )
	{
		if( m_bOnGround )
		{
			m_bSwimmingOnSurface = false;
		}
	}
	// Not in water.
	else
	{
		m_bSwimmingOnSurface = false;
		m_bCanSwimJump       = false;
	}

	// If we're doing a swimming jump, keep jumping while we're not out of
	// the water (and we're still trying to get out)...
	if( m_bSwimmingJump )
	{
		m_bSwimmingJump = (m_bBodyInLiquid && bJumping);
	}

	// We can jump if we are not dead...

	bool bOkayToJump = false;
	if (bJumping && g_pPlayerMgr->IsPlayerAlive())
	{
		if( !bHeadInLiquid && m_bBodyInLiquid )
		{
			if (m_bCanSwimJump)
			{
				m_bSwimmingJump = true;
				m_bCanSwimJump  = false;
			}
			// If our head is out of the liquid and we're standing on the
			// ground, let us jump out of the water...
			else if (m_bOnGround)
			{
				m_bSwimmingJump = true;
			}
		}

		bOkayToJump = (m_bSwimmingJump || (m_bOnGround && !m_bBodyInLiquid));

		if (bOkayToJump)
		{
			vVelocity.y = m_fJumpVel * m_fJumpMultiplier;

  			m_bJumped				= true;
			m_bSwimJumped			= m_bSwimmingJump;
			m_bSwimmingOnSurface	= false;
			g_bJumpRequested		= false;
		}
	}

	// If our height offset is within a certain range... dampen the velocity
	float fHeightOffset = g_pPlayerMgr->GetPlayerCamera( )->GetCameraSmoothingOffset( );

	if( fabs( fHeightOffset ) > 1.0f )
	{
		static ClientDB& iClientDB = ClientDB::Instance();
		static float fVelocityDampUp = iClientDB.GetFloat( iClientDB.GetClientSharedRecord(), CDB_CameraHeightVelocityDampUp );
		static float fVelocityDampDown = iClientDB.GetFloat( iClientDB.GetClientSharedRecord(), CDB_CameraHeightVelocityDampDown );

		float fDampenValue = ( ( fHeightOffset > 0.0f ) ? fVelocityDampDown : fVelocityDampUp );
		float fDampenScale = ( 1.0f - ( LTMIN( fabs( fHeightOffset ), fDampenValue ) / fDampenValue ) );

		vVelocity.x *= fDampenScale;
		vVelocity.z *= fDampenScale;
	}

	// Leash the velocity if we need to...

	float fLeashScale = 1.0f;

	struct LeashHelper
	{
		// From		: Where we're coming from
		// To		: Where we're going to
		// TestFunc	: Are we testing if can get Closer or Further away?
		// Closer	: Distance from Origin where the velocity is still 100%
		// Farther	: Distance from Origin where the velocity should be 0%
		// Accel	: Current acceleration (to determine which direction we're moving)
		//
		// SlipMax/SlipMin - Percent of the arc to apply the full effect of the
		// scaling, and to interpolate to zero scaling.  1.0 = running straight
		// toward the AI in question, 0.5 = running perpendicular to the AI in
		// question, 0.0 = running straight away from the AI in question.
		//
		// If we're trying to keep the player inside a circle, then pass the
		// center of the circle in for From, the player's position for To
		// use Farther as the TestFunc, the inner radius for Closer, and the
		// outer radius for Farther.
		//
		// If we're trying to keep the player outside of a circle, then pass the
		// player's position in for From, the center of the circle for To
		// user Closer as the TestFunc, the outer radius for Closer, and the
		// inner radius for Farther.
		//
		static float GetLeashScale(
			const LTVector& vFrom, const LTVector& vTo,
			bool(*pTestFunc)(float A, float B),
			float fCloser, float fFarther,
			const LTVector& vAccel, float fSlipMin, float fSlipMax,
			bool b2D=true)
	{
			float fLeashScale = 1.0f;

			LTVector vDelta = (vTo - vFrom);
			if (b2D) vDelta.y = 0.0f;

		float fLeashLength = vDelta.Mag();

			if ((*pTestFunc)(fLeashLength, fCloser))
		{
				fLeashScale = LTCLAMP(LTALERP(fFarther, fCloser, fLeashLength), 0.0f, 1.0f);

				// Now determine which way we were trying to move... to make sure that we want to stop the player
				if( vAccel.x || vAccel.z )
				{
					LTVector vDirection = LTVector( -vAccel.x, 0.0f, -vAccel.z ).GetUnit();
					LTVector vAngle = LTVector( vDelta.x, 0.0f, vDelta.z ).GetUnit();

					float fTangentVal = vAngle.Dot( vDirection );

					// if fTangentVal == 1.0f, that means we're walking straight away from our limit
					// if fTangentVal == -1.0f, that means we're walking straight into our limit
					// if fTangentVal == 0.0f, that means we're walking perpendicular to our limit

					float fAngle = (float)acos(fTangentVal);	//!!ARL: It would be nice if we could avoid linearizing this.
					if (fSlipMin == fSlipMax)
			{
						fLeashScale = (fAngle < (fSlipMin * MATH_PI)) ? 1.0f : 0.0f;
					}
					else
					{
						fLeashScale += LTCLAMP(LTALERP((fSlipMax * MATH_PI), (fSlipMin * MATH_PI), fAngle), 0.0f, 1.0f);
						if (fLeashScale > 1.0f)
							fLeashScale = 1.0f;
					}
				}
			}

			return fLeashScale;
			}

		static bool Closer(float A, float B){ return (A < B); }
		static bool Farther(float A, float B){ return (A > B); }
	};

	// Check the PlayerLeash.
	if (IsPlayerLeashed())
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hObject, &vPos);

			LTVector vAccel;
			g_pPhysicsLT->GetAcceleration( m_hObject, &vAccel );

		fLeashScale = LTMIN(fLeashScale, LeashHelper::GetLeashScale(
			m_vLeashPosition, vPos,
			LeashHelper::Farther,
			m_vLeashParameters.x, m_vLeashParameters.y,
			vAccel, g_vtPlayerLeashSlipMin.GetFloat(), g_vtPlayerLeashSlipMax.GetFloat()));
	}

	// Check for nearby AIs.
	LTVector2 vMinProximity = GetCurrentMinProximity();
	if (vMinProximity.y > 0.0f)
			{
		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hObject, &vPos);

		LTVector vAccel;
		g_pPhysicsLT->GetAcceleration(m_hObject, &vAccel);

		for (CCharacterFX::CharFXList::iterator iter = CCharacterFX::GetCharFXList( ).begin( );
				iter != CCharacterFX::GetCharFXList( ).end( ); iter++ ) 
				{
			CCharacterFX* pChar = (*iter);
			if (!pChar->m_cs.bIsPlayer && !pChar->m_cs.bIsDead
				&& pChar->GetModel())	//speaker objects don't have models and we don't want to collide with anything like this that the player can't see.
			{
				LTVector vAIPos;
				g_pLTClient->GetObjectPos(pChar->GetServerObj(), &vAIPos);

				fLeashScale = LTMIN(fLeashScale, LeashHelper::GetLeashScale(
					vPos, vAIPos,
					LeashHelper::Closer,
					vMinProximity.y, vMinProximity.x,
					vAccel, g_vtPersonalSlipMin.GetFloat(), g_vtPersonalSlipMax.GetFloat(),
					false));
			}
		}
	}

	// Scale the velocity.
	if( fLeashScale <= 0.0f )
	{
				// Go ahead and force everything to zero...
					LTVector vZero( 0.0f, 0.0f, 0.0f );
					g_pPhysicsLT->SetAcceleration( m_hObject, vZero );
					g_pPhysicsLT->SetVelocity( m_hObject, vZero );
				}
	else if( fLeashScale < 1.0f )
	{
		vVelocity.x *= fLeashScale;
		vVelocity.z *= fLeashScale;
	}

	// If in spectator mode, dampen velocity...
	if( g_pPlayerMgr->IsSpectating() )
	{
		LTVector vAccel;
		g_pLTBase->Physics()->GetAcceleration( m_hObject, &vAccel );
		float fAccelMag = vAccel.Mag();

		// Check if we're stopped.
		if( fAccelMag < 0.01f )
		{
			float fVelocityMag = vVelocity.Mag();
			if( fVelocityMag < 0.01f )
			{
				vVelocity.Init( );
			}
		}
		else
		{
			// Set our velocity in the direction of the acceleration.  Use
			// the full velocity plus any speed multiplier.
			vVelocity = vAccel * ( GetMaxVelMag() * m_CV_SpectatorSpeedMul.GetFloat() / fAccelMag );
		}

		// Clear out the acceleration, we only use it for direction to travel.
		vAccel.Init( );
		g_pLTBase->Physics()->SetAcceleration( m_hObject, vAccel );
	}

	// Add any container currents to my velocity..
	vVelocity += m_vTotalCurrent;
	g_pPhysicsLT->SetVelocity( m_hObject, vVelocity );

	// Handle case when we just start moving...
	UpdateStartMotion( bOkayToJump, NULL, NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateSprintBoost
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateSprintBoost()
{
	float fFrameTime = g_pLTClient->GetFrameTime();

	if( m_bSprintBoostEnabled )
	{
		m_fSprintBoostPercent -= ( fFrameTime / g_vtSprintDrainSpeed.GetFloat() );

		if( m_fSprintBoostPercent <= 0.0f )
		{
			EnableSprintBoost( false );
			m_fSprintBoostPercent = 0.0f;
		}
	}
	else
	{
		if( m_fSprintBoostPercent < 1.0f )
		{
			m_fSprintBoostPercent += ( fFrameTime / g_vtSprintRecoverSpeed.GetFloat() );

			if( m_fSprintBoostPercent > 1.0f )
			{
				m_fSprintBoostPercent = 1.0f;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateContainerMotion
//
//	PURPOSE:	Update motion while in a container
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateContainerMotion()
{
	// Initialize containter totals...

	m_vTotalCurrent.Init();
	m_fTotalViscosity = 0.0f;

	// Normally we have gravity on, but the containers might turn it off.
	bool bNoGravity = g_pPlayerMgr->IsSpectating() || LadderMgr::Instance().IsClimbing();
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, bNoGravity ? 0 : FLAG_GRAVITY, FLAG_GRAVITY);


	m_eBodyContainerCode = CC_NO_CONTAINER;
	m_eBodyContainerSurface = ST_UNKNOWN;
    m_bBodyInLiquid = false;

	UpdateContainerList();

	uint32 i;
    bool bDidLiquid = false;
	for (i=0; i < m_nContainers; i++)
	{
		// Adjust the player's velocity based on the friction of the container...

		if (!m_Containers[i].m_bHidden && m_Containers[i].m_ContainerCode != CC_LADDER)
		{
			m_eBodyContainerCode = m_Containers[i].m_ContainerCode;

			// Retrieve the surface of the container.
			// If valid this surface will be used as the "standing on" surface...
			if( m_Containers[i].m_hObject )
			{
				m_eBodyContainerSurface = GetSurfaceType( m_Containers[i].m_hObject );
			}
			
			// Only do water viscosity once...

            bool bUpdateViscosity = true;
			if (IsLiquid(m_Containers[i].m_ContainerCode) && bDidLiquid)
			{
				bUpdateViscosity = false;
			}

			if (bUpdateViscosity)
			{
				UpdateContainerViscosity(&m_Containers[i]);
				UpdateContainerGravity( &m_Containers[i] );
			}

			if (IsLiquid(m_Containers[i].m_ContainerCode) && !bDidLiquid)
			{
				UpdateInLiquid(&m_Containers[i]);
                bDidLiquid = true;
			}

			m_vTotalCurrent += m_Containers[i].m_Current * ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateContainerList
//
//	PURPOSE:	Update the client-side container list
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateContainerList()
{
	// If we're a non-local client, refill the container array
	bool bClientOnServer = false;
	g_pLTClient->IsLocalToServer(&bClientOnServer);
	if (bClientOnServer)
		return;

	HOBJECT aContainers[MAX_TRACKED_CONTAINERS];
	m_nContainers = 0;
	uint32 nContainers = g_pLTClient->GetObjectContainers(m_hObject, aContainers, MAX_TRACKED_CONTAINERS);

	nContainers = LTMIN(nContainers, MAX_TRACKED_CONTAINERS);
    float fCoeff = 1.0f;
	for (uint32 i=0; i < nContainers; i++)
	{
		CVolumeBrushFX *pVBFX = (CVolumeBrushFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_VOLUMEBRUSH_ID, aContainers[i]);
		// Check if this isn't a volumebrush container, which means it won't
		// affect our movement.
		if (!pVBFX)
		{
			continue;
		}
		uint32 nServerObjFlags;
		g_pCommonLT->GetObjectFlags(pVBFX->GetServerObj(), OFT_User, nServerObjFlags);
		m_Containers[m_nContainers].m_bHidden = (nServerObjFlags & USRFLG_VISIBLE) == 0;
		m_Containers[m_nContainers].m_ContainerCode = pVBFX->GetCode();
		m_Containers[m_nContainers].m_Current = pVBFX->GetCurrent();
		m_Containers[m_nContainers].m_ePPhysicsModel = pVBFX->GetPhysicsModel();
		m_Containers[m_nContainers].m_fGravity = pVBFX->GetGravity();
		m_Containers[m_nContainers].m_fViscosity = pVBFX->GetViscosity();
		m_Containers[m_nContainers].m_hObject = aContainers[i];
		fCoeff = LTMIN(fCoeff, pVBFX->GetFriction());
		m_nContainers++;
	}

	fCoeff *= DEFAULT_FRICTION;
	g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fCoeff);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateStartMotion
//
//	PURPOSE:	Update starting to move
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateStartMotion(bool bForce, HRECORD hLandSound, CPolyGridFX* pPolyGridFXSplash )
{
	// Play a footstep sound to indicate we are moving...

	if (!bForce)
	{
		// Handle a change in crouching or moving...
		if( ((m_dwLastControlFlags & BC_CFLG_MOVING) && !(m_dwControlFlags & BC_CFLG_MOVING)) ||
			(!(m_dwLastControlFlags & BC_CFLG_MOVING) && (m_dwControlFlags & BC_CFLG_MOVING)) ||
			((m_dwLastControlFlags & BC_CFLG_DUCK) && !(m_dwControlFlags & BC_CFLG_DUCK)) ||
			(!(m_dwLastControlFlags & BC_CFLG_DUCK) && (m_dwControlFlags & BC_CFLG_DUCK)) )
		{
			g_pPlayerMgr->GetPlayerCamera( )->HandleStartMotion( );
		}

		uint32 dwTestFlags = (BC_CFLG_MOVING | BC_CFLG_DUCK);
		if (( (m_dwLastControlFlags & dwTestFlags) ||
			 !(m_dwControlFlags & dwTestFlags)))
		{
			return;
		}
	}


	// Tell server if the player just started to jump or just landed...

	if (bForce)
	{
		if( m_bJumped )
		{
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, kAP_MOV_Jump, CPlayerBodyMgr::kLocked );
		}
		else if( !m_bWasMovementEncoded )
		{
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Movement, kAP_MOV_Land, CPlayerBodyMgr::kLocked );
		}

        uint8 nStatus = m_bJumped ? MS_JUMPED : MS_LANDED;
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
		cMsg.Writeuint8(CP_MOTION_STATUS);
        cMsg.WriteBits( nStatus, FNumBitsExclusive<MS_COUNT>::k_nValue );
		if( nStatus == MS_LANDED )
		{
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hLandSound );
			// Write out the polygrid we splashed into.
			if( pPolyGridFXSplash )
			{
				cMsg.Writebool( true );
				cMsg.WriteObject( pPolyGridFXSplash->GetServerObj( ));
			}
			else
			{
				cMsg.Writebool( false );
			}
		}
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		// Play the jump sound for this client so the sound isnt't lagged in multiplayer...
		if (m_bJumped)
		{
			bool bPlaySound = false;
			if (!IsHeadInLiquid())
			{
				bPlaySound = (m_bBodyInLiquid ? m_bOnGround : (m_eStandingOnSurface != ST_INVISIBLE));
			}
			if (bPlaySound)
			{
				HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord("JumpLocal");
				if (hSoundRec)
				{
					g_pClientSoundMgr->PlayDBSoundLocal(hSoundRec, SOUNDPRIORITY_PLAYER_HIGH);
				}
			}
		}
	}

	if( m_pCharFX )
	{
		g_pPlayerMgr->UpdateCamera();

		if( g_pPlayerMgr->GetSpectatorMode() == eSpectatorMode_None )
		{
			if( GetConsoleInt( "InitialMovementSound", 1 ) > 0 )
			{
				HRECORD hMoveSound = ModelsDB::Instance().GetModelInitialMovementSoundRecord( m_pCharFX->m_cs.hModel );
				g_pClientSoundMgr->PlayDBSoundLocal( hMoveSound );
			}
			else
			{
				m_pCharFX->DoFootStep( CCharacterFX::eFootStep_Alternate );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdatePushers
//
//	PURPOSE:	Update anything that might push us
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdatePushers()
{
    LTVector myPos, pushVec, vel;
	float dist, velocity;
	CollisionInfo info;
	IntersectQuery iQuery;
	IntersectInfo iInfo;

	if (!m_hObject || !g_pLTClient || !g_pPhysicsLT) return;

	g_pLTClient->GetObjectPos(m_hObject, &myPos);
	for(LTListIter<Pusher*> itCur=m_Pushers.Begin(); itCur != m_Pushers.End(); )
	{
		Pusher *pPusher = *itCur;
		itCur++;

		pPusher->m_Delay -= SimulationTimer::Instance().GetTimerElapsedS( );
		if(pPusher->m_Delay <= 0.0f)
		{
			pPusher->m_TimeLeft -= SimulationTimer::Instance().GetTimerElapsedS( );
			if(pPusher->m_TimeLeft <= 0.0f)
			{
				// Expired..
				pPusher->m_Link.Remove();
				Pusher::GetBank()->Delete(pPusher);
			}
			else
			{
				// Are we within range?
				dist = pPusher->m_Pos.Dist(myPos);

				// Don't apply the pusher if the player is not close enough or if
				// they are playing a scripted animation...
				bool bApplyPusher = dist < pPusher->m_Radius;
                bApplyPusher &= !CPlayerBodyMgr::Instance( ).IsPlayingSpecial( );
				bApplyPusher &= (g_pPlayerMgr->GetPlayerNodeGoto( ) == NULL);
				if( bApplyPusher )
				{
					memset(&iQuery, 0, sizeof(iQuery));
					iQuery.m_From = pPusher->m_Pos;
					iQuery.m_To = myPos;
					if(!g_pLTClient->IntersectSegment(iQuery, &iInfo))
					{
						velocity = 1.0f - (dist / pPusher->m_Radius);
						velocity *= pPusher->m_Strength;

						// If we're in the air, apply less (since there's no friction).
						g_pPhysicsLT->GetStandingOn(m_hObject, &info);
						if(!info.m_hObject)
						{
							velocity /= 10.0f;
						}

						pushVec = myPos - pPusher->m_Pos;
						pushVec *= velocity / pushVec.Mag();

						vel = GetVelocity();
						vel += pushVec;

						// [KLS 5/20/02] Cap Y velocity.  Don't allow us to
						// fly too high...
						if (vel.y > g_vtMaxPushYVelocity.GetFloat())
						{
							DebugCPrint(1,"Old Pusher Y Vel = %.2f", vel.y);
							vel.y = g_vtMaxPushYVelocity.GetFloat();
						}

						g_pPhysicsLT->SetVelocity(m_hObject, vel);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::AreDimsCorrect
//
//	PURPOSE:	Validate our dims
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::AreDimsCorrect()
{
	if(!m_hObject || !g_pPhysicsLT)
        return true;

    LTVector curDims;

	g_pPhysicsLT->GetObjectDims(m_hObject, &curDims);
	return curDims.NearlyEquals(m_vWantedDims, 0.1f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::ResetDims
//
//	PURPOSE:	Reset our dims
//
// ----------------------------------------------------------------------- //

void CMoveMgr::ResetDims(LTVector *pOffset)
{
	// Remember our old position and size, just in case
	LTVector vOldDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vOldDims);
	LTVector vOldPos;
	g_pLTClient->GetObjectPos(m_hObject, &vOldPos);

	// Save off our wanted dims they dont change;
	LTVector vNewDims = m_vWantedDims;
	
	// Try to set our wanted dims... 
	if (g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, SETDIMS_PUSHOBJECTS) != LT_OK)
	{
		// Go back to where we were..
		g_pPhysicsLT->SetObjectDims(m_hObject, &vOldDims, 0);
		g_pLTClient->SetObjectPos(m_hObject, vOldPos);
	}
	// Move them if they want
	else if (pOffset && (*pOffset != LTVector(0.0f, 0.0f, 0.0f)))
	{
		LTVector vNewPos = vOldPos + *pOffset;
		g_pPhysicsLT->MoveObject(m_hObject, vNewPos, 0);
		LTVector vResultPos;
		g_pLTClient->GetObjectPos( m_hObject, &vResultPos );
		// Update the "last on ground" position so we don't think we're falling based on the position change
		CollisionInfo Info;
		g_pPhysicsLT->GetStandingOn(m_hObject, &Info);
		if (Info.m_hObject)
			m_fLastOnGroundY = vResultPos.y;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::CanStandUp
//
//	PURPOSE:	Determine whether or not we can stand up in our current position (and state)
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::CanStandUp()
{
	// Are you currently crouching
	LTVector vCurDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vCurDims);
	if (vCurDims.y > m_vCrouchDims.y)
	{
		// You're already standing..  What are you asking me for?
		return true;
	}

	LTVector vOldPos;
	g_pLTClient->GetObjectPos(m_hObject, &vOldPos);

	// Can we stand?
	LTVector vWantedDims = LTVector( vCurDims.x, m_vStandDims.y, vCurDims.z );
	bool bResult = (g_pPhysicsLT->SetObjectDims(m_hObject, &vWantedDims, SETDIMS_PUSHOBJECTS) == LT_OK);

	// Reset our dims to what they were before.
	g_pPhysicsLT->SetObjectDims(m_hObject, &vCurDims, 0);
	g_pPhysicsLT->MoveObject( m_hObject, vOldPos, 0 );

	return bResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::SetPlayerLeash
//
//	PURPOSE:	Setup the leash values...
//
// ----------------------------------------------------------------------- //

void CMoveMgr::SetPlayerLeash( float fInnerRadius, float fOuterRadius, LTVector* pvPos )
{
	m_vLeashParameters.x = LTMAX( 0.0f, fInnerRadius );
	m_vLeashParameters.y = LTMAX( m_vLeashParameters.x, fOuterRadius );

	if (pvPos)
	{
		m_vLeashPosition = *pvPos;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::IsPlayerLeashed
//
//	PURPOSE:	Query if the player is actively leashed...
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::IsPlayerLeashed()
{
	return ( m_vLeashParameters.y > 0.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::EnableSprintBoost
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CMoveMgr::EnableSprintBoost( bool bEnable )
{
	// Make sure we're gonna allow the sprint boost...
	if( !m_bSprintBoostEnabled && bEnable )
	{
		// Make sure we have a fair amount of boost 'juice'
		if( m_fSprintBoostPercent < g_vtSprintValidStartRange.GetFloat() )
		{
			return;
		}

		// Only boost if we're already moving as fast as we can... give or take a bit
		LTVector vVelocity = GetVelocity();

		if( vVelocity.Mag() < ( m_fWalkVel - g_vtSprintWalkSpeedDelta.GetFloat() ) )
		{
			return;
		}
	}

	m_bSprintBoostEnabled = bEnable;

	if( m_bSprintBoostEnabled )
	{
		m_dwControlFlags |= BC_CFLG_RUN;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::IsSprintBoostEnabled
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::IsSprintBoostEnabled()
{
	return m_bSprintBoostEnabled;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetSprintBoostPercent
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetSprintBoostPercent()
{
	return m_fSprintBoostPercent;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetDistanceFallenPercent
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetDistanceFallenPercent()
{
	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hObject, &vPos);
	float fDistFell = m_fLastOnGroundY - vPos.y;

	float fPercent;

	float fDamageMaxHeight = g_vtFallDamageMaxHeight.GetFloat();
	fPercent = fDistFell*100.0f/fDamageMaxHeight;
	fPercent = LTCLAMP(fPercent, 0.0f, 100.0f);
	return fPercent;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::MoveLocalSolidObject
//
//	PURPOSE:	Move our object
//
// ----------------------------------------------------------------------- //

void CMoveMgr::MoveLocalSolidObject()
{
	MoveInfo info;
    LTVector newPos, curPos;

	// Check if we're using a vehicle physics model.
	if( m_pVehicleMgr->IsVehiclePhysics( ))
	{
		// See if the vehiclemgr handled our move.
		if( m_pVehicleMgr->MoveLocalSolidObject( ))
			return;
	}

	// Check if we're supposed to follow someone in spectator mode.
	if( g_pPlayerMgr->GetSpectatorMode() == eSpectatorMode_Follow )
	{
		HOBJECT hFollow = g_pPlayerMgr->GetSpectatorFollowTarget();
		if( !hFollow )
			return;

		LTRigidTransform transform;
		g_pLTBase->GetObjectTransform( hFollow, &transform );
		g_pLTBase->SetObjectTransform( m_hObject, transform );
		return;
	}

	ILTClientPhysics* pPhysics = (ILTClientPhysics*)g_pPhysicsLT;

	// We may want gravity to be different for us...

	if (g_vtPlayerGravity.GetFloat() != DEFAULT_PLAYER_GRAVITY)
	{
		m_fGravity = g_vtPlayerGravity.GetFloat();
	}
	
	// Set the stair step height from the server...
	
	float fStairHeight = DEFAULT_STAIRSTEP_HEIGHT;
	g_pLTClient->GetSConValueFloat( STAIR_STEP_HEIGHT_CVAR, fStairHeight );
	if( fStairHeight !=	DEFAULT_STAIRSTEP_HEIGHT )
	{
		g_pPhysicsLT->SetStairHeight( fStairHeight );
	}


	LTVector vOldGlobalForce(0, 0, 0);
	LTVector vNewGlobalForce(0, m_bGravityOverride ? m_fTotalContainerGravity : m_fGravity, 0);

	pPhysics->GetGlobalForce(vOldGlobalForce);
	pPhysics->SetGlobalForce(vNewGlobalForce);

	info.m_hObject = m_hObject;
	info.m_dt = ObjectContextTimer( GetServerObject( )).GetTimerElapsedS( );
	pPhysics->UpdateMovement(&info);

	// Make sure we moved at least a little
	if (info.m_Offset.MagSqr() > 0.0f)
	{
		// Cap the velocity if it exceeds our maximum
		{
			LTVector vVelocity;
			pPhysics->GetVelocity( m_hObject, &vVelocity );

			float fOffsetY = vVelocity.y;

			if( !g_pPlayerMgr->IsSpectating() )
				vVelocity.y = 0.0f;

			float fCapVelocityMag = vVelocity.Mag();
			float fMaxVelocityMag = GetMaxVelMag();

			if( fCapVelocityMag > fMaxVelocityMag )
			{
				float fCapScalar = ( fMaxVelocityMag / fCapVelocityMag );
				vVelocity *= fCapScalar;

				if( !g_pPlayerMgr->IsSpectating() )
					vVelocity.y = fOffsetY;

				pPhysics->SetVelocity( m_hObject, vVelocity );

				if( !g_pPlayerMgr->IsSpectating() )
				{
					// Cap the offset using the same technique
					fOffsetY = info.m_Offset.y;
					info.m_Offset.y = 0.0f;
					info.m_Offset *= fCapScalar;
					info.m_Offset.y = fOffsetY;
				}
			}
		}

		// Apply the offset to calculate the new position
		g_pLTClient->GetObjectPos(m_hObject, &curPos);
		newPos = curPos + info.m_Offset;

		if( m_pVehicleMgr->IsVehiclePhysics( ) )
		{
			m_pVehicleMgr->MoveVehicleObject( newPos );
		}
		else
		{
			pPhysics->MoveObject(m_hObject, newPos, 0);
		}

		LTVector targetPos = newPos;
		g_pLTClient->GetObjectPos(m_hObject, &newPos);

		// Deal with the issue that comes up when you're standing on something
		// you should be sliding on, but you're blocked, so gravity keeps
		// getting applied to the velocity.  Only do this if the UpdateMovement
		// affected our y direction.
		LTVector vCurDims;
		g_pPhysicsLT->GetObjectDims(m_hObject, &vCurDims);
		const float k_fMaxBlockedYDelta = vCurDims.y;
		if ( fabsf( info.m_Offset.y ) > 0.01f && fabsf(targetPos.y - newPos.y) > k_fMaxBlockedYDelta)
		{
			LTVector vObjVel;
			pPhysics->GetVelocity(m_hObject, &vObjVel);
			vObjVel.y = 0.0f;
			pPhysics->SetVelocity(m_hObject, vObjVel);
		}

		// Prevent us from sliding along while mid-air
		if (m_bJumped && !m_bOnGround && !newPos.NearlyEquals(targetPos, 0.01f))
		{
			LTVector vObjVel;
			pPhysics->GetVelocity(m_hObject, &vObjVel);
			LTVector vNewVel = (targetPos - curPos) / info.m_dt;
			vNewVel.y = vObjVel.y;
			if (vNewVel.MagSqr() < vObjVel.MagSqr())
				pPhysics->SetVelocity(m_hObject, vNewVel);
		}
	}

	pPhysics->SetGlobalForce(vOldGlobalForce);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetVelMagnitude
//
//	PURPOSE:	Get our velocity's magnitude
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetVelMagnitude()
{
	if (!g_pPhysicsLT || !g_pGameClientShell || !m_hObject) return 0.0f;

    return GetVelocity().Mag();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::SetClientObjNonsolid
//
//	PURPOSE:	Set the client object to non-solid
//
// ----------------------------------------------------------------------- //

void CMoveMgr::SetClientObjNonsolid()
{
	HOBJECT hObj = g_pLTClient->GetClientObject();

	if( hObj )
	{
		g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, FLAG_CLIENTNONSOLID, FLAG_CLIENTNONSOLID);
	}

	// We need to update our physics solid flag.  Since we are set to ~FLAG_SOLID,
	// the engine automatically sets us to physics non-solid.
	uint32 nNumRigidBodies = 0;
	g_pLTBase->PhysicsSim( )->GetNumModelRigidBodies( hObj, nNumRigidBodies );
	for( uint32 nIndex = 0; nIndex < nNumRigidBodies; nIndex++ )
	{
		HPHYSICSRIGIDBODY hRigidBody;
		if (LT_OK == g_pLTBase->PhysicsSim( )->GetModelRigidBody( hObj, nIndex, hRigidBody ))
		{
			g_pLTBase->PhysicsSim( )->SetRigidBodySolid( hRigidBody, true );
			g_pLTBase->PhysicsSim( )->ReleaseRigidBody(hRigidBody);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateFriction
//
//	PURPOSE:	Update fricton
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateFriction()
{
	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		m_pVehicleMgr->UpdateFriction();
	}
	else
	{
		UpdateNormalFriction();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateNormalFriction
//
//	PURPOSE:	Update normal fricton
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateNormalFriction()
{
	// Dampen our velocity so we don't slide around too much...
    uint32 dwTestFlags = BC_CFLG_MOVING;
	LTVector vZero(0, 0, 0);

	if ( !(m_dwControlFlags & dwTestFlags) && ( g_pPlayerMgr->IsSpectating( ) || ( !m_bJumped && m_bOnGround &&
		 m_vTotalCurrent == vZero && m_fTotalViscosity == 0.0f)))
	{
        LTVector vCurVel = GetVelocity();

		bool bFrictionOnY = g_pPlayerMgr->IsSpectating( );

        float fYVal = vCurVel.y;
		if( !bFrictionOnY )
			vCurVel.y = 0.0f;

        LTVector vVel(0, 0, 0);
		if (vCurVel.MagSqr() > 5.0f)
		{
            LTVector vDir = vCurVel;
			vDir.Normalize();

            float fSlideToStopTime = g_vtSlideToStopTime.GetFloat();
			fSlideToStopTime = fSlideToStopTime <= 0.0f ? 0.1f : fSlideToStopTime;

            float fAdjust = ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( ) * (m_fRunVel/fSlideToStopTime);

			vVel = (vDir * fAdjust);

			if (vVel.MagSqr() < vCurVel.MagSqr())
			{
				vVel = vCurVel - vVel;
			}
			else
			{
				vVel.Init();
			}

			if( !bFrictionOnY )
				vVel.y = fYVal;
		}
		else
		{
			vVel.Init();
		}

		g_pPhysicsLT->SetVelocity(m_hObject, vVel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::ShowPos
//
//	PURPOSE:	Show our object's position
//
// ----------------------------------------------------------------------- //

void CMoveMgr::ShowPos(char *pBlah)
{
    LTVector pos;
	g_pLTClient->GetObjectPos(m_hObject, &pos);
	g_pLTClient->CPrint("%s: %.1f %.1f %.1f", pBlah, VEC_EXPAND(pos));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::Update
//
//	PURPOSE:	Update the move mgr
//
// ----------------------------------------------------------------------- //

void CMoveMgr::Update()
{
	if (g_pGameClientShell->IsServerPaused()) return;
	
	HOBJECT hObj = g_pLTClient->GetClientObject();
	if (!m_hObject || !hObj) return;

	m_eLastContainerCode = m_eCurContainerCode;
	m_eCurContainerCode  = g_pPlayerMgr->GetCurContainerCode();

	// If were're underwater or on a ladder or being controlled by the player
	// lure reset our last on ground Y value so we won't take fall damage the 
	// next time we touch the ground...
	if ( IsHeadInLiquid() || LadderMgr::Instance().IsClimbing() || SpecialMoveMgr::Instance().IsActive() || (m_pVehicleMgr->GetPhysicsModel( ) == PPM_LURE) )
	{
		m_fLastOnGroundY = MIN_ONGROUND_Y;
	}

	// We don't want to hit the real client object....

	SetClientObjNonsolid();

	if( m_pVehicleMgr->IsVehiclePhysics() )
	{
		m_pVehicleMgr->UpdatePlayerLure();
	}

	UpdateControlFlags();

    UpdateMotion();

	UpdateFriction();

	UpdatePushers();

	UpdateSound();

	MoveLocalSolidObject();

	if (LadderMgr::Instance().IsClimbing())
	{
		if (g_bJumpFromLadder && LadderMgr::Instance().CanReleaseLadder())
		{
			DebugCPrint(0,"%s - jump from ladder",__FUNCTION__);
			JumpFromLadder();
		}
		else if (m_bBodyInLiquid && (m_dwControlFlags & BC_CFLG_REVERSE) && !LadderMgr::Instance( ).IsAtTop( ))
		{
			DebugCPrint(0,"%s - ReleaseLadder",__FUNCTION__);
			LadderMgr::Instance().ReleaseLadder();
		}
	}
	else
	{
		g_bJumpFromLadder = false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnPhysicsUpdate
//
//	PURPOSE:	Handle a physics update
//
// ----------------------------------------------------------------------- //

void CMoveMgr::OnPhysicsUpdate(uint16 changeFlags, ILTMessage_Read *pMsg)
{
	if (!pMsg) return;


	// See if our container state changed...

	if (changeFlags & PSTATE_POSITION)
	{
		OnServerForcePos( pMsg );
	}
	else
	{
		// Put the serverobject on our location so it has the correct position
		// if we need to use it for anything.
		LTVector vRealPos;
		g_pLTClient->GetObjectPos(m_hObject, &vRealPos);
		g_pLTClient->SetObjectPos(g_pLTClient->GetClientObject(), vRealPos);
	}

	// Note : This has to remain in order to avoid double-querying the containers, since it's 
	// done automatically on the server-local clients.
	if (changeFlags & PSTATE_CONTAINERTYPE)
	{
		m_nContainers = pMsg->Readuint8();
	
		for (DWORD i=0; i < m_nContainers && i < MAX_TRACKED_CONTAINERS; i++)
		{
			m_Containers[i].m_ContainerCode = (ContainerCode)pMsg->Readuint8();
			m_Containers[i].m_Current = pMsg->ReadLTVector();
			m_Containers[i].m_fGravity = pMsg->Readfloat();
			m_Containers[i].m_fViscosity = pMsg->Readfloat();
			m_Containers[i].m_bHidden = !!pMsg->Readuint8();
			m_Containers[i].m_hObject = pMsg->ReadObject();
			m_Containers[i].m_ePPhysicsModel = (PlayerPhysicsModel)pMsg->Readuint8();
		}

        float fCoeff = pMsg->Readfloat();
		g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fCoeff);
	}

	// Change our model file?

	if (changeFlags & PSTATE_MODELFILENAMES)
	{
		// Reset the player body to match what was dictated by the server...
		CPlayerBodyMgr::Instance( ).ResetModel( (ModelsDB::HMODEL )pMsg->ReadDatabaseRecord( g_pLTDatabase, 
			g_pModelsDB->GetModelsCategory()));

		// This object will be forced to the server pos...
		m_bForceToServerPos = true;

		if( m_hObject )
		{
			// Note : This queries the anim for the dims of the player.  That information is all available
			// on the server, but it's not on the client.  This is a lot easier than trying to transmit
			// that information or something.
			HMODELANIM hAnim = g_pLTClient->GetAnimIndex(m_hObject, PLAYERANIM_CROUCH);
			if (hAnim == INVALID_MODEL_ANIM)
			{
				ASSERT(!"Unable to find player crouch animation");
				m_vCrouchDims.Init(40.0f, 61.0f, 40.0f);
			}
			else
			{
				g_pModelLT->GetModelAnimUserDims(m_hObject, hAnim, &m_vCrouchDims);
			}

			hAnim = g_pLTClient->GetAnimIndex(m_hObject, PLAYERANIM_STAND);
			if (hAnim == INVALID_MODEL_ANIM)
			{
				ASSERT(!"Unable to find player stand animation");
				m_vStandDims.Init(40.0f, 90.0f, 40.0f);
			}
			else
			{
				g_pModelLT->GetModelAnimUserDims(m_hObject, hAnim, &m_vStandDims);
			}

			m_PlayerRigidBody.Init( m_hObject );
		}
	}

	// Gravity change...

	if (changeFlags & PSTATE_GRAVITY)
	{
        LTVector vGravity;
		vGravity = pMsg->ReadLTVector();
		g_pPhysicsLT->SetGlobalForce(vGravity);
	}

	// Speed change...

	if (changeFlags & PSTATE_SPEEDS)
	{
		m_fWalkVel = pMsg->Readfloat();
		m_fRunVel = pMsg->Readfloat();
		m_fSwimVel = pMsg->Readfloat();
		m_fJumpVel = pMsg->Readfloat();
		m_fCrawlVel = pMsg->Readfloat();

		m_fMoveAccelMultiplier = m_fMoveMultiplier = pMsg->Readfloat();
		m_fBaseMoveAccel = pMsg->Readfloat();
		m_fJumpMultiplier = pMsg->Readfloat();
		float fLadderVel = pMsg->Readfloat();

		m_fGravity = pMsg->Readfloat();
		g_vtPlayerGravity.SetFloat(m_fGravity);

        float fCoeff = pMsg->Readfloat();
		g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fCoeff);

		//force idle at 1/4 of crawl vel
		m_fForceIdleVel = (m_fMoveMultiplier * m_fCrawlVel * 0.25f); 

	}

	// Vehicle status change...

	if (changeFlags & PSTATE_PHYSICS_MODEL)
	{
		PlayerPhysicsModel eModel = (PlayerPhysicsModel) pMsg->Readuint8();

		// If it's a lure physics model, then read the lure object in.
		if( eModel == PPM_LURE )
		{
			// Get the player lure id.
			DWORD nPlayerLureId = pMsg->Readuint32( );

			// SetPhysicsModel doesn't allow changing the model to the same model.  Trick
			// it by changing it here.
			if( m_pVehicleMgr->GetPhysicsModel( ) == PPM_LURE )
			{
				m_pVehicleMgr->SetPhysicsModel( PPM_NORMAL );
			}

			// Tell the vehiclemgr about the lure object.
			m_pVehicleMgr->SetPlayerLureId( nPlayerLureId );
		}
		else
		{
			// Make sure the lure is cleared.
			m_pVehicleMgr->SetPlayerLureId( PlayerLureId_Invalid );
		}

		// Turn off duck lock...
		if (eModel != PPM_NORMAL)
			SetDuckLock(false);

		m_pVehicleMgr->SetPhysicsModel( eModel );
	}

	if( changeFlags & PSTATE_MOVEMENT )
	{
		bool bAllowMovement = pMsg->Readbool( );
		g_pPlayerMgr->AllowPlayerMovement( bAllowMovement );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnObjectMove
//
//	PURPOSE:	Handle the object moving
//
// ----------------------------------------------------------------------- //

LTRESULT CMoveMgr::OnObjectMove(HOBJECT hObj, bool bTeleport, LTVector *pPos)
{
	if (!m_hObject) return LT_OK;

	HOBJECT hClientObj = g_pLTClient->GetClientObject();

	// If the object can move us, call MovePushObjects to make sure the player object
	// is carried/pushed around.

	if (CanMovePlayer(hObj, bTeleport))
	{
		((ILTClientPhysics*)g_pPhysicsLT)->MovePushObjects(hObj, *pPos, &m_hObject.GetData(), 1);
	}

	// If the moving object is a the thing we're standing on and it's a rigidbody object, then
	// detech us from standing on.  The reason is rigid bodies can move out from under us
	// and leave us floating.  We only do this for rigid bodies and not keyframed objects
	// because we do want to follow lifts.
	CollisionInfo collisionInfo;
	if( g_pPhysicsLT->GetStandingOn(m_hObject, &collisionInfo) == LT_OK && collisionInfo.m_hObject == hObj )
	{
		uint32 nFlags2 = 0;
		g_pCommonLT->GetObjectFlags(hObj, OFT_Flags2, nFlags2);
		if( nFlags2 & FLAG2_RIGIDBODY )
		{
			// Log this object for later so we know we need to detach ourselves.
			m_hStandingOnRigidBody = hObj;
		}
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnObjectRotate
//
//	PURPOSE:	Handle the object rotating
//
// ----------------------------------------------------------------------- //

LTRESULT CMoveMgr::OnObjectRotate(HOBJECT hObj, bool bTeleport, LTRotation *pNewRot)
{
	if (!m_hObject) return LT_OK;

	// If the object can move us, call RotatePushObjects to make sure the player object
	// is carried/pushed around.

	if (CanMovePlayer(hObj, bTeleport))
	{
		((ILTClientPhysics*)g_pPhysicsLT)->RotatePushObjects(hObj, *pNewRot, &m_hObject.GetData(), 1);
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::CanMovePlayer
//
//	PURPOSE:	Check to see if the passed in object can move the player
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::CanMovePlayer(HOBJECT hObj, bool bTeleport)
{
	if (!m_hObject) return false;

	HOBJECT hClientObj = g_pLTClient->GetClientObject();

	// If it's a solid world model that isn't using rigid body physics and
	// it's not telporting, then it can move the player...

	if (!bTeleport && hObj != hClientObj && hObj != m_hObject)
	{
		uint32 type = GetObjectType(hObj);
		if (type == OT_WORLDMODEL)
		{
			uint32 dwFlags, dwFlags2;
			g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
			g_pCommonLT->GetObjectFlags(hObj, OFT_Flags2, dwFlags2);
			if( (dwFlags & FLAG_SOLID) && !(dwFlags2 & FLAG2_RIGIDBODY) )
			{
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::AddPusher
//
//	PURPOSE:	Add a pusher object
//
// ----------------------------------------------------------------------- //

LTRESULT CMoveMgr::AddPusher(const LTVector &pos, float radius, float startDelay, float duration, float strength)
{
	// Don't push a dead player...
	if( !g_pPlayerMgr->IsPlayerAlive() || g_pPlayerMgr->IsSpectating())
		return LT_ERROR;

	Pusher *pPusher = Pusher::GetBank()->New();

	if(!pPusher)
		return LT_ERROR;

	pPusher->m_Pos = pos;
	pPusher->m_Radius = radius;
	pPusher->m_Delay = startDelay;
	pPusher->m_TimeLeft = duration;
	pPusher->m_Strength = strength;
	m_Pushers.AddHead(&pPusher->m_Link);

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::SetSpectatorMode
//
//	PURPOSE:	Set spectator mode
//
// ----------------------------------------------------------------------- //

void CMoveMgr::SetSpectatorMode( SpectatorMode eSpectatorMode )
{
	if(!m_hObject)
		return;

	// Check if leaving clip mode.
	if ( eSpectatorMode == eSpectatorMode_None )
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID | FLAG_GOTHRUWORLD);
		return;
	}

	// Make sure we have desired dims....
	LTVector oldDims = m_vWantedDims;
	m_vWantedDims = m_vCrouchDims;

	if (!AreDimsCorrect())
	{
		// Figure out a position offset.
		LTVector offset;
		offset.Init();
		if (m_vWantedDims.y < oldDims.y)
		{
			offset.y = -(oldDims.y - m_vWantedDims.y);
		}

		ResetDims(&offset);
	}
	
	// Allow us to go through the world if we're going into clip mode.
	uint32 nFlags = ( eSpectatorMode == eSpectatorMode_Clip ) ? FLAG_GOTHRUWORLD : 0;
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, nFlags, FLAG_GOTHRUWORLD | FLAG_SOLID);

	uint32 dwFlags2 = FLAG2_PLAYERCOLLIDE | FLAG2_PLAYERSTAIRSTEP | FLAG2_SPECIALNONSOLID;
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, dwFlags2, dwFlags2 );

	// Make sure we start stopped so we don't go careening through the floor or whatever.
	LTVector vZero(0, 0, 0);
	g_pPhysicsLT->SetVelocity(m_hObject, vZero);
	g_pPhysicsLT->SetAcceleration(m_hObject, vZero);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnServerForcePos
//
//	PURPOSE:	Handle forcing our position
//
// ----------------------------------------------------------------------- //

void CMoveMgr::OnServerForcePos(ILTMessage_Read *pMsg)
{
 	if (!pMsg) return;

 	m_ClientMoveCode	= pMsg->Readuint8();
	LTVector vPos		= pMsg->ReadLTVector( );
	LTVector vCurDims	= pMsg->ReadLTVector();

	if( pMsg->Readbool())
	{
		LTVector vForward = pMsg->ReadCompLTPolarCoord();
		LTRotation rRot( vForward, LTVector( 0.0f, 1.0f, 0.0f ));
		g_pPlayerMgr->GetPlayerCamera()->SetCameraRotation( rRot );
		// Overwrite any backup rotation with this one so the server rotation doesn't overwritten.
		g_pPlayerMgr->GetPlayerCamera()->SaveBackupRotation();

		// If the camera is attached, then it has a cached rotation.  Update this to the same
		// rotation, since the old rotation isn't meaningful anymore now that the server
		// has told us to rotate.
		if( g_pPlayerMgr->GetPlayerCamera()->IsAttachedToTarget( ))
		{
			g_pPlayerMgr->GetPlayerCamera()->SetTargetAttachRot( rRot );
		}

		// Don't force the body rotation while climbing a ladder
		if( !LadderMgr::Instance( ).IsClimbing( ) )
		{
			// Set the server and client side objects of the player too.
			if( GetServerObject())
			{
				g_pLTClient->SetObjectRotation( GetServerObject(), rRot );
			}
			if( GetObject( ))
			{
				g_pLTClient->SetObjectRotation( GetObject( ), rRot );
			}
		}
	}

	if( !m_hObject  )
		return;

	SetClientObjNonsolid();

	// Move there.  We make our object a point first and then resize the dims so
	// we don't teleport clipping into the world.
	

	LTVector vTempDims(0.5f, 0.5f, 0.5f);

	g_pPhysicsLT->SetObjectDims(m_hObject, &vTempDims, 0);
	g_pPhysicsLT->MoveObject(m_hObject, vPos, MOVEOBJECT_TELEPORT);

	// Turn off camera smoothing after a teleport...
	g_pPlayerMgr->GetPlayerCamera( )->DisableCameraSmoothing( );

	// We use special dims while spectating.
	if( g_pPlayerMgr->IsSpectating( ))
	{
		vCurDims = m_vCrouchDims;
	}

	//if the target dims don't fit, force the largest dims that do fit, the dims will be corrected
	//	auto-magically on a later update.
	bool bPushObjects = !CPlayerBodyMgr::Instance( ).IsMovementEncoded( );
	if( LT_OK != g_pPhysicsLT->SetObjectDims( m_hObject, &vCurDims, ( bPushObjects ? SETDIMS_PUSHOBJECTS : 0 ) ) )
		g_pPhysicsLT->SetObjectDims( m_hObject, &vCurDims, 0 );

	// Clear our velocity and acceleration.

	LTVector vVel(0, 0, 0);
	g_pPhysicsLT->SetAcceleration(m_hObject, vVel);

	if (m_bLoading)
	{
		vVel = m_vSavedVel;
		m_bLoading = false;
	}
	else
	{
		m_fLastOnGroundY = MIN_ONGROUND_Y;
	}

	g_pPhysicsLT->SetVelocity(m_hObject, vVel);

	// Make sure our server object is at the new position.  It may
	// not get there immediately, since positions are unguaranteed.
	HOBJECT hClientObj = g_pLTClient->GetClientObject();
	if (hClientObj)
	{
		g_pLTClient->SetObjectPos(hClientObj, vPos);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::WritePositionInfo
//
//	PURPOSE:	Write our position info
//
// ----------------------------------------------------------------------- //

void CMoveMgr::WritePositionInfo(ILTMessage_Write *pMsg)
{
    LTVector myPos, myVel;

	if (m_hObject)
	{
		g_pLTClient->GetObjectPos(m_hObject, &myPos);
		// If we're paused on our end, tell the server we're not going to move
		if (g_pGameClientShell->IsServerPaused())
			myVel = LTVector(0.0f, 0.0f, 0.0f);
		else
			myVel = GetVelocity();
	}
	else
	{
		myPos.Init();
		myVel.Init();
	}

	pMsg->Writeuint8(m_ClientMoveCode);
	pMsg->WriteLTVector(myPos);
	pMsg->WriteLTVector(myVel);
    pMsg->Writebool(m_bOnGround);
    pMsg->Writeuint8((uint8)m_eStandingOnSurface);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateModels
//
//	PURPOSE:	Update the player-view models
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateModels()
{
	if( m_pVehicleMgr->IsVehiclePhysics())
		m_pVehicleMgr->UpdateModels();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetMaxVelMag
//
//	PURPOSE:	Get the max velocity for our current mode
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetMaxVelMag() const
{
	float fMaxVel = 0.0f;

	float fMoveMultiplier = m_fMoveMultiplier;

	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		fMaxVel = m_pVehicleMgr->GetMaxVelMag();
	}
	else
	{
		bool bRunning = !!(m_dwControlFlags & BC_CFLG_RUN);

		if (m_bBodyInLiquid && m_bWaterAffectsSpeed )
		{
			fMaxVel = bRunning ? m_fSwimVel : m_fSwimVel/2.0f;
			fMoveMultiplier = fMoveMultiplier < 1.0f ? 1.0f : fMoveMultiplier;
		}
		else
		{
			if( m_dwControlFlags & BC_CFLG_DUCK )
			{
				fMaxVel = m_fCrawlVel;
			}
			else if( bRunning && !g_pPlayerMgr->GetPlayerCamera()->IsZoomed() && !g_pPlayerMgr->GetLeanMgr()->IsLeaning() )
			{
				fMaxVel = m_fRunVel;
			}
			else
			{
				fMaxVel = m_fWalkVel;
			}
		}

		//Calculate any movement penalties
		float fDamageMult = GetDamageMovementMultiplier();
		fMaxVel *= ( fDamageMult * fMoveMultiplier );
	}

	return fMaxVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetBaseVelMag
//
//	PURPOSE:	Get the unadjusted velocity for our current mode
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetBaseVelMag() const
{
	float fBaseVel = 0.0f;

	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		fBaseVel = m_pVehicleMgr->GetMaxVelMag();
	}
	else
	{
		bool bRunning = !!(m_dwControlFlags & BC_CFLG_RUN);
		if (m_bBodyInLiquid && m_bWaterAffectsSpeed )
		{
			fBaseVel = bRunning ? m_fSwimVel : m_fSwimVel/2.0f;
		}
		else
		{
			if( m_dwControlFlags & BC_CFLG_DUCK )
			{
				fBaseVel = m_fCrawlVel;
			}
			else if( bRunning && !g_pPlayerMgr->GetPlayerCamera()->IsZoomed() && !g_pPlayerMgr->GetLeanMgr()->IsLeaning() )
			{
				fBaseVel = m_fRunVel;
			}
			else
			{
				fBaseVel = m_fWalkVel;
			}
		}

	}
	return fBaseVel;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetVelocity
//
//	PURPOSE:	Get our current velocity
//
// ----------------------------------------------------------------------- //

LTVector CMoveMgr::GetVelocity() const
{
    LTVector vVel(0, 0, 0);

	if (g_pPhysicsLT && m_hObject)
	{
		g_pPhysicsLT->GetVelocity(m_hObject, &vVel);
	}

	return vVel;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::SetVelocity
//
//	PURPOSE:	Set our current velocity
//
// ----------------------------------------------------------------------- //

void CMoveMgr::SetVelocity(const LTVector &vVel)
{
	if (g_pPhysicsLT && m_hObject)
	{
		g_pPhysicsLT->SetVelocity(m_hObject, vVel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetMovementPercent
//
//	PURPOSE:	Get current percentage of maximum velocity player is moving
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetMovementPercent() const
{
    float fPerturb = 0.0f;
    LTVector vVel = GetVelocity();
    float  fMaxVel = GetMaxVelMag();

	if (fMaxVel > 0.0f)
	{
		fPerturb = vVel.Mag() / fMaxVel;
	}

	return fPerturb;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnTouchNotify
//
//	PURPOSE:	Handle our object touching something...
//
// ----------------------------------------------------------------------- //

void CMoveMgr::OnTouchNotify(CollisionInfo *pInfo, float forceMag)
{
	if (!pInfo->m_hObject) return;

	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		m_pVehicleMgr->OnTouchNotify(pInfo, forceMag);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::Load
//
//	PURPOSE:	Handle loading move mgr data
//
// ----------------------------------------------------------------------- //

void CMoveMgr::Load(ILTMessage_Read *pMsg, SaveDataState eLoadDataState)
{
	if (!pMsg) return;

	m_vSavedVel = pMsg->ReadLTVector();
	m_fLastOnGroundY = pMsg->Readfloat();

	// If we are going to a different level we should ignore our last height...

	if( (eLoadDataState == eSaveDataStateTransitionLevels) ||
		(eLoadDataState == eSaveDataStateSwitchLevels) )
	{
		m_fLastOnGroundY = MIN_ONGROUND_Y;
	}

	double fTimeLeft = pMsg->Readdouble();
	if( fTimeLeft > 0.0f )
		m_DamageTimer.Start( fTimeLeft );
	m_fDamageMoveMultiplier = pMsg->Readfloat();


	m_vWantedDims = pMsg->ReadLTVector();

	m_bLoading = true;

	m_pVehicleMgr->Load(pMsg, eLoadDataState);

	m_bJumped = pMsg->Readbool( );
	m_bSwimJumped = pMsg->Readbool( );

	m_vLeashPosition = pMsg->ReadLTVector();
	m_vLeashParameters = pMsg->ReadLTVector2();

	m_fSprintBoostPercent = pMsg->Readfloat();
	m_bSprintBoostEnabled = pMsg->Readbool();

	m_bHasSavedEncodedTransform = pMsg->Readbool( );
	if( m_bHasSavedEncodedTransform )
	{
		m_rtSavedEncodedTransform = pMsg->ReadLTRigidTransform( );
	}

	m_bWasSlideKicking = pMsg->Readbool();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::Save
//
//	PURPOSE:	Handle saving move mgr data
//
// ----------------------------------------------------------------------- //

void CMoveMgr::Save(ILTMessage_Write *pMsg, SaveDataState eSaveDataState)
{
	if (!pMsg) return;

	// Save velocity...

	LTVector myVel(0, 0, 0);

	if (m_hObject)
	{
		g_pPhysicsLT->GetVelocity(m_hObject, &myVel);
	}

	pMsg->WriteLTVector(myVel);
	pMsg->Writefloat(m_fLastOnGroundY);

	pMsg->Writedouble( m_DamageTimer.GetTimeLeft( ));
	pMsg->Writefloat( m_fDamageMoveMultiplier );
	pMsg->WriteLTVector( m_vWantedDims );

	m_pVehicleMgr->Save(pMsg, eSaveDataState);

	pMsg->Writebool( m_bJumped );
	pMsg->Writebool( m_bSwimJumped );

	pMsg->WriteLTVector( m_vLeashPosition );
	pMsg->WriteLTVector2( m_vLeashParameters );

	pMsg->Writefloat( m_fSprintBoostPercent );
	pMsg->Writebool( m_bSprintBoostEnabled );

	bool bSaveEncodingTransform = CPlayerBodyMgr::Instance( ).IsMovementEncoded( );
	pMsg->Writebool( bSaveEncodingTransform );
	if( bSaveEncodingTransform )
	{
		LTRigidTransform rtSavedEncodingTransform;
		g_pModelLT->GetMovementEncodingTransform( m_hObject, rtSavedEncodingTransform );
		pMsg->WriteLTRigidTransform( rtSavedEncodingTransform );
	}

	pMsg->Writebool(m_bWasSlideKicking);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::AddDamagePenalty
//
//	PURPOSE:	Add a movement penalty
//
// ----------------------------------------------------------------------- //
void CMoveMgr::AddDamagePenalty(float fDuration, float fMultiplier)
{
	fDuration *= GetSkillValue(eStaminaMoveDamage);
	if (fDuration > m_DamageTimer.GetTimeLeft( ))
		m_DamageTimer.Start(fDuration);
	m_fDamageMoveMultiplier = LTCLAMP(fMultiplier,0.0f,1.0f);
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetDamageMovementMultiplier
//
//	PURPOSE:	How much is the player's movement being penalized
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetDamageMovementMultiplier() const
{
	float fMult = 1.0f;

	if( !m_DamageTimer.IsTimedOut( ))
	{
		if (m_fDamageMoveMultiplier < fMult)
			fMult = m_fDamageMoveMultiplier;
	}

	return fMult;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::ClearAllRequests
//
//	PURPOSE:	Clears any pending reqeusts, called when commands have been cleared.
//
// ----------------------------------------------------------------------- //
void CMoveMgr::ClearAllRequests()
{
	g_bJumpRequested = false;
	g_bJumpFromLadder = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetServerObject
//
//	PURPOSE:	Get the server object associated with the player
//
// ----------------------------------------------------------------------- //

HOBJECT	CMoveMgr::GetServerObject() const
{
	if (!GetCharacterFX())
		return NULL;
	return GetCharacterFX()->GetServerObj();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::IsMovingQuietly
//
//  PURPOSE:	Are we being very quiet?
//
// ----------------------------------------------------------------------- //

bool CMoveMgr::IsMovingQuietly() const
{ 
	bool bRet = false;

	if ((m_dwControlFlags & BC_CFLG_DUCK) || !(m_dwControlFlags & BC_CFLG_RUN) ||
		g_pPlayerMgr->GetLeanMgr()->IsLeaning())
	{
		if (!IsFreeMovement() && !LadderMgr::Instance().IsClimbing() && IsOnGround())
		{
			bRet = true;
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::GetContainerInfo
//
//  PURPOSE:	Get the info for a specific container we are tracking
//
// ----------------------------------------------------------------------- //

CContainerInfo* CMoveMgr::GetContainerInfo( uint32 nContainer )
{
	if( m_nContainers == 0 || m_nContainers > MAX_TRACKED_CONTAINERS || nContainer > (MAX_TRACKED_CONTAINERS - 1) )
		return NULL;

	return &m_Containers[nContainer];
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::GetCrouchHeightDifference
//
//  PURPOSE:	Get the difference in height from the stand position to the crouch position...
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetCrouchHeightDifference( ) const 
{
	if( !m_hObject )
		return 0.0f;

	return (m_vStandDims.y - m_vCrouchDims.y);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::GetStandHeightDifference
//
//  PURPOSE:	Get the difference in height from the stand position to the current position...
//
// ----------------------------------------------------------------------- //

float CMoveMgr::GetCurrentHeightDifference( ) const
{
	if( !m_hObject )
		return 0.0f;

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

	return (m_vStandDims.y - vDims.y);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::UpdateMovementEncoding
//
//  PURPOSE:	Handle movement that is encoded in animations...
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateMovementEncoding( )
{
	if( !CPlayerBodyMgr::Instance( ).IsMovementEncoded( ))
	{
		UpdateNormalMotion( );
		return;
	}

	if (LadderMgr::Instance().IsClimbing())
	{
		// Using movement encoding only.
		LTVector vAccel(0, 0, 0);
		g_pPhysicsLT->SetAcceleration(GetObject(), vAccel);
		g_pPhysicsLT->SetVelocity(GetObject(), vAccel);

		m_bGravityOverride = true;
		m_fTotalContainerGravity = 0.0f;
	}

	// Movement encoding may move the player in or out of a container so update them...
	UpdateContainerMotion( );

	// Update m_bOnGround data member...
	UpdateOnGround();

	// Zero out the acceleration to start with...
	g_pPhysicsLT->SetAcceleration( m_hObject, LTVector::GetIdentity() );

	// Get the current movement encoding transformation...
	LTRigidTransform tMovementEncoding;
	g_pModelLT->GetMovementEncodingTransform( m_hObject, tMovementEncoding );

	// Get the current object transformation...
	LTRigidTransform tObject;
	g_pLTClient->GetObjectTransform( m_hObject, &tObject );

	// The transform gets reset every update but we may have had an accumulated transform
	// not previously moved to because the save occurred before the last MoveMgr update.
	// That unused accumulated transfrom gets saved and used here.
	if( m_bHasSavedEncodedTransform )
	{
		tMovementEncoding *= m_rtSavedEncodedTransform;
		
		// Only need to initially factor in the saved transformm since MoveMgr resets each update.
		m_rtSavedEncodedTransform.Init( );
		m_bHasSavedEncodedTransform = false;
	}

	// Calculate the world space movement encoding transform...
	LTRigidTransform tMovementWS = tObject * tMovementEncoding;

	// check to see if we need to synchronize the player velocity to the movement encoding
	if( CPlayerBodyMgr::Instance( ).GetMovementDescriptor( ) == kAD_MOV_Encode_Velocity )
	{
		// divide the distance by the time to get the velocity
		LTVector vCalculatedVelocity = tMovementWS.m_vPos - tObject.m_vPos;
        
		float fFrameTime = g_pLTClient->GetFrameTime( );
		if( fFrameTime > 0.0f )
		{
			vCalculatedVelocity /= fFrameTime;
		}
		        
		// Maintain the current vertical velocity.  Otherwise the laws of gravity are not obeyed...
		LTVector vCurrentVelocity;
		g_pPhysicsLT->GetVelocity( m_hObject, &vCurrentVelocity );
		vCalculatedVelocity.y += vCurrentVelocity.y;

		g_pPhysicsLT->SetVelocity( m_hObject, vCalculatedVelocity );
	}
	else
	{
		// move the player
		if (CPlayerBodyMgr::Instance( ).GetMovementDescriptor() == kAD_MOV_Encode_NP)
			g_pLTClient->SetObjectPos( m_hObject, tMovementWS.m_vPos );
		else
			g_pPhysicsLT->MoveObject( m_hObject, tMovementWS.m_vPos, 0 );
	}

	// Don't allow rotation when doing a scripted animation...
	if( !CPlayerBodyMgr::Instance( ).IsPlayingSpecial( ) && !LadderMgr::Instance().IsClimbing() && !SpecialMoveMgr::Instance().IsActive())
	{
		UpdateRotation( );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::SetCharacterFX
//
//  PURPOSE:	Sets the characterfx object to use for the local player.
//
// ----------------------------------------------------------------------- //

void CMoveMgr::SetCharacterFX(CCharacterFX* pFX)
{
	// Check if we already have this characterfx.
	if( m_pCharFX == pFX )
		return;

	double fDamageTimerTimeLeft = ( m_DamageTimer.IsStarted( )) ? m_DamageTimer.GetTimeLeft( ) : 0.0f;

	if( m_pCharFX )
	{
		// Reset any timers that rely on the object timer.
		if( !pFX )
		{
			m_DamageTimer.SetEngineTimer( SimulationTimer::Instance( ));
			m_YawTimer.SetEngineTimer( SimulationTimer::Instance( ));
		}
	}

	m_pCharFX = pFX;

	// Use the new object for timers.
	if( m_pCharFX )
	{
		m_DamageTimer.SetEngineTimer( ObjectContextTimer( m_pCharFX->GetServerObj( )));
		m_YawTimer.SetEngineTimer( ObjectContextTimer( m_pCharFX->GetServerObj( )));
	}

	if( fDamageTimerTimeLeft > 0.0f )
		m_DamageTimer.Start( fDamageTimerTimeLeft );
	else
		m_DamageTimer.Stop( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::JumpFromLadder
//
//  PURPOSE:	Handle jumping off of a ladder
//
// ----------------------------------------------------------------------- //

void CMoveMgr::JumpFromLadder()
{
	if (!LadderMgr::Instance().IsClimbing()) return;


	CLadderFX *pLadder = LadderMgr::Instance().GetLadder();

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hObject, &vPos);
	m_fLastOnGroundY = vPos.y;

	//you don't jump far off of the ladder
	float fJumpVel = (m_fJumpVel * m_fJumpMultiplier) * 0.33f;
	
	LTRotation rRot = pLadder->GetRotation();
	rRot.Rotate(rRot.Right(),DEG2RAD(30.0f));
	LTVector vJump = rRot.Forward();
	vJump *= fJumpVel;

	g_pPhysicsLT->SetVelocity(m_hObject, vJump);

	LadderMgr::Instance().ReleaseLadder();

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::UpdateGotoNodeMovement
//
//  PURPOSE:	Handle movement towards a goto node...
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateGotoNodeMovement( )
{
	HOBJECT hGotoNode = g_pPlayerMgr->GetPlayerNodeGoto( );
	if( hGotoNode )
	{
		LTVector vPlayerPos;
		g_pLTClient->GetObjectPos( m_hObject, &vPlayerPos );

		LTVector vNodePos;
		g_pLTClient->GetObjectPos( hGotoNode, &vNodePos );

		// Determine if we overshot the node...
		LTVector vOldDirToDest;
		LTVector vNewDirToDest;

		vOldDirToDest = vNodePos - m_vLastPos;
		vOldDirToDest.y = 0.0f;
		vOldDirToDest.Normalize();

		vNewDirToDest = vNodePos - vPlayerPos;
		vNewDirToDest.y = 0.0f;
		vNewDirToDest.Normalize();

		// Test if we have reached the node...
		if( vOldDirToDest.Dot( vNewDirToDest ) < 0.0f )
		{
			// Reached destination...
			g_pPlayerMgr->ReachedNodePosition( );
		}
		else
		{
			// Use this as a fail safe just in case the player was never actually able to reach the node position
			LTVector vDiff = m_vLastPos - vPlayerPos;
			if( GetVelocity().Mag() > 0.1f && .01f > vDiff.Mag() )
			{
				g_pPlayerMgr->ReachedNodePosition( );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::UpdateRotation
//
//  PURPOSE:	Update the rotation movement...
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateRotation( )
{
	// Get the current camera rotation.
	LTRotation rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation();
	float fCamYaw = Eul_FromQuat( rRot, EulOrdYXZr ).x;

	rRot = LTRotation( 0.0f, fCamYaw, 0.0f );

	// See if the player is idle...
	if( g_vtPlayerYawShuffleEnabled.GetFloat() && !IsMoving() )
	{
		LTRotation rPlayerRot;
		g_pLTClient->GetObjectRotation( m_hObject, &rPlayerRot );

		LTVector vCamF = rRot.Forward();
		LTVector vPlayerF = rPlayerRot.Forward();

		float fFreeRangeCos = LTCos( MATH_DEGREES_TO_RADIANS(g_vtPlayerYawFreeRangeAngle.GetFloat() ) );
		float fRotateCos = LTCos( MATH_DEGREES_TO_RADIANS(g_vtPlayerYawRotateAngle.GetFloat() ) );

		float fDot = vPlayerF.Dot( vCamF );

		if( fDot < fRotateCos )
		{
			m_bInterpolateYaw = false;
		}
		else if( fDot < fFreeRangeCos )
		{
			if( !m_bInterpolateYaw )
			{
				m_fYawSnapShot = Eul_FromQuat( rPlayerRot, EulOrdYXZr ).x;
				m_bInterpolateYaw = true;
				m_YawTimer.Start( g_vtPlayerYawInterpolateTime.GetFloat() );

				// Expand the Yaw limits of aim tracking to allow for the rotation of the lower body back to center...
				LTRect2f rLimits;
				if( CPlayerBodyMgr::Instance().GetAimTrackingLimits( rLimits ) )
				{
					rLimits.m_vMin.y = -MATH_PI;
					rLimits.m_vMax.y = MATH_PI;

					CPlayerBodyMgr::Instance().SetAimTrackingLimits( rLimits );
				}
			}
		}
		else
		{
			// Continue rotating if not yet complete...
			if( !m_bInterpolateYaw )
			{
				// Otherwise allow free range of camera rotation by not rotating the move object...
				return;
			}
		}

		if( m_bInterpolateYaw )
		{
			if( m_YawTimer.IsTimedOut( ))
			{
				m_bInterpolateYaw = false;
				CPlayerBodyMgr::Instance().SetAimTrackingDefaultLimits();
			}
			else
			{
				double fDuration = m_YawTimer.GetDuration();
				double fTimeLeft = m_YawTimer.GetTimeLeft();
				float fT = (float)(( fDuration - fTimeLeft ) / fDuration);

				rRot.Slerp( LTRotation( 0.0f, m_fYawSnapShot, 0.0f ), rRot, fT );

				// Determine which direction the player is rotating...
				if( rPlayerRot.Right().Dot( vCamF ) > 0.0f )
				{
					if( !CPlayerBodyMgr::Instance().IsLocked( kAPG_MovementDir, kAP_MDIR_ShuffleRight ) )
					{
						CPlayerBodyMgr::Instance().SetAnimProp( kAPG_MovementDir, kAP_MDIR_ShuffleRight, CPlayerBodyMgr::kLocked );
					}
				}
				else
				{
					if( !CPlayerBodyMgr::Instance().IsLocked( kAPG_MovementDir, kAP_MDIR_ShuffleLeft ) ) 
					{
						CPlayerBodyMgr::Instance().SetAnimProp( kAPG_MovementDir, kAP_MDIR_ShuffleLeft, CPlayerBodyMgr::kLocked );
					}
				}
			}
		}
		else
		{
			// Maintain the difference of the camera rotation to the player...
			LTRotation rDiff = ~m_rLastCamRot * rRot;
			rRot = rPlayerRot * rDiff;
		}
	}

	g_pLTClient->SetObjectRotation( m_hObject, rRot );
	m_rLastCamRot = LTRotation( 0.0f, fCamYaw, 0.0f );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::GetCurrentMinProximity
//
//  PURPOSE:	Get the min proximity from the current player weapon
//
// ----------------------------------------------------------------------- //

LTVector2 CMoveMgr::GetCurrentMinProximity()
{
	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pClientWeapon)
		return pClientWeapon->GetMinProximity();
	return LTVector2(0.0f, 0.0f);
}

// EOF
