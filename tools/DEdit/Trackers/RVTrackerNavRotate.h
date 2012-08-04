//////////////////////////////////////////////////////////////////////
// RVTrackerNavRotate.h - Header for the navigation movement tracker 

#ifndef __RVTRACKERNAVROTATE_H__
#define __RVTRACKERNAVROTATE_H__

#include "rvtracker.h"

class CRVTrackerNavRotate : public CRegionViewTracker
{
public:
	CRVTrackerNavRotate(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerNavRotate();

	// Track whether or not the right mouse button is down
	BOOL m_bForward;
	CRegionViewTracker* m_pForwardTracker;

	// Track whether or not the left mouse button is down
	BOOL m_bBackward;
	CRegionViewTracker* m_pBackwardTracker;

	// Only update on idle processing
	BOOL m_bIdleOnly;

	// Time accumulated since the last idle
	DWORD m_dwIdleDelta;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();

	void SetupChildTracker(CRegionViewTracker* pTracker);
};

#endif //__RVTRACKERNAVROTATE_H__