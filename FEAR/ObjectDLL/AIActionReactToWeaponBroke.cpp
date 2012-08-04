// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReactToWeaponBroke.cpp
//
// PURPOSE : 
//
// CREATED : 3/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionReactToWeaponBroke.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReactToWeaponBroke, kAct_ReactToWeaponBroke );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToWeaponBroke::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionReactToWeaponBroke::CAIActionReactToWeaponBroke()
{
}

CAIActionReactToWeaponBroke::~CAIActionReactToWeaponBroke()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToWeaponBroke::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToWeaponBroke::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to being shoved.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_WeaponBroke );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToWeaponBroke::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionReactToWeaponBroke::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// Fail if the AI isn't attempting to react to a broken weapon.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_ReactedToWorldStateEvent, pAI->GetHOBJECT() );
	if ( NULL == pProp || kWSE_WeaponBroke != pProp->eAIWorldStateEventWSValue )
	{
		return false;
	}

	// Fail if the action does not have a smartobject specified (and assert, as 
	// this action us useless without one)

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if ( NULL == pSmartObjectRecord )
	{
		AIASSERT1( 0, pAI->GetHOBJECT(), "CAIActionReactToWeaponBroke::ValidateContextPreconditions: Action %s does not specify a smartobject in the game database -- this action cannot be used.", s_aszActionTypes[GetActionRecord()->eActionType] );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToWeaponBroke::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToWeaponBroke::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Apply the animation specified in the smartobject directly.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	AIASSERT( pSmartObjectRecord, pAI->GetHOBJECT(), "CAIActionReactToWeaponBroke::ActivateAction: No SmartObjectRecord specified." );

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Do not track or face.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToWeaponBroke::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionReactToWeaponBroke::IsActionComplete( CAI* pAI )
{
	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}
	
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReactToWeaponBroke::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionReactToWeaponBroke::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
