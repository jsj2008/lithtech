// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAlert.cpp
//
// PURPOSE : AIGoalAlert class implementation
//
// CREATED : 8/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalAlert.h"
#include "AI.h"
//#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
//#include "AIUtils.h"
//#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalAlert, kGoal_Alert );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlert::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAlert::CAIGoalAlert()
{
}

CAIGoalAlert::~CAIGoalAlert()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAlert::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalAnimate
//              
//----------------------------------------------------------------------------

void CAIGoalAlert::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalAlert::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlert::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalAlert::CalculateGoalRelevance()
{
	// Goal is only relevant if we have an Alert task.

	// Bail if we have no Goto task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Alert );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// No Alert task.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlert::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAlert::ActivateGoal()
{
	super::ActivateGoal();

	m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Set );
}	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlert::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAlert::DeactivateGoal()
{
	super::DeactivateGoal();

	// Clear the AIs world state of the animation just played.

	m_pAI->GetAIWorldState()->SetWSProp( kWSK_AnimLooped, m_pAI->m_hObject, kWST_int, (int)INVALID_MODEL_ANIM );

	// Clear the task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Alert );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlert::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalAlert::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Bail if the Goal's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pGoalRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Bail if animation does not exist.

	CAnimationProps Props;
	Props = pSmartObjectRecord->Props;
	Props.Set( kAPG_Weapon, m_pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	HMODELANIM hAni = m_pAI->GetAnimationContext()->GetAni( Props );
	if( hAni == INVALID_MODEL_ANIM )
	{
		return;
	}

	// Goal is satisfied by looping the alert animation.

	WorldState.SetWSProp( kWSK_AnimLooped, m_pAI->m_hObject, kWST_int, (int)hAni );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAlert::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalAlert::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Loop the alert animation forever.

	return false;
}
