
#ifndef __CLIENT_PHYSICS_H__
#define __CLIENT_PHYSICS_H__

#include "SharedDefs.h"

//SCHLEGZ: this is a straight rip from Shogo....if you don't like it, tough.

#define MO_RESTING		(1<<1)	// Object is in a resting state.
#define MO_LIQUID		(1<<2)	// Object in water
#define MO_NOGRAVITY	(1<<3)	// Ignore gravity
#define MO_HALFGRAVITY	(1<<4)	// Use 1/2 gravity value
#define MO_STICKY		(1<<5)	// Sticky object, don't bounce
#define MO_SLIDE		(1<<6)	// Slide down a wall or slope


	typedef struct MovingObject_t
	{
		// Internal stuff.
		DDWORD	m_PhysicsFlags; // MO_ flags above.
		DVector m_Pos;
		DVector m_LastPos;
		DVector m_Velocity;
		DVector m_Acceleration;
	} MovingObject;


	typedef struct PhysicsState_t
	{
		ClientDE *m_pClientDE;
		float m_TimeStep;			// Time elapsed.  Only set with SetPhysicsStateTimeStep.
		float m_TimeStepIntegral;	// Used internally.
		DVector m_GravityAccel;		// A reasonable value is (0, -1000, 0).
		float m_VelocityDampen;		// Velocity dampen when it hits something.
	} PhysicsState;						

	
	void InitMovingObject(MovingObject *pObject, DVector *pPos, DVector *pVelocity);

	void SetPhysicsStateTimeStep(PhysicsState *pState, float timeStep);

	// Updates velocity and acceleration.  Fills in pNewPos with the new position
	// vector.  If pNewPos is NULL, it'll fill in pObject->m_Pos.  Returns DFALSE if
	// the object shouldn't move.
	DBOOL UpdateMovingObject(PhysicsState *pState, MovingObject *pObject, DVector *pNewPos);

	// Bounce the moving object off the world.  If the object hits anything, it reflects
	// the velocity vector, changes pNewPos, and scales the velocity by pState->m_VelocityDampen
	// (if you want it to stop when it hits something, set it to 0).
	DBOOL BounceMovingObject(PhysicsState *pState, MovingObject *pObject, DVector *pNewPos, ClientIntersectInfo *pInfo, SurfaceType *eType=DNULL );

	// Based on the current position of the object, determine the physics state
	// we should to use
	PhysicsState* GetCurPhysicsState(MovingObject* pObject);

#endif  // __CLIENT_PHYSICS_H__





