//////////////////////////////////////////////////////////////////////
// RVTrackerBrushSize.h - Header for the brush size tracker 

#ifndef __RVTRACKERBRUSHSIZE_H__
#define __RVTRACKERBRUSHSIZE_H__

#include "rvtracker.h"

class CRVTrackerBrushSize : public CRegionViewTracker
{
public:
	CRVTrackerBrushSize(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerBrushSize() {};

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

#endif //__RVTRACKERBRUSHSIZE_H__