// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollow.cpp
//
// PURPOSE : AIGoalFollow class implementation
//
// CREATED : 4/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalFollow.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFollow, kGoal_Follow );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SAI_FOLLOW_REQUEST::Constructor
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SAI_FOLLOW_REQUEST::SAI_FOLLOW_REQUEST()
{
	eFactID = kFactID_Invalid;
	hFollow = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalFollow::CAIGoalFollow()
{
}

CAIGoalFollow::~CAIGoalFollow()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalFollow::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalFollow
//              
//----------------------------------------------------------------------------

void CAIGoalFollow::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_FollowCurrent.eFactID );
	SAVE_HOBJECT( m_FollowCurrent.hFollow );

	SAVE_DWORD( m_FollowPending.eFactID );
	SAVE_HOBJECT( m_FollowPending.hFollow );
}

void CAIGoalFollow::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST( m_FollowCurrent.eFactID, ENUM_FactID );
	LOAD_HOBJECT( m_FollowCurrent.hFollow );

	LOAD_DWORD_CAST( m_FollowPending.eFactID, ENUM_FactID );
	LOAD_HOBJECT( m_FollowPending.hFollow );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::CalculateGoalRelevance()
{
	// Bail if we are targeting something other than the leader.

	if( !( m_pAI->HasTarget( kTarget_Leader ) ||
		   m_pAI->HasTarget( kTarget_Interest ) ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Bail if we have no Follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) )
	{
		// New follow requested.

		if( m_FollowCurrent.eFactID != pFact->GetFactID() )
		{
			m_FollowPending.hFollow = pFact->GetTargetObject();
			m_FollowPending.eFactID = pFact->GetFactID();
		}

		// Bail if we have no path to our leader.

		ENUM_NMPolyID ePolyLeader =  m_pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly();
		if( !g_pAIPathMgrNavMesh->HasPath( m_pAI, m_pAI->GetCharTypeMask(), ePolyLeader ) )
		{
			m_fGoalRelevance = 0.f;
			return;
		}

		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// There is no Follow task.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::ReplanRequired
//
//	PURPOSE:	Return true if AI needs to make a new plan.
//
// ----------------------------------------------------------------------- //

bool CAIGoalFollow::ReplanRequired()
{
	// Replan if the follow target has changed.

	if(	m_FollowPending.hFollow != m_FollowCurrent.hFollow )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::ActivateGoal()
{
	super::ActivateGoal();

	m_FollowCurrent = m_FollowPending;

	m_pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Set );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::DeactivateGoal()
{
	super::DeactivateGoal();

	m_FollowCurrent.eFactID = kFactID_Invalid;
	m_FollowCurrent.hFollow = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalFollow::SetWSSatisfaction( CAIWorldState& WorldState )
{
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalFollow::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalFollow::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = NULL;

	pProp = pwsWorldState->GetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject );
	if( !pProp || ( pProp && !pProp->bWSValue ) )
	{
		return false;
	}

	return true;
}

