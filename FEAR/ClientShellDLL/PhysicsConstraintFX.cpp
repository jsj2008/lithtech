// ----------------------------------------------------------------------- //
//
// MODULE  : CPhysicsConstraintFX.cpp
//
// PURPOSE : Client side physics simulation constraint
//
// CREATED : 05/26/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "PhysicsConstraintFX.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsConstraintFX::CPhysicsConstraintFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CPhysicsConstraintFX::CPhysicsConstraintFX( )
:	m_hConstraint( INVALID_PHYSICS_CONSTRAINT )
,	m_bHasDependentObjects( false )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsConstraintFX::~CPhysicsConstraintFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CPhysicsConstraintFX::~CPhysicsConstraintFX( )
{
	if( m_hConstraint != INVALID_PHYSICS_CONSTRAINT )
	{
		g_pLTClient->PhysicsSim( )->RemoveConstraintFromSimulation( m_hConstraint );
		g_pLTClient->PhysicsSim( )->ReleaseConstraint( m_hConstraint );
		m_hConstraint = INVALID_PHYSICS_CONSTRAINT;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsConstraintFX::Init
//
//  PURPOSE:	Initialize the client side class...
//
// ----------------------------------------------------------------------- //

bool CPhysicsConstraintFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg ))
		return false;

	// Cache the portion of the message with the constraint data for polling in the update...
	m_cInitMsg = pMsg->SubMsg( pMsg->Tell( ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsConstraintFX::Update
//
//  PURPOSE:	Poll the dependent objects until they become available...
//
// ----------------------------------------------------------------------- //

bool CPhysicsConstraintFX::Update( )
{
	if( m_bHasDependentObjects )
		return true;

	if( !CSpecialFX::Update( ))
		return false;

	ILTMessage_Read *pMsg = m_cInitMsg->Clone( );

	m_bHasDependentObjects = true;
	uint32 nConstraintType = static_cast<EConstraintType>(pMsg->ReadBits( FNumBitsExclusive<kConstraintType_NumTypes>::k_nValue ));
	
	bool bHasObject1 = pMsg->Readbool();
	if( bHasObject1 )
		m_bHasDependentObjects &= (pMsg->ReadObject( ) != INVALID_HOBJECT);

	bool bHasObject2 = pMsg->Readbool( );
	if( bHasObject2 )
		m_bHasDependentObjects &= (pMsg->ReadObject( ) != INVALID_HOBJECT);

	if( m_bHasDependentObjects )
	{
		// All the dependent objects are available so read the entire message...
		m_cs.Read( m_cInitMsg );

		CreatePhysicsConstraint( );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsConstraintFX::CreatePhysicsConstraint
//
//  PURPOSE:	Initialize the client side class...
//
// ----------------------------------------------------------------------- //

bool CPhysicsConstraintFX::CreatePhysicsConstraint( )
{
	ILTPhysicsSim *pLTPhysicsSim = g_pLTClient->PhysicsSim( );
	if( !pLTPhysicsSim )
		return false;

	HPHYSICSRIGIDBODY hBody1 = INVALID_PHYSICS_RIGID_BODY;
	if( m_cs.m_hObject1 )
	{
		uint32 nType1 = 0;
		g_pCommonLT->GetObjectType( m_cs.m_hObject1, &nType1 );
		if( nType1 == OT_WORLDMODEL )
		{
			pLTPhysicsSim->GetWorldModelRigidBody( m_cs.m_hObject1, hBody1 );
		}
	}

	HPHYSICSRIGIDBODY hBody2 = INVALID_PHYSICS_RIGID_BODY;
	if( m_cs.m_hObject2 )
	{
		uint32 nType2 = 0;
		g_pCommonLT->GetObjectType( m_cs.m_hObject2, &nType2 );
		if( nType2 == OT_WORLDMODEL )
		{
			pLTPhysicsSim->GetWorldModelRigidBody( m_cs.m_hObject2, hBody2 );
		}
	}

    m_hConstraint = ConstrainRigidbodies( hBody1, hBody2 );

	pLTPhysicsSim->ReleaseRigidBody( hBody1 );
	pLTPhysicsSim->ReleaseRigidBody( hBody2 );

	if( m_hConstraint == INVALID_PHYSICS_CONSTRAINT )
		return false;

	if( pLTPhysicsSim->AddConstraintToSimulation( m_hConstraint ) != LT_OK )
	{
		pLTPhysicsSim->ReleaseConstraint( m_hConstraint );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsConstraintFX::ConstrainRigidbodies
//
//  PURPOSE:	Initialize the client side class...
//
// ----------------------------------------------------------------------- //

HPHYSICSCONSTRAINT CPhysicsConstraintFX::ConstrainRigidbodies( HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2 )
{
	ILTPhysicsSim *pLTPhysicsSim = g_pLTClient->PhysicsSim( );
	if( !pLTPhysicsSim )
		return INVALID_PHYSICS_CONSTRAINT;

	switch( m_cs.m_eConstraintType )
	{
		case kConstraintType_Hinge:
		{
			return pLTPhysicsSim->CreateHinge( hBody1, hBody2, 
											   m_cs.m_vPivotPt1, m_cs.m_vPivotPt2, 
											   m_cs.m_vPivotDir1, m_cs.m_vPivotDir2 );
		}
		break;

		case kConstraintType_LimitedHinge:
		{
			return pLTPhysicsSim->CreateLimitedHinge( hBody1, hBody2,
													  m_cs.m_vPivotPt1, m_cs.m_vPivotPt2,
													  m_cs.m_vPivotDir1, m_cs.m_vPivotDir2,
													  m_cs.m_vPivotPerp1, m_cs.m_vPivotPerp2,
													  m_cs.m_fAngleMin, m_cs.m_fAngleMax,
													  m_cs.m_fFriction );
		}
		break;

		case kConstraintType_Point:
		{
			return pLTPhysicsSim->CreateBallAndSocket( hBody1, hBody2,
													   m_cs.m_vPivotPt1, m_cs.m_vPivotPt2 );
		}
		break;

		case kConstraintType_Prismatic:
		{
			return pLTPhysicsSim->CreatePrismatic( hBody1, hBody2,
												   m_cs.m_vPivotPt1, m_cs.m_vPivotPt2, 
												   m_cs.m_vAxis1, m_cs.m_vAxis2, 
												   m_cs.m_vAxisPerp1, m_cs.m_vAxisPerp2,
												   m_cs.m_fMovementMin, m_cs.m_fMovementMax,
												   m_cs.m_fFriction );
		}
		break;

		case kConstraintType_Ragdoll:
		{	
			return pLTPhysicsSim->CreateRagdoll( hBody1, hBody2,
												 m_cs.m_vPivotPt1, m_cs.m_vPivotPt2, 
												 m_cs.m_vTwist1, m_cs.m_vTwist2, 
												 m_cs.m_vPlane1, m_cs.m_vPlane2,
												 m_cs.m_fConeAngle, m_cs.m_fPosCone,
												 m_cs.m_fNegCone, m_cs.m_fTwistMin,
												 m_cs.m_fTwistMax, m_cs.m_fFriction);
		}
		break;

		case kConstraintType_StiffSpring:
		{
			return pLTPhysicsSim->CreateStiffSpring( hBody1, hBody2,
													 m_cs.m_vPivotPt1, m_cs.m_vPivotPt2,
													 m_cs.m_fDistance );
		}
		break;

		case kConstraintType_Wheel:
		{
			return pLTPhysicsSim->CreateWheel( hBody1, hBody2,
											   m_cs.m_vPivotPt1, m_cs.m_vPivotPt2,
											   m_cs.m_vRotation1, m_cs.m_vRotation2,
											   m_cs.m_vSuspension2, m_cs.m_fSuspensionMin,
											   m_cs.m_fSuspensionMax, m_cs.m_fSuspensionStrength,
											   m_cs.m_fSuspensionDamping );
		}
		break;

		default:
		break;
	}

	return INVALID_PHYSICS_CONSTRAINT;
}

// EOF
