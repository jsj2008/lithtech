// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDropWeapon.cpp
//
// PURPOSE : 
//
// CREATED : 7/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDropWeapon.h"
#include "AI.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"

LINKFROM_MODULE(AIActionDropWeapon);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDropWeapon, kAct_DropWeapon );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDropWeapon::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDropWeapon::CAIActionDropWeapon()
{
}

CAIActionDropWeapon::~CAIActionDropWeapon()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDropWeapon::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDropWeapon::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Weapon is not armed.

	m_wsWorldStateEffects.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDropWeapon::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDropWeapon::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set holster animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_Action, kAP_ACT_DropWeapon );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDropWeapon::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDropWeapon::IsActionComplete( CAI* pAI )
{
	// Dropping is complete when state has completed.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Did not need to drop weapon.

	if( !pAI->GetAnimationContext()->IsLocked() )
	{
		SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponArmed, pAI->m_hObject );
		if( !pProp->bWSValue )
		{
			return true;
		}
	}

	// Dropping weapon is not complete.

	return false;
}
