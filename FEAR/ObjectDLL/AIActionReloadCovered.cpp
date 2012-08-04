// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReloadCovered.cpp
//
// PURPOSE : AIActionReloadCovered class implementation
//
// CREATED : 5/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionReloadCovered.h"
#include "AI.h"
#include "AINode.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReloadCovered, kAct_ReloadCovered );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadCovered::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionReloadCovered::CAIActionReloadCovered()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadCovered::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionReloadCovered::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// AI must already be at a cover node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode && ( pNode->GetType() == kNode_Cover ) )
		{
			// The cover node must be valid.

			if( pAI->HasTarget( kTarget_Character | kTarget_Object ) && 
				( pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_ThreatOutsideFOV ) ) )
			{
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionReloadCovered::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionReloadCovered::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( pAI->GetState()->GetStateClassType() != kState_Animate )
	{
		return;
	}

	// Set the additional anim props for attacking from cover.

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	if( pStateAnimate )
	{
		// Crouch while reloading at a crouch cover node.

		CAnimationProps PropsCover;
		SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
		if( pProp )
		{
			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
			if( pNode && ( pNode->GetType() == kNode_Cover ) )
			{
				AINodeCover* pNodeCover = (AINodeCover*)pNode;
				pNodeCover->GetAnimProps( &PropsCover );
			}
		}

		CAnimationProps Props;
		pStateAnimate->GetAnimationProps( &Props );
		Props.Set( kAPG_Posture, PropsCover.Get( kAPG_Posture ) );

		pStateAnimate->SetAnimation( Props, !LOOP );
	}

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}
