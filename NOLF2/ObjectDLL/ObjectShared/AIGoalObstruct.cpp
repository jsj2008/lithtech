//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalObstruct.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	11.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIGOALOBSTRUCT_H__
#include "AIGoalObstruct.h"		
#endif

#include "AI.h"
#include "AIGoalButeMgr.h"
#include "AIGoalMgr.h"
#include "AIState.h"
#include "AINodeMgr.h"
#include "AIHumanStateObstruct.h"

// Forward declarations

// Globals

// Statics

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalObstruct, kGoal_Obstruct);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalObstruct::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //
void CAIGoalObstruct::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hNodeObstruct );
	SAVE_HOBJECT( m_hNodeObstructFailed );
}

void CAIGoalObstruct::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hNodeObstruct );
	LOAD_HOBJECT( m_hNodeObstructFailed );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalObstruct::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalObstruct::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(GetObstructNode() != NULL, m_pAI->m_hObject, "CAIGoalObstruct::ActivateGoal: NodeObstruct is NULL.");

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags(kSense_SeeEnemy);

	m_pGoalMgr->LockGoal(this);

	m_pAI->SetState(kState_HumanObstruct);

	CAIHumanStateObstruct* pObstruct = (CAIHumanStateObstruct*)m_pAI->GetState();
	pObstruct->SetNode( *(GetObstructObject()) );
	pObstruct->SetObjectToObstruct( GetLastStimulusSource() );
	pObstruct->SetAcceptableDistanceToNode( 20.0 );

	// Clear the stimulus source
	SetLastStimulusSource( NULL );
}

AINodeObstruct* CAIGoalObstruct::GetObstructObject(void)
{
	return (AINodeObstruct*)g_pLTServer->HandleToObject(GetObstructNode());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalObstruct::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //
void CAIGoalObstruct::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanObstruct:
			HandleStateObstruct();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalObstruct::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalObstruct::HandleStateCover
//
//	PURPOSE:	Determine what to do when in state Cover.
//
// ----------------------------------------------------------------------- //
void CAIGoalObstruct::HandleStateObstruct()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Moving:
			break;

		case kSStat_Holding:
			
			break;

		case kSStat_StateComplete:
			SetObstructNode( NULL );
			m_pGoalMgr->UnlockGoal(this);
			m_fCurImportance = 0.f;
			break;

		case kSStat_FailedComplete:
			SetFailedNode( GetObstructNode() );
			SetObstructNode( NULL );
			m_pGoalMgr->UnlockGoal(this);
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalObstruct::HandleStateCover: Unexpected State Status.");
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalObstruct::GetAttractorCount()
//              
//	PURPOSE:	Returns the number of attractors this goal has.
//              
//----------------------------------------------------------------------------
int CAIGoalObstruct::GetAttractorCount()
{
	AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
	AIASSERT(pTemplate->cAttractors > 0, m_pAI->m_hObject, "CAIGoalObstruct::HandleGoalAttractors: Goal has no attractors.");
	return pTemplate->cAttractors;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalObstruct::HandleGoalAttractors
//
//	PURPOSE:	If we have a stimulus, and if we can aquire an obstruction
//				node, then 
//
// ----------------------------------------------------------------------- //
AINode* CAIGoalObstruct::HandleGoalAttractors()
{
	AINode* pNode = NULL;

	if( GetLastStimulusSource() != NULL && GetObstructNode() == NULL)
	{
		// Lock the failed Obstruct node, so that we don't try to use it again.
		BlockAttractorNodeFromSearch( GetFailedNode() );
		
		FindAndSetupNode( kNode_Obstruct );
		
		// Unlock the fail node so that we don't develope any locking issues
		UnblockAttractorNodeFromSearch( GetFailedNode() );
	}

	return pNode;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalObstruct::FindAndSetupNode()
//              
//	PURPOSE:	Searches for a node of the appropriate type for this action
//              
//----------------------------------------------------------------------------
AINode* CAIGoalObstruct::FindAndSetupNode(EnumAINodeType kNodeType)
{
	AINode* pNode;

	// Look for a obstruct node that obstructs from the threat.
	// If one is found, this goal activates.
	for(uint32 iAttractor=GetAttractorCount(); iAttractor > 0; --iAttractor)
	{
		pNode = g_pAINodeMgr->FindNearestNodeInSameDirectionAsThreat(m_pAI, kNodeType, 
			m_pAI->GetPosition(), GetLastStimulusSource());

		if (pNode != NULL)	
		{
			SetObstructNode( pNode->m_hObject );

			SetCurToBaseImportance();
			m_pAI->Target( GetLastStimulusSource() );
		}
		break;
	}

	return pNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalObstruct::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //
LTBOOL CAIGoalObstruct::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if(super::HandleGoalSenseTrigger(pSenseRecord))
	{
		SetLastStimulusSource(pSenseRecord->hLastStimulusSource);
	}

	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalObstruct::SetLastStimulusSource()
//              
//	PURPOSE:	Remember who stimulated us last (time stamp this so that
//				old stimulii don't enable goals inappropriately?)  A NULL node
//				is legal.
//              
//----------------------------------------------------------------------------
void CAIGoalObstruct::SetLastStimulusSource(HOBJECT hStim)
{
	m_hStimulusSource = hStim;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalObstruct::SetLastStimulus()
//              
//	PURPOSE:	Retreive the last stimulus.  This may be null (indicating
//				none)
//              
//----------------------------------------------------------------------------
HOBJECT	CAIGoalObstruct::GetLastStimulusSource(void)
{
	return m_hStimulusSource;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalObstruct::SetObstructNode()
//              
//	PURPOSE:	Unlink the old node, remember the object, and link to it
//              
//----------------------------------------------------------------------------
void CAIGoalObstruct::SetObstructNode(HOBJECT hNode)
{
	m_hNodeObstruct = hNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalObstruct::SetObstructNode()
//              
//	PURPOSE:	Retreive the ObstructNode
//              
//----------------------------------------------------------------------------
HOBJECT CAIGoalObstruct::GetObstructNode(void)
{
	return m_hNodeObstruct;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalObstruct::SetFailedNode()
//              
//	PURPOSE:	Unlink the old node, remember the object, and link to it
//              
//----------------------------------------------------------------------------
void CAIGoalObstruct::SetFailedNode(HOBJECT hNode)
{
	m_hNodeObstructFailed = hNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalObstruct::GetFailedNode()
//              
//	PURPOSE:	Retreive the ObstructNode
//              
//----------------------------------------------------------------------------
HOBJECT CAIGoalObstruct::GetFailedNode(void)
{
	return m_hNodeObstructFailed;
}
