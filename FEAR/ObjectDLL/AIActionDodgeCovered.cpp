// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeCovered.cpp
//
// PURPOSE : AIActionDodgeCovered class implementation
//
// CREATED : 5/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDodgeCovered.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"
#include "AIStateUseSmartObject.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeCovered, kAct_DodgeCovered );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeCovered::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDodgeCovered::CAIActionDodgeCovered()
{
	// Do not invalidate a node based on the threat aiming at the AI.

	m_dwNodeStatusFlags = kNodeStatus_All & ~kNodeStatus_ThreatAimingAtNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeCovered::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeCovered::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Clear super classes' precondition for NodeAt, because
	// we don't want the planner to try to find the node.
	// This action should only activate if the AI is already
	// at a cover node.

	m_wsWorldStatePreconditions.ClearWSProp( kWSK_AtNode, NULL );

	// Clear the super classes' effects for setting the node used,
	// because we don't want the planner to try to use this action
	// to satisfy goals based on nodes.

	m_wsWorldStateEffects.ClearWSProp( kWSK_UsingObject, NULL );
	m_wsWorldStateEffects.ClearWSProp( kWSK_AtNodeType, NULL );

	// Set effects.
	// Target is no longer aiming at me.

	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsAimingAtMe, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeCovered::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeCovered::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// AI must already be at a cover node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp )
	{
		return false;
	}

	AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if( pNodeSmartObject && ( pNodeSmartObject->GetType() == kNode_Cover ) )
	{
		// The cover node must be valid.

		if( !( pAI->HasTarget( kTarget_Character | kTarget_Object ) && 
			  ( pNodeSmartObject->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, m_dwNodeStatusFlags ) ) ) )
		{
			return false;
		}
	}

	// Bail if AI has valid cover to go to.

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, kNode_Cover, kNodeStatus_All, NULL, hTarget ) )
	{
		return false;
	}

	// Bail if AI has valid ambush to go to.

	if( pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, kNode_Ambush, kNodeStatus_All, NULL, hTarget ) )
	{
		return false;
	}

	// Only dodge if the AI is not trying to dodge a blow

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_CounterMelee );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact && ( 0.0f != pFact->GetConfidence( CAIWMFact::kFactMask_DesireType ) ) )
	{
		return false;
	}

	// Do not plan to dodge into cover unless we are currently uncovered.

	if( bIsPlanning )
	{
		pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_CoverStatus, pAI->m_hObject );
		if( !pProp )
		{
			return false;
		}

		if( pProp->eAnimPropWSValue != kAP_ATVT_Uncovered )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeCovered::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeCovered::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	// Intentionally do NOT call super::ActivateAction().
	// DodgeCovered uses a SmartObject from the Action record, 
	// rather than from the node.
	//
	// Be sure to call down to the base class however to insure that any 
	// awareness modifications are applied.
	CAIActionAbstract::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if we are not at a cover node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}

	// Bail if node does not exist.

	AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if( !pNodeSmartObject )
	{
		return;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Set UseSmartObject state.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );

	// Set the node to use.

	pStateUseSmartObject->SetNode( pNodeSmartObject );

	// Set the smart object command for the node.

	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Use the node's assigned posture.

	pSmartObjectRecord = pNodeSmartObject->GetSmartObject();
	pStateUseSmartObject->SetProp( kAPG_Posture, pSmartObjectRecord->Props.Get( kAPG_Posture ) );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeCovered::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeCovered::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
