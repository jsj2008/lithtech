// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalBlitzCharacter.cpp
//
// PURPOSE : 
//
// CREATED : 2/26/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalBlitzCharacter.h"
#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalBlitzCharacter, kGoal_BlitzCharacter );

static void ClearBlitzCharacterTask( CAI* pAI, bool bReportError )
{
	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_BlitzCharacter );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( pFact )
	{
		// If this fact was scripted, return a warning for level designers.

		if ( bReportError && pFact->GetFactFlags() & kFactFlag_Scripted )
		{
			char szObjectName[64] = { '\0' };
			if ( LT_OK == g_pLTServer->GetObjectName( pFact->GetTargetObject(), szObjectName, LTARRAYSIZE(szObjectName) ) )
			{
				AIASSERT1( 0, pAI->GetHOBJECT(), "CAIGoalBlitzCharacter::HandleBuildPlanFailure : Failed to find a plan to blitz: %s", szObjectName );
			}
		}

		// Do not repeatedly try to blitz the character if the goal failed.

		pAI->GetAIWorkingMemory()->ClearWMFact( pFact );
	}

	// Clear the scripted target.

	// NOTE: This is bit risky.  It may have been changed since the target was 
	// set.  Unfortunately, the target object specified in the fact may have
	// been killed.  If this causes any bugs, we can address it later.

	pAI->GetAIBlackBoard()->SetBBScriptedTargetObject( NULL );

	// Update the task status.

	pAI->GetAIBlackBoard()->SetBBTaskStatus( kTaskStatus_Done );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBlitzCharacter::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalBlitzCharacter::CAIGoalBlitzCharacter() : 
	m_eActiveGoalContext( kContext_Invalid )
{
}

CAIGoalBlitzCharacter::~CAIGoalBlitzCharacter()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzCharacter::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalBlitzCharacter
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzCharacter::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BYTE_CAST( m_eActiveGoalContext, EnumAIContext );
}

void CAIGoalBlitzCharacter::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BYTE( m_eActiveGoalContext );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzCharacter::CalculateGoalRelevance()
//              
//	PURPOSE:	Calculate the current goal relevance.
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzCharacter::CalculateGoalRelevance()
{
	// If this goal is active and unsatisfied, it is relevant.

	if ( m_pAI->GetGoalMgr()->IsCurGoal( this )
		&& !IsWSSatisfied( m_pAI->GetAIWorldState() ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// If the AI has a kTask_BlitzCharacter task, this goal is relevant.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_BlitzCharacter );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( pFact 
		&& pFact->GetTargetObject() == m_pAI->GetAIBlackBoard()->GetBBTargetObject()
		&& m_pAI->HasTarget( kTarget_Character ) )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// Goal is not relevant.

	m_fGoalRelevance = 0.0f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzCharacter::ActivateGoal
//              
//	PURPOSE:	Activates the goal
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzCharacter::ActivateGoal()
{
	super::ActivateGoal();

	// Store the GoalContext.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_BlitzCharacter );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pFact )
	{
		AIASSERT( 0, m_pAI->GetHOBJECT(), "CAIGoalBlitzCharacter::ActivateGoal : Failed to find a blitz task.  Goal should not be relevant." );
		return;
	}

	m_eActiveGoalContext = (EnumAIContext)pFact->GetIndex();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzCharacter::DeactivateGoal
//              
//	PURPOSE:	Deactivates the goal
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzCharacter::DeactivateGoal()
{
	super::DeactivateGoal();

	// Clear the context to insure its value doesn't leak from one activation
	// to the next.

	m_eActiveGoalContext = kContext_Invalid;

	bool kbReportError = false;
	ClearBlitzCharacterTask( m_pAI, kbReportError );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzCharacter::DeactivateGoal
//              
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzCharacter::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	bool kbReportError = true;
	ClearBlitzCharacterTask( m_pAI, kbReportError );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzCharacter::OnGetContext
//              
//	PURPOSE:	Returns the Context that is in effect during the execution 
//				of the goal.
//              
//----------------------------------------------------------------------------

EnumAIContext CAIGoalBlitzCharacter::OnGetContext() const
{
	return m_eActiveGoalContext;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBlitzCharacter::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalBlitzCharacter::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBlitzCharacter::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalBlitzCharacter::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	SAIWORLDSTATE_PROP* pProp = NULL;

	pProp = pwsWorldState->GetWSProp( kWSK_AtTargetPos, m_pAI->m_hObject );
	if( !pProp || ( pProp && !pProp->bWSValue ) )
	{
		return false;
	}

	return true;
}
