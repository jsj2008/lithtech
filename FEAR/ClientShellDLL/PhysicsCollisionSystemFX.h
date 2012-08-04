// ----------------------------------------------------------------------- //
//
// MODULE  : CPhysicsCollisionSystemFX.h
//
// PURPOSE : Client side physics simulation collision system
//
// CREATED : 05/31/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PHYSICS_COLLISION_SYSTEM_FX_H__
#define __PHYSICS_COLLISION_SYSTEM_FX_H__

//
// Includes...
//

#include "SpecialFX.h"
#include "SharedFXStructs.h"

class CPhysicsCollisionSystemFX : public CSpecialFX
{
public: // Methods...

	CPhysicsCollisionSystemFX( );
	virtual ~CPhysicsCollisionSystemFX( );

	// Initialize the client side class...
	virtual bool Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg );

	// Poll the dependent objects until they become available...
	virtual bool Update( );

	// Retrieve the ID associated with CPhysicsConstraintFX objects...
	virtual uint32 GetSFXID( ) { return SFX_PHYSICS_COLLISION_SYSTEM_ID; }


private: // Methods...

	// Actually create the collision system and apply it to all specified objects...
	virtual bool CreatePhysicsCollisionSystem( );


private: // Members...

	// Message used to initialize collision system.
	// This needs to be cached so the dependent objects may be polled for when they are
	// available for use on the client.
	CLTMsgRef_Read m_cInitMsg;
};

#endif // __PHYSICS_COLLISION_SYSTEM_FX_H__

// EOF
