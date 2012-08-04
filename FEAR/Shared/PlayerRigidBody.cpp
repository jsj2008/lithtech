// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerRigidBody.cpp
//
// PURPOSE : Shared module between client and server for controlling physics collisions with the player...
//
// CREATED : 06/05/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "PlayerRigidBody.h"
#include "PhysicsUtilities.h"
#include <algorithm>


static VarTrack s_vtPRBHeightOffset;
static VarTrack s_vtPRBRadiusScale;
static VarTrack s_vtPRBRadiusScaleClosestPt;
static VarTrack s_vtPRBForce;
static VarTrack s_vtPRBForceClosestPt;
static VarTrack s_vtPRBForceY;
static VarTrack s_vtPRBForceYClosestPt;
static VarTrack s_vtPRBUseClosestPoint;
static VarTrack s_vtPRBInitialDimsX;
static VarTrack s_vtPRBInitialDimsY;
static VarTrack s_vtPRBPenetratingDistance;
static VarTrack s_vtPRBPenetratingMoveMag;
static VarTrack s_vtPRBPenetratingVelocity;
static VarTrack s_vtPRBPenetratingTime;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerRigidBody::CPlayerRigidBody
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CPlayerRigidBody::CPlayerRigidBody( )
	:m_hObject					( INVALID_HOBJECT )
	,m_hLastPenetratingObject	( INVALID_HOBJECT )
	,m_vLastPenetratingMoveDir	( LTVector::GetIdentity( ))
	,m_fRadiusScale				( 0.0f )
	,m_fForce					( 0.0f )
	,m_fForceY					( 0.0f )
	,m_fRegenMovementVel		( 100.0f )
	,m_hRigidBody				( INVALID_PHYSICS_RIGID_BODY )
	,m_hAction					( INVALID_PHYSICS_ACTION )
	,m_hContainer				( INVALID_PHYSICS_CONTAINER )
	,m_hRigidBodyACB			( INVALID_PHYSICS_RIGID_BODY )
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerRigidBody::~CPlayerRigidBody
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CPlayerRigidBody::~CPlayerRigidBody( )
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerRigidBody::Term
//
//	PURPOSE:	Release all physics objects...
//
// ----------------------------------------------------------------------- //

void CPlayerRigidBody::Term( )
{
	ILTPhysicsSim *pILTPhysicsSim = g_pLTBase->PhysicsSim( );

	if( INVALID_PHYSICS_RIGID_BODY != m_hRigidBody )
	{
		pILTPhysicsSim->ReleaseRigidBody( m_hRigidBody );
		m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
	}

	if( INVALID_PHYSICS_RIGID_BODY != m_hRigidBodyACB )
	{
		pILTPhysicsSim->ReleaseRigidBody( m_hRigidBodyACB );
		m_hRigidBodyACB = INVALID_PHYSICS_RIGID_BODY;
	}

	if( INVALID_PHYSICS_ACTION != m_hAction )
	{
		pILTPhysicsSim->ReleaseAction( m_hAction );
		m_hAction = INVALID_PHYSICS_ACTION;
	}

	if( INVALID_PHYSICS_CONTAINER != m_hContainer )
	{
		pILTPhysicsSim->ReleaseContainer( m_hContainer );
		m_hContainer = INVALID_PHYSICS_CONTAINER;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerRigidBody::Init
//
//	PURPOSE:	Initialize the player rigid body data...
//
// ----------------------------------------------------------------------- //

void CPlayerRigidBody::Init( HOBJECT hPlayer )
{
	if( !hPlayer )
		return;

	// Release any previously created physics objects...
	Term( );

	if ( !s_vtPRBRadiusScale.IsInitted() )
	{
		s_vtPRBRadiusScale.Init(g_pLTBase, "PRBRadiusScale", NULL, 2.0f);
	}
	if ( !s_vtPRBRadiusScaleClosestPt.IsInitted() )
	{
		s_vtPRBRadiusScaleClosestPt.Init(g_pLTBase, "PRBRadiusScaleCP", NULL, 1.25f);
	}
	if ( !s_vtPRBHeightOffset.IsInitted() )
	{
		s_vtPRBHeightOffset.Init(g_pLTBase, "PRBHeightOffset", NULL, 1.0f);
	}
	if ( !s_vtPRBForce.IsInitted() )
	{
		s_vtPRBForce.Init(g_pLTBase, "PRBForce", NULL, 22000.0f);
	}
	if ( !s_vtPRBForceClosestPt.IsInitted() )
	{
		s_vtPRBForceClosestPt.Init(g_pLTBase, "PRBForceCP", NULL, 8800.0f);
	}
	if ( !s_vtPRBForceY.IsInitted() )
	{
		s_vtPRBForceY.Init(g_pLTBase, "PRBForceY", NULL, 0.0f);
	}
	if ( !s_vtPRBForceYClosestPt.IsInitted() )
	{
		s_vtPRBForceYClosestPt.Init(g_pLTBase, "PRBForceYCP", NULL, 0.0f);
	}
	if ( !s_vtPRBUseClosestPoint.IsInitted() )
	{
		s_vtPRBUseClosestPoint.Init(g_pLTBase, "PRBUseClosestPoint", NULL, 1.0f);
	}
	if( !s_vtPRBInitialDimsX.IsInitted( ))
	{
		s_vtPRBInitialDimsX.Init( g_pLTBase, "PRBInitialDimsX", NULL, 40.0f );
	}
	if( !s_vtPRBInitialDimsY.IsInitted( ))
	{
		s_vtPRBInitialDimsY.Init( g_pLTBase, "PRBInitialDimsY", NULL, 90.0f );
	}
	if( !s_vtPRBPenetratingDistance.IsInitted( ) )
	{
		s_vtPRBPenetratingDistance.Init( g_pLTBase, "PRBPenetratingDistance", NULL, 35.0f );
	}
	if( !s_vtPRBPenetratingMoveMag.IsInitted( ) )
	{
		s_vtPRBPenetratingMoveMag.Init( g_pLTBase, "PRBPenetratingMoveMag", NULL, 3.0f );
	}
	if( !s_vtPRBPenetratingVelocity.IsInitted( ) )
	{
		s_vtPRBPenetratingVelocity.Init( g_pLTBase, "PRBPenetratingVelocity", NULL, 10.0f );
	}
	if( !s_vtPRBPenetratingTime.IsInitted( ) )
	{
		s_vtPRBPenetratingTime.Init( g_pLTBase, "PRBPenetratingTime", NULL, 0.5f );
	}

	m_hObject = hPlayer;

	m_swLastPenetratingTimer.SetEngineTimer( RealTimeTimer::Instance( ) );

	// Cache the physics interface...
	ILTPhysicsSim *pILTPhysicsSim = g_pLTBase->PhysicsSim( );


	// Initialize variables based on current console values...

	if( s_vtPRBUseClosestPoint.GetFloat( ) != 0.0f )
	{
		m_fRadiusScale	= s_vtPRBRadiusScaleClosestPt.GetFloat( );
		m_fForce		= s_vtPRBForceClosestPt.GetFloat( );
		m_fForceY		= s_vtPRBForceYClosestPt.GetFloat( );
	}
	else
	{
		m_fRadiusScale	= s_vtPRBRadiusScale.GetFloat( );
		m_fForce		= s_vtPRBForce.GetFloat( );
		m_fForceY		= s_vtPRBForceY.GetFloat( );
	}


	// Create rigid body...

	LTVector vPlayerDims( s_vtPRBInitialDimsX.GetFloat( ), s_vtPRBInitialDimsY.GetFloat( ), s_vtPRBInitialDimsX.GetFloat( ) );
	LTRigidTransform tPlayerTrans;
	g_pLTBase->GetObjectTransform( m_hObject, &tPlayerTrans );

	float fDensityG	= 1.0f;
	float fMass		= 1000.0f;
	float fRadius	= (vPlayerDims.x * m_fRadiusScale);

	HPHYSICSSHAPE hShape = INVALID_PHYSICS_SHAPE;

	LTVector vHalfDims = vPlayerDims;
	vHalfDims *= m_fRadiusScale;
	vHalfDims.y = vPlayerDims.y + s_vtPRBHeightOffset.GetFloat( );

	hShape = pILTPhysicsSim->CreateBoxShape( vHalfDims, fMass, fDensityG, fRadius );
	if( hShape == INVALID_PHYSICS_SHAPE )
		return;

	// We need to create the container object that will handle maintaining the
	// list of overlapping objects

	m_hContainer = pILTPhysicsSim->CreateContainer( );
	if( m_hContainer == INVALID_PHYSICS_CONTAINER )
	{
		pILTPhysicsSim->ReleaseShape( hShape );
		return;
	}

	HPHYSICSSHAPE hContainerShape = pILTPhysicsSim->CreateContainerShape( hShape, m_hContainer );

	// Only collide with World, WorldModels, and Models...
	m_hRigidBody = pILTPhysicsSim->CreateRigidBody( hContainerShape, tPlayerTrans, false,
		PhysicsUtilities::ePhysicsGroup_UserPlayerRigidBody, 0, 0.0f, 0.0f );

	// We can now free our shapes
	pILTPhysicsSim->ReleaseShape( hShape );
	pILTPhysicsSim->ReleaseShape( hContainerShape );

	// Verify that our rigid body was constructed properly
	if( m_hRigidBody == INVALID_PHYSICS_RIGID_BODY )
		return;

	// Make sure rigid body always moves where we tell it to move
	pILTPhysicsSim->PinRigidBody( m_hRigidBody, true );

	// Make sure to setup the rigid body so that it can receive pinned->pinned collisions
	pILTPhysicsSim->EnableRigidBodyPinnedCollisions( m_hRigidBody, true );

	// Create our action so we'll get callbacks...
	m_hAction = pILTPhysicsSim->CreateAction( m_hRigidBody );
	if( m_hAction == INVALID_PHYSICS_ACTION )
		return;

	// Setup our callback to notify us when we need to apply forces
	pILTPhysicsSim->SetActionCallback( m_hAction, ActionCB, this );


	// Create a rigid body to be used in the Action Callback, but doesn't collide with
	// anything (this is needed for the call to GetClosestPoint to work)...

	HPHYSICSSHAPE hSphereShape = pILTPhysicsSim->CreateSphereShape( 1.0f, fMass, fDensityG );

	m_hRigidBodyACB = pILTPhysicsSim->CreateRigidBody( hSphereShape, tPlayerTrans, false,
		ePhysicsGroup_NonSolid, 0, 0.5f, 0.0f );

	pILTPhysicsSim->ReleaseShape( hSphereShape );

	// Verify that our rigid body was constructed properly
	if( m_hRigidBodyACB == INVALID_PHYSICS_RIGID_BODY )
		return;

	// Make sure rigid body always moves where we tell it to move
	pILTPhysicsSim->PinRigidBody( m_hRigidBodyACB, true );

	// Make sure to setup the rigid body so that it can receive pinned->pinned collisions
	pILTPhysicsSim->EnableRigidBodyPinnedCollisions( m_hRigidBodyACB, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerRigidBody::Update
//
//	PURPOSE:	Move the player rigid body to the new position...
//				The passed in position should be the new position value that
//				the player will move to.  This should be called before the player
//				actualy moves to that position.
//
// ----------------------------------------------------------------------- //

void CPlayerRigidBody::Update( const LTVector &vNewPos )
{
	if( !m_hObject )
		return;

	// Cache the physics interface
	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	// Update rigid body position...
	LTRigidTransform tPlayerTrans;
	g_pLTBase->GetObjectTransform( m_hObject, &tPlayerTrans );
	tPlayerTrans.m_vPos = vNewPos;

	float fTimeInterval = SimulationTimer::Instance().GetTimerElapsedS();
	pILTPhysicsSim->KeyframeRigidBody( m_hRigidBody, tPlayerTrans, fTimeInterval);

	if( INVALID_PHYSICS_RIGID_BODY != m_hRigidBodyACB )
	{
		pILTPhysicsSim->KeyframeRigidBody( m_hRigidBodyACB, tPlayerTrans, fTimeInterval );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerRigidBody::ActionCB
//
//	PURPOSE:	Callback function that it is triggered by the physics simulation...
//
// ----------------------------------------------------------------------- //

void CPlayerRigidBody::ActionCB( HPHYSICSRIGIDBODY hBody, float fUpdateTimeS, void* pUserData )
{
	// Track the current execution shell scope for proper SEM behavior
	#if defined(PLATFORM_SEM)

#if defined(_CLIENTBUILD) && defined(_SERVERBUILD)
	#error PRBActionCB requires seperate compilation.  Please remove module from the shared project.
#else

	CLIENT_CODE
	(
		CGameClientShell::CClientShellScopeTracker cScopeTracker;
	)
	SERVER_CODE
	(
		CGameServerShell::CServerShellScopeTracker cScopeTracker;
	)

#endif // _CLIENTBUILD && _SERVERBUILD
#endif // PLATFORM_SEM

		LTASSERT(pUserData, "Error: Found invalid user data passed into the player rigid body action callback");

	CPlayerRigidBody *pPlayerRigidBody = reinterpret_cast<CPlayerRigidBody*>(pUserData);
	if( !pPlayerRigidBody )
		return;

	// Determine the shape of the box...
	LTVector vPlayerDims;
	g_pPhysicsLT->GetObjectDims( pPlayerRigidBody->m_hObject, &vPlayerDims );

	LTRigidTransform tPlayerTrans;
	g_pLTBase->GetObjectTransform( pPlayerRigidBody->m_hObject, &tPlayerTrans );

	//for now just use a fixed sized list, but when we have a stack allocator, allocate a block big enough
	//to always hold the number of objects
	HPHYSICSRIGIDBODY hContainedObjects[256] = { INVALID_PHYSICS_RIGID_BODY };

	// The above list may contain duplicates.  We don't want to apply a force to a rigid body multiple times
	// so maintain this list of rigid bodies that have been applied a force and compare the current rigid body
	// against this list to determine if it is a duplicate.
	HPHYSICSRIGIDBODY hRigidBodiesUsed[256] = { INVALID_PHYSICS_RIGID_BODY };

	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	uint32 nNumReturnedObjects = 0;
	pILTPhysicsSim->GetRigidBodiesInContainer( pPlayerRigidBody->m_hContainer, hContainedObjects, LTARRAYSIZE(hContainedObjects), nNumReturnedObjects );

	float fForcePerUpdate = pPlayerRigidBody->m_fForce * fUpdateTimeS;

	//now we need to apply the forces on the objects
	for(uint32 nCurrObject = 0; nCurrObject < nNumReturnedObjects; nCurrObject++)
	{
		HPHYSICSRIGIDBODY hBody = hContainedObjects[nCurrObject];
		LTASSERT(hBody != INVALID_PHYSICS_RIGID_BODY, "Error: Found invalid physics rigid body in the list of contained objects");

		// If the rigid body is pinned, we can avoid all force calculations
		bool bPinned;
		pILTPhysicsSim->IsRigidBodyPinned(hBody, bPinned);

		if (!bPinned)
		{
			// The contained object list may have duplicates so ignore any rigid bodies that previously had an impulse applied.
			if( std::find( hRigidBodiesUsed, hRigidBodiesUsed + nCurrObject, hBody ) == hRigidBodiesUsed + nCurrObject )
			{
				hRigidBodiesUsed[ nCurrObject ] = hBody;

				LTVector vClosestPt;
				float fClosestDist;
				bool bUseClosestPoint = bool(s_vtPRBUseClosestPoint.GetFloat() != 0.0f);

				LTVector vPlayerPos = tPlayerTrans.m_vPos;

				if (!bUseClosestPoint || (LT_OK != pILTPhysicsSim->GetClosestPoint( pPlayerRigidBody->m_hRigidBodyACB, hBody, vClosestPt, fClosestDist)))
				{
					// Didn't find the closest point, so use the center of mass...
					pILTPhysicsSim->GetRigidBodyCenterOfMassInWorld(hBody, vClosestPt);
				}

				// Make sure the object is not below or above the player.
				if (   (vPlayerPos.y - vPlayerDims.y) < vClosestPt.y
					&& (vPlayerPos.y + vPlayerDims.y) >= vClosestPt.y )
				{
					vPlayerPos.y = vClosestPt.y = 0.0f; // Do 2d calc only
					LTVector vWSForce = vClosestPt - vPlayerPos;

					// If the rigid body is close enough to the player we may want to move it away...
					if( vPlayerPos.Dist( vClosestPt ) <= s_vtPRBPenetratingDistance.GetFloat( ) )
					{
						if( pPlayerRigidBody->m_hLastPenetratingObject != INVALID_HOBJECT &&
							( pPlayerRigidBody->m_swLastPenetratingTimer.IsStarted( ) && !pPlayerRigidBody->m_swLastPenetratingTimer.IsTimedOut( ) ) )
						{
							HOBJECT hBodyObj = INVALID_HOBJECT;
							pILTPhysicsSim->GetRigidBodyObject( hBody, hBodyObj );

							if( hBodyObj == pPlayerRigidBody->m_hLastPenetratingObject )
							{
								// Still colliding check if it has any velocity...
								LTVector vVel;
								pILTPhysicsSim->GetRigidBodyVelocity( hBody, vVel );
								if( vVel.Mag( ) <= s_vtPRBPenetratingVelocity.GetFloat( ) )
								{
									// Move the object in a random direction.
									LTVector vObjPos;
									g_pLTBase->GetObjectPos( hBodyObj, &vObjPos );

									vObjPos += pPlayerRigidBody->m_vLastPenetratingMoveDir * s_vtPRBPenetratingMoveMag.GetFloat( );
									g_pPhysicsLT->MoveObject( hBodyObj, vObjPos, 0 );

									// Recalculate the closest point...
									if (!bUseClosestPoint || (LT_OK != pILTPhysicsSim->GetClosestPoint( pPlayerRigidBody->m_hRigidBodyACB, hBody, vClosestPt, fClosestDist)))
									{
										// Didn't find the closest point, so use the center of mass...
										pILTPhysicsSim->GetRigidBodyCenterOfMassInWorld(hBody, vClosestPt);
									}
									vClosestPt.y = 0.0f;
								}
							}
						}
						else
						{
							// Randomize a direction to try and move the object away from the player...
							pPlayerRigidBody->m_vLastPenetratingMoveDir.x = GetRandom( -5.0f, 5.0f );
							pPlayerRigidBody->m_vLastPenetratingMoveDir.y = GetRandom( 0.0f, 1.0f );
							pPlayerRigidBody->m_vLastPenetratingMoveDir.z = GetRandom( -5.0f, 5.0f );
							pPlayerRigidBody->m_vLastPenetratingMoveDir.Normalize( );

							// Begin the penetrating counter...
							pPlayerRigidBody->m_swLastPenetratingTimer.Start( s_vtPRBPenetratingTime.GetFloat( ) );

							// Cache the penetrating object so we may try and move it away from the player...
							HOBJECT hBodyObj = INVALID_HOBJECT;
							pILTPhysicsSim->GetRigidBodyObject( hBody, hBodyObj );
							pPlayerRigidBody->m_hLastPenetratingObject = hBodyObj;
						}
					}
					else
					{
						// Object is no longer penetrating so reset the values...
						pPlayerRigidBody->m_hLastPenetratingObject = INVALID_HOBJECT;
						pPlayerRigidBody->m_vLastPenetratingMoveDir = LTVector::GetIdentity( );
						pPlayerRigidBody->m_swLastPenetratingTimer.Stop( );
					}

					// Only apply forces if the closest point is inside our radius (this
					// helps to make the box feel like a cylinder)...
					float fRadius = (vPlayerDims.x * pPlayerRigidBody->m_fRadiusScale);
					if (vWSForce.Mag() <= fRadius)
					{
						float fWSForceMag = vWSForce.Mag( );
						if( fWSForceMag > 0.001f )
						{
							vWSForce /= fWSForceMag;
						}
						else
						{
							vWSForce = tPlayerTrans.m_rRot.Forward();
						}

						// Add some Y force if specified...
						vWSForce.y = pPlayerRigidBody->m_fForceY;

						float fDistRatio = 1.0f; // TBD: Calculate ratio?
						vWSForce *= (fForcePerUpdate * fDistRatio);

						// Don't apply downward forces...
						if (vWSForce.y < 0.0f)
							vWSForce.y = 0.0f;

						// Get the center of mass of the object
						LTVector vCOM;
						pILTPhysicsSim->GetRigidBodyCenterOfMassInWorld(hBody, vCOM);

						//and apply the force through that
						pILTPhysicsSim->ApplyRigidBodyImpulseWorldSpace(hBody, vCOM, vWSForce);
					}
				}
			}
		}

		//and finally, make sure to release our reference to the rigid body so we don't create
		//any resource leaks
		pILTPhysicsSim->ReleaseRigidBody(hBody);
	}
}


// EOF
