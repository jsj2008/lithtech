//////////////////////////////////////////////////////////////////////
// RVTrackerZoom.h - Header for the mouse wheel zooming tracker 

#ifndef __RVTRACKERZOOM_H__
#define __RVTRACKERZOOM_H__

#include "rvtracker.h"

class CRVTrackerZoom : public CRegionViewTracker
{
public:
	CRVTrackerZoom(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerZoom();

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();

	// Track whether or not the shift key is down
	BOOL				m_bFast;
	CRegionViewTracker	*m_pFastTracker;

	// Only update on idle processing
	BOOL		m_bIdleOnly;
};

#endif //__RVTRACKERNAVMOVE_H__