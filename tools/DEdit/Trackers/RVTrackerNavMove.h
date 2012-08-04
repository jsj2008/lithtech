//////////////////////////////////////////////////////////////////////
// RVTrackerNavMove.h - Header for the navigation movement tracker 

#ifndef __RVTRACKERNAVMOVE_H__
#define __RVTRACKERNAVMOVE_H__

#include "rvtracker.h"

class CRVTrackerNavMove : public CRegionViewTracker
{
public:
	CRVTrackerNavMove(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerNavMove();

	// Track whether or not the right mouse button is down
	BOOL m_bPerp;
	CRegionViewTracker *m_pPerpTracker;

	// Track whether or not the left mouse button is down
	BOOL m_bFast;
	CRegionViewTracker *m_pFastTracker;

	// Only update on idle processing
	BOOL m_bIdleOnly;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();

	void SetupChildTracker(CRegionViewTracker* pTracker);
};

#endif //__RVTRACKERNAVMOVE_H__