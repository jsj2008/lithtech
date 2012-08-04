// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarPlanner.cpp
//
// PURPOSE : AStar Node, Goal, Storage, and Map classes for finding
//           action plans with the AIPlanner.
//
// CREATED : 1/30/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"

// Includes required for AIAStarPlanner.h

#include "AIClassFactory.h"
#include "AIAStarMachine.h"

// Includes required for AIAStarPlanner.cpp

#include "AI.h"
#include "AIDB.h"
#include "AIAStarPlanner.h"
#include "AIPlanner.h"
#include "AIGoalAbstract.h"
#include "AIActionMgr.h"
#include "AIUtils.h"
#include "AIBlackBoard.h"

// Factories.

DEFINE_AI_FACTORY_CLASS( CAIAStarNodePlanner );

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarNodePlanner
//

CAIAStarNodePlanner::CAIAStarNodePlanner()
{
	pAStarMachine = NULL;
	wsWorldStateCur.ResetWS();
	wsWorldStateGoal.ResetWS();
}

void CAIAStarNodePlanner::DebugPrintExpand()
{
	CAIAStarMapPlanner*	pMap = (CAIAStarMapPlanner*)pAStarMachine->GetAStarMap();
	if( pMap )
	{
		CAIActionAbstract* pAction = pMap->GetAIAction( eAStarNodeID );
		if( pAction )
		{
			AITRACE( AIShowPlanner, ( (HOBJECT)NULL, "Planner Expanding Action: %s", s_aszActionTypes[pAction->GetActionRecord()->eActionType] ) );
		}
		else {
			AITRACE( AIShowPlanner, ( (HOBJECT)NULL, "Planner Expanding Goal" ) );
		}
	}
}

void CAIAStarNodePlanner::DebugPrintNeighbor()
{
	CAIAStarMapPlanner*	pMap = (CAIAStarMapPlanner*)pAStarMachine->GetAStarMap();
	if( pMap )
	{
		CAIActionAbstract* pAction = pMap->GetAIAction( eAStarNodeID );
		if( pAction )
		{
			AITRACE( AIShowPlanner, ( (HOBJECT)NULL, "   neighbor Action: %s", s_aszActionTypes[pAction->GetActionRecord()->eActionType] ) );
		}
	}
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarGoalPlanner
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalPlanner::Con/Destructor
//              
//	PURPOSE:	Construction / Destruction.
//              
//----------------------------------------------------------------------------

CAIAStarGoalPlanner::CAIAStarGoalPlanner()
{
	m_eAStarNodeDest = kASTARNODE_Invalid;
	m_pAStarMapPlanner = NULL;
	m_pAIGoal = NULL;
}

CAIAStarGoalPlanner::~CAIAStarGoalPlanner()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalPlanner::InitAStarGoalPlanner
//              
//	PURPOSE:	Initialize astar goal.
//              
//----------------------------------------------------------------------------

void CAIAStarGoalPlanner::InitAStarGoalPlanner( CAI* pAI, CAIAStarMapPlanner* pAStarMapPlanner, CAIGoalAbstract* pGoal )
{
	m_pAI = pAI;
	m_pAStarMapPlanner = pAStarMapPlanner; 
	m_pAIGoal = pGoal;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalPlanner::GetHeuristicDistance
//              
//	PURPOSE:	Get a best-guess distance from a node to the goal.
//              
//----------------------------------------------------------------------------

float CAIAStarGoalPlanner::GetHeuristicDistance( CAIAStarNodeAbstract* pAStarNode )
{
	CAIAStarNodePlanner* pNode = ( CAIAStarNodePlanner* )pAStarNode;

	// The AIAStarMapPlanner converts between astar node IDs and AIAction enums.
	// The initial node in the search tree is the goal, and does not refer to 
	// any action. It has a node ID of -1 == kAct_InvalidType.

	// This node refers to an AIAction.

	CAIActionAbstract* pAction = m_pAStarMapPlanner->GetAIAction( pAStarNode->eAStarNodeID );
	if( pAction )
	{
		// Use the AIAction to solve unsatisfied world state properties.

		CAIAStarNodePlanner* pNode = ( CAIAStarNodePlanner* )pAStarNode;
		pAction->SolvePlanWSVariable( m_pAI, pNode->wsWorldStateCur, pNode->wsWorldStateGoal );

		// Add additional preconditions from the AIAction to the goal world state.

		pAction->SetPlanWSPreconditions( m_pAI, pNode->wsWorldStateGoal );

		// Determine the current world state values for any new world state properties.

		g_pAIPlanner->MergeWorldStates( m_pAI, pNode->wsWorldStateCur, pNode->wsWorldStateGoal );
	}

	// This node refers to an AIGoal.

	else if( m_pAIGoal )
	{
		// Set the initial goal world state from the AIGoal.

		m_pAIGoal->SetWSSatisfaction( pNode->wsWorldStateGoal );

		// Determine the current world state values for world state properties.

		g_pAIPlanner->MergeWorldStates( m_pAI, pNode->wsWorldStateCur, pNode->wsWorldStateGoal );
	}

	// Return the number of unsatisfied world state properties as an estimate of the 
	// number of actions it will take to satisfy the goal.
	// A property is unsatisfied if the value differs between the current
	// and goal world states.

	return (float)( pNode->wsWorldStateCur.GetNumWorldStateDifferences( pNode->wsWorldStateGoal ) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalPlanner::GetActualCost
//              
//	PURPOSE:	Return the actual cost from one node to another.
//              
//----------------------------------------------------------------------------

float CAIAStarGoalPlanner::GetActualCost( CAI* pAI, CAIAStarNodeAbstract* pAStarNodeA, CAIAStarNodeAbstract* pAStarNodeB )
{
	CAIAStarNodePlanner* pNodeA = (CAIAStarNodePlanner*)pAStarNodeA;
	CAIAStarNodePlanner* pNodeB = (CAIAStarNodePlanner*)pAStarNodeB;

	// Copy the current and goal world states from a parent node.

	pNodeB->wsWorldStateCur.CopyWorldState( pNodeA->wsWorldStateCur );
	pNodeB->wsWorldStateGoal.CopyWorldState( pNodeA->wsWorldStateGoal );

	// Return the cost of this node.

	CAIActionAbstract* pAction = m_pAStarMapPlanner->GetAIAction( pAStarNodeB->eAStarNodeID );
	if( pAction )
	{
		return pAction->GetActionCost();
	}

	return 1.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalPlanner::IsAStarFinished
//              
//	PURPOSE:	Return true if node is null, or a valid plan has been found.
//              
//----------------------------------------------------------------------------

bool CAIAStarGoalPlanner::IsAStarFinished( CAIAStarNodeAbstract* pAStarNode )
{
	// AStar is finished, because there are no more nodes.

	if( !pAStarNode )
	{
		return true;
	}

	// AStar is finished because a plan has been found that 
	// satisfies the AIGoal.

	CAIAStarNodePlanner* pNode = (CAIAStarNodePlanner*)pAStarNode;
	if( IsPlanValid( pNode ) )
	{
		return true;
	}

	// AStar is not finished.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalPlanner::IsPlanValid
//              
//	PURPOSE:	Return true if the plan satisfies the AIGoal, from
//              the current world state.
//              
//----------------------------------------------------------------------------

bool CAIAStarGoalPlanner::IsPlanValid( CAIAStarNodePlanner* pAStarNode )
{
	AITRACE( AIShowPlanner, ( (HOBJECT)NULL, "Testing plan validity..." ) );

	// Uncomment for debugging plans!
	/**
	uint32 iStep = 1;
	CAIAStarNodePlanner* pAStarNodeTemp = pAStarNode;
	CAIActionAbstract* pActionTemp = m_pAStarMapPlanner->GetAIAction( pAStarNodeTemp->eAStarNodeID );
	while( pActionTemp )
	{
		AITRACE( AIShowPlanner, ( (HOBJECT)NULL, "  %d: %s", iStep, s_aszActionTypes[pActionTemp->GetActionRecord()->eActionType] ) );
		++iStep;
		pAStarNodeTemp = (CAIAStarNodePlanner*)( pAStarNodeTemp->pAStarParent );
		pActionTemp = pAStarNodeTemp ? m_pAStarMapPlanner->GetAIAction( pAStarNodeTemp->eAStarNodeID ) : NULL;
	}
	**/

	// Get the true current world state.

	CAIWorldState wsWorldState;
	g_pAIPlanner->MergeWorldStates( m_pAI, wsWorldState, pAStarNode->wsWorldStateCur );

	// Walk the sequence of AIActions, modifying the world 
	// state on each iteration.

	CAIActionAbstract* pAction = m_pAStarMapPlanner->GetAIAction( pAStarNode->eAStarNodeID );
	if( !pAction )
	{
		AITRACE( AIShowPlanner, ( m_pAI->m_hObject, "Plan invalid - No actions." ) );
		return false;
	}

	CAIAStarNodePlanner* pNodeParent = NULL;
	while( pAction )
	{
		AITRACE( AIShowPlanner, ( (HOBJECT)NULL, "PlanStep: %s", s_aszActionTypes[pAction->GetActionRecord()->eActionType] ) );

		pNodeParent = (CAIAStarNodePlanner*)( pAStarNode->pAStarParent );

		// Bail if the AIAction's effects are already met by the current world state.

		if( !pAction->ValidateWSEffects( m_pAI, wsWorldState, pNodeParent->wsWorldStateGoal ) )
		{
			AITRACE( AIShowPlanner, ( m_pAI->m_hObject, "Effect failed: %s", s_aszActionTypes[pAction->GetActionRecord()->eActionType] ) );
			return false;
		}

		// Bail if the AIAction's preconditions are not met by the current world state.

		ENUM_AIWORLDSTATE_PROP_KEY eFailedWSK = kWSK_InvalidKey;
		if( !pAction->ValidateWSPreconditions( m_pAI, wsWorldState, pNodeParent->wsWorldStateGoal, &eFailedWSK ) )
		{
			AITRACE( AIShowPlanner, ( m_pAI->m_hObject, "Precondition failed: %s (%s)", s_aszActionTypes[pAction->GetActionRecord()->eActionType], GetAIWorldStatePropName( eFailedWSK ) ) );
			return false;
		}

		if( !pAction->ValidateContextPreconditions( m_pAI, pNodeParent->wsWorldStateGoal, IS_PLANNING ) )
		{
			AITRACE( AIShowPlanner, ( m_pAI->m_hObject, "Real-time Precondition failed: %s", s_aszActionTypes[pAction->GetActionRecord()->eActionType] ) );
			return false;
		}

		// Bail if probability fails.

		if( ( pAction->GetActionProbability( m_pAI ) < 1.f ) &&
			( pAction->GetActionProbability( m_pAI ) < GetRandom( 0.0f, 1.0f ) ) )
		{
			AITRACE( AIShowPlanner, ( m_pAI->m_hObject, "Probability failed: %s", s_aszActionTypes[pAction->GetActionRecord()->eActionType] ) );
			pAction->FailActionProbability( m_pAI );
			return false;
		}

		// Apply the AIActions effects to the world state.

		pAction->ApplyWSEffect( m_pAI, &wsWorldState, &( pNodeParent->wsWorldStateGoal ) );
		
		// Get the next AIAction in the sequence.

		pAStarNode = (CAIAStarNodePlanner*)( pAStarNode->pAStarParent );
		if( pAStarNode )
		{
			pAction = m_pAStarMapPlanner->GetAIAction( pAStarNode->eAStarNodeID );
		}
	}

	// The final node is the AIGoal.
	// Return true if the goal is satisfied by the world state created
	// by the sequence of actions.

	if( m_pAIGoal && pNodeParent )
	{
		if( pNodeParent->wsWorldStateGoal.GetNumUnsatisfiedWorldStateProps( wsWorldState ) > 0 )
		{
			AITRACE( AIShowPlanner, ( m_pAI->m_hObject, "Satisfaction conditions not met." ) );
			return false;
		}

		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarStoragePlanner
// Most of the code is in AIAStarStorageLinkedList.
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStoragePlanner::InitAStarStoragePlanner
//              
//	PURPOSE:	Initialize storage. 
//              
//----------------------------------------------------------------------------

void CAIAStarStoragePlanner::InitAStarStoragePlanner( CAIAStarMachine* pAStarMachine )
{
	m_pAIAStarMachine = pAStarMachine;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStoragePlanner::CreateAStarNode
//              
//	PURPOSE:	Create an AStarNode with a specified ID. 
//              
//----------------------------------------------------------------------------

CAIAStarNodeAbstract* CAIAStarStoragePlanner::CreateAStarNode( ENUM_AStarNodeID eAStarNode )
{
	// Create a node with a specified AStarNodeID.
	// The AIAStarMapPlanner converts vetween NodeIDs and AIAction enums.

	CAIAStarNodePlanner* pNode = AI_FACTORY_NEW( CAIAStarNodePlanner );
	pNode->eAStarNodeID = eAStarNode;
	pNode->pAStarMachine = m_pAIAStarMachine;

	return pNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStoragePlanner::DestroyAStarNode
//              
//	PURPOSE:	Destroy an AStarNode. 
//              
//----------------------------------------------------------------------------

void CAIAStarStoragePlanner::DestroyAStarNode( CAIAStarNodeAbstract* pAStarNode )
{
	AI_FACTORY_DELETE( pAStarNode );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarMapPlanner
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::Con/Destructor
//              
//	PURPOSE:	Construction / Destruction.
//              
//----------------------------------------------------------------------------

CAIAStarMapPlanner::CAIAStarMapPlanner()
{
	m_pAI = NULL;
	m_bEffectTableBuilt = false;
}

CAIAStarMapPlanner::~CAIAStarMapPlanner()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::BuildEffectActionsTable
//              
//	PURPOSE:	Build the table that maps AIACtions to their effects.
//              
//----------------------------------------------------------------------------

void CAIAStarMapPlanner::BuildEffectActionsTable()
{
	// Check if table already built.
	if( m_bEffectTableBuilt )
		return;

	// Build a table that maps effects to actions.

	int iKey;
	CAIActionAbstract* pAction;
	CAIWorldState* pEffects;
	AIWORLDSTATE_PROP_SET_FLAGS* pFlags;

	// Iterate over all actions, storing them in the table 
	// indexed by their effects.

	int cActions = g_pAIActionMgr->GetNumAIActions();
	for( int iAction=0; iAction < cActions; ++iAction )
	{
		// Access an action.

		pAction = g_pAIActionMgr->GetAIAction( ( EnumAIActionType )iAction );
		if( !pAction )
		{
			continue;
		}

		// Get the list of effects for this action.

		pEffects = pAction->GetActionEffects();
		if( !pEffects )
		{
			continue;
		}

		// Get the bit flags representing the effects.

		pFlags = pEffects->GetWSPropSetFlags();
		if( !pFlags )
		{
			continue;
		}

		// Iterate over all possible WorldState property keys.

		for( iKey=0; iKey < kWSK_Count; ++iKey )
		{
			// If the effect has something set for this key,
			// add the action's class enum to the table under this effect.

			if( pFlags->test( iKey ) )
			{
				m_lstEffectActions[iKey].push_back( pAction->GetActionRecord()->eActionType );
			}
		}
	}

	m_bEffectTableBuilt = true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::InitAStarMapPlanner
//              
//	PURPOSE:	Initialize the AStar map.
//              
//----------------------------------------------------------------------------

void CAIAStarMapPlanner::InitAStarMapPlanner( CAI* pAI )
{
	if( m_pAI == pAI )
	{
		return;
	}

	m_pAI = pAI;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::CompareActions
//              
//	PURPOSE:	Compare function used to qsort actions.
//              
//----------------------------------------------------------------------------

int CompareActions( const void *arg1, const void *arg2 )
{
	if( !( arg1 && arg2 ) )
	{
		return 0;
	}

	// Actions are sorted by precedence.

	EnumAIActionType eAction1 = *( EnumAIActionType* )arg1;
	EnumAIActionType eAction2 = *( EnumAIActionType* )arg2;

	CAIActionAbstract* pAction1 = g_pAIActionMgr->GetAIAction( eAction1 );
	CAIActionAbstract* pAction2 = g_pAIActionMgr->GetAIAction( eAction2 );
	if( !( pAction1 && pAction2 ) )
	{
		return 0;
	}

	if( pAction1->GetActionPrecedence() < pAction2->GetActionPrecedence() )
	{
		return 1;
	}

	if( pAction1->GetActionPrecedence() > pAction2->GetActionPrecedence() )
	{
		return -1;
	}

	return 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::GetNumAStarNeighbors
//              
//	PURPOSE:	Return number of neighbors for an AStarNode.
//              Also build a temporary list of who the neighbors
//              are, to be used in GetAStarNeighbor().
//              
//----------------------------------------------------------------------------

int CAIAStarMapPlanner::GetNumAStarNeighbors( CAI* /*pAI*/, CAIAStarNodeAbstract* pAStarNode )
{
	// Sanity check.

	if( !pAStarNode )
	{
		return 0;
	}

	// A neighbor is an Action that has an effect that has the potential
	// to satisfy one of the unsatisfied properties of the goal.
	// Neighbors are based only on the property key enum, not on the associated
	// value.  For example, both DrawWeapon and HoslterWeapon are neighbors
	// of an AStarNode that has an unsatisfied property of kWSK_WeaponArmed.

	m_cNeighborActions = 0;
	AI_ACTION_TYPE_LIST::iterator itAction;
	EnumAIActionType eAction;
	CAIActionAbstract* pAction;

	CAIAStarNodePlanner* pNode = (CAIAStarNodePlanner*)pAStarNode;
	SAIWORLDSTATE_PROP* pPropCur;
	SAIWORLDSTATE_PROP* pPropGoal;
	ENUM_AIWORLDSTATE_PROP_KEY eWSKey;

	// Get the flags representing properties set in the 
	// current and goal world states.

	AIWORLDSTATE_PROP_SET_FLAGS* pFlagsCur = pNode->wsWorldStateCur.GetWSPropSetFlags();
	AIWORLDSTATE_PROP_SET_FLAGS* pFlagsGoal = pNode->wsWorldStateGoal.GetWSPropSetFlags();

	// Iterate over all possible effect world state properties.

	for( unsigned int iEffect=0; iEffect < kWSK_Count; ++iEffect )
	{
		// Ignore the effect if it does not appear in current and goal states.

		if( !( pFlagsCur->test( iEffect ) && pFlagsGoal->test( iEffect ) ) )
		{
			continue;
		}

		eWSKey = (ENUM_AIWORLDSTATE_PROP_KEY)iEffect;

		// Determine if the effect matches any of the world state properties
		// that are different in the current and goal world states.

		pPropCur = pNode->wsWorldStateCur.GetWSProp( eWSKey, m_pAI->m_hObject );
		pPropGoal = pNode->wsWorldStateGoal.GetWSProp( eWSKey, m_pAI->m_hObject );

		if( pPropCur && pPropGoal )
		{
			if( ( pPropCur->hWSValue != pPropGoal->hWSValue ) ||
				( pPropCur->eWSType != pPropGoal->eWSType ) )
			{
				// Add actions for this effect to the list of neighbors.

				for( itAction = m_lstEffectActions[eWSKey].begin(); itAction != m_lstEffectActions[eWSKey].end(); ++itAction )
				{
					eAction = *itAction;

					// Ignore actions if they are not in the AIs action set.

					if (!g_pAIActionMgr->IsActionInAIActionSet(
						m_pAI->GetAIBlackBoard()->GetBBAIActionSet(), 
						eAction))
					{
						continue;
					}

					pAction = g_pAIActionMgr->GetAIAction( eAction );

					// Ignore actions if their context preconditions are not met.

					if( !( pAction && pAction->ValidateContextPreconditions( m_pAI, pNode->wsWorldStateGoal, IS_PLANNING ) ) )
					{
						AITRACE( AIShowPlanner, ( (HOBJECT)NULL, "  Action not neighbor due to context preconditions: %s", s_aszActionTypes[pAction->GetActionRecord()->eActionType] ) );
						continue;
					}

					m_aNeighborActions[m_cNeighborActions] = eAction;
					++m_cNeighborActions;

					if( m_cNeighborActions >= kAct_Count )
					{
						break;
					}
				}
			}
		}

		if( m_cNeighborActions >= kAct_Count )
		{
			break;
		}
	}

	// Sort Actions by precedence.
	// This sorting will produce plans that have actions sorted by precedence,
	// as specified in AIActions.txt.
	// (e.g. AI will first SurveyArea and then GotoPointOfInterest, if 
	// SurveyArea has a higher precedence value).

	if( m_cNeighborActions > 1 )
	{
		qsort( (void*)m_aNeighborActions, (size_t)m_cNeighborActions, sizeof( EnumAIActionType ), CompareActions );
	}

	return m_cNeighborActions;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::GetAStarNeighbor
//              
//	PURPOSE:	Return the node ID of a specified neighbor.
//              
//----------------------------------------------------------------------------

ENUM_AStarNodeID CAIAStarMapPlanner::GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage )
{
	return ConvertID_AIAction2AStarNode( m_aNeighborActions[iNeighbor] );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::SetAStarFlags
//              
//	PURPOSE:	Set AStar flags for a specified node.
//              
//----------------------------------------------------------------------------

void CAIAStarMapPlanner::SetAStarFlags( ENUM_AStarNodeID eAStarNode, unsigned long dwFlags, unsigned long dwMask )
{
	// AIAStarMapPlanner does not keep track of flags because a nodeID may 
	// be reused many times, and does not refer to the same node.
	// This is due to the fact that an AIAction may be used multiple times 
	// in the same plan for different reasons (e.g. GotoNode to different nodes).
	// So, reusing the same nodeID does not indicate a cycle, as it would 
	// in physical pathfinding, where a nodeID refers to a point in space.
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::GetAStarFlags
//              
//	PURPOSE:	Get AStar flags for a specified node.
//              
//----------------------------------------------------------------------------

unsigned long CAIAStarMapPlanner::GetAStarFlags( ENUM_AStarNodeID eAStarNode )
{
	return kASTAR_Unchecked;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapPlanner::GetAIAction
//              
//	PURPOSE:	Return a pointer to the AIAction referenced by the nodeID.
//              
//----------------------------------------------------------------------------

CAIActionAbstract* CAIAStarMapPlanner::GetAIAction( ENUM_AStarNodeID eAStarNode )
{
	return g_pAIActionMgr->GetAIAction( ConvertID_AStarNode2AIAction( eAStarNode ) ); 
}



