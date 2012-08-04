//////////////////////////////////////////////////////////////////////
// RVTrackerVertScale.h - Header for the vertex scaling tracker 

#ifndef __RVTRACKERVERTSCALE_H__
#define __RVTRACKERVERTSCALE_H__

#include "rvtracker.h"

class CRVTrackerVertScale : public CRegionViewTracker
{
public:
	typedef enum {esm_Poly, esm_SelVert, esm_ImmVert} ESelectMode;

	CRVTrackerVertScale(LPCTSTR pName = "", CRegionView *pView = NULL, ESelectMode eSelMode = esm_Poly);
	virtual ~CRVTrackerVertScale();

	ESelectMode m_eSelectMode;

	CVertRefArray m_cMovingVerts;

	CVector m_vScaleRefPt, m_vPerpNormal;


	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();

private:

	// Track whether or not to use perpendicular mode
	BOOL m_bPerp;
	CRegionViewTracker*		m_pPerpTracker;

};

#endif //__RVTRACKERVERTSCALE_H__