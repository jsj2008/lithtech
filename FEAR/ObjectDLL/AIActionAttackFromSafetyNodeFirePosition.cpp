// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromSafetyNodeFirePosition.cpp
//
// PURPOSE : This action handles moving from an AINodeSafety instance to 
//			an AINodeSafetyFirePosition instance to attack the player.
//
// CREATED : 1/31/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackFromSafetyNodeFirePosition.h"
#include "AIWorkingMemory.h"
#include "AINodeSafetyFirePosition.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromSafetyNodeFirePosition, kAct_AttackFromSafetyNodeFirePosition );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromSafetyNodeFirePosition::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackFromSafetyNodeFirePosition::CAIActionAttackFromSafetyNodeFirePosition()
{
}

CAIActionAttackFromSafetyNodeFirePosition::~CAIActionAttackFromSafetyNodeFirePosition()
{
}

void CAIActionAttackFromSafetyNodeFirePosition::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Must be at a SafetyFirePosition node.
	// Must have an weapon armed
	// Must have a weapon loaded

	m_wsWorldStatePreconditions.SetWSProp( kWSK_AtNodeType, NULL, kWST_EnumAINodeType, kNode_SafetyFirePosition );
	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponLoaded, NULL, kWST_bool, true );
	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );

	// Set effects.
	// AI used object specified in the parent worldstate's kWSK_UsingObject 
	// field.
	m_wsWorldStateEffects.SetWSProp( kWSK_UsingObject, NULL, kWST_Variable, kWSK_UsingObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromSafetyNodeFirePosition::ValidateContextPreconditions()
//
//	PURPOSE:	Return true if real-time preconditions are valid.  Note 
//				that this function does NOT need to verify that the AI is 
//				at a AINodeSafety instance; if they aren't (or if they 
//				weren't very recently), they AI will not have any memories 
//				about any nodes of the required type.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromSafetyNodeFirePosition::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if the primary weapon is not a ranged weapon (this means the 
	// WorldState properties are not reflecting the ranged weapon status)

	if ( kAIWeaponType_Ranged != pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType() )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromSafetyNodeFirePosition::ActivateAction()
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromSafetyNodeFirePosition::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );
	
	// Use blind firing to insure the AI actually shoots.  If this isn't 
	// set, and the AIs enemy does not meet the firing constraints, the
 	// AI will end up locked in an Aim animation instead.

	pAI->GetAIBlackBoard()->SetBBBlindFire( true );

	// This action handles looping internally.  Clear the states looping 
	// flag.  If looping is false, handle this in the ApplyContextEffect 
	// function where successful use of the node is reflected into the 
	// worldstate.

	// Assumes the base class sets the state to CAIStateUseSmartObject.  
	// If it does not in the future, update this code.

	CAIStateUseSmartObject* pStateUseSmartObject = dynamic_cast<CAIStateUseSmartObject*>( pAI->GetState() );
	if ( !pStateUseSmartObject )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionAttackFromSafetyNodeFirePosition::ActivateAction : Unexpected AIState." );
		return;
	}

	pStateUseSmartObject->SetLooping( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromSafetyNodeFirePosition::ActivateAction()
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromSafetyNodeFirePosition::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Clear blind firing.

	pAI->GetAIBlackBoard()->SetBBBlindFire( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromSafetyNodeFirePosition::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromSafetyNodeFirePosition::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	super::ApplyContextEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );

	// To honor the smartobjects looping flag, clear the kWSK_UsingObject
	// if the AI is:
	// 1) Currently using a SmartObject node.
	// 2) The node has a smartobject
	// 3) The smartobject is looping
	// 4) The AI still has ammo loaded.

	SAIWORLDSTATE_PROP* pProp = pwsWorldStateGoal->GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( pProp->hWSValue );
		if( pNodeSmartObject )
		{
			AIDB_SmartObjectRecord* pSmartObject = pNodeSmartObject->GetSmartObject();
			if ( pSmartObject && pSmartObject->bLooping )
			{
				SAIWORLDSTATE_PROP* pWSProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponLoaded, pAI->GetHOBJECT() );
				if ( pWSProp && pWSProp->bWSValue )
				{
					pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, NULL,  kWST_HOBJECT, 0 );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromSafetyNodeFirePosition::IsActionValidForNodeType()
//
//	PURPOSE:	This action is only valid for SafetyFirePosition 
//			nodes.  This overrides the base, which is valid for 
// 			any other type of node.
//
// 			This is all a workaround to prevent the base class 
// 			from being valid when this action is not.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromSafetyNodeFirePosition::IsActionValidForNodeType( EnumAINodeType eNodeType ) const
{
	return ( eNodeType == kNode_SafetyFirePosition );
}
