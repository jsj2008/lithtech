// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityExchangeWeapons.cpp
//
// PURPOSE : 
//
// CREATED : 7/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActivityExchangeWeapons.h"
#include "AIUtils.h"
#include "AI.h"
#include "WeaponItems.h"
#include "AIWorkingMemory.h"
#include "AIBlackBoard.h"
#include "AINode.h"
#include "Weapon.h"
#include "AISquad.h"
#include "AIWorkingMemoryCentral.h"

LINKFROM_MODULE(AIActivityExchangeWeapons);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivityExchangeWeapons, kActivity_ExchangeWeapons );

struct BestWeaponItemCollector
{
	BestWeaponItemCollector( CAI* pAI ) : 
		m_pAI( pAI )
		, m_pBestWeaponItem( NULL )
		, m_nBestWeaponPreference( 0 )
	{
	}

	void operator()( CAIWMFact* pFact )
	{
		// Ignore if:
		//	Fact is invalid
		//	Fact is not knowledge
		//	Knowledge is not about a weaponitem
		//	Fact is timed out

		if ( !pFact 
			|| kFact_Knowledge != pFact->GetFactType()
			|| kKnowledge_WeaponItem != pFact->GetKnowledgeType()
			|| g_pLTServer->GetTime() < pFact->GetTime() )
		{
			return;
		}

		// Verify that the WeaponItem associated with this fact still exists.

		if ( pFact->GetTargetObject() == NULL )
		{
			return;
		}

		WeaponItem* pWeaponItem = WeaponItem::DynamicCast( pFact->GetTargetObject() );
		if ( !pWeaponItem )
		{
			AIASSERT( 0, pFact->GetTargetObject(), "Object is not a WeaponItem instance." );
			return;
		}

		// Verify that this WeaponItem represents an improvement over the 
		// current best WeaponItem found.

		const AIDB_AIWeaponRecord* pAIWeaponRecord = AIWeaponUtils::GetAIWeaponRecord( 
			pWeaponItem->GetWeaponRecord(), 
			m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );
		
		if ( !pAIWeaponRecord )
		{
			return;
		}

		// Verify that the AI will be able to use this weapon in its primary form.  
		// One of the following must be true to consider using this weaponitem:
		//	The AI must be given ammo
		//	The weapon must allow generation
		//	The AI must already have ammo for it.
		// TODO: Handle potential secondary form use.

		if ( 0 == pWeaponItem->GetAmmoAmount() 
			&& !pAIWeaponRecord->bAllowAmmoGeneration 
			&& !AIWeaponUtils::HasAmmo( m_pAI, pWeaponItem->GetWeaponRecord() ))
		{
			return;
		}

		// Already found a better weapon record
		// TODO: Resolve handling equal records if needed (distance based?)

		if ( pAIWeaponRecord->nPreference <= m_nBestWeaponPreference )
		{
			return;
		}
		
		// WeaponItem has a node, and the node is invalid.

		if (pFact->GetSourceObject())
		{
			AINode* pNode = AINode::HandleToObject(pFact->GetSourceObject());
			if (!pNode->IsNodeValid(m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All))
			{
				return;
			}
		}

		// Someone has already owns this WeaponItem.

		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_Ownership );
		queryFact.SetTargetObject( pFact->GetTargetObject() );
		if ( NULL != g_pAIWorkingMemoryCentral->FindWMFact( queryFact ) )
		{
			return;
		}

		m_nBestWeaponPreference = pAIWeaponRecord->nPreference;
		m_pBestWeaponItem = pWeaponItem;
	}

	CAI*		m_pAI;
	WeaponItem* m_pBestWeaponItem;
	int			m_nBestWeaponPreference;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActivityExchangeWeapons::CAIActivityExchangeWeapons()
{
}

CAIActivityExchangeWeapons::~CAIActivityExchangeWeapons()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIActivityExchangeWeapons::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIActivityExchangeWeapons
//              
//----------------------------------------------------------------------------

void CAIActivityExchangeWeapons::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hSelectedAI );
	LOAD_HOBJECT( m_hSelectedWeaponItem );
}

void CAIActivityExchangeWeapons::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hSelectedAI );
	SAVE_HOBJECT( m_hSelectedWeaponItem );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAbstract::InitActivity
//
//	PURPOSE:	Initialize the activity.
//
// ----------------------------------------------------------------------- //

void CAIActivityExchangeWeapons::InitActivity()
{
	super::InitActivity();

	m_hSelectedAI = NULL;
	m_hSelectedWeaponItem = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::FindActivityParticipants
//
//	PURPOSE:	Return true if successfully found participants.
//
// ----------------------------------------------------------------------- //

bool CAIActivityExchangeWeapons::FindActivityParticipants()
{
	// Look for an AI who needs a new weapon.  If there are multiple, prefer
	// the AI which can be improved the most by picking up a particular 
	// WeaponItem

	float flWeaponPreferencePerEnemy = CalculateWeaponPreferencePerEnemy();

	// TODO: Allow multiple AIs to get weapons at once.

	int nBestImprovement = 0;
	HOBJECT hBestAI = NULL;
	HOBJECT hBestWeaponItem = NULL;

	for( uint32 iPotential = 0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		// Sanity Checks:

		// Object is not an AI.

		if ( !IsAI( m_aPotentialParticipants[iPotential] ) )
		{
			continue;
		}

		CAI* pCurAI = CAI::DynamicCast( m_aPotentialParticipants[iPotential] );

		// Failed to convert handle to AI.

		if( !pCurAI )
		{
			continue;
		}

		// Skip AI who have a scripted task (ambush or cover).

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetFactFlags( kFactFlag_Scripted );
		if( pCurAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
		{
			continue;
		}

		// AI does not need a better weapon.

		if ( !NeedsBetterWeapon( pCurAI, flWeaponPreferencePerEnemy ) )
		{
			continue;
		}

		// Failed to find the best WeaponItem this AI can consider picking up 
		// right now.

		WeaponItem* pWeaponItem = GetBestWeaponItem( pCurAI );
		if ( NULL == pWeaponItem )
		{
			continue;
		}

		// Best WeaponItem is not better than the current weapon.
		
		int nImprovement = GetWeaponImprovement( pCurAI, pWeaponItem->GetWeaponRecord() );
		if ( 0 == nImprovement)
		{
			continue;
		}

		// Already found an AI which can be improved more by picking up a weapon.

		if ( nImprovement < nBestImprovement )
		{
			continue;
		}

		// Success!  Record the best information so far.

		hBestAI = pCurAI->GetHOBJECT();
		hBestWeaponItem = pWeaponItem->GetHOBJECT();
		nBestImprovement = nImprovement;
	}
	
	// Failed to find an AI or weapon.

	if ( !hBestWeaponItem || !hBestAI )
	{
		return false;
	}

	// Store off the state information which needs to be maintained for activity
	// activation.

	m_hSelectedAI = hBestAI;
	m_hSelectedWeaponItem = hBestWeaponItem;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::ActivateActivity
//
//	PURPOSE:	Return true if the activity activates successfully.
//
// ----------------------------------------------------------------------- //

bool CAIActivityExchangeWeapons::ActivateActivity()
{
	if ( !super::ActivateActivity() )
	{
		return false;
	}

	if ( !m_hSelectedAI )
	{
		return false;
	}

	// Build the string.

	const char* const pszWeaponItemName = GetObjectName(m_hSelectedWeaponItem);
	if ( NULL == pszWeaponItemName)
	{
		return false;
	}

	char szMsg[128];
	LTSNPrintF( szMsg, LTARRAYSIZE( szMsg ), "%s %s", "PRIVATE_EXCHANGEWEAPON", pszWeaponItemName);

	// Order the AI to pick up the weapon.

	CAI* pAI = CAI::DynamicCast( m_hSelectedAI );
	g_pCmdMgr->QueueMessage( pAI, pAI, szMsg );
	AITRACE( AIShowActivities, ( m_hSelectedAI, "Ordered to pickup WeaponItem" ) );

	// Initialize activity.

	m_eActStatus = kActStatus_Initialized;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::DeactivateActivity
//
//	PURPOSE:	Deactivate an active activity.
//
// ----------------------------------------------------------------------- //

void CAIActivityExchangeWeapons::DeactivateActivity()
{
	super::DeactivateActivity();

	if( m_hSelectedAI )
	{
		CAI* pAI = CAI::DynamicCast( m_hSelectedAI );

		// Remove the task.  Do this explictly (instead of with the 
		// CLEARTASK message) to avoid clearing all types.  We only want 
		// to clear the type this activity added.

		if ( pAI )
		{
			CAIWMFact queryTaskFact;
			queryTaskFact.SetFactType( kFact_Task );
			queryTaskFact.SetTaskType( kTask_ExchangeWeapon );
			pAI->GetAIWorkingMemory()->ClearWMFacts( queryTaskFact );
		}

		m_hSelectedAI = NULL;
	}

	m_hSelectedAI = NULL;
	m_hSelectedWeaponItem = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::UpdateActivity
//
//	PURPOSE:	Return true if Activity should continue running.
//
// ----------------------------------------------------------------------- //

bool CAIActivityExchangeWeapons::UpdateActivity()
{
	if ( !super::UpdateActivity() )
	{
		return false;
	}

	// (Failure) Abort if the AI is gone or dead.

	if ( m_hSelectedAI == NULL )
	{
		return false;
	}

	CAI* pAI = CAI::DynamicCast( m_hSelectedAI );
	if ( !pAI )
	{
		return false;
	}

	if ( pAI->IsDead() )
	{
		return false;
	}

	// (Success) WeaponItem is gone.

	if ( m_hSelectedWeaponItem == NULL )
	{
		return false;
	}

	//
	// Check progress.
	//

	switch( m_eActStatus )
	{
	case kActStatus_Initialized:
		{
			m_eActStatus = kActStatus_Updating;
		}
		break;
	
	case kActStatus_Updating:
		{
			// (Unknown) Abort if the AI removed the task.

			CAIWMFact queryTaskFact;
			queryTaskFact.SetFactType( kFact_Task );
			queryTaskFact.SetTaskType( kTask_ExchangeWeapon );
			queryTaskFact.SetTargetObject( m_hSelectedWeaponItem );
			if ( NULL == pAI->GetAIWorkingMemory()->FindWMFact( queryTaskFact ) )
			{
				return false;
			}
		}

		break;
	}

	// Continue;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::NeedsBetterWeapon
//
//	PURPOSE:	Returns true if the passed in AI needs a better weapon.
//
// ----------------------------------------------------------------------- //

bool CAIActivityExchangeWeapons::NeedsBetterWeapon(CAI* pAI, float flSquadWeaponPreferencePerEnemy ) const
{
	HWEAPON hCurrentBestWeapon = AIWeaponUtils::GetBestPrimaryWeapon( pAI );

	const AIDB_AIWeaponRecord* pCurrentRecord = AIWeaponUtils::GetAIWeaponRecord( 
		hCurrentBestWeapon, 
		pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );

	int nCurrentWeaponPreference = pCurrentRecord ? pCurrentRecord->nPreference : 0;

	// AI have a character target that it can see from its weapon..

	if( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	if( !pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	CCharacter* pEnemy = CCharacter::DynamicCast( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	if ( !pEnemy ) 
	{
		return false;
	}

	HWEAPON hWeapon = pEnemy->GetArsenal()->GetCurWeapon() ? pEnemy->GetArsenal()->GetCurWeapon()->GetWeaponRecord() : NULL;
	const AIDB_AIWeaponRecord* pEnemyWeaponRecord = AIWeaponUtils::GetAIWeaponRecord( 
		hWeapon 
		, pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());

	// Evaluate the AI, its squads weapon:
	// ( Enemy Health Ratio / SquadWeaponPreference ) / ( AI Health Ratio / PlayerWeaponPreference ) * ( WeaponUpgradeDesire[0-1] )

	float flWeaponUpgradeDesire = 1.0f;

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_ObtainBetterWeapon );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact )
	{
		flWeaponUpgradeDesire = pFact->GetConfidence( CAIWMFact::kFactMask_DesireType );
	}

	float flAIWeaponPreference = flSquadWeaponPreferencePerEnemy;
	float flPlayerWeaponPreference = pEnemyWeaponRecord ? (float)pEnemyWeaponRecord->nPreference : 1.0f;
	float flEnemyHPRatio = pEnemy->GetDestructible()->GetHitPoints() / pEnemy->GetDestructible()->GetMaxHitPoints();
	float flAIHPRatio = pAI->GetDestructible()->GetHitPoints() / pAI->GetDestructible()->GetMaxHitPoints();
	float flRatio = ( flEnemyHPRatio / flAIWeaponPreference ) / ( flAIHPRatio / flPlayerWeaponPreference );
	
	return ( flRatio * flWeaponUpgradeDesire > 1.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::GetBestWeaponItem
//
//	PURPOSE:	Returns a pointer to the best WeaponItem this AI can 
//				use right now.  Returns NULL if there is no such item.
//
// ----------------------------------------------------------------------- //

WeaponItem* CAIActivityExchangeWeapons::GetBestWeaponItem( CAI* pAI ) const
{
	BestWeaponItemCollector BestWeaponItem( pAI );
	pAI->GetAIWorkingMemory()->CollectFact( BestWeaponItem );
	return BestWeaponItem.m_pBestWeaponItem;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::GetWeaponImprovement
//
//	PURPOSE:	Returns the difference between the AIs current weapon and
//				the passed in handle.
//
// ----------------------------------------------------------------------- //

int CAIActivityExchangeWeapons::GetWeaponImprovement( CAI* pAI, HRECORD hExchangeWeaponRecord ) const
{
	HWEAPON hCurrentBestWeapon = AIWeaponUtils::GetBestPrimaryWeapon( pAI );

	const AIDB_AIWeaponRecord* pExchangeRecord = AIWeaponUtils::GetAIWeaponRecord( 
		hExchangeWeaponRecord, 
		pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );

	const AIDB_AIWeaponRecord* pCurrentRecord = AIWeaponUtils::GetAIWeaponRecord( 
		hCurrentBestWeapon, 
		pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );

	int nCurrentWeaponPreference = pCurrentRecord ? pCurrentRecord->nPreference : 0;
	int nExchangeWeaponPreference = pExchangeRecord ? pExchangeRecord->nPreference : 0;

	return nExchangeWeaponPreference - nCurrentWeaponPreference;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::CalculateWeaponPreferencePerEnemy
//
//	PURPOSE:	Returns the sum of the squads weapon Preferences, divided by 
//				the number of enemies the squad is engaging.
//
// ----------------------------------------------------------------------- //

float CAIActivityExchangeWeapons::CalculateWeaponPreferencePerEnemy() const
{
	struct EnemyElement
	{
		EnemyElement()
		{
			hEnemy = NULL;
			nSquadMembersTargetingCount = 0;
		}

		HOBJECT hEnemy;
		int		nSquadMembersTargetingCount;
	};

	int nEnemyCount = 0;
	EnemyElement hEnemyList[MAX_AI_SQUAD_SIZE];

	// Count the number of enemies the squad is engaging
	int nEnemies = 0;
	float flWeaponPreference = 0.f;
	for( uint32 iEachParticipant = 0; iEachParticipant < m_cPotentialParticipants; ++iEachParticipant )
	{
		// Sanity Checks:

		// Object is not an AI.

		if ( !IsAI( m_aPotentialParticipants[iEachParticipant] ) )
		{
			continue;
		}

		CAI* pCurAI = CAI::DynamicCast( m_aPotentialParticipants[iEachParticipant] );
		if ( !pCurAI )
		{
			continue;
		}

		//
		// Add the AIs weapon preference to the squad total.
		//

		HWEAPON hCurrentBestWeapon = AIWeaponUtils::GetBestPrimaryWeapon( pCurAI );
		
		const AIDB_AIWeaponRecord* pCurrentRecord = AIWeaponUtils::GetAIWeaponRecord( 
			hCurrentBestWeapon,
			pCurAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());

		flWeaponPreference += pCurrentRecord ? pCurrentRecord->nPreference : 0.f;

		//
		// Add the AIs enemy to the list.
		//

		// Target is not an AI.

		if( !pCurAI->HasTarget( kTarget_Character ) )
		{
			continue;
		}

		HOBJECT hEnemy = pCurAI->GetAIBlackBoard()->GetBBTargetObject();

		// If the enemy is already in the list, add to the number of members 
		// targeting him.

		bool bFound = false;
		for ( int nEachEnemy = 0; nEachEnemy < nEnemyCount; ++nEachEnemy )
		{
			if ( hEnemy == hEnemyList[ nEachEnemy ].hEnemy )
			{
				++hEnemyList[ nEachEnemy ].nSquadMembersTargetingCount;
				bFound = true;
			}
		}

		// If this is a new enemy, add him to the list.

		if ( !bFound )
		{
			hEnemyList[ nEnemyCount ].hEnemy = hEnemy;
			hEnemyList[ nEnemyCount ].nSquadMembersTargetingCount = 1;
			++nEnemyCount;
		}
	}

	return ( nEnemyCount == 0 ? FLT_MAX : ( flWeaponPreference / nEnemyCount ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityExchangeWeapons::ClearDeadAI
//
//	PURPOSE:	Clear handles to dead AI.
//
// ----------------------------------------------------------------------- //

void CAIActivityExchangeWeapons::ClearDeadAI()
{
	super::ClearDeadAI();

	if( m_hSelectedAI && IsDeadAI( m_hSelectedAI ) )
	{
		m_hSelectedAI = NULL;
	}
}
