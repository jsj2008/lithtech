// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTaseredLevel1.cpp
//
// PURPOSE : 
//
// CREATED : 2/16/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionTaseredLevel1.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTaseredLevel1, kAct_TaseredLevel1 );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredLevel1::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionTaseredLevel1::CAIActionTaseredLevel1()
{
	SetFourQuadrantDirectionalRecoils( true );

	// Don't apply the context effect; if the AI is tasered again while 
	// recovering, he won't play the this action as it will already be 
	// satisfied.

	SetApplyContextEffect( false );
}

CAIActionTaseredLevel1::~CAIActionTaseredLevel1()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredLevel1::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionTaseredLevel1::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// Remove response to damage, as this action is triggered by a different 
	// action which responds to the damage.
	// Add 'reacted to taser stunned'

	m_wsWorldStateEffects.ClearWSProp( kWSK_ReactedToWorldStateEvent, NULL );
	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Taser1Stunned );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTaseredLevel1::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionTaseredLevel1::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Perform the cheap tests first, as the base class does an animation 
	// tree search.  This action is only valid in the rare case that the ai
	// was hit by a taser.

	// Fail if there is no damage fact.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pFact )
	{
		return false;
	}

	// Fail if the damage type is not taser1.

	DamageType eDamageType;
	pFact->GetDamage( &eDamageType, NULL, NULL );

	if ( DT_TASER != eDamageType )
	{
		return false;
	}

	// Verify the base class is valid.

	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	return true;
}
