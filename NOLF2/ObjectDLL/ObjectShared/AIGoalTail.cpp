// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalTail.cpp
//
// PURPOSE : AIGoalTail implementation
//
// CREATED : 7/23/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalTail.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AI.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"

extern CCharacterMgr* g_pCharacterMgr;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalTail, kGoal_Tail);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTail::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalTail::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalTail::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTail::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalTail::ActivateGoal()
{
	super::ActivateGoal();

	// Default Target to player if not set.
	if(m_hTarget == LTNULL)
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		m_hTarget = pPlayer->m_hObject;
	}

	m_pAI->SetState( kState_HumanIdle );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTail::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalTail::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanIdle:
			// Set target, and tail.
			m_pAI->Target(m_hTarget);
			m_pAI->SetState( kState_HumanTail );
			break;

		case kState_HumanTail:
			HandleStateTail();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalTail::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalTail::HandleStateTail
//
//	PURPOSE:	Determine what to do when in state Tail.
//
// ----------------------------------------------------------------------- //

void CAIGoalTail::HandleStateTail()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Posing:
			break;

		case kSStat_Moving:
			break;

		case kSStat_FailedComplete:
			m_pAI->SetState( kState_HumanIdle );
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalTail::HandleStateTail: Unexpected State Status.");
	}
}

