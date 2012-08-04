
#ifndef __CLIENT_PHYSICS_H__
#define __CLIENT_PHYSICS_H__

#define MO_RESTING		(1<<1)	// Object is in a resting state.
#define MO_LIQUID		(1<<2)	// Object in water
#define MO_NOGRAVITY	(1<<3)	// Ignore gravity
#define MO_HALFGRAVITY	(1<<4)	// Use 1/2 gravity value

	
	class ILTClient;
	

	typedef struct MovingObject_t
	{
		// Internal stuff.
		uint32	m_PhysicsFlags; // MO_ flags above.
		LTVector m_Pos;
		LTVector m_LastPos;
		LTVector m_Velocity;
		LTVector m_Acceleration;
	} MovingObject;


	typedef struct PhysicsState_t
	{
		ILTClient *m_pClientDE;
		LTFLOAT m_TimeStep;			// Time elapsed.  Only set with SetPhysicsStateTimeStep.
		LTFLOAT m_TimeStepIntegral;	// Used internally.
		LTVector m_GravityAccel;		// A reasonable value is (0, -1000, 0).
		LTFLOAT m_VelocityDampen;		// Velocity dampen when it hits something.
	} PhysicsState;						

	
	void InitMovingObject(MovingObject *pObject, LTVector *pPos, LTVector *pVelocity);

	void SetPhysicsStateTimeStep(PhysicsState *pState, LTFLOAT timeStep);

	// Updates velocity and acceleration.  Fills in pNewPos with the new position
	// vector.  If pNewPos is NULL, it'll fill in pObject->m_Pos.  Returns LTFALSE if
	// the object shouldn't move.
	LTBOOL UpdateMovingObject(PhysicsState *pState, MovingObject *pObject, LTVector *pNewPos);

	// Bounce the moving object off the world.  If the object hits anything, it reflects
	// the velocity vector, changes pNewPos, and scales the velocity by pState->m_VelocityDampen
	// (if you want it to stop when it hits something, set it to 0).
	LTBOOL BounceMovingObject(PhysicsState *pState, MovingObject *pObject, LTVector *pNewPos, ClientIntersectInfo* pInfo);

	// Based on the current position of the object, determine the physics state
	// we should to use
	PhysicsState* GetCurPhysicsState(MovingObject* pObject);

#endif  // __CLIENT_PHYSICS_H__





