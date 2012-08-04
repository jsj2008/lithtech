// ----------------------------------------------------------------------- //
//
// MODULE  : CAIPlanner.cpp
//
// PURPOSE : CAIPlanner class implementation
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIPlanner.h"

#include "AI.h"
#include "AIDB.h"
#include "AIAssert.h"
#include "AIBlackBoard.h"
#include "AIGoalAbstract.h"
#include "AIActionMgr.h"
#include "Weapon.h"
#include "AINodeTypes.h"
#include "AIUtils.h"
#include "iperformancemonitor.h"


// Globals / Statics

CAIPlanner* g_pAIPlanner = NULL;

// Performance monitoring.
///CTimedSystem g_tsAIPlanner("AIPlanner", "AI");

DEFINE_AI_FACTORY_CLASS( CAIPlanStep );
DEFINE_AI_FACTORY_CLASS( CAIPlan );


// ----------------------------------------------------------------------- //

//
// CAIPlan
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlan::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIPlan::CAIPlan()
{
	m_pAI = NULL;
	m_iPlanStep = 0;
	m_fPlanActivationTime = 0.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPlan::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIPlan
//              
//----------------------------------------------------------------------------
void CAIPlan::Save(ILTMessage_Write *pMsg)
{
	int nSteps = m_lstAIPlanSteps.size();
	SAVE_INT(nSteps);
	
	for (int i = 0; i < nSteps; ++i)
	{
		m_lstAIPlanSteps[i]->wsWorldState.Save(pMsg);
		SAVE_INT(m_lstAIPlanSteps[i]->eAIAction);
	}
	
	SAVE_INT(m_iPlanStep);
	SAVE_COBJECT(m_pAI);
	SAVE_TIME(m_fPlanActivationTime);
}

void CAIPlan::Load(ILTMessage_Read *pMsg)
{
	int nSteps = 0;
	LOAD_INT(nSteps);
	m_lstAIPlanSteps.reserve(nSteps);
	for (int i = 0; i < nSteps; ++i)
	{
		CAIPlanStep* pPlanStep = AI_FACTORY_NEW( CAIPlanStep );
		pPlanStep->wsWorldState.Load(pMsg);
		LOAD_INT_CAST(pPlanStep->eAIAction, EnumAIActionType);
		m_lstAIPlanSteps.push_back(pPlanStep);
	}
	
	LOAD_INT(m_iPlanStep);
	LOAD_COBJECT(m_pAI, CAI);
	LOAD_TIME(m_fPlanActivationTime);
}


CAIPlan::~CAIPlan()
{
	DeactivatePlan();

	// Delete the plan.

	CAIPlanStep* pPlanStep;
	AIPLAN_STEP_LIST::iterator itPlan;
	for( itPlan = m_lstAIPlanSteps.begin(); itPlan != m_lstAIPlanSteps.end(); ++itPlan )
	{
		pPlanStep = *itPlan;
		AI_FACTORY_DELETE( pPlanStep );
	}

	m_lstAIPlanSteps.resize( 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlan::ActivatePlan
//
//	PURPOSE:	Activate a new plan.
//
// ----------------------------------------------------------------------- //

void CAIPlan::ActivatePlan( CAI* pAI )
{
	if( !pAI )
	{
		return;
	}

	m_pAI = pAI;

	m_fPlanActivationTime = g_pLTServer->GetTime();

	CAIPlanStep* pPlanStep = m_lstAIPlanSteps[0];
	if( pPlanStep )
	{
		CAIActionAbstract* pAction = g_pAIActionMgr->GetAIAction( pPlanStep->eAIAction );
		if( pAction )
		{
			// Action's preconditions are not met. Bail.

			if( !pAction->ValidateContextPreconditions( m_pAI, pPlanStep->wsWorldState, !IS_PLANNING ) )
			{
				AIASSERT2( 0, m_pAI->m_hObject, "Failed to activate plan due to first actions failed context preconditions: %s (%.2f)", s_aszActionTypes[pAction->GetActionRecord()->eActionType], pAction->GetActionRecord()->fActionCost );
				m_pAI->GetAIBlackBoard()->SetBBInvalidatePlan( true );
				return;
			}

			AITRACE( AIShowActions, ( m_pAI->m_hObject, "Activating Action: %s (%.2f)", s_aszActionTypes[pAction->GetActionRecord()->eActionType], pAction->GetActionRecord()->fActionCost ) );
			pAction->ActivateAction( m_pAI, pPlanStep->wsWorldState );

			// If action was immediately complete, advance to another action.

			if( pAction->IsActionComplete( m_pAI ) )
			{
				AdvancePlan();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlan::DeactivatePlan
//
//	PURPOSE:	Deactivate a current plan.
//
// ----------------------------------------------------------------------- //

void CAIPlan::DeactivatePlan( )
{
	// Deactivate the current action.

	CAIActionAbstract* pAction = GetCurrentPlanStepAction();
	if( pAction )
	{
		pAction->DeactivateAction( m_pAI );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlan::PlanStepIsComplete
//
//	PURPOSE:	Return true if current plan step is complete.
//
// ----------------------------------------------------------------------- //

bool CAIPlan::PlanStepIsComplete()
{
	if( m_iPlanStep >= m_lstAIPlanSteps.size() )
	{
		return false;
	}

	CAIPlanStep* pPlanStep = m_lstAIPlanSteps[m_iPlanStep];
	if( pPlanStep )
	{
		CAIActionAbstract* pAction = g_pAIActionMgr->GetAIAction( pPlanStep->eAIAction );
		if( pAction )
		{
			return pAction->IsActionComplete( m_pAI );
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlan::PlanStepIsInterruptible
//
//	PURPOSE:	Return true if current plan step is interruptible.
//
// ----------------------------------------------------------------------- //

bool CAIPlan::PlanStepIsInterruptible()
{
	if( m_iPlanStep >= m_lstAIPlanSteps.size() )
	{
		return true;
	}

	CAIPlanStep* pPlanStep = m_lstAIPlanSteps[m_iPlanStep];
	if( pPlanStep )
	{
		CAIActionAbstract* pAction = g_pAIActionMgr->GetAIAction( pPlanStep->eAIAction );
		if( pAction )
		{
			return pAction->IsActionInterruptible(m_pAI);
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlan::AdvancePlan
//
//	PURPOSE:	Advance plan to next step and activate action.
//
// ----------------------------------------------------------------------- //

bool CAIPlan::AdvancePlan()
{
	CAIActionAbstract* pAction;
	CAIPlanStep* pPlanStep;

	// Continue advancing the plan until we find
	// one that can activate and is not immediately
	// complete.

	while( 1 )
	{
		// Apply Effects to context.
		// By default, Actions do not have any effect on the real world.
		// They only affect the planners representation of the world.
		// This gives more control of where WorldState variables are set,
		// or allows them to never be set.
		// (e.g. AtTargetPos is never really true in the real world,
		// because targets more continuously.)

		pPlanStep = m_lstAIPlanSteps[m_iPlanStep];
		if( pPlanStep )
		{
			pAction = g_pAIActionMgr->GetAIAction( pPlanStep->eAIAction );
			if( pAction )
			{
				pAction->ApplyContextEffect( m_pAI, m_pAI->GetAIWorldState(), &( pPlanStep->wsWorldState ) );
				pAction->DeactivateAction( m_pAI );
			}
		}


		// Advance the step.
		// Bail if no more steps.

		++m_iPlanStep;
		if( m_iPlanStep >= m_lstAIPlanSteps.size() )
		{
			return false;
		}

		pPlanStep = m_lstAIPlanSteps[m_iPlanStep];
		if( pPlanStep )
		{
			pAction = g_pAIActionMgr->GetAIAction( pPlanStep->eAIAction );
			if( pAction )
			{
				// Action's preconditions are not met. Bail.

				if( !pAction->ValidateContextPreconditions( m_pAI, pPlanStep->wsWorldState, !IS_PLANNING ) )
				{
					return false;
				}

				// Bail if action is not immediately complete.

				AITRACE( AIShowActions, ( m_pAI->m_hObject, "Activating Action: %s (%.2f)", s_aszActionTypes[pAction->GetActionRecord()->eActionType], pAction->GetActionRecord()->fActionCost ) );
				pAction->ActivateAction( m_pAI, pPlanStep->wsWorldState );
				if( !pAction->IsActionComplete( m_pAI ) )
				{
					return true;
				}

				// Deactivate action if it was immediately complete.

				pAction->DeactivateAction( m_pAI );
			}
		}
	}

	// Something is wrong.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlan::IsPlanValid
//
//	PURPOSE:	Return true if the plan is valid.
//
// ----------------------------------------------------------------------- //

bool CAIPlan::IsPlanValid()
{
	if( m_iPlanStep >= m_lstAIPlanSteps.size() )
	{
		return false;
	}

	CAIPlanStep* pPlanStep = m_lstAIPlanSteps[m_iPlanStep];
	if( pPlanStep )
	{
		CAIActionAbstract* pAction = g_pAIActionMgr->GetAIAction( pPlanStep->eAIAction );
		if( pAction )
		{
			return pAction->ValidateAction( m_pAI );
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlan::GetCurrentPlanStepAction
//
//	PURPOSE:	Return the current plan step's action.
//
// ----------------------------------------------------------------------- //

CAIActionAbstract* CAIPlan::GetCurrentPlanStepAction()
{
	if( m_iPlanStep >= m_lstAIPlanSteps.size() )
	{
		return NULL;
	}

	CAIPlanStep* pPlanStep = m_lstAIPlanSteps[m_iPlanStep];
	if( pPlanStep )
	{
		return g_pAIActionMgr->GetAIAction( pPlanStep->eAIAction );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

//
// CAIPlanner
//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlanner::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIPlanner::CAIPlanner()
{
	AIASSERT( !g_pAIPlanner, NULL, "CAIPlanner: Singleton already set." );
	g_pAIPlanner = this;
}

CAIPlanner::~CAIPlanner()
{
	AIASSERT( g_pAIPlanner, NULL, "CAIPlanner: No singleton." );
	g_pAIPlanner = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlanner::InitAIPlanner
//
//	PURPOSE:	Initialize the planner.
//
// ----------------------------------------------------------------------- //

void CAIPlanner::InitAIPlanner()
{
	m_AStarStoragePlanner.InitAStarStoragePlanner( &m_AStar );
	m_AStar.InitAStar( &m_AStarStoragePlanner, &m_AStarGoalPlanner, &m_AStarMapPlanner );
	m_AStarMapPlanner.BuildEffectActionsTable();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlanner::BuildPlan
//
//	PURPOSE:	Return true if successful in building a plan that 
//              will satisfy the AIGoal.
//
// ----------------------------------------------------------------------- //

bool CAIPlanner::BuildPlan( CAI* pAI, CAIGoalAbstract* pGoal )
{
	//track our performance
	///CTimedSystemBlock TimingBlock(g_tsAIPlanner);

	// Initialize the planner.

	m_AStarMapPlanner.InitAStarMapPlanner( pAI );
	m_AStarGoalPlanner.InitAStarGoalPlanner( pAI, &m_AStarMapPlanner, pGoal );

	// Set the start of the search to -1, indicating that
	// the search starts from the AIGoal rather than from 
	// an AIAction.

	m_AStar.SetAStarSource( (ENUM_AStarNodeID)-1 );

	// Run the AStar machine to search for a valid plan
	// to satisfy the AIGoal.

	AITRACE( AIShowPlanner, ( pAI->m_hObject, "Planner starting AStar for Goal '%s'...", s_aszGoalTypes[pGoal->GetGoalType()] ) );
	m_AStar.RunAStar( pAI );

	// If after the search the current node is NULL, then no
	// valid plan was found.

	CAIAStarNodePlanner* pNode = (CAIAStarNodePlanner*)( m_AStar.GetAStarNodeCur() );
	if( !pNode )
	{
		AITRACE( AIShowPlanner, ( pAI->m_hObject, "No plan found." ) );
		return false;
	}

	// Create a new plan.

	CAIPlan* pPlan = AI_FACTORY_NEW( CAIPlan );
	AITRACE( AIShowPlanner, ( pAI->m_hObject, "Found plan:" ) );

	// Iterate over nodes in the planner's search path,
	// and add them to the plan.

	EnumAIActionType eAction;
	CAIPlanStep* pPlanStep;
	AIPLAN_STEP_LIST::iterator itPlan = pPlan->m_lstAIPlanSteps.end();
	while( pNode )
	{
		// If the AIAction is Invalid, this is the final node.

		eAction = m_AStarMapPlanner.ConvertID_AStarNode2AIAction( pNode->eAStarNodeID );
		if( eAction == kAct_InvalidType )
		{
			break; 
		}

		// Create a new plan step.

		pPlanStep = AI_FACTORY_NEW( CAIPlanStep );

		// Set the AIAction for this plan step.

		AITRACE( AIShowPlanner, ( pAI->m_hObject, "  Action: %s", s_aszActionTypes[eAction] ) );
		pPlanStep->eAIAction = eAction;

		// Advance the plan to the next node.

		pNode = (CAIAStarNodePlanner*)( pNode->pAStarParent );

		// Copy the world state from the node.
		// This is the world state that should be achieved
		// taking this step of the plan.

		pPlanStep->wsWorldState.CopyWorldState( pNode->wsWorldStateGoal );

		// Add the new step to the plan.

		pPlan->m_lstAIPlanSteps.push_back( pPlanStep );
	}

	// Set the new plan for the AIGoal.

	pGoal->SetAIPlan( pPlan );

	// Successfully built a plan.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlanner::MergeAndEvaluateWorldStates
//
//	PURPOSE:	Merge properties from the goal world state into
//              the current world state, if not already present.
//              Evaluate true world state of properties that were not 
//              originally in the current world state.
//
// ----------------------------------------------------------------------- //

void CAIPlanner::MergeWorldStates( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal )
{
	SAIWORLDSTATE_PROP* pProp;

	AIWORLDSTATE_PROP_SET_FLAGS* pFlagsCur = wsWorldStateCur.GetWSPropSetFlags();
	AIWORLDSTATE_PROP_SET_FLAGS* pFlagsGoal = wsWorldStateGoal.GetWSPropSetFlags();

	// NOTE: The current and goal world states only contain
	// properties relevant to the AIGoal that the planner is trying
	// to satisfy, and properties added by preconditions of AIActions.
	// The AI's view of the true world state may contain many more
	// properties.

	// Iterate over properties in goal world state.

	for( unsigned int iProp=0; iProp < kWSK_Count; ++iProp )
	{
		// Continue if property already exists in current 
		// world state.

		if( ( !pFlagsGoal->test( iProp ) ) ||
			(  pFlagsCur->test( iProp ) ) )
		{
			continue;
		}

		pProp = wsWorldStateGoal.GetWSProp( iProp );	
		if( !pProp )
		{
			continue;
		}

		// Add property to current world state.

		SAIWORLDSTATE_PROP prop;
		prop.eWSKey = pProp->eWSKey;

		prop.eWSType = kWST_Unset;

		// Get the true current value of the world state
		// property, as determined by the AI's actual world 
		// state.

		EvaluateWorldStateProp( pAI, prop );

		wsWorldStateCur.SetWSProp( &prop );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlanner::EvaluateWorldStateProp
//
//	PURPOSE:	Evaluate the true value of a world state property.
//
// ----------------------------------------------------------------------- //

void CAIPlanner::EvaluateWorldStateProp( CAI* pAI, SAIWORLDSTATE_PROP& prop )
{
	CAIWorldState* pWorldState = pAI->GetAIWorldState();
	if( !pWorldState )
	{
		return;
	}

	SAIWORLDSTATE_PROP* pWSProp = pWorldState->GetWSProp( prop.eWSKey, NULL );
	if( !pWSProp )
	{
		AIASSERT( 0, pAI->m_hObject, "CAIPlanner::EvaluateWorldStateProp: Unhandled World State." );
		return;
	}
	
	prop = *pWSProp;
}

// ----------------------------------------------------------------------- //

