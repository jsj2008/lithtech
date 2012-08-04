// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalHolsterWeapon.cpp
//
// PURPOSE : AIGoalHolsterWeapon implementation
//
// CREATED : 11/02/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalHolsterWeapon.h"
#include "AIGoalMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIHuman.h"
#include "AIHumanState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalHolsterWeapon, kGoal_HolsterWeapon);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalHolsterWeapon::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalHolsterWeapon::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalHolsterWeapon::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalHolsterWeapon::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalHolsterWeapon::ActivateGoal()
{
	super::ActivateGoal();

	// Ignore all senses.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeUndetermined | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal( this );

	m_pAI->SetState( kState_HumanIdle );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalHolsterWeapon::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalHolsterWeapon::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanIdle:
			m_pAI->SetState( kState_HumanHolster );
			break;

		case kState_HumanHolster:
			HandleStateHolster();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalHolsterWeapon::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalHolsterWeapon::HandleStateHolster
//
//	PURPOSE:	Determine what to do when in state Holster.
//
// ----------------------------------------------------------------------- //

void CAIGoalHolsterWeapon::HandleStateHolster()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
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
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalHolsterWeapon::HandleStateHolster: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalHolsterWeapon::RecalcImportance
//
//	PURPOSE:	Recalculate the goal importance based on the presence
//              of a weapon.
//
// ----------------------------------------------------------------------- //

void CAIGoalHolsterWeapon::RecalcImportance()
{
	// HACK: for TO2 AI with grenades.

	if( m_pAI->GetPrimaryWeaponType() == kAIWeap_Thrown )
	{
		m_fCurImportance = 0.f;
		return;
	}

	// END HACK


	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( m_pAI->GetPrimaryWeapon() && 
		pAIHuman->HasHolsterString() &&
		( !m_pAI->GetSenseRecorder()->HasAnyStimulation( kSense_All ) ) )
	{
		SetCurToBaseImportance();
	}
	else {
		m_fCurImportance = 0.f;
	}
}

