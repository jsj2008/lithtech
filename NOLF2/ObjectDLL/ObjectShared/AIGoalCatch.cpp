
//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalCatch.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	08.11.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIGOALCATCH_H__
#include "AIGoalCatch.h"		
#endif

#ifndef __AIHUMANSTATECATCH_H__
#include "AIHumanStateCatch.h"
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

#include "AIUtils.h"

// Forward declarations

// Globals

// Statics

//static CAIHumanStateCatch test;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalCatch, kGoal_Catch);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCatch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCatch::CAIGoalCatch()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCatch::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalCatch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalCatch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCatch::OnLinkBroken
//
//	PURPOSE:	Handles a deleted object reference.
//
// ----------------------------------------------------------------------- //
void CAIGoalCatch::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	float flImportance = GetCurImportance();

	CAIGoalAbstractStimulated::OnLinkBroken( pRef, hObj );
	
	SetCurImportance(flImportance);
	m_hStimulusTarget = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCatch::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalCatch::ActivateGoal(void)
{
	super::ActivateGoal();

	if (m_hStimulusTarget==LTNULL)
	{
		SetCurImportance(0);
		AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Aborting Goal %s\n", s_aszGoalTypes[GetGoalType()] ) );
		return;
	}

	// Ignore senses other than see enemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal(this);

	m_pAI->SetState( kState_HumanCatch );

	// Set state specific information..
	CAIHumanStateCatch* pCatchInstance = (CAIHumanStateCatch*)m_pAI->GetState();

	// Save the object we intend to catch
	pCatchInstance->SetObjectToCatch( m_hStimulusTarget );

	// Set the animations to use
	pCatchInstance->SetAnimationSequence( kAP_StartCatch, kAP_HoldCatch, kAP_EndCatch );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCatch::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalCatch::UpdateGoal(void)
{
	CAIState* pState = m_pAI->GetState();

	// Handle each State this Goal is implemented in terms of
	switch(pState->GetStateType())
	{
		case kState_HumanCatch:
			HandleStateCatch();
			break;

		// Unexpected State.
		default: UBER_ASSERT(0, "CAIGoalCatch::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCatch::HandleStateCatch
//
//	PURPOSE:	Determine what to do when in state Catch.
//
// ----------------------------------------------------------------------- //

void CAIGoalCatch::HandleStateCatch()
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
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: UBER_ASSERT(0, "CAIGoalCatch::HandleStateCatch: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCatch::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //
LTBOOL CAIGoalCatch::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// Check for our parent refusing to allow handling
	if( LTFALSE == CAIGoalAbstractStimulated::HandleGoalSenseTrigger(pSenseRecord) )
	{
		return LTFALSE;
	}

	// If our context is locked, or if we are already catching, or if it 
	// is too soon to catch again then ignore the stimulus, 
	if( m_pAI->GetState()->GetStateType() == kState_HumanCatch )
	{
		return LTFALSE;
	}

	if ( !pSenseRecord->hLastStimulusTarget )
	{
		return LTFALSE;
	}


	// Determine if we are the owner.  We currently can only catch 
	// things that we threw.
	if ( pSenseRecord->hLastStimulusSource != m_pAI->GetObject() )
	{
		return LTFALSE;
	}

	// Check the speed!  If it is still a long ways away, we don't need
	// to start the catch yet!
//	LTVector vVelocity, vOrigin;
//	g_pPhysicsLT->GetVelocity(pSenseRecord->hStimulusTarget, &vVelocity);
//	g_pLTServer->GetObjectPos(pSenseRecord->hStimulusTarget, &vOrigin);
//
//	// Time = Distance / Velecity
//	float flDist = ( m_pAI->GetPosition() - vOrigin ).Mag();
//	float flTime = flDist / vVelocity.Mag();
//
//	if ( flTime > 0.5 )
//	{
//		return LTFALSE;
//	}

	return LTTRUE;
}
