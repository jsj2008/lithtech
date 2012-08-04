// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollowFootprint.cpp
//
// PURPOSE : AIGoalFollowFootprint implementation
//
// CREATED : 7/26/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalFollowFootprint.h"
#include "AISenseRecorderAbstract.h"
#include "AIHumanState.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFollowFootprint, kGoal_FollowFootprint);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowFootprint::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFollowFootprint::CAIGoalFollowFootprint()
{
	m_vStimulusPos		= LTVector( 0.0f, 0.0f, 0.0f );
	m_fStimulationTime	= 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowFootprint::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalFollowFootprint::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_VECTOR(m_vStimulusPos);
	SAVE_TIME(m_fStimulationTime);
}

void CAIGoalFollowFootprint::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_VECTOR(m_vStimulusPos);
	LOAD_TIME(m_fStimulationTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowFootprint::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollowFootprint::ActivateGoal()
{
	super::ActivateGoal();
	
	AIASSERT(m_hStimulusSource != LTNULL, m_pAI->m_hObject, "CAIGoalFollowFootprint::ActivateGoal: No stimulus.");

	if(m_pAI->GetState()->GetStateType() != kState_HumanFollowFootprint)
	{
		m_pAI->SetState( kState_HumanFollowFootprint );
	}

	// Reset AIs default senses, from aibutes.txt.
	// This is necessary because SetStateSearch may have turned some off
	// before this goal was reactivated.

	m_pAI->ResetBaseSenseFlags();

	// Record the latest footstep seen.
	CAIHumanStateFollowFootprint* pState = (CAIHumanStateFollowFootprint*)m_pAI->GetState();
	pState->ResetFootprint(m_fStimulationTime, m_vStimulusPos);
	pState->SetSearch(m_bSearch);

	m_pGoalMgr->LockGoal(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowFootprint::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollowFootprint::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanFollowFootprint:
			HandleStateFollowFootprint();
			break;

		case kState_HumanAware:
			break;

		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalFollowFootprint::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowFootprint::HandleStateFollowFootprint
//
//	PURPOSE:	Determine what to do when in state FollowFootprint.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollowFootprint::HandleStateFollowFootprint()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			m_pAI->SetState( kState_HumanAware );
			m_pGoalMgr->UnlockGoal(this);
			break;

		case kSStat_StateComplete:
			{
				// Search or go aware.

				if( (m_pAI->IsMajorlyAlarmed() || m_bSearch) && m_pAI->CanSearch())
				{
					SetStateSearch();
				}
				else {
					m_pAI->SetState( kState_HumanAware );
					m_pGoalMgr->UnlockGoal(this);
				}
			}
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalFollowFootprint::HandleStateFollowFootprint: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowFootprint::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalFollowFootprint::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Do not investigate without a weapon.

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( !( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
		{
			return LTFALSE;
		}

		// Record whatever triggered this goal.
		m_vStimulusPos		= pSenseRecord->vLastStimulusPos;
		m_fStimulationTime	= pSenseRecord->fLastStimulationTime;
		m_pAI->Target(m_hStimulusSource);

		// Update the alarm level with this investigation.

		m_pAI->IncrementAlarmLevel( pSenseRecord->nLastStimulusAlarmLevel );

		return LTTRUE;
	}

	return LTFALSE;
}

