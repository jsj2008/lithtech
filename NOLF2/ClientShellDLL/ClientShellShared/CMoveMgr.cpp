// ----------------------------------------------------------------------- //
//
// MODULE  : CMoveMgr.cpp
//
// PURPOSE : Client side player movement mgr - Implementation
//
// CREATED : 10/2/98
//
// (c) 1998-2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "ClientMultiplayerMgr.h"
#include "ProfileMgr.h"
#include "TargetMgr.h"
#include "GadgetDisabler.h"
#include "VolumeBrushFX.h"

#define SPECTATOR_ACCELERATION			100000.0f
#define MIN_ONGROUND_Y					-10000000.0f
#define DEFAULT_WORLD_GRAVITY			-2000.0f

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
		m_Link.m_pData = this;
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
    LTLink   m_Link;
};

VarTrack	g_vtPlayerGravity;
VarTrack	g_vtInAirAccelMultiplier;

VarTrack	g_vtPlayerViewModel;
VarTrack	g_vtPlayerViewOffsetX;
VarTrack	g_vtPlayerViewOffsetY;
VarTrack	g_vtPlayerViewOffsetZ;
VarTrack	g_vtPlayerViewScale;

VarTrack	g_vtCamLandMinHeight;
VarTrack	g_vtFallDamageMinHeight;
VarTrack	g_vtFallDamageMaxHeight;
VarTrack	g_vtFallDamageMin;
VarTrack	g_vtFallDamageMax;

VarTrack	g_vtCamLandMoveDist;
VarTrack	g_vtCamLandDownTime;
VarTrack	g_vtCamLandUpTime;

VarTrack	g_vtCamLandRollVal;
VarTrack	g_vtCamLandRollTime1;
VarTrack	g_vtCamLandRollTime2;

VarTrack	g_vtCamLandPitchVal;
VarTrack	g_vtCamLandPitchTime1;
VarTrack	g_vtCamLandPitchTime2;

VarTrack	g_vtSlideToStopTime;

VarTrack	g_vtMaxPushYVelocity;


LTBOOL g_bJumpRequested  = LTFALSE;

extern VarTrack g_vtVehicleFallDamageMinHeight;
extern VarTrack g_vtVehicleFallDamageMaxHeight;

CMoveMgr* g_pMoveMgr = LTNULL;

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

    m_hObject = LTNULL;

	m_bLoading = LTFALSE;
	m_vSavedVel.Init();

	m_vTotalCurrent.Init();
	m_fTotalViscosity = 0.0f;

	m_vGroundNormal.Init();

	// Always have this...
	m_pVehicleMgr = debug_new(CVehicleMgr);

	InitWorldData();

	m_DamageTimer.Stop();
	
	m_bJumped	= LTFALSE;
    m_bRunLock	= LTFALSE;

	m_bGravityOverride = false;
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
	g_pMoveMgr = LTNULL;

	TermLevel();

	if (m_pVehicleMgr)
	{
		debug_delete(m_pVehicleMgr);
		m_pVehicleMgr = LTNULL;
	}

	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::Init
//
//	PURPOSE:	Initialize mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CMoveMgr::Init()
{
	m_CV_SpectatorSpeedMul.Init(g_pLTClient, "SpectatorSpeedMul", NULL, 1.0f);

 	g_vtFallDamageMinHeight.Init(g_pLTClient, "FallDamageMinHeight", LTNULL, 300.0f);
    g_vtFallDamageMaxHeight.Init(g_pLTClient, "FallDamageMaxHeight", LTNULL, 500.0f);
    g_vtFallDamageMin.Init(g_pLTClient, "FallDamageMin", LTNULL, 5.0f);
    g_vtFallDamageMax.Init(g_pLTClient, "FallDamageMax", LTNULL, 500.0f);

    g_vtCamLandMinHeight.Init(g_pLTClient, "CamLandMinHeight", LTNULL, 50.0f);
    g_vtCamLandMoveDist.Init(g_pLTClient, "CamLandMoveDist", LTNULL, -15.0f);
    g_vtCamLandDownTime.Init(g_pLTClient, "CamLandDownTime", LTNULL, 0.15f);
    g_vtCamLandUpTime.Init(g_pLTClient, "CamLandUpTime", LTNULL, 0.4f);

    g_vtCamLandRollVal.Init(g_pLTClient, "CamLandRollVal", LTNULL, 5.0f);
    g_vtCamLandRollTime1.Init(g_pLTClient, "CamLandRollTime1", LTNULL, 0.05f);
    g_vtCamLandRollTime2.Init(g_pLTClient, "CamLandRollTime2", LTNULL, 0.15f);

    g_vtCamLandPitchVal.Init(g_pLTClient, "CamLandPitchVal", LTNULL, 5.0f);
    g_vtCamLandPitchTime1.Init(g_pLTClient, "CamLandPitchTime1", LTNULL, 0.05f);
    g_vtCamLandPitchTime2.Init(g_pLTClient, "CamLandPitchTime2", LTNULL, 0.15f);

    g_vtPlayerGravity.Init(g_pLTClient, "PlayerGravity", LTNULL, DEFAULT_WORLD_GRAVITY);
	g_vtInAirAccelMultiplier.Init(g_pLTClient, "InAirAccelMultiplier", LTNULL, 0.1f);

    g_vtSlideToStopTime.Init(g_pLTClient, "SlideToStopTime", LTNULL, 0.1f);

    g_vtMaxPushYVelocity.Init(g_pLTClient, "PusherMaxYVelocity", LTNULL, 100.0f);


 	// Init some defaults.  These should NEVER get used because we don't
	// have our object until the server sends the physics update.

	m_fSwimVel			= 0.0f;
	m_fWalkVel			= 0.0f;
	m_fRunVel			= 400.0f;
	m_fJumpVel			= 550.0f;
	m_fSuperJumpVel		= 550.0f;
	m_fMoveMultiplier	= 1.0f;

	m_pVehicleMgr->Init();

	// Init world specific data members...

	InitWorldData();

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if (pProfile)
	{
		SetRunLock(pProfile->m_bAlwaysRun);
	}

    return LTTRUE;
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

	m_fGravity				= DEFAULT_WORLD_GRAVITY;

	dl_TieOff(&m_Pushers);
    
	m_eBodyContainerCode	= CC_NO_CONTAINER;
    m_bBodyInLiquid         = LTFALSE;
    m_bBodyOnLadder         = LTFALSE;
    m_bOnGround             = LTTRUE;
    m_bOnLift               = LTFALSE;
    m_bFalling              = LTFALSE;
    m_bAllowMovement        = LTTRUE;
	m_eStandingOnSurface	= ST_UNKNOWN;
    m_bSwimmingOnSurface    = LTFALSE;
    m_bSwimmingJump         = LTFALSE;
    m_bCanSwimJump          = LTFALSE;
	m_hStandingOnPoly		= INVALID_HPOLY;
	m_bUsingPlayerModel		= LTFALSE;

    m_bDuckLock             = LTFALSE;


	m_fBaseMoveAccel		= 0.0f;
	m_fMoveAccelMultiplier	= 1.0f;
	m_fJumpMultiplier		= 1.0f;

    m_eCurContainerCode		= CC_NO_CONTAINER;
	m_eLastContainerCode	= CC_NO_CONTAINER;
	m_nContainers			= 0;

    m_bFirstAniUpdate       = LTTRUE;

    m_pCharFX               = LTNULL;

	m_bForceToServerPos		= LTTRUE;

	// Don't overwrite our last ground pos if loading a game...
	
	if( !m_bLoading )
	{
		m_fLastOnGroundY	= MIN_ONGROUND_Y;
	}
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
    LTLink *pCur, *pNext;
	for (pCur=m_Pushers.m_pNext; pCur != &m_Pushers; pCur=pNext)
	{
		pNext = pCur->m_pNext;
		Pusher::GetBank()->Delete((Pusher*)pCur->m_pData);
	}
	dl_TieOff(&m_Pushers);

	m_pVehicleMgr->TermLevel();

	m_DamageTimer.Stop();

	if(m_hObject)
	{
		g_pLTClient->RemoveObject(m_hObject);
		m_hObject = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateMouseStrafeFlags
//
//	PURPOSE:	Update the mouse strafe flags
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateMouseStrafeFlags(float *pAxisOffsets)
{
	if (pAxisOffsets[0] < 0.0f)
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}
	else if (pAxisOffsets[0] > 0.0f)
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if (pAxisOffsets[1] < 0.0f)
	{
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}
	else if (pAxisOffsets[1] > 0.0f)
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

	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		m_pVehicleMgr->UpdateControlFlags();
		m_dwControlFlags = m_pVehicleMgr->GetControlFlags();
	}
	else
	{
		UpdateNormalControlFlags();
	}
	
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
	if( !g_pInterfaceMgr->AllowCameraMovement() ) 
	{
		// Don't clear the duck flag...
		m_dwControlFlags |= (m_dwLastControlFlags & BC_CFLG_DUCK);
		return;
	}

	// If we're doing mouse strafing, hook us up...

	if (g_pPlayerMgr->IsMouseStrafing())
	{
		float offsets[3];
		g_pLTClient->GetAxisOffsets(offsets);

		UpdateMouseStrafeFlags(offsets);
	}


	// Determine what commands are currently on...

    if (g_pLTClient->IsCommandOn(COMMAND_ID_LEFT))
	{
		m_dwControlFlags |= BC_CFLG_LEFT;
	}

    if (g_pLTClient->IsCommandOn(COMMAND_ID_RIGHT))
	{
		m_dwControlFlags |= BC_CFLG_RIGHT;
	}

	//if you are a ghost, you can look around, but that's it
	if (g_pPlayerMgr->GetPlayerState() == PS_GHOST) return;


	// Use the player's user flags instead of checking this flag directly
	// (this insures that the camera will be moved accurately...)

	if (g_pLTClient->IsCommandOn(COMMAND_ID_DUCK) || m_bDuckLock)
	{
		m_dwControlFlags |= BC_CFLG_DUCK;
	}

	if (g_pPlayerMgr->IsCarryingHeavyObject())
	{
		m_dwControlFlags &= ~BC_CFLG_DUCK;
	}

	// Force the duck flag back on if we can't stand up in our current position
	if (((m_dwControlFlags & BC_CFLG_DUCK) == 0) && ((m_dwLastControlFlags & BC_CFLG_DUCK) != 0))
	{
		if (!CanStandUp())
			m_dwControlFlags |= BC_CFLG_DUCK;
	}

	// Only process jump and run if we aren't ducking and aren't leaning...

	if (!(m_dwControlFlags & BC_CFLG_DUCK) && !g_pPlayerMgr->GetLeanMgr()->IsLeaning())
	{
		if (g_pLTClient->IsCommandOn(COMMAND_ID_RUN) ^ (m_bRunLock != LTFALSE))
		{
			m_dwControlFlags |= BC_CFLG_RUN;
		}

		//also can't jump while carrying a body
		if (g_pLTClient->IsCommandOn(COMMAND_ID_JUMP) && !g_pPlayerMgr->IsCarryingHeavyObject())
		{
			m_dwControlFlags |= BC_CFLG_JUMP;
		}
	}

	// If we're not finished moving up after ducking, we can't jump...
	if (!g_pPlayerMgr->IsFinishedDucking())
	{
		m_dwControlFlags &= ~BC_CFLG_JUMP;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_FORWARD))
	{
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}

	if (g_pLTClient->IsCommandOn(COMMAND_ID_REVERSE))
	{
		m_dwControlFlags |= BC_CFLG_REVERSE;
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


	if (!g_pPlayerMgr->IsCarryingHeavyObject())
	{
		if (g_pLTClient->IsCommandOn(COMMAND_ID_FIRING))
		{
			m_dwControlFlags |= BC_CFLG_FIRING;
		}

		if (g_pLTClient->IsCommandOn(COMMAND_ID_ALT_FIRING))
		{
			m_dwControlFlags |= BC_CFLG_ALT_FIRING;
		}


		//special case for activating a gadget target
		if (g_pLTClient->IsCommandOn(COMMAND_ID_ACTIVATE) && g_pPlayerMgr->FireOnActivate())
		{
			m_dwControlFlags |= BC_CFLG_FIRING;
		}

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
		 (m_dwControlFlags & BC_CFLG_ROLL_LEFT) ||
		 (m_dwControlFlags & BC_CFLG_ROLL_RIGHT) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_LEFT) ||
		 (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) ||
		 (m_dwControlFlags & BC_CFLG_JUMP))
	{
		m_dwControlFlags |= BC_CFLG_MOVING;
	}

	if ((m_dwControlFlags & BC_CFLG_JUMP) &&
		!(m_dwLastControlFlags & BC_CFLG_JUMP))
	{
        g_bJumpRequested = LTTRUE;
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

	if (!m_bBodyOnLadder && m_bJumped && !IsHeadInLiquid()) return;


	// Update the total viscosity for the frame...

	m_fTotalViscosity += pInfo->m_fViscosity;


	// Do REAL viscosity dampening (i.e., actually change our velocity)...

    LTVector vVel = GetVelocity();
    LTVector vCurVel = vVel;

	if (pInfo->m_fViscosity > 0.0f && VEC_MAG(vCurVel) > 1.0f)
	{
        LTVector vDir = vCurVel;
		vDir.Normalize();

        LTFLOAT fAdjust = MAX_CONTAINER_VISCOSITY * pInfo->m_fViscosity;
		fAdjust *= g_pGameClientShell->GetFrameTime();

		vVel = (vDir * fAdjust);

		if (vVel.MagSqr() < vCurVel.MagSqr())
		{
			vVel = vCurVel - vVel;
		}
		else
		{
			vVel.Init();
		}

		g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
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
	return m_pVehicleMgr->CanSave(); 
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
    m_bBodyInLiquid = LTTRUE;

	// Ladder physics takes priority over liquid...

	if (m_bBodyOnLadder) return;

    LTBOOL bHeadInLiquid = IsHeadInLiquid();

    LTVector vVel = GetVelocity();

    LTVector curAccel;
	g_pPhysicsLT->GetAcceleration(m_hObject, &curAccel);

	// Handle floating around on the surface...

	if (m_bSwimmingOnSurface)
	{
		LTBOOL bMoving = ((curAccel.Length() > 0.01f) || (vVel.Length() > 0.01f));

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

	}
	else if (bHeadInLiquid)
	{
		// Disable gravity.
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_GRAVITY);

		curAccel.y += pInfo->m_fGravity;
	}

	g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
	g_pPhysicsLT->SetAcceleration(m_hObject, &curAccel);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateOnLadder
//
//	PURPOSE:	Update movement when on a ladder
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateOnLadder(CContainerInfo *pInfo)
{
    m_bBodyOnLadder = LTTRUE;
	m_hLadderObject = pInfo->m_hObject;	
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_GRAVITY);
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


    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hObject, &vPos);

	// Clear surface we're standing on...

    m_bOnLift            = LTFALSE;
	m_hStandingOnPoly    = INVALID_HPOLY;
    m_bOnGround          = LTFALSE;

	if (Info.m_hObject)
	{
		m_bOnGround = LTTRUE;

		// If we didn't jump, tell the server we landed (if we jumped, then
		// this will be sent in UpdateStartMotion()...

		if (!m_bJumped && m_bFalling)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
			cMsg.Writeuint8(CP_MOTION_STATUS);
			cMsg.Writeuint8(MS_LANDED);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
		}

        m_bFalling = LTFALSE;

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
            m_bOnLift = LTTRUE;
		}


		// See if we fell...

        LTFLOAT fDistFell = m_fLastOnGroundY - vPos.y;
		m_fLastOnGroundY = vPos.y;

		if (fDistFell > 1.0f && !m_bBodyOnLadder)
		{
			HandleFallLand(fDistFell);
		}

		// Handle landing after jumping...

		if (m_bJumped)
		{
            m_bJumped = LTFALSE;
            UpdateStartMotion(LTTRUE);
		}
		
		// Save the normal of the surface we are standing on...

		m_vGroundNormal = Info.m_Plane.m_Normal;

		// Make sure we fall down step slopes...

		if (m_vGroundNormal.y < 0.707)
		{
		   m_bOnGround = LTFALSE;
		}

		// Don't allow standing on characters
		if (dwUserFlags & USRFLG_CHARACTER)
		{
			m_bOnGround = LTFALSE;
		}
	}


	// Cases when we can't be on the ground...

	if (m_bJumped || IsHeadInLiquid())
	{
		m_bOnGround = LTFALSE;
	}

	if (!m_bOnGround)
	{
		if (g_pPlayerMgr->IsSpectatorMode())
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
                m_bJumped = LTFALSE;
                UpdateStartMotion(LTTRUE);
			}
		}
	}


	// Update our standing on surface...

	if (m_bBodyOnLadder)
	{
		m_eStandingOnSurface = ST_LADDER;
	}
	else if (m_bBodyInLiquid)
	{
		m_eStandingOnSurface = ST_LIQUID;
	}


	// See if we just started falling (i.e., we're not in liquid, we didn't
	// jump, and there is a bit of distance between us and the ground)...

    LTBOOL bFreeMovement = (IsFreeMovement() || m_bBodyOnLadder || g_pPlayerMgr->IsSpectatorMode());

	if (!bFreeMovement && !m_bJumped && !CanDoFootstep() && !m_bFalling)
	{
        m_bFalling = LTTRUE;

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

LTBOOL CMoveMgr::CanDoFootstep()
{
	// This is basically just the same as m_bOnGround, however, when going
	// up/down stairs often m_bOnGround is false...So...

	// No footsteps if standing on sky/invisible texture (this should
	// only happen in really special case levels)...

	if (m_eStandingOnSurface == ST_INVISIBLE || m_eStandingOnSurface == ST_SKY)
	{
        return LTFALSE;
	}

	// NOTE the order of these trivial case returns is important...

    if (IsBodyOnLadder()) return LTTRUE;     // Even if underwater
    if (IsHeadInLiquid()) return LTFALSE;    // Even if on the ground
    if (m_bOnGround) return LTTRUE;          // Even if we just jumped
    if (m_bJumped) return LTFALSE;


	// This is sort of lame...but can't think of a better approach.  Cast
	// a ray down and see if we hit something...

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hObject, &vPos);

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	iQuery.m_From  = vPos;
	iQuery.m_To	   = iQuery.m_From;
	iQuery.m_To.y -= 150.0f;

	// Don't hit ourself...

    HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), m_hObject, LTNULL};

	iQuery.m_FilterFn  = ObjListFilterFn;
	iQuery.m_pUserData = hFilterList;

	if (!g_pLTClient->IntersectSegment(&iQuery, &iInfo))
	{
		m_eStandingOnSurface = ST_UNKNOWN;
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::HandleFallLand
//
//	PURPOSE:	Handle landing
//
// ----------------------------------------------------------------------- //

void CMoveMgr::HandleFallLand(LTFLOAT fDistFell)
{
	if (IsHeadInLiquid()) return;

	float fMinLandHeight = g_vtCamLandMinHeight.GetFloat();
	float fDamageMinHeight = m_pVehicleMgr->IsVehiclePhysics() ? g_vtVehicleFallDamageMinHeight.GetFloat() : g_vtFallDamageMinHeight.GetFloat();
	float fDamageMaxHeight = m_pVehicleMgr->IsVehiclePhysics() ? g_vtVehicleFallDamageMaxHeight.GetFloat() : g_vtFallDamageMaxHeight.GetFloat();
	float fFallDamageMin = g_vtFallDamageMin.GetFloat();
	float fFallDamageMax = g_vtFallDamageMax.GetFloat();

	if (fDistFell > fMinLandHeight)
	{
		// Adjust camera...

		CameraDelta delta;

		delta.PosY.fVar		= g_vtCamLandMoveDist.GetFloat();
		delta.PosY.fTime1	= g_vtCamLandDownTime.GetFloat();
		delta.PosY.fTime2	= g_vtCamLandUpTime.GetFloat();
		delta.PosY.eWave1	= Wave_SlowOff;
		delta.PosY.eWave2	= Wave_SlowOff;

		if (fDistFell >= fDamageMinHeight)
		{
			float fHeightRange = fDamageMaxHeight - fDamageMinHeight;
			float fDamageRange = fFallDamageMax - fFallDamageMin;
			float fDamage = fFallDamageMin + fDamageRange * (fDistFell - fDamageMinHeight) / fHeightRange;

			// Send damage message to server...

			// See if we're in a SafteyNet, if so, no damage...

			if (!g_pPlayerMgr->InSafetyNet())
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
				cMsg.Writeuint8(CP_DAMAGE);
				cMsg.Writeuint8(DT_CRUSH);
				cMsg.Writefloat(fDamage);
				// Ground caused damage...
				cMsg.WriteLTVector(LTVector(0.0f, 1.0f, 0.0f));
				cMsg.Writeuint8(0);
				cMsg.WriteObject(g_pLTClient->GetClientObject());
				g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
			}

			// Tweak camera...

			delta.Roll.fVar		= DEG2RAD(g_vtCamLandRollVal.GetFloat());
			delta.Roll.fVar		= GetRandom(0, 1) == 1 ? -delta.Roll.fVar : delta.Roll.fVar;
			delta.Roll.fTime1	= g_vtCamLandRollTime1.GetFloat();
			delta.Roll.fTime2	= g_vtCamLandRollTime2.GetFloat();
			delta.Roll.eWave1	= Wave_SlowOff;
			delta.Roll.eWave2	= Wave_SlowOff;

			delta.Pitch.fVar	= DEG2RAD(g_vtCamLandPitchVal.GetFloat());
			delta.Pitch.fTime1	= g_vtCamLandPitchTime1.GetFloat();
			delta.Pitch.fTime2	= g_vtCamLandPitchTime2.GetFloat();
			delta.Pitch.eWave1	= Wave_SlowOff;
			delta.Pitch.eWave2	= Wave_SlowOff;

			// Play land sound...

			LTBOOL bPlaySound = LTFALSE;
			if (!IsHeadInLiquid())
			{
				bPlaySound = (m_bBodyInLiquid ? m_bOnGround : (m_eStandingOnSurface != ST_INVISIBLE));
			}

			if (bPlaySound)
			{
				char* pNormalSounds[] = { "Chars\\Snd\\player\\landing1.wav", "Chars\\Snd\\player\\landing2.wav" };
				char* pVehicleSounds[] = { "Snd\\vehicle\\snowmobile\\landing1.wav", "Snd\\vehicle\\snowmobile\\landing2.wav" };
				char** pSounds = (m_pVehicleMgr->IsVehiclePhysics() ? pVehicleSounds : pNormalSounds);
				
				g_pClientSoundMgr->PlaySoundLocal(pSounds[GetRandom(0,1)], SOUNDPRIORITY_PLAYER_HIGH);
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
			g_pLTClient->SetObjectPos(m_hObject, &vServerPos);
			m_bForceToServerPos = LTFALSE;
		}
	}

	if (!m_bAllowMovement || !g_pDamageFXMgr->AllowMovement() )
	{
		// Clear accel/velocity...
	    LTVector vVec;
		g_pPhysicsLT->GetAcceleration(m_hObject, &vVec);

		vVec.x = vVec.z = 0.0f;
		if (vVec.y > 0.0f) vVec.y = 0.0f;

		g_pPhysicsLT->SetAcceleration(m_hObject, &vVec);

		// Dampen velocity...
		g_pPhysicsLT->GetVelocity(m_hObject, &vVec);
		vVec.x *= 0.5f;
		vVec.z *= 0.5f;
		if (vVec.y > 0.0f) vVec.y = 0.0f;

		g_pPhysicsLT->SetVelocity(m_hObject, &vVec);
		return;
	}

	// Make sure this gets reset...
	m_fTotalContainerGravity = 0.0f;
	m_bGravityOverride = false;

	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		m_pVehicleMgr->UpdateMotion();
	}
	else
	{
		UpdateNormalMotion();
	}
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
    LTFLOAT fTime = g_pLTClient->GetTime();

	// Zero out the acceleration to start with.

    LTVector vAccel(0, 0, 0);
	g_pPhysicsLT->SetAcceleration(m_hObject, &vAccel);


	// Update motion due to any containers

	UpdateContainerMotion();


	// Update m_bOnGround data member...

	UpdateOnGround();


	// Set our current rotation.

    LTRotation rRot;
	g_pPlayerMgr->GetPlayerRotation(rRot);
	g_pLTClient->SetObjectRotation(m_hObject, &rRot);


    LTVector myVel = GetVelocity();

    LTVector moveVel = myVel;
	moveVel.y = 0;

	float fMaxVel;
    LTBOOL bHeadInLiquid = IsHeadInLiquid();
    LTBOOL bInLiquid     = (bHeadInLiquid || m_bBodyInLiquid);
    LTBOOL bFreeMovement = (IsFreeMovement() || m_bBodyOnLadder || g_pPlayerMgr->IsSpectatorMode());

	fMaxVel = GetMaxVelMag();

	// Determine if we are currently trying to jump...

    LTBOOL bJumping = LTFALSE;
	if (bFreeMovement)
	{
		if (m_dwControlFlags & BC_CFLG_JUMP)
		{
            bJumping = LTTRUE;
		}

        g_bJumpRequested = LTFALSE;
	}
	else // normal case
	{
		bJumping = g_bJumpRequested && !m_bJumped;
	}



	// Limit velocity...

	if (m_bBodyInLiquid || m_bBodyOnLadder)
	{
        LTBOOL bCapY = LTTRUE;

		// Cap the velocity in all directions, unless we're falling into,
		// or jumping out of, liquid...

		if (m_bBodyInLiquid && !m_bBodyOnLadder)
		{
			if (!bHeadInLiquid && !m_bSwimmingOnSurface)
			{
				// Don't cap velocity in the y direction...

                bCapY = LTFALSE;
			}
		}

		if (bCapY)
		{
			moveVel = myVel;
		}

		if (moveVel.Length() > fMaxVel)
		{
			moveVel.Normalize();
			moveVel *= fMaxVel;

			myVel.x = moveVel.x;
			myVel.z = moveVel.z;
			if (bCapY)
			{
				myVel.y = moveVel.y;
			}

			g_pPhysicsLT->SetVelocity(m_hObject, &myVel);
		}
	}
	else if (m_bOnGround && !g_pPlayerMgr->IsSpectatorMode() && !bJumping)
	{
		float fCurLen = (float)sqrt(myVel.x*myVel.x + myVel.z*myVel.z);
		if (fCurLen > fMaxVel)
		{
			myVel *= (fMaxVel/fCurLen);

			g_pPhysicsLT->SetVelocity(m_hObject, &myVel);
		}
	}
	else if (moveVel.Length() > fMaxVel)
	{
		// Don't cap velocity in the y direction...

		moveVel.Normalize();
		moveVel *= fMaxVel;

		myVel.x = moveVel.x;
		myVel.z = moveVel.z;

		g_pPhysicsLT->SetVelocity(m_hObject, &myVel);
	}



	// See if we just broke the surface of water...

	if ((IsLiquid(m_eLastContainerCode) && !bHeadInLiquid) && !m_bOnGround && !m_bBodyOnLadder )
	{
        m_bSwimmingOnSurface = LTTRUE;
	}
	else if (bHeadInLiquid)  // See if we went back under...
	{
        m_bSwimmingOnSurface = LTFALSE;
        m_bCanSwimJump       = LTTRUE;
	}
	else if( !m_bBodyInLiquid )
	{
		m_bSwimmingOnSurface = LTFALSE;
		m_bCanSwimJump       = LTFALSE;
	}


	// If we're doing a swimming jump, keep jumping while we're not out of
	// the water (and we're still trying to get out)...

	if (m_bSwimmingJump)
	{
		m_bSwimmingJump = (m_bBodyInLiquid && bJumping);
	}


	if (g_pPlayerMgr->IsSpectatorMode() || m_bSwimmingOnSurface ||
		m_bBodyOnLadder || IsFreeMovement())
	{
		g_pPlayerMgr->GetCameraRotation(rRot);
	}
	else
	{
		g_pLTClient->GetObjectRotation(m_hObject, &rRot);
	}

    LTVector vRight, vForward;
	vRight = rRot.Right();
	vForward = rRot.Forward();
	vRight.y = 0.0f;


	LTFLOAT fMoveAccelMulti = m_fMoveAccelMultiplier;
	if (m_bBodyInLiquid || m_bBodyOnLadder)
	{
		fMoveAccelMulti = fMoveAccelMulti < 1.0f ? 1.0f : fMoveAccelMulti;
	}

    LTFLOAT fMoveAccel = (m_fBaseMoveAccel * fMoveAccelMulti);

	if (g_pPlayerMgr->IsSpectatorMode())
	{
        LTVector vZero(0, 0, 0);
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);

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
	else if (m_bBodyInLiquid && !bHeadInLiquid && !m_bBodyOnLadder)
	{
		// No up acceleration...

		vForward.y = vForward.y > 0.0 ? 0.0f : vForward.y;
		vForward.Normalize();
	}

    LTFLOAT fJumpVel = g_pPlayerMgr->GetJumpVelocity(m_fJumpVel,m_fSuperJumpVel) * m_fJumpMultiplier;

	// If we're ducking make us move slower....

	if (((m_dwControlFlags & BC_CFLG_DUCK) && !m_bBodyInLiquid && !m_bBodyOnLadder) || g_pPlayerMgr->GetLeanMgr()->IsLeaning() )
	{
		fMoveAccel /= 2.0f;
	}

	// If we're in the air during a normal jump, adjust how much we can move...

	if (m_bJumped && !bFreeMovement)
	{
		fMoveAccel *= g_vtInAirAccelMultiplier.GetFloat();
	}


	// If we aren't dead we can walk around

	if (!g_pPlayerMgr->IsPlayerDead())
	{
		if (m_dwControlFlags & BC_CFLG_FORWARD)
		{
			vAccel += (vForward * fMoveAccel);
		}

		if (m_dwControlFlags & BC_CFLG_REVERSE)
		{
			vAccel -= (vForward * fMoveAccel);
		}


		// If we are in a container that supports free movement, see if we are
		// moving up or down...

		if (bInLiquid || bFreeMovement)
		{
			if (bJumping)
			{
				if (bInLiquid && !m_bBodyOnLadder)
				{
					if (bHeadInLiquid || m_bSwimmingOnSurface)
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

	g_pPhysicsLT->SetAcceleration(m_hObject, &vAccel);

    LTVector vel = GetVelocity();;

	// We can jump if we are not dead...

    LTBOOL bOkayToJump = LTFALSE;
	if (bJumping && !g_pPlayerMgr->IsPlayerDead())
	{
		if (!bHeadInLiquid && m_bBodyInLiquid)
		{
			if (m_bCanSwimJump)
			{
                m_bSwimmingJump = LTTRUE;
                m_bCanSwimJump  = LTFALSE;
			}
			// If our head is out of the liquid and we're standing on the
			// ground, let us jump out of the water...
			else if (m_bOnGround)
			{
                m_bSwimmingJump = LTTRUE;
			}
		}

		bOkayToJump = (m_bSwimmingJump || (m_bOnGround && !m_bBodyInLiquid && !m_bBodyOnLadder));

		if (bOkayToJump)
		{
			vel.y = fJumpVel;

            m_bJumped            = LTTRUE;
            m_bSwimmingOnSurface = LTFALSE;
            g_bJumpRequested     = LTFALSE;
		}
	}


	// If in spectator mode, dampen velocity...

	if (g_pPlayerMgr->IsSpectatorMode())
	{
		vel *= 0.9f;
		if (vel.Length() < 0.1f)
		{
			vel.Init();
		}
	}


	// Add any container currents to my velocity..

	vel += m_vTotalCurrent;
	g_pPhysicsLT->SetVelocity(m_hObject, &vel);

	// If we're dead, we can't move....

	if (g_pPlayerMgr->IsPlayerDead() ||
		!g_pPlayerMgr->IsPlayerMovementAllowed())
	{
        LTVector vZero(0, 0, 0);
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);
		g_pPhysicsLT->SetAcceleration(m_hObject, &vZero);
	}


	// Handle case when we just start moving...

	UpdateStartMotion(bOkayToJump);
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

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, g_pPlayerMgr->IsSpectatorMode() ? 0 : FLAG_GRAVITY, FLAG_GRAVITY);


	m_eBodyContainerCode = CC_NO_CONTAINER;
    m_bBodyInLiquid = m_bBodyOnLadder = LTFALSE;
	m_hLadderObject = LTNULL;

	UpdateContainerList();

	// Do ladder containers first...

    uint32 i;

    LTBOOL bDidLadder = LTFALSE;
    for (i=0; i < m_nContainers; i++)
	{
		// Adjust the player's velocity based on the friction of the container...

		if (!m_Containers[i].m_bHidden && m_Containers[i].m_ContainerCode == CC_LADDER)
		{
			m_eBodyContainerCode = m_Containers[i].m_ContainerCode;

			if (m_Containers[i].m_ContainerCode == CC_LADDER && !bDidLadder)
			{
				UpdateOnLadder(&m_Containers[i]);

				// Only do ladder viscosity once...Make sure we update if
				// we're on a ladder first...

				if (!bDidLadder)
				{
					UpdateContainerViscosity(&m_Containers[i]);
				}

                bDidLadder = LTTRUE;
			}

			m_vTotalCurrent += m_Containers[i].m_Current * g_pGameClientShell->GetFrameTime();
		}
	}


	// Do Non-ladder containers...

    LTBOOL bDidLiquid = LTFALSE;
	for (i=0; i < m_nContainers; i++)
	{
		// Adjust the player's velocity based on the friction of the container...

		if (!m_Containers[i].m_bHidden && m_Containers[i].m_ContainerCode != CC_LADDER)
		{
			m_eBodyContainerCode = m_Containers[i].m_ContainerCode;
			
			// Only do water viscosity once...

            LTBOOL bUpdateViscosity = LTTRUE;
			if (IsLiquid(m_Containers[i].m_ContainerCode) && bDidLiquid)
			{
				bUpdateViscosity = LTFALSE;
			}

			if (bUpdateViscosity)
			{
				UpdateContainerViscosity(&m_Containers[i]);
				UpdateContainerGravity( &m_Containers[i] );
			}

			if (IsLiquid(m_Containers[i].m_ContainerCode) && !bDidLiquid)
			{
				UpdateInLiquid(&m_Containers[i]);
                bDidLiquid = LTTRUE;
			}

			m_vTotalCurrent += m_Containers[i].m_Current * g_pGameClientShell->GetFrameTime();
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
    LTFLOAT fCoeff = 1.0f;
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

void CMoveMgr::UpdateStartMotion(LTBOOL bForce)
{
	// Play a footstep sound to indicate we are moving...

	if (!bForce)
	{
		// If we're not moving, check to see if we're ducking...

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
        uint8 nStatus = m_bJumped ? MS_JUMPED : MS_LANDED;
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
		cMsg.Writeuint8(CP_MOTION_STATUS);
        cMsg.Writeuint8(nStatus);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		// Play the jump sound for this client so the sound isnt't lagged in multiplayer...

		if (m_bJumped)
		{
			LTBOOL bPlaySound = LTFALSE;
			if (!IsHeadInLiquid())
			{
				bPlaySound = (m_bBodyInLiquid ? m_bOnGround : (m_eStandingOnSurface != ST_INVISIBLE));
			}

			if (bPlaySound)
			{
				char* pSounds[] = { "Chars\\Snd\\jump1.wav", "Chars\\Snd\\jump2.wav" };
				g_pClientSoundMgr->PlaySoundLocal(pSounds[GetRandom(0,1)], SOUNDPRIORITY_PLAYER_HIGH);
			}
		}
	}


	if (m_pCharFX)
	{
		m_pCharFX->DoFootStep();
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
    LTLink *pCur, *pNext;
	Pusher *pPusher;
    LTVector myPos, pushVec, vel;
	float dist, velocity;
	CollisionInfo info;
	ClientIntersectQuery iQuery;
	ClientIntersectInfo iInfo;

	if (!m_hObject || !g_pLTClient || !g_pPhysicsLT) return;

	g_pLTClient->GetObjectPos(m_hObject, &myPos);
	for(pCur=m_Pushers.m_pNext; pCur != &m_Pushers; pCur=pNext)
	{
		pNext = pCur->m_pNext;

		pPusher = (Pusher*)pCur->m_pData;

		pPusher->m_Delay -= g_pGameClientShell->GetFrameTime();
		if(pPusher->m_Delay <= 0.0f)
		{
			pPusher->m_TimeLeft -= g_pGameClientShell->GetFrameTime();
			if(pPusher->m_TimeLeft <= 0.0f)
			{
				// Expired..
				dl_Remove(&pPusher->m_Link);
				Pusher::GetBank()->Delete(pPusher);
			}
			else
			{
				// Are we within range?
				dist = VEC_DIST(pPusher->m_Pos, myPos);
				if(dist < pPusher->m_Radius)
				{
					memset(&iQuery, 0, sizeof(iQuery));
					iQuery.m_From = pPusher->m_Pos;
					iQuery.m_To = myPos;
					if(!g_pLTClient->IntersectSegment(&iQuery, &iInfo))
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
						pushVec *= velocity / pushVec.Length();

						vel = GetVelocity();
						vel += pushVec;

						// [KLS 5/20/02] Cap Y velocity.  Don't allow us to
						// fly too high...
						if (vel.y > g_vtMaxPushYVelocity.GetFloat())
						{
							g_pLTClient->CPrint("Old Pusher Y Vel = %.2f", vel.y);
							vel.y = g_vtMaxPushYVelocity.GetFloat();
						}

						g_pPhysicsLT->SetVelocity(m_hObject, &vel);
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdatePlayerAnimation
//
//	PURPOSE:	Update our animation
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdatePlayerAnimation()
{
	HOBJECT hClientObj;
    uint32 modelAnim(0), curModelAnim(0), curFlags(0);
	HRESULT result(LT_OK);

	if (!(hClientObj = g_pLTClient->GetClientObject())) return;

	// Make sure our solid object is on the same animation.
	
	curModelAnim = g_pLTClient->GetModelAnimation(m_hObject);

	// Make sure we are playing the animation corresponding to the dims...
	if (m_pCharFX)
	{
		HRESULT result = g_pModelLT->GetCurAnim(m_pCharFX->GetServerObj(), m_pCharFX->m_cs.nDimsTracker, modelAnim);
		//ASSERT(result == LT_OK);
	}

	//  See if we should use the main anim...
	if (modelAnim == 0)
	{
		modelAnim = g_pLTClient->GetModelAnimation(hClientObj);
	}

	if (modelAnim != curModelAnim || m_bFirstAniUpdate)
	{
		// Force this once to make sure our dims get set correctly...

        m_bFirstAniUpdate = LTFALSE;

		// Kind of wierd what we do here.. the engine sets the dims automatically when
		// we change animations but it doesn't do collision detection (and we don't want
		// it to) so we may end up clipping into the world so we set it to a small cube
		// and resize the dims with collision detection.
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, curFlags);
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_GOTHRUWORLD, FLAG_GOTHRUWORLD | FLAG_SOLID);

		g_pLTClient->SetModelAnimation(m_hObject, modelAnim);

		// Get our wanted dims.
	    LTVector oldDims;
		oldDims = m_vWantedDims;
		m_vWantedDims.Init(1.0f, 1.0f, 1.0f);
		g_pCommonLT->GetModelAnimUserDims(m_hObject, &m_vWantedDims, modelAnim);

		// Figure out a position offset.
		LTVector offset;
		offset.Init();
		if (m_vWantedDims.y < oldDims.y)
		{
			offset.y = -(oldDims.y - m_vWantedDims.y);
			offset.y -= .01f; // Fudge factor
		}

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, curFlags, FLAGMASK_ALL);

		// This makes you small before setting the dims so you don't clip thru stuff.

		if (!AreDimsCorrect())
		{
			ResetDims(&offset);
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

LTBOOL CMoveMgr::AreDimsCorrect()
{
	if(!m_hObject || !g_pPhysicsLT)
        return LTTRUE;

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
		g_pLTClient->SetObjectPos(m_hObject, &vOldPos);
	}
	// Move them if they want
	else if (pOffset && (*pOffset != LTVector(0.0f, 0.0f, 0.0f)))
	{
		vOldPos += *pOffset;
		g_pPhysicsLT->MoveObject(m_hObject, &vOldPos, 0);
		g_pLTClient->SetObjectPos(m_hObject, &vOldPos);
		// Update the "last on ground" position so we don't think we're falling based on the position change
		CollisionInfo Info;
		g_pPhysicsLT->GetStandingOn(m_hObject, &Info);
		if (Info.m_hObject)
			m_fLastOnGroundY = vOldPos.y;
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
	LTVector vWantedDims = m_vStandDims;
	bool bResult = (g_pPhysicsLT->SetObjectDims(m_hObject, &vWantedDims, SETDIMS_PUSHOBJECTS) == LT_OK);

	// Reset our dims to what they were before.
	g_pPhysicsLT->SetObjectDims(m_hObject, &vCurDims, 0);
	g_pLTClient->SetObjectPos(m_hObject, &vOldPos);

	return bResult;
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

	ILTClientPhysics* pPhysics = (ILTClientPhysics*)g_pPhysicsLT;

	// We may want gravity to be different for us...

	if (g_vtPlayerGravity.GetFloat() != DEFAULT_WORLD_GRAVITY)
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
	info.m_dt = g_pGameClientShell->GetFrameTime();
	pPhysics->UpdateMovement(&info);

	if (info.m_Offset.MagSqr() > 0.01f)
	{
		g_pLTClient->GetObjectPos(m_hObject, &curPos);
		newPos = curPos + info.m_Offset;
	
		if( m_pVehicleMgr->IsVehiclePhysics( ) )
		{
			m_pVehicleMgr->MoveVehicleObject( newPos );
		}
		else
		{
			pPhysics->MoveObject(m_hObject, &newPos, 0);
		}

		g_pLTClient->GetObjectPos(m_hObject, &newPos);

		// Deal with the issue that comes up when you're standing on something
		// you should be sliding on, but you're blocked, so gravity keeps
		// getting applied to the velocity.  Only do this if the UpdateMovement
		// affected our y direction.
		if ( fabsf( info.m_Offset.y ) > 0.01f && fabsf(newPos.y - curPos.y) < 0.01f)
		{
			LTVector vObjVel;
			pPhysics->GetVelocity(m_hObject, &vObjVel);
			vObjVel.y = 0.0f;
			pPhysics->SetVelocity(m_hObject, &vObjVel);
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

LTFLOAT CMoveMgr::GetVelMagnitude()
{
	if (!g_pPhysicsLT || !g_pGameClientShell || !m_hObject) return 0.0f;

    return GetVelocity().Length();
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
	HOBJECT hObj;

	if(hObj = g_pLTClient->GetClientObject())
	{
		g_pCommonLT->SetObjectFlags(hObj, OFT_Flags, FLAG_CLIENTNONSOLID, FLAG_CLIENTNONSOLID);
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

	if ( !(m_dwControlFlags & dwTestFlags) && !m_bJumped && m_bOnGround &&
		 m_vTotalCurrent == vZero && m_fTotalViscosity == 0.0f)
	{
        LTVector vCurVel = GetVelocity();

        LTFLOAT fYVal = vCurVel.y;
		vCurVel.y = 0.0f;

        LTVector vVel(0, 0, 0);

		if (vCurVel.Length() > 5.0f)
		{
            LTVector vDir = vCurVel;
			vDir.Normalize();

            LTFLOAT fSlideToStopTime = g_vtSlideToStopTime.GetFloat();
			fSlideToStopTime = fSlideToStopTime <= 0.0f ? 0.1f : fSlideToStopTime;

            LTFLOAT fAdjust = g_pGameClientShell->GetFrameTime() * (m_fRunVel/fSlideToStopTime);

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

		g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
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
	if (g_pGameClientShell->IsGamePaused()) return;
	
	HOBJECT hObj = g_pLTClient->GetClientObject();
	if (!m_hObject || !hObj) return;

	m_eLastContainerCode = m_eCurContainerCode;
	m_eCurContainerCode  = g_pPlayerMgr->GetCurContainerCode();

	// If were're underwater (or on a ladder), we reset our fall damage data...

	if (IsHeadInLiquid() || m_bBodyOnLadder)
	{
		m_fLastOnGroundY = MIN_ONGROUND_Y;
	}

	// We don't want to hit the real client object....

	SetClientObjNonsolid();

	UpdatePlayerAnimation();

	UpdateControlFlags();

    UpdateMotion();

	UpdateFriction();

	UpdatePushers();

	UpdateSound();

	// Make sure we have desired dims....

	if (!AreDimsCorrect())
	{
		ResetDims();
	}

	MoveLocalSolidObject();
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
			m_Containers[i].m_bHidden = pMsg->Readuint8();
			m_Containers[i].m_hObject = pMsg->ReadObject();
			m_Containers[i].m_ePPhysicsModel = (PlayerPhysicsModel)pMsg->Readuint8();
		}

        LTFLOAT fCoeff = pMsg->Readfloat();
		g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fCoeff);
	}

	// Change our model file?

	if (changeFlags & PSTATE_MODELFILENAMES)
	{
		if (m_hObject)
		{
			g_pLTClient->RemoveObject(m_hObject);
            m_hObject = LTNULL;
		}

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		pMsg->ReadString(theStruct.m_Filename, sizeof(theStruct.m_Filename));
		uint8 cSkins = pMsg->Readuint8();
		for(uint8 iSkin=0; iSkin < cSkins; ++iSkin)
		{
			pMsg->ReadString(theStruct.m_SkinNames[iSkin], sizeof(theStruct.m_SkinNames[iSkin]));
		}

		// Save off if we are using one of the Cate models or not...

		m_bUsingPlayerModel = LTFALSE;
		if (theStruct.m_Filename[0])
		{
			char buffer[128];
			SAFE_STRCPY(buffer, theStruct.m_Filename);
			strupr(buffer);
			if (strstr(buffer, "PLAYER") || strstr(buffer, "HERO"))
			{
				m_bUsingPlayerModel = LTTRUE;
			}
		}

        theStruct.m_ObjectType = OT_MODEL;
		theStruct.m_Flags = FLAG_SOLID | FLAG_GRAVITY | FLAG_STAIRSTEP;
		theStruct.m_Flags2 = FLAG2_PLAYERCOLLIDE | FLAG2_SPECIALNONSOLID;

		HOBJECT hClientObj = g_pLTClient->GetClientObject();
		if( hClientObj )
		{
			g_pLTClient->GetObjectPos(hClientObj, &theStruct.m_Pos);
			g_pCommonLT->SetObjectFilenames(hClientObj, &theStruct);

			// Since the object was created in the correct position we don't need to force it later...

			m_bForceToServerPos = LTFALSE;
		}
		else
			m_bForceToServerPos = LTTRUE;

		m_hObject = g_pLTClient->CreateObject(&theStruct);

		if (m_hObject)
		{
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Client, CF_DONTSETDIMS, CF_DONTSETDIMS);

			// Note : This queries the anim for the dims of the player.  That information is all available
			// on the server, but it's not on the client.  This is a lot easier than trying to transmit
			// that information or something.
			HMODELANIM hAnim = g_pLTClient->GetAnimIndex(m_hObject, PLAYERANIM_CROUCH);
			if (hAnim == INVALID_MODEL_ANIM)
			{
				ASSERT(!"Unable to find player crouch animation");
				m_vCrouchDims.Init(24.0f, 31.5f, 24.0f);
			}
			else
			{
				g_pCommonLT->GetModelAnimUserDims(m_hObject, &m_vCrouchDims, hAnim);
			}

			hAnim = g_pLTClient->GetAnimIndex(m_hObject, PLAYERANIM_STAND);
			if (hAnim == INVALID_MODEL_ANIM)
			{
				ASSERT(!"Unable to find player stand animation");
				m_vStandDims.Init(24.0f, 53.0f, 24.0f);
			}
			else
			{
				g_pCommonLT->GetModelAnimUserDims(m_hObject, &m_vStandDims, hAnim);
			}

		}
	}

	// Gravity change...

	if (changeFlags & PSTATE_GRAVITY)
	{
        LTVector vGravity;
		vGravity = pMsg->ReadLTVector();
		g_pPhysicsLT->SetGlobalForce(vGravity);

		m_fGravity = vGravity.y;
		g_vtPlayerGravity.SetFloat(m_fGravity);
	}

	// Speed change...

	if (changeFlags & PSTATE_SPEEDS)
	{
		m_fWalkVel = pMsg->Readfloat();
		m_fRunVel = pMsg->Readfloat();
		m_fSwimVel = pMsg->Readfloat();
		m_fJumpVel = pMsg->Readfloat();
		m_fSuperJumpVel = pMsg->Readfloat();

		m_fMoveAccelMultiplier = m_fMoveMultiplier = pMsg->Readfloat();

		m_fBaseMoveAccel = pMsg->Readfloat();
		m_fJumpMultiplier = pMsg->Readfloat();
		m_fLadderVel = pMsg->Readfloat();

        LTFLOAT fCoeff = pMsg->Readfloat();
		g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fCoeff);
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
			m_pVehicleMgr->SetPlayerLureId( 0 );
		}

		// Turn off duck lock...
		if (eModel != PPM_NORMAL)
			SetDuckLock(LTFALSE);

		m_pVehicleMgr->SetPhysicsModel(eModel);
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

	// If it's a solid world model moving, do a regular MoveObject on it so it
	// can carry/push the player object around.

	if (!bTeleport && hObj != hClientObj && hObj != m_hObject)
	{
        uint32 type = GetObjectType(hObj);
		if (type == OT_WORLDMODEL)
		{
			uint32 dwFlags;
			g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
			if( dwFlags & FLAG_SOLID )
			{
				((ILTClientPhysics*)g_pPhysicsLT)->MovePushObjects(hObj, *pPos, &m_hObject, 1);
			}
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

	HOBJECT hClientObj = g_pLTClient->GetClientObject();

	// If it's a solid world model moving, do a regular MoveObject on it so it
	// can carry/push the player object around.

	if (!bTeleport && hObj != hClientObj && hObj != m_hObject)
	{
		uint32 type = GetObjectType(hObj);
		if (type == OT_WORLDMODEL)
		{
			uint32 dwFlags;
			g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
			if( dwFlags & FLAG_SOLID )
			{
				((ILTClientPhysics*)g_pPhysicsLT)->RotatePushObjects(hObj, *pNewRot, &m_hObject, 1);
			}
		}
	}

	return LT_OK;
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
	
	if( g_pPlayerMgr->IsPlayerDead() )
		return LT_ERROR;

	Pusher *pPusher = Pusher::GetBank()->New();

	if(!pPusher)
		return LT_ERROR;

	pPusher->m_Pos = pos;
	pPusher->m_Radius = radius;
	pPusher->m_Delay = startDelay;
	pPusher->m_TimeLeft = duration;
	pPusher->m_Strength = strength;
	dl_Insert(&m_Pushers, &pPusher->m_Link);

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::SetSpectatorMode
//
//	PURPOSE:	Set spectator mode
//
// ----------------------------------------------------------------------- //

void CMoveMgr::SetSpectatorMode(LTBOOL bSet)
{
	if(!m_hObject)
		return;

	if (bSet)  // Spectator mode...
	{
		// Move up a little bit.
	    LTVector vPos;
		g_pLTClient->GetObjectPos(m_hObject, &vPos);
		vPos.y += 50;

		g_pPhysicsLT->MoveObject(m_hObject, &vPos, MOVEOBJECT_TELEPORT);

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_GOTHRUWORLD, FLAG_GOTHRUWORLD | FLAG_SOLID);

		LTVector vZero(0, 0, 0);
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);
		g_pPhysicsLT->SetAcceleration(m_hObject, &vZero);
	}
	else  // No longer in spectator mode...
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID | FLAG_GOTHRUWORLD);
	}
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

	if( !m_hObject  )
		return;

	SetClientObjNonsolid();

	// Move there.  We make our object a point first and then resize the dims so
	// we don't teleport clipping into the world.
	

	LTVector vTempDims(0.5f, 0.5f, 0.5f);

	g_pPhysicsLT->SetObjectDims(m_hObject, &vTempDims, 0);
	g_pPhysicsLT->MoveObject(m_hObject, &vPos, MOVEOBJECT_TELEPORT);

	//if the target dims don't fit, force the largest dims that do fit, the dims will be corrected
	//	auto-magically on a later update.
	if (LT_OK != g_pPhysicsLT->SetObjectDims(m_hObject, &vCurDims, SETDIMS_PUSHOBJECTS))
		g_pPhysicsLT->SetObjectDims(m_hObject, &vCurDims, 0);

	// Clear our velocity and acceleration.

	LTVector vVel(0, 0, 0);
	g_pPhysicsLT->SetAcceleration(m_hObject, &vVel);

	if (m_bLoading)
	{
		vVel = m_vSavedVel;
		m_bLoading = LTFALSE;
	}
	else
	{
		m_fLastOnGroundY = MIN_ONGROUND_Y;
	}

	g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
	
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
		if (g_pGameClientShell->IsGamePaused())
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
    pMsg->Writeuint8((uint8)m_bOnGround);
    pMsg->Writeuint8((uint8)m_eStandingOnSurface);
    pMsg->WriteType(m_hStandingOnPoly);
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

LTFLOAT CMoveMgr::GetMaxVelMag() const
{
    LTFLOAT fMaxVel = 0.0f;

	LTFLOAT fMoveMultiplier = m_fMoveMultiplier;

	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		fMaxVel = m_pVehicleMgr->GetMaxVelMag();
	}
	else
	{
		LTBOOL bRunning = (m_dwControlFlags & BC_CFLG_RUN);

		if (m_bBodyInLiquid)
		{
			fMaxVel = bRunning ? m_fSwimVel : m_fSwimVel/2.0f;
			fMoveMultiplier = fMoveMultiplier < 1.0f ? 1.0f : fMoveMultiplier;
		}
		else if (m_bBodyOnLadder)
		{
			fMaxVel =  bRunning ? m_fLadderVel : m_fLadderVel/2.0f;
			fMoveMultiplier = fMoveMultiplier < 1.0f ? 1.0f : fMoveMultiplier;
		}
		else
		{
			if (bRunning && !(m_dwControlFlags & BC_CFLG_DUCK) && 
				!g_pPlayerMgr->IsZoomed() && !g_pPlayerMgr->GetLeanMgr()->IsLeaning())
			{
				fMaxVel = m_fRunVel;
			}
			else
			{
				fMaxVel = m_fWalkVel;
			}
		}

		if( g_pPlayerMgr->IsCarryingHeavyObject() && fMaxVel > m_fWalkVel)
		{

			float fDiff = (fMaxVel - m_fWalkVel);

			fDiff *= g_pPlayerStats->GetSkillModifier(SKL_CARRY,CarryModifiers::eCarryBodyMovement);

			fMaxVel -= fDiff;

		}



		//Calculate any movement penalties
		LTFLOAT fPenalty = CalculateMovementPenalty();

		fMaxVel *= ((1.0f - fPenalty) * fMoveMultiplier);

	}

	return fMaxVel;
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
		g_pPhysicsLT->SetVelocity(m_hObject, const_cast<LTVector*>(&vVel));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetMovementPercent
//
//	PURPOSE:	Get current percentage of maximum velocity player is moving
//
// ----------------------------------------------------------------------- //

LTFLOAT CMoveMgr::GetMovementPercent() const
{
    LTFLOAT fPerturb = 0.0f;
    LTVector vVel = GetVelocity();
    LTFLOAT  fMaxVel = GetMaxVelMag();

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

	m_DamageTimer.Start(pMsg->Readfloat());
	m_vWantedDims = pMsg->ReadLTVector();

	m_bLoading = LTTRUE;

	m_pVehicleMgr->Load(pMsg, eLoadDataState);

	m_bJumped = (pMsg->Readbool() ? LTTRUE : LTFALSE);
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

	pMsg->Writefloat(m_DamageTimer.GetCountdownTime());
	pMsg->WriteLTVector( m_vWantedDims );

	m_pVehicleMgr->Save(pMsg, eSaveDataState);

	pMsg->Writebool(m_bJumped ? true : false);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::IsPlayerModel
//
//	PURPOSE:	Are we using one of the player (cate) models...
//
// ----------------------------------------------------------------------- //

LTBOOL CMoveMgr::IsPlayerModel()
{
	return m_bUsingPlayerModel;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::AddDamagePenalty
//
//	PURPOSE:	Add a movement penalty
//
// ----------------------------------------------------------------------- //
void CMoveMgr::AddDamagePenalty(LTFLOAT fDuration)
{
	fDuration *= g_pPlayerStats->GetSkillModifier(SKL_STAMINA,StaminaModifiers::eMoveDamage);
	if (fDuration > m_DamageTimer.GetCountdownTime())
		m_DamageTimer.Start(fDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::CalculateMovementPenalty
//
//	PURPOSE:	How much is the player's movement being penalized
//
// ----------------------------------------------------------------------- //

LTFLOAT CMoveMgr::CalculateMovementPenalty() const
{
	LTFLOAT fPenalty = 0.0f;

	// [KLS 8/22/02] Removed heavy weapon stuff from weapon mgr...
/*
	WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(g_pPlayerStats->GetCurrentWeapon());
	if( pWeaponData && pWeaponData->bHeavyWeapon)
	{
		LTFLOAT fHeavyWeaponMovePenalty = GetConsoleFloat("HeavyWeaponMovePenalty",0.333f);

		fHeavyWeaponMovePenalty *= g_pPlayerStats->GetSkillModifier(SKL_CARRY,CarryModifiers::eHeavyWeaponMovement);

		if (fHeavyWeaponMovePenalty > fPenalty)
			fPenalty = fHeavyWeaponMovePenalty;
	}
*/

	if (m_DamageTimer.GetCountdownTime() )
	{
		LTFLOAT fDamageMovePenalty = GetConsoleFloat("DamageMovePenalty",0.5f);

		fDamageMovePenalty *= g_pPlayerStats->GetSkillModifier(SKL_STAMINA,StaminaModifiers::eMoveDamage);

		if (fDamageMovePenalty > fPenalty)
			fPenalty = fDamageMovePenalty;

	}
	return fPenalty;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::GetServerObject
//
//	PURPOSE:	Get the server object associated with the player
//
// ----------------------------------------------------------------------- //

HOBJECT	CMoveMgr::GetServerObject()
{
	if (!GetCharacterFX())
		return LTNULL;
	return GetCharacterFX()->GetServerObj();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CMoveMgr::IsMovingQuietly
//
//  PURPOSE:	Are we being very quiet?
//
// ----------------------------------------------------------------------- //

LTBOOL CMoveMgr::IsMovingQuietly() const
{ 
	LTBOOL bRet = LTFALSE;

	if ((m_dwControlFlags & BC_CFLG_DUCK) || !(m_dwControlFlags & BC_CFLG_RUN) ||
		g_pPlayerMgr->GetLeanMgr()->IsLeaning())
	{
		if (!IsFreeMovement() && !IsBodyOnLadder() && IsOnGround())
		{
			bRet = LTTRUE;
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
		return LTNULL;

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