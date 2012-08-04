// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalBerserker.cpp
//
// PURPOSE : 
//
// CREATED : 8/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalBerserker.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIWorkingMemoryCentral.h"
#include "AIGoalMgr.h"

LINKFROM_MODULE(AIGoalBerserker);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalBerserker, kGoal_Berserker );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBerserker::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalBerserker::CAIGoalBerserker()
{
}

CAIGoalBerserker::~CAIGoalBerserker()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalBerserker::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalBerserker
//              
//----------------------------------------------------------------------------

void CAIGoalBerserker::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hBerserkerTarget );
}

void CAIGoalBerserker::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hBerserkerTarget );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBerserker::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalBerserker::CalculateGoalRelevance( )
{
	// Do not chase if the AI does not have a character target.

	if( !m_pAI->HasTarget( kTarget_Berserker ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// AIs which are currently mounted may not engage in berserker behavior.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->DereferenceWSProp( kWSK_MountedObject );
	if ( pProp 
		&& pProp->hWSValue 
		&& ( pProp->hWSValue != m_pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// AI is aware of a character enemy which is a player.

	if( IsPlayer( m_pAI->GetAIBlackBoard()->GetBBTargetObject() ) ) 
	{
		// Insure no other AIs are currently engaging this target with 
		// berserker behaviors.

		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
		queryFact.SetTargetObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
		CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( queryFact );
		if ( NULL == pFact || pFact->GetSourceObject() == m_pAI->GetHOBJECT() )
		{
			m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
			return;
		}
	}

	// No enemy present.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBerserker::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalBerserker::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_UsingObject, m_pAI->m_hObject, kWST_HOBJECT, m_pAI->GetAIBlackBoard()->GetBBTargetObject() ); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBerserker::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalBerserker::ActivateGoal()
{
	super::ActivateGoal();

	// Remember who this goal is targeting with Berserker, so that central
	// memory can be cleaned up.

	m_hBerserkerTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalBerserker::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalBerserker::DeactivateGoal()
{
	super::DeactivateGoal();

	// Clear the handle to the targeted object.

	m_hBerserkerTarget = NULL;
}
