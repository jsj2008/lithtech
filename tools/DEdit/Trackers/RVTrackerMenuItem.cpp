//////////////////////////////////////////////////////////////////////
// RVTrackerMenuItem.h - Implementation for the mouse wheel zooming tracker 

#include "bdefs.h"
#include "RVTrackerMenuItem.h"


class CCmdUITrackerMenuItem : public CCmdUI
{
	public:
		CCmdUITrackerMenuItem() : CCmdUI()			{ m_bEnabled = FALSE; }
		virtual void Enable(BOOL bOn = TRUE)		{ m_bEnabled = bOn; }

		//these must be overridden to prevent common menus from calling into the 
		//real function which will cause crashes
		virtual void SetText( LPCTSTR lpszText )	{}
		virtual void SetCheck(int nCheck = 1)		{}
		virtual void SetRadio(BOOL bOn = TRUE)		{}
	

		BOOL m_bEnabled;
};





CRVTrackerMenuItem::CRVTrackerMenuItem( LPCTSTR pName, CRegionView *pView, ONCOMMANDFUNCTION pOnCommand, ONUPDATEFUNCTION pOnUpdate, BOOL bIdleOnly ) :
	CRegionViewTracker(pName, pView)
{
	m_bAutoCenter = FALSE;
	m_bAutoHide = FALSE;
	m_bAutoCapture = FALSE;
	m_bPhantomEnd = FALSE;

	m_pOnCommand = pOnCommand;
	m_pOnUpdate = pOnUpdate;

	m_bStart = FALSE;
	m_bIdleOnly = bIdleOnly;
}

CRVTrackerMenuItem::~CRVTrackerMenuItem()
{
}

BOOL CRVTrackerMenuItem::OnStart()
{
	//// See if the menu item is enabled:
	if( m_pOnUpdate )
	{
		CCmdUITrackerMenuItem item;
		item.m_bEnabled = TRUE;
		(m_pView->*m_pOnUpdate)( &item );
		if( !item.m_bEnabled )
		{
			// not enabled, so don't execute the command.
			return FALSE;
		}
	}

	if( m_pOnCommand )
	{
		if( m_bIdleOnly )
		{
			// delay calling of callback until OnUpdate is called
			m_bStart = TRUE;	// set this flag so that the command can be executed the next time OnUpdate is called.
			return TRUE;
		}
		else
		{
			// call the callback NOW.
			if( m_pView )
			{
				// execute the command:
				(m_pView->*m_pOnCommand)();
				return TRUE;
			}
		}
	}

	return FALSE;
}


BOOL CRVTrackerMenuItem::OnUpdate( const CUIEvent& cEvent )
{
	if( cEvent.GetType() == UIEVENT_NONE )
	{
		if( m_bStart )
		{
			if( m_pView && m_pOnCommand )
			{
				// execute the command:
				(m_pView->*m_pOnCommand)();
			}
			m_bStart = FALSE;
		}
		return TRUE;
	}
	return TRUE;
}
