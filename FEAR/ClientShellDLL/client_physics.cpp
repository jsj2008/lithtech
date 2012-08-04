// ----------------------------------------------------------------------- //
//
// MODULE  : client_physics.cpp
//
// PURPOSE : Client-side, intersect-segment based collision - Implementation
//
// CREATED : 1998
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "clientheaders.h"
#include "client_physics.h"
#include "ClientServerShared.h"
#include "ContainerCodes.h"
#include "GameClientShell.h"

extern PhysicsState	  g_normalPhysicsState;
extern PhysicsState	  g_waterPhysicsState;
extern PhysicsState	  g_playerNormalPhysicsState;
extern PhysicsState	  g_playerWaterPhysicsState;

static bool CP_NonSolidFilterFn(HOBJECT hTest, void *pUserData)
{
	// Ignore non-solid objects (even if ray-hit is true)...

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hTest, OFT_Flags, dwFlags);

	if (!(dwFlags & FLAG_SOLID))
	{
		return false;
	}

    return true;
}

static bool CP_NonSolidNonMovingFilterFn(HOBJECT hTest, void *pUserData)
{
	// Ignore non-solid objects (even if ray-hit is true)...
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hTest, OFT_Flags, dwFlags);

	if (!(dwFlags & FLAG_SOLID))
	{
		return false;
	}

	//alright, now determine if this is a world model
	uint32 nType;
	if(g_pCommonLT->GetObjectType(hTest, &nType) != LT_OK)
		return false;

	if(nType != OT_WORLDMODEL)
		return false;

	//alright, we have a world model, now only obey the ones that aren't moveable
	uint32 nUserFlags;
	if(g_pCommonLT->GetObjectFlags(hTest, OFT_User, nUserFlags) != LT_OK)
		return false;

	//ignore moveable world models
	if(nUserFlags & USRFLG_MOVEABLE)
		return false;

    return true;
}

void InitMovingObject(MovingObject *pObject, const LTVector &vPos, const LTVector &vVelocity)
{
	if (!pObject) return;

	pObject->Init();
	pObject->m_vPos			= vPos;
	pObject->m_vLastPos		= vPos;
	pObject->m_vVelocity	= vVelocity;

	// Determine if we are in any containers (like liquid) that would
	// affect physics...

	HLOCALOBJ objList[1];
    uint32 dwNum = ::GetPointContainers(pObject->m_vPos, objList, 1, ::GetLiquidFlags());

	if (dwNum > 0)
	{
		pObject->m_dwPhysicsFlags |= MO_LIQUID;
	}
}


void SetPhysicsStateTimeStep(PhysicsState *pState, float timeStep)
{
	if (pState)
		pState->m_fTimeStep = timeStep;
}


bool UpdateMovingObject(PhysicsState *pUserState, MovingObject *pObject, LTVector &vNewPos)
{
    if (!pObject) 
		return false;

	PhysicsState* pState = pUserState ? pUserState : GetCurPhysicsState(pObject);
    if (!pState) 
		return false;

    if (pObject->m_dwPhysicsFlags & MO_RESTING) 
		return false;

	// Prevent tiny movements.
	if (pObject->m_vAcceleration.MagSqr() < 0.01f)
	{
		pObject->m_vAcceleration.Init();
	}

	if (pObject->m_vVelocity.MagSqr() < 0.01f)
	{
		pObject->m_vVelocity.Init();
	}

	// AccelerationDelta = Sum of forces acting upon object
	LTVector vAccelerationDelta = pObject->m_vAcceleration;
	if (!(pObject->m_dwPhysicsFlags & MO_NOGRAVITY))
	{
		//need to determine the gravitational force on this object
		vAccelerationDelta += pState->m_vGravityAccel * pObject->m_fGravityScale;
	}

	// VelocityDelta = Acceleration * dt;
	LTVector vVelocityDelta = vAccelerationDelta * pState->m_fTimeStep;

	// PositionDelta = Velocity * dt + 0.5 * Acceleration * dt * dt
	LTVector vPosDelta =	(pObject->m_vVelocity * pState->m_fTimeStep) + 
							(0.5f * pObject->m_vAcceleration * pState->m_fTimeStep * pState->m_fTimeStep);
	
	//Accumulate our deltas into our values, and clear out our acceleration for the next
	//time step so that the same forces aren't continually applied
	pObject->m_vAcceleration.Init();
	pObject->m_vVelocity += vVelocityDelta;
	vNewPos = pObject->m_vPos + vPosDelta;

    return true;
}


// Based on the current position of the object, determine the physics state
// we should use...

PhysicsState* GetCurPhysicsState(MovingObject *pObject)
{
    if (!pObject) return NULL;

	// See which physics state to use...
	if (pObject->m_dwPhysicsFlags & MO_PLAYERPHYSICS)
	{
		return ((pObject->m_dwPhysicsFlags & MO_LIQUID) ? &g_playerWaterPhysicsState : &g_playerNormalPhysicsState);
	}

	return ((pObject->m_dwPhysicsFlags & MO_LIQUID) ? &g_waterPhysicsState : &g_normalPhysicsState);
}



