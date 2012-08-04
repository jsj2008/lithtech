// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSurveyArea.cpp
//
// PURPOSE : AIActionSurveyArea class implementation
//
// CREATED : 3/27/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionSurveyArea.h"
#include "AI.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemoryCentral.h"
#include "AIUtils.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSurveyArea, kAct_SurveyArea );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurveyArea::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionSurveyArea::CAIActionSurveyArea()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurveyArea::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionSurveyArea::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// AI is armed.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );

	// Set effects.
	// Area has been surveyed.

	m_wsWorldStateEffects.SetWSProp( kWSK_SurveyedArea, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurveyArea::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionSurveyArea::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set aware animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Lower );
	animProps.Set( kAPG_Action, kAP_ACT_Alert );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurveyArea::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionSurveyArea::IsActionComplete( CAI* pAI )
{
	// Surveying is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Surveying is not complete.

	return false;
}

