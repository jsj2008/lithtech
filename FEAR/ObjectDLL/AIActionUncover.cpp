// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionUncover.cpp
//
// PURPOSE : AIActionUncover class implementation
//
// CREATED : 6/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIActionUncover.h

#include "Stdafx.h"
#include "AIWorldState.h"
#include "AIActionAbstract.h"
#include "AIActionUncover.h"

// Includes required for AIActionUncover.cpp

#include "AI.h"
#include "AINode.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIBrain.h"
#include "AIPathMgrNavMesh.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionUncover, kAct_Uncover );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUncover::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionUncover::CAIActionUncover()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUncover::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionUncover::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI is not covered.

	m_wsWorldStateEffects.SetWSProp( kWSK_CoverStatus, NULL, kWST_EnumAnimProp, kAP_None );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUncover::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionUncover::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if no Cover node.

	AINode* pNode;
	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}
	pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );


	// Determine if there's room to roll.

	float fDodgeDist = pAI->GetBrain()->GetDodgeVectorRollDist();
	LTVector vPos = pAI->GetPosition();
	LTVector vOffset;

	if( pNode->GetRot().Up().y < 0.0f )
	{
		vOffset = -pNode->GetRot().Right() * fDodgeDist;
	}
	else {
		vOffset = pNode->GetRot().Right() * fDodgeDist;
	}

	// No straight path exists, so just shuffle.

	EnumAnimProp ePosture = kAP_POS_Crouch;
	EnumAnimProp eAction = kAP_ACT_Roll;
	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), vPos, vPos + vOffset, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		ePosture = kAP_POS_Stand;
		eAction = kAP_ACT_Shuffle;
	}


	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set uncover animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, ePosture );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Action, eAction );
	if( pNode->GetRot().Up().y < 0.0f )
	{
		animProps.Set( kAPG_MovementDir, kAP_MDIR_Left );
	}
	else {
		animProps.Set( kAPG_MovementDir, kAP_MDIR_Right );
	}

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Depart from an AINode.

	pStateAnimate->DepartNode();

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionUncover::IsActionComplete( CAI* pAI )
{
	// Unocvering is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Uncovering is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUncover::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionUncover::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
