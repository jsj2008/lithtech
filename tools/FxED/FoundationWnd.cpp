// FoundationWnd.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "FoundationWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFoundationWnd

CFoundationWnd::CFoundationWnd()
{
}

CFoundationWnd::~CFoundationWnd()
{
}


BEGIN_MESSAGE_MAP(CFoundationWnd, CWnd)
	//{{AFX_MSG_MAP(CFoundationWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFoundationWnd message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnCreate()
//
//   PURPOSE  : Handles WM_CREATE
//
//------------------------------------------------------------------

int CFoundationWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	// Create the time bar dialog
	
	m_timeBarDlg.Create(IDD_TIMEBAR, this);

	// Create the phase info dialog
	
	m_phaseInfoDlg.Create(IDD_PHASEINFO, this);
	
	// Create the sequencer window	
	
	return 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CFoundationWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// Resize dialogs

	CRect rcClient;
	GetClientRect(&rcClient);

	// Move the dialogs

	if (IsWindow(m_phaseInfoDlg.GetSafeHwnd()))
	{
		CRect rcPhaseInfo;
		m_phaseInfoDlg.GetWindowRect(&rcPhaseInfo);

//		m_phaseInfoDlg.MoveWindow(0, 0, rcClient.Width(), rcPhaseInfo.Height());
	}

	if (IsWindow(m_timeBarDlg.GetSafeHwnd()))
	{
		CRect rcTimeBar;
		m_timeBarDlg.GetWindowRect(&rcTimeBar);

		m_timeBarDlg.MoveWindow(0, rcClient.Height() - rcTimeBar.Height(), rcClient.Width(), rcTimeBar.Height());
	}
}
