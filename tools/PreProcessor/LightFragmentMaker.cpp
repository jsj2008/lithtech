//////////////////////////////////////////////////////////////////////////////
// Polygon subdivision lighting engine implementation

#include "bdefs.h"
#include "lightfragmentmaker.h"

#include "lightmapmaker.h"

#include "prepoly.h"
#include "prepolyfragments.h"

#include "processing.h" // For DrawStatusText

#include <stack>

//////////////////////////////////////////////////////////////////////////////
// Math functions used by this module

float PtLineSegDist(const LTVector& vPt, const LTVector &vOrigin, const LTVector &vOfs,
    float* pfParam)
{
	LTVector vDiff = vPt - vOrigin;
    float fT = vDiff.Dot(vOfs);

    if ( fT <= 0.0f )
    {
        fT = 0.0f;
    }
    else
    {
        float fSqrLen= vOfs.MagSqr();
        if ( fT >= fSqrLen )
        {
            fT = 1.0f;
            vDiff -= vOfs;
        }
        else
        {
            fT /= fSqrLen;
            vDiff -= fT*vOfs;
        }
    }

    if ( pfParam )
        *pfParam = fT;

    return vDiff.Mag();
}

// Code from elsewhere inside the engine...

// Is the given point inside of the given triangle?
bool PointInTriangle
(
	const LTVector& p,	//point
	const LTVector& v0,	//triangle vertices
	const LTVector& v1,
	const LTVector& v2
)
{
	const LTVector n = (v2-v0).Cross(v1-v0);

	if( n.Dot( (p-v0).Cross(v1-v0) ) < 0 )
		return false;

	if( n.Dot( (p-v1).Cross(v2-v1) ) < 0 )
		return false;

	if( n.Dot( (p-v2).Cross(v0-v2) ) < 0 )
		return false;

	return true;
}

inline void GetBarycentricCoords(
	const LTVector &vPt,
	const LTVector &vOrigin,
	const LTVector &vEdge0,
	const LTVector &vEdge1,
	float *pU, 
	float *pV)
{
	float fInvTriArea = 1.0f / vEdge0.Cross(vEdge1).Mag();
	LTVector vOffset = vPt - vOrigin;
	*pU = vEdge0.Cross(vOffset).Mag() * fInvTriArea;
	*pV = vEdge1.Cross(vOffset).Mag() * fInvTriArea;
}

// Determine which side of a plane a point is on, and return the distance
PolySide GetPointSide(const LTPlane &cPlane, const LTVector &vPt, float *pDist)
{
	const float k_fPlaneSideEpsilon = 0.0001f;

	*pDist = cPlane.DistTo(vPt);
	if (*pDist < -k_fPlaneSideEpsilon) 
		return BackSide;
	else if (*pDist > k_fPlaneSideEpsilon) 
		return FrontSide;
	else
		return Intersect;
}

// Project a point onto a plane given a projection point
// Returns false if unable to project onto the plane
bool ProjectPoint(const LTPlane &cPlane, const LTVector &vProjPt, const LTVector &vPt, LTVector *pResultPt)
{
	const float k_fProjectEpsilon = 0.001f;

	LTVector vProjDir = vPt - vProjPt;
	float fMag = vProjDir.Mag();
	if (fMag < k_fProjectEpsilon)
	{
		// Just project it onto the plane if it's too close
		*pResultPt = vPt - cPlane.m_Normal * cPlane.DistTo(vPt);
		return true;
	}

	vProjDir /= fMag;
	float fPlaneDot = cPlane.m_Normal.Dot(vProjDir);

	// Calculate the intersection point
	if (fabsf(fPlaneDot) > k_fProjectEpsilon)
	{
		float fProjDist = cPlane.DistTo(vPt);
		*pResultPt = vPt + vProjDir * (fProjDist / -fPlaneDot);
	}
	else
		*pResultPt = vPt;

	// Project it onto the plane for safety...
	*pResultPt -= cPlane.m_Normal * cPlane.DistTo(*pResultPt);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// SLFM_Outline implementation

SLFM_Outline::SLFM_Outline() :
	m_pPrePoly(0)
{
}

SLFM_Outline::SLFM_Outline(CPrePoly *pPoly) :
	m_pPrePoly(pPoly),
	m_cPlane(*pPoly->GetPlane())
{
	m_aVertices.reserve(pPoly->NumVertsAndTVerts());
	for (uint32 nCopyVec = 0; nCopyVec < pPoly->NumVertsAndTVerts(); ++nCopyVec)
		m_aVertices.push_back(pPoly->GetVertOrTVert(nCopyVec).m_Vec);
	ASSERT(!m_aVertices.empty());
	RecalcExtents();
	m_aNeighbors.resize(m_aVertices.size(), 0);
}

void SLFM_Outline::RecalcExtents(float fExpandExtents)
{
	if (m_aVertices.empty())
	{
		m_vExtentsMin.Init(0.0f, 0.0f, 0.0f);
		m_vExtentsMax.Init(0.0f, 0.0f, 0.0f);
		ASSERT(!"Extents initialized with empty vertex list");
		return;
	}

	m_vExtentsMin = m_aVertices.front();
	m_vExtentsMax = m_aVertices.front();
	TVertList::iterator iCurVert = m_aVertices.begin() + 1;
	for (; iCurVert != m_aVertices.end(); ++iCurVert)
	{
		VEC_MIN(m_vExtentsMin, m_vExtentsMin, *iCurVert);
		VEC_MAX(m_vExtentsMax, m_vExtentsMax, *iCurVert);
	}

	m_vExtentsMin -= LTVector(fExpandExtents, fExpandExtents, fExpandExtents);
	m_vExtentsMax += LTVector(fExpandExtents, fExpandExtents, fExpandExtents);
}

//////////////////////////////////////////////////////////////////////////////
// SLFM_Edge implementation

SLFM_Edge::SLFM_Edge(
	const LTVector &vPt1, const LTVector &vPt2, 
	const SLFM_Outline *pNeighbor0, const SLFM_Outline *pNeighbor1) :
	m_vStart(vPt1), m_vEnd(vPt2)
{
	m_pOutlines[0] = pNeighbor0;
	m_pOutlines[1] = pNeighbor1;
	Recalc();
}

SLFM_Edge::SLFM_Edge(const SLFM_Edge &cOther) :
	m_vStart(cOther.m_vStart),
	m_vEnd(cOther.m_vEnd)
{
	m_pOutlines[0] = cOther.m_pOutlines[0];
	m_pOutlines[1] = cOther.m_pOutlines[1];
}

SLFM_Edge &SLFM_Edge::operator=(const SLFM_Edge &cOther)
{
	m_vStart = cOther.m_vStart;
	m_vEnd = cOther.m_vEnd;
	m_pOutlines[0] = cOther.m_pOutlines[0];
	m_pOutlines[1] = cOther.m_pOutlines[1];
	return *this;
}

void SLFM_Edge::Recalc()
{
}

LTPlane SLFM_Edge::CreatePlane(const LTVector &vPt) const
{
	LTVector vNormal = (vPt - m_vStart).Cross(m_vEnd - m_vStart);
	vNormal.Normalize();
	return LTPlane(vNormal, vPt);
}

PolySide SLFM_Edge::GetPlaneSide(const LTPlane &cPlane, float *pIntersectTime, bool *pStartFront) const
{
	float fStartDist, fEndDist;
	PolySide nStartSide = GetPointSide(cPlane, m_vStart, &fStartDist);
	PolySide nEndSide = GetPointSide(cPlane, m_vEnd, &fEndDist);

	if (nStartSide == Intersect)
		nStartSide = nEndSide;
	else if (nEndSide == Intersect)
		nEndSide = nStartSide;

	if (nStartSide == nEndSide)
	{
		// If it's on the plane, don't consider it to be an intersection
		if (nStartSide == Intersect)
			// Testing
			// nStartSide = FrontSide;
			nStartSide = BackSide;
			// End Testing
		*pStartFront = nStartSide == FrontSide;
		return nStartSide;
	}

	*pIntersectTime = fStartDist / (fStartDist - fEndDist);
	*pStartFront = nStartSide == FrontSide;

	return Intersect;
}

bool SLFM_Edge::SplitPlane(const LTPlane &cPlane, SLFM_Edge *pNewEdge, PolySide *pSideResult, bool *pStartFront)
{
	// Perform the plane intersection
	float fIntersectTime;
	*pSideResult = GetPlaneSide(cPlane, &fIntersectTime, pStartFront);
	if (*pSideResult != Intersect)
		return false;

	// Get the intersection point
	LTVector vIntersectPt;
	VEC_LERP(vIntersectPt, m_vStart, m_vEnd, fIntersectTime);

	SplitPt(vIntersectPt, pNewEdge);

	return true;
}

void SLFM_Edge::SplitPt(const LTVector &vPt, SLFM_Edge *pNewEdge)
{
	// Copy this edge into the result
	*pNewEdge = *this;

	// Divide up the segments
	pNewEdge->m_vStart = vPt;
	m_vEnd = vPt;
}

bool SLFM_Edge::ProjectPlane(const LTPlane &cPlane, const LTVector &vStartPt, const LTVector &vEndPt, float fRayEdgeLen)
{
	SLFM_Edge cTempEdge(*this);
	// Handle the case where we cross the plane
	PolySide nSide;
	bool bStartFront;
	if (SplitPlane(cPlane, &cTempEdge, &nSide, &bStartFront))
	{
		if (!bStartFront)
			std::swap(*this, cTempEdge);
	}
	else
	{
		// Don't project onto back-facing polys
		if (nSide == BackSide)
			return false;

		// Check for an edge which is on the plane
		float fTemp;
		nSide = GetPointSide(cPlane, m_vStart, &fTemp);
		if ((nSide == Intersect) && (nSide == GetPointSide(cPlane, m_vEnd, &fTemp)))
			return false;
	}

	// Project the end points
	LTVector vTemp;
	if (!ProjectPoint(cPlane, vStartPt, m_vStart, &vTemp))
		return false;
	m_vStart = vTemp;
	if (!ProjectPoint(cPlane, vEndPt, m_vEnd, &vTemp))
		return false;
	m_vEnd = vTemp;

	ASSERT(fabsf(cPlane.DistTo(m_vStart)) < 0.01f);
	ASSERT(fabsf(cPlane.DistTo(m_vEnd)) < 0.01f);

	// Recalc the extra data
	Recalc();

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// SLFM_ShadowEdge implementation

SLFM_ShadowEdge::SLFM_ShadowEdge(const LTVector &vStart, const LTVector &vEnd, const CLightDef *pLight, float fStart, float fEnd) :
	m_vStart(vStart), m_vEnd(vEnd), m_pLight(pLight), m_fStartLight(fStart), m_fEndLight(fEnd)
{
}

SLFM_ShadowEdge::SLFM_ShadowEdge(const SLFM_ShadowEdge &cOther) :
	m_vStart(cOther.m_vStart), m_vEnd(cOther.m_vEnd), 
	m_pLight(cOther.m_pLight), 
	m_fStartLight(cOther.m_fStartLight), m_fEndLight(cOther.m_fEndLight)
{
}

SLFM_ShadowEdge &SLFM_ShadowEdge::operator=(const SLFM_ShadowEdge &cOther)
{
	m_vStart = cOther.m_vStart;
	m_vEnd = cOther.m_vEnd;
	m_pLight = cOther.m_pLight;
	m_fStartLight = cOther.m_fStartLight;
	m_fEndLight = cOther.m_fEndLight;
	return *this;
}

PolySide SLFM_ShadowEdge::GetPlaneSide(const LTPlane &cPlane, float *pIntersectTime, bool *pStartFront) const
{
	float fStartDist, fEndDist;
	PolySide nStartSide = GetPointSide(cPlane, m_vStart, &fStartDist);
	PolySide nEndSide = GetPointSide(cPlane, m_vEnd, &fEndDist);

	if (nStartSide == Intersect)
		nStartSide = nEndSide;
	else if (nEndSide == Intersect)
		nEndSide = nStartSide;

	if (nStartSide == nEndSide)
	{
		// If it's on the plane, don't consider it to be an intersection
		if (nStartSide == Intersect)
			return (nStartSide <= 0.0f) ? BackSide : FrontSide;
		else
			return nStartSide;
	}

	*pIntersectTime = fStartDist / (fStartDist - fEndDist);
	*pStartFront = nStartSide == FrontSide;

	return Intersect;
}

PolySide SLFM_ShadowEdge::SplitPlane(const LTPlane &cPlane, SLFM_ShadowEdge *pResultFront, SLFM_ShadowEdge *pResultBack) const
{
	// Perform the plane intersection
	float fIntersectTime;
	bool bStartFront;
	PolySide nSideResult = GetPlaneSide(cPlane, &fIntersectTime, &bStartFront);
	if (nSideResult != Intersect)
		return nSideResult;

	if (!bStartFront)
		std::swap(pResultFront, pResultBack);

	Split(fIntersectTime, pResultFront, pResultBack);

	return Intersect;
}

void SLFM_ShadowEdge::Split(float fInterpolant, SLFM_ShadowEdge *pStartResult, SLFM_ShadowEdge *pEndResult) const
{
	LTVector vMidPt;
	VEC_LERP(vMidPt, m_vStart, m_vEnd, fInterpolant);
	float fMidLight = LTLERP(m_fStartLight, m_fEndLight, fInterpolant);

	pStartResult->m_vStart = m_vStart;
	pStartResult->m_vEnd = vMidPt;
	pStartResult->m_pLight = m_pLight;
	pStartResult->m_fStartLight = m_fStartLight;
	pStartResult->m_fEndLight = fMidLight;

	pEndResult->m_vStart = vMidPt;
	pEndResult->m_vEnd = m_vEnd;
	pEndResult->m_pLight = m_pLight;
	pEndResult->m_fStartLight = fMidLight;
	pEndResult->m_fEndLight = m_fEndLight;
}

//////////////////////////////////////////////////////////////////////////////
// SLFM_FinalLightingVert implementation

void SLFM_FinalLightingVert::SetLightIntensity(const CLightDef *pLight, float fIntensity)
{
	TLightIntensityMap::iterator iLight = m_aLightIntensities.find(pLight);
	if (iLight == m_aLightIntensities.end())
		m_aLightIntensities[pLight] = fIntensity;
	else
		iLight->second = fIntensity;
}

float SLFM_FinalLightingVert::GetLightIntensity(const CLightDef *pLight, float fDefault) const
{
	TLightIntensityMap::const_iterator iLight = m_aLightIntensities.find(pLight);
	if (iLight == m_aLightIntensities.end())
		return fDefault;

	return iLight->second;
}

void SLFM_FinalLightingVert::Lerp(const SLFM_FinalLightingVert &sStart, const SLFM_FinalLightingVert &sEnd, float fInterpolant)
{
	// Interpolate the position & normal
	VEC_LERP(m_vPos, sStart.m_vPos, sEnd.m_vPos, fInterpolant);
	VEC_LERP(m_vNormal, sStart.m_vNormal, sEnd.m_vNormal, fInterpolant);
	m_vNormal.Normalize();
	
	// Interpolate the lights from the starting vert
	TLightIntensityMap::const_iterator iCurLight = sStart.m_aLightIntensities.begin();
	for (; iCurLight != sStart.m_aLightIntensities.end(); ++iCurLight)
	{
		float fStartIntensity = iCurLight->second;
		float fEndIntensity = sEnd.GetLightIntensity(iCurLight->first, fStartIntensity);
		float fLerpIntensity = LTLERP(fStartIntensity, fEndIntensity, fInterpolant);
		SetLightIntensity(iCurLight->first, fLerpIntensity);
	}
	// Interpolate the lights from the ending vert
	iCurLight = sEnd.m_aLightIntensities.begin();
	for (; iCurLight != sEnd.m_aLightIntensities.end(); ++iCurLight)
	{
		float fEndIntensity = iCurLight->second;
		float fStartIntensity = sStart.GetLightIntensity(iCurLight->first, fEndIntensity);
		float fLerpIntensity = LTLERP(fStartIntensity, fEndIntensity, fInterpolant);
		SetLightIntensity(iCurLight->first, fLerpIntensity);
	}
}

void SLFM_FinalLightingVert::BiLerp(const SLFM_FinalLightingVert &sStart, const SLFM_FinalLightingVert &sEndU, const SLFM_FinalLightingVert &sEndV, float fInterpolantU, float fInterpolantV)
{
	SLFM_FinalLightingVert sTemp;
	sTemp.Lerp(sStart, sEndU, fInterpolantU);
	Lerp(sTemp, sEndV, fInterpolantV);
}

bool SLFM_FinalLightingVert::Equals(const SLFM_FinalLightingVert &sOther) const
{
	const float k_fDupePosEpsilon = 0.01f;
	const float k_fDupeLightEpsilon = 0.1f;

	if (!m_vPos.NearlyEquals(sOther.m_vPos, k_fDupePosEpsilon))
		return false;

	if (m_aLightIntensities.empty() || sOther.m_aLightIntensities.empty())
		return true;

	TLightIntensityMap::const_iterator iCurLight = m_aLightIntensities.begin();
	for (; iCurLight != m_aLightIntensities.end(); ++iCurLight)
	{
		float fIntensity = iCurLight->second;
		float fOtherIntensity = sOther.GetLightIntensity(iCurLight->first, fIntensity);
		if (fabsf(fIntensity - fOtherIntensity) > k_fDupeLightEpsilon)
			return false;
	}

	return true;
}

void SLFM_FinalLightingVert::CopyExtra(const SLFM_FinalLightingVert &sOther)
{
	TLightIntensityMap::const_iterator iCurLight = sOther.m_aLightIntensities.begin();
	for (; iCurLight != sOther.m_aLightIntensities.end(); ++iCurLight)
	{
		m_aLightIntensities[iCurLight->first] = iCurLight->second;
	}
}

//////////////////////////////////////////////////////////////////////////////
// SLFM_Triangulation implementation

template <class T_Vert>
uint32 SLFM_Triangulation<T_Vert>::GetVertIndex(const T_Vert &vPos)
{
	const float k_fDuplicatePtEpsilon = 0.01f;
	const float k_fPtOnLineEpsilon = 0.001f;

	// Try a linear search..
	TVertList::iterator iCurVert = m_aVertices.begin();
	for (; iCurVert != m_aVertices.end(); ++iCurVert)
	{
		if (iCurVert->m_vPos.NearlyEquals(vPos.m_vPos, k_fDuplicatePtEpsilon))
			return iCurVert - m_aVertices.begin();
	}

	// Search the edges
	TEdgeList::iterator iCurEdge = m_aEdges.begin();
	for (; iCurEdge != m_aEdges.end(); ++iCurEdge)
	{
		const SVert &vStart = m_aVertices[iCurEdge->m_nStart];
		const SVert &vEnd = m_aVertices[iCurEdge->m_nEnd];
		float fIntersectTime;
		float fDist = PtLineSegDist(vPos.m_vPos, vStart.m_vPos, vEnd.m_vPos - vStart.m_vPos, &fIntersectTime);
		if (fDist < k_fPtOnLineEpsilon)
		{
			// Split the edge and return that one
			return SplitEdge(iCurEdge - m_aEdges.begin(), vPos, fIntersectTime);
		}
	}

	// Search the triangles
	TTriangleList::iterator iCurTri = m_aTriangles.begin();
	for (; iCurTri != m_aTriangles.end(); ++iCurTri)
	{
		uint32 nVert0 = m_aEdges[iCurTri->m_nEdge0].m_nStart;
		uint32 nVert1 = m_aEdges[iCurTri->m_nEdge0].m_nEnd;
		uint32 nVert2 = m_aEdges[iCurTri->m_nEdge1].m_nStart;
		if ((nVert2 == nVert1) || (nVert2 == nVert0))
			nVert2 = m_aEdges[iCurTri->m_nEdge1].m_nEnd;
		const SVert &vPt0 = m_aVertices[nVert0];
		const SVert &vPt1 = m_aVertices[nVert1];
		const SVert &vPt2 = m_aVertices[nVert2];
		if (PointInTriangle(vPos.m_vPos, vPt0.m_vPos, vPt1.m_vPos, vPt2.m_vPos))
		{
			// Get the interpolants
			float fU, fV;
			GetBarycentricCoords(vPos.m_vPos, vPt0.m_vPos, vPt1.m_vPos - vPt0.m_vPos, vPt2.m_vPos - vPt0.m_vPos, &fU, &fV);
			SVert sNewVert;
			// Interpolate
			sNewVert.BiLerp(vPt0, vPt1, vPt2, fU, fV);
			// Force the position & lighting
			sNewVert.m_vPos = vPos.m_vPos;
			sNewVert.CopyExtra(vPos);
			// Add the new vertex
			uint32 nNewIndex = m_aVertices.size();
			m_aVertices.push_back(sNewVert);
			// Add the new edges
			uint32 nNewEdges = m_aEdges.size();
			m_aEdges.push_back(SEdge(nVert0, nNewIndex, false));
			m_aEdges.push_back(SEdge(nVert1, nNewIndex, false));
			m_aEdges.push_back(SEdge(nVert2, nNewIndex, false));
			// Pull the triangle out of the list
			STriangle sOldTri(*iCurTri);
			m_aTriangles.erase(iCurTri);
			// Add 3 new triangles
			SEdge &sEdge1 = m_aEdges[sOldTri.m_nEdge1];
			bool bEdgesReversed = (sEdge1.m_nStart != nVert0) && (sEdge1.m_nEnd != nVert0);
			m_aTriangles.push_back(STriangle(sOldTri.m_nEdge0, nNewEdges, nNewEdges + 1));
			m_aTriangles.push_back(STriangle(bEdgesReversed ? sOldTri.m_nEdge1 : sOldTri.m_nEdge2, nNewEdges + 1, nNewEdges + 2));
			m_aTriangles.push_back(STriangle(bEdgesReversed ? sOldTri.m_nEdge2 : sOldTri.m_nEdge1, nNewEdges + 2, nNewEdges));
			// Return the new vertex
			return nNewIndex;
		}
	}

	ASSERT(!"Vertex insert encountered outside of polygon");
	m_aVertices.push_back(vPos);
	return m_aVertices.size() - 1;
}

template <class T_Vert>
uint32 SLFM_Triangulation<T_Vert>::GetExistingVertIndex(const T_Vert &vPos)
{
	// Try a linear search..
	TVertList::iterator iCurVert = m_aVertices.begin();
	for (; iCurVert != m_aVertices.end(); ++iCurVert)
	{
		if (iCurVert->Equals(vPos))
			return iCurVert - m_aVertices.begin();
	}

	return INVALID_INDEX;
}

template <class T_Vert>
bool SLFM_Triangulation<T_Vert>::FindEdge(uint32 nStart, uint32 nEnd, uint32 *pResultEdge, uint32 nStartSearch) const
{
	ASSERT(nStartSearch <= m_aEdges.size());

	TEdgeList::const_iterator iCurEdge = m_aEdges.begin() + nStartSearch;
	for (; iCurEdge != m_aEdges.end(); ++iCurEdge)
	{
		// Check this edge, in both directions
		if (((iCurEdge->m_nStart == nStart) && (iCurEdge->m_nEnd == nEnd)) ||
			((iCurEdge->m_nStart == nEnd) && (iCurEdge->m_nEnd == nStart)))
		{
			*pResultEdge = iCurEdge - m_aEdges.begin();
			return true;
		}
	}

	// No edge was found
	return false;
}

bool DoLinesCross(const LTVector &vLine1Start, const LTVector &vLine1End, const LTVector &vLine2Start, const LTVector &vLine2End, const LTPlane &cPlane)
{
	const float k_fParallelEpsilon = 0.001f;
	const float k_fSameSideEpsilon = 0.001f;
	const float k_fEmptyEdgeEpsilon = 0.001f;
	const float k_fEndOfLineEpsilon = 0.001f;

	LTVector vLine2Offset = vLine2End - vLine2Start;
	float fLine2OffsetMag = vLine2Offset.Mag();
	if (fLine2OffsetMag < k_fEmptyEdgeEpsilon)
		return false;

	LTVector vLine2PlaneNormal = cPlane.m_Normal.Cross(vLine2Offset);
	vLine2PlaneNormal.Normalize();
	LTPlane cLine2Plane(vLine2PlaneNormal, vLine2Start);

	float fLine1StartDist = cLine2Plane.DistTo(vLine1Start);
	float fLine1EndDist = cLine2Plane.DistTo(vLine1End);

	if ((fabsf(fLine1StartDist) < k_fSameSideEpsilon) ||
		(fabsf(fLine1EndDist) < k_fSameSideEpsilon) ||
		((fLine1StartDist * fLine1EndDist) > 0.0f))
		return false;

	float fLine1DistDiff = fLine1StartDist - fLine1EndDist;
	if (fabsf(fLine1DistDiff) < k_fParallelEpsilon)
		return false;

	float fLine1IntersectTime = fLine1StartDist / fLine1DistDiff;
	fLine1IntersectTime = LTCLAMP(fLine1IntersectTime, 0.0f, 1.0f);
	LTVector vLine1PlaneIntersect;
	VEC_LERP(vLine1PlaneIntersect, vLine1Start, vLine1End, fLine1IntersectTime);

	LTVector vLine2Dir = vLine2Offset / fLine2OffsetMag;
	float fLine2Start = vLine2Dir.Dot(vLine2Start);

	float fLine1IntersectTimeOnLine2 = vLine2Dir.Dot(vLine1PlaneIntersect) - fLine2Start;

	if ((fLine1IntersectTimeOnLine2 < -k_fEndOfLineEpsilon) ||
		(fLine1IntersectTimeOnLine2 > (fLine2OffsetMag + k_fEndOfLineEpsilon)))
		return false;

	return true;
}

template <class T_Vert>
bool SLFM_Triangulation<T_Vert>::InsertHardEdge(uint32 nStart, uint32 nEnd)
{
	const float k_fIntersectEpsilon = 0.01f;
	const float k_fEndOfLineEpsilon = 0.001f;
	const float k_fEmptyTriangleEpsilon = 0.01f;
	const uint k_nIterationLimit = m_aEdges.size();

	// Don't insert empty edges
	if (nStart == nEnd)
		return true;

	SVert &vNewStart = m_aVertices[nStart];
	SVert &vNewEnd = m_aVertices[nEnd];

	// Use the edge-flip algorithm until there are no soft edges which cross the provided hard edge
	bool bFoundCrossing;
	bool bFoundStuckCrossing;
	bool bInsertedEdge = false;
	uint nIteration = 0;
	do
	{
		bFoundCrossing = false;
		bFoundStuckCrossing = false;
		// Go looking for crossings
		TEdgeList::iterator iCurEdge = m_aEdges.begin();
		for (; iCurEdge != m_aEdges.end(); ++iCurEdge)
		{
			// If we've already got this edge, mark it and continue
			if (((iCurEdge->m_nStart == nStart) && (iCurEdge->m_nEnd == nEnd)) ||
				((iCurEdge->m_nEnd == nStart) && (iCurEdge->m_nStart == nEnd)))
			{
				iCurEdge->m_bHardEdge = true;
				bInsertedEdge = true;
				break;
			}
			// Skip hard edges and edges that are already touching the new edge
			if (iCurEdge->m_bHardEdge)
				continue;
			// If it's guaranteed not to cross this edge, skip over it
			if ((iCurEdge->m_nStart == nStart) || (iCurEdge->m_nEnd == nEnd) ||
				(iCurEdge->m_nEnd == nStart) || (iCurEdge->m_nStart == nEnd))
				continue;
			// Does it cross the new edge?
			SVert &vStart = m_aVertices[iCurEdge->m_nStart];
			SVert &vEnd = m_aVertices[iCurEdge->m_nEnd];
			if (!DoLinesCross(vNewStart.m_vPos, vNewEnd.m_vPos, vStart.m_vPos, vEnd.m_vPos, m_cPlane))
				continue;
			
			// Find the two triangles which share this edge
			uint32 nCurEdgeIndex = iCurEdge - m_aEdges.begin();
			uint32 nTrisFound = 0;
			TTriangleList::iterator iCrossedTris[2];
			TTriangleList::iterator iCurTri = m_aTriangles.begin();
			for (; (iCurTri != m_aTriangles.end()) && (nTrisFound < 2); ++iCurTri)
			{
				if ((iCurTri->m_nEdge0 == nCurEdgeIndex) ||
					(iCurTri->m_nEdge1 == nCurEdgeIndex) ||
					(iCurTri->m_nEdge2 == nCurEdgeIndex))
				{
					iCrossedTris[nTrisFound] = iCurTri;
					++nTrisFound;
				}
			}
			if (nTrisFound < 2)
			{
				// Crossed edge without neighboring triangles found - this happens at singularities
				continue;
			}
			// Get the quad edges
			uint32 nQuadEdges[4];
			uint32 nNumQuadEdges = 0;
			for (uint32 nQuadTri = 0; nQuadTri < 2; ++nQuadTri)
			{
				for (uint32 nEdgeIndex = 0; nEdgeIndex < 3; ++nEdgeIndex)
				{
					if (iCrossedTris[nQuadTri]->Edge(nEdgeIndex) != nCurEdgeIndex)
					{
						nQuadEdges[nNumQuadEdges] = iCrossedTris[nQuadTri]->Edge(nEdgeIndex);
						++nNumQuadEdges;
					}
				}
			}
			ASSERT(nNumQuadEdges == 4);
			// Put the edges from the triangles in "touching start" to "touching end" order
			SEdge &sEdge0 = m_aEdges[nQuadEdges[0]];
			if ((sEdge0.m_nStart != iCurEdge->m_nStart) &&
				(sEdge0.m_nEnd != iCurEdge->m_nStart))
				std::swap(nQuadEdges[0], nQuadEdges[1]);
			SEdge &sEdge2 = m_aEdges[nQuadEdges[2]];
			if ((sEdge2.m_nStart != iCurEdge->m_nStart) &&
				(sEdge2.m_nEnd != iCurEdge->m_nStart))
				std::swap(nQuadEdges[2], nQuadEdges[3]);
			// Make an edge out of the other 2 points in the quad
			SEdge sSwapEdge;
			bool bReverseStart = (m_aEdges[nQuadEdges[0]].m_nStart != iCurEdge->m_nStart);
			sSwapEdge.m_nStart = bReverseStart ? m_aEdges[nQuadEdges[0]].m_nStart : m_aEdges[nQuadEdges[0]].m_nEnd;
			bool bReverseEnd = (m_aEdges[nQuadEdges[2]].m_nStart != iCurEdge->m_nStart);
			sSwapEdge.m_nEnd = bReverseEnd ? m_aEdges[nQuadEdges[2]].m_nStart : m_aEdges[nQuadEdges[2]].m_nEnd;
			// If this edge is the same as the one we're trying to insert, it will be the final "hard" edge
			sSwapEdge.m_bHardEdge = 
				((sSwapEdge.m_nStart == nStart) && (sSwapEdge.m_nEnd == nEnd)) ||
				((sSwapEdge.m_nStart == nEnd) && (sSwapEdge.m_nEnd == nStart));
			// If we can't flip it, set bFoundStuckCrossing and continue
			LTVector vQuadPts[4];
			vQuadPts[0] = m_aVertices[sSwapEdge.m_nStart].m_vPos;
			vQuadPts[1] = m_aVertices[sSwapEdge.m_nEnd].m_vPos;
			vQuadPts[2] = m_aVertices[iCurEdge->m_nStart].m_vPos;
			vQuadPts[3] = m_aVertices[iCurEdge->m_nEnd].m_vPos;
			LTVector vCurEdgeDir = vQuadPts[3] - vQuadPts[2];
			vCurEdgeDir.Normalize();
			LTVector vOtherTriEdge = (vQuadPts[0] - vQuadPts[2]);
			vOtherTriEdge.Normalize();
			LTVector vTriNormal = vCurEdgeDir.Cross(vOtherTriEdge);
			if (vTriNormal.MagSqr() > k_fEmptyTriangleEpsilon)
			{
				if (PointInTriangle(m_aVertices[iCurEdge->m_nEnd].m_vPos, vQuadPts[0], vQuadPts[1], vQuadPts[2]))
				{
					bFoundStuckCrossing = true;
					continue;
				}
			}
			vOtherTriEdge = (vQuadPts[1] - vQuadPts[2]);
			vOtherTriEdge.Normalize();
			vTriNormal = vCurEdgeDir.Cross(vOtherTriEdge);
			if (vTriNormal.MagSqr() > k_fEmptyTriangleEpsilon)
			{
				if (PointInTriangle(m_aVertices[iCurEdge->m_nStart].m_vPos, vQuadPts[0], vQuadPts[1], vQuadPts[3]))
				{
					bFoundStuckCrossing = true;
					continue;
				}
			}

			bInsertedEdge |= sSwapEdge.m_bHardEdge;

			// Flip the quad and set bFoundCrossing
			*iCurEdge = sSwapEdge;
			iCrossedTris[0]->m_nEdge0 = nCurEdgeIndex;
			iCrossedTris[0]->m_nEdge1 = nQuadEdges[0];
			iCrossedTris[0]->m_nEdge2 = nQuadEdges[2];
			iCrossedTris[1]->m_nEdge0 = nCurEdgeIndex;
			iCrossedTris[1]->m_nEdge1 = nQuadEdges[1];
			iCrossedTris[1]->m_nEdge2 = nQuadEdges[3];
			bFoundCrossing = true;
		}
		++nIteration;
		if (nIteration > k_nIterationLimit)
			break;
	} while (bFoundCrossing);

	return bInsertedEdge;
}

template <class T_Vert>
uint32 SLFM_Triangulation<T_Vert>::SplitEdge(uint32 nEdgeIndex, const T_Vert &vPos, float fInterpolant)
{
	SEdge &sEdge = m_aEdges[nEdgeIndex];
	SEdge sOriginalEdge = sEdge;
	SVert &vStart = m_aVertices[sEdge.m_nStart];
	SVert &vEnd = m_aVertices[sEdge.m_nEnd];
	// Make sure we've got the interpolant...
	if (fInterpolant < 0.0f)
	{
		float fDist = PtLineSegDist(vPos.m_vPos, vStart.m_vPos, vEnd.m_vPos - vStart.m_vPos, &fInterpolant);
		ASSERT((fInterpolant > 0.0f) && (fInterpolant < 1.0f));
	}
	SVert sNewVert;
	// Interpolate
	sNewVert.Lerp(vStart, vEnd, fInterpolant);
	// Override the actual position and lighting
	sNewVert.m_vPos = vPos.m_vPos;
	sNewVert.CopyExtra(vPos);
	// Add the vertex to the list
	uint32 nVertIndex = m_aVertices.size();
	m_aVertices.push_back(sNewVert);
	// Split up the edge
	uint32 nNewEdge = m_aEdges.size();
	uint32 nOldEnd = sEdge.m_nEnd;
	sEdge.m_nEnd = nVertIndex;
	// Note : No touching sEdge after this line!!!
	m_aEdges.push_back(SEdge(nVertIndex, nOldEnd, sEdge.m_bHardEdge));
	// Fix the triangles (Note : Can't use iterator here due to this process adding triangles..)
	for (uint32 nTriIndex = 0; nTriIndex != m_aTriangles.size(); ++nTriIndex)
	{
		STriangle &sCurTri = m_aTriangles[nTriIndex];
		uint32 nOldEdge;
		// Find the edge in this triangle
		for (nOldEdge = 0; nOldEdge < 3; ++nOldEdge)
			if (sCurTri.Edge(nOldEdge) == nEdgeIndex)
				break;
		if (nOldEdge == 3)
			continue;
		// Note : Everything below this line in the loop is really hard to follow, and I'm very
		// sorry about that.  Unfortunately, the edge ordering is completely un-guaranteed, so
		// it has to deal with every contingency by variably indexing into the triangle's edge list.

		// Find the edge that's on the "start" side of the old edge
		uint32 nStartEdge;
		bool bStartEdgeConnectedByStart;
		for (nStartEdge = 0; nStartEdge < 3; ++nStartEdge)
		{
			if (nStartEdge == nOldEdge)
				continue;
			SEdge &sTestEdge = m_aEdges[sCurTri.Edge(nStartEdge)];
			if (sTestEdge.m_nStart == sOriginalEdge.m_nStart)
			{
				bStartEdgeConnectedByStart = true;
				break;
			}
			else if (sTestEdge.m_nEnd == sOriginalEdge.m_nStart)
			{
				bStartEdgeConnectedByStart = false;
				break;
			}
		}
		if (nStartEdge == 3)
		{
			ASSERT(!"Invalid triangle detected!");
			continue;
		}
		// Find the "other" edge
		uint32 nOtherEdge;
		switch (nOldEdge)
		{
			case 0 : nOtherEdge = (nStartEdge == 1) ? 2 : 1; break;
			case 1 : nOtherEdge = (nStartEdge == 2) ? 0 : 2; break;
			case 2 : nOtherEdge = (nStartEdge == 0) ? 1 : 0; break;
		}
		// Create the new edge
		uint32 nSplitTriEdge = m_aEdges.size();
		const SEdge &sStartEdge = m_aEdges[sCurTri.Edge(nStartEdge)];
		m_aEdges.push_back(SEdge(nVertIndex, (bStartEdgeConnectedByStart) ? sStartEdge.m_nEnd : sStartEdge.m_nStart, false)); 
		// Get the index of the "other" edge so we can create the new triangle
		uint32 nOtherEdgeIndex = sCurTri.Edge(nOtherEdge);
		// Update the old triangle
		sCurTri.Edge(nOtherEdge) = nSplitTriEdge;
		// Create a new triangle
		m_aTriangles.push_back(STriangle(nNewEdge, nSplitTriEdge, nOtherEdgeIndex));
	}

	return nVertIndex;
}

template <class T_Vert>
void SLFM_Triangulation<T_Vert>::GetTriVerts(const STriangle &sTri, uint32 *pVert0, uint32 *pVert1, uint32 *pVert2) const
{
	const SEdge &sFirstEdge = m_aEdges[sTri.m_nEdge0];
	*pVert0 = sFirstEdge.m_nStart;
	*pVert1 = sFirstEdge.m_nEnd;
	const SEdge &sOtherEdge = m_aEdges[sTri.m_nEdge1];
	if ((sOtherEdge.m_nStart == sFirstEdge.m_nStart) || (sOtherEdge.m_nStart == sFirstEdge.m_nEnd))
		*pVert2 = sOtherEdge.m_nEnd;
	else
		*pVert2 = sOtherEdge.m_nStart;
}

template <class T_Vert>
bool SLFM_Triangulation<T_Vert>::IsFrontFacing(uint32 nIndex0, uint32 nIndex1, uint32 nIndex2, const LTVector &vNormal) const
{
	ASSERT(nIndex0 < m_aVertices.size());
	ASSERT(nIndex1 < m_aVertices.size());
	ASSERT(nIndex2 < m_aVertices.size());
	const LTVector &vOrigin = m_aVertices[nIndex1].m_vPos;
	LTVector vEdge0 = m_aVertices[nIndex0].m_vPos - vOrigin;
	LTVector vEdge1 = m_aVertices[nIndex2].m_vPos - vOrigin;
	return vEdge0.Cross(vEdge1).Dot(vNormal) > 0.0f;
}

//////////////////////////////////////////////////////////////////////////////
// CLightFragmentMaker implementation

CLightFragmentMaker::CLightFragmentMaker(const CLightMapMaker &cLMM) :
	m_cLMM(cLMM) 
{
}

CLightFragmentMaker::~CLightFragmentMaker()
{
	ClearOutlines();
}

void CLightFragmentMaker::LoadPolys(CPrePoly * const *pPolyArray, uint32 nPolyCount)
{
	m_aWorldPolys.insert(m_aWorldPolys.end(), pPolyArray, &pPolyArray[nPolyCount]);
}

void CLightFragmentMaker::LoadLights(const CLightDef * const *pLights, uint32 nLightCount)
{
	m_aWorldLights.insert(m_aWorldLights.end(), pLights, &pLights[nLightCount]);
}

void CLightFragmentMaker::CountCastingLights()
{
	m_nCastingLights = 0;

	TLightDefList::const_iterator iCurLight = m_aWorldLights.begin();
	for (; iCurLight != m_aWorldLights.end(); ++iCurLight)
	{
		if ((*iCurLight)->m_bShadowMesh)
			++m_nCastingLights;
	}
}

void CLightFragmentMaker::Execute()
{
	// Set up the polygon data
	BuildOutlines();

	CountCastingLights();

	// Find neighboring polygons if we're actually going to do something interesting
	if ((m_nCastingLights != 0) && (m_nReceiverCount != 0))
		FindNeighbors();

	// Reserve space for the edges
	m_aFragmentEdges.resize(m_aWorldPolys.size());

	// Temporary edge lists
	TEdgeList aEdges;
	TEdgeList aNewEdges;

	DrawStatusText(eST_Normal, "  Generating shadow edges");

	// For each light in the world...
	TLightDefList::iterator iCurLight = m_aWorldLights.begin();
	for (; iCurLight != m_aWorldLights.end(); ++iCurLight)
	{
		// Update the progress bar
		SetProgressBar((float)(iCurLight - m_aWorldLights.begin()) / (float)m_aWorldLights.size());

		if (!(*iCurLight)->m_bShadowMesh)
			continue;

		// Get the silhouette edges for this light
		aEdges.clear();
		FindSilhouetteEdges(*iCurLight, &aEdges);
		if (aEdges.empty())
			continue;

		// Create projected edge fragments for the polys
		ProjectEdges(*iCurLight, aEdges);
	}

	DrawStatusText(eST_Normal, "  Triangulating");

	// Triangulate the polys and add the projected shadow edges to the triangulations
	m_aTriangulations.resize(m_aOutlines.size());
	TOutlineList::iterator iCurOutline = m_aOutlines.begin();
	TTriangulationList::iterator iCurPoly = m_aTriangulations.begin();
	TShadowEdgeListList::iterator iCurEdgeList = m_aFragmentEdges.begin();
	TShadowEdgeList aNewShadowEdges;
	for (; iCurOutline != m_aOutlines.end(); ++iCurOutline, ++iCurPoly, ++iCurEdgeList)
	{
		// Update the progress bar
		SetProgressBar((float)(iCurOutline - m_aOutlines.begin()) / (float)m_aOutlines.size());

		aNewShadowEdges.clear();
		FixCrossedEdges((*iCurOutline)->m_cPlane, *iCurEdgeList, &aNewShadowEdges);
		iCurEdgeList->swap(aNewShadowEdges);
		Triangulate(**iCurOutline, *iCurEdgeList, &(*iCurPoly));
	}

	DrawStatusText(eST_Normal, "  Lighting");

	// Apply lighting to the triangulations
	iCurPoly = m_aTriangulations.begin();
	for (; iCurPoly != m_aTriangulations.end(); ++iCurPoly)
	{
		// Update the progress bar
		SetProgressBar((float)(iCurPoly - m_aTriangulations.begin()) / (float)m_aTriangulations.size());

		SLFM_Triangulation<SLFM_FinalLightingVert>::TVertList::iterator iCurVert = iCurPoly->m_aVertices.begin();
		for (; iCurVert != iCurPoly->m_aVertices.end(); ++iCurVert)
		{
			iCurVert->m_vColor = m_cLMM.GetAmbientLight(iCurVert->m_vPos);
			iCurVert->m_vColor += m_cLMM.GetSunlight(iCurVert->m_vPos, iCurVert->m_vNormal);

			TLightDefList::iterator iCurLight = m_aWorldLights.begin();
			for (; iCurLight != m_aWorldLights.end(); ++iCurLight)
			{
				float fIntensity = iCurVert->GetLightIntensity(*iCurLight, -1.0f);
				if (fIntensity < 0.0f)
					fIntensity = m_cLMM.DoesLightHit(*iCurLight, iCurVert->m_vPos + iCurVert->m_vNormal * g_pGlobs->m_fLMSampleExtrude) ? 1.0f : 0.0f;
				if (fIntensity > 0.0f)
					iCurVert->m_vColor += m_cLMM.CalcLightColor(*iCurLight, iCurVert->m_vPos, iCurVert->m_vNormal) * fIntensity;
			}

			float fBiggest = LTMAX(iCurVert->m_vColor.x, iCurVert->m_vColor.y);
			fBiggest = LTMAX(fBiggest, iCurVert->m_vColor.z);
			if (fBiggest > 255.0f)
				iCurVert->m_vColor *= 255.0f / fBiggest;
		}
	}

	DrawStatusText(eST_Normal, "  Writing results");

	// Copy the results into the world polys
	iCurPoly = m_aTriangulations.begin();
	iCurOutline = m_aOutlines.begin();
	for (; iCurPoly != m_aTriangulations.end(); ++iCurPoly, ++iCurOutline)
	{
		// Update the progress bar
		SetProgressBar((float)(iCurPoly - m_aTriangulations.begin()) / (float)m_aTriangulations.size());

		WriteTriangulation(*iCurPoly, (*iCurOutline)->m_pPrePoly);
	}
}

void CLightFragmentMaker::BuildOutlines()
{
	m_aOutlines.reserve(m_aWorldPolys.size());
	m_nReceiverCount = 0;

	TPrePolyList::iterator iCurWorldPoly = m_aWorldPolys.begin();
	for (; iCurWorldPoly != m_aWorldPolys.end(); ++iCurWorldPoly)
	{
		m_aOutlines.push_back(new SLFM_Outline(*iCurWorldPoly));
		if (((*iCurWorldPoly)->GetSurfaceFlags() & SURF_SHADOWMESH) != 0)
			++m_nReceiverCount;
	}
}

void CLightFragmentMaker::ClearOutlines()
{
	while (!m_aOutlines.empty())
	{
		delete m_aOutlines.back();
		m_aOutlines.pop_back();
	}
}

void CLightFragmentMaker::FindNeighbors()
{
	DrawStatusText(eST_Normal, "  Finding Neighbors");

	// Note : This is currently implemented with a very very brute-force method
	// Also note : T-Junctions are not yet handled, but will need to be, since the physics poly generation
	// isn't going to guarantee that in the future.
	TOutlineList::iterator iCurOutline = m_aOutlines.begin();
	for (; iCurOutline != m_aOutlines.end(); ++iCurOutline)
	{
		// Update the progress bar
		SetProgressBar((float)(iCurOutline - m_aOutlines.begin()) / (float)m_aOutlines.size());

		SLFM_Outline *pOutline = *iCurOutline;
		// Initialize the neighbor list
		pOutline->m_aNeighbors.resize(pOutline->m_aVertices.size(), 0);
		SLFM_Outline::TNeighborList::iterator iCurNeighbor = pOutline->m_aNeighbors.begin();
		LTVector vPrevPt = pOutline->m_aVertices.back();
		SLFM_Outline::TVertList::iterator iCurVert = pOutline->m_aVertices.begin();
		for (; iCurVert != pOutline->m_aVertices.end(); ++iCurVert, ++iCurNeighbor)
		{
			LTVector &vNextPt = *iCurVert;

			// Look through the outline list for a polygon with the same edge
			TOutlineList::iterator iNeighborOutline = m_aOutlines.begin();
			for (; iNeighborOutline != m_aOutlines.end() && !*iCurNeighbor; ++iNeighborOutline)
			{
				SLFM_Outline *pNeighbor = *iNeighborOutline;

				// Don't look in the current poly
				if (pNeighbor == pOutline)
					continue;

				// Don't bother looking for a neighbor if it doesn't overlap..
				if (!pOutline->OverlapExtents(*pNeighbor))
					continue;

				// Skip inverse co-planar polys
				if (pNeighbor->m_cPlane.m_Normal.NearlyEquals(-pOutline->m_cPlane.m_Normal, 0.0001f))
					continue;

				SLFM_Outline::TVertList::iterator iNeighborVert = pNeighbor->m_aVertices.begin();
				LTVector vNeighborPrev = pNeighbor->m_aVertices.back();
				for (; iNeighborVert != pNeighbor->m_aVertices.end(); ++iNeighborVert)
				{
					if (vNextPt.NearlyEquals(vNeighborPrev, 0.0001f) && vPrevPt.NearlyEquals(*iNeighborVert, 0.0001f))
					{
						*iCurNeighbor = pNeighbor;
						break;
					}
					vNeighborPrev = *iNeighborVert;
				}
			}

			vPrevPt = vNextPt;
		}
	}
}

PolySide GetPlaneSide(const SLFM_Outline &sOutline, const LTPlane &cPlane)
{
	uint32 nFront = 0;
	uint32 nBack = 0;

	LTVector vPrevPt = sOutline.m_aVertices.back();
	SLFM_Outline::TVertList::const_iterator iCurVert = sOutline.m_aVertices.begin();
	for (; iCurVert != sOutline.m_aVertices.end(); ++iCurVert)
	{
		float fNextDist;
		PolySide nNextSide = GetPointSide(cPlane, *iCurVert, &fNextDist);
		switch (nNextSide)
		{
			case Intersect :
				break;
			case FrontSide :
				// Add it to the front
				++nFront;
				break;
			case BackSide :
				// Add it to the back
				++nBack;
				break;
		}
		vPrevPt = *iCurVert;
	}

	if (nFront == 0)
	{
		if (nBack == 0)
		{
			// Consider co-planar polys to be on the back-side (algorithmic decision)
			return BackSide;
			// return sOutline.m_cPlane.m_Normal.Dot(cPlane.m_Normal) > 0 ? FrontSide : BackSide;
		}
		else
			return BackSide;
	}
	else if (nBack == 0)
		return FrontSide;
	else
		return Intersect;
}

void CLightFragmentMaker::FindSilhouetteEdges(const CLightDef *pLightDef, TEdgeList *pEdges) const
{
	const float k_fSilhouetteEpsilon = -0.001f;

	// Run through the polys looking for neighbors that face away
	TOutlineList::const_iterator iCurOutline = m_aOutlines.begin();
	for (; iCurOutline != m_aOutlines.end(); ++iCurOutline)
	{
		// Skip back-facing polys
		if ((*iCurOutline)->m_cPlane.DistTo(pLightDef->m_Pos) < k_fSilhouetteEpsilon)
			continue;

		// Do we want to cast a shadow?
		bool bCurClipLight = ((*iCurOutline)->m_pPrePoly->GetSurfaceFlags() & SURF_CASTSHADOWMESH) != 0;

		// Go through the neighbors looking for silhouette edges
		LTVector vPrevPt = (*iCurOutline)->m_aVertices.back();
		SLFM_Outline::TVertList::iterator iNextPt = (*iCurOutline)->m_aVertices.begin();
		SLFM_Outline::TNeighborList::iterator iCurNeighbor = (*iCurOutline)->m_aNeighbors.begin();
		for (; iNextPt != (*iCurOutline)->m_aVertices.end(); ++iNextPt, ++iCurNeighbor)
		{
			bool bNeighborClipLight = bCurClipLight;
			if (*iCurNeighbor)
			{
				bNeighborClipLight |= (((*iCurNeighbor)->m_pPrePoly->GetSurfaceFlags() & SURF_CASTSHADOWMESH) != 0);
			}
			// If there's no neighbor, or it's a back-facing poly, it's a silhouette edge as long as it's supposed to cast a shadow
			if (bNeighborClipLight && 
				(!*iCurNeighbor || ((*iCurNeighbor)->m_cPlane.DistTo(pLightDef->m_Pos) < k_fSilhouetteEpsilon)))
			{
				pEdges->push_back(SLFM_Edge(vPrevPt, *iNextPt, *iCurOutline, *iCurNeighbor));
			}

			vPrevPt = *iNextPt;
		}
	}
}

bool CLightFragmentMaker::LightTouchesPoly(const CLightDef *pLightDef, const SLFM_Outline &sOutline) const
{
	// Cull against the main plane of the poly
	float fPlaneDist = sOutline.m_cPlane.DistTo(pLightDef->m_Pos);
	if ((fPlaneDist < 0.001f) || (fPlaneDist > pLightDef->m_MaxDist))
		return false;

	// Look for an edge whose plane has the light too far on the front side
	LTVector vPrevPt = sOutline.m_aVertices.back();
	SLFM_Outline::TVertList::const_iterator iCurVert = sOutline.m_aVertices.begin();
	for (; iCurVert != sOutline.m_aVertices.end(); ++iCurVert)
	{
		LTVector vNormal = sOutline.m_cPlane.m_Normal.Cross(*iCurVert - vPrevPt);
		vNormal.Normalize();
		LTPlane cEdgePlane(vNormal, *iCurVert);
		vPrevPt = *iCurVert;
		if (cEdgePlane.DistTo(pLightDef->m_Pos) > pLightDef->m_MaxDist)
			return false;
	}

	// Assume it's touching the poly then...
	return true;
}

// Temporary implementation details start here

// Calc a ray/plane intersection  (The easy/slow way for now...)
LTVector GetRayPlaneIntersect(const LTVector &vStart, const LTVector &vEnd, const LTPlane &cPlane)
{
	LTVector vDir = vEnd - vStart;
	vDir.Normalize();
	float fDirPlaneDot = -vDir.Dot(cPlane.m_Normal);
	if (fabsf(fDirPlaneDot) < 0.0001f)
		return vStart - cPlane.m_Normal * cPlane.DistTo(vStart);
	else
		return vStart + vDir * (cPlane.DistTo(vStart) / fDirPlaneDot);
}

struct SProjEdge
{
public:
	SProjEdge() { m_aOutlines[0] = 0; m_aOutlines[1] = 0; }
	SProjEdge(const SProjEdge &sOther) : 
		m_vStartPos(sOther.m_vStartPos), m_vEndPos(sOther.m_vEndPos),
		m_vStartProj(sOther.m_vStartProj), m_vEndProj(sOther.m_vEndProj)
	{
		m_aOutlines[0] = sOther.m_aOutlines[0];
		m_aOutlines[1] = sOther.m_aOutlines[1];
	}
	SProjEdge(const SLFM_Edge &sEdge, const LTPlane &cPlane, const LTVector &vPos) :
		m_vStartPos(sEdge.m_vStart), m_vEndPos(sEdge.m_vEnd)
	{
		m_aOutlines[0] = sEdge.m_pOutlines[0];
		m_aOutlines[1] = sEdge.m_pOutlines[1];
		ReProject(cPlane, vPos);
	}
	SProjEdge &operator=(const SProjEdge &sOther)
	{
		m_vStartPos = sOther.m_vStartPos;
		m_vEndPos = sOther.m_vEndPos;
		m_vStartProj = sOther.m_vStartProj;
		m_vEndProj = sOther.m_vEndProj;
		m_aOutlines[0] = sOther.m_aOutlines[0];
		m_aOutlines[1] = sOther.m_aOutlines[1];
		return *this;
	}
	bool IsNeighbor(const SLFM_Outline *pOutline) const { return (pOutline == m_aOutlines[0]) || (pOutline == m_aOutlines[1]); }

	void ReProject(const LTPlane &cPlane, const LTVector &vPos);

	PolySide SplitPlane(const LTPlane &cPlane, SProjEdge *pResultFront, SProjEdge *pResultBack);
	PolySide GetPlaneSide(const LTPlane &cPlane, float *pIntersectTime, bool *pStartFront);
public:
	LTVector m_vStartPos, m_vEndPos;
	LTVector m_vStartProj, m_vEndProj;
	const SLFM_Outline *m_aOutlines[2];
};

void SProjEdge::ReProject(const LTPlane &cPlane, const LTVector &vPos)
{
	bool bDidProject;
	bDidProject = ProjectPoint(cPlane, vPos, m_vStartPos, &m_vStartProj);
	ASSERT(bDidProject);
	bDidProject = ProjectPoint(cPlane, vPos, m_vEndPos, &m_vEndProj);
	ASSERT(bDidProject);
}

PolySide SProjEdge::GetPlaneSide(const LTPlane &cPlane, float *pIntersectTime, bool *pStartFront)
{
	float fStartDist, fEndDist;
	PolySide nStartSide = GetPointSide(cPlane, m_vStartProj, &fStartDist);
	PolySide nEndSide = GetPointSide(cPlane, m_vEndProj, &fEndDist);

	if (nStartSide == Intersect)
		nStartSide = nEndSide;
	else if (nEndSide == Intersect)
		nEndSide = nStartSide;

	if (nStartSide == nEndSide)
	{
		// If it's on the plane, don't consider it to be an intersection
		if (nStartSide == Intersect)
			return FrontSide;
		else
			return nStartSide;
	}

	*pIntersectTime = fStartDist / (fStartDist - fEndDist);
	*pStartFront = nStartSide == FrontSide;

	return Intersect;
}

PolySide SProjEdge::SplitPlane(const LTPlane &cPlane, SProjEdge *pResultFront, SProjEdge *pResultBack)
{
	// Perform the plane intersection
	float fIntersectTime;
	bool bStartFront;
	PolySide nSideResult = GetPlaneSide(cPlane, &fIntersectTime, &bStartFront);
	if (nSideResult != Intersect)
		return nSideResult;

	// Get the intersection points
	LTVector vIntersectProj;
	VEC_LERP(vIntersectProj, m_vStartProj, m_vEndProj, fIntersectTime);
	LTVector vIntersectPos;
	VEC_LERP(vIntersectPos, m_vStartPos, m_vEndPos, fIntersectTime);

	if (!bStartFront)
		std::swap(pResultFront, pResultBack);

	pResultFront->m_vStartPos = m_vStartPos;
	pResultFront->m_vStartProj = m_vStartProj;
	pResultFront->m_vEndPos = vIntersectPos;
	pResultFront->m_vEndProj = vIntersectProj;
	pResultFront->m_aOutlines[0] = m_aOutlines[0];
	pResultFront->m_aOutlines[1] = m_aOutlines[1];

	pResultBack->m_vStartPos = vIntersectPos;
	pResultBack->m_vStartProj = vIntersectProj;
	pResultBack->m_vEndPos = m_vEndPos;
	pResultBack->m_vEndProj = m_vEndProj;
	pResultBack->m_aOutlines[0] = m_aOutlines[0];
	pResultBack->m_aOutlines[1] = m_aOutlines[1];

	return Intersect;
}

typedef std::vector<SProjEdge> TProjEdgeList;

// Subdivision of space created by the intersection of planes
// Note that this is only actually a convex polyhedron if m_bInteriorSpace is true
class CConvexPolyhedron
{
public:
	CConvexPolyhedron() : m_pOutline(0) {}
	void Flip() {
		TPlaneList::iterator iCurPlane = m_aPlanes.begin();
		for (; iCurPlane != m_aPlanes.end(); ++iCurPlane)
			*iCurPlane = -*iCurPlane;
		m_bInteriorSpace = !m_bInteriorSpace;
	}

	typedef std::vector<LTPlane> TPlaneList;
	TPlaneList m_aPlanes;
	const SLFM_Outline *m_pOutline;

	bool m_bInteriorSpace;
};
typedef std::vector<CConvexPolyhedron> TConvexPolyhedronList;

void CreateLightFrustum(const SLFM_Outline &sOutline, const CLightDef *pLightDef, CConvexPolyhedron *pPolyhedron)
{
	ASSERT(pPolyhedron);

	pPolyhedron->m_aPlanes.clear();
	pPolyhedron->m_pOutline = &sOutline;
	pPolyhedron->m_bInteriorSpace = true;

	// Add the base of the frustum
	pPolyhedron->m_aPlanes.push_back(sOutline.m_cPlane);

	// Add the planes around the edges
	LTVector vPrevPt = sOutline.m_aVertices.back();
	SLFM_Outline::TVertList::const_iterator iCurVert = sOutline.m_aVertices.begin();
	for (; iCurVert != sOutline.m_aVertices.end(); ++iCurVert)
	{
		LTVector vEdge = *iCurVert - vPrevPt;
		LTVector vNormal = vEdge.Cross(pLightDef->m_Pos - vPrevPt);
		vNormal.Normalize();
		pPolyhedron->m_aPlanes.push_back(LTPlane(vNormal, vPrevPt));
		vPrevPt = *iCurVert;
	}

	// Add the top of the frustum
	LTVector vInvNormal = -sOutline.m_cPlane.m_Normal;
	pPolyhedron->m_aPlanes.push_back(LTPlane(vInvNormal, pLightDef->m_Pos + vInvNormal * pLightDef->m_fSize));
}

void CreateShadowFrustum(const SLFM_Outline &sOutline, const CLightDef *pLightDef, CConvexPolyhedron *pPolyhedron)
{
	ASSERT(pPolyhedron);

	pPolyhedron->m_aPlanes.clear();
	pPolyhedron->m_pOutline = &sOutline;
	pPolyhedron->m_bInteriorSpace = false;

	// Add the planes around the edges
	LTVector vPrevPt = sOutline.m_aVertices.back();
	SLFM_Outline::TVertList::const_iterator iCurVert = sOutline.m_aVertices.begin();
	for (; iCurVert != sOutline.m_aVertices.end(); ++iCurVert)
	{
		LTVector vEdge = *iCurVert - vPrevPt;
		LTVector vNormal = vEdge.Cross(vPrevPt - pLightDef->m_Pos);
		vNormal.Normalize();
		pPolyhedron->m_aPlanes.push_back(LTPlane(vNormal, vPrevPt));
		vPrevPt = *iCurVert;
	}
}

void CreateProjShadowFrustum(const SLFM_Outline &sOutline, const CLightDef *pLightDef, const LTPlane &cPlane, CConvexPolyhedron *pPolyhedron)
{
	ASSERT(pPolyhedron);

	pPolyhedron->m_aPlanes.clear();
	pPolyhedron->m_pOutline = &sOutline;
	pPolyhedron->m_bInteriorSpace = false;

	// Add the planes around the edges
	LTVector vPrevPt = GetRayPlaneIntersect(pLightDef->m_Pos, sOutline.m_aVertices.back(), cPlane);
	SLFM_Outline::TVertList::const_iterator iCurVert = sOutline.m_aVertices.begin();
	for (; iCurVert != sOutline.m_aVertices.end(); ++iCurVert)
	{
		LTVector vCurPt = GetRayPlaneIntersect(pLightDef->m_Pos, *iCurVert, cPlane); 
		LTVector vEdge = vCurPt - vPrevPt;
		LTVector vNormal = cPlane.m_Normal.Cross(vEdge);
		vNormal.Normalize();
		pPolyhedron->m_aPlanes.push_back(LTPlane(vNormal, vPrevPt));
		vPrevPt = vCurPt;
	}
}

// Note : Outline neighbors and planes are not updated by this function.
PolySide SplitOutline(const SLFM_Outline &sOutline, const LTPlane &cPlane, SLFM_Outline *pFrontOutline, SLFM_Outline *pBackOutline)
{
	uint32 nFront = 0;
	uint32 nBack = 0;

	pFrontOutline->m_aVertices.clear();
	pBackOutline->m_aVertices.clear();

	LTVector vPrevPt = sOutline.m_aVertices.back();
	float fPrevDist;
	PolySide nPrevSide = GetPointSide(cPlane, vPrevPt, &fPrevDist);
	SLFM_Outline::TVertList::const_iterator iCurVert = sOutline.m_aVertices.begin();
	for (; iCurVert != sOutline.m_aVertices.end(); ++iCurVert)
	{
		float fNextDist;
		PolySide nNextSide = GetPointSide(cPlane, *iCurVert, &fNextDist);
		if ((nNextSide != nPrevSide) && (nNextSide != Intersect) && (nPrevSide != Intersect))
		{
			// Calculate an intersect
			float fInterpolant = fNextDist / (fNextDist - fPrevDist);
			LTVector vIntersectPt;
			VEC_LERP(vIntersectPt, *iCurVert, vPrevPt, fInterpolant);
			// Add it to both sides
			pFrontOutline->m_aVertices.push_back(vIntersectPt);
			pBackOutline->m_aVertices.push_back(vIntersectPt);
		}
		switch (nNextSide)
		{
			case Intersect :
				// Add it to both sides
				pFrontOutline->m_aVertices.push_back(*iCurVert);
				pBackOutline->m_aVertices.push_back(*iCurVert);
				break;
			case FrontSide :
				// Add it to the front
				++nFront;
				pFrontOutline->m_aVertices.push_back(*iCurVert);
				break;
			case BackSide :
				// Add it to the back
				++nBack;
				pBackOutline->m_aVertices.push_back(*iCurVert);
				break;
		}
		vPrevPt = *iCurVert;
		nPrevSide = nNextSide;
		fPrevDist = fNextDist;
	}

	if (nFront == 0)
	{
		if (nBack == 0)
		{
			// Consider co-planar polys to be on the back-side (algorithmic decision)
			return BackSide;
			// return sOutline.m_cPlane.m_Normal.Dot(cPlane.m_Normal) > 0 ? FrontSide : BackSide;
		}
		else
			return BackSide;
	}
	else if (nBack == 0)
		return FrontSide;
	else
		return Intersect;
}

void ClipOutlineListToPolyhedron(const TSLFM_OutlineList &sOutlineList, const CConvexPolyhedron &cPolyhedron, TSLFM_OutlineList *pResultList)
{
	if (cPolyhedron.m_bInteriorSpace)
	{
		TSLFM_OutlineList::const_iterator iCurOutline = sOutlineList.begin();
		for (; iCurOutline != sOutlineList.end(); ++iCurOutline)
		{
			bool bFoundInteriorPortion = true;
			SLFM_Outline sInteriorPortion = *iCurOutline;
			SLFM_Outline sFront = sInteriorPortion, sBack = sInteriorPortion;
			CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
			for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
			{
				PolySide nPolySide = SplitOutline(sInteriorPortion, *iCurPlane, &sFront, &sBack);
				if (nPolySide == BackSide)
				{
					bFoundInteriorPortion = false;
					break;
				}
				if (nPolySide == Intersect)
					sInteriorPortion.Swap(sFront);
			}
			if (bFoundInteriorPortion)
				pResultList->push_back(sInteriorPortion);
		}
	}
	else
	{
		TSLFM_OutlineList::const_iterator iCurOutline = sOutlineList.begin();
		for (; iCurOutline != sOutlineList.end(); ++iCurOutline)
		{
			SLFM_Outline sInteriorPortion = *iCurOutline;
			SLFM_Outline sFront = sInteriorPortion, sBack = sInteriorPortion;
			CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
			for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
			{
				PolySide nPolySide = SplitOutline(sInteriorPortion, *iCurPlane, &sFront, &sBack);
				if (nPolySide != BackSide)
					pResultList->push_back(sFront);
				if (nPolySide == FrontSide)
					break;
				sInteriorPortion.Swap(sBack);
			}
		}
	}
}

bool OutlineTouchesPolyhedron(const SLFM_Outline &sOutline, const CConvexPolyhedron &cPolyhedron)
{
	if (cPolyhedron.m_bInteriorSpace)
	{
		SLFM_Outline sInteriorPortion = sOutline;
		SLFM_Outline sFront = sInteriorPortion, sBack = sInteriorPortion;
		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
		for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
		{
			PolySide nPolySide = SplitOutline(sInteriorPortion, *iCurPlane, &sFront, &sBack);
			if (nPolySide == BackSide)
				return false;
			if (nPolySide == Intersect)
				sInteriorPortion.Swap(sFront);
		}
		return true;
	}
	else
	{
		SLFM_Outline sInteriorPortion = sOutline;
		SLFM_Outline sFront = sInteriorPortion, sBack = sInteriorPortion;
		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
		for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
		{
			PolySide nPolySide = SplitOutline(sInteriorPortion, *iCurPlane, &sFront, &sBack);
			if (nPolySide != BackSide)
				return true;
			sInteriorPortion.Swap(sBack);
		}
		return false;
	}
}

// Note : pResultEdgeList is appended to instead of re-created
bool ClipEdgeToPolyhedron(const SLFM_Edge &sEdge, const CConvexPolyhedron &cPolyhedron, TSLFM_EdgeList *pResultEdgeList)
{
	if (cPolyhedron.m_bInteriorSpace)
	{
		SLFM_Edge sInteriorPortion = sEdge;
		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
		for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
		{
			PolySide nPolySide;
			bool bStartFront;
			SLFM_Edge sOtherEdge;
			sInteriorPortion.SplitPlane(*iCurPlane, &sOtherEdge, &nPolySide, &bStartFront);
			if (nPolySide == BackSide)
				return false;
			if (!bStartFront)
				std::swap(sInteriorPortion, sOtherEdge);
		}
		pResultEdgeList->push_back(sInteriorPortion);
		return true;
	}
	else
	{
		bool bFoundFront = false;

		SLFM_Edge sCurEdge = sEdge;
		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
		for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
		{
			PolySide nPolySide;
			bool bStartFront;
			SLFM_Edge sOtherEdge;
			sCurEdge.SplitPlane(*iCurPlane, &sOtherEdge, &nPolySide, &bStartFront);
			if (nPolySide == FrontSide)
			{
				pResultEdgeList->push_back(sCurEdge);
				return true;
			}
			else if (nPolySide == Intersect)
			{
				if (bStartFront)
				{
					pResultEdgeList->push_back(sCurEdge);
					std::swap(sCurEdge, sOtherEdge);
				}
				else
					pResultEdgeList->push_back(sOtherEdge);
				bFoundFront = true;
			}
		}
		return bFoundFront;
	}
}

bool ClipEdgeToPolyhedron(const SProjEdge &sEdge, const CConvexPolyhedron &cPolyhedron, TProjEdgeList *pResultEdgeList)
{
	if (cPolyhedron.m_bInteriorSpace)
	{
		SProjEdge sInteriorPortion = sEdge;
		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
		for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
		{
			SProjEdge sFront, sBack;
			PolySide nPolySide = sInteriorPortion.SplitPlane(*iCurPlane, &sFront, &sBack);
			if (nPolySide == BackSide)
				return false;
			if (nPolySide == Intersect)
				std::swap(sInteriorPortion, sFront);
		}
		pResultEdgeList->push_back(sInteriorPortion);
		return true;
	}
	else
	{
		bool bFoundFront = false;

		SProjEdge sCurEdge = sEdge;
		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
		for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
		{
			SProjEdge sFront, sBack;
			PolySide nPolySide = sCurEdge.SplitPlane(*iCurPlane, &sFront, &sBack);
			if (nPolySide == FrontSide)
			{
				pResultEdgeList->push_back(sCurEdge);
				return true;
			}
			else if (nPolySide == Intersect)
			{
				pResultEdgeList->push_back(sFront);
				std::swap(sCurEdge, sBack);
				bFoundFront = true;
			}
		}
		return bFoundFront;
	}
}

bool ClipEdgeToPolyhedron(const SLFM_ShadowEdge &sEdge, const CConvexPolyhedron &cPolyhedron, TSLFM_ShadowEdgeList *pResultEdgeList)
{
	if (cPolyhedron.m_bInteriorSpace)
	{
		SLFM_ShadowEdge sInteriorPortion = sEdge;
		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
		for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
		{
			SLFM_ShadowEdge sFront, sBack;
			PolySide nPolySide = sInteriorPortion.SplitPlane(*iCurPlane, &sFront, &sBack);
			if (nPolySide == BackSide)
				return false;
			if (nPolySide == Intersect)
				std::swap(sInteriorPortion, sFront);
		}
		pResultEdgeList->push_back(sInteriorPortion);
		return true;
	}
	else
	{
		bool bFoundFront = false;

		SLFM_ShadowEdge sCurEdge = sEdge;
		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
		for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
		{
			SLFM_ShadowEdge sFront, sBack;
			PolySide nPolySide = sCurEdge.SplitPlane(*iCurPlane, &sFront, &sBack);
			if (nPolySide == FrontSide)
			{
				pResultEdgeList->push_back(sCurEdge);
				return true;
			}
			else if (nPolySide == Intersect)
			{
				pResultEdgeList->push_back(sFront);
				std::swap(sCurEdge, sBack);
				bFoundFront = true;
			}
		}
		return bFoundFront;
	}
}

void ClipEdgeListToPolyhedron(
	const TProjEdgeList &sEdgeList, 
	const CConvexPolyhedron &cPolyhedron, 
	bool bKeepNeighbors,
	TProjEdgeList *pResultEdgeList)
{
	pResultEdgeList->clear();
	TProjEdgeList::const_iterator iCurEdge = sEdgeList.begin();
	for (; iCurEdge != sEdgeList.end(); ++iCurEdge)
	{
		// Handle neighbors as a special case
		if (iCurEdge->IsNeighbor(cPolyhedron.m_pOutline))
		{
			if (bKeepNeighbors)
				pResultEdgeList->push_back(*iCurEdge);
			continue;
		}

		// Clip the current edge
		ClipEdgeToPolyhedron(*iCurEdge, cPolyhedron, pResultEdgeList);
	}
}

void ClipEdgeListToPolyhedron(
	const TSLFM_EdgeList &sEdgeList, 
	const CConvexPolyhedron &cPolyhedron, 
	bool bKeepNeighbors,
	TSLFM_EdgeList *pResultEdgeList)
{
	pResultEdgeList->clear();
	TSLFM_EdgeList::const_iterator iCurEdge = sEdgeList.begin();
	for (; iCurEdge != sEdgeList.end(); ++iCurEdge)
	{
		// Handle neighbors as a special case
		if (iCurEdge->IsNeighbor(cPolyhedron.m_pOutline))
		{
			if (bKeepNeighbors)
				pResultEdgeList->push_back(*iCurEdge);
			continue;
		}

		// Clip the current edge
		ClipEdgeToPolyhedron(*iCurEdge, cPolyhedron, pResultEdgeList);
	}
}

void ClipEdgeListToPolyhedron(
	const TSLFM_ShadowEdgeList &sEdgeList, 
	const CConvexPolyhedron &cPolyhedron, 
	TSLFM_ShadowEdgeList *pResultEdgeList)
{
	pResultEdgeList->clear();
	TSLFM_ShadowEdgeList::const_iterator iCurEdge = sEdgeList.begin();
	for (; iCurEdge != sEdgeList.end(); ++iCurEdge)
	{
		// Clip the current edge
		ClipEdgeToPolyhedron(*iCurEdge, cPolyhedron, pResultEdgeList);
	}
}

void MergeSimilarEdges(const TProjEdgeList &aProjEdges, TProjEdgeList *pResultEdges)
{
	// Note : Changing these values could effectively be a Q&D way of doing LOD on the shadows..
	const float k_fDuplicatePtEpsilon = 0.001f;
	const float k_fDuplicateDirEpsilon = 0.001f;

	// Operate on the result array...
	*pResultEdges = aProjEdges;

	if (pResultEdges->size() < 2)
		return;

	TProjEdgeList::iterator iCurEdge = pResultEdges->begin();
	for (; ((iCurEdge + 1) != pResultEdges->end()) && (iCurEdge != pResultEdges->end()); ++iCurEdge)
	{
		LTVector vEdge = iCurEdge->m_vEndProj - iCurEdge->m_vStartProj;
		float fEdgeMag = vEdge.Mag();
		// Remove 0-length edges from the list
		if (fEdgeMag < k_fDuplicatePtEpsilon)
		{
			if ((iCurEdge + 1) != pResultEdges->end())
				std::swap(*iCurEdge, pResultEdges->back());
			--iCurEdge;
			pResultEdges->pop_back();
			continue;
		}
		LTVector vEdgeDir = vEdge / fEdgeMag;
		float fEdgeStart = vEdgeDir.Dot(iCurEdge->m_vStartProj);

		TProjEdgeList::iterator iNeighborEdge = iCurEdge + 1;
		for (; iNeighborEdge != pResultEdges->end(); ++iNeighborEdge)
		{
			ASSERT((iNeighborEdge > pResultEdges->begin()) && (iNeighborEdge < pResultEdges->end()));
			
			LTVector vNeighborDir = iNeighborEdge->m_vEndProj - iNeighborEdge->m_vStartProj;
			vNeighborDir.Normalize();

			// Don't worry about it if it's not pointing the same way
			if (fabsf(vNeighborDir.Dot(vEdgeDir)) < (1.0f - k_fDuplicateDirEpsilon))
				continue;

			// Get the range of the neighbor on the new edge
			float fNeighborStart = vEdgeDir.Dot(iNeighborEdge->m_vStartProj) - fEdgeStart;
			float fNeighborEnd = vEdgeDir.Dot(iNeighborEdge->m_vEndProj) - fEdgeStart;
			bool bNeighborReversed = fNeighborStart > fNeighborEnd;
			if (bNeighborReversed)
				std::swap(fNeighborStart, fNeighborEnd);

			// Skip it if they don't overlap
			if ((fNeighborStart > (fEdgeMag + k_fDuplicatePtEpsilon)) || (fNeighborEnd < -k_fDuplicatePtEpsilon))
				continue;

			// Don't worry about it if it's on a parallel, but different line
			LTVector vNeighborOffset = (bNeighborReversed ? iNeighborEdge->m_vStartProj : iNeighborEdge->m_vEndProj) - iCurEdge->m_vStartProj;
			float fNeighborOffsetMag = vNeighborOffset.Mag();
			if (fNeighborOffsetMag > k_fDuplicatePtEpsilon)
			{
				LTVector vNeighborOffsetDir = vNeighborOffset / fNeighborOffsetMag;
				LTVector vNeighborOffsetCross = vNeighborOffsetDir.Cross(vEdgeDir);
				if (vNeighborOffsetCross.MagSqr() > (k_fDuplicateDirEpsilon * k_fDuplicateDirEpsilon))
					continue;
			}

			// Extend the current edge
			if (fNeighborStart < 0.0f)
			{
				iCurEdge->m_vStartPos = bNeighborReversed ? iNeighborEdge->m_vEndPos : iNeighborEdge->m_vStartPos;
				iCurEdge->m_vStartProj = bNeighborReversed ? iNeighborEdge->m_vEndProj : iNeighborEdge->m_vStartProj;
			}
			if (fNeighborEnd > fEdgeMag)
			{
				iCurEdge->m_vEndPos = bNeighborReversed ? iNeighborEdge->m_vStartPos : iNeighborEdge->m_vEndPos;
				iCurEdge->m_vEndProj = bNeighborReversed ? iNeighborEdge->m_vStartProj : iNeighborEdge->m_vEndProj;
			}

			// Take the neighbor out of the list
			if ((iNeighborEdge + 1) != pResultEdges->end())
				std::swap(*iNeighborEdge, pResultEdges->back());
			pResultEdges->pop_back();
			// Start over
			iNeighborEdge = iCurEdge;

			// Re-calculate the edge parameters
			vEdge = iCurEdge->m_vEndProj - iCurEdge->m_vStartProj;
			fEdgeMag = vEdge.Mag();
			fEdgeStart = vEdgeDir.Dot(iCurEdge->m_vStartProj);

			ASSERT(fEdgeMag > 0.0f);
		}
		if ((iCurEdge + 1) == pResultEdges->end())
			break;
	}
}

void GetEdgeStartEndLocks(const TSLFM_ShadowEdgeList::const_iterator &iCurEdge, const TSLFM_ShadowEdgeList &aEdges, bool *pStartLock, bool *pEndLock)
{
	const float k_fDuplicatePtEpsilon = 0.01f;
	const float k_fDuplicateDirEpsilon = 0.001f;

	*pStartLock = false;
	*pEndLock = false;
	
	LTVector vEdgeDir = iCurEdge->m_vEnd - iCurEdge->m_vStart;
	vEdgeDir.Normalize();

	TSLFM_ShadowEdgeList::const_iterator iSearchEdge = aEdges.begin();
	for (; (iSearchEdge != aEdges.end()) && (!pStartLock || !pEndLock); ++iSearchEdge)
	{
		LTVector vSearchDir = iSearchEdge->m_vEnd - iSearchEdge->m_vStart;
		vSearchDir.Normalize();
		if (fabsf(vSearchDir.Dot(vEdgeDir)) > (1.0f - k_fDuplicateDirEpsilon))
			continue;

		if (!*pStartLock)
		{
			if (iCurEdge->m_vStart.NearlyEquals(iSearchEdge->m_vStart, k_fDuplicatePtEpsilon) ||
				iCurEdge->m_vStart.NearlyEquals(iSearchEdge->m_vEnd, k_fDuplicatePtEpsilon))
				*pStartLock = true;
		}
		if (!*pEndLock)
		{
			if (iCurEdge->m_vEnd.NearlyEquals(iSearchEdge->m_vStart, k_fDuplicatePtEpsilon) ||
				iCurEdge->m_vEnd.NearlyEquals(iSearchEdge->m_vEnd, k_fDuplicatePtEpsilon))
				*pEndLock = true;
		}
	}
}

void MergeSimilarEdges(const TSLFM_ShadowEdgeList &aShadowEdges, TSLFM_ShadowEdgeList *pResultEdges)
{
	// Note : Changing these values could effectively be a Q&D way of doing LOD on the shadows..
	const float k_fDuplicatePtEpsilon = 0.001f;
	const float k_fDuplicateDirEpsilon = 0.00001f;

	// Operate on the result array...
	*pResultEdges = aShadowEdges;

	if (pResultEdges->size() < 2)
		return;

	// Remove 0-length edges from the list
	TSLFM_ShadowEdgeList::iterator iCurEdge = pResultEdges->begin();
	for (; iCurEdge != pResultEdges->end(); ++iCurEdge)
	{
		if (iCurEdge->m_vEnd.NearlyEquals(iCurEdge->m_vStart, k_fDuplicatePtEpsilon))
		{
			if ((iCurEdge + 1) != pResultEdges->end())
				std::swap(*iCurEdge, pResultEdges->back());
			--iCurEdge;
			pResultEdges->pop_back();
		}
	}

	if (pResultEdges->size() < 2)
		return;

	iCurEdge = pResultEdges->begin();
	for (; (iCurEdge + 1) != pResultEdges->end(); ++iCurEdge)
	{
		LTVector vEdge = iCurEdge->m_vEnd - iCurEdge->m_vStart;
		float fEdgeMag = vEdge.Mag();
		LTVector vEdgeDir = vEdge / fEdgeMag;
		float fEdgeStart = vEdgeDir.Dot(iCurEdge->m_vStart);

		bool bRecalcStartEndLocks = true;
		bool bStartLock = false;
		bool bEndLock = false;

		TSLFM_ShadowEdgeList::iterator iNeighborEdge = iCurEdge + 1;
		for (; iNeighborEdge != pResultEdges->end(); ++iNeighborEdge)
		{
			ASSERT((iNeighborEdge > pResultEdges->begin()) && (iNeighborEdge < pResultEdges->end()));
			
			LTVector vNeighborDir = iNeighborEdge->m_vEnd - iNeighborEdge->m_vStart;
			vNeighborDir.Normalize();

			// Don't worry about it if it's not pointing the same way
			if (fabsf(vNeighborDir.Dot(vEdgeDir)) < (1.0f - k_fDuplicateDirEpsilon))
				continue;

			// Get the range of the neighbor on the new edge
			float fNeighborStart = vEdgeDir.Dot(iNeighborEdge->m_vStart) - fEdgeStart;
			float fNeighborEnd = vEdgeDir.Dot(iNeighborEdge->m_vEnd) - fEdgeStart;
			bool bNeighborReversed = fNeighborStart > fNeighborEnd;
			if (bNeighborReversed)
				std::swap(fNeighborStart, fNeighborEnd);

			// Skip it if they don't overlap
			if ((fNeighborStart > (fEdgeMag + k_fDuplicatePtEpsilon)) || (fNeighborEnd < -k_fDuplicatePtEpsilon))
				continue;

			// Don't worry about it if it's on a parallel, but different line
			LTVector vNeighborOffset = (bNeighborReversed ? iNeighborEdge->m_vStart : iNeighborEdge->m_vEnd) - iCurEdge->m_vStart;
			float fNeighborOffsetMag = vNeighborOffset.Mag();
			if (fNeighborOffsetMag > k_fDuplicatePtEpsilon)
			{
				LTVector vNeighborOffsetDir = vNeighborOffset / fNeighborOffsetMag;
				LTVector vNeighborOffsetCross = vNeighborOffsetDir.Cross(vEdgeDir);
				if (vNeighborOffsetCross.MagSqr() > (k_fDuplicateDirEpsilon * k_fDuplicateDirEpsilon))
					continue;
			}

			// Make sure the lighting is what we would expect
			// NYI

			// Make sure we're not creating a T-Junction
			if (bRecalcStartEndLocks)
			{
				GetEdgeStartEndLocks(iCurEdge, *pResultEdges, &bStartLock, &bEndLock);
				bRecalcStartEndLocks = false;
			}

			// Extend the current edge
			if (!bStartLock && (fNeighborStart < 0.0f))
			{
				iCurEdge->m_vStart = bNeighborReversed ? iNeighborEdge->m_vEnd : iNeighborEdge->m_vStart;
				iCurEdge->m_fStartLight = bNeighborReversed ? iNeighborEdge->m_fEndLight : iNeighborEdge->m_fStartLight;
				bRecalcStartEndLocks = true;
			}
			if (!bEndLock && (fNeighborEnd > fEdgeMag))
			{
				iCurEdge->m_vEnd = bNeighborReversed ? iNeighborEdge->m_vStart : iNeighborEdge->m_vEnd;
				iCurEdge->m_fEndLight = bNeighborReversed ? iNeighborEdge->m_fStartLight : iNeighborEdge->m_fEndLight;
				bRecalcStartEndLocks = true;
			}

			// Take the neighbor out of the list
			if ((iNeighborEdge + 1) != pResultEdges->end())
				std::swap(*iNeighborEdge, pResultEdges->back());
			pResultEdges->pop_back();

			// Just take it out of the list if we didn't actually change this edge
			if (!bRecalcStartEndLocks)
			{
				if (iNeighborEdge == pResultEdges->end())
					break;
				else
					continue;
			}

			// Start over
			iNeighborEdge = iCurEdge;

			// Re-calculate the edge parameters
			vEdge = iCurEdge->m_vEnd - iCurEdge->m_vStart;
			fEdgeMag = vEdge.Mag();
			fEdgeStart = vEdgeDir.Dot(iCurEdge->m_vStart);

			ASSERT(fEdgeMag > 0.0f);
		}
		// Make sure we're not going to run over the end of our list due to removing an edge
		if ((iCurEdge + 1) == pResultEdges->end())
			break;
	}
}

void AppendProjEdgeListToShadowEdgeList(const TProjEdgeList &aProjEdges, const CLightDef *pLight, float fIntensity, TSLFM_ShadowEdgeList *pResultEdgeList)
{
	TProjEdgeList::const_iterator iCurEdge = aProjEdges.begin();
	for (; iCurEdge != aProjEdges.end(); ++iCurEdge)
	{
		SLFM_ShadowEdge sNewEdge;
		sNewEdge.m_vStart = iCurEdge->m_vStartProj;
		sNewEdge.m_vEnd = iCurEdge->m_vEndProj;
		sNewEdge.m_fStartLight = fIntensity;
		sNewEdge.m_fEndLight = fIntensity;
		sNewEdge.m_pLight = pLight;

		pResultEdgeList->push_back(sNewEdge);
	}
}

void CreatePenumbraEdges(const TProjEdgeList &aProjEdges, const LTPlane &cPlane, const CLightDef *pLight, TSLFM_ShadowEdgeList *pResultEdges)
{
	const float k_fDuplicatePtEpsilon = 0.001f;
	// Testing
	const float k_fMinPenumbraSize = 0.2f;
	//const float k_fMinPenumbraSize = 0.0f;
	// End Testing

	TProjEdgeList::const_iterator iCurEdge = aProjEdges.begin();
	for (; iCurEdge != aProjEdges.end(); ++iCurEdge)
	{
		SLFM_ShadowEdge sPenumbraEdge;
		sPenumbraEdge.m_fStartLight = 1.0f;
		sPenumbraEdge.m_fEndLight = 1.0f;
		sPenumbraEdge.m_pLight = pLight;

		LTVector vEdge = iCurEdge->m_vEndProj - iCurEdge->m_vStartProj;
		float fEdgeMag = vEdge.Mag();
		if (fEdgeMag < k_fDuplicatePtEpsilon)
			continue;
		LTVector vEdgeDir = vEdge / vEdge.Mag();
		LTVector vLightOnEdge = vEdgeDir * vEdgeDir.Dot(pLight->m_Pos - iCurEdge->m_vStartProj) + iCurEdge->m_vStartProj;
		LTVector vLightDir = vLightOnEdge - pLight->m_Pos;
		LTVector vPenumbraNormal = vEdge.Cross(vLightDir);
		vPenumbraNormal.Normalize();

		float fStartProjDist = iCurEdge->m_vStartProj.Dist(iCurEdge->m_vStartPos);
		float fEndProjDist = iCurEdge->m_vEndProj.Dist(iCurEdge->m_vEndPos);

		float fStartLightDist = iCurEdge->m_vStartPos.Dist(pLight->m_Pos);
		float fEndLightDist = iCurEdge->m_vEndPos.Dist(pLight->m_Pos);

		float fStartPenumbraSize = (pLight->m_fSize * fStartProjDist) / fStartLightDist;
		float fEndPenumbraSize = (pLight->m_fSize * fEndProjDist) / fEndLightDist;

		fStartPenumbraSize = LTMAX(fStartPenumbraSize, k_fMinPenumbraSize);
		fEndPenumbraSize = LTMAX(fEndPenumbraSize, k_fMinPenumbraSize);

		LTVector vStartPenumbraPos = iCurEdge->m_vStartProj + vPenumbraNormal * fStartPenumbraSize;
		LTVector vEndPenumbraPos = iCurEdge->m_vEndProj + vPenumbraNormal * fEndPenumbraSize;

		LTVector vStartLightDir = iCurEdge->m_vStartProj - pLight->m_Pos;
		vStartLightDir.Normalize();
		float fStartLightAngle = vStartLightDir.Dot(cPlane.m_Normal);
		LTVector vEndLightDir = iCurEdge->m_vEndProj - pLight->m_Pos;
		vEndLightDir.Normalize();
		float fEndLightAngle = vEndLightDir.Dot(cPlane.m_Normal);

		sPenumbraEdge.m_vStart = vStartPenumbraPos - vStartLightDir * (cPlane.DistTo(vStartPenumbraPos) / fStartLightAngle);
		sPenumbraEdge.m_vEnd = vEndPenumbraPos - vEndLightDir * (cPlane.DistTo(vEndPenumbraPos) / fEndLightAngle);

		ASSERT(vEdge.Dot(sPenumbraEdge.m_vEnd - sPenumbraEdge.m_vStart) > 0.0f);

		// Use to adjust 0-area penumbrae into a non-planar tesselation
		// Note : If you use this, get rid of the LTMAX calls above.  This will probably cause the tesselator to freak out.
		/*
		if (fStartPenumbraSize < k_fMinPenumbraSize)
			sPenumbraEdge.m_vStart += cPlane.m_Normal * k_fMinPenumbraSize;
		if (fEndPenumbraSize < k_fMinPenumbraSize)
			sPenumbraEdge.m_vEnd += cPlane.m_Normal * k_fMinPenumbraSize;
		*/

		LTVector vTestNormal = cPlane.m_Normal.Cross(vEdgeDir);
		vTestNormal.Normalize();
		LTPlane cTestPlane(vTestNormal, iCurEdge->m_vEndProj);
		ASSERT(cTestPlane.DistTo(sPenumbraEdge.m_vStart) > -0.001f);
		ASSERT(cTestPlane.DistTo(sPenumbraEdge.m_vEnd) > -0.001f);

		pResultEdges->push_back(sPenumbraEdge);
	}
}

class CShadowPoly
{
public:
	CShadowPoly() {}
	CShadowPoly(const CShadowPoly &sOther) : m_aVertices(sOther.m_aVertices) {}
	CShadowPoly &operator=(const CShadowPoly &sOther) {
		m_aVertices = sOther.m_aVertices;
		return *this;
	}

	void Swap(CShadowPoly &sOther) {
		m_aVertices.swap(sOther.m_aVertices);
	}

public:
	struct SVert
	{
		SVert() {}
		SVert(const LTVector &vPos, float fIntensity) :
			m_vPos(vPos), m_fIntensity(fIntensity) {}
		SVert(const SVert &sOther) :
			m_vPos(sOther.m_vPos), m_fIntensity(sOther.m_fIntensity) {}
		SVert &operator=(const SVert &sOther) {
			m_vPos = sOther.m_vPos;
			m_fIntensity = sOther.m_fIntensity;
			return *this;
		}
		LTVector m_vPos;
		float m_fIntensity;
	};

	typedef std::vector<SVert> TVertList;

public:
	TVertList::iterator NextVert(const TVertList::iterator &iVert) {
		TVertList::iterator iResult = iVert + 1;
		if (iResult == m_aVertices.end())
			return m_aVertices.begin();
		else
			return iResult;
	}
	TVertList::iterator PrevVert(const TVertList::iterator &iVert) {
		return (iVert == m_aVertices.begin()) ? (m_aVertices.end() - 1) : (iVert - 1);
	}
	TVertList::const_iterator NextVert(const TVertList::const_iterator &iVert) const {
		TVertList::const_iterator iResult = iVert + 1;
		if (iResult == m_aVertices.end())
			return m_aVertices.begin();
		else
			return iResult;
	}
	TVertList::const_iterator PrevVert(const TVertList::const_iterator &iVert) const {
		return (iVert == m_aVertices.begin()) ? (m_aVertices.end() - 1) : (iVert - 1);
	}
public:

	TVertList m_aVertices;
};

typedef std::vector<CShadowPoly> TShadowPolyList;

typedef std::vector<uint32> TIndexList;

// Note : This function appends to the result list
void CreatePenumbraPolys(
	const TSLFM_ShadowEdgeList &aPenumbraEdges, 
	const TProjEdgeList &aShadowEdges, 
	const LTPlane &cProjPlane, 
	const TIndexList &aNeighbors, 
	TShadowPolyList *pResultPolys)
{
	const float k_fDuplicatePtEpsilon = 0.001f;
	const float k_fPlanarPolyEpsilon = 0.001f;

	TSLFM_ShadowEdgeList::const_iterator iCurPenumbraEdge = aPenumbraEdges.begin();
	TProjEdgeList::const_iterator iCurShadowEdge = aShadowEdges.begin();
	for (; iCurPenumbraEdge != aPenumbraEdges.end(); ++iCurPenumbraEdge, ++iCurShadowEdge)
	{
		CShadowPoly cNewPoly;
		cNewPoly.m_aVertices.reserve(4);

		LTVector vInvProjStart = iCurShadowEdge->m_vStartProj + cProjPlane.m_Normal;
		LTVector vInvProjEnd = iCurShadowEdge->m_vEndProj + cProjPlane.m_Normal;

		// Calculate the plane for this polygon
		LTVector vProjectedEdge = vInvProjEnd - vInvProjStart;
		LTVector vCurNormal = vProjectedEdge.Cross(iCurPenumbraEdge->m_vStart - vInvProjStart);
		vCurNormal.Normalize();
		LTPlane cTestPlane(vCurNormal, vInvProjStart);
		float fEndDist = cTestPlane.DistTo(iCurPenumbraEdge->m_vEnd);

		if (fabs(fEndDist) >= k_fPlanarPolyEpsilon)
		{
			// Try the other plane
			vCurNormal = vProjectedEdge.Cross(vInvProjEnd - iCurPenumbraEdge->m_vEnd);
			LTPlane cOtherPlane(vCurNormal, vInvProjEnd);
			float fStartDist = cOtherPlane.DistTo(iCurPenumbraEdge->m_vStart);
			if (fabsf(fStartDist) < k_fPlanarPolyEpsilon)
			{
				cTestPlane = cOtherPlane;
				fEndDist = fStartDist;
			}
		}

		bool bUseQuad = fabs(fEndDist) < k_fPlanarPolyEpsilon;

		if (iCurShadowEdge->m_vStartProj.NearlyEquals(iCurShadowEdge->m_vEndProj, k_fDuplicatePtEpsilon) ||
			iCurPenumbraEdge->m_vStart.NearlyEquals(iCurPenumbraEdge->m_vEnd, k_fDuplicatePtEpsilon) ||
			iCurShadowEdge->m_vStartProj.NearlyEquals(iCurPenumbraEdge->m_vStart, k_fDuplicatePtEpsilon) ||
			iCurShadowEdge->m_vEndProj.NearlyEquals(iCurPenumbraEdge->m_vEnd, k_fDuplicatePtEpsilon))
		{
			bUseQuad = true;
		}

		// Can we just use a quad?
		if (bUseQuad)
		{
			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(vInvProjEnd, 0.0f));
			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(vInvProjStart, 0.0f));

			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(iCurPenumbraEdge->m_vStart, 1.0f));
			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(iCurPenumbraEdge->m_vEnd, 1.0f));

			pResultPolys->push_back(cNewPoly);
		}
		// Use 2 triangles
		else
		{
			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(vInvProjStart, 0.0f));
			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(vInvProjEnd, 0.0f));

			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(iCurPenumbraEdge->m_vStart, 1.0f));

			pResultPolys->push_back(cNewPoly);
			
			cNewPoly.m_aVertices.clear();

			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(vInvProjEnd, 0.0f));

			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(iCurPenumbraEdge->m_vEnd, 1.0f));
			cNewPoly.m_aVertices.push_back(CShadowPoly::SVert(iCurPenumbraEdge->m_vStart, 1.0f));

			pResultPolys->push_back(cNewPoly);
		}
	}
}

// Note : This function appends to the result list
void CreatePenumbraVolumes(
	const TSLFM_ShadowEdgeList &aPenumbraEdges, 
	const TProjEdgeList &aShadowEdges, 
	const LTPlane &cProjPlane, 
	TConvexPolyhedronList *pResultPolyhedrons)
{
	const float k_fDuplicatePtEpsilon = 0.001f;
	const float k_fPlanarPolyEpsilon = 0.001f;

	TSLFM_ShadowEdgeList::const_iterator iCurPenumbraEdge = aPenumbraEdges.begin();
	TProjEdgeList::const_iterator iCurShadowEdge = aShadowEdges.begin();
	for (; iCurPenumbraEdge != aPenumbraEdges.end(); ++iCurPenumbraEdge, ++iCurShadowEdge)
	{
		CConvexPolyhedron cCurPolyhedron;
		cCurPolyhedron.m_pOutline = 0;
		cCurPolyhedron.m_bInteriorSpace = false;
		cCurPolyhedron.m_aPlanes.reserve(4);

		LTVector vCurNormal;

		// Deal with 0-length edges in all directions
		// NYI

		// Temporary hack...  If any of the vertices are shared, create an empty volume and move on
		if (iCurShadowEdge->m_vStartProj.NearlyEquals(iCurShadowEdge->m_vEndProj, k_fDuplicatePtEpsilon) ||
			iCurPenumbraEdge->m_vStart.NearlyEquals(iCurPenumbraEdge->m_vEnd, k_fDuplicatePtEpsilon) ||
			iCurShadowEdge->m_vStartProj.NearlyEquals(iCurPenumbraEdge->m_vStart, k_fDuplicatePtEpsilon) ||
			iCurShadowEdge->m_vEndProj.NearlyEquals(iCurPenumbraEdge->m_vEnd, k_fDuplicatePtEpsilon))
		{
			pResultPolyhedrons->push_back(cCurPolyhedron);
			continue;
		}


		LTVector vInvProjStart = iCurShadowEdge->m_vStartProj + cProjPlane.m_Normal;
		LTVector vInvProjEnd = iCurShadowEdge->m_vEndProj + cProjPlane.m_Normal;

		// Calculate the main plane(s) for this polygon
		LTVector vProjectedEdge = vInvProjEnd - vInvProjStart;
		LTVector vPenumbraEdge = iCurPenumbraEdge->m_vEnd - iCurPenumbraEdge->m_vStart;
		vCurNormal = vProjectedEdge.Cross(iCurPenumbraEdge->m_vStart - vInvProjStart);
		vCurNormal.Normalize();
		LTPlane cBasePlane(vCurNormal, iCurPenumbraEdge->m_vStart);
		float fEndDist = cBasePlane.DistTo(iCurPenumbraEdge->m_vEnd);

		bool bUseQuad = fabs(fEndDist) < k_fPlanarPolyEpsilon;

		if (fabs(fEndDist) >= k_fPlanarPolyEpsilon)
		{
			// Try the other plane
			vCurNormal = vProjectedEdge.Cross(vInvProjEnd - iCurPenumbraEdge->m_vEnd);
			vCurNormal.Normalize();
			LTPlane cOtherPlane(vCurNormal, vInvProjEnd);
			float fStartDist = cOtherPlane.DistTo(iCurPenumbraEdge->m_vStart);
			if (fabsf(fStartDist) < k_fPlanarPolyEpsilon)
			{
				cBasePlane = cOtherPlane;
				fEndDist = fStartDist;
				bUseQuad = true;
			}
		}

		if (cBasePlane.m_Normal.Dot(cProjPlane.m_Normal) < 0.0f)
			cBasePlane = -cBasePlane;

		// Did we use a quad?
		if (bUseQuad)
		{
			cCurPolyhedron.m_aPlanes.push_back(cBasePlane);

			// Plane on the shadow side
			vCurNormal = vProjectedEdge.Cross(cProjPlane.m_Normal);
			vCurNormal.Normalize();
			cCurPolyhedron.m_aPlanes.push_back(LTPlane(vCurNormal, iCurShadowEdge->m_vStartProj));

			// End cap on the start side
			vCurNormal = cProjPlane.m_Normal.Cross(iCurPenumbraEdge->m_vStart - iCurShadowEdge->m_vStartProj);
			vCurNormal.Normalize();
			cCurPolyhedron.m_aPlanes.push_back(LTPlane(vCurNormal, iCurShadowEdge->m_vStartProj));

			// End cap on the end side
			vCurNormal = cProjPlane.m_Normal.Cross(iCurShadowEdge->m_vEndProj - iCurPenumbraEdge->m_vEnd);
			vCurNormal.Normalize();
			cCurPolyhedron.m_aPlanes.push_back(LTPlane(vCurNormal, iCurShadowEdge->m_vEndProj));

			pResultPolyhedrons->push_back(cCurPolyhedron);

			ASSERT(fabsf(cBasePlane.DistTo(vInvProjStart)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cBasePlane.DistTo(vInvProjEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cBasePlane.DistTo(iCurPenumbraEdge->m_vStart)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cBasePlane.DistTo(iCurPenumbraEdge->m_vEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(cBasePlane.DistTo(iCurShadowEdge->m_vEndProj) < k_fPlanarPolyEpsilon);
			ASSERT(cBasePlane.DistTo(iCurShadowEdge->m_vStartProj) < k_fPlanarPolyEpsilon);
		}
		// Create two volumes instead of just one
		else
		{
			cCurPolyhedron.m_aPlanes.push_back(cBasePlane);

			ASSERT(fabsf(cBasePlane.DistTo(vInvProjStart)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cBasePlane.DistTo(vInvProjEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cBasePlane.DistTo(iCurPenumbraEdge->m_vStart)) < k_fPlanarPolyEpsilon);
			ASSERT(cBasePlane.DistTo(iCurShadowEdge->m_vEndProj) < k_fPlanarPolyEpsilon);
			ASSERT(cBasePlane.DistTo(iCurShadowEdge->m_vStartProj) < k_fPlanarPolyEpsilon);

			// Plane on the shadow side
			vCurNormal = vProjectedEdge.Cross(cProjPlane.m_Normal);
			vCurNormal.Normalize();
			LTPlane cShadowPlane(vCurNormal, iCurShadowEdge->m_vStartProj);
			cCurPolyhedron.m_aPlanes.push_back(cShadowPlane);

			ASSERT(fabsf(cShadowPlane.DistTo(vInvProjStart)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cShadowPlane.DistTo(vInvProjEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cShadowPlane.DistTo(iCurShadowEdge->m_vStartProj)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cShadowPlane.DistTo(iCurShadowEdge->m_vEndProj)) < k_fPlanarPolyEpsilon);
			ASSERT(cShadowPlane.DistTo(iCurPenumbraEdge->m_vStart) < k_fPlanarPolyEpsilon);
			ASSERT(cShadowPlane.DistTo(iCurPenumbraEdge->m_vEnd) < k_fPlanarPolyEpsilon);

			// End cap on the start side
			vCurNormal = cProjPlane.m_Normal.Cross(iCurPenumbraEdge->m_vStart - iCurShadowEdge->m_vStartProj);
			vCurNormal.Normalize();
			LTPlane cStartEndCap(vCurNormal, iCurPenumbraEdge->m_vStart);
			if (cStartEndCap.DistTo(iCurShadowEdge->m_vEndProj) > k_fPlanarPolyEpsilon)
				cStartEndCap = -cStartEndCap;
			cCurPolyhedron.m_aPlanes.push_back(cStartEndCap);

			ASSERT(fabsf(cStartEndCap.DistTo(vInvProjStart)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cStartEndCap.DistTo(iCurPenumbraEdge->m_vStart)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cStartEndCap.DistTo(iCurShadowEdge->m_vStartProj)) < k_fPlanarPolyEpsilon);
			ASSERT(cStartEndCap.DistTo(vInvProjEnd) < k_fPlanarPolyEpsilon);
			ASSERT(cStartEndCap.DistTo(iCurPenumbraEdge->m_vEnd) < k_fPlanarPolyEpsilon);
			ASSERT(cStartEndCap.DistTo(iCurShadowEdge->m_vEndProj) < k_fPlanarPolyEpsilon);

			// Diagonal end cap
			vCurNormal = cProjPlane.m_Normal.Cross(iCurShadowEdge->m_vEndProj - iCurPenumbraEdge->m_vStart);
			vCurNormal.Normalize();
			LTPlane cDiagonalPlane(vCurNormal, iCurPenumbraEdge->m_vStart);
			cCurPolyhedron.m_aPlanes.push_back(cDiagonalPlane);

			ASSERT(fabsf(cDiagonalPlane.DistTo(iCurShadowEdge->m_vEndProj)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cDiagonalPlane.DistTo(vInvProjEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cDiagonalPlane.DistTo(iCurPenumbraEdge->m_vStart)) < k_fPlanarPolyEpsilon);
			ASSERT(cDiagonalPlane.DistTo(iCurShadowEdge->m_vStartProj) < k_fPlanarPolyEpsilon);
			ASSERT(cDiagonalPlane.DistTo(vInvProjStart) < k_fPlanarPolyEpsilon);
			ASSERT(cDiagonalPlane.DistTo(iCurPenumbraEdge->m_vEnd) > -k_fPlanarPolyEpsilon);

			pResultPolyhedrons->push_back(cCurPolyhedron);

			cCurPolyhedron.m_aPlanes.clear();

			// Second triangle plane
			vCurNormal = vPenumbraEdge.Cross(iCurPenumbraEdge->m_vEnd - vInvProjEnd);
			vCurNormal.Normalize();
			LTPlane cOtherBasePlane(vCurNormal, iCurPenumbraEdge->m_vEnd);
			if (cOtherBasePlane.m_Normal.Dot(cProjPlane.m_Normal) < 0.0f)
				cOtherBasePlane = -cOtherBasePlane;
			cCurPolyhedron.m_aPlanes.push_back(cOtherBasePlane);

			ASSERT(fabsf(cOtherBasePlane.DistTo(vInvProjEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cOtherBasePlane.DistTo(iCurPenumbraEdge->m_vStart)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cOtherBasePlane.DistTo(iCurPenumbraEdge->m_vEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(cOtherBasePlane.DistTo(iCurShadowEdge->m_vStartProj) < k_fPlanarPolyEpsilon);
			ASSERT(cOtherBasePlane.DistTo(iCurShadowEdge->m_vEndProj) < k_fPlanarPolyEpsilon);

			// End cap on the end side
			vCurNormal = cProjPlane.m_Normal.Cross(iCurShadowEdge->m_vEndProj - iCurPenumbraEdge->m_vEnd);
			vCurNormal.Normalize();
			LTPlane cEndEndCap(vCurNormal, iCurPenumbraEdge->m_vEnd);
			if (cEndEndCap.DistTo(iCurShadowEdge->m_vStartProj) > k_fPlanarPolyEpsilon)
				cEndEndCap = -cEndEndCap;
			cCurPolyhedron.m_aPlanes.push_back(cEndEndCap);

			ASSERT(fabsf(cEndEndCap.DistTo(vInvProjEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cEndEndCap.DistTo(iCurPenumbraEdge->m_vEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cEndEndCap.DistTo(iCurShadowEdge->m_vEndProj)) < k_fPlanarPolyEpsilon);
			ASSERT(cEndEndCap.DistTo(vInvProjStart) < k_fPlanarPolyEpsilon);
			ASSERT(cEndEndCap.DistTo(iCurPenumbraEdge->m_vStart) < k_fPlanarPolyEpsilon);
			ASSERT(cEndEndCap.DistTo(iCurShadowEdge->m_vStartProj) < k_fPlanarPolyEpsilon);

			// Diagonal end cap
			cDiagonalPlane = -cDiagonalPlane;
			cCurPolyhedron.m_aPlanes.push_back(cDiagonalPlane);

			ASSERT(cDiagonalPlane.DistTo(iCurShadowEdge->m_vEndProj) < k_fPlanarPolyEpsilon);
			ASSERT(cDiagonalPlane.DistTo(vInvProjEnd) < k_fPlanarPolyEpsilon);

			// And a cap on the penumbra side for good measure
			vCurNormal = cProjPlane.m_Normal.Cross(vPenumbraEdge);
			vCurNormal.Normalize();
			LTPlane cPenumbraPlane(vCurNormal, iCurPenumbraEdge->m_vEnd);
			cCurPolyhedron.m_aPlanes.push_back(cPenumbraPlane);

			ASSERT(fabsf(cPenumbraPlane.DistTo(iCurPenumbraEdge->m_vStart)) < k_fPlanarPolyEpsilon);
			ASSERT(fabsf(cPenumbraPlane.DistTo(iCurPenumbraEdge->m_vEnd)) < k_fPlanarPolyEpsilon);
			ASSERT(cPenumbraPlane.DistTo(vInvProjStart) < k_fPlanarPolyEpsilon);
			ASSERT(cPenumbraPlane.DistTo(vInvProjEnd) < k_fPlanarPolyEpsilon);
			ASSERT(cPenumbraPlane.DistTo(iCurShadowEdge->m_vStartProj) < k_fPlanarPolyEpsilon);
			ASSERT(cPenumbraPlane.DistTo(iCurShadowEdge->m_vEndProj) < k_fPlanarPolyEpsilon);


			pResultPolyhedrons->push_back(cCurPolyhedron);
		}
	}
}

void CreateProjEdgeNeighborList(const TProjEdgeList &aEdges, TIndexList *pResultNeighbors)
{
	const float k_fDuplicatePtEpsilon = 0.01f;

	pResultNeighbors->resize(aEdges.size(), INVALID_INDEX);

	TProjEdgeList::const_iterator iCurEdge = aEdges.begin();
	TIndexList::iterator iCurNeighborIndex = pResultNeighbors->begin();
	for (; iCurEdge != aEdges.end(); ++iCurEdge, ++iCurNeighborIndex)
	{
		TProjEdgeList::const_iterator iNeighborEdge = aEdges.begin();
		for (; iNeighborEdge != aEdges.end(); ++iNeighborEdge)
		{
			if (iNeighborEdge == iCurEdge)
				continue;

			if (iNeighborEdge->m_vStartProj.NearlyEquals(iCurEdge->m_vEndProj, k_fDuplicatePtEpsilon) ||
				iNeighborEdge->m_vEndProj.NearlyEquals(iCurEdge->m_vEndProj, k_fDuplicatePtEpsilon))
			{
				*iCurNeighborIndex = iNeighborEdge - aEdges.begin();
				break;
			}
		}
	}
}

bool IsOutsideCorner(const SLFM_ShadowEdge &sEdge1, const SLFM_ShadowEdge &sEdge2, const LTVector &vNormal)
{
	const float k_fCornerDotEpsilon = 0.001f;

	LTVector vTriNormal = (sEdge1.m_vEnd - sEdge1.m_vStart).Cross(sEdge2.m_vEnd - sEdge2.m_vStart);
	return vNormal.Dot(vTriNormal) < k_fCornerDotEpsilon;
}

bool IsOutsideCorner(const SProjEdge &sEdge1, const SProjEdge &sEdge2, const LTVector &vNormal)
{
	const float k_fCornerDotEpsilon = 0.001f;

	LTVector vTriNormal = (sEdge1.m_vEndProj - sEdge1.m_vStartProj).Cross(sEdge2.m_vEndProj - sEdge2.m_vStartProj);
	return vNormal.Dot(vTriNormal) < k_fCornerDotEpsilon;
}

// Note : This function appends to the result list
void CreatePenumbraCornerPolys(
	const TSLFM_ShadowEdgeList &aPenumbraEdges, 
	const TProjEdgeList &aShadowEdges, 
	const TIndexList &aNeighbors, 
	const LTPlane &cProjPlane, 
	TShadowPolyList *pResultPolys)
{
	const float k_fCollapsedCornerEpsilon = 0.001f;

	TSLFM_ShadowEdgeList::const_iterator iCurPenumbraEdge = aPenumbraEdges.begin();
	TProjEdgeList::const_iterator iCurShadowEdge = aShadowEdges.begin();
	TIndexList::const_iterator iCurNeighborIndex = aNeighbors.begin();
	for (; iCurPenumbraEdge != aPenumbraEdges.end(); ++iCurPenumbraEdge, ++iCurShadowEdge, ++iCurNeighborIndex)
	{
		if (*iCurNeighborIndex == INVALID_INDEX)
			continue;

		const SProjEdge &sCurNeighborShadow = aShadowEdges[*iCurNeighborIndex];

		if (!IsOutsideCorner(*iCurShadowEdge, sCurNeighborShadow, cProjPlane.m_Normal))
			continue;

		const SLFM_ShadowEdge &sCurNeighborPenumbra = aPenumbraEdges[*iCurNeighborIndex];

		CShadowPoly cCurPoly;

		cCurPoly.m_aVertices.reserve(3);
		cCurPoly.m_aVertices.push_back(CShadowPoly::SVert(iCurShadowEdge->m_vEndProj + cProjPlane.m_Normal, 0.0f));
		cCurPoly.m_aVertices.push_back(CShadowPoly::SVert(iCurPenumbraEdge->m_vEnd, 1.0f));
		cCurPoly.m_aVertices.push_back(CShadowPoly::SVert(sCurNeighborPenumbra.m_vStart, 1.0f));
		pResultPolys->push_back(cCurPoly);
	}
}

// Note : This function appends to the result list
void CreatePenumbraCornerVolumes(
	const TSLFM_ShadowEdgeList &aPenumbraEdges, 
	const TProjEdgeList &aShadowEdges, 
	const TIndexList &aNeighbors, 
	const LTPlane &cProjPlane, 
	TConvexPolyhedronList *pResultPolyhedrons)
{
	const float k_fDuplicatePtEpsilon = 0.001f;
	const float k_fCollapsedCornerEpsilon = 0.001f;

	TSLFM_ShadowEdgeList::const_iterator iCurPenumbraEdge = aPenumbraEdges.begin();
	TProjEdgeList::const_iterator iCurShadowEdge = aShadowEdges.begin();
	TIndexList::const_iterator iCurNeighborIndex = aNeighbors.begin();
	for (; iCurPenumbraEdge != aPenumbraEdges.end(); ++iCurPenumbraEdge, ++iCurShadowEdge, ++iCurNeighborIndex)
	{
		if (*iCurNeighborIndex == INVALID_INDEX)
			continue;

		const SProjEdge &sCurNeighborShadow = aShadowEdges[*iCurNeighborIndex];

		if (!IsOutsideCorner(*iCurShadowEdge, sCurNeighborShadow, cProjPlane.m_Normal))
			continue;

		const SLFM_ShadowEdge &sCurNeighborPenumbra = aPenumbraEdges[*iCurNeighborIndex];

		CConvexPolyhedron cCurPolyhedron;

		cCurPolyhedron.m_pOutline = 0;
		cCurPolyhedron.m_bInteriorSpace = false;
		cCurPolyhedron.m_aPlanes.reserve(3);

		// Temporary hack...  If any of the vertices are shared, create an empty volume and move on
		if (iCurShadowEdge->m_vEndProj.NearlyEquals(iCurPenumbraEdge->m_vEnd, k_fDuplicatePtEpsilon) ||
			sCurNeighborShadow.m_vStartProj.NearlyEquals(sCurNeighborPenumbra.m_vStart, k_fDuplicatePtEpsilon) ||
			iCurPenumbraEdge->m_vEnd.NearlyEquals(sCurNeighborPenumbra.m_vStart, k_fDuplicatePtEpsilon))
		{
			pResultPolyhedrons->push_back(cCurPolyhedron);
			continue;
		}

		LTVector vInvProjEnd = iCurShadowEdge->m_vEndProj + cProjPlane.m_Normal;

		LTVector vCurNormal;
		vCurNormal = (sCurNeighborPenumbra.m_vStart - vInvProjEnd).Cross(iCurPenumbraEdge->m_vEnd - vInvProjEnd);
		vCurNormal.Normalize();
		cCurPolyhedron.m_aPlanes.push_back(LTPlane(vCurNormal, vInvProjEnd));

		vCurNormal = cProjPlane.m_Normal.Cross(iCurPenumbraEdge->m_vEnd - vInvProjEnd);
		vCurNormal.Normalize();
		cCurPolyhedron.m_aPlanes.push_back(LTPlane(vCurNormal, vInvProjEnd));

		vCurNormal = cProjPlane.m_Normal.Cross(vInvProjEnd - sCurNeighborPenumbra.m_vStart);
		vCurNormal.Normalize();
		cCurPolyhedron.m_aPlanes.push_back(LTPlane(vCurNormal, vInvProjEnd));

		pResultPolyhedrons->push_back(cCurPolyhedron);
	}
}

PolySide SplitShadowPoly(const CShadowPoly &sPoly, const LTPlane &cPlane, CShadowPoly *pFrontPoly, CShadowPoly *pBackPoly, LTVector *pIntersectStart, LTVector *pIntersectEnd)
{
	uint32 nFront = 0;
	uint32 nBack = 0;

	pFrontPoly->m_aVertices.clear();
	pBackPoly->m_aVertices.clear();

	CShadowPoly::SVert vPrevPt = sPoly.m_aVertices.back();
	float fPrevDist;
	PolySide nPrevSide = GetPointSide(cPlane, vPrevPt.m_vPos, &fPrevDist);
	CShadowPoly::TVertList::const_iterator iCurVert = sPoly.m_aVertices.begin();
	for (; iCurVert != sPoly.m_aVertices.end(); ++iCurVert)
	{
		float fNextDist;
		PolySide nNextSide = GetPointSide(cPlane, iCurVert->m_vPos, &fNextDist);
		if ((nNextSide != nPrevSide) && (nNextSide != Intersect) && (nPrevSide != Intersect))
		{
			// Calculate an intersect
			float fInterpolant = fNextDist / (fNextDist - fPrevDist);
			LTVector vIntersectPt;
			VEC_LERP(vIntersectPt, iCurVert->m_vPos, vPrevPt.m_vPos, fInterpolant);
			float fIntersectIntensity = LTLERP(iCurVert->m_fIntensity, vPrevPt.m_fIntensity, fInterpolant);
			// Add it to both sides
			pFrontPoly->m_aVertices.push_back(CShadowPoly::SVert(vIntersectPt, fIntersectIntensity));
			pBackPoly->m_aVertices.push_back(CShadowPoly::SVert(vIntersectPt, fIntersectIntensity));

			*pIntersectStart = vIntersectPt;
			std::swap(pIntersectStart, pIntersectEnd);
		}
		switch (nNextSide)
		{
			case Intersect :
				// Add it to both sides
				pFrontPoly->m_aVertices.push_back(*iCurVert);
				pBackPoly->m_aVertices.push_back(*iCurVert);

				*pIntersectStart = iCurVert->m_vPos;
				std::swap(pIntersectStart, pIntersectEnd);
				break;
			case FrontSide :
				// Add it to the front
				++nFront;
				pFrontPoly->m_aVertices.push_back(*iCurVert);
				break;
			case BackSide :
				// Add it to the back
				++nBack;
				pBackPoly->m_aVertices.push_back(*iCurVert);
				break;
		}
		vPrevPt = *iCurVert;
		nPrevSide = nNextSide;
		fPrevDist = fNextDist;
	}

	if (nFront == 0)
	{
		if (nBack == 0)
		{
			LTVector vPolyNormal = (sPoly.m_aVertices[2].m_vPos - sPoly.m_aVertices[0].m_vPos).Cross(sPoly.m_aVertices[1].m_vPos - sPoly.m_aVertices[0].m_vPos);
			return cPlane.m_Normal.Dot(vPolyNormal) > 0 ? FrontSide : BackSide;
		}
		else
			return BackSide;
	}
	else if (nBack == 0)
		return FrontSide;
	else
		return Intersect;
}

bool MergeShadowPolyList(const TShadowPolyList &sPolyList, TShadowPolyList *pResultList)
{
	const float k_fDuplicatePtEpsilon = 0.01f;
	const float k_fPtOnLineEpsilon = 0.1f;

	*pResultList = sPolyList;

	bool bResult = false;

	if (sPolyList.size() < 2)
		return false;

	TShadowPolyList::iterator iCurPoly = pResultList->begin();
	for (; iCurPoly != pResultList->end(); ++iCurPoly)
	{
		bool bFoundMerge = false;

		TShadowPolyList::iterator iMergePoly = iCurPoly + 1;
		for (; (iMergePoly != pResultList->end()) && (!bFoundMerge); ++iMergePoly)
		{
			// Compare the vertices of the current polygon with the vertices of the merge poly
			CShadowPoly::TVertList::iterator iCurVert = iCurPoly->m_aVertices.begin();
			for (; (iCurVert != iCurPoly->m_aVertices.end()) && (!bFoundMerge); ++iCurVert)
			{
				CShadowPoly::TVertList::iterator iCurNext = iCurPoly->NextVert(iCurVert);

				CShadowPoly::TVertList::iterator iMergeVert = iMergePoly->m_aVertices.begin();
				for (; (iMergeVert != iMergePoly->m_aVertices.end()) && (!bFoundMerge); ++iMergeVert)
				{
					// Find out if we've got a shared edge
					if (!iCurVert->m_vPos.NearlyEquals(iMergeVert->m_vPos, k_fDuplicatePtEpsilon))
						continue;

					CShadowPoly::TVertList::iterator iMergePrev = iMergePoly->PrevVert(iMergeVert);

					CShadowPoly::TVertList::iterator iCurPrev = iCurPoly->PrevVert(iCurVert);
					CShadowPoly::TVertList::iterator iCurNextNext = iCurPoly->NextVert(iCurNext);
					CShadowPoly::TVertList::iterator iMergeNext = iMergePoly->NextVert(iMergeVert);
					CShadowPoly::TVertList::iterator iMergePrevPrev = iMergePoly->PrevVert(iMergePrev);
					
					if (!iCurNext->m_vPos.NearlyEquals(iMergePrev->m_vPos, k_fDuplicatePtEpsilon))
						continue;

					// Check to make sure this merge won't create a concave polygon
					// NYI

					// Merge the polys
					CShadowPoly::TVertList aNewVerts;
					CShadowPoly::TVertList::iterator iCurMerge;
					TShadowPolyList::iterator iCurMergePoly = iMergePoly;
					float fT;
					if (PtLineSegDist(iCurVert->m_vPos, iCurPrev->m_vPos, iMergeNext->m_vPos - iCurPrev->m_vPos, &fT) < k_fPtOnLineEpsilon)
						iCurMerge = iMergeNext;
					else
						iCurMerge = iMergeVert;
					while (iCurMerge != iCurVert)
					{
						aNewVerts.push_back(*iCurMerge);
						CShadowPoly::TVertList::iterator iCurMergeNext = iCurMergePoly->NextVert(iCurMerge);
						if (iCurMergeNext == iMergePrev)
						{
							iCurMergePoly = iCurPoly;
							if (PtLineSegDist(iCurNext->m_vPos, iCurMerge->m_vPos, iCurNextNext->m_vPos - iCurNext->m_vPos, &fT) < k_fPtOnLineEpsilon)
								iCurMerge = iCurNextNext;
							else
								iCurMerge = iCurNext;
						}
						else
							iCurMerge = iCurMergeNext;
					}
					// Use the new vertex list in the current poly
					iCurPoly->m_aVertices.swap(aNewVerts);

					// Remove the merge poly from the list
					if (iMergePoly != (pResultList->end() - 1))
						iMergePoly->Swap(pResultList->back());
					pResultList->pop_back();

					// Jump out of the loops
					bFoundMerge = true;
				}
			}
		}

		if (bFoundMerge)
		{
			bResult = true;
			--iCurPoly;
		}
	}

	return bResult;
}

bool ClipShadowPolyListToPolyhedron(const TShadowPolyList &sPolyList, const CConvexPolyhedron &cPolyhedron, TShadowPolyList *pResultList)
{
	pResultList->clear();

	if (cPolyhedron.m_bInteriorSpace)
	{
		TShadowPolyList::const_iterator iCurPoly = sPolyList.begin();
		for (; iCurPoly != sPolyList.end(); ++iCurPoly)
		{
			bool bFoundInteriorPortion = true;
			CShadowPoly sInteriorPortion = *iCurPoly;
			CShadowPoly sFront = sInteriorPortion, sBack = sInteriorPortion;
			CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
			for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
			{
				LTVector vIntersectStart, vIntersectEnd;
				PolySide nPolySide = SplitShadowPoly(sInteriorPortion, *iCurPlane, &sFront, &sBack, &vIntersectStart, &vIntersectEnd);
				if (nPolySide == BackSide)
				{
					bFoundInteriorPortion = false;
					break;
				}
				if (nPolySide == Intersect)
					sInteriorPortion.Swap(sFront);
			}
			if (bFoundInteriorPortion)
				pResultList->push_back(sInteriorPortion);
		}
		return true;
	}
	else
	{
		bool bFoundFront = true;

		TShadowPolyList::const_iterator iCurPoly = sPolyList.begin();
		for (; iCurPoly != sPolyList.end(); ++iCurPoly)
		{
			bool bFoundLocalFront = cPolyhedron.m_aPlanes.empty();
			CShadowPoly sInteriorPortion = *iCurPoly;
			CShadowPoly sFront = sInteriorPortion, sBack = sInteriorPortion;
			CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyhedron.m_aPlanes.begin();
			for (; iCurPlane != cPolyhedron.m_aPlanes.end(); ++iCurPlane)
			{
				LTVector vIntersectStart, vIntersectEnd;
				PolySide nPolySide = SplitShadowPoly(sInteriorPortion, *iCurPlane, &sFront, &sBack, &vIntersectStart, &vIntersectEnd);
				if (nPolySide != BackSide)
					pResultList->push_back(sFront);
				if (nPolySide == FrontSide)
				{
					bFoundLocalFront = true;
					break;
				}
				sInteriorPortion.Swap(sBack);
			}
			bFoundFront &= bFoundLocalFront;
		}

		if (bFoundFront)
			return false;

		return true;
	}
}

void ClipShadowPolysToConvexPolyhedronList(const TShadowPolyList &aPolys, const TConvexPolyhedronList &aConvexPolyhedrons, TShadowPolyList *pResultPolys)
{
	pResultPolys->clear();

	// The polyhedron list is supposed to have one entry for each polygon, in order
	// The position in that list is used to avoid clipping a poly into the polyhedron it's associated with
	ASSERT(aPolys.size() <= aConvexPolyhedrons.size());

	TShadowPolyList::const_iterator iCurPoly = aPolys.begin();
	uint32 nPolyIndex = 0;
	for (; iCurPoly != aPolys.end(); ++iCurPoly, ++nPolyIndex)
	{
		TShadowPolyList aInsideFragments;
		aInsideFragments.push_back(*iCurPoly);

		TConvexPolyhedronList::const_iterator iCurPolyhedron = aConvexPolyhedrons.begin();
		uint32 nPolyhedronIndex = 0;
		for (; (iCurPolyhedron != aConvexPolyhedrons.end()) && (!aInsideFragments.empty()); ++iCurPolyhedron, ++nPolyhedronIndex)
		{
			if (nPolyIndex == nPolyhedronIndex)
				continue;

			TShadowPolyList aNewFragments;
			if (ClipShadowPolyListToPolyhedron(aInsideFragments, *iCurPolyhedron, &aNewFragments))
				aInsideFragments.swap(aNewFragments);
		}

		if (!aInsideFragments.empty())
		{
			TShadowPolyList aCleanPolyList;
			if (MergeShadowPolyList(aInsideFragments, &aCleanPolyList))
				aInsideFragments.swap(aCleanPolyList);

			pResultPolys->insert(pResultPolys->end(), aInsideFragments.begin(), aInsideFragments.end());
		}
	}
}

void GetShadowPolyEdges(const TShadowPolyList &aPolys, const LTPlane &cPlane, const CLightDef *pLight, TSLFM_ShadowEdgeList *pResultEdges)
{
	pResultEdges->clear();

	TShadowPolyList::const_iterator iCurPoly = aPolys.begin();
	for (; iCurPoly != aPolys.end(); ++iCurPoly)
	{
		CShadowPoly::TVertList::const_iterator iPrevPt = iCurPoly->m_aVertices.end() - 1;
		CShadowPoly::TVertList::const_iterator iCurVert = iCurPoly->m_aVertices.begin();
		for (; iCurVert != iCurPoly->m_aVertices.end(); ++iCurVert)
		{
			SLFM_ShadowEdge sCurEdge;

			sCurEdge.m_pLight = pLight;
			sCurEdge.m_vStart = iPrevPt->m_vPos;
			sCurEdge.m_vEnd = iCurVert->m_vPos;
			sCurEdge.m_fStartLight = iPrevPt->m_fIntensity;
			sCurEdge.m_fEndLight = iCurVert->m_fIntensity;

			pResultEdges->push_back(sCurEdge);

			iPrevPt = iCurVert;
		}
	}
}

void FilterPenumbraEdgeList(const TSLFM_ShadowEdgeList &aInputEdges, TSLFM_ShadowEdgeList *pResultEdges)
{
	const float k_fBlackEpsilon = 0.001f;

	pResultEdges->clear();
	pResultEdges->reserve(aInputEdges.size());

	TSLFM_ShadowEdgeList::const_iterator iInputEdge = aInputEdges.begin();
	for (; iInputEdge != aInputEdges.end(); ++iInputEdge)
	{
		if ((iInputEdge->m_fStartLight > k_fBlackEpsilon) || (iInputEdge->m_fEndLight > k_fBlackEpsilon))
			pResultEdges->push_back(*iInputEdge);
	}
}

void FixEdgeOfPolygonPenumbrae(
	const TSLFM_ShadowEdgeList &aPenumbraEdges, 
	const TProjEdgeList &aShadowEdges,
	const CConvexPolyhedron &cPolyFrustum,
	TSLFM_ShadowEdgeList *pResultEdges)
{
	const float k_fPtOnPlaneEpsilon = 0.001f;

	pResultEdges->clear();
	pResultEdges->reserve(aPenumbraEdges.size());

	TSLFM_ShadowEdgeList::const_iterator iCurPenumbraEdge = aPenumbraEdges.begin();
	TProjEdgeList::const_iterator iCurShadowEdge = aShadowEdges.begin();
	for (; iCurPenumbraEdge != aPenumbraEdges.end(); ++iCurPenumbraEdge, ++iCurShadowEdge)
	{
		SLFM_ShadowEdge sCurEdge = *iCurPenumbraEdge;

		CConvexPolyhedron::TPlaneList::const_iterator iCurPlane = cPolyFrustum.m_aPlanes.begin();
		for (; iCurPlane != cPolyFrustum.m_aPlanes.end(); ++iCurPlane)
		{
			float fStartDist = fabsf(iCurPlane->DistTo(iCurShadowEdge->m_vStartProj));
			float fEndDist = fabsf(iCurPlane->DistTo(iCurShadowEdge->m_vEndProj));
			// Don't fix edges which are entirely on a polygon plane
			if ((fStartDist < k_fPtOnPlaneEpsilon) && (fEndDist < k_fPtOnPlaneEpsilon))
				continue;
			LTVector vPreFiddlingEdge = sCurEdge.m_vEnd - sCurEdge.m_vStart;

			if (fStartDist < k_fPtOnPlaneEpsilon)
			{
				sCurEdge.m_vStart = GetRayPlaneIntersect(sCurEdge.m_vEnd, sCurEdge.m_vStart, *iCurPlane);
			}
			if (fEndDist < k_fPtOnPlaneEpsilon)
			{
				sCurEdge.m_vEnd = GetRayPlaneIntersect(sCurEdge.m_vStart, sCurEdge.m_vEnd, *iCurPlane);
			}

			// I'm not 100% sure why this ends up happening, but extending the edge to the plane intersection
			// swaps the start/end order sometimes
			if (vPreFiddlingEdge.Dot(sCurEdge.m_vEnd - sCurEdge.m_vStart) < 0.0f)
				std::swap(sCurEdge.m_vStart, sCurEdge.m_vEnd);
		}

		pResultEdges->push_back(sCurEdge);
	}
}	

void ProjectEdgeListToPlane(const TProjEdgeList &aInputEdges, const LTPlane &cPlane, TProjEdgeList *pResultEdges)
{
	pResultEdges->clear();
	pResultEdges->reserve(aInputEdges.size());

	TProjEdgeList::const_iterator iCurEdge = aInputEdges.begin();
	for (; iCurEdge != aInputEdges.end(); ++iCurEdge)
	{
		pResultEdges->push_back(*iCurEdge);
		SProjEdge &sResultEdge = pResultEdges->back();
		sResultEdge.m_vStartProj -= cPlane.m_Normal * cPlane.DistTo(sResultEdge.m_vStartProj);
		sResultEdge.m_vEndProj -= cPlane.m_Normal * cPlane.DistTo(sResultEdge.m_vEndProj);
	}
}

void ProjectEdgeListToPlane(const TSLFM_ShadowEdgeList &aInputEdges, const LTPlane &cPlane, TSLFM_ShadowEdgeList *pResultEdges)
{
	pResultEdges->clear();
	pResultEdges->reserve(aInputEdges.size());

	TSLFM_ShadowEdgeList::const_iterator iCurEdge = aInputEdges.begin();
	for (; iCurEdge != aInputEdges.end(); ++iCurEdge)
	{
		pResultEdges->push_back(*iCurEdge);
		SLFM_ShadowEdge &sResultEdge = pResultEdges->back();
		sResultEdge.m_vStart -= cPlane.m_Normal * cPlane.DistTo(sResultEdge.m_vStart);
		sResultEdge.m_vEnd -= cPlane.m_Normal * cPlane.DistTo(sResultEdge.m_vEnd);
	}
}

void SnapNeighborEndpoints(const TProjEdgeList &aInputEdges, const TIndexList &aNeighborList, TProjEdgeList *pResultEdges)
{
	*pResultEdges = aInputEdges;

	TProjEdgeList::iterator iCurEdge = pResultEdges->begin();
	TIndexList::const_iterator iCurNeighbor = aNeighborList.begin();
	for (; iCurEdge != pResultEdges->end(); ++iCurEdge, ++iCurNeighbor)
	{
		if (*iCurNeighbor == INVALID_INDEX)
			continue;

		const SProjEdge &sNeighbor = (*pResultEdges)[*iCurNeighbor];

		iCurEdge->m_vEndProj = sNeighbor.m_vStartProj;
	}
}

void CreateRevNeighborList(const TIndexList &aNeighborList, TIndexList *pResultList)
{
	pResultList->resize(aNeighborList.size(), INVALID_INDEX);

	TIndexList::const_iterator iCurNeighbor = aNeighborList.begin();
	for (; iCurNeighbor != aNeighborList.end(); ++iCurNeighbor)
	{
		if (*iCurNeighbor != INVALID_INDEX)
			(*pResultList)[*iCurNeighbor] = iCurNeighbor - aNeighborList.begin();
	}
}

LTVector CalcEdgeConnectPt(const SLFM_ShadowEdge &sEdge1, const SLFM_ShadowEdge &sEdge2)
{
	// This is temporary...
	// NYI
	return (sEdge1.m_vEnd + sEdge2.m_vStart) * 0.5f;
}

void ConnectNeighborPenumbrae(
	const TSLFM_ShadowEdgeList &aEdges,
	const TIndexList &aNextNeighborList,
	const TIndexList &aPrevNeighborList,
	const LTPlane &cPlane,
	TSLFM_ShadowEdgeList *pResultList)
{
	ASSERT(aEdges.size() == aNextNeighborList.size());
	ASSERT(aEdges.size() == aPrevNeighborList.size());

	pResultList->reserve(aEdges.size());

	TSLFM_ShadowEdgeList::const_iterator iCurEdge = aEdges.begin();
	TIndexList::const_iterator iCurNext = aNextNeighborList.begin();
	TIndexList::const_iterator iCurPrev = aPrevNeighborList.begin();
	for (; iCurEdge != aEdges.end(); ++iCurEdge, ++iCurNext, ++iCurPrev)
	{
		SLFM_ShadowEdge sCurEdge = *iCurEdge;

		if (*iCurNext != INVALID_INDEX)
		{
			const SLFM_ShadowEdge &sNeighbor = aEdges[*iCurNext];
			if (!IsOutsideCorner(sCurEdge, sNeighbor, cPlane.m_Normal))
			{
				sCurEdge.m_vEnd = CalcEdgeConnectPt(sCurEdge, sNeighbor);
			}
		}

		if (*iCurPrev != INVALID_INDEX)
		{
			const SLFM_ShadowEdge &sNeighbor = aEdges[*iCurPrev];
			if (!IsOutsideCorner(sNeighbor, sCurEdge, cPlane.m_Normal))
			{
				sCurEdge.m_vStart = CalcEdgeConnectPt(sNeighbor, sCurEdge);
			}
		}

		pResultList->push_back(sCurEdge);
	}
}

void FixPenumbraEdgeOrdering(const TSLFM_ShadowEdgeList &aPenumbraEdges, const TProjEdgeList &aShadowEdges, TSLFM_ShadowEdgeList *pResultEdges)
{
	pResultEdges->reserve(aPenumbraEdges.size());

	TSLFM_ShadowEdgeList::const_iterator iCurPenumbra = aPenumbraEdges.begin();
	TProjEdgeList::const_iterator iCurShadow = aShadowEdges.begin();
	for (; iCurPenumbra != aPenumbraEdges.end(); ++iCurPenumbra, ++iCurShadow)
	{
		pResultEdges->push_back(*iCurPenumbra);

		// Handle shadow edges that were 0-length
		if (iCurShadow->m_vStartProj.Dist(iCurShadow->m_vEndProj) < 0.001f)
		{
			--iCurShadow;
			continue;
		}
			
		LTVector vOffset = iCurShadow->m_vEndProj - iCurShadow->m_vStartProj;
		if (vOffset.Dot(iCurPenumbra->m_vEnd - iCurPenumbra->m_vStart) < 0.0f)
			std::swap(pResultEdges->back().m_vStart, pResultEdges->back().m_vEnd);
	}
}

// Temporary implementation details end here

void CLightFragmentMaker::ProjectEdges(const CLightDef *pLightDef, const TEdgeList &aEdges)
{
	// Get the polys touching the light
	TOutlineList aPolysInLight;
	TIndexList aPolysInLightIndices;
	TOutlineList::iterator iGatherPolysInLight = m_aOutlines.begin();
	for (; iGatherPolysInLight != m_aOutlines.end(); ++iGatherPolysInLight)
	{
		if (LightTouchesPoly(pLightDef, **iGatherPolysInLight))
		{
			aPolysInLight.push_back(*iGatherPolysInLight);
			aPolysInLightIndices.push_back(iGatherPolysInLight - m_aOutlines.begin());
		}
	}

	// Get the shadows for the polygon..
	TOutlineList::iterator iReceiver = aPolysInLight.begin();
	TIndexList::iterator iReceiverIndex = aPolysInLightIndices.begin();
	for (; iReceiver != aPolysInLight.end(); ++iReceiver, ++iReceiverIndex)
	{
		SLFM_Outline &sReceiver = **iReceiver;
		TShadowEdgeList &sReceiverEdgeList = m_aFragmentEdges[*iReceiverIndex];

		// Skip non-shadowed polys
		if ((sReceiver.m_pPrePoly->GetSurfaceFlags() & SURF_SHADOWMESH) == 0)
			continue;
	
		// Create the lighting frustum
		CConvexPolyhedron cLightFrustum;
		CreateLightFrustum(sReceiver, pLightDef, &cLightFrustum);

		CConvexPolyhedron cUncappedLightFrustum = cLightFrustum;
		cUncappedLightFrustum.m_aPlanes.erase(cUncappedLightFrustum.m_aPlanes.begin());

		// Clip the edge list to the lighting frustum
		TEdgeList aShadowEdges;
		ClipEdgeListToPolyhedron(aEdges, cLightFrustum, false, &aShadowEdges);

		if (aShadowEdges.empty())
			continue;

		// Project the edges onto the poly
		TProjEdgeList aProjEdges;
		TEdgeList::iterator iProjectEdge = aShadowEdges.begin();
		for (; iProjectEdge != aShadowEdges.end(); ++iProjectEdge)
		{
			aProjEdges.push_back(SProjEdge(*iProjectEdge, sReceiver.m_cPlane, pLightDef->m_Pos));
		}

		// Clip the edges to the shadows of the polys touching the light frustum
		TProjEdgeList aClippedShadowEdges;
		TOutlineList aPolysBlocking;
		TOutlineList::const_iterator iCurShadowPoly = aPolysInLight.begin();
		for (; (iCurShadowPoly != aPolysInLight.end()) && (!aProjEdges.empty()); ++iCurShadowPoly)
		{
			if (iCurShadowPoly == iReceiver)
				continue;

			SLFM_Outline &sBlocker = **iCurShadowPoly;

			if ((sBlocker.m_pPrePoly->GetSurfaceFlags() & SURF_CASTSHADOWMESH) == 0)
				continue;

			if (!OutlineTouchesPolyhedron(sBlocker, cLightFrustum))
				continue;

			aPolysBlocking.push_back(*iCurShadowPoly);

			CConvexPolyhedron cShadowFrustum;
			CreateShadowFrustum(sBlocker, pLightDef, &cShadowFrustum);

			ClipEdgeListToPolyhedron(aProjEdges, cShadowFrustum, true, &aClippedShadowEdges);
			aProjEdges.swap(aClippedShadowEdges);
		}

		// Make absolutely sure all edges are touching the polygon
		ProjectEdgeListToPlane(aProjEdges, sReceiver.m_cPlane, &aClippedShadowEdges);
		ClipEdgeListToPolyhedron(aClippedShadowEdges, cUncappedLightFrustum, false, &aProjEdges);

		// Add the resulting edges to the final edge list
		TProjEdgeList aFinalEdges;
		MergeSimilarEdges(aProjEdges, &aFinalEdges);

		TIndexList aEdgeNeighborList;
		CreateProjEdgeNeighborList(aFinalEdges, &aEdgeNeighborList);

		TIndexList aEdgeRevNeighborList;
		CreateRevNeighborList(aEdgeNeighborList, &aEdgeRevNeighborList);

		SnapNeighborEndpoints(aFinalEdges, aEdgeNeighborList, &aProjEdges);
		aProjEdges.swap(aFinalEdges);
		
		// Convert them to shadow edges
		AppendProjEdgeListToShadowEdgeList(aFinalEdges, pLightDef, 0.0f, &sReceiverEdgeList);

		// NYI
		/*
			for each projected edge
				Determine penumbra
				Create polygon from penumbra edge to un-projected edge
				Create polyhedron around penumbra volume
			For each penumbra poly
				Clip to all shadow volumes
				Clip to all penumbra volumes
				Throw away edges which are entirely black
				Project onto plane
				Add to edge list (merge? Keep 0-length span edges?)
		*/

		// Not Testing
		//*

		TShadowEdgeList aPenumbraEdges;
		CreatePenumbraEdges(aFinalEdges, sReceiver.m_cPlane, pLightDef, &aPenumbraEdges);

		TShadowEdgeList aEdgeFixedPenumbraEdges;
		FixEdgeOfPolygonPenumbrae(aPenumbraEdges, aFinalEdges, cUncappedLightFrustum, &aEdgeFixedPenumbraEdges);
		aEdgeFixedPenumbraEdges.swap(aPenumbraEdges);

		TShadowEdgeList aNeighborFixedPenumbraEdges;
		ConnectNeighborPenumbrae(aPenumbraEdges, aEdgeNeighborList, aEdgeRevNeighborList, sReceiver.m_cPlane, &aNeighborFixedPenumbraEdges);
		aNeighborFixedPenumbraEdges.swap(aPenumbraEdges);

		TShadowEdgeList aOrderFixedPenumbraEdges;
		FixPenumbraEdgeOrdering(aPenumbraEdges, aFinalEdges, &aOrderFixedPenumbraEdges);
		aOrderFixedPenumbraEdges.swap(aPenumbraEdges);

		TShadowPolyList aPenumbraPolys;
		CreatePenumbraPolys(aPenumbraEdges, aFinalEdges, sReceiver.m_cPlane, aEdgeNeighborList, &aPenumbraPolys);

		TConvexPolyhedronList aPenumbraClipList;
		CreatePenumbraVolumes(aPenumbraEdges, aFinalEdges, sReceiver.m_cPlane, &aPenumbraClipList);

		CreatePenumbraCornerPolys(aPenumbraEdges, aFinalEdges, aEdgeNeighborList, sReceiver.m_cPlane, &aPenumbraPolys);
		CreatePenumbraCornerVolumes(aPenumbraEdges, aFinalEdges, aEdgeNeighborList, sReceiver.m_cPlane, &aPenumbraClipList);

		TOutlineList::iterator iCurBlocker = aPolysBlocking.begin();
		for (; iCurBlocker != aPolysBlocking.end(); ++iCurBlocker)
		{
			SLFM_Outline sBlocker = **iCurBlocker;

			CConvexPolyhedron cShadowFrustum;
			CreateProjShadowFrustum(sBlocker, pLightDef, sReceiver.m_cPlane, &cShadowFrustum);
			
			aPenumbraClipList.push_back(cShadowFrustum);
		}
		
		TShadowPolyList aClippedPenumbrae;
		ClipShadowPolysToConvexPolyhedronList(aPenumbraPolys, aPenumbraClipList, &aClippedPenumbrae);

		GetShadowPolyEdges(aClippedPenumbrae, sReceiver.m_cPlane, pLightDef, &aPenumbraEdges);

		TShadowEdgeList aFilteredPenumbraEdges;
		FilterPenumbraEdgeList(aPenumbraEdges, &aFilteredPenumbraEdges);

		// Make absolutely sure all edges are touching the polygon
		ProjectEdgeListToPlane(aFilteredPenumbraEdges, sReceiver.m_cPlane, &aPenumbraEdges);
		TShadowEdgeList aClippedPenumbraEdges;
		ClipEdgeListToPolyhedron(aPenumbraEdges, cUncappedLightFrustum, &aClippedPenumbraEdges);

		TShadowEdgeList aMergedPenumbraEdges;
		MergeSimilarEdges(aClippedPenumbraEdges, &aMergedPenumbraEdges);

		sReceiverEdgeList.insert(sReceiverEdgeList.end(), aMergedPenumbraEdges.begin(), aMergedPenumbraEdges.end());
		//*/
		// End Testing

		// Not Testing
		/*
		{
			TProjEdgeList::const_iterator iCurEdge = aFinalEdges.begin();
			for (; iCurEdge != aFinalEdges.end(); ++iCurEdge)
			{
				SLFM_ShadowEdge sNewEdge;
				sNewEdge.m_vStart = iCurEdge->m_vEndProj;
				sNewEdge.m_vEnd = iCurEdge->m_vStartProj;
				sNewEdge.m_fStartLight = 1.0f;
				sNewEdge.m_fEndLight = 1.0f;
				sNewEdge.m_pLight = pLightDef;

				sReceiverEdgeList.push_back(sNewEdge);
			}
		}
		//*/
		// End Testing

	}
}

void CLightFragmentMaker::FixCrossedEdges(const LTPlane &cPlane, const TShadowEdgeList &cEdges, TShadowEdgeList *pResultEdges) const
{
	// Don't process it if it's going to take too long
	const uint32 k_nTooManyTris = 1000;
	if (cEdges.size() > k_nTooManyTris)
	{
		ASSERT(!"Too many triangles found on polygon.  I'm not going to do that.");
		return;
	}

	// Note : This routine will currently get stuck if points are too far off of the polygon plane

	const float k_fEndOfLineEpsilon = 0.01f;
	const float k_fEmptyEdgeEpsilon = 0.1f;

	const float k_fDuplicatePointEpsilon = 0.1f;
	const float k_fPtOnPlaneEpsilon = 0.01f;

	std::stack<SLFM_ShadowEdge> cWaitingEdges;

	TShadowEdgeList::const_iterator iCurEdge = cEdges.begin();
	while (iCurEdge != cEdges.end() || !cWaitingEdges.empty())
	{
		bool bFromList;

		SLFM_ShadowEdge sCurEdge;
		if (cWaitingEdges.empty())
		{
			sCurEdge = *iCurEdge;
			++iCurEdge;
			bFromList = true;
		}
		else
		{
			bFromList = false;
			sCurEdge = cWaitingEdges.top();
			cWaitingEdges.pop();
		}

		// Calculate the parametric line describing this edge
		LTVector vEdgeDir = sCurEdge.m_vEnd - sCurEdge.m_vStart;
		float fEdgeMag = vEdgeDir.Mag();
		if (fEdgeMag < k_fDuplicatePointEpsilon)
			continue;
		vEdgeDir /= fEdgeMag;
		float fEdgeStart = vEdgeDir.Dot(sCurEdge.m_vStart);

		ASSERT(fabsf(cPlane.DistTo(sCurEdge.m_vEnd)) < k_fPtOnPlaneEpsilon);
		ASSERT(fabsf(cPlane.DistTo(sCurEdge.m_vStart)) < k_fPtOnPlaneEpsilon);
		
		// Get a plane perpendicular to the poly plane along this edge
		LTPlane cEdgePlane;
		cEdgePlane.m_Normal = cPlane.m_Normal.Cross(vEdgeDir);
		cEdgePlane.m_Normal.Normalize();
		cEdgePlane.m_Dist = cEdgePlane.m_Normal.Dot(sCurEdge.m_vStart);

		// Go through our edge list looking for splits...
		TShadowEdgeList::const_iterator iSearchEdge = cEdges.begin();
		for (; iSearchEdge != cEdges.end(); ++iSearchEdge)
		{
			// Don't check the edge against itself
			if (bFromList && ((iCurEdge - 1) == iSearchEdge))
				continue;

			// Shortcut out if we ended up with an empty edge
			if (sCurEdge.m_vStart.NearlyEquals(sCurEdge.m_vEnd, k_fEmptyEdgeEpsilon))
				break;

			// Get the projection of the search edge onto the current edge
			float fSearchEdgeStart = vEdgeDir.Dot(iSearchEdge->m_vStart) - fEdgeStart;
			float fSearchEdgeEnd = vEdgeDir.Dot(iSearchEdge->m_vEnd) - fEdgeStart;
			float fSearchEdgeProjMin = LTMIN(fSearchEdgeStart, fSearchEdgeEnd);
			float fSearchEdgeProjMax = LTMAX(fSearchEdgeStart, fSearchEdgeEnd);

			// Skip edges that don't project onto this edge
			if ((fSearchEdgeProjMin > (fEdgeMag - k_fDuplicatePointEpsilon)) ||
				(fSearchEdgeProjMax < k_fDuplicatePointEpsilon))
				continue;

			// Get the search edge's plane distances
			float fPlaneDistStart = cEdgePlane.DistTo(iSearchEdge->m_vStart);
			float fPlaneDistEnd = cEdgePlane.DistTo(iSearchEdge->m_vEnd);

			// Skip edges that don't intersect the edge plane
			if (((fPlaneDistStart > k_fPtOnPlaneEpsilon) && (fPlaneDistEnd > -k_fPtOnPlaneEpsilon)) ||
				((fPlaneDistStart < -k_fPtOnPlaneEpsilon) && (fPlaneDistEnd < k_fPtOnPlaneEpsilon)) ||
				((fPlaneDistEnd > k_fPtOnPlaneEpsilon) && (fPlaneDistStart > -k_fPtOnPlaneEpsilon)) ||
				((fPlaneDistEnd < -k_fPtOnPlaneEpsilon) && (fPlaneDistStart < k_fPtOnPlaneEpsilon)))
				continue;

			// Get the point of intersection
			LTVector vIntersectPt;
			// Handle edges which overlap some portion of the current edge
			if ((fabsf(fPlaneDistStart) < k_fPtOnPlaneEpsilon) &&
				(fabsf(fPlaneDistEnd) < k_fPtOnPlaneEpsilon))
			{
				// Handle an edge totally overlapping the current edge
				if ((fSearchEdgeProjMin < k_fDuplicatePointEpsilon) &&
					(fSearchEdgeProjMax > (fEdgeMag - k_fDuplicatePointEpsilon)))
				{
					// Note : This is probably the same edge.  The tesselator handles filtering duplicate edges.
				}
				// Handle an edge which overlaps the beginning
				else if (fSearchEdgeProjMin < k_fDuplicatePointEpsilon)
				{
					float fIntersectLight = LTLERP(sCurEdge.m_fStartLight, sCurEdge.m_fEndLight, fSearchEdgeProjMax / fEdgeMag);
					sCurEdge.m_vStart = (fSearchEdgeProjMax == fSearchEdgeStart) ? iSearchEdge->m_vStart : iSearchEdge->m_vEnd;
					sCurEdge.m_fStartLight = fIntersectLight;
					fEdgeStart += fSearchEdgeProjMax;
					fEdgeMag -= fSearchEdgeProjMax;
				}
				// Handle an edge which overlaps the ending
				else if (fSearchEdgeProjMax > (fEdgeMag - k_fDuplicatePointEpsilon))
				{
					float fIntersectLight = LTLERP(sCurEdge.m_fStartLight, sCurEdge.m_fEndLight, fSearchEdgeProjMin / fEdgeMag);
					sCurEdge.m_vEnd = (fSearchEdgeProjMin == fSearchEdgeStart) ? iSearchEdge->m_vStart : iSearchEdge->m_vEnd;
					sCurEdge.m_fEndLight = fIntersectLight;
					fEdgeMag -= fSearchEdgeProjMin;
				}
				// Handle an edge which overlaps in the middle
				else
				{
					// Put the end-portion onto the stack
					float fIntersectLight = LTLERP(sCurEdge.m_fStartLight, sCurEdge.m_fEndLight, fSearchEdgeProjMax / fEdgeMag);
					vIntersectPt = (fSearchEdgeProjMax == fSearchEdgeStart) ? iSearchEdge->m_vStart : iSearchEdge->m_vEnd;
					cWaitingEdges.push(SLFM_ShadowEdge(vIntersectPt, sCurEdge.m_vEnd, sCurEdge.m_pLight, fIntersectLight, sCurEdge.m_fEndLight));

					// Adjust the current edge
					fIntersectLight = LTLERP(sCurEdge.m_fStartLight, sCurEdge.m_fEndLight, fSearchEdgeProjMin / fEdgeMag);
					vIntersectPt = (fSearchEdgeProjMin == fSearchEdgeStart) ? iSearchEdge->m_vStart : iSearchEdge->m_vEnd;
					sCurEdge.m_vEnd = vIntersectPt;
					sCurEdge.m_fEndLight = fIntersectLight;
					fEdgeMag -= fSearchEdgeProjMin;
				}
				// Overlaps are never interpolated in the normal fashion..
				continue;
			}
			else if (fabsf(fPlaneDistStart) < k_fPtOnPlaneEpsilon)
			{
				vIntersectPt = iSearchEdge->m_vStart;
			}
			else if (fabsf(fPlaneDistEnd) < k_fPtOnPlaneEpsilon)
			{
				vIntersectPt = iSearchEdge->m_vEnd;
			}
			else
			{
				float fSearchIntersectTime = fPlaneDistStart / (fPlaneDistStart - fPlaneDistEnd);
				ASSERT((fSearchIntersectTime > 0.0f) && (fSearchIntersectTime < 1.0f));
				VEC_LERP(vIntersectPt, iSearchEdge->m_vStart, iSearchEdge->m_vEnd, fSearchIntersectTime);
			}
			// Check the projection of the intersection
			float fIntersectProj = vEdgeDir.Dot(vIntersectPt) - fEdgeStart;
			// Skip it if it intersected on an endpoint
			if ((fIntersectProj < k_fDuplicatePointEpsilon) || 
				(fIntersectProj > (fEdgeMag - k_fDuplicatePointEpsilon)))
				continue;

			// We now know that we've got an intersection, as well as where we've got it
			float fNormalizedIntersect = fIntersectProj / fEdgeMag;
			float fIntersectLight = LTLERP(sCurEdge.m_fStartLight, sCurEdge.m_fEndLight, fNormalizedIntersect);
			if (!sCurEdge.m_vEnd.NearlyEquals(vIntersectPt, k_fEmptyEdgeEpsilon))
				cWaitingEdges.push(SLFM_ShadowEdge(vIntersectPt, sCurEdge.m_vEnd, sCurEdge.m_pLight, fIntersectLight, sCurEdge.m_fEndLight));
			sCurEdge.m_vEnd = vIntersectPt;
			sCurEdge.m_fEndLight = fIntersectLight;
			fEdgeMag = fIntersectProj;
		}

		if (sCurEdge.m_vStart.NearlyEquals(sCurEdge.m_vEnd, k_fEmptyEdgeEpsilon))
			continue;

		// Add whatever's left of the edge to the results
		pResultEdges->push_back(sCurEdge);
	}
}

struct SWaitingEdge
{
	SWaitingEdge(uint32 nStart, uint32 nEnd) : m_nStart(nStart), m_nEnd(nEnd) {}
	SWaitingEdge(const SWaitingEdge &sOther) : m_nStart(sOther.m_nStart), m_nEnd(sOther.m_nEnd) {}
	SWaitingEdge &operator=(const SWaitingEdge &sOther) { m_nStart = sOther.m_nStart; m_nEnd = sOther.m_nEnd; return *this; }
	uint32 m_nStart, m_nEnd;
};
typedef std::vector<SWaitingEdge> TWaitingEdgeList;

typedef SLFM_Triangulation<SLFM_FinalLightingVert> TTriangulation;
void Triangulate_Peel_Double(
	const SLFM_ShadowEdge &cEdge, 
	const SWaitingEdge &cEdgeIndices, 
	TIndexList *pSinglePeelEdges, 
	TIndexList *pSinglePeelOldVertices, 
	TIndexList *pSinglePeelNewVertices, 
	TTriangulation *pTriangulation)
{
	// Get the singularity edge index
	uint32 nOldEdge;
	if (!pTriangulation->FindEdge(cEdgeIndices.m_nStart, cEdgeIndices.m_nEnd, &nOldEdge))
	{
		if (!pTriangulation->InsertHardEdge(cEdgeIndices.m_nStart, cEdgeIndices.m_nEnd))
		{
			ASSERT(!"Unable to insert prior edge in double peel");
			return;
		}
		if (!pTriangulation->FindEdge(cEdgeIndices.m_nStart, cEdgeIndices.m_nEnd, &nOldEdge))
		{
			ASSERT(!"Unable to find inserted edge in double peel");
			return;
		}
	}
	// Make sure it's a hard edge...
	pTriangulation->m_aEdges[nOldEdge].m_bHardEdge = true;

	// Add vertices for the new edge
	uint32 nNewStartVert = pTriangulation->m_aVertices.size();
	SLFM_FinalLightingVert sNewStart = pTriangulation->GetVert(cEdgeIndices.m_nStart);
	sNewStart.SetLightIntensity(cEdge.m_pLight, cEdge.m_fStartLight);
	pTriangulation->m_aVertices.push_back(sNewStart);
	uint32 nNewEndVert = pTriangulation->m_aVertices.size();
	SLFM_FinalLightingVert sNewEnd = pTriangulation->GetVert(cEdgeIndices.m_nEnd);
	sNewEnd.SetLightIntensity(cEdge.m_pLight, cEdge.m_fEndLight);
	pTriangulation->m_aVertices.push_back(sNewEnd);

	// Get the vertices for the peeling edge
	SLFM_FinalLightingVert &sPeelStart = pTriangulation->GetVert(cEdgeIndices.m_nStart);
	SLFM_FinalLightingVert &sPeelEnd = pTriangulation->GetVert(cEdgeIndices.m_nEnd);

	// Add a new edge
	uint32 nNewEdge = pTriangulation->m_aEdges.size();
	pTriangulation->m_aEdges.push_back(TTriangulation::SEdge(nNewStartVert, nNewEndVert));

	bool bPeeledEdge = false;

	TTriangulation::TTriangleList::iterator iCurTri = pTriangulation->m_aTriangles.begin();
	for (; iCurTri != pTriangulation->m_aTriangles.end(); ++iCurTri)
	{
		// Skip over triangles that don't include this edge
		if (!iCurTri->HasEdge(nOldEdge))
			continue;

		// Figure out which vertex it is that we don't already know about
		uint32 nTriVert0, nTriVert1, nTriVert2;
		pTriangulation->GetTriVerts(*iCurTri, &nTriVert0, &nTriVert1, &nTriVert2);
		// Note : I love the XOR operator...
		uint32 nOtherVert = nTriVert0 ^ nTriVert1 ^ nTriVert2 ^ cEdgeIndices.m_nStart ^ cEdgeIndices.m_nEnd;

		// Get the lighting from the other vertex
		SLFM_FinalLightingVert &sOtherVert = pTriangulation->GetVert(nOtherVert);
		float fOtherLight = sOtherVert.GetLightIntensity(cEdge.m_pLight, -1.0f);

		// Because the shadow edges are provided in clockwise order, we can use that
		// information to determine which side of the edge should be assigned to the
		// new, lit edge
		LTVector vOtherVertOfs = sOtherVert.m_vPos - cEdge.m_vStart;
		LTVector vTestNormal = vOtherVertOfs.Cross(cEdge.m_vEnd - cEdge.m_vStart);
		if (vTestNormal.Dot(pTriangulation->m_cPlane.m_Normal) < 0.0f)
			continue;

		// Get the edges connecting the other vertex to the new indices
		uint32 nStartEdge, nEndEdge;
		if (!pTriangulation->FindEdge(nNewStartVert, nOtherVert, &nStartEdge))
		{
			nStartEdge = pTriangulation->m_aEdges.size();
			pTriangulation->m_aEdges.push_back(TTriangulation::SEdge(nNewStartVert, nOtherVert, true));
		}
		if (!pTriangulation->FindEdge(nNewEndVert, nOtherVert, &nEndEdge))
		{
			nEndEdge = pTriangulation->m_aEdges.size();
			pTriangulation->m_aEdges.push_back(TTriangulation::SEdge(nNewEndVert, nOtherVert, true));
		}

		// Add the old edges to the single-peel list, since they now neighbor triangles
		// that are pointing at the old vertex
		if (iCurTri->m_nEdge0 != nOldEdge)
		{
			TTriangulation::SEdge &sEdge = pTriangulation->m_aEdges[iCurTri->m_nEdge0];
			bool bStart = (sEdge.m_nStart == cEdgeIndices.m_nStart) || (sEdge.m_nEnd == cEdgeIndices.m_nStart);
			pSinglePeelEdges->push_back(iCurTri->m_nEdge0);
			pSinglePeelOldVertices->push_back((bStart) ? cEdgeIndices.m_nStart : cEdgeIndices.m_nEnd);
			pSinglePeelNewVertices->push_back((bStart) ? nNewStartVert : nNewEndVert);
		}
		if (iCurTri->m_nEdge1 != nOldEdge)
		{
			TTriangulation::SEdge &sEdge = pTriangulation->m_aEdges[iCurTri->m_nEdge1];
			bool bStart = (sEdge.m_nStart == cEdgeIndices.m_nStart) || (sEdge.m_nEnd == cEdgeIndices.m_nStart);
			pSinglePeelEdges->push_back(iCurTri->m_nEdge1);
			pSinglePeelOldVertices->push_back((bStart) ? cEdgeIndices.m_nStart : cEdgeIndices.m_nEnd);
			pSinglePeelNewVertices->push_back((bStart) ? nNewStartVert : nNewEndVert);
		}
		if (iCurTri->m_nEdge2 != nOldEdge)
		{
			TTriangulation::SEdge &sEdge = pTriangulation->m_aEdges[iCurTri->m_nEdge2];
			bool bStart = (sEdge.m_nStart == cEdgeIndices.m_nStart) || (sEdge.m_nEnd == cEdgeIndices.m_nStart);
			pSinglePeelEdges->push_back(iCurTri->m_nEdge2);
			pSinglePeelOldVertices->push_back((bStart) ? cEdgeIndices.m_nStart : cEdgeIndices.m_nEnd);
			pSinglePeelNewVertices->push_back((bStart) ? nNewStartVert : nNewEndVert);
		}

		// Re-wire the triangle
		iCurTri->m_nEdge0 = nStartEdge;
		iCurTri->m_nEdge1 = nEndEdge;
		iCurTri->m_nEdge2 = nNewEdge;

		bPeeledEdge = true;
	}

	ASSERT(bPeeledEdge);
}

void Triangulate_Peel_Single(uint32 nEdgeIndex, uint32 nOldVertIndex, uint32 nNewVertIndex, SLFM_Triangulation<SLFM_FinalLightingVert> *pTriangulation)
{
	std::stack<uint32> aPeelEdges;
	aPeelEdges.push(nEdgeIndex);

	// Go back through and peel off the secondary edges
	while (!aPeelEdges.empty())
	{
		uint32 nPeelEdge = aPeelEdges.top();
		aPeelEdges.pop();

		TTriangulation::SEdge &sPeelEdge = pTriangulation->m_aEdges[nPeelEdge];
		uint32 nStartVert = sPeelEdge.m_nStart;
		uint32 nEndVert = sPeelEdge.m_nEnd;

		if (nStartVert != nOldVertIndex)
		{
			std::swap(nStartVert, nEndVert);
			ASSERT(nStartVert == nOldVertIndex);
		}

		uint32 nNewPeelEdge;
		bool bNewEdgeFound = pTriangulation->FindEdge(nNewVertIndex, nEndVert, &nNewPeelEdge);
		ASSERT(bNewEdgeFound);

		TTriangulation::TTriangleList::iterator iCurTri = pTriangulation->m_aTriangles.begin();
		for (; iCurTri != pTriangulation->m_aTriangles.end(); ++iCurTri)
		{
			// Skip over triangles that don't include this edge
			if (!iCurTri->HasEdge(nPeelEdge))
				continue;

			// Figure out which vertex it is that we don't already know about
			uint32 nTriVert0, nTriVert1, nTriVert2;
			pTriangulation->GetTriVerts(*iCurTri, &nTriVert0, &nTriVert1, &nTriVert2);
			// Note : I love the XOR operator...
			uint32 nOtherVert = nTriVert0 ^ nTriVert1 ^ nTriVert2 ^ nStartVert ^ nEndVert;

			// Figure out what the other edges of the triangle were
			uint32 nOtherTriEdge;
			if (iCurTri->m_nEdge0 == nPeelEdge)
				std::swap(iCurTri->m_nEdge0, iCurTri->m_nEdge2);
			else if (iCurTri->m_nEdge1 == nPeelEdge)
				std::swap(iCurTri->m_nEdge1, iCurTri->m_nEdge2);
			TTriangulation::SEdge &sEdge0 = pTriangulation->m_aEdges[iCurTri->m_nEdge0];
			if ((sEdge0.m_nStart == nEndVert) || (sEdge0.m_nEnd == nEndVert))
				nOtherTriEdge = iCurTri->m_nEdge0;
			else
				nOtherTriEdge = iCurTri->m_nEdge1;
			uint32 nOldOpposingEdge = iCurTri->m_nEdge0 ^ iCurTri->m_nEdge1 ^ iCurTri->m_nEdge2 ^ nPeelEdge ^ nOtherTriEdge;

			// Find the new opposing edge
			uint32 nNewOpposingEdge;
			bool bNewOpposingEdgeFound = pTriangulation->FindEdge(nNewVertIndex, nOtherVert, &nNewOpposingEdge);
			if (!bNewOpposingEdgeFound)
			{
				// If we didn't find the opposing edge already, that means we have to chain again
				nNewOpposingEdge = pTriangulation->m_aEdges.size();
				pTriangulation->m_aEdges.push_back(TTriangulation::SEdge(nNewVertIndex, nOtherVert, true));
				aPeelEdges.push(nOldOpposingEdge);
			}

			// Set up the tri to point at the new stuff
			iCurTri->m_nEdge0 = nNewPeelEdge;
			iCurTri->m_nEdge1 = nNewOpposingEdge;
			iCurTri->m_nEdge2 = nOtherTriEdge;
		}
	}
}

// Note : This is implemented inside of CLightFragmentMaker because it takes an edge list as a parameter
void CLightFragmentMaker::Triangulate(const SLFM_Outline &cPoly, const TShadowEdgeList &cEdges, SLFM_Triangulation<SLFM_FinalLightingVert> *pTriangulation)
{
	// Note : For now, this doesn't do any triangulation optimization.
	// And it's not very optimized.

	const uint32 k_nTooManyTris = 1000;

	pTriangulation->m_cPlane = cPoly.m_cPlane;

	// Insert the edges & triangles for the main poly
	const CPrePoly *pWorldPoly = cPoly.m_pPrePoly;
	uint32 nNumVerts = pWorldPoly->NumVerts();
	uint32 nCurVert;
	for (nCurVert = 0; nCurVert < nNumVerts; ++nCurVert)
	{
		const CPolyVert &cVert = pWorldPoly->Vert(nCurVert);
		pTriangulation->m_aVertices.push_back(SLFM_Triangulation<SLFM_FinalLightingVert>::SVert(cVert.m_Vec, cVert.m_Normal));
	}

	for (nCurVert = 0; nCurVert < nNumVerts; ++nCurVert)
	{
		pTriangulation->m_aEdges.push_back(SLFM_Triangulation<SLFM_FinalLightingVert>::SEdge(nCurVert, (nCurVert + 1) % pWorldPoly->NumVerts(), true));
	}

	for (nCurVert = 2; nCurVert < (nNumVerts - 1); ++nCurVert)
	{
		pTriangulation->m_aEdges.push_back(SLFM_Triangulation<SLFM_FinalLightingVert>::SEdge(0, nCurVert, false));
	}

	if (nNumVerts > 3)
	{
		pTriangulation->m_aTriangles.push_back(SLFM_Triangulation<SLFM_FinalLightingVert>::STriangle(0, 1, nNumVerts));
		for (nCurVert = 2; nCurVert < (nNumVerts - 2); ++nCurVert)
			pTriangulation->m_aTriangles.push_back(SLFM_Triangulation<SLFM_FinalLightingVert>::STriangle(nNumVerts + nCurVert - 2, nCurVert, nNumVerts + nCurVert - 1));
		pTriangulation->m_aTriangles.push_back(SLFM_Triangulation<SLFM_FinalLightingVert>::STriangle(pTriangulation->m_aEdges.size() -  1, nNumVerts - 2, nNumVerts - 1));
	}
	else
	{
		pTriangulation->m_aTriangles.push_back(SLFM_Triangulation<SLFM_FinalLightingVert>::STriangle(0, 1, 2));
	}

	if (cEdges.size() > k_nTooManyTris)
	{
		ASSERT(!"Too many triangles found on polygon.  I'm not going to do that.");
		return;
	}

	// Insert all edges, grouped in light order...	Note that this could be accomplished
	// by sorting the list by light and doing the per-light modifications whenever
	// the light changes, but the code isn't quite set up that way right now.
	TLightDefList::const_iterator iCurLight = m_aWorldLights.begin();
	for (; iCurLight != m_aWorldLights.end(); ++iCurLight)
	{
		TWaitingEdgeList aWaitingDoubleIndices;
		TShadowEdgeList aWaitingDoubleEdges;
		// Note : These single-peel edges aren't implemented at this point, as they're
		// for peeling penumbra edges, and I haven't figured out how to do that reliably yet...
		TWaitingEdgeList aWaitingStartIndices;
		TShadowEdgeList aWaitingStartEdges;
		TWaitingEdgeList aWaitingEndIndices;
		TShadowEdgeList aWaitingEndEdges;
		TWaitingEdgeList aWaitingReInsertEdges;

		// Insert the extra edges
		TShadowEdgeList::const_iterator iCurEdge = cEdges.begin();
		for (; iCurEdge != cEdges.end(); ++iCurEdge)
		{
			if (iCurEdge->m_pLight != *iCurLight)
				continue;

			SLFM_FinalLightingVert sStartVert;
			sStartVert.m_vPos = iCurEdge->m_vStart;
			sStartVert.SetLightIntensity(iCurEdge->m_pLight, iCurEdge->m_fStartLight);
			uint32 nStartIndex = pTriangulation->GetVertIndex(sStartVert);
			SLFM_FinalLightingVert &sTriStartVert = pTriangulation->GetVert(nStartIndex);
			bool bStartValid = sStartVert.Equals(sTriStartVert);
			if (bStartValid)
			{
				// Force the light intensity, in case this is actually a vertex from a previous light
				sTriStartVert.SetLightIntensity(iCurEdge->m_pLight, iCurEdge->m_fStartLight);
			}

			SLFM_FinalLightingVert sEndVert;
			sEndVert.m_vPos = iCurEdge->m_vEnd;
			sEndVert.SetLightIntensity(iCurEdge->m_pLight, iCurEdge->m_fEndLight);
			uint32 nEndIndex = pTriangulation->GetVertIndex(sEndVert);
			SLFM_FinalLightingVert &sTriEndVert = pTriangulation->GetVert(nEndIndex);
			bool bEndValid = sEndVert.Equals(sTriEndVert);
			if (bEndValid)
			{
				// Force the light intensity, in case this is actually a vertex from a previous light
				sTriEndVert.SetLightIntensity(iCurEdge->m_pLight, iCurEdge->m_fEndLight);
			}

			// Remember which edges marked singularities, as well as which kind it was
			if (!bEndValid && !bStartValid)
			{
				aWaitingDoubleIndices.push_back(SWaitingEdge(nStartIndex, nEndIndex));
				aWaitingDoubleEdges.push_back(*iCurEdge);
				continue;
			}
			else if (!bStartValid)
			{
				aWaitingStartIndices.push_back(SWaitingEdge(nStartIndex, nEndIndex));
				aWaitingStartEdges.push_back(*iCurEdge);
				continue;
			}
			else if (!bEndValid)
			{
				aWaitingEndIndices.push_back(SWaitingEdge(nStartIndex, nEndIndex));
				aWaitingEndEdges.push_back(*iCurEdge);
				continue;
			}

			// Find out if we've already got this edge inserted
			uint32 nEdgeIndex;
			if (!pTriangulation->FindEdge(nStartIndex, nEndIndex, &nEdgeIndex))
			{
				// Insert the edge
				if (!pTriangulation->InsertHardEdge(nStartIndex, nEndIndex))
					aWaitingReInsertEdges.push_back(SWaitingEdge(nStartIndex, nEndIndex));
			}
			else
			{
				// Looks like we already inserted an edge for this one due to inserting a vertex...
				SLFM_Triangulation<SLFM_FinalLightingVert>::SEdge &sEdge = pTriangulation->m_aEdges[nEdgeIndex];
				sEdge.m_bHardEdge = true;
				// Make sure they're in the right order
				if (sEdge.m_nStart != nStartIndex)
					std::swap(sEdge.m_nStart, sEdge.m_nEnd);
			}
		}

		ASSERT(aWaitingReInsertEdges.empty());

		TIndexList aSinglePeelEdges;
		TIndexList aSinglePeelOldVertices;
		TIndexList aSinglePeelNewVertices;

		TWaitingEdgeList::iterator iCurWaitingEdgeIndices = aWaitingDoubleIndices.begin();
		TShadowEdgeList::iterator iCurWaitingEdge = aWaitingDoubleEdges.begin();
		for (; iCurWaitingEdge != aWaitingDoubleEdges.end(); ++iCurWaitingEdge, ++iCurWaitingEdgeIndices)
		{
			Triangulate_Peel_Double(*iCurWaitingEdge, *iCurWaitingEdgeIndices, &aSinglePeelEdges, &aSinglePeelOldVertices, &aSinglePeelNewVertices, pTriangulation);
		}

		TIndexList::iterator iCurSinglePeelEdge = aSinglePeelEdges.begin();
		TIndexList::iterator iCurSinglePeelOldVert = aSinglePeelOldVertices.begin();
		TIndexList::iterator iCurSinglePeelNewVert = aSinglePeelNewVertices.begin();
		for (; iCurSinglePeelEdge != aSinglePeelEdges.end(); ++iCurSinglePeelEdge, ++iCurSinglePeelOldVert, ++iCurSinglePeelNewVert)
		{
			Triangulate_Peel_Single(*iCurSinglePeelEdge, *iCurSinglePeelOldVert, *iCurSinglePeelNewVert, pTriangulation);
		}
	}

	// Optimize the final triangulation
	// NYI
}

void CLightFragmentMaker::WriteTriangulation(const SLFM_Triangulation<SLFM_FinalLightingVert> &cTriangulation, CPrePoly *pPoly)
{
	CPrePolyFragments *pFragments = new CPrePolyFragments;
	delete pPoly->m_pFragments;
	pPoly->m_pFragments = pFragments;

	// Add the vertices
	pFragments->m_aVertices.reserve(cTriangulation.m_aVertices.size());
	SLFM_Triangulation<SLFM_FinalLightingVert>::TVertList::const_iterator iCurVert = cTriangulation.m_aVertices.begin();
	for (; iCurVert != cTriangulation.m_aVertices.end(); ++iCurVert)
		pFragments->m_aVertices.push_back(CPrePolyFragments::SVert(iCurVert->m_vPos, iCurVert->m_vNormal, iCurVert->m_vColor, 255.0f));

	// Add the triangles
	pFragments->m_aIndices.reserve(cTriangulation.m_aTriangles.size() * 3);
	SLFM_Triangulation<SLFM_FinalLightingVert>::TTriangleList::const_iterator iCurTri = cTriangulation.m_aTriangles.begin();
	for (; iCurTri != cTriangulation.m_aTriangles.end(); ++iCurTri)
	{
		uint32 nIndex0, nIndex1, nIndex2;
		cTriangulation.GetTriVerts(*iCurTri, &nIndex0, &nIndex1, &nIndex2);
		if (!cTriangulation.IsFrontFacing(nIndex0, nIndex1, nIndex2, pPoly->Normal()))
			std::swap(nIndex1, nIndex2);
		pFragments->m_aIndices.push_back(nIndex0);
		pFragments->m_aIndices.push_back(nIndex1);
		pFragments->m_aIndices.push_back(nIndex2);
	}
}
