//////////////////////////////////////////////////////////////////////
// RVTrackerNodeRotate.h - Header for the node rotation tracker 

#ifndef __RVTRACKERNODEROTATE_H__
#define __RVTRACKERNODEROTATE_H__

#include "rvtracker.h"

class CRVTrackerNodeRotate : public CRegionViewTracker
{
protected:
	CRegionViewTracker *m_pStepTracker;
	BOOL m_bStep;

	int m_iTotalRotate, m_iTotalMove;

	void FlushTracker();

public:
	CRVTrackerNodeRotate(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerNodeRotate();

	CVector	m_vTrackRotationVector, m_vTrackRotationCenter;

	virtual void WatchEvent(const CUIEvent &cEvent);
	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();
};

#endif //__RVTRACKERNODEROTATE_H__