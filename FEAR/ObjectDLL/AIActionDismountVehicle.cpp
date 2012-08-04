// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountVehicle.cpp
//
// PURPOSE : AIActionDismountVehicle class implementation
//
// CREATED : 12/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDismountVehicle.h"
#include "AI.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIPathKnowledgeMgr.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountVehicle, kAct_DismountVehicle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountVehicle::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDismountVehicle::CAIActionDismountVehicle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountVehicle::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountVehicle::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// AI is no longer riding anything.

	m_wsWorldStateEffects.SetWSProp( kWSK_RidingVehicle, NULL, kWST_EnumAnimProp, kAP_None );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountVehicle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountVehicle::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if AI is not attached to anything.

	HOBJECT hVehicle = pAI->GetAIBlackBoard()->GetBBAttachedTo();
	if( !hVehicle )
	{
		return;
	}

	// Detach the AI from the vehicle.

	std::string strCmd = "DETACH ";
	strCmd += pAI->GetName();
	g_pCmdMgr->QueueMessage( pAI, g_pLTServer->HandleToObject(hVehicle), strCmd.c_str() );

	// Record the vehicle the AI was attached to from the Blackboard.

	pAI->GetAIBlackBoard()->SetBBAttachedTo( NULL );

	// Get vehicle animProp.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}
	EnumAnimProp eVehicle = pProp->eAnimPropWSValue;

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set idle animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Lower );
	animProps.Set( kAPG_Action, kAP_ACT_Dismount );
	animProps.Set( kAPG_Activity, eVehicle );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountVehicle::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDismountVehicle::IsActionComplete( CAI* pAI )
{
	// Animating is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Animating is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountVehicle::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountVehicle::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Clear any cached pathfinding data, because the AI may have
	// travelled somewhere completely new.

	pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();

	// Clear the blackboard's KeyframeToRigidBody object.

	pAI->GetAIBlackBoard()->SetBBVehicleKeyframeToRigidBody( NULL );

	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

