// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterPhysics.h
//
// PURPOSE : Shared module between client and server for handling common physics functionality...
//
// CREATED : 11/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_PHYSICS_H__
#define __CHARACTER_PHYSICS_H__

//
// Includes...
//

#include "iltphysicssim.h"


class CCharacterPhysics
{
	public: // Methods...

		CCharacterPhysics( );
		~CCharacterPhysics( );

		// Initialize character physics for the specified object...
		bool Init( HOBJECT hObject );

		// Handle any updating required of the physics...
		void Update( );

		// Release any physics objects and reset data...
		void Reset( const char *pszWeightSet, EPhysicsGroup ePhysicsGroup, bool bClientOnly );

		// Test if the object is allowed to ragdoll...
		bool CanRagDoll( ) const;

		// Initialize ragdoll death setting weightsets and applying forces... 
		void RagDollDeath( const LTVector &rvDeathDir, float fDeathImpulseForce,
						   ModelsDB::HNODE hDeathNode, float fDeathNodeImpulseForceScale, 
						   bool bClampRagdollVelocity, bool bClientOnly );

		// Sticks characters to walls based on the node hit and the direction of the damage...
		bool StickToGeometry( ModelsDB::HNODE hDeathNode, const LTVector &rvDeathDir, HAMMO hDeathAmmo );

		const HPHYSICSRIGIDBODY& GetWallStickRigidBody( ) const { return m_hWallStickRigidBody; }


	private: // Methods...

		static void WallStickCollisionNotifier( HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
												const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
												float fVelocity, bool& bIgnoreCollision, void* pUser );
	

	private: // Members...

		// Character object to apply physics functionality to...
		LTObjRef			m_hObject;

		// Timer used for interpolating the rigidbody into position for sticking to walls...
		StopWatchTimer		m_tmrWallStick;

		// Transforms for interpolating the rigidbody into a "stuck" position...
		LTRigidTransform	m_tInitialWallStick;
		LTRigidTransform	m_tFinalWallStick;

		// Actual rigidbody that will be pinned to the wall...
		HPHYSICSRIGIDBODY	m_hWallStickRigidBody;

		// Notifier used to determine when the rigid body has come into contact
		// with the object it should be pinned against...
		HPHYSICSCOLLISIONNOTIFIER m_hWallStickCollisionNotifier;

		// The collision notifier was called with a valid collision...
		bool m_bWallStickCollisionRegistered;
};

#endif // __CHARACTER_PHYSICS_H__

// EOF
