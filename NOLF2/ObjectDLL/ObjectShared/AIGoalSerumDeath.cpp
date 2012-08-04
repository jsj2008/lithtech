// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSerumDeath.cpp
//
// PURPOSE : AIGoalSerumDeath implementation
//
// CREATED : 2/13/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalSerumDeath.h"
#include "AIGoalMgr.h"
#include "AI.h"
#include "AIHumanState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSerumDeath, kGoal_SerumDeath);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSerumDeath::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSerumDeath::CAIGoalSerumDeath()
{
	// Min and Max can be buted if necessary.

	m_fSleepTimeMin = 2.f;
	m_fSleepTimeMax = 3.f;
	m_fSleepTimer = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSerumDeath::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSerumDeath::InitGoal(CAI* pAI)
{
	super::InitGoal(pAI);

	m_fSleepTimeMin = m_pAI->GetBrain()->GetAIData( kAIData_SleepTimeMin );
	m_fSleepTimeMax = m_pAI->GetBrain()->GetAIData( kAIData_SleepTimeMax );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSerumDeath::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalSerumDeath::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_animProps.Save( pMsg );

	SAVE_FLOAT(	m_fSleepTimeMin );
	SAVE_FLOAT(	m_fSleepTimeMax );
	SAVE_FLOAT(	m_fSleepTimer );
}

void CAIGoalSerumDeath::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_animProps.Load( pMsg );

	LOAD_FLOAT(	m_fSleepTimeMin );
	LOAD_FLOAT(	m_fSleepTimeMax );
	LOAD_FLOAT(	m_fSleepTimer );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSerumDeath::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSerumDeath::ActivateGoal()
{
	super::ActivateGoal();

	m_pGoalMgr->LockGoal(this);
	m_pAI->SetState( kState_HumanAnimate );

	// Ignore all senses.

	m_pAI->SetCurSenseFlags(kSense_None);

	// Setup animprops for the PowerDown animation.

	m_animProps.Set( kAPG_Posture, kAP_Stand );
	m_animProps.Set( kAPG_Weapon, kAP_Weapon1 );
	m_animProps.Set( kAPG_WeaponPosition, kAP_Down );
	m_animProps.Set( kAPG_Action, kAP_PowerDown );

	CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());
	pStateAnimate->SetAnimation( m_animProps, LTFALSE );

	// Reset sleep timer.

	m_fSleepTimer = GetRandom( m_fSleepTimeMin, m_fSleepTimeMax );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSerumDeath::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSerumDeath::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAnimate:
			HandleStateAnimate();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalSerumDeath::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSerumDeath::HandleStateAnimate
//
//	PURPOSE:	Determine what to do when in state Animate.
//
// ----------------------------------------------------------------------- //

void CAIGoalSerumDeath::HandleStateAnimate()
{
	CAIHumanStateAnimate* pStateAnimate = (CAIHumanStateAnimate*)(m_pAI->GetState());

	switch( pStateAnimate->GetStateStatus() )
	{
		// Animation is playing.

		case kSStat_Initialized:

			// Decrement sleep timer if sleeping.

			if( m_animProps.Get( kAPG_Action ) == kAP_Asleep )
			{
				m_fSleepTimer -= g_pLTServer->GetFrameTime();

				// Sleep timer expired.

				if( m_fSleepTimer <= 0.f )
				{
					m_animProps.Set( kAPG_Action, kAP_PowerUp );
					pStateAnimate->SetAnimation( m_animProps, LTFALSE );
				}
			}
			break;

		// Animation finished.

		case kSStat_StateComplete:

			switch( m_animProps.Get( kAPG_Action ) )
			{
				// PowerDown is finished. Go to sleep.

				case kAP_PowerDown:
					m_animProps.Set( kAPG_Action, kAP_Asleep );
					pStateAnimate->SetAnimation( m_animProps, LTTRUE );
					break;

				// PowerUp is finished. Wake up.

				case kAP_PowerUp:
					{
						CDestructible* pDestructable = m_pAI->GetDestructible();
						pDestructable->Heal( pDestructable->GetMaxHitPoints() * 0.5f );
						pDestructable->SetNeverDestroy( LTFALSE );

						m_pGoalMgr->UnlockGoal(this);
						m_fCurImportance = 0.f;
					}
					break;
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalSerumDeath::HandleStateAnimate: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSerumDeath::HandleDamage
//
//	PURPOSE:	Handle damage by powering down or dying.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalSerumDeath::HandleDamage(const DamageStruct& damage)
{
	CDestructible* pDestructable = m_pAI->GetDestructible();

	// Only let Anti-SuperSoldierSerum (ASSS) through to the normal 
	// damage handling.  ASSS instantly kills.
	// ASSS only kills when SSoldier is powered down.

	if( ( damage.eType == DT_ASSS ) &&
		( m_fSleepTimer > 0.f ) )
	{
		pDestructable->HandleDestruction( damage.hDamager, DT_ASSS );
		return LTFALSE;
	}

	// Regenerate hit points when dead to half of the original max.

	if( pDestructable->IsDead() )
	{	
		pDestructable->SetNeverDestroy( LTTRUE );
		pDestructable->Heal( pDestructable->GetMaxHitPoints() * 0.5f );

		// Activate goal to power-down / power-up.

		SetCurToBaseImportance();
		return LTTRUE;
	}

	// Allow other goals to respond, most importantly Investigate to turn
	// the SuperSoldier toward the shooter.

	return LTFALSE;
}
