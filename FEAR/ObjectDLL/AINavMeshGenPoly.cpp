// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenPoly.cpp
//
// PURPOSE : AI NavMesh generator polygon class implementation.
//
// CREATED : 11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshGenPoly.h"
#include "AINavMeshGen.h"
#include "AINavMeshGenMacros.h"


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::Con/destructor
//              
//	PURPOSE:	Con/destructor
//              
//----------------------------------------------------------------------------

CAINavMeshGenPoly::CAINavMeshGenPoly( IGameWorldPacker::PFNERRORCALLBACK pfnErrorCallback ) :
	m_pfnErrorCallback(pfnErrorCallback)
{
	m_vNMGPolyCenter.Init();
	m_aabbNMGPolyBounds.InitAABB();

	m_eNMGPolyID = kNMGPoly_Invalid;
	m_eNMGConvertedPolyID = kNMGConvertedPoly_Invalid;
	m_eNMGParentPolyID = kNMGPoly_Invalid;
	m_eNMGParentNavMeshID = kNMGNavMesh_Invalid;
	m_eNMGComponentID = kNMGComponent_Invalid;
	m_eNMGLinkID = kNMGLink_Invalid;
	m_eNMGNormalID = kNMGNormal_Invalid;
	m_dwNMGCharTypeMask = 0;
	m_cNMGPolyEdges = 0;
	m_cNMGRegions = 0;
}

CAINavMeshGenPoly::~CAINavMeshGenPoly()
{
	m_lstNMGVerts.resize( 0 );
	m_aabbNMGPolyBounds.InitAABB();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::InitNMGPoly
//              
//	PURPOSE:	Initialize poly.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::InitNMGPoly()
{
	// Calculate the center point and axis-aligned bounding box of the polygon.

	m_vNMGPolyCenter = LTVector( 0.f, 0.f, 0.f );

	LTVector vVert;
	NMGVERT_LIST::iterator itVert;
	for( itVert = m_lstNMGVerts.begin(); itVert != m_lstNMGVerts.end(); ++itVert )
	{
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( *itVert, &vVert );
		m_vNMGPolyCenter += vVert;

		m_aabbNMGPolyBounds.GrowAABB( vVert );
	}

	m_vNMGPolyCenter /= (float)m_lstNMGVerts.size();


	// Calculate the poly normal.

	// Get the first 3 verts.

	LTVector p0, p1, p2;
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( 0 ), &p0 );
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( 1 ), &p1 );
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( 2 ), &p2 );

	LTVector v0, v1;
	
	v0 = p0 - p1;
	v0.Normalize();

	v1 = p2 - p1;
	v1.Normalize();

	LTVector vNMGPolyN = v0.Cross( v1 );
	if( vNMGPolyN != LTVector::GetIdentity() )
	{
		vNMGPolyN.Normalize();
	}
	m_eNMGNormalID = CAINavMeshGen::GetAINavMeshGen()->AddNormalToPool( vNMGPolyN );

	// Initialize polys plane.

	m_plnNMGPolyPlane.InitNMGPlane( m_eNMGNormalID, p0 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::AddNMGVert()
//              
//	PURPOSE:	Add a vertex to poly.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::AddNMGVert( ENUM_NMGVertID eVert )
{
	m_lstNMGVerts.push_back( eVert );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::RemoveNMGVert()
//              
//	PURPOSE:	Remove a vertex from poly.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::RemoveNMGVert( const int iVert )
{
	m_lstNMGVerts.erase( m_lstNMGVerts.begin() + iVert );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::InsertNMGVert()
//              
//	PURPOSE:	Insert a vertex into poly between specified verts.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::InsertNMGVert( ENUM_NMGVertID eVert, ENUM_NMGVertID ePrevVert, ENUM_NMGVertID eNextVert )
{
	NMGVERT_LIST::iterator itVert;
	ENUM_NMGVertID eVert0, eVert1;

	// Find boundary verts.

	int iNext;
	int cVerts = GetNumNMGVerts();
	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		iNext = ( iVert + 1 ) % cVerts;
		eVert0 = GetNMGVert( iVert );
		eVert1 = GetNMGVert( iNext );

		// Insert vert.

		if( ( ( eVert0 == ePrevVert ) && ( eVert1 == eNextVert ) ) ||
			( ( eVert0 == eNextVert ) && ( eVert1 == ePrevVert ) ) )
		{
			itVert = m_lstNMGVerts.begin() + iNext;
			m_lstNMGVerts.insert( itVert, eVert );
			return;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::ContainsNMGVert()
//              
//	PURPOSE:	Returns true if poly contains vertex.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::ContainsNMGVert( ENUM_NMGVertID eVert )
{
	NMGVERT_LIST::iterator itVert;
	for( itVert = m_lstNMGVerts.begin(); itVert != m_lstNMGVerts.end(); ++itVert )
	{
		if( eVert == *itVert )
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::CopyNMGVerts()
//              
//	PURPOSE:	Copy verts from one poly to another, starting at a specified NMGVert index.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::CopyNMGVerts( CAINavMeshGenPoly* pPolySource, ENUM_NMGVertID eVertStart )
{
	int iVert;
	for( iVert=0; iVert < pPolySource->GetNumNMGVerts(); ++iVert )
	{
		if( pPolySource->GetNMGVert( iVert ) == eVertStart )
		{
			break;
		}
	}

	if( iVert == pPolySource->GetNumNMGVerts() )
	{
		return;
	}

	while( iVert < pPolySource->GetNumNMGVerts() )
	{
		AddNMGVert( pPolySource->GetNMGVert( iVert ) );
		++iVert;
	}

	iVert = 0;
	while( pPolySource->GetNMGVert( iVert ) != eVertStart )
	{
		AddNMGVert( pPolySource->GetNMGVert( iVert ) );
		++iVert;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::SimplifyNMGPoly()
//              
//	PURPOSE:	Simplify poly by removing duplicate and superflous verts.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::SimplifyNMGPoly()
{
	// First remove duplicate verts.

	int cVerts = GetNumNMGVerts();
	ENUM_NMGVertID eLastVert = GetNMGVert( cVerts - 1 );

	ENUM_NMGVertID eCurVert, eNextVert;
	int iVert = 0;
	while( iVert < cVerts )
	{
		eCurVert = GetNMGVert( iVert );
		if( eCurVert == eLastVert )
		{
			RemoveNMGVert( iVert );
			--cVerts;
		}
		else {
			eLastVert = eCurVert;
			++iVert;
		}
	}


	// Next remove superfluous verts.

	eLastVert = GetNMGVert( cVerts - 1 );

	iVert = 0;
	while( iVert < cVerts - 1 )
	{
		eCurVert = GetNMGVert( iVert );
		eNextVert = GetNMGVert( iVert + 1 );
		
		if( IsNMGVertSuperfluous( eLastVert, eCurVert, eNextVert ) )
		{
			RemoveNMGVert( iVert );
			--cVerts;
		}
		else {
			eLastVert = eCurVert;
			++iVert;
		}
	}

	eCurVert = GetNMGVert( iVert );
	eNextVert = GetNMGVert( 0 );
	if( IsNMGVertSuperfluous( eLastVert, eCurVert, eNextVert ) )
	{
		RemoveNMGVert( iVert );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::IsNMGVertSuperfluous()
//              
//	PURPOSE:	Returns true if vert is superfluous.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::IsNMGVertSuperfluous( ENUM_NMGVertID eVertPrev, ENUM_NMGVertID eVertCur, ENUM_NMGVertID eVertNext )
{
	LTVector p0, p1, p2, v0, v1, vDiff;

	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertPrev, &p0 );
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertCur,  &p1 );
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertNext, &p2 );

	v0 = p1 - p0;
	v1 = p2 - p0;
	v0.Normalize();
	v1.Normalize();

	// Use a more stringent epsilon for co-linear verts.

	float fEpsilon = 0.01f;

	// A vert is superflous if it falls on the line between the prev and next verts.

	vDiff = v1 - v0;
	if( vDiff.MagSqr() < ( fEpsilon * fEpsilon ) )
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::IsNMGPolyConvex()
//              
//	PURPOSE:	Returns true if poly is convex.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::IsNMGPolyConvex()
{
	// Bail if degenerate poly.

	int cVerts = GetNumNMGVerts();
	if( cVerts < 3 )
	{
		return false;
	}

	// Triangles are always convex.

	if( cVerts == 3 )
	{
		return true;
	}

	// Get actual normal by pool ID.

	LTVector vNMGPolyN;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( m_eNMGNormalID, &vNMGPolyN );

	// Test edge normals, and check that all points are on inside of edge.

	int iTest, iNext;
	LTVector ptStart, ptEnd, vEdge, vEdgeN, ptTest, vTest;
	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		iNext = ( iVert + 1 ) % cVerts;
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iVert ), &ptStart );
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iNext ), &ptEnd );

		vEdge = ptEnd - ptStart;
		vEdge.Normalize();

		vEdgeN = vNMGPolyN.Cross( vEdge );

		// Iterate over verts to determine if they are all on the same
		// side of the edge.

		for( iTest=0; iTest < cVerts; ++iTest )
		{
			if( ( iTest == iVert ) ||
				( iTest == iNext ) )
			{
				continue;
			}

			CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iTest ), &ptTest );
			vTest = ptStart - ptTest;

			// Vert is on wrong side of edge.

			if( vEdgeN.Dot( vTest ) < 0.f )
			{
				return false;
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::PolyHasDuplicateVerts()
//              
//	PURPOSE:	Returns true if poly has duplicate verts.
//              Useful for debugging NavMesh.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::PolyHasDuplicateVerts()
{
	ENUM_NMGVertID eVertA, eVertB;

	int cVerts = GetNumNMGVerts();
	for( int iVertA=0; iVertA < cVerts - 1; ++iVertA )
	{
		eVertA = GetNMGVert( iVertA );
		for( int iVertB = iVertA + 1; iVertB < cVerts; ++iVertB )
		{
			eVertB = GetNMGVert( iVertB );
			if( eVertA == eVertB )
			{
				return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::IsNMGPolyAreaGreaterThan()
//              
//	PURPOSE:	Returns true if area of poly is greater than some threshold.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::IsNMGPolyAreaGreaterThan( float fThreshold )
{
	LTVector vVert0, vVert1;

	// Poly only has 3 sides (is a triangle).

	int cVerts = GetNumNMGVerts();
	if( cVerts == 3 )
	{
		LTVector vVert2;
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( 0 ), &vVert0 );
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( 1 ), &vVert1 );
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( 2 ), &vVert2 );

		return ( ComputeNMGPolyTriArea( vVert0, vVert1, vVert2 ) > fThreshold );
	}

	// Poly has more than 3 verts.

	int iNext;
	float fTotalArea = 0.f;
	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		iNext = ( iVert + 1 ) % cVerts;
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iVert ), &vVert0 );
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iNext ), &vVert1 );

		fTotalArea += ComputeNMGPolyTriArea( vVert0, vVert1, m_vNMGPolyCenter );
		if( fTotalArea > fThreshold )
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::ComputeNMGPolyTriArea()
//              
//	PURPOSE:	Get area of triangle made up from 3 verts of a 
//              NavMeshGen poly.
//              
//----------------------------------------------------------------------------

float CAINavMeshGenPoly::ComputeNMGPolyTriArea( const LTVector& vVert0,  const LTVector& vVert1, const LTVector& vVert2 )
{
	// Compute the magnitudes of the triangle sides, a, b, and c.

	float a, b, c;
	LTVector vSide;

	vSide = vVert0 - vVert1;
	a = vSide.Mag();

	vSide = vVert1 - vVert2;
	b = vSide.Mag();

	vSide = vVert2 - vVert0;
	c = vSide.Mag();

	// Compute the semi-perimeter:
	// s = 0.5f*p = 0.5f * ( a + b + c )
	// where s is the semiperimeter, and p is the perimeter.

	float s = 0.5f * ( a + b + c );

	// Heron's formula:
	// area = sqrt( s * (s-a) * (s-b) * (s-c) )

	float fArea = (float)sqrt( s * ( s - a ) * ( s - b ) * ( s - c ) );
	return fArea;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::GetNeighborNMGVerts()
//              
//	PURPOSE:	Get verts on either side of given vert.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::GetNeighborNMGVerts( ENUM_NMGVertID eVert, ENUM_NMGVertID* peVertPrev, ENUM_NMGVertID* peVertNext )
{
	*peVertPrev = *( m_lstNMGVerts.end() - 1 );
	*peVertNext = *( m_lstNMGVerts.begin() );

	// Find the verts before and after the given vert.

	NMGVERT_LIST::iterator itVert;
	for( itVert = m_lstNMGVerts.begin(); itVert != m_lstNMGVerts.end(); ++itVert )
	{
		if( *itVert == eVert )
		{
			if( itVert != m_lstNMGVerts.begin() )
			{
				*peVertPrev = *( itVert - 1 ); 
			}
			if( itVert != m_lstNMGVerts.end() - 1 )
			{
				*peVertNext = *( itVert + 1 ); 
			}
			break;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::SharesNMGEdge()
//              
//	PURPOSE:	Returns true if polys share an edge.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::SharesNMGEdge( ENUM_NMGVertID eVertShared, ENUM_NMGPolyID eNMGPolyID )
{
	ENUM_NMGVertID eVertPrev;
	ENUM_NMGVertID eVertNext;

	// Find the verts before and after the shared vert.

	GetNeighborNMGVerts( eVertShared, &eVertPrev, &eVertNext );

	// Check if edge from prev to shared vert is shared with the poly.

	SAINAVMESHGEN_EDGE* pEdge = CAINavMeshGen::GetAINavMeshGen()->GetNMGEdge( eVertPrev, eVertShared );
	if( pEdge &&
		( ( pEdge->ePolyID1 == eNMGPolyID ) ||
		  ( pEdge->ePolyID2 == eNMGPolyID ) ) )
	{
		return true;
	}

	// Check if edge from next to shared vert is shared with the poly.

	pEdge = CAINavMeshGen::GetAINavMeshGen()->GetNMGEdge( eVertNext, eVertShared );
	if( pEdge &&
		( ( pEdge->ePolyID1 == eNMGPolyID ) ||
		  ( pEdge->ePolyID2 == eNMGPolyID ) ) )
	{
		return true;
	}

	// Neither edge is shared with the poly.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::IsValidForMerge32()
//              
//	PURPOSE:	Returns true if polys are adjacent and share a neighbor.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::IsValidForMerge32( SAINAVMESHGEN_MERGE_3_2_RESULTS* pmrg32, CAINavMeshGenPoly* pPolyB, CAINavMeshGen* pAINavMeshGen )
{
	// Sanity check.

	if( !( pmrg32 && pPolyB && pAINavMeshGen ) )
	{
		return false;
	}

	// Character type flags must match to validate a merge.

	if( m_dwNMGCharTypeMask != pPolyB->GetNMGCharTypeMask() )
	{
		return false;
	}

	ENUM_NMGVertID eVertShared = pmrg32->eVert1;

	ENUM_NMGVertID eVertA[2], eVertB[2];
	CAINavMeshGenPoly* pPolyC;

	// Find verts on either side of shared vert in each poly.

	GetNeighborNMGVerts( eVertShared, &(eVertA[0]), &(eVertA[1]) );
	pPolyB->GetNeighborNMGVerts( eVertShared, &(eVertB[0]), &(eVertB[1]) );

	// Find polyC shared by verts of polyA and polyB.
	// PolyC must have the same normal as polyA and polyB.
	// PolyC may not have an associated NavMeshLink,
	// because Link polys may not be used in a 32 merge.

	int iVertA, iVertB;
	ENUM_NMGPolyID ePolyC = kNMGPoly_Invalid;
	for( iVertA=0; iVertA < 2; ++iVertA )
	{
		for( iVertB=0; iVertB < 2; ++iVertB )
		{
			ePolyC = FindSharedNeighborPoly( eVertA[iVertA], eVertB[iVertB] );
			if( ePolyC != kNMGPoly_Invalid )
			{
				pPolyC = pAINavMeshGen->GetNMGPoly( ePolyC );
				if(   pPolyC && 
					( pPolyC->GetNMGLinkID() == kNMGLink_Invalid ) &&
					( pPolyC->GetNMGNormalID() == m_eNMGNormalID ) )
				{
					break;
				}
			}
		}
	
		if( ePolyC != kNMGPoly_Invalid )
		{
			pPolyC = pAINavMeshGen->GetNMGPoly( ePolyC );
			if(   pPolyC && 
				( pPolyC->GetNMGLinkID() == kNMGLink_Invalid ) &&
				( pPolyC->GetNMGNormalID() == m_eNMGNormalID ) )
			{
				break;
			}
		}
	}

	// No shared neighbor was found.

	if( ePolyC == kNMGPoly_Invalid )
	{
		return false;
	}

	// Verts sharing polyC must form an edge of polyC.

	SAINAVMESHGEN_EDGE* pEdge = CAINavMeshGen::GetAINavMeshGen()->GetNMGEdge( eVertA[iVertA], eVertB[iVertB] );
	if( !( pEdge &&
		 ( ( pEdge->ePolyID1 == ePolyC ) ||
		   ( pEdge->ePolyID2 == ePolyC ) ) ) )
	{
		return false;
	}

	pmrg32->eVert6 = eVertA[iVertA];
	pmrg32->eVert7 = eVertB[iVertB];

	// Verts on polyC must form single straight line with shared vert.

	SAINAVMESHGEN_VERT* pVertA = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( eVertA[iVertA] );
	SAINAVMESHGEN_VERT* pVertB = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( eVertB[iVertB] );
	SAINAVMESHGEN_VERT* pVertShared = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( eVertShared );

	LTVector vSeg1 = pVertShared->vVert - pVertA->vVert;
	LTVector vSeg2 = pVertB->vVert - pVertShared->vVert;

	vSeg1.Normalize();
	vSeg2.Normalize();
	if( vSeg1 != vSeg2 )
	{
		return false;
	}

	pmrg32->ePolyC = ePolyC;

	// Of the verts neighboring the shared vert, that do not touch polyC, 
	// determine which is closer to the shared vert.
	// This will be vert2.

	iVertA = ( iVertA + 1 ) % 2;
	iVertB = ( iVertB + 1 ) % 2;
	pVertA = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( eVertA[iVertA] );
	pVertB = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( eVertB[iVertB] );

	vSeg1 = pVertA->vVert - pVertShared->vVert;
	vSeg2 = pVertB->vVert - pVertShared->vVert;

	// Once vert2 is discovered, vert4 is the neighbor of vert2 that
	// is not the vert shared between polyA and polyB.

	if( vSeg1.MagSqr() < vSeg2.MagSqr() )
	{
		pmrg32->ePolyA = m_eNMGPolyID;
		pmrg32->ePolyB = pPolyB->GetNMGPolyID();

		pmrg32->eVert2 = eVertA[iVertA];
		pmrg32->eVert3 = eVertB[iVertB];

		GetNeighborNMGVerts( pmrg32->eVert2, &(eVertA[0]), &(eVertA[1]) );
		if( eVertA[0] == eVertShared )
		{
			pmrg32->eVert4 = eVertA[1];
		}
		else {
			pmrg32->eVert4 = eVertA[0];
		}
	}
	else {
		pmrg32->ePolyA = pPolyB->GetNMGPolyID();
		pmrg32->ePolyB = m_eNMGPolyID;

		pmrg32->eVert2 = eVertB[iVertB];
		pmrg32->eVert3 = eVertA[iVertA];

		pPolyB->GetNeighborNMGVerts( pmrg32->eVert2, &(eVertB[0]), &(eVertB[1]) );
		if( eVertB[0] == eVertShared )
		{
			pmrg32->eVert4 = eVertB[1];
		}
		else {
			pmrg32->eVert4 = eVertB[0];
		}
	}

	// Ensure that polyA and polyB are adjacent.

	vSeg1.Normalize();
	vSeg2.Normalize();
	if( vSeg1 != vSeg2 )
	{
		return false;
	}


TRACE( "PolyA: %d PolyB: %d PolyC: %d\n", pmrg32->ePolyA, pmrg32->ePolyB, pmrg32->ePolyC );

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::FindSharedNeighborPoly()
//              
//	PURPOSE:	Find a neighbor poly shared by both verts.
//              
//----------------------------------------------------------------------------

ENUM_NMGPolyID CAINavMeshGenPoly::FindSharedNeighborPoly( ENUM_NMGVertID eVertA, ENUM_NMGVertID eVertB )
{
	SAINAVMESHGEN_VERT* pVertA = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( eVertA );
	SAINAVMESHGEN_VERT* pVertB = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( eVertB );

	if( !( pVertA && pVertB ) )
	{
		return kNMGPoly_Invalid;
	}

	// Iterate over polys referenced by vertA and check if any are referenced by vertB.

	NMGPOLY_LIST::iterator itPolyA, itPolyB;
	for( itPolyA = pVertA->lstPolyRefs.begin(); itPolyA != pVertA->lstPolyRefs.end(); ++itPolyA )
	{
		if( *itPolyA == m_eNMGPolyID )
		{
			continue;
		}

		for( itPolyB = pVertB->lstPolyRefs.begin(); itPolyB != pVertB->lstPolyRefs.end(); ++itPolyB )
		{
			// Found a match.

			if( *itPolyA == *itPolyB )
			{
				return *itPolyA;
			}
		}
	}

	// No match found.

	return kNMGPoly_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::IsPolyOnOneSideOfRay()
//              
//	PURPOSE:	Return true of entire poly is on same side of the ray.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::IsPolyOnOneSideOfRay( SAINAVMESHGEN_POLY_INTERSECT* pintersect )
{
	// Get poly normal by pool ID.

	LTVector vNMGPolyN;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( m_eNMGNormalID, &vNMGPolyN );

	// Calculate ray direction.

	LTVector vRayDir = pintersect->v1 - pintersect->v0;
	vRayDir.Normalize();

	// Calculate the ray's normal.

	LTVector vRayN = vNMGPolyN.Cross( vRayDir );

	float fLast = 0.f;

	// Iterate over all verts in poly to determine if they are all on 
	// the same side of the ray.

	float fDot;
	LTVector vVert, vVertDir;
	int cVerts = GetNumNMGVerts();
	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iVert ), &vVert );

		// Ignore verts that are basically on the ray,
		// within some floating point error.

		vVertDir = vVert - pintersect->v0;
		if( vVertDir != LTVector::GetIdentity() )
		{
			vVertDir.Normalize();
		}
		fDot = vVertDir.Dot( vRayDir );

		float fHyp = vVert.Dist( pintersect->v0 );
		float fTheta = (float)acos( fDot );
		float fDist = fHyp * (float)sin( fTheta );
		if( fDist < 1.f )
		{
			continue;
		}

		// If the sign of the dot product has changed,
		// then there are verts on both sides of the ray,
		// so the poly is not all on one side of the ray.

		fDot = vVertDir.Dot( vRayN );
		if( fDot != 0.f )
		{
			if( ( fLast > 0.f ) && ( fDot < 0.f ) )
			{
				return false;
			}

			if( ( fLast < 0.f ) && ( fDot > 0.f ) )
			{
				return false;
			}

			fLast = fDot;
		}
	}

	// The poly is all on one side of the ray.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::IsPolyOnOutsideOfRay()
//              
//	PURPOSE:	Return true of entire poly is on the outside side of the ray.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::IsPolyOnOutsideOfRay( SAINAVMESHGEN_POLY_INTERSECT* pintersect, const LTVector& vInside )
{
	// Get poly normal by pool ID.

	LTVector vNMGPolyN;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( m_eNMGNormalID, &vNMGPolyN );

	// Calculate ray direction.

	LTVector vRayDir = pintersect->v1 - pintersect->v0;
	vRayDir.Normalize();

	// Calculate the ray's normal.

	LTVector vRayN = vNMGPolyN.Cross( vRayDir );

	// Calculate the dot product of the inside point and the ray.

	LTVector vVertDir = vInside - pintersect->v0;	
	vVertDir.Normalize();
	float fInside = vVertDir.Dot( vRayN );

	// Iterate over all verts in poly to determine if they are all on 
	// the same side of the ray.

	float fDot;
	LTVector vVert;
	int cVerts = GetNumNMGVerts();
	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iVert ), &vVert );

		// Ignore verts that are basically on the ray,
		// within some floating point error.

		vVertDir = vVert - pintersect->v0;
		if( vVertDir != LTVector::GetIdentity() )
		{
			vVertDir.Normalize();
		}
		fDot = vVertDir.Dot( vRayDir );

		float fHyp = vVert.Dist( pintersect->v0 );
		float fTheta = (float)acos( fDot );
		float fDist = fHyp * (float)sin( fTheta );
		if( fDist < 1.f )
		{
			continue;
		}

		// If the sign of the dot product matches the
		// dot product of the inside point, then the 
		// poly has a point on the inside of the ray,
		// so it cannot be all outside of the ray.

		fDot = vVertDir.Dot( vRayN );
		if( fDot != 0.f )
		{
			if( ( fInside > 0.f ) && ( fDot > 0.f ) )
			{
				return false;
			}

			if( ( fInside < 0.f ) && ( fDot < 0.f ) )
			{
				return false;
			}
		}
	}

	// The poly is all on the outside of the ray.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::CoplanarRayIntersectPoly()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::CoplanarRayIntersectPoly( SAINAVMESHGEN_POLY_INTERSECT* pintersect )
{	
	// Adapted from Haines ray intersection algorithm in Watt's "3D Games", p.19.

	// Points of ray must be coplanar with poly.

	if( !( m_plnNMGPolyPlane.IsCoplanar( pintersect->v0 ) &&
		   m_plnNMGPolyPlane.IsCoplanar( pintersect->v1 ) ) )
	{
		return false;
	}

	// Get poly normal by pool ID.

	LTVector vNMGPolyN;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( m_eNMGNormalID, &vNMGPolyN );

	// Calculate ray direction.

	LTVector vRayDir = pintersect->v1 - pintersect->v0;
	vRayDir.Normalize();

	// Extend ray in both directions.

	LTVector vRay0 = pintersect->v0 - ( vRayDir * 500000.f );
	LTVector vRay1 = pintersect->v1 + ( vRayDir * 500000.f );

	LTVector vt;
	float tnear = 0.f;
	float tfar  = FLT_MAX;
	float t, dot;
	int iNext;
	int cVerts = GetNumNMGVerts();
	SAINAVMESHGEN_PLANE plnEdge;
	LTVector ptStart, ptEnd, vEdge, vEdgeN, ptTest, vTest;
	LTVector vIntersect;
	LTVector vNear, vFar;

	// Iterate over poly edges, and test for intersections.

	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		iNext = ( iVert + 1 ) % cVerts;
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iVert ), &ptStart );
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iNext ), &ptEnd );

		vEdge = ptEnd - ptStart;
		vEdge.Normalize();

		// Calculate the edge's normal.

		vEdgeN = vNMGPolyN.Cross( vEdge );
		ENUM_NMGNormalID eNMGNormalID = CAINavMeshGen::GetAINavMeshGen()->AddNormalToPool( vEdgeN );

		// Calculate the edge's plane.

		plnEdge.InitNMGPlane( eNMGNormalID, ptStart );

		// Determine if/where ray intersects plane.

		if( !plnEdge.RayIntersectNMGPlane( vRay0, vRay1, &vIntersect ) )
		{
			continue;
		}

		// Keep track of the furtherest tnear and nearest tfar.

		vt = vIntersect - vRay0;
		t = vt.MagSqr();
		dot = vEdgeN.Dot( vRayDir );

		// tfar is where the ray hits a back facing poly.

		if( ( dot > 0.f ) &&
			( t < tfar ) )
		{
			tfar = t;
			vFar = vIntersect;
			pintersect->eVertIntersectFar = CAINavMeshGen::GetAINavMeshGen()->AddVertToPool( vIntersect );
			pintersect->eVertEdgeFar0 = GetNMGVert( iVert );
			pintersect->eVertEdgeFar1 = GetNMGVert( iNext );
		}

		// tnear is where the ray hits a front facing poly.

		else if( ( dot < 0.f ) &&
				 ( t > tnear ) )
		{
			tnear = t;
			vNear = vIntersect;
			pintersect->eVertIntersectNear = CAINavMeshGen::GetAINavMeshGen()->AddVertToPool( vIntersect );
			pintersect->eVertEdgeNear0 = GetNMGVert( iVert );
			pintersect->eVertEdgeNear1 = GetNMGVert( iNext );
		}
	}

	// Ray missed the poly is tnear is greater than tfar.

	if( tnear > tfar )
	{
		return false;
	}
	
	// Check if intersection point is within the given line segment.

	if( pintersect->v1.NearlyEquals( vNear ) ||
		pintersect->v0.NearlyEquals( vFar ) )
	{
		return false;
	}

	vTest = pintersect->v1 - vRay0;
	if( vTest.MagSqr() <= tnear )
	{
		return false;
	}

	vTest = pintersect->v0 - vRay0;
	if( vTest.MagSqr() >= tfar )
	{
		return false;
	}

	// Valid intersection was found.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::SplitPoly()
//              
//	PURPOSE:	Split Poly into polyA and polyB.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::SplitPoly( SAINAVMESHGEN_POLY_INTERSECT* pintersect, CAINavMeshGenPoly*& pPolyA, CAINavMeshGenPoly*& pPolyB )
{
	ENUM_NMGVertID eVert0, eVert1;
	NMGVERT_LIST lstNewVerts;

	// Generate list of verts in poly, with new verts inserted in edges to split.
	// Reserve space in the new vert list for all of the verts (with 2 new verts) 
	// plus 1.  The extra 1 is a buffer used when shifting the list, explained below.

	int iNext;
	int cVerts = GetNumNMGVerts();
	lstNewVerts.reserve( cVerts + 3 );
	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		iNext = ( iVert + 1 ) % cVerts;
		eVert0 = GetNMGVert( iVert );
		eVert1 = GetNMGVert( iNext );

		lstNewVerts.push_back( eVert0 );

		// Split edge between VertEdgeFar0 and VertEdgeFar1.

		if( ( ( eVert0 == pintersect->eVertEdgeFar0 ) && ( eVert1 == pintersect->eVertEdgeFar1 ) ) ||
			( ( eVert0 == pintersect->eVertEdgeFar1 ) && ( eVert1 == pintersect->eVertEdgeFar0 ) ) )
		{
			// Do not insert duplicate verts, if the intersection
			// vert is identical to a vert already in the poly.

			if( ( eVert0 != pintersect->eVertIntersectFar ) &&
				( eVert1 != pintersect->eVertIntersectFar ) )
			{
				lstNewVerts.push_back( pintersect->eVertIntersectFar );
			}
		}

		// Split edge between VertEdgeNear0 and VertEdgeNear1.

		else if( ( ( eVert0 == pintersect->eVertEdgeNear0 ) && ( eVert1 == pintersect->eVertEdgeNear1 ) ) ||
				 ( ( eVert0 == pintersect->eVertEdgeNear1 ) && ( eVert1 == pintersect->eVertEdgeNear0 ) ) )
		{
			// Do not insert duplicate verts, if the intersection
			// vert is identical to a vert already in the poly.

			if( ( eVert0 != pintersect->eVertIntersectNear ) &&
				( eVert1 != pintersect->eVertIntersectNear ) )
			{
				lstNewVerts.push_back( pintersect->eVertIntersectNear );
			}
		}
	}

	// Shift vert list to start with vertIntersectNear.
	// It is safe to push_back while continuing to use
	// the same iterator, because we reserved extra space
	// ahead of time (explained above).

	NMGVERT_LIST::iterator itVert = lstNewVerts.begin();
	while( *itVert != pintersect->eVertIntersectNear )
	{
		lstNewVerts.push_back( *itVert );
		itVert = lstNewVerts.erase( itVert );
	}

	// Create first new poly.

	pPolyA = NAVMESH_NEW1( CAINavMeshGenPoly, m_pfnErrorCallback );
	itVert = lstNewVerts.begin();
	while( *itVert != pintersect->eVertIntersectFar )
	{
		pPolyA->AddNMGVert( *itVert );
		++itVert;
	}
	pPolyA->AddNMGVert( *itVert );
	pPolyA->InitNMGPoly();
	pPolyA->SetNMGCharTypeMask( m_dwNMGCharTypeMask );
	pPolyA->SetNMGParentNavMeshID( m_eNMGParentNavMeshID );

	// If this poly has an ID, set it as the new poly's parent.
	// Otherwise, this is a new poly too, so the new poly's parent is this poly's parent.

	pPolyA->m_eNMGParentPolyID = ( m_eNMGPolyID != kNMGPoly_Invalid ) ? m_eNMGPolyID : m_eNMGParentPolyID;

	if( pPolyA->PolyHasDuplicateVerts() )
	{
		NAVMESH_ERROR( "CAINavMeshGenPoly::SplitPoly: Created a poly with duplicate verts!" );
	}

	// Create second new poly.

	pPolyB = NAVMESH_NEW1( CAINavMeshGenPoly, m_pfnErrorCallback );
	while( itVert != lstNewVerts.end() )
	{
		pPolyB->AddNMGVert( *itVert );
		++itVert;
	}
	pPolyB->AddNMGVert( *( lstNewVerts.begin() ) );
	pPolyB->InitNMGPoly();
	pPolyB->SetNMGCharTypeMask( m_dwNMGCharTypeMask );
	pPolyB->SetNMGParentNavMeshID( m_eNMGParentNavMeshID );

	// If this poly has an ID, set it as the new poly's parent.
	// Otherwise, this is a new poly too, so the new poly's parent is this poly's parent.

	pPolyB->m_eNMGParentPolyID = ( m_eNMGPolyID != kNMGPoly_Invalid ) ? m_eNMGPolyID : m_eNMGParentPolyID;

	if( pPolyB->PolyHasDuplicateVerts() )
	{
		NAVMESH_ERROR( "CAINavMeshGenPoly::SplitPoly: Created a poly with duplicate verts!" );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::SplitPolyOnVert()
//              
//	PURPOSE:	Return true if poly can be split on specified vert.
//              Pointers passed as parameters get assigned to new polys.
//              This splitting routine does not add any new verts.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::SplitPolyOnVert( ENUM_NMGVertID eVert, AINAVMESHGEN_SPLITTING_VERT_LIST* plstSplittingVerts, CAINavMeshGenPoly*& pPolyA, CAINavMeshGenPoly*& pPolyB )
{
	// Find the specified vert.

	int cVerts = GetNumNMGVerts();
	int iVert;
	for( iVert=0; iVert < cVerts; ++iVert )
	{
		if( eVert == GetNMGVert( iVert ) )
		{
			break;
		}
	}

	// Specified vert not found in poly.

	if( iVert == cVerts )
	{
		return false;
	}

	// Search for a vert to use as the other end of the split.

	int iSplit0 = iVert;
	int iSplit1A = FindNextOriginalVert( iSplit0, 1,  1, plstSplittingVerts );
	int iSplit1B = FindNextOriginalVert( iSplit0, 1, -1, plstSplittingVerts );

	SAINAVMESHGEN_VERT* pVert = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( GetNMGVert( iSplit0 ) );
	if( !pVert ) return false;
	LTVector vSplit0 = pVert->vVert;

	pVert = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( GetNMGVert( iSplit1A ) );
	if( !pVert ) return false;
	LTVector vSplit1A = pVert->vVert;

	pVert = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( GetNMGVert( iSplit1B ) );
	if( !pVert ) return false;
	LTVector vSplit1B = pVert->vVert;

	// No valid vert found for other end of the split.

	if( ( iSplit1A == cVerts ) || ( iSplit1B == cVerts ) )
	{
		return false;
	}

	// Choose iSplit1A or iSplit1B to use as the other end of the split;
	// whichever is further.

	int iSplit1;
	if( vSplit0.DistSqr( vSplit1A ) > vSplit0.DistSqr( vSplit1B ) )
	{
		iSplit1 = iSplit1A;
	}
	else {
		iSplit1 = iSplit1B;
	}

	// Create PolyA with verts from iSplit0 to iSplit1.

	pPolyA = NAVMESH_NEW1( CAINavMeshGenPoly, m_pfnErrorCallback );

	iVert = iSplit0;
	while( iVert != iSplit1 )
	{
		pPolyA->AddNMGVert( GetNMGVert( iVert ) );
		++iVert;
		iVert = iVert % cVerts;
	}
	pPolyA->AddNMGVert( GetNMGVert( iVert ) );
	pPolyA->InitNMGPoly();
	pPolyA->SetNMGCharTypeMask( m_dwNMGCharTypeMask );
	pPolyA->SetNMGParentNavMeshID( m_eNMGParentNavMeshID );

	// If this poly has an ID, set it as the new poly's parent.
	// Otherwise, this is a new poly too, so the new poly's parent is this poly's parent.

	pPolyA->m_eNMGParentPolyID = ( m_eNMGPolyID != kNMGPoly_Invalid ) ? m_eNMGPolyID : m_eNMGParentPolyID;

	// Create PolyB with verts from iSplit1 to iSplit0.

	pPolyB = NAVMESH_NEW1( CAINavMeshGenPoly, m_pfnErrorCallback );

	iVert = iSplit1;
	while( iVert != iSplit0 )
	{
		pPolyB->AddNMGVert( GetNMGVert( iVert ) );
		++iVert;
		iVert = iVert % cVerts;
	}
	pPolyB->AddNMGVert( GetNMGVert( iVert ) );
	pPolyB->InitNMGPoly();
	pPolyB->SetNMGCharTypeMask( m_dwNMGCharTypeMask );
	pPolyB->SetNMGParentNavMeshID( m_eNMGParentNavMeshID );

	// If this poly has an ID, set it as the new poly's parent.
	// Otherwise, this is a new poly too, so the new poly's parent is this poly's parent.

	pPolyB->m_eNMGParentPolyID = ( m_eNMGPolyID != kNMGPoly_Invalid ) ? m_eNMGPolyID : m_eNMGParentPolyID;

	// Succesfully split poly.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::FindNextOriginalVert()
//              
//	PURPOSE:	Return the next original vert.
//              An original vert is a vert that was there before
//              splitting poly edges to create adjacencies.
//              
//----------------------------------------------------------------------------

int CAINavMeshGenPoly::FindNextOriginalVert( int iStart, int cSkip, int nIncr, AINAVMESHGEN_SPLITTING_VERT_LIST* plstSplittingVerts )
{
	int nSkip = 0;
	int iNext = iStart + nIncr;
	int cVerts = GetNumNMGVerts();

	AINAVMESHGEN_SPLITTING_VERT_LIST::iterator itSplit;
	ENUM_NMGVertID eVert;

	// Iterate over verts.

	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		// Keep verts in bounds.

		if( iNext < 0 )
		{
			iNext = cVerts - 1;
		}
		iNext %= cVerts;
		eVert = GetNMGVert( iNext );

		// Check if this vert is found in the list of verts added to split edges.

		for( itSplit = plstSplittingVerts->begin(); itSplit != plstSplittingVerts->end(); ++itSplit )
		{
			if( ( eVert == itSplit->eVert ) &&
				( m_eNMGPolyID == itSplit->ePoly ) )
			{
				break;
			}
		}

		// Found an original vert.

		if( itSplit == plstSplittingVerts->end() )
		{
			// Break if we have skipped enough verts.

			if( nSkip == cSkip )
			{
				break;
			}
			++nSkip;
		}

		iNext += nIncr;
	}

	return iNext;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::FindAdjacentNMGEdges()
//              
//	PURPOSE:	Return true if an edjacent edge is found between polyA and polyB.
//              Return false if polyA and polyB already share an entire edge.
//              If adjacent edge is found, pointers for edgeA and edgeB are set.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::FindAdjacentNMGEdges( CAINavMeshGenPoly* pPolyB, SAINAVMESHGEN_EDGE*& pEdgeA, SAINAVMESHGEN_EDGE*& pEdgeB )
{
	SAINAVMESHGEN_AABB aabbA, aabbB;
	ENUM_NMGVertID eVertA0, eVertA1, eVertB0, eVertB1;
	LTVector vVertA0, vVertA1, vVertB0, vVertB1;
	LTVector v0, v1, vCross;

	// Do not use AINavMeshGen::fEpsilon, because the cross product
	// requires a more restrictive epsilon value.

	float fEpsilonSqr = 0.01f * 0.01f;

	// Iterate over edges of polyA.

	int iVertA, iVertB, iNext;
	int cVertsA, cVertsB;
	cVertsA = GetNumNMGVerts();
	for( iVertA=0; iVertA < cVertsA; ++iVertA )
	{
		iNext = ( iVertA + 1 ) % cVertsA;
		eVertA0 = GetNMGVert( iVertA );
		eVertA1 = GetNMGVert( iNext );
	
		// Ignore if edge does not exist.

		pEdgeA = CAINavMeshGen::GetAINavMeshGen()->GetNMGEdge( eVertA0, eVertA1 );
		if( !pEdgeA )
		{
			continue;
		}

		// Return false if polyA and polyB already share an entire edge.

		if( ( pEdgeA->ePolyID1 == pPolyB->GetNMGPolyID() ) ||
			( pEdgeA->ePolyID2 == pPolyB->GetNMGPolyID() ) )
		{
			return false;
		}

		// Ignore edges that are shared with other polys.

		if( ( pEdgeA->ePolyID1 != kNMGPoly_Invalid ) &&
			( pEdgeA->ePolyID2 != kNMGPoly_Invalid ) )
		{
			continue;
		}

		// Create bounding box for edge.

		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertA0, &vVertA0 );
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertA1, &vVertA1 );

		aabbA.InitAABB();
		aabbA.GrowAABB( vVertA0 );
		aabbA.GrowAABB( vVertA1 );

		// Iterate over edges of polyB and compare them to edge of polyA.

		cVertsB = pPolyB->GetNumNMGVerts();
		for( iVertB=0; iVertB < cVertsB; ++iVertB )
		{
			iNext = ( iVertB + 1 ) % cVertsB;
			eVertB0 = pPolyB->GetNMGVert( iVertB );
			eVertB1 = pPolyB->GetNMGVert( iNext );
	
			// Ignore if edge does not exist.

			pEdgeB = CAINavMeshGen::GetAINavMeshGen()->GetNMGEdge( eVertB0, eVertB1 );
			if( !pEdgeB )
			{
				continue;
			}

			// Return false if polyA and polyB already share an entire edge.

			if( ( pEdgeB->ePolyID1 == m_eNMGPolyID ) ||
				( pEdgeB->ePolyID2 == m_eNMGPolyID ) )
			{
				return false;
			}

			// Ignore edges that are shared with other polys.

			if( ( pEdgeB->ePolyID1 != kNMGPoly_Invalid ) &&
				( pEdgeB->ePolyID2 != kNMGPoly_Invalid ) )
			{
				continue;
			}

			// Create bounding box for edge.

			CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertB0, &vVertB0 );
			CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertB1, &vVertB1 );

			aabbB.InitAABB();
			aabbB.GrowAABB( vVertB0 );
			aabbB.GrowAABB( vVertB1 );

			// Ignore if bounding box of edgeA and edgeB do not intersect.

			if( !aabbA.IntersectAABB( aabbB ) )
			{
				continue;
			}
			
			// Determine if edges are colinear:
			// |(x2 - x1) X (x1 - x3)| = 0

			v0 = vVertB0 - vVertA0;
			if( v0 != LTVector::GetIdentity() )
			{
				v0.Normalize();
			}

			v1 = vVertA0 - vVertA1;
			if( v1 != LTVector::GetIdentity() )
			{
				v1.Normalize();
			}
			vCross = v0.Cross( v1 );
			if( vCross.MagSqr() > fEpsilonSqr )
			{
				continue;
			}


			v0 = vVertB1 - vVertA0;
			if( v0 != LTVector::GetIdentity() )
			{
				v0.Normalize();
			}

			v1 = vVertA0 - vVertA1;
			if( v1 != LTVector::GetIdentity() )
			{
				v1.Normalize();
			}
			
			vCross = v0.Cross( v1 );
			if( vCross.MagSqr() > fEpsilonSqr )
			{
				continue;
			}

			// Check if edges overlap.

			if( pEdgeA->IntersectsNMGEdge( eVertB0 ) )
			{
				// vVertB0 is between vVertA0 and vVertA1.
				return true;
			}
			
			if( pEdgeA->IntersectsNMGEdge( eVertB1 ) )
			{
				// vVertB1 is between vVertA0 and vVertA1.
				return true;
			}
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::SplitAdjacentNMGEdge()
//              
//	PURPOSE:	Try to split an edge of PolyA with and edge of PolyB.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::SplitAdjacentNMGEdge( SAINAVMESHGEN_EDGE* pEdgeA, SAINAVMESHGEN_EDGE* pEdgeB, ENUM_NMGVertID* peSplittingVertMin, ENUM_NMGVertID* peSplittingVertMax )
{
	// Sanity check.

	if( !( peSplittingVertMin && peSplittingVertMax ) )
	{
		return;
	}

	*peSplittingVertMin = kNMGVert_Invalid;
	*peSplittingVertMax = kNMGVert_Invalid;

	// It is assumed that EdgeA and EdgeB are colinear.

	// Determine if the min vert of EdgeB falls between the min 
	// and max verts of EdgeA.

	if( pEdgeA->IntersectsNMGEdge( pEdgeB->eVertMin ) )
	{
		// Insert the min vert of EdgeB into EdgeA, splitting EdgeA into Edge0 and Edge1.

		SAINAVMESHGEN_EDGE* pEdge0 = CAINavMeshGen::GetAINavMeshGen()->AddNMGEdge( m_eNMGPolyID, pEdgeA->eVertMin, pEdgeB->eVertMin );
		SAINAVMESHGEN_EDGE* pEdge1 = CAINavMeshGen::GetAINavMeshGen()->AddNMGEdge( m_eNMGPolyID, pEdgeA->eVertMax, pEdgeB->eVertMin );
		
		*peSplittingVertMin = pEdgeB->eVertMin;
		InsertNMGVert( pEdgeB->eVertMin, pEdgeA->eVertMin, pEdgeA->eVertMax );
		CAINavMeshGen::GetAINavMeshGen()->RemoveNMGEdge( m_eNMGPolyID, pEdgeA->eVertMin, pEdgeA->eVertMax );

		// Determine if the max vert of EdgeB falls between the min 
		// and max verts of Edge0.

		if( pEdge0->IntersectsNMGEdge( pEdgeB->eVertMax ) )
		{
			// Insert the max vert of EdgeB into Edge0, splitting Edge0 into 2 parts.

			CAINavMeshGen::GetAINavMeshGen()->AddNMGEdge( m_eNMGPolyID, pEdge0->eVertMin, pEdgeB->eVertMax );
			CAINavMeshGen::GetAINavMeshGen()->AddNMGEdge( m_eNMGPolyID, pEdge0->eVertMax, pEdgeB->eVertMax );

			*peSplittingVertMax = pEdgeB->eVertMax;
			InsertNMGVert( pEdgeB->eVertMax, pEdge0->eVertMin, pEdge0->eVertMax );
			CAINavMeshGen::GetAINavMeshGen()->RemoveNMGEdge( m_eNMGPolyID, pEdge0->eVertMin, pEdge0->eVertMax );
		}

		// Determine if the max vert of EdgeB falls between the min 
		// and max verts of Edge1.

		else if( pEdge1->IntersectsNMGEdge( pEdgeB->eVertMax ) )
		{
			// Insert the max vert of EdgeB into Edge1, splitting Edge0 into 2 parts.

			CAINavMeshGen::GetAINavMeshGen()->AddNMGEdge( m_eNMGPolyID, pEdge1->eVertMin, pEdgeB->eVertMax );
			CAINavMeshGen::GetAINavMeshGen()->AddNMGEdge( m_eNMGPolyID, pEdge1->eVertMax, pEdgeB->eVertMax );

			*peSplittingVertMax = pEdgeB->eVertMax;
			InsertNMGVert( pEdgeB->eVertMax, pEdge1->eVertMin, pEdge1->eVertMax );
			CAINavMeshGen::GetAINavMeshGen()->RemoveNMGEdge( m_eNMGPolyID, pEdge1->eVertMin, pEdge1->eVertMax );
		}

		return;
	}

	// Determine if the max vert of EdgeB falls between the min 
	// and max verts of EdgeA.

	if( pEdgeA->IntersectsNMGEdge( pEdgeB->eVertMax ) )
	{
		// Insert the max vert of EdgeB into EdgeA, splitting EdgeA into 2 parts.

		CAINavMeshGen::GetAINavMeshGen()->AddNMGEdge( m_eNMGPolyID, pEdgeA->eVertMin, pEdgeB->eVertMax );
		CAINavMeshGen::GetAINavMeshGen()->AddNMGEdge( m_eNMGPolyID, pEdgeA->eVertMax, pEdgeB->eVertMax );

		*peSplittingVertMax = pEdgeB->eVertMax;
		InsertNMGVert( pEdgeB->eVertMax, pEdgeA->eVertMin, pEdgeA->eVertMax );
		CAINavMeshGen::GetAINavMeshGen()->RemoveNMGEdge( m_eNMGPolyID, pEdgeA->eVertMin, pEdgeA->eVertMax );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::GetNMGPolyNeighbor()
//              
//	PURPOSE:	Return the ID of the neignor poly at the specified 
//              index, or invalid if there is none.
//              
//----------------------------------------------------------------------------

ENUM_NMGPolyID CAINavMeshGenPoly::GetNMGPolyNeighbor( int iNeighbor ) const
{
	// Sanity check.

	int cVerts = m_lstNMGVerts.size();
	if( iNeighbor >= cVerts )
	{
		return kNMGPoly_Invalid;
	}

	// Find the verts associated with the neighbor index.

	ENUM_NMGVertID eVert0 = GetNMGVert( iNeighbor );
	ENUM_NMGVertID eVert1 = GetNMGVert( ( iNeighbor + 1 ) % cVerts );

	// Bail if the associated edge does not exist.

	SAINAVMESHGEN_EDGE* pNMGEdge = CAINavMeshGen::GetAINavMeshGen()->GetNMGEdge( eVert0, eVert1 );
	if( !pNMGEdge )
	{
		return kNMGPoly_Invalid;
	}

	// Return the poly ID that does not reference this poly.

	if( pNMGEdge->ePolyID1 == m_eNMGPolyID )
	{
		return pNMGEdge->ePolyID2;
	}
	return pNMGEdge->ePolyID1;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshPoly::ContainsPoint2D()
//              
//	PURPOSE:	Returns true if poly contains point.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::ContainsPoint2D( const LTVector& vPos )
{
	// First test the pos against the poly's bounding box.

	if( ( m_aabbNMGPolyBounds.vMin.x >= vPos.x ) ||
		( m_aabbNMGPolyBounds.vMax.x <= vPos.x ) ||
		( m_aabbNMGPolyBounds.vMin.z >= vPos.z ) ||
		( m_aabbNMGPolyBounds.vMax.z <= vPos.z ) )
	{
		return false;
	}

	LTVector ptStart, ptEnd, vEdge, vEdgeN, vMidPt, vTest;
	SAINAVMESHGEN_PLANE plnEdge;

	// Get poly normal by pool ID.

	LTVector vNMGPolyN;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( m_eNMGNormalID, &vNMGPolyN );

	// Determine if point is on the outside of any
	// of the poly's edges.

	int iNext;
	int cVerts = GetNumNMGVerts();
	for( int iVert=0; iVert < cVerts; ++iVert )
	{
		iNext = ( iVert + 1 ) % cVerts;
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iVert ), &ptStart );
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( GetNMGVert( iNext ), &ptEnd );

		// Calculate mid-point.

		vMidPt = ( ptStart + ptEnd ) * 0.5f;

		// Create test vector.

		vTest = vMidPt - vPos;
		vTest.y = 0.f;

		// Calculate the edge's normal.

		vEdge = ptEnd - ptStart;
		vEdge.y = 0.f;
		vEdge.Normalize();
		vEdgeN = vNMGPolyN.Cross( vEdge );

		// Point is on the outside of the edge, or on the edge itself.

		static float fEpsilon = 0.1f;
		if( vEdgeN.Dot( vTest ) < fEpsilon )
		{
			return false;
		}
	}

	// Point is not outside of any edge, so it must be inside.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::FindNMGOverlappingVert()
//              
//	PURPOSE:	Return true if polys intersect.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenPoly::FindNMGOverlappingVert( CAINavMeshGenPoly* pPolyB, LTVector* pvOverlap )
{
	// Sanity check.

	if( !( pPolyB && pvOverlap ) )
	{
		return false;
	}

	// Test for overlapping verts between polyB and this poly.

	ENUM_NMGVertID eVertB;
	LTVector vVertB;
	int cVertsB = pPolyB->GetNumNMGVerts();
	for( int iVertB=0; iVertB < cVertsB; ++iVertB )
	{
		eVertB = pPolyB->GetNMGVert( iVertB );
		CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertB, &vVertB );

		// Found an overlapping vert.

		if( ContainsPoint2D( vVertB ) )
		{
			*pvOverlap = vVertB;
			return true;
		}
	}

	// No overlap.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::GetNMGBoundingRadius()
//              
//	PURPOSE:	Return bounding radius of a poly.
//              
//----------------------------------------------------------------------------

float CAINavMeshGenPoly::GetNMGBoundingRadius() const
{
	return (m_aabbNMGPolyBounds.vMax - m_aabbNMGPolyBounds.vMin).Mag() / 2.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::PrintNMGVerts()
//              
//	PURPOSE:	Print verts of poly.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::PrintNMGVerts()
{
	TRACE( "Poly: %d, Parent: %d\n", m_eNMGPolyID, m_eNMGParentPolyID );

	NMGVERT_LIST::iterator itVert;
	for( itVert = m_lstNMGVerts.begin(); itVert != m_lstNMGVerts.end(); ++itVert )
	{
		TRACE( "  %d\n", *itVert );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenPoly::PrintActualVerts()
//              
//	PURPOSE:	Print actual verts of poly.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenPoly::PrintActualVerts()
{
	TRACE( "Poly: %d\n", m_eNMGPolyID );

	SAINAVMESHGEN_VERT* pVert;
	NMGVERT_LIST::iterator itVert;
	for( itVert = m_lstNMGVerts.begin(); itVert != m_lstNMGVerts.end(); ++itVert )
	{
		pVert = CAINavMeshGen::GetAINavMeshGen()->GetNMGVert( *itVert );
		if( pVert )
		{
			TRACE( "  %d: %.2f %.2f %.2f\n", *itVert, pVert->vVert.x, pVert->vVert.y, pVert->vVert.z );
		}
	}
}
