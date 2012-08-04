//////////////////////////////////////////////////////////////////////
// RVTrackerCurveEdit.h - Header for the curve edit tracker 

#ifndef __RVTRACKERCURVEEDIT_H__
#define __RVTRACKERCURVEEDIT_H__

#include "rvtracker.h"

class CRVTrackerCurveEdit : public CRegionViewTracker
{
public:
	CRVTrackerCurveEdit(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerCurveEdit() {};

	int m_iCurObject, m_iCurProp;
	// Only update on idle processing
	BOOL m_bIdleOnly;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();
};

#endif //__RVTRACKERCURVEEDIT_H__