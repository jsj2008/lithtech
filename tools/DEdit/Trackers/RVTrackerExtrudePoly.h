//////////////////////////////////////////////////////////////////////
// RVTrackerExtrudePoly.h - Header for the extrusion tracker 

#ifndef __RVTRACKEREXTRUDEPOLY_H__
#define __RVTRACKEREXTRUDEPOLY_H__

#include "rvtracker.h"

class CRVTrackerExtrudePoly : public CRegionViewTracker
{
public:
	CRVTrackerExtrudePoly(LPCTSTR pName = "", CRegionView *pView = NULL, bool bNewBrush = false);
	virtual ~CRVTrackerExtrudePoly();

	CVertRefArray m_cMovingVerts;

	CVector m_vScaleRefPt, m_vPerpNormal;

	// Track whether or not the units should be snapped to the grid
	BOOL m_bSnap;
	CRegionViewTracker *m_pSnapTracker;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();

private:
	
	float	m_fTotalMoved;
	bool	m_bNewBrush;

};

#endif //__RVTRACKEREXTRUDEPOLY_H__