
// For some reason we can't use certain optimizations in this module or 
// else the raycasting is all screwed up.  This module also can't use
// precompiled headers or else these pragmas don't take effect.
#pragma optimize("", off)
#pragma optimize("as", on)

#include "bdefs.h"
#include "de_world.h"
#include "objectmgr.h"
#include "geometry.h"
#include "syscounter.h"
#include "intersect_line.h"

#define INTERSECT_EPSILON	0.01f

//user callback structure
struct	SSweptSphereUserCBData
{
	//the start and end vector of the sphere
	LTVector			m_vStart;
	LTVector			m_vEnd;

	//the normalized direction vector
	LTVector			m_vUnitDir;

	//the unnormalized direction vector
	LTVector			m_vDir;

	//the radius of the sphere
	float				m_fRadius;

	//the radius squared
	float				m_fRadiusSqr;

	//the closest hit parameterized along m_vDir
	float				m_fIntersection;

	//whether or not an intersection has been hit
	bool				m_bHit;

	//the normal of the closest intersection
	LTVector			m_vHitNormal;

	//a bounding sphere that encloses the entire sweep
	LTVector			m_vBSphereCenter;
	float				m_fBSphereRadius;
};


uint32 g_Ticks_IntersectSweptSphere, g_nIntersectSweptSphereCalls;

// Tells if pPt is inside the convex poly.
inline bool InsideConvex(const WorldPoly *pPoly, const LTVector& Pt)
{
	LTPlane edgePlane;
	float edgeDot;
	uint32 iCur, iPrev, iEnd;

	const LTVector& Normal = pPoly->GetPlane()->m_Normal;
	iEnd  = pPoly->GetNumVertices();
	iPrev = iEnd - 1;

	LTVector vTemp;

	for(iCur=0; iCur != iEnd; )
	{
		const LTVector &currVec = pPoly->GetVertex(iCur);

		vTemp = pPoly->GetVertex(iPrev) - currVec;
		edgePlane.m_Normal = Normal.Cross(vTemp);
		edgePlane.m_Normal.Norm();

		edgePlane.m_Dist = edgePlane.m_Normal.Dot(currVec);

		edgeDot = edgePlane.DistTo(Pt);
		if(edgeDot < -INTERSECT_EPSILON)
			return false;

		iPrev = iCur;
		++iCur;
	}

	return true;
}

static bool SweptSphereToEdge(const LTVector& vLinePt, const LTVector& vLinePt2, const LTVector& vStart,
							  const LTVector& vEnd, const LTVector& vDir, float fRadius, float* pfNearestHit)
{
	//This equation was derived through x = P1 + T(P2 - P1) for the sphere line
	//and the vector from the line start to the point is v = x - s. Then the distance
	//from the line to that ray can be found by d = v - L(V . L), and when |d| = r,
	//then we have our solution (note that in most cases this won't occur, and we determine
	//that by checking the polynomial values)

	//build up line information
	LTVector vLineDir = vLinePt2 - vLinePt;
	float fLineLen = vLineDir.Mag();
	vLineDir /= fLineLen;

	LTVector vF = vDir + vLineDir * vLineDir.Dot(vDir);
	LTVector vU = vStart - vLinePt - vLineDir * (vLineDir.Dot(vStart) + vLineDir.Dot(vLinePt));

	//alright, we now have the vectors of the form |TF + U| = r, so we rearrange it so that
	//|TF + U|^2 - r^2 = 0, and expand for the components and solve
	float fA = vF.MagSqr();
	float fB = vF.Dot(vU);
	float fC = vU.MagSqr() - fRadius * fRadius;

	//check for degenerate cases
	float fDescriminant = fB * fB - 4 * fA * fC;

	if((fDescriminant < 0.0f) || (fA < 0.001f))
	{
		//no hit
		return false;
	}

	//we have a hit, figure out what it is
	fDescriminant = sqrtf(fDescriminant);
	float f2A = fA * 2.0f;

	float fAnswer1 = (-fB + fDescriminant) / f2A;
	float fAnswer2 = (-fB - fDescriminant) / f2A;

	//we need to pick the smaller of the two time steps
	float fT = LTMIN(fAnswer1, fAnswer2);

	//don't even bother though if the nearest hit is already shorter
	if(fT >= *pfNearestHit)
		return false;

	//alright, now the tricky part, did we actually hit the line or just the endpoint, we first need
	//to determine where on the line we hit
	float fDistOnLine = vLineDir.Dot(vStart + fT * vDir - vLinePt);

	//see if we are outside of the line
	if((fDistOnLine < 0.0f) || (fDistOnLine > fLineLen))
		return false;

	//we are on the line, fill out the information and return success
	*pfNearestHit = fT;
	return true;
}

//handles collisions between all the edges of the polygon and a swept sphere
static bool SweptSphereToPolyEdges(	const WorldPoly* pPoly, const LTVector& vStart, const LTVector& vEnd, 
									const LTVector& vDir, float fRadius, float* pfNearestHit)
{
	uint32 nNumVerts = pPoly->GetNumVertices();

	LTVector vPrevVert = pPoly->GetVertex(nNumVerts - 1);

	bool bHit = false;

	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		const LTVector &vCurrVert = pPoly->GetVertex(nCurrVert);

		if(SweptSphereToEdge(vPrevVert, vCurrVert, vStart, vEnd, vDir, fRadius, pfNearestHit))
			bHit = true;

		vPrevVert = vCurrVert;
	}

	return bHit;
}

//handles collision with a single point in space with the sphere
static bool SweptSphereToPoint(const LTVector& vPt, const LTVector& vStart, const LTVector& vEnd, 
							   const LTVector& vDir, float fRadius, float* pfNearestHit)
{
	//we are going to use pythagorean in order to determine where to move. We basically create
	//two right triangles, one going from start center to the point with the adjacent edge
	//going along the direciton vector. We then form another triangle with the adjacent along
	//the direction and has a hypotenuse of length radius. We can then subtract this base length
	//from the other base length to get the T value.

	//find the direction to the point
	LTVector vStartToPt = vPt - vStart;

	//remove the direction parallel to the dir vector
	float fAmountParallel = vStartToPt.Dot(vDir) / vDir.MagSqr();

	//now remove the parallel portion and find the length of the vector to get the height of both
	//triangles
	float fTriHeightSqr = (vStartToPt - (vDir * fAmountParallel)).MagSqr();

	//now we can find the length of the base of the second triangle
	float fSecondTriBaseSqr = fRadius * fRadius - fTriHeightSqr;

	//our first early out, if the base is negative we can't hit it, it is behind us
	if(fSecondTriBaseSqr < 0.0f)
		return false;

	//now find the length of the first triangle's base
	float fFirstTriBaseSqr = vStartToPt.MagSqr() - fTriHeightSqr;

	//we can early out if this is negative as well
	if(fFirstTriBaseSqr < 0.0f)
		return false;

	//alright, now find the actual T value
	float fT = sqrtf(fFirstTriBaseSqr) - sqrtf(fSecondTriBaseSqr);

	//now store the T if it is valid
	if(fT > 0.0f)
	{
		if(fT < *pfNearestHit)
			*pfNearestHit = fT;
		return true;
	}

	//T was negative
	return false;
}

//handle colliding the swept sphere with every vertex of the polygon
static bool SweptSphereToPolyVertices(const WorldPoly* pPoly, const LTVector& vStart, const LTVector& vEnd, 
									  const LTVector& vDir, float fRadius, float* pfNearestHit)
{
	//just run through all vertices and see if it hits any of them
	uint32 nNumVerts = pPoly->GetNumVertices();
	bool bHit = false;

	for(uint32 nCurrVert = 0; nCurrVert < nNumVerts; nCurrVert++)
	{
		const LTVector &vCurrVert = pPoly->GetVertex(nCurrVert);

		if(SweptSphereToPoint(vCurrVert, vStart, vEnd, vDir, fRadius, pfNearestHit))
			bHit = true;
	}

	return bHit;
}


static bool SweptSphereToPoly(const WorldPoly* pPoly, const LTVector& vStart, const LTVector& vEnd,
					   const LTVector& vDir, float fRadius, float fToPlane, float* pfNearestHit)
{
	//cache the plane pointer
	const LTPlane* pPlane = pPoly->GetPlane();

	//alright, first off, lets see if we hit the inside of the polygon at all, we do this
	//by moving the polygon to where it hits the plane, finding the point of intersection
	//on the plane, and then determining if that is within the polygon
	LTVector vPtInPlane = vStart + vDir * fToPlane - pPlane->m_Normal * fRadius;

	//alright, now we need to go through and determine if that point lies within the polygon, if
	//so it is our hit point, otherwise we need to test the vertices and edges
	if(InsideConvex(pPoly, vPtInPlane))
	{
		//alright, we have our hit point
		if(fToPlane < *pfNearestHit)
			*pfNearestHit = fToPlane;

		return true;
	}

	//alright, so we didn't hit inside of the polygon, that means we need to check the edges and
	//vertices of the polygon, we check the edges first
	if(SweptSphereToPolyEdges(pPoly, vStart, vEnd, vDir, fRadius, pfNearestHit))
		return true;

	//alright, we didn't hit any edges either, so now we need to check the vertices
	if(SweptSphereToPolyVertices(pPoly, vStart, vEnd, vDir, fRadius, pfNearestHit))
		return true;

	return false;
}

bool SweptSphereToBSP_R(const Node* pRoot, const LTVector& vStart, const LTVector& vEnd, 
						const LTVector& vDir, float fRadius, const LTVector& vBSphereCenter,
						float fBSphereRadius, float* pfNearestHit)
{
	float fBSphereDot;
	uint32 side1;
	bool bHit = false;

	while((pRoot->m_Flags & (NF_IN|NF_OUT)) == 0)
	{
		// determine the distance from the plane to the sphere
		fBSphereDot = pRoot->GetPlane()->DistTo(vBSphereCenter);

		// Handle the segment being entirely on one side of the plane
		if(fBSphereDot > fBSphereRadius)
		{
			pRoot = pRoot->m_Sides[FrontSide];
		}
		else if(fBSphereDot < -fBSphereRadius)
		{
			pRoot = pRoot->m_Sides[BackSide];
		}
		else
		{
			const LTPlane& Plane = *pRoot->GetPlane();

			//the bounding sphere thinks that we are crossing this plane. It is probably correct
			//but lets find out if we do indeed cross the plane, and if so, what the scalar is in
			//order to move the point to that plane
			float fPlaneToDirAngle = Plane.m_Normal.Dot(vDir);

			float fToPlane = 0.0f;

			if(fabsf(fPlaneToDirAngle) < 0.001f)
			{
				//alright, we are moving perpindicular to this plane, we can use the bounding sphere
				//dot product though since it is the same distance to do a check
				if(fBSphereDot > fRadius)
				{
					pRoot = pRoot->m_Sides[FrontSide];
					continue;
				}
				else if(fBSphereDot < -fRadius)
				{
					pRoot = pRoot->m_Sides[BackSide];
					continue;
				}
			}
			else
			{
				float fNormalDotStart = Plane.m_Normal.Dot(vStart);

				//we aren't moving perp to the plane, so we can find the intersection
				fToPlane = (Plane.m_Dist + fRadius - fNormalDotStart + INTERSECT_EPSILON) / fPlaneToDirAngle;
			}

			//alright, if we weren't intersecting this plane we would have bailed out by now, so first
			//lets recurse on the side that the start is on
			side1 = (Plane.DistTo(vStart) > 0.0f) ? 1 : 0;

			//recurse
			if(SweptSphereToBSP_R(pRoot->m_Sides[side1], vStart, vEnd, vDir, fRadius, vBSphereCenter, fBSphereRadius, pfNearestHit))
				bHit = true;

			//unfortunately we can't early out if something was hit on the front side since we could
			//still hit something on the back side sooner.

			//lets see if we perhaps hit the polygon that lies on this node, but we only need to
			//worry about it if the polygon is facing in a direction that we could move through
			//on the front side
			if(	(fPlaneToDirAngle < 0.001f) &&
				(pRoot->m_pPoly->GetSurface()->m_Flags & SURF_SOLID) &&
				(fToPlane > -0.01f) &&
				(fToPlane < *pfNearestHit)) 
			{
				//ok, it is on the front, and worth checking for a hit, lets make sure it is close enough
				float fDistToPolySqr = (vBSphereCenter - pRoot->m_pPoly->GetCenter()).MagSqr();
				float fMaxDistToPoly = fBSphereRadius + pRoot->m_pPoly->GetRadius();
				fMaxDistToPoly *= fMaxDistToPoly;

				if(fDistToPolySqr <= fMaxDistToPoly)
				{
					//we have a potential intersection, lets find the real intersection point
					if(SweptSphereToPoly(pRoot->m_pPoly, vStart, vEnd, vDir, fRadius, fToPlane, pfNearestHit))
						bHit = true;
				}
			}
			
			//alright, now we can try the back side
			pRoot = pRoot->m_Sides[(side1 + 1) % 2];
		}
	}

	return bHit;
}

//this function given a world model will filter the swept sphere through its BSP and handle collisions
//with each polygon, determining the closest intersection
static void SweptSphereToWorldModel(WorldModelInstance* pWM, SSweptSphereUserCBData* pCBData)
{
	//alright, we need to convert our sphere into the world model space
	LTVector vStart, vEnd;
    MatVMul_H(&vStart, &pWM->m_BackTransform, &pCBData->m_vStart);
    MatVMul_H(&vEnd, &pWM->m_BackTransform, &pCBData->m_vEnd);

	//also make sure to evaluate dependant variables
	LTVector vDir			= vEnd - vStart;
	LTVector vBSpherePos	= (vEnd + vStart) * 0.5f;

	//filter the swept sphere through the BSP of the world model
	if(SweptSphereToBSP_R(	pWM->m_pOriginalBsp->GetRootNode(), vStart, vEnd, vDir, pCBData->m_fRadius, 
							vBSpherePos, pCBData->m_fBSphereRadius + INTERSECT_EPSILON, 
							&pCBData->m_fIntersection))
	{
		pCBData->m_bHit = true;
	}
}


//the callback that is called per potentially intersecting object and is used to filter out anything
//that is nonsolid, anything that isn't a world model, and anything that is too far away to intersect
static void SweptSphereToObjectCB(WorldTreeObj *pWTObj, void *pUser)
{
	LTObject* pObj = (LTObject*)pWTObj;

	//only bother with solid world models
	if((pObj->m_ObjectType == OT_WORLDMODEL) && (pObj->m_Flags & FLAG_SOLID))
	{
		//get the user data
		SSweptSphereUserCBData* pCBData = (SSweptSphereUserCBData*)pUser;

		//alright, we have a main world or world model, so lets try and collide, first by doing a
		//bounding sphere check
		float fDistSqr		= (pCBData->m_vBSphereCenter - pObj->GetPos()).MagSqr();
		float fMaxDistSqr	= pCBData->m_fBSphereRadius + pObj->m_Radius;
		fMaxDistSqr *= fMaxDistSqr;

		if(fDistSqr > fMaxDistSqr)
		{
			//too far out
			return;
		}

		//alright, we have a potentially colliding world, lets do the full expensive test...
		SweptSphereToWorldModel(pObj->ToWorldModel(), pCBData);
	}
}

       
bool i_IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal, WorldTree *pWorldTree)
{
	//sanity checks...
	assert(pWorldTree);

	//handle updating performance counters
    ++g_nIntersectSweptSphereCalls;
	CountAdder cTicks_Intersect(&g_Ticks_IntersectSweptSphere);

	//handle degenerate cases
	if(fRadius < 0.1f)
	{
		//such a small sphere, should just use ray intersection

		//TODO: Add this support, until then just fall through to normal
	}

	//first off we need to build up a bounding box and determine what objects we are hitting
	LTVector vMin, vMax;
	VEC_MIN(vMin,	LTVector(vStart.x - fRadius, vStart.y - fRadius, vStart.z - fRadius),
					LTVector(vEnd.x - fRadius, vEnd.y - fRadius, vEnd.z - fRadius));

	VEC_MAX(vMax,	LTVector(vStart.x + fRadius, vStart.y + fRadius, vStart.z + fRadius),
					LTVector(vEnd.x + fRadius, vEnd.y + fRadius, vEnd.z + fRadius));

	//fill out the user callback information
	//user callback structure
	SSweptSphereUserCBData		UserData;

	UserData.m_vStart			= vStart;
	UserData.m_vEnd				= vEnd;
	UserData.m_vDir				= (vEnd - vStart);
	UserData.m_vUnitDir			= UserData.m_vDir / UserData.m_vDir.Mag();
	UserData.m_fRadius			= fRadius;
	UserData.m_fRadiusSqr		= fRadius * fRadius;
	UserData.m_fIntersection	= 1.0f;
	UserData.m_bHit				= false;
	UserData.m_vBSphereCenter	= (vStart + vEnd) * 0.5f;
	UserData.m_fBSphereRadius	= UserData.m_vDir.Mag() * 0.5f + fRadius;

	//now we need to get all objects in the box, and for any that are world models, find intersecting
	//polygons
	pWorldTree->FindObjectsInBox(&vMin, &vMin, SweptSphereToObjectCB, &UserData);

	//see if we hit anything
	if(UserData.m_bHit)
	{
		//we hit
		vPos	= UserData.m_vStart + UserData.m_vDir * UserData.m_fIntersection;
		vNormal = UserData.m_vHitNormal;

		return true;
	}
	
	//no hit
	vPos = vEnd;
	vNormal.Init();

	return false;
}

