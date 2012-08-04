// ----------------------------------------------------------------------- //
//
// MODULE  : ClientClientPhysicsCollisionMgr.cpp
//
// PURPOSE : Definition of physics collision mgr for server
//
// CREATED : 08/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ClientPhysicsCollisionMgr.h"
#include "CollisionsDB.h"
#include "ClientFXMgr.h"

#ifndef _FINAL
extern void CollisionStatsLevel( uint32& nLevel, bool& bServer, HRECORD& hCollisionPropertyFilter );
#endif // _FINAL

// Callback function to handle all server rigid body collisions...
static void GlobalRigidBodyCollisionCB(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
									   const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
									   float fVelocity, bool& bIgnoreCollision, void* pUser)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	CollisionData collisionData;
	collisionData.hBodyA = hBody1;
	collisionData.hBodyB = hBody2;
	g_pLTBase->PhysicsSim()->IsRigidBodyPinned(hBody1, collisionData.bIsPinnedA);
	g_pLTBase->PhysicsSim()->IsRigidBodyPinned(hBody2, collisionData.bIsPinnedB);
	collisionData.vCollisionPt = vCollisionPt;
	collisionData.vCollisionNormal = vCollisionNormal;
	collisionData.fVelocity = fVelocity;
	ClientPhysicsCollisionMgr::Instance().HandleRigidBodyCollision( collisionData );
}


ClientPhysicsCollisionMgr::ClientPhysicsCollisionMgr( )
{
	// Pre-allocate our list of sounds responses.
	static uint32 const nMaxSounds = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
		DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), MaxSounds );
	m_lstSoundCollisionResponses.reserve( nMaxSounds );
}

ClientPhysicsCollisionMgr::~ClientPhysicsCollisionMgr( )
{
	Term( );
}

void ClientPhysicsCollisionMgr::PostStartWorld( )
{
	PhysicsCollisionMgr::PostStartWorld();

	g_pLTBase->PhysicsSim()->SetGlobalCollisionListener(GlobalRigidBodyCollisionCB, NULL);
}


void ClientPhysicsCollisionMgr::Term( )
{
	PhysicsCollisionMgr::Term( );

	// Make sure all the sounds are cleared out.
	SoundCollisionResponses::iterator iter = m_lstSoundCollisionResponses.begin( );
	SoundCollisionResponses::iterator end = m_lstSoundCollisionResponses.end( );
	for( ; iter != end; iter++ )
	{
		CollisionResponse* pCollisionResponse = *iter;
		if( pCollisionResponse->GetSound( ))
		{
			g_pLTBase->SoundMgr()->KillSound( pCollisionResponse->GetSound( ));
			pCollisionResponse->SetSound( NULL );
		}
	}

	m_lstSoundCollisionResponses.clear( );
}

void ClientPhysicsCollisionMgr::HandlePhysicsMessage( ILTMessage_Read *pMsg )
{
	// Server sent the client a collision event so 
	// the client can do clientside tasks.
	CollisionData collisionData;
	collisionData.bFromServer = true;
	collisionData.hBodyA = ( HPHYSICSRIGIDBODY )pMsg->Readuint32( );
	collisionData.hBodyB = ( HPHYSICSRIGIDBODY )pMsg->Readuint32( );
	if( collisionData.hBodyA != INVALID_PHYSICS_RIGID_BODY )
		collisionData.hObjectA = pMsg->ReadObject( );
	if( collisionData.hBodyB != INVALID_PHYSICS_RIGID_BODY )
		collisionData.hObjectB = pMsg->ReadObject( );
	collisionData.hCollisionPropertyA = g_pLTDatabase->GetRecordByIndex( DATABASE_CATEGORY( CollisionProperty ).GetCategory( ),
		pMsg->Readuint8( ));
	collisionData.hCollisionPropertyB = g_pLTDatabase->GetRecordByIndex( DATABASE_CATEGORY( CollisionProperty ).GetCategory( ),
		pMsg->Readuint8( ));
	collisionData.fImpulse = pMsg->Readfloat( );
	collisionData.vCollisionPt = pMsg->ReadLTVector( );
	collisionData.vCollisionNormal = pMsg->ReadLTVector( );

	HandleRigidBodyCollision( collisionData );
}

bool ClientPhysicsCollisionMgr::StartResponses( CollisionResponse& collisionResponse, CollisionData& collisionData )
{
	StartSoundResponse( collisionResponse );
	StartClientFXResponse( collisionResponse );

	return true;
}

bool ClientPhysicsCollisionMgr::DoSoundResponse( CollisionResponse& collisionResponse, HRECORD hSoundDB, uint8 nVolume )
{
	// Check if the maximum number of sounds are already playing.  We may be able
	// to override a playing sound if our volume is bigger.
	static uint32 const nMaxSounds = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
		DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), MaxSounds );
	if( m_lstSoundCollisionResponses.size( ) == nMaxSounds )
	{
		SoundCollisionResponses::iterator iter = m_lstSoundCollisionResponses.begin( );
		SoundCollisionResponses::iterator end = m_lstSoundCollisionResponses.end( );
		for( ; iter != end; iter++ )
		{
			CollisionResponse* pCollisionResponse = *iter;
			// Check if the other guy isn't playing a sound.
			if( !pCollisionResponse->GetSound( ))
				continue;
			// Check if the current sound's volume is greater than the new volume.
			if( pCollisionResponse->GetSoundVolume( ) >= nVolume )
				continue;

			// Kill the other sound and take its slot.
			m_lstSoundCollisionResponses.erase( iter );
			g_pLTBase->SoundMgr()->KillSound( pCollisionResponse->GetSound( ));
			pCollisionResponse->SetSound( NULL );
			break;
		}

		// Check if we didn't find any sounds to override.
		if( m_lstSoundCollisionResponses.size( ) == nMaxSounds )
			return false;
	}

	// Play the sound.
	HLTSOUND hSound = g_pClientSoundMgr->PlayDBSoundFromPos( collisionResponse.GetCollisionPoint( ), hSoundDB, 
		SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, PLAYSOUND_GETHANDLE | PLAYSOUND_TIME, nVolume );
	if( !hSound )
		return false;

	collisionResponse.SetSound( hSound );
	collisionResponse.SetSoundVolume( nVolume );

#ifndef _FINAL
	uint32 nStatsLevel;
	bool bServer;
	HRECORD hCollisionPropertyFilter = NULL;
	CollisionStatsLevel( nStatsLevel, bServer, hCollisionPropertyFilter );
	if( nStatsLevel > 0 &&
		( !hCollisionPropertyFilter || hCollisionPropertyFilter == collisionResponse.GetCollisionProperty( )))
	{
		g_pLTBase->CPrint( "C:  SoundRecord(%s) volume(%d)", g_pLTDatabase->GetRecordName( hSoundDB ), nVolume );
	}
#endif _FINAL

	m_lstSoundCollisionResponses.push_back( &collisionResponse );

	return true;
}

bool ClientPhysicsCollisionMgr::DoClientFXResponse( CollisionResponse& collisionResponse, HATTRIBUTE hStruct, uint32 nIndexFound, 
												   char const* pszClientFx )
{
	// Check if it's supposed to be attached.  Only allow attachment if we have an object.
	bool bAttached = false;
	if( collisionResponse.GetCollisionActor( )->GetObject( ))
	{
		bAttached = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( ClientFX, hStruct, nIndexFound, Attached );
	}

	LTRigidTransform tTransform;
	if( bAttached )
	{
		tTransform.Init( );
	}
	// Otherwise use the collision point and normal.
	else
	{
		tTransform.m_vPos = collisionResponse.GetCollisionPoint( );
		// Send the collision normal as the forward and up, so rotation calculates correct
		// orthogonal vectors.
		tTransform.m_rRot = LTRotation( collisionResponse.GetCollisionNormal( ), collisionResponse.GetCollisionNormal( ));
	}

	CLIENTFX_CREATESTRUCT fxcs( pszClientFx, 0, ( bAttached ) ? collisionResponse.GetCollisionActor( )->GetObject( ) : NULL, tTransform );
	fxcs.m_bUseTargetData	= false;
	fxcs.m_hTargetObject	= NULL;
	fxcs.m_vTargetOffset.Init();

	g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxcs, true);

#ifndef _FINAL
	uint32 nStatsLevel;
	bool bServer;
	HRECORD hCollisionPropertyFilter = NULL;
	CollisionStatsLevel( nStatsLevel, bServer, hCollisionPropertyFilter );
	if( nStatsLevel > 0 &&
		( !hCollisionPropertyFilter || 
		( hCollisionPropertyFilter == collisionResponse.GetCollisionProperty())))
	{
		g_pLTBase->CPrint( "C:  Clientfx(%s)", pszClientFx );
	}
#endif // _FINAL

	return true;
}

void ClientPhysicsCollisionMgr::StopSound( CollisionResponse& collisionResponse )
{
	if( !collisionResponse.GetSound( ))
		return;

	g_pLTBase->SoundMgr()->KillSound( collisionResponse.GetSound( ));
	collisionResponse.SetSound( NULL );
	SoundCollisionResponses::iterator iter = m_lstSoundCollisionResponses.begin( );
	SoundCollisionResponses::iterator end = m_lstSoundCollisionResponses.end( );
	for( ; iter != end; iter++ )
	{
		if( &collisionResponse == *iter )
		{
			m_lstSoundCollisionResponses.erase( iter );
			break;
		}
	}
}

