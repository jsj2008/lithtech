// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAbstract.cpp
//
// PURPOSE : AIActionAbstract abstract class implementation.
//           AI use sequences of AIActions to satisfy AIGoals.
//           The AIPlanner finds the sequences of AIActions.
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "AIActionAbstract.h"
#include "AI.h"
#include "AIDB.h"
#include "AIUtils.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIState.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAbstract::CAIActionAbstract()
{
}

CAIActionAbstract::~CAIActionAbstract()
{
	m_wsWorldStatePreconditions.ResetWS();
	m_wsWorldStateEffects.ResetWS();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::InitAction
//
//	PURPOSE:	Initialize the Action.
//
// ----------------------------------------------------------------------- //

void CAIActionAbstract::InitAction( AIDB_ActionRecord* pActionRecord )
{
	m_pActionRecord = pActionRecord;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::SetPlanWSPreconditions
//
//	PURPOSE:	Set this action's preconditions in plan's goal world state.
//
// ----------------------------------------------------------------------- //

void CAIActionAbstract::SetPlanWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	SAIWORLDSTATE_PROP* pPrecondition;
	SAIWORLDSTATE_PROP* pProp;
	
	// Iterate over all preconditions.

	for( unsigned int iPrecondition=0; iPrecondition < kWSK_Count; ++iPrecondition )
	{
		// Get a pointer to the indexed precondition.

		pPrecondition = m_wsWorldStatePreconditions.GetWSProp( iPrecondition );
		if( !pPrecondition )
		{
			continue;
		}

		// The precondition is not a variable.
		// Set it's values in the goal state.

		if( pPrecondition->eWSType != kWST_Variable )
		{
			wsWorldStateGoal.SetWSProp( pPrecondition->eWSKey, pAI->m_hObject, pPrecondition->eWSType, pPrecondition->nWSValue );
		}

		// The precondition is a variable.
		// Get a pointer to the world state property that the variable references,
		// and set the value in the goal state with that referenced value.

		else {
			pProp = wsWorldStateGoal.GetWSProp( pPrecondition->eVariableWSKey, pAI->m_hObject );
			if( pProp )
			{
				wsWorldStateGoal.SetWSProp( pPrecondition->eWSKey, pAI->m_hObject, pProp->eWSType, pProp->nWSValue );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::ValidateWSPreconditions
//
//	PURPOSE:	Return true if this action's preconditions are met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionAbstract::ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK )
{
	SAIWORLDSTATE_PROP* pPrecondition;
	SAIWORLDSTATE_PROP* pPropA;
	SAIWORLDSTATE_PROP* pPropB;

	// Iterate over all preconditions.
	
	for( unsigned int iPrecondition=0; iPrecondition < kWSK_Count; ++iPrecondition )
	{
		// Get a pointer to the indexed precondition.

		pPrecondition = m_wsWorldStatePreconditions.GetWSProp( iPrecondition );
		if( !pPrecondition )
		{
			continue;
		}

		// The precondition is not a variable.
		// Determine if the current world state's value
		// matches the precondition's value.

		if( pPrecondition->eWSType != kWST_Variable )
		{
			pPropA = wsWorldStateCur.GetWSProp( pPrecondition->eWSKey, pAI->m_hObject );
			if( pPropA && 
				( ( pPropA->nWSValue != pPrecondition->nWSValue ) ||
				  ( pPropA->eWSType != pPrecondition->eWSType ) ) )
			{
				if( pFailedWSK )
				{
					*pFailedWSK = pPrecondition->eWSKey;
				}
				return false;
			}
		}

		// The precondition is a variable.
		// Determine if the current world state's value matches
		// the referenced value in the goal state.

		else {
			pPropA = wsWorldStateGoal.GetWSProp( pPrecondition->eVariableWSKey, pAI->m_hObject );
			pPropB = wsWorldStateCur.GetWSProp( pPrecondition->eWSKey, pAI->m_hObject );
			if( pPropA && pPropB )
			{
				if( ( pPropA->nWSValue != pPropB->nWSValue ) ||
					( pPropA->eWSType != pPropB->eWSType ) )
				{
					if( pFailedWSK )
					{
						*pFailedWSK = pPrecondition->eWSKey;
					}
					return false;
				}
			}
		}
	}

	// Preconditions are valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::ValidateWSEffects
//
//	PURPOSE:	Return true if this action's effects are not already met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionAbstract::ValidateWSEffects( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal )
{
	SAIWORLDSTATE_PROP* pEffect;
	SAIWORLDSTATE_PROP* pPropA;
	SAIWORLDSTATE_PROP* pPropB;

	// Iterate over all effects.

	for( unsigned int iEffect=0; iEffect < kWSK_Count; ++iEffect )
	{
		// Get a pointer to the indexed effect.

		pEffect = m_wsWorldStateEffects.GetWSProp( iEffect );
		if( !pEffect )
		{
			continue;
		}

		// The effect is not a variable.
		// Determine if the current world state's value
		// matches the effect's value.

		if( pEffect->eWSType != kWST_Variable )
		{
			pPropA = wsWorldStateCur.GetWSProp( pEffect->eWSKey, pAI->m_hObject );
			if( pPropA 
				&& ( ( pPropA->nWSValue == pEffect->nWSValue ) 
				&& ( pPropA->eWSType == pEffect->eWSType )
				&& ( pPropA->eWSType != kWST_Unset )
				&& ( pPropA->eWSType != kWST_InvalidType ) ) )
			{
				continue;
			}
		}

		// The effect is a variable.
		// Determine if the current world state's value matches
		// the referenced value in the goal state.

		else {
			pPropA = wsWorldStateGoal.GetWSProp( pEffect->eVariableWSKey, pAI->m_hObject );
			pPropB = wsWorldStateCur.GetWSProp( pEffect->eWSKey, pAI->m_hObject );
			if( pPropA && pPropB )
			{
				if( ( pPropA->nWSValue == pPropB->nWSValue ) 
					&& ( pPropA->eWSType == pPropB->eWSType ) 
					&& ( pPropA->eWSType != kWST_Unset )
					&& ( pPropA->eWSType != kWST_InvalidType ) 
					&& ( pPropB->eWSType != kWST_Unset )
					&& ( pPropB->eWSType != kWST_InvalidType ) )
				{
					continue;
				}
			}
		}

		// An effect not met detected.  This action is valid.

		return true;
	}

	// All effects are met.  This action is invalid.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::ValidateAction
//
//	PURPOSE:	Return true if this action is valid to continue running.
//
// ----------------------------------------------------------------------- //

bool CAIActionAbstract::ValidateAction( CAI* pAI )
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
//	ROUTINE:	CAIActionAbstract::CanSolvePlanWS
//
//	PURPOSE:	Return true if this action can potentially solve 
//              any of the unsatisfied properties of the plan's goal 
//              state.
//
//              This is used by the planner to find actions
//              that are neighbors of the current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionAbstract::CanSolvePlanWS( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal )
{
	SAIWORLDSTATE_PROP* pEffect;
	SAIWORLDSTATE_PROP* pPropCur;
	SAIWORLDSTATE_PROP* pPropGoal;

	// Iterate over all effects.

	for( unsigned int iEffect=0; iEffect < kWSK_Count; ++iEffect )
	{
		// Get a pointer to the indexed effect.

		pEffect = m_wsWorldStateEffects.GetWSProp( iEffect );
		if( !pEffect )
		{
			continue;
		}

		// Determine if the effect matches any of the world state properties
		// that are different in the current and goal world states.

		pPropCur = wsWorldStateCur.GetWSProp( pEffect->eWSKey, pAI->m_hObject );
		pPropGoal = wsWorldStateGoal.GetWSProp( pEffect->eWSKey, pAI->m_hObject );

		if( pPropCur && pPropGoal )
		{
			if( ( pPropCur->hWSValue != pPropGoal->hWSValue ) ||
				( pPropCur->eWSType != pPropGoal->eWSType ) )
			{
				return true;
			}
		}
	}

	// This action cannot satisfy any of the 
	// unmatched world state properties.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::SolvePlanWSVariable
//
//	PURPOSE:	Solve properties of the world state that need to 
//              be satisfied that match the action's effects.
//
// ----------------------------------------------------------------------- //

void CAIActionAbstract::SolvePlanWSVariable( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal )
{
	SAIWORLDSTATE_PROP* pEffect;
	SAIWORLDSTATE_PROP* pProp;

	// Iterate over all effects.

	for( unsigned int iEffect=0; iEffect < kWSK_Count; ++iEffect )
	{
		// Get a pointer to the indexed effect.

		pEffect = m_wsWorldStateEffects.GetWSProp( iEffect );
		if( !pEffect )
		{
			continue;
		}
		
		// The effect is not a variable.
		// Get the value of the property effected.

		if( pEffect->eWSType != kWST_Variable )
		{
			pProp = wsWorldStateGoal.GetWSProp( pEffect->eWSKey, pAI->m_hObject );
		}

		// The effect is a variable.
		// Get the value of the property referenced by the variable effected.

		else {
			pProp = wsWorldStateGoal.GetWSProp( pEffect->eVariableWSKey, pAI->m_hObject );
		}

		// Set the current world state's property to match the goal.

		if( pProp )
		{
			// Note that we may set the prop to a different key, as specified by a variable.

			AIASSERT( pProp->eWSType != kWST_Variable, pAI->m_hObject, "CAIActionAbstract::SolvePlanWSVariable: Setting a world prop to another variable!" );
			wsWorldStateCur.SetWSProp( pEffect->eWSKey, pProp );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::ApplyWSEffect
//
//	PURPOSE:	Apply effects of the action to the current world state.
//
// ----------------------------------------------------------------------- //

void CAIActionAbstract::ApplyWSEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	if( !( pwsWorldStateCur && pwsWorldStateGoal ) )
	{
		return;
	}

	SAIWORLDSTATE_PROP* pEffect;
	SAIWORLDSTATE_PROP* pProp;

	// Iterate over all effects.

	for( unsigned int iEffect=0; iEffect < kWSK_Count; ++iEffect )
	{
		// Get a pointer to the indexed effect.

		pEffect = m_wsWorldStateEffects.GetWSProp( iEffect );
		if( !pEffect )
		{
			continue;
		}

		// The effect is not a variable.
		// Set the effect on the current world state.

		if( pEffect->eWSType != kWST_Variable )
		{
			pwsWorldStateCur->SetWSProp( pEffect->eWSKey, pAI->m_hObject, pEffect->eWSType, pEffect->nWSValue );
		}

		// The effect is a variable.
		// Lookup the value of the effect's variable, and set the effect
		// on the current world state.

		else {
			pProp = pwsWorldStateGoal->GetWSProp( pEffect->eVariableWSKey, pAI->m_hObject );
			if( pProp )
			{
				pwsWorldStateCur->SetWSProp( pEffect->eWSKey, pAI->m_hObject, pProp->eWSType, pProp->nWSValue );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::GetActionCost
//
//	PURPOSE:	Return the cost of taking this action.
//
// ----------------------------------------------------------------------- //

float CAIActionAbstract::GetActionCost()
{
	return m_pActionRecord->fActionCost;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::GetActionPrecedence
//
//	PURPOSE:	Return the relative precedence of this action.
//
// ----------------------------------------------------------------------- //

float CAIActionAbstract::GetActionPrecedence()
{
	return m_pActionRecord->fActionPrecedence;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::IsActionInterruptible
//
//	PURPOSE:	Return true if the action is interruptable.
//
// ----------------------------------------------------------------------- //

bool CAIActionAbstract::IsActionInterruptible(CAI* pAI)
{
	return m_pActionRecord->bActionIsInterruptible;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::GetActionProbability
//
//	PURPOSE:	Return the probability of taking this action.
//
// ----------------------------------------------------------------------- //

float CAIActionAbstract::GetActionProbability( CAI* pAI )
{
	return m_pActionRecord->fActionProbability;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAbstract::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAbstract::ActivateAction( CAI* pAI, CAIWorldState& /*wsWorldStateGoal*/ )
{
	// If an awareness is specified on activating this action, apply this 
	// awareness.

	if ( pAI && kAware_Invalid != m_pActionRecord->eAwareness )
	{
		pAI->GetAIBlackBoard()->SetBBAwareness( m_pActionRecord->eAwareness );
	}
}
