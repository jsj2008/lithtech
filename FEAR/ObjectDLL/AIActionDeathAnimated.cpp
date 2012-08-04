// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDeathAnimated.cpp
//
// PURPOSE : 
//
// CREATED : 3/12/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDeathAnimated.h"
#include "AIStateAnimate.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDeathAnimated, kAct_DeathAnimated );

static void GetAnimationProps( CAI* pAI, AIDB_SmartObjectRecord* pSmartObject, CAnimationProps* pOutAnimProps )
{
	// Sanity check.

	if( !( pAI && pOutAnimProps && pSmartObject ) )
	{
		return;
	}

	// Copy the props from the SmartObjectRecord and override those which 
	// are dynamic (such as the weapon).

	*pOutAnimProps = pSmartObject->Props;
	pOutAnimProps->Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
}

static AIDB_SmartObjectRecord* GetActionSmartObject( CAIActionAbstract* pAction )
{
	return g_pAIDB->GetAISmartObjectRecord( pAction->GetActionRecord()->eSmartObjectID );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathAnimated::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDeathAnimated::CAIActionDeathAnimated()
{
}

CAIActionDeathAnimated::~CAIActionDeathAnimated()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathAnimated::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathAnimated::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathAnimated::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDeathAnimated::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Verify we have a smartobject.  If we don't, this action cannot be used.

	if ( NULL == GetActionSmartObject(this) )
	{
		AIASSERT1( 0, pAI->GetHOBJECT(), 
			"CAIActionDeathAnimated::ValidateContextPreconditions: AI action '%s' requires a SmartObject to specify animations.  No SmartObject was specified.", 
			s_aszActionTypes[GetActionRecord()->eActionType] );
		return false;
	}

	// AI must actually be dead

	if( !pAI->GetDestructible()->IsDead() )
	{
		return false;
	}

	// Death animation does not exist.

	CAnimationProps	animProps;
	GetAnimationProps( pAI, GetActionSmartObject(this), &animProps );
	uint32 cAnimations = pAI->GetAnimationContext()->CountAnimations( animProps );
	if( 0 == cAnimations )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathAnimated::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathAnimated::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set death animation.

	CAnimationProps	animProps;
	GetAnimationProps( pAI, GetActionSmartObject(this), &animProps );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Circumvent the normal CCharacter code path for handling 
	// deaths, and creating Body objects.

	pAI->GetAIBlackBoard()->SetBBAIHandlingDeath( true );

	// Ragdoll our body and perform death cleanup.

	pAI->SetDeathAnimation( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathAnimated::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDeathAnimated::IsActionComplete( CAI* pAI )
{
	// Death is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Death is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDeathAnimated::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDeathAnimated::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Stop updating the AI completely and remove the tag indicating this
	// behavior is handling death to allow normal body processing to resume.

	pAI->GetAIBlackBoard()->SetBBAIHandlingDeath( false );
	pAI->SetUpdateAI( false );
}
