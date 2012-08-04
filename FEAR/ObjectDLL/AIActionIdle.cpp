// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdle.cpp
//
// PURPOSE : AIActionIdle abstract class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionIdle.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AITarget.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdle, kAct_Idle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdle::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionIdle::CAIActionIdle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdle::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionIdle::InitAction( AIDB_ActionRecord* pActionRecord )
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
//	ROUTINE:	CAIActionIdle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionIdle::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
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
		// Set animate state.

		pAI->SetState( kState_Animate );

		// Set idle animation.

		EnumAnimProp ePosture = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Posture );
		if( ePosture == kAP_None 
			|| ePosture == kAP_POS_Mounted)
		{
			ePosture = kAP_POS_Stand;
		}

		CAnimationProps	animProps;
		animProps.Set( kAPG_Posture, ePosture );
		animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
		animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Down );

		CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
		pStateAnimate->SetAnimation( animProps, LOOP );
	}

	// Head tracking.

	if( pAI->HasTarget( kTarget_Character | kTarget_Object | kTarget_Interest ) 
		&& AIWeaponUtils::HasWeaponType( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ) 
		&& AIWeaponUtils::HasAmmo( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ) )
	{
		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
		pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
	}
	else {
		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
		pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	}
}

