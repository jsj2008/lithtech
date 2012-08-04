// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectAbstract.cpp
//
// PURPOSE : AITargetSelectAbstract abstract class definition
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "AITargetSelectAbstract.h"
#include "AI.h"
#include "AIDB.h"
#include "AIUtils.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIState.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectAbstract::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAITargetSelectAbstract::CAITargetSelectAbstract()
{
}

CAITargetSelectAbstract::~CAITargetSelectAbstract()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectAbstract::InitTargetSelect
//
//	PURPOSE:	Initialize the TargetSelect.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectAbstract::InitTargetSelect( AIDB_TargetSelectRecord* pTargetSelectRecord )
{
	m_pTargetSelectRecord = pTargetSelectRecord;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectAbstract::GetCost
//
//	PURPOSE:	Return the cost of selecting this target.
//
// ----------------------------------------------------------------------- //

float CAITargetSelectAbstract::GetCost()
{
	return m_pTargetSelectRecord->fCost;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectAbstract::Validate
//
//	PURPOSE:	Return true if this target is valid to continue running.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectAbstract::Validate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Something has gone wrong in the state.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Failed ) )
	{
		return false;
	}

	// Continue running.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectAbstract::Activate
//
//	PURPOSE:	Activate target selection.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectAbstract::Activate( CAI* pAI )
{
	// If an awareness is specified on activating this target selector, apply this 
	// awareness.

	if ( pAI && kAware_Invalid != m_pTargetSelectRecord->eAwareness )
	{
		pAI->GetAIBlackBoard()->SetBBAwareness( m_pTargetSelectRecord->eAwareness );
	}
}

