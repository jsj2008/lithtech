// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMenace.cpp
//
// PURPOSE : AIGoalMenace implementation
//
// CREATED : 7/20/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalMenace.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "AIHumanState.h"
#include "AnimationMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalMenace, kGoal_Menace);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMenace::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalMenace::CAIGoalMenace()
{
	m_bRequireBareHands = LTFALSE;
	m_bAllowDialogue = LTTRUE;
	m_bTurnOffLights = LTTRUE;
	m_bTurnOnLights = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMenace::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalMenace::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalMenace::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMenace::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalMenace::ActivateGoal()
{
	super::ActivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMenace::SetStateUseObject
//
//	PURPOSE:	Set state to UseObject.
//
// ----------------------------------------------------------------------- //

void CAIGoalMenace::SetStateUseObject()
{
	if( !m_pAI->GetPrimaryWeapon() )
	{
		m_pGoalMgr->LockGoal(this);
		m_pAI->SetState( kState_HumanDraw );
		return;
	}

	super::SetStateUseObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMenace::HandleStateDraw
//
//	PURPOSE:	Determine what to do when in state Draw.
//
// ----------------------------------------------------------------------- //

void CAIGoalMenace::HandleStateDraw()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			CompleteUseObject();
			break;

		case kSStat_StateComplete:
			super::SetStateUseObject();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalMenace::HandleStateDraw: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMenace::HandleGoalAttractors
//
//	PURPOSE:	React to an attractor.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalMenace::HandleGoalAttractors()
{
	// Do not search for attractors if AI has no weapon.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( ( !m_pAI->GetPrimaryWeapon() ) &&
		( !pAIHuman->HasHolsterString() ) )
	{
		return LTNULL;
	}

	return super::HandleGoalAttractors();
}
