// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalEscapeDanger.cpp
//
// PURPOSE : AIGoalEscapeDanger class implementation.
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalEscapeDanger.h"
#include "AI.h"
#include "AIDB.h"
#include "AIWorkingMemoryCentral.h"
#include "AINodeMgr.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalEscapeDanger, kGoal_EscapeDanger );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalEscapeDanger::CAIGoalEscapeDanger()
//
//	PURPOSE:	Handle Construction/Destruction of a CAIGoalEscapeDanger 
//				instance
//
// ----------------------------------------------------------------------- //

CAIGoalEscapeDanger::CAIGoalEscapeDanger()
{
}

CAIGoalEscapeDanger::~CAIGoalEscapeDanger()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalEscapeDanger::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalEscapeDanger::CalculateGoalRelevance()
{
	// Goal is irrelevant if we are not aware of a disturbance.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_DisturbanceExists, m_pAI->m_hObject );
	if( pProp && !pProp->bWSValue )
	{
		m_fGoalRelevance = 0.f;
		return;
	}

	// Goal is relevant if we are aware of danger.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
		return;
	}

	// We are not aware of danger.

	m_fGoalRelevance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalEscapeDanger::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalEscapeDanger::ActivateGoal()
{
	super::ActivateGoal();

	// Avoid cover nodes where grenades have landed.

	AvoidCoverNode();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalEscapeDanger::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalEscapeDanger::DeactivateGoal()
{
	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::HandleBuildPlanFailure
//
//	PURPOSE:	Handle plan construction failing to find a valid plan to
//				accomplish this goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalEscapeDanger::HandleBuildPlanFailure()
{
	super::HandleBuildPlanFailure();

	// Clear knowledge of danger.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );

	// Avoid cover nodes where grenades have landed.

	AvoidCoverNode();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalEscapeDanger::AvoidCoverNode
//
//	PURPOSE:	Avoid a cover node that is in danger.
//
// ----------------------------------------------------------------------- //

void CAIGoalEscapeDanger::AvoidCoverNode()
{
	// Avoid nodes where grenades have landed.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode )
		{
			static AINODE_LIST ClusteredNodeList;
			ClusteredNodeList.resize( 0 );

			// Find all nodes in cluster where AI died.

			if( pNode->GetAINodeClusterID() != kNodeCluster_Invalid )
			{
				g_pAINodeMgr->FindNodesInCluster( pNode->GetAINodeClusterID(), pNode->GetType(), &ClusteredNodeList );
			}
			else {
				ClusteredNodeList.push_back( pNode );
			}

			// Ensure all AI avoid all nodes in cluster.

			AINODE_LIST::iterator itNode;
			for( itNode = ClusteredNodeList.begin(); itNode != ClusteredNodeList.end(); ++itNode )
			{
				pNode = *itNode;

				// An AI has been damaged at this node.

				CAIWMFact factQuery;
				factQuery.SetFactType(kFact_Knowledge);
				factQuery.SetKnowledgeType(kKnowledge_DamagedAtNode);
				factQuery.SetTargetObject(pNode->m_hObject);

				CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
				if (!pFact)
				{
					pFact = g_pAIWorkingMemoryCentral->CreateWMFact(kFact_Knowledge);

					// The AI was not actually damaged at the node yet, but we want 
					// him to know to evacuate.

					pFact->SetDamage( DT_UNSPECIFIED, 0.f, LTVector( 0.f, 0.f, 0.f ) );
				}

				if (pFact)
				{
					float fDelay = GetRandom( g_pAIDB->GetAIConstantsRecord()->fDamagedAtNodeAvoidanceTimeMin,
											g_pAIDB->GetAIConstantsRecord()->fDamagedAtNodeAvoidanceTimeMax );

					pFact->SetKnowledgeType( kKnowledge_DamagedAtNode, 1.f );
					pFact->SetTargetObject( pNode->m_hObject, 1.f );
					pFact->SetTime( g_pLTServer->GetTime() + fDelay, 1.f );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalEscapeDanger::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalEscapeDanger::SetWSSatisfaction( CAIWorldState& WorldState )
{
	super::SetWSSatisfaction(WorldState);
	
	// Set satisfaction properties.

	WorldState.SetWSProp( kWSK_DisturbanceExists, m_pAI->m_hObject, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalEscapeDanger::IsWSSatisfied
//
//	PURPOSE:	Return true if the world state satisfies the goal.
//
// ----------------------------------------------------------------------- //

bool CAIGoalEscapeDanger::IsWSSatisfied( CAIWorldState* pwsWorldState )
{
	// Danger exists.

	SAIWORLDSTATE_PROP* pProp = pwsWorldState->GetWSProp( kWSK_DisturbanceExists, m_pAI->m_hObject );
	if( pProp && pProp->bWSValue )
	{
		return false;
	}

	// No more danger.

	return true;
}
