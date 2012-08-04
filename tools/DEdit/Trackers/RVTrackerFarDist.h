//////////////////////////////////////////////////////////////////////
// RVTrackerFarDist.h - Header for the far clipping distance tracker 

#ifndef __RVTRACKERFARDIST_H__
#define __RVTRACKERFARDIST_H__

#include "rvtracker.h"

class CRVTrackerFarDist : public CRegionViewTracker
{
public:
	CRVTrackerFarDist(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerFarDist() {};

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();
};

#endif //__RVTRACKERFARDIST_H__