// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDistress.cpp
//
// PURPOSE : AIGoalDistress implementation
//
// CREATED : 7/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalDistress.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AIHuman.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDistress, kGoal_Distress);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDistress::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDistress::CAIGoalDistress()
{
	m_bCanActivate		= LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDistress::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalDistress::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bCanActivate);
}

void CAIGoalDistress::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bCanActivate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDistress::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDistress::ActivateGoal()
{
	super::ActivateGoal();

	ASSERT(m_hStimulusSource != LTNULL);

	if(m_pAI->GetState()->GetStateType() != kState_HumanPanic)
	{
		m_pAI->SetState( kState_HumanDistress );

		CAIHumanStateDistress* pDistressState = (CAIHumanStateDistress*)m_pAI->GetState();
		pDistressState->SetCanActivate(m_bCanActivate);
	}
	m_pAI->Target(m_hStimulusSource);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDistress::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDistress::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanDistress:
			HandleStateDistress();
			break;

		case kState_HumanPanic:
			HandleStatePanic();
			break;

		// Unexpected State.
		default: ASSERT(!"CAIGoalDistress::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDistress::HandleStateDistress
//
//	PURPOSE:	Determine what to do when in state Distress.
//
// ----------------------------------------------------------------------- //

void CAIGoalDistress::HandleStateDistress()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		case kSStat_Panic:
			{
				// Ignore senses other than SeeEnemy.
				m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile);

				m_pAI->SetState( kState_HumanPanic );

				CAIHumanStatePanic* pPanicState = (CAIHumanStatePanic*)m_pAI->GetState();
				pPanicState->SetCanActivate(m_bCanActivate);
			}
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalDistress::HandleStateDistress: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDistress::HandleStatePanic
//
//	PURPOSE:	Determine what to do when in state Panic.
//
// ----------------------------------------------------------------------- //

void CAIGoalDistress::HandleStatePanic()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: ASSERT(!"CAIGoalDistress::HandleStatePanic: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDistress::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDistress::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Do not distress if we are armed.

		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() )
		{
			return LTFALSE;
		}

		// Only distress if see a character with a dangerous weapon aimed at me.

		m_pAI->Target( m_hStimulusSource );
		if( m_pAI->GetBrain()->TargetIsAimingAtMe() )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDistress::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDistress::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "CANACTIVATE") )
	{
		m_bCanActivate = IsTrueChar(*szValue);
		return LTTRUE;
	}

	return LTFALSE;
}
