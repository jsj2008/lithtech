// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAnimate.cpp
//
// PURPOSE : AIGoalAnimate class implementation
//
// CREATED : 4/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalAnimate.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "AIGoalMgr.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalAnimate, kGoal_Animate );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SAI_ANIMATION_REQUEST::Constructor
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SAI_ANIMATION_REQUEST::SAI_ANIMATION_REQUEST()
{
	eFactID = kFactID_Invalid;
	hAni = INVALID_MODEL_ANIM;
	bLoop = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAnimate::CAIGoalAnimate()
{
}

CAIGoalAnimate::~CAIGoalAnimate()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAnimate::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalAnimate
//              
//----------------------------------------------------------------------------

void CAIGoalAnimate::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_AnimCurrent.eFactID );
	SAVE_DWORD( m_AnimCurrent.hAni );
	SAVE_bool( m_AnimCurrent.bLoop );

	SAVE_DWORD( m_AnimPending.eFactID );
	SAVE_DWORD( m_AnimPending.hAni );
	SAVE_bool( m_AnimPending.bLoop );
}

void CAIGoalAnimate::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST( m_AnimCurrent.eFactID, ENUM_FactID );
	LOAD_DWORD( m_AnimCurrent.hAni );
	LOAD_bool( m_AnimCurrent.bLoop );

	LOAD_DWORD_CAST( m_AnimPending.eFactID, ENUM_FactID );
	LOAD_DWORD( m_AnimPending.hAni );
	LOAD_bool( m_AnimPending.bLoop );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::CalculateGoalRelevance()
{
	// Goal is only relevant if we have an Animate or AnimateLoop task.

	CAIWMFact* pFact = GetAnimateTaskFact();
	if( !pFact )
	{
		pFact = GetAnimateLoopTaskFact();
	}

	if( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) )
	{
		// New animation requested.

		if( m_AnimCurrent.eFactID != pFact->GetFactID() )
		{
			m_AnimPending.eFactID = pFact->GetFactID();
			m_AnimPending.hAni = pFact->GetIndex();
			m_AnimPending.bLoop = (pFact->GetTaskType() == kTask_AnimateLoop) ? true : false;
		}

		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// No Animate or AnimateLoop task.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalAnimate::ReplanRequired()
{
	// Replan if the animation has changed.

	if(	( m_AnimPending.hAni != m_AnimCurrent.hAni ) ||
		( m_AnimPending.bLoop != m_AnimCurrent.bLoop ) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::ActivateGoal()
{
	super::ActivateGoal();

	m_AnimCurrent = m_AnimPending;

	m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Set );

	// If AI is at a node that he owns, unlock the node so that he will find
	// it again once he finishes his animation.  Otherwise, he will not return to 
	// his SmartObject behavior if there are no other nodes to go to.
	// Only do this if the AI owns the node, in which case there is no risk of 
	// another AI stealing his node, and standing on top of him.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && IsAINode( pProp->hWSValue ) )
	{
		// Recursively find the owner of the AI's current node.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		AINode* pNodeOwner = pNode;
		while( IsAINode( pNodeOwner->GetNodeOwner() ) )
		{
			pNodeOwner = (AINode*)g_pLTServer->HandleToObject( pNodeOwner->GetNodeOwner() );
		}

		// The AI is at a locked node that he owns.
		// Unlock it so that Goals can find it again when checking relevancy.

		if( ( pNodeOwner->GetNodeOwner() == m_pAI->m_hObject ) && 
			pNode->IsNodeLocked() )
		{
			pNode->UnlockNode( m_pAI->m_hObject );
		}
	}
}	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::DeactivateGoal()
{
	super::DeactivateGoal();

	// Clear the AIs world state of the animation just played.

	m_pAI->GetAIWorldState()->SetWSProp( kWSK_AnimPlayed, m_pAI->m_hObject, kWST_int, (int)INVALID_MODEL_ANIM );

	// Clear the current animation request.

	m_AnimCurrent.eFactID = kFactID_Invalid;
	m_AnimCurrent.hAni = INVALID_MODEL_ANIM;
	m_AnimCurrent.bLoop = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::SetWSSatisfaction( CAIWorldState& WorldState )
{
	if( m_AnimPending.bLoop )
	{
		WorldState.SetWSProp( kWSK_AnimLooped, m_pAI->m_hObject, kWST_int, (int)m_AnimPending.hAni );
	}
	else {
		WorldState.SetWSProp( kWSK_AnimPlayed, m_pAI->m_hObject, kWST_int, (int)m_AnimPending.hAni );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalAnimate::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Goal is satisfied when we have played the desired animation.

	if( m_AnimCurrent.hAni != INVALID_MODEL_ANIM )
	{
		SAIWORLDSTATE_PROP* pProp;
		if( m_AnimCurrent.bLoop )
		{
			pProp = pwsWorldState->GetWSProp( kWSK_AnimLooped, m_pAI->m_hObject );
		}
		else {
			pProp = pwsWorldState->GetWSProp( kWSK_AnimPlayed, m_pAI->m_hObject );
		}

		if( pProp && ( pProp->nWSValue == m_AnimCurrent.hAni ) )
		{
			return true;
		}
	}

	// We have not finished the animation.

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAnimate::UpdateTaskStatus
//
//	PURPOSE:	Update the current status of the task.
//
// ----------------------------------------------------------------------- //

void CAIGoalAnimate::UpdateTaskStatus()
{
	if( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Done );

		// Clear the task.

		CAIWMFact factQuery;
		factQuery.SetFactID( m_AnimCurrent.eFactID );
		m_pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );
	}
}

CAIWMFact* CAIGoalAnimate::GetAnimateTaskFact()
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Task);
	factQuery.SetTaskType(kTask_Animate);
	return m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
}

CAIWMFact* CAIGoalAnimate::GetAnimateLoopTaskFact()
{
	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Task);
	factQuery.SetTaskType(kTask_AnimateLoop);
	return m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
}
