// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionUseSmartObjectNode.cpp
//
// PURPOSE : AIActionUseSmartObjectNode class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionUseSmartObjectNode.h"
#include "AI.h"
#include "AIDB.h"
#include "AIState.h"
#include "AIStateUseSmartObject.h"
#include "AINode.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"
#include "NodeTrackerContext.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionUseSmartObjectNode, kAct_UseSmartObjectNode );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionUseSmartObjectNode::CAIActionUseSmartObjectNode()
{
	// Validate all node conditions.

	m_dwNodeStatusFlags = kNodeStatus_All;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionUseSmartObjectNode::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// The node to use is a variable. Which node depends on the goal 
	// or action we are trying to satisfy.

	// Set preconditions.
	// Must be at the node.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_AtNode, NULL, kWST_Variable, kWSK_UsingObject );

	// Set effects.
	// Using the node.  

	m_wsWorldStateEffects.SetWSProp( kWSK_UsingObject, NULL, kWST_Variable, kWSK_UsingObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::SetPlanWSPreconditions
//
//	PURPOSE:	Set this action's preconditions in plan's goal world state.
//
// ----------------------------------------------------------------------- //

void CAIActionUseSmartObjectNode::SetPlanWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::SetPlanWSPreconditions( pAI, wsWorldStateGoal );

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		// Handle determing if weapon should be drawn or holstered.
		
		// If the AI is using a smartobject, he may have to have his weapon drawn or holstered,
		// depending on the requirements of the smartobject itself.

		if (IsKindOf(pProp->hWSValue, "AINodeSmartObject"))
		{
			// Check to see if this smart object AI is at has a weapon position requirement.

			AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( pProp->hWSValue );
			if (pNodeSmartObject)
			{
				AIDB_SmartObjectRecord* pSmartObject = pNodeSmartObject->GetSmartObject();
				if (pSmartObject)
				{
					// Requires AI has weapon draw.

					if (pSmartObject->eIsArmedRequirement == AIDB_SmartObjectRecord::kIsArmedRequirement_Drawn)
					{
						wsWorldStateGoal.SetWSProp( kWSK_WeaponArmed, pAI->m_hObject, kWST_bool, true );
					}

					// Requires the AI be holstered.

					if (pSmartObject->eIsArmedRequirement == AIDB_SmartObjectRecord::kIsArmedRequirement_Holstered)
					{
						wsWorldStateGoal.SetWSProp( kWSK_WeaponArmed, pAI->m_hObject, kWST_bool, false );
					}
				}
			}
		}

		// Requires a loaded weapon.

		if ( IsKindOf(pProp->hWSValue, "AINodeSafety") )
		{
			wsWorldStateGoal.SetWSProp( kWSK_WeaponLoaded, pAI->m_hObject, kWST_bool, true );
		}

		// Handle a destination dependency node

		// Add additional preconditions if using this node depends 
		// on first using another node.

		HOBJECT hDependency = GetNodeDependency( pProp->hWSValue );
		AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( hDependency );
		if( pNodeSmartObject && !pNodeSmartObject->IsNodeDisabled() )
		{
			// Only add a precondition if the type of dependency is a destination.

			AIDB_SmartObjectRecord* pRecord = pNodeSmartObject->GetSmartObject();
			if( pRecord && pRecord->eDependencyType == kDependency_Destination )
			{
				wsWorldStateGoal.SetWSProp( kWSK_UsingObject, pAI->m_hObject, kWST_HOBJECT, hDependency );

				// Determine how to use the depency node, in terms of which 
				// SmartObject node type flag.

				EnumAINodeType eNodeType = pNodeSmartObject->GetSmartObject()->eNodeType;
				wsWorldStateGoal.SetWSProp( kWSK_AtNodeType, pAI->m_hObject, kWST_EnumAINodeType, eNodeType );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::ValidateContextPreconditions
//
//	PURPOSE:	Insure the object in the UsingObject field is an 
//			AINodeSMartObject derived class.  If it isn't,
//			this action must not pass preconditions, as it 
//			copies the contents of this field into AtNode.
//			If this happens, several actions assume the object
//			really is a node, resulting in a crash
//
// ----------------------------------------------------------------------- //

bool CAIActionUseSmartObjectNode::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning) )
	{
		return false;
	}

	SAIWORLDSTATE_PROP* pUsingProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject );

	// A derived class may have cleared the effect of setting the using object.
	// If this happens, this pointer is NULL.  This is acceptable, and does not
	// invalidate the preconditions.  Eventually, this action should be split 
	// into CAIActionUseSmartObject and CAIActionUseSmartObjectNode for actions
	// which do not specify a node.

	if ( pUsingProp )
	{
		// If the contents of the kWSK_UsingObject field specifies an object, and 
		// if it is not an AINodeSmartObject, do not chain.  If this action did 
		// pass the context validation, the object in kWSK_UsingObject would be 
		// placed in kWSK_AtNode.  If this object is not a node, this could cause 
		// crashes as the contents of kWSK_AtNode is assumed to be a node.

		AINodeSmartObject* pGoalNode = AINodeSmartObject::DynamicCast( pUsingProp->hWSValue );
		if ( !pGoalNode )
		{
			return false;
		}

		// Fail if this action should not handle this node type.  This enables 
		// particular actions to be the 'exclusive' actions for using particular 
		// node types.

		if ( !IsActionValidForNodeType( pGoalNode->GetType() ) )
		{
			return false;
		}

		// Insure someone is at the dependency, if there is a dependency 
		// that requires occupation.

		HOBJECT hDependency = GetNodeDependency( pUsingProp->hWSValue );
		AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( hDependency );
		if( pNodeSmartObject )
		{
			AIDB_SmartObjectRecord* pRecord = pNodeSmartObject->GetSmartObject();
			if( pRecord && pRecord->eDependencyType == kDependency_Occupied )
			{
				// Node is unusable if dependency is disabled.

				if( pNodeSmartObject->IsNodeDisabled() )
				{
					return false;
				}

				// Node is unusable if locking AI does not exist.

				HOBJECT hLockingAI = pNodeSmartObject->GetLockingAI();
				CAI* pLockingAI = (CAI*)g_pLTServer->HandleToObject( hLockingAI );
				if( !pLockingAI )
				{
					return false;
				}

				// Node is not occupied if the locking AI is not at the node.

				SAIWORLDSTATE_PROP* pAtProp = pLockingAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pLockingAI->m_hObject );
				if( !( pAtProp && pAtProp->hWSValue == hDependency ) )
				{
					return false;
				}
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::ValidateWSPreconditions
//
//	PURPOSE:	Return true if this action's preconditions are met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionUseSmartObjectNode::ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK )
{
	if( !super::ValidateWSPreconditions( pAI, wsWorldStateCur, wsWorldStateGoal, pFailedWSK ) )
	{
		return false;
	}

	// Check additional preconditions for using the node that
	// this node depends on.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateCur.DereferenceWSProp( kWSK_AtNode );
	if( pProp )
	{
		// Insure the 
		if (IsKindOf(pProp->hWSValue, "AINodeSmartObject"))
		{
			AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( pProp->hWSValue );
			if (pNodeSmartObject)
			{
				AIDB_SmartObjectRecord* pSmartObject = pNodeSmartObject->GetSmartObject();
				if (pSmartObject)
				{
					//
					// Check to see if this smart object AI is at has a weapon position requirement.
					//

					SAIWORLDSTATE_PROP* pArmed = wsWorldStateCur.GetWSProp( kWSK_WeaponArmed, pAI->m_hObject );
					if (pArmed)
					{
						// Requires AI has weapon draw.

						if (pSmartObject->eIsArmedRequirement == AIDB_SmartObjectRecord::kIsArmedRequirement_Drawn)
						{
							if (pArmed->bWSValue == false)
							{
								if ( pFailedWSK ) *pFailedWSK = pArmed->eWSKey;
								return false;
							}
						}

						// Requires the AI be holstered.

						if (pSmartObject->eIsArmedRequirement == AIDB_SmartObjectRecord::kIsArmedRequirement_Holstered)
						{
							if (pArmed->bWSValue == true)
							{
								if ( pFailedWSK ) *pFailedWSK = pArmed->eWSKey;
								return false;
							}
						}
					}

					//
					// Check to see if the node:
					// 1) Requires mounting.
					// 2) The AI has not yet mounted it.
					// 3) This action does not mount.
					//

					if ( pSmartObject->Props.Get( kAPG_Posture ) == kAP_POS_Mounted )
					{
						SAIWORLDSTATE_PROP* pMountedPropCur = wsWorldStateCur.DereferenceWSProp( kWSK_MountedObject );
						if ( !pMountedPropCur || pProp->hWSValue != pMountedPropCur->hWSValue )
						{
							if ( !m_wsWorldStateEffects.HasWSProp( kWSK_MountedObject, NULL ) )
							{
								if ( pFailedWSK ) *pFailedWSK = kWSK_MountedObject;
								return false;
							}
						}
					}
				}
			}
		}

		// Insure the AI is at the destination dependency, if there is one.

		HOBJECT hDependency = GetNodeDependency( pProp->hWSValue );
		AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( hDependency );
		if( pNodeSmartObject && !pNodeSmartObject->IsNodeDisabled() )
		{
			// Only require a precondition if the dependency type is a destination.

			AIDB_SmartObjectRecord* pRecord = pNodeSmartObject->GetSmartObject();
			if( pRecord && pRecord->eDependencyType == kDependency_Destination )
			{
				pProp = wsWorldStateCur.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
				if( pProp )
				{
					return ( hDependency == pProp->hWSValue );
				}
			}
		}
	}

	// No dependency.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::GetNodeDependency
//
//	PURPOSE:	Return the handle to the node the specified node depends on.
//
// ----------------------------------------------------------------------- //

HOBJECT CAIActionUseSmartObjectNode::GetNodeDependency( HOBJECT hNode )
{
	// The passed in node is not a SmartObject and therefore has no 
	// dependencies.

	if (!IsKindOf(hNode, "AINodeSmartObject"))
	{
		return NULL;
	}

	// AINodeSmartObjects may specify a node that they depend on.
	// The optional dependency must be used before this node.

	AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( hNode );
	if( pNodeSmartObject )
	{
		return pNodeSmartObject->GetDependency();
	}

	// No dependency.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionUseSmartObjectNode::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Find which node we are using from the goal world state.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if( pProp )
	{
		// Bail if node does not exist.

		AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( pProp->hWSValue );
		if( !pNodeSmartObject )
		{
			return;
		}

		// Bail if the smart object command for the node does not exist.
		// Consider the node immediately used.

		AIDB_SmartObjectRecord* pSmartObject = pNodeSmartObject->GetSmartObject();
		if( ( !pSmartObject ) ||
			(  pSmartObject->eNodeType == kNode_InvalidType ) )
		{
			pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, pAI->m_hObject, kWST_HOBJECT, pProp->hWSValue );
			return;
		}

		// Set UseSmartObject state.

		pAI->SetState( kState_UseSmartObject );
		CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );

		// Set the node to use.

		pStateUseSmartObject->SetNode( pNodeSmartObject );

		// Set the smart object command for the node.

		pStateUseSmartObject->SetSmartObject( pSmartObject );

		// Handle animation on Cover nodes specially, 
		// because the right/left direction of the Evasive 
		// may be reversed if the node is rotated.

		if( pSmartObject->eNodeType == kNode_Cover )
		{
			CAnimationProps Props;
			AINodeCover* pNodeCover = (AINodeCover*)pNodeSmartObject;
			pNodeCover->GetAnimProps( &Props );
			pStateUseSmartObject->SetProp( kAPG_MovementDir, Props.Get( kAPG_MovementDir ) );
		}

		// Torso tracking.

		if( pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert )
		{
			// Aim ranged weapon at target.

			if( AIWeaponUtils::HasWeaponType( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ) 
				&& AIWeaponUtils::HasAmmo( pAI, kAIWeaponType_Ranged, !AIWEAP_CHECK_HOLSTER ))
			{
				pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
			}

			// Look at target.

			else {
				pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
			}
		}
		else {
			pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
		}
		pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::DeactivateAction
//
//	PURPOSE:	Deactivate the action.
//
// ----------------------------------------------------------------------- //

void CAIActionUseSmartObjectNode::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	if ( !pAI )
	{
		return;
	}

	// Unlock the animation if it is currently locked.  This will prevent 
	// an AI going to idle from looping whatever animation is currently 
	// playing.

	if ( pAI->GetAnimationContext()->IsLocked() )
	{
		pAI->GetAnimationContext()->Unlock();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionUseSmartObjectNode::IsActionComplete( CAI* pAI )
{
	// If the state is something other than UseSmartObject,
	// then the Action tried to activate with an invalid node.

	if( ( !pAI->GetState() ) ||
		(  pAI->GetState()->GetStateClassType() != kState_UseSmartObject ) )
	{
		return true;
	}

	// Animation is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Animation is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionUseSmartObjectNode::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// AI has just arrived at the node.

	if( g_pLTServer->GetTime() == pAI->GetAIBlackBoard()->GetBBStateChangeTime() )
	{
		return true;
	}

	// AI is transitioning or playing a locked anim.

	if( pAI->GetAnimationContext()->IsTransitioning() ||
		pAI->GetAnimationContext()->IsLocked() )
	{
		return true;
	}

	// Action is valid if the node status is invalid.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp )
	{
		// Something has gone awry if the AI is not at a node!
		// JEFFO: This is needed due to an AI failing to pathfind,
		// and returning kAIStateStatus_Complete from AIStateGoto.
		// We need to somehow handle failure of teh Goto state.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( !pNode )
		{
			return false;
		}

		// Node must still be valid.

		if( !pNode->IsNodeValid( pAI, pAI->GetPosition(), pAI->GetAIBlackBoard()->GetBBTargetObject(), kThreatPos_TargetPos, m_dwNodeStatusFlags ) )
		{
			return false;
		}
	}

	// Action is still valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionUseSmartObjectNode::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !( pAI && pwsWorldStateCur && pwsWorldStateGoal ) )
	{
		return;
	}

	// Bail if WorldState has no knowledge of AINode being used.

	SAIWORLDSTATE_PROP* pProp = pwsWorldStateGoal->GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if( !( pProp && pProp->hWSValue ) )
	{
		return;
	}

	// Bail if AINode is not an AINodeSmartObject.

	AINodeSmartObject* pNodeSmartObject = AINodeSmartObject::DynamicCast( pProp->hWSValue );
	if( !pNodeSmartObject )
	{
		return;
	}

	// Run the PostActivate command on the node.

	pNodeSmartObject->PostActivate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNode::IsActionValidForNodeType
//
//	PURPOSE:	Allow UseSmartObjectNode derived actions to filter out 
//				node types the generic action should not handle.  This
//				enables other actions to be the exclusive handlers for 
//				particular node types; derived actions may impose 
//				additional constraints for using a node such as a loaded 
//				weapon.  Without preventing the base class from operating,
//				this class will be valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionUseSmartObjectNode::IsActionValidForNodeType( EnumAINodeType eNodeType ) const
{
	return ( eNodeType != kNode_SafetyFirePosition );
}
