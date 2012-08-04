//////////////////////////////////////////////////////////////////////
// RVTrackerMarkerMove.h - Header for the marker placement tracker 

#ifndef __RVTRACKERMARKERMOVE_H__
#define __RVTRACKERMARKERMOVE_H__

#include "rvtracker.h"

class CRVTrackerMarkerMove : public CRegionViewTracker
{
public:
	CRVTrackerMarkerMove(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerMarkerMove() {};

	CVector m_vStartVec;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();
};

#endif //__RVTRACKERMARKERMOVE_H__