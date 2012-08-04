// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAnimate.cpp
//
// PURPOSE : AIActionAnimate class implementation
//
// CREATED : 4/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIActionAnimate.h

#include "Stdafx.h"
#include "AIWorldState.h"
#include "AIActionAbstract.h"
#include "AIActionAnimate.h"

// Includes required for AIActionAnimate.cpp

#include "AI.h"
#include "AIBlackBoard.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAnimate, kAct_Animate );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAnimate::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAnimate::CAIActionAnimate()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAnimate::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionAnimate::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI has played the requested animation.

	m_wsWorldStateEffects.SetWSProp( kWSK_AnimPlayed, NULL, kWST_Variable, kWSK_AnimPlayed );
	m_wsWorldStateEffects.SetWSProp( kWSK_AnimLooped, NULL, kWST_Variable, kWSK_AnimLooped );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAnimate::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAnimate::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Animation may be played once or looped.

	bool bLoop = false;
	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_AnimPlayed, pAI->m_hObject );
	if( !pProp )
	{
		pProp = wsWorldStateGoal.GetWSProp( kWSK_AnimLooped, pAI->m_hObject );
		bLoop = true;
	}

	if( pProp )
	{
		// Set animate state.
	
		pAI->SetState( kState_Animate );

		// Set the animation to play.

		HMODELANIM hAni = pProp->nWSValue;
		CAIStateAnimate* pAnimate = (CAIStateAnimate*)pAI->GetState();
		pAnimate->SetAnimation( hAni, bLoop );

		// Check for moving from a node.

		pAnimate->CheckDepartedNode( true );

		// Torso tracking.

		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
		pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAnimate::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionAnimate::IsActionComplete( CAI* pAI )
{
	// Animating is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Animating is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAnimate::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionAnimate::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

