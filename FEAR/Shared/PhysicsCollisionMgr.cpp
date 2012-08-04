// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsCollisionMgr.cpp
//
// PURPOSE : PhysicsCollisionMgr - Implementation
//
// CREATED : 07/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PhysicsCollisionMgr.h"
#include "iltphysicssim.h"			// for HPHYSICSRIGIDBODY
#include "iltcsbase.h"				// for ILTCSBase
#include "SurfaceFunctions.h"		// for GetSurfaceType()
#include "SoundDB.h"				// for g_pSoundDB
#include "PhysicsUtilities.h"		// for ePhysicsGroup_UserXXX
#include "CollisionsDB.h"
#include <algorithm>
#include "VarTrack.h"
#include "iperformancemonitor.h"

#ifdef _CLIENTBUILD
VarTrack s_vtcPhysicsCollisionStats;
VarTrack s_vtcPhysicsCollisionStatsFilter;
VarTrack s_vtcPhysicsCollisionEnable;
CTimedSystem g_tsClientGamePhysicsCollisionMgr("GameClient_PhysicsCollisionMgr", "GameClient");
#endif // _CLIENTBUILD
#ifdef _SERVERBUILD
VarTrack s_vtsPhysicsCollisionStats;
VarTrack s_vtsPhysicsCollisionStatsFilter;
VarTrack s_vtsPhysicsCollisionEnable;
CTimedSystem g_tsServerGamePhysicsCollisionMgr("GameServer_PhysicsCollisionMgr", "GameServer");
#endif // _SERVERBUILD

// Maximum impulse that can be generated due to a collision.
static float s_fMaxImpulse = 1000000.0f;

#ifndef _FINAL
void CollisionStatsLevel( uint32& nLevel, bool& bServer, HRECORD& hCollisionPropertyFilter )
{
	CLIENT_CODE
	(
		bServer = false;
		nLevel = ( uint32 )s_vtcPhysicsCollisionStats.GetFloat( );
		hCollisionPropertyFilter = NULL;
		char const* pszCollisionPropertyFilter = s_vtcPhysicsCollisionStatsFilter.GetStr( );
		if( !LTStrEmpty( pszCollisionPropertyFilter ))
		{
			hCollisionPropertyFilter = DATABASE_CATEGORY( CollisionProperty ).GetRecordByName( pszCollisionPropertyFilter );
		}
	)

	SERVER_CODE
	(
		bServer = true;
		nLevel = ( uint32 )s_vtsPhysicsCollisionStats.GetFloat( );
		hCollisionPropertyFilter = NULL;
		char const* pszCollisionPropertyFilter = s_vtsPhysicsCollisionStatsFilter.GetStr( );
		if( !LTStrEmpty( pszCollisionPropertyFilter ))
		{
			hCollisionPropertyFilter = DATABASE_CATEGORY( CollisionProperty ).GetRecordByName( pszCollisionPropertyFilter );
		}
	)
}
#endif // _FINAL

//////////////////////////////////////////////////////////////////////////
// Function name   : FindResponsesIndex
// Description     : Finds an index into the list of responses structs
//						by the matching the collisionpropertythem to the
//						WhenHitBy property of the structs.
// Return type     : static uint32 - Index into Responses struct.
// Argument        : HATTRIBUTE hResponses - Responses struct.
// Argument        : HRECORD hCollisionPropertyUs - Our collisionproperty record.
// Argument        : HRECORD hCollisionPropertyThem - The other collisionproperty record.
//////////////////////////////////////////////////////////////////////////
static uint32 FindResponsesIndex( HATTRIBUTE hResponses, HRECORD hCollisionPropertyUs, HRECORD hCollisionPropertyThem )
{
	// Iterate through all the responses structs and look at the WhenHitBy attribute.  If it
	// matches the them input, then use that specific responses struct.  Otherwise, use the responses
	// struct that is set to NULL.  If that doesn't exist, then there's an error.
	uint32 nResponsesIndex = (uint32)-1;
	uint32 nNumResponses = g_pLTDatabase->GetNumValues( hResponses );
	for( uint32 i = 0; i < nNumResponses; i++ )
	{
		// Get the whenhitby recordlink.
		HRECORD hWhenHitBy = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( Responses, hResponses, i, WhenHitBy );

		// If not set, then this is the default response to use.  
		if( !hWhenHitBy )
		{
			nResponsesIndex = i;

			// If we have a valid them, then continue to look for a specific match.
			if( hCollisionPropertyThem )
				continue;
			else
				break;
		}

		// Found an exact match, use this index.
		if( hWhenHitBy == hCollisionPropertyThem )
		{
			nResponsesIndex = i;
			break;
		}
	}

	return nResponsesIndex;
}



//////////////////////////////////////////////////////////////////////////
// Function name   : void CalculateRigidBodyPhysicsData
// Description     : Helper function for CalculateImpulse
// Return type     : none
// Argument        : HPHYSICSRIGIDBODY hRigidBody - RigidBody to get data from.
// Argument        : bool bIsPinned - whether this rigid body is pinned or not.
// Argument        : CollisionData const& collisionData - collision data.
// Argument        : LTVector& vDenominator - Resulting denominator.
// Argument        : float& fInvMass - resulting inverse mass.
//////////////////////////////////////////////////////////////////////////
static inline void CalculateRigidBodyPhysicsData( HPHYSICSRIGIDBODY hRigidBody, bool bIsPinned,
												 CollisionData const& collisionData,
												 LTVector& vDenominator, float& fInvMass  )
{
	// Get physical properties about rigid body.
	fInvMass = 0.0f;
	if( !hRigidBody || bIsPinned )
	{
		vDenominator.Init( );
		return;
	}

	LTVector vCenterOfMass;
	g_pLTBase->PhysicsSim()->GetRigidBodyCenterOfMassInWorld( hRigidBody, vCenterOfMass );
	LTVector vRelOffset = collisionData.vCollisionPt - vCenterOfMass;

	LTMatrix3x4 matInertiaInv;
	g_pLTBase->PhysicsSim()->GetRigidBodyInertiaInvWorld( hRigidBody, matInertiaInv );
	LTVector vRelOffsetProj = vRelOffset.Cross( collisionData.vCollisionNormal );
	LTVector vInertiaProj = matInertiaInv * vRelOffsetProj;
	vDenominator = vInertiaProj.Cross( vRelOffset );

	HPHYSICSSHAPE hShape = NULL;
	g_pLTBase->PhysicsSim()->GetRigidBodyShape( hRigidBody, hShape );
	if( hShape )
	{
		float fMass = 1.0f;
		if( g_pLTBase->PhysicsSim()->GetShapeMass( hShape, fMass ) == LT_OK && fMass != 0.0f )
			fInvMass = 1.0f / fMass;

		g_pLTBase->PhysicsSim()->ReleaseShape( hShape );
		hShape = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name   : CalculateImpulse
// Description     : Calculate the impulse of collision between 2 rigid bodies.  This formula 
//						comes from the Havok sdk help under the heading "16. How do I 
//						approximate the impulse applied to a collidable at a contact point?"
//						Havok got it from the following research paper: 
//						http://www-2.cs.cmu.edu/~baraff/sigcourse/notesd2.pdf
// Return type     : static float - Resulting impulse.
// Argument        : HPHYSICSRIGIDBODY hRigidBodyA - first rigid body in collision.
// Argument        : float fHardnessA - Hardness of first rigid body.
// Argument        : HPHYSICSRIGIDBODY hRigidBodyB - second rigid body in collision.
// Argument        : float fHardnessB - Hardness of second rigid body.
// Argument        : CollisionData const& collisionData - collection of data regarding collision.
//////////////////////////////////////////////////////////////////////////
static float CalculateImpulse( HPHYSICSRIGIDBODY hRigidBodyA, float fHardnessA, HPHYSICSRIGIDBODY hRigidBodyB, 
							  float fHardnessB, CollisionData const& collisionData )
{
	// Calculate the joint coef. of restitution for the collision.
	float fCoeffOfRestitution = LTSqrt( fHardnessA * fHardnessB );

	// Numerator for impulse formula.
	float fNumerator = -( 1.0f + fCoeffOfRestitution ) * collisionData.fAbsVelocity;
	if( fNumerator == 0.0f )
		return 0.0f;

	// Get physical properties about the rigid bodies.
	LTVector vDenominatorA;
	float fInvMassA;
	CalculateRigidBodyPhysicsData( hRigidBodyA, collisionData.bIsPinnedA, collisionData, vDenominatorA, fInvMassA );
	LTVector vDenominatorB;
	float fInvMassB;
	CalculateRigidBodyPhysicsData( hRigidBodyB, collisionData.bIsPinnedB, collisionData, vDenominatorB, fInvMassB );

	// Calculate the denominator for the impulse formula.
	LTVector vDenominator = vDenominatorA + vDenominatorB;
	float fDenominator = fInvMassA + fInvMassB + collisionData.vCollisionNormal.Dot( vDenominator );

	// Calculate the final impulse.  If the denominator is very small, we need to use 
	// limit value.
	float fImpulse;
	if( fDenominator < fabs( fNumerator ) / s_fMaxImpulse )
	{
		fImpulse = s_fMaxImpulse;
	}
	else
	{
		fImpulse = fNumerator / fDenominator;
	}

	return fImpulse;
}



//////////////////////////////////////////////////////////////////////////
// Function name   : FindCollisionProperty
// Description     : Finds a CollsionProperty record by checking
//						the user flags for a collision property override.
//						If none found, then it checks for surface override.  
//						If that surface points at a collisionproperty it is used.
//						If not, then it checks if it's a worldmodel, and checks
//						for a surface type from the poly hit and then gets the collisionproperty.
// Return type     : static HRECORD - CollisionProperty record found.
// Argument        : HOBJECT hObject - object to inspect.
// Argument        : HPHYSICSRIGIDBODY hRigidBody - Rigid body to inspect, only used if hObject in NULL.
// Argument        : LTVector const& vCollisionPoint - Point of collision.
// Argument        : LTVector const& vCollisionNormal - Normal of collision.
//////////////////////////////////////////////////////////////////////////
static HRECORD FindCollisionProperty( HOBJECT hObject, 
									 HPHYSICSRIGIDBODY hRigidBody, 
									 LTVector const& vCollisionPoint, 
									 LTVector const& vCollisionNormal )
{
	// Get the object of the rigid body.
	HSURFACE hSurf = NULL;
	if( !hObject && hRigidBody != INVALID_PHYSICS_RIGID_BODY )
	{
		g_pLTBase->PhysicsSim( )->GetRigidBodyObject( hRigidBody, hObject );
	}

	if( hObject )
	{
		// First check to see if the object has a collisionproperty override.
		uint32 nUserFlags;
		g_pCommonLT->GetObjectFlags( hObject, OFT_User, nUserFlags );
		HRECORD hCollisionProperty = UserFlagToCollisionPropertyRecord( nUserFlags );
		if( hCollisionProperty )
			return hCollisionProperty;

		// Now check if the object has a surfacetype override.
		SurfaceType eSurfaceType = UserFlagToSurface( nUserFlags );
		if( eSurfaceType != ST_UNKNOWN )
		{
			hSurf = g_pSurfaceDB->GetSurface( eSurfaceType );
		}
	}

	// If we don't have a surface type yet, then we can try to get one from
	// the polygons.  This only works if the HOBJECT is NULL, which means the
	// rigid body is the main world, or if the HOBJECT is a worldmodel.
	if( !hSurf && ( !hObject || GetObjectType( hObject ) == OT_WORLDMODEL ))
	{
		// Find the polygon hit and get the surface info from that.
		IntersectInfo iInfo;
		IntersectQuery qInfo;
		LTVector vExpandedCollisionNormal = vCollisionNormal * 5.0f;
		qInfo.m_From		= vCollisionPoint + vExpandedCollisionNormal;
		qInfo.m_To			= vCollisionPoint - vExpandedCollisionNormal;

		// If we don't have a null, then that means we are hitting the world.
		if( hObject && GetObjectType( hObject ) == OT_WORLDMODEL )
		{
			qInfo.m_Flags		= INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
			qInfo.m_FilterFn	= ExceptObjectFilterFn;
			qInfo.m_pUserData	= hObject;
		}
		else
		{
			qInfo.m_Flags		= INTERSECT_HPOLY | IGNORE_NONSOLID;
		}

		// Try to hit the object going the first direction.
		if( !g_pLTBase->IntersectSegmentAgainst( qInfo, &iInfo, hObject ))
		{
			// Didn't find in the first direction, try the opposite way.
			qInfo.m_From		= qInfo.m_To;
			qInfo.m_To			= vCollisionPoint + vExpandedCollisionNormal;
			if( !g_pLTBase->IntersectSegmentAgainst( qInfo, &iInfo, hObject ))
			{
				// Couldn't find the poly, so no surface.
				return NULL;
			}
		}

		// Resolve the surfacetype to the HSURFACE.
		SurfaceType eSurfaceType = GetSurfaceType( iInfo );
		if( eSurfaceType != ST_UNKNOWN )
		{
			hSurf = g_pSurfaceDB->GetSurface( eSurfaceType );
		}
	}

	// Couldn't find a surface to get the collisionproperty.
	if( !hSurf )
		return NULL;

	// Get the collisionproperty from the surface.
	return g_pSurfaceDB->GetRecordLink( hSurf, SrfDB_Srf_rCollisionProperty );
}


bool CollisionResponse::Init( PhysicsCollisionMgr* pPhysicsCollisionMgr, CollisionActor& collisionActor, bool bWeArA,
							 CollisionData& collisionData )
{
	// Start fresh.
	Term( );

	m_pPhysicsCollisionMgr = pPhysicsCollisionMgr;

#ifndef _FINAL
	uint32 nStatsLevel;
	bool bServer;
	HRECORD hCollisionPropertyFilter = NULL;
	CollisionStatsLevel( nStatsLevel, bServer, hCollisionPropertyFilter );
#endif // _FINAL

	// Figure out which collisionproperty is us and them from collisiondata.
	HRECORD hCollisionPropertyThem;
	if( bWeArA )
	{
		m_hCollisionProperty = collisionData.hCollisionPropertyA;
		hCollisionPropertyThem = collisionData.hCollisionPropertyB;
	}
	else
	{
		m_hCollisionProperty = collisionData.hCollisionPropertyB;
		hCollisionPropertyThem = collisionData.hCollisionPropertyA;
	}

	// Get our responses struct based on who we hit.
	m_hResponses = g_pLTDatabase->GetAttributeByIndex( m_hCollisionProperty, pPhysicsCollisionMgr->GetResponsesAttributeIndex( ));
	if( !m_hResponses )
	{
		LTERROR( "Invalid CollisionProperty.  No responses specified." );
		return false;
	}
	m_nResponsesIndex = FindResponsesIndex( m_hResponses, m_hCollisionProperty, hCollisionPropertyThem );
	if( m_nResponsesIndex == ( uint32 )-1 )
	{
		LTERROR( "Invalid CollisionProperty.  No default response specified." );
		return false;
	}

	// Get the default duration of this actor.
	m_fDuration = DATABASE_CATEGORY( CollisionProperty ).GETRECORDATTRIB( m_hCollisionProperty, Duration );

#ifndef _FINAL
	if( nStatsLevel > 0 && 
		( !hCollisionPropertyFilter || 
		hCollisionPropertyFilter == m_hCollisionProperty ))
	{
		g_pLTBase->CPrint( "%s:  Duration(%.3f)", bServer ? "S" : "C", m_fDuration );
	}
#endif // _FINAL

	m_pCollisionActor = &collisionActor;
	m_vCollisionPoint = collisionData.vCollisionPt;
	m_vCollisionNormal = collisionData.vCollisionNormal;
	m_fImpulse = collisionData.fImpulse;
	m_fStartTime = SimulationTimer::Instance( ).GetTimerAccumulatedS( );

	// Make our responses happen.
	m_pPhysicsCollisionMgr->StartResponses( *this, collisionData );

	return true;
}

void CollisionResponse::Term( )
{
	if( m_pPhysicsCollisionMgr )
	{
		if( GetSound( ))
		{
			m_pPhysicsCollisionMgr->StopSound( *this );
		}
		m_pPhysicsCollisionMgr = NULL;
	}
	m_pCollisionActor = NULL;
}

bool CollisionResponse::IsDone( ) const 
{ 
	// If we're not valid, then consider ourselves done.
	if( !IsValid( ))
		return true;
	// If we have a sound, then we're not done until it's done playing.
	if( GetSound( ))
	{
		bool bDone = true;
		g_pLTBase->SoundMgr()->IsSoundDone( GetSound( ), bDone );
		if( !bDone )
			return false;
	}

	// Check if our timer timed out.
	return ( SimulationTimer::Instance( ).GetTimerAccumulatedS( ) > ( m_fStartTime + GetDuration()));
}

bool CollisionActor::Init( bool bWeAreA, CollisionData& collisionData, CollisionActor const& otherCollisionActor )
{
	// Start fresh.
	if(( m_hRigidBody != INVALID_PHYSICS_RIGID_BODY ) || m_hObject )
		Term( );

	// Get the rigid body that belongs to us.
	m_hRigidBody = bWeAreA ? collisionData.hBodyA : collisionData.hBodyB;

	// Add a reference to it if we didn't get it from the server.  The
	// server rigidbody handles are just for bookkeeping and
	// can't be dereferenced.
	if( !collisionData.bFromServer && m_hRigidBody != INVALID_PHYSICS_RIGID_BODY )
	{
		g_pLTBase->PhysicsSim( )->AddRigidBodyRef( m_hRigidBody );
	}

	if( !collisionData.bFromServer )
	{
		// Get the object associated with this rigidbody.
		HOBJECT hObject = NULL;

		// Try to get the object from the rigidbody.
		if( m_hRigidBody != INVALID_PHYSICS_RIGID_BODY )
		{
			g_pLTBase->PhysicsSim( )->GetRigidBodyObject( m_hRigidBody, hObject );
			m_hObject = hObject;
		}

		if( bWeAreA )
		{
			//if an object was passed in, use that
			if (collisionData.hObjectA)
			{
				m_hObject = collisionData.hObjectA;
			}
			else
			{
				//otherwise update the collision data
				collisionData.hObjectA = m_hObject;
			}
		}
		else
		{
			//if an object was passed in, use that
			if (collisionData.hObjectB)
			{
				m_hObject = collisionData.hObjectB;
			}
			else
			{
				//otherwise update the collision data
				collisionData.hObjectB = m_hObject;
			}
		}

		if (!m_hObject)
		{
			//no object... no actor
			return false;
		}
	}
	else
	{
		// Server tells us what object to use.
		m_hObject = bWeAreA ? collisionData.hObjectA : collisionData.hObjectB;
	}

	m_bFromServer = collisionData.bFromServer;

	m_pOtherCollisionActor = &otherCollisionActor;

	return true;
}

void CollisionActor::Term( )
{
	m_hObject = NULL;

	// Release the rigid body if not from server.
	if( !m_bFromServer )
	{
		if( m_hRigidBody != INVALID_PHYSICS_RIGID_BODY )
		{
			g_pLTBase->PhysicsSim( )->ReleaseRigidBody( m_hRigidBody );
			m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
		}
	}

	for( uint32 nIndex = 0; nIndex < MAX_RESPONSES; nIndex++ )
	{
		m_aCollisionResponses[nIndex].Term( );
	}

	m_pOtherCollisionActor = NULL;
}

bool CollisionActor::HandleCollision( PhysicsCollisionMgr* pPhysicsCollisionMgr, bool bWeAreA, 
									 CollisionData& collisionData )
{
	if( !IsValid( ))
		return false;

#ifndef _FINAL
	uint32 nStatsLevel;
	bool bServer;
	HRECORD hCollisionPropertyFilter = NULL;
	CollisionStatsLevel( nStatsLevel, bServer, hCollisionPropertyFilter );
#endif // _FINAL

#ifndef _FINAL
	if( nStatsLevel > 0 && 
		( !hCollisionPropertyFilter || 
		hCollisionPropertyFilter == ( bWeAreA ? collisionData.hCollisionPropertyA : collisionData.hCollisionPropertyB )))
	{
		g_pLTBase->CPrint( "%s: Responses from CollisionProperty%s(%s)", bServer ? "S" : "C", bWeAreA ? "A" : "B",
			g_pLTDatabase->GetRecordName( bWeAreA ? collisionData.hCollisionPropertyA : collisionData.hCollisionPropertyB ) );
	}
#endif //_FINAL

	// See if this actor has any open responses it can use.
	uint32 nIndexLowestImpulse = 0;
	uint32 nIndex;
	for( nIndex = 0; nIndex < MAX_RESPONSES; nIndex++ )
	{
		if( m_aCollisionResponses[nIndex].IsDone( ))
			break;
		if( m_aCollisionResponses[nIndexLowestImpulse].GetImpulse() < m_aCollisionResponses[nIndex].GetImpulse( ))
			nIndexLowestImpulse = nIndex;
	}
	// No open responses, override the lowest impulse response if this new response has a bigger impulse.
	if( nIndex == MAX_RESPONSES )
	{

#ifndef _FINAL
		if( m_aCollisionResponses[nIndexLowestImpulse].GetImpulse() >= collisionData.fImpulse )
		{
			if( nStatsLevel > 0 && 
				( !hCollisionPropertyFilter ||
				hCollisionPropertyFilter == ( bWeAreA ? collisionData.hCollisionPropertyA : collisionData.hCollisionPropertyB )))
			{
				g_pLTBase->CPrint( "%s:  No response created.  No free responses.", bServer ? "S" : "C" );
			}
			return false;
		}
#endif //_FINAL

		nIndex = nIndexLowestImpulse;
	}

	// Initialize the response.  If we are overriding a slot, init will term the
	// old data first.
	if( !m_aCollisionResponses[nIndex].Init( pPhysicsCollisionMgr, *this, bWeAreA, collisionData ))
		return false;

	return true;
}

bool CollisionActor::Update( )
{ 
	// Done if we are invalid.
	if( !IsValid( ))
		return false;

	// Check each of the collision responses to see if any is still not done.
	bool bDone = true;
	for( uint32 nIndex = 0; nIndex < MAX_RESPONSES; nIndex++ )
	{
		// If it is done, terminate to make room for more.
		if( m_aCollisionResponses[nIndex].IsDone( ))
		{
			m_aCollisionResponses[nIndex].Term( );
		}
		else
		{
			bDone = false;
		}
	}

	return !bDone;
}

bool CollisionActor::IsDone( ) const 
{ 
	// Done if we are invalid.
	if( !IsValid( ))
		return true;

	// Check each of the collision responses to see if any is still not done.
	for( uint32 nIndex = 0; nIndex < MAX_RESPONSES; nIndex++ )
	{
		if( !m_aCollisionResponses[nIndex].IsDone( ))
			return false;
	}

	return true;
}

void CollisionActor::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	// If our object goes invalid, then terminate, since
	// we can't assume any of our responses are still valid.
	// As this is an engine callback, we need to make sure we activate 
	// the correct shell.  If we don't, we will crash if the Term 
	// function uses engine interfaces.

	#ifdef _CLIENTBUILD
	if ( eExecutionShellContext_Client == m_eExecutionShell )
	{
		CGameClientShell::CClientShellScopeTracker  cScopeTracker;
		Term( );
	}
	#endif

	#ifdef _SERVERBUILD
	if ( eExecutionShellContext_Server == m_eExecutionShell )
	{
		CGameServerShell::CServerShellScopeTracker  cScopeTracker;
		Term( );
	}
	#endif
}

bool CollisionPair::Init( CollisionData& collisionData )
{
	// Need to call init on both actors before starting, since they can access each other internally.
	m_CollisionActor[0].Init( true, collisionData, m_CollisionActor[1] );
	m_CollisionActor[1].Init( false, collisionData, m_CollisionActor[0] );

	// If either of the CollisionActors is invalid, then this CollisionPair is invalid.
	if( !m_CollisionActor[0].IsValid( ) || !m_CollisionActor[1].IsValid( ))
		return false;

	return true;
}

bool CollisionPair::HandleCollision( PhysicsCollisionMgr* pPhysicsCollisionMgr, CollisionData& collisionData )
{
	if( !IsValid( ))
		return false;

	// If this isn't from the server, then we need to determine the collisionproperty records.
	if( !collisionData.bFromServer )
	{
		// Get the default collisionproperty if not able to find in the object or surface types.
		static HRECORD hDefault = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
			DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), Default );

		// Get the collisionproperty's for the objects.
		if( !collisionData.hCollisionPropertyA )
			collisionData.hCollisionPropertyA = FindCollisionProperty( m_CollisionActor[0].GetObject(), m_CollisionActor[0].GetRigidBody(), 
																	collisionData.vCollisionPt, collisionData.vCollisionNormal );
		if( !collisionData.hCollisionPropertyA )
			collisionData.hCollisionPropertyA = hDefault;

		if( !collisionData.hCollisionPropertyB )
			collisionData.hCollisionPropertyB = FindCollisionProperty( m_CollisionActor[1].GetObject(), m_CollisionActor[1].GetRigidBody(), 
																	collisionData.vCollisionPt, collisionData.vCollisionNormal );
		if( !collisionData.hCollisionPropertyB )
			collisionData.hCollisionPropertyB = hDefault;
	}

	// Get the hardness from the collisions.
	HATTRIBUTE hHardnessAttribute = g_pLTDatabase->GetAttributeByIndex( 
		collisionData.hCollisionPropertyA, pPhysicsCollisionMgr->GetHardnessAttributeIndex());
	float fHardnessA = g_pLTDatabase->GetFloat( hHardnessAttribute, 0, 0.0f );
	hHardnessAttribute = g_pLTDatabase->GetAttributeByIndex( 
		collisionData.hCollisionPropertyB, pPhysicsCollisionMgr->GetHardnessAttributeIndex());
	float fHardnessB = g_pLTDatabase->GetFloat( hHardnessAttribute, 0, 0.0f );

#ifndef _FINAL
	uint32 nStatsLevel;
	bool bServer;
	HRECORD hCollisionPropertyFilter = NULL;
	CollisionStatsLevel( nStatsLevel, bServer, hCollisionPropertyFilter );
#endif // _FINAL

#ifndef _FINAL
	if( nStatsLevel > 0 && 
		( !hCollisionPropertyFilter || 
		( hCollisionPropertyFilter == collisionData.hCollisionPropertyA || 
		hCollisionPropertyFilter == collisionData.hCollisionPropertyB )))
	{
		g_pLTBase->CPrint( "%s: CollisionPropertyA(%s) HardnessA(%.3f)", g_pLTDatabase->GetRecordName( collisionData.hCollisionPropertyA ), bServer ? "S" : "C", fHardnessA );
		g_pLTBase->CPrint( "%s: CollisionPropertyB(%s) HardnessB(%.3f)", g_pLTDatabase->GetRecordName( collisionData.hCollisionPropertyB ), bServer ? "S" : "C", fHardnessB );
	}
#endif // _FINAL

	if( !collisionData.bFromServer )
	{
		// Calculate the impulse that both actors will use if not already known.
		if( collisionData.fImpulse == 0.0f )
		{
			collisionData.fImpulse = fabs( CalculateImpulse( m_CollisionActor[0].GetRigidBody(), fHardnessA, 
				m_CollisionActor[1].GetRigidBody(), fHardnessB, collisionData ));
		}

#ifndef _FINAL
		if( nStatsLevel > 0 && 
			( !hCollisionPropertyFilter ||
			( hCollisionPropertyFilter == collisionData.hCollisionPropertyA || 
			hCollisionPropertyFilter == collisionData.hCollisionPropertyB )))
		{
			g_pLTBase->CPrint( "%s: Impulse(%.3f)", bServer ? "S" : "C", collisionData.fImpulse );
		}
#endif // _FINAL

		static uint32 const nMinImpulse = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
			DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), MinImpulse );
		if( collisionData.fImpulse < nMinImpulse )
		{

#ifndef _FINAL
			if( nStatsLevel > 0 && 
				( !hCollisionPropertyFilter ||
				( hCollisionPropertyFilter == collisionData.hCollisionPropertyA || 
				hCollisionPropertyFilter == collisionData.hCollisionPropertyB )))
			{
				g_pLTBase->CPrint( "%s: Collision ignored.  Below minimum impulse(%d)", bServer ? "S" : "C", nMinImpulse );
			}
#endif //_FINAL

			return false;
		}
	}
	else
	{
#ifndef _FINAL
		if( nStatsLevel > 0 && 
			( !hCollisionPropertyFilter ||
			( hCollisionPropertyFilter == collisionData.hCollisionPropertyA || 
			hCollisionPropertyFilter == collisionData.hCollisionPropertyB )))
		{
			g_pLTBase->CPrint( "%s: Impulse(%.3f)", bServer ? "S" : "C", collisionData.fImpulse );
		}
#endif // _FINAL
	}

	// Kick off the CollisionActors.
	if( m_CollisionActor[0].IsValid( ) && fHardnessB > 0.0f )
	{
		m_CollisionActor[0].HandleCollision( pPhysicsCollisionMgr, true, collisionData );
	}
	if( m_CollisionActor[1].IsValid( ) && fHardnessA > 0.0f )
	{
		m_CollisionActor[1].HandleCollision( pPhysicsCollisionMgr, false, collisionData );
	}

	return true;
}

bool CollisionPair::Update( )
{
	// Consider ourselves done if both actors are done.
	bool bDoneA = !m_CollisionActor[0].Update( );
	bool bDoneB = !m_CollisionActor[1].Update( );
	// Return false if done.
	return !( bDoneA && bDoneB );
}

PhysicsCollisionMgr::PhysicsCollisionMgr( )
{
	// Pre-allocate our freelist.
	static uint32 const nMaxPairs = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
		DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), MaxPairs );
	m_CollisionPairBank.Init( nMaxPairs, nMaxPairs );
	m_lstCollisionPairs.reserve( nMaxPairs );

	m_fSettleTime = 0.0f;
	m_bEnabled = true;
	m_nHardnessAttributeIndex = (uint32)-1;
	m_nResponsesAttributeIndex = (uint32)-1;
}

PhysicsCollisionMgr::~PhysicsCollisionMgr( )
{
	Term( );
}

bool PhysicsCollisionMgr::Init( )
{
	// Start fresh.
	Term( );

	m_bEnabled = true;

	CLIENT_CODE
	(
		if( !s_vtcPhysicsCollisionStats.IsInitted( ))
		{
			s_vtcPhysicsCollisionStats.Init( g_pLTBase, "PhysicsCollisionStats", NULL, 0.0f );
		}
		if( !s_vtcPhysicsCollisionStatsFilter.IsInitted( ))
		{
			s_vtcPhysicsCollisionStatsFilter.Init( g_pLTBase, "PhysicsCollisionStatsFilter", "", 0.0f );
		}
		if( !s_vtcPhysicsCollisionEnable.IsInitted( ))
		{
			s_vtcPhysicsCollisionEnable.Init( g_pLTBase, "PhysicsCollisionEnable", NULL, 1.0f );
		}
	)

	SERVER_CODE
	(
		if( !s_vtsPhysicsCollisionStats.IsInitted( ))
		{
			s_vtsPhysicsCollisionStats.Init( g_pLTBase, "PhysicsCollisionStats", NULL, 0.0f );
		}
		if( !s_vtsPhysicsCollisionStatsFilter.IsInitted( ))
		{
			s_vtsPhysicsCollisionStatsFilter.Init( g_pLTBase, "PhysicsCollisionStatsFilter", "", 0.0f );
		}
		if( !s_vtsPhysicsCollisionEnable.IsInitted( ))
		{
			s_vtsPhysicsCollisionEnable.Init( g_pLTBase, "PhysicsCollisionEnable", NULL, 1.0f );
		}
	)

	m_fSettleTime = 0.0f;

	// Cache some indexes to attributes for faster lookup.
	m_nHardnessAttributeIndex = (uint32)-1;
	m_nResponsesAttributeIndex = (uint32)-1;
	HCATEGORY hCategory = DATABASE_CATEGORY( CollisionProperty ).GetCategory( );
	HRECORD hCollisionProperty = g_pLTDatabase->GetRecordByIndex( hCategory, 0 );
	if( hCollisionProperty )
	{
		HATTRIBUTE hAttribute = g_pLTDatabase->GetAttribute( hCollisionProperty, "Hardness" );
		if( hAttribute )
		{
			m_nHardnessAttributeIndex = g_pLTDatabase->GetAttributeIndex( hAttribute );
		}
		hAttribute = g_pLTDatabase->GetAttribute( hCollisionProperty, "Responses" );
		if( hAttribute )
		{
			m_nResponsesAttributeIndex = g_pLTDatabase->GetAttributeIndex( hAttribute );
		}
	}

	return true;
}

void PhysicsCollisionMgr::Term( )
{
	RemoveAllCollisionPairs();

	m_bEnabled = false;
}

void PhysicsCollisionMgr::RemoveAllCollisionPairs( )
{
	// Delete all the CollisionPairs.
	for( CollisionPairList::iterator iter = m_lstCollisionPairs.begin( ); iter != m_lstCollisionPairs.end( ); iter++ )
	{
		DeleteCollisionPair( *iter );
	}
	m_lstCollisionPairs.clear();
}

void PhysicsCollisionMgr::PreStartWorld( )
{
	RemoveAllCollisionPairs();
}

void PhysicsCollisionMgr::PostStartWorld( )
{
	EstablishGroupCollisions( );

	static float fSettleTime = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
		DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), SettleTime );
	m_fSettleTime = SimulationTimer::Instance().GetTimerAccumulatedS() + fSettleTime;
}


// Binary predicate object used to compare CollisionPair* objects for std::sort.  Also
// has BinarySearch function for finding CollisionPairs in list.
struct FindCollisionPairPred
{
	// Binary predicate for sorting the CollisionPairList.
	bool operator()( CollisionPair const* pLhs, CollisionPair const* pRhs ) const
	{
		return ( Compare( pLhs->GetCollisionActorA().GetFromServer(), 
			pLhs->GetCollisionActorA().GetRigidBody(), 
			pLhs->GetCollisionActorA().GetObject(),
			pLhs->GetCollisionActorB().GetRigidBody(), 
			pLhs->GetCollisionActorB().GetObject(),
			pRhs->GetCollisionActorA().GetFromServer(), 
			pRhs->GetCollisionActorA().GetRigidBody(), 
			pRhs->GetCollisionActorA().GetObject(),
			pRhs->GetCollisionActorB( ).GetRigidBody(),
			pRhs->GetCollisionActorB( ).GetObject( )) < 0 );
	}

	// Does a binary search on a CollisionPairList.
	static CollisionPair* BinarySearch( PhysicsCollisionMgr::CollisionPairList& lst, 
		bool bFromServer, HPHYSICSRIGIDBODY hA, HOBJECT hObjA, HPHYSICSRIGIDBODY hB, HOBJECT hObjB )
	{
		int32 nLowerBound = 0;
		int32 nUpperBound = ( int32 )( lst.size( ) - 1 );
		while( nLowerBound <= nUpperBound )
		{
			int32 nMiddle = ( nLowerBound + nUpperBound ) / 2;
			CollisionPair* pCollisionPair = lst[nMiddle];
			int32 nResult = Compare( pCollisionPair->GetCollisionActorA().GetFromServer(), 
				pCollisionPair->GetCollisionActorA().GetRigidBody(), 
				pCollisionPair->GetCollisionActorA().GetObject(),
				pCollisionPair->GetCollisionActorB().GetRigidBody(), 
				pCollisionPair->GetCollisionActorB().GetObject(),
				bFromServer, hA, hObjA, hB, hObjB );
			if( nResult < 0 )
			{
				nLowerBound = nMiddle + 1;
			}
			else if( nResult > 0 )
			{
				nUpperBound = nMiddle - 1;
			}
			else
			{
				return pCollisionPair;
			}
		}

		return NULL;
	}

	// Sorts first by rigidbody A, then by rigidbody B.
	static int32 Compare(	bool bLhsServer, HPHYSICSRIGIDBODY hLhsA, HOBJECT hLhsObjA, HPHYSICSRIGIDBODY hLhsB, HOBJECT hLhsObjB,
							bool bRhsServer, HPHYSICSRIGIDBODY hRhsA, HOBJECT hRhsObjA, HPHYSICSRIGIDBODY hRhsB, HOBJECT hRhsObjB )
	{
		// Sort by whether from server or not.
		if( bLhsServer != bRhsServer )
		{
			if( bLhsServer )
				return 1;
			else
				return -1;
		}

		// Sort by object A.
		if( hLhsObjA < hRhsObjA )
			return -1;
		else if( hLhsObjA > hRhsObjA )
			return 1;

		// Sort by object B.
		if( hLhsObjB < hRhsObjB )
			return -1;
		else if( hLhsObjB > hRhsObjB )
			return 1;

		// Sort by rigidbody A.
		if( hLhsA < hRhsA )
			return -1;
		else if( hLhsA > hRhsA )
			return 1;

		// Sort by rigidbody B.
		if( hLhsB < hRhsB )
			return -1;
		else if( hLhsB > hRhsB )
			return 1;

		// Equal in every way.
		return 0;
	}
};


bool PhysicsCollisionMgr::HandleRigidBodyCollision( CollisionData& collisionData )
{
	CTimedSystem* pTimingSystem = NULL;
	SERVER_CODE
	(
		pTimingSystem = &g_tsServerGamePhysicsCollisionMgr;
	)
	CLIENT_CODE
	(
		pTimingSystem = &g_tsClientGamePhysicsCollisionMgr;
	)

	CTimedSystemBlock TimingBlock( *pTimingSystem );

	// Ignore if not enabled.
	if( !IsEnabled( ))
		return false;

	// Check if they disabled it through the console.
	CLIENT_CODE
	(
		if( s_vtcPhysicsCollisionEnable.GetFloat( ) == 0.0f )
			return false;
	)
	SERVER_CODE
	(
		if( s_vtsPhysicsCollisionEnable.GetFloat( ) == 0.0f )
			return false;
	)

	// Check if we've waited long enough for the world to settle.
	if( SimulationTimer::Instance().GetTimerAccumulatedS() < GetSettleTime( ))
		return false;

#ifndef _FINAL
	uint32 nStatsLevel;
	bool bServer;
	HRECORD hCollisionPropertyFilter = NULL;
	CollisionStatsLevel( nStatsLevel, bServer, hCollisionPropertyFilter );
#endif // _FINAL

#ifndef _FINAL
	if( nStatsLevel > 0 )
	{
		g_pLTBase->CPrint( "%s: Physics Collision Event", bServer ? "S" : "C" );
		if( nStatsLevel > 1 )
		{
			g_pLTBase->CPrint( "%s: RigidBodyA(0x%X), RigidBodyB(0x%X)", bServer ? "S" : "C", collisionData.hBodyA, collisionData.hBodyB);
			LTVector vOffset;
			g_pLTBase->GetSourceWorldOffset(vOffset);
			g_pLTBase->CPrint( "%s: CollisionPoint(%.3f,%.3f,%.3f)", bServer ? "S" : "C", VEC_EXPAND( collisionData.vCollisionPt + vOffset ));
			g_pLTBase->CPrint( "%s: Normal(%.3f,%.3f,%.3f)", bServer ? "S" : "C", VEC_EXPAND( collisionData.vCollisionNormal ) );
			g_pLTBase->CPrint( "%s: Velocity(%.3f)", bServer ? "S" : "C", collisionData.fVelocity );
		}
	}
#endif // _FINAL


	// Get the absolute velocity of the collision.  If no velocity, there won't be any impulse.
	if( !collisionData.bFromServer )
	{
		collisionData.fAbsVelocity = fabs( collisionData.fVelocity );
		static float const fMinVelocity = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
			DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), MinVelocity );
		if( collisionData.fAbsVelocity < fMinVelocity )
		{
#ifndef _FINAL
			if( nStatsLevel > 0 )
			{
				g_pLTBase->CPrint( "%s: Collision ignored.  Below minimum velocity(%.3f)", bServer ? "S" : "C", fMinVelocity );
			}
#endif // _FINAL

			return false;
		}
	}

	if( !collisionData.bFromServer )
	{
		// If both rigidbodies are pinned, then they aren't allowed to have
		// collision responses.
		if( collisionData.bIsPinnedA && collisionData.bIsPinnedB )
		{

#ifndef _FINAL
			if( nStatsLevel > 0 )
			{
				g_pLTBase->CPrint( "%s: Collision ignored.  Both rigid bodies are pinned.", bServer ? "S" : "C" );
			}
#endif // _FINAL

			return false;
		}
	}

	if( !collisionData.bFromServer )
	{
		// First make sure the inputs are sorted correctly so that A < B.  The 
		// sorting and binary search require this.
		if( collisionData.hBodyA > collisionData.hBodyB )
		{
			HPHYSICSRIGIDBODY hTemp;
			hTemp = collisionData.hBodyA;
			collisionData.hBodyA = collisionData.hBodyB;
			collisionData.hBodyB = hTemp;

			HOBJECT hTempObj = collisionData.hObjectA;
			collisionData.hObjectA = collisionData.hObjectB;
			collisionData.hObjectB = hTempObj;

			bool bIsPinnedTemp = collisionData.bIsPinnedA;
			collisionData.bIsPinnedA = collisionData.bIsPinnedB;
			collisionData.bIsPinnedA = bIsPinnedTemp;
		}
	}

	// See if we already have a CollisionPair defined for this pair.
	CollisionPair* pCollisionPair = FindCollisionPair( collisionData.bFromServer, collisionData.hBodyA, collisionData.hObjectA, 
		collisionData.hBodyB, collisionData.hObjectB );
	if (!pCollisionPair)
	{
		// Check if we're full of pairs.
		static uint32 const nMaxPairs = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
			DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), MaxPairs );
		if( m_lstCollisionPairs.size( ) == nMaxPairs )
		{

#ifndef _FINAL
			if( nStatsLevel > 0 && 
				( !hCollisionPropertyFilter ||
				( hCollisionPropertyFilter == collisionData.hCollisionPropertyA || 
				hCollisionPropertyFilter == collisionData.hCollisionPropertyB )))
			{
				g_pLTBase->CPrint( "%s: Collision ignored.  Max pairs reached(%d)", bServer ? "S" : "C", nMaxPairs );
			}
#endif // _FINAL

			return false;
		}

		// Create a CollisionPair.
		pCollisionPair = CreateCollisionPair( );
		if( !pCollisionPair )
			return false;

		// Initialize it with the rigid bodies.
		if( !pCollisionPair->Init( collisionData ))
		{
			DeleteCollisionPair( pCollisionPair );
			return false;
		}

		// Add it to the active lists.
		m_lstCollisionPairs.push_back( pCollisionPair );

		// Have to keep list sorted so we can do quick searches.  Most of the time we expect to
		// have multiple collisions with a pair in rapid succession, so that the balance
		// between sorting and binary searching should be favorable over no sorting and linear searching.
		std::sort( m_lstCollisionPairs.begin(), m_lstCollisionPairs.end( ), FindCollisionPairPred( ));
	}

	// Tell the pair to handle the collision.
	return pCollisionPair->HandleCollision( this, collisionData );
}

CollisionPair* PhysicsCollisionMgr::CreateCollisionPair( )
{
	return m_CollisionPairBank.Allocate( );
}

void PhysicsCollisionMgr::DeleteCollisionPair( CollisionPair* pCollisionPair )
{
	m_CollisionPairBank.Free( pCollisionPair );
}

CollisionPair* PhysicsCollisionMgr::FindCollisionPair( bool bFromServer, HPHYSICSRIGIDBODY hBodyA, HOBJECT hObjA,
													  HPHYSICSRIGIDBODY hBodyB, HOBJECT hObjB )
{
	return FindCollisionPairPred::BinarySearch( m_lstCollisionPairs, bFromServer, hBodyA, hObjA, hBodyB, hObjB );
}

void PhysicsCollisionMgr::Update( )
{
	// Iterate through all the CollisionPairs and check if they are done.
	CollisionPairList::iterator iter = m_lstCollisionPairs.begin( );
	while( iter != m_lstCollisionPairs.end( ))
	{
		CollisionPair* pCollisionPair = *iter;
		if( !pCollisionPair->Update( ))
		{
			DeleteCollisionPair( pCollisionPair );
			iter = m_lstCollisionPairs.erase( iter );
		}
		else
		{
			iter++;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Function name   : FindImpulseInTable
// Description     : Function used to lookup an impulse entry in a
//						database table.  Used by responses.
// Return type     : static bool - true if found entry.
// Argument        : float fImpulse - Input impulse to lookup.
// Argument        : HATTRIBUTE hStruct - Database table to look in.
// Argument        : ImpulseAccessCB pImpulseAccessCB - Callback to call for 
//						getting impulse by index.
// Argument        : uint32& nIndexFound - index of impulse match.
// Argument        : uint32& nImpulseFound - impulse found for nIndexFound.
//////////////////////////////////////////////////////////////////////////
bool PhysicsCollisionMgr::FindImpulseInTable( float fImpulse, HATTRIBUTE hStruct, ImpulseAccessCB pImpulseAccessCB, 
											 uint32& nIndexFound, uint32& nImpulseFound )
{
	nIndexFound = (uint32)-1;
	nImpulseFound = 0;

	// Get the number of structs.
	uint32 nNumStructs = g_pLTDatabase->GetNumValues( hStruct );
	if( nNumStructs == 0 )
		return false;

	// Pre-convert to integer.
	uint32 nCollisionImpulse = ( uint32 )fImpulse;

	// Find the struct to use based on the impulse.
	for( uint32 nIndex = 0; nIndex < nNumStructs; nIndex++ )
	{
		// Get this struct's impulse.
		uint32 nImpulse = pImpulseAccessCB( hStruct, nIndex );

		// If the impulse is negative, that means infinite, so we've reached the end of the table.
		if( nImpulse == ( uint32 )-1 )
			break;

		// Check if this entry is less than our collision impulse.
		if( nImpulse <= nCollisionImpulse )
		{
			// Consider this the best match so far.
			nIndexFound = nIndex;
			nImpulseFound = nImpulse;
		}
		// The impulse entry is greater than the impulse and we
		// have found a best match, then use the best match.
		else if( nIndexFound != ( uint32 )-1 )
		{
			break;
		}
	}

	// Check if we didn't match any impulse entries.
	if( nIndexFound == ( uint32 )-1 )
		return false;

	return true;
}

static uint32 SoundDBRecordImpulseAccessCB( HATTRIBUTE hStruct, uint32 nIndex )
{
	return DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( SoundRecord, hStruct, nIndex, Impulse );
}

HRECORD PhysicsCollisionMgr::FindSoundDBRecord( CollisionResponse const& collisionResponse )
{
	// Get the soundrecord struct.
	HATTRIBUTE hStruct = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTSTRUCT( Responses, 
		collisionResponse.GetResponses(), collisionResponse.GetResponsesIndex( ), SoundRecord );
	if( !hStruct )
		return NULL;

	uint32 nIndexFound;
	uint32 nImpulseFound;
	if( !FindImpulseInTable( collisionResponse.GetImpulse( ), hStruct, SoundDBRecordImpulseAccessCB, nIndexFound, nImpulseFound ))
		return NULL;

	// Get the sounddb entry.
	return DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( SoundRecord, hStruct, nIndexFound, Sound );
}


static uint32 SoundVolumeImpulseAccessCB( HATTRIBUTE hStruct, uint32 nIndex )
{
	return DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( SoundVolume, hStruct, nIndex, Impulse );
}

uint8 PhysicsCollisionMgr::FindSoundVolume( CollisionResponse const& collisionResponse )
{
	// Get the soundvolume struct.  If none, then assume default.
	HATTRIBUTE hStruct = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTSTRUCT( Responses, 
		collisionResponse.GetResponses(), collisionResponse.GetResponsesIndex( ), SoundVolume );
	if( !hStruct )
		return 100;

	uint32 nIndexFound;
	uint32 nImpulseFound;
	if( !FindImpulseInTable( collisionResponse.GetImpulse( ), hStruct, SoundVolumeImpulseAccessCB, nIndexFound, nImpulseFound ))
		return 100;

	// Get the minimum volume to use.
	uint32 nMinVolume = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( SoundVolume, hStruct, nIndexFound, Volume );

	// If there are no more struct entries, then just use the minimum volume.
	uint32 nNumStructs = g_pLTDatabase->GetNumValues( hStruct );
	if( nIndexFound + 1 == nNumStructs )
	{
		return ( uint8 )LTCLAMP( nMinVolume, 0, 100 );
	}

	// Get the maximum impulse from the next higher struct entry.
	uint32 nMaxImpulse = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( SoundVolume, hStruct, nIndexFound + 1, Impulse );
	// If it's infinite, then just use the minimum.
	if( nMaxImpulse == ( uint32 )-1 )
		return ( uint8 )LTCLAMP( nMinVolume, 0, 100 );

	// Get the maximum volume from the next higher struct entry.
	uint32 nMaxVolume = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( SoundVolume, hStruct, nIndexFound + 1, Volume );

	// Linearly interpolate the volume based on position in the impulse table.
	uint32 nVolume = ( uint32 )((( collisionResponse.GetImpulse( ) - nImpulseFound ) / ( nMaxImpulse - nImpulseFound )) * 
		( nMaxVolume - nMinVolume ) + 0.5f ) + nMinVolume;
	return ( uint8 )LTCLAMP( nVolume, 0, 100 );
}

bool PhysicsCollisionMgr::StartSoundResponse( CollisionResponse& collisionResponse )
{
	// Get the sounddb record to use.  If there wasn't a sounddb entry, then no sound response.
	HRECORD hSoundDB = FindSoundDBRecord( collisionResponse );
	if( !hSoundDB )
		return false;

	// Get the volume to use with sound. If there's no volume, then no sound response.
	uint8 nVolume = FindSoundVolume( collisionResponse );
	static uint32 const nMinVolume = DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( 
		DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), MinVolume );
	if( nVolume < nMinVolume )
		return false;

	return DoSoundResponse( collisionResponse, hSoundDB, nVolume );
}

static uint32 ClientFXImpulseAccessCB( HATTRIBUTE hStruct, uint32 nIndex )
{
	return DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( ClientFX, hStruct, nIndex, Impulse );
}

bool PhysicsCollisionMgr::StartClientFXResponse( CollisionResponse& collisionResponse )
{
	// Get the clientfx struct.
	HATTRIBUTE hStruct = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTSTRUCT( Responses, 
		collisionResponse.GetResponses(), collisionResponse.GetResponsesIndex( ), ClientFX );
	if( !hStruct )
		return false;

	uint32 nIndexFound = (uint32)-1;
	uint32 nImpulseFound;
	if( !FindImpulseInTable( collisionResponse.GetImpulse( ), hStruct, ClientFXImpulseAccessCB, nIndexFound, nImpulseFound ))
		return false;

	// Get the clientfx name.
	char const* pszClientFX = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( ClientFX, hStruct, 
		nIndexFound, ClientFX );
	if( !pszClientFX || !pszClientFX[0] )
		return false;

	return DoClientFXResponse( collisionResponse, hStruct, nIndexFound, pszClientFX );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EstablishGroupCollisions()
//
//	PURPOSE:	Creates groups of collision types and sets the relationship
//				of what groups are solid or non-solid with other groups.
//				This follows the following grid: (X - client & server, C - client only, S - server only)
//
//							Non-solid	World	WorldModels	Models	Player	AI	Ragdoll	Pickup	Debris	Projectile	RemoteMelee	LocalMelee	BlockMelee	Filtered	Multiplayer	PlayerRB
//				Non-solid	-			-		-			-		-		-	-		-		-		-			-			-			-			-			-			-
//				World		-			-		X			X		-		X	X		X		X		X			C			C			-			X			X			X
//				WorldModels	-			X		X			X		-		X	X		X		X		X			C			C			-			X			X			X
//				Models		-			X		X			X		-		X	X		X		-		X			-			-			-			X			X			X
//				Player		-			-		-			-		-		-	-		-		-		X			C			-			-			X			-			-
//				AI			-			X		X			X		-		-	-		-		-		-			C			C			-			X			-			-
//				Ragdoll		-			X		X			X		-		-	X		-		-		-			-			C			-			X			C			-
//				Pickup		-			X		X			X		-		-	-		X		-		-			-			C			-			X			-			-
//				Debris		-			X		X			-		-		-	-		-		-		-			C			C			-			X			C			X
//				Projectile	-			X		X			X		X		-	-		-		-		-			-			C			-			X			C			-
//				RemoteMelee	-			C		C			-		C		C	-		-		C		-			-			-			C			X			-			-
//				LocalMelee	-			C		C			-		-		C	C		C		C		C			-			-			-			X			-			-
//				BlockMelee	-			-		-			-		-		-	-		-		-		-			C			-			-			X			-			-
//				Filtered	-			X		X			X		X		X	X		X		X		X			X			X			-			X			-			-
//				Multiplayer -			X		X			X		C		-	X		X		X		C			-			-			-			X			C			C
//				PlayerRB	-			X		X			X		-		-	-		-		X		-			-			-			-			-			C			-
//				
// ----------------------------------------------------------------------- //
bool PhysicsCollisionMgr::EstablishGroupCollisions( )
{
	// Register the physics groups that can collide with each other.
	ILTPhysicsSim *pILTPhysicsSim = g_pLTBase->PhysicsSim();
	if( !pILTPhysicsSim )
	{
		LTERROR( "PhysicsSim interface not initialized yet." );
		return false;
	}

	bool bOk = true;
	// First set all the collisions to false, then just set the few true ones.
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( ePhysicsGroup_World, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( ePhysicsGroup_WorldModels, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( ePhysicsGroup_Models, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserPlayer, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserAI, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserRagDoll, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserPickup, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserDebris, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserProjectile, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserRemoteMelee, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserLocalMelee, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserBlockMelee, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserFiltered, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserMultiplayer, false ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( PhysicsUtilities::ePhysicsGroup_UserPlayerRigidBody, false ) == LT_OK );

	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, ePhysicsGroup_WorldModels, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, ePhysicsGroup_Models, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, PhysicsUtilities::ePhysicsGroup_UserAI, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, PhysicsUtilities::ePhysicsGroup_UserRagDoll, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, PhysicsUtilities::ePhysicsGroup_UserPickup, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, PhysicsUtilities::ePhysicsGroup_UserDebris, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, PhysicsUtilities::ePhysicsGroup_UserProjectile, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, PhysicsUtilities::ePhysicsGroup_UserMultiplayer, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_World, PhysicsUtilities::ePhysicsGroup_UserPlayerRigidBody, true ) == LT_OK );

	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, ePhysicsGroup_WorldModels, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, ePhysicsGroup_Models, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, PhysicsUtilities::ePhysicsGroup_UserAI, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, PhysicsUtilities::ePhysicsGroup_UserRagDoll, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, PhysicsUtilities::ePhysicsGroup_UserPickup, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, PhysicsUtilities::ePhysicsGroup_UserDebris, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, PhysicsUtilities::ePhysicsGroup_UserProjectile, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, PhysicsUtilities::ePhysicsGroup_UserMultiplayer, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_WorldModels, PhysicsUtilities::ePhysicsGroup_UserPlayerRigidBody, true ) == LT_OK );

	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_Models, ePhysicsGroup_Models, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_Models, PhysicsUtilities::ePhysicsGroup_UserAI, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_Models, PhysicsUtilities::ePhysicsGroup_UserRagDoll, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_Models, PhysicsUtilities::ePhysicsGroup_UserPickup, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_Models, PhysicsUtilities::ePhysicsGroup_UserProjectile, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_Models, PhysicsUtilities::ePhysicsGroup_UserMultiplayer, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( ePhysicsGroup_Models, PhysicsUtilities::ePhysicsGroup_UserPlayerRigidBody, true ) == LT_OK );

	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserPlayer, PhysicsUtilities::ePhysicsGroup_UserProjectile, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserAI, PhysicsUtilities::ePhysicsGroup_UserProjectile, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserRagDoll, PhysicsUtilities::ePhysicsGroup_UserRagDoll, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserPickup, PhysicsUtilities::ePhysicsGroup_UserPickup, true ) == LT_OK );
	bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserPlayerRigidBody, PhysicsUtilities::ePhysicsGroup_UserDebris, true ) == LT_OK );
	
	CLIENT_CODE
	(
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserRemoteMelee, PhysicsUtilities::ePhysicsGroup_UserPlayer, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserRemoteMelee, PhysicsUtilities::ePhysicsGroup_UserAI, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserRemoteMelee, PhysicsUtilities::ePhysicsGroup_UserBlockMelee, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserRemoteMelee, PhysicsUtilities::ePhysicsGroup_UserDebris, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserRemoteMelee, ePhysicsGroup_WorldModels, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserRemoteMelee, ePhysicsGroup_World, true ) == LT_OK );

		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserLocalMelee, PhysicsUtilities::ePhysicsGroup_UserAI, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserLocalMelee, PhysicsUtilities::ePhysicsGroup_UserRagDoll, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserLocalMelee, PhysicsUtilities::ePhysicsGroup_UserPickup, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserLocalMelee, PhysicsUtilities::ePhysicsGroup_UserDebris, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserLocalMelee, PhysicsUtilities::ePhysicsGroup_UserProjectile, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserLocalMelee, ePhysicsGroup_WorldModels, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserLocalMelee, ePhysicsGroup_World, true ) == LT_OK );
		
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserMultiplayer, PhysicsUtilities::ePhysicsGroup_UserProjectile, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserMultiplayer, PhysicsUtilities::ePhysicsGroup_UserPlayerRigidBody, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserMultiplayer, PhysicsUtilities::ePhysicsGroup_UserRagDoll, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserMultiplayer, PhysicsUtilities::ePhysicsGroup_UserDebris, true ) == LT_OK );
		bOk = bOk && ( pILTPhysicsSim->EnableCollisions( PhysicsUtilities::ePhysicsGroup_UserMultiplayer, PhysicsUtilities::ePhysicsGroup_UserMultiplayer, true ) == LT_OK );

	)

	// Just make sure nonsolid is nonsolid.
	bOk = bOk && ( pILTPhysicsSim->EnableAllCollisions( ePhysicsGroup_NonSolid, false ) == LT_OK );

	// Need to call this after all collisions between groups have been established...
	bOk = bOk && ( pILTPhysicsSim->ApplyCollisionGroupChanges( ) == LT_OK );

	if( !bOk )
	{
		LTERROR( "Failed to apply changes to the collision groups." );
		return false;
	}

	// Success...
	return true;
}

