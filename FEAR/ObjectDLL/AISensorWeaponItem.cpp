// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorWeaponItem.cpp
//
// PURPOSE : 
//
// CREATED : 7/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorWeaponItem.h"
#include "AIStimulusMgr.h"
#include "AI.h"
#include "AIWorkingMemory.h"
#include "AIBlackBoard.h"
#include "AINodeMgr.h"
#include "AINodePickupWeapon.h"
#include "AIWorkingMemoryCentral.h"
#include "WeaponItems.h"

LINKFROM_MODULE(AISensorWeaponItem);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorWeaponItem, kSensor_WeaponItem );

struct ClosestWeaponItem
{
	ClosestWeaponItem(CAI* pAI) : 
		m_pFact(NULL)
		, m_pAI(pAI)
		, m_flDistanceSqr(FLT_MAX)
		, m_vPos(pAI->GetPosition())
	{
	}

	void operator()(CAIWMFact* pFact)
	{
		// Return if:
		//	Fact is NULL 
		//	Fact is not knowledge
		//	Knowledge is not about a weapon in the world
		//	Fact is timed out
		if (!pFact
			|| kFact_Knowledge != pFact->GetFactType()
			|| kKnowledge_WeaponItem != pFact->GetKnowledgeType()
			|| g_pLTServer->GetTime() < pFact->GetTime())
		{
			return;
		}

		// Already have a closer WeaponItem

		float flDistanceToWeaponItemSqr = (pFact->GetPos() - m_vPos).MagSqr(); 
		if (m_flDistanceSqr < m_flDistanceSqr)
		{
			return;
		}

		// WeaponPickup has a node, and the node is invalid.

		if (pFact->GetSourceObject())
		{
			AINode* pNode = AINode::HandleToObject(pFact->GetSourceObject());
			if (!pNode->IsNodeValid(m_pAI, m_vPos, m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All))
			{
				return;
			}
		}
		else
		{
			// This fixes weapon items not supported for pickup
			// (with a priority less than 0) being picked up.  
			// This does NOT prevent AIs from picking up items 
			// with nodes pointing at them.  These are assumed 
			// good (as level designers want the AI to pick them up).

 			WeaponItem* pWeaponItem = WeaponItem::DynamicCast( pFact->GetTargetObject() );
			if ( pWeaponItem )
			{
				const AIDB_AIWeaponRecord* pAIWeaponRecord = AIWeaponUtils::GetAIWeaponRecord( 
					pWeaponItem->GetWeaponRecord(), 
					m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );

				// AI doesn't know about this weapon type.  This is an error.

				if ( !pAIWeaponRecord )
				{
					const char* pszWeaponRecordName = g_pWeaponDB->GetRecordName( pWeaponItem->GetWeaponRecord() );
					AIASSERT1( pAIWeaponRecord, m_pAI->GetHOBJECT(), "ClosestWeaponItem: WeaponItem of type %s does not have a AIWeaponRecord.",  
						( pszWeaponRecordName != NULL ) ? pszWeaponRecordName : "<null>" );
					return;
				}

				// Weapon item is considered 'useless' by the AI.

				if ( pAIWeaponRecord->nPreference <= 0 )
				{
					return;
				}
			}
		}

		// Someone else has already owns this WeaponItem.

		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_Ownership );
		queryFact.SetTargetObject( pFact->GetTargetObject() );
		CAIWMFact* pWeaponItemOwnerFact = g_pAIWorkingMemoryCentral->FindWMFact( queryFact );
		if ( NULL != pWeaponItemOwnerFact 
			&& pWeaponItemOwnerFact->GetSourceObject() != m_pAI->GetHOBJECT() )
		{
			return;
		}

		// Store the fact.

		m_flDistanceSqr = flDistanceToWeaponItemSqr;
		m_pFact = pFact;
	}

	CAI*		m_pAI;
	LTVector	m_vPos;
	float		m_flDistanceSqr;
	CAIWMFact*	m_pFact;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetPickupWeaponNode
//
//	PURPOSE:	Utility function to correlate a WeaponItem with a 
//				AINodeWeaponPickup instance.  Some statically WeaponItems
//				may have a node associated with them.  If one does, the 
//				WeaponItem must be acquired through the node.  This 
//				function handles finding the AINode (if any) which points 
//				at the passed in WeaponItem.
//
// ----------------------------------------------------------------------- //

static HOBJECT GetPickupWeaponNode(HOBJECT hWeaponItem)
{
	AINODE_LIST* pList = g_pAINodeMgr->GetNodeList(kNode_PickupWeapon);
	if (pList)
	{
		for (AINODE_LIST::iterator itEach = pList->begin(); itEach != pList->end(); ++itEach)
		{
			AINode* pNode = *itEach;
			
			// AINode does not exist.

			if (!pNode)
			{
				continue;
			}

			// AINode is not a AINodePickupWeapon

			if (!IsKindOf(pNode->GetHOBJECT(), "AINodePickupWeapon"))
			{
				AIASSERT(0, pNode->GetHOBJECT(), "GetPickupWeaponNode : Node in list is not a AINodePickupWeapon instance.");
				continue;
			}

			AINodePickupWeapon* pPickupWeaponNode = (AINodePickupWeapon*)pNode;

			// hWeaponItem is not a match.

			if ( hWeaponItem != pPickupWeaponNode->GetWeaponItem())
			{
				continue;
			}

			return pPickupWeaponNode->GetHOBJECT();
		}
	}

	return NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorWeaponItem::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorWeaponItem::CAISensorWeaponItem()
{
}

CAISensorWeaponItem::~CAISensorWeaponItem()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorWeaponItem::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorWeaponItem
//              
//----------------------------------------------------------------------------

void CAISensorWeaponItem::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAISensorWeaponItem::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorWeaponItem::GetSenseDistSqr
//
//	PURPOSE:	Do not handle the radius in the SenseDist
//
// ----------------------------------------------------------------------- //

float CAISensorWeaponItem::GetSenseDistSqr( float fStimulusRadius )
{
	return FLT_MAX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorWeaponItem::DoComplexCheck
//
//	PURPOSE:	Handle the radius check, as it depends on if the 
//				WeaponItem has an associated node or not.
//
// ----------------------------------------------------------------------- //

bool CAISensorWeaponItem::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier )
{
	if (!super::DoComplexCheck( pStimulusRecord, pfRateModifier) )
	{
		return false;
	}

	// SanityCheck
	
	if (!pStimulusRecord)
	{
		return false;
	}

	// Handle the radius.  If this has an node associated with it, use the 
	// node for the radius check.  Otherwise, use the sensor's SmartObject.

	float flRadiusSqr = 0.f;
	HOBJECT hNode = GetPickupWeaponNode(pStimulusRecord->m_hStimulusSource);
	if ( hNode)
	{
		AINode* pNode = AINode::HandleToObject(hNode);
		flRadiusSqr = pNode->GetRadiusSqr();
	}
	else
	{
		AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pSensorRecord->eSmartObjectID );
		if (!pSmartObjectRecord)
		{
			AIASSERT(0, m_pAI->GetHOBJECT(), "CAISensorWeaponItem::GetSenseDistSqr : Sensor requires a SmartObjectRecord.");
			return 0.f;
		}

		flRadiusSqr = pSmartObjectRecord->fMaxDist*pSmartObjectRecord->fMaxDist;
	}

	// If the object is outside the radius, return false.
	
	float fDistanceSqr = pStimulusRecord->m_vStimulusPos.DistSqr( m_pAI->GetPosition() );
	if( fDistanceSqr > flRadiusSqr )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorWeaponItem::CreateWorkingMemoryFact
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorWeaponItem::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Sanity check.

	if( !pStimulusRecord )
	{
		return NULL;
	}

	// Find an existing memory for this weapon.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_WeaponItem ); 
	factQuery.SetTargetObject( GetPickupWeaponNode(pStimulusRecord->m_hStimulusSource) );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for this stimulus.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
		if (pFact)
		{
			pFact->SetKnowledgeType( kKnowledge_WeaponItem ); 
			pFact->SetSourceObject( GetPickupWeaponNode(pStimulusRecord->m_hStimulusSource) );
		}
	}

	if( pFact )
	{
		pFact->SetTime(g_pLTServer->GetTime());
	}

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorWeaponItem::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
//	TODO:		Consider moving this all of this logic into a 
//			separate sensor if it becomes any more complex.
//
// ----------------------------------------------------------------------- //

bool CAISensorWeaponItem::UpdateSensor()
{
	// Do not bail if super returns false.
	// AISensorAbstractStimulatable::UpdateSensor returns false
	// if there are no delayed reactions to process.

	super::UpdateSensor();

	//
	// Clear out any facts about other usable weapons in the world.
	//

	CAIWMFact factQueryClearUseableWeaponItems;
	factQueryClearUseableWeaponItems.SetFactType(kFact_Knowledge);
	factQueryClearUseableWeaponItems.SetKnowledgeType(kKnowledge_UsableWeaponItem);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts(factQueryClearUseableWeaponItems);


	//
	// Clear out any facts about weaponitems which no longer exist.
	//
	
	CAIWMFact factQueryClearWeaponItemsNULL;
	factQueryClearWeaponItemsNULL.SetFactType(kFact_Knowledge);
	factQueryClearWeaponItemsNULL.SetKnowledgeType(kKnowledge_WeaponItem);
	factQueryClearWeaponItemsNULL.SetTargetObject(NULL);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts(factQueryClearWeaponItemsNULL);


	//
	// First handle any WeaponExchange tasks.  If there is one, execute it.
	//

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Task );
	queryFact.SetTaskType( kTask_ExchangeWeapon );
	CAIWMFact* pTaskFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pTaskFact )
	{
		CAIWMFact queryFact;
		queryFact.SetKnowledgeType( kKnowledge_UsableWeaponItem ); 
		queryFact.SetSourceObject( GetPickupWeaponNode( pTaskFact->GetTargetObject() ) );
		queryFact.SetTargetObject( pTaskFact->GetTargetObject() );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
		if ( !pFact )
		{
			pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
		}

		// Create a fact about the weapon which should be evaluated by the AIActionTarget

		if ( pFact )
		{
			pFact->SetKnowledgeType( kKnowledge_UsableWeaponItem ); 
			pFact->SetSourceObject( GetPickupWeaponNode( pTaskFact->GetTargetObject() ) );
			pFact->SetTargetObject( pTaskFact->GetTargetObject() );
			pFact->SetTime(g_pLTServer->GetTime());

			// Reevaluate the Target.

			m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget(true);
		}

		return true;
	}

	//
	// Handle autonomously picking up a weapon.  This occurs when an AI is unarmed.
	//

	// Don't update unless the AI has a target of some kind.

	if ( !m_pAI->HasTarget(kTarget_All) )
	{
		return false;
	}

	// AI already has a weapon.

	if ( AIWeaponUtils::HasWeaponType(m_pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER)
		|| AIWeaponUtils::HasWeaponType(m_pAI, kAIWeaponType_Melee, AIWEAP_CHECK_HOLSTER))
	{
		return false;
	}

	// Determine if there is a fact about a weapon on the ground which is 
	// not timed out.  If there is, give the AI a chance to reevaluate 
	// the target to determine if it should target this weapon.

	ClosestWeaponItem WeaponCollector(m_pAI);
	m_pAI->GetAIWorkingMemory()->CollectFact(WeaponCollector);
	if (WeaponCollector.m_pFact)
	{
		LTVector vToWeaponItem = WeaponCollector.m_pFact->GetPos() - m_pAI->GetPosition();
		if ( vToWeaponItem.MagSqr() < GetSenseDistSqr(1.0f))
		{
			// Create a fact about the weapon which should be evaluated by the AIActionTarget

			CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
			if (pFact)
			{
				pFact->SetKnowledgeType( kKnowledge_UsableWeaponItem ); 
				pFact->SetSourceObject( WeaponCollector.m_pFact->GetSourceObject() );
				pFact->SetTargetObject( WeaponCollector.m_pFact->GetTargetObject() );
				pFact->SetTime(g_pLTServer->GetTime());
			}

			// Reevaluate the Target.

			m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget(true);
		}
	}

	return true;
}
