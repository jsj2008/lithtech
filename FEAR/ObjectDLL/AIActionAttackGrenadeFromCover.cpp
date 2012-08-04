// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackGrenadeFromCover.cpp
//
// PURPOSE : AIActionAttackGrenadeFromCover class implementation
//
// CREATED : 01/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackGrenadeFromCover.h"
#include "AI.h"
#include "AINode.h"
#include "AITarget.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackGrenadeFromCover, kAct_AttackGrenadeFromCover );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenadeFromCover::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackGrenadeFromCover::CAIActionAttackGrenadeFromCover()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenadeFromCover::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackGrenadeFromCover::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Target is flushed out.

	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsFlushedOut, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenadeFromCover::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackGrenadeFromCover::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// AI must already be at a cover node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( !( pNode && ( pNode->GetType() == kNode_Cover ) ) )
		{
			return false;
		}

		bool bThrowGrenade = false;

		// Do not throw a grenade if the cover node is invalid in terms of FOV.

		if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_ThreatOutsideFOV ) )
		{
			return false;
		}

		// Throw a grenade if AI has taken damage at this node.

		double fNodeArrivalTime = pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime();

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Damage);
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && 
			( DidDamage(pAI, pFact) ) &&
			( pFact->GetUpdateTime() > fNodeArrivalTime ) &&
			( pFact->GetUpdateTime() < g_pLTServer->GetTime() - 4.f ) )
		{
			bThrowGrenade = true;
		}

		if( pAI->GetAIBlackBoard()->GetBBTargetVisibilityConfidence() < 0.75f )
		{
			bThrowGrenade = true;		
		}

		// Do not throw a grenade.

		if( !bThrowGrenade )
		{
			return false;
		}
	}

	// Default behavior.

	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Throw a grenade from cover.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackGrenadeFromCover::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackGrenadeFromCover::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !( pAI && pAI->GetState() && pAI->GetState()->GetStateClassType() == kState_Animate ) )
	{
		return;
	}

	// Invalid node handle.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !( pProp && pProp->hWSValue ) )
	{
		return;
	}
	HOBJECT hNode = pProp->hWSValue;

	// Node must be derived from SmartObject.

	HCLASS hTest  = g_pLTServer->GetClass( "AINodeSmartObject" );
	HCLASS hClass = g_pLTServer->GetObjectClass( hNode );
	if( !g_pLTServer->IsKindOf( hClass, hTest ) )
	{
		return;
	}

	// Set the anim props for attacking from a node.
	// The props are defined by the SmartObject of the Node.

	CAnimationProps	nodeProps;
	AINodeSmartObject* pNode = (AINodeSmartObject*)g_pLTServer->HandleToObject( hNode );
	if( pNode )
	{
		pNode->GetAnimProps( &nodeProps );
	}

	// Set the anim props for throwing grenades from this node.

	CAnimationProps	animProps;
	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->GetAnimationProps( &animProps );

	animProps.Set( kAPG_Posture, nodeProps.Get( kAPG_Posture ) );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Activity, kAP_ATVT_Uncovered );
	animProps.Set( kAPG_MovementDir, nodeProps.Get( kAPG_MovementDir ) );

	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );

	// Face the direction of the node.

	if( pNode && pNode->GetFaceNode() )
	{
		pAI->GetAIBlackBoard()->SetBBFaceDir( pNode->GetNodeFaceDir() );
	}
}
