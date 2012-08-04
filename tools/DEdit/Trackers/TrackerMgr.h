//////////////////////////////////////////////////////////////////////
// TrackersMgr.h - Header for the default UI tracker manager

#ifndef __TRACKERMGR_H__
#define __TRACKERMGR_H__

#include "uitracker.h"

// Tracker manager class
//	Important implementation note: On destruction, this class will delete all CUITracker objects
//		which remain in the list to allow un-owned trackers.
class CUITrackerMgr
{
protected:
	CUITracker *m_pActiveTracker;
	CUITrackerList m_cTrackerList;

	virtual void ClearTrackerList();

public:
	CUITrackerMgr();
	~CUITrackerMgr();

	// Member access functions
	virtual CUITracker *GetActiveTracker() const { return m_pActiveTracker; };
	virtual void SetActiveTracker(CUITracker *pTracker);

	virtual const CUITrackerList &GetTrackerList() const { return m_cTrackerList; };
	virtual void SetTrackerList(CUITrackerList &cOtherList) { ClearTrackerList(); m_cTrackerList.CopyArray(cOtherList); };

	// Add a tracker to the tracker list
	virtual void AddTracker(CUITracker *cTracker);
	// Remove a tracker from the tracker list
	//		Returns: FALSE if the tracker was not found
	virtual BOOL RemoveTracker(CUITracker *cTracker);
	// Finds a tracker in the list by name (case insensitive)
	//		Returns: The pointer to the first tracker by that name or NULL if not found
	virtual CUITracker *FindTracker(LPCTSTR pName) const;

	// Process an input event
	//		Returns: TRUE if a tracker processed the event & is therefore active
	virtual BOOL ProcessEvent(const CUIEvent &cEvent);
	// Cancel all tracking
	virtual void Cancel();

	//called to flush all events that are currently being processed
	virtual void FlushTrackers();

	//sorts the trackers so that those with the most number of keys are processed first
	virtual void SortTrackers();
};

#endif //__TRACKERMGR_H__