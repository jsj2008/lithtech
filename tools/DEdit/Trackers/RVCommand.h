//////////////////////////////////////////////////////////////////////
// RVCommand.h - Header for the RegionView Command tracker 

#ifndef __RVCOMMAND_H__
#define __RVCOMMAND_H__

#include "rvtracker.h"

class CRegionViewCommand : public CRegionViewTracker
{
protected:
	BOOL m_bFireOnStart;

	CUIEventList m_cToggleList;
	CMoArray<BOOL> m_cToggleStateList;

	DWORD m_dwData;

public:
	CRegionViewCommand(LPCTSTR pName = "", CRegionView *pView = NULL, BOOL bOnStart = TRUE);
	virtual ~CRegionViewCommand();


	// Member access
	virtual void SetFireOnStart(BOOL bFireOnStart) { m_bFireOnStart = bFireOnStart; };
	virtual BOOL GetFireOnStart() const { return m_bFireOnStart; };

	// Set a toggle event
	virtual void SetToggle(DWORD dwIndex, const CUIEvent &cEvent);
	// Get a toggle event
	virtual const CUIEvent &GetToggle(DWORD dwIndex) const;

	// Set the number of toggle events
	virtual void SetToggleCount(DWORD dwSize);
	// Get the number of toggle events
	virtual DWORD GetToggleCount() const { return m_cToggleList.GetSize(); };

	// Set a toggle state
	virtual void SetToggleState(DWORD dwIndex, BOOL bValue);
	// Get a toggle state
	virtual BOOL GetToggleState(DWORD dwIndex) const;

	// Set data specific to the callback object
	virtual void SetData(DWORD dwData) { m_dwData = dwData; };
	virtual DWORD GetData() const { return m_dwData; };


	// Override for the watch event to keep track of the toggle states
	virtual void WatchEvent(const CUIEvent &cEvent);

	// Overrides for the standard start/end behavior
	virtual BOOL OnStart();
	virtual BOOL OnEnd();
	virtual void OnCancel() {}; // Don't call the end handler..

	// Notification callbacks for subclass expansion
	// Called when the command should fire
	//		Returns : Same meaning as OnStart or OnEnd depending on m_bFireOnStart
	virtual BOOL OnCommand() { return TRUE; };
};

#endif //__RVCOMMAND_H__