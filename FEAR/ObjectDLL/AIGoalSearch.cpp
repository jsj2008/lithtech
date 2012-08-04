// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSearch.cpp
//
// PURPOSE : AIGoalSearch class implementation
//
// CREATED : 8/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalSearch.h"
#include "AI.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSearch, kGoal_Search );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSearch::CAIGoalSearch()
{
	m_eTaskType = kTask_Search;
	m_bCheckNodeValidity = true;
	m_bClearScriptedTaskIfThreatened = false;
}

CAIGoalSearch::~CAIGoalSearch()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalSearch::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalGoto
//              
//----------------------------------------------------------------------------

void CAIGoalSearch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalSearch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearch::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearch::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Intentionally do not call super::SetWSSatisfaction.
	// Satisfaction requires UsingNode rather than AtNode.

	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_NodePending.hNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearch::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalSearch::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Intentionally do not call super::SetWSSatisfaction.
	// Satisfaction requires UsingNode rather than AtNode.

	// Goal is satisfied if we have used the node.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_UsingObject, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue && ( pProp->hWSValue == m_NodeCurrent.hNode ) )
	{
		return true;
	}

	// We have not yet used the node.

	return false;
}
