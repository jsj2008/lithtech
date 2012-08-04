// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCharge.cpp
//
// PURPOSE : AIGoalCharge implementation
//
// CREATED : 7/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalCharge.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalCharge, kGoal_Charge);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCharge::CAIGoalCharge()
{
	m_fAttackDistanceSqr = 750.f * 750.f;
	m_fYellDistanceSqr = 900.f * 900.f;
	m_fStopDistanceSqr = 50.f * 50.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalCharge::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT(m_fAttackDistanceSqr);
	SAVE_FLOAT(m_fYellDistanceSqr);
	SAVE_FLOAT(m_fStopDistanceSqr);
}

void CAIGoalCharge::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT(m_fAttackDistanceSqr);
	LOAD_FLOAT(m_fYellDistanceSqr);
	LOAD_FLOAT(m_fStopDistanceSqr);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCharge::ActivateGoal()
{
	super::ActivateGoal();
	
	ASSERT(m_hStimulusSource != LTNULL);

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pAI->SetState( kState_HumanCharge );
	CAIHumanStateCharge* pChargeState = (CAIHumanStateCharge*)(m_pAI->GetState());
	pChargeState->SetAttackDistSqr(m_fAttackDistanceSqr);
	pChargeState->SetYellDistSqr(m_fYellDistanceSqr);
	pChargeState->SetStopDistSqr(m_fStopDistanceSqr);

	m_pAI->Target(m_hStimulusSource);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCharge::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanCharge:
			HandleStateCharge();
			break;

		case kState_HumanIdle:
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalCharge::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::HandleStateCharge
//
//	PURPOSE:	Determine what to do when in state Charge.
//
// ----------------------------------------------------------------------- //

void CAIGoalCharge::HandleStateCharge()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			// Reset AIs default senses, from aibutes.txt.
			m_pAI->ResetBaseSenseFlags();
			m_pAI->SetState( kState_HumanIdle );
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalCharge::HandleStateCharge: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalCharge::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		LTFLOAT fTargetDistanceSqr = VEC_DISTSQR( pSenseRecord->vLastStimulusPos, m_pAI->GetPosition());
		if( fTargetDistanceSqr > m_fStopDistanceSqr )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCharge::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalCharge::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "ATTACKDIST") )
	{
		m_fAttackDistanceSqr = (LTFLOAT)atof(szValue);
		m_fAttackDistanceSqr *= m_fAttackDistanceSqr;
		return LTTRUE;
	}
	else if ( !_stricmp(szName, "YELLDIST") )
	{
		m_fYellDistanceSqr = (LTFLOAT)atof(szValue);
		m_fYellDistanceSqr *= m_fYellDistanceSqr;
		return LTTRUE;
	}
	else if ( !_stricmp(szName, "STOPDIST") )
	{
		m_fStopDistanceSqr = (LTFLOAT)atof(szValue);
		m_fStopDistanceSqr *= m_fStopDistanceSqr;
		return LTTRUE;
	}

	return LTFALSE;
}

