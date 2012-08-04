
// This interface implements physics functionality.

#ifndef __PHYSICS_LT_H__
#define __PHYSICS_LT_H__


	// Sets the dims as large as it can by testing to see what's around the object.
	// The object will get touch notifications if it hits anything while resizing.
	#define SETDIMS_PUSHOBJECTS	(1<<0)

	
	// Teleport the object to its destination.
	#define MOVEOBJECT_TELEPORT	(1<<0)


	class MoveInfo
	{
	public:
		HOBJECT	m_hObject;
		float	m_dt;
		DVector	m_Offset;
	};


	class PhysicsLT
	{
	public:
		
		// Get/Set friction coefficient.
		virtual DRESULT GetFrictionCoefficient(HOBJECT hObj, float &coeff);
		virtual DRESULT SetFrictionCoefficient(HOBJECT hObj, float coeff);

		// Get/set force ignore limit.
		virtual DRESULT GetForceIgnoreLimit(HOBJECT hObj, float &limit);
		virtual DRESULT SetForceIgnoreLimit(HOBJECT hObj, float limit);
		
		// Get/Set acceleration and velocity
		virtual DRESULT	GetVelocity(HOBJECT hObj, DVector *pVel);
		virtual DRESULT	SetVelocity(HOBJECT hObj, DVector *pVel)=0;
		
		virtual DRESULT	GetAcceleration(HOBJECT hObj, DVector *pAccel);
		virtual DRESULT	SetAcceleration(HOBJECT hObj, DVector *pAccel)=0;

		// Get/Set an object's mass (default is 30).
		virtual DRESULT GetObjectMass(HOBJECT hObj, float &mass);
		virtual DRESULT SetObjectMass(HOBJECT hObj, float mass);

		// Get the object's current dimensions.
		virtual DRESULT GetObjectDims(HOBJECT hObj, DVector *pNewDims);
		
		// Changes the object's dimensions without pushing against objects and world.
		// Flags is a combination of SETDIMS_ flags above.
		virtual DRESULT	SetObjectDims(HOBJECT hObj, DVector *pNewDims, DDWORD flags)=0;

		// This function moves an object, colliding/pushing/crushing objects
		// in its way (if it's solid..)  Flags is a combination of MOVEOBJECT_ flags.
		virtual DRESULT MoveObject(HOBJECT hObj, DVector *pPos, DDWORD flags)=0;

		// Find out what the object is standing on.
		virtual DRESULT GetStandingOn(HOBJECT hObj, CollisionInfo *pInfo);

		// Get the world object.
		virtual DRESULT GetWorldObject(HOBJECT *hObj)=0;

		// Get/set global force.  This is an acceleration applied to all objects
		// when they move.  Default is (0,-2000,0) which simulates gravity.
		virtual DRESULT GetGlobalForce(DVector &vec)=0;
		virtual DRESULT SetGlobalForce(DVector &vec)=0;

	protected:
		ClientServerType	m_ClientServerType; // Tells if this is on the client or server.
	};


	// Client-specific physics stuff.
	class CPhysicsLT : public PhysicsLT
	{
	public:
		// Updates the object's movement using its velocity, acceleration, and the
		// time delta passed in (usually the frame time).  Fills in m_Offset with the
		// position delta you should apply.
		virtual DRESULT UpdateMovement(MoveInfo *pInfo)=0;

		// Move the specified object but only test for collisions/pushing on
		// the objects specified.  It'll carry things standing on it.
		virtual DRESULT MovePushObjects(HOBJECT hToMove, DVector &newPos, 
			HOBJECT *hPushObjects, DDWORD nPushObjects)=0;
		
		// Rotate the specified object but only test for collisions/pushing on
		// the objects specified.  It'll carry things standing on it.
		// This only works on world models.
		virtual DRESULT RotatePushObjects(HOBJECT hToMove, DRotation &newRot, 
			HOBJECT *hPushObjects, DDWORD nPushObjects)=0;
	};


#endif  

