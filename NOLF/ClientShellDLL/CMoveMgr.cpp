// ----------------------------------------------------------------------- //
//
// MODULE  : CMoveMgr.cpp
//
// PURPOSE : Client side player movement mgr - Implementation
//
// CREATED : 10/2/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
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

#define SPECTATOR_ACCELERATION			100000.0f
#define MIN_ONGROUND_Y					-10000000.0f
#define DEFAULT_WORLD_GRAVITY			-2000.0f

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

VarTrack	g_vtCatPhysics;

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

	// Always have this...
	m_pVehicleMgr = debug_new(CVehicleMgr);

	InitWorldData();
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
    g_vtCamLandMoveDist.Init(g_pLTClient, "CamLandMoveDist", LTNULL, -30.0f);
    g_vtCamLandDownTime.Init(g_pLTClient, "CamLandDownTime", LTNULL, 0.15f);
    g_vtCamLandUpTime.Init(g_pLTClient, "CamLandUpTime", LTNULL, 0.4f);

    g_vtCamLandRollVal.Init(g_pLTClient, "CamLandRollVal", LTNULL, 5.0f);
    g_vtCamLandRollTime1.Init(g_pLTClient, "CamLandRollTime1", LTNULL, 0.05f);
    g_vtCamLandRollTime2.Init(g_pLTClient, "CamLandRollTime2", LTNULL, 0.15f);

    g_vtCamLandPitchVal.Init(g_pLTClient, "CamLandPitchVal", LTNULL, 5.0f);
    g_vtCamLandPitchTime1.Init(g_pLTClient, "CamLandPitchTime1", LTNULL, 0.05f);
    g_vtCamLandPitchTime2.Init(g_pLTClient, "CamLandPitchTime2", LTNULL, 0.15f);

    g_vtPlayerGravity.Init(g_pLTClient, "PlayerGravity", LTNULL, DEFAULT_WORLD_GRAVITY);

    g_vtSlideToStopTime.Init(g_pLTClient, "SlideToStopTime", LTNULL, 0.1f);

    g_vtCatPhysics.Init(g_pLTClient, "CatPhysics", LTNULL, 0.0f);

 	// Init some defaults.  These should NEVER get used because we don't
	// have our object until the server sends the physics update.

	m_fSwimVel			= 0.0f;
	m_fWalkVel			= 0.0f;
	m_fRunVel			= 400.0f;
	m_fJumpVel			= 550.0f;
	m_fZipCordVel		= 100.0f;
	m_fMoveMultiplier	= 1.0f;

	m_pVehicleMgr->Init();

	// Init world specific data members...

	InitWorldData();

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
    m_bZipCordOn            = LTFALSE;
	m_fZipCordVel			= 0.0f;
	m_vZipCordPos.Init();
	m_vZipHookDims.Init();

	m_eBodyContainerCode	= CC_NO_CONTAINER;
    m_bBodyInLiquid         = LTFALSE;
    m_bBodyOnLadder         = LTFALSE;
    m_bOnGround             = LTTRUE;
    m_bOnLift               = LTFALSE;
    m_bFalling              = LTFALSE;
    m_bAllowMovement        = LTTRUE;
	m_eStandingOnSurface	= ST_UNKNOWN;
	m_nMouseStrafeFlags		= 0;
    m_bSwimmingOnSurface    = LTFALSE;
    m_bSwimmingJump         = LTFALSE;
    m_bCanSwimJump          = LTFALSE;
	m_hStandingOnPoly		= INVALID_HPOLY;
	m_bUsingPlayerModel		= LTFALSE;

	m_fBaseMoveAccel		= 0.0f;
	m_fMoveAccelMultiplier	= 1.0f;
	m_fJumpMultiplier		= 1.0f;

    m_eCurContainerCode		= CC_NO_CONTAINER;
	m_eLastContainerCode	= CC_NO_CONTAINER;
	m_nContainers			= 0;

	m_fLastOnGroundY		= MIN_ONGROUND_Y;
    m_bJumped               = LTFALSE;

    m_bFirstAniUpdate       = LTTRUE;

    m_pCharFX               = LTNULL;

	m_hZipcordSnd			= LTNULL;
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

	TurnOffZipCord();
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
	m_nMouseStrafeFlags = 0;

	if (pAxisOffsets[0] < 0.0f)
	{
		m_nMouseStrafeFlags |= SF_LEFT;
	}
	else if (pAxisOffsets[0] > 0.0f)
	{
		m_nMouseStrafeFlags |= SF_RIGHT;
	}

	if (pAxisOffsets[1] < 0.0f)
	{
		m_nMouseStrafeFlags |= SF_FORWARD;
	}
	else if (pAxisOffsets[1] > 0.0f)
	{
		m_nMouseStrafeFlags |= SF_BACKWARD;
	}

	pAxisOffsets[0] = 0.0f;
	pAxisOffsets[1] = 0.0f;
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

	if (!m_hObject || !g_pInterfaceMgr->AllowCameraMovement()) return;


	if (m_pVehicleMgr->IsVehiclePhysics())
	{
		m_pVehicleMgr->UpdateControlFlags();
		m_dwControlFlags = m_pVehicleMgr->GetControlFlags();
	}
	else
	{
		UpdateNormalControlFlags();
	}
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
	if (g_pGameClientShell->GetPlayerState() == PS_GHOST) return;


	if (g_pLTClient->IsCommandOn(COMMAND_ID_RUN) ^ g_pInterfaceMgr->GetSettings()->RunLock())
	{
		m_dwControlFlags |= BC_CFLG_RUN;
	}


	// Use the player's user flags instead of checking this flag directly
	// (this insures that the camera will be moved accurately...)

    // Allow normal duck if we have free movement...

    LTBOOL bFreeMovement = (IsFreeMovement() || m_bBodyOnLadder ||
		g_pGameClientShell->IsSpectatorMode());


	// Update 1.002 [KLS].  If multiplayer remove the lagged feel.  The 
	// animation will still be lagged, but the camera should move as
	// expected...

	if (IsMultiplayerGame())
	{
		if (g_pLTClient->IsCommandOn(COMMAND_ID_DUCK))
		{
			m_dwControlFlags |= BC_CFLG_DUCK;
		}
	}
	else
	{
		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (!bFreeMovement && hPlayerObj)
		{
			uint32 dwUsrFlags;
			g_pLTClient->GetObjectUserFlags(hPlayerObj, &dwUsrFlags);
			if (dwUsrFlags & USRFLG_PLAYER_DUCK)
			{
				m_dwControlFlags |= BC_CFLG_DUCK;
			}
		}
		else  // This is needed for some menu specific cheats (player may not be around)
		{
			if (g_pLTClient->IsCommandOn(COMMAND_ID_DUCK))
			{
				m_dwControlFlags |= BC_CFLG_DUCK;
			}
		}
	}

	// Only process jump if we aren't ducking...

	if (!(m_dwControlFlags & BC_CFLG_DUCK))
	{
		if (g_pLTClient->IsCommandOn(COMMAND_ID_JUMP))
		{
			m_dwControlFlags |= BC_CFLG_JUMP;
		}
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

	// Don't do viscosity dampening, if we're jumping out of water...

	if (!m_bBodyOnLadder && m_bJumped && !IsHeadInLiquid()) return;


	// Update the total viscosity for the frame...

	m_fTotalViscosity += pInfo->m_fViscosity;


	// Do REAL viscosity dampening (i.e., actually change our velocity)...

    LTVector vVel = GetVelocity();;
    LTVector vCurVel = vVel;

	if (pInfo->m_fViscosity > 0.0f && VEC_MAG(vCurVel) > 1.0f)
	{
        LTVector vDir = vCurVel;
		vDir.Norm();

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

    LTVector vVel = GetVelocity();;

    LTVector curAccel;
	g_pPhysicsLT->GetAcceleration(m_hObject, &curAccel);

    uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hObject);

	// Handle floating around on the surface...

	if (m_bSwimmingOnSurface)
	{
        LTBOOL bMoving = ((curAccel.Mag() > 0.01f) || (vVel.Mag() > 0.01f));

		// Disable gravity.
		g_pLTClient->SetObjectFlags(m_hObject, dwFlags & ~FLAG_GRAVITY);

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
		g_pLTClient->SetObjectFlags(m_hObject, dwFlags & ~FLAG_GRAVITY);

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

    uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hObject);
	g_pLTClient->SetObjectFlags(m_hObject, dwFlags & ~FLAG_GRAVITY);
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
            HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
            g_pLTClient->WriteToMessageByte(hMessage, CP_MOTION_STATUS);
            g_pLTClient->WriteToMessageByte(hMessage, MS_LANDED);
            g_pLTClient->EndMessage(hMessage);
		}

        m_bFalling = LTFALSE;

		m_eStandingOnSurface = ST_UNKNOWN;
		if (Info.m_hPoly && Info.m_hPoly != INVALID_HPOLY)
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
        g_pLTClient->GetObjectUserFlags(Info.m_hObject, &dwUserFlags);

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

		// Make sure we fall down step slopes...

		if (Info.m_Plane.m_Normal.y < 0.707)
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
		if (g_pGameClientShell->IsSpectatorMode())
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

    LTBOOL bFreeMovement = (IsFreeMovement() || m_bBodyOnLadder || g_pGameClientShell->IsSpectatorMode());

	if (!bFreeMovement && !m_bJumped && !CanDoFootstep() && !m_bFalling)
	{
        m_bFalling = LTTRUE;

		// Tell the server we're falling...

        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
        g_pLTClient->WriteToMessageByte(hMessage, CP_MOTION_STATUS);
        g_pLTClient->WriteToMessageByte(hMessage, MS_FALLING);
        g_pLTClient->EndMessage(hMessage);
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

			if (m_eCurContainerCode != CC_SAFTEY_NET)
			{
				LTVector vDir(0, 1, 0); // Ground caused damage...

				HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
				g_pLTClient->WriteToMessageByte(hMessage, CP_DAMAGE);
				g_pLTClient->WriteToMessageByte(hMessage, DT_CRUSH);
				g_pLTClient->WriteToMessageFloat(hMessage, fDamage);
				g_pLTClient->WriteToMessageVector(hMessage, &vDir);
				g_pLTClient->WriteToMessageByte(hMessage, 0);
				g_pLTClient->WriteToMessageObject(hMessage, g_pLTClient->GetClientObject());
				g_pLTClient->EndMessage(hMessage);
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
			if (!IsLiquid(m_eCurContainerCode))
			{
				bPlaySound = (m_bBodyInLiquid ? m_bOnGround : (m_eStandingOnSurface != ST_INVISIBLE));
			}

			if (bPlaySound)
			{
				char* pSounds[] = { "Chars\\Snd\\player\\landing1.wav", "Chars\\Snd\\player\\landing2.wav" };
				g_pClientSoundMgr->PlaySoundLocal(pSounds[GetRandom(0,1)], SOUNDPRIORITY_PLAYER_HIGH);
			}
		}

		g_pGameClientShell->GetCameraOffsetMgr()->AddDelta(delta);

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
	if (!m_bAllowMovement)
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
	g_pGameClientShell->GetPlayerRotation(&rRot);
	g_pLTClient->SetObjectRotation(m_hObject, &rRot);


    LTVector myVel = GetVelocity();

    LTVector moveVel = myVel;
	moveVel.y = 0;

	float fMaxVel;
    LTBOOL bHeadInLiquid = IsHeadInLiquid();
    LTBOOL bInLiquid     = (bHeadInLiquid || m_bBodyInLiquid);
    LTBOOL bFreeMovement = (IsFreeMovement() || m_bBodyOnLadder || g_pGameClientShell->IsSpectatorMode());

	fMaxVel = GetMaxVelMag();


	// Determine if we are currently trying to jump...

    LTBOOL bJumping = LTFALSE;
	if (bFreeMovement /*bHeadInLiquid*/)
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

		if (moveVel.Mag() > fMaxVel)
		{
			moveVel.Norm();
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
	else if (m_bOnGround && !g_pGameClientShell->IsSpectatorMode() && !bJumping)
	{
		float fCurLen = (float)sqrt(myVel.x*myVel.x + myVel.z*myVel.z);
		if (fCurLen > fMaxVel)
		{
			myVel *= (fMaxVel/fCurLen);

			g_pPhysicsLT->SetVelocity(m_hObject, &myVel);
		}
	}
	else if (moveVel.Mag() > fMaxVel)
	{
		// Don't cap velocity in the y direction...

		moveVel.Norm();
		moveVel *= fMaxVel;

		myVel.x = moveVel.x;
		myVel.z = moveVel.z;

		g_pPhysicsLT->SetVelocity(m_hObject, &myVel);
	}



	// See if we just broke the surface of water...

	if ((IsLiquid(m_eLastContainerCode) && !bHeadInLiquid) && !m_bOnGround)
	{
        m_bSwimmingOnSurface = LTTRUE;
	}
	else if (bHeadInLiquid)  // See if we went back under...
	{
        m_bSwimmingOnSurface = LTFALSE;
        m_bCanSwimJump       = LTTRUE;
	}


	// If we're doing a swimming jump, keep jumping while we're not out of
	// the water (and we're still trying to get out)...

	if (m_bSwimmingJump)
	{
		m_bSwimmingJump = (m_bBodyInLiquid && bJumping);
	}


	if (g_pGameClientShell->IsSpectatorMode() || m_bSwimmingOnSurface ||
		m_bBodyOnLadder || IsFreeMovement())
	{
		g_pGameClientShell->GetCameraRotation(&rRot);
	}
	else
	{
		g_pLTClient->GetObjectRotation(m_hObject, &rRot);
	}

    LTVector vUp, vRight, vForward;
	g_pLTClient->GetRotationVectors(&rRot, &vUp, &vRight, &vForward);
	vRight.y = 0.0f;


	LTFLOAT fMoveAccelMulti = m_fMoveAccelMultiplier;
	if (m_bBodyInLiquid || m_bBodyOnLadder)
	{
		fMoveAccelMulti = fMoveAccelMulti < 1.0f ? 1.0f : fMoveAccelMulti;
	}

    LTFLOAT fMoveAccel = (m_fBaseMoveAccel * fMoveAccelMulti);

	if (g_pGameClientShell->IsSpectatorMode())
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
		vForward.Norm();
	}
	else if (m_bBodyInLiquid && !bHeadInLiquid && !m_bBodyOnLadder)
	{
		// No up acceleration...

		vForward.y = vForward.y > 0.0 ? 0.0f : vForward.y;
		vForward.Norm();
	}


    LTFLOAT fJumpVel = (m_fJumpVel * m_fJumpMultiplier);

	// If we're ducking make us move slower....

	if ((m_dwControlFlags & BC_CFLG_DUCK) && !m_bBodyInLiquid && !m_bBodyOnLadder)
	{
		fMoveAccel /= 2.0f;
	}


	// If we aren't dead we can walk around

	if (!g_pGameClientShell->IsPlayerDead())
	{
		if ((m_dwControlFlags & BC_CFLG_FORWARD) || (m_nMouseStrafeFlags & SF_FORWARD))
		{
			vAccel += (vForward * fMoveAccel);
		}

		if ((m_dwControlFlags & BC_CFLG_REVERSE) || (m_nMouseStrafeFlags & SF_BACKWARD))
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


		if ((m_dwControlFlags & BC_CFLG_STRAFE_LEFT) || (m_nMouseStrafeFlags & SF_LEFT))
		{
			vAccel -= (vRight * fMoveAccel);
		}

		if ((m_dwControlFlags & BC_CFLG_STRAFE_RIGHT) || (m_nMouseStrafeFlags & SF_RIGHT))
		{
			vAccel += (vRight * fMoveAccel);
		}
	}

	// Reset the mouse strafe flags in case they are set
	m_nMouseStrafeFlags = 0;


	g_pPhysicsLT->SetAcceleration(m_hObject, &vAccel);

    LTVector vel = GetVelocity();;

	// We can jump if we are not dead...

    LTBOOL bOkayToJump = LTFALSE;
	if (bJumping && !g_pGameClientShell->IsPlayerDead())
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

	if (g_pGameClientShell->IsSpectatorMode())
	{
		vel *= 0.9f;
		if (vel.Mag() < 0.1f)
		{
			vel.Init();
		}
	}


	// If zip cord is on, move us towards it the destination point...

	if (m_bZipCordOn && !g_pGameClientShell->IsPlayerDead())
	{
        LTVector myPos;
		g_pLTClient->GetObjectPos(m_hObject, &myPos);
		vel = m_vZipCordPos - myPos;

		// If we're pretty close to the destination, turn off the zipcord.
		// NOTE:  Need to account for dims of the ZipHook object (since
		// it is solid)

        LTVector myDims;
		g_pPhysicsLT->GetObjectDims(m_hObject, &myDims);
		if (vel.Mag() < (myDims.Mag() + m_vZipHookDims.Mag()))
		{
			vel.Init();
			TurnOffZipCord();
		}
		else
		{
			vel.Norm();
			vel *= m_fZipCordVel;
		}
	}

	// Add any container currents to my velocity..

	vel += m_vTotalCurrent;
	g_pPhysicsLT->SetVelocity(m_hObject, &vel);

	// If we're dead, we can't move....

	if (g_pGameClientShell->IsPlayerDead() ||
		!g_pGameClientShell->IsPlayerMovementAllowed())
	{
        LTVector vZero(0, 0, 0);
		g_pPhysicsLT->SetAcceleration(m_hObject, &vZero);
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);
		TurnOffZipCord();
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

    uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hObject) | FLAG_GRAVITY;
	if (g_pGameClientShell->IsSpectatorMode() || m_bZipCordOn)
	{
		dwFlags &= ~FLAG_GRAVITY;
	}
	g_pLTClient->SetObjectFlags(m_hObject, dwFlags);


	m_eBodyContainerCode = CC_NO_CONTAINER;
    m_bBodyInLiquid = m_bBodyOnLadder = LTFALSE;


	// Do ladder containers first...

    LTBOOL bDidLadder = LTFALSE;
    uint32 i;
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
        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
        g_pLTClient->WriteToMessageByte(hMessage, CP_MOTION_STATUS);
        g_pLTClient->WriteToMessageByte(hMessage, nStatus);
        g_pLTClient->EndMessage(hMessage);

		// Play the jump sound for this client so the sound isnt't
		// lagged in multiplayer...

		if (m_bJumped)
		{
			LTBOOL bPlaySound = LTFALSE;
			if (!IsLiquid(m_eCurContainerCode))
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
						pushVec.Norm(velocity);

						vel = GetVelocity();
						vel += pushVec;
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
    uint32 modelAnim, curModelAnim, curFlags;
    LTVector oldDims, offset;

	if (!(hClientObj = g_pLTClient->GetClientObject())) return;

	// Make sure our solid object is on the same animation.
	
	curModelAnim = g_pLTClient->GetModelAnimation(m_hObject);

	// Make sure we are playing the animation corresponding to the dims...
	if (m_pCharFX && m_pCharFX->m_cs.nDimsTracker < m_pCharFX->m_cs.nTrackers)
	{
		g_pModelLT->GetCurAnim(&(m_pCharFX->m_aAnimTrackers[m_pCharFX->m_cs.nDimsTracker]), modelAnim);
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
		curFlags = g_pLTClient->GetObjectFlags(m_hObject);
		g_pLTClient->SetObjectFlags(m_hObject, (curFlags|FLAG_GOTHRUWORLD) & ~FLAG_SOLID);

		g_pLTClient->SetModelAnimation(m_hObject, modelAnim);

		// Get our wanted dims.
		oldDims = m_vWantedDims;
		VEC_SET(m_vWantedDims, 1, 1, 1);
		g_pLTClient->Common()->GetModelAnimUserDims(m_hObject, &m_vWantedDims, modelAnim);

		// Figure out a position offset.
		VEC_INIT(offset);
		if (m_vWantedDims.y < oldDims.y)
		{
			offset.y = -(oldDims.y - m_vWantedDims.y);
			offset.y += .01f; // Fudge factor
		}

		g_pLTClient->SetObjectFlags(m_hObject, curFlags);

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
    LTVector curDims;

	if(!m_hObject || !g_pPhysicsLT)
        return LTTRUE;

	g_pPhysicsLT->GetObjectDims(m_hObject, &curDims);
	return fabs(curDims.x-m_vWantedDims.x) < 0.1f && fabs(curDims.y-m_vWantedDims.y) < 0.1f &&
		fabs(curDims.z-m_vWantedDims.z) < 0.1f;
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
    LTVector smallDims, pos;

	// Don't do stair stepping when we change the dims...

    uint32 dwFlags;
    g_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	dwFlags &= ~FLAG_STAIRSTEP;
    g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);

	// Use normal box physics when changing dims (works better)...

    uint32 dwFlags2;
    g_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags2);
	dwFlags2 &= ~FLAG2_PLAYERCOLLIDE;
    g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags2);

	smallDims.Init(0.5f, 0.5f, 0.5f);

	// See if we're using the non "cat physics" method, i.e., shrinking
	// the bounding box down to a point before moving the object...

	if (g_vtCatPhysics.GetFloat() != 0.0f)
	{
		g_pPhysicsLT->GetObjectDims(m_hObject, &smallDims);
		smallDims.y = DEFAULT_STAIRSTEP_HEIGHT;
	}

	g_pPhysicsLT->SetObjectDims(m_hObject, &smallDims, 0);

	// Move them if they want.
	if (pOffset)
	{
		g_pLTClient->GetObjectPos(m_hObject, &pos);
		pos += *pOffset;
		g_pPhysicsLT->MoveObject(m_hObject, &pos, 0);
	}

	g_pPhysicsLT->SetObjectDims(m_hObject, &m_vWantedDims, SETDIMS_PUSHOBJECTS);

	// Okay, enough of that, back to player physics...

	dwFlags2 |= FLAG2_PLAYERCOLLIDE;
    g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags2);

	// Turn it back on ;)

    dwFlags |= FLAG_STAIRSTEP;
    g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);
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

	ILTClientPhysics* pPhysics = (ILTClientPhysics*)g_pPhysicsLT;

	// We may want gravity to be different for us...

	if (g_vtPlayerGravity.GetFloat() != DEFAULT_WORLD_GRAVITY)
	{
		m_fGravity = g_vtPlayerGravity.GetFloat();
	}


	LTVector vOldGlobalForce(0, 0, 0);
	LTVector vNewGlobalForce(0, m_fGravity, 0);

	pPhysics->GetGlobalForce(vOldGlobalForce);
	pPhysics->SetGlobalForce(vNewGlobalForce);

	info.m_hObject = m_hObject;
	info.m_dt = g_pGameClientShell->GetFrameTime();
	pPhysics->UpdateMovement(&info);

	if (info.m_Offset.MagSqr() > 0.01f)
	{
		g_pLTClient->GetObjectPos(m_hObject, &curPos);
		newPos = curPos + info.m_Offset;
		pPhysics->MoveObject(m_hObject, &newPos, 0);
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
	HOBJECT hObj;

	if(hObj = g_pLTClient->GetClientObject())
	{
		g_pLTClient->SetObjectFlags(hObj, g_pLTClient->GetObjectFlags(hObj)|FLAG_CLIENTNONSOLID);
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

		if (vCurVel.Mag() > 5.0f)
		{
            LTVector vDir = vCurVel;
			vDir.Norm();

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
	HOBJECT hObj = g_pLTClient->GetClientObject();
	if (!m_hObject || !hObj) return;

	m_eLastContainerCode = m_eCurContainerCode;
	m_eCurContainerCode  = g_pGameClientShell->GetCurContainerCode();

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

	// Clear these and they'll get set if mouse is moving...

	m_nMouseStrafeFlags = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnPhysicsUpdate
//
//	PURPOSE:	Handle a physics update
//
// ----------------------------------------------------------------------- //

void CMoveMgr::OnPhysicsUpdate(HMESSAGEREAD hRead)
{
	if (!g_pLTClient || !hRead) return;

    uint16 changeFlags = g_pLTClient->ReadFromMessageWord(hRead);

	// See if our container state changed...

	if (changeFlags & PSTATE_CONTAINERTYPE)
	{
		m_nContainers = g_pLTClient->ReadFromMessageByte(hRead);
		if (m_nContainers >= MAX_TRACKED_CONTAINERS) return;

		for (DWORD i=0; i < m_nContainers; i++)
		{
			m_Containers[i].m_ContainerCode = (ContainerCode)g_pLTClient->ReadFromMessageByte(hRead);
			g_pLTClient->ReadFromMessageVector(hRead, &m_Containers[i].m_Current);
			m_Containers[i].m_fGravity = g_pLTClient->ReadFromMessageFloat(hRead);
			m_Containers[i].m_fViscosity = g_pLTClient->ReadFromMessageFloat(hRead);
			m_Containers[i].m_bHidden = g_pLTClient->ReadFromMessageByte(hRead);
		}

        LTFLOAT fCoeff = g_pLTClient->ReadFromMessageFloat(hRead);
		g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fCoeff);
	}

	// Change our model file?

	if (changeFlags & PSTATE_MODELFILENAMES)
	{
		if (m_hObject)
		{
			g_pLTClient->DeleteObject(m_hObject);
            m_hObject = LTNULL;
		}

		ObjectCreateStruct theStruct;
		INIT_OBJECTCREATESTRUCT(theStruct);

		hRead->ReadStringFL(theStruct.m_Filename, sizeof(theStruct.m_Filename));
		hRead->ReadStringFL(theStruct.m_SkinNames[0], sizeof(theStruct.m_SkinNames[0]));
		hRead->ReadStringFL(theStruct.m_SkinNames[1], sizeof(theStruct.m_SkinNames[1]));

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
		theStruct.m_Flags2 = FLAG2_PLAYERCOLLIDE;

		HOBJECT hClientObj;
		if (hClientObj = g_pLTClient->GetClientObject())
		{
			g_pLTClient->GetObjectPos(hClientObj, &theStruct.m_Pos);
		}

		m_hObject = g_pLTClient->CreateObject(&theStruct);

		if (m_hObject)
		{
            uint32 dwCFlags = g_pLTClient->GetObjectClientFlags(m_hObject);
			g_pLTClient->SetObjectClientFlags(m_hObject, dwCFlags | CF_DONTSETDIMS);
		}
	}

	// Gravity change...

	if (changeFlags & PSTATE_GRAVITY)
	{
        LTVector vGravity;
		g_pLTClient->ReadFromMessageVector(hRead, &vGravity);
		g_pPhysicsLT->SetGlobalForce(vGravity);

		m_fGravity = vGravity.y;
	}

	// Speed change...

	if (changeFlags & PSTATE_SPEEDS)
	{
		m_fWalkVel = g_pLTClient->ReadFromMessageFloat(hRead);
		m_fRunVel = g_pLTClient->ReadFromMessageFloat(hRead);
		m_fSwimVel = g_pLTClient->ReadFromMessageFloat(hRead);
		m_fJumpVel = g_pLTClient->ReadFromMessageFloat(hRead);
		m_fZipCordVel = g_pLTClient->ReadFromMessageFloat(hRead);

		m_fMoveAccelMultiplier = m_fMoveMultiplier = g_pLTClient->ReadFromMessageFloat(hRead);

		m_fBaseMoveAccel = g_pLTClient->ReadFromMessageFloat(hRead);
		m_fJumpMultiplier = g_pLTClient->ReadFromMessageFloat(hRead);
		m_fLadderVel = g_pLTClient->ReadFromMessageFloat(hRead);

        LTFLOAT fCoeff = g_pLTClient->ReadFromMessageFloat(hRead);
		g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fCoeff);
	}

	// Vehicle status change...

	if (changeFlags & PSTATE_PHYSICS_MODEL)
	{
		PlayerPhysicsModel eModel = (PlayerPhysicsModel) g_pLTClient->ReadFromMessageByte(hRead);
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

LTRESULT CMoveMgr::OnObjectMove(HOBJECT hObj, LTBOOL bTeleport, LTVector *pPos)
{
	if (!m_hObject) return LT_OK;

	ILTClientPhysics* pPhysics = (ILTClientPhysics*)g_pPhysicsLT;

	HOBJECT hClientObj = g_pLTClient->GetClientObject();

	// If it's a solid world model moving, do a regular MoveObject on it so it
	// can carry/push the player object around.

	if (!bTeleport && hObj != hClientObj && hObj != m_hObject)
	{
        uint32 type = g_pLTClient->GetObjectType(hObj);
		if (type == OT_WORLDMODEL)
		{
			if (g_pLTClient->GetObjectFlags(hObj) & FLAG_SOLID)
			{
				pPhysics->MovePushObjects(hObj, *pPos, &m_hObject, 1);
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

LTRESULT CMoveMgr::OnObjectRotate(HOBJECT hObj, LTBOOL bTeleport, LTRotation *pNewRot)
{
	if (!m_hObject) return LT_OK;

	ILTClientPhysics* pPhysics = (ILTClientPhysics*)g_pPhysicsLT;

	HOBJECT hClientObj = g_pLTClient->GetClientObject();

	// If it's a solid world model moving, do a regular MoveObject on it so it
	// can carry/push the player object around.
	if (!bTeleport && hObj != hClientObj && hObj != m_hObject)
	{
		uint32 type = g_pLTClient->GetObjectType(hObj);
		if (type == OT_WORLDMODEL)
		{
			if (g_pLTClient->GetObjectFlags(hObj) & FLAG_SOLID)
			{
				pPhysics->RotatePushObjects(hObj, *pNewRot, &m_hObject, 1);
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

LTRESULT CMoveMgr::AddPusher(LTVector &pos, float radius, float startDelay, float duration, float strength)
{
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
//	ROUTINE:	CMoveMgr::MoveToClientObj
//
//	PURPOSE:	Move to the client object pos
//
// ----------------------------------------------------------------------- //

void CMoveMgr::MoveToClientObj()
{
    LTVector cObjPos;
	HOBJECT hObj;

	if(!m_hObject || !(hObj = g_pLTClient->GetClientObject()))
		return;

	g_pLTClient->GetObjectPos(hObj, &cObjPos);
	g_pPhysicsLT->MoveObject(m_hObject, &cObjPos, MOVEOBJECT_TELEPORT);
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
    LTVector vPos, vZero;
    uint32 curFlags;

	if(!m_hObject || !g_pPhysicsLT)
		return;

	if(bSet)
	{
		// Move up.
		g_pLTClient->GetObjectPos(m_hObject, &vPos);
		vPos.y += 50;
		g_pPhysicsLT->MoveObject(m_hObject, &vPos, MOVEOBJECT_TELEPORT);

		curFlags = g_pLTClient->GetObjectFlags(m_hObject);
		curFlags |= FLAG_GOTHRUWORLD;
		curFlags &= ~FLAG_SOLID;
		g_pLTClient->SetObjectFlags(m_hObject, curFlags);

		vZero.Init();
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);
		g_pPhysicsLT->SetAcceleration(m_hObject, &vZero);
	}
	else
	{
		curFlags = g_pLTClient->GetObjectFlags(m_hObject);
		curFlags &= ~FLAG_GOTHRUWORLD;
		curFlags |= FLAG_SOLID;
		g_pLTClient->SetObjectFlags(m_hObject, curFlags);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::OnServerForcePos
//
//	PURPOSE:	Handle forcing our position
//
// ----------------------------------------------------------------------- //

void CMoveMgr::OnServerForcePos(HMESSAGEREAD hRead)
{
 	if (!g_pLTClient || !g_pPhysicsLT) return;

 	m_ClientMoveCode = g_pLTClient->ReadFromMessageByte(hRead);

	// Teleport to where it says.
	if (m_hObject)
	{
		SetClientObjNonsolid();

		// Move there.  We make our object a point first and then resize the dims so
		// we don't teleport clipping into the world.

		LTVector vPos, vCurDims;
		LTVector vTempDims(0.5f, 0.5f, 0.5f);

		g_pLTClient->ReadFromMessageVector(hRead, &vPos);

		g_pPhysicsLT->GetObjectDims(m_hObject, &vCurDims);
		g_pPhysicsLT->SetObjectDims(m_hObject, &vTempDims, 0);
		g_pPhysicsLT->MoveObject(m_hObject, &vPos, MOVEOBJECT_TELEPORT);
		g_pPhysicsLT->SetObjectDims(m_hObject, &vCurDims, SETDIMS_PUSHOBJECTS);

		// Clear our velocity and acceleration.

		LTVector vVel(0, 0, 0);
		g_pPhysicsLT->SetAcceleration(m_hObject, &vVel);

		if (m_bLoading)
		{
			vVel = m_vSavedVel;
			m_bLoading = LTFALSE;
		}

		g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
		m_fLastOnGroundY = MIN_ONGROUND_Y;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::WritePositionInfo
//
//	PURPOSE:	Write our position info
//
// ----------------------------------------------------------------------- //

void CMoveMgr::WritePositionInfo(HMESSAGEWRITE hWrite)
{
    LTVector myPos, myVel;

	if (m_hObject)
	{
		g_pLTClient->GetObjectPos(m_hObject, &myPos);
		myVel = GetVelocity();
	}
	else
	{
		myPos.Init();
		myVel.Init();
	}

	g_pLTClient->WriteToMessageByte(hWrite, m_ClientMoveCode);
	g_pLTClient->WriteToMessageVector(hWrite, &myPos);
	g_pLTClient->WriteToMessageVector(hWrite, &myVel);
    g_pLTClient->WriteToMessageByte(hWrite, (uint8)m_bOnGround);
    g_pLTClient->WriteToMessageByte(hWrite, (uint8)m_eStandingOnSurface);
    g_pLTClient->WriteToMessageDWord(hWrite, (uint32)m_hStandingOnPoly);
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
			if (bRunning && !(m_dwControlFlags & BC_CFLG_DUCK) && !g_pGameClientShell->IsZoomed())
			{
				fMaxVel = m_fRunVel;
			}
			else
			{
				fMaxVel = m_fWalkVel;
			}
		}

		fMaxVel *= fMoveMultiplier;
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

void CMoveMgr::SetVelocity(LTVector vVel)
{
	if (g_pPhysicsLT && m_hObject)
	{
		g_pPhysicsLT->SetVelocity(m_hObject, &vVel);
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
//	ROUTINE:	CMoveMgr::TurnOnZipCord
//
//	PURPOSE:	Turn on the zipcord (if it is off)
//
// ----------------------------------------------------------------------- //

void CMoveMgr::TurnOnZipCord(HOBJECT hHookObj)
{
	if (!hHookObj) return;

	if (!m_bZipCordOn)
	{
        m_bZipCordOn = LTTRUE;

        g_pLTClient->GetObjectPos(hHookObj, &m_vZipCordPos);
		g_pPhysicsLT->GetObjectDims(hHookObj, &m_vZipHookDims);

		// Tell the server the zipcord is  on...

        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
        g_pLTClient->WriteToMessageByte(hMessage, CP_ZIPCORD);
        g_pLTClient->WriteToMessageByte(hMessage, ZC_ON);
        g_pLTClient->WriteToMessageVector(hMessage, &m_vZipCordPos);
        g_pLTClient->EndMessage(hMessage);

		// Start the zip-cord sound...

		if (!m_hZipcordSnd)
		{
			char* pSound = "Guns\\snd\\Zipcord\\Zipcord.wav";
			uint32 dwFlags = PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT;
			m_hZipcordSnd = g_pClientSoundMgr->PlaySoundLocal(pSound, SOUNDPRIORITY_PLAYER_HIGH, dwFlags);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::TurnOffZipCord
//
//	PURPOSE:	Turn off the zipcord (if it is on)
//
// ----------------------------------------------------------------------- //

void CMoveMgr::TurnOffZipCord()
{
	if (m_bZipCordOn)
	{
        m_bZipCordOn = LTFALSE;

		// Tell the server the zipcord is no longer on...

        HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
        g_pLTClient->WriteToMessageByte(hMessage, CP_ZIPCORD);
        g_pLTClient->WriteToMessageByte(hMessage, ZC_OFF);
        g_pLTClient->EndMessage(hMessage);


		// Stop the zipcord sound...

		if (m_hZipcordSnd)
		{
			g_pLTClient->KillSoundLoop(m_hZipcordSnd);
			m_hZipcordSnd = LTNULL;
		}
	}
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

    uint32 dwFlags = g_pLTClient->GetObjectFlags(pInfo->m_hObject);
	if (!(dwFlags & FLAG_SOLID)) return;

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

void CMoveMgr::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	m_vSavedVel.Init();
	g_pLTClient->ReadFromMessageVector(hRead, &m_vSavedVel);
	m_fLastOnGroundY = g_pLTClient->ReadFromMessageFloat(hRead);

	m_bLoading = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::Save
//
//	PURPOSE:	Handle saving move mgr data
//
// ----------------------------------------------------------------------- //

void CMoveMgr::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	// Save velocity...

	LTVector myVel(0, 0, 0);

	if (m_hObject)
	{
		g_pPhysicsLT->GetVelocity(m_hObject, &myVel);
	}

	g_pLTClient->WriteToMessageVector(hWrite, &myVel);
	g_pLTClient->WriteToMessageFloat(hWrite, m_fLastOnGroundY);
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