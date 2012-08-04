// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFinishBlockAttack.cpp
//
// PURPOSE : 
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFinishBlockAttack.h"
#include "AIStateAnimate.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFinishBlockAttack, kAct_FinishBlockAttack );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishBlockAttack::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionFinishBlockAttack::CAIActionFinishBlockAttack()
{
}

CAIActionFinishBlockAttack::~CAIActionFinishBlockAttack()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishBlockAttack::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionFinishBlockAttack::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// No preconditions.

	// Set effects.
	// Reacted to a successful block.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_MeleeBlockSuccess );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFinishBlockAttack::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionFinishBlockAttack::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
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
//	ROUTINE:	CAIActionFinishBlockAttack::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionFinishBlockAttack::IsActionComplete( CAI* pAI )
{
	// Attack is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Attack is not complete.

	return false;
}
