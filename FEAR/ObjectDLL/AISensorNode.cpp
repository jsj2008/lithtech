// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNode.cpp
//
// PURPOSE : AISensorNode class implementation
//
// CREATED : 2/27/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorNode.h"
#include "AI.h"
#include "AIDB.h"
#include "AIWorkingMemory.h"
#include "AINodeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNode, kSensor_Node );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNode::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorNode::CAISensorNode()
{
	m_fSearchMult = 1.f;
	m_cNodesFound = 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorNode::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorNode 
//              
//----------------------------------------------------------------------------
void CAISensorNode::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT(m_fSearchMult);
	SAVE_DWORD( m_cNodesFound );
}

void CAISensorNode::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	
	LOAD_FLOAT(m_fSearchMult);
	LOAD_DWORD( m_cNodesFound );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNode::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorNode::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Bail immediately if no nodes of this type exist in the level.

	AINODE_LIST* pNodeList = g_pAINodeMgr->GetNodeList( m_pSensorRecord->eNodeType );
	if( !( pNodeList && pNodeList->size() ) )
	{
		return false;
	}

	// Clear existing memory of nodes.

	if( m_cNodesFound > 0 )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Node);
		factQuery.SetNodeType(m_pSensorRecord->eNodeType);
		m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
	}

	// Get a list of valid nodes.
	// Re-use static memory for list of nodes.
	// Note: In our implementation of STL, resize(0) does 
	// not free allocated memory, but clear() does.

	static AIVALID_NODE_LIST lstValidNodes;
	lstValidNodes.resize( 0 );
	SearchForNodes( lstValidNodes );

	// Create memories in AIWorkingMemory.

	m_cNodesFound = lstValidNodes.size();
	CreateNodeMemories( lstValidNodes );

	// Allow other sensors to update if there was an insignificant
	// number of nodes.

	if( pNodeList->size() < 10 )
	{
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNode::SearchForNodes
//
//	PURPOSE:	Generate a list of potentially valid nodes.
//
// ----------------------------------------------------------------------- //

void CAISensorNode::SearchForNodes( AIVALID_NODE_LIST& lstValidNodes )
{
	uint32 dwNodeValid = kNodePotential_RadiusOrRegion | 
						 kNodePotential_HasPathToNode | 
						 kNodePotential_NodeInNavMesh |
						 kNodePotential_NodeInGuardedArea |
						 kNodePotential_NodeUnowned;

	SAIVALIDATE_NODE ValidateNodeStruct;
	ValidateNodeStruct.pAI = m_pAI;
	ValidateNodeStruct.vPos = m_pAI->GetPosition();
	ValidateNodeStruct.fSearchMult = m_fSearchMult;
	ValidateNodeStruct.dwPotentialFlags = dwNodeValid;

	g_pAINodeMgr->FindPotentiallyValidNodes( m_pSensorRecord->eNodeType, 
											 lstValidNodes, 
											 ValidateNodeStruct );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNode::CreateNodeMemories
//
//	PURPOSE:	Create facts in AIWorkingMemory for sensed nodes.
//
// ----------------------------------------------------------------------- //

void CAISensorNode::CreateNodeMemories( AIVALID_NODE_LIST& lstValidNodes )
{
	// Iterate over all nodes, searching for the node that is the furthest away.

	float fMax = 0.f;
	SAIVALID_NODE* pValidNode;
	AIVALID_NODE_LIST::iterator itNode;
	for( itNode = lstValidNodes.begin(); itNode != lstValidNodes.end(); ++itNode )
	{
		pValidNode = &*itNode;
		if( pValidNode->fDistSqr > fMax )
		{
			fMax = pValidNode->fDistSqr;
		}
	}
	fMax += 0.1f;


	// Create memories for each node.
	// The confidence value is the relative to the distance to the node.

	AssignNodeConfidenceValues( lstValidNodes, fMax );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNode::AssignNodeConfidenceValues
//
//	PURPOSE:	Assign confidence values to nodes.
//
// ----------------------------------------------------------------------- //

void CAISensorNode::AssignNodeConfidenceValues( AIVALID_NODE_LIST& lstValidNodes, float fMaxDistSqr )
{
	// Create memories for each node.
	// The confidence value is the relative to the distance to the node.

	AINode* pNode;
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

		pNode = (AINode*)g_pLTServer->HandleToObject( pValidNode->hNode );
		if( pNode )
		{
			fConfidence = ( fMaxDistSqr - pValidNode->fDistSqr ) / fMaxDistSqr;
			pFact->SetPos( pNode->GetPos(), fConfidence );
		}
	}
}

