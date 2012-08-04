#include "bdefs.h"

#include "world_blocker_data.h"

#include "world_blocker_math.h"

#include <vector>

// Some base-type vectors
typedef std::vector<LTVector> TVectorList;
typedef std::vector<int> TIntList;

struct CBlockerPoly
{
	CBlockerPoly() {}
	CBlockerPoly(const CBlockerPoly &cOther) :
		m_cPlane(cOther.m_cPlane),
		m_aVertices(cOther.m_aVertices) 
	{}
	CBlockerPoly &operator=(const CBlockerPoly &cOther) {
		m_cPlane = cOther.m_cPlane;
		m_aVertices = cOther.m_aVertices;
		return *this;
	}

	LTPlane m_cPlane;
	TVectorList m_aVertices;

	// Precalculated data
	LTVector m_vCenter;
	float m_fRadius;

	void PreCalc();

	float GetPtDistToCircle(const LTVector &vPt) const;
	bool IsPtInside(const LTVector &vPt) const;
};

void CBlockerPoly::PreCalc()
{
	if (m_aVertices.empty())
	{
		m_vCenter.Init();
		m_fRadius = 0.0f;
		return;
	}

	// Calc the bounding box
	LTVector vMin = m_aVertices.front();
	LTVector vMax = m_aVertices.front();

	TVectorList::iterator iCurVert = m_aVertices.begin();
	for (++iCurVert; iCurVert != m_aVertices.end(); ++iCurVert)
	{
		VEC_MIN(vMin, vMin, *iCurVert);
		VEC_MAX(vMax, vMax, *iCurVert);
	}

	// Project the center of the bounding box onto the plane
	m_vCenter = (vMin + vMax) * 0.5f;
	m_vCenter -= m_cPlane.m_Normal * m_cPlane.DistTo(m_vCenter);

	// Calc the radius
	m_fRadius = m_vCenter.Dist(m_aVertices.front());
	iCurVert = m_aVertices.begin();
	for (++iCurVert; iCurVert != m_aVertices.end(); ++iCurVert)
	{
		float fNewRadius = m_vCenter.Dist(*iCurVert);
		m_fRadius = LTMIN(m_fRadius, fNewRadius);
	}
}

float CBlockerPoly::GetPtDistToCircle(const LTVector &vPt) const
{
	LTVector vOffset = vPt - m_vCenter;
	float fResult = vOffset.Mag();
	vOffset /= fResult;
	fResult -= (1.0f - fabsf(vOffset.Dot(m_cPlane.m_Normal))) * m_fRadius;
	return fResult;
}

bool CBlockerPoly::IsPtInside(const LTVector &vPt) const
{
	LTVector vPrevPt = m_aVertices.back();
	TVectorList::const_iterator iCurPt = m_aVertices.begin();
	for (; iCurPt != m_aVertices.end(); ++iCurPt)
	{
		// Get the next edge
		const LTVector &vCurPt = *iCurPt;
		LTVector vEdge = vCurPt - vPrevPt;
		LTVector vEdgeDir = vEdge.Unit();
		LTPlane cEdgePlane(m_cPlane.m_Normal.Cross(vEdgeDir), vCurPt);

		if (cEdgePlane.DistTo(vPt) > 0.001f)
			return false;

		vPrevPt = vCurPt;
	}

	return true;
}

// Snap normals that are very close to axis aligned back on the proper axis
LTPlane ScrubPlane(const LTPlane &cPlane)
{
	LTPlane cResult = cPlane;
	bool bReNorm = false;
	if (fabsf(cResult.m_Normal.x) < 0.0001f)
	{
		cResult.m_Normal.x = 0.0f;
		bReNorm = true;
	}
	if (fabsf(cResult.m_Normal.y) < 0.0001f)
	{
		cResult.m_Normal.y = 0.0f;
		bReNorm = true;
	}
	if (fabsf(cResult.m_Normal.z) < 0.0001f)
	{
		cResult.m_Normal.z = 0.0f;
		bReNorm = true;
	}
	if (bReNorm)
		cResult.m_Normal.Normalize();

	return cResult;
}

ILTStream &operator>>(ILTStream &cStream, CBlockerPoly &cPoly)
{
	cStream >> cPoly.m_cPlane.m_Normal.x;
	cStream >> cPoly.m_cPlane.m_Normal.y;
	cStream >> cPoly.m_cPlane.m_Normal.z;
	cStream >> cPoly.m_cPlane.m_Dist;

	cPoly.m_cPlane = ScrubPlane(cPoly.m_cPlane);

	uint32 nVertCount = 0;
	cStream >> nVertCount;

	cPoly.m_aVertices.resize(nVertCount);

	TVectorList::iterator iCurVert = cPoly.m_aVertices.begin();
	for (; iCurVert != cPoly.m_aVertices.end(); ++iCurVert)
	{
		cStream >> *iCurVert;
	}

	cPoly.PreCalc();

	return cStream;
}

// Swept-sphere class (capsule)
class CSweptSphere
{
public:
	LTVector m_vOrigin, m_vOffset, m_vOffsetDir;
	float m_fRadius;

	// Get the time of intersection for a moving swept sphere (losenge)
	bool GetPlaneIntersect(
		const LTPlane &cPlane, 
		const LTVector &vMoveDir, 
		float fMaxTime, 
		float *pIntersectTime,
		LTVector *pIntersectPt) const;
};

bool CSweptSphere::GetPlaneIntersect(
	const LTPlane &cPlane, 
	const LTVector &vMoveDir, 
	float fMaxTime, 
	float *pIntersectTime, 
	LTVector *pIntersectPt) const
{
	ASSERT(pIntersectTime);
	ASSERT(pIntersectPt);

	// Ignore back-facing planes
	// Note : Very important!  This avoids a divide by 0 at the end
	float fPlaneMoveDot = vMoveDir.Dot(cPlane.m_Normal);
	if (fPlaneMoveDot > -0.0001f)
		return false;

	// Figure out the orientation of the poly's plane in relation to the rep
	float fPlaneOffsetDot = m_vOffsetDir.Dot(cPlane.m_Normal);
	bool bTop = fPlaneOffsetDot < 0.0f;
	// Figure out how the "closest" origin relates to the plane
	LTVector vPlaneTestOrigin = (bTop ? m_vOrigin : m_vOrigin + m_vOffset);
	LTVector vPlaneTestNonOrigin = (bTop ? m_vOrigin + m_vOffset : m_vOrigin);
	float fOriginDist = cPlane.DistTo(vPlaneTestOrigin);
	float fNonOriginDist = cPlane.DistTo(vPlaneTestNonOrigin);
	// Check for the plane intersecting the swept sphere
	if ((fOriginDist < m_fRadius) || (fNonOriginDist < m_fRadius))
	{
		// If both points are entirely behind the plane, we're not going to intersect
		if ((fOriginDist < -m_fRadius) && (fOriginDist < -m_fRadius))
			return false;
		ASSERT(fOriginDist >= fNonOriginDist);
		// If the origin is touching the plane, that's the intersection point
		if (fabsf(fOriginDist) <= (m_fRadius + 0.001f))
		{
			*pIntersectPt = vPlaneTestOrigin - cPlane.m_Normal * fOriginDist;
			*pIntersectTime = 0.0f;
			return true;
		}

		// Get the intersection time of the sweep
		ASSERT(fOriginDist > m_fRadius);
		ASSERT(fNonOriginDist < m_fRadius);
		ASSERT(fabsf(fPlaneOffsetDot) > 0.001f);
		float fSweepIntersect = (fOriginDist - m_fRadius) / -fPlaneOffsetDot;
		ASSERT(fSweepIntersect < m_vOffset.Dot(m_vOffsetDir));
		LTVector vSweepDir = (bTop ? m_vOffsetDir : -m_vOffsetDir);
		*pIntersectPt = vPlaneTestOrigin + vSweepDir * fSweepIntersect;
		*pIntersectTime = 0.0f;
		return true;
	}

	// Get the movement intersection time of the vPlaneTestOrigin
	float fPlaneIntersectTime = (fOriginDist - m_fRadius) / -fPlaneMoveDot;
	if (fPlaneIntersectTime >= fMaxTime)
		return false;

	// Calc the intersection point
	*pIntersectTime = fPlaneIntersectTime;
	*pIntersectPt = vPlaneTestOrigin + vMoveDir * *pIntersectTime - (cPlane.m_Normal * m_fRadius);

	// Yup, you intersected
	return true;
}

class CWorldBlockerData : public IWorldBlockerData
{
public:
    declare_interface(CWorldBlockerData);

	CWorldBlockerData();
	virtual ~CWorldBlockerData();

	virtual void Term();
	
    virtual ELoadWorldStatus Load(ILTStream *pStream);

	virtual bool CollidePlayer(
		const LTVector &vStartPt, // Starting point
		const LTVector &vEndPt, // Ending point
		const LTVector &vDims, // Dims of the player
		const TNormalList *pRestrictNormals, // Optional restriction normal list
		LTVector *pNewEndPt // New ending point result
		);

	virtual bool Intersect(
		const LTVector &vStartPt, // Starting point
		const LTVector &vEndPt, // Ending point
		const LTVector &vDims, // Dims of the player
		float *pTime, // Time of first intersection
		LTVector *pNormal // Normal of first intersection
		);

private:
	typedef std::vector<CBlockerPoly> TBlockerPolyList;
	TBlockerPolyList m_aPolys;

	bool GetPolysInSphere(const LTVector &vCenter, float fRadius, TIntList *pResults);

	bool CalcPolyIntersectTime(
		const CBlockerPoly &cPoly, // The poly we're testing
		const CSweptSphere &cPlayerRep, // The player representation we're testing
		const LTVector &vMoveDir, // Moving direction (unit)
		float fMaxTime, // Maximum time of intersection,
		float *pResultTime, // First intersection point,
		LTVector *pIntersectPlane // Plane of intersection
	);

	LTVector GetSeperatingAxis(
		const SBlockerTri &cAxisTri,	// The triangle we intersected with
		const SBlockerTri &cCurMove,	// The movement parallelogram, adjusted back 
										// to an approximation of the intersection point
		uint32 nEdgeMask);				// The mask determining which edges are on the 
										// outside of the polygon.  Bit0 = m_vEdge0,
										// Bit1 = m_vEdge1, Bit2 = m_vEdge1 - m_vEdge0
};

define_interface(CWorldBlockerData, IWorldBlockerData);

//////////////////////////////////////////////////////////////////////////////
// CWorldBlockerData - Blocking data implementation

CWorldBlockerData::CWorldBlockerData()
{
}

CWorldBlockerData::~CWorldBlockerData()
{
	Term();
}

void CWorldBlockerData::Term()
{
	m_aPolys.clear();
}

ELoadWorldStatus CWorldBlockerData::Load(ILTStream *pStream)
{
	uint32 nNumPolys;
	*pStream >> nNumPolys;

	LT_MEM_TRACK_ALLOC(m_aPolys.resize(nNumPolys), LT_MEM_TYPE_WORLD);

	TBlockerPolyList::iterator iCurPoly = m_aPolys.begin();
	for (; iCurPoly != m_aPolys.end(); ++iCurPoly)
	{
		*pStream >> *iCurPoly;
	}

	// Dummy field for future expansion
	uint32 nDummy;
	*pStream >> nDummy;
	ASSERT(nDummy == 0);

	return LoadWorld_Ok;
}

// Get the polys touching a sphere
bool CWorldBlockerData::GetPolysInSphere(const LTVector &vCenter, float fRadius, TIntList *pResults)
{
	ASSERT(pResults);

	uint nOldSize = pResults->size();

	TBlockerPolyList::iterator iCurPoly = m_aPolys.begin();
	for (int nIndex = 0; iCurPoly != m_aPolys.end(); ++iCurPoly, ++nIndex)
	{
		float fPolyDist = (*iCurPoly).GetPtDistToCircle(vCenter);
		if (fPolyDist <= fRadius)
			pResults->push_back(nIndex);
	}

	return nOldSize != pResults->size();
}

// Calculating the seperating axis between a triangle and a completed, adjusted movement parallelogram
LTVector CWorldBlockerData::GetSeperatingAxis(const SBlockerTri &cAxisTri, const SBlockerTri &cCurMove, uint32 nEdgeMask)
{
	ASSERT(nEdgeMask);

	//*
	LTVector vSegStart = cCurMove.m_vOrigin + cCurMove.m_vEdge0;
	LTVector vSegEnd = vSegStart + cCurMove.m_vEdge1;
	LTVector vTriPlaneNormal = cAxisTri.m_vEdge1.Cross(cAxisTri.m_vEdge0);
	vTriPlaneNormal.Normalize();
	LTPlane cTriPlane(vTriPlaneNormal, cAxisTri.m_vOrigin);

	float fStartDist = cTriPlane.DistTo(vSegStart);
	float fEndDist = cTriPlane.DistTo(vSegEnd);

	float fStartU, fStartV;
	GetBarycentricCoords(vSegStart, cAxisTri, &fStartU, &fStartV);
	float fEndU, fEndV;
	GetBarycentricCoords(vSegEnd, cAxisTri, &fEndU, &fEndV);

	const float k_fInsideEpsilon = 0.1f;
	bool bStartInside = ((fStartU > -k_fInsideEpsilon) && (fStartV > -k_fInsideEpsilon) && ((fStartU + fStartV) < (1.0f + k_fInsideEpsilon)));
	bool bEndInside = ((fEndU > -k_fInsideEpsilon) && (fEndV > -k_fInsideEpsilon) && ((fEndU + fEndV) < (1.0f + k_fInsideEpsilon)));

	// Handle the case where both points are on the same side of the triangle's plane,
	// And the closest point is inside the triangle
	if ((bStartInside || bEndInside) && ((fStartDist * fEndDist) > 0.0f))
	{
		fStartDist = fabsf(fStartDist);
		fEndDist = fabsf(fEndDist);
		if ((fStartDist <= fEndDist) && (bStartInside))
		{
			return cTriPlane.m_Normal;
		}
		else if ((fEndDist <= fStartDist) && (bEndInside))
		{
			return cTriPlane.m_Normal;
		}
	}

	// Find the closest edge
	SBlockerSeg sLeadingEdge;
	sLeadingEdge.m_vOrigin = cCurMove.m_vOrigin + cCurMove.m_vEdge0;
	sLeadingEdge.m_vDirection = cCurMove.m_vEdge1;

	float fClosestDistSqr, fTestDistSqr;
	LTVector aClosestPts[2];
	SBlockerSeg sTriEdge;

	float fSegP, fTriP;

	fClosestDistSqr = FLT_MAX;
	uint32 nClosestEdge = 0;

	sTriEdge.m_vOrigin = cAxisTri.m_vOrigin;
	sTriEdge.m_vDirection = cAxisTri.m_vEdge0;

	fTestDistSqr = DistSqrSegSeg(sLeadingEdge, sTriEdge, &fSegP, &fTriP);
	if (fTestDistSqr < fClosestDistSqr)
	{
		fClosestDistSqr = fTestDistSqr;
		aClosestPts[0] = sLeadingEdge.m_vOrigin + sLeadingEdge.m_vDirection * fSegP;
		aClosestPts[1] = sTriEdge.m_vOrigin + sLeadingEdge.m_vDirection * fTriP;
		if ((aClosestPts[0] - aClosestPts[1]).MagSqr() > (fClosestDistSqr + 0.001f))
		{
			LTVector vOfs = aClosestPts[1] - sLeadingEdge.m_vOrigin;
			float fOfsDotDir = sLeadingEdge.m_vDirection.Dot(vOfs) / sLeadingEdge.m_vDirection.MagSqr();
			LTCLAMP(fOfsDotDir, 0.0f, 1.0f);
			aClosestPts[0] = sLeadingEdge.m_vOrigin + sLeadingEdge.m_vDirection * fOfsDotDir;
		}
		nClosestEdge = 1;
	}

	sTriEdge.m_vOrigin = cAxisTri.m_vOrigin;
	sTriEdge.m_vDirection = cAxisTri.m_vEdge1;

	fTestDistSqr = DistSqrSegSeg(sLeadingEdge, sTriEdge, &fSegP, &fTriP);
	if (fTestDistSqr < fClosestDistSqr)
	{
		fClosestDistSqr = fTestDistSqr;
		aClosestPts[0] = sLeadingEdge.m_vOrigin + sLeadingEdge.m_vDirection * fSegP;
		aClosestPts[1] = sTriEdge.m_vOrigin + sLeadingEdge.m_vDirection * fTriP;
		if ((aClosestPts[0] - aClosestPts[1]).MagSqr() > (fClosestDistSqr + 0.001f))
		{
			LTVector vOfs = aClosestPts[1] - sLeadingEdge.m_vOrigin;
			float fOfsDotDir = sLeadingEdge.m_vDirection.Dot(vOfs) / sLeadingEdge.m_vDirection.MagSqr();
			LTCLAMP(fOfsDotDir, 0.0f, 1.0f);
			aClosestPts[0] = sLeadingEdge.m_vOrigin + sLeadingEdge.m_vDirection * fOfsDotDir;
		}
		nClosestEdge = 2;
	}

	sTriEdge.m_vOrigin = cAxisTri.m_vOrigin + cAxisTri.m_vEdge0;
	sTriEdge.m_vDirection = cAxisTri.m_vEdge1 - cAxisTri.m_vEdge0;

	fTestDistSqr = DistSqrSegSeg(sLeadingEdge, sTriEdge, &fSegP, &fTriP);
	if (fTestDistSqr < fClosestDistSqr)
	{
		fClosestDistSqr = fTestDistSqr;
		aClosestPts[0] = sLeadingEdge.m_vOrigin + sLeadingEdge.m_vDirection * fSegP;
		aClosestPts[1] = sTriEdge.m_vOrigin + sLeadingEdge.m_vDirection * fTriP;
		if ((aClosestPts[0] - aClosestPts[1]).MagSqr() > (fClosestDistSqr + 0.001f))
		{
			LTVector vOfs = aClosestPts[1] - sLeadingEdge.m_vOrigin;
			float fOfsDotDir = sLeadingEdge.m_vDirection.Dot(vOfs) / sLeadingEdge.m_vDirection.MagSqr();
			LTCLAMP(fOfsDotDir, 0.0f, 1.0f);
			aClosestPts[0] = sLeadingEdge.m_vOrigin + sLeadingEdge.m_vDirection * fOfsDotDir;
		}
		nClosestEdge = 4;
	}

	// Is this an outside edge?
	if ((nEdgeMask & nClosestEdge) != 0)
	{
		// Return the difference
		LTVector vResult = aClosestPts[0] - aClosestPts[1];
		// Force a perpendicular result if things are wonky
		float fResultDotEdge = vResult.Dot(sLeadingEdge.m_vDirection);
		if (((fSegP > 0.001f) && (fSegP < 0.99f)) ||
			((fSegP < 0.99f) && (fResultDotEdge < -0.01f)) ||
			((fSegP > 0.001f) && (fResultDotEdge > 0.01f)))
		{
			vResult -= sLeadingEdge.m_vDirection * (fResultDotEdge / sLeadingEdge.m_vDirection.Mag());
		}
		vResult.Normalize();

		return vResult;
	}
	else
	{
		// Use the plane normal
		return cTriPlane.m_Normal;
	}

	//*/

	// This solution would be great if it actually worked...
	/*
	SBlockerSeg sLeadingEdge;
	sLeadingEdge.m_vOrigin = cCurMove.m_vOrigin + cCurMove.m_vEdge0;
	sLeadingEdge.m_vDirection = cCurMove.m_vEdge1;

	float fSegP, fTriP0, fTriP1;
	float fDistToTri = DistSegTri(sLeadingEdge, cAxisTri, &fSegP, &fTriP0, &fTriP1);

	LTVector vPtOnTri = cAxisTri.m_vOrigin + cAxisTri.m_vEdge0 * fTriP0 + cAxisTri.m_vEdge1 * fTriP1;
	LTVector vPtOnSeg = sLeadingEdge.m_vOrigin + sLeadingEdge.m_vDirection * fSegP;
	LTVector vResult = vPtOnSeg - vPtOnTri;
	// Force a perpendicular result if things are wonky
	float fResultDotEdge = vResult.Dot(sLeadingEdge.m_vDirection);
	if (((fSegP > 0.001f) && (fSegP < 0.99f)) ||
		((fSegP < 0.99f) && (fResultDotEdge < 0.0f)) ||
		((fSegP > 0.001f) && (fResultDotEdge > 0.0f)))
	{
		float fResultMag = vResult.Mag();
		float fEdgeMag = sLeadingEdge.m_vDirection.Mag();
		vResult -= sLeadingEdge.m_vDirection * (fResultDotEdge / (fResultMag * fEdgeMag));
	}
	vResult.Normalize();

	return vResult;

	//*/
}

// Get a poly intersection time
bool CWorldBlockerData::CalcPolyIntersectTime(
	const CBlockerPoly &cPoly,  
	const CSweptSphere &cPlayerRep,  
	const LTVector &vMoveDir,  
	float fMaxTime,  
	float *pResultTime,  
	LTVector *pIntersectPlane)
{
	ASSERT(pResultTime);
	ASSERT(pIntersectPlane);

	// Ignore back-facing polys
	if (vMoveDir.Dot(cPoly.m_cPlane.m_Normal) > 0.0f)
		return false;

	// Get the poly plane intersection time
	float fPolyPlaneIntersectTime;
	LTVector vPolyPlaneIntersectPt;
	if (!cPlayerRep.GetPlaneIntersect(
		cPoly.m_cPlane, 
		vMoveDir, 
		fMaxTime, 
		&fPolyPlaneIntersectTime,
		&vPolyPlaneIntersectPt))
		return false;

	bool bResult = false;
	float fClosestIntersect = fMaxTime;
	LTVector vClosestNormal;
	uint32 nClosestTri = 2;

	SBlockerTri cCurMove;
	cCurMove.m_vOrigin = cPlayerRep.m_vOrigin;
	cCurMove.m_vEdge0 = vMoveDir * fClosestIntersect;
	cCurMove.m_vEdge1 = cPlayerRep.m_vOffset;

	// Look for an intersecting triangle
	LTVector vCenterPt = cPoly.m_aVertices[0];
	LTVector vPrevPt = cPoly.m_aVertices[1];
	TVectorList::const_iterator iCurPt = cPoly.m_aVertices.begin() + 2;
	uint32 nTriIndex = 2;
	for (; (iCurPt != cPoly.m_aVertices.end()) && (fClosestIntersect > 0.0001f); ++iCurPt)
	{
		LTVector vCurPt = *iCurPt;
		SBlockerTri cCurTri;
		cCurTri.m_vOrigin = vCenterPt;
		cCurTri.m_vEdge0 = vPrevPt - vCenterPt;
		cCurTri.m_vEdge1 = vCurPt - vCenterPt;

		uint32 nTriMask = 4;
		if (iCurPt == (cPoly.m_aVertices.begin() + 2))
			nTriMask |= 1;
		if ((iCurPt + 1) == (cPoly.m_aVertices.end()))
			nTriMask |= 2;

		// Note: The retries are because the seperating axis handling can end up
		// with an intersection time that's not the full radius away
		const uint32 k_nNumRetries = 10;
		uint32 nNumRetries = k_nNumRetries;
		float fTriDist;
		do
		{
			--nNumRetries;

			float fTriP0, fTriP1, fPgmP0, fPgmP1;
			fTriDist = DistTriPgm(cCurTri, cCurMove, &fTriP0, &fTriP1, &fPgmP0, &fPgmP1);
			if (fTriDist < (cPlayerRep.m_fRadius - 0.01f))
			{
				// We found an intersection
				// Figure out when it happened
				float fIntersectTime = fPgmP0 * fClosestIntersect;
				SBlockerTri sTempMove;
				sTempMove.m_vOrigin = cCurMove.m_vOrigin;
				// Figure out a testing time that should move us to a partial intersection instead of a complete one
				float fTestTime;
				// If we've exhausted our retry count, go back to time 0
				if (!nNumRetries)
					fTestTime = 0.0f;
				else if ((fPgmP0 < 1.0f) || (fTriDist < 0.01f))
				{
					fTestTime = fIntersectTime - (cPlayerRep.m_fRadius - fTriDist);
					fTestTime = LTMAX(fTestTime, 0.0f);
				}
				else
					fTestTime = 1.0f;
				sTempMove.m_vEdge0 = vMoveDir * fTestTime;
				sTempMove.m_vEdge1 = cCurMove.m_vEdge1;
				LTVector vSepAxis = GetSeperatingAxis(cCurTri, sTempMove, nTriMask);
				float fMoveDotAxis = vSepAxis.Dot(vMoveDir);
				if (!nNumRetries)
					fIntersectTime = 0.0f;
				else if (fMoveDotAxis < -0.001f)
				{
					// Note: This is not always accurate.  The edges of the polygons
					// often make this come up with a position that's < the radius away from the polygon
					fIntersectTime -= (cPlayerRep.m_fRadius - fTriDist) / -fMoveDotAxis;
					fIntersectTime = LTMAX(0.0f, fIntersectTime);
				}
				else 
					break;
				if (fIntersectTime < fClosestIntersect)
				{
					// Remember the information
					bResult = true;
					fClosestIntersect = fIntersectTime;
					nClosestTri = nTriIndex;
					vClosestNormal = vSepAxis;
					// Re-adjust the movement so it doesn't intersect any more
					cCurMove.m_vEdge0 = vMoveDir * fClosestIntersect;
				}
				else
					break;
			}
			else
				break;
		} while ((fClosestIntersect > 0.0f) && (nNumRetries));

		++nTriIndex;
		vPrevPt = vCurPt;
	}

	if (bResult)
	{
		*pResultTime = fClosestIntersect;
		LTPlane cTempPlane = ScrubPlane(LTPlane(vClosestNormal, 0.0f));
		*pIntersectPlane = cTempPlane.m_Normal;
	}

	return bResult;
}

// Main collision routine, based heavily on CollideCylinderWithTree in collision.cpp
bool CWorldBlockerData::CollidePlayer(
	const LTVector &vStartPt,  
	const LTVector &vEndPt,  
	const LTVector &vDims,  
	const TNormalList *pRestrictNormals,
	LTVector *pNewEndPt)
{
	if (!pNewEndPt)
		return false;

	// Get our movement information
	LTVector vMoveOffset = vEndPt - vStartPt;
	float fMoveMag = vMoveOffset.Mag();
	if (fMoveMag < 0.0001f)
		return false;

	// Query a gross estimation of polys that the player "might" touch
	static TIntList aPolySet;
	aPolySet.clear();
	LTVector vMidPt = (vStartPt + vEndPt) * 0.5f;
	float fSphereRadius = fMoveMag * 0.5f + vDims.Mag();
	if (!GetPolysInSphere(vMidPt, fSphereRadius, &aPolySet))
		return false;

	static TVectorList aRestrictDir;
	aRestrictDir.clear();
	aRestrictDir.reserve(5);

	const float k_fConflictingPlaneEpsilon = -0.001f;
	const float k_fBadPlaneEpsilon = 0.001f;
	
	if (pRestrictNormals)
	{
		// Make sure there aren't any conflicting normals in the normal list
		TVectorList::const_iterator iCurInputNormal = pRestrictNormals->begin();
		for (; iCurInputNormal != pRestrictNormals->end(); ++iCurInputNormal)
		{
			LTVector vCurNormal = *iCurInputNormal;
			bool bBadNormal = false;
			TVectorList::const_iterator iCurTestNormal = aRestrictDir.begin();
			for (; iCurTestNormal != aRestrictDir.end(); ++iCurTestNormal)
			{
				const LTVector &vRestrictNormal = *iCurTestNormal;
				float fRestrictDot = vRestrictNormal.Dot(vCurNormal);
				if (fRestrictDot < k_fConflictingPlaneEpsilon)
				{
					vCurNormal -= fRestrictDot * *iCurTestNormal;
					float fNormalMag = vCurNormal.Mag();
					if (fNormalMag < k_fBadPlaneEpsilon)
					{
						bBadNormal = true;
						break;
					}
					vCurNormal /= fNormalMag;
				}
			}
			if (!bBadNormal)
				aRestrictDir.push_back(vCurNormal);
		}
	}

	bool bResult = false;

	bool bIntersect;
	LTVector vCurStartPt = vStartPt;
	LTVector vMoveDir = vMoveOffset / fMoveMag;

	CSweptSphere cPlayerRep;
	// Make sure the radius encloses the entire dims of the player
	//cPlayerRep.m_fRadius = sqrtf(vDims.x * vDims.x + vDims.z * vDims.z) + 0.01f;
	cPlayerRep.m_fRadius = LTMAX(vDims.x, vDims.z);
	float fHalfVerticalOfs = vDims.y - cPlayerRep.m_fRadius;
	fHalfVerticalOfs = LTMAX(fHalfVerticalOfs, 0.0f);
	cPlayerRep.m_vOffset.Init(0.0f, fHalfVerticalOfs * 2, 0.0f);
	cPlayerRep.m_vOffsetDir.Init(0.0f, 1.0f, 0.0f);
	LTVector vRepAdj(0.0f, -fHalfVerticalOfs, 0.0f);
	cPlayerRep.m_vOrigin = vStartPt + vRepAdj;

	// An arbitrary number of maximum iterations
	uint32 nMaxIterations = 5;

	do
	{
		// Calculate the poly intersection time & plane for the polys
		bIntersect = false;
		float fEarliestTime = fMoveMag;
		LTVector vEarliestPlane;
		TIntList::iterator iEarliestIndex = aPolySet.begin();
		TIntList::iterator iCurPoly = aPolySet.begin();
		for (uint nCurIndex = 0; iCurPoly != aPolySet.end(); ++iCurPoly, ++nCurIndex)
		{
			if (*iCurPoly >= (int)m_aPolys.size())
				continue;
			CBlockerPoly &cCurPoly = m_aPolys[*iCurPoly];
			float fIntersectTime;
			LTVector vIntersectPlane;
			if (CalcPolyIntersectTime(cCurPoly, cPlayerRep, vMoveDir, fEarliestTime, &fIntersectTime, &vIntersectPlane))
			{
				bIntersect = true;
				fEarliestTime = fIntersectTime - 0.001f;
				vEarliestPlane = vIntersectPlane;
				iEarliestIndex = iCurPoly;
				if (fEarliestTime <= 0.0f)
				{
					fEarliestTime = 0.0f;
					break;
				}
			}
		}

		if (bIntersect)
		{
			bResult = true;

			// Move to the first intersection time
			vCurStartPt = vCurStartPt + vMoveDir * fEarliestTime;
			cPlayerRep.m_vOrigin = vCurStartPt + vRepAdj;

			// Add a restriction plane for the intersection
			TVectorList::iterator iCurPlane = aRestrictDir.begin();
			bool bBadPlane = false;
			for (; iCurPlane != aRestrictDir.end(); ++iCurPlane)
			{
				LTVector &vCurPlane = *iCurPlane;
				float fRestrictDot = vCurPlane.Dot(vEarliestPlane);
				if (fRestrictDot < k_fConflictingPlaneEpsilon)
				{
					vEarliestPlane -= fRestrictDot * vCurPlane;
					float fPlaneMag = vEarliestPlane.Mag();
					if (fPlaneMag < k_fBadPlaneEpsilon)
					{
						bBadPlane = true;
						break;
					}
					vEarliestPlane /= fPlaneMag;
				}
			}
			if (!bBadPlane)
				aRestrictDir.push_back(vEarliestPlane);

			// Calculate a new movement step based on the restriction planes
			vMoveOffset = vMoveDir * (fMoveMag - fEarliestTime);
			iCurPlane = aRestrictDir.begin();
			for (; iCurPlane != aRestrictDir.end(); ++iCurPlane)
			{
				// Check this plane
				LTVector &vCurPlane = *iCurPlane;
				float fDot = vCurPlane.Dot(vMoveOffset);
				if (fDot >= -0.001f)
					continue;
				// If we're moving into the plane, project it back onto the plane
				vMoveOffset -= vCurPlane * fDot;
				// Jump out of the loop if we're not moving
				if (vMoveOffset.MagSqr() < 0.0001f)
				{
					vMoveOffset.Init();
					break;
				}
			}

			// Remember where we're going to end up in case we don't intersect
			*pNewEndPt = vCurStartPt + vMoveOffset;

			// Recalc our movement parameters
			fMoveMag = vMoveOffset.Mag();
			if (fMoveMag < 0.0001f)
				break;
			vMoveDir = vMoveOffset / fMoveMag;

			// Remove the intersected poly from the set
			*iEarliestIndex = m_aPolys.size();
		}
		--nMaxIterations;
	} while (bIntersect && nMaxIterations);

	// Couldn't resolve the collision testing to a happy place...
	if (bIntersect)
	{
		*pNewEndPt = vStartPt;
		return true;
	}

	return bResult;
}

bool CWorldBlockerData::Intersect(
		const LTVector &vStartPt,
		const LTVector &vEndPt,
		const LTVector &vDims,
		float *pTime,
		LTVector *pNormal
		)
{
	// Get our movement information
	LTVector vMoveOffset = vEndPt - vStartPt;
	float fMoveMag = vMoveOffset.Mag();
	if (fMoveMag < 0.0001f)
		return false;

	// Query a gross estimation of polys that the player "might" touch
	static TIntList aPolySet;
	aPolySet.clear();
	LTVector vMidPt = (vStartPt + vEndPt) * 0.5f;
	float fSphereRadius = fMoveMag * 0.5f + vDims.Mag();
	if (!GetPolysInSphere(vMidPt, fSphereRadius, &aPolySet))
		return false;

	bool bResult = false;

	LTVector vMoveDir = vMoveOffset / fMoveMag;

	CSweptSphere cPlayerRep;
	// Make sure the radius encloses the entire dims of the player
	cPlayerRep.m_fRadius = LTMAX(vDims.x, vDims.z);
	float fHalfVerticalOfs = vDims.y - cPlayerRep.m_fRadius;
	fHalfVerticalOfs = LTMAX(fHalfVerticalOfs, 0.0f);
	cPlayerRep.m_vOffset.Init(0.0f, fHalfVerticalOfs * 2, 0.0f);
	cPlayerRep.m_vOffsetDir.Init(0.0f, 1.0f, 0.0f);
	LTVector vRepAdj(0.0f, -fHalfVerticalOfs, 0.0f);
	cPlayerRep.m_vOrigin = vStartPt + vRepAdj;

	// Find the earliest intersection in the polygons
	float fEarliestTime = fMoveMag;
	LTVector vEarliestPlane;
	TIntList::iterator iCurPoly = aPolySet.begin();
	for (uint nCurIndex = 0; iCurPoly != aPolySet.end(); ++iCurPoly, ++nCurIndex)
	{
		if (*iCurPoly >= (int)m_aPolys.size())
			continue;
		CBlockerPoly &cCurPoly = m_aPolys[*iCurPoly];
		float fIntersectTime;
		LTVector vIntersectPlane;
		if (CalcPolyIntersectTime(cCurPoly, cPlayerRep, vMoveDir, fEarliestTime, &fIntersectTime, &vIntersectPlane))
		{
			bResult = true;
			fEarliestTime = fIntersectTime - 0.001f;
			vEarliestPlane = vIntersectPlane;
			if (fEarliestTime <= 0.0f)
			{
				fEarliestTime = 0.0f;
				break;
			}
		}
	}

	// Return the results
	if (bResult)
	{
		*pTime = fEarliestTime / fMoveMag;
		*pNormal = vEarliestPlane;
	}

	return bResult;
}

