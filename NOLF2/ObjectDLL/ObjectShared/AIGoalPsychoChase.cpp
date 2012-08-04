// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalPsychoChase.cpp
//
// PURPOSE : AIGoalPsychoChase implementation
//
// CREATED : 2/26/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalPsychoChase.h"
#include "AI.h"
#include "AIHumanState.h"
#include "AIGoalMgr.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalPsychoChase, kGoal_PsychoChase);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPsychoChase::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalPsychoChase::ActivateGoal()
{
	// The chase goal needs a stimulus.
	// PsychoChase is a special case, where AI should
	// relentlessly hunt down target.  Target defaults
	// to the player.

	CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
	if( pPlayer )
	{
		m_hStimulusSource = pPlayer->m_hObject;
	}

	super::ActivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPsychoChase::HandleStateChase
//
//	PURPOSE:	Determine what to do when in state Chase.
//
// ----------------------------------------------------------------------- //

void CAIGoalPsychoChase::HandleStateChase()
{
	// Do not set importance to 0 when complete.
	// Ignore junctions.

	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Junction:
			{
				CAIHumanStateChase* pStateChase = (CAIHumanStateChase*)m_pAI->GetState();
				pStateChase->SetStateStatus( kSStat_Pursue );
			}
			break;

		case kSStat_StateComplete:
			if( m_pGoalMgr->IsGoalLocked( this ) )
			{
				m_pGoalMgr->UnlockGoal( this );
			}
			break;

		default:
			super::HandleStateChase();
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalPsychoChase::RecalcImportance
//
//	PURPOSE:	If chase state completed, try to chase again.
//
// ----------------------------------------------------------------------- //

void CAIGoalPsychoChase::RecalcImportance()
{
	if( ( m_pAI->GetState()->GetStateType() == kState_HumanChase ) &&
		( m_pAI->GetState()->GetStateStatus() == kSStat_StateComplete ) )
	{
		SetStateChase( LTTRUE );
	}
}
