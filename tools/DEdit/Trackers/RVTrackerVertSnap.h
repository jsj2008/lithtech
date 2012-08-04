//////////////////////////////////////////////////////////////////////
// RVTrackerVertSnap.h - Header for the snapping vertex movement tracker 

#ifndef __RVTRACKERVERTSNAP_H__
#define __RVTRACKERVERTSNAP_H__

#include "rvtracker.h"
#include "rvtrackervertmove.h"

class CRVTrackerVertSnap : public CRVTrackerVertMove
{
public:
	enum ESnapTarget { est_Edge, est_Vert };
	CRVTrackerVertSnap(LPCTSTR pName = "", CRegionView *pView = NULL, ESelectMode eSelMode = esm_SelVert, ESnapTarget eSnapTarget = est_Vert);

	virtual BOOL OnStart();

protected:
	// Where we're going to snap to
	ESnapTarget m_eSnapTarget;

	// The set of active brushes
	CMoArray<CEditBrush *> m_BrushSet;

	// Fill in the set of brushes
	void FillBrushSet();

	// Override where CRVTrackerVertMove gets its vertices from
	virtual BOOL GetVertexFromPoint(const CPoint &cPt, CVector &vResult);

	// Find the closest vertex in a brush that's not in the vertex list
	//	pBrush = the brush in which to search
	//	vPt = the point on the view plane to compare against
	//  vResult = if the result is true, this will contain the new closest vertex point
	//	fDist = the minimum distance to consider a match.  If the result is true, this will
	//			be filled in with the new distance
	BOOL FindClosestVertInBrush(CEditBrush *pBrush, const CEditRay &cRay, CVector &vResult, float &fDist);
	BOOL FindClosestEdgePtInBrush(CEditBrush *pBrush, const CEditRay &cRay, CVector &vResult, float &fDist);

	// Find the closest point to the ray that's in a brush, restricted to no greater than fMinDist away
	BOOL SnapPointVert(const CEditRay &cRay, CVector &vResult, float fMinDist);
	// SnapPointVert, but on an edge
	BOOL SnapPointEdge(const CEditRay &cRay, CVector &vResult, float fMinDist);
};

#endif // __RVTRACKERVERTSNAP_H__