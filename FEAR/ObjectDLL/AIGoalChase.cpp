// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChase.cpp
//
// PURPOSE : AIGoalChase class implementation
//
// CREATED : 3/13/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalChase.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AINodeGuard.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemoryCentral.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalChase, kGoal_Chase );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalChase::CAIGoalChase()
{
}

CAIGoalChase::~CAIGoalChase()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::CalculateGoalRelevance()
{
	// Do not chase if AI is attached to something.

	if( m_pAI->GetAIBlackBoard()->GetBBAttachedTo() != NULL )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// AI is not aware of an enemy.

	if( !m_pAI->HasTarget( kTarget_Character ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Do not chase if AI has a ranged weapon and knows of potential cover.
	// Allow AI to try to chase while incapacitated, as this will give him reason to get up.

	if( AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER ) 
		&& AIWeaponUtils::HasAmmo( m_pAI, kAIWeaponType_Ranged, AIWEAP_CHECK_HOLSTER )
		&& !m_pAI->GetAIBlackBoard()->GetBBIncapacitated() )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Node );
		factQuery.SetNodeType( kNode_Cover );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			m_fGoalRelevance = 0.f;
			return;
		}
	}

	// Do not chase if target is outside of guarded area.

	CAIWMFact factGuardQuery;
	factGuardQuery.SetFactType(kFact_Node);
	factGuardQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factGuardQuery );
	if( pFact && IsAINode( pFact->GetTargetObject() ) )
	{
		HOBJECT hNode = pFact->GetTargetObject();
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
		if( pNode->GetType() == kNode_Guard )
		{
			AINodeGuard* pNodeGuard = (AINodeGuard*)pNode;
			HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
			if( !pNodeGuard->IsCharacterInRadiusOrRegion( hTarget ) )
			{
				m_fGoalRelevance = 0.f;
				return;
			}
		}
	}

	// Chase!

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::ActivateGoal()
{
	super::ActivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject, kWST_bool, true );
	WorldState.SetWSProp( kWSK_WeaponArmed, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalChase::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = NULL;

	pProp = pwsWorldState->GetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject );
	if( !pProp || ( pProp && !pProp->bWSValue ) )
	{
		return false;
	}

	pProp = pwsWorldState->GetWSProp( kWSK_WeaponArmed, m_pAI->m_hObject );
	if( !pProp || ( pProp && !pProp->bWSValue ) )
	{
		return false;
	}

	return true;
}


