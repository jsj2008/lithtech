// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDrawWeapon.cpp
//
// PURPOSE : AIGoalDrawWeapon implementation
//
// CREATED : 10/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalDrawWeapon.h"
#include "AIGoalMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "AIUtils.h"
#include "AIHumanState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDrawWeapon, kGoal_DrawWeapon);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDrawWeapon::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalDrawWeapon::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalDrawWeapon::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDrawWeapon::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDrawWeapon::ActivateGoal()
{
	super::ActivateGoal();

	// Bail if no holstered weapon, or already have a weapon armed.

	if( ( !m_pAI->HasHolsterString() ) ||
		( m_pAI->GetPrimaryWeapon() ) )
	{
		m_fCurImportance = 0.f;
		return;
	}

//	ASSERT(m_hStimulusSource != LTNULL);

	// Ignore senses other than see enemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal( this );

	// Set Draw state.

	m_pAI->SetState( kState_HumanDraw );

	if( m_pAI->GetAlarmLevel() >= m_pAI->GetBrain()->GetMajorAlarmThreshold() )
	{
		m_pAI->SetAwareness( kAware_Alert );
	}

	// If stimulated by an AI, target whatever he is targeting.

	if( IsAI( m_hStimulusSource ) )
	{
		CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_hStimulusSource );
		if( pAI )
		{
			if( pAI->HasTarget() )
			{
				m_pAI->Target( pAI->GetTarget()->GetObject() );
			}
		}
	}
	
	else if( IsCharacter( m_hStimulusSource ) )
	{
		// Only set a target to turn towards for specified senses.

		switch( m_eSenseType )
		{
			case kSense_SeeEnemy:
			case kSense_SeeEnemyLean:
			case kSense_HearEnemyWeaponFire:
			case kSense_HearEnemyFootstep:
				{
					// Only target hated characters.

					CCharacter *pChar = (CCharacter*)g_pLTServer->HandleToObject( m_hStimulusSource );
					if( pChar )
					{
						CharacterAlignment eAlignment = GetAlignment( pChar->GetRelationSet(), m_pAI->GetRelationData() );
						if( eAlignment == HATE )
						{
							m_pAI->Target( m_hStimulusSource );

							CAIHumanStateDraw* pStateDraw = (CAIHumanStateDraw*)m_pAI->GetState();
							pStateDraw->SetFaceTarget( LTTRUE );
						}
					}
				}
				break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDrawWeapon::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalDrawWeapon::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDrawWeapon::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDrawWeapon::HandleStateDraw
//
//	PURPOSE:	Determine what to do when in state Draw.
//
// ----------------------------------------------------------------------- //

void CAIGoalDrawWeapon::HandleStateDraw()
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
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalDrawWeapon::HandleStateDraw: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDrawWeapon::HandleDamage
//
//	PURPOSE:	Handle damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDrawWeapon::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage( damage );

	// Activate the goal.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( pAIHuman->HasHolsterString() && !pAIHuman->GetPrimaryWeapon() )
	{
		m_bRequiresImmediateResponse = LTTRUE;
		SetCurToBaseImportance();
	}

	// HACK: for TO2 AI with grenades.

	else if( ( m_pAI->GetPrimaryWeaponType() == kAIWeap_Thrown ) &&
		pAIHuman->HasHolsterString() )
	{
		m_bRequiresImmediateResponse = LTTRUE;
		SetCurToBaseImportance();
	}

	// END HACK

	// Always return false to allow normal damage handling.

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDrawWeapon::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalDrawWeapon::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( pAIHuman->HasHolsterString() && !pAIHuman->GetPrimaryWeapon() )
		{
			return LTTRUE;
		}

		// HACK: for TO2 AI with grenades.

		if( ( m_pAI->GetPrimaryWeaponType() == kAIWeap_Thrown ) &&
			pAIHuman->HasHolsterString() )
		{
			return LTTRUE;
		}

		// END HACK
	}

	return LTFALSE;
}
