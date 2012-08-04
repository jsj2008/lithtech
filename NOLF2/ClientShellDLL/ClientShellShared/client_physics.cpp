// ----------------------------------------------------------------------- //
//
// MODULE  : client_physics.cpp
//
// PURPOSE : Client-side, intersect-segment based collision - Implementation
//
// CREATED : 1998
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
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
	pState->m_fTimeStep = timeStep;
	pState->m_fTimeStepIntegral = timeStep*timeStep*0.5f;
}


LTBOOL UpdateMovingObject(PhysicsState *pUserState, MovingObject *pObject, LTVector &vNewPos)
{
    if (!pObject) return LTFALSE;

	PhysicsState* pState = pUserState ? pUserState : GetCurPhysicsState(pObject);
    if (!pState) return LTFALSE;

    LTVector vTemp, velocityDelta, posDelta;

    if (pObject->m_dwPhysicsFlags & MO_RESTING) return LTFALSE;

	// Prevent tiny movements.
	if (VEC_MAGSQR(pObject->m_vAcceleration) < 0.01f)
	{
		pObject->m_vAcceleration.Init();
	}

	if (VEC_MAGSQR(pObject->m_vVelocity) < 0.01f)
	{
		pObject->m_vVelocity.Init();
	}

	// velocityDelta = ( acceleration + accelDelta * 0.5 ) * dt;
	vTemp.Init();
	if (!(pObject->m_dwPhysicsFlags & MO_NOGRAVITY))
	{
        LTFLOAT fScale = 0.5f * pObject->m_fGravityScale;
		VEC_MULSCALAR(vTemp, pState->m_vGravityAccel, fScale);
	}
	VEC_ADD(vTemp, vTemp, pObject->m_vAcceleration);
	VEC_MULSCALAR(velocityDelta, vTemp, pState->m_fTimeStep);

	// Apply the velocity to the position (p = p + vt + 0.5a(t^2)).
	VEC_MULSCALAR(posDelta, pObject->m_vAcceleration, pState->m_fTimeStepIntegral);
	VEC_ADDSCALED(posDelta, posDelta, pObject->m_vVelocity, pState->m_fTimeStep);

	// Add the final velocity to the new velocity.
	VEC_ADD(pObject->m_vVelocity, pObject->m_vVelocity, velocityDelta);

	VEC_ADD(vNewPos, pObject->m_vPos, posDelta);

	// Zero out the acceleration.
	pObject->m_vAcceleration.Init();

    return LTTRUE;
}


LTBOOL BounceMovingObject(PhysicsState *pUserState, MovingObject *pObject,
                         LTVector &vNewPos, ClientIntersectInfo* pInfo,
                         uint32 dwQueryFlags, bool bIgnoreMovingObjects,
						 LTBOOL & bBounceOnGround)
{
    if (!pObject || !pInfo) return LTFALSE;

	bBounceOnGround = LTFALSE;

	PhysicsState* pState = pUserState ? pUserState : GetCurPhysicsState(pObject);
    if (!pState) return LTFALSE;

	IntersectQuery query;
	float dot;

	// Only do an intersection test if the line is long enough (sometimes the
	// intersection test will fail on really short lines).

	query.m_From		= pObject->m_vPos;
	query.m_To			= vNewPos;
	query.m_Flags		= dwQueryFlags;
	query.m_FilterFn	= (bIgnoreMovingObjects) ? CP_NonSolidNonMovingFilterFn : CP_NonSolidFilterFn;

    if (g_pLTClient->IntersectSegment(&query, pInfo))
	{
		// Move the dest point a little in front of the plane.
		VEC_ADDSCALED(vNewPos, pInfo->m_Point, pInfo->m_Plane.m_Normal, 0.3f);

		// Reflect the velocity.
		dot = VEC_DOT(pObject->m_vVelocity, pInfo->m_Plane.m_Normal);
		dot *= -2.0f;

		VEC_ADDSCALED(pObject->m_vVelocity, pObject->m_vVelocity, pInfo->m_Plane.m_Normal, dot);

		// Dampen it.
		VEC_MULSCALAR(pObject->m_vVelocity, pObject->m_vVelocity, pState->m_fVelocityDampen);

		const float c_fMinPixels = 15.0f;
        LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();

		// This assumes we're updating at 1.0/fDelta fps and our minimum velocity
		// is c_fMinPixels pixels per second
		// (c_fMinPixels*c_fMinPixels/fDelta is the maximum magnitude).

		fDelta = fDelta > 0.5f ? 0.1f : (fDelta < 0.001f ? 0.001f : fDelta);

        LTFLOAT fMaxMag = ((c_fMinPixels*c_fMinPixels)/fDelta);

		bBounceOnGround = (pInfo->m_Plane.m_Normal.y > 0.6f);

		if (bBounceOnGround && (VEC_MAGSQR(pObject->m_vVelocity) < fMaxMag))
		{
			pObject->m_dwPhysicsFlags |= MO_RESTING;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

// Based on the current position of the object, determine the physics state
// we should use...

PhysicsState* GetCurPhysicsState(MovingObject *pObject)
{
    if (!pObject) return LTNULL;

	return ((pObject->m_dwPhysicsFlags & MO_LIQUID) ? &g_waterPhysicsState : &g_normalPhysicsState);
}



