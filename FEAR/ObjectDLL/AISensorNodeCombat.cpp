// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCover.cpp
//
// PURPOSE : AISensorNodeCombat class implementation
//
// CREATED : 3/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorNodeCombat.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "AIWorkingMemory.h"
#include "AINodeFollow.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeCombat, kSensor_NodeCombat );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeCombat::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorNodeCombat::UpdateSensor()
{
	// Search further for nodes if we have been damaged recently.

	m_fSearchMult = 1.f;

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && DidDamage( m_pAI, pFact ) )
	{
		double fCurTime = g_pLTServer->GetTime();
		if( fCurTime - pFact->GetUpdateTime() < 2.f )
		{
			m_fSearchMult = 1.5f;
		}
	}

	// Search for nodes.

	if( !super::UpdateSensor() )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeCombat::SearchForNodes
//
//	PURPOSE:	Generate a list of potentially valid nodes.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeCombat::SearchForNodes( AIVALID_NODE_LIST& lstValidNodes )
{
	// Search for potentially valid nodes around the target.

	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		uint32 dwNodeValid = kNodePotential_BoundaryRadius | 
							 kNodePotential_HasPathToNode | 
							 kNodePotential_NodeInNavMesh |
							 kNodePotential_NodeInGuardedArea |
							 kNodePotential_NodeUnowned;

		SAIVALIDATE_NODE ValidateNodeStruct;
		ValidateNodeStruct.pAI = m_pAI;
		ValidateNodeStruct.vPos = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
		ValidateNodeStruct.fSearchMult = 1.f;
		ValidateNodeStruct.dwPotentialFlags = dwNodeValid;

		g_pAINodeMgr->FindPotentiallyValidNodes( m_pSensorRecord->eNodeType, 
												 lstValidNodes, 
												 ValidateNodeStruct );
	}

	// Default behavior.

	else 
	{
		super::SearchForNodes( lstValidNodes );
	}

	// Filter node list if currently following someone.

	if( ( !lstValidNodes.empty() ) &&
		( m_pAI->GetAIBlackBoard()->GetBBTargetedTypeMask() & kTarget_Leader ) )
	{
		FilterNodesValidForFollow( lstValidNodes );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeCombat::FilterNodesValidForFollow
//
//	PURPOSE:	Filter the list to only include nodes valid for 
//              following the current leader.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeCombat::FilterNodesValidForFollow( AIVALID_NODE_LIST& lstValidNodes )
{
	// This is kindof a hack to be filtering nodes for follwing inside of this sensor.
	// Maybe eventually this could be moved into a separate sensor that updates every
	// frame, but knows to only really do anything when nodes of some types have been
	// refreshed.

	// Bail if we are not following anyone.

	CAIWMFact factTaskQuery;
	factTaskQuery.SetFactType( kFact_Task );
	factTaskQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factTaskQuery );
	if( !pFact )
	{
		return;
	}

	static AINODE_LIST lstFollowNodes;
	lstFollowNodes.resize( 0 );

	// Collect known follow nodes.

	AINodeFollow* pNodeFollow;
	AIWORKING_MEMORY_FACT_LIST::const_iterator itFact;
	const AIWORKING_MEMORY_FACT_LIST* pFactList = m_pAI->GetAIWorkingMemory()->GetFactList();
	for( itFact = pFactList->begin(); itFact != pFactList->end(); ++itFact )
	{
		// Ignore deleted facts.

		pFact = *itFact;
		if( pFact->IsDeleted() )
		{
			continue;
		}

		if( ( pFact->GetFactType() == kFact_Node ) &&
			( pFact->GetNodeType() == kNode_Follow ) )
		{
			pNodeFollow = (AINodeFollow*)g_pLTServer->HandleToObject( pFact->GetTargetObject() );
			if( pNodeFollow )
			{
				lstFollowNodes.push_back( pNodeFollow );
			}
		}
	}

	// If we are standing at a node of the correct type,
	// add it to the list of potentially valid nodes.
	// Ordinarily, locked nodes are ommitted, but in the case of 
	// following we need to consider the node we are at valid.

	SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( pProp->hWSValue );
		if( pNode && 
			( pNode->GetLockingAI() == m_pAI->m_hObject ) &&
			( pNode->GetType() == m_pSensorRecord->eNodeType ) )
		{
			SAIVALID_NODE ValidNode;
			ValidNode.hNode = pNode->m_hObject;
			ValidNode.fDistSqr = 0.f;

			lstValidNodes.push_back( ValidNode );
		}
	}


	// Remove nodes from list that are not valid for follow.

	HOBJECT hNode;
	bool bNodeIsValidForFollow;
	AINODE_LIST::iterator itNode;
	AIVALID_NODE_LIST::iterator itValidNode = lstValidNodes.begin();
	while( itValidNode != lstValidNodes.end() )
	{
		hNode = itValidNode->hNode;

		// Find the node listed in a follow node's list.

		bNodeIsValidForFollow = false;
		for( itNode = lstFollowNodes.begin(); itNode != lstFollowNodes.end(); ++itNode )
		{
			pNodeFollow = (AINodeFollow*)(*itNode);
			if( pNodeFollow &&
				pNodeFollow->GetWaitingNodes() &&
				pNodeFollow->GetWaitingNodes()->DoesContain( hNode ) )
			{
				bNodeIsValidForFollow = true;
				break;
			}
		}

		// Remove invalid node from list.

		if( !bNodeIsValidForFollow )
		{
			itValidNode = lstValidNodes.erase( itValidNode );
			continue;
		}

		// Continue iterating over list.

		++itValidNode;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNode::AssignNodeConfidenceValues
//
//	PURPOSE:	Assign confidence values to nodes.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeCombat::AssignNodeConfidenceValues( AIVALID_NODE_LIST& lstValidNodes, float fMaxDistSqr )
{
	// Special-case cover nodes to handle dependencies.

	if( m_pSensorRecord->eNodeType != kNode_Cover )
	{
		super::AssignNodeConfidenceValues( lstValidNodes, fMaxDistSqr );
		return;
	}

	// Create memories for each node.
	// The confidence value is the relative to the distance to the node.

	AINodeCover* pNodeCover;
	CAIWMFact* pFact;
	float fConfidence;
	SAIVALID_NODE* pValidNode;
	AIVALID_NODE_LIST::iterator itNode;
	for( itNode = lstValidNodes.begin(); itNode != lstValidNodes.end(); ++itNode )
	{
		pValidNode = &*itNode;
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Node );

		pFact->SetNodeType( m_pSensorRecord->eNodeType, 1.f );
		pFact->SetTargetObject( pValidNode->hNode, 1.f );

		pNodeCover = (AINodeCover*)g_pLTServer->HandleToObject( pValidNode->hNode );
		if( !pNodeCover )
		{
			continue;
		}

		// Make nodes that depend on occupied dependencies appear more attractive
		// by artificially boosting the confidence value to 1.0.

		if( pNodeCover->GetDependency() && IsAINodeSmartObject( pNodeCover->GetDependency() ) )
		{
			AINodeSmartObject* pNodeSmartObject = (AINodeSmartObject*)g_pLTServer->HandleToObject( pNodeCover->GetDependency() );
			if( pNodeSmartObject->GetSmartObject() &&
				pNodeSmartObject->GetSmartObject()->eDependencyType == kDependency_Occupied )
			{
				pFact->SetPos( pNodeCover->GetPos(), 1.f );
				continue;
			}
		}

		fConfidence = ( fMaxDistSqr - pValidNode->fDistSqr ) / fMaxDistSqr;
		pFact->SetPos( pNodeCover->GetPos(), fConfidence );
	}
}





