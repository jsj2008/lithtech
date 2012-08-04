// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsUtilities.cpp
//
// PURPOSE : PhysicsUtilities - Physics utility functions
//
// CREATED : 04/16/04
//
// (c) 2004-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PhysicsUtilities.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsUtilities::ApplyPhysicsImpulseForce()
//
//	PURPOSE:	Called to apply the appropriate physical impulse force to the 
//				specified object.  If bUseCenterOfMass is set to true, the 
//				passed in vHitPos will be ignored and the impulse force will
//				be applied to the center of mass of the rigid body. 
//
// ----------------------------------------------------------------------- //
void PhysicsUtilities::ApplyPhysicsImpulseForce(const HOBJECT hObject, 
							  const float fImpulse, 
							  const LTVector& vDir, 
							  const LTVector& vHitPos,
							  bool bUseCenterOfMass /*=true*/)
{
	//make sure that we have an object and an impulse
	if(!hObject || fImpulse <= 0.0f || IsMainWorld(hObject))
		return;

	//now determine the type of object we're apply a force to
	uint32 nObjectType;
	if(g_pLTBase->Common()->GetObjectType(hObject, &nObjectType) != LT_OK)
		return;

	//get our physics simulation interface
	ILTPhysicsSim* pPhysicsSim = g_pLTBase->PhysicsSim();

	//For world models all we need to do is apply the force to the single rigid body
	if(nObjectType == OT_WORLDMODEL)
	{
		//get the world model rigid body
		HPHYSICSRIGIDBODY hRigidBody;
		if(pPhysicsSim->GetWorldModelRigidBody(hObject, hRigidBody) != LT_OK)
			return;

		bool bPinned = false;
		pPhysicsSim->IsRigidBodyPinned( hRigidBody, bPinned );
		
		// Ignore pinned rigid bodies...
		if( !bPinned )
		{
			// Determine if we should use the hit position or the center of masss...
			LTVector vPos = vHitPos;
			if (bUseCenterOfMass)
			{
				// Use the center of mass of the rigid body
				pPhysicsSim->GetRigidBodyCenterOfMassInWorld(hRigidBody, vPos);
			}

			LTVector vImpulse = vDir * fImpulse;
			if( LTIsNaN( vImpulse ) || vImpulse.MagSqr() > 1000000.0f * 1000000.0f )
			{
				LTERROR( "Invalid impulse detected." );
				vImpulse.Init( 0.0f, 10.0f, 0.0f );
			}

			//now apply the force to it
			pPhysicsSim->ApplyRigidBodyImpulseWorldSpace(hRigidBody, vPos, vImpulse);
		}

		//and release our rigid body handle
		pPhysicsSim->ReleaseRigidBody(hRigidBody);
	}
	else if(nObjectType == OT_MODEL)
	{
		//If we have a model, we need to apply the force to all of it's rigid bodies
		uint32 nNumBodies = 0;
		if(pPhysicsSim->GetNumModelRigidBodies(hObject, nNumBodies) != LT_OK)
			return;

		//run through and apply the force to all of the bodies
		for(uint32 nCurrBody = 0; nCurrBody < nNumBodies; nCurrBody++)
		{
			HPHYSICSRIGIDBODY hRigidBody;
			if(pPhysicsSim->GetModelRigidBody(hObject, nCurrBody, hRigidBody) != LT_OK)
				continue;

			bool bPinned = false;
			pPhysicsSim->IsRigidBodyPinned( hRigidBody, bPinned );

			// Ignore pinned rigid bodies...
			if( !bPinned )
			{
				// Use the center of mass of the rigid body
				LTVector vCenterOfMass;
				pPhysicsSim->GetRigidBodyCenterOfMassInWorld(hRigidBody, vCenterOfMass);

				LTVector vImpulse = vDir * fImpulse;
				if( LTIsNaN( vImpulse ) || vImpulse.MagSqr() > 1000000.0f * 1000000.0f )
				{
					LTERROR( "Invalid impulse detected." );
					vImpulse.Init( 0.0f, 10.0f, 0.0f );
				}

				//now apply the force to it
				pPhysicsSim->ApplyRigidBodyImpulseWorldSpace(hRigidBody, vCenterOfMass, vImpulse);
			}

			//and release our rigid body handle
			pPhysicsSim->ReleaseRigidBody(hRigidBody);
		}		
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsUtilities::SetPhysicsWeightSet()
//
//	PURPOSE:	Set the physics weight set for the object and clear/set the FLAG2_RIGIDBODY flag
//				as appropriate. In addition, this takes a parameter that will determine if the physics should
//				be setup to only run on the client simulation and not on the server simulation (useful for
//				enabling physics in multiplayer)
//
//  RETURNS:    True if FLAG2_RIGIDBODY is set on the object
//
// ----------------------------------------------------------------------- //

PhysicsUtilities::EnumRigidBodyState PhysicsUtilities::SetPhysicsWeightSet(HOBJECT hObj, const char* pWeightSet, bool bClientOnly)
{
	static CParsedMsg::CToken s_cTok_PhysicsWeightSetNone( PhysicsUtilities::WEIGHTSET_NONE );
	static CParsedMsg::CToken s_cTok_PhysicsWeightSetRigidBody( PhysicsUtilities::WEIGHTSET_RIGID_BODY );

	// Make sure our parameters are valid...
	if (!pWeightSet || !hObj) return RIGIDBODY_None;

	// Only change the physics weight set on models...
	uint32 nType;
	g_pCommonLT->GetObjectType(hObj, &nType);
	if (nType != OT_MODEL) return RIGIDBODY_None;

	// Assume we'll set the rigid body flag...
	EnumRigidBodyState eRigidBodyState;

	//the flag that we will be using
	uint32 nPhysicsFlag = (bClientOnly) ? FLAG2_CLIENTRIGIDBODY : FLAG2_RIGIDBODY;

	CParsedMsg::CToken cToken( pWeightSet );

	if ( LTStrEmpty(pWeightSet) || cToken == s_cTok_PhysicsWeightSetNone )
	{
		// Clear the use of rigid body and the weight set...
		g_pCommonLT->SetObjectFlags(hObj, OFT_Flags2, 0, nPhysicsFlag);
		g_pModelLT->SetPhysicsWeightSet(hObj, INVALID_PHYSICS_WEIGHT_SET);

		// Cleared the rigid body flag
		eRigidBodyState = RIGIDBODY_None;
	}
	else if ( cToken == s_cTok_PhysicsWeightSetRigidBody )
	{
		// Full on rag-doll...
		g_pCommonLT->SetObjectFlags(hObj, OFT_Flags2, nPhysicsFlag, nPhysicsFlag);
		g_pModelLT->SetPhysicsWeightSet(hObj, INVALID_PHYSICS_WEIGHT_SET);
		eRigidBodyState = RIGIDBODY_Full;
	}
	else 
	{
		// If the weight set is valid, set it on the model...
		HPHYSICSWEIGHTSET hSet;
		if (g_pModelLT->FindPhysicsWeightSet(hObj, pWeightSet, hSet) == LT_OK)
		{
			// Use the specified physics weight set...
			g_pCommonLT->SetObjectFlags(hObj, OFT_Flags2, nPhysicsFlag, nPhysicsFlag);
			g_pModelLT->SetPhysicsWeightSet(hObj, hSet);
		}
		eRigidBodyState = RIGIDBODY_Partial;
	}

	// Return the state of the rigid body flag.
	return eRigidBodyState;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PhysicsUtilities::GetClosestRigidBody()
//
//	PURPOSE:	Get the closest rigid body to the given location on the
//				specified character.
//
//	NOTES:		Caller is responsible for releasing the returned rigidbody.
//
// ----------------------------------------------------------------------- //

HPHYSICSRIGIDBODY PhysicsUtilities::GetClosestRigidBody(HOBJECT hObj, LTVector vInPos)
{
	//get our physics simulation interface
	ILTPhysicsSim* pPhysicsSim = g_pLTBase->PhysicsSim();

	uint32 nNumBodies = 0;
	if (pPhysicsSim->GetNumModelRigidBodies(hObj, nNumBodies) != LT_OK)
		return false;

	float fBestDist = 0.0f;
	HPHYSICSRIGIDBODY hBestRigidBody = INVALID_PHYSICS_RIGID_BODY;
	for (uint32 nCurrBody = 0; nCurrBody < nNumBodies; nCurrBody++)
	{
		HPHYSICSRIGIDBODY hRigidBody;
		if (pPhysicsSim->GetModelRigidBody(hObj, nCurrBody, hRigidBody) != LT_OK)
			continue;

		LTRigidTransform tRigidBody;
		if (pPhysicsSim->GetRigidBodyTransform(hRigidBody, tRigidBody) != LT_OK)
			continue;

		float fDist = tRigidBody.m_vPos.DistSqr(vInPos);
		if (hBestRigidBody == INVALID_PHYSICS_RIGID_BODY)
		{
			fBestDist = fDist;
			hBestRigidBody = hRigidBody;
			// don't release the best (leave that to the caller)
		}
		else
		{
			if (fDist < fBestDist)
			{
				fBestDist = fDist;
				std::swap(hBestRigidBody, hRigidBody);
			}
			pPhysicsSim->ReleaseRigidBody(hRigidBody);
		}
	}

	return hBestRigidBody;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetRigidBodyModelNode()
//
//	PURPOSE:	Get the model node this rigidbody is attached to.
//
// ----------------------------------------------------------------------- //
HMODELNODE PhysicsUtilities::GetRigidBodyModelNode(HOBJECT hObj, HPHYSICSRIGIDBODY hBody)
{
	HMODELNODE hCurNode = INVALID_MODEL_NODE;
	while (g_pLTBase->GetModelLT()->GetNextNode(hObj, hCurNode, hCurNode) == LT_OK)
	{
		HPHYSICSRIGIDBODY hCurBody;
		if (g_pLTBase->PhysicsSim()->GetModelNodeRigidBody(hObj, hCurNode, hCurBody) == LT_OK)
		{
			if (hCurBody == hBody)
			{
				g_pLTBase->PhysicsSim()->ReleaseRigidBody(hCurBody);
				return hCurNode;
			}
			g_pLTBase->PhysicsSim()->ReleaseRigidBody(hCurBody);
		}
	}

	return INVALID_MODEL_NODE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetNodeParentRigidBody()
//
//	PURPOSE:	Given a model node, this will try to find the rigid body
//				associated with that node, and will traverse up the node 
//				heirarchy until it finds a rigid body
//
// ----------------------------------------------------------------------- //
HPHYSICSRIGIDBODY PhysicsUtilities::GetNodeParentRigidBody(HOBJECT hObj, HMODELNODE hNode)
{
	//traverse up the node tree
	while(hNode != INVALID_MODEL_NODE)
	{
		//see if this node has a rigid body associated with it
		HPHYSICSRIGIDBODY hCurBody;
		if(g_pLTBase->PhysicsSim()->GetModelNodeRigidBody(hObj, hNode, hCurBody) == LT_OK)
		{
			return hCurBody;
		}

		//no node at this point, traverse up the heirarchy
		HMODELNODE hParent;
		if(g_pLTBase->GetModelLT()->GetParent(hObj, hNode, hParent) != LT_OK)
		{
			//no parent node, so fail
			return INVALID_PHYSICS_RIGID_BODY;
		}

		//we do have a parent, so move up to the parent
		hNode = hParent;
	}

	//unable to find a rigid body, so fail
	return INVALID_PHYSICS_RIGID_BODY;
}


