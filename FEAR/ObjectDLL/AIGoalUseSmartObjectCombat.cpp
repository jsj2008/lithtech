// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalUseSmartObjectCombat.cpp
//
// PURPOSE : AIGoalUseSmartObjectCombat class implementation
//
// CREATED : 2/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalUseSmartObjectCombat.h"
#include "AI.h"
#include "AINode.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "AIStimulusMgr.h"
#include "AIDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalUseSmartObjectCombat, kGoal_UseSmartObjectCombat );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObjectCombat::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalUseSmartObjectCombat::CalculateGoalRelevance()
{
	// Goal is only relevant if we are aware of an enemy,
	// or goal was already active.  The node (m_hNode) may be cleared
	// due to failure to find a node, so querying its assignment to 
	// determine if the goal is current may fail and cause the goal
	// to abort prematurely.

	if( IsGoalInProgress() )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	if( m_pAI->HasTarget( kTarget_All ) )
	{
		super::CalculateGoalRelevance();
		return;
	}

	// No enemy present.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObjectCombat::IsGoalInProgress
//
//	PURPOSE:	Return true if goal is already in progress.
//
// ----------------------------------------------------------------------- //

bool CAIGoalUseSmartObjectCombat::IsGoalInProgress()
{
	if( !super::IsGoalInProgress() )
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

			AINodeSmartObject* pNode = (AINodeSmartObject*)g_pLTServer->HandleToObject( m_hNode );
			if( pNode && pNode->IsNodeValid( m_pAI, m_pAI->GetPosition(), m_pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, m_dwNodeStatus ) )
			{
				return true;
			}
		}

	// Goal is no longer in progress.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObjectCombat::FindBestNode
//
//	PURPOSE:	Return the best available node.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIGoalUseSmartObjectCombat::FindBestNode( EnumAINodeType eNodeType )
{
	HOBJECT hThreatObject = NULL;
	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object | kTarget_Disturbance ) )
	{
		hThreatObject = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
	}
	else if( m_pAI->HasTarget( kTarget_CombatOpportunity ) )
	{
		hThreatObject = m_pAI->GetAIBlackBoard()->GetBBCombatOpportunityTarget();
	}

	if (hThreatObject)
	{
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindFactNodeMax( m_pAI, eNodeType, m_dwNodeStatus, m_hNode, hThreatObject );
		if( pFact )
		{
			return pFact->GetTargetObject();
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObjectCombat::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalUseSmartObjectCombat::ActivateGoal()
{
	super::ActivateGoal();

	// Become alert (run).  This is required as disturbances are allowed 
	// to adjust cause the AI to enter this goal.  This insures that AIs 
	// run to the destination node.

	m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Alert );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalUseSmartObjectCombat::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalUseSmartObjectCombat::SetWSSatisfaction( CAIWorldState& WorldState )
{
	super::SetWSSatisfaction( WorldState );

	// Override satisfaction properties.
	// Weapon may be drawn or holstered.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponArmed, m_pAI->m_hObject );
	if( pProp )
	{
		WorldState.SetWSProp( pProp );
	}
}


