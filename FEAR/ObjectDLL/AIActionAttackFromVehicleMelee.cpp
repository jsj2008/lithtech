// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromVehicleMelee.cpp
//
// PURPOSE : 
//
// CREATED : 3/22/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackFromVehicleMelee.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromVehicleMelee, kAct_AttackFromVehicleMelee );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicleMelee::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackFromVehicleMelee::CAIActionAttackFromVehicleMelee()
{
}

CAIActionAttackFromVehicleMelee::~CAIActionAttackFromVehicleMelee()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicleMelee::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromVehicleMelee::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Action is only valid if AI is riding a Vehicle.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( ( !pProp ) ||
		( pProp->eAnimPropWSValue == kAP_None ) )
	{
		return false;
	}

	// AI does not have any ammo for this weapon.

	if ( !AIWeaponUtils::HasAmmo( pAI, GetWeaponType(), bIsPlanning ) )
	{
		return false;
	}

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackMelee::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromVehicleMelee::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	super::SetAttackAnimProps( pAI, pProps );

	// Sanity check.

	if( !pAI || !pProps )
	{
		return;
	}

	// Action is only valid if AI is riding a Vehicle.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( ( !pProp ) ||
		( pProp->eAnimPropWSValue == kAP_None ) )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionAttackFromVehicleMelee::SetAttackAnimProps: AI is not riding a vehicle." );
		return;
	}

	pProps->Set( kAPG_Activity, pProp->eAnimPropWSValue );
}
