// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeRoll.cpp
//
// PURPOSE : AIActionDodgeRoll class implementation
//
// CREATED : 3/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDodgeRoll.h"
#include "AI.h"
#include "AIBrain.h"
#include "AIBlackBoard.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeRoll, kAct_DodgeRoll );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeRoll::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeRoll::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Do not dodge into a crouch if we have not been standing 
	// for long enough.

	if( pAI->GetAnimationContext()->GetCurrentProp( kAPG_Posture ) != kAP_POS_Stand )
	{
		return false;
	}

	if( g_pLTServer->GetTime() - pAI->GetAIBlackBoard()->GetBBPostureChangeTime() < pAI->GetBrain()->GetAttackPoseCrouchTime() )
	{
		return false;
	}

	return super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeRoll::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeRoll::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeRoll::GetDodgeDist
//
//	PURPOSE:	Return the distance the dodge animation covers.
//
// ----------------------------------------------------------------------- //

float CAIActionDodgeRoll::GetDodgeDist( CAI* pAI )
{
	return pAI->GetBrain()->GetDodgeVectorRollDist();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeRoll::GetDodgeAnim
//
//	PURPOSE:	Return the dodge animation prop.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeRoll::SetDodgeAnim( CAI* pAI, float fDir, CAnimationProps& animProps )
{
	animProps.Set( kAPG_Posture, kAP_POS_Crouch );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Action, kAP_ACT_Roll );

	if( fDir < 0.f )
	{
		animProps.Set( kAPG_MovementDir, kAP_MDIR_Left );
	}
	else {
		animProps.Set( kAPG_MovementDir, kAP_MDIR_Right );
	}
}

