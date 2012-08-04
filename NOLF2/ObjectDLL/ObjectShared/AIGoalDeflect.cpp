//----------------------------------------------------------------------------
//				
//	MODULE: 	AIGoalDeflect.cpp
//				
//	PURPOSE:	AIGoalDeflect implementation
//				
//	CREATED:	06.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//				To Do: Save/Load, game play testing, and more completed
//						implementation.
//				
//				
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIGOALDEFLECT_H__
#include "AIGoalDeflect.h"		
#endif

#ifndef __AIHUMANSTATEDEFLECT_H__
#include "AIHumanStateDeflect.h"
#endif

#ifndef __AISENSE_RECORDER_ABSTRACT_H__
#include "AISenseRecorderAbstract.h"
#endif

#ifndef __AIGOAL_MGR_H__
#include "AIGoalMgr.h"
#endif

#ifndef __AI_H__
#include "AI.h"
#endif


#ifndef __WEAPON_H__
#include "Weapon.h"
#endif

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDeflect, kGoal_Deflect);


// Forward declarations

// Globals

// Statics

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeflect::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalDeflect::CAIGoalDeflect()
{
	m_fDeflectTimeNext = g_pLTServer->GetTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeflect::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalDeflect::Save(ILTMessage_Write *pMsg)
{
	CAIGoalAbstractStimulated::Save(pMsg);
	SAVE_TIME(m_fDeflectTimeNext);
}

void CAIGoalDeflect::Load(ILTMessage_Read *pMsg)
{
	CAIGoalAbstractStimulated::Load(pMsg);
	LOAD_TIME(m_fDeflectTimeNext);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractStimulated::OnLinkBroken
//
//	PURPOSE:	Handles a deleted object reference.
//
// ----------------------------------------------------------------------- //
void CAIGoalDeflect::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	float flImportance = GetCurImportance();

	CAIGoalAbstractStimulated::OnLinkBroken( pRef, hObj );
	
	SetCurImportance(flImportance);
	m_hStimulusTarget = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeflect::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalDeflect::ActivateGoal(void)
{
	CAIGoalAbstractStimulated::ActivateGoal();

	// If we have no stimulus target, then fail out of the goal and exit
	if (m_hStimulusTarget==LTNULL)
	{
		SetCurImportance(0);
		return;
	}

	// If we don't have a weapon, or if we have less than 1 ammo in the
	// clip, then fail out of the goal and exit
	if ( !m_pAI->GetPrimaryWeapon() )
	{
		SetCurImportance(0);
		return;
	}

	if ( m_pAI->GetPrimaryWeapon()->GetAmmoInClip() < 1 )
	{
		SetCurImportance(0);
		return;
	}


	// Ignore senses other than see enemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal(this);

	m_pAI->SetState( kState_HumanDeflect );

	// Set state specific information..
	CAIHumanStateDeflect* pDeflectInstance = (CAIHumanStateDeflect*)m_pAI->GetState();

	// Save the object we intend to deflect
	pDeflectInstance->SetObjectToDeflect( m_hStimulusTarget );

	// Stop blocking early if we block the object that caused us to 
	// enter the goal
	pDeflectInstance->SetStopCondition( CAIHumanStateDeflect::SPECIAL_STOP_CONDITIONS::DEFLECT_NAMED_OBJECT );

	// Set the animations to use
	pDeflectInstance->SetAnimationSequence( kAP_StartDeflect, kAP_HoldDeflect, kAP_EndDeflect );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeflect::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalDeflect::UpdateGoal(void)
{
	CAIGoalAbstractStimulated::UpdateGoal();

	CAIState* pState = m_pAI->GetState();

	// Handle each State this Goal is implemented in terms of
	switch(pState->GetStateType())
	{
		case kState_HumanDeflect:
			HandleStateDeflect();
			break;

		// Unexpected State.
		default: UBER_ASSERT(0, "CAIGoalDeflect::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeflect::HandleStateDeflect
//
//	PURPOSE:	Determine what to do when in state Deflect.
//
// ----------------------------------------------------------------------- //

void CAIGoalDeflect::HandleStateDeflect()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_TriplePhaseOne:
			break;

		case kSStat_TriplePhaseTwo:
			break;

		case kSStat_TriplePhaseThree:
			break;

		case kSStat_StateComplete:
		case kSStat_FailedComplete:
			m_fDeflectTimeNext = g_pLTServer->GetTime() + GetRandom(1, 2);
			m_fCurImportance = 0.f;
			m_pAI->ResetBaseSenseFlags();

			if ( m_pGoalMgr->IsGoalLocked( this ) )
			{
				m_pGoalMgr->UnlockGoal(this);
			}
			break;

		// Unexpected StateStatus.
		default: UBER_ASSERT(0, "CAIGoalDeflect::HandleStateDeflect: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalDeflect::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //
LTBOOL CAIGoalDeflect::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// Check for our parent refusing to allow handling
	if ( LTFALSE == CAIGoalAbstractStimulated::HandleGoalSenseTrigger(pSenseRecord) )
	{
		return LTFALSE;
	}

	// If our context is locked, or if we are already catching, or if it 
	// is too soon to catch again then ignore the stimulus, 
	if( m_pAI->GetState()->GetStateType() == kState_HumanDeflect)
	{
		return LTFALSE;
	}

	// AIs can only block if they have ammo in their clip
	if ( !m_pAI->GetPrimaryWeapon() ||
		( m_pAI->GetPrimaryWeapon() && m_pAI->GetPrimaryWeapon()->GetAmmoInClip() < 1 ))
	{
		return LTFALSE;
	}

	// fail if the stimulus target is gone!
	if ( !pSenseRecord->hLastStimulusTarget )
	{
		return LTFALSE;
	}

	LTVector vVel, vObjectOrigin;
	g_pPhysicsLT->GetVelocity( pSenseRecord->hLastStimulusTarget, &vVel );
	g_pLTServer->GetObjectPos( pSenseRecord->hLastStimulusTarget, &vObjectOrigin);

	// Make sure the object isn't already 'too close' to deflect
	// Assume the animation takes .3 seconds to start the block (bute this later?)
	float flDist = (m_pAI->GetPosition() - vObjectOrigin).Mag();
	float flTime = flDist / vVel.Mag(); // Time = Distance / Rate
	if ( flTime < 0.4 )
	{
		return LTFALSE;
	}

	// Make sure that the disc is actually coming at us before blocking
	vVel.Normalize();
	if ( m_pAI->GetForwardVector().Dot(vVel) > -0.95 )
	{
		return LTFALSE;
	}

	return LTTRUE;
}

	