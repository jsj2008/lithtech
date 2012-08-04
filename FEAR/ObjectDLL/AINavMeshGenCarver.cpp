// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenCarver.cpp
//
// PURPOSE : AI NavMesh generator Carver class implementation.
//
// CREATED : 11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshGenCarver.h"
#include "AINavMeshGenPoly.h"
#include "AINavMeshGen.h"
#include "AINavMeshGenMacros.h"


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenCarver::Con/destructor
//              
//	PURPOSE:	Con/destructor
//              
//----------------------------------------------------------------------------

CAINavMeshGenCarver::CAINavMeshGenCarver( IGameWorldPacker::PFNERRORCALLBACK pfnErrorCallback ) :
	m_pfnErrorCallback(pfnErrorCallback)
{
	m_eNMGCarverID = kNMGCarver_Invalid;
	m_fNMGCarverHeight = 0.f;
	m_aabbNMGCarverBounds.InitAABB();
}

CAINavMeshGenCarver::~CAINavMeshGenCarver()
{
	m_lstNMGCarverVerts.resize( 0 );
	m_aabbNMGCarverBounds.InitAABB();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenCarver::InitNMGCarver
//              
//	PURPOSE:	Initialize poly.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenCarver::InitNMGCarver()
{
	// Calculate the axis-aligned bounding box.

	LTVector vVert;
	VECTOR_LIST::iterator itVert;
	for( itVert = m_lstNMGCarverVerts.begin(); itVert != m_lstNMGCarverVerts.end(); ++itVert )
	{
		vVert = *itVert;
		m_aabbNMGCarverBounds.GrowAABB( vVert );
	}
	m_aabbNMGCarverBounds.vMin.y = vVert.y - m_fNMGCarverHeight;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenCarver::AddNMGCarverVert
//              
//	PURPOSE:	Add a vert to the Carver.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenCarver::AddNMGCarverVert( const LTVector& vVert )
{
	m_lstNMGCarverVerts.push_back( vVert );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenCarver::CarveNMGPoly
//              
//	PURPOSE:	Carve hole in poly where Carver lies.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenCarver::CarveNMGPoly( CAINavMeshGenPoly* pPoly, AINAVMESHGEN_POLY_LIST* plstNewPolys, AINAVMESHGEN_POLY_LIST* plstDeletePolys, AINAVMESHGEN_POLY_LIST* plstCarvedPolys )
{
	// Sanity check.

	if( !pPoly )
	{
		return false;
	}

//	PrintCarverVerts();
//	pPoly->PrintActualVerts();

	SAINAVMESHGEN_PLANE* plnNMGPoly = pPoly->GetNMGPlane();
	
	VECTOR_LIST lstCarverIntersects;
	LTVector vCenter = LTVector( 0.f, 0.f, 0.f );

	LTVector v0, v1, vIntersect;
	VECTOR_LIST::iterator itVert;
	for( itVert = m_lstNMGCarverVerts.begin(); itVert != m_lstNMGCarverVerts.end(); ++itVert )
	{
		v0 = *itVert;
		v1 = v0;
		v1.y -= m_fNMGCarverHeight;
		if( plnNMGPoly->RayIntersectNMGPlane( v0, v1, &vIntersect ) )
		{
			lstCarverIntersects.push_back( vIntersect );
			vCenter += v0;
		}
	}

	if( lstCarverIntersects.size() < 3 )
	{
		return false;
	}


	vCenter /= (float)lstCarverIntersects.size();

	lstCarverIntersects.push_back( *( lstCarverIntersects.begin() ) );

	// Find the optimal first carving plane. This is the plane
	// which leaves the polygon with the most uncut space.

	SelectFirstCarvingPlane( pPoly, &lstCarverIntersects );

	// Get poly normal by pool ID.

	LTVector vNMGPolyN;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( pPoly->GetNMGNormalID(), &vNMGPolyN );

	CAINavMeshGenPoly* pPolyA;
	CAINavMeshGenPoly* pPolyB;
	LTVector vEdge, vEdgeN, vPolyDir;
	SAINAVMESHGEN_POLY_INTERSECT pintersect;
	VECTOR_LIST::iterator itIntersect;

	// If the entire poly is on the outside of one of the edges of the
	// carver, then the carver does not carve the polygon.

	for( itIntersect = lstCarverIntersects.begin(); itIntersect != lstCarverIntersects.end() - 1; ++itIntersect )
	{
		pintersect.v0 = *itIntersect;
		pintersect.v1 = *( itIntersect + 1 );

		if( pPoly->IsPolyOnOutsideOfRay( &pintersect, vCenter ) )
		{
			return false;
		}
	}

	// Attempt to carve the poly by each edge of the carver.

	uint32 cSplits = 0;
	uint32 cInsideEdge = 0;
	bool bDiscardCarvedPoly = true;
	for( itIntersect = lstCarverIntersects.begin(); itIntersect != lstCarverIntersects.end() - 1; ++itIntersect )
	{
		bDiscardCarvedPoly = true;

		pintersect.v0 = *itIntersect;
		pintersect.v1 = *( itIntersect + 1 );

		// Ignore polys that are entirely on one side of the carver's edge.

		if( pPoly->IsPolyOnOneSideOfRay( &pintersect ) )
		{
			if( pPoly->IsPolyOnOutsideOfRay( &pintersect, vCenter ) )
			{
				// Whole poly is outside of a cutting plane.  We don't need to test others as 
				// the carver is convex.
				bDiscardCarvedPoly = false;
				break;
			}
			else {
				++cInsideEdge;
			}
			continue;
		}

		if( pPoly->CoplanarRayIntersectPoly( &pintersect ) )
		{
			pPolyA = NULL;
			pPolyB = NULL;
			pPoly->SplitPoly( &pintersect, pPolyA, pPolyB );

			// Bail if split created any degenerate polygons.
			// This can happen if the ray is coincident with one of 
			// the original poly edges.

			if( ( pPolyA->GetNumNMGVerts() < 3 ) ||
				( pPolyB->GetNumNMGVerts() < 3 ) )
			{
				continue;
			}

			++cSplits;

//			pPoly->PrintActualVerts();
//			pPolyA->PrintActualVerts();
//			pPolyB->PrintActualVerts();

			if( pPolyA && pPolyB )
			{
				if( plstDeletePolys )
				{
					plstDeletePolys->push_back( pPoly );
				}

				vEdge = pintersect.v1 - pintersect.v0;
				vEdge.Normalize();

				// Calculate the edge's normal.

				vEdgeN = vNMGPolyN.Cross( vEdge );

				vPolyDir = pintersect.v1 - pPolyA->GetNMGPolyCenter();
				vPolyDir.Normalize();

				if( vPolyDir.Dot( vEdgeN ) < 0.f )
				{
					if( plstNewPolys )
					{
						plstNewPolys->push_back( pPolyA );
					}
					pPoly = pPolyB;
				}
				else {
					if( plstNewPolys )
					{
						plstNewPolys->push_back( pPolyB );
					}
					pPoly = pPolyA;
				}
			}
		}
	}

	// Nothing was carved.

	if( ( cSplits == 0 ) &&
		( cInsideEdge != lstCarverIntersects.size() - 1 ) )
	{
		return false;
	}

	// Add the poly that fits inside the carver.
	// The bCarve variable keeps track of whether
	// the last remaining polygon was inside of the carver, 
	// or on the outside of the last carving plane.

	if( bDiscardCarvedPoly )
	{
		if( plstCarvedPolys )
		{
			plstCarvedPolys->push_back( pPoly );
		}
	}
	else {
		if( plstNewPolys )
		{
			if( pPoly->GetNMGPolyID() == kNMGPoly_Invalid )
			{
				plstNewPolys->push_back( pPoly );
			}
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenCarver::SelectFirstCarvingPlane
//              
//	PURPOSE:	Find the carving plane which leaves the polygon with 
//              most uncut space. Reorder the list of planes to start
//              with this optimal plane.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenCarver::SelectFirstCarvingPlane( CAINavMeshGenPoly* pPoly, VECTOR_LIST* plstCarverIntersects )
{
	float fDiff = 0.0f;
	float fDiffSqr = 0.0f;

	// Determine which side of the carver's AABB leaves the most 
	// uncut space in the poly's AABB.

	float fMaxDiff = pPoly->GetAABB()->vMax.x - m_aabbNMGCarverBounds.vMax.x;
	ENUM_SIDE eSide = kRight;

	fDiff = m_aabbNMGCarverBounds.vMin.x - pPoly->GetAABB()->vMin.x; 
	if( fDiff > fMaxDiff )
	{
		fMaxDiff = fDiff;
		eSide = kLeft;
	}

	fDiff = pPoly->GetAABB()->vMax.z - m_aabbNMGCarverBounds.vMax.z;
	if( fDiff > fMaxDiff )
	{
		fMaxDiff = fDiff;
		eSide = kBottom;
	}

	fDiff = m_aabbNMGCarverBounds.vMin.z - pPoly->GetAABB()->vMin.z;
	if( fDiff > fMaxDiff )
	{
		fMaxDiff = fDiff;
		eSide = kTop;
	}

	// Find the carving plane that is closest to the optimal side 
	// of the carver's AABB.

	LTVector v0, v1, vMin;
	float fMinDiffSqr = FLT_MAX;
	VECTOR_LIST::iterator itPlane = plstCarverIntersects->end();
	VECTOR_LIST::iterator itMin = plstCarverIntersects->end();
	for( itPlane = plstCarverIntersects->begin(); itPlane != plstCarverIntersects->end() - 1; ++itPlane )
	{
		v0 = *itPlane;
		v1 = *( itPlane + 1 );

		switch( eSide )
		{
			case kRight:
				fDiff = ( v0.x - m_aabbNMGCarverBounds.vMax.x ) + ( v1.x - m_aabbNMGCarverBounds.vMax.x );
				fDiffSqr = fDiff * fDiff;
				break;

			case kLeft:
				fDiff = ( m_aabbNMGCarverBounds.vMin.x - v0.x ) + ( m_aabbNMGCarverBounds.vMin.x - v1.x );
				fDiffSqr = fDiff * fDiff;
				break;

			case kBottom:
				fDiff = ( v0.z - m_aabbNMGCarverBounds.vMax.z ) + ( v1.z - m_aabbNMGCarverBounds.vMax.z );
				fDiffSqr = fDiff * fDiff;
				break;

			case kTop:
				fDiff = ( m_aabbNMGCarverBounds.vMin.z - v0.z ) + ( m_aabbNMGCarverBounds.vMin.z - v1.z );
				fDiffSqr = fDiff * fDiff;
				break;
		}

		if( fDiffSqr < fMinDiffSqr )
		{
			fMinDiffSqr = fDiffSqr;
			itMin = itPlane;
		}
	}

	// This shouldn't happen unless something went invalid.  

	if (plstCarverIntersects->end() == itMin)
	{
		NAVMESH_ERROR("CAINavMeshGenCarver::SelectFirstCarvingPlane : No valid carving plane found.");
		return;
	}

	// Reorder the list of plane verts to start with the optimal plane.

	vMin = *itMin;
	plstCarverIntersects->pop_back();
	itPlane = plstCarverIntersects->begin();
	while( *itPlane != vMin )
	{
		plstCarverIntersects->push_back( *itPlane );
		itPlane = plstCarverIntersects->erase( itPlane );
	}
	plstCarverIntersects->push_back( *( plstCarverIntersects->begin() ) );
}



//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenCarver::PrintCarverVerts
//              
//	PURPOSE:	Print vertex coordinates for debugging.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenCarver::PrintCarverVerts()
{
	TRACE( "Carver: %d\n", m_eNMGCarverID );

	LTVector vVert;
	VECTOR_LIST::iterator itVert;
	for( itVert = m_lstNMGCarverVerts.begin(); itVert != m_lstNMGCarverVerts.end(); ++itVert )
	{
		vVert = *itVert;
		TRACE( "  %.2f %.2f %.2f\n", vVert.x, vVert.y, vVert.z );
	}
}

//----------------------------------------------------------------------------
