// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollowHide.cpp
//
// PURPOSE : AIGoalFollowHide class implementation
//
// CREATED : 07/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalFollowHide.h"
#include "AI.h"
#include "AITarget.h"
#include "AIWorkingMemory.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFollowHide, kGoal_FollowHide );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowHide::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFollowHide::CAIGoalFollowHide()
{
	m_dwNodeStatus = kNodeStatus_All & ~kNodeStatus_Expired;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFollowHide::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoal
//              
//----------------------------------------------------------------------------

void CAIGoalFollowHide::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalFollowHide::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowHide::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollowHide::CalculateGoalRelevance()
{
	// Bail if we are not aware of an enemy.

	if( !m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		m_fGoalRelevance = 0.f;
		return;	
	}

	// Bail if we have no Follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Default behavior.

	super::CalculateGoalRelevance();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollowHide::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollowHide::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Intentionally do NOT call super::SetWSSatisfaction.
	// This Goal should have no requirements regarding armed/unarmed.

	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_hNodeBest );
}
