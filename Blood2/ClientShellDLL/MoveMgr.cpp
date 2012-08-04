#include "client_de.h"
#include "physics_lt.h"
#include "MoveMgr.h"
#include "BloodClientShell.h"
#include "SharedMovement.h"
#include "Commands.h"
#include "ClientServerShared.h"
#include "SharedDefs.h"
#include "ClientUtilities.h"


#define MIN_SWIM_ON_SURFACE_TIME	0.5f
#define SPECTATOR_ACCELERATION		100000.0f

#define LADDER_STOP_TIME			0.2f
#define SWIM_STOP_TIME				1.0f
#define DEFAULT_JUMPVEL				600.0f


// Values to use for acceleration & velocity
//DFLOAT gMoveAccel[6] = { 3300.0f, 3750.0f, 4200.0f, 4650.0f, 4950.0f, 5250.0f };
static DFLOAT fDragCoeff = 15.0f;

// Max velocity is gMoveAccel/fDragCoeff




class Pusher
{
public:
			Pusher()
			{
				m_Link.m_pData = this;
			}
	
	DVector	m_Pos;
	float	m_Radius;
	float	m_Delay;	// How long before it starts actually pushing.
	float	m_TimeLeft; // Time left for this sphere.
	float	m_Strength;
	DLink	m_Link;
};


CMoveMgr::CMoveMgr(CBloodClientShell *pShell)
{
	VEC_SET(m_WantedDims, 1, 1, 1);
	m_nStillAnims		= 0;
	m_bSwimmingJump		= DFALSE;
	m_eLastContainerCode= CC_NOTHING;
	m_nContainers		= 0;
	m_hObject			= DNULL;
	m_pClientDE			= DNULL;
	m_pClientShell		= pShell;
	dl_TieOff(&m_Pushers);
	m_dwControlFlags	= 0;
	m_dwLastControlFlags= 0;
	m_bMovementBlocked	= DFALSE;
	m_bForcedCrouch		= DFALSE;
	m_wLastChangeFlags	= 0;
	m_eLastSurface		= SURFTYPE_UNKNOWN;
}


CMoveMgr::~CMoveMgr()
{
	DLink *pCur, *pNext;

	for(pCur=m_Pushers.m_pNext; pCur != &m_Pushers; pCur=pNext)
	{
		pNext = pCur->m_pNext;
		delete (Pusher*)pCur->m_pData;
	}
	dl_TieOff(&m_Pushers);
}


DBOOL CMoveMgr::Init(ClientDE *pClientDE)
{
	m_pClientDE = pClientDE;
	m_pPhysicsLT = pClientDE->Physics();

	// Init some defaults.  These should NEVER get used because we don't
	// have our object until the server sends the physics update.
	m_fMoveVel = 0.0f;
	m_fJumpVel = DEFAULT_JUMPVEL;
	m_fSwimVel = DEFAULT_SWIM_VEL;
	m_fLadderVel = DEFAULT_LADDER_VEL;
	m_fFrictionCoeff = 0.0f;
	m_fMoveMultiplier = 1.0f;
	m_bBodyInLiquid = DFALSE;
	m_bBodyOnLadder = DFALSE;
	m_bBodyOnConveyor = DFALSE;
	m_bOnGround = DTRUE;
	m_nMouseStrafeFlags = 0;
	m_bSwimmingOnSurface = DFALSE;
	m_bCanSwimJump = DFALSE;

	m_fLeashLen = 100.0f;
	m_fBaseMoveAccel = 0.0f;
	m_fMoveAccelMultiplier = 1.0f;
	m_fJumpMultiplier = 1.0f;

	VEC_INIT( m_vServerVelocity );

	return DTRUE;
}


void CMoveMgr::UpdateAxisMovement(DBOOL bUseAxisForwardBackward, float fForwardBackward, float fForwardBackwardDeadZone,
								 DBOOL bUseAxisLeftRight, float fLeftRight, float fLeftRightDeadZone )
{
	m_bUseAxisForwardBackward = bUseAxisForwardBackward;
	m_fAxisForwardBackwardVel = fForwardBackward;
	m_fAxisForwardBackwardDeadZone = fForwardBackwardDeadZone;
	m_bUseAxisLeftRight = bUseAxisLeftRight;
	m_fAxisLeftRightVel = fLeftRight;
	m_fAxisLeftRightDeadZone = fLeftRightDeadZone;
}


void CMoveMgr::UpdateControlFlags()
{
	// Clear control flags...

	m_dwLastControlFlags = m_dwControlFlags;
	m_dwControlFlags = 0; 


	// Make sure it's ok for us to move...

	if (g_pBloodClientShell->IsDead()) return;
	if (!(m_pClientShell->GetCDataFlags() & CDATA_CANMOVE)) return;
	if (g_pBloodClientShell->IsWonkyNoMove()) return;	// [blg] 01/13/99


	// Determine what commands are currently on...

	if (m_pClientDE->IsCommandOn(COMMAND_RUN) || m_pClientShell->GetCDataFlags() & CDATA_RUNLOCK)
	{
		m_dwControlFlags |= CTRLFLAG_RUN;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_JUMP))
	{
		m_dwControlFlags |= CTRLFLAG_JUMP;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_DUCK))
	{
		m_dwControlFlags |= CTRLFLAG_CROUCH;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_FORWARD))
	{
		m_dwControlFlags |= CTRLFLAG_FORWARD;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_BACKWARD))
	{
		m_dwControlFlags |= CTRLFLAG_BACKWARD;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_LEFT))
	{
		m_dwControlFlags |= CTRLFLAG_LEFT;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_RIGHT))
	{
		m_dwControlFlags |= CTRLFLAG_RIGHT;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_STRAFE))
	{
		m_dwControlFlags |= CTRLFLAG_STRAFE;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_STRAFERIGHT))
	{
		m_dwControlFlags |= CTRLFLAG_STRAFERIGHT;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_STRAFELEFT))
	{
		m_dwControlFlags |= CTRLFLAG_STRAFELEFT;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_FIRE))
	{
		m_dwControlFlags |= CTRLFLAG_FIRE;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_ALTFIRE))
	{
		m_dwControlFlags |= CTRLFLAG_ALTFIRE;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_GRAB))
	{
		m_dwControlFlags |= CTRLFLAG_GRAB;
	}

	if (m_pClientDE->IsCommandOn(COMMAND_TAUNT))
	{
		m_dwControlFlags |= CTRLFLAG_TAUNT;
	}
}


void CMoveMgr::UpdateInLiquid(CContainerInfo *pInfo)
{
	if(pInfo->m_bHidden)
		return;

	// If head is above water and standing on something, then don't inhibit movement.
	DBOOL bHeadInLiquid = IsLiquid(m_pClientShell->GetCurContainerCode());

	// Don't limit velocity if on the ground
	if( !bHeadInLiquid && m_bOnGround )
		return;

	m_bBodyInLiquid = DTRUE;

	// Do REAL friction dampening (i.e., actually change our velocity)...

	DVector vVel;
	m_pPhysicsLT->GetVelocity(m_hObject, &vVel);

	DFLOAT fTimeDelta = m_FrameTime;

	DVector vCurVel;
	VEC_COPY(vCurVel, vVel);

	if (VEC_MAG(vCurVel) > 1.0f)
	{
		DVector vDir;
		VEC_COPY(vDir, vCurVel);
		VEC_NORM(vDir);

		DFLOAT fAdjust = m_FrameTime*(m_fSwimVel/SWIM_STOP_TIME);

		VEC_MULSCALAR(vVel, vDir, fAdjust);

		if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
		{
			VEC_SUB(vVel, vCurVel, vVel);
		}
		else
		{
			VEC_INIT(vVel);
		}
	}

	DVector curAccel;
	m_pPhysicsLT->GetAcceleration(m_hObject, &curAccel);

	// Handle floating around on the surface...

	if (m_bSwimmingOnSurface)
	{
		DBOOL bMoving = ((VEC_MAG(curAccel) > 0.01f) || (VEC_MAG(vVel) > 0.01f));
	
		// Disable gravity.
		m_pClientDE->SetObjectFlags(m_hObject, 
			m_pClientDE->GetObjectFlags(m_hObject) & ~FLAG_GRAVITY);

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
	else if (IsLiquid(pInfo->m_ContainerCode))
	{
		m_pClientDE->SetObjectFlags(m_hObject, 
			m_pClientDE->GetObjectFlags(m_hObject) & ~FLAG_GRAVITY);

		curAccel.y += pInfo->m_fGravity;
	}

	m_pPhysicsLT->SetVelocity(m_hObject, &vVel);
	m_pPhysicsLT->SetAcceleration(m_hObject, &curAccel);
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
	if(pInfo->m_bHidden)
		return;

	m_bBodyOnLadder = DTRUE;

	// Do REAL friction dampening (i.e., actually change our velocity)...

	DVector vVel;
	m_pPhysicsLT->GetVelocity(m_hObject, &vVel);

	DVector vCurVel;
	VEC_COPY(vCurVel, vVel);

	if (VEC_MAG(vCurVel) > 1.0f)
	{
		DVector vDir;
		VEC_COPY(vDir, vCurVel);
		VEC_NORM(vDir);

		DFLOAT fAdjust = m_FrameTime*(m_fLadderVel/LADDER_STOP_TIME);

		VEC_MULSCALAR(vVel, vDir, fAdjust);

		if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
		{
			VEC_SUB(vVel, vCurVel, vVel);
		}
		else
		{
			VEC_INIT(vVel);
		}

		m_pPhysicsLT->SetVelocity(m_hObject, &vVel);
	}

	m_pClientDE->SetObjectFlags(m_hObject, 
		m_pClientDE->GetObjectFlags(m_hObject) & ~FLAG_GRAVITY);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::UpdateOnConveyor
//
//	PURPOSE:	Update movement when on a conveyor.  Just use ladder
//				restrictions.
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateOnConveyor(CContainerInfo *pInfo)
{
	if(pInfo->m_bHidden)
		return;

	m_bBodyOnConveyor = DTRUE;

	// Do REAL friction dampening (i.e., actually change our velocity)...

	DVector vVel;
	m_pPhysicsLT->GetVelocity(m_hObject, &vVel);

	DVector vCurVel;
	VEC_COPY(vCurVel, vVel);

	if (VEC_MAG(vCurVel) > 1.0f)
	{
		DVector vDir;
		VEC_COPY(vDir, vCurVel);
		VEC_NORM(vDir);

		DFLOAT fAdjust = m_FrameTime*(m_fLadderVel/LADDER_STOP_TIME);

		VEC_MULSCALAR(vVel, vDir, fAdjust);

		if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
		{
			VEC_SUB(vVel, vCurVel, vVel);
		}
		else
		{
			VEC_INIT(vVel);
		}

		m_pPhysicsLT->SetVelocity(m_hObject, &vVel);
	}
}

void CMoveMgr::UpdateOnGround(CollisionInfo *pInfo)
{
	DVector vAccel, vCross1, vCross2;
	DVector vVel;

	// Lets see if we are in the ground or in the air.

	m_pPhysicsLT->GetStandingOn(m_hObject, pInfo);

	if (pInfo->m_hObject) 
	{
		m_bOnGround = DTRUE;

		// Get the last surface type (for footsteps, etc)
		m_eLastSurface = ::GetSurfaceType(pInfo->m_hObject, pInfo->m_hPoly);

		// Don't allow climbing slopes greater than 60 degrees.
		if( pInfo->m_Plane.m_Normal.y < 0.5 )
		{
			m_pPhysicsLT->GetAcceleration( m_hObject, &vAccel );
			DVector vCross1, vCross2;
			VEC_SET(vCross1, 0.0, 1.0, 0.0);
			VEC_CROSS(vCross2, pInfo->m_Plane.m_Normal, vCross1);
			VEC_NORM(vCross2);
			VEC_CROSS(vCross1, vCross2, pInfo->m_Plane.m_Normal);
			VEC_MULSCALAR(vCross1, vCross1, -2000.0f);
			VEC_ADD(vAccel, vAccel, vCross1);
			m_pPhysicsLT->SetAcceleration( m_hObject, &vAccel );

			m_bOnGround = DFALSE;

			// Force us down...

			m_pPhysicsLT->GetVelocity(m_hObject, &vVel);

			vVel.y = - VEC_MAG(vVel);
			vVel.x = vVel.z = 0.0f;
			m_pPhysicsLT->SetVelocity(m_hObject, &vVel);
		}
	} 
	else 
	{
		m_bOnGround = DFALSE;
	}
}


DBOOL CMoveMgr::IsBodyInLiquid()
{
	DDWORD i;

	for(i=0; i < m_nContainers; i++)
	{
		if(IsLiquid(m_Containers[i].m_ContainerCode))
			return DTRUE;
	}
	return DFALSE;
}


DBOOL CMoveMgr::IsDead()
{
	return m_pClientShell->IsDead();
}


void CMoveMgr::UpdateMotion()
{
	DVector		vMyVel;
	DVector		vAccel;
	DVector		vUp;
	DVector		vRight;
	DVector		vForward;
	DVector		vMyPos;
	DFLOAT		fMaxMoveAccel;
	DFLOAT		fMaxMoveVel;
	DFLOAT		fMoveAccel = 0.0;
	DFLOAT		fStrafeAccel = 0;
	DFLOAT		fMoveVel;
	DFLOAT		fOrigMaxMoveVel;

	// Normally we have gravity on, but the containers might turn it off.
	DDWORD dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

	if(!m_pClientShell->IsSpectatorMode())
	{
		dwFlags |= FLAG_GRAVITY | FLAG_SOLID;
		dwFlags &= ~(FLAG_GOTHRUWORLD);
	}
	else
	{
		dwFlags &= ~(FLAG_SOLID | FLAG_GRAVITY);
		dwFlags |= FLAG_GOTHRUWORLD;
	}

	m_pClientDE->SetObjectFlags(m_hObject, dwFlags);
	
	// Zero acceleration to start with
	VEC_INIT(vAccel);
	m_pPhysicsLT->SetAcceleration(m_hObject, &vAccel);


	// Get the axis offset
	DFLOAT offsets[3];
	g_pClientDE->GetAxisOffsets(offsets);

	DFLOAT fMouseAxis0 = offsets[0];
	DFLOAT fMouseAxis1 = offsets[1];

	
	// Update containers
	DBOOL		bDidLiquid = DFALSE;
	DBOOL		bDidLadder = DFALSE;
	DBOOL		bDidConveyor = DFALSE;
	for(unsigned i=0; i < m_nContainers; i++)
	{
		if(IsLiquid(m_Containers[i].m_ContainerCode) && !bDidLiquid)
		{
			UpdateInLiquid(&m_Containers[i]);
			bDidLiquid = DTRUE;
		}
		else if(m_Containers[i].m_ContainerCode == CC_LADDER && !bDidLadder)
		{
			UpdateOnLadder(&m_Containers[i]);
			bDidLadder = DTRUE;
		}
		else if( m_Containers[i].m_ContainerCode == CC_CONVEYOR && !bDidConveyor )
		{
			UpdateOnConveyor(&m_Containers[i]);
			bDidConveyor = DTRUE;
		}
	
		if(!m_Containers[i].m_bHidden)
		{
			m_pPhysicsLT->GetVelocity(m_hObject, &vMyVel);
			vMyVel += m_Containers[i].m_Current * m_FrameTime;
			m_pPhysicsLT->SetVelocity(m_hObject, &vMyVel);
		}
	}

	DBOOL		bHeadInLiquid = IsLiquid(m_pClientShell->GetCurContainerCode());
	DBOOL		bInLiquid	  = bHeadInLiquid || m_bBodyInLiquid;
	DBOOL		bFreeMovement = bHeadInLiquid || m_bBodyOnLadder || m_pClientShell->IsSpectatorMode();
	DBOOL		bLateralMovement = DFALSE;


	// Get my position
	m_pClientDE->GetObjectPos(m_hObject, &vMyPos);

	CollisionInfo collisionInfo;
	UpdateOnGround(&collisionInfo);

	m_pPhysicsLT->GetAcceleration(m_hObject, &vAccel);

	// Get current velocity
	m_pPhysicsLT->GetVelocity(m_hObject, &vMyVel);

	DFLOAT fMyVelMag = VEC_MAG(vMyVel);

	// BEGIN 10/6/98 Kevin additions ////////////////////////////////////

	// KEVIN's ASSY ADDITIONS 1/9/99
	m_fSwimVel	 = (m_dwControlFlags & CTRLFLAG_RUN) ? DEFAULT_SWIM_VEL : DEFAULT_SWIM_VEL/2.0f;
	m_fLadderVel = (m_dwControlFlags & CTRLFLAG_RUN) ? DEFAULT_LADDER_VEL : DEFAULT_LADDER_VEL/2.0f;

	fMoveVel = m_fMoveVel;

	if (m_dwControlFlags & CTRLFLAG_RUN)
	{
		fMoveVel *= 1.5;
	}

	if (((m_dwControlFlags & CTRLFLAG_CROUCH) || m_bForcedCrouch) && !bFreeMovement)
	{
		fMoveVel /= 2;
	}

	// Use swim vel if all the way in water or on surface but not standing on something.
	if( m_bBodyInLiquid )
		fMaxMoveVel = m_fSwimVel;
	// Use ladder velocity
	else if( m_bBodyOnLadder )
		fMaxMoveVel = m_fLadderVel;
	// Use regular velocity
	else
		fMaxMoveVel = fMoveVel;


	// Cap Velocity for joystick or other velocity related input device
	{
		fOrigMaxMoveVel = fMaxMoveVel;

		DBOOL OverrideForwardBackward = ((m_dwControlFlags & CTRLFLAG_FORWARD) || (m_dwControlFlags & CTRLFLAG_BACKWARD));
		DBOOL OverrideLeftRight = ((m_dwControlFlags & CTRLFLAG_STRAFELEFT) || (m_dwControlFlags & CTRLFLAG_STRAFERIGHT));

		if ((m_bUseAxisForwardBackward || m_bUseAxisLeftRight) && !(OverrideForwardBackward && OverrideLeftRight))
		{
			float fLeftRightCap = fMaxMoveVel;
			float fForwardBackwardCap = fMaxMoveVel;

			if (m_bUseAxisForwardBackward && !OverrideForwardBackward)
			{
				fForwardBackwardCap = fMaxMoveVel * m_fAxisForwardBackwardVel;
			}

			if (m_bUseAxisLeftRight && !OverrideLeftRight)
			{
				fLeftRightCap = fMaxMoveVel * m_fAxisLeftRightVel;
			}

			fMaxMoveVel = (float)sqrt(fForwardBackwardCap*fForwardBackwardCap + fLeftRightCap*fLeftRightCap);

			if (fMaxMoveVel > fOrigMaxMoveVel) fMaxMoveVel = fOrigMaxMoveVel;
		}
	}


	fMaxMoveAccel = ( m_bBodyOnLadder ) ? fMaxMoveVel * fDragCoeff * 0.5f : fMaxMoveVel * fDragCoeff;

	// KEVIN's ASSY ADDITIONS 1/9/99

	
	if (!m_bBodyInLiquid) m_bSwimmingJump = DFALSE;

	// Limit velocity when in certain containers...
	if (m_bBodyOnLadder || m_bBodyOnConveyor)
	{
		if (fMyVelMag > m_fLadderVel)
		{
			VEC_NORM(vMyVel);
			VEC_MULSCALAR(vMyVel, vMyVel, m_fLadderVel);
		}
	}
	else if ( m_bBodyInLiquid && ( bHeadInLiquid || !m_bOnGround ))
	{
		if (fMyVelMag > m_fSwimVel)
		{
			VEC_NORM(vMyVel);
			VEC_MULSCALAR(vMyVel, vMyVel, m_fSwimVel);
			if (m_bSwimmingJump)	// Keep setting jump speed until I jump out
				vMyVel.y = m_fJumpVel * 0.75f;
		}
	}
	// END 10/6/98 Kevin additions ////////////////////////////////////


	DRotation myRot;
	m_pClientShell->GetCameraRotation(&myRot);
	m_pClientDE->SetObjectRotation(m_hObject, &myRot);

	// See if dead
	if (!IsDead())
	{
		// get the object's vectors

		m_pClientDE->GetRotationVectors(&myRot, &vUp, &vRight, &vForward);

		if (m_pClientShell->IsSpectatorMode())
		{
//			DVector vel;
//			VEC_INIT(vel);
//			m_pPhysicsLT->SetVelocity(m_hObject, &vel);
			fMaxMoveAccel = 1.0f;
			fMaxMoveVel = 500.0f;
		}
		else if (!bFreeMovement && !bInLiquid)
		{
			// Only want x and z components for movement
			vRight.y = 0;
			VEC_NORM(vRight)

			vForward.y = 0;
			VEC_NORM(vForward)
		}

		// Set acceleration based on player controls

		// Move forward/backwards
		if ((m_dwControlFlags & CTRLFLAG_FORWARD) && !m_bMovementBlocked)
		{
			fMoveAccel += fMaxMoveAccel;
			bLateralMovement = DTRUE;
		}
		if ((m_dwControlFlags & CTRLFLAG_BACKWARD) && !m_bMovementBlocked)
		{
			fMoveAccel += -fMaxMoveAccel;
			bLateralMovement = DTRUE;
		}
/*
		// set the movement for velocity axis
		if (m_bUseAxisForwardBackward || m_bUseAxisLeftRight)
		{
			DVector vTemp;

			// get current velocity
			DVector velCurrent;
			m_pPhysicsLT->GetVelocity(m_hObject, &velCurrent);

			if (m_bUseAxisForwardBackward)
			{
				// figure out current velocity component forward
				float fVelCurrentForward;
				fVelCurrentForward = VEC_DOT(vForward, velCurrent);

				// is joystick pointing forward?
				if (m_fAxisForwardBackwardVel > m_fAxisForwardBackwardDeadZone)
				{
					// check if we are going to apply acceleration forward
					if (fVelCurrentForward < (m_fAxisForwardBackwardVel * fOrigMaxMoveVel))
					{
//						VEC_MULSCALAR(vTemp, vForward, fMoveAccel);
//						VEC_ADD(vAccel, vAccel, vTemp);
						fMoveAccel += fMaxMoveAccel;
					}
				}

				// is joystick pointing backward?
				else if (m_fAxisForwardBackwardVel < -m_fAxisForwardBackwardDeadZone)
				{
					// check if we are going to apply acceleration backward
					if (fVelCurrentForward > (m_fAxisForwardBackwardVel * fOrigMaxMoveVel))
					{
//						VEC_MULSCALAR(vTemp, vForward, -fMoveAccel);
//						VEC_ADD(vAccel, vAccel, vTemp);
						fMoveAccel -= fMaxMoveAccel;
					}
				}
			}

			if (m_bUseAxisLeftRight)
			{
				// figure out current velocity component to the right
				float fVelCurrentRight;
				fVelCurrentRight = VEC_DOT(vRight, velCurrent);

				// is joystick pointing right?
				if (m_fAxisLeftRightVel > m_fAxisLeftRightDeadZone)
				{
					// check if we are going to apply acceleration right
					if (fVelCurrentRight < (m_fAxisLeftRightVel * fOrigMaxMoveVel))
					{
//						VEC_MULSCALAR(vTemp, vRight, fMoveAccel);
//						VEC_ADD(vAccel, vAccel, vTemp);
						fStrafeAccel += +fMaxMoveAccel;
						bLateralMovement = DTRUE;
					}
				}

				// is joystick pointing left?
				else if (m_fAxisLeftRightVel < -m_fAxisLeftRightDeadZone)
				{
					// check if we are going to apply acceleration left
					if (fVelCurrentRight > (m_fAxisLeftRightVel * fOrigMaxMoveVel))
					{
//						VEC_MULSCALAR(vTemp, vRight, -fMoveAccel);
//						VEC_ADD(vAccel, vAccel, vTemp);
						fStrafeAccel += -fMaxMoveAccel;
						bLateralMovement = DTRUE;
					}
				}
			}
		}
*/
		if (!(m_pClientShell->GetCDataFlags() & CDATA_MOUSEAIMING) && !m_bMovementBlocked)
		{
			fMaxMoveAccel *= 2;	// [blg] allow a little more acceleration when moving with the mouse
			fMoveAccel += -(fMouseAxis1) * fMaxMoveAccel * 10.0f;	// [blg] 01/25/99 turned mouse movement back on
			m_dwControlFlags |= CTRLFLAG_FORWARD;
			bLateralMovement = DTRUE;
		}

		if (fMoveAccel > fMaxMoveAccel) 
			fMoveAccel = fMaxMoveAccel;
		else if (fMoveAccel < -fMaxMoveAccel) 
			fMoveAccel = -fMaxMoveAccel;

		VEC_MULSCALAR(vForward, vForward, fMoveAccel)
		VEC_ADD(vAccel, vAccel, vForward)

		// Strafe.. Check for strafe modifier first
		if (m_dwControlFlags & CTRLFLAG_STRAFE)
		{
			if (m_dwControlFlags & CTRLFLAG_LEFT)
				fStrafeAccel += -fMaxMoveAccel;
			if (m_dwControlFlags & CTRLFLAG_RIGHT)
				fStrafeAccel += fMaxMoveAccel;

			fStrafeAccel += (fMouseAxis0) * fMaxMoveAccel * 4.0f;	// [blg] 01/25/99 turned mouse movement back on

			bLateralMovement = DTRUE;
		}

		// Check individual strafe commands
		if (m_dwControlFlags & CTRLFLAG_STRAFELEFT)
		{
			fStrafeAccel += -fMaxMoveAccel;
			bLateralMovement = DTRUE;
		}
		if (m_dwControlFlags & CTRLFLAG_STRAFERIGHT)
		{
			fStrafeAccel += fMaxMoveAccel;
			bLateralMovement = DTRUE;
		}

		if (fStrafeAccel > fMaxMoveAccel) 
			fStrafeAccel = fMaxMoveAccel;
		else if (fStrafeAccel < -fMaxMoveAccel) 
			fStrafeAccel = -fMaxMoveAccel;

		VEC_MULSCALAR(vRight, vRight, fStrafeAccel)
		VEC_ADD(vAccel, vAccel, vRight)

		// tone down run-strafing, but not too much.
		DFLOAT fAccelMag = VEC_MAG(vAccel);
		DFLOAT fMaxRunStrafeAccel = (DFLOAT)sqrt((fMaxMoveAccel * fMaxMoveAccel) * 2);
		if (fAccelMag > fMaxRunStrafeAccel)
		{
			VEC_MULSCALAR(vAccel, vAccel, (fMaxRunStrafeAccel/fAccelMag));
		}

		DBOOL  bSteep = DFALSE;
		if (!m_pClientShell->IsSpectatorMode() && !m_bMovementBlocked)
		{
			// Make sure the slope isn't too steep to climb or jump up (prevent jump-skip)

			if (m_bOnGround && !bFreeMovement)
			{
				// If standing on something, tilt the acceleration so it's aligned with
				// the surface.
				DVector &v = collisionInfo.m_Plane.m_Normal;
				if ((v.z + v.x) > v.y)			// Check for slope > 45 degrees
				{
					vAccel.y -= 4000.0f;		// Just to make sure I slide down
					bSteep = DTRUE;
				}
				else
				{
					TiltVectorToPlane(&vAccel, &collisionInfo.m_Plane.m_Normal);
				}
			}

			// See if we just broke the surface of water...
			if ((IsLiquid(m_eLastContainerCode) && !bHeadInLiquid) && !m_bOnGround && !m_bBodyOnLadder)
			{
				m_bSwimmingOnSurface = DTRUE;
			}
			else if (bHeadInLiquid)  // See if we went back under...
			{
				m_bSwimmingOnSurface = DFALSE;
				m_bCanSwimJump		 = DTRUE;
			}	

			// See if we landed (after falling)...
			if (!bFreeMovement && m_bFalling && m_bOnGround)
			{
/*				// See if we should play a landing sound
				if ((m_fMaxFallPos - m_fMinFallPos) > 56.0f && !m_bPowerupActivated[PU_INCORPOREAL])
				{
					char	szSound[MAX_CS_FILENAME_LEN+1];
					if (GetLandingSound(szSound))
					{
						PlayPlayerSound(szSound, 200.0f, 70, DFALSE, SOUNDPRIORITY_LOW);
					}
				}
  */          
				m_startFall = -1.0;
			}
			else if (bFreeMovement)
			{
				m_startFall = -1.0;
			}

			// now update m_bFalling
			m_bFalling = m_bOnGround ? DFALSE : DTRUE;

/*			if (m_bFalling)
			{
				if (m_startFall < 0)
				{
					m_startFall = m_pClientDE->GetTime();
					m_fMinFallPos = m_fMaxFallPos = vMyPos.y;
				}
				else 
				{
					if (vMyPos.y > m_fMaxFallPos)
						m_fMaxFallPos = vMyPos.y;
					if (vMyPos.y < m_fMinFallPos)
						m_fMinFallPos = vMyPos.y;
				}
			}
*/
			// Jumping
			if (m_dwControlFlags & CTRLFLAG_JUMP)
			{
				// If we are in a container that supports free movement, see if we are 
				// moving up or down...
				if (bFreeMovement)
				{
					vAccel.y += fMaxMoveAccel;

//					m_bLastJumpCommand = DFALSE;
				}
				else if (!m_bLastJumpCommand)
				{
					// BEGIN 10/5/98 Kevin additions ////////////////////////////////////

					// Handling jumping out of water...
					if (m_bBodyInLiquid && !bHeadInLiquid)
					{
						if (m_bCanSwimJump)
						{
							m_bSwimmingJump = DTRUE;
							m_bCanSwimJump  = DFALSE;
						}
						// If our head is out of the liquid and we're standing on the
						// ground, let us jump out of the water...
						else if (m_bOnGround)
						{
							m_bSwimmingJump = DTRUE;
						}

						if (m_bSwimmingJump)
						{
							m_bSwimmingOnSurface = DFALSE;
							vMyVel.y += m_fJumpVel * 0.75f;
							m_bLastJumpCommand = DTRUE;
							m_bOnGround = DFALSE;
							bFreeMovement = DFALSE;
						}
					}
					else if (m_bOnGround && !bSteep)
					{
						if (m_bForcedCrouch)
							vMyVel.y += m_fJumpVel * 0.75f;
						else
							vMyVel.y += m_fJumpVel;

						// Play the jump sound
						g_pBloodClientShell->GetVoiceMgr()->PlayEventSound(g_pBloodClientShell->GetCharacter(), VME_JUMP);

						m_bLastJumpCommand = DTRUE;
						m_bOnGround = DFALSE;
					}
				}
			}
			else
				m_bLastJumpCommand = DFALSE;


			// Crouching
			if (m_dwControlFlags & CTRLFLAG_CROUCH)
			{
				if ((bInLiquid || bFreeMovement) && !m_bOnGround)
				{
					vAccel.y -= fMaxMoveAccel;
				}
				else
				{
					m_bLastCrouchCommand = DTRUE;
					m_bForcedCrouch = DTRUE;
				}
			}
			else	// Stand up again
			{
				m_bLastCrouchCommand = DFALSE;
			}
		}
	}

	// Calculate drag for in-air movement, we want air movement consistent
	// with ground movement. fDragCoeff needs to be the same as the object's 
	// friction coefficient.
	
	DVector vAirVel;

	// No air friction in liquid
//	if( !m_bBodyInLiquid )
	if( !m_bBodyInLiquid && !m_bBodyOnLadder && !m_bBodyOnConveyor )
	{
		VEC_COPY(vAirVel, vMyVel);
		// Apply drag to y when going down, but not going up.
		if (!m_bOnGround)
		{
			// Don't add friction if we are jumping up
			if (vAirVel.y >= 0.0f ) 
				vAirVel.y = 0.0f;	
			else
			{
				vAirVel.y = DCLAMP(vAirVel.y/2.0f, -25.0f, 0.0f);
			}
		}

		DFLOAT fVel = VEC_MAGSQR(vAirVel);

	//	if (!bFreeMovement && !m_bOnGround && fVel > 0.01f)
		if (!bFreeMovement && fVel > 0.01f )
		{
			DFLOAT fDragAccel = fDragCoeff;

			VEC_MULSCALAR(vAirVel, vAirVel, -fDragAccel);

			VEC_ADD(vAccel, vAccel, vAirVel);
		}
	}

	if (!m_pClientShell->IsSpectatorMode())
	{
		// ADDED BY ANDY 9-20-98
		if(m_bSlowMode)
		{
			VEC_MULSCALAR(vMyVel, vMyVel, 0.5f);

			if(m_pClientDE->GetTime() > m_fSlowTime)
				m_bSlowMode = DFALSE;
		}

		// Cap horizontal velcity at max to make sure.
		DFLOAT fYVel = vMyVel.y;
		vMyVel.y = 0.0f;

		DFLOAT fMag = VEC_MAG(vMyVel);
		fMaxMoveVel = (DFLOAT)sqrt(fMaxMoveVel * fMaxMoveVel * 2);
		if (fMag > fMaxMoveVel)
		{
			fMag = fMaxMoveVel / fMag;
			VEC_MULSCALAR(vMyVel, vMyVel, fMag);
		}
		vMyVel.y = fYVel;
		m_pPhysicsLT->SetVelocity(m_hObject, &vMyVel);
	}
	else	// m_bSpectatorMode
	{
		if (bLateralMovement)
		{
			VEC_NORM(vAccel);
			VEC_MULSCALAR(vAccel, vAccel, fMaxMoveVel);
		}
		else
		{
			VEC_INIT(vAccel);
		}
		m_pPhysicsLT->SetVelocity(m_hObject, &vAccel);
	}

	if (m_bLastJumpCommand && vMyVel.y > 0 && vAccel.y > 0)
		vAccel.y = 0;

	m_pPhysicsLT->SetAcceleration(m_hObject, &vAccel);

	// Reset these value
	m_bBodyInLiquid				= DFALSE;
	m_bBodyOnLadder				= DFALSE;
	m_bBodyOnConveyor			= DFALSE;

	m_eLastContainerCode = m_pClientShell->GetCurContainerCode();
}


void CMoveMgr::UpdatePushers()
{
	DLink *pCur, *pNext;
	Pusher *pPusher;
	DVector myPos, pushVec, vel;
	float dist, velocity;
	CollisionInfo info;
	ClientIntersectQuery iQuery;
	ClientIntersectInfo iInfo;


	if(!m_hObject || !m_pClientDE || !m_pPhysicsLT)
		return;

	//frameTime = m_pClientDE->GetFrameTime();

	m_pClientDE->GetObjectPos(m_hObject, &myPos);
	for(pCur=m_Pushers.m_pNext; pCur != &m_Pushers; pCur=pNext)
	{
		pNext = pCur->m_pNext;

		pPusher = (Pusher*)pCur->m_pData;

		pPusher->m_Delay -= m_FrameTime;
		if(pPusher->m_Delay <= 0.0f)
		{
			pPusher->m_TimeLeft -= m_FrameTime;
			if(pPusher->m_TimeLeft <= 0.0f)
			{
				// Expired..
				dl_Remove(&pPusher->m_Link);
				delete pPusher;
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
					if(!m_pClientDE->IntersectSegment(&iQuery, &iInfo))
					{
						velocity = 1.0f - (dist / pPusher->m_Radius);
						velocity *= pPusher->m_Strength;

						// If we're in the air, apply less (since there's no friction).
						m_pPhysicsLT->GetStandingOn(m_hObject, &info);					
						if(!info.m_hObject) 
						{
							velocity /= 10.0f;
						}

						pushVec = myPos - pPusher->m_Pos;
						pushVec.Norm(velocity);

						m_pPhysicsLT->GetVelocity(m_hObject, &vel);
						vel += pushVec;
						m_pPhysicsLT->SetVelocity(m_hObject, &vel);
					}
				}
			}
		}
	}
}


void CMoveMgr::UpdatePlayerAnimation()
{
	HOBJECT hClientObj;
	DDWORD modelAnim, curModelAnim, curFlags;
	DVector oldDims, offset;

	
	if(!(hClientObj = m_pClientDE->GetClientObject()))
		return;
	
	// Make sure our solid object is on the same animation.
	modelAnim = m_pClientDE->GetModelAnimation(hClientObj);
	curModelAnim = m_pClientDE->GetModelAnimation(m_hObject);
	if(modelAnim != curModelAnim)
	{
		// Kind of wierd what we do here.. the engine sets the dims automatically when
		// we change animations but it doesn't do collision detection (and we don't want
		// it to) so we may end up clipping into the world so we set it to a small cube
		// and resize the dims with collision detection.
		curFlags = m_pClientDE->GetObjectFlags(m_hObject);
		m_pClientDE->SetObjectFlags(m_hObject, (curFlags|FLAG_GOTHRUWORLD) & ~FLAG_SOLID);

		m_pClientDE->SetModelAnimation(m_hObject, modelAnim);
		
		// Get our wanted dims.
		oldDims = m_WantedDims;
		VEC_SET(m_WantedDims, 1, 1, 1);
		m_pClientDE->Common()->GetModelAnimUserDims(m_hObject, &m_WantedDims, modelAnim);
//		m_WantedDims *= m_DimsScale[MS_NORMAL];

		// Figure out a position offset.
		VEC_INIT(offset);
		if (m_WantedDims.y < oldDims.y)
		{
			offset.y = -(oldDims.y - m_WantedDims.y);
			offset.y += .01f; // Fudge factor
		}

		m_pClientDE->SetObjectFlags(m_hObject, curFlags);
		
		// This makes you small before setting the dims so you don't clip thru stuff.
		ResetDims(&offset);
	}
}


DBOOL CMoveMgr::AreDimsCorrect()
{
	DVector curDims;

	if(!m_hObject || !m_pPhysicsLT)
		return DTRUE;

	m_pPhysicsLT->GetObjectDims(m_hObject, &curDims);
	return fabs(curDims.x-m_WantedDims.x) < 0.1f && fabs(curDims.y-m_WantedDims.y) < 0.1f &&
		fabs(curDims.z-m_WantedDims.z) < 0.1f;
}


void CMoveMgr::ResetDims(DVector *pOffset)
{
	DVector smallDims, pos;

	smallDims.Init(0.5f, 0.5f, 0.5f);
	m_pPhysicsLT->SetObjectDims(m_hObject, &smallDims, 0);
	
	// Move them if they want.
	if(pOffset)
	{
		m_pClientDE->GetObjectPos(m_hObject, &pos);
		pos += *pOffset;
		m_pPhysicsLT->MoveObject(m_hObject, &pos, 0);
	}
	
	m_pPhysicsLT->SetObjectDims(m_hObject, &m_WantedDims, SETDIMS_PUSHOBJECTS);
}


void CMoveMgr::MoveLocalSolidObject()
{
	MoveInfo info;
	DVector newPos, curPos, vVel;

	info.m_hObject = m_hObject;
	info.m_dt = m_FrameTime;
	m_pPhysicsLT->UpdateMovement(&info);

	if(info.m_Offset.MagSqr() > 0.01f)
	{
		m_pClientDE->GetObjectPos(m_hObject, &curPos);
		newPos = curPos + info.m_Offset;

		// Get the velocity before moving the object, because the moveobject will zero the velocity if 
		// we hit anything.
		m_pPhysicsLT->GetVelocity( m_hObject, &m_vServerVelocity );
		m_pPhysicsLT->MoveObject(m_hObject, &newPos, 0);
	}
}


void CMoveMgr::UpdateVelMagnitude()
{
	DVector vel;

	if(!m_pPhysicsLT || !m_pClientShell || !m_hObject)
		return;

	m_pPhysicsLT->GetVelocity(m_hObject, &vel);
	vel.y = 0;
	m_pClientShell->SetVelMagnitude(vel.Mag());
}


void CMoveMgr::SetClientObjNonsolid()
{
	HOBJECT hObj;

	if(hObj = m_pClientDE->GetClientObject())
	{
		DDWORD dwFlags = m_pClientDE->GetObjectFlags(hObj);
		m_pClientDE->SetObjectFlags(hObj, dwFlags | FLAG_CLIENTNONSOLID);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMoveMgr::UpdateFriction
//
//	PURPOSE:	Update player fricton
//
// ----------------------------------------------------------------------- //

void CMoveMgr::UpdateFriction()
{

	if (m_bOnGround && (m_pClientShell->GetCurContainerCode() == CC_NOTHING))
	{
		DVector vCurVel;
		m_pPhysicsLT->GetVelocity(m_hObject, &vCurVel);

		DFLOAT fYVal = vCurVel.y;
		vCurVel.y = 0.0f;

		if (VEC_MAG(vCurVel) > 0.1f)
		{
			DVector vDir, vVel;
			VEC_COPY(vDir, vCurVel);
			VEC_NORM(vDir);
/*
			DFLOAT fExp = (DFLOAT)exp(-fDragCoeff*m_pClientDE->GetFrameTime());
			DVector vTemp, vTemp2;
			m_pPhysicsLT->GetAcceleration(m_hObject, &vTemp);
			vTemp /= fDragCoeff;
			vTemp2 = vCurVel - vTemp;
			vTemp2 *= fExp;
			vVel = vTemp2 + vTemp;
*/
/*			DFLOAT fAdjust = m_pClientDE->GetFrameTime()*(m_fMoveVel/SLIDE_TO_STOP_TIME);

			VEC_MULSCALAR(vVel, vDir, fAdjust);

			if (VEC_MAG(vVel) < VEC_MAG(vCurVel))
			{
				VEC_SUB(vVel, vCurVel, vVel);
			}
			else
			{
				VEC_INIT(vVel);
			}

			vVel.y = fYVal;
			m_pPhysicsLT->SetVelocity(m_hObject, &vVel);
*/
		}
	}
}


void CMoveMgr::ShowPos(char *pBlah)
{
	DVector pos;
	m_pClientDE->GetObjectPos(m_hObject, &pos);
	m_pClientDE->CPrint("%s: %.1f %.1f %.1f", pBlah, VEC_EXPAND(pos));
}

void CMoveMgr::Update()
{
	HOBJECT hObj;

	hObj = m_pClientDE->GetClientObject();
	if(!m_hObject || !hObj)
		return;
			   
	// m_pClientDE->CPrint("On Ground: %s", m_bOnGround ? "TRUE" : "FALSE");

	m_FrameTime = m_pClientDE->GetFrameTime();
	if(m_FrameTime > 0.1f)
		m_FrameTime = 0.1f;

	// We don't want to hit the real client object.
	SetClientObjNonsolid();

	UpdatePlayerAnimation();

	UpdateControlFlags();

	if(!m_pClientShell->IsTrapped())
	{
		UpdateMotion();
		// Update friction...
//		UpdateFriction();
		UpdatePushers();
	}

	// Make sure we have desired dims.
	if(!AreDimsCorrect())
		ResetDims();

	if(!m_pClientShell->IsTrapped())
	{
		MoveLocalSolidObject();
		UpdateVelMagnitude();
	}
	else
	{
		DVector vTemp;
		VEC_INIT(vTemp);
		m_pPhysicsLT->SetVelocity(m_hObject, &vTemp);
	}

	// Clear these and they'll get set if mouse is moving.
	m_nMouseStrafeFlags = 0;
}


void CMoveMgr::OnPhysicsUpdate(HMESSAGEREAD hRead)
{
	ObjectCreateStruct theStruct;
	D_WORD changeFlags;
	DVector grav;
	HOBJECT hClientObj;
	DDWORD i; 

	if(!m_pClientDE)
		return;

	changeFlags = m_pClientDE->ReadFromMessageWord(hRead);

	// Change our model file?
	if (changeFlags & PSTATE_MODELFILENAMES)
	{
		INIT_OBJECTCREATESTRUCT(theStruct);

		hClientObj = m_pClientDE->GetClientObject();

		if(m_hObject)
		{
			m_pClientDE->GetObjectPos( m_hObject, &theStruct.m_Pos );
			m_pClientDE->DeleteObject(m_hObject);
		}
		else if( hClientObj )
		{
			m_pClientDE->GetObjectPos(hClientObj, &theStruct.m_Pos);
		}

		SAFE_STRCPY(theStruct.m_Filename, m_pClientDE->ReadFromMessageString(hRead));
		SAFE_STRCPY(theStruct.m_SkinName, m_pClientDE->ReadFromMessageString(hRead));
		theStruct.m_ObjectType = OT_MODEL;
		theStruct.m_Flags = FLAG_SOLID | FLAG_GRAVITY | FLAG_STAIRSTEP;
		
		m_hObject = m_pClientDE->CreateObject(&theStruct);
		if(m_hObject)
		{
			m_pClientDE->SetObjectClientFlags(m_hObject, CF_DONTSETDIMS|CF_NOTIFYREMOVE);
			m_pPhysicsLT->SetFrictionCoefficient( m_hObject, m_fFrictionCoeff );
		}

		// Reset dims..
		m_pClientDE->Common()->GetModelAnimUserDims(m_hObject, &m_WantedDims, 0);
		
		// This makes you small before setting the dims so you don't clip thru stuff.
		ResetDims();
	}

	if (changeFlags & PSTATE_ADDVELOCITY)
	{
		DVector vAddVel, vVel;
		m_pClientDE->ReadFromMessageVector(hRead, &vAddVel);
		m_pPhysicsLT->GetVelocity(m_hObject, &vVel);
		vVel = vVel + vAddVel;
		m_pPhysicsLT->SetVelocity(m_hObject, &vVel);
	}

	if (changeFlags & PSTATE_GRAVITY)
	{
		m_pClientDE->ReadFromMessageVector(hRead, &grav);
		m_pPhysicsLT->SetGlobalForce(grav);
	}

	if (changeFlags & PSTATE_CONTAINERTYPE)
	{
		m_nContainers = m_pClientDE->ReadFromMessageByte(hRead);
		if(m_nContainers >= MAX_TRACKED_CONTAINERS)
			return;

		for(i=0; i < m_nContainers; i++)
		{
			m_Containers[i].m_ContainerCode = (ContainerCode)m_pClientDE->ReadFromMessageByte(hRead);

			m_pClientDE->ReadFromMessageVector(hRead, &m_Containers[i].m_Current);
			m_Containers[i].m_fGravity = m_pClientDE->ReadFromMessageFloat(hRead);
			m_Containers[i].m_bHidden = m_pClientDE->ReadFromMessageByte(hRead);
		}
	}

	if (changeFlags & PSTATE_SPEEDS)
	{
		m_fMoveVel = m_pClientDE->ReadFromMessageFloat(hRead);
		m_fJumpVel = m_pClientDE->ReadFromMessageFloat(hRead);
		m_fSwimVel = m_pClientDE->ReadFromMessageFloat(hRead);

		m_fMoveMultiplier = m_pClientDE->ReadFromMessageFloat(hRead);
		m_fMoveAccelMultiplier = m_pClientDE->ReadFromMessageFloat(hRead);

		m_fLeashLen = m_pClientDE->ReadFromMessageFloat(hRead);
		m_fBaseMoveAccel = m_pClientDE->ReadFromMessageFloat(hRead);
		m_fJumpMultiplier = m_pClientDE->ReadFromMessageFloat(hRead);
		m_fLadderVel = m_pClientDE->ReadFromMessageFloat(hRead);
		
		m_fFrictionCoeff = m_pClientDE->ReadFromMessageFloat(hRead);
		m_pPhysicsLT->SetFrictionCoefficient(m_hObject, m_fFrictionCoeff);
	}

	// Update crouch state
	if (changeFlags & PSTATE_CROUCH)
	{
		m_bForcedCrouch = m_pClientDE->ReadFromMessageByte(hRead);
	}
}


DRESULT CMoveMgr::OnObjectMove(HOBJECT hObj, DBOOL bTeleport, DVector *pPos)
{
	HOBJECT hClientObj;
	DDWORD type;

	if(!m_hObject || !m_pClientDE)
		return LT_OK;

	hClientObj = m_pClientDE->GetClientObject();

	// If it's a solid world model moving, do a regular MoveObject on it so it
	// can carry/push the player object around.
	if(!bTeleport && hObj != hClientObj && hObj != m_hObject)
	{
		type = m_pClientDE->GetObjectType(hObj);
		if(type == OT_WORLDMODEL)
		{
			if(m_pClientDE->GetObjectFlags(hObj) & FLAG_SOLID)
			{
				m_pPhysicsLT->MovePushObjects(hObj, *pPos, &m_hObject, 1);
			}
		}	
	}

	return LT_OK;
}


DRESULT CMoveMgr::OnObjectRotate(HOBJECT hObj, DBOOL bTeleport, DRotation *pNewRot)
{
	HOBJECT hClientObj;
	DDWORD type;

	if(!m_hObject || !m_pClientDE)
		return LT_OK;

	hClientObj = m_pClientDE->GetClientObject();

	// If it's a solid world model moving, do a regular MoveObject on it so it
	// can carry/push the player object around.
	if(!bTeleport && hObj != hClientObj && hObj != m_hObject)
	{
		type = m_pClientDE->GetObjectType(hObj);
		if(type == OT_WORLDMODEL)
		{
			if(m_pClientDE->GetObjectFlags(hObj) & FLAG_SOLID)
			{
				m_pPhysicsLT->RotatePushObjects(hObj, *pNewRot, &m_hObject, 1);
			}
		}	
	}

	return LT_OK;
}


void CMoveMgr::OnObjectRemove(HOBJECT hObj)
{
	if(hObj == m_hObject)
	{
		m_hObject = DNULL;
	}
}


DRESULT CMoveMgr::AddPusher(DVector &pos, float radius, float startDelay, float duration, float strength)
{
	Pusher *pPusher;

	if(!(pPusher = new Pusher))
		return LT_ERROR;
	
	pPusher->m_Pos = pos;
	pPusher->m_Radius = radius;
	pPusher->m_Delay = startDelay;
	pPusher->m_TimeLeft = duration;
	pPusher->m_Strength = strength;
	dl_Insert(&m_Pushers, &pPusher->m_Link);
	
	return LT_OK;
}


void CMoveMgr::MoveToClientObj()
{
	DVector cObjPos;
	HOBJECT hObj;

	if(!m_hObject || !(hObj = m_pClientDE->GetClientObject()))
		return;

	m_pClientDE->GetObjectPos(hObj, &cObjPos);
	m_pPhysicsLT->MoveObject(m_hObject, &cObjPos, MOVEOBJECT_TELEPORT);
}


void CMoveMgr::OnServerForcePos(HMESSAGEREAD hRead)
{
	DVector zeroVec, tempDims, pos, curDims;

	if(!m_pClientDE || !m_pPhysicsLT)
		return;

	m_ClientMoveCode = m_pClientDE->ReadFromMessageByte(hRead);

	// Teleport to where it says.
	if(m_hObject)
	{
		SetClientObjNonsolid();

		// Move there.  We make our object a point first and then resize the dims so 
		// we don't teleport clipping into the world.
		m_pClientDE->ReadFromMessageVector(hRead, &pos);
		
		tempDims.Init(0.5f, 0.5f, 0.5f);
		m_pPhysicsLT->GetObjectDims(m_hObject, &curDims);
		m_pPhysicsLT->SetObjectDims(m_hObject, &tempDims, 0);
		m_pPhysicsLT->MoveObject(m_hObject, &pos, MOVEOBJECT_TELEPORT);
		m_pPhysicsLT->SetObjectDims(m_hObject, &curDims, SETDIMS_PUSHOBJECTS);

		// Clear our velocity and acceleration.
		zeroVec.Init();
		m_pPhysicsLT->SetVelocity(m_hObject, &zeroVec);
		m_pPhysicsLT->SetAcceleration(m_hObject, &zeroVec);
	}
}


void CMoveMgr::WritePositionInfo(HMESSAGEWRITE hWrite)
{
	DVector myPos, myVel;
	short compVel[3];
	DBYTE nMovementFlags;

	if(!m_pClientDE)
		return;

	if(m_hObject)
	{
		m_pClientDE->GetObjectPos(m_hObject, &myPos);
//		m_pPhysicsLT->GetVelocity(m_hObject, &myVel);
		VEC_COPY( myVel, m_vServerVelocity );
	}
	else
	{
		myPos.Init();
		myVel.Init();
	}
	
	m_pClientDE->WriteToMessageByte(hWrite, m_ClientMoveCode);
	m_pClientDE->WriteToMessageVector(hWrite, &myPos);
	
	compVel[0] = (short)myVel.x;
	compVel[1] = (short)myVel.y;
	compVel[2] = (short)myVel.z;
	m_pClientDE->WriteToMessageWord(hWrite, compVel[0]);
	m_pClientDE->WriteToMessageWord(hWrite, compVel[1]);
	m_pClientDE->WriteToMessageWord(hWrite, compVel[2]);

	nMovementFlags = 0;
	if( m_bOnGround )
		nMovementFlags |= CLIENTMOVEMENTFLAG_ONGROUND;
	m_pClientDE->WriteToMessageByte(hWrite, (DBYTE)nMovementFlags);
	m_pClientDE->WriteToMessageByte(hWrite, (DBYTE)m_eLastSurface);
}


