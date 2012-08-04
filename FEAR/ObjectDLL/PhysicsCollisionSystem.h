// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsCollisionSystem.h
//
// PURPOSE : PhysicsCollisionSystem - Definition
//
// CREATED : 1/13/2004
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PHYSICSCOLLISIONSYSTEM_H__
#define __PHYSICSCOLLISIONSYSTEM_H__

#include "GameBase.h"
#include "CommonUtilities.h"

LINKTO_MODULE( PhysicsCollisionSystem );

class PhysicsCollisionSystem : public GameBase
{
	public:

		PhysicsCollisionSystem();
		~PhysicsCollisionSystem();

		StringArray*	GetObjectNames() { return &m_saObjectNames; }

	private:

		uint32		EngineMessageFn( uint32 messageID, void *pvData, float fData );

		bool		ReadProp(const GenericPropList *pProps);
		void		InitialUpdate( );

		StringArray	m_saObjectNames;

		// In multiplayer games the collision system needs to be created on the client...
		bool					m_bMPClientOnly;
};

#endif  // __PHYSICSCOLLISIONSYSTEM_H__
