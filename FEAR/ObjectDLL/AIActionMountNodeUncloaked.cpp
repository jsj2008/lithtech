// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMountNodeUncloaked.cpp
//
// PURPOSE : AIActionMountNodeUncloaked class implementation
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionMountNodeUncloaked.h"
#include "AI.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMountNodeUncloaked, kAct_MountNodeUncloaked );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountNodeUncloaked::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionMountNodeUncloaked::CAIActionMountNodeUncloaked()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountNodeUncloaked::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionMountNodeUncloaked::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
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
//	ROUTINE:	CAIActionMountNodeUncloaked::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionMountNodeUncloaked::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Remove the desire to uncloak.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Desire);
	factQuery.SetDesireType(kDesire_Uncloak);
	pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}

