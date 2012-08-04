// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDefeatedTaserRecoil.cpp
//
// PURPOSE : 
//
// CREATED : 9/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDefeatedTaserRecoil.h"
#include "AI.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDefeatedTaserRecoil, kAct_DefeatedTaserRecoil );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedTaserRecoil::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDefeatedTaserRecoil::CAIActionDefeatedTaserRecoil()
{
}

CAIActionDefeatedTaserRecoil::~CAIActionDefeatedTaserRecoil()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedTaserRecoil::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDefeatedTaserRecoil::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// AI must already have reacted to being stunned by a taser before he can 
	// recover from the stun.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Taser2Stunned );

	// Set effects.
	// AI reacted to damage.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Damage );
}
 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedTaserRecoil::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDefeatedTaserRecoil::ActivateAction(CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Clear the facedir set in the base class, and turn off rotation to 
	// face all together.

	pAI->GetAIBlackBoard()->SetBBFaceType( kFaceType_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );

	// AI is now a one hit kill.

	pAI->GetAIBlackBoard()->SetBBInstantDeath( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedTaserRecoil::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDefeatedTaserRecoil::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Constrain the times this action to use only when it directly satisfies the goal.

	SAIWORLDSTATE_PROP* pEffectProp = m_wsWorldStateEffects.GetWSProp( kWSK_ReactedToWorldStateEvent, pAI->GetHOBJECT() );
	SAIWORLDSTATE_PROP* pGoalProp = wsWorldStateGoal.GetWSProp( kWSK_ReactedToWorldStateEvent, pAI->GetHOBJECT() );
	if ( !pEffectProp 
		|| !pGoalProp
		|| ( pEffectProp->eWSType != pGoalProp->eWSType )
		|| ( pEffectProp->nWSValue != pGoalProp->nWSValue ) )
	{
		return false;
	}

	// Fail if there is no damage fact.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pFact )
	{
		return false;
	}

	// Fail if the damage type is not taser2.

	DamageType eDamageType;
	pFact->GetDamage( &eDamageType, NULL, NULL );

	if ( DT_TASER_UPGRADE != eDamageType )
	{
		return false;
	}

	// Normally we do the base class validation first.  By performing it
	//  second, we can do cheap tests first, as the base class does animation
	// tree searches.

	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	return true;
}
