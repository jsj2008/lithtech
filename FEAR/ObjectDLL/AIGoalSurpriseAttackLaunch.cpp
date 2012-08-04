// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSurpriseAttackLaunch.cpp
//
// PURPOSE : Defines the SurpriseAttackLaunch goal.  This goal provides 
//			the AI with a desire to perform a blow from a Surprise node 
//			if the enemy is in place.
//
// CREATED : 2/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalSurpriseAttackLaunch.h"
#include "AINodeSurprise.h"
#include "AIGoalMgr.h"
#include "AIWorldState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSurpriseAttackLaunch, kGoal_SurpriseAttackLaunch );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSurpriseAttackLaunch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSurpriseAttackLaunch::CAIGoalSurpriseAttackLaunch()
{
}

CAIGoalSurpriseAttackLaunch::~CAIGoalSurpriseAttackLaunch()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSurpriseAttackLaunch::CalculateGoalRelevance()
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalSurpriseAttackLaunch::CalculateGoalRelevance()
{
	// Only perform these tests if the AI is not currently executing this 
	// goal.  If the AI is, let it continue.

	if ( !m_pAI->GetGoalMgr()->IsCurGoal( this ) 
		|| IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		// If the AI does not have a character target, this goal is invalid 
		// (only characters can be surprise attacked)

		if ( m_pAI->GetAIBlackBoard()->GetBBTargetType() != kTarget_Character )
		{
			m_fGoalRelevance = 0;
			return;
		}

		// Fail if the AI is not at a node of type Surprise.

		SAIWORLDSTATE_PROP* pAtNodeTypeProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNodeType, m_pAI->GetHOBJECT() );
		if ( !pAtNodeTypeProp || 
			( kNode_Surprise != pAtNodeTypeProp->eAINodeTypeWSValue ) )
		{
			m_fGoalRelevance = 0;
			return;
		}

		// Fail if we cannot get a pointer to the node.

		SAIWORLDSTATE_PROP* pAtNodeProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->GetHOBJECT() );
		if ( !pAtNodeProp ||
			( !pAtNodeProp->hWSValue ) )
		{
			m_fGoalRelevance = 0;
			return;
		}

		// Type should always be correct as we check AtNodeType earlier.  Planner should keep these in sync.
		AIASSERT( 
			IsKindOf( pAtNodeProp->hWSValue, "AINodeSurprise" ),
			pAtNodeProp->hWSValue, "CAIGoalSurpriseAttackLaunch::CalculateGoalRelevance: Object is not an AINodeSurprise instance.  Verify the node type matches expectations." );
		AINodeSurprise* pSurprise = (AINodeSurprise*)g_pLTServer->HandleToObject( pAtNodeProp->hWSValue );
		if ( !pSurprise )
		{
			m_fGoalRelevance = 0;
			return;
		}

		// Fail if the node does not specify an action.
	
		EnumAnimProp eProp = pSurprise->GetSurpriseAttackAnimationProp( 
			m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
		if ( kAP_Invalid == eProp )
		{
			m_fGoalRelevance = 0;
			return;
		}
	}

	// Success!	

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSurpriseAttackLaunch::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalSurpriseAttackLaunch::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Set satisfaction properties:
	// AI must have reacted to the enemy being in place for a surprise attack.

	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_EnemyInPlaceForSurpriseAttack );
}
