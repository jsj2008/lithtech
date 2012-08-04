// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGen.cpp
//
// PURPOSE : AI NavMesh generator class implementation.
//
// CREATED : 11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshGenTypes.h"
#include "AINavMeshGen.h"
#include "AINavMeshGenPoly.h"
#include "AINavMeshGenCarver.h"
#include "AINavMeshGenQuadTree.h"
#include "AINavMeshGenMacros.h"
#include <time.h>
#include "ltsphere.h"
#include "iltoutconverter.h"

// Statics.

CAINavMeshGen* CAINavMeshGen::m_pAINavMeshGen = NULL;
const float CAINavMeshGen::kfEpsilon = 0.1f;
const float CAINavMeshGen::kfAreaThreshold = 32.f * 32.f;

// Defines.

#define NODE_CLUSTER_RADIUS			120.f

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::Con/destructor
//              
//	PURPOSE:	Con/destructor
//              
//----------------------------------------------------------------------------

CAINavMeshGen::CAINavMeshGen( IGameWorldPacker::PFNERRORCALLBACK pfnErrorCallback ) :
	m_pfnErrorCallback(pfnErrorCallback)
{
	m_pAINavMeshGen = this;

	m_bNMGInitialized = false;

	m_nAINavMeshVersion = 0;

	m_nNextNMGPolyID = 0;
	m_nNextNMGVertID = 0;
	m_nNextNMGCarverID = 0;
	m_nNextNMGRegionID = 0;
	m_nNextNMGEdgeID = 0;
	m_nNextNMGQTNodeID = 0;

	m_pNMGQuadTree = NULL;
}

CAINavMeshGen::~CAINavMeshGen()
{
	TermNavMeshGen();
	m_pAINavMeshGen = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetAINavMeshGen()
//              
//	PURPOSE:	Access the singleton.
//              
//----------------------------------------------------------------------------

CAINavMeshGen* CAINavMeshGen::GetAINavMeshGen()
{
	return m_pAINavMeshGen;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::InitNavMeshGen()
//              
//	PURPOSE:	Initialize NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::InitNavMeshGen( const LTVector& vWorldOffset )
{
	clock_t nStartTime = clock();

	m_vWorldOffset = vWorldOffset;

	NAVMESH_MSG( "Processing AINavMesh..." );
	NAVMESH_MSG( "Imported from WorldEdit:" );
	NAVMESH_MSG1( "   Polys: %d", m_lstNMGPolys.size() );
	NAVMESH_MSG1( "   Carvers: %d", m_lstNMGCarvers.size() );

	// Assign character type masks.

	AssignCharTypeMasks();

	// Merge neighbors.

	MergeNeighbors_HertelMehlhorn();
	MergeNeighbors_3_2();
	MergeNeighbors_HertelMehlhorn();

	NAVMESH_MSG( "Merged polys:" );
	NAVMESH_MSG1( "   Polys: %d", m_lstNMGPolys.size() );

	// Carve holes where Carvers lie.

	CarveNMGCarvers( &m_lstNMGCarvers, kNMGRegion_Invalid );

	NAVMESH_MSG( "Carved polys:" );
	NAVMESH_MSG1( "   Polys: %d", m_lstNMGPolys.size() );

	MergeNeighbors_HertelMehlhorn();

	NAVMESH_MSG( "Merged polys:" );
	NAVMESH_MSG1( "   Polys: %d", m_lstNMGPolys.size() );

	// Carve polys to match specified AIRegions.

	CarveNMGRegions();

	NAVMESH_MSG( "AIRegion polys:" );
	NAVMESH_MSG1( "   Polys: %d", m_lstNMGPolys.size() );

	// Find adjacencies between polys.  Break edges so that adjacent
	// polys share entire edges.

	FindAdjacencies( &m_lstSplittingVerts );

	// Where edges were split to form adjacencies, split polygons.
	// This creates a mesh that automatically prevents AI from hugging walls.

	SplitAdjacentPolys( &m_lstSplittingVerts );

	NAVMESH_MSG( "Split adjacent  polys:" );
	NAVMESH_MSG1( "   Polys: %d", m_lstNMGPolys.size() );

	// Analyze the NavMesh to precompute what portions are connected to what.

	CreateConnectedComponents();

	// Divide the NavMesh into sensory components, separated by LoseTargetLinks.

	CreateSensoryComponents();

	// Mark verts that touch border edges.

	MarkBorderVerts();

	clock_t nEndTime = clock();
	NAVMESH_MSG1( "Processed NavMesh in %.2f seconds.", ( nEndTime - nStartTime ) / (float)CLOCKS_PER_SEC );

	ClusterAINodes();

	m_bNMGInitialized = true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::TermNavMeshGen()
//              
//	PURPOSE:	Terminate NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::TermNavMeshGen()
{
	// Delete polys.

	CAINavMeshGenPoly* pPoly;
	AINAVMESHGEN_POLY_LIST::iterator itNavMesh;
	for( itNavMesh = m_lstNMGPolys.begin(); itNavMesh != m_lstNMGPolys.end(); ++itNavMesh )
	{
		pPoly = *itNavMesh;
		NAVMESH_DELETE( pPoly );
	}
	m_lstNMGPolys.resize( 0 );

	// Delete verts.

	SAINAVMESHGEN_VERT* pVert;
	AINAVMESHGEN_VERT_MAP::iterator itVert;
	for( itVert = m_mapNMGVertPool.begin(); itVert != m_mapNMGVertPool.end(); ++itVert )
	{
		pVert = itVert->second;
		NAVMESH_DELETE( pVert );
	}
	m_mapNMGVertPool.clear();

	// Delete edges.

	SAINAVMESHGEN_EDGE* pEdge;
	AINAVMESHGEN_EDGE_MAP::iterator itEdge;
	for( itEdge = m_mapNMGEdges.begin(); itEdge != m_mapNMGEdges.end(); ++itEdge )
	{
		pEdge = itEdge->second;
		NAVMESH_DELETE( pEdge );
	}
	m_mapNMGEdges.clear();

	// Delete carvers.

	CAINavMeshGenCarver* pCarver;
	AINAVMESHGEN_CARVER_LIST::iterator itCarver;
	for( itCarver = m_lstNMGCarvers.begin(); itCarver != m_lstNMGCarvers.end(); ++itCarver )
	{
		pCarver = *itCarver;
		NAVMESH_DELETE( pCarver );
	}
	m_lstNMGCarvers.resize( 0 );

	// Delete AIRegions.

	SAINAVMESHGEN_REGION* pAIRegion;
	AINAVMESHGEN_REGION_LIST::iterator itRegion;
	for( itRegion = m_lstNMGRegions.begin(); itRegion != m_lstNMGRegions.end(); ++itRegion )
	{
		pAIRegion = *itRegion;
		for( itCarver = pAIRegion->lstCarvers.begin(); itCarver != pAIRegion->lstCarvers.end(); ++itCarver )
		{
			pCarver = *itCarver;
			NAVMESH_DELETE( pCarver );
		}
		pAIRegion->lstCarvers.resize( 0 );

		NAVMESH_DELETE( pAIRegion );
	}
	m_lstNMGRegions.resize( 0 );

	// Delete Components.

	SAINAVMESHGEN_COMPONENT* pComponent;
	AINAVMESHGEN_COMPONENT_LIST::iterator itComponent;
	for( itComponent = m_lstNMGComponents.begin(); itComponent != m_lstNMGComponents.end(); ++itComponent )
	{
		pComponent = *itComponent;
		pComponent->lstNeighborComponentPolys.resize( 0 );
		NAVMESH_DELETE( pComponent );
	}
	m_lstNMGComponents.resize( 0 );

	// Delete quad tree.

	if( m_pNMGQuadTree )
	{
		NAVMESH_DELETE( m_pNMGQuadTree );
		m_pNMGQuadTree = NULL;
	}

	m_bNMGInitialized = false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::MergeNeighbors_HertelMehlhorn()
//              
//	PURPOSE:	Merge neighboring polys that share an edge and 2 verts,
//				using the Hertel-Mehlhorn algorithm.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::MergeNeighbors_HertelMehlhorn()
{
	// Merge polygons using the Hertel-Mehlhorn algorithm, as described
	// in AI Game Programming Wisdom, p. 176.

	AINAVMESHGEN_EDGE_MAP::iterator itEdge;
	
	CAINavMeshGenPoly* pPolyA;
	CAINavMeshGenPoly* pPolyB;
	CAINavMeshGenPoly* pPolyC;

	AINAVMESHGEN_POLY_LIST	lstNewNavMeshPolys;

	bool bMergedPolys = true;
	SAINAVMESHGEN_EDGE* pEdge;

	while( bMergedPolys )
	{
		bMergedPolys = false;
		for( itEdge = m_mapNMGEdges.begin(); itEdge != m_mapNMGEdges.end(); ++itEdge )
		{
			pEdge = itEdge->second;
			if( pEdge &&
				( pEdge->ePolyID1 != kNMGPoly_Invalid ) &&
				( pEdge->ePolyID2 != kNMGPoly_Invalid ) )
			{
				pPolyA = GetNMGPoly( pEdge->ePolyID1 );
				pPolyB = GetNMGPoly( pEdge->ePolyID2 );

				// Don't merge polys that have NavMeshLinks.

				if( ( pPolyA->GetNMGLinkID() != kNMGLink_Invalid ) ||
					( pPolyB->GetNMGLinkID() != kNMGLink_Invalid ) )
				{
					continue;
				}

				// Only merge polys that have the same normal.

				if( pPolyA->GetNMGNormalID() != pPolyB->GetNMGNormalID() )
				{
					continue;
				}

				// Only merge polys that have the same character type mask.

				if( pPolyA->GetNMGCharTypeMask() != pPolyB->GetNMGCharTypeMask() )
				{
					continue;
				}

				pPolyC = NAVMESH_NEW1( CAINavMeshGenPoly, m_pfnErrorCallback );
				MergeNMGPolys( pEdge->eVertMin, pEdge->eVertMax, pPolyA, pPolyB, pPolyC );

				// Do not create concave polys.

				if( !pPolyC->IsNMGPolyConvex() )
				{
					NAVMESH_DELETE( pPolyC );
					continue;
				}

				// Delete original polys.

				RemoveNMGPoly( pPolyA );
				RemoveNMGPoly( pPolyB );
				NAVMESH_DELETE( pPolyA );
				NAVMESH_DELETE( pPolyB );

				// Add new poly.

				lstNewNavMeshPolys.push_back( pPolyC );
				bMergedPolys = true;
			}
		}

		// Add new polys to NavMesh.

		AINAVMESHGEN_POLY_LIST::iterator itNew;
		for( itNew = lstNewNavMeshPolys.begin(); itNew != lstNewNavMeshPolys.end(); ++itNew )
		{
			AddNMGPoly( *itNew );
		}

		lstNewNavMeshPolys.resize( 0 );

		// Remove unused edges and verts.

		CompactNMGEdgeList();
		CompactNMGVertPool();

		// Cleaup quad tree.

		m_pNMGQuadTree->CompactNMGQTNode();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::MergeNMGPolys()
//              
//	PURPOSE:	Merge neighboring polys that share an edge.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::MergeNMGPolys( ENUM_NMGVertID eVert0, ENUM_NMGVertID eVert1, CAINavMeshGenPoly* pPolyA, CAINavMeshGenPoly* pPolyB, CAINavMeshGenPoly* pPolyC )
{
	// Find clockwise-most edge vert of polyA.

	int cMatch = 0;
	int iVert = 0;
	int cVerts = pPolyA->GetNumNMGVerts();
	ENUM_NMGVertID eStartVert = kNMGVert_Invalid;
	ENUM_NMGVertID eEndVert = kNMGVert_Invalid;

	while( cMatch != 2 )
	{
		eStartVert = pPolyA->GetNMGVert( iVert );
		if( eStartVert == eVert0 )
		{
			eEndVert = eVert1;
			++cMatch;
		}
		else if( eStartVert == eVert1 )
		{
			eEndVert = eVert0;
			++cMatch;
		}
		else {
			cMatch = 0;
		}

		iVert = ( iVert + 1 ) % cVerts;
	}

	// Add polyA's verts to polyC.

	pPolyC->CopyNMGVerts( pPolyA, eStartVert );

	// Add polyB's verts to polyC.

	pPolyC->CopyNMGVerts( pPolyB, eEndVert );

	pPolyC->SimplifyNMGPoly();

	// Init poly to calculate poly normal.

	pPolyC->InitNMGPoly();

	// Copy the parents' character type mask.

	pPolyC->SetNMGCharTypeMask( pPolyA->GetNMGCharTypeMask() );

	// Copy the parents' parent nav mesh object ID.

	pPolyC->SetNMGParentNavMeshID( pPolyA->GetNMGParentNavMeshID() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::MergeNeighbors_3_2()
//              
//	PURPOSE:	Merge neighboring polys that share another neighbor,
//				using the 3->2 Merge algorithm.
//            
//              Example: Merge 3 polys A,B,C into 2 polys.
//                       Remove shared verts 1 and 2, and create
//                       an edge from 4 to 5.
//
//                --------------        --------------
//                |  C         |        |  E         |
//                |            |        |            |
//                6------1-----7   ->   |            |
//                | A    |  B  |        |            |
//                |      |     |        |            |
//                4------2.....5        4------------5
//                       |     |               |  B2 |
//                       3------               3------
//
//----------------------------------------------------------------------------

void CAINavMeshGen::MergeNeighbors_3_2()
{
	// Merge polygons using the 3->2 Merge algorithm, as described
	// in AI Game Programming Wisdom, p. 177.

	ENUM_NMGPolyID ePolyA, ePolyB;
	CAINavMeshGenPoly* pPolyA;
	CAINavMeshGenPoly* pPolyB;
	CAINavMeshGenPoly* pPolyC;

	SAINAVMESHGEN_MERGE_3_2_RESULTS  mrg32;

	AINAVMESHGEN_POLY_LIST	lstNewNavMeshPolys;

	bool bMergedPolys = true;

	while( bMergedPolys )
	{
		bMergedPolys = false;
		mrg32.Init();

		// Find 3->2 Merge candidates.

		SAINAVMESHGEN_VERT* pVert;
		AINAVMESHGEN_VERT_MAP::iterator itVert;
		for( itVert = m_mapNMGVertPool.begin(); itVert != m_mapNMGVertPool.end(); ++itVert )
		{
			// Find verts shared by exactly 2 polys.
		
			pVert = itVert->second;
			if( pVert->lstPolyRefs.size() == 2 )
			{
				ePolyA = *( pVert->lstPolyRefs.begin() );
				pPolyA = GetNMGPoly( ePolyA );

				ePolyB = *( pVert->lstPolyRefs.begin() + 1 );
				pPolyB = GetNMGPoly( ePolyB );

				// Polys may have been deleted in previous iterations.

				if( !( pPolyA && pPolyB ) )
				{
					continue;
				}

				// Don't merge polys that have NavMeshLinks.

				if( ( pPolyA->GetNMGLinkID() != kNMGLink_Invalid ) ||
					( pPolyB->GetNMGLinkID() != kNMGLink_Invalid ) )
				{
					continue;
				}

				// Only merge polys that have the same normal.

				if( pPolyA->GetNMGNormalID() != pPolyB->GetNMGNormalID() )
				{
					continue;
				}

				// Ignore vert if polys A and B share an entire edge (share more than 1 vert).

				mrg32.eVert1 = itVert->first;
				if( pPolyA->SharesNMGEdge( mrg32.eVert1, ePolyB ) )
				{
					continue;
				}

				if( !pPolyA->IsValidForMerge32( &mrg32, pPolyB, this ) )
				{
					continue;
				}

				pPolyA = GetNMGPoly( mrg32.ePolyA );
				pPolyB = GetNMGPoly( mrg32.ePolyB );
				pPolyC = GetNMGPoly( mrg32.ePolyC );

				// Find new vertex vert5 to use to split polyB, if one exists.

				SAINAVMESHGEN_VERT* pVert2 = GetNMGVert( mrg32.eVert2 );
				SAINAVMESHGEN_VERT* pVert4 = GetNMGVert( mrg32.eVert4 );

				// Create a ray starting at vert4 and extending past vert2.

				LTVector vRayDir = pVert2->vVert - pVert4->vVert;
				vRayDir.Normalize();
				LTVector vRay0 = pVert4->vVert;
				LTVector vRay1 = pVert4->vVert + ( vRayDir * 50000.f );

				// Find ray intersection with poly.

				SAINAVMESHGEN_POLY_INTERSECT pintersect;
				pintersect.v0 = vRay0;
				pintersect.v1 = vRay1;
				if( !pPolyB->CoplanarRayIntersectPoly( &pintersect ) )
				{
					continue;
				}

				mrg32.eVert5 = pintersect.eVertIntersectFar;


				// Split polyB into polyB1 and polyB2.

				CAINavMeshGenPoly* pPoly0 = NULL;
				CAINavMeshGenPoly* pPoly1 = NULL;
				pPolyB->SplitPoly( &pintersect, pPoly0, pPoly1 );

				if( pPoly0 && pPoly0->ContainsNMGVert( mrg32.eVert1 ) )
				{
					mrg32.pPolyB1 = pPoly0;
					mrg32.pPolyB2 = pPoly1;
				}
				else {
					mrg32.pPolyB1 = pPoly1;
					mrg32.pPolyB2 = pPoly0;
				}

				
				// Merge polyA and polyB1 into polyD.

				CAINavMeshGenPoly* pPolyD = NAVMESH_NEW1( CAINavMeshGenPoly, m_pfnErrorCallback );
				MergeNMGPolys( mrg32.eVert1, mrg32.eVert2, pPolyA, mrg32.pPolyB1, pPolyD );

				// Merge polyC and polyD into polyE.

				CAINavMeshGenPoly* pPolyE = NAVMESH_NEW1( CAINavMeshGenPoly, m_pfnErrorCallback );
				MergeNMGPolys( mrg32.eVert6, mrg32.eVert7, pPolyC, pPolyD, pPolyE );

				// Bail if merging will create a concave poly.

				if( !pPolyE->IsNMGPolyConvex() )
				{
					NAVMESH_DELETE( pPolyD );
					NAVMESH_DELETE( pPolyE );
					NAVMESH_DELETE( mrg32.pPolyB1 );
					NAVMESH_DELETE( mrg32.pPolyB2 );
					continue;
				}

				// Delete original and intermediate polys.

				RemoveNMGPoly( pPolyA );
				RemoveNMGPoly( pPolyB );
				RemoveNMGPoly( pPolyC );
				NAVMESH_DELETE( pPolyA );
				NAVMESH_DELETE( pPolyB );
				NAVMESH_DELETE( pPolyC );
				NAVMESH_DELETE( pPolyD );
				NAVMESH_DELETE( mrg32.pPolyB1 );

				// Add new polys polyE and polyB2.

				lstNewNavMeshPolys.push_back( pPolyE );
				lstNewNavMeshPolys.push_back( mrg32.pPolyB2 );
				bMergedPolys = true;
			}
		}

		// Add new polys to NavMesh.

		AINAVMESHGEN_POLY_LIST::iterator itNew;
		for( itNew = lstNewNavMeshPolys.begin(); itNew != lstNewNavMeshPolys.end(); ++itNew )
		{
			AddNMGPoly( *itNew );
		}

		lstNewNavMeshPolys.resize( 0 );

		// Remove unused edges and verts.

		CompactNMGEdgeList();
		CompactNMGVertPool();
	}

	// Cleanup quad tree.

	m_pNMGQuadTree->CompactNMGQTNode();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::CarveNMGCarvers()
//              
//	PURPOSE:	Carve holes where Carvers lie in polys.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::CarveNMGCarvers( AINAVMESHGEN_CARVER_LIST* plstNMGCarvers, ENUM_NMGRegionID eRegionID )
{
	// Sanity check.

	if( !plstNMGCarvers )
	{
		return;
	}

	CAINavMeshGenPoly* pPoly;
	CAINavMeshGenCarver* pCarver;

	NMGPOLY_LIST lstIntersectingPolys;
	NMGPOLY_LIST::iterator itPolyRef;

	AINAVMESHGEN_POLY_LIST lstNewNavMeshPolys;
	AINAVMESHGEN_POLY_LIST lstDeleteNavMeshPolys;
	AINAVMESHGEN_POLY_LIST lstCarvedNavMeshPolys;

	// Iterate over all Carvers.

	AINAVMESHGEN_POLY_LIST::iterator itPoly;
	AINAVMESHGEN_CARVER_LIST::iterator itCarver;
	for( itCarver = plstNMGCarvers->begin(); itCarver != plstNMGCarvers->end(); ++itCarver )
	{
		pCarver = *itCarver;

		// Find polys that intersect the Carver's bounding box.

		m_pNMGQuadTree->GetIntersectingPolys( pCarver->GetAABB(), &lstIntersectingPolys );
		for( itPolyRef = lstIntersectingPolys.begin(); itPolyRef != lstIntersectingPolys.end(); ++itPolyRef )
		{
			pPoly = GetNMGPoly( *itPolyRef );
			if( pPoly )
			{
				// Don't carve polys that have NavMeshLinks.

				if( pPoly->GetNMGLinkID() != kNMGLink_Invalid )
				{
					if( eRegionID != kNMGRegion_Invalid )
					{
						if( pCarver->CarveNMGPoly( pPoly, NULL, NULL, NULL ) )
						{
							AddPolyToAIRegion( pPoly->GetNMGPolyID(), eRegionID );
						}
					}
					continue;
				}

				// TRACE( "Carver: %d Poly: %d\n", pCarver->GetNMGCarverID(), pPoly->GetNMGPolyID() );

				// Create list of new polys, and polys to delete after carving.

				pCarver->CarveNMGPoly( pPoly, &lstNewNavMeshPolys, &lstDeleteNavMeshPolys, &lstCarvedNavMeshPolys );
			}
		}


		// Delete old polys.

		for( itPoly = lstDeleteNavMeshPolys.begin(); itPoly != lstDeleteNavMeshPolys.end(); ++itPoly )
		{
			pPoly = *itPoly;
			if( pPoly->GetNMGPolyID() != kNMGPoly_Invalid )
			{
				RemoveNMGPoly( pPoly );
			}
			NAVMESH_DELETE( pPoly );
		}
		lstDeleteNavMeshPolys.resize( 0 );


		// Add or Delete carved polys.
		// Carved polys are the polys that fall inside of the carver.
		// They may either be discarded, if this carver is not carving an AIRegion,
		// or preserved is this carver is carving a new AIRegion.

		for( itPoly = lstCarvedNavMeshPolys.begin(); itPoly != lstCarvedNavMeshPolys.end(); ++itPoly )
		{
			pPoly = *itPoly;

			// Preserved the poly.

			if( eRegionID != kNMGRegion_Invalid )
			{
				if( pPoly->GetNMGPolyID() == kNMGPoly_Invalid )
				{
					AddNMGPoly( pPoly );
				}
				AddPolyToAIRegion( pPoly->GetNMGPolyID(), eRegionID );
			}

			// Discard the poly.

			else {
				if( pPoly->GetNMGPolyID() != kNMGPoly_Invalid )
				{
					RemoveNMGPoly( pPoly );
				}
				NAVMESH_DELETE( pPoly );
			}
		}
		lstCarvedNavMeshPolys.resize( 0 );


		// Add new polys.

		for( itPoly = lstNewNavMeshPolys.begin(); itPoly != lstNewNavMeshPolys.end(); ++itPoly )
		{
			pPoly = *itPoly;
			AddNMGPoly( pPoly );
		}
		lstNewNavMeshPolys.resize( 0 );


		// Remove unused edges and verts.

		CompactNMGEdgeList();
		CompactNMGVertPool();
		CompactAIRegions();
	}

	// Cleanup quad tree.

	m_pNMGQuadTree->CompactNMGQTNode();
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::CarveNMGRegions()
//              
//	PURPOSE:	Carve polys to match specified AIRegions.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::CarveNMGRegions()
{
	SAINAVMESHGEN_REGION* pAIRegion;
	AINAVMESHGEN_REGION_LIST::iterator itRegion;
	for( itRegion = m_lstNMGRegions.begin(); itRegion != m_lstNMGRegions.end(); ++itRegion )
	{
		pAIRegion = *itRegion;
		CarveNMGCarvers( &( pAIRegion->lstCarvers ), pAIRegion->eNMGRegionID );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::DiscardTrivialNMGPolys()
//              
//	PURPOSE:	Discard polys that are too small to walk on.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::DiscardTrivialNMGPolys()
{
	AINAVMESHGEN_POLY_LIST lstDeleteNavMeshPolys;

	// Iterate over all polys.

	CAINavMeshGenPoly* pPoly;
	AINAVMESHGEN_POLY_LIST::iterator itPoly;
	for( itPoly = m_lstNMGPolys.begin(); itPoly != m_lstNMGPolys.end(); ++itPoly )
	{
		pPoly = *itPoly;

		if( !pPoly->IsNMGPolyAreaGreaterThan( kfAreaThreshold ) )
		{
			lstDeleteNavMeshPolys.push_back( pPoly );
		}
	}

	// Delete old polys.

	for( itPoly = lstDeleteNavMeshPolys.begin(); itPoly != lstDeleteNavMeshPolys.end(); ++itPoly )
	{
		pPoly = *itPoly;
		RemoveNMGPoly( pPoly );
		NAVMESH_DELETE( pPoly );
	}
	lstDeleteNavMeshPolys.resize( 0 );

	// Remove unused edges and verts.

	CompactNMGEdgeList();
	CompactNMGVertPool();
	CompactAIRegions();

	// Cleanup quad tree.

	m_pNMGQuadTree->CompactNMGQTNode();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::FindAdjacencies()
//              
//	PURPOSE:	Find adjacent polys. Break edges of polys
//              so that adjacent polys share entire edges.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::FindAdjacencies( AINAVMESHGEN_SPLITTING_VERT_LIST* plstSplittingVerts )
{
	NMGPOLY_LIST lstIntersectingPolys;
	NMGPOLY_LIST::iterator itPolyRef;

	ENUM_NMGVertID eSplittingVertA, eSplittingVertB;
	SAINAVMESHGEN_SPLITTING_VERT SplittingVert;

	CAINavMeshGenPoly* pPolyA;
	CAINavMeshGenPoly* pPolyB;
	SAINAVMESHGEN_EDGE* pEdgeA;
	SAINAVMESHGEN_EDGE* pEdgeB;

	// Iterate over all polys.

	AINAVMESHGEN_POLY_LIST::iterator itPolyA;
	for( itPolyA = m_lstNMGPolys.begin(); itPolyA != m_lstNMGPolys.end(); ++itPolyA )
	{
		pPolyA = *itPolyA;

		// Find polys that intersect this poly's bounding box.

		lstIntersectingPolys.resize( 0 );
		m_pNMGQuadTree->GetIntersectingPolys( pPolyA->GetAABB(), &lstIntersectingPolys );

		// Iterate over all polys that have bounds that intersect polyA.

		for( itPolyRef = lstIntersectingPolys.begin(); itPolyRef != lstIntersectingPolys.end(); ++itPolyRef )
		{
			pPolyB = GetNMGPoly( *itPolyRef );
			if( ( !pPolyB ) ||
				( pPolyB == pPolyA ) )
			{
				continue;
			}

			// Determine if polyA and polyB are adjacent.

			if( pPolyA->FindAdjacentNMGEdges( pPolyB, pEdgeA, pEdgeB ) )
			{
				//TRACE( "Adjacent Polys: %d %d\n", pPolyA->GetNMGPolyID(), pPolyB->GetNMGPolyID() );

				// Remove polys from verts lists, since poly verts
				// may change when edges are split.

				ReleaseNMGVerts( pPolyA );
				ReleaseNMGVerts( pPolyB );

				// Split adjacent colinear edges, so that adjacent polys polyA 
				// and polyB share an entire edge.

				SplittingVert.ePoly = pPolyA->GetNMGPolyID();
				pPolyA->SplitAdjacentNMGEdge( pEdgeA, pEdgeB, &eSplittingVertA, &eSplittingVertB );
				if( eSplittingVertA != kNMGVert_Invalid )
				{
					SplittingVert.eVert = eSplittingVertA;
					plstSplittingVerts->push_back( SplittingVert );
				}
				if( eSplittingVertB != kNMGVert_Invalid )
				{
					SplittingVert.eVert = eSplittingVertB;
					plstSplittingVerts->push_back( SplittingVert );
				}

				SplittingVert.ePoly = pPolyB->GetNMGPolyID();
				pPolyB->SplitAdjacentNMGEdge( pEdgeB, pEdgeA, &eSplittingVertA, &eSplittingVertB );
				if( eSplittingVertA != kNMGVert_Invalid )
				{
					SplittingVert.eVert = eSplittingVertA;
					plstSplittingVerts->push_back( SplittingVert );
				}
				if( eSplittingVertB != kNMGVert_Invalid )
				{
					SplittingVert.eVert = eSplittingVertB;
					plstSplittingVerts->push_back( SplittingVert );
				}

				// Add polys to verts lists.

				AccessNMGVerts( pPolyA );
				AccessNMGVerts( pPolyB );
			}
		}
	}

	// Clean up.

	CompactNMGVertPool();
	CompactNMGEdgeList();
	CompactAIRegions();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::SplitAdjacentPolys()
//              
//	PURPOSE:	Split polys that have edges that were split during the 
//              adjacency finding process.  Splitting these polys creates
//              a mesh that automatically keeps AI from hugging walls.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::SplitAdjacentPolys( AINAVMESHGEN_SPLITTING_VERT_LIST* plstSplittingVerts )
{
	AINAVMESHGEN_SPLITTING_VERT_LIST::iterator itSplit;
	SAINAVMESHGEN_SPLITTING_VERT Split;

	CAINavMeshGenPoly* pPolyA = NULL;
	CAINavMeshGenPoly* pPolyB = NULL;

	CAINavMeshGenPoly* pPoly;

	// Iterate thru splitting verts -- verts 
	// added during the adjacency finding step.

	while( !plstSplittingVerts->empty() )
	{
		// Erase each splitting vert.

		itSplit = plstSplittingVerts->begin();
		Split = *itSplit;
		plstSplittingVerts->erase( itSplit );

		// Bail if split poly does not exist.

		pPoly = GetNMGPoly( Split.ePoly );
		if( !pPoly )
		{
			continue;
		}

		// Don't split polys that have NavMeshLinks.

		if( pPoly->GetNMGLinkID() != kNMGLink_Invalid )
		{
			continue;
		}

		// Split poly on the inserted vert.

		if( !pPoly->SplitPolyOnVert( Split.eVert, plstSplittingVerts, pPolyA, pPolyB ) )
		{
			continue;
		}

		// Remove the old poly.

		RemoveNMGPoly( pPoly );
		NAVMESH_DELETE( pPoly );

		// Add the new polys.

		AddNMGPoly( pPolyA );
		AddNMGPoly( pPolyB );

		// Reassign poly IDs for splitting verts referring 
		// to deleted poly.

		for( itSplit = plstSplittingVerts->begin(); itSplit != plstSplittingVerts->end(); ++itSplit )
		{
			if( itSplit->ePoly == Split.ePoly )
			{
				if( pPolyA->ContainsNMGVert( itSplit->eVert ) )
				{
					itSplit->ePoly = pPolyA->GetNMGPolyID();
				}

				else if( pPolyB->ContainsNMGVert( itSplit->eVert ) )
				{
					itSplit->ePoly = pPolyB->GetNMGPolyID();
				}
			}
		}
	}

	// Clean up.

	CompactNMGVertPool();
	CompactNMGEdgeList();
	CompactAIRegions();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddNMGComponent()
//              
//	PURPOSE:	Add a new NavMesh component.
//              
//----------------------------------------------------------------------------

SAINAVMESHGEN_COMPONENT* CAINavMeshGen::AddNMGComponent( ENUM_NMGComponentID eComponent )
{
	// Sanity check.

	if( eComponent == kNMGComponent_Invalid )
	{
		return NULL;
	}

	// Create a new component.

	SAINAVMESHGEN_COMPONENT* pComponent = NAVMESH_NEW( SAINAVMESHGEN_COMPONENT );
	pComponent->eNMGComponentID = eComponent;

	// Add the component to the list.

	m_lstNMGComponents.push_back( pComponent );

	// Return the component.

	return pComponent;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetNMGComponent()
//              
//	PURPOSE:	Return the specified component.
//              
//----------------------------------------------------------------------------

SAINAVMESHGEN_COMPONENT* CAINavMeshGen::GetNMGComponent( ENUM_NMGComponentID eComponent )
{
	// Sanity check.

	if( ( eComponent == kNMGComponent_Invalid ) || 
		( (uint32)eComponent >= m_lstNMGComponents.size() ) )
	{
		return NULL;
	}

	// Return the component.

	return m_lstNMGComponents[eComponent];
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::CreateConnectedComponents()
//              
//	PURPOSE:	Create connected components.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::CreateConnectedComponents()
{
	int nNextComponentID = 0;
	ENUM_NMGComponentID eComponent;

	int cNeighbors;
	int iNeighbor;
	ENUM_NMGPolyID ePolyNeighbor;

	SAINAVMESHGEN_COMPONENT* pComponent;

	// Assign every poly to a component.

	CAINavMeshGenPoly* pPoly;
	AINAVMESHGEN_POLY_LIST::iterator itPoly;
	for( itPoly = m_lstNMGPolys.begin(); itPoly != m_lstNMGPolys.end(); ++itPoly )
	{
		// Skip polys that have already been assigned to components.

		pPoly = *itPoly;
		if( pPoly->GetNMGComponentID() != kNMGComponent_Invalid )
		{
			continue;
		}

		// Create a new component, and assign it the next available 
		// unique component ID.

		eComponent = (ENUM_NMGComponentID)nNextComponentID;
		pComponent = AddNMGComponent( eComponent );
		++nNextComponentID;

		// Assign the poly the next available unique component ID.

		pPoly->SetNMGComponentID( eComponent );

		// Flood-fill poly's neighbors with the same ID.
		// NavMeshLinks are their own components.

		if( pPoly->GetNMGLinkID() == kNMGLink_Invalid )
		{
			FloodFillComponentID( pPoly );
		}

		// Record the link and its neighbor component polys.

		else 
		{
			pComponent->eNMGLinkID = pPoly->GetNMGLinkID();

			cNeighbors = pPoly->GetNumNMGPolyNeighbors();
			for( iNeighbor=0; iNeighbor < cNeighbors; ++iNeighbor )
			{
				// Skip non-existent neighbors.

				ePolyNeighbor = pPoly->GetNMGPolyNeighbor( iNeighbor );
				if( ePolyNeighbor != kNMGPoly_Invalid )
				{
					pComponent->lstNeighborComponentPolys.push_back( ePolyNeighbor );
				}
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::FloodFillComponentID()
//              
//	PURPOSE:	Recursively flood-fill neighbors with matching component ID.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::FloodFillComponentID( CAINavMeshGenPoly* pPoly )
{
	// Sanity check.

	if( !pPoly )
	{
		return;
	}

	ENUM_NMGComponentID eComponent = pPoly->GetNMGComponentID();

	// Iterate over neighbor polys and recursively flood-fill.

	ENUM_NMGPolyID ePolyNeighbor;
	CAINavMeshGenPoly* pPolyNeighbor;
	int cNeighbors = pPoly->GetNumNMGPolyNeighbors();
	for( int iNeighbor=0; iNeighbor < cNeighbors; ++iNeighbor )
	{
		// Skip non-existent neighbors.

		ePolyNeighbor = pPoly->GetNMGPolyNeighbor( iNeighbor );
		if( ePolyNeighbor == kNMGPoly_Invalid )
		{
			continue;
		}

		pPolyNeighbor = GetNMGPoly( ePolyNeighbor );
		if( !pPolyNeighbor )
		{
			continue;
		}

		// Stop flood-filling at NavMeshLinks.
		// Stop flood-filling if polys have different restrictions.

		if( ( pPolyNeighbor->GetNMGLinkID() != kNMGLink_Invalid ) ||
			( pPolyNeighbor->GetNMGCharTypeMask() != pPoly->GetNMGCharTypeMask() ) )
		{
			// Record component's neighbor component polys.

			SAINAVMESHGEN_COMPONENT* pComponent = GetNMGComponent( eComponent );
			if( pComponent )
			{
				// Find the neighbor in the list.

				NMGPOLY_LIST::iterator itNeighbor;
				for( itNeighbor = pComponent->lstNeighborComponentPolys.begin(); itNeighbor != pComponent->lstNeighborComponentPolys.end(); ++itNeighbor )
				{
					if( ePolyNeighbor == *itNeighbor )
					{
						break;
					}
				}
				
				// Add the neighbor component to the list, if it is not already in the list.

				if( itNeighbor == pComponent->lstNeighborComponentPolys.end() )
				{
					pComponent->lstNeighborComponentPolys.push_back( ePolyNeighbor );
				}
			}
			continue;
		}

		// Skip polys that are already assigned to components.

		if( pPolyNeighbor->GetNMGComponentID() != kNMGComponent_Invalid )
		{
			if( pPolyNeighbor->GetNMGComponentID() != eComponent )
			{
				NAVMESH_ERROR( "CAINavMeshGen::FloodFillComponentID: Multiple component IDs found for same component." );
			}
			continue;
		}

		// Assign poly to component.

		pPolyNeighbor->SetNMGComponentID( eComponent );

		// Flood-fill neighbor's neighbors.

		FloodFillComponentID( pPolyNeighbor );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::CreateSensoryComponents()
//              
//	PURPOSE:	Create sensory components.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::CreateSensoryComponents()
{
	int nNextSensoryComponentID = 0;
	ENUM_NMGSensoryComponentID eSensoryComponent;

	// Assign every component to a sensory component.

	SAINAVMESHGEN_COMPONENT* pComponent;
	AINAVMESHGEN_COMPONENT_LIST::iterator itComponent;
	for( itComponent = m_lstNMGComponents.begin(); itComponent != m_lstNMGComponents.end(); ++itComponent )
	{
		// Skip components that have already been assigned to sensory components.

		pComponent = *itComponent;
		if( pComponent->eNMGSensoryComponentID != kNMGSensoryComponent_Invalid )
		{
			continue;
		}

		// Assign the next available unique sensory component ID to the component.

		eSensoryComponent = (ENUM_NMGSensoryComponentID)nNextSensoryComponentID;
		++nNextSensoryComponentID;
		pComponent->eNMGSensoryComponentID = eSensoryComponent;

		// NavMeshLinkLoseTargets are their own sensory components.

		if( pComponent->eNMGLinkID != kNMGLink_Invalid )
		{
			SAINAVMESHGEN_LINK* pLink = GetNMGLink( pComponent->eNMGLinkID );
			if( pLink && pLink->bIsSensoryLink )
			{
				continue;
			}
		}

		// Flood-fill component's neighbors with the same ID.

		FloodFillSensoryComponentID( pComponent );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::FloodFillSensoryComponentID()
//              
//	PURPOSE:	Recursively flood-fill neighbors with matching sensory component ID.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::FloodFillSensoryComponentID( SAINAVMESHGEN_COMPONENT* pComponent )
{
	// Sanity check.

	if( !pComponent )
	{
		return;
	}

	ENUM_NMGSensoryComponentID eSensoryComponent = pComponent->eNMGSensoryComponentID;

	// Iterate over neighbor components and recursively flood-fill.

	ENUM_NMGPolyID ePolyNeighbor;
	CAINavMeshGenPoly* pPolyNeighbor;
	NMGPOLY_LIST::iterator itNeighbor;
	SAINAVMESHGEN_COMPONENT* pComponentNeighbor;
	for( itNeighbor = pComponent->lstNeighborComponentPolys.begin(); itNeighbor != pComponent->lstNeighborComponentPolys.end(); ++itNeighbor )
	{
		// Skip non-existent neighbors.

		ePolyNeighbor = *itNeighbor;
		pPolyNeighbor = GetNMGPoly( ePolyNeighbor );
		if( !pPolyNeighbor )
		{
			continue;
		}

		pComponentNeighbor = GetNMGComponent( pPolyNeighbor->GetNMGComponentID() );
		if( !pComponentNeighbor )
		{
			continue;
		}

		// Stop flood-filling at LoseTarget NavMeshLinks.

		if( pComponentNeighbor->eNMGLinkID != kNMGLink_Invalid )
		{
			SAINAVMESHGEN_LINK* pLink = GetNMGLink( pComponentNeighbor->eNMGLinkID );
			if( pLink && pLink->bIsSensoryLink )
			{
				continue;
			}
		}

		// Skip components that are already assigned to sensory components.

		if( pComponentNeighbor->eNMGSensoryComponentID != kNMGSensoryComponent_Invalid )
		{
			if( pComponentNeighbor->eNMGSensoryComponentID != eSensoryComponent )
			{
				NAVMESH_ERROR( "CAINavMeshGen::FloodFillSensoryComponentID: Multiple sensory component IDs found for same sensory component." );
			}
			continue;
		}

		// Assign component to sensory component.

		pComponentNeighbor->eNMGSensoryComponentID = eSensoryComponent;

		// Flood-fill neighbor's neighbors.

		FloodFillSensoryComponentID( pComponentNeighbor );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::MarkBorderVerts()
//              
//	PURPOSE:	Mark verts that touch a border edge or that touches a navmesh 
//				link. If we don't mark navmesh link edges as borders, AIs may 
//				try to path through an edge which happens to share a point 
//				with a link which the AI can't enter. 
//				
//				If the vertex isn't a border vert, the AI may end up attempting 
//				to enter the link it is prohibitted from entering (or that it 
//				is entering in an unexpected way) which may cause an AI to run 
//				in place or leave the level (ie climbing a ladder on the wrong 
//				side).
//              
//				The navmesh link issue was originally seen when an AI tried to 
//				run around a disabled link. The quadtree lookup placed him in 
//				the link as he ran through the vertex, which prevented him 
//				from moving.
//              
//              We don't technically need AIs to avoid every link vertex (ie 
//				an enabled link doesn't have this issue), but we don't 
//				currently have a better place to store this info (would need 
//				to be refcounted per vertex to handle multiple polies, 
//				serialized, etc). We should do this in the future, but we need
//				to ship at this point.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::MarkBorderVerts()
{
	SAINAVMESHGEN_EDGE* pNMGEdge;
	SAINAVMESHGEN_VERT* pNMGVert;
	AINAVMESHGEN_EDGE_MAP::iterator itEdge;
	for( itEdge = m_mapNMGEdges.begin(); itEdge != m_mapNMGEdges.end(); ++itEdge )
	{
		pNMGEdge = itEdge->second;

		// Found a border edge (an edge that has only one neighbor poly).

		if( ( pNMGEdge->ePolyID1 == kNMGPoly_Invalid ) ||
			( pNMGEdge->ePolyID2 == kNMGPoly_Invalid ) )
		{
			// Mark the min vert as a BorderVert.

			pNMGVert = GetNMGVert( pNMGEdge->eVertMin );
			if( pNMGVert )
			{
				pNMGVert->bIsBorderVert = true;
			}

			// Mark the max vert as a BorderVert.

			pNMGVert = GetNMGVert( pNMGEdge->eVertMax );
			if( pNMGVert )
			{
				pNMGVert->bIsBorderVert = true;
			}
		}
	}

	// Mark all link verts as border verts.

	for( AINAVMESHGEN_LINK_LIST::iterator itNMGLink = m_lstNMGLinks.begin(); itNMGLink != m_lstNMGLinks.end(); ++itNMGLink )
	{
		SAINAVMESHGEN_LINK* pNMGLink = &( *itNMGLink );
		if (!pNMGLink)
		{
			NAVMESH_MSG( "NAVMESH PACKER ASSERT: Failed to find link in list." );
			continue;
		}

		for( std::vector<LTVector>::iterator itBoundary = pNMGLink->lstLinkBounds.begin(); itBoundary != pNMGLink->lstLinkBounds.end(); ++itBoundary )
		{
			const LTVector& vVert = *itBoundary;
			ENUM_NMGVertID eVert = GetExistingVertInPool( vVert );
			pNMGVert = GetNMGVert( eVert );
			if ( pNMGVert )
			{
				pNMGVert->bIsBorderVert = true;
			}
			else
			{
				NAVMESH_MSG("NAVMESH PACKER ASSERT: Failed to find link vertex in pool to mark as border. AI may walk too close to this vertex." );
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetExistingVertInPool()
//              
//	PURPOSE:	Return vertex pool ID for given vertex that already exists. If
//				it doesn't exist, returns kNMGVert_Invalid. 
//              
//----------------------------------------------------------------------------

ENUM_NMGVertID CAINavMeshGen::GetExistingVertInPool( const LTVector& vVert )
{
	// Do not use AINavMeshGen::fEpsilon, because the verts should
	// merge using a more leanient epsilon value.

	float fVertEpsilon = 0.2f;

	// Find existing matching vert.

	SAINAVMESHGEN_VERT* pVert;
	AINAVMESHGEN_VERT_MAP::iterator itVert;
	for( itVert = m_mapNMGVertPool.begin(); itVert != m_mapNMGVertPool.end(); ++itVert )
	{
		pVert = itVert->second;
		if( vVert.NearlyEquals( pVert->vVert, fVertEpsilon ) )
		{	
			if (kNMGVert_Invalid == itVert->first)
			{
				NAVMESH_MSG("NAVMESH PACKER LOGIC ASSERT: Invalid vertex found in VertPool.");
			}
			return itVert->first;
		}
	}

	return kNMGVert_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddVertToPool()
//              
//	PURPOSE:	Return vertex pool ID for given vertex.
//              
//----------------------------------------------------------------------------

ENUM_NMGVertID CAINavMeshGen::AddVertToPool( const LTVector& vVert )
{ 
	ENUM_NMGVertID eVert = GetExistingVertInPool( vVert );
	if (kNMGVert_Invalid != eVert)
	{
		return eVert;
	}

	// No match was found, so add a vert.

	ENUM_NMGVertID eVertID = ( ENUM_NMGVertID )m_nNextNMGVertID;
	++m_nNextNMGVertID;

	SAINAVMESHGEN_VERT* pVert = NAVMESH_NEW( SAINAVMESHGEN_VERT );
	pVert->vVert = vVert;
	m_mapNMGVertPool.insert( AINAVMESHGEN_VERT_MAP::value_type( eVertID, pVert ) );

	return eVertID;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetNMGVert()
//              
//	PURPOSE:	Get vertex by pool ID.
//              
//----------------------------------------------------------------------------

SAINAVMESHGEN_VERT* CAINavMeshGen::GetNMGVert( ENUM_NMGVertID eVert )
{
	SAINAVMESHGEN_VERT* pVert = NULL;
	AINAVMESHGEN_VERT_MAP::iterator itVert = m_mapNMGVertPool.find( eVert );
	if( itVert != m_mapNMGVertPool.end() )
	{
		pVert = itVert->second;
	}

	return pVert;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetActualVertFromPool()
//              
//	PURPOSE:	Get vertex by pool ID.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::GetActualVertFromPool( ENUM_NMGVertID eVert, LTVector* pvVert )
{
	SAINAVMESHGEN_VERT* pVert;
	AINAVMESHGEN_VERT_MAP::iterator itVert = m_mapNMGVertPool.find( eVert );
	if( itVert != m_mapNMGVertPool.end() )
	{
		pVert = itVert->second;
		*pvVert = pVert->vVert;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AccessNMGVert()
//              
//	PURPOSE:	Add poly reference to vert.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AccessNMGVert( ENUM_NMGVertID eVert, ENUM_NMGPolyID ePoly )
{
	SAINAVMESHGEN_VERT* pVert;
	AINAVMESHGEN_VERT_MAP::iterator itVert = m_mapNMGVertPool.find( eVert );
	if( itVert != m_mapNMGVertPool.end() )
	{
		pVert = itVert->second;
		pVert->lstPolyRefs.push_back( ePoly );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::ReleaseNMGVert()
//              
//	PURPOSE:	Remove poly reference from vert.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::ReleaseNMGVert( ENUM_NMGVertID eVert, ENUM_NMGPolyID ePoly )
{
	SAINAVMESHGEN_VERT* pVert;
	AINAVMESHGEN_VERT_MAP::iterator itVert = m_mapNMGVertPool.find( eVert );
	if( itVert != m_mapNMGVertPool.end() )
	{
		pVert = itVert->second;

		NMGPOLY_LIST::iterator itPoly;
		for( itPoly = pVert->lstPolyRefs.begin(); itPoly != pVert->lstPolyRefs.end(); ++itPoly )
		{
			if( *itPoly == ePoly )
			{
				pVert->lstPolyRefs.erase( itPoly );
				return;
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AccessNMGVerts()
//              
//	PURPOSE:	Add poly references to verts.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AccessNMGVerts( CAINavMeshGenPoly* pPoly )
{
	ENUM_NMGPolyID ePolyID = pPoly->GetNMGPolyID();

	// Access verts to create poly references.

	for( int iVert=0; iVert < pPoly->GetNumNMGVerts(); ++iVert )
	{
		AccessNMGVert( pPoly->GetNMGVert( iVert ), ePolyID );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::ReleaseNMGVerts()
//              
//	PURPOSE:	Remove poly reference from verts.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::ReleaseNMGVerts( CAINavMeshGenPoly* pPoly )
{
	ENUM_NMGPolyID ePolyID = pPoly->GetNMGPolyID();

	for( int iVert=0; iVert < pPoly->GetNumNMGVerts(); ++iVert )
	{
		ReleaseNMGVert( pPoly->GetNMGVert( iVert ), ePolyID );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::CompactNMGVertPool()
//              
//	PURPOSE:	Delete unused verts from NavMesh vert pool.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::CompactNMGVertPool()
{
	SAINAVMESHGEN_VERT* pVert;
	AINAVMESHGEN_VERT_MAP::iterator itNext;
	AINAVMESHGEN_VERT_MAP::iterator itVert = m_mapNMGVertPool.begin();
	while( itVert != m_mapNMGVertPool.end() )
	{
		pVert = itVert->second;

		// Remove verts that are no longer associated with any polys.

		if( pVert->lstPolyRefs.empty() )
		{
			NAVMESH_DELETE( pVert );
			itNext = itVert;
			++itNext;
			m_mapNMGVertPool.erase( itVert );
			itVert = itNext;
		}
		else {
			++itVert;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddNormalToPool()
//              
//	PURPOSE:	Return normal pool ID for given normal.
//              
//----------------------------------------------------------------------------

ENUM_NMGNormalID CAINavMeshGen::AddNormalToPool( const LTVector& vNormal )
{
	// Find existing matching vert.

	// BJL, 5/5/004
	//
	// Changed fEpsilon from 0.01 to 0.0001.  The epsilon at .01 was causing 
	// several issues with carving.  
	//
	// CoplanarRayIntersectPoly was failing due to the front/back test on 
	// thin polies.  
	//
	// SAINAVMESHGEN_PLANE::RayIntersectNMGPlane parallel line test (with 
	// fDenom == 0.f), was failing due to denominators of 0.01...
	//
	// CAINavMeshGenPoly::IsPolyOnOutsideOfRay was failing due to false 
	// 'inside/outside' tests.

	float fEpsilon = 0.0001f;

	int iNormal = 0;
	LTVector vN;
	VECTOR_LIST::iterator itNormal;
	for( itNormal = m_lstNMGNormalPool.begin(); itNormal != m_lstNMGNormalPool.end(); ++itNormal )
	{
		vN = *itNormal;
		if( vNormal.NearlyEquals( vN, fEpsilon ) )
		{	
			return (ENUM_NMGNormalID)iNormal;
		}

		++iNormal;
	}

	// No match was found, so add a normal.

	m_lstNMGNormalPool.push_back( vNormal );
	return (ENUM_NMGNormalID)( m_lstNMGNormalPool.size() - 1 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetActualNormalFromPool()
//              
//	PURPOSE:	Get normal by pool ID.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::GetActualNormalFromPool( ENUM_NMGNormalID eNormal, LTVector* pvNormal )
{
	*pvNormal = m_lstNMGNormalPool[eNormal];
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddNMGEdge()
//              
//	PURPOSE:	Add polygon edge to NavMesh edge list.
//              
//----------------------------------------------------------------------------

SAINAVMESHGEN_EDGE* CAINavMeshGen::AddNMGEdge( ENUM_NMGPolyID ePolyID, ENUM_NMGVertID eVertA, ENUM_NMGVertID eVertB )
{
	if( eVertA == eVertB )
	{
		NAVMESH_ERROR( "CAINavMeshGen::AddEdge: VertA cannot equal VertB!" );
		return NULL;
	}

	// Sort verts.

	ENUM_NMGVertID eVertMin = LTMIN<ENUM_NMGVertID>( eVertA, eVertB );
	ENUM_NMGVertID eVertMax = LTMAX<ENUM_NMGVertID>( eVertA, eVertB );

	// Find an existing edge with a matching min vert.

	AINAVMESHGEN_EDGE_MAP::iterator itEdge;
	SAINAVMESHGEN_EDGE* pEdge;

	AINAVMESHGEN_EDGE_BOUNDS EdgeRange = m_mapNMGEdges.equal_range( eVertMin );
	for( itEdge = EdgeRange.first; itEdge != EdgeRange.second; ++itEdge )
	{
		// If edge also has matching max vert, then this is a neighbor of an existing edge.

		pEdge = itEdge->second;
		if( pEdge->eVertMax == eVertMax )
		{
			// An edge may only have 2 neighbors.

			if( pEdge->ePolyID1 == kNMGPoly_Invalid )
			{
				pEdge->ePolyID1 = ePolyID;
			}
			else if( pEdge->ePolyID2 == kNMGPoly_Invalid )
			{
				pEdge->ePolyID2 = ePolyID;
			}
			else
			{
				// Flag this poly as having too many neighbors so that this error can be reported
				// to level design with ids of neighboring polies.  This needs to be done when the
				// nav mesh is actually written out, as this is the only time the poly ids visible
				// in the game are known.
				pEdge->bTooManyNeighbors = true;

				SAINAVMESHGEN_VERT* pVA = GetNMGVert( eVertA );
				SAINAVMESHGEN_VERT* pVB = GetNMGVert( eVertB );

				if ( pVA && pVB )
				{
					LTVector vEdgeCenter = pVB->vVert + ( ( pVA->vVert - pVB->vVert ) / 2.0f );
					LTVector vDisplayableCenter = NAVMESH_CONVERTPOS( vEdgeCenter );
					NAVMESH_ERROR3("Nav Mesh Generation Error!  Edge has more than 2 neighbors in the area of: %f %f %f", 
						vDisplayableCenter.x, vDisplayableCenter.y, vDisplayableCenter.z);
				}
				else
				{
					NAVMESH_ERROR( "CAINavMeshGen::AddEdge: Edge has more than 2 neighbors." );
				}
			}

			if( ( pEdge->ePolyID1 == kNMGPoly_Invalid ) ||
				( pEdge->ePolyID2 == kNMGPoly_Invalid ) )
			{
				pEdge->eNMGEdgeType = kNMGEdgeType_Border;
			}
			else {
				pEdge->eNMGEdgeType = kNMGEdgeType_Shared;
			}

			return pEdge;
		}
	}

	// Create a new edge.

	pEdge = NAVMESH_NEW( SAINAVMESHGEN_EDGE );
	pEdge->eVertMin = eVertMin;
	pEdge->eVertMax = eVertMax;
	pEdge->ePolyID1 = ePolyID;

	pEdge->eNMGEdgeType = kNMGEdgeType_Border;

	// Assign unique ID to edge.

	pEdge->eNMGEdgeID = ( ENUM_NMGEdgeID )m_nNextNMGEdgeID;
	++m_nNextNMGEdgeID;

	// Add new edge to map of edges.

	m_mapNMGEdges.insert( AINAVMESHGEN_EDGE_MAP::value_type( eVertMin, pEdge ) );

	return pEdge;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::RemoveNMGEdge()
//              
//	PURPOSE:	Remove polygon edge from NavMesh edge list.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::RemoveNMGEdge( ENUM_NMGPolyID ePolyID, ENUM_NMGVertID eVertA, ENUM_NMGVertID eVertB )
{
	// Sort verts.

	ENUM_NMGVertID eVertMin = LTMIN<ENUM_NMGVertID>( eVertA, eVertB );
	ENUM_NMGVertID eVertMax = LTMAX<ENUM_NMGVertID>( eVertA, eVertB );

	// Find an existing edge with a matching min vert.

	AINAVMESHGEN_EDGE_MAP::iterator itEdge;
	SAINAVMESHGEN_EDGE* pEdge;

	AINAVMESHGEN_EDGE_BOUNDS EdgeRange = m_mapNMGEdges.equal_range( eVertMin );
	for( itEdge = EdgeRange.first; itEdge != EdgeRange.second; ++itEdge )
	{
		// If edge also has matching max vert, then this is the specified edge.

		pEdge = itEdge->second;
		if( pEdge->eVertMax == eVertMax )
		{
			if( pEdge->ePolyID1 == ePolyID )
			{
				pEdge->ePolyID1 = kNMGPoly_Invalid;
			}
			else if( pEdge->ePolyID2 == ePolyID )
			{
				pEdge->ePolyID2 = kNMGPoly_Invalid;
			}

			pEdge->eNMGEdgeType = kNMGEdgeType_Border;

			return;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddNMGEdges()
//              
//	PURPOSE:	Add polygon edges to NavMesh edge list.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AddNMGEdges( CAINavMeshGenPoly* pPoly )
{
	ENUM_NMGPolyID ePolyID = pPoly->GetNMGPolyID();

	// Add poly edges to edge list.

	int cVerts = pPoly->GetNumNMGVerts();
	for( int iVert=0; iVert < cVerts - 1; ++iVert )
	{
		AddNMGEdge( ePolyID, pPoly->GetNMGVert( iVert ), pPoly->GetNMGVert( iVert + 1 ) );
	}
	AddNMGEdge( ePolyID, pPoly->GetNMGVert( cVerts - 1 ), pPoly->GetNMGVert( 0 ) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::RemoveNMGEdges()
//              
//	PURPOSE:	Remove polygon edges from NavMesh edge list.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::RemoveNMGEdges( CAINavMeshGenPoly* pPoly )
{
	ENUM_NMGPolyID ePolyID = pPoly->GetNMGPolyID();

	for( int iVert=0; iVert < pPoly->GetNumNMGVerts() - 1; ++iVert )
	{
		RemoveNMGEdge( ePolyID, pPoly->GetNMGVert( iVert ), pPoly->GetNMGVert( iVert + 1 ) );
	}
	RemoveNMGEdge( ePolyID, pPoly->GetNMGVert( pPoly->GetNumNMGVerts() - 1 ), pPoly->GetNMGVert( 0 ) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::CompactNMGEdgeList()
//              
//	PURPOSE:	Delete unused edges from NavMesh edge list.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::CompactNMGEdgeList()
{
	SAINAVMESHGEN_EDGE* pEdge;
	AINAVMESHGEN_EDGE_MAP::iterator itNext;
	AINAVMESHGEN_EDGE_MAP::iterator itEdge = m_mapNMGEdges.begin();
	while( itEdge != m_mapNMGEdges.end() )
	{
		pEdge = itEdge->second;

		// Remove edges that are no longer associated with any polys.

		if( ( pEdge->ePolyID1 == kNMGPoly_Invalid ) &&
			( pEdge->ePolyID2 == kNMGPoly_Invalid ) )
		{
			NAVMESH_DELETE( pEdge );
			itNext = itEdge;
			++itNext;
			m_mapNMGEdges.erase( itEdge );
			itEdge = itNext;
		}
		else {
			++itEdge;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetNMGEdge()
//              
//	PURPOSE:	Get an edge by looking up verts.
//              
//----------------------------------------------------------------------------

SAINAVMESHGEN_EDGE* CAINavMeshGen::GetNMGEdge( ENUM_NMGVertID eVertA, ENUM_NMGVertID eVertB )
{
	SAINAVMESHGEN_EDGE* pEdge = NULL;

	// Sort verts.

	ENUM_NMGVertID eVertMin = LTMIN<ENUM_NMGVertID>( eVertA, eVertB );
	ENUM_NMGVertID eVertMax = LTMAX<ENUM_NMGVertID>( eVertA, eVertB );

	// Find an existing edge with a matching min vert.

	AINAVMESHGEN_EDGE_MAP::iterator itEdge;
	AINAVMESHGEN_EDGE_BOUNDS EdgeRange = m_mapNMGEdges.equal_range( eVertMin );
	for( itEdge = EdgeRange.first; itEdge != EdgeRange.second; ++itEdge )
	{
		// If edge also has matching max vert, then this is the specified edge.

		pEdge = itEdge->second;
		if( pEdge->eVertMax == eVertMax )
		{
			return pEdge;
		}
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddNMGPoly()
//              
//	PURPOSE:	Add polygon to NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AddNMGPoly( CAINavMeshGenPoly* pPoly )
{
	// Set and increment the PolyID.

	ENUM_NMGPolyID ePolyID = ( ENUM_NMGPolyID )m_nNextNMGPolyID;
	pPoly->SetNMGPolyID( ePolyID );
	++m_nNextNMGPolyID;

	// Add the poly to poly list.

	m_lstNMGPolys.push_back( pPoly );

	// Add the poly to its parents AIRegions.

	AddPolyToParentsAIRegions( pPoly );

	// Access verts to create poly references.

	AccessNMGVerts( pPoly );
	
	// Add poly edges to edge list.

	AddNMGEdges( pPoly );

	// Add poly to quad tree.

	m_pNMGQuadTree->AddNMGPoly( pPoly );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::RemoveNMGPoly()
//              
//	PURPOSE:	Remove polygon from NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::RemoveNMGPoly( CAINavMeshGenPoly* pPoly )
{
	AINAVMESHGEN_POLY_LIST::iterator itNavMesh;
	for( itNavMesh = m_lstNMGPolys.begin(); itNavMesh != m_lstNMGPolys.end(); ++itNavMesh )
	{
		if( pPoly == *itNavMesh )
		{
			ReleaseNMGVerts( pPoly );
			RemoveNMGEdges( pPoly );
			m_lstNMGPolys.erase( itNavMesh );

			// Remove poly from the AIRegions.

			RemovePolyFromAIRegions( pPoly->GetNMGPolyID() );

			// Remove poly from quad tree.

			m_pNMGQuadTree->RemoveNMGPoly( pPoly );
			return;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddPolyToAIRegion()
//              
//	PURPOSE:	Add NavMesh polygon to an AIRegion.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AddPolyToAIRegion( ENUM_NMGPolyID ePolyID, ENUM_NMGRegionID eRegionID )
{
	if( ( ePolyID == kNMGPoly_Invalid ) ||
		( eRegionID == kNMGRegion_Invalid ) )
	{
		return;
	}

	m_mapNMGPolyToAIRegion.insert( AINAVMESHGEN_POLY_TO_REGION_MAP::value_type( ePolyID, eRegionID ) );
	m_mapAIRegionToNMGPoly.insert( AINAVMESHGEN_REGION_TO_POLY_MAP::value_type( eRegionID, ePolyID ) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::RemovePolyFromAIRegions()
//              
//	PURPOSE:	Remove NavMesh polygon from a AIRegions.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::RemovePolyFromAIRegions( ENUM_NMGPolyID ePolyID )
{
	if( ePolyID == kNMGPoly_Invalid )
	{
		return;
	}

	m_lstDeletedAIRegionPolys.push_back( ePolyID );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::CompactAIRegions()
//              
//	PURPOSE:	Cleanup AIRegion maps by removing references to deleted polys.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::CompactAIRegions()
{
	ENUM_NMGPolyID ePolyID;

	ENUM_NMGRegionID eRegionID;
	ENUM_NMGPolyID eRegionPolyID;
	AINAVMESHGEN_POLY_TO_REGION_MAP::iterator itRegion;
	AINAVMESHGEN_REGION_TO_POLY_MAP::iterator itPoly;

	AINAVMESHGEN_REGION_TO_POLY_BOUNDS PolyRange;
	AINAVMESHGEN_POLY_TO_REGION_BOUNDS RegionRange;

	NMGPOLY_LIST::iterator itDeletedPoly;
	for( itDeletedPoly = m_lstDeletedAIRegionPolys.begin(); itDeletedPoly != m_lstDeletedAIRegionPolys.end(); ++itDeletedPoly )
	{
		ePolyID = *itDeletedPoly;

		// Iterate over all AIRegions that NavMesh poly is part of.
		// For each AIRegion, find the reference to the specified 
		// NavMesh poly, and erase the record with the reference.

		RegionRange = m_mapNMGPolyToAIRegion.equal_range( ePolyID );
		for( itRegion = RegionRange.first; itRegion != RegionRange.second; ++itRegion )
		{
			// Find the AIRegion's reference to the specified NavMesh poly.

			eRegionID = itRegion->second;
			PolyRange = m_mapAIRegionToNMGPoly.equal_range( eRegionID );
			for( itPoly = PolyRange.first; itPoly != PolyRange.second; ++itPoly )
			{
				eRegionPolyID = itPoly->second;
				if( ePolyID == eRegionPolyID )
				{
					// Erase the reference.

					m_mapAIRegionToNMGPoly.erase( itPoly );
					break;
				}
			}
		}

		// Erase all records for this NavMesh poly.

		m_mapNMGPolyToAIRegion.erase( ePolyID );
	}

	m_lstDeletedAIRegionPolys.resize( 0 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddPolyToParentsAIRegions()
//              
//	PURPOSE:	Add NavMesh polygon to parent NavMesh poly's AIRegions.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AddPolyToParentsAIRegions( CAINavMeshGenPoly* pPoly )
{
	// Sanity check.

	if( !pPoly )
	{
		return;
	}

	// Poly has no parent.

	ENUM_NMGPolyID eParentPolyID = pPoly->GetNMGParentPolyID();
	if( eParentPolyID == kNMGPoly_Invalid )
	{
		return;
	}

	static NMGREGION_LIST lstNMGRegions;
	lstNMGRegions.resize( 0 );

	// Iterate over parent's AIRegions, and create list.

	ENUM_NMGRegionID eRegionID;
	AINAVMESHGEN_POLY_TO_REGION_MAP::iterator itRegion;

	AINAVMESHGEN_POLY_TO_REGION_BOUNDS RegionRange = m_mapNMGPolyToAIRegion.equal_range( eParentPolyID );
	for( itRegion = RegionRange.first; itRegion != RegionRange.second; ++itRegion )
	{
		lstNMGRegions.push_back( itRegion->second );
	}

	// Iterate over list created in last step, and add poly to each AIRegion listed.

	NMGREGION_LIST::iterator itNMGRegion;
	for( itNMGRegion = lstNMGRegions.begin(); itNMGRegion != lstNMGRegions.end(); ++itNMGRegion )
	{
		eRegionID = *itNMGRegion;
		AddPolyToAIRegion( pPoly->GetNMGPolyID(), eRegionID );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetNMGPoly()
//              
//	PURPOSE:	Get polygon from NavMesh by ID.
//              
//----------------------------------------------------------------------------

CAINavMeshGenPoly* CAINavMeshGen::GetNMGPoly( ENUM_NMGPolyID ePolyID )
{
	CAINavMeshGenPoly* pPoly;
	AINAVMESHGEN_POLY_LIST::iterator itNavMesh;
	for( itNavMesh = m_lstNMGPolys.begin(); itNavMesh != m_lstNMGPolys.end(); ++itNavMesh )
	{
		pPoly = *itNavMesh;
		if( pPoly->GetNMGPolyID() == ePolyID )
		{
			return pPoly;
		}
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetNMGLink()
//              
//	PURPOSE:	Get link from NavMesh by ID.
//              
//----------------------------------------------------------------------------

SAINAVMESHGEN_LINK* CAINavMeshGen::GetNMGLink( ENUM_NMGLinkID eLinkID )
{
	SAINAVMESHGEN_LINK* pLink;
	AINAVMESHGEN_LINK_LIST::iterator itLink;
	for( itLink = m_lstNMGLinks.begin(); itLink != m_lstNMGLinks.end(); ++itLink )
	{
		pLink = &( *itLink );
		if( pLink->eNMGLinkID == eLinkID )
		{
			return pLink;
		}
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddNMGCarver()
//              
//	PURPOSE:	Add Carver to NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AddNMGCarver( CAINavMeshGenCarver* pCarver )
{
	// Set and increment the CarverID.

	ENUM_NMGCarverID eCarverID = ( ENUM_NMGCarverID )m_nNextNMGCarverID;
	pCarver->SetNMGCarverID( eCarverID );
	++m_nNextNMGCarverID;

	// Add the Carver to Carver list.

	m_lstNMGCarvers.push_back( pCarver );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AddNMGRegion()
//              
//	PURPOSE:	Add AIRegion to NavMesh.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AddNMGRegion( SAINAVMESHGEN_REGION* pAIRegion )
{
	// Set and increment the AIRegionID.

	ENUM_NMGRegionID eRegionID = ( ENUM_NMGRegionID )m_nNextNMGRegionID;
	pAIRegion->eNMGRegionID = eRegionID;
	++m_nNextNMGRegionID;

	// Add the AIRegion to AIRegion list.

	m_lstNMGRegions.push_back( pAIRegion );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::CreateNMGQTNode()
//              
//	PURPOSE:	Create a new QuadTree node.
//              
//----------------------------------------------------------------------------

CAINavMeshGenQuadTreeNode* CAINavMeshGen::CreateNMGQTNode()
{
	CAINavMeshGenQuadTreeNode* pNMGQTNode = NAVMESH_NEW( CAINavMeshGenQuadTreeNode );
	pNMGQTNode->m_eNMQTNodeID = ( ENUM_NMGQTNodeID )m_nNextNMGQTNodeID;
	++m_nNextNMGQTNodeID;

	m_lstNMGQuadTreeNodes.push_back( pNMGQTNode );

	return pNMGQTNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::DestroyNMGQTNode()
//              
//	PURPOSE:	Destroy a QuadTree node.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::DestroyNMGQTNode( CAINavMeshGenQuadTreeNode* pNMGQTNode )
{
	if( pNMGQTNode )
	{
		NAVMESH_DELETE( pNMGQTNode );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::ConvertToNMxxxID()
//              
//	PURPOSE:	Convert a NavMeshGen ID to a NavMesh ID.
//              
//----------------------------------------------------------------------------

ENUM_NMGConvertedEdgeID CAINavMeshGen::ConvertToNMEdgeID( ENUM_NMGEdgeID eNMGEdgeID )
{
	NMEDGEID_CONVERT_MAP::iterator itConvert;
	itConvert = m_mapConvertEdgeIDs.find( eNMGEdgeID );
	if( itConvert != m_mapConvertEdgeIDs.end() )
	{
		return itConvert->second;
	}

	return kNMGConvertedEdge_Invalid;
}

ENUM_NMGConvertedPolyID CAINavMeshGen::ConvertToNMPolyID( ENUM_NMGPolyID eNMGPolyID )
{
	NMPOLYID_CONVERT_MAP::iterator itConvert;
	itConvert = m_mapConvertPolyIDs.find( eNMGPolyID );
	if( itConvert != m_mapConvertPolyIDs.end() )
	{
		return itConvert->second;
	}

	return kNMGConvertedPoly_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::AssignCharTypeMasks()
//              
//	PURPOSE:	Assign character type masks to NavMeshGen polies.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::AssignCharTypeMasks()
{
	CAINavMeshGenPoly* pPoly;
	ENUM_NMGNavMeshID eNavMeshID;
	uint32 dwCharTypeMask;
	AINAVMESHGEN_POLY_LIST::iterator itNavMesh;
	for( itNavMesh = m_lstNMGPolys.begin(); itNavMesh != m_lstNMGPolys.end(); ++itNavMesh )
	{
		pPoly = *itNavMesh;
		eNavMeshID = pPoly->GetNMGParentNavMeshID();

		dwCharTypeMask = GetCharTypeMask( eNavMeshID );
		pPoly->SetNMGCharTypeMask( dwCharTypeMask );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::GetCharTypeMask()
//              
//	PURPOSE:	Return the mask for the specified NavMesh.
//              
//----------------------------------------------------------------------------

uint32 CAINavMeshGen::GetCharTypeMask( ENUM_NMGNavMeshID eNavMeshID )
{
	// Sanity check.

	if( ( eNavMeshID == kNMGNavMesh_Invalid ) ||
		( (uint32)eNavMeshID >= m_lstNMGRestrictions.size() ) )
	{
		return 0;
	}

	// Return the mask for the specified NavMesh.

	SAINAVMESHGEN_RESTRICTIONS* pNMGRestrictions = &( m_lstNMGRestrictions[eNavMeshID] );
	return pNMGRestrictions->dwCharTypeMask;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::ClusterAINodes()
//              
//	PURPOSE:	Create node clusters.
//              
//----------------------------------------------------------------------------

void CAINavMeshGen::ClusterAINodes()
{
	// Node clustering is part of NavMeshGen because it utilizes the 
	// NavMeshGenQuadTree, and should eventually be preprocessed
	// along with the NavMesh.

	// Currently only cluster Cover nodes.
	// Bail if no cover nodes exist.

	if( m_lstNMGNodes.empty() )
	{
		return;
	}

	// Add nodes to QuadTree.

	SAINAVMESHGEN_NODE* pNode;
	AINAVMESHGEN_NODE_LIST::iterator itNode;
	for( itNode = m_lstNMGNodes.begin(); itNode != m_lstNMGNodes.end(); ++itNode )
	{
		pNode = &( *itNode );
		m_pNMGQuadTree->AddAINode( pNode );
	}

	// Cluster neighboring nodes.

	float fRadius = NODE_CLUSTER_RADIUS;
	float fHeight = 100.f;
	LTVector vDims( fRadius, fHeight, fRadius );

	uint32 iNextCluster = 0;
	ENUM_NMGNodeClusterID eCluster;
	ENUM_NMGNodeClusterID eNeighborCluster;
	SAINODECLUSTER_MERGE ClusterMerge;
	AINODECLUSTER_MERGE_LIST lstMerge;

	SAINAVMESHGEN_NODE* pNeighbor;
	AINAVMESHGEN_NODE_PTR_LIST lstNeighbors;
	SAINAVMESHGEN_AABB aabb;
	AINAVMESHGEN_NODE_PTR_LIST::iterator itNeighbor;
	for( itNode = m_lstNMGNodes.begin(); itNode != m_lstNMGNodes.end(); ++itNode )
	{
		pNode = &( *itNode );

		aabb.vMin = pNode->vPos - vDims;
		aabb.vMax = pNode->vPos + vDims;

		// Find neighbors.

		lstNeighbors.resize( 0 );
		m_pNMGQuadTree->GetIntersectingNodes( &aabb, &lstNeighbors, pNode );

		// Skip nodes that have no neighbors. These are not in a cluster.

		if( lstNeighbors.empty() )
		{
			continue;
		}

		// Find a neighbor's clusterID.

		eCluster = kNMGNodeCluster_Invalid;
		for( itNeighbor = lstNeighbors.begin(); itNeighbor != lstNeighbors.end(); ++itNeighbor )
		{
			pNeighbor = *itNeighbor;
			eNeighborCluster = pNeighbor->eNMGNodeClusterID;
			if( eNeighborCluster != kNMGNodeCluster_Invalid )
			{
				// First neighbor's clusterID.

				if( eCluster == kNMGNodeCluster_Invalid )
				{
					eCluster = eNeighborCluster;
				}

				// Adjacent clusters need to be merged.

				else if( eCluster != eNeighborCluster )
				{
					ClusterMerge.eClusterFrom = (ENUM_NMGNodeClusterID)LTMIN<uint32>( (uint32)eCluster, (uint32)eNeighborCluster );
					ClusterMerge.eClusterTo = (ENUM_NMGNodeClusterID)LTMAX<uint32>( (uint32)eCluster, (uint32)eNeighborCluster );
					lstMerge.push_back( ClusterMerge );
				}
			}
		}

		// Create a new cluster ID.

		if( eCluster == kNMGNodeCluster_Invalid )
		{
			eCluster = (ENUM_NMGNodeClusterID)iNextCluster;
			++iNextCluster;
		}

		// Assign the node to a cluster.

		pNode->eNMGNodeClusterID = eCluster;
	}

	// Merge neighboring clusters.

	SAINODECLUSTER_MERGE* pMerge;
	SAINODECLUSTER_MERGE* pNextMerge;
	AINODECLUSTER_MERGE_LIST::iterator itMerge;
	AINODECLUSTER_MERGE_LIST::iterator itNextMerge;
	for( itMerge = lstMerge.begin(); itMerge != lstMerge.end(); ++itMerge )
	{
		pMerge = &( *itMerge );

		// Skip merges that have already been handled by previous merges.

		if( pMerge->eClusterFrom == pMerge->eClusterTo )
		{
			continue;
		}

		// Nodes that previously pointed at the cluster being merged
		// need to point at the cluster they are merging into.

		for( itNode = m_lstNMGNodes.begin(); itNode != m_lstNMGNodes.end(); ++itNode )
		{
			pNode = &( *itNode );
			if( pNode->eNMGNodeClusterID == pMerge->eClusterFrom )
			{
				pNode->eNMGNodeClusterID = pMerge->eClusterTo;
			}
		}

		// Any future references to the merged cluster need to
		// point to the cluster being merged into.

		for( itNextMerge = itMerge; itNextMerge != lstMerge.end(); ++itNextMerge )
		{
			pNextMerge = &( *itNextMerge );
			if( pNextMerge->eClusterFrom == pMerge->eClusterFrom )
			{
				pNextMerge->eClusterFrom = pMerge->eClusterTo;
			}
			if( pNextMerge->eClusterTo == pMerge->eClusterFrom )
			{
				pNextMerge->eClusterTo = pMerge->eClusterTo;
			}
		}
	}
}

#define VERIFY_ALIGNMENT( step ) if( Converter.GetPos() % sizeof(uint32) != 0 ) NAVMESH_ERROR1( "Misaligned NavMesh after step: %s", #step );

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGen::PackRunTimeNavMesh()
//              
//	PURPOSE:	Pack the run-time NavMesh into one block of data.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGen::PackRunTimeNavMesh( ILTOutConverter& Converter )
{
	// Sanity check.
	uint32 nSizeFloat = sizeof(float);
	uint32 nSizeInt = sizeof(uint32);
	uint32 nSizeVector = sizeof(LTVector);


	// Polys and edges get re-indexed sequentially in the run-time NavMesh.
	// Meaning, poly1, poly2, poly8 becomes poly1, poly2, poly3.

	// Conversion maps convert from a NavMeshGen edge or PolyID to a 
	// sequential NavMesh edge or PolyID.

	m_mapConvertEdgeIDs.clear();
	m_mapConvertPolyIDs.clear();


	// Convert poly IDs to re-indexed sequential IDs.

	int iPoly = 0;
	CAINavMeshGenPoly* pNMGPoly;
	AINAVMESHGEN_POLY_LIST::iterator itPoly;
	for( itPoly = m_lstNMGPolys.begin(); itPoly != m_lstNMGPolys.end(); ++itPoly )
	{
		pNMGPoly = *itPoly;
		pNMGPoly->SetNMGConvertedPolyID( (ENUM_NMGConvertedPolyID)iPoly );

		// Add poly IDs to conversion map.

		m_mapConvertPolyIDs.insert( NMPOLYID_CONVERT_MAP::value_type( pNMGPoly->GetNMGPolyID(), (ENUM_NMGConvertedPolyID)iPoly ) );
		++iPoly;
	}


	//
	// Write header.
	//

	// Write NavMesh version number.
	Converter << m_nAINavMeshVersion;

	// Write fixed-up flag placeholder.
	// Game code sets this to true after the pointer fix-up pass.
	// This allows the BlindObjectData to stay in memory,
	// after indices have been coverted to pointers.
	Converter << (uint32)false;

	//
	//
	// Write edges.
	//

	// Write the number of edges.
	Converter << (uint32)m_mapNMGEdges.size();

	// Iterate over edges in NavMeshGen, and write each one.

	int iEdge = 0;
	int cEdgeRefs = 0;
	uint32 iNext, iVert, cVerts;
	ENUM_NMGVertID eVert0, eVert1;
	LTVector vEdge, vEdgeN, vNMGEdgeMidPt, vNMGPolyN;
	SAINAVMESHGEN_VERT* pNMGVert0;
	SAINAVMESHGEN_VERT* pNMGVert1;
	SAINAVMESHGEN_EDGE* pNMGEdge;
	AINAVMESHGEN_EDGE_MAP::iterator itEdge;
	for( itEdge = m_mapNMGEdges.begin(); itEdge != m_mapNMGEdges.end(); ++itEdge )
	{
		pNMGEdge = itEdge->second;

		// Write sequential NavMesh edgeID.
		// Add edge IDs to conversion map.

		m_mapConvertEdgeIDs.insert( NMEDGEID_CONVERT_MAP::value_type( pNMGEdge->eNMGEdgeID, (ENUM_NMGConvertedEdgeID)iEdge ) );
		Converter << (uint32)(ENUM_NMGConvertedEdgeID)iEdge;
		++iEdge;

		// Set edge type.
		Converter << (uint32)pNMGEdge->eNMGEdgeType;

		// Set actual verts of NavMesh edge, rather than references.
		// Mark verts that are BorderVerts.

		pNMGVert0 = GetNMGVert( pNMGEdge->eVertMin );
		if( !pNMGVert0 )
		{
			return false;
		}
		Converter << pNMGVert0->vVert;
		Converter << (uint32)pNMGVert0->bIsBorderVert;

		pNMGVert1 = GetNMGVert( pNMGEdge->eVertMax );
		if( !pNMGVert1 )
		{
			return false;
		}
		Converter << pNMGVert1->vVert;
		Converter << (uint32)pNMGVert1->bIsBorderVert;

		// Pre-Calculate the edge's MidPt.

		vNMGEdgeMidPt = ( pNMGVert0->vVert + pNMGVert1->vVert ) * 0.5f;
		Converter << vNMGEdgeMidPt;

		// Count how many times edges have been referenced.  If the edge has 
		// too many neighbors, skip it as this edge will not be added.  If this
		// is done, then a test MUST be done while iterating over the polies to 
		// insure that this edge is skipped.

		if (!pNMGEdge->bTooManyNeighbors)
		{
			if( pNMGEdge->ePolyID1 != kNMGPoly_Invalid )
			{
				++cEdgeRefs;
			}

			if( pNMGEdge->ePolyID2 != kNMGPoly_Invalid )
			{
				++cEdgeRefs;
			}
		}

		// Write poly IDs.
		Converter << (uint32)ConvertToNMPolyID( pNMGEdge->ePolyID1 );
		Converter << (uint32)ConvertToNMPolyID( pNMGEdge->ePolyID2 );

		// Precompute the edge normal for neighbor poly A.
		// Edge normals point inwards.
		// Discard the y component, so that the negation of
		// the edge normal can be used for the edge normal
		// with neighbor poly B.
		// The edge normal will only be used to check 
		// horizontal containment.

		pNMGPoly = GetNMGPoly( pNMGEdge->ePolyID1 );
		if( !pNMGPoly )
		{
			return false;
		}
		GetActualNormalFromPool( pNMGPoly->GetNMGNormalID(), &vNMGPolyN );

		// Direction of edge is determined by the winding order of the poly verts.

		cVerts = pNMGPoly->GetNumNMGVerts();
		for( iVert=0; iVert < cVerts; ++iVert )
		{
			iNext = ( iVert + 1 ) % cVerts;
			eVert0 = pNMGPoly->GetNMGVert( iVert );
			eVert1 = pNMGPoly->GetNMGVert( iNext );

			if( ( eVert0 == pNMGEdge->eVertMin ) &&
				( eVert1 == pNMGEdge->eVertMax ) )
			{
				vEdge = pNMGVert0->vVert - pNMGVert1->vVert;
			}
			else if( ( eVert1 == pNMGEdge->eVertMin ) &&
					 ( eVert0 == pNMGEdge->eVertMax ) )
			{
				vEdge = pNMGVert1->vVert - pNMGVert0->vVert;
			}
		}
		vEdge.Normalize();

		vEdgeN = vNMGPolyN.Cross( vEdge );
		vEdgeN.y = 0.f;
		if( vEdgeN != LTVector::GetIdentity() )
		{
			vEdgeN.Normalize();
		}

		// Write the edge normal.
		Converter << vEdgeN;
	}
	VERIFY_ALIGNMENT( Edges );

	//
	// Write polys.
	//

	// Write number of edge references, which create edge lists
	// in NavMesh polys.

	Converter << (uint32)cEdgeRefs;

	// Iterate over polys and add edges to edge lists.

	int cPolyEdges;
	NMEDGEID_CONVERT_MAP::iterator itConvert;
	ENUM_NMGConvertedEdgeID eNMEdge;
	int iEdgeList = 0;

	for( itPoly = m_lstNMGPolys.begin(); itPoly != m_lstNMGPolys.end(); ++itPoly )
	{
		pNMGPoly = *itPoly;
		cPolyEdges = 0;

		// Iterate over NavMeshGen poly's verts to get edges.

		cVerts = pNMGPoly->GetNumNMGVerts();
		for( iVert=0; iVert < cVerts; ++iVert )
		{
			iNext = ( iVert + 1 ) % cVerts;
			eVert0 = pNMGPoly->GetNMGVert( iVert );
			eVert1 = pNMGPoly->GetNMGVert( iNext );

			// Find NavMeshGen edge.

			pNMGEdge = GetNMGEdge( eVert0, eVert1 );
			if( !pNMGEdge )
			{
				continue;
			}

			// If an edge is found, check to see if it has too many neighbors.
			// If it does, report an error message now that the poly IDs level design
			// can see are set.

			if ( pNMGEdge->bTooManyNeighbors )
			{
				LTVector vCenter = pNMGPoly->GetNMGPolyCenter();
				LTVector vDisplayableCenter = NAVMESH_CONVERTPOS( vCenter );
				NAVMESH_ERROR5("Nav Mesh Generation Error!  Poly: %d, with link %d at WorldEdit pos %f %f %f, has an edge with too many neighbors", 
					pNMGPoly->GetNMGConvertedPolyID(), 
					pNMGPoly->GetNMGLinkID(),
					vDisplayableCenter.x, vDisplayableCenter.y, vDisplayableCenter.z);

				continue;
			}

			// Look up NavMesh edgeID in conversion map.

			itConvert = m_mapConvertEdgeIDs.find( pNMGEdge->eNMGEdgeID );
			if( itConvert == m_mapConvertEdgeIDs.end() )
			{
				continue;
			}
			eNMEdge = itConvert->second;

			// Record edge in NavMesh edge list.

			if (iEdgeList >= 0 && iEdgeList < cEdgeRefs)
			{
				// Write edge ID.
				Converter << (uint32)eNMEdge;

				++iEdgeList;
				++cPolyEdges;
			}
			else
			{
				NAVMESH_ERROR("CAINavMeshGen::PackRunTimeNavMesh : iEdge is out of bounds.  Some poly likely added more than 2 edges.");
			}
		}

		// Record the number of valid edges per poly.

		pNMGPoly->SetNumNMGPolyEdges( cPolyEdges );
	}
	VERIFY_ALIGNMENT( EdgeLists );

	// Write number of AIRegion references, which create AIRegion lists
	// in NavMesh polys.
	Converter << (uint32)m_mapNMGPolyToAIRegion.size();

	uint32 cRegions;
	AINAVMESHGEN_POLY_TO_REGION_MAP::iterator itPolyToAIRegion = m_mapNMGPolyToAIRegion.begin();
	for( itPoly = m_lstNMGPolys.begin(); itPoly != m_lstNMGPolys.end(); ++itPoly )
	{
		pNMGPoly = *itPoly;
		cRegions = 0;

		// Point poly's AIRegionList into the master list of AIRegions,
		// and count how many AIRegions the poly is a part of.

		while((itPolyToAIRegion != m_mapNMGPolyToAIRegion.end()) && (itPolyToAIRegion->first == pNMGPoly->GetNMGPolyID()))
		{
			// Write the region ID.
			Converter << (uint32)itPolyToAIRegion->second;

			++cRegions;
			++itPolyToAIRegion;
		}

		// Record number of regions per poly.

		pNMGPoly->SetNumNMGRegions( cRegions );
	}
	VERIFY_ALIGNMENT( AIRegionLists );

	// Write number of Poly normals.
	Converter << (uint32)m_lstNMGNormalPool.size();

	// Write each normal.

	VECTOR_LIST::iterator itNormal;
	for( itNormal = m_lstNMGNormalPool.begin(); itNormal != m_lstNMGNormalPool.end(); ++itNormal )
	{
		Converter << *itNormal;
	}


	// Write number of polys.
	Converter << (uint32)m_lstNMGPolys.size();

	// Write each poly.

	iPoly = 0;
	uint32 iEdgeListIndex = 0;
	uint32 iRegionListIndex = 0;
	for( itPoly = m_lstNMGPolys.begin(); itPoly != m_lstNMGPolys.end(); ++itPoly )
	{
		// Write sequential NavMesh polyID.

		pNMGPoly = *itPoly;
		Converter << (uint32)iPoly;
		++iPoly;

		// Write NavMesh poly NormalID.

		Converter << (uint32)pNMGPoly->GetNMGNormalID();

		// Write NavMesh poly ComponentID.

		Converter << (uint32)pNMGPoly->GetNMGComponentID();

		// Write NavMesh poly LinkID.

		Converter << (uint32)pNMGPoly->GetNMGLinkID();

		// Write the parent NavMesh polyID, which will get converted 
		// to the character type mask.

		Converter << (uint32)pNMGPoly->GetNMGParentNavMeshID();

		// Write NavMesh poly center.

		Converter << pNMGPoly->GetNMGPolyCenter();

		// Write NavMesh poly bounding box.

		Converter << pNMGPoly->GetAABB()->vMin;
		Converter << pNMGPoly->GetAABB()->vMax;

		// Write number of edges.

		Converter << (uint32)pNMGPoly->GetNumNMGPolyEdges();

		// Write index into list of edge references to beginning 
		// of this poly's edge list.

		Converter << (uint32)iEdgeListIndex;

		// Advance edge list index past this poly's list.

		iEdgeListIndex += pNMGPoly->GetNumNMGPolyEdges();


		// Write number of regions.

		Converter << (uint32)pNMGPoly->GetNumNMGRegions();

		// Write index into list of region references to beginning 
		// of this poly's list.

		Converter << (uint32)iRegionListIndex;

		// Advance region list index past this poly's list.

		iRegionListIndex += pNMGPoly->GetNumNMGRegions();
	}
	VERIFY_ALIGNMENT( Polys );

	//
	// Write regions.
	//

	// Create AIRegion references to NavMesh poly lists.

	// Write number of poly references, which create poly lists
	// in AIRegions.

	Converter << (uint32)m_mapAIRegionToNMGPoly.size();

	AINAVMESHGEN_REGION_TO_POLY_MAP::iterator itAIRegionToPoly = m_mapAIRegionToNMGPoly.begin();

	// Iterate over AIRegions, and setup their NavMesh poly list pointers.

	uint32 cNMGPolys;
	ENUM_NMGPolyID ePoly;
	SAINAVMESHGEN_REGION* pNMGRegion;
	AINAVMESHGEN_REGION_LIST::iterator itRegion;
	for( itRegion = m_lstNMGRegions.begin(); itRegion != m_lstNMGRegions.end(); ++itRegion )
	{
		pNMGRegion = *itRegion;
		cNMGPolys = 0;


		// Set the center and the radius, based on the iteration over the 
		// contained polies.  This is done here, as it should eventually be
		// moved into the preprocessor along with the rest of the AIRegion
		// precalculation.

		bool bSphereInitialized = false;
		LTSphere ContainingSphere;

		while( itAIRegionToPoly->first == pNMGRegion->eNMGRegionID 
			&& itAIRegionToPoly != m_mapAIRegionToNMGPoly.end() )
		{
			// Write poly ID.

			ePoly = itAIRegionToPoly->second;
			Converter << (uint32)ConvertToNMPolyID( ePoly );

			// Count polys per region.

			++cNMGPolys;
			++itAIRegionToPoly;

			// Calculate bounding sphere.

			pNMGPoly = GetNMGPoly( ePoly );
			if( pNMGPoly )
			{
				if (!bSphereInitialized)
				{
					ContainingSphere.Init(pNMGPoly->GetNMGPolyCenter(), pNMGPoly->GetNMGBoundingRadius());
					bSphereInitialized = true;
				}
				else
				{
					LTSphere PolySphere(pNMGPoly->GetNMGPolyCenter(), pNMGPoly->GetNMGBoundingRadius());
					ContainingSphere.Encompass(PolySphere);
				}
			}
		}

		// Record calculated data per region.

		pNMGRegion->cNMGPolys = cNMGPolys;
		pNMGRegion->vBoundingSphereCenter = ContainingSphere.Center();
		pNMGRegion->fBoundingSphereRadius = ContainingSphere.Radius();
	}

	// Write number of regions.

	Converter << (uint32)m_lstNMGRegions.size();

	// Write each region.

	uint32 iPolyListIndex = 0;
	for( itRegion = m_lstNMGRegions.begin(); itRegion != m_lstNMGRegions.end(); ++itRegion )
	{
		// Write region ID.

		pNMGRegion = *itRegion;
		Converter << (uint32)pNMGRegion->eNMGRegionID;

		// Write the number of polys.

		Converter << (uint32)pNMGRegion->cNMGPolys;

		// Write index into list of poly references to beginning 
		// of this regions's list.

		Converter << (uint32)iPolyListIndex;

		// Advance poly list index past this region's list.

		iPolyListIndex += pNMGRegion->cNMGPolys;

		// Write bounding sphere center and radius.

		Converter << pNMGRegion->vBoundingSphereCenter;
		Converter << pNMGRegion->fBoundingSphereRadius;
	}
	VERIFY_ALIGNMENT( AIRegions );

	//
	// Write components.
	//

	// Write number of components.

	Converter << (uint32)m_lstNMGComponents.size();

	// Iterate over components in NavMeshGen and write each one.

	uint32 iNeighborList = 0;
	SAINAVMESHGEN_COMPONENT* pNMGComponent;
	AINAVMESHGEN_COMPONENT_LIST::iterator itComponent;
	for( itComponent = m_lstNMGComponents.begin(); itComponent != m_lstNMGComponents.end(); ++itComponent )
	{
		// Write component ID.

		pNMGComponent = *itComponent;
		Converter << (uint32)pNMGComponent->eNMGComponentID;

		// Write sensory component ID.

		Converter << (uint32)pNMGComponent->eNMGSensoryComponentID;

		// Write number of neighbors.

		Converter << (uint32)pNMGComponent->lstNeighborComponentPolys.size();

		// Write index into list of component neighbors to beginning 
		// of this component's list.

		Converter << (uint32)iNeighborList;

		// Advance neighbor list index past this component's list.

		iNeighborList += pNMGComponent->lstNeighborComponentPolys.size();
	}
	VERIFY_ALIGNMENT( Components );

	// Write total number of neighbor references, which create 
	// neighbor lists in NavMesh components.

	Converter << (uint32)iNeighborList;

	// Iterate over components in NavMeshGen and add neighbors to neighbor list.

	ENUM_NMGPolyID eNeighborPoly;
	CAINavMeshGenPoly* pNeighborPoly;
	NMGPOLY_LIST::iterator itNeighbor;
	for( itComponent = m_lstNMGComponents.begin(); itComponent != m_lstNMGComponents.end(); ++itComponent )
	{
		// Export neighbor list.

		pNMGComponent = *itComponent;
		for( itNeighbor = pNMGComponent->lstNeighborComponentPolys.begin(); itNeighbor != pNMGComponent->lstNeighborComponentPolys.end(); ++itNeighbor )
		{
			eNeighborPoly = *itNeighbor;
			pNeighborPoly = GetNMGPoly( eNeighborPoly );
			if( !pNeighborPoly )
			{
				return false;
			}

			Converter << (uint32)pNeighborPoly->GetNMGComponentID();
		}
	}
	VERIFY_ALIGNMENT( NeighborLists );

	//
	// Write NavMeshLinks.
	//

	// Write number of NavMeshLinks.

	Converter << (uint32)m_lstNMGLinks.size();

	// Write each NavMeshLink.

	uint32 iBoundaryList = 0;
	SAINAVMESHGEN_LINK* pNMGLink;
	AINAVMESHGEN_LINK_LIST::iterator itNMGLink;
	for( itNMGLink = m_lstNMGLinks.begin(); itNMGLink != m_lstNMGLinks.end(); ++itNMGLink )
	{
		// Write link ID.

		pNMGLink = &( *itNMGLink );
		Converter << (uint32)pNMGLink->eNMGLinkID;

		// Write number of boundary verts.

		Converter << (uint32)pNMGLink->lstLinkBounds.size();

		// Write index into list of boundary verts.

		Converter << (uint32)iBoundaryList;

		// Advance index into boundary vert list.

		iBoundaryList += pNMGLink->lstLinkBounds.size();
	}
	VERIFY_ALIGNMENT( NavMeshLinks );

	// Write the total number of boundary verts.

	Converter << (uint32)iBoundaryList;

	// Iterate over NavMeshLinks in NavMeshGen and add boundary verts to the list.

	LTVector* pBoundary;
	std::vector<LTVector>::iterator itBoundary;
	for( itNMGLink = m_lstNMGLinks.begin(); itNMGLink != m_lstNMGLinks.end(); ++itNMGLink )
	{
		pNMGLink = &( *itNMGLink );
		for( itBoundary = pNMGLink->lstLinkBounds.begin(); itBoundary != pNMGLink->lstLinkBounds.end(); ++itBoundary )
		{
			// Write vert.

			pBoundary = &( *itBoundary );

			Converter << *pBoundary;
		}
	}
	VERIFY_ALIGNMENT( BoundaryVerts );


	//
	// Write node clusters.
	//

	// Write number of nodes.

	Converter << (uint32)m_lstNMGNodes.size();

	// Write each node.

	uint32 iNodeNameIndex = 0;
	SAINAVMESHGEN_NODE* pNMGNode;
	AINAVMESHGEN_NODE_LIST::iterator itNMGNode;
	for( itNMGNode = m_lstNMGNodes.begin(); itNMGNode != m_lstNMGNodes.end(); ++itNMGNode )
	{
		// Write index into node name list.

		pNMGNode = &( *itNMGNode );

		Converter << (uint32)iNodeNameIndex;

		// Advance name index to beginning of next string.

		iNodeNameIndex += pNMGNode->strName.length() + 1;

		// Write cluster ID.

		Converter << (uint32)pNMGNode->eNMGNodeClusterID;
	}
	VERIFY_ALIGNMENT( NodeClusters );

	// Pad the size of the name list to a 4-byte boundary, and write the total size.

	uint32 nPad = 4 - ( iNodeNameIndex % 4 );

	Converter << (uint32)(iNodeNameIndex + nPad);

	// Write the node names.

	for( itNMGNode = m_lstNMGNodes.begin(); itNMGNode != m_lstNMGNodes.end(); ++itNMGNode )
	{
		// Write node name.

		pNMGNode = &( *itNMGNode );

		uint32 nStrLen = LTStrLen(pNMGNode->strName.c_str());
		for(uint32 nCurrChar = 0; nCurrChar <= nStrLen; nCurrChar++)
			Converter << pNMGNode->strName.c_str()[nCurrChar];
	}

	// Advance the buffer pointer past the padding.
	for(;nPad > 0;--nPad)
		Converter << (uint8)0;
	VERIFY_ALIGNMENT( NodeNames );

	//
	// Quad tree.
	//

	// Write number of quad tree nodes.

	Converter << (uint32)m_lstNMGQuadTreeNodes.size();

	// Write each quad tree node.

	uint32 iPolyList = 0;
	CAINavMeshGenQuadTreeNode* pNMGQuadTreeNode;
	NMQUAD_TREE_LIST::iterator itQuadTree;
	for( itQuadTree = m_lstNMGQuadTreeNodes.begin(); itQuadTree != m_lstNMGQuadTreeNodes.end(); ++itQuadTree )
	{
		// Write quad tree node ID.

		pNMGQuadTreeNode = *itQuadTree;
		Converter << (uint32)pNMGQuadTreeNode->GetNMQTNodeID();

		// Write bounding box.

		Converter << pNMGQuadTreeNode->GetAABB()->vMin;
		Converter << pNMGQuadTreeNode->GetAABB()->vMax;

		// Write number of polys.

		Converter << (uint32)pNMGQuadTreeNode->GetNumNMQTNodePolyRefs();

		// Write index into the list of of polys.

		Converter << (uint32)iPolyList;

		// Advance the index to the next list of polys.

		iPolyList += pNMGQuadTreeNode->GetNumNMQTNodePolyRefs();

		// Write children IDs.

		for( int iNode=0; iNode < 4; ++iNode )
		{
			if( pNMGQuadTreeNode->GetNMQTChildNode(iNode) )
			{
				Converter << (uint32)pNMGQuadTreeNode->GetNMQTChildNode(iNode)->GetNMQTNodeID();
			}
			else 
			{
				Converter << (uint32)kNMGQTNode_Invalid;
			}
		}
	}
	VERIFY_ALIGNMENT( QuadTree );

	// Write total number of poly references.

	Converter << (uint32)iPolyList;

	// Write the poly list from each quad tree node into the master list.

	float fHeight;
	ENUM_NMGPolyID eNMGPolyID;
	NMGPOLY_HEIGHT_MAP mapPolyHeights;
	NMGPOLY_LIST::const_iterator itPolyRef;
	const NMGPOLY_LIST* plstPolyRefs;
	for( itQuadTree = m_lstNMGQuadTreeNodes.begin(); itQuadTree != m_lstNMGQuadTreeNodes.end(); ++itQuadTree )
	{
		pNMGQuadTreeNode = *itQuadTree;
		plstPolyRefs = pNMGQuadTreeNode->GetNMQTNodePolyRefs();
		mapPolyHeights.clear();

		// Sort polys by height.

		for( itPolyRef = plstPolyRefs->begin(); itPolyRef != plstPolyRefs->end(); ++itPolyRef )
		{
			// Get the poly's height, determined by the bottom of its bounding box.

			eNMGPolyID = *itPolyRef;
			pNMGPoly = CAINavMeshGen::GetAINavMeshGen()->GetNMGPoly( eNMGPolyID );
			fHeight = pNMGPoly->GetAABB()->vMin.y;

			// Check if the new poly overlaps any previously inserted poly.

			NMGPOLY_HEIGHT_MAP::iterator itHeight;
			for( itHeight = mapPolyHeights.lower_bound( fHeight ); itHeight != mapPolyHeights.upper_bound( fHeight ); ++itHeight )
			{
				// Bail if other doesn't exist.

				ENUM_NMGPolyID eNMGOtherID = itHeight->second;
				CAINavMeshGenPoly* pOther = GetNMGPoly( eNMGOtherID );
				if( !pOther )
				{
					continue;
				}						

				// Don't bother testing if we already know that these polies are considered neighbors.

				bool bNeighbor = false;
				int cNeighbors = pOther->GetNumNMGPolyNeighbors();
				for( int iNeighbor=0; iNeighbor < cNeighbors; ++iNeighbor )
				{
					if( pOther->GetNMGPolyNeighbor( iNeighbor ) == eNMGPolyID )
					{
						bNeighbor = true;
						break;
					}
				}

				if( bNeighbor )
				{
					continue;
				}

				// Warn if polys overlap.

				LTVector vOverlap;
				if( pOther->FindNMGOverlappingVert( pNMGPoly, &vOverlap ) )
				{
					LTVector vDisplayableOverlap = NAVMESH_CONVERTPOS( vOverlap );
					NAVMESH_ERROR3("NAVMESH WARNING: Potential overlapping NavMesh brushes at WorldEdit pos: %f %f %f",
						vDisplayableOverlap.x, vDisplayableOverlap.y, vDisplayableOverlap.z );
				}
			}

			// Insert the poly into the map of sorted heights.

			mapPolyHeights.insert( NMGPOLY_HEIGHT_MAP::value_type( fHeight, eNMGPolyID ) );
		}

		// Write poly IDs, sorted in descending order.

		int iPoly = 0;
		NMGPOLY_HEIGHT_MAP::reverse_iterator ritHeights;
		for( ritHeights = mapPolyHeights.rbegin(); ritHeights != mapPolyHeights.rend(); ++ritHeights )
		{
			eNMGPolyID = ritHeights->second;

			Converter << (uint32)ConvertToNMPolyID( eNMGPolyID );
		}
	}
	VERIFY_ALIGNMENT( PolyIDs );

	// Successful write!

	return true;
}


