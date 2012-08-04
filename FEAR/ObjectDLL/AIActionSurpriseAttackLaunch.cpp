// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSurpriseAttackLaunch.cpp
//
// PURPOSE : 
//
// CREATED : 2/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionSurpriseAttackLaunch.h"
#include "AINodeSurprise.h"
#include "AIStateAnimate.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSurpriseAttackLaunch, kAct_SurpriseAttackLaunch );

static AINodeSurprise* GetSurpriseNode( CAIWorldState& rWorldState, HOBJECT hObject )
{
	// Fail if the AI is not at a node of type Surprise.

	SAIWORLDSTATE_PROP* pAtNodeTypeProp = rWorldState.GetWSProp( kWSK_AtNodeType, hObject );
	if ( !pAtNodeTypeProp || 
		( kNode_Surprise != pAtNodeTypeProp->eAINodeTypeWSValue ) )
	{
		return NULL;
	}

	// Fail if we cannot get a pointer to the node.

	SAIWORLDSTATE_PROP* pAtNodeProp = rWorldState.GetWSProp( kWSK_AtNode, hObject );
	if ( !pAtNodeProp ||
		( !pAtNodeProp->hWSValue ) )
	{
		return NULL;
	}

	// Type should always be correct as we check AtNodeType earlier.  Planner
	// should keep these in sync.

	AIASSERT( 
		IsKindOf( pAtNodeProp->hWSValue, "AINodeSurprise" ),
		pAtNodeProp->hWSValue, "CAIGoalSurpriseAttackLaunch::CalculateGoalRelevance: Object is not an AINodeSurprise instance.  Verify the node type matches expectations." );
	AINodeSurprise* pSurprise = (AINodeSurprise*)g_pLTServer->HandleToObject( pAtNodeProp->hWSValue );
	if ( !pSurprise )
	{
		return NULL;
	}

	return pSurprise;
}

// Returns the attack animprop which can be used to attack the passed in 
// threat from the AINodeSurprise instance the AI is at in the passed in
// worldstate.  If the AI is not at such a node, or if there is not a 
// valid attack prop, kAP_Invalid is returned. 
static EnumAnimProp GetAtNodeAttackProp( CAIWorldState& rWorldState, HOBJECT hObject, HOBJECT hThreat )
{
	AINodeSurprise* pSurprise = GetSurpriseNode( rWorldState, hObject );
	if ( !pSurprise )
	{
		return kAP_Invalid;
	}

	return pSurprise->GetSurpriseAttackAnimationProp( hThreat );
}

// Helper function to return the AIDB_SmartObjectRecord record of the 
// AINodeSmartObject instance the AI is at in the passted in worldspace.
// Returns NULL if the AI is not at a node, the node is not a SmartObject,
// or the SmartObject doesn't have a AIDB_SmartObjectRecord.
static const AIDB_SmartObjectRecord* GetAtNodeSmartObjectRecord( CAIWorldState& rWorldState, HOBJECT hObject )
{
	SAIWORLDSTATE_PROP* pProp = rWorldState.GetWSProp( kWSK_AtNode, hObject );
	if ( !pProp || !pProp->hWSValue )
	{
		return NULL;
	}

	if ( !IsKindOf( pProp->hWSValue, "AINodeSmartObject" ) )
	{
		return NULL;
	}

	AINodeSmartObject* pSmartObjectNode = (AINodeSmartObject*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if ( !pSmartObjectNode )
	{
		return NULL;
	}

	return pSmartObjectNode->GetSmartObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurpriseAttackLaunch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionSurpriseAttackLaunch::CAIActionSurpriseAttackLaunch()
{
}

CAIActionSurpriseAttackLaunch::~CAIActionSurpriseAttackLaunch()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurpriseAttackLaunch::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionSurpriseAttackLaunch::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to his enemy being in place for a surprise attack.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_EnemyInPlaceForSurpriseAttack );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurpriseAttackLaunch::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionSurpriseAttackLaunch::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Verify the node has a smartobject.  (if it doesn't, this node will 
	// not be used.  This is an level design error, but an easy one to make,
	// so report it.

	if ( NULL == GetAtNodeSmartObjectRecord( *pAI->GetAIWorldState(), pAI->GetHOBJECT() ) )
	{
		return false;
	}

	// AI is either not at a surprise node, or the node does not have a 
	// valid action.

	if ( kAP_Invalid == GetAtNodeAttackProp(
		*pAI->GetAIWorldState(), pAI->GetHOBJECT(), 
		pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		return false;
	}

	// Fail if the AI is not facing the node.

	SAIWORLDSTATE_PROP* pAtNodeProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->GetHOBJECT() );
	if ( !pAtNodeProp ||
		( !pAtNodeProp->hWSValue ) )
	{
		return false;
	}

	AINode* pNode = AINode::HandleToObject( pAtNodeProp->hWSValue );
	if ( !pNode )
	{
		return false;
	}

	if ( pNode->GetFaceNode() )
	{
		LTVector vNodeDir = pNode->GetNodeFaceDir();
		vNodeDir.y = 0.0f;
		vNodeDir.Normalize();
		if ( vNodeDir == LTVector::GetIdentity() )
		{
			return false;
		}

		LTVector vAIForward = pAI->GetForwardVector();
		vAIForward.y = 0.0f;
		vAIForward.Normalize();
		if ( vAIForward == LTVector::GetIdentity() )
		{
			return false;
		}

		if ( vAIForward.Dot( vNodeDir ) < 0.9999f )
		{
			return false;
		}
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurpriseAttackLaunch::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionSurpriseAttackLaunch::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Verify we selected a valid action.

	EnumAnimProp eAction = GetAtNodeAttackProp( 
		*pAI->GetAIWorldState(), pAI->GetHOBJECT(), 
		pAI->GetAIBlackBoard()->GetBBTargetObject() );
	if ( kAP_Invalid == eAction )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionSurpriseAttackLaunch::ActivateAction: Failed to find an action despite passing precondition test."  );
		return;
	}

	// Verify the node specifies a smartobject.

	const AIDB_SmartObjectRecord* pSmartObject = GetAtNodeSmartObjectRecord( *pAI->GetAIWorldState(), pAI->GetHOBJECT() );
	if ( !pSmartObject )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionSurpriseAttackLaunch::ActivateAction: Failed to smartobject for node despite passing precondition test."  );
		return;
	} 

	// Notify the surprise node that it has been used.

	AINodeSurprise* pSurprise = GetSurpriseNode( *pAI->GetAIWorldState(), pAI->GetHOBJECT() );
	if ( pSurprise )
	{
		pSurprise->HandleSurpriseAttack();
	}

	// Depart from a node (this must be done AFTER we get the node)

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			pNode->HandleAIDeparture( pAI );

			// Insure we call the PostActivate to fire off any commands/to 
			// reset the activation time/to dispatch any post activate commands.

			AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( pNode->GetHOBJECT() );
			if( pNodeSmartObject )
			{
				pNodeSmartObject->PostActivate();
			}
		}
	}

	// Get the nodes smartobject and replace the action with the action 
	// determined dynamically.  The activity specifies the direction/etc.

	CAnimationProps Props = pSmartObject->Props;
	Props.Set( kAPG_Action, eAction );

	// Set the animation to play.

	pAI->SetState( kState_Animate );
	CAIStateAnimate* pAnimate = (CAIStateAnimate*)pAI->GetState();
	pAnimate->SetAnimation( Props, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );

	// Ignore the AIs radius when validating movement encoding, as this
	// animation should be fit to the geometry by level designers.

	pAI->GetAIBlackBoard()->SetBBMovementEncodeUseRadius( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurpriseAttackLaunch::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionSurpriseAttackLaunch::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction(pAI);

	// Restore the default setting

	pAI->GetAIBlackBoard()->SetBBMovementEncodeUseRadius( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSurpriseAttackLaunch::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionSurpriseAttackLaunch::IsActionComplete( CAI* pAI )
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
