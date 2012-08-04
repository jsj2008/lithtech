// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalUseArmoredAutonomous.cpp
//
// PURPOSE : Implementation of the UseArmoredAutonomous goal.
//
// CREATED : 6/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalUseArmoredAutonomous.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalUseArmoredAutonomous, kGoal_UseArmoredAutonomous );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseArmoredAutonomous::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalUseArmoredAutonomous::CAIGoalUseArmoredAutonomous()
{
}

CAIGoalUseArmoredAutonomous::~CAIGoalUseArmoredAutonomous()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseArmoredAutonomous::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalUseArmoredAutonomous::CalculateGoalRelevance()
{
	// Using armored nodes requires a ranged weapon.

	if( !AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Using armored nodes require ammo to fire the ranged weapon.

	if( !AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// If we are not executing the goal, and if we are already at a valid 
	// node of the correct type, then the goal is not relevant.

	if (!IsGoalInProgress())
	{
		SAIWORLDSTATE_PROP* pAtNodeProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNodeType, m_pAI->m_hObject );
		if (pAtNodeProp && ( pAtNodeProp->eAINodeTypeWSValue == m_pGoalRecord->eNodeType) )
		{
			SAIWORLDSTATE_PROP* pAtNode = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
			if (pAtNode)
			{
				AINode* pNode = AINode::HandleToObject(pAtNode->hWSValue);
				if( pNode && pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All ) )
				{
					m_fGoalRelevance = 0.f;
					return;
				}
			}
		}
	}

	// Default Behavior.

	super::CalculateGoalRelevance();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseArmoredAutonomous::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalUseArmoredAutonomous::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Goal is satisfied when we have reached the desired node.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue && ( pProp->hWSValue == m_hNode ) )
	{
		return true;
	}

	return false;
}
