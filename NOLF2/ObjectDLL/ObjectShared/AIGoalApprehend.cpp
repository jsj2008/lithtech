// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalApprehend.cpp
//
// PURPOSE : AIGoalApprehend implementation
//
// CREATED : 1/25/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalApprehend.h"
#include "AIHumanStateApprehend.h"
#include "AIGoalMgr.h"
#include "AIGoalButeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIHuman.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalApprehend, kGoal_Apprehend);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalApprehend::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalApprehend::CAIGoalApprehend()
{
	m_fHoldTime = 3.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalApprehend::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalApprehend::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT( m_fHoldTime );
}

void CAIGoalApprehend::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT( m_fHoldTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalApprehend::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalApprehend::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hStimulusSource, m_pAI->m_hObject, "CAIGoalApprehend::ActivateGoal: Stimulus is NULL.");

	m_pGoalMgr->LockGoal( this );

	m_pAI->Target( m_hStimulusSource );

	if( m_pAI->GetState()->GetStateType() != kState_HumanApprehend )
	{
		m_pAI->SetState( kState_HumanApprehend );
		CAIHumanStateApprehend* pState = (CAIHumanStateApprehend*)m_pAI->GetState();
		pState->SetHoldTime( m_fHoldTime );
		pState->SetCloseEnoughDist( 64.f );

		// Only pay attention to senses for this goal.

		AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
		if( pTemplate )
		{
			m_pAI->SetCurSenseFlags( pTemplate->flagSenseTriggers );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalApprehend::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalApprehend::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanApprehend:
			HandleStateApprehend();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalApprehend::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalApprehend::HandleStateApprehend
//
//	PURPOSE:	Determine what to do when in state Apprehend.
//
// ----------------------------------------------------------------------- //

void CAIGoalApprehend::HandleStateApprehend()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Holding:
			break;

		case kSStat_Moving:
			break;

		case kSStat_FailedComplete:
			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			{
				// Send a MissionFailed message.

				char szMsg[64];
				sprintf( szMsg, "MissionFailed %d", g_pAIButeMgr->GetTextIDs()->nApprehendMissionFailedID );
				CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
				if( pPlayer )
				{
					SendTriggerMsgToObject( m_pAI, pPlayer->m_hObject, LTFALSE, szMsg );
				}
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalApprehend::HandleStateApprehend: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalApprehend::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalApprehend::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( ( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() )
			&& ( pSenseRecord->eSenseType == kSense_SeeEnemy ) )
		{
			return LTTRUE;
		}

		// Any other sense means the AI heard someone shoot, and
		// should lose all interest in apprehending.

		m_bDeleteGoalNextUpdate = LTTRUE;
		m_fCurImportance = 0.f;
	}

	return LTFALSE;
}
