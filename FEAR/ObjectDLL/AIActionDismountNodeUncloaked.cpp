// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountNodeUncloaked.cpp
//
// PURPOSE : AIActionDismountNodeUncloaked class implementation
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDismountNodeUncloaked.h"
#include "AI.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountNodeUncloaked, kAct_DismountNodeUncloaked );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNodeUncloaked::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDismountNodeUncloaked::CAIActionDismountNodeUncloaked()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNodeUncloaked::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountNodeUncloaked::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction(pAI, wsWorldStateGoal);

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Find an existing memory for the desire to uncloak, or create one.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Uncloak);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		pFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
	}

	// Set the current desire.

	if( pFact )
	{
		pFact->SetDesireType( kDesire_Uncloak, 1.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountNodeUncloaked::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountNodeUncloaked::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Remove the desire to uncloak.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Uncloak);
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}
