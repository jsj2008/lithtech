//////////////////////////////////////////////////////////////////////
// RVTrackerFocus.h - Implementation for the focus tracker 

#include "bdefs.h"
#include "mainfrm.h"
#include "rvtrackerfocus.h"
#include "eventnames.h"
#include "OptionsControls.h"

int CRVTrackerFocus::m_bLocked = FALSE;

CRVTrackerFocus::CRVTrackerFocus(LPCTSTR pName, CRegionView *pView) :
	CRegionViewTracker(pName, pView),
	m_bWasActive(FALSE),
	m_pLockTracker(NULL)
{
	m_bAutoCenter = FALSE;
	m_bAutoHide = FALSE;
	m_bPhantomEnd = FALSE;

	RefreshKeyList();

	FlushTracker();
}

CRVTrackerFocus::~CRVTrackerFocus()
{
	delete m_pLockTracker; 
}

void CRVTrackerFocus::FlushTracker()
{
	//setup the new tracker
	delete m_pLockTracker;

	m_pLockTracker = new CRegionViewTracker(UIE_LOCK_FOCUS, m_pView);
	if(m_pLockTracker)
	{
		m_pLockTracker->SetAutoCapture(FALSE);
		m_pLockTracker->SetAutoCenter(FALSE);
		m_pLockTracker->SetAutoHide(FALSE);
	}

	CRegionViewTracker::FlushTracker();
}

BOOL CRVTrackerFocus::OnStart()
{
	// Jump out if we've already got the focus
	if (m_bWasActive || m_bLocked)
		return FALSE;

	//make sure that the user is in a mode that the mouse will be automatically 
	//captured
	if(!GetApp()->GetOptions().GetControlsOptions()->IsAutoCaptureFocus())
	{
		return FALSE;
	}

	// Set the focus
	m_pView->SetFocus();
	m_pView->GetParentFrame()->SetActiveView(m_pView);
	m_pView->OnActivateView(TRUE, m_pView, NULL);
	m_bWasActive = TRUE;

	// Update the keyboard state
	ResyncKeyList();

	return (GetEndEventList().GetSize() > 0);
}

void CRVTrackerFocus::OnCancel()
{
	m_bWasActive	= FALSE;
	m_bLocked		= FALSE;
}

void CRVTrackerFocus::WatchEvent(const CUIEvent &cEvent)
{
	if (m_pLockTracker)
	{
		BOOL bOldActive = m_pLockTracker->GetActive();
		m_pLockTracker->ProcessEvent(cEvent);
		if(m_pLockTracker->GetActive() != bOldActive)
		{		
			m_bLocked = m_pLockTracker->GetActive();
		}
	}

	CRegionViewTracker::WatchEvent(cEvent);

	if (((GetWaitingEventList().GetSize() != 0) || (!GetEndEventList().GetSize())) && (!m_bLocked))
	{
		if (m_bWasActive && !m_pView->m_bInFocus)
			RefreshKeyList();
		m_bWasActive = m_pView->m_bInFocus;
	}
}

void CRVTrackerFocus::RefreshKeyList()
{
	// Get the keyboard state from windows
	GetKeyboardState(m_aKeyStates);
}

void CRVTrackerFocus::ResyncKeyList()
{
	if (!m_pView)
		return;

	CUITrackerMgr *pTrackerMgr = &(m_pView->m_cTrackerMgr);

	BYTE aCurStates[256];
	// Copy what used to be there
	memcpy(aCurStates, m_aKeyStates, 256);

	// Update the master key list array
	RefreshKeyList();

	// Run through the key list and send a key event for anything that doesn't match
	DWORD i;
	for (i = 0; i < 256; i++)
	{
		if ((aCurStates[i] & 0x80) != (m_aKeyStates[i] & 0x80))
			pTrackerMgr->ProcessEvent(CUIKeyEvent((m_aKeyStates[i] & 0x80) ? UIEVENT_KEYDOWN : UIEVENT_KEYUP, i));
	}
}
