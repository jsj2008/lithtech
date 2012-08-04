// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorWork.cpp
//
// PURPOSE : AISensorNodeGuarded class implementation
//
// CREATED : 10/09/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorNodeGuarded.h"
#include "AI.h"
#include "AIDB.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeGuarded, kSensor_NodeGuarded );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeGuarded::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorNodeGuarded::CAISensorNodeGuarded()
{
	m_hLastGuardNode = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorNodeGuarded::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorNodeGuarded and all 
//				of its sensors.
//              
//----------------------------------------------------------------------------
void CAISensorNodeGuarded::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT(m_hLastGuardNode);
}

void CAISensorNodeGuarded::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT(m_hLastGuardNode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeGuarded::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorNodeGuarded::UpdateSensor()
{
	// Intentionally skip super class' UpdateSensor().
	// Behavior varies depending on presence of Guard node.

	if( !CAISensorAbstract::UpdateSensor() )
	{
		return false;
	}

	// Bail immediately if no nodes of this type exist in the level.

	AINODE_LIST* pNodeList = g_pAINodeMgr->GetNodeList( m_pSensorRecord->eNodeType );
	if( !( pNodeList && pNodeList->size() ) )
	{
		return false;
	}

	// Re-use static memory for list of nodes.

	static AIVALID_NODE_LIST lstValidNodes;
	lstValidNodes.resize( 0 );

	// AI is guarding a Guard node.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Guard);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact && pFact->GetTargetObject() )
	{	
		// Guard node has not changed, so bail because no further sensing 
		// needs to occur. We have previously sensed the list of Guarded nodes.

		if( pFact->GetTargetObject() == m_hLastGuardNode )
		{
			return false;
		}

		// Record new Guard node.

		m_hLastGuardNode = pFact->GetTargetObject();

		// Clear existing memory of nodes.
		// Note: In our implementation of STL, resize(0) does 
		// not free allocated memory, but clear() does.

		if( m_cNodesFound > 0 )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Node);
			factQuery.SetNodeType(m_pSensorRecord->eNodeType);
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
		}

		// Get a list of valid nodes.

		g_pAINodeMgr->FindGuardedNodes( m_pAI, m_pSensorRecord->eNodeType, lstValidNodes );
	}

	// AI is NOT guarding a Guard node.

	else {

		// Clear existing memory of nodes.
		// Note: In our implementation of STL, resize(0) does 
		// not free allocated memory, but clear() does.

		if( m_cNodesFound > 0 )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Node);
			factQuery.SetNodeType(m_pSensorRecord->eNodeType);
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
		}

		// Get a list of valid nodes.

		uint32 dwNodeValid = kNodePotential_RadiusOrRegion | 
							 kNodePotential_HasPathToNode | 
							 kNodePotential_NodeInNavMesh |
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

