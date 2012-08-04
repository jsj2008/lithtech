// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerRigidBody.h
//
// PURPOSE : Shared module between client and server for controlling physics collisions with the player...
//
// CREATED : 06/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_RIGID_BODY_H__
#define __PLAYER_RIGID_BODY_H__

//
// Includes...
//

#include "iltphysicssim.h"

class CPlayerRigidBody
{
	public: // Methods...

		CPlayerRigidBody( );
		~CPlayerRigidBody( );

		// Initialize the player rigid body data...
		void Init( HOBJECT hPlayer );

		// Move the player rigid body to the new position...
		void Update( const LTVector &vNewPos );


	private: // Methods...

		// Release all physics objects...
		void Term( );

		// Callback function that it is triggered by the physics simulation...
		static void ActionCB( HPHYSICSRIGIDBODY hBody, float fUpdateTimeS, void* pUserData );


	private: // Members...

		// Object the rigid body is following...
		LTObjRef m_hObject;

		// Object that the rigid body was last penetrating with...
		LTObjRef m_hLastPenetratingObject;

		// Direction the the last penetrating object tried to move in order to stop penetrating...
		LTVector m_vLastPenetratingMoveDir;

		// Timer to recalculate the direction to move the object...
		StopWatchTimer m_swLastPenetratingTimer;

		float m_fRadiusScale;
		float m_fForce;
		float m_fForceY;
		float m_fRegenMovementVel;


		HPHYSICSRIGIDBODY	m_hRigidBody;
		HPHYSICSACTION		m_hAction;
		HPHYSICSCONTAINER	m_hContainer;
		HPHYSICSRIGIDBODY	m_hRigidBodyACB;
};

#endif // __PLAYER_RIGID_BODY_H__

// EOF
