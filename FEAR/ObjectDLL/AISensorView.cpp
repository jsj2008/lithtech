// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorView.cpp
//
// PURPOSE : AISensorView class implementation
//
// CREATED : 10/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorView.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AINavMesh.h"
#include "AIRegion.h"
#include "AINodeMgr.h"
#include "AIQuadTree.h"
#include "AIWorkingMemory.h"
#include "AIDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorView, kSensor_View );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorView::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorView::CAISensorView()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorView::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorView::UpdateSensor()
{
	// Intentionally skip super class, and call base class.
	// SensorView treats node sensing differently, due to AIRegions.

	if( !CAISensorAbstract::UpdateSensor() )
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
		m_cNodesFound = 0;
	}

	// Bail if AI is not targeting a character.

	ENUM_NMPolyID ePoly = kNMPoly_Invalid;
	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
		if( !pChar )
		{
			return false;
		}

		// Bail if target is not in the NavMesh.

		ePoly = pChar->GetCurrentNavMeshPoly();
	}
	else if( m_pAI->HasTarget( kTarget_Disturbance ) )
	{
		ePoly = g_pAIQuadTree->GetContainingNMPoly( m_pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPosition(), m_pAI->GetCharTypeMask(), kNMPoly_Invalid );
	}

	// Bail if no character.

	if( ePoly == kNMPoly_Invalid )
	{
		return false;
	}

	// Bail if no NavMesh poly.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
	if( !pPoly )
	{
		return false;
	}


	// Get a list of valid nodes.
	// Re-use static memory for list of nodes.
	// Note: In our implementation of STL, resize(0) does 
	// not free allocated memory, but clear() does.

	static AIVALID_NODE_LIST lstValidNodes;
	lstValidNodes.resize( 0 );

	// Iterate over all AIRegions that the NavMesh poly is a part of.

	ENUM_AIRegionID eAIRegion;
	int cAIRegions = pPoly->GetNumAIRegions();
	for( int iAIRegion=0; iAIRegion < cAIRegions; ++iAIRegion )
	{
		// Skip invalid AIRegions.

		eAIRegion = pPoly->GetAIRegion( iAIRegion );
		if( eAIRegion == kAIRegion_Invalid )
		{
			continue;
		}

		// Get list of valid view nodes.

		g_pAINodeMgr->FindPotentiallyValidViewNodesInAIRegion( m_pAI, eAIRegion, m_pAI->GetPosition(), lstValidNodes );
	}

	// Create memories in AIWorkingMemory.

	m_cNodesFound = lstValidNodes.size();
	CreateNodeMemories( lstValidNodes );

	return true;
}
