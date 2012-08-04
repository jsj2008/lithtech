//////////////////////////////////////////////////////////////////////
// RVTrackerMenuItem.h - Header for the menu item tracker 

#ifndef __RVTRACKERMENUITEM_H__
#define __RVTRACKERMENUITEM_H__

#include "rvtracker.h"

typedef void (CRegionView::* ONCOMMANDFUNCTION)();
typedef void (CRegionView::* ONUPDATEFUNCTION)( CCmdUI* );


class CRVTrackerMenuItem : public CRegionViewTracker
{
public:
	CRVTrackerMenuItem( LPCTSTR pName = "",				// name of event
						CRegionView *pView = NULL,		// region view
						ONCOMMANDFUNCTION = NULL,		// function called when command is activated
						ONUPDATEFUNCTION = NULL,		// function called to see if command should be activated
						BOOL bIdleOnly = false );		// if this is TRUE, the command will be activated when idle; if FALSE, it is activated immediately

	virtual ~CRVTrackerMenuItem();

	virtual BOOL OnStart();
	virtual BOOL OnUpdate( const CUIEvent& cEvent );
	inline virtual void OnHotKeysChange() { m_bStart = FALSE; CRegionViewTracker::FlushTracker(); }

protected:
	ONCOMMANDFUNCTION m_pOnCommand;
	ONUPDATEFUNCTION	m_pOnUpdate;
	BOOL							m_bStart;
	BOOL							m_bIdleOnly;
};

#endif //__RVTRACKERMENUITEM_H__