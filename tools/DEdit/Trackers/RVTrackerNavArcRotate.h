//////////////////////////////////////////////////////////////////////
// RVTrackerNavArcRotate.h - Header for the navigation arc rotate tracker 

#ifndef __RVTRACKERNAVARCROTATE_H__
#define __RVTRACKERNAVARCROTATE_H__

#include "rvtracker.h"

class CRVTrackerNavArcRotate : public CRegionViewTracker
{
public:
	CRVTrackerNavArcRotate(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerNavArcRotate();

	// Only update on idle processing
	BOOL m_bIdleOnly;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();

private:

	LTVector			m_vOrbitFocus;

	CRegionViewTracker	*m_pRollTracker;
	BOOL				m_bRoll;
	
	HCURSOR				m_hRotateXY;
	HCURSOR				m_hRotateZ;

};

#endif //__RVTRACKERNAVARCROTATE_H__