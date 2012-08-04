// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromNode.cpp
//
// PURPOSE : AIActionAttackFromNode class implementation
//
// CREATED : 6/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackFromNode.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromNode, kAct_AttackFromNode );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromNode::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackFromNode::CAIActionAttackFromNode()
{
	m_dwNodeStatus = kNodeStatus_ThreatOutsideFOV;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromNode::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromNode::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Intentionally do not call super::ValidateContextPreconditions(). 
	// Firing from a node ignores range.

	// AI does not have a weapon of the correct type

	if (!AIWeaponUtils::HasWeaponType(pAI, GetWeaponType(), bIsPlanning))
	{
		return false;
	}

	// AI does not have any ammo required by this weapon type.

	if ( !AIWeaponUtils::HasAmmo(pAI, GetWeaponType(), bIsPlanning ) )
	{
		return false;
	}

	// AI must already be at a node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !( pProp && pProp->hWSValue ) )
	{
		return false;
	}
	HOBJECT hNode = pProp->hWSValue;

	// Node must be derived from SmartObject.

	HCLASS hTest  = g_pLTServer->GetClass( "AINodeSmartObject" );
	HCLASS hClass = g_pLTServer->GetObjectClass( hNode );
	if( !g_pLTServer->IsKindOf( hClass, hTest ) )
	{
		return false;
	}

	// Node must be correct type.

	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
	if( !( pNode && ( pNode->GetType() == m_pActionRecord->eNodeType ) ) )
	{
		return false;
	}

	// The node must be valid in terms of FOV.

	if( pAI->HasTarget( kTarget_Character | kTarget_Object | kTarget_CombatOpportunity ) && 
		pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, m_dwNodeStatus ) )
	{
		return true;
	}

	// Preconditions are not valid.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromNode::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromNode::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );

	// Face the direction of the node.

	CAnimationProps Props;
	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode && pNode->GetFaceNode() )
		{
			pAI->GetAIBlackBoard()->SetBBFaceDir( pNode->GetNodeFaceDir() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromNode::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromNode::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Intentionally do not call super::SetAttackAnimProps(). 
	// Firing from node gets AnimProps from the node.

	// Sanity check.

	if( !( pAI && pProps ) )
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

	AINodeSmartObject* pNode = (AINodeSmartObject*)g_pLTServer->HandleToObject( hNode );
	if( pNode )
	{
		pNode->GetAnimProps( pProps );
	}

	// Fire from the node.

	pProps->Set( kAPG_Action, kAP_ACT_Fire );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromNode::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromNode::ValidateAction( CAI* pAI )
{
	// Intentionally do not call super::ValidateAction(). 
	// Firing from a node ignores range.

	if( !CAIActionAbstract::ValidateAction( pAI ) )
	{
		return false;
	}

	// Weapon is unloaded.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponLoaded, pAI->m_hObject );
	if( pProp && !pProp->bWSValue )
	{
		return false;
	}

	// Action is valid if the node status is valid in terms of FOV in relation to the threat.

	pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, m_dwNodeStatus ) )
			{
				return false;
			}
		}
	}

	return true;
}

