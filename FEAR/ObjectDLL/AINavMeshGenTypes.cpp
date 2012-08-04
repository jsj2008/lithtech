// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenTypes.cpp
//
// PURPOSE : AI NavMesh generator type implementations.
//
// CREATED : 11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshGenTypes.h"
#include "AINavMeshGen.h"


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//
// SAINAVMESHGEN_AABB - Axis-aligned bounding box struct.
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_AABB::InitAABB
//              
//	PURPOSE:	Initialize AABB to extremes.
//              
//----------------------------------------------------------------------------

void SAINAVMESHGEN_AABB::InitAABB() 
{
	vMin.x = vMin.y = vMin.z = FLT_MAX;
	vMax.x = vMax.y = vMax.z = -FLT_MAX; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_AABB::GrowAABB
//              
//	PURPOSE:	Grow AABB to include a vert.
//              
//----------------------------------------------------------------------------

void SAINAVMESHGEN_AABB::GrowAABB( const LTVector& vVert )
{
	if( vVert.x > vMax.x )
	{
		vMax.x = vVert.x;
	}
	if( vVert.x < vMin.x )
	{
		vMin.x = vVert.x;
	}

	if( vVert.y > vMax.y )
	{
		vMax.y = vVert.y;
	}
	if( vVert.y < vMin.y )
	{
		vMin.y = vVert.y;
	}

	if( vVert.z > vMax.z )
	{
		vMax.z = vVert.z;
	}
	if( vVert.z < vMin.z )
	{
		vMin.z = vVert.z;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_AABB::IntersectAABB
//              
//	PURPOSE:	Returns true if AABBs intersect.
//              Intersecting includes containment and adjacency.
//              
//----------------------------------------------------------------------------

bool SAINAVMESHGEN_AABB::IntersectAABB( const SAINAVMESHGEN_AABB& aabb )
{
	float fEpsilon = 0.01f;

	if( ( vMin.y > aabb.vMax.y + fEpsilon ) ||
		( vMax.y < aabb.vMin.y - fEpsilon ) ||
		( vMin.x > aabb.vMax.x + fEpsilon ) ||
		( vMax.x < aabb.vMin.x - fEpsilon ) ||
		( vMin.z > aabb.vMax.z + fEpsilon ) ||
		( vMax.z < aabb.vMin.z - fEpsilon ) )
	{
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_AABB::ContainsPoint
//              
//	PURPOSE:	Returns true if AABB contains point.
//              Containment includes borders.
//              
//----------------------------------------------------------------------------

bool SAINAVMESHGEN_AABB::ContainsPoint( const LTVector& vPos )
{
	if( ( vMin.y > vPos.y ) ||
		( vMax.y < vPos.y ) ||
		( vMin.x > vPos.x ) ||
		( vMax.x < vPos.x ) ||
		( vMin.z > vPos.z ) ||
		( vMax.z < vPos.z ) )
	{
		return false;
	}

	return true;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// AINAVMESHGEN_PLANE_STRUCT
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_PLANE::InitNMGPlane
//              
//	PURPOSE:	Initialize the plane given a normal and a vertex.
//              
//----------------------------------------------------------------------------

void SAINAVMESHGEN_PLANE::InitNMGPlane( ENUM_NMGNormalID eNormalID, const LTVector& vVert )
{
	// Find D for the plane equation.

	LTVector vNMGPlaneNormal;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( eNormalID, &vNMGPlaneNormal );
	eNMGPlaneNormal = eNormalID;

	D = -( ( vNMGPlaneNormal.x * vVert.x ) +
		   ( vNMGPlaneNormal.y * vVert.y ) +
		   ( vNMGPlaneNormal.z * vVert.z ) );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_PLANE::IsCoplanar
//              
//	PURPOSE:	Returns true if given point is coplanar.
//              
//----------------------------------------------------------------------------

bool SAINAVMESHGEN_PLANE::IsCoplanar( const LTVector& vPt )
{
	LTVector vNMGPlaneNormal;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( eNMGPlaneNormal, &vNMGPlaneNormal );

	float fVal = ( vNMGPlaneNormal.x * vPt.x ) +
				 ( vNMGPlaneNormal.y * vPt.y ) +
				 ( vNMGPlaneNormal.z * vPt.z ) + D;

	if( ( fVal <  CAINavMeshGen::kfEpsilon ) && 
		( fVal > -CAINavMeshGen::kfEpsilon ) )
	{
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_PLANE::RayIntersectNMGPlane
//              
//	PURPOSE:	Find point where ray intersects plane.
//              
//----------------------------------------------------------------------------

bool SAINAVMESHGEN_PLANE::RayIntersectNMGPlane( const LTVector& vRay0, const LTVector& vRay1, LTVector* pvIntersect )
{
	LTVector vNMGPlaneNormal;
	CAINavMeshGen::GetAINavMeshGen()->GetActualNormalFromPool( eNMGPlaneNormal, &vNMGPlaneNormal );

	LTVector vRayDir = vRay1 - vRay0;
	vRayDir.Normalize();

	// Determine if ray endpoints lie on opposite side of the plane.

	float fSign1 = ( ( vNMGPlaneNormal.x * vRay0.x ) +
					 ( vNMGPlaneNormal.y * vRay0.y ) +
				     ( vNMGPlaneNormal.z * vRay0.z ) + D );

	float fSign2 = ( ( vNMGPlaneNormal.x * vRay1.x ) +
				     ( vNMGPlaneNormal.y * vRay1.y ) +
				     ( vNMGPlaneNormal.z * vRay1.z ) + D );

	if( fSign1 * fSign2 >= 0.f )
	{
		return false;
	}

	// Determine if ray runs parallel to the plane.


	float fDenom = ( ( vNMGPlaneNormal.x * vRayDir.x ) + 
				     ( vNMGPlaneNormal.y * vRayDir.y ) + 
			  	     ( vNMGPlaneNormal.z * vRayDir.z ) );

	if( fDenom == 0.f )
	{
		return false;
	}

	// Calculate the point of intersection between the ray and the plane.

	float fNum = -( ( vNMGPlaneNormal.x * vRay0.x ) +
				    ( vNMGPlaneNormal.y * vRay0.y ) +
				    ( vNMGPlaneNormal.z * vRay0.z ) + D );

	float d = fNum / fDenom;
	*pvIntersect = ( vRay0 + ( vRayDir * d ) );

	return true;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// SAINAVMESHGEN_EDGE
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_PLANE::IntersectsNMGEdge
//              
//	PURPOSE:	Returns true if a vert intersects an edge.
//              
//----------------------------------------------------------------------------

bool SAINAVMESHGEN_EDGE::IntersectsNMGEdge( ENUM_NMGVertID eVert )
{
	LTVector vVertMin, vVertMax, vVert;

	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertMin, &vVertMin );
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertMax, &vVertMax );
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVert, &vVert );

	LTVector v0 = vVertMax - vVertMin;
	LTVector v1 = vVert - vVertMin;
	if( v0.Dot( v1 ) > 0.f )
	{
		v0 = vVertMin - vVertMax;
		v1 = vVert - vVertMax;
		if( v0.Dot( v1 ) > 0.f )
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	SAINAVMESHGEN_PLANE::PrintNMGEdge
//              
//	PURPOSE:	Prints the verts of an edge.
//              
//----------------------------------------------------------------------------

void SAINAVMESHGEN_EDGE::PrintNMGEdge()
{
	TRACE( "Edge: %d\n", eNMGEdgeID );

	LTVector vVertMin, vVertMax;
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertMin, &vVertMin );
	CAINavMeshGen::GetAINavMeshGen()->GetActualVertFromPool( eVertMax, &vVertMax );

	TRACE( "  %.2f %.2f %.2f  ->  %.2f %.2f %.2f\n", 
			vVertMin.x, vVertMin.y, vVertMin.z,
			vVertMax.x, vVertMax.y, vVertMax.z );

	TRACE( "  Polys: %d, %d\n", ePolyID1, ePolyID2 );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
