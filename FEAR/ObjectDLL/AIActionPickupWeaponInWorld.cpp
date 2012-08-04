// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionPickupWeaponInWorld.cpp
//
// PURPOSE : 
//
// CREATED : 7/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionPickupWeaponInWorld.h"
#include "AIWorldState.h"
#include "AIBlackBoard.h"
#include "AIDB.h"
#include "AI.h"
#include "AIStateAnimate.h"
#include "WeaponItems.h"

LINKFROM_MODULE(AIActionPickupWeaponInWorld);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionPickupWeaponInWorld, kAct_PickupWeaponInWorld );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodePickupWeaponInWorld::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionPickupWeaponInWorld::CAIActionPickupWeaponInWorld()
{
}

CAIActionPickupWeaponInWorld::~CAIActionPickupWeaponInWorld()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionPickupWeaponInWorld::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionPickupWeaponInWorld::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// AI must be at the target'ts position.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_AtTargetPos, NULL, kWST_bool, true );
	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, false );

    // Set effects.
	// AI used object specified in the parent worldstate's kWSK_UsingObject 
	// field.

	m_wsWorldStateEffects.SetWSProp( kWSK_UsingObject, NULL, kWST_Variable, kWSK_UsingObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionPickupWeaponInWorld::ValidateContextPreconditions
//
//	PURPOSE:	We have a holstered weapon.  Better to draw it than to pick
//				up a weapon in the world.
//
// ----------------------------------------------------------------------- //

bool CAIActionPickupWeaponInWorld::ValidateContextPreconditions(CAI* pAI, CAIWorldState& wzWorldStateGoal, bool bIsPlanning)
{
	if (!super::ValidateContextPreconditions(pAI, wzWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// AI already has a holstered weapon.  No need to pick up a weapon.

	if (kAIWeaponID_Invalid != pAI->GetAIBlackBoard()->GetBBHolsterRightAIWeaponRecordID())
	{
		return false;
	}

	// kWST_UsingObject does not contain a WorldItem.

	SAIWORLDSTATE_PROP* pProp = wzWorldStateGoal.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if ( !pProp 
		|| !pProp->hWSValue
		|| !WeaponItem::DynamicCast(pProp->hWSValue) )
	{
		return false;
	}

	// Action does not specify a smartobject

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	AIASSERT(pSmartObjectRecord, pAI->GetHOBJECT(), "AIActionPickupWeaponInWorld action requires a smartobject");
	if (!pSmartObjectRecord)
	{
		return false;
	}

	// No object to use.

	SAIWORLDSTATE_PROP* pUsingObjectProp = wzWorldStateGoal.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if ( !pUsingObjectProp )
	{
		return false;
	}

	// AI does not have knowledge about the targeted PickupWeapon.

	CAIWMFact queryFact;
	queryFact.SetFactType(kFact_Knowledge);
	queryFact.SetKnowledgeType(kKnowledge_WeaponItem);
	queryFact.SetTargetObject(pUsingObjectProp->hWSValue);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact(queryFact);	
	if (!pFact)
	{
		return false;
	}

	// Targeted PickupWeapon has a node associated with it.

	if (pFact->GetSourceObject())
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionPickupWeaponInWorld::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionPickupWeaponInWorld::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	// Sanity Check.

	if ( !pAI )
	{
		return;
	}

	// Action does not specify a smartobject

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if (!pSmartObjectRecord)
	{
		return;
	}

	// Set animate state.

	pAI->SetState( kState_Animate );

	// Set the animation to play.

	CAIStateAnimate* pAnimate = (CAIStateAnimate*)pAI->GetState();
	pAnimate->SetAnimation( pSmartObjectRecord->Props, false );

	// No object to use.

	SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
	if ( !pProp )
	{
		return;
	}

	// Set the weapon as the AnimTarget

	pAI->SetAnimObject(pProp->hWSValue);

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );

	super::ActivateAction( pAI, wsWorldStateGoal );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionPickupWeaponInWorld::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionPickupWeaponInWorld::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Set the weapon as the AnimTarget

	pAI->SetAnimObject( NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionPickupWeaponInWorld::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionPickupWeaponInWorld::ValidateAction( CAI* pAI )
{
	if ( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Fail if the object no longer exists.
	
	if ( pAI->GetAnimObject() == NULL )
	{
		return false;
	}

	// Fail if the target object Target object changed.

	if ( pAI->GetAnimObject() != pAI->GetAIBlackBoard()->GetBBTargetObject() )
	{
		return false;
	}

	// Fail if the target type is not weapon.

	if ( false == pAI->HasTarget( kTarget_WeaponItem ) )
	{
		return false;
	}

	return true;
}
