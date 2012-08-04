// ----------------------------------------------------------------------- //
//
// MODULE  : AIPathMgrNavMesh.cpp
//
// PURPOSE : PathMgr implementation for finding paths on a NavMesh.
//
// CREATED : 12/02/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIPathMgrNavMesh.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIPathKnowledgeMgr.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AIAStarMachine.h"
#include "AIAStarNavMesh.h"
#include "AIAStarNavMeshEscape.h"
#include "AIAStarNavMeshSafe.h"
#include "AIQuadTree.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS( CAIPathNavMesh );


// Globals / Statics

CAIPathMgrNavMesh* g_pAIPathMgrNavMesh = NULL;


//----------------------------------------------------------------------------

//
// CAIPathNavMesh
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::Con/Destructor
//              
//	PURPOSE:	Create/Destroy a PathMgr.
//              
//----------------------------------------------------------------------------

CAIPathNavMesh::CAIPathNavMesh()
{
	m_iCurPathNode = 0;
}

CAIPathNavMesh::~CAIPathNavMesh()
{
	m_NMPath.resize( 0 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIPathNavMesh
//              
//----------------------------------------------------------------------------
void CAIPathNavMesh::Save(ILTMessage_Write *pMsg)
{
	SAVE_VECTOR(m_vSource);
	SAVE_VECTOR(m_vDest);
	SAVE_INT(m_NMPath.size());

	for (std::size_t i = 0; i < m_NMPath.size(); ++i)
	{
		SAVE_INT(m_NMPath[i].eEdge);
		SAVE_INT(m_NMPath[i].eOffsetEntryToLink);
		SAVE_INT(m_NMPath[i].ePoly);
		SAVE_VECTOR(m_NMPath[i].vAStarEntry);
		SAVE_VECTOR(m_NMPath[i].vLink0);
		SAVE_VECTOR(m_NMPath[i].vLink1);
		SAVE_VECTOR(m_NMPath[i].vWayPt);
		SAVE_bool(m_NMPath[i].bOptional);
		SAVE_bool(m_NMPath[i].bTrimLink0);
		SAVE_bool(m_NMPath[i].bTrimLink1);
	}

	SAVE_INT(m_iCurPathNode);
}

void CAIPathNavMesh::Load(ILTMessage_Read *pMsg)
{
	LOAD_VECTOR(m_vSource);
	LOAD_VECTOR(m_vDest);
	
	int nPathNodes = 0;
	LOAD_INT(nPathNodes);

	m_NMPath.resize(nPathNodes);

	for (int i = 0; i < nPathNodes; ++i)
	{
		LOAD_INT_CAST(m_NMPath[i].eEdge, ENUM_NMEdgeID);
		LOAD_INT_CAST(m_NMPath[i].eOffsetEntryToLink, ENUM_NMLinkID);
		LOAD_INT_CAST(m_NMPath[i].ePoly, ENUM_NMPolyID);
		LOAD_VECTOR(m_NMPath[i].vAStarEntry);
		LOAD_VECTOR(m_NMPath[i].vLink0);
		LOAD_VECTOR(m_NMPath[i].vLink1);
		LOAD_VECTOR(m_NMPath[i].vWayPt);
		LOAD_bool(m_NMPath[i].bOptional);
		LOAD_bool(m_NMPath[i].bTrimLink0);
		LOAD_bool(m_NMPath[i].bTrimLink1);
	}

	LOAD_INT(m_iCurPathNode);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::ResetPath
//              
//	PURPOSE:	Clear any existing path info.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::ResetPath( unsigned int cNodes )
{
	m_iCurPathNode = 1;
	m_NMPath.resize( cNodes );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::SetPathNodePoly
//              
//	PURPOSE:	Set the NMPoly_ID for a specified path node.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::SetPathNodePoly( unsigned int iNode, ENUM_NMPolyID ePoly, const LTVector& vEntry )
{
	if( iNode < m_NMPath.size() )
	{
		m_NMPath[iNode].ePoly = ePoly;
		m_NMPath[iNode].vWayPt = vEntry;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::GeneratePathLinks
//              
//	PURPOSE:	Use the NMPath to create a list of links to cross
//				between NMPolys.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::GeneratePathLinks( CAI* pAI )
{	
	SPATH_NODE* pNodeCur;
	SPATH_NODE* pNodeNext;

	CAINavMeshPoly* pPoly;
	CAINavMeshPoly* pPolyNext;
	CAINavMeshEdge* pEdge;

	AINavMeshLinkAbstract* pLink;

	float flTrimRadius = pAI ? pAI->GetRadius() : 0.0f;

	// Find the NMEdge between every pair of NMPolys.
	// Initially, the link is the same as the edge.

	NMPATH::iterator itPath;
	for( itPath = m_NMPath.begin(); itPath != m_NMPath.end()-1; ++itPath )
	{
		pNodeCur = &( *itPath );
		pNodeNext = &( *( itPath + 1 ) );
		pNodeCur->bOptional = false;

		// Allow NavMeshLinks to set the PathLink for nodes preceding links.

		pPolyNext = g_pAINavMesh->GetNMPoly( pNodeNext->ePoly );
		if( pAI && pPolyNext && ( pPolyNext->GetNMLinkID() != kNMLink_Invalid ) )
		{
			pLink = g_pAINavMesh->GetNMLink( pPolyNext->GetNMLinkID() );
			if( pLink )
			{
				// Allow the Link to set the endpoints.

				if( pLink->SetNMLinkOffsetEntryPathLink( pNodeCur, pAI ) )
				{
					// Trim the link to the radius of the AI.

					TrimLineSegmentByRadius( flTrimRadius, &( pNodeCur->vLink0 ), &( pNodeCur->vLink1 ), true, true );
					pNodeCur->eOffsetEntryToLink = pLink->GetNMLinkID();
					continue;
				}
			}
		}

		// Default behavior.

		pNodeCur->eOffsetEntryToLink = kNMLink_Invalid;
		pPoly = g_pAINavMesh->GetNMPoly( pNodeCur->ePoly );
		AIASSERT1( pPoly, 0, "CAIPathNavMesh::GeneratePathLinks : Failed to find poly: %d ", pNodeCur->ePoly );
		if( pPoly )
		{
			pEdge = pPoly->GetNMPolyNeighborEdge( pNodeNext->ePoly );
			AIASSERT2( pEdge, 0, "CAIPathNavMesh::GeneratePathLinks : Failed to find neighbor %d -> %d ", pNodeCur->ePoly, pNodeNext->ePoly );
			if( pEdge )
			{
				pNodeCur->eEdge = pEdge->GetNMEdgeID();

				// Endpoints of the link match the endpoint of the edge.
				// WayPoint on the link is initially the center of the edge.

				pNodeCur->vLink0 = pEdge->GetNMEdge0();
				pNodeCur->vLink1 = pEdge->GetNMEdge1();
				pNodeCur->vWayPt = pEdge->GetNMEdgeMidPt();

				pNodeCur->bTrimLink0 = pEdge->IsNMBorderVert0();
				pNodeCur->bTrimLink1 = pEdge->IsNMBorderVert1();

				// Trim the link to the radius of the AI.

				if( TrimLineSegmentByRadius( flTrimRadius, &( pNodeCur->vLink0 ), &( pNodeCur->vLink1 ), pNodeCur->bTrimLink0, pNodeCur->bTrimLink1 ) )
				{
					pNodeCur->vWayPt = ( pNodeCur->vLink0 + pNodeCur->vLink1 ) * 0.5f;
				}
			}
		}
	}

	pNodeCur = &( *itPath );
	pNodeCur->vLink0 = m_vDest;
	pNodeCur->vLink1 = m_vDest;
	pNodeCur->vWayPt = m_vDest;
	pNodeCur->eEdge = kNMEdge_Invalid;
	pNodeCur->bOptional = false;

	itPath = m_NMPath.begin();
	pNodeCur = &( *itPath );
	pNodeNext = &( *( itPath + 1 ) );
	pNodeCur->ePoly = pNodeNext->ePoly;
	pNodeCur->eEdge = kNMEdge_Invalid;
	pNodeCur->vLink0 = m_vSource;
	pNodeCur->vLink1 = m_vSource;
	pNodeCur->vWayPt = m_vSource;
	pNodeCur->bOptional = false;
	pNodeNext->bOptional = false;

	// Setup AIs knowledge of the first NMLink encountered on this path.

	AdvanceNextNMLink( pAI, 0 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::PullStrings
//              
//	PURPOSE:	Optimize the path by "string-pulling." Pull path
//              taught to use straight lines where possible.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::PullStrings( CAI* pAI, uint32 nMaxIterations )
{
	if( nMaxIterations == 0 )
	{
		return;
	}

	SPATH_NODE* pNodeCur;
	SPATH_NODE* pNodePrev;
	SPATH_NODE* pNodeNext;

	CAINavMeshPoly* pPoly;
	AINavMeshLinkAbstract* pLink;

	LTVector vPtCur, vPtPrev, vPtNext, vPtBest, vDiff;
	NMPATH::iterator itPath;

	// Iteratively smooth path until no way points move.

	EnumRayIntersectResult eResult;
	bool bLinkWayPt;
	bool bAdjusted = true;
	uint32 cIterations = 0;
	while( bAdjusted )
	{
		// Bail if we have exceeded our limit of iterations.

		if( cIterations > nMaxIterations )
		{
 			// If we failed to find all of our constraining points (ie
 			// non-optional points), we must assume that none are optional. If
 			// we don't do this, the AI may attempt to run a totally invalid
 			// path. For instance, imagine a u-turn with several waypoints. If
 			// the path doesn't converge, the AI may have all waypoints in the
 			// path tagged as optional, causing the AI to run from the start
 			// point to the end point.
 
 			for( itPath = m_NMPath.begin() + 1; itPath != m_NMPath.end()-1; ++itPath )
 			{
 				pNodeCur = &( *itPath );
 				pNodeCur->bOptional = false;
 			}
 
 			// Hitting the max iterations isn't good. There is a performance
 			// impact, and the path the AI follows will have points where the
 			// AI loses his 'velocity' while traveling along a straight path
 			// (as the AI stops for the remainder of a frame any time he
 			// reaches a waypoint). To determine how frequently this occurs, a
 			// debug statement is being added to make this situation more
 			// observable.
 
#if !defined(_FINAL)
 			ObjectCPrint(pAI ? pAI->GetHOBJECT() : (HOBJECT)NULL, "CAIPathNavMesh::PullStrings: Warning: Unable to optimize path." );
#endif
			return;
		}
		++cIterations;

		// Iterate over each set of three waypoints.

		bAdjusted = false;
		for( itPath = m_NMPath.begin() + 1; itPath != m_NMPath.end()-1; ++itPath )
		{
			pNodeCur = &( *itPath );
			pNodePrev = &( *( itPath - 1 ) );
			pNodeNext = &( *( itPath + 1 ) );

			vPtCur = pNodeCur->vWayPt;
			vPtPrev = pNodePrev->vWayPt;
			vPtNext = pNodeNext->vWayPt;

			bLinkWayPt = false;

			// Allow NavMeshLinks to handle string pulling.

			pPoly = g_pAINavMesh->GetNMPoly( pNodeCur->ePoly );
			if( pPoly && pAI && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
			{
				LTVector vNewPos = vPtCur;
				pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
				if( pLink->PullStrings( vPtPrev, vPtNext, pAI, &vNewPos ) )
				{
					vDiff = vNewPos - vPtCur;
					if( vDiff.MagSqr() > 0.01f )
					{
						pNodeCur->vWayPt = vNewPos;
						bAdjusted = true;
					}
					continue;
				}

				if( !pLink->AllowStraightPaths() )
				{
					bLinkWayPt = true;
				}
			}

			// Look ahead.  If the next poly has a link, and if the link 
			// modifies the path, then don't allow this node to make a 
			// modification.  This handles the fact that a link may adjust
			// a point to be non-optimal.  If this happens, this algorithm
			// may infinite loop
			
			NMPATH::iterator itNextPoly = itPath + 1;
			if (itNextPoly != m_NMPath.end()-1 && pAI)
			{
				CAINavMeshPoly* pNextPoly = g_pAINavMesh->GetNMPoly( itNextPoly->ePoly );
				if( pNextPoly && ( pNextPoly->GetNMLinkID() != kNMLink_Invalid ) )
				{
					AINavMeshLinkAbstract* pNextLink = g_pAINavMesh->GetNMLink( pNextPoly->GetNMLinkID() );
					if ( pNextLink->IsPullStringsModifying( pAI ))
					{
						continue;
					}
					if( !pNextLink->AllowStraightPaths() )
					{
						bLinkWayPt = true;
					}
				}
			}
			
			// Find where the path intersects the connection between the nodes.
			// Reset the 'optional' flag on every iteration of string-pulling.

			pNodeCur->bOptional = false;
			eResult = RayIntersectLineSegment( pNodeCur->vLink0, pNodeCur->vLink1, vPtPrev, vPtNext, true, &vPtBest );
			if( eResult != kRayIntersect_Failure )
			{
				// Adjust waypoint if its position is significantly different.
				// Note:  Checking for equality or near equality ( == or NearlyEquals() )
				//        may result in infinite loops due to floating point inaccuracy.
				//        So, be a bit more leanient.

				vDiff = vPtBest - vPtCur;
				if( vDiff.MagSqr() > 0.01f )
				{
					pNodeCur->vWayPt = vPtBest;
					bAdjusted = true;
				}

				// Flag optional waypoints.
				// These may be ignored when an AI is looking for the 
				// next waypoint to advance to.
				// Do not consider a waypoint optional if it is
				// very close to it previous or next waypoint.
				// Waypoints that are nearly equal confuse the local 
				// test for optional waypoints.

				if( ( eResult != kRayIntersect_SnappedToSegment ) &&
					( !bLinkWayPt ) &&
					( !vPtPrev.NearlyEquals( vPtBest, 1.f ) ) &&
					( !vPtNext.NearlyEquals( vPtBest, 1.f ) ) )
				{
					pNodeCur->bOptional = true;
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::ReserveNavMeshLinks
//              
//	PURPOSE:	Reserve NavMeshLinks to prevent other AI from 
//              trying to use the same links too soon.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::ReserveNavMeshLinks( CAI* pAI, bool bReserve )
{
	AINavMeshLinkAbstract* pLink;
	CAINavMeshPoly* pPoly;
	SPATH_NODE* pPathNode;
	NMPATH::iterator itPath;

	for( itPath = m_NMPath.begin(); itPath != m_NMPath.end(); ++itPath )
	{
		pPathNode = &( *itPath );
		if ( !pPathNode )
		{
			continue;
		}

		pPoly = g_pAINavMesh->GetNMPoly( pPathNode->ePoly );
		if( pPoly && pAI && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
		{
			pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
			pLink->ReserveNMLink( pAI, bReserve );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::ApplyNMLinkDeathDelay
//              
//	PURPOSE:	Apply delay after AI dies to prevent other 
//              AI from trying to use the same links too soon.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::ApplyNMLinkDeathDelay( CAI* pAI )
{
	AINavMeshLinkAbstract* pLink;
	CAINavMeshPoly* pPoly;
	SPATH_NODE* pPathNode;
	NMPATH::iterator itPath;

	for( itPath = m_NMPath.begin(); itPath != m_NMPath.end(); ++itPath )
	{
		pPathNode = &( *itPath );
		if ( !pPathNode )
		{
			continue;
		}

		pPoly = g_pAINavMesh->GetNMPoly( pPathNode->ePoly );
		if( pPoly && pAI && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
		{
			pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
			pLink->ApplyNMLinkDeathDelay( pAI );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::SkipOptionalPathNodes
//              
//	PURPOSE:	Increment the current path node index to a non-optional node.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::SkipOptionalPathNodes( CAI* pAI )
{
	SPATH_NODE* pPathNode;
	while( true )
	{
		pPathNode = GetCurPathNode();
		
		if ( !pPathNode )
		{
			return;
		}

		if ( !pPathNode->bOptional )
		{
			// Verify the AI is not already at this location.  If they are, 
			// they should continue skipping optional waypoints.

			LTVector vPos = pAI->GetPosition();
			if( ( vPos.x != pPathNode->vWayPt.x ) 
				|| ( vPos.z != pPathNode->vWayPt.z ) 
				|| ( pAI->GetLastNavMeshPoly() != pPathNode->ePoly ) )
			{
				return;
			}
		}

		IncrementCurPathNodeIndex( pAI );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::IncrementCurPathNodeIndex
//              
//	PURPOSE:	Increment the current path node index.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::IncrementCurPathNodeIndex( CAI* pAI )
{
	// If AI was headed towards a NMLink, advance to the next
	// NMLink if this one was reached.

	int iLastNode = m_iCurPathNode - 1;
	if( ( iLastNode > 0 ) &&
		( pAI->GetAIBlackBoard()->GetBBNextNMLink() != kNMLink_Invalid ) )
	{
		SPATH_NODE* pPathNode = GetPathNode( iLastNode );
		if( pPathNode )
		{
			CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( pPathNode->ePoly );
			if( pPoly && ( pPoly->GetNMLinkID() == pAI->GetAIBlackBoard()->GetBBNextNMLink() ) )
			{
				AdvanceNextNMLink( pAI, m_iCurPathNode );
			}
		}
	}

	// Increment the current path node.

	++m_iCurPathNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::GetPathDistance
//              
//	PURPOSE:	Calculate the actual distance of this path by traversing the nodes.
//              
//----------------------------------------------------------------------------

float CAIPathNavMesh::GetPathDistance() const
{
	float fDist=0.0f;
	for (uint32 i=1; i < m_NMPath.size(); i++)
	{
		fDist += (m_NMPath[i-1].vWayPt - m_NMPath[i].vWayPt).MagSqr();
	}
	return LTSqrt(fDist);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathNavMesh::AdvanceNextNMLink
//              
//	PURPOSE:	Advance the next NMLink in the path.
//              
//----------------------------------------------------------------------------

void CAIPathNavMesh::AdvanceNextNMLink( CAI* pAI, unsigned int iPathNode )
{
	ENUM_NMLinkID eNextLink = kNMLink_Invalid;

	// Iterate over remaining path nodes and look for the next NMLink.

	SPATH_NODE* pPathNode;
	CAINavMeshPoly* pPoly;
	NMPATH::iterator itPath;
	for( itPath = m_NMPath.begin() + iPathNode; itPath != m_NMPath.end(); ++itPath )
	{
		pPathNode = &( *itPath );

		pPoly = g_pAINavMesh->GetNMPoly( pPathNode->ePoly );
		if( pPoly )
		{
			// Record the next NMLink in the path.

			if( pPoly->GetNMLinkID() != kNMLink_Invalid )
			{
				eNextLink = pPoly->GetNMLinkID();
				break;
			}
		}

		++iPathNode;
	}

	if ( pAI )
	{
		pAI->GetAIBlackBoard()->SetBBNextNMLink( eNextLink );
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//
// CAIPathMgrNavMesh
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::Con/Destructor
//              
//	PURPOSE:	Create/Destroy a PathMgr.
//              
//----------------------------------------------------------------------------

CAIPathMgrNavMesh::CAIPathMgrNavMesh()
{
	AIASSERT( g_pAIPathMgrNavMesh == NULL, NULL, "CAIPathMgrNavMesh: Singleton already exists." );
	g_pAIPathMgrNavMesh = this;

	m_nPathKnowledgeIndex = 0;
	m_pAStarMachine = debug_new( CAIAStarMachine );
	m_pAStarStorage = debug_new( CAIAStarStorageNavMesh );
	m_pAStarMap = debug_new( CAIAStarMapNavMesh );
	m_pAStarGoal = debug_new( CAIAStarGoalNavMesh );
	m_pAStarGoalSafe = debug_new( CAIAStarGoalNavMeshSafe );
	m_pAStarMapStraightPath = debug_new( CAIAStarMapNavMeshStraightPath );
	m_pAStarGoalEscape = debug_new( CAIAStarGoalNavMeshEscape );
	m_pAStarMapEscape = debug_new( CAIAStarMapNavMeshEscape );

	m_CachedStraightPath.hAI = NULL;
	m_CachedStraightPath.dwCharTypeMask = 0;
	m_CachedStraightPath.vSource.Init();
	m_CachedStraightPath.vDest.Init();
	m_CachedStraightPath.iPathKnowledgeIndex = 0;
	m_CachedStraightPath.bResult = false;

	m_CachedEscapePath.hAI = NULL;
	m_CachedEscapePath.dwCharTypeMask = 0;
	m_CachedEscapePath.vSource.Init();
	m_CachedEscapePath.vDanger.Init();
	m_CachedEscapePath.fClearance = 0.f;
	m_CachedEscapePath.iPathKnowledgeIndex = 0;
	m_CachedEscapePath.vClearDest.Init();
	m_CachedEscapePath.bResult = false;
}

CAIPathMgrNavMesh::~CAIPathMgrNavMesh()
{
	AIASSERT( g_pAIPathMgrNavMesh != NULL, NULL, "~CAIPathMgrNavMesh: Singleton does not exist." );

	debug_delete( m_pAStarMapEscape );
	debug_delete( m_pAStarGoalEscape );
	debug_delete( m_pAStarMapStraightPath );
	debug_delete( m_pAStarGoalSafe );
	debug_delete( m_pAStarGoal );
	debug_delete( m_pAStarStorage );
	debug_delete( m_pAStarMap );
	debug_delete( m_pAStarMachine );
	g_pAIPathMgrNavMesh = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::Save/Load
//              
//	PURPOSE:	Save/Load.
//              
//----------------------------------------------------------------------------

void CAIPathMgrNavMesh::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT( m_nPathKnowledgeIndex );

	SAVE_INT( m_lstPathInvalidationRequests.size() );

	SPATH_INVALIDATION_REQUEST* pRequest;
	PATH_INVALIDATION_REQUEST_LIST::iterator itRequest;
	for( itRequest = m_lstPathInvalidationRequests.begin(); itRequest != m_lstPathInvalidationRequests.end(); ++itRequest )
	{
		pRequest = &( *itRequest );
		SAVE_HOBJECT( pRequest->hInvalidator );
		SAVE_TIME( pRequest->fTime );
	}

	SAVE_HOBJECT( m_CachedStraightPath.hAI );
	SAVE_DWORD( m_CachedStraightPath.dwCharTypeMask );
	SAVE_VECTOR( m_CachedStraightPath.vSource );
	SAVE_VECTOR( m_CachedStraightPath.vDest );
	SAVE_INT( m_CachedStraightPath.iPathKnowledgeIndex );
	SAVE_bool( m_CachedStraightPath.bResult );

	SAVE_HOBJECT( m_CachedEscapePath.hAI );
	SAVE_DWORD( m_CachedEscapePath.dwCharTypeMask );
	SAVE_VECTOR( m_CachedEscapePath.vSource );
	SAVE_VECTOR( m_CachedEscapePath.vDanger );
	SAVE_FLOAT( m_CachedEscapePath.fClearance );
	SAVE_INT( m_CachedEscapePath.iPathKnowledgeIndex );
	SAVE_VECTOR( m_CachedEscapePath.vClearDest );
	SAVE_bool( m_CachedEscapePath.bResult );
}

void CAIPathMgrNavMesh::Load(ILTMessage_Read *pMsg)
{
	LOAD_INT( m_nPathKnowledgeIndex );

	uint32 cRequests;
	LOAD_INT( cRequests );
	m_lstPathInvalidationRequests.resize( cRequests );

	SPATH_INVALIDATION_REQUEST* pRequest;
	PATH_INVALIDATION_REQUEST_LIST::iterator itRequest;
	for( itRequest = m_lstPathInvalidationRequests.begin(); itRequest != m_lstPathInvalidationRequests.end(); ++itRequest )
	{
		pRequest = &( *itRequest );
		LOAD_HOBJECT( pRequest->hInvalidator );
		LOAD_TIME( pRequest->fTime );
	}

	LOAD_HOBJECT( m_CachedStraightPath.hAI );
	LOAD_DWORD( m_CachedStraightPath.dwCharTypeMask );
	LOAD_VECTOR( m_CachedStraightPath.vSource );
	LOAD_VECTOR( m_CachedStraightPath.vDest );
	LOAD_INT( m_CachedStraightPath.iPathKnowledgeIndex );
	LOAD_bool( m_CachedStraightPath.bResult );

	LOAD_HOBJECT( m_CachedEscapePath.hAI );
	LOAD_DWORD( m_CachedEscapePath.dwCharTypeMask );
	LOAD_VECTOR( m_CachedEscapePath.vSource );
	LOAD_VECTOR( m_CachedEscapePath.vDanger );
	LOAD_FLOAT( m_CachedEscapePath.fClearance );
	LOAD_INT( m_CachedEscapePath.iPathKnowledgeIndex );
	LOAD_VECTOR( m_CachedEscapePath.vClearDest );
	LOAD_bool( m_CachedEscapePath.bResult );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::InitPathMgrNavMesh
//              
//	PURPOSE:	Initialize the PathMgr.
//              
//----------------------------------------------------------------------------

void CAIPathMgrNavMesh::InitPathMgrNavMesh()
{
	// Sanity check.

	if( !g_pAINavMesh )
	{
		return;
	}

	m_pAStarMapStraightPath->InitAStarMapNavMesh( g_pAINavMesh );
	m_pAStarMapEscape->InitAStarMapNavMesh( g_pAINavMesh );
	m_pAStarMap->InitAStarMapNavMesh( g_pAINavMesh );
	m_pAStarGoal->InitAStarGoalNavMesh( m_pAStarMap );
	m_pAStarGoalSafe->InitAStarGoalNavMesh( m_pAStarMap );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::TermPathMgrNavMesh
//              
//	PURPOSE:	Terminate the PathMgr.
//              
//----------------------------------------------------------------------------

void CAIPathMgrNavMesh::TermPathMgrNavMesh()
{
	m_pAStarMapStraightPath->TermAStarMapNavMesh();
	m_pAStarMapEscape->TermAStarMapNavMesh();
	m_pAStarMap->TermAStarMapNavMesh();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::HasPath
//              
//	PURPOSE:	Return true if AI can find a path to the destination.
//              
//----------------------------------------------------------------------------

bool CAIPathMgrNavMesh::HasPath( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vDest )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	ENUM_NMPolyID eDest = g_pAIQuadTree->GetContainingNMPoly( vDest, dwCharTypeMask, kNMPoly_Invalid, pAI );
	return HasPath( pAI, dwCharTypeMask, eDest );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::HasPath
//              
//	PURPOSE:	Return true if AI can find a path to the destination.
//              
//----------------------------------------------------------------------------

bool CAIPathMgrNavMesh::HasPath( CAI* pAI, uint32 dwCharTypeMask, ENUM_NMPolyID eNavMeshPolyDest )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// First check if AI has previously searched for a path to this
	// same destination.  If so, return the results of that search.

	if( pAI->GetPathKnowledgeMgr() )
	{
		EnumPathBuildStatus eStatus = pAI->GetPathKnowledgeMgr()->GetPathKnowledge( eNavMeshPolyDest );
		if( eStatus != kPath_Unknown )
		{
			return ( eStatus == kPath_PathFound );
		}
	}

	// No previous search results exist, so find a path.

	CAINavMeshPoly* pPolySource = g_pAINavMesh->GetNMPoly( pAI->GetLastNavMeshPoly() );
	if( !pPolySource )
	{
		return false;
	}

	CAINavMeshPoly* pPolyDest = g_pAINavMesh->GetNMPoly( eNavMeshPolyDest );
	if( !pPolyDest )
	{
		return false;
	}

	// If source and dest polies are in the same NavMesh component, 
	// we know there's a path (since caching is based on components).

	if( pPolySource->GetNMComponentID() == pPolyDest->GetNMComponentID() )
	{
		return true;
	}

	if( FindPath( pAI, dwCharTypeMask, pPolySource->GetNMPolyCenter(), pPolyDest->GetNMPolyCenter(), pAI->GetLastNavMeshPoly(), eNavMeshPolyDest, m_pAStarGoal ) )
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::FindPath
//              
//	PURPOSE:	Find a path on a NavMesh.
//              
//----------------------------------------------------------------------------

#ifdef _DEBUG
#define ENABLE_PATH_LOOP_CHECK 1
#endif //_DEBUG

bool CAIPathMgrNavMesh::FindPath( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, ENUM_NMPolyID eNavMeshPolySource, ENUM_NMPolyID eNavMeshPolyDest, uint32 nPullStringsMaxIters, ENUM_AI_PATH_TYPE ePathType, CAIPathNavMesh* pPath )
{
	// Sanity check.

	if( !pPath )
	{
		return false;
	}

	// Verify both a source and destination poly are specified.

	if( eNavMeshPolySource == kNMPoly_Invalid 
		|| eNavMeshPolyDest == kNMPoly_Invalid )
	{
		return false;
	}

	// Select the AStarGoal class depending on the desired type of path.

	CAIAStarGoalNavMesh* pAStarGoal = m_pAStarGoal;
	if( pAI && ( ePathType == kPath_Safe ) )
	{
		ENUM_NMPolyID eTargetPoly = pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly();
		if( eTargetPoly != kNMPoly_Invalid )
		{
			m_pAStarGoalSafe->SetTargetNMPoly( eTargetPoly );
			pAStarGoal = m_pAStarGoalSafe;
		}
	}

	// Run A*.

	CAIAStarNodeAbstract* pStartNode = FindPath( pAI, dwCharTypeMask, vSource, vDest, eNavMeshPolySource, eNavMeshPolyDest, pAStarGoal );
	CAIAStarNodeAbstract* pNode = pStartNode;

	// No path found.

	if( !pNode )
	{
		return false;
	}

	// Count the number of nodes in the path (plus 2, for the start and end nodes)

	unsigned int nMaxNodesPossible = g_pAINavMesh->GetNumNMPolys()+2;
	bool bMaxNodesExceeded = false;

	unsigned int cNodes = 1;
	while( pNode )
	{
#ifdef ENABLE_PATH_LOOP_CHECK
		ENUM_NMPolyID eCurrentPoly = m_pAStarMap->ConvertID_AStarNode2NMPoly(pNode->eAStarNodeID);

		if (g_pAINavMesh->TestNMPolyFlags(eCurrentPoly, kNMPolyFlag_Marked))
		{
			LTERROR( "Infinite loop detected while counting navmesh nodes." );
		}

		g_pAINavMesh->SetNMPolyFlags(eCurrentPoly, kNMPolyFlag_Marked);
#endif // ENABLE_PATH_LOOP_CHECK

		++cNodes;
		pNode = pNode->pAStarParent;

		// If we hit this, we have a loop in our path. This is an error which 
		// we have not yet tracked back to the source.
		if (cNodes >= nMaxNodesPossible)
		{
			LTERROR( "Infinite loop detected while counting navmesh nodes." );
			bMaxNodesExceeded = true;
			break;
		}
	}

#ifdef ENABLE_PATH_LOOP_CHECK
	// Clear the marks.

	CAIAStarNodeAbstract* pClearFlagsNode = pStartNode;
	while(pClearFlagsNode)
	{
		ENUM_NMPolyID eCurrentPoly = m_pAStarMap->ConvertID_AStarNode2NMPoly(pClearFlagsNode->eAStarNodeID);
		g_pAINavMesh->ClearNMPolyFlags(eCurrentPoly, kNMPolyFlag_Marked);
		pClearFlagsNode = pClearFlagsNode->pAStarParent;
	}
#endif //ENABLE_PATH_LOOP_CHECK

	// Sanity check to make sure we don't have an infinite loop in our pathing.

	if (bMaxNodesExceeded)
	{
		return false;
	}

	pPath->ResetPath( cNodes );

	// Generate a NavMesh path from the AStar nodes.

	pPath->SetPathSource( vSource );
	pPath->SetPathDest( vDest );

	CAIAStarNodeNavMesh* pNodeNavMesh;

	pPath->SetPathNodePoly( 0, eNavMeshPolySource, vSource );

	unsigned int iNode = 1;
	pNode = m_pAStarMachine->GetAStarNodeCur();
	while( pNode )
	{
		pNodeNavMesh = (CAIAStarNodeNavMesh*)pNode;
		pPath->SetPathNodePoly( iNode, m_pAStarMap->ConvertID_AStarNode2NMPoly( pNode->eAStarNodeID ), pNodeNavMesh->vTrueEntryPos );
		pNode = pNode->pAStarParent;
		++iNode;
	}

	if( nPullStringsMaxIters > 0 )
	{
		pPath->GeneratePathLinks( pAI );
		pPath->PullStrings( pAI, nPullStringsMaxIters );
	}

	// Debug output.
	/**
	AITRACE( AIShowPaths, ( pAI->m_hObject, "NEWPATH [%f %f %f] to [%f %f %f]:", 
		vSource.x, vSource.y, vSource.z, 
		vDest.x, vDest.y, vDest.z ) );

	SPATH_NODE* pPathNode;
	for( unsigned int iNode=0; iNode < pPath->GetPathLength(); ++iNode )
	{
		pPathNode = pPath->GetPathNode( iNode );
		if( pPathNode )
		{
			AITRACE( AIShowPaths, ( pAI->m_hObject, "  PATH: %d (%.2f %.2f %.2f)", pPathNode->ePoly,
				pPathNode->vWayPt.x, pPathNode->vWayPt.y, pPathNode->vWayPt.z ) );
		}
	}
	**/

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::FindPath
//              
//	PURPOSE:	Find a path on a NavMesh.
//              
//----------------------------------------------------------------------------

CAIAStarNodeAbstract* CAIPathMgrNavMesh::FindPath( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, ENUM_NMPolyID eNavMeshPolySource, ENUM_NMPolyID eNavMeshPolyDest, CAIAStarGoalNavMesh* pAStarGoal )
{
	// Sanity check.

	if( !pAStarGoal )
	{
		return NULL;
	}

	if( pAI )
	{
		// Check if we already know that we cannot find a path to this dest.

		if( pAI->GetPathKnowledgeMgr() && 
			( pAI->GetPathKnowledgeMgr()->GetPathKnowledge( eNavMeshPolyDest ) == kPath_NoPathFound ) )
		{
			return false;
		}
	}

	// No path can be found if the dest is in a NavMeshLink that
	// is not valid to use as a destination.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( eNavMeshPolyDest );
	if( pPoly && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
		if( pLink && !pLink->IsLinkValidDest() )
		{
			if ( pAI )
			{
				pAI->GetPathKnowledgeMgr()->RegisterPathKnowledge( eNavMeshPolySource, eNavMeshPolyDest, kPath_NoPathFound );
			}
			return false;
		}
	}


	CAIAStarNodeAbstract* pNode = NULL;

	if( ( eNavMeshPolySource != kNMPoly_Invalid ) &&
		( eNavMeshPolyDest != kNMPoly_Invalid ) )
	{
		// Setup the AStar machine, and run it.
		// Intentionally swap the source and dest, and search backwards.
		// This gives NavMeshLinks the opportunity to direct a search so 
		// that AI line up to approach the link correctly (e.g. start a 
		// DiveThru before hitting the border of the link).

		m_pAStarMap->InitAStarMapNavMeshSearch( dwCharTypeMask, vDest, vSource );

		m_pAStarMachine->InitAStar( m_pAStarStorage, pAStarGoal, m_pAStarMap );
		m_pAStarMachine->SetAStarSource( m_pAStarMap->ConvertID_NMPoly2AStarNode( eNavMeshPolyDest ) );
		m_pAStarMachine->SetAStarDest( m_pAStarMap->ConvertID_NMPoly2AStarNode( eNavMeshPolySource ) );
		m_pAStarMachine->RunAStar( pAI );

		pNode = m_pAStarMachine->GetAStarNodeCur();
	}

	// Cache path status.  
	//
	// Do not cache any information if the source and destination polies are
	// the same.  Registering this information can cause problems, as AIs are
	// allowed to path inside of a disabled link, but can't path in.  If an AI
	// is in a disabled link and paths to another location inside, PathFound 
	// would be registered; this causes later pathing attempts to succeed due
	// to an invalid cached success.

	if ( pAI )
	{
		if( eNavMeshPolySource != eNavMeshPolyDest && 
			pAI->GetPathKnowledgeMgr() )
		{
			if( pNode )
			{
				pAI->GetPathKnowledgeMgr()->RegisterPathKnowledge( eNavMeshPolySource, eNavMeshPolyDest, kPath_PathFound );
			}
			else {
				pAI->GetPathKnowledgeMgr()->RegisterPathKnowledge( eNavMeshPolySource, eNavMeshPolyDest, kPath_NoPathFound );
			}
		}
	}

	return pNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::StraightPathExists
//              
//	PURPOSE:	Return true if a straight path exists.
//              
//----------------------------------------------------------------------------

bool CAIPathMgrNavMesh::StraightPathExists( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, ENUM_NMPolyID eLastPoly, float fRadius )
{	
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Return a cached result.

	if( ( m_CachedStraightPath.hAI == pAI->m_hObject ) &&
		( m_CachedStraightPath.dwCharTypeMask == dwCharTypeMask ) &&
		( m_CachedStraightPath.vSource == vSource ) &&
		( m_CachedStraightPath.vDest == vDest ) &&
		( m_CachedStraightPath.iPathKnowledgeIndex == m_nPathKnowledgeIndex ) 
		)
	{
		return m_CachedStraightPath.bResult;
	}

	// Find the direction of the line.

	LTVector vDir = vDest - vSource;
	if( vDir != LTVector::GetIdentity() )
	{
		vDir.Normalize();
	}

	// Add the AI's radius to the end of the path.

	LTVector vAdjustedDest = vDest + ( vDir * fRadius );

	// Find the source NMPoly.

	ENUM_NMPolyID ePolySource = g_pAIQuadTree->GetContainingNMPoly( vSource, dwCharTypeMask, eLastPoly, pAI );
	CAINavMeshPoly* pPolySource = g_pAINavMesh->GetNMPoly( ePolySource );
	if( !pPolySource )
	{
		CacheStraightPathResult( pAI, dwCharTypeMask, vSource, vDest, false );
		return false;
	}

	// Find the dest NMPoly.

	ENUM_NMPolyID ePolyDest = g_pAIQuadTree->GetContainingNMPoly( vAdjustedDest, dwCharTypeMask, eLastPoly, pAI );
	CAINavMeshPoly* pPolyDest = g_pAINavMesh->GetNMPoly( ePolyDest );
	if( !pPolyDest )
	{
		CacheStraightPathResult( pAI, dwCharTypeMask, vSource, vDest, false );
		return false;
	}

	// Bail if source or dest polys have associated NavMeshLinks which does 
	// not allow straight paths

	if( pPolySource->GetNMLinkID() != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPolySource->GetNMLinkID() );
		if( !( pLink && pLink->AllowStraightPaths() ) )
		{
			CacheStraightPathResult( pAI, dwCharTypeMask, vSource, vDest, false );
			return false;
		}
	}

	if( pPolyDest->GetNMLinkID() != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPolyDest->GetNMLinkID() );
		if( !( pLink && pLink->AllowStraightPaths() ) )
		{
			CacheStraightPathResult( pAI, dwCharTypeMask, vSource, vDest, false );
			return false;
		}
	}

	// Check if we already know that we cannot find a path to this dest.

	if( pAI->GetPathKnowledgeMgr() && 
		( pAI->GetPathKnowledgeMgr()->GetPathKnowledge( ePolyDest ) == kPath_NoPathFound ) )
	{
		CacheStraightPathResult( pAI, dwCharTypeMask, vSource, vDest, false );
		return false;
	}

	// Setup the AStar machine, and run it.

	m_pAStarMapStraightPath->InitAStarMapNavMeshSearch( dwCharTypeMask, vSource, vAdjustedDest, fRadius );

	m_pAStarMachine->InitAStar( m_pAStarStorage, m_pAStarGoal, m_pAStarMapStraightPath );
	m_pAStarMachine->SetAStarSource( m_pAStarMap->ConvertID_NMPoly2AStarNode( ePolySource ) );
	m_pAStarMachine->SetAStarDest( m_pAStarMap->ConvertID_NMPoly2AStarNode( ePolyDest ) );
	m_pAStarMachine->RunAStar( pAI );

	bool bResult = !!m_pAStarMachine->GetAStarNodeCur();
	CacheStraightPathResult( pAI, dwCharTypeMask, vSource, vDest, bResult );
	return bResult;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::CacheStraightPathResult
//              
//	PURPOSE:	Cache results of a straight path test.
//              
//----------------------------------------------------------------------------

void CAIPathMgrNavMesh::CacheStraightPathResult( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, bool bResult )
{	
	m_CachedStraightPath.hAI = pAI->m_hObject;
	m_CachedStraightPath.dwCharTypeMask = dwCharTypeMask;
	m_CachedStraightPath.vSource = vSource;
	m_CachedStraightPath.vDest = vDest;
	m_CachedStraightPath.iPathKnowledgeIndex = m_nPathKnowledgeIndex;
	m_CachedStraightPath.bResult = bResult;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::EscapePathExists
//              
//	PURPOSE:	Return true if an escape path exists.
//              
//----------------------------------------------------------------------------

bool CAIPathMgrNavMesh::EscapePathExists( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDanger, float fClearance, ENUM_NMPolyID eLastPoly, LTVector* pvClearDest )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Return a cached result.

	if( ( m_CachedEscapePath.hAI == pAI->m_hObject ) &&
		( m_CachedEscapePath.dwCharTypeMask == dwCharTypeMask ) &&
		( m_CachedEscapePath.vSource == vSource ) &&
		( m_CachedEscapePath.vDanger == vDanger ) &&
		( m_CachedEscapePath.fClearance == fClearance ) &&
		( m_CachedEscapePath.iPathKnowledgeIndex == m_nPathKnowledgeIndex )
		)
	{
		if( pvClearDest )
		{
			*pvClearDest = m_CachedEscapePath.vClearDest;
		}
		return m_CachedEscapePath.bResult;
	}

	// Find the source NMPoly.

	LTVector vClearDest;
	ENUM_NMPolyID ePolySource = g_pAIQuadTree->GetContainingNMPoly( vSource, dwCharTypeMask, eLastPoly, pAI );
	CAINavMeshPoly* pPolySource = g_pAINavMesh->GetNMPoly( ePolySource );
	if( !pPolySource )
	{
		CacheEscapePathResult( pAI, dwCharTypeMask, vSource, vDanger, fClearance, vClearDest, false );
		return false;
	}

	// Setup the AStar machine, and run it.

	m_pAStarMapEscape->InitAStarMapNavMeshSearch( dwCharTypeMask, vSource, vDanger, fClearance );
	m_pAStarGoalEscape->InitAStarGoalNavMesh( m_pAStarMapEscape );

	m_pAStarMachine->InitAStar( m_pAStarStorage, m_pAStarGoalEscape, m_pAStarMapEscape );
	m_pAStarMachine->SetAStarSource( m_pAStarMapEscape->ConvertID_NMPoly2AStarNode( ePolySource ) );
	m_pAStarMachine->SetAStarDest( kASTARNODE_Invalid );
	m_pAStarMachine->RunAStar( pAI );

	// Set the clear destination pos.

	CAIAStarNodeNavMesh* pNodeNavMesh = (CAIAStarNodeNavMesh*)m_pAStarMachine->GetAStarNodeCur();
	ENUM_NMPolyID ePoly = kNMPoly_Invalid;
	CAINavMeshPoly* pPoly = NULL;

	if( pNodeNavMesh )
	{
		ePoly = m_pAStarMapEscape->ConvertID_AStarNode2NMPoly( pNodeNavMesh->eAStarNodeID );
		pPoly = g_pAINavMesh->GetNMPoly( ePoly );
	}

	if( pPoly )
	{
		vClearDest = pPoly->GetNMPolyCenter();
	}

	if( pvClearDest )
	{
		*pvClearDest = vClearDest;
	}

	bool bResult = !!m_pAStarMachine->GetAStarNodeCur();
	CacheEscapePathResult( pAI, dwCharTypeMask, vSource, vDanger, fClearance, vClearDest, bResult );
	return bResult;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::CacheEscapePathResult
//              
//	PURPOSE:	Cache results of a escape path test.
//              
//----------------------------------------------------------------------------

void CAIPathMgrNavMesh::CacheEscapePathResult( CAI* pAI, uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDanger, float fClearance, const LTVector& vClearDest, bool bResult )
{	
	m_CachedEscapePath.hAI = pAI->m_hObject;
	m_CachedEscapePath.dwCharTypeMask = dwCharTypeMask;
	m_CachedEscapePath.vSource = vSource;
	m_CachedEscapePath.vDanger = vDanger;
	m_CachedEscapePath.fClearance = fClearance;
	m_CachedEscapePath.iPathKnowledgeIndex = m_nPathKnowledgeIndex;
	m_CachedEscapePath.vClearDest = vClearDest;
	m_CachedEscapePath.bResult = bResult;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::GetPathKnowledgeIndex
//              
//	PURPOSE:	Return path knowledge index.
//              
//----------------------------------------------------------------------------

const uint32 CAIPathMgrNavMesh::GetPathKnowledgeIndex()
{
	// Handle any posted invalidation requests.
	// Requests are sorted by invalidation time.

	SPATH_INVALIDATION_REQUEST* pRequest;
	PATH_INVALIDATION_REQUEST_LIST::iterator itRequest;
	itRequest = m_lstPathInvalidationRequests.begin();
	while( itRequest != m_lstPathInvalidationRequests.end() )
	{
		pRequest = &( *itRequest );
		if( pRequest->fTime > g_pLTServer->GetTime() )
		{
			break;
		}

		// Time to invalidate the path knowledge.

		InvalidatePathKnowledge( pRequest->hInvalidator );
		m_lstPathInvalidationRequests.erase( itRequest );
		itRequest = m_lstPathInvalidationRequests.begin();
	}

	// Return the current index.

	return m_nPathKnowledgeIndex;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::InvalidatePathKnowledge
//              
//	PURPOSE:	Invalidate ALL AIs cached path info.
//              
//----------------------------------------------------------------------------

void CAIPathMgrNavMesh::InvalidatePathKnowledge( HOBJECT hInvalidator )
{
	AITRACE( AIShowPaths, ( hInvalidator, "Invalidating cached path info." ) );
	++m_nPathKnowledgeIndex; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPathMgrNavMesh::InvalidatePathKnowledge
//              
//	PURPOSE:	Invalidate ALL AIs cached path info.
//              
//----------------------------------------------------------------------------

void CAIPathMgrNavMesh::PostPathKnowledgeInvalidationRequest( HOBJECT hInvalidator, double fTime )
{
	SPATH_INVALIDATION_REQUEST Request;
	Request.hInvalidator = hInvalidator;
	Request.fTime = fTime;

	// Keep requests sorted with the earliest request first.

	SPATH_INVALIDATION_REQUEST* pRequest = NULL;
	PATH_INVALIDATION_REQUEST_LIST::iterator itRequest;
	for( itRequest = m_lstPathInvalidationRequests.begin(); itRequest != m_lstPathInvalidationRequests.end(); ++itRequest )
	{
		pRequest = &( *itRequest );
		if( fTime <= pRequest->fTime )
		{
			break;
		}
	}

	m_lstPathInvalidationRequests.insert( itRequest, Request );
}
