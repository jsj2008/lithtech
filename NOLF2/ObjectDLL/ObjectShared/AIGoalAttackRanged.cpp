// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackRanged.cpp
//
// PURPOSE : AIGoalAttackRanged implementation
//
// CREATED : 10/09/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackRanged.h"
#include "AIGoalMgr.h"
#include "AnimatorPlayer.h"
#include "AI.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackRanged, kGoal_AttackRanged);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRanged::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttackRanged::CAIGoalAttackRanged()
{
	m_eWeaponType = kAIWeap_Ranged;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRanged::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRanged::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalAttackRanged::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRanged::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRanged::ActivateGoal()
{
	super::ActivateGoal();
}

