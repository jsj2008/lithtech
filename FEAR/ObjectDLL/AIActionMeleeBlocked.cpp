// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMeleeBlocked.cpp
//
// PURPOSE : 
//
// CREATED : 9/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionMeleeBlocked.h"
#include "AIStateAnimate.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMeleeBlocked, kAct_MeleeBlocked );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMeleeBlocked::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionMeleeBlocked::CAIActionMeleeBlocked()
{
}

CAIActionMeleeBlocked::~CAIActionMeleeBlocked()
{
}

void CAIActionMeleeBlocked::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI recated to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_MeleeBlocked );
}

void CAIActionMeleeBlocked::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity checks; these should be verified in the preconditions.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	AIDB_AIWeaponRecord* pWeaponRecord = g_pAIDB->GetAIWeaponRecord(pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID());
	if ( !pWeaponRecord )
	{
		return;
	}

	// Set the animation based on the smartobject and the weapon held.

	pAI->SetState( kState_Animate );
	CAIStateAnimate* pAnimate = (CAIStateAnimate*)pAI->GetState();

	CAnimationProps Props = pSmartObjectRecord->Props;
	Props.Set( kAPG_Weapon, pWeaponRecord->eAIWeaponAnimProp );

	pAnimate->SetAnimation( Props, !LOOP );
}

bool CAIActionMeleeBlocked::IsActionComplete( CAI* pAI )
{
	// Reacting to being blocked is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	return false;
}


bool CAIActionMeleeBlocked::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if AI is attached to something.
	
	HOBJECT hAttached = pAI->GetAIBlackBoard()->GetBBAttachedTo();
	if( hAttached )
	{
		return false;
	}

	// Fail if there is no a smartobject assocaited with this action.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Fail if the AI is unarmed

	AIDB_AIWeaponRecord* pWeaponRecord = g_pAIDB->GetAIWeaponRecord(pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID());
	if ( !pWeaponRecord )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMeleeBlocked::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionMeleeBlocked::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
