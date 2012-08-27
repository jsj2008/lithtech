//////////////////////////////////////////////////////////////////////////////
// Render world occluder/occludee implementation

#include "precompile.h"

#include "d3d_renderworld_occluder.h"

#include "d3d_viewparams.h"
#include "d3d_renderblock.h"

#include "d3d_convar.h"

#include "rendererconsolevars.h"

#include "3d_ops.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////////
// COccludee implementation

// Functor to sort points by their angle
struct FPointOrder
{
	FPointOrder(const LTVector &vPivot) : m_vPivot(vPivot) {}
	bool operator()(const LTVector &v1, const LTVector &v2) const
	{
		float fArea = (v1.x - m_vPivot.x) * (v2.y - m_vPivot.y) - (v2.x - m_vPivot.x) * (v1.y - m_vPivot.y);
		if (fArea > 0.0f)
			return true;
		else if (fArea < 0.0f)
			return false;
		else // In the rare case that they are precisely co-linear...
		{
			float fX = fabsf(v1.x - m_vPivot.x) - fabsf(v2.x - m_vPivot.x);
			float fY = fabsf(v1.y - m_vPivot.y) - fabsf(v2.y - m_vPivot.y);
			if ((fX < 0.0f) || (fY < 0.0f))
				return true;
			else 
				// Note : I'm not 100% precisely sure what will happen if two
				// of the points are exactly coincident...
				return false;
		}
	}
	const LTVector &m_vPivot;
};

// Generate a convex hull, using Graham's scan
template <int NUM_PTS>
COccludee::CreatePtsOutline<NUM_PTS>::CreatePtsOutline(LTVector aPts[NUM_PTS], COutline *pResult)
{
	if (g_CV_DebugRBOldOccludeeShape.m_Val)
	{
		// Get the screen-space bounds
	  	float fMinScrX, fMinScrY, fMaxScrX, fMaxScrY;
	  	
	  	fMinScrX = fMaxScrX = aPts[0].x;
	  	fMinScrY = fMaxScrY = aPts[0].y;
	  
	  	for (uint32 nBoundsLoop = 1; nBoundsLoop < 8; ++nBoundsLoop)
	  	{
	  		fMinScrX = LTMIN(fMinScrX, aPts[nBoundsLoop].x);
	  		fMinScrY = LTMIN(fMinScrY, aPts[nBoundsLoop].y);
	  		fMaxScrX = LTMAX(fMaxScrX, aPts[nBoundsLoop].x);
	 		fMaxScrY = LTMAX(fMaxScrY, aPts[nBoundsLoop].y);
	  	}
	  
	  	pResult->push_back(LTVector(fMinScrX, fMinScrY, 0.0f));
	  	pResult->push_back(LTVector(fMaxScrX, fMinScrY, 0.0f));
	  	pResult->push_back(LTVector(fMaxScrX, fMaxScrY, 0.0f));
	  	pResult->push_back(LTVector(fMinScrX, fMaxScrY, 0.0f));
		return;
	}

	// Find the lowest point, and put it in aPts[0]
	for (uint32 nFindLowestLoop = 1; nFindLowestLoop < NUM_PTS; ++nFindLowestLoop)
	{
		if (aPts[nFindLowestLoop].y > aPts[0].y)
			std::swap(aPts[nFindLowestLoop], aPts[0]);
	}

	std::sort(aPts + 1, aPts + NUM_PTS, FPointOrder(aPts[0]));

	bool bBadOutline = false;

	// Get the actual hull
	LTVector *aStack[NUM_PTS];
	LTVector **pStackTop = &aStack[2];
	aStack[0] = &aPts[NUM_PTS - 1];
	aStack[1] = &aPts[0];
	uint32 i = 1;
	while (i < (NUM_PTS - 1))
	{
		LTVector *p1 = pStackTop[-2];
		LTVector *p2 = pStackTop[-1];
		float fArea = (p2->x - p1->x) * (aPts[i].y - p1->y) - (aPts[i].x - p1->x) * (p2->y - p1->y);
		if (fArea > -0.001f) 
		{
			*pStackTop = &aPts[i];
			++pStackTop;
			++i;
		}
		else
		{
			--pStackTop;
			if (pStackTop <= &aStack[2])
			{
				// Note : This should never happen, but if it does, we still need to provide
				// some sort of output.
				bBadOutline = true; 
				break;
			}
		}
	}

	// If the algorithm failed, go back to using a screen-extents quad :(
	if (bBadOutline)
	{
	  	// Get the screen-space bounds
	  	float fMinScrX, fMinScrY, fMaxScrX, fMaxScrY;
	  	
	  	fMinScrX = fMaxScrX = aPts[0].x;
	  	fMinScrY = fMaxScrY = aPts[0].y;
	  
	  	for (uint32 nBoundsLoop = 1; nBoundsLoop < 8; ++nBoundsLoop)
	  	{
	  		fMinScrX = LTMIN(fMinScrX, aPts[nBoundsLoop].x);
	  		fMinScrY = LTMIN(fMinScrY, aPts[nBoundsLoop].y);
	  		fMaxScrX = LTMAX(fMaxScrX, aPts[nBoundsLoop].x);
			// We already know aPts[0].y is the max..
	 		// fMaxScrY = LTMAX(fMaxScrY, aPts[nBoundsLoop].y);
	  	}
	  
	  	pResult->push_back(LTVector(fMinScrX, fMinScrY, 0.0f));
	  	pResult->push_back(LTVector(fMaxScrX, fMinScrY, 0.0f));
	  	pResult->push_back(LTVector(fMaxScrX, fMaxScrY, 0.0f));
	  	pResult->push_back(LTVector(fMinScrX, fMaxScrY, 0.0f));
		return;
	}

	// Convert the stack into an outline
	LTVector **pStackIterator = aStack;
	for (; pStackIterator != pStackTop; ++pStackIterator)
	{
		pResult->push_back(**pStackIterator);
	}
}

float COccludee::COutline::CalcArea() const
{
	if (size() < 3)
		return 0.0f;

	float fArea = 0.0f;

	LTVector vFanOrigin = front();
	COutline::const_iterator iCurPt = begin() + 1;
	for (; (iCurPt + 1) != end(); ++iCurPt)
	{
		LTVector vCurPt = *iCurPt;
		LTVector vNextPt = iCurPt[1];
		fArea += (vCurPt - vFanOrigin).Cross(vNextPt - vFanOrigin).Mag();
	}

	return fArea;
}

float COccludee::COutline::CalcArea2D() const
{
	if (size() < 3)
		return 0.0f;

	float fArea = 0.0f;

	LTVector vFanOrigin = front();
	COutline::const_iterator iCurPt = begin() + 1;
	for (; (iCurPt + 1) != end(); ++iCurPt)
	{
		LTVector vCurPt = *iCurPt;
		LTVector vNextPt = iCurPt[1];
		float fCurOfsX = vCurPt.x - vFanOrigin.x;
		float fCurOfsY = vCurPt.y - vFanOrigin.y;
		float fNextOfsX = vNextPt.x - vFanOrigin.x;
		float fNextOfsY = vNextPt.y - vFanOrigin.y;
		fArea += fNextOfsX * fCurOfsY - fNextOfsY * fCurOfsX;
	}

	return fabsf(fArea);
}

// Check an AABB against the current frustum and occluder list
// AABB outline is generated using Graham's scan
void COccludee::InitAABB(const ViewParams& Params, const LTVector &vMin, const LTVector &vMax, bool bVisible)
{
	Init();

	m_vMin = vMin;
	m_vMax = vMax;

	// Transform the corners of the box
	const uint32 k_nNumBoxPoints = 8;

	LTVector aPts[k_nNumBoxPoints];
	aPts[0].Init(vMin.x, vMin.y, vMin.z);
	aPts[1].Init(vMax.x, vMin.y, vMin.z);
	aPts[2].Init(vMin.x, vMax.y, vMin.z);
	aPts[3].Init(vMax.x, vMax.y, vMin.z);
	aPts[4].Init(vMin.x, vMin.y, vMax.z);
	aPts[5].Init(vMax.x, vMin.y, vMax.z);
	aPts[6].Init(vMin.x, vMax.y, vMax.z);
	aPts[7].Init(vMax.x, vMax.y, vMax.z);

	for (uint32 nTransformLoop = 0; nTransformLoop < k_nNumBoxPoints; ++nTransformLoop)
		MatVMul_InPlace_H(&Params.m_FullTransform, &aPts[nTransformLoop]);

	// Create the outline
	COutline aBoxOutline;
	CreatePtsOutline<k_nNumBoxPoints>(aPts, &aBoxOutline);

	if (bVisible)
		m_aVisible.push_back(aBoxOutline);
	else
		m_aOccluded.push_back(aBoxOutline);
}

void COccludee::InitPolyWorld(const SRBGeometryPoly &sPoly, bool bVisible)
{
	Init();

	// Add an outline for the poly
	if (bVisible)
		m_aVisible.resize(m_aVisible.size() + 1);
	else
		m_aOccluded.resize(m_aOccluded.size() + 1);

	// Copy the outline
	COutline &cOutline = (bVisible) ? m_aVisible.back() : m_aOccluded.back();
	SRBGeometryPoly::TVertList::const_iterator iCurVert = sPoly.m_aVerts.begin();
	for (; iCurVert != sPoly.m_aVerts.end(); ++iCurVert)
	{
		cOutline.push_back(*iCurVert);
	}

	// Copy the extents
	m_vMin = sPoly.m_vMin;
	m_vMax = sPoly.m_vMax;
}

PolySide COccludee::SplitOutline(const LTPlane &cPlane, const COutline &sInput, COutline &sVisible, COutline &sOccluded)
{
	if (sInput.empty())
		return BackSide;

	const float k_fPlaneEpsilon = 0.01f;

	// Split the visible outline by the provided plane
	sVisible.clear();
	sOccluded.clear();

	const LTVector *pPrev = &sInput.back();
	float fPrevDot = cPlane.DistTo(*pPrev);
	PolySide nPrevSide = (fPrevDot > k_fPlaneEpsilon) ? FrontSide : ((fPrevDot < -k_fPlaneEpsilon) ? BackSide : Intersect);

	uint32 nFrontCount = 0;
	uint32 nBackCount = 0;

	COutline::const_iterator iCurVert = sInput.begin();
	for (; iCurVert != sInput.end(); ++iCurVert)
	{
		const LTVector &vNext = *iCurVert;
		float fNextDot = cPlane.DistTo(vNext);
		PolySide nNextSide = (fNextDot > k_fPlaneEpsilon) ? FrontSide : ((fNextDot < -k_fPlaneEpsilon) ? BackSide : Intersect);
		if ((nNextSide != nPrevSide) && (nNextSide != Intersect) && (nPrevSide != Intersect))
		{
			// Get the split point
			LTVector vMidPt = *pPrev + (vNext - *pPrev) * (fPrevDot / (fPrevDot - fNextDot));
			// Update the outlines
			if (fNextDot > 0.0f)
			{
				sOccluded.push_back(vMidPt);
				sVisible.push_back(vMidPt);
				sVisible.push_back(vNext);
				++nFrontCount;
			}
			else
			{
				sVisible.push_back(vMidPt);
				sOccluded.push_back(vMidPt);
				sOccluded.push_back(vNext);
				++nBackCount;
			}
		}
		else if (nNextSide == FrontSide)
		{
			++nFrontCount;
			sVisible.push_back(vNext);
		}
		else if (nNextSide == BackSide)
		{
			++nBackCount;
			sOccluded.push_back(vNext);
		}
		else
		{
			sVisible.push_back(vNext);
			sOccluded.push_back(vNext);
		}

		pPrev = &vNext;
		fPrevDot = fNextDot;
		nPrevSide = nNextSide;
	}

	if (nBackCount == 0)
	{
		if (nFrontCount == 0)
			return Intersect;
		else
			return FrontSide;
	}
	else if (nFrontCount == 0)
		return BackSide;
	else
		return Intersect;
}

PolySide COccludee::SplitOutline2D(const LTPlane &cPlane, const COutline &sInput, PolySide nCoplanarResult, COutline &sVisible, COutline &sOccluded)
{
	if (sInput.empty())
		return BackSide;

	const float k_fPlaneEpsilon = 0.1f;

	// Split the visible outline by the provided plane
	sVisible.clear();
	sOccluded.clear();

	const LTVector *pPrev = &sInput.back();
	float fPrevDot = cPlane.m_Normal.x * pPrev->x + cPlane.m_Normal.y * pPrev->y - cPlane.m_Dist;
	PolySide nPrevSide = (fPrevDot > k_fPlaneEpsilon) ? FrontSide : ((fPrevDot < -k_fPlaneEpsilon) ? BackSide : Intersect);

	uint32 nFrontCount = 0;
	uint32 nBackCount = 0;

	COutline::const_iterator iCurVert = sInput.begin();
	for (; iCurVert != sInput.end(); ++iCurVert)
	{
		const LTVector &vNext = *iCurVert;
		float fNextDot = cPlane.m_Normal.x * vNext.x + cPlane.m_Normal.y * vNext.y - cPlane.m_Dist;
		PolySide nNextSide = (fNextDot > k_fPlaneEpsilon) ? FrontSide : ((fNextDot < -k_fPlaneEpsilon) ? BackSide : Intersect);
		if ((nNextSide != nPrevSide) && (nNextSide != Intersect) && (nPrevSide != Intersect))
		{
			// Get the split point
			LTVector vMidPt;
			float fIntersect = (fPrevDot / (fPrevDot - fNextDot));
			vMidPt.x = pPrev->x + (vNext.x - pPrev->x) * fIntersect;
			vMidPt.y = pPrev->y + (vNext.y - pPrev->y) * fIntersect;
			vMidPt.z = 0.0f;
			// Update the outlines
			if (fNextDot > 0.0f)
			{
				sOccluded.push_back(vMidPt);
				sVisible.push_back(vMidPt);
				sVisible.push_back(vNext);
				++nFrontCount;
			}
			else
			{
				sVisible.push_back(vMidPt);
				sOccluded.push_back(vMidPt);
				sOccluded.push_back(vNext);
				++nBackCount;
			}
		}
		else if (nNextSide == FrontSide)
		{
			++nFrontCount;
			sVisible.push_back(vNext);
		}
		else if (nNextSide == BackSide)
		{
			++nBackCount;
			sOccluded.push_back(vNext);
		}
		else
		{
			sVisible.push_back(vNext);
			sOccluded.push_back(vNext);
		}

		pPrev = &vNext;
		fPrevDot = fNextDot;
		nPrevSide = nNextSide;
	}

	if (nBackCount == 0)
	{
		if (nFrontCount == 0)
			return nCoplanarResult;
		else
			return FrontSide;
	}
	else if (nFrontCount == 0)
		return BackSide;
	else
		return Intersect;
}

PolySide COccludee::SplitPlane(const LTPlane &cPlane, bool bSplitOccluded)
{
	// Split the outlines into visible and occluded portions

	COutline sVisible;
	COutline sOccluded;

	uint32 nOccludedSize = m_aOccluded.size();

	TOutlineList::iterator iCurOutline;
	if (!bSplitOccluded)
	{
		iCurOutline = m_aVisible.begin();
		while (iCurOutline != m_aVisible.end())
		{
			PolySide nResult = SplitOutline(cPlane, *iCurOutline, sVisible, sOccluded);
			switch (nResult)
			{
				case FrontSide : 
					++iCurOutline;
					goto endLoop;
				case BackSide : 
					if ((iCurOutline + 1) != m_aVisible.end())
						*iCurOutline = m_aVisible.back();
					m_aVisible.pop_back();
					ASSERT(sOccluded.size() > 2);
					m_aOccluded.push_back(sOccluded);
					goto endLoop;
				case Intersect :
					ASSERT(sVisible.size() > 2);
					*iCurOutline = sVisible;
					++iCurOutline;
					ASSERT(sOccluded.size() > 2);
					m_aOccluded.push_back(sOccluded);
					goto endLoop;
			}
		}
	}

	if (bSplitOccluded)
	{
		iCurOutline = m_aOccluded.begin();
		while (nOccludedSize)
		{
			ASSERT(iCurOutline != m_aOccluded.end());
			--nOccludedSize;
			PolySide nResult = SplitOutline(cPlane, *iCurOutline, sVisible, sOccluded);
			switch (nResult)
			{
				case FrontSide :
					if ((iCurOutline + 1) != m_aOccluded.end())
						*iCurOutline = m_aOccluded.back();
					m_aOccluded.pop_back();
					ASSERT(sVisible.size() > 2);
					m_aVisible.push_back(sVisible);
					goto endLoop;
				case BackSide :
					++iCurOutline;
					goto endLoop;
				case Intersect :
					ASSERT(sOccluded.size() > 2);
					*iCurOutline = sOccluded;
					++iCurOutline;
					ASSERT(sVisible.size() > 2);
					m_aVisible.push_back(sVisible);
					goto endLoop;
			}
		}
	}

	endLoop:
	if (m_aVisible.empty())
		return BackSide;
	if (m_aOccluded.empty())
		return FrontSide;
	return Intersect;
}

PolySide COccludee::SplitPlane2D(const LTPlane &cPlane, bool bSplitOccluded)
{
	// Split the outlines into visible and occluded portions

	COutline sVisible;
	COutline sOccluded;

	TOutlineList::iterator iCurOutline;

	if (!bSplitOccluded)
	{
		iCurOutline = m_aVisible.begin();
		while (iCurOutline != m_aVisible.end())
		{
			PolySide nResult = SplitOutline2D(cPlane, *iCurOutline, FrontSide, sVisible, sOccluded);
			switch (nResult)
			{
				case FrontSide : 
					++iCurOutline;
					break;
				case BackSide : 
					if ((iCurOutline + 1) != m_aVisible.end())
						*iCurOutline = m_aVisible.back();
					m_aVisible.pop_back();
					ASSERT(sOccluded.size() > 2);
					m_aOccluded.push_back(sOccluded);
					goto endLoop;
				case Intersect :
					ASSERT(sVisible.size() > 2);
					*iCurOutline = sVisible;
					++iCurOutline;
					ASSERT(sOccluded.size() > 2);
					m_aOccluded.push_back(sOccluded);
					goto endLoop;
			}
		}
	}

	if (bSplitOccluded)
	{
		iCurOutline = m_aOccluded.begin();
		while (iCurOutline != m_aOccluded.end())
		{
			PolySide nResult = SplitOutline2D(cPlane, *iCurOutline, BackSide, sVisible, sOccluded);
			switch (nResult)
			{
				case FrontSide :
					if ((iCurOutline + 1) != m_aOccluded.end())
						*iCurOutline = m_aOccluded.back();
					m_aOccluded.pop_back();
					ASSERT(sVisible.size() > 2);
					m_aVisible.push_back(sVisible);
					goto endLoop;
				case BackSide :
					++iCurOutline;
					break;
				case Intersect :
					ASSERT(sOccluded.size() > 2);
					*iCurOutline = sOccluded;
					++iCurOutline;
					ASSERT(sVisible.size() > 2);
					m_aVisible.push_back(sVisible);
					goto endLoop;
			}
		}
	}

	endLoop:
	if (m_aVisible.empty())
		return BackSide;
	if (m_aOccluded.empty())
		return FrontSide;
	return Intersect;
}

PolySide COccludee::ClassifyOutline(const LTPlane &cPlane, const COutline &sInput)
{
	uint32 nFrontCount = 0;
	uint32 nBackCount = 0;
	COutline::const_iterator iCurVert = sInput.begin();
	for (; iCurVert != sInput.end(); ++iCurVert)
	{
		float fDist = cPlane.DistTo(*iCurVert);
		if (fDist > 0.001f)
		{
			++nFrontCount;
			if (nBackCount)
				return Intersect;
		}
		else if (fDist < -0.001f)
		{
			++nBackCount;
			if (nFrontCount)
				return Intersect;
		}
	}
	if ((nFrontCount == 0) && (nBackCount == 0))
		return Intersect;
	if (nBackCount == 0)
		return FrontSide;
	if (nFrontCount == 0)
		return BackSide;
	return Intersect;
}

PolySide COccludee::ClassifyPlane(const LTPlane &cPlane, bool bVisible) const
{
	const TOutlineList &aOutlines = (bVisible) ? m_aVisible : m_aOccluded;
	uint32 nFrontCount = 0;
	uint32 nBackCount = 0;
	TOutlineList::const_iterator iCurOutline = aOutlines.begin();
	for (; iCurOutline != aOutlines.end(); ++iCurOutline)
	{
		switch (ClassifyOutline(cPlane, *iCurOutline))
		{
			case FrontSide : 
				++nFrontCount;
				if (nBackCount)
					return Intersect;
			case BackSide :
				++nBackCount;
				if (nFrontCount)
					return Intersect;
			case Intersect :
				return Intersect;
		}
	}
	if (nBackCount == 0)
		return FrontSide;
	if (nFrontCount == 0)
		return BackSide;
	return Intersect; // This one should never happen, but if it does, no big deal
}

float COccludee::CalcArea(bool bVisible) const
{
	float fArea = 0.0f;

	const TOutlineList &aOutlineList = (bVisible) ? m_aVisible : m_aOccluded;
	TOutlineList::const_iterator iCurOutline = aOutlineList.begin();
	for (; iCurOutline != aOutlineList.end(); ++iCurOutline)
	{
		const COutline &cCurOutline = *iCurOutline;
		fArea += cCurOutline.CalcArea();
	}

	return fArea;
}

float COccludee::CalcArea2D(bool bVisible) const
{
	float fArea = 0.0f;

	const TOutlineList &aOutlineList = (bVisible) ? m_aVisible : m_aOccluded;
	TOutlineList::const_iterator iCurOutline = aOutlineList.begin();
	for (; iCurOutline != aOutlineList.end(); ++iCurOutline)
	{
		const COutline &cCurOutline = *iCurOutline;
		fArea += cCurOutline.CalcArea2D();
	}

	return fArea;
}

//////////////////////////////////////////////////////////////////////////////
// COccluder implementation

void COccluder::AddPlane(const LTPlane &cScreenPlane)
{
	m_aEdgePlanes.push_back(cScreenPlane);
}

PolySide COccluder::Occlude(COccludee &cOccludee) const
{
	// Don't occlude unless the occluder's entirely behind this plane
	if (!GetAABBPlaneSideBack(m_ePolyPlaneCorner, m_cPolyPlane, cOccludee.m_vMin, cOccludee.m_vMax))
	{
		if (cOccludee.m_aVisible.empty())
			cOccludee.m_aOccluded.swap(cOccludee.m_aVisible);
		else
		{
			cOccludee.m_aVisible.insert(cOccludee.m_aVisible.end(), cOccludee.m_aOccluded.begin(), cOccludee.m_aOccluded.end());
			cOccludee.m_aOccluded.clear();
		}
		return FrontSide;
	}

	TPlaneList::const_iterator iCurPlane = m_aEdgePlanes.begin();
	for (; iCurPlane != m_aEdgePlanes.end(); ++iCurPlane)
	{
		PolySide nResult = cOccludee.SplitPlane(*iCurPlane, true);
		if (nResult == FrontSide)
			return FrontSide;
	}

	if (cOccludee.m_aVisible.empty())
		return BackSide;

	return Intersect;
}

PolySide COccluder::Occlude2D(COccludee &cOccludee) const
{
	// Don't occlude unless the occluder's entirely behind this plane
	if (!GetAABBPlaneSideBack(m_ePolyPlaneCorner, m_cPolyPlane, cOccludee.m_vMin, cOccludee.m_vMax))
	{
		if (cOccludee.m_aVisible.empty())
			cOccludee.m_aOccluded.swap(cOccludee.m_aVisible);
		else
		{
			cOccludee.m_aVisible.insert(cOccludee.m_aVisible.end(), cOccludee.m_aOccluded.begin(), cOccludee.m_aOccluded.end());
			cOccludee.m_aOccluded.clear();
		}
		return FrontSide;
	}

	TPlaneList::const_iterator iCurPlane = m_aEdgePlanes.begin();
	for (; iCurPlane != m_aEdgePlanes.end(); ++iCurPlane)
	{
		PolySide nResult = cOccludee.SplitPlane2D(*iCurPlane, true);
		if (nResult == FrontSide)
			return FrontSide;
	}

	if (cOccludee.m_aVisible.empty())
		return BackSide;

	if (g_CV_DebugRBFindSlivers.m_Val)
	{
		static float fMinArea = FLT_MAX;
		float fOccludeeArea = cOccludee.CalcArea2D(true);
		if (fOccludeeArea < fMinArea)
		{
			fMinArea = fOccludeeArea;
			dsi_ConsolePrint("Smallest occludee visible area: %0.4f", fMinArea);
		}
		if (fOccludeeArea < 2.0f)
			return BackSide;
	}

	return Intersect;
}

//////////////////////////////////////////////////////////////////////////////
// Occluder_Frustum implementation

void COccluder_Frustum::AddPlane(const LTPlane &cScreenPlane, const LTPlane &cWorldPlane)
{
	COccluder::AddPlane(cScreenPlane);
	m_aWorldEdgePlanes.push_back(cWorldPlane);
	m_aEdgeCorners.push_back(GetAABBPlaneCorner(cWorldPlane.m_Normal));
}

void COccluder_Frustum::InitFrustum(const ViewParams& Params)
{
	// Clear!
	Init();

	// Get the points of the near plane in order
	const uint32 k_nNumScreenPts = 4;
	LTVector aScreenPts[k_nNumScreenPts];
	aScreenPts[0] = Params.m_ViewPoints[2];
	aScreenPts[1] = Params.m_ViewPoints[3];
	aScreenPts[2] = Params.m_ViewPoints[1];
	aScreenPts[3] = Params.m_ViewPoints[0];

	// Gimmie some room
	m_aEdgePlanes.reserve(k_nNumScreenPts);
	m_aWorldEdgePlanes.reserve(k_nNumScreenPts);

	// Remember the near plane
	m_cPolyPlane = Params.m_ClipPlanes[CPLANE_NEAR_INDEX];
	m_ePolyPlaneCorner = GetAABBPlaneCorner(m_cPolyPlane.m_Normal);

	// Build the occluder
	LTVector vPrevWorld = aScreenPts[3];
	LTVector vPrevScr;
	MatVMul_H(&vPrevScr, &Params.m_FullTransform, &vPrevWorld);
	for (const LTVector *pCurVert = aScreenPts; pCurVert != &aScreenPts[k_nNumScreenPts]; ++pCurVert)
	{
		const LTVector &vNextWorld = *pCurVert;
		LTVector vNextScr;
		MatVMul_H(&vNextScr, &Params.m_FullTransform, &vNextWorld);
		float fXDiff = (vNextScr.x - vPrevScr.x);
		float fYDiff = (vNextScr.y - vPrevScr.y);
		if ((fXDiff * fXDiff + fYDiff * fYDiff) > 0.001f)
		{
			LTPlane cScreenPlane;
			LTVector vEdgeScr = vNextScr - vPrevScr;
			cScreenPlane.m_Normal.Init(vEdgeScr.y, -vEdgeScr.x, 0.0f);
			cScreenPlane.m_Normal.Normalize();
			cScreenPlane.m_Dist = cScreenPlane.m_Normal.x * vNextScr.x + cScreenPlane.m_Normal.y * vNextScr.y;

			LTPlane cWorldPlane;
			LTVector vEdgeWorld = vNextWorld - vPrevWorld;
			cWorldPlane.m_Normal = vEdgeWorld.Cross(vNextWorld - Params.m_Pos);
			cWorldPlane.m_Normal.Normalize();
			cWorldPlane.m_Dist = cWorldPlane.m_Normal.Dot(vNextWorld);

			AddPlane(cScreenPlane, cWorldPlane);
		}

		vPrevWorld = vNextWorld;
		vPrevScr = vNextScr;
	}
}

PolySide COccluder_Frustum::ClipNear(COccludee &cOccludee) const
{
	// Do the polygon plane split explicitly
	return cOccludee.SplitPlane(m_cPolyPlane, false);
}

PolySide COccluder_Frustum::Classify(COccludee &cOccludee) const
{
	// Shortcut the near plane classification
	if (GetAABBPlaneSideBack(m_ePolyPlaneCorner, m_cPolyPlane, cOccludee.m_vMin, cOccludee.m_vMax))
		return BackSide;

	// Classify based on the poly plane
	if (cOccludee.ClassifyPlane(m_cPolyPlane, true) == BackSide)
		return BackSide;

	// Classify based on the edge planes
	TPlaneList::const_iterator iCurPlane = m_aWorldEdgePlanes.begin();
	for (; iCurPlane != m_aWorldEdgePlanes.end(); ++iCurPlane)
	{
		PolySide nResult = cOccludee.ClassifyPlane(*iCurPlane, true);
		if (nResult == BackSide)
			return BackSide;
	}

	// This might actually be front, but I don't have a way of just classifying that.
	return Intersect;
}

PolySide COccluder_Frustum::ClassifyAABB(const LTVector &vMin, const LTVector &vMax, float fFarZ)  const
{
	uint32 nFront = 0;

	// Check if the corner hasn't been set yet.
	if( m_ePolyPlaneCorner == eAABB_None )
		return FrontSide;

	PolySide nCurSide = GetAABBPlaneSide(m_ePolyPlaneCorner, m_cPolyPlane, vMin, vMax);
	if (nCurSide == BackSide)
		return BackSide;
	if (nCurSide == FrontSide)
		++nFront;

	// Is it beyond the far z?
	nCurSide = GetAABBPlaneSide(m_ePolyPlaneCorner, 
		LTPlane(m_cPolyPlane.m_Normal, m_cPolyPlane.m_Dist + fFarZ), vMin, vMax);
	if (nCurSide == FrontSide)
		return BackSide;

	TPlaneList::const_iterator iCurPlane = m_aWorldEdgePlanes.begin();
	TCornerList::const_iterator iCurCorner = m_aEdgeCorners.begin();
	for (; iCurPlane != m_aWorldEdgePlanes.end(); ++iCurPlane, ++iCurCorner)
	{
		nCurSide = GetAABBPlaneSide(*iCurCorner, *iCurPlane, vMin, vMax);
		if (nCurSide == BackSide)
			return BackSide;
		if (nCurSide == FrontSide)
			++nFront;
	}

	if (nFront == (m_aWorldEdgePlanes.size() + 1))
		return FrontSide;
	
	return Intersect;
}

PolySide COccluder_Frustum::Occlude(COccludee &cOccludee) const
{
	PolySide nPlaneSide = GetAABBPlaneSide(m_ePolyPlaneCorner, m_cPolyPlane, cOccludee.m_vMin, cOccludee.m_vMax);
	if (nPlaneSide != FrontSide)
		return nPlaneSide;

	TPlaneList::const_iterator iCurPlane = m_aEdgePlanes.begin();
	for (; iCurPlane != m_aEdgePlanes.end(); ++iCurPlane)
	{
		PolySide nResult = cOccludee.SplitPlane(*iCurPlane, false);
		if (nResult == BackSide)
			return BackSide;
	}

	if (cOccludee.m_aVisible.empty())
		return BackSide;

	return Intersect;
}

PolySide COccluder_Frustum::Occlude2D(COccludee &cOccludee) const
{
	PolySide nPlaneSide = GetAABBPlaneSide(m_ePolyPlaneCorner, m_cPolyPlane, cOccludee.m_vMin, cOccludee.m_vMax);
	if (nPlaneSide != FrontSide)
		return nPlaneSide;

	TPlaneList::const_iterator iCurPlane = m_aEdgePlanes.begin();
	for (; iCurPlane != m_aEdgePlanes.end(); ++iCurPlane)
	{
		PolySide nResult = cOccludee.SplitPlane2D(*iCurPlane, false);
		if (nResult == BackSide)
			return BackSide;
	}

	if (cOccludee.m_aVisible.empty())
		return BackSide;

	return Intersect;
}

PolySide COccluder_Frustum::OccludeWorld(COccludee &cOccludee) const 
{
	// Shortcut the near plane classification
	if (GetAABBPlaneSideBack(m_ePolyPlaneCorner, m_cPolyPlane, cOccludee.m_vMin, cOccludee.m_vMax))
		return BackSide;

	// Do the polygon plane split if it's a world-oriented occludee
	if (cOccludee.SplitPlane(m_cPolyPlane, false) == BackSide)
		return BackSide;

	TPlaneList::const_iterator iCurPlane = m_aWorldEdgePlanes.begin();
	for (; iCurPlane != m_aWorldEdgePlanes.end(); ++iCurPlane)
	{
		PolySide nResult = cOccludee.SplitPlane(*iCurPlane, false);
		if (nResult == BackSide)
			return BackSide;
	}

	if (cOccludee.m_aVisible.empty())
		return BackSide;

	return Intersect;
}

//////////////////////////////////////////////////////////////////////////////
// COccluder_2D implementation

void COccluder_2D::InitOutline(const ViewParams& Params, const COccludee::COutline &cOutline, const LTPlane &cPlane)
{
	// Make room
	m_aEdgePlanes.reserve(cOutline.size());

	// Get the plane
	m_cPolyPlane = cPlane;

	// Get the reversal flag
	bool bReverseWinding = false;
	if (m_cPolyPlane.DistTo(Params.m_Pos) < 0.0f)
	{
		m_cPolyPlane = -m_cPolyPlane;
		bReverseWinding = !bReverseWinding;
	}

	m_ePolyPlaneCorner = GetAABBPlaneCorner(m_cPolyPlane.m_Normal);

	static COccludee::COutline cOutline2D;
	cOutline2D.clear();

	// Read in from the un-transformed occluder
	LTVector vPrevWorld = cOutline.back();
	LTVector vPrevScr;
	MatVMul_H(&vPrevScr, &Params.m_FullTransform, &vPrevWorld);
	COccludee::COutline::const_iterator iCurVert = cOutline.begin();
	for (; iCurVert != cOutline.end(); ++iCurVert)
	{
		const LTVector &vNextWorld = *iCurVert;
		LTVector vNextScr;
		MatVMul_H(&vNextScr, &Params.m_FullTransform, &vNextWorld);
		cOutline2D.push_back(vNextScr);
		float fXDiff = (vNextScr.x - vPrevScr.x);
		float fYDiff = (vNextScr.y - vPrevScr.y);
		if ((fXDiff * fXDiff + fYDiff * fYDiff) > 0.001f)
		{
			LTPlane cScreenPlane;
			LTVector vEdgeScr = vNextScr - vPrevScr;
			cScreenPlane.m_Normal.Init(vEdgeScr.y, -vEdgeScr.x, 0.0f);
			cScreenPlane.m_Normal.Normalize();
			if (bReverseWinding)
				cScreenPlane.m_Normal = -cScreenPlane.m_Normal;
			cScreenPlane.m_Dist = cScreenPlane.m_Normal.x * vNextScr.x + cScreenPlane.m_Normal.y * vNextScr.y;

			AddPlane(cScreenPlane);
		}

		vPrevWorld = vNextWorld;
		vPrevScr = vNextScr;
	}

	// Figure out how big we are on-screen
	m_fScreenArea = cOutline2D.CalcArea2D();
}

