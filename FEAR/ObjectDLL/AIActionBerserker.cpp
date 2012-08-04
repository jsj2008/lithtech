// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionBerserker.cpp
//
// PURPOSE : 
//
// CREATED : 8/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionBerserker.h"
#include "AIStateUseSmartObject.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"

LINKFROM_MODULE(AIActionBerserker);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionBerserker, kAct_Berserker );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserker::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionBerserker::CAIActionBerserker()
{
}

CAIActionBerserker::~CAIActionBerserker()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserker::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionBerserker::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Must have mounted the player.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_MountedObject, NULL, kWST_Variable, kWSK_UsingObject );

	// Set effects.
	//
	// AI is using the object specified in the parents worldstate's 
	// kWSK_UsingObject field.

	m_wsWorldStateEffects.SetWSProp( kWSK_UsingObject, NULL, kWST_Variable, kWSK_UsingObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserker::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionBerserker::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ))
	{
		return false;
	}

	if ( bIsPlanning )
	{
		// An AI which is currently at a node may not perform a berserker attack.
		// This prevents ambiguity; an AI mounted at an ambush node needs to resolve 
		// the mounted flag; the player must be mounted, not the node.  As mounting 
		// does not specify a target object, we need to insure this does not 
		// introduce confusion about the state.

		SAIWORLDSTATE_PROP* pAtNodeProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
		if ( pAtNodeProp && NULL != pAtNodeProp->hWSValue )
		{
			return false;
		}

		// Goals UsingObject must be a player object

		SAIWORLDSTATE_PROP* pUsingObjectProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject );
		if ( !pUsingObjectProp || !IsPlayer( pUsingObjectProp->hWSValue ) )
		{
			return false;
		}

		// Fail if there is not smartobject record for this action.

		AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
		if( !pSmartObjectRecord )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserker::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionBerserker::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Must have a UsingObject specified.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject );
	if ( !pProp 
		|| !pProp->hWSValue 
		|| !IsPlayer( pProp->hWSValue ) )
	{
		AIASSERT( pProp, pAI->GetHOBJECT(), "CAIActionMountPlayer::ActivateAction : No UsingObject specified." );
		return;
	}

	// Set the UsingObject as AnimObject
	// Play the animation

	// Fail if there is not smartobject record for this action.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );

	// AI.cpp assumes the the activity is kAPG_ATVT_Berserking and the action 
	// kAPG_ACT_AttackMelee, as this is used to set the bodystate to the correct
	// value.  This really should be data driven.  If the props change, update 
	// this assert and the bodystate code in ai.cpp.
	
	AIASSERT( kAP_ACT_AttackBerserker == pSmartObjectRecord->Props.Get( kAPG_Action ), pAI->GetHOBJECT(), "CAIActionBerserker::ActivateAction: Berserking must use Action=AttackBerserker for the attack." );

	// Set the state to animate, and configure the state.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );

	// Turn towards target, but don't track.  Face this instantly, so that 
	// the AI doesn't 'miss'

	pAI->GetAIBlackBoard()->SetBBFaceObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	pAI->GetAIBlackBoard()->SetBBFaceTargetRotImmediately( true );

	// Set the player as the anim object

	pAI->SetAnimObject( pProp->hWSValue );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserker::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionBerserker::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Clear the anim object

	pAI->SetAnimObject( NULL );
}

void CAIActionBerserker::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	super::ApplyContextEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );

	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserker::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionBerserker::IsActionComplete( CAI* pAI )
{
	// Berserker is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Berserker is not complete.

	return false;
}
