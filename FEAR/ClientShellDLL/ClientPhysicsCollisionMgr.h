// ----------------------------------------------------------------------- //
//
// MODULE  : ClientPhysicsCollisionMgr.h
//
// PURPOSE : Definition of physics collision mgr for Client
//
// CREATED : 08/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_PHYSICS_COLLISION_MGR_H__
#define __CLIENT_PHYSICS_COLLISION_MGR_H__

#include "PhysicsCollisionMgr.h"

//////////////////////////////////////////////////////////////////////////
//
// ClientPhysicsCollisionMgr
//
// The main interface for handling collisions on client.
//
//////////////////////////////////////////////////////////////////////////
class ClientPhysicsCollisionMgr : public PhysicsCollisionMgr
{
	DECLARE_SINGLETON( ClientPhysicsCollisionMgr )

public:

	// Destroys data.
	virtual void	Term( );

	// Called when world finishes loading.
	void			PostStartWorld( );

	// Called by collision responses so that client/server behavior can be specialized.
	// Must be defined by Client and Server implementations of PhysicsCollisionMgr.
	virtual bool	StartResponses( CollisionResponse& collisionResponse, CollisionData& collisionData );

	// Called by CollisionResponse objects so that client/server behavior can be specialized.
	virtual void	StopSound( CollisionResponse& collisionResponse );

	// Called when a MID_PHYSICSCOLLISION message is received from server.
	void			HandlePhysicsMessage( ILTMessage_Read *pMsg );

protected:

	// Called by StartSoundResponse.  Allows specialized behavior on client or server context.
	virtual bool	DoSoundResponse( CollisionResponse& collisionResponse, HRECORD hSoundDB, uint8 nVolume );
	// Called by StartClientFXResponse.  Allows specialized behavior on client or server context.
	virtual bool	DoClientFXResponse( CollisionResponse& collisionResponse, HATTRIBUTE hStruct, uint32 nIndexFound, char const* pszClientFx );

private:

	// List of collisionresponses that are playing sounds.  There will only
	// be a few simultaneous sounds allowed.
	typedef std::vector< CollisionResponse* > SoundCollisionResponses;
	SoundCollisionResponses m_lstSoundCollisionResponses;
};

#endif // __CLIENT_PHYSICS_COLLISION_MGR_H__
