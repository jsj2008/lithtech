// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTraverseLinkUncloaked.cpp
//
// PURPOSE : AIActionTraverseLinkUncloaked class implementation
//
// CREATED : 04/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionTraverseLinkUncloaked.h"
#include "AI.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTraverseLinkUncloaked, kAct_TraverseLinkUncloaked );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLinkUncloaked::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionTraverseLinkUncloaked::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

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
//	ROUTINE:	CAIActionTraverseLinkUncloaked::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionTraverseLinkUncloaked::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Remove the desire to uncloak.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Uncloak);
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}

