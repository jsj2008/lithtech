// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalExitLevel.cpp
//
// PURPOSE : AIGoalExitLevel implementation
//
// CREATED : 3/16/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalExitLevel.h"
#include "AINodeGuard.h"
#include "AINodeMgr.h"
#include "AIState.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalExitLevel, kGoal_ExitLevel);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevel::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalExitLevel::CAIGoalExitLevel()
{
	m_eNodeType = kNode_ExitLevel; 
	m_fMinImportance = 0.f;
	m_fExitAllowedTime = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevel::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevel::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	
	SAVE_TIME( m_fExitAllowedTime );
}

void CAIGoalExitLevel::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_TIME( m_fExitAllowedTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevel::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevel::HandleStateGoto()
{
	// If I made it to my ExitLevel node, remove me!

	if( m_pAI->GetState()->GetStateStatus() == kSStat_StateComplete )
	{
		SendTriggerMsgToObject(m_pAI, m_pAI->m_hObject, LTFALSE, "REMOVE");
		return;
	}

	super::HandleStateGoto();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalExitLevel::RecalcImportance
//
//	PURPOSE:	Bail if AI has a guard node.
//
// ----------------------------------------------------------------------- //

void CAIGoalExitLevel::RecalcImportance()
{
	// Ensure that AI has been in the level for at least 5 secs before
	// trying to exit.  This gives AIs a chance to sense whatever may 
	// be going on.

	if( m_fExitAllowedTime == 0.f )
	{
		m_fExitAllowedTime = g_pLTServer->GetTime() + 5.f;
	}
	else if( g_pLTServer->GetTime() < m_fExitAllowedTime )
	{
		m_fCurImportance = 0.f;
		return;
	}

	// If we already own a Patrol node or Guard node, we will Patrol or Guard so bail.
	// An AI cannot both Patrol or Guard, and ExitLevel.

	if( g_pAINodeMgr->FindOwnedNode( kNode_Guard, m_pAI->m_hObject ) )
	{
		m_fCurImportance = 0.f;
		return;
	}

	super::RecalcImportance();
}

