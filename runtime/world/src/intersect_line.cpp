// This module does line segment intersects in the BSP.

#include "bdefs.h"
#include "de_world.h"
#include "intersect_line.h"


#define INTERSECT_EPSILON	0.01f


struct RayStack
{
	// The node to go into when this gets popped off the stack.
	Node		*m_GoInto, *m_pNode;
	LTVector	m_IPoint, m_Pt2;
};


// Tells if pPt is inside the convex poly.
inline LTBOOL InsideConvex(WorldPoly *pPoly, LTVector *pPt)
{
	LTPlane edgePlane;
	float edgeDot;
	LTVector *pNormal;
	uint32 iCur, iPrev, iEnd;

	// Reject it if it's outside the radius of the poly
	if (pPoly->GetCenter().DistSqr(*pPt) > (pPoly->GetRadius() * pPoly->GetRadius()))
		return LTFALSE;

	pNormal = &pPoly->GetPlane()->m_Normal;
	iEnd  = pPoly->GetNumVertices();
	iPrev = iEnd - 1;

	for(iCur=0; iCur != iEnd; )
	{
		LTVector &currVec = pPoly->GetVertex(iCur);
		LTVector vTemp = pPoly->GetVertex(iPrev) - currVec;
		edgePlane.m_Normal = pNormal->Cross(vTemp);
		edgePlane.m_Normal.Norm();
		edgePlane.m_Dist = edgePlane.m_Normal.Dot(currVec);

		edgeDot = edgePlane.DistTo(*pPt);
		if(edgeDot < -INTERSECT_EPSILON)
			return LTFALSE;

		iPrev = iCur;
		++iCur;
	}

	return LTTRUE;
}

LTBOOL InternalIntersectLineNode(
	const Node *pRoot,
	IntersectRequest *pRequest,
	LTVector point1,
	const LTVector& point2)
{
	LTVector iPoint;
	float intersection_t;
	float dot1, dot2;
	int side1;

	while((pRoot->m_Flags & (NF_IN|NF_OUT)) == 0)
	{
		// Go into the correct side.
		dot1 = pRoot->GetPlane()->DistTo(point1);
		dot2 = pRoot->GetPlane()->DistTo(point2);

		// Handle the segment being entirely on one side of the plane
		if(dot1 > INTERSECT_EPSILON && dot2 > INTERSECT_EPSILON)
		{
			pRoot = pRoot->m_Sides[FrontSide];
		}
		else if(dot1 < -INTERSECT_EPSILON && dot2 < -INTERSECT_EPSILON)
		{
			pRoot = pRoot->m_Sides[BackSide];
		}
		else
		{
			// Ok, it crosses this plane.. go into the side that pFrom is on first.
			if ((dot1 < -INTERSECT_EPSILON) || (dot1 > INTERSECT_EPSILON))
				side1 = (int)(dot1 > 0.0f);
			else
				side1 = (int)(dot2 < 0.0f);

			// Get the difference between the distances
			intersection_t = dot2 - dot1;
			bool bOnPlane = false;
			// Find the point of intersection
			if (intersection_t != 0.0f)
				intersection_t = -dot1 / intersection_t;
			else
				bOnPlane = true;

			if ((dot1 < -INTERSECT_EPSILON) || (dot1 > INTERSECT_EPSILON) || bOnPlane)
			{
				float fBackSideT = intersection_t + INTERSECT_EPSILON;
				fBackSideT = LTMIN(fBackSideT, 1.0f);
				VEC_LERP(iPoint, point1, point2, fBackSideT);
				// Test the side the starting point is on.
				if (InternalIntersectLineNode(
					pRoot->m_Sides[side1], 
					pRequest,
					point1,
					bOnPlane ? point2 : iPoint))
				{
					return LTTRUE;
				}
			}

			// Check for a polygon intersection
			if((side1 == FrontSide) && (pRoot->m_pPoly))
			{
				VEC_LERP(iPoint, point1, point2, intersection_t);
				if(InsideConvex(pRoot->m_pPoly, &iPoint))
				{
					LTBOOL bDone = LTTRUE;

					IntersectQuery* pQuery = pRequest->m_pQuery;
					if (pQuery && pQuery->m_PolyFilterFn && pRequest->m_pWorldBsp)
					{
						if (!pQuery->m_PolyFilterFn(pRequest->m_pWorldBsp->MakeHPoly(pRoot), pQuery->m_pUserData))
						{
							bDone = LTFALSE;
						}
					}

					if (bDone)
					{
						// Congratulations, we have a winner!
						pRequest->m_pNodeHit = pRoot;
						*pRequest->m_pIPos = iPoint;
						return LTTRUE;
					}
				}
			}
			// Jump out if the ray doesn't go to the "other" side
			else if (intersection_t > (1.0f - INTERSECT_EPSILON))
				break;

			// Clip the segment to the plane
			float fFontSideT = intersection_t - INTERSECT_EPSILON;
			fFontSideT = LTMAX(fFontSideT, 0.0f);
			VEC_LERP(iPoint, point1, point2, fFontSideT);
			point1 = iPoint;

			// Go into the other side.
			pRoot = pRoot->m_Sides[!side1];
		}
	}

	return LTFALSE;
}


LTBOOL IntersectLineNode(
	const Node *pRoot,
	IntersectRequest *pRequest)
{
	return InternalIntersectLineNode(
		pRoot,
		pRequest,
		*pRequest->m_pPoints[0],
		*pRequest->m_pPoints[1]
		);
}


const Node* IntersectLine(const Node *pRoot, LTVector *pPoint1, LTVector *pPoint2, LTVector *pIPos, LTPlane *pIPlane)
{
	IntersectRequest req;

	req.m_pPoints[0] = pPoint1;
	req.m_pPoints[1] = pPoint2;
	req.m_pIPos = pIPos;

	if( IntersectLineNode( pRoot, &req ) )
	{
		*pIPlane = *req.m_pNodeHit->GetPlane();
		return req.m_pNodeHit;
	}

	return LTNULL;
}
