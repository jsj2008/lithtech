// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromVehicle.cpp
//
// PURPOSE : AIActionAttackFromVehicle abstract class implementation
//
// CREATED : 1/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackFromVehicle.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromVehicle, kAct_AttackFromVehicle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicle::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackFromVehicle::CAIActionAttackFromVehicle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicle::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromVehicle::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Remove super classes' precondition.
	// Weapon does not need to be loaded -- AI will 
	// automatically reload while suppressing.

	m_wsWorldStatePreconditions.ClearWSProp( kWSK_WeaponLoaded, NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicle::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromVehicle::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Intentionally do not call super::ValidateContextPreconditions(). 
	// Firing from a vehicle ignores range.

	// AI does not have a weapon of the correct type

	if( !AIWeaponUtils::HasWeaponType( pAI, GetWeaponType(), bIsPlanning ) )
	{
		return false;
	}

	// AI does not have any ammo for this weapon.

	if ( !AIWeaponUtils::HasAmmo( pAI, GetWeaponType(), bIsPlanning ) )
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

	// Action is valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromVehicle::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Automatically reload behind the scenes while riding vehicle.

	pAI->GetAIBlackBoard()->SetBBAutoReload( true );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_Arm );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicle::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackFromVehicle::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	// Bail is no vehicle animProp.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( !pProp )
	{
		return;
	}
	EnumAnimProp eVehicle = pProp->eAnimPropWSValue;

	// Default behavior.

	super::SetAttackAnimProps( pAI, pProps );

	// Set the additional anim props for attacking from a vehicle.

	EnumAnimProp eDir = SelectAttackPose( pAI );
	pProps->Set( kAPG_MovementDir, eDir );
	pProps->Set( kAPG_Activity, eVehicle );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicle::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

EnumAnimProp CAIActionAttackFromVehicle::SelectAttackPose( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return kAP_Invalid;
	}

	// Determine the correct pose based on the direction to the target.

	LTVector vDir = pAI->GetAIBlackBoard()->GetBBTargetPosition() - pAI->GetPosition();
	vDir.Normalize();

	float fDot = pAI->GetForwardVector().Dot( vDir );

	// Convert angle to represent the degree on a 360 circle.

	float fAngle = acos( fDot );
	float fDotRight = pAI->GetRightVector().Dot( vDir );
	if( fDotRight > 0.f )
	{
		fAngle = MATH_DEGREES_TO_RADIANS( 360.0f ) - fAngle;
	}

	// Rotate the cutoffs into a space where the forward
	// cutoff is at zero degrees.

	float fCutOffs[4];
	enum { kForward, kLeft, kBackward, kRight, };

	float fRotate = g_pAIDB->GetAIConstantsRecord()->fCycleCutOffForward;
	fCutOffs[kForward] = 0.f;
	fCutOffs[kLeft] = g_pAIDB->GetAIConstantsRecord()->fCycleCutOffLeft - fRotate;
	fCutOffs[kBackward] = g_pAIDB->GetAIConstantsRecord()->fCycleCutOffBackward - fRotate;
	fCutOffs[kRight] = g_pAIDB->GetAIConstantsRecord()->fCycleCutOffRight - fRotate;

	// Bound the cutoffs to be between 0 and 360.

	for( uint32 iCutOff = kLeft; iCutOff <= kRight; ++iCutOff )
	{
		if( fCutOffs[iCutOff] < 0.f )
		{
			fCutOffs[iCutOff] += MATH_DEGREES_TO_RADIANS( 360.0f );
		}
	}

	// Rotate the angle into the space of the circle where
	// the forward cutoff is at zero.

	fAngle -= fRotate;
	if( fAngle < 0.f )
	{
		fAngle += MATH_DEGREES_TO_RADIANS( 360.0f );
	}

	// Find the corresponding animation to the angle.

	if( fAngle < fCutOffs[kLeft] )
	{
		return kAP_MDIR_Left;
	}

	if( fAngle < fCutOffs[kBackward] )
	{
		return kAP_MDIR_Backward;
	}

	if( fAngle < fCutOffs[kRight] )
	{
		return kAP_MDIR_Right;
	}

	return kAP_MDIR_Forward;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackFromVehicle::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackFromVehicle::ValidateAction( CAI* pAI )
{
	// Intentionally do not call super::ValidateAction(). 
	// Firing from a vehicle ignores range and never reloads.

	if( !CAIActionAbstract::ValidateAction( pAI ) )
	{
		return false;
	}

	// Invalidate the Action if the AI is not in the 
	// correct pose.

	EnumAnimProp eCurrent = pAI->GetAnimationContext()->GetCurrentProp( kAPG_MovementDir );
	if( eCurrent != SelectAttackPose( pAI ) )
	{
		return false;
	}

	// Action is valid.

	return true;
}

