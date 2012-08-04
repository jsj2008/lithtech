// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCritterFlee.cpp
//
// PURPOSE : AIGoalCritterFlee class implementation
//
// CREATED : 02/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalCritterFlee.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCritterFlee, kGoal_CritterFlee );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCritterFlee::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCritterFlee::CAIGoalCritterFlee()
{
	m_hCachedBestNode = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCritterFlee::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalFlee
//              
//----------------------------------------------------------------------------

void CAIGoalCritterFlee::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_HOBJECT(m_hCachedBestNode);
}

void CAIGoalCritterFlee::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_HOBJECT(m_hCachedBestNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCritterFlee::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalCritterFlee::CalculateGoalRelevance()
{
	// Maintain relevance if goal was already active, and animation
	// is locked.  (e.g. while AI is mounting a node).

	if( m_hNode && m_pAI->GetAnimationContext()->IsLocked() )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// Goal is only relevant if we have the desire to retreat or flee.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Flee);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		CAIGoalUseSmartObject::CalculateGoalRelevance();
	}

	// Do not flee.

	else 
	{
		m_fGoalRelevance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCritterFlee::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCritterFlee::ActivateGoal()
{
	// Default behavior.

	super::ActivateGoal();

	m_hCachedBestNode = m_hNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCritterFlee::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCritterFlee::DeactivateGoal()
{
	// Clear the desire to flee.
	// Only clear the desire if it has not been updated
	// since arriving at the node.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && ( pProp->hWSValue == m_hNode ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Desire);
		factQuery.SetDesireType(kDesire_Flee);
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && pFact->GetUpdateTime() < m_pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime() )
		{
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
		}

		// Clear the cache when we reach the node.

		m_hCachedBestNode = NULL;
	}

	m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Relaxed );

	// Default behavior.

	super::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCritterFlee::IsGoalInProgress
//
//	PURPOSE:	Return true if goal is already in progress.
//
// ----------------------------------------------------------------------- //

bool CAIGoalCritterFlee::IsGoalInProgress()
{
	// Initionally skip super::IsGoalInProgress().
	// Allow AI to flee to invalid nodes when necessary.

	if( !CAIGoalUseSmartObject::IsGoalInProgress() )
	{
		return false;
	}

	// Goal is no longer in progress if we have no desire to flee.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Flee);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		return false;
	}

	// Node is invalid if we have updated our desire to flee since
	// arriving at the node.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && 
		( pProp->hWSValue == m_hNode ) &&
		( pFact->GetUpdateTime() > m_pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime() ) )
	{
		return false;
	}

	// Node's status is OK from threat.

	uint32 dwNodeStatus = kNodeStatus_Damaged | 
						  kNodeStatus_Disabled | 
						  kNodeStatus_LockedByOther | 
						  kNodeStatus_ThreatInsideRadius;

	AINodeSmartObject* pNode = (AINodeSmartObject*)g_pLTServer->HandleToObject( m_hNode );
	if( pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), pFact->GetTargetObject(), kThreatPos_TruePos, dwNodeStatus ) )
	{
		return true;
	}

	// Goal is no longer in progress.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCritterFlee::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalCritterFlee::FindBestNode( EnumAINodeType eNodeType )
{
	// Bail if no desire to flee.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Flee);
	CAIWMFact* pFleeFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFleeFact )
	{
		return NULL;
	}

	// Return the cached node if still valid.
	// This ensures an AI keeps heading to the same node if interrupted.

	if( m_hCachedBestNode )
	{
		// Node is invalid if we have updated our desire to flee since
		// arriving at the node.

		bool bNodeStillValid = true;
		SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
		if( pProp && 
			( pProp->hWSValue == m_hCachedBestNode ) &&
			( pFleeFact->GetUpdateTime() > m_pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime() ) )
		{
			bNodeStillValid = false;
		}

		// Node is still valid.

		if( bNodeStillValid )
		{
			HOBJECT hThreat = pFleeFact->GetTargetObject();
			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hCachedBestNode );
			if( pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), hThreat, kThreatPos_TruePos, m_dwNodeStatus ) )
			{
				return m_hCachedBestNode;
			}
		}
	}

	// Find a new node.

	HOBJECT hThreat = pFleeFact->GetTargetObject();
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, kNode_Ambush, m_dwNodeStatus, m_hNode, hThreat );
	if( pFact )
	{
		return pFact->GetTargetObject();
	}

	// No nodes.

	return NULL;
}

