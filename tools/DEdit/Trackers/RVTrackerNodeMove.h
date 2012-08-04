//////////////////////////////////////////////////////////////////////
// RVTrackerNodeMove.h - Header for the node movement tracker 

#ifndef __RVTRACKERNODEMOVE_H__
#define __RVTRACKERNODEMOVE_H__

#include "rvtracker.h"

class CRVTrackerNodeMove : public CRegionViewTracker
{
public:
	enum EctorFlags { 
		FLAG_NONE = 0,
		FLAG_SNAP = 1, 
		FLAG_HANDLE = 2,
		FLAG_PERP = 4,
		FLAG_CLONE = 8,
	};

	CRVTrackerNodeMove(LPCTSTR pName = "", CRegionView *pView = NULL, int iFlags = FLAG_NONE);
	virtual ~CRVTrackerNodeMove();

	BOOL m_bSnap, m_bHandle, m_bPerp;

	CVector m_vStartVec, m_vTotalMoveOffset, m_vMoveSnapAxis;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void WatchEvent(const CUIEvent &cEvent);

private:

	void FlushTracker();

	void SetupChildTracker(CRegionViewTracker* pTracker);

	CRegionViewTracker*		m_pLockAxisTracker;
	BOOL					m_bLockAxis;

	// (Note : This only has an effect at OnStart)
	CRegionViewTracker*		m_pCloneTracker;
	BOOL					m_bClone;
};

#endif //__RVTRACKERNODEMOVE_H__