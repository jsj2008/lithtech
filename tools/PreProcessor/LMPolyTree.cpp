//////////////////////////////////////////////////////////////////////////////
// Lighmap-specific polygon intersection tree

#include "bdefs.h"

#include "lmpolytree.h"

#include "prepoly.h"
#include "processing.h"
#include <float.h>

#define PLANE_EPSILON 0.001f

// CLMPoly implementation helper function for finding a line segment's point of intersection
PolySide CalcPlaneIntersect(const LTPlane &cPlane, const LTVector &vOrigin, const LTVector &vDest, LTVector *pIntersectPt)
{
	// Get the distances for each point
	float fOriginDist = cPlane.DistTo(vOrigin);
	float fDestDist = cPlane.DistTo(vDest);

	// Check for non-intersection  (Note : Line-On-Plane does not return intersection)
	if ((fOriginDist > -PLANE_EPSILON) && (fDestDist > -PLANE_EPSILON))
		return FrontSide;
	else if ((fOriginDist < PLANE_EPSILON) && (fDestDist < PLANE_EPSILON))
		return BackSide;

	// Calc the intersection point
	float fDistDiff = fOriginDist - fDestDist;
	ASSERT(fabsf(fDistDiff) > PLANE_EPSILON); // Dunno quite how the difference would be small...

	float fIntersectTime = fOriginDist / fDistDiff;
	fIntersectTime = LTCLAMP(fIntersectTime, 0.0f, 1.0f);
	VEC_LERP(*pIntersectPt, vOrigin, vDest, fIntersectTime);
	return Intersect;
}

//////////////////////////////////////////////////////////////////////////////
// CLMPoly implementation

bool CLMPoly::IntersectSeg(const LTVector &vOrigin, const LTVector &vDest, LTVector *pIntersectPt) const
{
	// Get the plane intersection of the segment
	PolySide eSide = CalcPlaneIntersect(m_aPlanes.front(), vOrigin, vDest, pIntersectPt);
	if (eSide != Intersect)
		return false;

	// Check the edge planes for the intersection being on the front side of the edge plane
	TPlaneList::const_iterator iCurPlane = m_aPlanes.begin() + 1;
	for (; iCurPlane != m_aPlanes.end(); ++iCurPlane)
	{
		float fIntersectDist = iCurPlane->DistTo(*pIntersectPt);
		if (fIntersectDist > PLANE_EPSILON)
			return false;
	}

	// If it got here, it was on all back-sides
	return true;
}

void CLMPoly::CopyPoly(const CPrePoly *pPoly, const CPrePoly *pOriginalPoly)
{
	// Remember where we came from
	m_pPrePoly = pOriginalPoly;

	// Add the root poly plane
	m_aPlanes.reserve(pPoly->NumVerts() + 1);
	m_aPlanes.push_back(*(pPoly->GetPlane()));

	// Initialize the previous point variable
	LTVector vPrevPt = pPoly->Pt(pPoly->NumVerts() - 1);
	// Start the AABB
	m_cAABB = vPrevPt;
	for (uint nCurPlane = 0; nCurPlane < pPoly->NumVerts(); ++nCurPlane)
	{
		// Get the next point
		const LTVector &vNextPt = pPoly->Pt(nCurPlane);
		// Calculate the edge direction vector
		LTVector vEdge = vNextPt - vPrevPt;
		float fEdgeMagSqr = vEdge.MagSqr();
		if (fEdgeMagSqr < 0.0001f)
			continue;
		vEdge /= sqrtf(fEdgeMagSqr);
		// Create the edge plane
		LTPlane cEdgePlane(pPoly->GetPlane()->m_Normal.Cross(vEdge), vNextPt);
		// Add it to the list
		m_aPlanes.push_back(cEdgePlane);
		// Extend the bounds
		m_cAABB += vNextPt;
		// Remember this vertex for the next loop
		vPrevPt = vNextPt;
	}

	// Expand the bounds a bit (fudge factor..)
	m_cAABB.m_vMin -= LTVector(1.0f, 1.0f, 1.0f);
	m_cAABB.m_vMax += LTVector(1.0f, 1.0f, 1.0f);
}

//////////////////////////////////////////////////////////////////////////////
// CLMPolyTree implementation

// AABB/Line Segment intersection routine from the physics code, optimized for use in the BBox tree
bool CLMPolyTree::Intersect_AABB_LineSeg(const CAABB &cAABB, const LTVector &vOrigin, const LTVector &vDest)
{
	//ALGORITHM:  Use the separating axis theorem to
	//see if the line segment and the box intersect. A
	//line segment is a degenerate OBB.
	//NOTE:  the multiplies by 0.5 cancel out

	const LTVector vOffset = vDest - vOrigin;
	const LTVector vAbsOffset(fabsf(vOffset.x), fabsf(vOffset.y), fabsf(vOffset.z));
	const LTVector vSpan = vDest + vOrigin;
	const LTVector T = vSpan - (cAABB.m_vMax + cAABB.m_vMin);//translation
	const LTVector E = cAABB.Size();//Extents

		//NOTE:  dropping out early with else/if
		//statements doesn't seem to speed up
		//this routine at all

		//do any of the principal axes form a separating axis?
		if( (fabsf(T.x) > vAbsOffset.x + E.x)
			||
			(fabsf(T.y) > vAbsOffset.y + E.y)
			||
			(fabsf(T.z) > vAbsOffset.z + E.z) )
		{
			return false;
		}

		//do l.Cross(x), l.Cross(y), or l.Cross(z)
		//form a separating axis?
		if( (fabsf(T.y*vOffset.z - T.z*vOffset.y) > E.y*vAbsOffset.z + E.z*vAbsOffset.y)
			||
			(fabsf(T.z*vOffset.x - T.x*vOffset.z) > E.x*vAbsOffset.z + E.z*vAbsOffset.x)
			||
			(fabsf(T.x*vOffset.y - T.y*vOffset.x) > E.x*vAbsOffset.y + E.y*vAbsOffset.x)
			)
		{
			return false;
		}

	return true;
}

// Split the bounds into left/right portions, based on which dimension is largest
uint CLMPolyTree::SplitAABB(const CAABB &cCurAABB, CAABB *pLeft, CAABB *pRight)
{
	LTVector vSize = cCurAABB.Size();
	uint nAxis = 0;
	if (vSize[1] > vSize[nAxis])
		nAxis = 1;
	if (vSize[2] > vSize[nAxis])
		nAxis = 2;
	float fNewWidth = vSize[nAxis] * 0.5f;
	*pLeft = cCurAABB;
	pLeft->m_vMax[nAxis] = cCurAABB.m_vMin[nAxis] + fNewWidth;
	*pRight = cCurAABB;
	pRight->m_vMin[nAxis] = pLeft->m_vMax[nAxis];

	return nAxis;
}

CLMPolyTree::ESplitLineSegResult CLMPolyTree::SplitLineSeg(
	const LTVector &vOrigin, const LTVector &vDest, 
	uint nAxis, float fSplit,
	LTVector *pLeftOrigin, LTVector *pLeftDest,
	LTVector *pRightOrigin, LTVector *pRightDest)
{
	bool bOriginLeft = vOrigin[nAxis] <= fSplit;
	bool bDestLeft = vDest[nAxis] <= fSplit;
	if (bOriginLeft == bDestLeft)
	{
		if (bOriginLeft)
		{
			*pLeftOrigin = vOrigin;
			*pLeftDest = vDest;
			return eSLS_Left;
		}
		else
		{
			*pRightOrigin = vOrigin;
			*pRightDest = vDest;
			return eSLS_Right;
		}
	}

	float fTime = (fSplit - vOrigin[nAxis]) / (vDest[nAxis] - vOrigin[nAxis]);
	LTVector vIntersect;
	// Note : This could avoid setting nAxis by using a loop, but it's faster to just
	// do the extra calc and avoid all the compares
	VEC_LERP(vIntersect, vOrigin, vDest, fTime);
	if (!bOriginLeft)
		std::swap(pLeftOrigin, pRightDest);
	*pLeftOrigin = vOrigin;
	*pLeftDest = vIntersect;
	*pRightOrigin = vIntersect;
	*pRightDest = vDest;
	return eSLS_Intersect;
}

bool CLMPolyTree::IntersectNodePolys(const SLMPolyTreeNode &sNode, const LTVector &vOrigin, const LTVector &vDest, const SIntersectData &sData, uint *pPolyHit) const
{
	if (!sNode.m_nPolyCount)
		return false;

	// Check the polys
	TPolyList::const_iterator iCurPoly = m_aPolys.begin() + sNode.m_nFirstPoly;
	TPolyList::const_iterator iEndPoly = iCurPoly + sNode.m_nPolyCount;

	for(; iCurPoly != iEndPoly; ++iCurPoly)
	{
		// Check for an intersection on the poly
		if (iCurPoly->IntersectSeg(vOrigin, vDest, sData.m_pResultPt))
		{
			if (pPolyHit)
				*pPolyHit = iCurPoly - m_aPolys.begin();
			return true;
		}
	}

	// No intersection on this node
	return false;
}

bool CLMPolyTree::InternalIntersectSeg(uint nNodeIndex, const CAABB &cBounds, const CLMPolyTree::SIntersectData &sData, LTVector vBoxOrigin, LTVector vBoxDest) const
{
	CAABB cCurAABB = cBounds;
	for(;;) // Effectively "while (nNodeIndex != k_InvalidNode)"
	{
		const SLMPolyTreeNode &cCurNode = m_aNodes[nNodeIndex];

		// Do we hit any of the polys?
		uint nPolyHit;
		if (IntersectNodePolys(cCurNode, vBoxOrigin, vBoxDest, sData, &nPolyHit))
		{
			// Remember which poly we hit
			m_nLastPolyHit = nPolyHit;
			return true;
		}

		// Check the children of this node
		if (cCurNode.m_nLeftNode == k_InvalidNode)
			return false;

		// Get the new bounding boxes
		CAABB cLeftAABB, cRightAABB;
		uint nSplitAxis = SplitAABB(cCurAABB, &cLeftAABB, &cRightAABB);
		// Split the line segment
		ESplitLineSegResult eSplitResult;
		LTVector vLeftOrigin, vLeftDest, vRightOrigin, vRightDest;
		eSplitResult = SplitLineSeg(
			vBoxOrigin, vBoxDest, nSplitAxis, cLeftAABB.m_vMax[nSplitAxis],
			&vLeftOrigin, &vLeftDest, &vRightOrigin, &vRightDest);
		// Determine our walking behavior based on the result
		switch (eSplitResult)
		{
			case eSLS_Left : 
			{
				vBoxOrigin = vLeftOrigin;
				vBoxDest = vLeftDest;
				nNodeIndex = cCurNode.m_nLeftNode;
				cCurAABB = cLeftAABB;
				break;
			}
			case eSLS_Right :
			{
				vBoxOrigin = vRightOrigin;
				vBoxDest = vRightDest;
				nNodeIndex = cCurNode.m_nLeftNode + 1;
				cCurAABB = cRightAABB;
				break;
			}
			case eSLS_Intersect :
			{
				// Did we hit the left side?
				if (InternalIntersectSeg(cCurNode.m_nLeftNode, cLeftAABB, sData, vLeftOrigin, vLeftDest))
				{
					return true;
				}

				// Iterate to the right
				nNodeIndex = cCurNode.m_nLeftNode + 1;
				cCurAABB = cRightAABB;
				break;
			}
		}
	}
}

void ClipLineSegFront(const CAABB &cBounds, LTVector *pOrigin, const LTVector &vDest)
{
	float fMaxT[3];
	fMaxT[0] = fMaxT[1] = fMaxT[2] = -1;
	for (uint nLoop = 0; nLoop < 3; ++nLoop)
	{
		if ((*pOrigin)[nLoop] < cBounds.m_vMin[nLoop])
			fMaxT[nLoop] = (cBounds.m_vMin[nLoop] - (*pOrigin)[nLoop]) / (vDest[nLoop] - (*pOrigin)[nLoop]);
		else if ((*pOrigin)[nLoop] > cBounds.m_vMax[nLoop])
			fMaxT[nLoop] = ((*pOrigin)[nLoop] - cBounds.m_vMax[nLoop]) / ((*pOrigin)[nLoop] - vDest[nLoop]);
	}
	uint nChooseT = 0;
	if (fMaxT[1] > fMaxT[0])
		nChooseT = 1;
	if (fMaxT[2] > fMaxT[nChooseT])
		nChooseT = 2;
	VEC_LERP(*pOrigin, *pOrigin, vDest, fMaxT[nChooseT]);
}

bool CLMPolyTree::ClipLineSeg(const CAABB &cBounds, const LTVector &vOrigin, const LTVector &vDest, LTVector *pResultOrigin, LTVector *pResultDest)
{
	// Make sure it actually intersects the bounding box...
	if (!Intersect_AABB_LineSeg(cBounds, vOrigin, vDest))
		return false;

	// Get the point states
	bool bOriginInside = cBounds.Intersects(vOrigin);
	bool bDestInside = cBounds.Intersects(vDest);
	// Start out with the original values as the results
	*pResultOrigin = vOrigin;
	*pResultDest = vDest;
	// If it's already inside, we're done
	if (bOriginInside == bDestInside)
		return true;
	// Clip the origin
	if (!bOriginInside)
	{
		ClipLineSegFront(cBounds, pResultOrigin, *pResultDest);
	}
	// Clip the dest
	if (!bDestInside)
	{
		ClipLineSegFront(cBounds, pResultDest, *pResultOrigin);
	}
	return true;
}

bool CLMPolyTree::IntersectSeg(const LTVector &vOrigin, const LTVector &vDest, LTVector *pIntersectPt) const
{
	// No intersections allowed if there's no geometry
	if (m_aNodes.empty())
		return false;

	// Set up the intersection structure
	SIntersectData sData;
	sData.m_vOrigin = vOrigin;
	sData.m_vDest = vDest;
	sData.m_pResultPt = pIntersectPt;
	sData.m_vOffset = vDest - vOrigin;

	// Try the most recent node we intersected
	if (m_nLastPolyHit != k_InvalidNode)
	{
		if (m_aPolys[m_nLastPolyHit].IntersectSeg(vOrigin, vDest, pIntersectPt))
		{
			// Early-out
			return true;
		}
	}

	LTVector vClippedOrigin, vClippedDest;
	if (!ClipLineSeg(m_cBounds, sData.m_vOrigin, sData.m_vDest, &vClippedOrigin, &vClippedDest))
		return false;

	// Do the full intersection test
	return InternalIntersectSeg(0, m_cBounds, sData, vClippedOrigin, vClippedDest);
}

void CLMPolyTree::AddPoly(const CPrePoly *pPoly, const CPrePoly *pOriginalPoly)
{
	// Add it to the poly list
	m_aPolys.push_back(CLMPoly());
	m_aPolys.back().CopyPoly(pPoly, pOriginalPoly);
	// Extend the world bounds
	if (m_aPolys.size() == 1)
		m_cBounds = m_aPolys.back().GetBounds();
	else
		m_cBounds += m_aPolys.back().GetBounds();
}

void CLMPolyTree::SplitNode(uint nIndex, const CAABB &cBounds)
{
	CAABB cCurBounds = cBounds;

	for (;;) // Loop for eliminating tail recursion
	{
		// Get the node we're operating on
		SLMPolyTreeNode &sNode = m_aNodes[nIndex];

		ASSERT(sNode.m_nPolyCount != 0);

		// Split up our bounds
		CAABB cLeftBounds, cRightBounds;
		SplitAABB(cCurBounds, &cLeftBounds, &cRightBounds);
		// Get a box representing the seperation
		CAABB cMiddle = cLeftBounds;
		cMiddle.Intersection(cRightBounds);

		TPolyList::iterator iBlockBegin = m_aPolys.begin() + sNode.m_nFirstPoly;
		TPolyList::iterator iBlockEnd = iBlockBegin + sNode.m_nPolyCount;
		ASSERT((sNode.m_nFirstPoly + sNode.m_nPolyCount) <= m_aPolys.size());
		uint nPolysRemaining = sNode.m_nPolyCount;
		uint nTotalPolys = sNode.m_nPolyCount;
		// Sort the polys based on intersection with the middle to determine
		// the set of polys that are "on" this node
		TPolyList::iterator iCurPoly = iBlockBegin;
		TPolyList::iterator iLastPoly = iBlockEnd - 1;
		while (iCurPoly != iLastPoly)
		{
			if (cMiddle.Intersects(iCurPoly->GetBounds()))
			{
				++iCurPoly;
			}
			else
			{
				iCurPoly->Swap(*iLastPoly);
				--iLastPoly;
				ASSERT(sNode.m_nPolyCount);
				--sNode.m_nPolyCount;
			}
		}
		// Remember to check the one in the middle
		if (!cMiddle.Intersects(iCurPoly->GetBounds()))
		{
			ASSERT(sNode.m_nPolyCount);
			--sNode.m_nPolyCount;
			--iLastPoly;
		}

		// Remember how many we've got left
		nPolysRemaining -= sNode.m_nPolyCount;
		// If all of the polys crossed the middle, don't perform a split
		if (!nPolysRemaining)
			return;

		// Point at the new remaining block
		iBlockBegin = iLastPoly + 1;

		// Point at the children
		sNode.m_nLeftNode = m_aNodes.size();

		// Create the children
		SLMPolyTreeNode sParent = sNode;
		// Note : sNode is invalid after this line
		m_aNodes.resize(m_aNodes.size() + 2);
		SLMPolyTreeNode &sLeft = m_aNodes[sParent.m_nLeftNode];
		SLMPolyTreeNode &sRight = m_aNodes[sParent.m_nLeftNode + 1];
		sLeft.m_nLeftNode = sRight.m_nLeftNode = k_InvalidNode;
		sLeft.m_nFirstPoly = sParent.m_nFirstPoly + sParent.m_nPolyCount;
		sLeft.m_nPolyCount = nPolysRemaining;
		sRight.m_nFirstPoly = sLeft.m_nFirstPoly + sLeft.m_nPolyCount;
		sRight.m_nPolyCount = 0;

		// Sort the polys from the left node into the right node
		iCurPoly = iBlockBegin;
		iLastPoly = iBlockEnd - 1;
		while (iCurPoly != iLastPoly)
		{
			if (cLeftBounds.Intersects(iCurPoly->GetBounds()))
			{
				++iCurPoly;
			}
			else
			{
				iCurPoly->Swap(*iLastPoly);
				--iLastPoly;
				ASSERT(sLeft.m_nPolyCount);
				--sLeft.m_nPolyCount;
				// Add to the right node
				ASSERT(sRight.m_nFirstPoly > sLeft.m_nFirstPoly);
				--sRight.m_nFirstPoly;
				++sRight.m_nPolyCount;
			}
		}

		// Handle the final poly
		if (!cLeftBounds.Intersects(iCurPoly->GetBounds()))
		{
			// Move it to the right-hand side
			ASSERT(sLeft.m_nPolyCount);
			--sLeft.m_nPolyCount;
			ASSERT(sRight.m_nFirstPoly > sLeft.m_nFirstPoly);
			--sRight.m_nFirstPoly;
			++sRight.m_nPolyCount;
		}

		ASSERT((sLeft.m_nFirstPoly + sLeft.m_nPolyCount) <= m_aPolys.size());
		ASSERT((sRight.m_nFirstPoly + sRight.m_nPolyCount) <= m_aPolys.size());
		ASSERT((sLeft.m_nFirstPoly + sLeft.m_nPolyCount) == sRight.m_nFirstPoly);
		ASSERT((sRight.m_nFirstPoly + sRight.m_nPolyCount) == (sParent.m_nFirstPoly + nTotalPolys));

		// Remember the polycounts so we can recurse and resize the list without crashing
		uint32 nLeftPolyCount = sLeft.m_nPolyCount;
		uint32 nRightPolyCount = sRight.m_nPolyCount;

		// Recurse left if necessary
		if ((nLeftPolyCount > k_nPolysPerNode) && (nRightPolyCount > k_nPolysPerNode))
		{
			SplitNode(sParent.m_nLeftNode, cLeftBounds);
		}
		// Iterate right if we can
		if (nRightPolyCount > k_nPolysPerNode)
		{
			nIndex = sParent.m_nLeftNode + 1;
			cCurBounds = cRightBounds;
		}
		// Iterate left if we can
		else if (nLeftPolyCount > k_nPolysPerNode)
		{
			nIndex = sParent.m_nLeftNode;
			cCurBounds = cLeftBounds;
		}
		// Otherwise we're done
		else
			return;
	}
}

void CLMPolyTree::BuildNodes()
{
	ASSERT(m_aNodes.empty());

	// Don't build a tree if we don't have any polys
	if (m_aPolys.empty())
		return;

	// Expand the bounds a bit to help with the world edges
	m_cBounds.m_vMin -= LTVector(1.0f, 1.0f, 1.0f);
	m_cBounds.m_vMax += LTVector(1.0f, 1.0f, 1.0f);

	// Initialize the root node
	m_aNodes.resize(1);
	SLMPolyTreeNode &sRoot = m_aNodes.front();
	sRoot.m_nFirstPoly = 0;
	sRoot.m_nLeftNode = k_InvalidNode;
	sRoot.m_nPolyCount = m_aPolys.size();

	// Split
	if (sRoot.m_nPolyCount > k_nPolysPerNode)
		SplitNode(0, m_cBounds);
}

void CLMPolyTree::Clear()
{
	m_aPolys.clear();
	m_aNodes.clear();
}

float CLMPolyTree::GetClosestPoly(const LTVector &vPos, const CPrePoly **pPoly) const
{
	*pPoly = 0;
	float fClosest = FLT_MAX;

	CAABB cNodeBounds = m_cBounds;
	if (!cNodeBounds.Intersects(vPos))
		return fClosest;

	const SLMPolyTreeNode *pCurNode = &m_aNodes.front();
	for(;;)
	{
		// Check the triangles in this node
		TPolyList::const_iterator iCurPoly = m_aPolys.begin() + pCurNode->m_nFirstPoly;
		TPolyList::const_iterator iEndPoly = iCurPoly + pCurNode->m_nPolyCount;
		for (; iCurPoly != iEndPoly; ++iCurPoly)
		{
			const CPrePoly *pPrePoly = iCurPoly->GetPrePoly();
			float fPolyDist = (vPos - pPrePoly->GetCenter()).Mag() + pPrePoly->GetRadius();
			if (fPolyDist < fClosest)
			{
				fClosest = fPolyDist;
				*pPoly = pPrePoly;
			}
		}

		// If we're at a leaf, we're done
		if (pCurNode->m_nLeftNode == k_InvalidNode)
			break;

		// Figure out which way to go down the tree
		CAABB cLeftBounds, cRightBounds;
		SplitAABB(cNodeBounds, &cLeftBounds, &cRightBounds);
		if (cLeftBounds.Intersects(vPos))
		{
			cNodeBounds = cLeftBounds;
			pCurNode = (&(*(m_aNodes.begin()))) + pCurNode->m_nLeftNode;
		}
		else
		{
			cNodeBounds = cRightBounds;
			pCurNode = (&(*(m_aNodes.begin()))) + (pCurNode->m_nLeftNode + 1);
		}
	}

	// Return our result
	return fClosest;
}
