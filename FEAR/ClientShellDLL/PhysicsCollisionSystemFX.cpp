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

//
// Includes...
//

#include "stdafx.h"
#include "PhysicsCollisionSystemFX.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsCollisionSystemFX::CPhysicsCollisionSystemFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CPhysicsCollisionSystemFX::CPhysicsCollisionSystemFX( )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsCollisionSystemFX::~CPhysicsCollisionSystemFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CPhysicsCollisionSystemFX::~CPhysicsCollisionSystemFX( )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsCollisionSystemFX::Init
//
//  PURPOSE:	Initialize the client side class...
//
// ----------------------------------------------------------------------- //

bool CPhysicsCollisionSystemFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg ))
		return false;

	// Cache the portion of the message with the constraint data for polling in the update...
	m_cInitMsg = pMsg->SubMsg( pMsg->Tell( ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsCollisionSystemFX::Update
//
//  PURPOSE:	Poll the dependent objects until they become available...
//
// ----------------------------------------------------------------------- //

bool CPhysicsCollisionSystemFX::Update( )
{
	if( !CSpecialFX::Update( ))
		return false;

	ILTMessage_Read *pMsg = m_cInitMsg->Clone( );

	// Read the message to see if all dependent objects are available...
	uint32 nNumObjects = pMsg->Readuint32( );

	for( uint32 nObject = 0; nObject < nNumObjects; ++nObject )
	{
		// If any of the objects are invalid we can't create the system.
		// Try again next update...
		if( pMsg->ReadObject( ) == INVALID_HOBJECT )
			return true;
	}

	CreatePhysicsCollisionSystem( );
	
	// After the collision system has been setup this object is no longer needed...
	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPhysicsCollisionSystemFX::CreatePhysicsCollisionSystem
//
//  PURPOSE:	Actually create the collision system and apply it to all specified objects...
//
// ----------------------------------------------------------------------- //

bool CPhysicsCollisionSystemFX::CreatePhysicsCollisionSystem( )
{
	ILTPhysicsSim *pLTPhysicsSim = g_pLTClient->PhysicsSim( );
	if( !pLTPhysicsSim )
		return false;

	// Obtain a new, unique system group ID that will be set for all objects within the system...
	uint32 nSystem = 0;
	pLTPhysicsSim->GetUniqueSystemGroup( nSystem );

	// Run through all objects in the message and set their system ID...
	uint32 nNumObjects = m_cInitMsg->Readuint32( );
	for( uint32 nObject = 0; nObject < nNumObjects; ++nObject )
	{
		HOBJECT hObject = m_cInitMsg->ReadObject( );
		if( hObject == INVALID_HOBJECT )
			continue;
		
		HPHYSICSRIGIDBODY hRigidBody = INVALID_PHYSICS_RIGID_BODY;
		if( pLTPhysicsSim->GetWorldModelRigidBody( hObject, hRigidBody ) != LT_OK)
			continue;

		pLTPhysicsSim->SetRigidBodyCollisionSystem( hRigidBody, nSystem );
		pLTPhysicsSim->ReleaseRigidBody( hRigidBody );
	}

	return true;
}

// EOF
