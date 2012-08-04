#include "bdefs.h"
#include "ShadowCalc.h"
#include "editregion.h"
#include "editpoly.h"
#include "LMLight.h"

CShadowCalc::CShadowCalc() :
	m_pRegion(NULL),
	m_nLightLeakAmount(16)
{
}

CShadowCalc::~CShadowCalc()
{
}

//sets the region that this shadow calculator is associated with
void CShadowCalc::SetRegion(CEditRegion* pRegion)
{
	m_pRegion	= pRegion;
}

inline bool DoesRayHitPoly(CEditPoly* pPoly, const LTVector& vStart, 
						   const LTVector& vDir, float fSegLen, uint32 nLeakAmount)
{
	//find the factors needed to calculate T
	float fDotPerp = vDir.Dot(pPoly->Normal());

	//check for parallel sections
	if(fDotPerp == 0.0f)
		return false;

	float fDotToPt = pPoly->Dist() - vStart.Dot(pPoly->Normal());
	
	//find the actual intersection
	float fT = fDotToPt / fDotPerp;

	//see if we intersect the plane close enough to consider
	if((fT > fSegLen) || (fT < 1.0f))
	{
		//we don't hit the plane soon enough, can't possibly hit the poly
		return false;
	}

	//we hit the plane, now we need to go through all the actual edges and do an
	//in/out test

	//find the point where we hit the plane
	LTVector vPt = vStart + vDir * fT;

	uint32 nPrev = pPoly->NumVerts() - 1;
	for(uint32 nCurr = 0; nCurr < pPoly->NumVerts(); nPrev = nCurr, nCurr++)
	{
		//get the edge normal
		LTVector vEdge = (pPoly->Pt(nCurr) - pPoly->Pt(nPrev)).Cross(pPoly->Normal());

		if(vEdge.Dot(vPt - pPoly->Pt(nCurr)) < 0.01f)
		{
			//outside that point. Not in the poly
			return false;
		}
	}

	//we have hit the polygon, but we may need to let it bleed through a certain amount
	//to prevent dark corners
	if((vPt - vStart).Dot(pPoly->Normal()) < nLeakAmount)
		return false;

	//we are inside the polygon
	return true;

}

inline bool DoesRayHitBrush(CEditBrush* pBrush, const LTVector& vStart, 
						   const LTVector& vDir, float fSegLen, uint32 nLeakAmount)
{
	//bail if this brush doesn't block light
	if(!pBrush->IsFlagSet(BRUSHFLAG_CLIPLIGHT))
		return false;

	//see if we intersect this brush's sphere
	if(!pBrush->m_BoundingSphere.IntersectsSegment(vStart, vDir, fSegLen))
	{
		//we miss the sphere, can't possibly hit this brush.
		return false;
	}

	//we can potentially hit this brush, have to do the poly by poly check.
	for(uint32 nCurrPoly = 0; nCurrPoly < pBrush->m_Polies.GetSize(); nCurrPoly++)
	{
		CEditPoly* pPoly = pBrush->m_Polies[nCurrPoly];

		if(DoesRayHitPoly(pPoly, vStart, vDir, fSegLen, nLeakAmount))
			return true;
	}

	return false;
}

//determines if a segment traveling from the starting point to the ending
//point is intersected by any geometry in the region
bool CShadowCalc::IsSegmentBlocked(const LTVector& vStart, const LTVector& vEnd, CLMLight* pLight)
{
	ASSERT(m_pRegion);

	//convert it into the ray form we need
	LTVector vRayDir = vEnd - vStart;

	//normalize it
	float fMag = vRayDir.Mag();
	vRayDir /= fMag;

	return IsSegmentBlocked(vStart, vRayDir, fMag, pLight);
}

//similar to above, but is given a unit direction along with the length of the
//segment
bool CShadowCalc::IsSegmentBlocked(	const LTVector& vStart, const LTVector& vDir, 
									float fLen, CLMLight* pLight)
{
	//check for coherency
	if(pLight->m_pLastHitBrush)
	{
		if(DoesRayHitBrush(pLight->m_pLastHitBrush, vStart, vDir, fLen, m_nLightLeakAmount))
		{
			//successful hit
			return true;
		}
		else
		{
			//clear the coherency
			pLight->m_pLastHitBrush = NULL;
		}
	}
	

	//now run through all the brushes
	for(LPOS pos = m_pRegion->m_Brushes; pos;)
	{
		CEditBrush* pBrush = m_pRegion->m_Brushes.GetNext(pos);

		if(DoesRayHitBrush(pBrush, vStart, vDir, fLen, m_nLightLeakAmount))
		{
			pLight->m_pLastHitBrush = pBrush;
			return true;
		}
	}

	//no intersection
	return false;
}

//specify the amount of light that is allowed to leak through walls.
void CShadowCalc::SetLightLeakAmount(uint32 nAmount)
{
	m_nLightLeakAmount = nAmount;
}

//retreives the amount of light that is allowed to leak through walls.
uint32 CShadowCalc::GetLightLeakAmount() const
{
	return m_nLightLeakAmount;
}

