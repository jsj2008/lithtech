//////////////////////////////////////////////////////////////////////
// RVTracker.h - Header for the RegionView tracker 

#ifndef __RVTRACKER_H__
#define __RVTRACKER_H__

#include "uitracker.h"
#include "regiondoc.h"
#include "regionview.h"
#include "trackermgr.h"

class CRegionViewTracker : public CUITracker
{
protected:
	// Start the tracker
	BOOL StartTracker();
	// End the tracker
	BOOL EndTracker();
	// Update the tracker
	BOOL Update(const CUIEvent &cEvent);

	// Add an event to a list
	void AddEvent(CUIEventList &cList, const CUIEvent &cEvent, BOOL bActive);
	// Remove an event from a list
	void RemoveEvent(CUIEventList &cList, const CUIEvent &cEvent, BOOL bActive);
	// Find an event in a list
	BOOL IsEvent(const CUIEventList &cList, const CUIEvent &cEvent, DWORD *pIndex = NULL) const;
	// Find an inverse event in a list
	BOOL IsInverseEvent(const CUIEventList &cList, const CUIEvent &cEvent, DWORD *pIndex = NULL) const;
	// Clear an event list
	void ClearEventList(CUIEventList &cList);
	//called to flush out the tracker, usually when focus is lost or the hotkeys
	//have been changed
	virtual void FlushTracker();

	//Called when another event masks this event (this occurs when a user activates
	//a tracker, then another tracker which requires more events)
	virtual void WasMasked()	{ m_cWaitingEventList.RemoveAll(); SetActive(FALSE); };

	CRegionView		*m_pView;

	CPoint			m_cStartPt;
	CPoint			m_cCurPt;
			
	// This is set if m_bAutoCenter is TRUE.
	CPoint			m_cLastPt;

	// Updated as the mouse is moved around (this is basically m_StartPt and m_CurPt).
	CRect			m_cRect, m_cLastRect;
			
	// The last time that the tracker was updated.  This is used for time based trackers.
	DWORD			m_dwLastUpdateTime;

	// The delta time between tracker updates
	DWORD			m_dwTimeDelta;

	// Set this if you want it to automatically recenter the cursor.
	BOOL			m_bAutoCenter;

	// Set this if you want it to automatically hide the cursor
	BOOL			m_bAutoHide;

	// Set this if you want it to automatically capture the input focus
	BOOL			m_bAutoCapture;

	// Whether or not to wait for an extra end if someone else takes over before we should.
	BOOL			m_bPhantomEnd;

	//the amount that the mouse wheel has turned
	int32			m_nWheelDelta;

	// The events which are waiting until the tracker changes states
	CUIEventList m_cWaitingEventList;

	// The override tracker list
	CUITrackerMgr m_cOverrideMgr;

public:
	CRegionViewTracker(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRegionViewTracker();

	// Re-center the cursor manually
	virtual void CenterCursor();

	// Hide/unhide the cursor manually
	virtual void HideCursor();
	virtual void UnHideCursor();

	// Call to scroll the frame view & adjust the tracking positions
	virtual void DoAutoScroll();

	// Waiting list control
	virtual void AddWaitingEvent(const CUIEvent &cEvent) { AddEvent(m_cWaitingEventList, cEvent, GetActive()); };
	virtual void RemoveWaitingEvent(const CUIEvent &cEvent) { RemoveEvent(m_cWaitingEventList, cEvent, GetActive()); };
	virtual BOOL IsWaitingEvent(const CUIEvent &cEvent) const { return IsEvent(m_cWaitingEventList, cEvent); };
	virtual void FlushWaitingEventList() { m_cWaitingEventList.RemoveAll(); };

	// Member access
	virtual BOOL GetPhantomEnd() const { return m_bPhantomEnd; };
	virtual void SetPhantomEnd(BOOL bPhantom) { m_bPhantomEnd = bPhantom; };

	virtual const CUIEventList &GetStartEventList() const;
	virtual const CUIEventList &GetEndEventList() const;

	virtual const CUIEventList &GetWaitingEventList() const { return m_cWaitingEventList; };
	virtual void SetWaitingEventList(const CUIEventList &cEventList) { m_cWaitingEventList.CopyArray(cEventList); };

	virtual CRegionView *GetView() const { return m_pView; };
	virtual void SetView(CRegionView *pView) { m_pView = pView; };

	virtual const CPoint &GetStartPt() const { return m_cStartPt; };
	virtual void SetStartPt(const CPoint &cPoint) { m_cStartPt = cPoint; };

	virtual const CPoint &GetCurPt() const { return m_cCurPt; };
	virtual void SetCurPt(const CPoint &cPoint) { m_cCurPt = cPoint; };
			
	virtual const CPoint &GetLastPt() const { return m_cLastPt; };
	virtual void SetLastPt(const CPoint &cPoint) { m_cLastPt = cPoint; };

	virtual const CRect &GetRect() const { return m_cRect; };
	virtual void SetRect(const CRect &cRect) { m_cRect = cRect; };

	virtual const CRect &GetLastRect() const { return m_cLastRect; };
	virtual void SetLastRect(const CRect &cRect) { m_cLastRect = cRect; };

	virtual DWORD GetLastUpdateTime() const { return m_dwLastUpdateTime; };
	virtual void SetLastUpdateTime(DWORD dwTime) { m_dwLastUpdateTime = dwTime; };

	virtual DWORD GetTimeDelta() const { return m_dwTimeDelta; };
	virtual void SetTimeDelta(DWORD dwTime) { m_dwTimeDelta = dwTime; };

	virtual BOOL GetAutoCenter() const { return m_bAutoCenter; };
	virtual void SetAutoCenter(BOOL bAutoCenter) { m_bAutoCenter = bAutoCenter; };

	virtual BOOL GetAutoHide() const { return m_bAutoHide; };
	virtual void SetAutoHide(BOOL bAutoHide) { m_bAutoHide = bAutoHide; };

	virtual BOOL GetAutoCapture() const { return m_bAutoCapture; };
	virtual void SetAutoCapture(BOOL bAutoCapture) { m_bAutoCapture = bAutoCapture; };

	virtual int32 GetWheelDelta() const	{return m_nWheelDelta;}
	virtual void  SetWheelDelta(int32 nDelta) {m_nWheelDelta = nDelta;}

	// Event Processing and watching
	virtual BOOL ProcessEvent(const CUIEvent &cEvent);
	virtual void WatchEvent(const CUIEvent &cEvent);
	virtual void Cancel();

	// Override manager control
	virtual CUITrackerMgr *GetOverrideMgr() { return &m_cOverrideMgr; };
	virtual void AddOverride(CUITracker *pTracker) { m_cOverrideMgr.AddTracker(pTracker); };

	// Notification callbacks for subclass expansion

	// Called when the events are processed which will start the tracker
	//		Returns: TRUE if the tracker should start
	virtual BOOL OnStart() { return TRUE; };
	// Called when an event is processed and the tracker is active
	//		Returns: TRUE if the tracker should continue being active
	virtual BOOL OnUpdate(const CUIEvent &cEvent) { return GetActive(); };
	// Called when the events are processed which will end the tracker
	//		Returns: TRUE if the tracker should end
	virtual BOOL OnEnd() { return TRUE; };
	// Called when the tracker is cancelled (calls OnEnd by default)
	virtual void OnCancel() { OnEnd(); };
};

#endif //__RVTRACKER_H__