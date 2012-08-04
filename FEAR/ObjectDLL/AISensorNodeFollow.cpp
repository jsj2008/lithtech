// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeFollow.cpp
//
// PURPOSE : AISensorNodeFollow class implementation
//
// CREATED : 07/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorNodeFollow.h"
#include "AI.h"
#include "AIDB.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeFollow, kSensor_NodeFollow );


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorNodeFollow::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorNode 
//              
//----------------------------------------------------------------------------
void CAISensorNodeFollow::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAISensorNodeFollow::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeFollow::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorNodeFollow::UpdateSensor()
{
	// Bail if we have no Follow task.

	CAIWMFact factTaskQuery;
	factTaskQuery.SetFactType( kFact_Task );
	factTaskQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factTaskQuery );
	if( !pFact )
	{
		// Clear existing memory of nodes.

		if( m_cNodesFound > 0 )
		{
			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Node);
			factQuery.SetNodeType(m_pSensorRecord->eNodeType);
			m_pAI->GetAIWorkingMemory()->ClearWMFacts( factQuery );
			m_cNodesFound = 0;
		}

		return false;
	}

	// Default behavior.

	if( !super::UpdateSensor() )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorNodeFollow::SearchForNodes
//
//	PURPOSE:	Generate a list of potentially valid nodes.
//
// ----------------------------------------------------------------------- //

void CAISensorNodeFollow::SearchForNodes( AIVALID_NODE_LIST& lstValidNodes )
{
	// Bail if we are not following anyone.

	CAIWMFact factTaskQuery;
	factTaskQuery.SetFactType( kFact_Task );
	factTaskQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factTaskQuery );
	if( !pFact )
	{
		return;
	}

	// Find nodes containing the leader in their radius or AIRegion.

	uint32 dwNodeValid = kNodePotential_HasPathToNode | 
						 kNodePotential_NodeInNavMesh |
						 kNodePotential_NodeInGuardedArea |
						 kNodePotential_CharacterInRadiusOrRegion |
						 kNodePotential_NodeUnowned;

	SAIVALIDATE_NODE ValidateNodeStruct;
	ValidateNodeStruct.pAI = m_pAI;
	ValidateNodeStruct.vPos = m_pAI->GetPosition();
	ValidateNodeStruct.fSearchMult = m_fSearchMult;
	ValidateNodeStruct.dwPotentialFlags = dwNodeValid;
	ValidateNodeStruct.hChar = pFact->GetSourceObject();

	g_pAINodeMgr->FindPotentiallyValidNodes( m_pSensorRecord->eNodeType, 
											 lstValidNodes, 
											 ValidateNodeStruct );
}

