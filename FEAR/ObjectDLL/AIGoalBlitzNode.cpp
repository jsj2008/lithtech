// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalBlitzNode.cpp
//
// PURPOSE :	Contains the definition of the BlitzNode goal.  This goal 
//				handles scripted movement to nodes using a particular 
//				movement animation.
//
// CREATED : 2/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalBlitzNode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalBlitzNode, kGoal_BlitzGotoNode );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBlitzNode::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalBlitzNode::CAIGoalBlitzNode() :
	m_eActiveGoalContext( kContext_Invalid )
{
	m_eTaskType = kTask_BlitzNode;
	m_bClearScriptedTaskIfThreatened = false;
}

CAIGoalBlitzNode::~CAIGoalBlitzNode()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzNode::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalBlitzNode
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzNode::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BYTE_CAST( m_eActiveGoalContext, EnumAIContext );
}

void CAIGoalBlitzNode::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BYTE( m_eActiveGoalContext );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzNode::ActivateGoal
//              
//	PURPOSE:	Activates the goal
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzNode::ActivateGoal()
{
	super::ActivateGoal();

	// Set the AIContext.

	CAIWMFact queryFact;
	queryFact.SetFactID( m_NodeCurrent.eFactID );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		AIASSERT( pFact, m_pAI->GetHOBJECT(), "CAIGoalBlitzNode::ActivateGoal : Failed to find the fact associated with the selected task." );
		return;
	}

	m_eActiveGoalContext = (EnumAIContext)pFact->GetIndex();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzNode::DeactivateGoal
//              
//	PURPOSE:	Deactivates the goal
//              
//----------------------------------------------------------------------------

void CAIGoalBlitzNode::DeactivateGoal()
{
	// Clear the task when the goal deactivates.  The base class only does 
	// this on a successful deactivation

	ClearGotoTask( &m_NodeCurrent );

	super::DeactivateGoal();

	// Clear the context to insure its value doesn't leak from one activation
	// to the next.

	m_eActiveGoalContext = kContext_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBlitzNode::OnGetContext
//              
//	PURPOSE:	Returns the Context that is in effect during the execution 
//				of the goal.
//              
//----------------------------------------------------------------------------

EnumAIContext CAIGoalBlitzNode::OnGetContext() const
{
	return m_eActiveGoalContext;
}
