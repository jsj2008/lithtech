// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromView.cpp
//
// PURPOSE : AIActionAttackFromView abstract class implementation
//
// CREATED : 10/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIActionAttackFromView.h

#include "Stdafx.h"
#include "AIActionAttackFromView.h"
#include "AI.h"
#include "AINode.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromView, kAct_AttackFromView );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromView::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackFromView::CAIActionAttackFromView()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromView::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromView::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions, in addition to those set by AIActionAttack.
	// Must be at a node of the type specifed in the record.

	if ( m_pActionRecord )
	{
		m_wsWorldStatePreconditions.SetWSProp( kWSK_AtNodeType, NULL, kWST_EnumAINodeType, m_pActionRecord->eNodeType );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromView::ValidateWSPreconditions
//
//	PURPOSE:	Return true if this action's preconditions are met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromView::ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK )
{
	if( !super::ValidateWSPreconditions( pAI, wsWorldStateCur, wsWorldStateGoal, pFailedWSK ) )
	{
		return false;
	}

	// If the AI is truly already at a ViewNode, ensure that it is a valid node.
	// We know the AI is already at a ViewNode if the precondition passed the super:: check,
	// and there is no AtNode property in the plan's WorldStateCur.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateCur.GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( !pProp )
	{
		pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
		if( pProp )
		{
			HOBJECT hThreat = pAI->GetAIBlackBoard()->GetBBTargetObject();
			AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
			if( ( !pNode ) ||
				( !pNode->IsNodeValid( pAI, pAI->GetPosition(), hThreat, kThreatPos_TargetPos, kNodeStatus_All ) ) )
			{
				return false;
			}
		}
	}

	// Node is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromView::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromView::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Intentionally do not call super::ValidateContextPreconditions(). 
	// Firing from view ignores range.

	// AI has no ammo for this weapon.

	HAMMO hAmmo = AIWeaponUtils::GetWeaponAmmo(pAI, GetWeaponType(), bIsPlanning);
	if (!hAmmo || 0 == pAI->GetArsenal()->GetAmmoCount(hAmmo))
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromView::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromView::ValidateAction( CAI* pAI )
{
	// Intentionally do not call super::ValidateAction(). 
	// Firing from view ignores range.

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

	// Action is valid if the node status is valid in relation to the threat.

	pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			// Bail if the node is not actually a ViewNode.  This allows other
			// attack actions to take over (e.g. AttackFromCover).
			// This is kindof a hack!  But it lets us use the ViewNode system to
			// get the AI to other kinds of nodes.

			if( pNode->GetType() != m_pActionRecord->eNodeType )
			{
				return false;
			}

			if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, kNodeStatus_All ) )
			{
				return false;
			}
		}
	}

	return true;
}

