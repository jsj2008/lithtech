// ----------------------------------------------------------------------- //
//
// MODULE  : CharacterPhysics.cpp
//
// PURPOSE : Shared module between client and server for handling common physics functionality...
//
// CREATED : 11/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "CharacterPhysics.h"
#include "PhysicsUtilities.h"
#include "SurfaceFunctions.h"
#include "ModelsDB.h"
#if defined(_SERVERBUILD)
#include "../ObjectDLL/DebugLineSystem.h"
#endif

//
// Statics...
//

static VarTrack s_vtBodyDeathDirMinY;
static VarTrack s_vtApplyDeathForce;
static VarTrack s_vtApplyDirDeathForce;
static VarTrack s_vtRagdollOnDeath;
static VarTrack s_vtDeathForceScale;
static VarTrack s_vtDirDeathForceScale;
static VarTrack s_vtBodyStickDist;
static VarTrack s_vtBodyStickTime;
static VarTrack s_vtBodyDeathScaleVelocity;
static VarTrack s_vtBodyDeathClampVelocityMax;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ReduceRigidBodyVelocities
//
//	PURPOSE:	Helper function to reduce the velocity of a characters 
//				rigid bodies on death.  Undesired velocity may come from 
//				animation/animation changes.  This function allows 
//				experimenting with reducing this velocity.
//
// ----------------------------------------------------------------------- //

static void ReduceRigidBodyVelocities( HOBJECT hObj )
{
	//get our physics simulation interface
	ILTPhysicsSim* pPhysicsSim = g_pLTBase->PhysicsSim();

	uint32 nNumBodies = 0;
	if (pPhysicsSim->GetNumModelRigidBodies(hObj, nNumBodies) != LT_OK)
		return;

	HPHYSICSRIGIDBODY hBestRigidBody = INVALID_PHYSICS_RIGID_BODY;
	for (uint32 nCurrBody = 0; nCurrBody < nNumBodies; nCurrBody++)
	{
		HPHYSICSRIGIDBODY hRigidBody;
		if( pPhysicsSim->GetModelRigidBody(hObj, nCurrBody, hRigidBody) == LT_OK )
		{
			LTVector vInitialVelocity;
			pPhysicsSim->GetRigidBodyVelocity(hRigidBody, vInitialVelocity);

			// Nothing to do with the velocity if it is already 0.
			if ( LTVector::GetIdentity() != vInitialVelocity )
			{
				// Handle tweaking the velocity of the rigid bodies. 
				float flInitialMag = vInitialVelocity.Mag();
				float flDampenedMag = flInitialMag * s_vtBodyDeathScaleVelocity.GetFloat();
				float flClampedMag = LTMIN( flDampenedMag, s_vtBodyDeathClampVelocityMax.GetFloat() );
				LTVector vNewVelocity = vInitialVelocity.GetUnit() * flClampedMag;

				// Apply the new velocity.
				pPhysicsSim->SetRigidBodyVelocity(hRigidBody, vNewVelocity);
			}

			// Always free the retrieved rigid body handle...
			pPhysicsSim->ReleaseRigidBody(hRigidBody);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FixRigidbodyPenetrationPositions_R
//
//	PURPOSE:	Recurse through the skeleton, looking for rigidbodies in a 
//				chain separated by world geometry. If any are found, move 
//				them to a 'safer' location and zero their velocity.
//
// ----------------------------------------------------------------------- //

static void FixRigidbodyPenetrationPositions_R(HOBJECT hCharacter, const LTVector& vLastTestPos, HMODELNODE hCurrentNode)
{
	uint32 ChildNodeCount = 0;
	g_pModelLT->GetNumChildren(hCharacter, hCurrentNode, ChildNodeCount);

	ILTPhysicsSim* pPhysicsSim = g_pLTBase->PhysicsSim();
	for (uint32 i = 0; i < ChildNodeCount; ++i)
	{
		HMODELNODE hChildNode = INVALID_MODEL_NODE;
		g_pModelLT->GetChild(hCharacter, hCurrentNode, i, hChildNode);

		// If this node has a rigidbody, test for a collision between this node
		// and the previous. If there is one, move it.

		LTVector vNextTestPos = vLastTestPos;

		HPHYSICSRIGIDBODY hRigidBody = INVALID_PHYSICS_RIGID_BODY;
		pPhysicsSim->GetModelNodeRigidBody(hCharacter, hChildNode, hRigidBody);
		if (hRigidBody != INVALID_PHYSICS_RIGID_BODY)
		{
			LTVector vCenterOfMass;
			pPhysicsSim->GetRigidBodyCenterOfMassInWorld(hRigidBody, vCenterOfMass);

			// Test for nodes close to penetrating a wall. This may occur when
			// velocity is applied, etc. For now, just add a buffer to the
			// tested distance.
			LTVector vBuffer = (vCenterOfMass - vLastTestPos).GetUnit()*16.0f;

			IntersectQuery IQuery;
			IQuery.m_From = vLastTestPos;
			IQuery.m_To = vCenterOfMass+vBuffer;
			IQuery.m_Flags = CHECK_FROM_POINT_INSIDE_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
			IQuery.m_FilterFn = GroundFilterFn;

			IntersectInfo IInfo;
			bool bIntersection = g_pLTBase->IntersectSegment(IQuery, &IInfo);
			if (bIntersection)
			{
				// Move the rigidbody to be on the correct side of the normal.
				LTRigidTransform vNewTransform;
				pPhysicsSim->GetRigidBodyTransform(hRigidBody, vNewTransform);
				vNewTransform.m_vPos = IInfo.m_Point + IInfo.m_Plane.m_Normal * 16.0f;
				pPhysicsSim->TeleportRigidBody(hRigidBody, vNewTransform);
				pPhysicsSim->SetRigidBodyVelocity(hRigidBody, LTVector::GetIdentity());
				vNextTestPos = vNewTransform.m_vPos;
			}
			else
			{
				vNextTestPos = vCenterOfMass;
			}

			// Uncomment to draw the skeleton traversal and the rigidbody updating.
			//SERVER_CODE
			//(
			//	DebugLineSystem& sys = LineSystem::GetSystem(hCharacter, "Skel");
			//	if (bIntersection)
			//	{
			//		sys.AddArrow(vLastTestPos, vCenterOfMass, Color::Red);
			//		sys.AddArrow(vLastTestPos, vNextTestPos, Color::White);
			//	}
			//	else
			//	{
			//		sys.AddArrow(vLastTestPos, vCenterOfMass, Color::Green);
			//	}
			//)	
		}

		pPhysicsSim->ReleaseRigidBody(hRigidBody);

		// Process all of the child nodes.

		FixRigidbodyPenetrationPositions_R(hCharacter, vNextTestPos, hChildNode);
	}	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FixRigidbodyPenetrationPositions
//
//	PURPOSE:	Start recursing through the skeleton, looking for world 
//				geometry splitting a connection between two rigidbodies.
//
// ----------------------------------------------------------------------- //

static void FixRigidbodyPenetrationPositions(HOBJECT hCharacter)
{
	LTVector vCharacterCenter;
	g_pLTBase->GetObjectPos(hCharacter, &vCharacterCenter);

	HMODELNODE hRoot = INVALID_MODEL_NODE;
	g_pModelLT->GetRootNode(hCharacter, hRoot);

	FixRigidbodyPenetrationPositions_R(hCharacter, vCharacterCenter, hRoot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::CCharacterPhysics
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CCharacterPhysics::CCharacterPhysics( )
:	m_hObject						( NULL ),
	m_tInitialWallStick				( ),
	m_tFinalWallStick				( ),
	m_hWallStickRigidBody			( INVALID_PHYSICS_RIGID_BODY ),
	m_hWallStickCollisionNotifier	( INVALID_PHYSICS_COLLISION_NOTIFIER ),
	m_bWallStickCollisionRegistered	( false )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::~CCharacterPhysics
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CCharacterPhysics::~CCharacterPhysics( )
{
	ILTPhysicsSim *pPhysicsSim = g_pLTBase->PhysicsSim( );
	
	if( pPhysicsSim )
	{
		if( m_hWallStickRigidBody )
		{
			pPhysicsSim->ReleaseRigidBody( m_hWallStickRigidBody );
			m_hWallStickRigidBody = INVALID_PHYSICS_RIGID_BODY;
		}

		if( m_hWallStickCollisionNotifier )
		{
			pPhysicsSim->ReleaseCollisionNotifier( m_hWallStickCollisionNotifier );
			m_hWallStickCollisionNotifier = INVALID_PHYSICS_COLLISION_NOTIFIER;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::Init
//
//	PURPOSE:	Initialize character physics for the specified object...
//
//	NOTE:		There is no class type checking on the object since this is a 
//				a shared functionality class and we have no class information 
//				on the client.
//
// ----------------------------------------------------------------------- //

bool CCharacterPhysics::Init( HOBJECT hObject )
{
	// Just make sure it is valid.  Don't check class type for reasons mentioned in the comment header...
	if( !hObject )
		return false;

	m_hObject = hObject;

	if( !s_vtRagdollOnDeath.IsInitted( ))
		s_vtRagdollOnDeath.Init( g_pLTBase, "BodyRagdoll", NULL, g_pModelsDB->GetBodyRagdoll() );
	
	if( !s_vtBodyDeathDirMinY.IsInitted( ))
		s_vtBodyDeathDirMinY.Init( g_pLTBase, "BodyDeathDirMinY", NULL, g_pModelsDB->GetBodyDeathDirMinY() );
	
	if( !s_vtApplyDeathForce.IsInitted( ))
		s_vtApplyDeathForce.Init( g_pLTBase, "BodyDeathForce", NULL, g_pModelsDB->GetBodyDeathForce() );
	
	if( !s_vtDeathForceScale.IsInitted( ))
		s_vtDeathForceScale.Init( g_pLTBase, "BodyDeathForceScale", NULL, g_pModelsDB->GetBodyDeathForceScale() );
	
	if( !s_vtApplyDirDeathForce.IsInitted( ))
		s_vtApplyDirDeathForce.Init( g_pLTBase, "BodyDirDeathForce", NULL, g_pModelsDB->GetBodyDirDeathForce() );
	
	if( !s_vtDirDeathForceScale.IsInitted( ))
		s_vtDirDeathForceScale.Init( g_pLTBase, "BodyDirDeathForceScale", NULL, g_pModelsDB->GetBodyDirDeathForceScale() );

	if( !s_vtBodyStickDist.IsInitted( ))
		s_vtBodyStickDist.Init( g_pLTBase, "BodyStickDist", NULL, g_pModelsDB->GetBodyStickDist() );

	if( !s_vtBodyStickTime.IsInitted( ))
		s_vtBodyStickTime.Init( g_pLTBase, "BodyStickTime", NULL, g_pModelsDB->GetBodyStickTime() );

	if ( !s_vtBodyDeathScaleVelocity.IsInitted() )
		s_vtBodyDeathScaleVelocity.Init( g_pLTBase, "BodyDeathScaleVelocity", NULL, g_pModelsDB->GetBodyDeathScaleVelocity() );

	if ( !s_vtBodyDeathClampVelocityMax.IsInitted() )
		s_vtBodyDeathClampVelocityMax.Init( g_pLTBase, "BodyDeathClampVelocityMax", NULL, g_pModelsDB->GetBodyDeathClampVelocityMax() );

	m_tmrWallStick.SetEngineTimer( SimulationTimer::Instance( ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::Update
//
//	PURPOSE:	Handle any updating required of the physics...
//
// ----------------------------------------------------------------------- //

void CCharacterPhysics::Update( )
{
	if( m_hWallStickRigidBody != INVALID_PHYSICS_RIGID_BODY )
	{
		ILTPhysicsSim *pPhysicsSim = g_pLTBase->PhysicsSim( );
		bool bPinned = false;
		pPhysicsSim->IsRigidBodyPinned( m_hWallStickRigidBody, bPinned );
		if( !bPinned )
		{
			if( m_bWallStickCollisionRegistered )
			{
				m_bWallStickCollisionRegistered = false;
				pPhysicsSim->KeyframeRigidBody( m_hWallStickRigidBody, m_tFinalWallStick, g_pLTBase->GetFrameTime( ));

				// Pin the body in place so it appears to be stuck to the wall...
				pPhysicsSim->PinRigidBody( m_hWallStickRigidBody, true );
			}
			else
			{
				// Continue keyframing the rigidbody until a collision was registered...
				double fDuration = m_tmrWallStick.GetDuration();
				double fTimeLeft = m_tmrWallStick.GetTimeLeft();
				float fT = (float)(( fDuration - fTimeLeft ) / fDuration);

				LTRigidTransform tTrans;
				tTrans.Interpolate( m_tInitialWallStick, m_tFinalWallStick, fT );

				pPhysicsSim->KeyframeRigidBody( m_hWallStickRigidBody, tTrans, g_pLTBase->GetFrameTime( ));
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::Reset
//
//	PURPOSE:	Release any physics objects and reset data...
//
// ----------------------------------------------------------------------- //

void CCharacterPhysics::Reset( const char *pszWeightSet, EPhysicsGroup ePhysicsGroup, bool bClientOnly )
{
	ILTPhysicsSim *pPhysicsSim = g_pLTBase->PhysicsSim( );
	if( pPhysicsSim )
	{
		if( m_hWallStickRigidBody )
		{
			pPhysicsSim->ReleaseRigidBody( m_hWallStickRigidBody );
			m_hWallStickRigidBody = INVALID_PHYSICS_RIGID_BODY;
		}

		if( m_hWallStickCollisionNotifier )
		{
			pPhysicsSim->ReleaseCollisionNotifier( m_hWallStickCollisionNotifier );
			m_hWallStickCollisionNotifier = INVALID_PHYSICS_COLLISION_NOTIFIER;
		}
		
		PhysicsUtilities::SetPhysicsWeightSet( m_hObject, pszWeightSet, bClientOnly );
		pPhysicsSim->SetObjectPhysicsGroup( m_hObject, ePhysicsGroup );
	}

	m_tmrWallStick.Stop( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::CanRagDoll
//
//	PURPOSE:	Test if the object is allowed to ragdoll...
//
// ----------------------------------------------------------------------- //

bool CCharacterPhysics::CanRagDoll( ) const
{
	// Don't do ragdoll death if disabled (used for debugging)...
	if( !s_vtRagdollOnDeath.GetFloat( )) 
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::RagDollDeath
//
//	PURPOSE:	Initialize ragdoll death setting weightsets and applying forces... 
//
// ----------------------------------------------------------------------- //

void CCharacterPhysics::RagDollDeath( const LTVector &rvDeathDir, float fDeathImpulseForce,
									  ModelsDB::HNODE hDeathNode, float fDeathNodeImpulseForceScale, 
									  bool bClampRagdollVelocity, bool bClientOnly )
{
	if( !m_hObject )
		return;

	ILTPhysicsSim *pPhysicsSim = g_pLTBase->PhysicsSim( );

	// First handle dampening and clamping any initial velocity coming from 
	// animations or other sources.

	if ( bClampRagdollVelocity )
	{
		ReduceRigidBodyVelocities( m_hObject );
	}

	// Go to a full rigid-body physics weight set and set the physics group to ragdoll...
	PhysicsUtilities::SetPhysicsWeightSet( m_hObject, PhysicsUtilities::WEIGHTSET_RIGID_BODY, bClientOnly );
	pPhysicsSim->SetObjectPhysicsGroup( m_hObject, PhysicsUtilities::ePhysicsGroup_UserRagDoll );

	// Direction death blow came from...
	LTVector vBodyForceDir = rvDeathDir;
	if( vBodyForceDir != LTVector::GetIdentity() )
	{
		vBodyForceDir.Normalize();
	}

	// Make sure body is pushed "up"...
	if( vBodyForceDir.y < s_vtBodyDeathDirMinY.GetFloat( ))
	{
		vBodyForceDir.y = s_vtBodyDeathDirMinY.GetFloat( );
		vBodyForceDir.Normalize();
	}

	// Don't apply death forces if disabled (used for physics debugging)...
	if( s_vtApplyDeathForce.GetFloat( ))
	{	
		float fForce = s_vtDeathForceScale.GetFloat() * fDeathImpulseForce;
		PhysicsUtilities::ApplyPhysicsImpulseForce( m_hObject, fForce, vBodyForceDir, LTVector(0, 0, 0) );
	}

	// Applying a scaled impulse force to the specific rigid body associated
	// with the node that was hit.  This allows for more "realistic" ragdolling...
	if( s_vtApplyDirDeathForce.GetFloat( ))
	{	
		HMODELNODE hNode = INVALID_MODEL_NODE;
		if (hDeathNode)
		{
			const char* szNodeName = g_pModelsDB->GetNodeName( hDeathNode );
			if( g_pModelLT->GetNode( m_hObject, szNodeName, hNode ) != LT_OK )
				return;
		}

		//if we don't have a specific node... distribute the impulse across the whole body
		if (hNode == INVALID_MODEL_NODE)
		{
			// Start with the normal death impulse force that has already been applied
			// to all of the rigid bodies on the model (see call to 
			// ApplyPhysicsImpulseForce() above)
			LTVector vDeathNodeImpulse = vBodyForceDir * fDeathImpulseForce;

			// Scale the force based as if we hit a specific node...
			vDeathNodeImpulse *= fDeathNodeImpulseForceScale;

			// And for good measure, apply the global scale...
			vDeathNodeImpulse *= s_vtDirDeathForceScale.GetFloat();

			if( LTIsNaN( vDeathNodeImpulse ) || vDeathNodeImpulse.MagSqr() > 1000000.0f * 1000000.0f )
			{
				LTERROR( "Invalid impulse detected." );
				vDeathNodeImpulse.Init( 0.0f, 10.0f, 0.0f );
			}

			//now distribute this force across the rigid bodies...
			uint32 nNumBodies = 0;
			if (pPhysicsSim->GetNumModelRigidBodies(m_hObject, nNumBodies) == LT_OK)
			{
				if (nNumBodies > 0)
				{
					vDeathNodeImpulse *= 1.0f/float(nNumBodies);
					for (uint32 nCurrBody = 0; nCurrBody < nNumBodies; nCurrBody++)
					{
						HPHYSICSRIGIDBODY hRigidBody;
						if (pPhysicsSim->GetModelRigidBody(m_hObject, nCurrBody, hRigidBody) != LT_OK)
							continue;
						// Use the center of mass of the rigid body
						LTVector vCenterOfMass;
						pPhysicsSim->GetRigidBodyCenterOfMassInWorld( hRigidBody, vCenterOfMass );

						//apply the impulse
						pPhysicsSim->ApplyRigidBodyImpulseWorldSpace( hRigidBody, vCenterOfMass, vDeathNodeImpulse );
					}
				}
			}

			

		}
		else
		{
			HPHYSICSRIGIDBODY hRigidBody = PhysicsUtilities::GetNodeParentRigidBody(m_hObject, hNode);
			if(hRigidBody != INVALID_PHYSICS_RIGID_BODY)
			{
				bool bActive = false;
				if( LT_OK == pPhysicsSim->IsRigidBodyActive( hRigidBody, bActive ) && bActive )
				{
					// Use the center of mass of the rigid body
					LTVector vCenterOfMass;
					pPhysicsSim->GetRigidBodyCenterOfMassInWorld( hRigidBody, vCenterOfMass );

					// Start with the normal death impulse force that has already been applied
					// to all of the rigid bodies on the model (see call to 
					// ApplyPhysicsImpulseForce() above)
					LTVector vDeathNodeImpulse = vBodyForceDir * fDeathImpulseForce;

					// Scale the force based on the hitting a specific node...
					vDeathNodeImpulse *= fDeathNodeImpulseForceScale;

					// Apply the scale associated with this particular model node...
					vDeathNodeImpulse *= g_pModelsDB->GetNodeInstDamageImpulseForceScale( hDeathNode );

					// And for good measure, apply the global scale...
					vDeathNodeImpulse *= s_vtDirDeathForceScale.GetFloat();

					if( LTIsNaN( vDeathNodeImpulse ) || vDeathNodeImpulse.MagSqr() > 1000000.0f * 1000000.0f )
					{
						LTERROR( "Invalid impulse detected." );
						vDeathNodeImpulse.Init( 0.0f, 10.0f, 0.0f );
					}

					pPhysicsSim->ApplyRigidBodyImpulseWorldSpace( hRigidBody, vCenterOfMass, vDeathNodeImpulse );
			
				}

				//and release our rigid body handle
				pPhysicsSim->ReleaseRigidBody( hRigidBody );
			}
		}
	}

	// Look for rigidbodies in the skeleton separated by world geometry. If
	// there are any, attempt to 'fix' them by updating their positions. If a
	// fix occurs, zero the magnitude of the velocity, to reduce the likelihood
	// that this (plus any constraint fixing in Havok) push the rigidbody to the
	// wrong side.

	FixRigidbodyPenetrationPositions(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::StickToGeometry
//
//	PURPOSE:	Sticks characters to walls based on the node hit and the direction of the damage...
//
// ----------------------------------------------------------------------- //

bool CCharacterPhysics::StickToGeometry( ModelsDB::HNODE hDeathNode, const LTVector &rvDeathDir, HAMMO hDeathAmmo )
{
	if( !hDeathAmmo || !hDeathNode )
		return false;

	// Only stick nodes that are specified to stick...
	if( !g_pModelsDB->GetNodeCanWallStick( hDeathNode ))
		return false;

	HAMMODATA hDeathAmmoData = g_pWeaponDB->GetAmmoData( hDeathAmmo );
	if( !g_pWeaponDB->GetBool( hDeathAmmoData, WDB_AMMO_bCanWallStick ))
		return false;

	const char* szNodeName = g_pModelsDB->GetNodeName( hDeathNode );

	HMODELNODE hNode;
	if( g_pModelLT->GetNode( m_hObject, szNodeName, hNode ) != LT_OK )
		return false;

	if( hNode == INVALID_MODEL_NODE )
		return false;	

	LTTransform tNode;
	if( g_pModelLT->GetNodeTransform( m_hObject, hNode, tNode, true ) != LT_OK )
		return false;

	LTVector vPos = tNode.m_vPos;
	LTVector vDir = rvDeathDir;

	// Direction death blow came from...

	if( vDir == LTVector::GetIdentity( ))
		return false;

	vDir.Normalize();
	

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From			= vPos;
	IQuery.m_To				= (vPos + (vDir * s_vtBodyStickDist.GetFloat()));
	IQuery.m_Flags			= INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn		= WorldFilterFn;
	IQuery.m_PolyFilterFn	= NULL;

	// Has to hit something
	if( g_pLTBase->IntersectSegment( IQuery, &IInfo ))
	{
		if( !IsMainWorld( IInfo.m_hObject ))
			return false;

		// Don't stick to flat ground...
		const float kfMinGroundDotValue = 0.707f;//cos(45 degrees)
		if( IInfo.m_Plane.m_Normal.y > kfMinGroundDotValue )
			return false;

		// Can the rigidbody stick to the surface?

		SurfaceType eSurf = GetSurfaceType( IInfo) ;
		HSURFACE hSurf = g_pSurfaceDB->GetSurface( eSurf );

		if( hSurf && g_pSurfaceDB->GetBool( hSurf, SrfDB_Srf_bCanWallStick ))
		{
			m_tFinalWallStick.m_vPos = IInfo.m_Point;

			m_hWallStickRigidBody = PhysicsUtilities::GetNodeParentRigidBody(m_hObject, hNode);
			if( m_hWallStickRigidBody != INVALID_PHYSICS_RIGID_BODY )
			{
				ILTPhysicsSim *pPhysicsSim = g_pLTBase->PhysicsSim( );
				pPhysicsSim->GetRigidBodyTransform( m_hWallStickRigidBody, m_tInitialWallStick );

				// Keep the rotation...
				m_tFinalWallStick.m_rRot = m_tInitialWallStick.m_rRot;

				// Start the position interpolation...
				m_tmrWallStick.Start( s_vtBodyStickTime.GetFloat( ));
				
				// Setup the collision notifier so we now when to pin the wall stick rigid body...
				m_hWallStickCollisionNotifier = pPhysicsSim->RegisterCollisionNotifier( m_hWallStickRigidBody, WallStickCollisionNotifier, this );
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterPhysics::WallStickCollisionNotifier
//
//	PURPOSE:	Collision notifier for the rigid body that will be pinned to a wall...
//
// ----------------------------------------------------------------------- //

void CCharacterPhysics::WallStickCollisionNotifier( HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2, 
													const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
													float fVelocity, bool& bIgnoreCollision, void* pUser )
{
	CCharacterPhysics *pCharacterPhysics = reinterpret_cast<CCharacterPhysics*>( pUser );
	if( !pCharacterPhysics )
	{
		LTERROR( "Invalid user data" );
		return;
	}

	// Object the wall stick rigid body collided with...
	HOBJECT hCollisionObject = INVALID_HOBJECT;

	LTVector vNormal;
	if( pCharacterPhysics->m_hWallStickRigidBody == hBody1 )
	{
		vNormal = -vCollisionNormal;
		g_pLTBase->PhysicsSim( )->GetRigidBodyObject( hBody2, hCollisionObject );
	}
	else if( pCharacterPhysics->m_hWallStickRigidBody == hBody2 )
	{
		vNormal = vCollisionNormal;
		g_pLTBase->PhysicsSim( )->GetRigidBodyObject( hBody1, hCollisionObject );
	}
	else
	{
		LTERROR( "Collision registered for non WallStick rigid body!" );
		return;
	}

	// Only collide with the main world...
	if( !IsMainWorld( hCollisionObject ))
		return;

	LTRigidTransform tCollision;
	g_pLTBase->PhysicsSim()->GetRigidBodyTransform( pCharacterPhysics->m_hWallStickRigidBody, tCollision );

	// Set the final position so the rigid bodies are in surface contact...
	pCharacterPhysics->m_tFinalWallStick.m_vPos = tCollision.m_vPos + vNormal;
	pCharacterPhysics->m_bWallStickCollisionRegistered = true;
}

// EOF
