//////////////////////////////////////////////////////////////////////
// RVTrackerFocus.h - Header for the focus tracker 

#ifndef __RVTRACKERFOCUS_H__
#define __RVTRACKERFOCUS_H__

#include "rvtracker.h"

class CRVTrackerFocus : public CRegionViewTracker
{
private:
	BOOL m_bWasActive;

	CRegionViewTracker*		m_pLockTracker;
	
	static BOOL m_bLocked;

	BYTE m_aKeyStates[256];

	// Synchronize the trackers with the current key state
	void ResyncKeyList();

public:
	// Refresh the current key list from windows
	void RefreshKeyList();

	CRVTrackerFocus(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerFocus();

	virtual void SetLocked(BOOL bLocked) { m_bLocked = bLocked; };
	virtual BOOL GetLocked() const { return m_bLocked; };

	virtual BOOL OnStart();
	virtual void OnCancel();

	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual void FlushTracker();
};

#endif //__RVTRACKERFOCUS_H__