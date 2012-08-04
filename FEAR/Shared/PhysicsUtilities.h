// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsUtilities.h
//
// PURPOSE : PhysicsUtilities - Physics utility functions
//
// CREATED : 04/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PHYSICS_UTILITIES_H__
#define __PHYSICS_UTILITIES_H__

#include "ltbasetypes.h"
#include "ltphysicsgroup.h"
#include "iltphysicssim.h" // for HPHYSICSRIGIDBODY

namespace PhysicsUtilities
{
	const float DEFAULT_IMPULSE_FORCE = 5000.0f;
	const float DEFAULT_BREAK_FORCE = 150.0f;
	
	const char* const WEIGHTSET_RIGID_BODY	 = "RigidBody";
	const char* const WEIGHTSET_NONE		 = "None";

	// Establish some physics groups usable by the game...
	// NOTE: We can not exceed the limit set by ePhysicsGroup_NumUserGroups, which happens to be 24!
	const EPhysicsGroup ePhysicsGroup_UserPlayer			= EPhysicsGroup(ePhysicsGroup_UserStart+0);
	const EPhysicsGroup ePhysicsGroup_UserAI				= EPhysicsGroup(ePhysicsGroup_UserStart+1);
	const EPhysicsGroup ePhysicsGroup_UserRagDoll			= EPhysicsGroup(ePhysicsGroup_UserStart+2);
	const EPhysicsGroup ePhysicsGroup_UserPickup			= EPhysicsGroup(ePhysicsGroup_UserStart+3);
	const EPhysicsGroup ePhysicsGroup_UserDebris			= EPhysicsGroup(ePhysicsGroup_UserStart+4);
	const EPhysicsGroup ePhysicsGroup_UserProjectile		= EPhysicsGroup(ePhysicsGroup_UserStart+5);
	const EPhysicsGroup ePhysicsGroup_UserRemoteMelee		= EPhysicsGroup(ePhysicsGroup_UserStart+6);
	const EPhysicsGroup ePhysicsGroup_UserLocalMelee		= EPhysicsGroup(ePhysicsGroup_UserStart+7);
	const EPhysicsGroup ePhysicsGroup_UserBlockMelee		= EPhysicsGroup(ePhysicsGroup_UserStart+8);
	const EPhysicsGroup ePhysicsGroup_UserFiltered			= EPhysicsGroup(ePhysicsGroup_UserStart+9);
	const EPhysicsGroup ePhysicsGroup_UserMultiplayer		= EPhysicsGroup(ePhysicsGroup_UserStart+10);
	const EPhysicsGroup ePhysicsGroup_UserPlayerRigidBody	= EPhysicsGroup(ePhysicsGroup_UserStart+11);

	// ----------------------------------------------------------------------- //
	//
	//	ROUTINE:	ApplyPhysicsImpulseForce()
	//
	//	PURPOSE:	Called to apply the appropriate physical impulse force to the 
	//				specified object.  If bUseCenterOfMass is set to true, the 
	//				passed in vHitPos will be ignored and the impulse force will
	//				be applied to the center of mass of the rigid body. 
	//
	// ----------------------------------------------------------------------- //
	void ApplyPhysicsImpulseForce(const HOBJECT hObject, 
								const float fImpulse, 
								const LTVector& vDir, 
								const LTVector& vHitPos,
								bool bUseCenterOfMass=true);

	// ----------------------------------------------------------------------- //
	//
	//	ROUTINE:	SetPhysicsWeightSet()
	//
	//	PURPOSE:	Set the physics weight set for the object and clear/set the
	//				FLAG2_RIGIDBODY flag as appropriate. In addition, this takes
	//				a parameter that will determine if the physics should be setup
	//				to only run on the client simulation and not on the server simulation
	//				(useful forenabling physics in multiplayer)
	//
	// ----------------------------------------------------------------------- //
	enum EnumRigidBodyState { RIGIDBODY_None, RIGIDBODY_Partial, RIGIDBODY_Full };
	EnumRigidBodyState SetPhysicsWeightSet(HOBJECT hObj, const char* pWeightSet, bool bClientOnly);

	// ----------------------------------------------------------------------- //
	//
	//	ROUTINE:	GetClosestRigidBody()
	//
	//	PURPOSE:	Get the closest rigid body to the given location on the
	//				specified character.
	//
	//	NOTES:		Caller is responsible for releasing the returned rigidbody.
	//
	// ----------------------------------------------------------------------- //
	HPHYSICSRIGIDBODY GetClosestRigidBody(HOBJECT hObj, LTVector vInPos);

	// ----------------------------------------------------------------------- //
	//
	//	ROUTINE:	GetRigidBodyModelNode()
	//
	//	PURPOSE:	Get the model node this rigidbody is attached to.
	//
	// ----------------------------------------------------------------------- //
	HMODELNODE GetRigidBodyModelNode(HOBJECT hObj, HPHYSICSRIGIDBODY hBody);

	// ----------------------------------------------------------------------- //
	//
	//	ROUTINE:	GetNodeParentRigidBody()
	//
	//	PURPOSE:	Given a model node, this will try to find the rigid body
	//				associated with that node, and will traverse up the node 
	//				heirarchy until it finds a rigid body
	//
	// ----------------------------------------------------------------------- //
	HPHYSICSRIGIDBODY GetNodeParentRigidBody(HOBJECT hObj, HMODELNODE hNode);
}

#endif // __PHYSICS_UTILITIES_H__
