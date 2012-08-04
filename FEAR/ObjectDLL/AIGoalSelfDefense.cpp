// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSelfDefense.cpp
//
// PURPOSE : AIGoalSelfDefense class implementation
//
// CREATED  04/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalSelfDefense.h"
#include "AIWeaponUtils.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSelfDefense, kGoal_SelfDefense );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSelfDefense::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSelfDefense::CAIGoalSelfDefense()
{
}

CAIGoalSelfDefense::~CAIGoalSelfDefense()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSelfDefense::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalSelfDefense::CalculateGoalRelevance()
{
	// No relevance if not in melee range.

	if( !AIWeaponUtils::IsInRange( m_pAI, kAIWeaponType_Melee, !AIWEAP_CHECK_HOLSTER ) &&
		!AIWeaponUtils::IsInRange( m_pAI, kAIWeaponType_Kick, !AIWEAP_CHECK_HOLSTER ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Default handling from KillEnemy.

	super::CalculateGoalRelevance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSelfDefense::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSelfDefense::ActivateGoal()
{
	super::ActivateGoal();

	// Clear any outstanding blitz tasks.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_BlitzCharacter );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}
