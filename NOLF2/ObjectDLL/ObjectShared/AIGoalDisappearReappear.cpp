// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDisappearReappear.cpp
//
// PURPOSE : AIGoalDisappearReappear implementation
//
// CREATED : 11/09/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalDisappearReappear.h"
#include "AIGoalMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIHuman.h"
#include "AIHumanStateDisappearReappear.h"
#include "AIBrain.h"
#include "AIHumanState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDisappearReappear, kGoal_DisappearReappear);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappear::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDisappearReappear::CAIGoalDisappearReappear()
{
	m_fDisappearDistMinSqr = 0.f;
	m_fDisappearDistMaxSqr = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappear::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappear::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT( m_fDisappearDistMinSqr );
	SAVE_FLOAT( m_fDisappearDistMaxSqr );
}

void CAIGoalDisappearReappear::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT( m_fDisappearDistMinSqr );
	LOAD_FLOAT( m_fDisappearDistMaxSqr );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappear::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappear::InitGoal(CAI* pAI)
{
	super::InitGoal(pAI);

	m_fDisappearDistMinSqr = m_pAI->GetBrain()->GetAIData( kAIData_DisappearDistMin );
	m_fDisappearDistMinSqr *= m_fDisappearDistMinSqr;

	m_fDisappearDistMaxSqr = m_pAI->GetBrain()->GetAIData( kAIData_DisappearDistMax );
	m_fDisappearDistMaxSqr *= m_fDisappearDistMaxSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappear::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappear::ActivateGoal()
{
	super::ActivateGoal();

	// Ignore all senses.
	m_pAI->SetCurSenseFlags(kSense_None);

	m_pGoalMgr->LockGoal( this );

	if( m_hStimulusSource )
	{
		m_pAI->Target( m_hStimulusSource );
	}

	m_pAI->SetState( kState_HumanDisappearReappear );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappear::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappear::DeactivateGoal()
{
	super::DeactivateGoal();

	// Goals DisappearReappear and DisappearReappearEvasive should share timing info.

	if( GetGoalType() != kGoal_DisappearReappearEvasive )
	{
		CAIGoalAbstract* pGoal = m_pGoalMgr->FindGoalByType( kGoal_DisappearReappearEvasive );
		if( pGoal && ( m_fNextUpdateTime > pGoal->GetNextUpdateTime() ) )
		{
			pGoal->SetNextUpdateTime( m_fNextUpdateTime );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappear::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappear::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanDisappearReappear:
			HandleStateDisappearReappear();
			break;

		// Unexpected State.
		default: AIASSERT1(0, m_pAI->m_hObject, "CAIGoalDisappearReappear::UpdateGoal: Unexpected State: %s.", s_aszStateTypes[pState->GetStateType()] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappear::HandleStateDisappearReappear
//
//	PURPOSE:	Determine what to do when in state DisappearReappear.
//
// ----------------------------------------------------------------------- //

void CAIGoalDisappearReappear::HandleStateDisappearReappear()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Disappearing:
			break;

		case kSStat_Moving:
			break;

		case kSStat_Reappearing:
			break;

		case kSStat_FailedComplete:
			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDisappearReappear::HandleStateDisappearReappear: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSpecialDamage::HandleDamage
//
//	PURPOSE:	Handle special damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDisappearReappear::HandleDamage(const DamageStruct& damage)
{
	if( m_pAI->IsDead() && ( m_pAI->GetState()->GetStateType() == kState_HumanDisappearReappear ) )
	{
		CAIHumanStateDisappearReappear* pStateDisappear = (CAIHumanStateDisappearReappear*)m_pAI->GetState();

		// Make sure AI reappears if she dies while invisible.
	
		pStateDisappear->Reappear();
		g_pLTServer->SetObjectColor( m_pAI->m_hObject, 1.f, 1.f, 1.f, 1.f );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDisappearReappear::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDisappearReappear::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// AI may only disappear immediately after attacking.

		if( m_pAI->GetAnimationContext()->IsLocked() &&
			( m_pAI->GetAnimationContext()->IsPropSet( kAPG_WeaponAction, kAP_Fire )
			|| m_pAI->GetAnimationContext()->IsPropSet( kAPG_WeaponAction, kAP_FireSecondary ) ) )
		{
			// Check the distance.

			LTFLOAT fDistSqr = m_pAI->GetPosition().DistSqr( pSenseRecord->vLastStimulusPos );
			if(	( fDistSqr >= m_fDisappearDistMinSqr ) && ( fDistSqr <= m_fDisappearDistMaxSqr ) )
			{
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}
