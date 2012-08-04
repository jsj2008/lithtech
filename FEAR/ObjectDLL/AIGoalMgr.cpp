// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMgr.cpp
//
// PURPOSE : AIGoalMgr implementation
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalMgr.h"
#include "AIGoalAbstract.h"
#include "AI.h"
#include "AIState.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AnimationContext.h"
#include "ParsedMsg.h"
#include "AIUtils.h"
#include "AIMovement.h"
#include "AIBrain.h"
#include "iperformancemonitor.h"

#define GOAL_CMD_PREFIX		"GOAL_" 
#define GOAL_CMD_GOALSET	"GoalSet" 
#define GOAL_CMD_ADDGOAL	"AddGoal" 
#define GOAL_CMD_REMOVEGOAL	"RemoveGoal" 
#define GOAL_CMD_GOALSCRIPT	"GoalScript" 

DEFINE_AI_FACTORY_CLASS(CAIGoalMgr);

// Performance monitoring.
CTimedSystem g_tsAIGoalSelection("AIGoalSelection", "AI");

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIGoalMgr::CAIGoalMgr()
{
	m_pAI = NULL;
	Init( NULL );
}

CAIGoalMgr::~CAIGoalMgr()
{
	Term( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::Save/Load
//
//	PURPOSE:	Save/Load
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_COBJECT(m_pAI);

	SAVE_INT(m_lstGoals.size());
	for (std::size_t i = 0; i < m_lstGoals.size(); ++i)
	{
		SAVE_INT(m_lstGoals[i]->GetGoalClassType());
		m_lstGoals[i]->Save(pMsg);
	}

	SAVE_INT(m_pCurGoal ? m_pCurGoal->GetGoalType() : kGoal_InvalidType);

	std::string strGoalSet;
	strGoalSet = g_pAIDB->GetAIGoalSetRecordName( (ENUM_AIGoalSetID)m_iGoalSet );
	SAVE_STDSTRING( strGoalSet );

	SAVE_DOUBLE(m_fGoalSetTime);
}

void CAIGoalMgr::Load(ILTMessage_Read *pMsg)
{
	LOAD_COBJECT(m_pAI, CAI);

	int nGoals = 0;
	LOAD_INT(nGoals);
	for (int i = 0; i < nGoals; ++i)
	{
		EnumAIGoalType eGoalClass;
		LOAD_INT_CAST(eGoalClass, EnumAIGoalType);
		CAIGoalAbstract* pGoal = AI_FACTORY_NEW_Goal(eGoalClass);
		pGoal->Load(pMsg);
		m_lstGoals.push_back(pGoal);
	}

	EnumAIGoalType eCurrentGoalType;
	LOAD_INT_CAST(eCurrentGoalType, EnumAIGoalType);
	m_pCurGoal = FindGoalByType(eCurrentGoalType);

	std::string strGoalSet;
	LOAD_STDSTRING( strGoalSet );
	m_iGoalSet = g_pAIDB->GetAIGoalSetRecordID( strGoalSet.c_str() );

	LOAD_DOUBLE(m_fGoalSetTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::Init
//
//	PURPOSE:	Initialization. 
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::Init(CAI* pAI)
{
	m_pAI = pAI;

	m_pCurGoal			= NULL;

	m_iGoalSet			= (uint32)kAIGoalSetID_Invalid;
	m_fGoalSetTime		= 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::Term
//
//	PURPOSE:	Termination. 
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::Term(bool bDestroyAll)
{
	// Delete all goals.
	// Use RemoveGoal so that all goal removals follow the same 
	// code path.  When SetGoalSet is called, it may call Term()
	// to clear the goals.  Calling RemoveGoal ensures that there
	// are no dangling pointers.

	CAIGoalAbstract* pGoal;
	AIGOAL_LIST::iterator it = m_lstGoals.begin();
	while( it != m_lstGoals.end() )
	{
		pGoal = (*it);
		if( !bDestroyAll && pGoal->IsPermanentGoal() )
		{
			++it;
		}
		else {
			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Removing Goal %s.", s_aszGoalTypes[pGoal->GetGoalType()] ) );
			RemoveGoal( pGoal->GetGoalType() );
			it = m_lstGoals.begin();
		}
	}

	for( it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it )
	{
		pGoal = (*it);
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Goal %s is Permanent. NOT removing.", s_aszGoalTypes[pGoal->GetGoalType()] ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::SetGoalSet()
//
//	PURPOSE:	Set current goal set.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::SetGoalSet(const char* szGoalSet, const char* szName, bool bClearGoals)
{
	// Get the GoalSet from the database.
	uint32 iGoalSet = g_pAIDB->GetAIGoalSetRecordID( szGoalSet );
	AIASSERT( iGoalSet != -1, m_pAI->m_hObject, "CAIGoalMgr::SetGoalSet: Invalid GoalSet name." );

	SetGoalSet( iGoalSet, szName, bClearGoals );
}

void CAIGoalMgr::SetGoalSet(uint32 iGoalSet, const char* szName, bool bClearGoals)
{
	if( iGoalSet == -1 )
	{
		return;
	}
	
	AIDB_GoalSetRecord* pGoalSet = g_pAIDB->GetAIGoalSetRecord( iGoalSet );

	// Check if GoalSet requires a specific AIType.

	if( !pGoalSet->lstRequiredAITypes.empty() )
	{
		if( m_pAI->GetAIAttributes() )
		{
			ENUM_AIAttributesID eAttributesID = m_pAI->GetAIAttributes()->eAIAttributesID;

			AIATTRIBUTES_LIST::iterator it_AIType;
			for( it_AIType = pGoalSet->lstRequiredAITypes.begin(); it_AIType != pGoalSet->lstRequiredAITypes.end(); ++it_AIType )
			{
				// Found a matching AIType.

				if( eAttributesID == *it_AIType )
				{
					break;
				}
			}

			// No matching AIType was found. Bail!

			if( it_AIType == pGoalSet->lstRequiredAITypes.end() )
			{
				AIDB_AttributesRecord* pRecord = g_pAIDB->GetAIAttributesRecord( eAttributesID );
				if( pRecord )
				{
					AIASSERT3( 0, m_pAI->m_hObject, "%s : GoalSet '%s' invalid for AIType '%s'.", szName, pGoalSet->strName.c_str(), pRecord->strName.c_str() );
				}
				else {
					AIASSERT2( 0, m_pAI->m_hObject, "%s : GoalSet '%s' invalid for AIType.", szName, pGoalSet->strName.c_str() );
				}
				return;
			}
		}
	}

	// Delete existing goals if flag is set.

	if( bClearGoals )
	{
		Term( false );
		Init( m_pAI );
		
		// Allow transitions to finish.  Stomp anything else.

		if( !m_pAI->GetAnimationContext()->IsTransitioning() )
		{
			m_pAI->GetAnimationContext()->ClearSpecial();
		}

		// Clear memory of characters.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Character );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		// Clear memory of disturbances.

		factQuery.SetFactType( kFact_Disturbance );
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

		// Clear anything we are targeting.

		m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
	}

	m_iGoalSet = iGoalSet;
	m_fGoalSetTime = g_pLTServer->GetTime();

	// Create goals for every entry in map.

	CAIGoalAbstract* pGoal;
	double fTime = g_pLTServer->GetTime();
	AIGOAL_TYPE_LIST::iterator itGoal;
	for( itGoal = pGoalSet->lstGoalSet.begin(); itGoal != pGoalSet->lstGoalSet.end(); ++itGoal )
	{
		pGoal = AddGoal( *itGoal, fTime );

//		pGoal->SetPermanentGoal( bPermanent );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::AI_FACTORY_NEW_Goal
//
//	PURPOSE:	Create a goal
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract* CAIGoalMgr::AI_FACTORY_NEW_Goal(EnumAIGoalType eGoalType)
{
	// Call AI_FACTORY_NEW for the requested type of goal.

	switch( eGoalType )
	{
		#define GOAL_TYPE_AS_SWITCH 1
		#include "AIGoalTypeEnums.h"
		#undef GOAL_TYPE_AS_SWITCH

		default: AIASSERT( 0, m_pAI->m_hObject, "CAIGoalMgr::CreateFactoryGoal: Unrecognized goal type.");
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::AddGoal
//
//	PURPOSE:	Add a goal
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract* CAIGoalMgr::AddGoal(EnumAIGoalType eGoalType, double fTime)
{
	CAIGoalAbstract* pGoal = NULL;
	AIDB_GoalRecord* pRecord;

	ASSERT(eGoalType != kGoal_InvalidType && eGoalType < kGoal_Count);
	if( eGoalType != kGoal_InvalidType && eGoalType < kGoal_Count)
	{
		// Check if AI already has this goal.
		AIGOAL_LIST::iterator it;
		for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
		{
			pGoal = *it;

			// If AI has the goal, return it.
			if(pGoal->GetGoalType() == eGoalType)
			{
				break;
			}
		}

		if( it == m_lstGoals.end() )
		{
			pRecord = g_pAIDB->GetAIGoalRecord( eGoalType );
			if( pRecord && ( pRecord->eGoalClass != kGoal_InvalidType ) )
			{
				// Add goal to list of goals.
				pGoal = NULL;
				pGoal = AI_FACTORY_NEW_Goal( pRecord->eGoalClass );
				if( pGoal == NULL )
				{
					AIASSERT( 0, m_pAI->m_hObject, "CAIGoalMgr::AddGoal: Failed to create goal.");
					return NULL;
				}

				m_lstGoals.push_back(pGoal);


				// Initialize goal.

				pGoal->InitGoal( m_pAI, eGoalType, pRecord );
			}
			else {
				AIASSERT1( 0, m_pAI->m_hObject, "CAIGoalMgr::AddGoal: Unable to create goal '%s'", s_aszGoalTypes[eGoalType] );
			}
		}
	}

	return pGoal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::RemoveGoal
//
//	PURPOSE:	Remove a goal
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::RemoveGoal(EnumAIGoalType eGoalType)
{
	ASSERT(eGoalType != kGoal_InvalidType);

	// Find goal of specified type.
	CAIGoalAbstract* pGoal;
	AIGOAL_LIST::iterator it;
	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = *it;

		if(pGoal->GetGoalType() == eGoalType)
		{
			if(m_pCurGoal == pGoal)
			{
				m_pCurGoal =  NULL;
			}

			// Delete the goal.
			pGoal->TermGoal();
			AI_FACTORY_DELETE(pGoal);
			m_lstGoals.erase(it);

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::FindGoalByType
//
//	PURPOSE:	Find a goal of some type.
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract* CAIGoalMgr::FindGoalByType(EnumAIGoalType eGoalType)
{
	AIGOAL_LIST::iterator it;
	CAIGoalAbstract* pGoal;
	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = (*it);
		if( pGoal->GetGoalType() == eGoalType )
		{
			return pGoal;
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::UpdateGoalRelevances
//
//	PURPOSE:	Select most relevant goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::UpdateGoalRelevances( bool bReplan )
{
	bool bRequiresHigherInterruptPriority = false;
	bool bGoalChanged = false;

	if( m_pAI->GetAnimationContext()->IsTransitioning() )
	{
		bRequiresHigherInterruptPriority = true;
	}

	// Some actions may be interrupted.

	if( (m_pCurGoal ) 
		&& ( m_pAI->GetAnimationContext()->IsLocked() ) 
		&& ( !m_pCurGoal->IsPlanInterruptible() ) )
	{
		bRequiresHigherInterruptPriority = true;
	}

	CAIGoalAbstract* pGoalMax = FindMostRelevantGoal( true );

	// Set the new current goal, which is the
	// goal with the highest relevance that can
	// formulate a valid plan of actions.

	while( pGoalMax )
	{
		// If the pGoalMax has the same relevance as the current goal, prefer the 
		// current to avoid abandoning a goal prematurely when there isn't anything
		// better to do.

		if ( (m_pCurGoal && pGoalMax) &&
			(m_pCurGoal != pGoalMax) &&
			(m_pCurGoal->GetGoalRelevance() == pGoalMax->GetGoalRelevance()))
		{
			pGoalMax = m_pCurGoal;
		}

		// Goal has not changed.
		// No planning necessary.

		if( ( pGoalMax == m_pCurGoal ) && 
			( !bReplan ) &&
			( !m_pCurGoal->ReplanRequired() ) )
		{
			break;
		}

		// Do not reactivate the same goal while transitioning.

		if( ( pGoalMax == m_pCurGoal ) 
			&& ( m_pAI->GetAnimationContext()->IsTransitioning() ) 
			&& ( !m_pCurGoal->CanReactivateDuringTransitions() ) )
		{
			break;
		}

		// Goal must interrupt to be valid (ie damage response which must
		// be played).

		if ( bRequiresHigherInterruptPriority
			&& pGoalMax && m_pCurGoal
			&& ( pGoalMax != m_pCurGoal )
			&& ( pGoalMax->GetInterruptPriority() <= m_pCurGoal->GetInterruptPriority() ) )
		{
			pGoalMax->ClearGoalRelevance();
		}

		// Failed to meet random probability of activation.

		else if( ( pGoalMax->GetActivateChance() < 1.f ) &&
			( pGoalMax->GetActivateChance() < GetRandom(0.0f, 1.0f) ) )
		{
			pGoalMax->ClearGoalRelevance();
		}

		// Activate the goal if we formulated
		// a valid plan.

		else if( pGoalMax->BuildPlan() )
		{
			if( m_pCurGoal )
			{
				m_pCurGoal->DeactivateGoal();
				AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Deactivating Goal: %s", s_aszGoalTypes[m_pCurGoal->GetGoalType()] ) );
			}

			m_pCurGoal = pGoalMax;
			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Activating Goal: %s (%.2f)", s_aszGoalTypes[m_pCurGoal->GetGoalType()], m_pCurGoal->GetGoalRelevance() ) );
			m_pCurGoal->ActivateGoal();
			m_pCurGoal->ActivatePlan();
			bGoalChanged = true;
			break;
		}

		// Could not build a valid plan.

		else {
			pGoalMax->HandleBuildPlanFailure();
		}

		pGoalMax = FindMostRelevantGoal( false );
	}

	// No max goal was found, so the AI has no current goal or state.

	if( bReplan && !pGoalMax 
		&& !bRequiresHigherInterruptPriority
		&& !m_pAI->GetAnimationContext()->IsTransitioning() )
	{
		m_pCurGoal = NULL;
		m_pAI->ClearState();
	}

	// Clear the SelectAction flag from the blackboard.
	// If the Goal required a interrupt, and the Goal did not change,
	// then do not clear the SelectAction flag.

	if( bRequiresHigherInterruptPriority && !bGoalChanged )
	{
		return;
	}
	else {
		m_pAI->GetAIBlackBoard()->SetBBSelectAction( false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::FindMostRelevantGoal
//
//	PURPOSE:	Return the most relevant goal.
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract* CAIGoalMgr::FindMostRelevantGoal( bool bRecalculate )
{
	float fRelevance;
	float fMaxRelevance = 0.f;

	double fNextRecalcTime;

	CAIGoalAbstract* pGoal;
	CAIGoalAbstract* pGoalMax = NULL;
	
	AIGOAL_LIST::iterator itGoal;
	for( itGoal = m_lstGoals.begin(); itGoal != m_lstGoals.end(); ++itGoal )
	{
		pGoal = *itGoal;

		// Recalculate the current relevance.

		if( bRecalculate )
		{
			// Do not re-evaluate this goal if it is not time yet.

			fNextRecalcTime = pGoal->GetNextRecalcTime();
			if( ( pGoal != m_pCurGoal ) &&
				( fNextRecalcTime > 0.f ) &&
				( fNextRecalcTime > g_pLTServer->GetTime() ) )
			{
				pGoal->ClearGoalRelevance();
			}

			// Do not re-evaluate this goal if it is currently satisfied.

			else if( ( !pGoal->GetReEvalOnSatisfaction() ) &&
				pGoal->IsWSSatisfied( m_pAI->GetAIWorldState() ) )
			{
				pGoal->ClearGoalRelevance();
			}

			// Do not re-evaluate if the goals awareness tests are not met.
			// This would ideally be part of the goals CalculateGoalRelevance,
			// but was not safe to add in to a base class test.

			else if ( !pGoal->IsAwarenessValid() )
			{
				pGoal->ClearGoalRelevance();
			}

			// Goal is unsatisfied, or should always be re-evaluated.

			else {
				pGoal->CalculateGoalRelevance();

				if( fNextRecalcTime > 0.f )
				{
					pGoal->SetNextRecalcTime();
				}
			}
		}

		fRelevance = pGoal->GetGoalRelevance();

		// Select a new max.

		if( fRelevance > fMaxRelevance )
		{
			fMaxRelevance = fRelevance;
			pGoalMax = pGoal;
		}
	}

	return pGoalMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::SelectRelevantGoal
//
//	PURPOSE:	Update goals.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::SelectRelevantGoal()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAIGoalSelection);

	//
	// Determine if the goals should be updated, and if so, if replanning is 
	// required.
	//

	bool bUpdateGoals = false;
	bool bForceReplanning = false;

	if ( m_pCurGoal )
	{
		// Test for an invalid plan.

		if ( !m_pCurGoal->IsPlanValid() )
		{
			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Plan is invalid for Goal: %s.", s_aszGoalTypes[m_pCurGoal->GetGoalType()] ) );
			bUpdateGoals = true;
			bForceReplanning = true;
		}

		// Test for a satisfied plan.

		else if ( m_pCurGoal->IsWSSatisfied( m_pAI->GetAIWorldState() ) )
		{
			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Goal is satisfied: %s.", s_aszGoalTypes[m_pCurGoal->GetGoalType()] ) );
			bUpdateGoals = true;
			bForceReplanning = true;
		}
	}

	//
	// Test for a request to reselect actions.  
	//

	if ( m_pAI->GetAIBlackBoard()->GetBBSelectAction() )
	{
		bUpdateGoals = true;
	}

	//
	// Plan has been invalidated, so immediately force a replan.
	//

	if ( m_pAI->GetAIBlackBoard()->GetBBInvalidatePlan() )
	{
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Honoring forced replanning request." ) );
		bUpdateGoals = true;
		bForceReplanning = true;
	}

	//
	// Perform the actual goal updating/replanning if requested
	//

	if ( bUpdateGoals )
	{
		UpdateGoalRelevances( bForceReplanning );
	}

	// Clear the forced replanning flag.  
	// Clear this AFTER updating goal relevances. Some
	// legacy code depends on the state of the InvalidatePlan flag.
	// (e.g. AINavMeshLinkCrawl).

	m_pAI->GetAIBlackBoard()->SetBBInvalidatePlan( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::UpdateGoal
//
//	PURPOSE:	Update current goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::UpdateGoal()
{
	if( m_pCurGoal )
	{
		if( !m_pCurGoal->UpdateGoal() )
		{
			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
			m_pCurGoal->ClearPlan();
		}

		if( m_pAI->GetAIBlackBoard()->GetBBTaskStatus() == kTaskStatus_Set )
		{
			m_pCurGoal->UpdateTaskStatus();
		}
	}
}
