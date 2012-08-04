//////////////////////////////////////////////////////////////////////
// RVTrackerVertMove.h - Header for the vertex movement tracker 

#ifndef __RVTRACKERVERTMOVE_H__
#define __RVTRACKERVERTMOVE_H__

#include "rvtracker.h"

class CRVTrackerVertMove : public CRegionViewTracker
{
public:
	typedef enum {esm_Poly, esm_SelVert, esm_ImmVert} ESelectMode;

	CRVTrackerVertMove(LPCTSTR pName = "", CRegionView *pView = NULL, ESelectMode eSelMode = esm_SelVert);
	virtual ~CRVTrackerVertMove();

	ESelectMode m_eSelectMode;

	CVertRefArray m_cMovingVerts;

	CVector m_vStartVec;

	// Tracking for the snapping mode
	CRegionViewTracker* m_pSnapTracker;
	BOOL m_bSnapState, m_bSnap;
	CVector m_vSnapDist;

	// Track whether or not to use perpendicular mode
	CRegionViewTracker* m_pPerpTracker;
	BOOL m_bPerp;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	void FlushTracker();
	void SetupChildTracker(CRegionViewTracker* pTracker);

protected:
	// Get a vertex position from the given point
	virtual BOOL GetVertexFromPoint(const CPoint &cPt, CVector &vResult) { return m_pView->GetVertexFromPoint(cPt, vResult); };
};

#endif //__RVTRACKERVERTMOVE_H__