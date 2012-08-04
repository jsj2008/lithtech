//////////////////////////////////////////////////////////////////////
// RVTrackerObjectSize.h - Header for the object size tracker 

#ifndef __RVTRACKEROBJECTSIZE_H__
#define __RVTRACKEROBJECTSIZE_H__

#include "rvtracker.h"

class CRVTrackerObjectSize : public CRegionViewTracker
{
public:
	CRVTrackerObjectSize(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerObjectSize() {};

	int m_iCurHandle;
	CVector m_vScaleOrigin;
	CVector m_vStartVal;

	CVector GetHandleCoordinate(int iHandle);
	void SetHandleCoordinate(int iHandle, CVector vCoord);

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();
	virtual void OnCancel();
};

#endif //__RVTRACKEROBJECTSIZE_H__