// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFinishBlockNormal.cpp
//
// PURPOSE : 
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFinishBlockNormal.h"
#include "AIStateAnimate.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFinishBlockNormal, kAct_FinishBlockNormal );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishBlockNormal::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionFinishBlockNormal::CAIActionFinishBlockNormal()
{
}

CAIActionFinishBlockNormal::~CAIActionFinishBlockNormal()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishBlockNormal::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionFinishBlockNormal::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// No preconditions.

	// Set effects.
	// Reacted to a successful block.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_MeleeBlockFailure );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishBlockNormal::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionFinishBlockNormal::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Set the state.

	pAI->SetState( kState_Animate );
	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	CAnimationProps props = pSmartObjectRecord->Props;
	props.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	pStateAnimate->SetAnimation( props, !LOOP );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishBlockNormal::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionFinishBlockNormal::IsActionComplete( CAI* pAI )
{
	// Action is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Normal is not complete.

	return false;
}
