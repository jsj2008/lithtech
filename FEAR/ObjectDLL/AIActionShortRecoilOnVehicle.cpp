// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionShortRecoilOnVehicle.cpp
//
// PURPOSE : 
//
// CREATED : 3/22/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionShortRecoilOnVehicle.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionShortRecoilOnVehicle, kAct_ShortRecoilOnVehicle );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoilOnVehicle::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionShortRecoilOnVehicle::CAIActionShortRecoilOnVehicle()
{
}

CAIActionShortRecoilOnVehicle::~CAIActionShortRecoilOnVehicle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoilOnVehicle::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionShortRecoilOnVehicle::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Action is only valid if AI is riding a Vehicle.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( ( !pProp ) ||
		( pProp->eAnimPropWSValue == kAP_None ) )
	{
		return false;
	}

	// Perform the expensive animation tree search last.

	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShortRecoilOnVehicle::GetRecoilProps
//
//	PURPOSE:	Returns by parameter the animation props for a recoil
//				animation, given an AI and a fact.  Returns true if the
//				props are set up, false if they were not.
//
// ----------------------------------------------------------------------- //

bool CAIActionShortRecoilOnVehicle::GetRecoilProps( CAI* pAI, CAIWMFact* pFact, CAnimationProps& outAnimProps )
{
	super::GetRecoilProps( pAI, pFact, outAnimProps );

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_RidingVehicle, pAI->m_hObject );
	if( ( !pProp ) ||
		( pProp->eAnimPropWSValue == kAP_None ) )
	{
		return false;
	}

	outAnimProps.Set( kAPG_Activity, pProp->eAnimPropWSValue );
	return true;
}

