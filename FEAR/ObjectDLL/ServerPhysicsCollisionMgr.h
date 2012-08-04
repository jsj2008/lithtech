// ----------------------------------------------------------------------- //
//
// MODULE  : ServerPhysicsCollisionMgr.h
//
// PURPOSE : Definition of physics collision mgr for server
//
// CREATED : 08/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_PHYSICS_COLLISION_MGR_H__
#define __SERVER_PHYSICS_COLLISION_MGR_H__

#include "PhysicsCollisionMgr.h"

//////////////////////////////////////////////////////////////////////////
//
// ServerPhysicsCollisionMgr
//
// The main interface for handling collisions on server.
//
//////////////////////////////////////////////////////////////////////////
class ServerPhysicsCollisionMgr : public PhysicsCollisionMgr
{
	DECLARE_SINGLETON_SIMPLE( ServerPhysicsCollisionMgr )

public:

	virtual bool	Init( );

	// Called when world finishes loading.
	void			PostStartWorld( );

	// Main entry function for collision event from engine.  Returns false if ignored.
	virtual bool	HandleRigidBodyCollision( CollisionData& collisionData );

	// Called by collision responses so that client/server behavior can be specialized.
	// Must be defined by Client and Server implementations of PhysicsCollisionMgr.
	virtual bool	StartResponses( CollisionResponse& collisionResponse, CollisionData& collisionData );

	// Starts an ai stimulus response.
	bool			StartAIStimulusResponse( CollisionResponse& collisionResponse );
	float			FindStimulusRadius( CollisionResponse const& collisionResponse );
	// Starts damage response.
	bool			StartDamageResponse( CollisionResponse& collisionResponse );

};

#endif // __SERVER_PHYSICS_COLLISION_MGR_H__
