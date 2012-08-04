//////////////////////////////////////////////////////////////////////
// RVTrackerGridSize.h - Header for the grid size tracker 

#ifndef __RVTRACKERGRIDSIZE_H__
#define __RVTRACKERGRIDSIZE_H__

#include "rvtracker.h"

class CRVTrackerGridSize : public CRegionViewTracker
{
public:
	CRVTrackerGridSize(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerGridSize() {};

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();
};

#endif //__RVTRACKERGRIDSIZE_H__