// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChangePrimaryWeapon.cpp
//
// PURPOSE : This goal handles prompting the AI to change weapons when the 
//			 AIs' primary weapon has no ammo.  For instance, when an AI 
//			 runs out of shells for his shotgun, he should flip it over 
//			 and whack people with it.  This goal prompts the change from
//			 ranged to melee, in this case.
//
// CREATED : 6/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalChangePrimaryWeapon.h"
#include "AI.h"
#include "AIWorldState.h"
#include "AIBlackBoard.h"
#include "AIWeaponUtils.h"

LINKFROM_MODULE(AIGoalChangePrimaryWeapon);


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalChangePrimaryWeapon, kGoal_ChangePrimaryWeapon );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChangePrimaryWeapon::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalChangePrimaryWeapon::CAIGoalChangePrimaryWeapon()
{
}

CAIGoalChangePrimaryWeapon::~CAIGoalChangePrimaryWeapon()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalChangePrimaryWeapon::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalChangePrimaryWeapon
//              
//----------------------------------------------------------------------------

void CAIGoalChangePrimaryWeapon::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAIGoalChangePrimaryWeapon::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalChangePrimaryWeapon::CalculateGoalRelevance
//              
//	PURPOSE:	This goal is relevant when the AIs' primary weapons' clip is
//				empty and he has no ammo for his primary weapon.
//              
//----------------------------------------------------------------------------

void CAIGoalChangePrimaryWeapon::CalculateGoalRelevance()
{
	m_fGoalRelevance = 0.f;

	// AIs' clip is not empty.

	if ( IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		return;
	}

	// AI has ammo for his primary weapon.

	if ( AIWeaponUtils::HasAmmo(m_pAI, m_pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType(), AIWEAP_CHECK_HOLSTER ) )
	{
		return;
	}

	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalChangePrimaryWeapon::SetWSSatisfaction
//              
//	PURPOSE:	To satisfy this goal, a weapon must be loaded.
//              
//----------------------------------------------------------------------------

void CAIGoalChangePrimaryWeapon::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Weapon must be loaded.

	WorldState.SetWSProp(kWSK_WeaponLoaded, m_pAI->GetHOBJECT(), kWST_bool, true);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalChangePrimaryWeapon::IsWSSatisfied
//              
//	PURPOSE:	This goal is satisfied when the weapon is loaded.
//              
//----------------------------------------------------------------------------

bool CAIGoalChangePrimaryWeapon::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Sanity Check.

	if ( !pwsWorldState )
	{
		return false;
	}

	// Weapon is not yet loaded
	// TODO: What if there is no alternate weapon?  Is kWeap_Hands loadable?
	// Should the ChangePrimaryWeapon action handle this case?  This would allow
	// other actions to be implemented in the future without being encumbered by
	// this particular solution.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp(kWSK_WeaponLoaded, m_pAI->GetHOBJECT());

	if ( !pProp || !pProp->bWSValue )
	{
		return false;
	}

	// Goal is satisfied!

	return true;
}
