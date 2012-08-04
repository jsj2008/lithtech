// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalStalk.cpp
//
// PURPOSE : Stalking is the act of finding a covered location while 
//				traversing a path.  Stalking interrupts the behavior to
//				get the AI to a safe place.  From here, the AI will move 
//				along the path.
//
// CREATED : 5/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalStalk.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIDB.h"
#include "AIGoalMgr.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalStalk, kGoal_Stalk );

// TODO: Move to the AIDatabase?
const float kThreatTooCloseThresholdSqr = 256.f*256.f;

struct StalkingNodeFinder
{
public:
	StalkingNodeFinder(CAI* pAI, HOBJECT hThreat, HOBJECT hIgnoreNode, float flDistanceToDestinationSqr)
	{
		m_pAI = pAI;
		m_hThreat = hThreat;
		m_flDistanceToDestinationSqr = flDistanceToDestinationSqr;
		m_hIgnoreStalkingNode = hIgnoreNode;

		m_pBestFact = NULL;
		m_flBestDistanceFromAISqr = FLT_MAX;
	}

	int operator()(CAIWMFact* pFact)
	{
		// Ignore facts that are not node facts.
		// Ignore node facts with the wrong node type.
		// Ignore node facts whose object matches the hExclude.

		if( ( pFact->GetFactType() != kFact_Node ) ||
			( pFact->GetNodeType() != kNode_Stalk ))
		{
			return 0;
		}

		AINode* pNode = AINode::HandleToObject( pFact->GetTargetObject() );
		if( !pNode )
		{
			return 0;
		}

		// Bail if node is locked by someone else, disabled, or timed out.

		// TODO: Handle the AI locking the node.  This can work when code to 
		// handle an invalid node is written.
		if (pNode->GetHOBJECT() == m_hIgnoreStalkingNode)
		{
			return false;
		}

		if (pNode->IsNodeLocked()/* && pNode->GetLockingAI() != m_pAI->GetHOBJECT()*/)
		{
			return false;
		}

		if( pNode->IsNodeDisabled() || pNode->IsNodeTimedOut() )
		{
			return false;
		}

		// Require the AI be in the radius or region

		if (!pNode->IsAIInRadiusOrRegion( m_pAI, m_pAI->GetPosition(), 1.f ))
		{
			return 0;
		}

		// If AI is aware of a threat, ignore nodes that are invalid.

		if( m_hThreat && 
			( !pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), m_hThreat, kThreatPos_TargetPos, kNodeStatus_All ^ kNodeStatus_ThreatOutsideFOV) ) )
		{
			return 0;
		}

		// Failed to find a valid destination position.

		LTVector vDestination;
		if (!pNode->GetDestinationPosition(m_pAI, m_pAI->GetAIBlackBoard()->GetBBTargetPosition(), vDestination))
		{
			return 0;
		}

		// The stalking position is further from the AIs goal than the AI 
		// currently is (don't run backwards to stalk).

		float flNodeDestToAIDestDistSqr = vDestination.DistSqr(m_pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPosition());
		if (flNodeDestToAIDestDistSqr > m_flDistanceToDestinationSqr)
		{
			return 0;
		}

		// Node is behind the AI.

		if (0 > m_pAI->GetForwardVector().Dot( vDestination - m_pAI->GetPosition()))
		{
			return 0;
		}

		// Already found a closer node.

		float flDistanceFromAISqr = vDestination.DistSqr(m_pAI->GetPosition());
		if (flDistanceFromAISqr > m_flBestDistanceFromAISqr)
		{
			return 0;
		}

		m_vBestPosition = vDestination;
		m_flBestDistanceFromAISqr = flDistanceFromAISqr;
		m_pBestFact = pFact;

		return 0;
	}

	CAIWMFact* GetFact()
	{
		return m_pBestFact;
	}

	const LTVector& GetPosition() const
	{
		return m_vBestPosition;
	}

private:
	CAI*		m_pAI;
	HOBJECT		m_hThreat;
	HOBJECT		m_hIgnoreStalkingNode;
	float		m_flDistanceToDestinationSqr;
	
	LTVector	m_vBestPosition;
	CAIWMFact*	m_pBestFact;
	float		m_flBestDistanceFromAISqr;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStalk::Con/destructor
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

CAIGoalStalk::CAIGoalStalk()
{
}

CAIGoalStalk::~CAIGoalStalk()
{
}

void CAIGoalStalk::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_VECTOR(m_vPendingStalkingPosition);
	SAVE_HOBJECT(m_hPendingStalkingNode);
	SAVE_HOBJECT(m_hStalkingNode);
	SAVE_HOBJECT(m_hPreviousStalkingNode);
}

void CAIGoalStalk::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_VECTOR(m_vPendingStalkingPosition);
	LOAD_HOBJECT(m_hPendingStalkingNode);
	LOAD_HOBJECT(m_hStalkingNode);
	LOAD_HOBJECT(m_hPreviousStalkingNode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStalk::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalStalk::CalculateGoalRelevance()
{
	// The target MUST be a character, regardless of current goal status.

	if ( !m_pAI->HasTarget( kTarget_Character ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Goal in progress, and the node is still valid.

	if ( m_pAI->GetGoalMgr()->IsCurGoal(this) )
	{
		if (!IsWSSatisfied(m_pAI->GetAIWorldState()))
		{
			AINode* pNode = AINode::HandleToObject(m_hStalkingNode);
			if (pNode)
			{
				if (pNode->IsNodeValid(m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All))
				{
					m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
					return;
				}
			}
		}
	}

	// Damaged recently

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pDamagedFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pDamagedFact 
		&& ( DidDamage(m_pAI, pDamagedFact) )
		&& ( pDamagedFact->GetUpdateTime() < g_pLTServer->GetTime() - 4.f ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// No target object

	if (NULL == m_pAI->GetAIBlackBoard()->GetBBTargetObject())
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Enemy is too close (note that this NOT the destination, this is the 
	// threat -- AIs next to their enemies should not attempt to stalk)

	float flAIToAIDestDistSqr = m_pAI->GetPosition().DistSqr(m_pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPosition());
	if ( flAIToAIDestDistSqr < kThreatTooCloseThresholdSqr)
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Stalking is relevent when the AI is in the process of going somewhere

	if( m_pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Failed to find a potential stalking node.

	StalkingNodeFinder finder(m_pAI, m_pAI->GetAIBlackBoard()->GetBBTargetObject(), m_hPreviousStalkingNode, flAIToAIDestDistSqr);
	m_pAI->GetAIWorkingMemory()->CollectFact(finder);
	CAIWMFact* pFact = finder.GetFact();
	if (!pFact)
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	m_vPendingStalkingPosition = finder.GetPosition();
	m_hPendingStalkingNode = pFact->GetTargetObject();
	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStalk::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalStalk::ActivateGoal( )
{
	super::ActivateGoal();

	m_hStalkingNode = m_hPendingStalkingNode;
	m_hPreviousStalkingNode = m_hPendingStalkingNode;

	// Add Stalk position knowledge to allow later systems to determine where 
	// the AI is going, regardless of the actions used in the plan
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact(kFact_Knowledge);
	if (pFact)
	{
		pFact->SetKnowledgeType(kKnowledge_StalkPosition);
		pFact->SetTargetObject(m_hStalkingNode);
		pFact->SetPos(m_vPendingStalkingPosition);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStalk::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalStalk::DeactivateGoal( )
{
	super::DeactivateGoal();

	// Clear out facts about stalking positions when this goal deactivates.
	CAIWMFact factQuery;
	factQuery.SetKnowledgeType(kKnowledge_StalkPosition);
	factQuery.SetTargetObject(m_hStalkingNode);
	m_pAI->GetAIWorkingMemory()->ClearWMFacts(factQuery);

	m_hStalkingNode = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStalk::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalStalk::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_hPendingStalkingNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStalk::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalStalk::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_UsingObject, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue && ( pProp->hWSValue == m_hStalkingNode ) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalStalk::IsPlanValid
//
//	PURPOSE:	Return true if the plan is still valid.  This step is 
//				required as the current action does not know about the 
//				intent of the goal.  If the intent (getting to an out of 
//				sight node) is invalidated, there fact that the current 
//				action, running to the node, is valid does not matter.
//
// ----------------------------------------------------------------------- //

bool CAIGoalStalk::IsPlanValid()
{
	if (!super::IsPlanValid())
	{
		return false;
	}

	// Insure the stalking node is still valid.

	AINode* pNode = AINode::HandleToObject(m_hStalkingNode);
	if (!pNode)
	{
		return false;
	}

	if(!pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All ) )
	{
		return false;
	}

	return true;
}
