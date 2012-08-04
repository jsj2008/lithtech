// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorBerserker.cpp
//
// PURPOSE : This sensor handles determining when the AI should consider
//			 a berserker attack.  If the sensor finds a potential target,
//			 it causes a target reevaluation to enable the AI to decide 
//			 to attack.
//
// CREATED : 8/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorBerserker.h"
#include "AI.h"
#include "AIWorkingMemory.h"
#include "AIBlackBoard.h"
#include "PlayerObj.h"

LINKFROM_MODULE(AISensorBerserker);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorBerserker, kSensor_Berserker );

class BerserkerTargetCollector
{
public:
	BerserkerTargetCollector( CAI* pAI, float flMinDist, float flMaxDist ) :
	  m_pBestTargetFact( NULL )
	  , m_flMinDistSqr( flMinDist )
	  , m_flMaxDistSqr( flMaxDist )
	  , m_pAI( pAI )
	{
	}

	void operator()( CAIWMFact* pFact )
	{
		// Fact is not about a character
		// Facts target is not a player

		if ( kFact_Character != pFact->GetFactType()
			|| !IsPlayer( pFact->GetTargetObject() ) )
		{
			return;
		}

		// TODO: AI either failed an attempt at this plan, or just performed 
		// a berserker attack.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Desire );
		factQuery.SetDesireType( kDesire_Berserker );
		factQuery.SetTargetObject( pFact->GetTargetObject() );
		CAIWMFact* pBerserkerDesire = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if ( pBerserkerDesire )
		{
			if ( g_pLTServer->GetTime() < pBerserkerDesire->GetTime() )
			{
				return;
			}
		}

		// Target is outside of berserker range
		
		LTVector vPlayerPos;
		g_pLTServer->GetObjectPos( pFact->GetTargetObject(), &vPlayerPos );
		float flDistSqr = m_pAI->GetPosition().DistSqr( vPlayerPos );
		if ( flDistSqr < m_flMinDistSqr || flDistSqr > m_flMaxDistSqr )
		{
			return;
		}

		// Store the fact.

		m_pBestTargetFact = pFact;
	}

	float		m_flMinDistSqr;
	float		m_flMaxDistSqr;
	CAIWMFact*	m_pBestTargetFact;
	CAI*		m_pAI;
};

class AllBerserkerFacts
{
	enum kConst
	{
		// This system currently only handles a single berserker target.  It may
		// handle more in the future, so keep it scalable.  Exceeding 1 here 
		// simply means this functionality should be verified again to insure it 
		// handles multiple targets.
		kMaxBerserkerFacts = 1
	};

public:
	AllBerserkerFacts() : 
		m_FactCount(0)
	{
		for ( int i = 0 ; i < kMaxBerserkerFacts; ++i )
		{
			m_FactList[i] = NULL;
		}
	}

	void operator()( CAIWMFact* pFact )
	{
		if ( !pFact
			|| kFact_Desire != pFact->GetFactType()
			|| kDesire_Berserker != pFact->GetDesireType() )
		{
			return;
		}

		if ( m_FactCount == kMaxBerserkerFacts )
		{
			AIASSERT( 0, NULL, "AllBerserkerFacts : Too many berserker facts." );
			return;
		}

		m_FactList[m_FactCount] = pFact;
		m_FactCount++;
	}

	int GetCount()
	{
		return m_FactCount;
	}

	CAIWMFact* GetFact(int i)
	{
		if ( i < 0 || i > m_FactCount )
		{
			return NULL;
		}

		return m_FactList[i]; 
	}


private:
	CAIWMFact*	m_FactList[kMaxBerserkerFacts];
	int			m_FactCount;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorBerserker::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorBerserker::CAISensorBerserker()
{
}

CAISensorBerserker::~CAISensorBerserker()
{
	// Verify that the AI is not still mounted.  If they are, clean it up,
	// as we don't want to leave the player 

	AIASSERT( m_pAI, NULL, "CAISensorBerserker::~CAISensorBerserker: No AI." );
	AIASSERT( m_pAI->GetAIWorkingMemory(), NULL, "CAISensorBerserker::~CAISensorBerserker: No AIWorkingMemory." );
	if ( m_pAI && m_pAI->GetAIWorkingMemory() )
	{
		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
		if ( pFact )
		{
			// Release the player.

			CPlayerObj* pPlayer = CPlayerObj::DynamicCast( pFact->GetTargetObject() );
			if ( pPlayer )
			{
				pPlayer->BerserkerAbort( m_pAI->GetHOBJECT() );
			}

			// Perform cleanup to keep everything in sync. (?? Do we need to 
			// reset UsingObject as well?)

			m_pAI->GetAIWorkingMemory()->ClearWMFact( pFact );
			m_pAI->GetAIWorldState()->SetWSProp( kWSK_MountedObject, NULL, kWST_bool, false ); 
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorBerserker::UpdateSensor
//              
//	PURPOSE:	Update the sensor.
//              
//----------------------------------------------------------------------------

bool CAISensorBerserker::UpdateSensor()
{
	if ( !m_pAI )
	{
		return false;
	}

	// Fail if AI was not previously aware of a character target.

	if( !(m_pAI->GetAIBlackBoard()->GetBBTargetedTypeMask() & kTarget_Character) )
	{
		return false;
	}

	// Fail if the AI already has a berserker target

	if ( m_pAI->HasTarget( kTarget_Berserker ) )
	{
		return false;
	}

	float flMin = 0.f;
	float flMax = 0.f;
	GetBerserkerSenseRange( &flMin, &flMax );
	float flMinSqr = flMin*flMin;
	float flMaxSqr = flMax*flMax;

	//
	// Remove the any current berserker facts if the AI is out of range.
	//

	AllBerserkerFacts all;
	m_pAI->GetAIWorkingMemory()->CollectFact( all );
	for ( int iFact = 0; iFact < all.GetCount(); ++iFact )
	{
		CAIWMFact* pFact = all.GetFact( iFact );
		if ( pFact )
		{
			LTVector vTargetPos;
			g_pLTServer->GetObjectPos( pFact->GetTargetObject(), &vTargetPos );
			float flDistSqr = vTargetPos.DistSqr( m_pAI->GetPosition());
			if ( flDistSqr > flMaxSqr || flDistSqr < flMinSqr)
			{
				m_pAI->GetAIWorkingMemory()->ClearWMFact( pFact );
			}
		}
	}

	//
	// Find a valid berserker target.
	//

	// Fail if there no target to perform a berserk attack against.

	BerserkerTargetCollector Berserker( m_pAI, flMinSqr, flMaxSqr );
	m_pAI->GetAIWorkingMemory()->CollectFact( Berserker );

	if ( !Berserker.m_pBestTargetFact )
	{
		return false;
	}

	// Add a desire to perform a berserk attack against if this target if one
	// doesn't already exist.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Berserker );
	factQuery.SetTargetObject( Berserker.m_pBestTargetFact->GetTargetObject() );
	CAIWMFact* pBerserkerDesire = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pBerserkerDesire )
	{
		CAIWMFact* pBerserkerDesire = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
		pBerserkerDesire->SetDesireType( kDesire_Berserker );
		pBerserkerDesire->SetTargetObject( Berserker.m_pBestTargetFact->GetTargetObject() );
		pBerserkerDesire->SetTime( g_pLTServer->GetTime() );
	}

	// Flag a target re-evaluation, but only if the AI is unarmed

	if ( !AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER ) 
		&& !AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER ) )
	{
		m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorBerserkerDark::GetBerserkerSenseRange
//              
//	PURPOSE:	Returns the sensor range of for berserker attacks, by default 
//				using the smartobject associated data.
//              
//----------------------------------------------------------------------------

void CAISensorBerserker::GetBerserkerSenseRange( float* pOutMinRange, float* pOutMaxRange )
{
	if ( !pOutMinRange || !pOutMaxRange )
	{
		LTASSERT( 0, "CAISensorBerserker::GetBerserkerSenseRange : Invalid output pointers." );
		return;
	}

	// Fail if the Sensor's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pSensorRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		LTASSERT( 0, "CAISensorBerserker::GetBerserkerSenseRange : No min and max range specified; no associated smartobject." );
		return;
	}

	*pOutMinRange = pSmartObjectRecord->fMinDist;
	*pOutMaxRange = pSmartObjectRecord->fMaxDist;
}
