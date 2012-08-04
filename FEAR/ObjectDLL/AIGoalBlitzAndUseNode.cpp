// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalBlitzAndUseNode.cpp
//
// PURPOSE : Contains the definition of the 'blitz and use node' goal.  
//			 This is similar to AIGoalBlitzNode.  The difference is what
//			 this goal additionally requires the AI to use the the 
//			 smartobject node instead of just arriving at its position.
//
// CREATED : 2/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalBlitzAndUseNode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalBlitzAndUseNode, kGoal_BlitzAndUseNode );

static void ClearBlitzUseNodeTask( CAI* pAI, bool bReportError )
{
	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_BlitzUseNode );
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
//	ROUTINE:	CAIGoalBlitzAndUseNode::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalBlitzAndUseNode::CAIGoalBlitzAndUseNode() :
	m_eActiveGoalContext( kContext_Invalid )
{
}

CAIGoalBlitzAndUseNode::~CAIGoalBlitzAndUseNode()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzAndUseNode::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalBlitzAndUseNode
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzAndUseNode::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BYTE_CAST( m_eActiveGoalContext, EnumAIContext );
}

void CAIGoalBlitzAndUseNode::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BYTE( m_eActiveGoalContext );
}

HOBJECT CAIGoalBlitzAndUseNode::FindBestNode( EnumAINodeType eNodeType )
{
	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Task );
	queryFact.SetTaskType( kTask_BlitzUseNode );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact )
	{
		return pFact->GetTargetObject();
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzAndUseNode::ActivateGoal()
//              
//	PURPOSE:	Activates the goal
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzAndUseNode::ActivateGoal()
{
	super::ActivateGoal();

	// Set the AIContext.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Task );
	queryFact.SetTaskType( kTask_BlitzUseNode );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		AIASSERT( pFact, m_pAI->GetHOBJECT(), "CAIGoalBlitzAndUseNode::ActivateGoal : Failed to find the fact associated with the selected task." );
		return;
	}

	m_eActiveGoalContext = (EnumAIContext)pFact->GetIndex();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzAndUseNode::DeactivateGoal()
//              
//	PURPOSE:	Deactivates the goal
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzAndUseNode::DeactivateGoal()
{
	super::DeactivateGoal();

	// Clear the context to insure its value doesn't leak from one activation
	// to the next.

	m_eActiveGoalContext = kContext_Invalid;

	bool kbFailureIsError = false;
	ClearBlitzUseNodeTask( m_pAI, kbFailureIsError );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalGoto::HandleBuildPlanFailure()
//              
//	PURPOSE:	Deactivates the goal
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzAndUseNode::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	bool kbFailureIsError = true;
	ClearBlitzUseNodeTask( m_pAI, kbFailureIsError );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzAndUseNode::OnGetContext()
//              
//	PURPOSE:	Returns the Context that is in effect during the execution 
//				of the goal.
//              
//----------------------------------------------------------------------------

EnumAIContext CAIGoalBlitzAndUseNode::OnGetContext() const
{
	return m_eActiveGoalContext;
}
