//////////////////////////////////////////////////////////////////////
// RVTrackerNavDrag.h - Header for the navigation drag tracker 

#ifndef __RVTRACKERNAVDRAG_H__
#define __RVTRACKERNAVDRAG_H__

#include "rvtracker.h"

class CRVTrackerNavDrag : public CRegionViewTracker
{
public:
	CRVTrackerNavDrag(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerNavDrag();

	// Only update on idle processing
	BOOL m_bIdleOnly;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();

	CRegionViewTracker	*m_pFastTracker;
	BOOL				m_bFast;
	HCURSOR				m_hHandCursor;

};

#endif //__RVTRACKERNAVDRAG_H__