// ----------------------------------------------------------------------- //
//
// MODULE  : client_physics.h
//
// PURPOSE : Client-side, intersect-segment based collision - Declaration
//
// CREATED : 1998
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_PHYSICS_H__
#define __CLIENT_PHYSICS_H__

#define MO_RESTING			(1<<1)	// Object is in a resting state.
#define MO_LIQUID			(1<<2)	// Object in water
#define MO_NOGRAVITY		(1<<3)	// Ignore gravity
#define MO_PLAYERPHYSICS	(1<<4)	// Use player physics

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
    uint32   m_dwPhysicsFlags;   // MO_ flags above.
    LTVector m_vPos;
    LTVector m_vLastPos;
    LTVector m_vVelocity;
    LTVector m_vAcceleration;
	float	 m_fGravityScale;	// Amount to scale gravity by (optional)
} MovingObject;


struct PhysicsState
{
	PhysicsState()
	{
		m_fTimeStep			= 0.0f;
		m_fVelocityDampen	= 0.0f;
		m_vGravityAccel.Init();
	}

	float		m_fTimeStep;			// Time elapsed.  Only set with SetPhysicsStateTimeStep.
	LTVector	m_vGravityAccel;        // A reasonable value is (0, -1000, 0).
	float		m_fVelocityDampen;		// Velocity dampen when it hits something.
};

void InitMovingObject(MovingObject *pObject, const LTVector &vPos, const LTVector &vVelocity);
void SetPhysicsStateTimeStep(PhysicsState *pState, float timeStep);

// Updates velocity and acceleration.  Fills in vNewPos with the new position
// vector.  Returns false if the object shouldn't move.

bool UpdateMovingObject(PhysicsState *pState, MovingObject *pObject, LTVector &vNewPos);

// Based on the current position of the object, determine the physics state
// we should to use

PhysicsState* GetCurPhysicsState(MovingObject* pObject);

#endif  // __CLIENT_PHYSICS_H__



