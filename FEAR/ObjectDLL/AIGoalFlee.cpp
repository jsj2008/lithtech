// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlee.cpp
//
// PURPOSE : AIGoalFlee class implementation
//
// CREATED : 4/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalFlee.h"
#include "AI.h"
#include "AINode.h"
#include "AINodeGuard.h"
#include "AIPathMgrNavMesh.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFlee, kGoal_Flee );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFlee::CAIGoalFlee()
{
	m_hCachedBestNode = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFlee::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalFlee
//              
//----------------------------------------------------------------------------

void CAIGoalFlee::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_HOBJECT(m_hCachedBestNode);
}

void CAIGoalFlee::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_HOBJECT(m_hCachedBestNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::CalculateGoalRelevance()
{
	// Don't flee if we have not yet seen our target.

	if( m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() == 0.f )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Maintain relevance if goal was already active, and animation
	// is locked.  (e.g. while AI is mounting a node).

	if( m_hNode && m_pAI->GetAnimationContext()->IsLocked() )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// Always try to flee when target is unreachable.

	LTVector vTargetPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
	if( !g_pAIPathMgrNavMesh->HasPath( m_pAI, m_pAI->GetCharTypeMask(), vTargetPos ) )
	{
		super::CalculateGoalRelevance();
		return;
	}

	// Always try to flee when target exist AI's guard radius.

	if( !m_pAI->GetAnimationContext()->IsLocked() )
	{
		CAIWMFact factQueryGuard;
		factQueryGuard.SetFactType(kFact_Node);
		factQueryGuard.SetNodeType(kNode_Guard);
		CAIWMFact* pFactGuard = m_pAI->GetAIWorkingMemory()->FindWMFact( factQueryGuard );
		if( pFactGuard )
		{
			AINodeGuard* pGuardNode = (AINodeGuard*)g_pLTServer->HandleToObject( pFactGuard->GetTargetObject() );
			if( pGuardNode && pGuardNode->IsAIInRadiusOrRegion( m_pAI, m_pAI->GetPosition(), 1.f ) )
			{
				ENUM_NMPolyID ePoly = m_pAI->GetAIBlackBoard()->GetBBTargetTrueNavMeshPoly();
				if( pGuardNode && !pGuardNode->IsPosInRadiusOrRegion( vTargetPos, ePoly, 1.f ) )
				{
					super::CalculateGoalRelevance();
					return;
				}
			}
		}
	}

	// We may be at some node.

	HOBJECT hNode = NULL;
	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp )
	{
		hNode = pProp->hWSValue;
	}

	// Goal has no relevance if we are already at a node, and this goal is not currently active.

	if( hNode && !m_hNode )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Goal has no relevance if we are already at an invalid node.

	if( hNode )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( pNode && ( !pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, m_dwNodeStatus ) ) )
		{
			m_fGoalRelevance = 0.f;
			return;
		}
	}

	// Goal is only relevant if we have the desire to retreat or flee.

	bool bFlee = false;
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Retreat);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		bFlee = true;
	}
	else {
		factQuery.SetDesireType(kDesire_Flee);
		pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			bFlee = true;
		}
	}

	// Default calculation.

	if( bFlee )
	{
		super::CalculateGoalRelevance();
	}

	// Do not flee.

	else 
	{
		m_fGoalRelevance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::ActivateGoal()
{
	// Change the desire to retreat to the desire to flee.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Retreat);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		pFact->SetDesireType( kDesire_Flee, 1.f );
	}

	// Default behavior.

	super::ActivateGoal();

	m_hCachedBestNode = m_hNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFlee::DeactivateGoal()
{
	// Clear the desire to retreat or flee.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && ( pProp->hWSValue == m_hNode ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Desire);
		factQuery.SetDesireType(kDesire_Retreat);
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		factQuery.SetDesireType(kDesire_Flee);
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		// Clear the cache when we reach the node.

		m_hCachedBestNode = NULL;
	}

	// Also clear any cover or ambush tasks.

	CAIWMFact factTaskQuery;
	factTaskQuery.SetFactType(kFact_Task);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factTaskQuery );

	// Default behavior.

	super::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::IsGoalInProgress
//
//	PURPOSE:	Return true if goal is already in progress.
//
// ----------------------------------------------------------------------- //

bool CAIGoalFlee::IsGoalInProgress()
{
	// Initionally skip super::IsGoalInProgress().
	// Allow AI to flee to invalid nodes when necessary.

	if( !CAIGoalUseSmartObject::IsGoalInProgress() )
	{
		return false;
	}

	// Do not check node validity until AI has finished transitioning.

	if( m_pAI->GetAnimationContext()->IsTransitioning() )
	{
		return true;
	}

	if( m_pAI->HasTarget( kTarget_All ) )
	{
		// Node's status is OK from threat.

		uint32 dwNodeStatus = kNodeStatus_Disabled | 
							  kNodeStatus_LockedByOther | 
							  kNodeStatus_ThreatInsideRadius;

		// Leave a damaged node only if there is some other node to goto.

		SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
		if( pProp && 
		   ( pProp->hWSValue == m_hNode ) &&
		   FindBestNode( kNode_Ambush ) )
		{
			dwNodeStatus |= kNodeStatus_Damaged;
		}

		AINodeSmartObject* pNode = (AINodeSmartObject*)g_pLTServer->HandleToObject( m_hNode );
		if( pNode && pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, dwNodeStatus ) )
		{
			return true;
		}
	}

	// Goal is no longer in progress.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFlee::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalFlee::FindBestNode( EnumAINodeType eNodeType )
{
	// Return the cached node if still valid.
	// This ensures an AI keeps heading to the same node if interrupted.

	if( m_hCachedBestNode )
	{
		HOBJECT hThreat = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( m_hCachedBestNode );
		if( pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), hThreat, kThreatPos_TargetPos, m_dwNodeStatus ) )
		{
			return m_hCachedBestNode;
		}
	}

	// Default behavior.

	HOBJECT hNode = super::FindBestNode( eNodeType );
	if( hNode )
	{
		return hNode;
	}

	// Fail-safe.

	uint32 dwNodeStatus = kNodeStatus_Disabled | 
						  kNodeStatus_LockedByOther | 
						  kNodeStatus_ThreatInsideRadius;

	HOBJECT hThreat = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, kNode_Cover, dwNodeStatus, m_hNode, hThreat );
	if( pFact )
	{
		return pFact->GetTargetObject();
	}
	pFact = m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, kNode_Ambush, dwNodeStatus, m_hNode, hThreat );
	if( pFact )
	{
		return pFact->GetTargetObject();
	}

	// No nodes.

	return NULL;
}

