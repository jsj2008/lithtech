// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdleCombat.cpp
//
// PURPOSE : 
//
// CREATED : 3/17/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionIdleCombat.h"
#include "AIStateAnimate.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdleCombat, kAct_IdleCombat );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleCombat::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionIdleCombat::CAIActionIdleCombat()
{
}

CAIActionIdleCombat::~CAIActionIdleCombat()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleCombat::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionIdleCombat::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Must not be mounting a node.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_MountedObject, NULL, kWST_HOBJECT, 0 );

	// Set effects.
	// AI is idling.

	m_wsWorldStateEffects.SetWSProp( kWSK_Idling, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleCombat::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionIdleCombat::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if there is no smartobject data.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Fail if the AI does not have a character target.
	
    if ( !pAI->HasTarget( kTarget_Character ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionIdleCombat::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Added an assert when an AI goes into the idle goal with an animation 
	// that is currently locked.  This suggests that the previous behavior 
	// did not perform the expected cleanup. 
	// If this assumption isn't valid, this assert can safely be removed.

	AIASSERT( !pAI->GetAnimationContext()->IsLocked(), pAI->GetHOBJECT(), 
		"CAIActionIdle::ActivateAction : AIs animation is still locked and will be looped.  This may indicate the previous action failed to clean itself up properly."  );

	// If AI is not in any state, set Idle state.
	// Otherwise, the AIActionIdle does not affect the current state.

	if( ( pAI->GetState() == NULL ) ||
		( pAI->GetAnimationContext()->WasLocked() ) ||
		( pAI->GetAnimationContext()->WasPlayingSpecial() ) ||
		( pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Unset ) )
	{
		AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
		AIASSERT( pSmartObjectRecord, pAI->GetHOBJECT(), "CAIActionDismountPlayer::ActivateAction : No SmartObjectRecord specified.");
		if ( !pSmartObjectRecord )
		{
			return;
		}

		// Set animate state.

		pAI->SetState( kState_Animate );

		// Set idle animation; use the smartobject props, then override particular ones which are dynamic.

		CAnimationProps	animProps = pSmartObjectRecord->Props;

		EnumAnimProp ePosture = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Posture );
		if( ePosture == kAP_None 
			|| ePosture == kAP_POS_Mounted)
		{
			ePosture = kAP_POS_Stand;
		}

		animProps.Set( kAPG_Posture, ePosture );
		animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );

		CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
		pStateAnimate->SetAnimation( animProps, LOOP );
	}

	// Head tracking.

	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
}
