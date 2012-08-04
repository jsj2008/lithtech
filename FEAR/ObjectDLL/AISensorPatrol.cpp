// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPatrol.cpp
//
// PURPOSE : AISensorPatrol class implementation
//
// CREATED : 2/18/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AISensorPatrol.h

#include "Stdafx.h"
#include "AISensorAbstract.h"
#include "AISensorPatrol.h"

// Includes required for AISensorPatrol.cpp

#include "AI.h"
#include "AIWorkingMemory.h"
#include "AINodeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPatrol, kSensor_Patrol );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPatrol::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorPatrol::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Bail immediately if no nodes of this type exist in the level.

	AINODE_LIST* pNodeList = g_pAINodeMgr->GetNodeList( kNode_Patrol );
	if( !( pNodeList && pNodeList->size() ) )
	{
		return false;
	}

	// Bail if we already have a memory of a patrol node.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Node);
	factQuery.SetNodeType(kNode_Patrol);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		return false;
	}

	// Find an unowned patrol node if we don't have one.

	AINodePatrol* pPatrolNode = (AINodePatrol*)g_pAINodeMgr->FindNearestNodeInRadius( m_pAI, kNode_Patrol, m_pAI->GetPosition(), NODEMGR_MAX_SEARCH, true );
	if( pPatrolNode )
	{
		// Create a memory for the patrol node.

		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Node );
		if( pFact )
		{
			pFact->SetNodeType( kNode_Patrol, 1.f );
			pFact->SetTargetObject( pPatrolNode->m_hObject, 1.f );
			pFact->SetPos( pPatrolNode->GetPos(), 1.f );
		}

		SetPatrolNode( pPatrolNode );
	}

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
//	ROUTINE:	CAISensorPatrol::SetPatrolNode
//
//	PURPOSE:	Set the current patrol path.
//
// ----------------------------------------------------------------------- //

void CAISensorPatrol::SetPatrolNode( AINodePatrol* pNodePatrol )
{
	// Unlock any previous path.

	AINodePatrol* pNodePatrolPrev = (AINodePatrol*)g_pAINodeMgr->FindOwnedNode( kNode_Patrol, m_pAI->m_hObject );
	if( pNodePatrolPrev )
	{
		pNodePatrolPrev->ClaimPatrolPath( m_pAI, false );
	}

	// Lock new path, if not already the owner.

	if( pNodePatrol && ( pNodePatrol->GetNodeOwner() != m_pAI->m_hObject ) )
	{
		pNodePatrol->ClaimPatrolPath( m_pAI, true );
	}
}
