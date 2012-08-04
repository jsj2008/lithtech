//////////////////////////////////////////////////////////////////////
// RVTrackerNavOribt.h - Header for the navigation orbiting tracker 

#ifndef __RVTRACKERNAVORBIT_H__
#define __RVTRACKERNAVORBIT_H__

#include "rvtracker.h"

class CRVTrackerNavOrbit : public CRegionViewTracker
{
public:
	CRVTrackerNavOrbit(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerNavOrbit();

	// Track whether or not the right mouse button is down
	BOOL m_bMove;
	CRegionViewTracker* m_pMoveTracker;

	// Only update on idle processing
	BOOL m_bIdleOnly;

	LTVector m_vOrbitFocus;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();

	void SetupChildTracker(CRegionViewTracker* pTracker);
};

#endif //__RVTRACKERNAVORBIT_H__