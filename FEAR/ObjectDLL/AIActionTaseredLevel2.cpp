// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTaserLevel2.cpp
//
// PURPOSE : 
//
// CREATED : 2/16/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionTaseredLevel2.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTaseredLevel2, kAct_TaseredLevel2 );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredLevel2::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionTaseredLevel2::CAIActionTaseredLevel2()
{
	SetFourQuadrantDirectionalRecoils( true );
}

CAIActionTaseredLevel2::~CAIActionTaseredLevel2()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredLevel2::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionTaseredLevel2::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// Remove response to damage, as this action is triggered by a different 
	// action which responds to the damage.
	// Add 'reacted to taser stunned'

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Taser2Stunned );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredLevel2::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionTaseredLevel2::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

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

	return true;
}
