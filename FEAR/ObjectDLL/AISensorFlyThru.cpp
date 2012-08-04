// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorFlyThru.cpp
//
// PURPOSE : AISensorFlyThru class implementation
//
// CREATED : 10/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorFlyThru.h"
#include "AIPathMgrNavMesh.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorFlyThru, kSensor_FlyThru );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlyThru::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorFlyThru::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}
	
	// Look for a FlyThru link if we do not already know our next link.

	if( m_pAI->GetAIBlackBoard()->GetBBNextNMLink() == kNMLink_Invalid )
	{
		FindNextFlyThruLink();
	}

	// Bail if no link on our path.

	ENUM_NMLinkID eLink = m_pAI->GetAIBlackBoard()->GetBBNextNMLink();
	if( eLink == kNMLink_Invalid )
	{
		return false;
	}

	// Bail if link is not a FlyThru link.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( eLink );
	if( !( pLink && pLink->GetNMLinkType() == kLink_FlyThru ) )
	{
		return false;
	}

	// Bail if poly is invalid.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pLink->GetNMPolyID() );
	if( !pPoly )
	{
		return false;
	}

	// Immediately re-evaluate goals if we are within the offset entry of a FlyThru link.

	float fOffset = pLink->GetNMLinkOffsetEntryDistA();
	LTVector vLookAhead = m_pAI->GetPosition() + ( m_pAI->GetForwardVector() * fOffset );
	if( pPoly->RayIntersectPoly2D( m_pAI->GetPosition(), vLookAhead, NULL ) )
	{
		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}

	// Always allow other sensors to update.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorFlyThru::FindNextFlyThruLink
//
//	PURPOSE:	Find the next FlyThru link on our path.
//
// ----------------------------------------------------------------------- //

void CAISensorFlyThru::FindNextFlyThruLink()
{
	// No path set.

	if( m_pAI->GetAIBlackBoard()->GetBBDestStatus() != kNav_Set )
	{
		return;
	}

	// No path exists.

	CAIPathNavMesh* pNMPath = m_pAI->GetAINavigationMgr()->GetNMPath();
	if( !pNMPath )
	{
		return;
	}

	// Not in the NavMesh!

	ENUM_NMPolyID eCurPoly = m_pAI->GetCurrentNavMeshPoly();
	if( eCurPoly == kNMPoly_Invalid )
	{
		return;
	}

	// Walk the path from our current position, searching for the next link.

	SPATH_NODE* pPathNode;
	bool bFoundCur = false;
	CAINavMeshPoly* pPoly;
	AINavMeshLinkAbstract* pLink = NULL;
	uint32 cPathNodes = pNMPath->GetPathLength();
	for( uint32 iNode=0; iNode < cPathNodes; ++iNode )
	{
		// Skip invalid nodes.

		pPathNode = pNMPath->GetPathNode( iNode );
		if( !pPathNode )
		{
			continue;
		}

		// First find our current node.

		if( !bFoundCur )
		{
			if( pPathNode->ePoly == eCurPoly )
			{
				bFoundCur = true;
			}
			continue;
		}

		// Next look for a link.

		pPoly = g_pAINavMesh->GetNMPoly( pPathNode->ePoly );
		if( ( !pPoly ) || 
			( pPoly->GetNMLinkID() == kNMLink_Invalid ) )
		{
			continue;
		}

		// Stop searching once a link is found.

		pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
		break;
	}

	// If the next link on our path is a FlyThru link, record it on
	// the blackboard.

	if( pLink && pLink->GetNMLinkType() == kLink_FlyThru )
	{
		m_pAI->GetAIBlackBoard()->SetBBNextNMLink( pLink->GetNMLinkID() );
	}
}
