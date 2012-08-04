// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalIntro.cpp
//
// PURPOSE : AIGoalIntro class implementation
//
// CREATED : 2/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalIntro.h"
#include "AINodeIntro.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalIntro, kGoal_Intro );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalIntro::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalIntro::CAIGoalIntro()
{
	m_eTaskType = kTask_Intro;
	m_bClearScriptedTaskIfThreatened = false;
}

CAIGoalIntro::~CAIGoalIntro()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalIntro::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGoto
//              
//----------------------------------------------------------------------------

void CAIGoalIntro::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalIntro::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalIntro::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalIntro::DeactivateGoal()
{
	// If we are targeting something, clear the task
	// so that this goal never re-activates.

	if( m_pAI->HasTarget( kTarget_All ) )
	{
		ClearGotoTask( &m_NodeCurrent );
	}

	// Default behavior.

	super::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalIntro::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalIntro::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Intentionally do not call super::SetWSSatisfaction.
	// Satisfaction requires UsingNode rather than AtNode.
	// This forces the AI to handle node dependencies.

	// If we are already at the node, we ned to idle here for some amount of time.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue && ( pProp->hWSValue == m_NodePending.hNode ) )
	{
		WorldState.SetWSProp( kWSK_Idling, m_pAI->m_hObject, kWST_bool, true );
		return;
	}

	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_NodePending.hNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalIntro::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalIntro::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Intentionally do not call super::IsWSSatisfied().
	// AI needs to pause at node for some amount of time.

	// Goal is not satisfied if we are not at the node.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( !( pProp && pProp->hWSValue && ( pProp->hWSValue == m_NodeCurrent.hNode ) ) )
	{
		return false;
	}

	// Bail if not at an intro node.

	if( !IsAINode( pProp->hWSValue ) )
	{
		return false;
	}
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if( pNode->GetType() != kNode_Intro )
	{
		return false;
	}

	// Goal is satisfied if we have been at the node for the pause time.

	AINodeIntro* pNodeIntro = (AINodeIntro*)pNode;
	if( m_pAI->GetAIBlackBoard()->GetBBStateChangeTime() > g_pLTServer->GetTime() - pNodeIntro->GetPauseTime() )
	{
		return false;
	}

	// Goal is satisfied.

	return true;
}
