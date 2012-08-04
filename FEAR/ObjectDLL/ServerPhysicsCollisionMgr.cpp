// ----------------------------------------------------------------------- //
//
// MODULE  : ServerServerPhysicsCollisionMgr.cpp
//
// PURPOSE : Definition of physics collision mgr for server
//
// CREATED : 08/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ServerPhysicsCollisionMgr.h"
#include "iltphysicssim.h"			// for HPHYSICSRIGIDBODY
#include "iltcsbase.h"				// for ILTCSBase
#include "AutoMessage.h"			// for CAutoMessage
#include "FxDefs.h"					// for SFX_CLIENTFXGROUPINSTANT
#include "CharacterDB.h"			// for String2Alignment
#include "CollisionsDB.h"
#include "AIStimulusMgr.h"
#include "AIDB.h"
#include "Destructible.h"

#ifndef _FINAL
extern void CollisionStatsLevel( uint32& nLevel, bool& bServer, HRECORD& hCollisionPropertyFilter );
#endif // _FINAL

// Callback function to handle all server rigid body collisions...
static void GlobalRigidBodyCollisionCB(HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
									   const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
									   float fVelocity, bool& bIgnoreCollision, void* pUser)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameServerShell::CServerShellScopeTracker cScopeTracker;

	CollisionData collisionData;
	collisionData.hBodyA = hBody1;
	collisionData.hBodyB = hBody2;
	g_pLTBase->PhysicsSim()->IsRigidBodyPinned(hBody1, collisionData.bIsPinnedA);
	g_pLTBase->PhysicsSim()->IsRigidBodyPinned(hBody2, collisionData.bIsPinnedB);
	collisionData.vCollisionPt = vCollisionPt;
	collisionData.vCollisionNormal = vCollisionNormal;
	collisionData.fVelocity = fVelocity;
	ServerPhysicsCollisionMgr::Instance().HandleRigidBodyCollision( collisionData );
}

bool ServerPhysicsCollisionMgr::Init( )
{
	if( !PhysicsCollisionMgr::Init( ))
		return false;

	return true;
}

void ServerPhysicsCollisionMgr::PostStartWorld( )
{
	PhysicsCollisionMgr::PostStartWorld();

	g_pLTBase->PhysicsSim()->SetGlobalCollisionListener(GlobalRigidBodyCollisionCB, NULL);
}

bool ServerPhysicsCollisionMgr::HandleRigidBodyCollision( CollisionData& collisionData )
{
	if( !PhysicsCollisionMgr::HandleRigidBodyCollision( collisionData ))
		return false;

	// Check if this event needs to get sent to the client.
	if( collisionData.bSendToClient )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PHYSICSCOLLISION );
		// We send the rigidbody values down to the client, even though
		// they can't be dereferenced.  They will be used to
		// uniquely identify this collision event on the client.
		cMsg.Writeuint32(( uint32 )collisionData.hBodyA );
		cMsg.Writeuint32(( uint32 )collisionData.hBodyB );
		// Send the objects down if we had rigid bodies.  We
		// can't get the objects without the rigid bodies, so
		// only sending them when we have non-null rigid bodies saves bandwidth.
		if( collisionData.hBodyA != INVALID_PHYSICS_RIGID_BODY )
			cMsg.WriteObject( collisionData.hObjectA );
		if( collisionData.hBodyB != INVALID_PHYSICS_RIGID_BODY )
			cMsg.WriteObject( collisionData.hObjectB );
		// We can write out 8bits of index here because we already
		// have a 255 limit on collisionproperties, since they must fit in the userflags.
		cMsg.Writeuint8(( uint8 )g_pLTDatabase->GetRecordIndex( collisionData.hCollisionPropertyA ));
		cMsg.Writeuint8(( uint8 )g_pLTDatabase->GetRecordIndex( collisionData.hCollisionPropertyB ));
		// Super fine resolution of the impulse isn't necessary
		// since the data tables are defined with integers.
		cMsg.Writefloat(collisionData.fImpulse);
		cMsg.WriteLTVector( collisionData.vCollisionPt );
		cMsg.WriteLTVector( collisionData.vCollisionNormal );
		// We're sending unguaranteed, since it's not essential the client is informed,
		// since it's just sounds and clientfx.
		g_pLTServer->SendToClient( cMsg.Read(), NULL, 0 );
	}

	return true;
}

bool ServerPhysicsCollisionMgr::StartResponses( CollisionResponse& collisionResponse, CollisionData& collisionData )
{
	collisionData.bSendToClient = StartSoundResponse( collisionResponse );
	collisionData.bSendToClient |= StartClientFXResponse( collisionResponse );
	StartAIStimulusResponse( collisionResponse );
	StartDamageResponse( collisionResponse );

	return true;
}

static uint32 AIStimulusRecordImpulseAccessCB( HATTRIBUTE hStruct, uint32 nIndex )
{
	return DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( AIStimulusRecord, hStruct, nIndex, Impulse );
}

static uint32 AIStimulusRadiusImpulseAccessCB( HATTRIBUTE hStruct, uint32 nIndex )
{
	return DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( AIStimulusRadius, hStruct, nIndex, Impulse );
}

bool ServerPhysicsCollisionMgr::StartAIStimulusResponse( CollisionResponse& collisionResponse )
{
	// Get the aistimulus record struct.
	HATTRIBUTE hStruct = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTSTRUCT( Responses, 
		collisionResponse.GetResponses(), collisionResponse.GetResponsesIndex( ), AIStimulusRecord );
	if( !hStruct )
		return false;

	uint32 nIndexFound;
	uint32 nImpulseFound;
	if( !FindImpulseInTable( collisionResponse.GetImpulse( ), hStruct, AIStimulusRecordImpulseAccessCB, nIndexFound, nImpulseFound ))
		return false;

	// Determine the stimulus type.
	HRECORD hStimulus = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( AIStimulusRecord, hStruct, nIndexFound, Stimulus );
	if( !hStimulus )
		return false;
	const char* pszStimulus = g_pLTDatabase->GetRecordName( hStimulus );
	if( !pszStimulus || !pszStimulus[0] )
		return false;

	// Bail if stimulus does not exist.
	EnumAIStimulusType eStimulus = (EnumAIStimulusType)g_pAIDB->String2BitFlag( pszStimulus, kStim_Count, s_aszStimulusTypes );
	if( eStimulus == kStim_InvalidType )
		return false;

	// Use the generic object alignment set in GDBEdit under AI/Constants.
	EnumCharacterAlignment eAlignment = g_pCharacterDB->String2Alignment(g_pAIDB->GetAIConstantsRecord()->strObjectAlignmentName.c_str());

	// Interpolate the radius of the stimulus.
	float fRadius = FindStimulusRadius( collisionResponse );

	// Bail if either character has sound disabled.
	// This is a hack to prevent AI from generating stimuli.
	// (e.g. at the begining of the FEAR Docks level, where
	// we have a scripted execution scene, and do not want the
	// AI to start investigating when a body hits the ground).
	HOBJECT hActor = collisionResponse.GetCollisionActor()->GetObject();
	HOBJECT hOtherActor = collisionResponse.GetCollisionActor()->GetOtherCollisionActor()->GetObject();

	if( IsCharacter( hActor ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hActor );
		if( pChar && pChar->GetSoundOuterRadius() == 0.f )
		{
			return false;
		}
	}
	if( IsCharacter( hOtherActor ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hOtherActor );
		if( pChar && pChar->GetSoundOuterRadius() == 0.f )
		{
			return false;
		}
	}

	// Register the stimulus.
	StimulusRecordCreateStruct scs( eStimulus, eAlignment, collisionResponse.GetCollisionPoint( ), hActor );
	scs.m_hStimulusTarget = hOtherActor;
	scs.m_flRadiusScalar = fRadius;
	g_pAIStimulusMgr->RegisterStimulus( scs );

#ifndef _FINAL
	uint32 nStatsLevel;
	bool bServer;
	HRECORD hCollisionPropertyFilter = NULL;
	CollisionStatsLevel( nStatsLevel, bServer, hCollisionPropertyFilter );
	if( nStatsLevel > 0 &&
		( !hCollisionPropertyFilter || hCollisionPropertyFilter == collisionResponse.GetCollisionProperty()))
	{
		g_pLTBase->CPrint( "S:  AIStimulus(%s)", pszStimulus );
	}
#endif // _FINAL

	return true;
}

float ServerPhysicsCollisionMgr::FindStimulusRadius( CollisionResponse const& collisionResponse )
{
	// Get the AIStimulusRadius struct.  If none, then assume default.
	HATTRIBUTE hStruct = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTSTRUCT( Responses, 
		collisionResponse.GetResponses(), collisionResponse.GetResponsesIndex( ), AIStimulusRadius );
	if( !hStruct )
		return 0.f;

	uint32 nIndexFound;
	uint32 nImpulseFound;
	if( !FindImpulseInTable( collisionResponse.GetImpulse( ), hStruct, AIStimulusRadiusImpulseAccessCB, nIndexFound, nImpulseFound ))
		return 0.f;

	// Get the minimum radius to use.
	float fMinRadius = (float)DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( AIStimulusRadius, hStruct, nIndexFound, Radius );

	// If there are no more struct entries, then just use the minimum radius.
	uint32 nNumStructs = g_pLTDatabase->GetNumValues( hStruct );
	if( nIndexFound + 1 == nNumStructs )
	{
		return fMinRadius;
	}

	// Get the  maximum impulse from the next higher struct entry.
	uint32 nMaxImpulse = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( AIStimulusRadius, hStruct, nIndexFound + 1, Impulse );
	// If it's infinite, then just use the minimum.
	if( nMaxImpulse == ( uint32 )-1 )
		return fMinRadius;

	// Get the maximum radius from the next higher struct entry.
	float fMaxRadius = (float)DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( AIStimulusRadius, hStruct, nIndexFound + 1, Radius );

	// Bail if invalid inputs.
	float fMinImpulse = (float)nImpulseFound;
	float fMaxImpulse = (float)nMaxImpulse;
	if( fMinImpulse >= fMaxImpulse )
		return fMinRadius;

	// Linearly interpolate the radius based on position in the impulse table.
	float fInterp = ( collisionResponse.GetImpulse() - fMinImpulse ) / ( fMaxImpulse - fMinImpulse );
	float fRadius = fMinRadius + ( fInterp * ( fMaxRadius - fMinRadius ) );

	return fRadius;
}




static uint32 DamageImpulseAccessCB( HATTRIBUTE hStruct, uint32 nIndex )
{
	return DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( DamageResponses, hStruct, nIndex, Impulse );
}



// Enum version of strings in database.
enum DamageWho
{
	eDamageWho_Me,
	eDamageWho_Them,
	eDamageWho_Both,
};

// Converts the damagewho string in the damage to an enum.
static DamageWho FindDamageWho( HATTRIBUTE hStruct )
{
	// Initialize our search table.
	struct StringToEnum
	{
		CParsedMsg::CToken	m_tokString;
		DamageWho			m_eEnum;
	};
	static StringToEnum table[] = 
	{
		{ "ME", eDamageWho_Me },
		{ "THEM", eDamageWho_Them },
		{ "BOTH", eDamageWho_Both },
	};
	static uint32 nTableCount = LTARRAYSIZE( table );

	// Find the string location and return the enum.
	CParsedMsg::CToken tokValue = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( Damage, hStruct, 0, DamageWho );
	for( uint32 i = 0; i < nTableCount; i++ )	
	{
		if( table[i].m_tokString == tokValue )
			return table[i].m_eEnum;
	}

	return eDamageWho_Me;
}

bool ServerPhysicsCollisionMgr::StartDamageResponse( CollisionResponse& collisionResponse )
{
	// Get the other actor in this pair.
	CollisionActor const* pOtherActor = collisionResponse.GetCollisionActor()->GetOtherCollisionActor();

	// If the other actor doesn't have an object, then there's nothing to damage.
	if( !pOtherActor->GetObject( ))
		return false;

	// Get the damage struct.
	HATTRIBUTE hStruct = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTSTRUCT( Responses, 
		collisionResponse.GetResponses(), collisionResponse.GetResponsesIndex( ), Damage );
	if( !hStruct )
		return false;

	// Get the damageresponses struct.
	HATTRIBUTE hResponses = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTSTRUCT( Damage, hStruct, 0, DamageResponses );
	if( !hResponses )
		return false;

	uint32 nIndexFound;
	uint32 nImpulseFound;
	if( !FindImpulseInTable( collisionResponse.GetImpulse( ), hResponses, DamageImpulseAccessCB, nIndexFound, nImpulseFound ))
		return false;

	// Get the damagetype record.
	HRECORD hDamageRecord = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( DamageResponses, hResponses, nIndexFound, DamageType );
	if( !hDamageRecord )
		return false;

	// Get who to damage.
	DamageWho eDamageWho = FindDamageWho( hStruct );

	// Do the damage.
	DamageStruct damage;
	damage.eType		 = g_pDTDB->GetDamageType( hDamageRecord );
	damage.fDamage		 = DATABASE_CATEGORY( CollisionProperty ).GETSTRUCTATTRIB( DamageResponses, hResponses, nIndexFound, Amount );
	damage.fPenetration  = 0.0f;
	damage.fImpulseForce = 0.0f; // No impulse force, since we're already doing that.
	damage.hContainer	 = NULL;
	damage.SetPositionalInfo( collisionResponse.GetCollisionPoint( ), collisionResponse.GetCollisionNormal( ));

	// Do the damage to appropriate object.
	if( eDamageWho == eDamageWho_Me || eDamageWho == eDamageWho_Both )
	{
		damage.hDamager = pOtherActor->GetObject( );
		damage.DoDamage( pOtherActor->GetObject( ), collisionResponse.GetCollisionActor( )->GetObject( ));
	}
	if( eDamageWho == eDamageWho_Them || eDamageWho == eDamageWho_Both )
	{
		damage.hDamager	= collisionResponse.GetCollisionActor( )->GetObject( );
		damage.DoDamage( collisionResponse.GetCollisionActor( )->GetObject( ), pOtherActor->GetObject( ));
	}

#ifndef _FINAL
	uint32 nStatsLevel;
	bool bServer;
	HRECORD hCollisionPropertyFilter = NULL;
	CollisionStatsLevel( nStatsLevel, bServer, hCollisionPropertyFilter );
	if( nStatsLevel > 0 &&
		( !hCollisionPropertyFilter || hCollisionPropertyFilter == collisionResponse.GetCollisionProperty()))
	{
		g_pLTBase->CPrint( "S:  DamageType(%s) Amount(%.3f)", g_pLTDatabase->GetRecordName( hDamageRecord ), damage.fDamage );
	}
#endif // _FINAL

	return true;
}

