// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionStunned.cpp
//
// PURPOSE : AIActionStunned class implementation
//
// CREATED : 09/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionStunned.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionStunned, kAct_Stunned );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionStunned::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionStunned::CAIActionStunned()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionStunned::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionStunned::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionStunned::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionStunned::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
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
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionStunned::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionStunned::IsActionComplete( CAI* pAI )
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
//	ROUTINE:	CAIActionStunned::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionStunned::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

