// ----------------------------------------------------------------------- //
//
// MODULE  : client_physics.h
//
// PURPOSE : Client-side, intersect-segment based collision - Declaration
//
// CREATED : 1998
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_PHYSICS_H__
#define __CLIENT_PHYSICS_H__

#define MO_RESTING		(1<<1)	// Object is in a resting state.
#define MO_LIQUID		(1<<2)	// Object in water
#define MO_NOGRAVITY	(1<<3)	// Ignore gravity

typedef struct MovingObject_t
{
    MovingObject_t()
	{
		Init();
	}

    void Init()
	{
		m_dwPhysicsFlags = 0;
		m_vPos.Init();
		m_vLastPos.Init();
		m_vVelocity.Init();
		m_vAcceleration.Init();
		m_fGravityScale	= 1.0f;
	}

	// Internal stuff.
    uint32  m_dwPhysicsFlags;   // MO_ flags above.
    LTVector m_vPos;
    LTVector m_vLastPos;
    LTVector m_vVelocity;
    LTVector m_vAcceleration;
	float	m_fGravityScale;	// Amount to scale gravity by (optional)
} MovingObject;


typedef struct PhysicsState_t
{
    PhysicsState_t()
	{
		m_fTimeStep			= 0.0f;
		m_fTimeStepIntegral	= 0.0f;
		m_fVelocityDampen	= 0.0f;
		m_vGravityAccel.Init();
	}

	float	m_fTimeStep;			// Time elapsed.  Only set with SetPhysicsStateTimeStep.
	float	m_fTimeStepIntegral;	// Used internally.
    LTVector m_vGravityAccel;        // A reasonable value is (0, -1000, 0).
	float	m_fVelocityDampen;		// Velocity dampen when it hits something.
} PhysicsState;


void InitMovingObject(MovingObject *pObject, LTVector *pPos, LTVector *pVelocity);
void SetPhysicsStateTimeStep(PhysicsState *pState, float timeStep);

// Updates velocity and acceleration.  Fills in pNewPos with the new position
// vector.  If pNewPos is NULL, it'll fill in pObject->m_Pos.  Returns LTFALSE if
// the object shouldn't move.

LTBOOL UpdateMovingObject(PhysicsState *pState, MovingObject *pObject, LTVector *pNewPos);

// Bounce the moving object off the world.  If the object hits anything, it reflects
// the velocity vector, changes pNewPos, and scales the velocity by pState->m_VelocityDampen
// (if you want it to stop when it hits something, set it to 0).

LTBOOL BounceMovingObject(PhysicsState *pState, MovingObject *pObject,
    LTVector *pNewPos, ClientIntersectInfo* pInfo, uint32 dwQueryFlags,
	LTBOOL & bBounceOnGround);

// Based on the current position of the object, determine the physics state
// we should to use

PhysicsState* GetCurPhysicsState(MovingObject* pObject);

#endif  // __CLIENT_PHYSICS_H__



