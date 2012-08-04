// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSurprised.cpp
//
// PURPOSE : 
//
// CREATED : 11/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionSurprised.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSurprised, kAct_Surprised );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurprised::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionSurprised::CAIActionSurprised()
{
}

CAIActionSurprised::~CAIActionSurprised()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurprised::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionSurprised::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Surprised );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurprised::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionSurprised::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Set SmartObject state.
	
	pAI->SetState( kState_UseSmartObject );

	// Set smartobject.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurprised::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionSurprised::IsActionComplete( CAI* pAI )
{
	// Recoiling is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Recoiling is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurprised::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionSurprised::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
