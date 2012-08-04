// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionChangePrimaryWeapon.cpp
//
// PURPOSE : 
//
// CREATED : 6/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionChangePrimaryWeapon.h"
#include "AIDB.h"
#include "AIWeaponUtils.h"
#include "AIBlackBoard.h"
#include "AI.h"
#include "AIStateAnimate.h"
#include "Weapon.h"

LINKFROM_MODULE(AIActionChangePrimaryWeapon);


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionChangePrimaryWeapon, kAct_ChangePrimaryWeapon );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionChangePrimaryWeapon::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionChangePrimaryWeapon::CAIActionChangePrimaryWeapon()
{
}

CAIActionChangePrimaryWeapon::~CAIActionChangePrimaryWeapon()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionChangePrimaryWeapon::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionChangePrimaryWeapon::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Weapon is loaded.

	m_wsWorldStateEffects.SetWSProp( kWSK_WeaponLoaded, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionChangePrimaryWeapon::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionChangePrimaryWeapon::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// Fail if AI does not currently have a primary weapon.

	CWeapon* pWeapon = pAI->GetAIWeaponMgr()->GetWeaponOfType(pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType());
	if ( !pWeapon )
	{
		return false;
	}
	
	// Fail if the AI still has ammo for his primary weapon.

	if ( AIWeaponUtils::HasAmmo( pAI, pWeapon->GetWeaponRecord() ) )
	{
		return false;
	}
	
	// Fail if the primary weapon does not have a 'compliment' weapon.

	HRECORD hComplimentaryWeapon = g_pWeaponDB->GetRecordLink( pWeapon->GetWeaponData(), WDB_WEAPON_rComplimentaryWeapon );
	if ( !hComplimentaryWeapon )
	{
		return false;
	}

	// Fail if the AI has no ammo for the complementary weapon.

	if ( !AIWeaponUtils::HasAmmo( pAI, hComplimentaryWeapon ) )
	{
		return false;
	}

	// Fail if the weapon does not have an AIDB_AIWeaponRecord, as this is 
	// the data source for animation props.

	const AIDB_AIWeaponRecord* pAICurrentPrimaryWeaponRecord = AIWeaponUtils::GetAIWeaponRecord(
		pWeapon->GetWeaponRecord()
		, pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );
	if ( !pAICurrentPrimaryWeaponRecord )
	{
		return false;
	}
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionChangePrimaryWeapon::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionChangePrimaryWeapon::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Get the record for the weapon the AI is switching away from.

	CWeapon* pWeapon = pAI->GetAIWeaponMgr()->GetWeaponOfType(pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType());
	AIASSERT( pWeapon, pAI->GetHOBJECT(), 
		"CAIActionChangePrimaryWeapon::ActivateAction : No primary weapon. This should be validated in the ContextPrecondition test." );
	
	const AIDB_AIWeaponRecord* pAIOldRecord = AIWeaponUtils::GetAIWeaponRecord(
		pWeapon->GetWeaponRecord()
		, pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
	AIASSERT( pAIOldRecord, pAI->GetHOBJECT(), 
		"CAIActionChangePrimaryWeapon::ActivateAction : No old AIDB_AIWeaponRecord.  This should be validated in the ContextPrecondition test." );

	// Get the weapon the AI is switching to.

	EnumAnimProp eNewWeaponProp = g_pAIDB->GetAIConstantsRecord()->eUnarmedWeaponProp;
	HRECORD hComplimentaryWeapon = g_pWeaponDB->GetRecordLink( pWeapon->GetWeaponData(), WDB_WEAPON_rComplimentaryWeapon );
	const AIDB_AIWeaponRecord* pAINewRecord = AIWeaponUtils::GetAIWeaponRecord(
		hComplimentaryWeapon
		, pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
	AIASSERT( pAINewRecord, pAI->GetHOBJECT(), 
		"CAIActionChangePrimaryWeapon::ActivateAction : No new AIDB_AIWeaponRecord.  This should be validated in the ContextPrecondition test." );
	eNewWeaponProp = pAINewRecord->eAIWeaponAnimProp;

	// Set ChangeWeapon animation.  

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon,  eNewWeaponProp);
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Action, pAIOldRecord->eAIChangeWeaponActionAnimProp );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionChangePrimaryWeapon::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionChangePrimaryWeapon::IsActionComplete( CAI* pAI )
{
	// Changing primary weapons is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Changine weapons is not complete.

	return false;
}
