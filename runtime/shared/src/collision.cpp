// ------------------------------------------------------------------ //
//
//	FILE	  : collision.cpp
//
//	PURPOSE	  : Implements physics-related stuff in the ServerMgr.
//
//	CREATED	  : January 12 1996
//
//	COPYRIGHT : Monolith Productions 1996 All Rights Reserved
//
// ------------------------------------------------------------------ //

#include "bdefs.h"

#include "de_world.h"
#include "collision.h"
#include "de_objects.h"
#include "impl_common.h"
#include "intersect_line.h"
#include "moveobject.h"
#include "ltsysoptim.h"
#include "world_blocker_data.h"
#include "world_blocker_math.h"
#include "iltphysics.h"

#ifdef _WIN32
#define isnan _isnan
#endif
/*
	BASIC COLLISION ALGORITHM

	Make solid box bounding the movement.

	Go down the tree...
		Quick maximum sphere test here to quickly go into one side?
		If the points straddle the root plane
			Clip the root plane's POLY into the box's 6 planes
				If any portion is inside
					An intersection happened .. push away from this plane.
				else
					Recurse into both sides
		else
			Recurse into the side all the points are on
*/

#define MAX_PHYSICS_ITERATIONS				40
#define NUM_BOX_POINTS						8
#define MAX_INTERSECT_PUSHBACK_ITERATIONS	50

#define EXTRA_PENETRATION_ADD				0.05f


// Interface to the blocker data
static IWorldBlockerData *g_iWorldBlockerData = LTNULL;
define_holder(IWorldBlockerData, g_iWorldBlockerData);

// ------------------------------------------------------------------ //
// Structures.
// ------------------------------------------------------------------ //
struct ClassifyPoints
{
	const LTPlane*			m_pPlane;
	const LTVector*			m_pPoints;
	const PhysicsSphere*	m_pSphere;
	float					min, max;
	bool*					m_bCalcMinMax;
	LTVector				m_MinPos, m_MaxPos;
};


typedef int (*ClassifyFn)(ClassifyPoints *pClassify);


//---------------------------------------------------------------------------//
static int ReallyClassifyPointsGeneric
(
	ClassifyPoints *pClassify
)
{
	const LTVector* pCur = pClassify->m_pPoints;
	int32 count = 8;

		pClassify->min = (float)MAX_CREAL;
		pClassify->max = (float)-MAX_CREAL;

		while(count--)
		{
			const float Dot = pClassify->m_pPlane->DistTo(*pCur);

			if(Dot < pClassify->min)
				pClassify->min = Dot;

			if(Dot > pClassify->max)
				pClassify->max = Dot;
		
			++pCur;
		}

	// Changed this so that if the min is zero, the points are flush, and it is
	// considered to be on front side rather than backside...
	// return (int)(min < 0.0f && max > 0.0f) ? Intersect : (int)(min > 0.0f);
	return (pClassify->min < -0.001f && pClassify->max > -0.001f) ? Intersect : (int)(pClassify->min > -0.001f);
}

static int ClassifyPointsGeneric
(
	ClassifyPoints *pClassify
)
{
	float Dot;

	Dot = pClassify->m_pPlane->DistTo(pClassify->m_pSphere->m_Center);
	if(Dot > pClassify->m_pSphere->m_Radius)
	{
		*pClassify->m_bCalcMinMax = false;
		return FrontSide;
	}
	else if(Dot < -pClassify->m_pSphere->m_Radius)
	{
		*pClassify->m_bCalcMinMax = false;
		return BackSide;
	}
	else
	{
		return ReallyClassifyPointsGeneric(pClassify);
	}
}

int ClassifyHighX(ClassifyPoints *pClassify)
{
	pClassify->min = (pClassify->m_MinPos.x) - pClassify->m_pPlane->m_Dist;
	pClassify->max = (pClassify->m_MaxPos.x) - pClassify->m_pPlane->m_Dist;
	return (int)(pClassify->min <= -0.001f && pClassify->max > -0.001f) ? Intersect : (int)(pClassify->min > -0.001f);
}
int ClassifyLowX(ClassifyPoints *pClassify)
{
	pClassify->min = -(pClassify->m_MaxPos.x) - pClassify->m_pPlane->m_Dist;
	pClassify->max = -(pClassify->m_MinPos.x) - pClassify->m_pPlane->m_Dist;
	return (int)(pClassify->min <= -0.001f && pClassify->max > -0.001f) ? Intersect : (int)(pClassify->min > -0.001f);
}
int ClassifyHighY(ClassifyPoints *pClassify)
{
	pClassify->min = (pClassify->m_MinPos.y) - pClassify->m_pPlane->m_Dist;
	pClassify->max = (pClassify->m_MaxPos.y) - pClassify->m_pPlane->m_Dist;
	return (int)(pClassify->min <= -0.001f && pClassify->max > -0.001f) ? Intersect : (int)(pClassify->min > -0.001f);
}
int ClassifyLowY(ClassifyPoints *pClassify)
{
	pClassify->min = -(pClassify->m_MaxPos.y) - pClassify->m_pPlane->m_Dist;
	pClassify->max = -(pClassify->m_MinPos.y) - pClassify->m_pPlane->m_Dist;
	return (int)(pClassify->min <= -0.001f && pClassify->max > -0.001f) ? Intersect : (int)(pClassify->min > -0.001f);
}
int ClassifyHighZ(ClassifyPoints *pClassify)
{
	pClassify->min = (pClassify->m_MinPos.z) - pClassify->m_pPlane->m_Dist;
	pClassify->max = (pClassify->m_MaxPos.z) - pClassify->m_pPlane->m_Dist;
	return (int)(pClassify->min <= -0.001f && pClassify->max > -0.001f) ? Intersect : (int)(pClassify->min > -0.001f);
}
int ClassifyLowZ(ClassifyPoints *pClassify)
{
	pClassify->min = -(pClassify->m_MaxPos.z) - pClassify->m_pPlane->m_Dist;
	pClassify->max = -(pClassify->m_MinPos.z) - pClassify->m_pPlane->m_Dist;
	return (int)(pClassify->min <= -0.001f && pClassify->max > -0.001f) ? Intersect : (int)(pClassify->min > -0.001f);
}

ClassifyFn g_ClassifyFns[] =
{
	ClassifyHighX, ClassifyLowX,
	ClassifyHighY, ClassifyLowY,
	ClassifyHighZ, ClassifyLowZ,
	ClassifyPointsGeneric
};



//--------------------------------------------------------------------------//
// Returns false if there isn't enough movement to generate the box info.
static bool SetupBox
(
	const LTVector&		P0,
	const LTVector&		P1,
	const LTVector&		dims,
	const LTVector&		offset,
	const float			radius,
	PhysicsSphere&		start_sphere,
	PhysicsSphere&		end_sphere,
	PhysicsSphere&		whole_sphere,
	AABB&				box,
	LTVector			move_pts[2][NUM_BOX_POINTS]
)
{
	const LTVector v = P1 - P0;//displacement

	if( v.Dot(v) < 0.0001f )
	{
		return false;
	}

	LTVector offsetPos[2];

	offsetPos[0] = P0 + offset;
	offsetPos[1] = P1 + offset;

	// Setup the box points.
	for( int32 i=0; i < 2; i++)
	{
		LTVector* pBoxPts = move_pts[i];
		LTVector& p = offsetPos[i];

		pBoxPts[0].Init( p.x - dims.x, p.y + dims.y, p.z - dims.z );
		pBoxPts[1].Init( p.x - dims.x, p.y + dims.y, p.z + dims.z );
		pBoxPts[2].Init( p.x + dims.x, p.y + dims.y, p.z + dims.z );
		pBoxPts[3].Init( p.x + dims.x, p.y + dims.y, p.z - dims.z );

		pBoxPts[4].Init( p.x - dims.x, p.y - dims.y, p.z - dims.z );
		pBoxPts[5].Init( p.x - dims.x, p.y - dims.y, p.z + dims.z );
		pBoxPts[6].Init( p.x + dims.x, p.y - dims.y, p.z + dims.z );
		pBoxPts[7].Init( p.x + dims.x, p.y - dims.y, p.z - dims.z );
	}

	// Setup the spheres.
	start_sphere.m_Center	= offsetPos[0];
	end_sphere.m_Center		= offsetPos[1];
	start_sphere.m_Radius	= end_sphere.m_Radius = radius;

	// The whole sphere encloses the other two (as small as possible tho!  the size of this sphere
	// directly relates to how fast the physics are).
	whole_sphere.m_Center = start_sphere.m_Center + v * 0.5f;
	whole_sphere.m_Radius = radius + v.Mag();

	VEC_MIN( box.Min, offsetPos[0], offsetPos[1] );
	VEC_MAX( box.Max, offsetPos[0], offsetPos[1] );

	box.Min -= dims;
	box.Max += dims;

	return true;
}


//--------------------------------------------------------------------------//
bool AABB::intersects_line_segment
(
	const LTVector&	l0,		//first point on segment
	const LTVector&	l1		//second point on segment
) const
{
	//ALGORITHM:  Use the separating axis theorem to
	//see if the line segment and the box intersect. A
	//line segment is a degenerate OBB.
	//NOTE:  the multiplies by 0.5 cancel out

	const LTVector l = l1 - l0;//unnormalized direction
	const LTVector T = (l0 + l1) - (this->Max + this->Min);//translation
	const LTVector E = this->Max - this->Min;//extents

		//do any of the principal axes form a separating axis?
		if( fabsf(T.x) > fabsf(l.x) + E.x )
			return false;

		if( fabsf(T.y) > fabsf(l.y) + E.y )
			return false;

		if( fabsf(T.z) > fabsf(l.z) + E.z )
			return false;

		//does l.Cross(x) form a separating axis
		if( fabsf(T.y*l.z - T.z*l.y) > E.y*fabsf(l.z) + E.z*fabsf(l.y) )
			return false;

		//does l.Cross(y) form a separating axis
		if( fabsf(T.z*l.x - T.x*l.z) > E.x*fabsf(l.z) + E.z*fabsf(l.x) )
			return false;

		//does l.Cross(z) form a separating axis
		if( fabsf(T.x*l.y - T.y*l.x) > E.x*fabsf(l.y) + E.y*fabsf(l.x) )
			return false;

	return true;
}

//--------------------------------------------------------------------------//
inline void QueueEdgeIfIntersectsX(float fX, float fY, float fZ, float fX2, float fDot0, float fDot1, LTVector* pOutList, uint32& nOutIndex)
{
	//if they are opposite in sign, then they're
	//on different sides of the plane
	if( fDot0 * fDot1 < 0 )
	{
		//interpolate to find the point of intersection
		pOutList[nOutIndex].x = (fDot0 * fX2 - fDot1 * fX) / (fDot0 - fDot1);
		pOutList[nOutIndex].y = fY;
		pOutList[nOutIndex].z = fZ;

		//add one to our count of intersections
		nOutIndex++;
	}
}

inline void QueueEdgeIfIntersectsY(float fX, float fY, float fZ, float fY2, float fDot0, float fDot1, LTVector* pOutList, uint32& nOutIndex)
{
	//if they are opposite in sign, then they're
	//on different sides of the plane
	if( fDot0 * fDot1 < 0 )
	{
		//interpolate to find the point of intersection
		pOutList[nOutIndex].x = fX;
		pOutList[nOutIndex].y = (fDot0 * fY2 - fDot1 * fY) / (fDot0 - fDot1);
		pOutList[nOutIndex].z = fZ;

		//add one to our count of intersections
		nOutIndex++;
	}
}

inline void QueueEdgeIfIntersectsZ(float fX, float fY, float fZ, float fZ2, float fDot0, float fDot1, LTVector* pOutList, uint32& nOutIndex)
{
	//if they are opposite in sign, then they're
	//on different sides of the plane
	if( fDot0 * fDot1 < 0 )
	{
		//interpolate to find the point of intersection
		pOutList[nOutIndex].x = fX;
		pOutList[nOutIndex].y = fY;
		pOutList[nOutIndex].z = (fDot0 * fZ2 - fDot1 * fZ) / (fDot0 - fDot1);

		//add one to our count of intersections
		nOutIndex++;
	}
}

//--------------------------------------------------------------------------//
inline bool AABBIntersectsLineSegment
(
	const LTVector&	l0,			//first point on segment
	const LTVector&	l1,			//second point on segment
	const LTVector& E,			//full extents of the bounding box
	const LTVector& vTwoCenter	//twice the center of the bounding box
) 
{
	//ALGORITHM:  Use the separating axis theorem to
	//see if the line segment and the box intersect. A
	//line segment is a degenerate OBB.
	//NOTE:  the multiplies by 0.5 cancel out

	//do any of the principal axes form a separating axis?

	float fTx = l0.x + l1.x - vTwoCenter.x;
	float fLx = l1.x - l0.x;
	if( fabsf(fTx) > fabsf(fLx) + E.x )
		return false;

	float fTy = l0.y + l1.y - vTwoCenter.y;
	float fLy = l1.y - l0.y;
	if( fabsf(fTy) > fabsf(fLy) + E.y )
		return false;

	float fTz = l0.z + l1.z - vTwoCenter.z;
	float fLz = l1.z - l0.z;
	if( fabsf(fTz) > fabsf(fLz) + E.z )
		return false;

	//does l.Cross(x) form a separating axis
	if( fabsf(fTy * fLz - fTz * fLy) > E.y * fabsf(fLz) + E.z * fabsf(fLy) )
		return false;

	//does l.Cross(y) form a separating axis
	if( fabsf(fTz * fLx - fTx * fLz) > E.x * fabsf(fLz) + E.z * fabsf(fLx) )
		return false;

	//does l.Cross(z) form a separating axis
	if( fabsf(fTx * fLy - fTy * fLx) > E.x * fabsf(fLy) + E.y * fabsf(fLx) )
		return false;

	return true;
}

//--------------------------------------------------------------------------//
bool WorldPolyIntersectsAABB
(
	const PhysicsSphere&	whole_sphere,	//motion encompassing sphere
	const WorldPoly&		poly,			//world polygon
	const AABB&				box				//axis-aligned bounding box
)
{
	// Nonsolid and non-polyvert surfaces don't get checked
	if( !(poly.GetSurface()->m_Flags & SURF_SOLID) )
		return false;

	//check to see if poly is even close to whole_sphere
	const float r_sum = whole_sphere.m_Radius + poly.GetRadius();
	const LTVector v = whole_sphere.m_Center - poly.GetCenter();

	if( v.Dot(v) > (r_sum*r_sum) )
		return false;

	//NOTE:  Building an AABB from the poly and doing a
	//box-box check actually slows the algorithm down.

	const LTVector& vPolyVert = poly.GetVertex(0);
	const LTVector& n = poly.GetPlane()->m_Normal;

	//the full dimensions of the box
	const LTVector vExtents = box.Max - box.Min;

	//two times the center of the box (Max + Min)
	const LTVector vTwoCenter	= box.Max + box.Min;
	const LTVector vCenter		= vTwoCenter * 0.5f;

	//if the box doesn't intersect the
	//plane of the triangle, then they
	//can't intersect
	{
		//distance from C to plane
		const float d = fabsf( n.Dot(vCenter - vPolyVert) );
		//radial extent of box along n
		const float r = 0.5f * (vExtents.x * fabsf(n.x) + vExtents.y * fabsf(n.y) + vExtents.z * fabsf(n.z));

		if( r < d )
			return false;
	}

	//now run through and test if any vertices are contained within AABB or if any polygon segments
	//intersect it
	uint32 nCurrVert;
	uint32 nNumVerts = poly.GetNumVertices();
	uint32 nPrevVert = nNumVerts - 1;

	for(nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		const LTVector& v0 = poly.GetVertex(nCurrVert);
		if(box.intersects(v0))
		{
			//the box contains a vertex, intersection
			return true;
		}

		//now see if this edge hits the box
		const LTVector& v1 = poly.GetVertex(nPrevVert);
		if(AABBIntersectsLineSegment(v0, v1, vExtents, vTwoCenter))
		{
			//hits the edge
			return true;
		}
		nPrevVert = nCurrVert;
	}

	//alright, now we need to detect if any of the lines of the box intersect the polygon

	//we need to run through and setup the box vertices
	const LTVector& vMin = box.Min;
	const LTVector& vMax = box.Max;

	//now setup the dot products for each vertex
	float fPlaneDist	= vPolyVert.Dot(n);

	float fNMinX		= n.x * vMin.x;
	float fNMaxX		= n.x * vMax.x;
	float fNMinY		= n.y * vMin.y;
	float fNMaxY		= n.y * vMax.y;
	float fNMinZPlane	= n.z * vMin.z - fPlaneDist;
	float fNMaxZPlane	= n.z * vMax.z - fPlaneDist;

	//note that the layout of the points form the top and bottom boxes wrapping around

	//the plane distance for each box vertex
	float  fBoxVertDists[8];
	fBoxVertDists[0]	= fNMinX + fNMinY + fNMinZPlane;
	fBoxVertDists[1]	= fNMinX + fNMinY + fNMaxZPlane;
	fBoxVertDists[2]	= fNMaxX + fNMinY + fNMaxZPlane;
	fBoxVertDists[3]	= fNMaxX + fNMinY + fNMinZPlane;

	fBoxVertDists[4]	= fNMinX + fNMaxY + fNMinZPlane;
	fBoxVertDists[5]	= fNMinX + fNMaxY + fNMaxZPlane;
	fBoxVertDists[6]	= fNMaxX + fNMaxY + fNMaxZPlane;
	fBoxVertDists[7]	= fNMaxX + fNMaxY + fNMinZPlane;

	//now run through and queue up any edges that cross the plane

	//the number of intersections we are testing for
	uint32 nNumIntersections = 0;

	//the intersection points of each segment
	static LTVector vIntersections[12];

	//Top square
	QueueEdgeIfIntersectsZ(vMin.x, vMin.y, vMin.z, vMax.z, fBoxVertDists[0], fBoxVertDists[1], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsX(vMin.x, vMin.y, vMax.z, vMax.x, fBoxVertDists[1], fBoxVertDists[2], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsZ(vMax.x, vMin.y, vMax.z, vMin.z, fBoxVertDists[2], fBoxVertDists[3], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsX(vMax.x, vMin.y, vMin.z, vMin.x, fBoxVertDists[3], fBoxVertDists[0], vIntersections, nNumIntersections);

	//bottom square
	QueueEdgeIfIntersectsZ(vMin.x, vMax.y, vMin.z, vMax.z, fBoxVertDists[4], fBoxVertDists[5], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsX(vMin.x, vMax.y, vMax.z, vMax.x, fBoxVertDists[5], fBoxVertDists[6], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsZ(vMax.x, vMax.y, vMax.z, vMin.z, fBoxVertDists[6], fBoxVertDists[7], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsX(vMax.x, vMax.y, vMin.z, vMin.x, fBoxVertDists[7], fBoxVertDists[0], vIntersections, nNumIntersections);

	//connecting edges
	QueueEdgeIfIntersectsY(vMin.x, vMin.y, vMin.z, vMax.y, fBoxVertDists[0], fBoxVertDists[4], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsY(vMin.x, vMin.y, vMax.z, vMax.y, fBoxVertDists[1], fBoxVertDists[5], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsY(vMax.x, vMin.y, vMin.z, vMax.y, fBoxVertDists[2], fBoxVertDists[6], vIntersections, nNumIntersections);
	QueueEdgeIfIntersectsY(vMax.x, vMin.y, vMax.z, vMax.y, fBoxVertDists[3], fBoxVertDists[7], vIntersections, nNumIntersections);

	//see if there are any intersections
	if(nNumIntersections == 0)
	{
		//no intersection
		return false;
	}

	//now run through and weed out intersections outside the polygon
	LTVector vEdgeNormal;
	float fEdgePlaneDist;
	uint32 nCurrIntersection;

	nPrevVert = nNumVerts - 1;
	for(nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{

		const LTVector& v0 = poly.GetVertex(nCurrVert);
		const LTVector& v1 = poly.GetVertex(nPrevVert);

		//build up the edge plane
		vEdgeNormal = n.Cross(v0 - v1);

		//the plane distance...
		fEdgePlaneDist = vEdgeNormal.Dot(v0);

		//see what side each intersection is on
		for(nCurrIntersection = 0; nCurrIntersection < nNumIntersections;)
		{
			if(vEdgeNormal.Dot(vIntersections[nCurrIntersection]) - fEdgePlaneDist > 0.0f)
			{
				//see if we will be empty
				if(nNumIntersections == 1)
					return false;

				//this vertex is outside, pull it from the list
				vIntersections[nCurrIntersection] = vIntersections[nNumIntersections - 1];
				nNumIntersections--;
			}
			else
			{
				nCurrIntersection++;
			}
		}

		nPrevVert = nCurrVert;
	}

	//we had intersections left, therefore we intersect
	return true;
}

//--------------------------------------------------------------------------//
static void MoveToFrontside
(
	const LTVector&	P0,
	const Node*		pRoot,
	CollideInfo*	pInfo,
	ClassifyPoints*	pCP,
	CollideRequest&	request,
	LTVector&		P1
)
{
	const LTPlane *pRootPlane = pRoot->GetPlane();

		if( pRootPlane->m_Normal.y > 0.01f )
		{
			if( !pInfo->m_pStandingOn || 
				( pInfo->m_pStandingOn->GetPlane()->m_Normal.y > pRootPlane->m_Normal.y ))
			{
				pInfo->m_pStandingOn = pRoot;
			}
		}

		DoObjectCollisionResponse(	request, pInfo, pRoot );
		
		++pInfo->m_nHits;

		if( !(*pCP[0].m_bCalcMinMax) )
		{
			ReallyClassifyPointsGeneric( &pCP[1] );
		}

		if( request.m_bSlide )
		{
			P1 += pRootPlane->m_Normal * (0.1f - pCP[1].min);
		}
		else
		{
			LTVector	v = P1 - P0;//displacement
			// Get the amount the start point is above the plane.  This works because abs( dot1 ) + abs( dot2 ) = 
			// g_vDeltaPos.Dot.Normal.  This Dot product is going to be negative because the object is known to be
			// going into the plane.  Both dot2 and dot1 will be negative then.
			const float dot2 = pCP[1].min;
			const float dot1 = dot2 - pRootPlane->m_Normal.Dot( v );

				P1 = P0 + v * ( -dot1 / ( dot2 - dot1 ));
		}
}


//--------------------------------------------------------------------------//
static bool ClipBoxIntoTree2
(
	const LTVector&	offset,
	const float		radius,
	CollideInfo*	pInfo,
	const bool		bPushOffIntersecting,
	CollideRequest& request,
	LTVector&		P0,
	LTVector&		P1
)
{
	const Node *stack[1024];
	const Node **pStackPos;
	bool bCalcMinMax;
	float Dot;
	int state1, state2;
	const Node *pRoot;
	ClassifyPoints cp[2];
	LTVector vTemp, moveDir, vBasePt;
	uint32 count;
	LTVector vOriginalPos;
	PhysicsSphere	start_sphere, end_sphere, whole_sphere;
	AABB			box;
	LTVector		move_pts[2][NUM_BOX_POINTS];

	//swept aabb
	SetupBox(	P0, P1,
				request.m_Dims,
				offset,
				radius,
				start_sphere,
				end_sphere,
				whole_sphere,
				box,
				move_pts );

	cp[0].m_pPoints = move_pts[0];
	cp[1].m_pPoints = move_pts[1];
	cp[0].m_bCalcMinMax = cp[1].m_bCalcMinMax = &bCalcMinMax;
	cp[0].m_pSphere = &start_sphere;
	cp[1].m_pSphere = &end_sphere;

	vBasePt = P0 + offset;

	cp[0].m_MinPos = vBasePt - request.m_Dims;
	cp[0].m_MaxPos = vBasePt + request.m_Dims;

	vBasePt = P1 + offset;

	cp[1].m_MinPos = vBasePt - request.m_Dims;
	cp[1].m_MaxPos = vBasePt + request.m_Dims;

	pStackPos = stack;
	pRoot = request.m_pWorld->GetRootNode();
	
	for(;;)
	{
		if( (pRoot==NODE_IN) || (pRoot==NODE_OUT) )
		{
			if( pStackPos == stack )
			{
				return true;
			}

			--pStackPos;
			pRoot = *pStackPos;
		}

		// Do the fast test on the whole movement area.
		Dot = pRoot->GetPlane()->DistTo(whole_sphere.m_Center);
		if(Dot > whole_sphere.m_Radius)
		{
			pRoot = pRoot->m_Sides[FrontSide];
			continue;
		}
		else if(Dot < -whole_sphere.m_Radius)
		{
			pRoot = pRoot->m_Sides[BackSide];
			continue;
		}

		// Classify where the starting and ending areas are.
		bCalcMinMax = true;
		cp[0].m_pPlane = cp[1].m_pPlane = pRoot->GetPlane();
		state1 = g_ClassifyFns[ pRoot->m_PlaneType ]( &cp[0] );
		state2 = g_ClassifyFns[ pRoot->m_PlaneType ]( &cp[1] );

		// Do something based on the values.
		if(state1 == state2)
		{
			if( state1 == Intersect )
			{
				// Have we run into it from the side?
				if( WorldPolyIntersectsAABB( whole_sphere, *pRoot->m_pPoly, box ) )
				{
					if( bPushOffIntersecting )
					{
						// Just move them backwards.  Picking other directions always leads
						// to problems like the player going to their knees or floating
						// up in the air.
						moveDir = P0 - P1;
						moveDir.Norm();

						DoObjectCollisionResponse( request, pInfo, pRoot );
						++pInfo->m_nHits;
						
						// We have to change P0 while moving back so we can actually
						// get into a nonintersecting position.
						vOriginalPos = P0;

						for( count=0; count < MAX_INTERSECT_PUSHBACK_ITERATIONS; count++ )
						{
							if( WorldPolyIntersectsAABB( whole_sphere, *pRoot->m_pPoly, box ) )
							{
								P0 += moveDir;
								P1 += moveDir;

								const LTVector v = P1 - P0;//displacement

								if( v.Dot(v) < 0.0001f )
								{
									return false;
								}
							}
							else
							{
								break;
							}
						}

						P0 = vOriginalPos;

						// If it never got out, just move back to the starting position.
						if(count == MAX_INTERSECT_PUSHBACK_ITERATIONS)
						{
							VEC_COPY(P1, P0);
							return false;
						}

						// Reset the classify points stuff that doesn't use a pointer.
						vBasePt = P0 + offset;
						cp[0].m_MinPos = vBasePt - request.m_Dims;
						cp[0].m_MaxPos = vBasePt + request.m_Dims;
						vBasePt = P1 + offset;
						cp[1].m_MinPos = vBasePt - request.m_Dims;
						cp[1].m_MaxPos = vBasePt + request.m_Dims;

					}
					else
					{
						// Need to do this here, cuz DoObjectCollisionResponse not always called...
						request.m_pCollisionInfo->m_Plane = *pRoot->GetPlane();
						request.m_pCollisionInfo->m_hObject = request.m_pWorldObj;
						request.m_pCollisionInfo->m_hPoly = 
							request.m_pWorld->MakeHPoly(pRoot);

						// Make sure it loops around again.
						++pInfo->m_nHits;
					}
				}

				state1 = pRoot->GetPlane()->DistTo(P0) > -0.001f;
				if((pRoot->m_Sides[!state1] != NODE_IN) && (pRoot->m_Sides[!state1] != NODE_OUT))
					*(pStackPos++) = pRoot->m_Sides[!state1];
				pRoot = pRoot->m_Sides[state1];
			}
			else
			{
				pRoot = pRoot->m_Sides[state1];
			}
		}
		else
		{
			if( (state1 == FrontSide) && WorldPolyIntersectsAABB( whole_sphere, *pRoot->m_pPoly, box ) )
			{
				MoveToFrontside( P0, pRoot, pInfo, cp, request, P1 );

				const LTVector v = P1 - P0;//displacement

				if( v.Dot(v) < 0.0001f )
				{
					return false;
				}

				// Reset the classify points stuff that doesn't use a pointer.
				vBasePt = P0 + offset;
				cp[0].m_MinPos = vBasePt - request.m_Dims;
				cp[0].m_MaxPos = vBasePt + request.m_Dims;
				vBasePt = P1 + offset;
				cp[1].m_MinPos = vBasePt - request.m_Dims;
				cp[1].m_MaxPos = vBasePt + request.m_Dims;

				pRoot = pRoot->m_Sides[FrontSide];
			}
			else
			{
				state1 = pRoot->GetPlane()->DistTo(P0) > -0.001f;
				if((pRoot->m_Sides[!state1] != NODE_IN) && (pRoot->m_Sides[!state1] != NODE_OUT))
					*(pStackPos++) = pRoot->m_Sides[!state1];
				pRoot = pRoot->m_Sides[state1];
			}
		}
	}

	return true;
}


//--------------------------------------------------------------------------//
class CMovingCylinder
{
public:
	// Re-calculates the non-provided information
	void Recalc();
	// Get which side of the plane the cylinder is on
	PolySide GetPlaneSide(const LTPlane *pPlane, LTVector &vCenter, bool bEnd = false, bool bOptimize = true);
	// Returns true if the polygon intersects with the moving cylinder
	// Note : This is only valid if GetPlaneSide returns Intersect
	bool CollideWith(WorldPoly *pPoly, const Node *pNode);
	// Takes the appropriate action based on a collision
	// Note : This will only operate correctly if CollideWith has been called first
	void HandleCollision
	(
		CollideRequest&	request,
		const Node*		pNode,
		CollideInfo*	pInfo
	);

	// Gets the height section of the Y value in relation to the end location
	enum EHeightSection {
		HEIGHT_MIDDLE = 0,
		HEIGHT_ABOVE = 1,
		HEIGHT_BELOW = 2 
	};
	EHeightSection GetHeightSection(float fYValue);
public:
	// ** Provided Members

	// Starting and ending positions of the cylinder
	LTVector m_vStart, m_vEnd;
	LTVector m_vRealEnd; // The final non-segmented ending position of the cylinder
	// Size of the cylinder  (m_fHeight is 1/2 total cylinder height)
	float m_fRadius, m_fHeight;
	// Perform stair-stepping
	bool m_bStairStep;

	// ** Calculated Members

	// Radius of a sphere around the cylinder
	float m_fSphere;

	// Movement vector
	LTVector m_vMovement, m_vMovementDir;
	// Velocity
	float m_fVelocity;

	// Center of the movement sphere
	LTVector m_vMoveMid;
	// Radius of the movement sphere
	float m_fMoveSphere;

	// Squared member functions for optimization
	float m_fRadiusSqr, m_fSphereSqr, m_fMoveSphereSqr;
	float m_fVelocitySqr;

	// Top/bottom of the cylinder in terms of movement
	float m_fMoveTop, m_fMoveBottom;

	// ** Collision information

	// Distance from m_vEnd to the plane
	float m_fDistToPlane;
	// Amount of the intrusion of the plane
	float m_fPlaneIntrusion;

	// Furthest intruding point at m_vEnd
	LTVector m_vClosestPt;
	// Direction of intrusion
	LTVector m_vClosestDir;
	// Distance of intrusion
	float m_fClosestDist;
	// Closest node
	const Node *m_pClosestNode;
};

//--------------------------------------------------------------------------//
void CMovingCylinder::Recalc()
{
	m_fRadiusSqr = m_fRadius * m_fRadius;
	m_fSphereSqr = m_fHeight * m_fHeight + m_fRadiusSqr;
	m_fSphere = ltsqrtf(m_fSphereSqr);

	m_vMovement = (m_vEnd - m_vStart);
	m_fVelocitySqr = m_vMovement.Dot(m_vMovement);
	m_fVelocity = ltsqrtf(m_fVelocitySqr);
	if (m_fVelocity)
		m_vMovementDir = m_vMovement * (1 / m_fVelocity);
	else
		m_vMovementDir.Init(0.0f, 0.0f, 0.0f);
	m_vMoveMid = (m_vStart + m_vEnd) * 0.5f;
	m_fMoveSphere = m_fSphere + (m_fVelocity * 0.5f);
	m_fMoveSphereSqr = m_fMoveSphere * m_fMoveSphere;

	m_fMoveTop = MAX(m_vStart.y, m_vEnd.y) + m_fHeight;
	m_fMoveBottom = MIN(m_vStart.y, m_vEnd.y) - m_fHeight;

	m_fClosestDist = 0;
	m_pClosestNode = LTNULL;
}


//--------------------------------------------------------------------------//
// Note : This is the general-case intersection method..  A specific case for horizontal
// planes would make this a lot faster
PolySide CMovingCylinder::GetPlaneSide(const LTPlane *pPlane, LTVector &vCenter, bool bEnd, bool bOptimize)
{
	float fDistToPlane = pPlane->DistTo(vCenter);

	if (bEnd)
		m_fDistToPlane = fDistToPlane;

	if (bOptimize)
	{
		// Handle the obvious case
		if (fDistToPlane >= m_fSphere)
			return FrontSide;
		else if (fDistToPlane <= -m_fSphere)
			return BackSide;

		// Handle the vertical plane case
		if (pPlane->m_Normal.y == 0.0)
		{
			if (fDistToPlane >= m_fRadius)
				return FrontSide;
			else if (fDistToPlane <= -m_fRadius)
				return BackSide;
			else
			{
				if (bEnd)
					m_fPlaneIntrusion = m_fRadius - fDistToPlane;
				return Intersect;
			}
		}
		// Handle the horizontal plane case
		else if ((pPlane->m_Normal.x == 0.0) && (pPlane->m_Normal.z == 0.0))
		{
			if (fDistToPlane >= m_fHeight)
				return FrontSide;
			else if (fDistToPlane <= -m_fHeight)
				return BackSide;
			else
			{
				if (bEnd)
					m_fPlaneIntrusion = m_fHeight - fDistToPlane;
				return Intersect;
			}
		}
	}

	// Handle the other cases
	float fPlaneCircle = ltsqrtf(pPlane->m_Normal.x * pPlane->m_Normal.x + pPlane->m_Normal.z * pPlane->m_Normal.z);
	// Truncate floating point error to avoid negative sqrts
	fPlaneCircle = LTMIN(fPlaneCircle, 1.0f);
	float fPlaneRadius = fPlaneCircle * m_fRadius;
	float fPlaneHeight = ltsqrtf(1 - fPlaneCircle * fPlaneCircle) * m_fHeight;
	float fProjMax = fPlaneRadius + fPlaneHeight;
	if (fDistToPlane >= fProjMax)
		return FrontSide;
	else if (fDistToPlane <= -fProjMax)
	{
		if (!bOptimize && bEnd)
			m_fPlaneIntrusion = (fProjMax * 2) - fDistToPlane;
		return BackSide;
	}
	else
	{
		if (bEnd)
			m_fPlaneIntrusion = fProjMax - fDistToPlane;
		return Intersect;
	}
}

CMovingCylinder::EHeightSection CMovingCylinder::GetHeightSection(float fYValue)
{
	if (fYValue < m_fMoveBottom)
		return HEIGHT_BELOW;
	else if (fYValue > m_fMoveTop)
		return HEIGHT_ABOVE;
	else
		return HEIGHT_MIDDLE;
}

bool ClipPolyToYRange(WorldPoly *pPoly, LTVector *aPolyVerts, uint32 nMaxVerts, uint32 *pNumVerts, float fMinY, float fMaxY)
{
	const uint32 k_nMaxVerts = 32;
	LTVector aClipBuffer[k_nMaxVerts];
	LTVector *pClipOut = aClipBuffer;
	LTVector *pClipEnd = &pClipOut[k_nMaxVerts];

	// Clip it to fMinY
	LTVector vPrevPos = pPoly->GetVertex(pPoly->GetNumVertices() - 1);
	float fPrevDist = vPrevPos.y - fMinY;
	for (uint32 nCurVert = 0; nCurVert < pPoly->GetNumVertices(); ++nCurVert)
	{
		const LTVector &vCurPos = pPoly->GetVertex(nCurVert);
		float fCurDist = vCurPos.y - fMinY;	

		if ((fCurDist * fPrevDist) < 0.0f)
		{
			float fInterpolant = fPrevDist / (fPrevDist - fCurDist);
			LTVector vMidPt;
			VEC_LERP(vMidPt, vPrevPos, vCurPos, fInterpolant);
			*pClipOut = vMidPt;
			++pClipOut;
			if (pClipOut == pClipEnd)
				return false;
		}

		if (fCurDist >= -0.0001f)
		{
			*pClipOut = vCurPos;
			++pClipOut;
			if (pClipOut == pClipEnd)
				return false;
		}

		vPrevPos = vCurPos;
		fPrevDist = fCurDist;
	}
	if (pClipOut == aClipBuffer)
		return false;

	pClipEnd = pClipOut;
	pClipOut = aClipBuffer;

	LTVector *pResultOut = aPolyVerts;
	LTVector *pResultEnd = &pResultOut[nMaxVerts];

	// Clip it to fMaxY
	vPrevPos = pClipEnd[-1];
	fPrevDist = vPrevPos.y - fMaxY;
	for (; pClipOut != pClipEnd; ++pClipOut)
	{
		const LTVector &vCurPos = *pClipOut;
		float fCurDist = vCurPos.y - fMaxY;

		if ((fCurDist * fPrevDist) < 0.0f)
		{
			float fInterpolant = fPrevDist / (fPrevDist - fCurDist);
			LTVector vMidPt;
			VEC_LERP(vMidPt, vPrevPos, vCurPos, fInterpolant);
			*pResultOut = vMidPt;
			++pResultOut;
			if (pResultOut == pResultEnd)
				return false;
		}

		if (fCurDist <= 0.0001f)
		{
			*pResultOut = vCurPos;
			++pResultOut;
			if (pResultOut == pResultEnd)
				return false;
		}

		vPrevPos = vCurPos;
		fPrevDist = fCurDist;
	}
	if (pResultOut == aPolyVerts)
		return false;

	// Return the clipped result
	*pNumVerts = pResultOut - aPolyVerts;
	return true;
}

//---------------------------------------------------------------------------//
// Collide the cylinder with a polygon
// Note : The top/bottom of the cylinder might have some outstanding collision
//issues.
bool CMovingCylinder::CollideWith(WorldPoly *pPoly, const Node *pNode)
{
	// Ignore non-solid polys
	if ((pPoly->GetSurface()->GetFlags() & SURF_SOLID) == 0)
		return false;

	LTVector *pPolyNormal = &pPoly->GetPlane()->m_Normal;
	// Don't worry about back-facing polys 
	if (m_vMovementDir.Dot(*pPolyNormal) > 0.01)
		return false;

	// There's not a collision if the radius of the polygon's outside the sphere radius
	LTVector vCenterDiff;
	float fPolyRadius = pPoly->GetRadius() + m_fMoveSphere;
	vCenterDiff = pPoly->GetCenter() - m_vMoveMid;
	if (vCenterDiff.Dot(vCenterDiff) > (fPolyRadius * fPolyRadius))
		return false;

	const uint32 k_nMaxPolyVerts = 32;
	LTVector aPolyVerts[k_nMaxPolyVerts];
	uint32 nNumVerts;

	// Handle the top & bottom of the cylinder
	if ((m_vStart.y != m_vEnd.y) && (fabsf(pPolyNormal->y) > 0.001f) && (m_fPlaneIntrusion > m_fClosestDist))
	{
		// Clip the polygon to the top/bottom of the swept cylinder
		// Skew the polygon based on the movement vector
		// If the unskewed cylinder representing the top/bottom movement intersects 
		// the skewed polygon portion, consider it a plane intersection and use the 
		// old sliding method
		bool bTop = pPolyNormal->y < 0.0f;
		float fHeightOffset = bTop ? m_fHeight : -m_fHeight;
		float fCapStart = m_vStart.y + fHeightOffset;
		float fCapEnd = m_vEnd.y + fHeightOffset;
		// Clip the poly to that range
		if (ClipPolyToYRange(pPoly, aPolyVerts, k_nMaxPolyVerts, &nNumVerts, LTMIN(fCapStart, fCapEnd), LTMAX(fCapStart, fCapEnd)))
		{
			// Skew the polygon so we can use a vertical cylinder
			LTVector vSkew = m_vEnd - m_vStart;
			vSkew /= vSkew.y;
			vSkew.y = 0.0f;

			float fMinY = FLT_MAX, fMaxY = -FLT_MAX;

			LTVector *pSkewVert = aPolyVerts;
			LTVector *pEndVert = &pSkewVert[nNumVerts];
			for (; pSkewVert != pEndVert; ++pSkewVert)
			{
				*pSkewVert += vSkew * (pSkewVert->y - m_vStart.y);

				fMinY = LTMIN(fMinY, pSkewVert->y);
				fMaxY = LTMAX(fMaxY, pSkewVert->y);
			}

			// Check for an intersection with the poly
			SBlockerSeg sCapSeg;
			sCapSeg.m_vOrigin = m_vStart;
			sCapSeg.m_vOrigin.y += fHeightOffset;
			sCapSeg.m_vDirection.Init(0.0f, m_vEnd.y - m_vStart.y, 0.0f);

			bool bIntersect = false;
			bool bInside = true;
			LTVector *pCurVert = aPolyVerts;
			LTVector *pPrevVert = &pCurVert[nNumVerts - 1];
			for (; (pCurVert != pEndVert) && (!bIntersect); ++pCurVert)
			{
				SBlockerSeg sEdgeSeg;
				sEdgeSeg.m_vOrigin = *pPrevVert;
				sEdgeSeg.m_vDirection = *pCurVert - *pPrevVert;

				float fDistSqr = DistSqrSegSeg(sCapSeg, sEdgeSeg, 0,0);
				if (fDistSqr < m_fRadiusSqr)
				{
					bIntersect = true;
					break;
				}

				if (bInside)
				{
					// Do a winding check based on the cross product of the edge and the offset from the start
					float fWindingCheck = (pPrevVert->z - m_vStart.z)*sEdgeSeg.m_vDirection.x - (pPrevVert->x - m_vStart.x)*sEdgeSeg.m_vDirection.z;
					if (bTop)
						fWindingCheck = -fWindingCheck;
					bInside = fWindingCheck >= 0.0f;
				}

				pPrevVert = pCurVert;
			}

			// Congratulations, you hit the polygon with the end of the cylinder
			if (bIntersect || bInside)
			{
				m_vClosestPt = m_vEnd;
				m_pClosestNode = pNode;
				float fClosestDist;
				LTVector vClosestDir;
				if (bTop)
				{
					if ((fMinY - 0.001f) > fCapStart)
					{
						fClosestDist = fCapEnd - fMinY;
						vClosestDir.Init(0.0f, 1.0f, 0.0f);
					}
					else
					{
						fClosestDist = m_fPlaneIntrusion;
						vClosestDir = -*pPolyNormal;
					}
				}
				else
				{
					if ((fMaxY + 0.001f) < fCapStart)
					{
						fClosestDist = fMaxY - fCapEnd;
						vClosestDir.Init(0.0f, -1.0f, 0.0f);
					}
					else
					{
						fClosestDist = m_fPlaneIntrusion;
						vClosestDir = -*pPolyNormal;
					}
				}
				if (fClosestDist <= m_fClosestDist)
					return false;
				
				m_fClosestDist = fClosestDist;
				m_vClosestDir = vClosestDir;
				return true;
			}
		}
	}

	// Clip the poly to the ending height of the cylinder
	float fMidRangeMinY = LTMAX(m_vStart.y, m_vEnd.y) - m_fHeight;
	float fMidRangeMaxY = LTMIN(m_vStart.y, m_vEnd.y) + m_fHeight;
	if (!ClipPolyToYRange(pPoly, aPolyVerts, k_nMaxPolyVerts, &nNumVerts, fMidRangeMinY, fMidRangeMaxY))
		return false;

	// Get the point of closest approach on the polygon
	SBlockerSeg sClosestSeg;
	float fClosestDist = FLT_MAX;
	float fClosestP0 = 1.0f;
	float fClosestP1;

	SBlockerSeg sMoveSeg;
	sMoveSeg.m_vOrigin.Init(m_vStart.x, 0.0f, m_vStart.z);
	sMoveSeg.m_vDirection = m_vEnd - m_vStart;
	sMoveSeg.m_vDirection.y = 0.0f;
	
	LTVector *pCurVert = aPolyVerts;
	LTVector *pEndVert = &pCurVert[nNumVerts];
	LTVector *pPrevVert = &pEndVert[-1];
	for (; pCurVert != pEndVert; ++pCurVert)
	{
		SBlockerSeg sCurSeg;
		sCurSeg.m_vOrigin.Init(pPrevVert->x, 0.0f, pPrevVert->z);
		sCurSeg.m_vDirection = *pCurVert - *pPrevVert;
		sCurSeg.m_vDirection.y = 0.0f;
		float fP0, fP1;
		float fSegDist = DistSqrSegSeg(sMoveSeg, sCurSeg, &fP0, &fP1);
		if (fSegDist < fClosestDist)
		{
			// Correctly deal with the distance in the case where the movement segment crosses more than one edge
			// NYI
			LTVector vPtOnSeg = sCurSeg.m_vOrigin + sCurSeg.m_vDirection * fP1;
			LTVector vOffset = vPtOnSeg - m_vStart;
			if (vOffset.Dot(sMoveSeg.m_vDirection) > 0.0f)
			{
				sClosestSeg = sCurSeg;
				fClosestP0 = fP0;
				fClosestP1 = fP1;
				fClosestDist = fSegDist;
			}
		}
		pPrevVert = pCurVert;
	}

	// If the edges are outside the radius, we're probably not touching it
	if (fClosestDist > m_fRadiusSqr)
	{
		return false;
	}

	// Make sure there isn't already a closer collision waiting
	fClosestDist = m_fRadius - sqrtf(fClosestDist);
	if (fClosestDist <= m_fClosestDist)
		return false;

	// Save the intersection information for later use
	if ((fClosestP0 < 1.0f) && (fClosestDist >= m_fRadius))
	{
		// Back the intersection point up to the starting point if we were intersecting
		// at the beginning of this intersection test
		// Note : This really should back up the intersection to the point of intersection
		// instead of going the whole way, but this really shouldn't even come up
		// based on the way this function is getting called
		sMoveSeg.m_vDirection.Init();
		fClosestP0 = 0.0f;
		DistSqrSegSeg(sMoveSeg, sClosestSeg, 0, &fClosestP1);
	}
	LTVector vPolyPt = sClosestSeg.m_vOrigin + sClosestSeg.m_vDirection * fClosestP1;
	m_vClosestPt = sMoveSeg.m_vOrigin + sMoveSeg.m_vDirection * fClosestP0;
	m_vClosestDir = vPolyPt - m_vClosestPt;
	m_vClosestDir.Normalize();
	m_vClosestPt.y = LTLERP(m_vStart.y, m_vEnd.y, fClosestP0);
	m_fClosestDist = fClosestDist;
	m_pClosestNode = pNode;

	return true;
}


//---------------------------------------------------------------------------//
void CMovingCylinder::HandleCollision
(
	CollideRequest&	request,
	const Node*		pNode,
	CollideInfo*	pInfo
)
{
	// Move away from the "closest" point
	LTVector vMoveAway = m_vClosestDir * -(m_fClosestDist * 1.01f);
	// Apply a sideways reflection vector to avoid some "sticky" situations caused by low framerate
	if (!m_vClosestDir.y)
	{
		LTVector vRight = m_vMovementDir.Cross(LTVector(0.0f, 1.0f, 0.0f));
		vMoveAway += vRight * (vRight.Dot(m_vClosestDir) * -1.0f);
	}
	m_vEnd = m_vClosestPt + vMoveAway;

	// Attach to horizontal planes
	if (m_bStairStep && (m_vClosestDir.y < 0.707f))
	{
		pInfo->m_pStandingOn = m_pClosestNode;
		DoObjectCollisionResponse( request, pInfo, pNode );
	}

	// Recalculate the pre-calculated data
	Recalc();
}


//---------------------------------------------------------------------------//
//NOTE: This doesn't call DoObjectCollisionResponse because the player
//shouldn't be bounced or have their velocity changed by this function.
bool CollideCylinderWithTree
(
	const LTVector&	P0,
	const LTVector&	offset,
	CollideRequest&	request,
	CollideInfo*	pInfo,
	LTVector&		P1,
	LTVector*		pCollideNormal // Returns the last collision poly's normal
)
{
	Node *stack[1024];
	Node **pStackPos;
	float Dot;
	int state1, state2;
	const Node *pRoot;
	LTVector vFullStart, vFullEnd, vDirection;
	float fVelocityLeft, fVelocityStep, fVelocitySegment;
	uint32 nOldHitCount = pInfo->m_nHits, nRetryHitCount = pInfo->m_nHits;
	int iRetryCollision = 3;

	CMovingCylinder theCylinder;

	// Disable cylinder stair stepping..  Maybe that code will come in handy some day...
	theCylinder.m_bStairStep = false;
	// Set up the cylinder size
	theCylinder.m_fRadius = MIN( request.m_Dims.x, request.m_Dims.z );
	theCylinder.m_fHeight = request.m_Dims.y;

	// No invalid cylinders!
	if ((theCylinder.m_fRadius < 0.01f) || (theCylinder.m_fHeight < 0.01f))
		return LTTRUE;

	// Set up the iterative test parameters
	vFullStart = P0 + offset;
	vFullEnd = P1 + offset;
	vDirection = vFullEnd - vFullStart;
	fVelocityLeft = vDirection.Mag();
	if (!fVelocityLeft)
		return false;
	// Get the radius & height velocities
	LTVector vRadiusDirection(vDirection.x, 0.0f, vDirection.z);
	float fRadiusVelocity = vRadiusDirection.Mag();
	float fHeightVelocity = (float)fabs(vDirection.y);
	// Normalize the direction vector
	vDirection *= 1.0f / fVelocityLeft;
	// Decide whether or not we need to restrict the velocity
	if ((fHeightVelocity > (theCylinder.m_fHeight * 1.8f)) || 
		(fRadiusVelocity > (theCylinder.m_fRadius * 0.9f)))
	{
		// OK, which one needs the biggest restriction?
		float fRadiusRatio = fRadiusVelocity / theCylinder.m_fRadius;
		float fHeightRatio = fHeightVelocity / (theCylinder.m_fHeight * 2);

		float fStepRadius, fStepHeight;
		// Moving forward/sideways faster?
		if (fRadiusRatio > fHeightRatio)
		{
			fStepRadius = theCylinder.m_fRadius;
			fStepHeight = (fStepRadius / fRadiusVelocity) * fHeightVelocity;
		}
		// Aah, moving vertically faster..
		else
		{
			fStepHeight = theCylinder.m_fHeight * 2;
			fStepRadius = (fStepHeight / fHeightVelocity) * fRadiusVelocity;
		}
		// Ok, figure out how long the steps are going to be..
		float fFullStep = sqrtf(fStepRadius * fStepRadius + fStepHeight * fStepHeight);
		fVelocityStep = fFullStep * 0.75f;
		fVelocitySegment = fFullStep * 0.9f;
	}
	// Otherwise just use the whole velocity
	else
	{
		fVelocityStep = fVelocityLeft;
		fVelocitySegment = fVelocityLeft;
	}

	if (fVelocityStep <= 0.001f)
	{
		//dsi_ConsolePrint("Warning: Zero Velocity Step in CollideCylinderWithTree()!");
		return false;
	}

	// Remember where the cylinder is REALLY going to go...
	theCylinder.m_vRealEnd = vFullEnd;

	while (fVelocityLeft > 0.0f)
	{
		if (nOldHitCount == pInfo->m_nHits)
		{
			// Get the next segment of the movement search
			if (fVelocityLeft < fVelocityStep)
				vFullEnd = vFullStart + (vDirection * fVelocityLeft);
			else
				vFullEnd = vFullStart + (vDirection * fVelocitySegment);

			// Set up the cylinder for this segment
			theCylinder.m_vStart = vFullStart;
			theCylinder.m_vEnd = vFullEnd;
			theCylinder.Recalc();
		}

		// Collide with the BSP for this segment
		pRoot = request.m_pWorld->GetRootNode();
		pStackPos = stack;
		for(;;)
		{
			if ((pRoot==NODE_IN) || (pRoot==NODE_OUT))
			{
				if(pStackPos == stack)
					break;

				--pStackPos;
				pRoot = *pStackPos;
			}

			// Do the fast test on the whole movement area.
			Dot = pRoot->GetPlane()->DistTo(theCylinder.m_vMoveMid);
			if(Dot > theCylinder.m_fMoveSphere)
			{
				pRoot = pRoot->m_Sides[FrontSide];
				continue;
			}
			else if(Dot < -theCylinder.m_fMoveSphere)
			{
				pRoot = pRoot->m_Sides[BackSide];
				continue;
			}
			
			// Decide which side of the plane we're on
			state1 = theCylinder.GetPlaneSide(pRoot->GetPlane(), theCylinder.m_vStart);
			state2 = theCylinder.GetPlaneSide(pRoot->GetPlane(), theCylinder.m_vEnd, true, state1 != FrontSide);


			// Do something based on the values.
			if(state1 == state2)
			{
				if(state1 == Intersect)
				{
					if (theCylinder.CollideWith(pRoot->m_pPoly, pRoot))
					{
						*pCollideNormal = pRoot->GetPlane()->m_Normal;
						++pInfo->m_nHits;
					}

					state1 = pRoot->GetPlane()->DistTo(theCylinder.m_vStart) > -0.001f;
					if ((pRoot->m_Sides[!state1] != NODE_IN) && (pRoot->m_Sides[!state1] != NODE_OUT))
					{
						assert(pStackPos < (stack + 1024));
						*(pStackPos++) = pRoot->m_Sides[!state1];
					}
					pRoot = pRoot->m_Sides[state1];
				}
				else
				{
					pRoot = pRoot->m_Sides[state1];
				}
			}
			else
			{
				if( state1 == FrontSide && theCylinder.CollideWith(pRoot->m_pPoly, pRoot))
				{
					*pCollideNormal = pRoot->GetPlane()->m_Normal;
					++pInfo->m_nHits;
				}

				state1 = pRoot->GetPlane()->DistTo(theCylinder.m_vStart) > -0.001f;
				if ((pRoot->m_Sides[!state1] != NODE_IN) && (pRoot->m_Sides[!state1] != NODE_OUT))
				{
					assert(pStackPos < (stack + 1024));
					*(pStackPos++) = pRoot->m_Sides[!state1];
				}
				pRoot = pRoot->m_Sides[state1];
			}
		}

		// Break out of the iterative search if there was a collision
		if (nOldHitCount != pInfo->m_nHits)
		{
			// Re-try the collision if necessary
			if (iRetryCollision && (nRetryHitCount != pInfo->m_nHits))
			{
				iRetryCollision--;

				// Try to move to a non-collision position
				theCylinder.HandleCollision( request, theCylinder.m_pClosestNode, pInfo );

				// Remember how many hits there were..
				nRetryHitCount = pInfo->m_nHits;
				continue;
			}
			else
			{
				if (nRetryHitCount != pInfo->m_nHits ||
					(isnan(theCylinder.m_vEnd[0])) ||
					(isnan(theCylinder.m_vEnd[1])) ||
					(isnan(theCylinder.m_vEnd[2])) )
					theCylinder.m_vEnd = P0 + offset;
				break;
			}
		}

		// Move to the next segment of the search
		vFullStart = vFullStart + (vDirection * fVelocityStep);
		fVelocityLeft -= fVelocityStep;
	}

	P1 = theCylinder.m_vEnd - offset;

	return pInfo->m_nHits != nOldHitCount;
}


//---------------------------------------------------------------------------//
//Initialize the swept AABB
static void SetupAxisAlignedBox
(
	const LTVector& P0,			//first and second position
	const LTVector& P1,
	const LTVector& dims,		//box dimensions
	const LTVector& offset,		//stairstep offset
	PhysicsSphere&	whole_sphere,	//motion encompassing sphere
	AABB&			box			//AABB
)
{
	LTVector vStart = P0 + offset;
	LTVector vEnd = P1 + offset;

	box.Min = vEnd - dims;
	box.Max = vEnd + dims;

	LTVector vWholeMin;
	LTVector vWholeMax;
	VEC_MIN(vWholeMin, vStart, vEnd);
	VEC_MAX(vWholeMax, vStart, vEnd);
	vWholeMin -= dims;
	vWholeMax += dims;

	whole_sphere.m_Center = (vWholeMin + vWholeMax) * 0.5f;
	whole_sphere.m_Radius = (vWholeMax - whole_sphere.m_Center).Mag() + 1;//add some slop to radius
}


//---------------------------------------------------------------------------//
// Clip an outline into the destination along an axis-aligned plane
// Returns the number of vertices written into the output buffer
// Note : This is designed for speed, not robustness.
static uint32 ClipOutline(const LTVector *pInput, LTVector *pOutput, uint32 nNumVerts, uint32 nAxis, float fPlane)
{
	if (!nNumVerts)
		return 0;

	uint32 nAxisIndex = nAxis % 3;
	float fAxisMult = (nAxis > 2) ? 1.0f : -1.0f;
	float fPlaneCompare = fPlane * fAxisMult;
	const LTVector *pPrev = &pInput[nNumVerts - 1];
	bool bPrevClip = ((*pPrev)[nAxisIndex] * fAxisMult) > fPlaneCompare;
	const LTVector *pEndInput = &pInput[nNumVerts];
	LTVector *pCurOutput = pOutput;
	for (; pInput != pEndInput; ++pInput)
	{
		const LTVector &vCur = *pInput;
		const LTVector &vPrev = *pPrev;
		bool bCurClip = (vCur[nAxisIndex] * fAxisMult) > fPlaneCompare;
		if (bCurClip != bPrevClip)
		{
			float fDist = vCur[nAxisIndex] - vPrev[nAxisIndex];
			float fTime;
			if (fabsf(fDist) > 0.001f)
			{
				fTime = (fPlane - vPrev[nAxisIndex]) / fDist;
			}
			else if (!bCurClip)
			{
				fTime = 0.0f;
			}
			else
			{
				fTime = 1.0f;
			}
			ASSERT(fTime >= 0.0f && fTime <= 1.0f);
			VEC_LERP(*pCurOutput, vPrev, vCur, fTime);
			++pCurOutput;
		}
		if (!bCurClip)
		{
			*pCurOutput = vCur;
			++pCurOutput;
		}

		pPrev = pInput;
		bPrevClip = bCurClip;
	}

	return pCurOutput - pOutput;
}

// Calculate the maximum Y value of the poly clipped to the box
// Returns false if the poly does not intersect the box
static bool GetMaxYIntrusion(const AABB &box, const WorldPoly *pPoly, float *pResult)
{
	const uint32 k_NumClipVerts = 64;
	LTVector aClipBuffer[2][k_NumClipVerts];
	uint32 nNumVerts = LTMIN(pPoly->GetNumVertices(), k_NumClipVerts - 2);

	*pResult = box.Min.y;

	// Init the clip buffer;
	for (uint32 nCurPt = 0; nCurPt < pPoly->GetNumVertices(); ++nCurPt)
		aClipBuffer[0][nCurPt] = pPoly->GetVertex(nCurPt);

	// Clip it
	nNumVerts = ClipOutline(aClipBuffer[0], aClipBuffer[1], nNumVerts, 0, box.Min.x);
	ASSERT(nNumVerts <= k_NumClipVerts - 2);
	nNumVerts = LTMIN(nNumVerts, k_NumClipVerts - 2);
	nNumVerts = ClipOutline(aClipBuffer[1], aClipBuffer[0], nNumVerts, 1, box.Min.y);
	ASSERT(nNumVerts <= k_NumClipVerts - 2);
	nNumVerts = LTMIN(nNumVerts, k_NumClipVerts - 2);
	nNumVerts = ClipOutline(aClipBuffer[0], aClipBuffer[1], nNumVerts, 2, box.Min.z);
	ASSERT(nNumVerts <= k_NumClipVerts - 2);
	nNumVerts = LTMIN(nNumVerts, k_NumClipVerts - 2);
	nNumVerts = ClipOutline(aClipBuffer[1], aClipBuffer[0], nNumVerts, 3, box.Max.x);
	ASSERT(nNumVerts <= k_NumClipVerts - 2);
	nNumVerts = LTMIN(nNumVerts, k_NumClipVerts - 2);
	nNumVerts = ClipOutline(aClipBuffer[0], aClipBuffer[1], nNumVerts, 4, box.Max.y);
	ASSERT(nNumVerts <= k_NumClipVerts - 2);
	nNumVerts = LTMIN(nNumVerts, k_NumClipVerts - 2);
	nNumVerts = ClipOutline(aClipBuffer[1], aClipBuffer[0], nNumVerts, 5, box.Max.z);
	if (!nNumVerts)
		return false;

	// Get the min Y value
	LTVector *pCheck = aClipBuffer[0];
	LTVector *pEndCheck = &aClipBuffer[0][nNumVerts];
	for (; pCheck != pEndCheck; ++pCheck)
		*pResult = LTMAX(*pResult, pCheck->y);

	return *pResult > box.Min.y;
}

bool StairStep_CylinderPolyIntersect(const LTVector &vBase, float fHeight, float fRadius, WorldPoly *pPoly, float *pResultMaxY)
{
	// Nonsolid surfaces don't get checked
	if ((pPoly->GetSurface()->m_Flags & SURF_SOLID) == 0)
		return false;

	// Adjust the radius a bit to avoid stepping onto things we should collide with
	if (fRadius > 0.1f)
		fRadius -= 0.1f;

	// Check the poly's radius distance from the cylinder for an early out
	// NYI - This should probably do a cylinder/sphere distance check instead of a sphere/sphere check
	float fHalfHeight = fHeight * 0.5f;
	float fSphereRadiusSqr = fHalfHeight * fHalfHeight + fRadius * fRadius;
	float fSphereRadius = sqrtf(fSphereRadiusSqr);
	LTVector vCylinderCenter(vBase.x, vBase.y + fHalfHeight, vBase.z);
	float fDistToPolySqr = (vCylinderCenter - pPoly->GetCenter()).MagSqr();
	float fTotalRadius = fSphereRadius + pPoly->GetRadius();
	if (fDistToPolySqr > (fTotalRadius * fTotalRadius))
		return false;

	// Get the height of the intersection of the plane and the cylinder and use that for the Y range
	const LTVector &vPolyNormal = pPoly->GetPlane()->Normal();
	float fPlaneYIntercept = (pPoly->GetPlane()->Dist() - vPolyNormal.x * vBase.x - vPolyNormal.z * vBase.z) / vPolyNormal.y;
	ASSERT(pPoly->GetPlane()->Normal().y != 0.0f);
	float fPlaneYProjection = fRadius * sqrtf(1.0f / (vPolyNormal.y * vPolyNormal.y) - 1.0f);
	float fPlaneMinY = fPlaneYIntercept - fPlaneYProjection;
	float fPlaneMaxY = fPlaneYIntercept + fPlaneYProjection;

	ASSERT(fabsf(pPoly->GetPlane()->DistTo(LTVector(vBase.x, fPlaneYIntercept, vBase.z))) < 0.1f);

	// Jump out if the cylinder's not low enough to touch the plane
	if (fPlaneMaxY < vBase.y)
		return false;

	// Get the intersection of the cylinder height and the plane/cylinder Y intersection.
	// Note the epsilons: when a horizontal polygon lies within the extents, it has the plane min y
	// the same as the plane max y, and at large values this can cause issues with accuracy that cause
	// the player to fall through the floor. These offsets are clipped to the extents of the cylinder
	// as well, so it won't report any false collisions.
	float fMinY = LTMAX(vBase.y, fPlaneMinY - 0.1f);
	float fMaxY = LTMIN(vBase.y + fHeight, fPlaneMaxY + 0.1f);

	// Clip the polygon to the Y range
	const uint32 k_nMaxPolyVerts = 32;
	LTVector aClippedPoly[k_nMaxPolyVerts];
	uint32 nClippedVerts;
	if (!ClipPolyToYRange(pPoly, aClippedPoly, k_nMaxPolyVerts, &nClippedVerts, fMinY, fMaxY))
		return false;

	// Did the cylinder touch an edge?
	bool bEdgeIntersect = false;
	// What was the maximum y intersection of an edge?
	float fMaxEdgeY = -FLT_MAX;
	// Find the high point on the plane
	LTVector vPlaneCircle(vPolyNormal.x, 0.0f, vPolyNormal.z);
	if ((vPolyNormal.x != 0.0f) || (vPolyNormal.z != 0.0f))
		vPlaneCircle.Normalize();
	LTVector vHighPoint(vBase.x - vPlaneCircle.x * fRadius, fPlaneMaxY, vBase.z - vPlaneCircle.z * fRadius);
	// Was the high point inside of the polygon?
	bool bHighPointInside = true;

	ASSERT(fabsf(pPoly->GetPlane()->DistTo(vHighPoint)) < 0.1f);

	// Cache the radius^2
	float fRadiusSqr = fRadius * fRadius;

	// Cylinder segment representation for use by the distance function
	// (Can go away if a 2D pt/line distance function is used...)
	SBlockerSeg sCylinderSeg;
	sCylinderSeg.m_vOrigin = vBase;
	sCylinderSeg.m_vDirection.Init(0.0f, fHeight, 0.0f);

	LTVector *pCurVert = aClippedPoly;
	LTVector *pEndVert = &pCurVert[nClippedVerts];
	LTVector *pPrevVert = &pEndVert[-1];
	for (; pCurVert != pEndVert; ++pCurVert)
	{
		SBlockerSeg sEdgeSeg;
		sEdgeSeg.m_vOrigin = *pPrevVert;
		sEdgeSeg.m_vDirection = *pCurVert - *pPrevVert;
		
		// Get the distance to the edge from the center of the cylinder
		// Note : Change this to a 2D pt/line distance function
		float fP0;
		float fDistSqr = DistSqrSegSeg(sCylinderSeg, sEdgeSeg, 0, &fP0);
		// Does it intersect?
		if (fDistSqr < fRadiusSqr)
		{
			bEdgeIntersect = true;
			// Get the high-point on the edge
			float fPtOnLineAdj = sqrtf(fRadiusSqr - fDistSqr) / sEdgeSeg.m_vDirection.Mag();
			if (sEdgeSeg.m_vDirection.y < 0.0f)
				fPtOnLineAdj = -fPtOnLineAdj;
			float fMaxYP0 = fP0 + fPtOnLineAdj;
			fMaxYP0 = LTCLAMP(fMaxYP0, 0.0f, 1.0f);
			float fCurEdgeMaxY = sEdgeSeg.m_vOrigin.y + sEdgeSeg.m_vDirection.y * fMaxYP0;
			if (fCurEdgeMaxY > fMaxEdgeY)
				fMaxEdgeY = fCurEdgeMaxY;
		}

		// Check to see if the high point is inside the polygon
		if (bHighPointInside)
		{
			float fWindingCheck = (pPrevVert->z - vHighPoint.z)*sEdgeSeg.m_vDirection.x - (pPrevVert->x - vHighPoint.x)*sEdgeSeg.m_vDirection.z;
			bHighPointInside = fWindingCheck >= -0.001f;
		}

		pPrevVert = pCurVert;
	}

	// If it didn't hit an edge, and the high point isn't inside the polygon, there was no intersection
	if (!bEdgeIntersect && !bHighPointInside)
		return false;

	*pResultMaxY = bHighPointInside ? fPlaneMaxY : fMaxEdgeY;
	return true;
}

//---------------------------------------------------------------------------//
static bool StairStep_Segment
(
	const LTVector&	offset,
	CollideInfo*	pInfo,
	CollideRequest&	request,
	LTVector&		P0,
	LTVector&		P1
)
{
	const Node *stack[1024];
	const Node *pRoot = request.m_pWorld->GetRootNode();
	bool bResult = false;
	float fDownRemaining;
	PhysicsSphere whole_sphere;
	AABB box;

	//setup the box
	SetupAxisAlignedBox( P0, P1, request.m_Dims, offset, whole_sphere, box );

	//remember how much we were moving down so we can stand on NotAStep brushes
	if( P0.y > P1.y )
		fDownRemaining = P0.y - P1.y;
	else
		fDownRemaining = 0.0f;

	const Node** stackPos = stack;

	LTVector vCylinderBase;
	float fCylinderHeight, fCylinderRadius;
	if (request.m_pObject->m_Flags2 & FLAG2_PLAYERCOLLIDE)
	{
		float fHalfBoxWidth = (box.Max.x - box.Min.x) * 0.5f;
		float fHalfBoxDepth = (box.Max.z - box.Min.z) * 0.5f;
		vCylinderBase.Init(box.Min.x + fHalfBoxWidth, box.Min.y, box.Min.z + fHalfBoxDepth);
		fCylinderHeight = box.Max.y - box.Min.y;
		fCylinderRadius = LTMIN(fHalfBoxWidth, fHalfBoxDepth);
	}

	while( true )
	{
		// Done?
		if((pRoot==NODE_IN) || (pRoot==NODE_OUT))
		{
			if( stackPos == stack )
				return bResult;

			--stackPos;
			pRoot = *stackPos;
		}

		const float dist = pRoot->GetPlane()->DistTo( whole_sphere.m_Center );

		if( dist > whole_sphere.m_Radius )
		{
			pRoot = pRoot->m_Sides[FrontSide];
		}
		else if( dist < -whole_sphere.m_Radius )
		{
			pRoot = pRoot->m_Sides[BackSide];
		}
		else
		{
			const float fLimit = 0.0f;
			const float fAllowUpLimit = 0.7071f;//(45 degree angle)

			// Is it something they could step onto?
			if( pRoot->m_pPoly && ( pRoot->GetPlane()->m_Normal.y > fLimit ) )
			{
				const LTVector fullVel = request.m_pObject->m_Velocity + pInfo->m_VelOffset;

				if( fullVel.Dot(pRoot->GetPlane()->m_Normal) <= 0.0f )
				{
					// Does it really intersect?
					float fMaxY;
					bool bPolyIntersect;
					if (request.m_pObject->m_Flags2 & FLAG2_PLAYERCOLLIDE)
					{
						bPolyIntersect = StairStep_CylinderPolyIntersect(vCylinderBase, fCylinderHeight, fCylinderRadius, pRoot->m_pPoly, &fMaxY);
					}
					else
					{
						bPolyIntersect = WorldPolyIntersectsAABB( whole_sphere, *pRoot->m_pPoly, box );
						if (bPolyIntersect)
						{
							if (!GetMaxYIntrusion(box, pRoot->m_pPoly, &fMaxY))
							{
								fMaxY = box.Min.y;
							}
						}
					}
					if (bPolyIntersect)
					{
						float maxPushAmt = fMaxY - box.Min.y;
						const LTPlane& root_plane = *pRoot->GetPlane();
						LTPlane& push_plane = request.m_pCollisionInfo->m_Plane;

						push_plane = root_plane;

						//Setup the collision info...
						//Because we could hit more than one poly,
						//the collision info will contain the last poly hit...
						//Need to do this here, cuz DoObjectCollisionResponse not always called...
						request.m_pCollisionInfo->m_hObject	= request.m_pWorldObj;
						request.m_pCollisionInfo->m_hPoly	= request.m_pWorld->MakeHPoly(pRoot);

						if( maxPushAmt > 0.0f )
						{
							// Add a little fudge
							maxPushAmt += 0.01f;

							// Figure out how much we would need to move
							LTVector toAdd(0.0f, maxPushAmt, 0.0f);

							// If we can step on this, or we're not stepping up to get on this...
							if( (((pRoot->m_pPoly->GetSurface()->GetFlags() & SURF_NOTASTEP) == 0)
								|| 
								(fDownRemaining >= toAdd.y)) &&
								(pRoot->GetPlane()->m_Normal.y > fAllowUpLimit)
								)
							{
								// Register the hit.
								++pInfo->m_nHits;

								// Don't change the velocity offset
								LTVector vSaveVel = pInfo->m_VelOffset;

								//NOTE:  this function overwrites the plane
								DoObjectCollisionResponse(	request, pInfo, pRoot );

								pInfo->m_VelOffset = vSaveVel;

								// Add to P0 and P1 so we don't hit any more polies on
								// the same plane again.
								P1 += toAdd;
								P0.y = P1.y;

								// Adjust the location of the bounding box back to where it was & make it smaller
								const LTVector new_offset( offset.x, offset.y - toAdd.y * 0.5f, offset.z );
								LTVector dims = request.m_Dims;

								dims.y -= toAdd.y * 0.5f;

								SetupAxisAlignedBox(P0, P1,
													dims,
													new_offset,
													whole_sphere,
													box);

								// Adjust the cylinder
								if (request.m_pObject->m_Flags2 & FLAG2_PLAYERCOLLIDE)
								{
									vCylinderBase.y = box.Min.y;
									fCylinderHeight = box.Max.y - box.Min.y;
								}

								// Say we're standing on this thing.
								pInfo->m_pStandingOn = pRoot;

								// Remove this movement from the up/down amount
								fDownRemaining -= toAdd.y;
							}
							else
							{
								// Remember that we hit a non-step
								bResult = true;
							}
						}

						// Now we're on the front side.
						pRoot = pRoot->m_Sides[FrontSide];
						continue;
					}
				}
			}

			// Go into both sides.
			if(	(pRoot->m_Sides[BackSide] != NODE_IN)
				&&
				(pRoot->m_Sides[BackSide] != NODE_OUT) )
			{
				assert(stackPos < (stack + 1024));
				*stackPos++ = pRoot->m_Sides[BackSide];
			}

			pRoot = pRoot->m_Sides[FrontSide];
		}
	}
}

// Wrapper function to prevent tunneling in the stair stepping code
static bool StairStep
(
	const LTVector&	offset,
	CollideInfo*	pInfo,
	CollideRequest&	request,
	LTVector&		P0,
	LTVector&		P1
)
{
	bool bResult = false;

	LTVector vMovement = P1 - P0;

	// Figure out how many subdivisions to use
	uint32 nMovementSteps;
	if (request.m_pObject->m_Flags2 & FLAG2_PLAYERCOLLIDE)
		nMovementSteps = (uint32)(fabsf(vMovement.y) / (request.m_Dims.y * 2.0f - 0.01f)) + 1;
	else
		nMovementSteps = 1;

	LTVector vCurP0 = P0;
	LTVector vMovementStep = vMovement / (float)nMovementSteps;

	if (nMovementSteps > 1)
		nMovementSteps = nMovementSteps;

	bool bStepped = false;

	do
	{
		LTVector vCurP1 = vCurP0 + vMovementStep;

		LTVector vTestP0 = vCurP0;
		LTVector vTestP1 = vCurP1;

		// Do the actual stair stepping
		bResult |= StairStep_Segment(offset, pInfo, request, vTestP0, vTestP1);

		// If the positions were changed by the stairstepping code, we've stepped
		// onto something
		if ((vTestP0 != vCurP0) || (vTestP1 != vCurP1))
		{
			vCurP1 = vTestP1;
			bStepped = true;
		}

		vCurP0 = vCurP1;
		--nMovementSteps;
	} while (nMovementSteps);

	if (bStepped)
		P1 = vCurP0;

	// We didn't find anything to step onto
	// So tell them whether or not we hit a non-step
	return bResult;
}


// Goes thru a solid BSP and tests if the box touches an outside node
static bool SolidBoxBSPIntersect
(
	const AABB&				box,
	const Node*				pRoot
)
{
	static Node *stack[1000];
	Node **stackPos = stack;

	// Pre-calculate some stuff..
	LTVector vBoxCenter = box.center();
	LTVector vBoxHalfDims = box.half_dimensions();

	for (;;)
	{
		// Are we done with this branch?
		if (pRoot->m_Flags & (NF_IN | NF_OUT))
		{
			// Are we touching an out-leaf?
			if ((pRoot->m_Flags & NF_OUT) != 0)
				return true;

			if (stackPos == stack)
				return false;

			--stackPos;
			pRoot = *stackPos;
		}

		const LTPlane &cRootPlane = *(pRoot->m_pPoly->GetPlane());

		LTVector vProjectedDims = cRootPlane.m_Normal * vBoxHalfDims;
		float fProjectedRadius = vProjectedDims.Mag();

		float fCenterDist = cRootPlane.DistTo(vBoxCenter);

		if (fCenterDist > fProjectedRadius)
		{
			pRoot = pRoot->m_Sides[FrontSide];
		}
		else if (fCenterDist < -fProjectedRadius)
		{
			pRoot = pRoot->m_Sides[BackSide];
		}
		else
		{
			// Are we touching an out-leaf?
			if ((pRoot->m_Sides[BackSide]->m_Flags & NF_OUT) != 0)
				return true;

			if ((pRoot->m_Sides[BackSide]->m_Flags & NF_IN) == 0)
				*stackPos++ = pRoot->m_Sides[BackSide];
			pRoot = pRoot->m_Sides[FrontSide];
		}
	}

	return false;
}


// Goes thru the BSP and tests if the box intersects the BSP.
// It uses whole_sphere as a quick approximation to the box first.
static bool SimpleBoxBSPIntersect
(
	const AABB&				box,
	const Node*				pRoot,
	const PhysicsSphere&	whole_sphere
)
{
	static Node *stack[1000];
	Node **stackPos = stack;

	for(;;)
	{
		// Done?
		if( pRoot->m_Flags & (NF_IN | NF_OUT) )
		{
			if( stackPos == stack )
				return false;

			--stackPos;
			pRoot = *stackPos;
		}

		const float Dot = pRoot->m_pPoly->GetPlane()->DistTo( whole_sphere.m_Center );

		if( Dot > whole_sphere.m_Radius )
		{
			pRoot = pRoot->m_Sides[FrontSide];
		}
		else if( Dot < -whole_sphere.m_Radius )
		{
			pRoot = pRoot->m_Sides[BackSide];
		}
		else
		{
			// Is it something they could step onto?
			if(pRoot->m_pPoly)
			{
				// Does it really intersect?
				if( WorldPolyIntersectsAABB( whole_sphere, *pRoot->m_pPoly, box ) )
				{
					return true;
				}
			}
		
			// Go into both sides.
			if (( pRoot->m_Sides[BackSide]->m_Flags & (NF_IN | NF_OUT) ) == 0)
				*stackPos++ = pRoot->m_Sides[BackSide];
			pRoot = pRoot->m_Sides[FrontSide];
		}
	}

	return false;
}


//---------------------------------------------------------------------------//
bool DoesBoxIntersectBSP
(
	const Node*		pRoot,	//root of BSP tree
	const LTVector&	min,	//box extents
	const LTVector&	max,
	bool bSolid
)
{
	if (bSolid)
	{
		return SolidBoxBSPIntersect( AABB(min,max), pRoot );
	}

	PhysicsSphere whole_sphere;//contains start and end sphere

	// Setup the sphere approximation.
	whole_sphere.m_Center = max - min;
	whole_sphere.m_Center *= 0.5f;
	whole_sphere.m_Radius = whole_sphere.m_Center.Mag() + 0.1f;
	whole_sphere.m_Center += min;

	return SimpleBoxBSPIntersect( AABB(min,max), pRoot, whole_sphere );
}


static void DoPointCollision
(
	CollideRequest& request,
	CollideInfo*	pInfo
)
{
	int i;
	const Node *pNode;
	LTVector iPos;
	LTPlane iPlane;
	bool bContinue;

	pInfo->m_FinalPos = request.m_NewPos;

	for(i=0; i < MAX_PHYSICS_ITERATIONS; i++)
	{
		bContinue = false;

		// Do a fast test to see if we hit anything.
		pNode = IntersectLine(	request.m_pWorld->GetRootNode(), 
								&request.m_OriginalPos,
								&pInfo->m_FinalPos,
								&iPos,
								&iPlane
								);

		if( pNode )
		{
			// Have it adjust the velocity.
			DoObjectCollisionResponse( request, pInfo, pNode );

			// Move back to the point of intersection
			pInfo->m_FinalPos = iPos;

			++pInfo->m_nHits;
			bContinue = true;
		}
		
		if(!bContinue)
			return;
	}

	// Got into a potentially recursive situation and kept hitting stuff.
	// Just put them back at their original position.
	if(g_DebugLevel > 0)
	{
		dsi_ConsolePrint("Error: (point) physics recursed infinitely on a %s",
			request.m_pAbstract->GetObjectClassName(request.m_pObject));
	}
}


void CollideWithWorld
(
	CollideRequest&	request,
	CollideInfo*	pInfo
)
{
	int32 i;
	uint32 curHits;

	pInfo->m_pStandingOn = LTNULL;

	pInfo->m_vForce.Init();
	pInfo->m_VelOffset.Init();
	pInfo->m_nHits = 0;

	// Do point collisions?
	if( request.m_pObject->m_Flags & FLAG_POINTCOLLIDE )
	{
		DoPointCollision( request, pInfo );
		return;
	}


	//1st and 2nd positions of object
	LTVector& P0 = request.m_OriginalPos;
	LTVector& P1 = request.m_NewPos;

	// First, obey the blockers 
	// Note : But only if this isn't a re-try..  This code will never allow the player
	// to penetrate a blocker, but the collision code requires leaving the player 
	// someplace where they might be touching a blocker.
	if (((request.m_pObject->m_Flags2 & FLAG2_PLAYERCOLLIDE) != 0) && (request.m_nRestart == 0))
	{
		LTVector vNewEndPos;
		bool bHitBlocker = g_iWorldBlockerData->CollidePlayer(
				P0, P1, request.m_Dims, 0, &vNewEndPos);
		if (bHitBlocker)
		{
			// Mark a hit..
			++pInfo->m_nHits;
			// Get the new ending position
			P1 = vNewEndPos;
		}
	}

	// Stairstep
	bool bHitStairStep = false;
	LTVector offset(0,0,0);//used for "stair-stepping"

	if( request.m_pObject->m_Flags & FLAG_STAIRSTEP )
	{
		// Get the stair height
		float fStairHeight;
		request.m_pAbstract->GetPhysics()->GetStairHeight(fStairHeight);
		if (fStairHeight < 0.0f)
			fStairHeight = request.m_Dims.y * 0.5f;

		// Adjust by 1/2 unit so it's an inclusive height
		fStairHeight += 0.5f;

		// Use half the stair height since that's the amount the adjustments are made by
		fStairHeight *= 0.5f;
		
		// Adjust the bounding box..
		offset.y = -(request.m_Dims.y - fStairHeight);
		LTVector dims = request.m_Dims;
		request.m_Dims.y = fStairHeight;

		const uint32 nPreStepHits = pInfo->m_nHits;

		//check for stair step
		const bool bHitNonStep = StairStep( offset, pInfo, request, P0, P1 );

		request.m_Dims = dims;//reset back to original value

		if( bHitNonStep )
		{
			// If we hit a poly w/ SURF_NOTASTEP set, then do a full height collision later
			offset.y = 0;
		}
		else
		{
			offset.y = fStairHeight;
			request.m_Dims.y -= fStairHeight;
		}

		// Remember that we hit the stairs, but don't count it as a collision
		bHitStairStep = (pInfo->m_nHits != nPreStepHits);
		pInfo->m_nHits = nPreStepHits;
	}

	const float radius = request.m_Dims.Mag() + 0.01f;

	// Loop around, testing for collisions with anything.
	for( i=0 ; i < MAX_PHYSICS_ITERATIONS ; i++ )
	{
		curHits = pInfo->m_nHits;

		// Test for collisions..
		if( request.m_pObject->m_Flags2 & FLAG2_PLAYERCOLLIDE )
		{
			// Do a cylinder collision
			LTVector vCollideNormal(0.0f, 0.0f, 0.0f);
			bool bCylinderCollide;
			bCylinderCollide = CollideCylinderWithTree( P0, offset, request, pInfo, P1, &vCollideNormal );

			// If we collided with something, re-check the blocker polys
			if (bCylinderCollide || bHitStairStep)
			{
				static IWorldBlockerData::TNormalList aRestrictNormals;
				aRestrictNormals.clear();

				// If we are standing on something, restrict the player to moving along the plane they're standing on
				if (bHitStairStep)
				{
					LTVector vStandingOnNormal = pInfo->m_pStandingOn->GetPlane()->m_Normal;
					aRestrictNormals.push_back(vStandingOnNormal);
					aRestrictNormals.push_back(-vStandingOnNormal);
				}

				// Restrict along the normal we collided against
				aRestrictNormals.push_back(vCollideNormal);

				LTVector vNewEndPos;
				bool bHitBlocker = g_iWorldBlockerData->CollidePlayer(
					P0 + offset, P1 + offset, request.m_Dims, &aRestrictNormals,
					&vNewEndPos);
				if (bHitBlocker)
				{
					// Get the new ending position
					P1 = vNewEndPos - offset;
				}
			}
		}
		else
		{
			ClipBoxIntoTree2(	offset,
								radius,
								pInfo,
								i > 0,
								request,
								P0,
								P1 );
		}

		if( curHits == pInfo->m_nHits )
		{
			// Ok, we finally didn't hit anything, so end.
			break;
		}
	}

	if( MAX_PHYSICS_ITERATIONS == i )
	{
		// Got into a potentially recursive situation and kept hitting stuff.
		// Just put them back at their original position.
		if(g_DebugLevel > 0)
		{
			dsi_ConsolePrint("Error: physics recursed infinitely on a %s",
				request.m_pAbstract->GetObjectClassName(request.m_pObject));
		}

		P1 = P0;
	}

	pInfo->m_FinalPos = P1;

	// If there was a stair collision, and nothing else, increment the hit count
	if (!pInfo->m_nHits && bHitStairStep)
		pInfo->m_nHits++;
}


static bool CalculateCollisionResponse
(
	LTObject *pObj,
	LTVector *pObjectVel, 
	LTVector *pStopPlane,
	LTVector *pVelAdd,
	LTVector *pForce
)
{
	float vDotN;
	LTVector accel;
	LTVector frictionZone;

	// *pVelAdd = -N * (V * N)
	vDotN = pStopPlane->Dot(*pObjectVel);

	if( vDotN < 0.0f )
	{
		*pVelAdd = *pStopPlane * -vDotN;

		// acceleration = *pVelAdd * IMPULSE_TIME_CONSTANT
		// force = acceleration * pObj->m_Mass
		accel = *pVelAdd * IMPULSE_TIME_CONSTANT;
		*pForce = accel * (float)pObj->m_Mass;
		
		// This makes it so the velocity doesn't actually switch all the way 
		// to zero in the direction of the normal, so friction will still
		// be applied and there is a tiny amount of force causing the guy
		// to stick to the floor.
		frictionZone = *pStopPlane * -EXTRA_PENETRATION_ADD;
		*pVelAdd += frictionZone;
		return true;
	}
	else
	{
		// If they weren't moving in the direction of the normal, don't do anything.
		return false;
	}
}


void DoObjectCollisionResponse
(
	CollideRequest&	request,
	CollideInfo*	pInfo,
	const Node*		pNode,
	bool			bUseNodePlane
)
{
	LTVector force, velAdd, objectVel;

	// Setup the collision info...
	// Because we could hit more than one poly, the collision info will contain the last poly hit...
	if( bUseNodePlane )
	{
		request.m_pCollisionInfo->m_Plane = *pNode->GetPlane();
	}

	request.m_pCollisionInfo->m_hObject = (HOBJECT)request.m_pWorldObj;
	
	if( request.m_pWorld )
	{
		request.m_pCollisionInfo->m_hPoly = request.m_pWorld->MakeHPoly(pNode);
	}
	else
	{
		request.m_pCollisionInfo->m_hPoly = INVALID_HPOLY;
	}

	objectVel = request.m_pObject->m_Velocity + pInfo->m_VelOffset;

	// Do the collision response.
	if(	CalculateCollisionResponse( request.m_pObject,
									&objectVel,
									&request.m_pCollisionInfo->m_Plane.m_Normal,
									&velAdd,
									&force ) )
	{
		// Notify them if they want to be notified.
		if(request.m_pObject->m_Flags & FLAG_TOUCH_NOTIFY)
		{
			pInfo->m_vForce += force;
		}

		request.m_pObject->m_InternalFlags |= IFLAG_APPLYPHYSICS;


		// Stop their velocity on this plane!
		pInfo->m_VelOffset += velAdd;
	}
}


void DoInterObjectCollisionResponse(MoveAbstract *pAbstract,
	LTObject *pObj1, LTObject *pObj2, LTVector *pPlane, float fPlaneDist, HPOLY hPoly)
{
	float vDotN[2], forceScale, forceMagSqr, forceMag;
	LTVector oppositeStopPlane, velAdd[2], combinedVelAdd;
	LTVector combinedForce;
	CollisionInfo *pCollisionInfo;

	pCollisionInfo = pAbstract->GetCollisionInfo();

	// Get stopping velocities (if any).
	vDotN[0] = VEC_DOT(*pPlane, pObj1->m_Velocity);
	velAdd[0] = *pPlane * -vDotN[0];

	oppositeStopPlane = -*pPlane;
	vDotN[1] = oppositeStopPlane.Dot(pObj2->m_Velocity);
	velAdd[1] = oppositeStopPlane * -vDotN[1];

	pCollisionInfo->m_Plane.m_Normal = *pPlane;
	pCollisionInfo->m_Plane.m_Dist = fPlaneDist;
	pCollisionInfo->m_hPoly = hPoly;

	// Get combined force (velAdd[0] - velAdd[1] .. ie: if velAdd[1] is in opposite direction,
	// force is greater).
	combinedVelAdd = velAdd[0] - velAdd[1];
	forceScale = IMPULSE_TIME_CONSTANT * (pObj1->m_Mass + pObj2->m_Mass);
	combinedForce = combinedVelAdd * forceScale;

	forceMagSqr = combinedForce.MagSqr();
	forceMag = ltsqrtf(forceMagSqr);

	// Notify the next of kin...
	if(pObj1->m_Flags & FLAG_TOUCH_NOTIFY)
	{
		if(forceMagSqr >= pObj1->m_ForceIgnoreLimitSqr)
		{				
			pAbstract->DoTouchNotify(pObj1, pObj2, velAdd[0], forceMag);
		}
	}

	if(pObj2->m_Flags & FLAG_TOUCH_NOTIFY)
	{
		if(forceMagSqr >= pObj2->m_ForceIgnoreLimitSqr)
		{
			pAbstract->DoTouchNotify(pObj2, pObj1, velAdd[1], -forceMag);
		}
	}


	// Stop their velocities and apply the collision to their acceleration.
	if(vDotN[0] < 0.0f && (pObj1->m_BPriority <= pObj2->m_BPriority))
	{
		pObj1->m_InternalFlags |= IFLAG_APPLYPHYSICS;
		pObj1->m_Velocity += velAdd[0];
	}

	if(vDotN[1] < 0.0f && (pObj2->m_BPriority <= pObj1->m_BPriority))
	{
		pObj2->m_InternalFlags |= IFLAG_APPLYPHYSICS;
		pObj2->m_Velocity += velAdd[1];
	}
}

//EOF
