// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "SpellEd.h"

#include "MainFrm.h"
#include "Splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ACTIVATEAPP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_bFirstActivation = TRUE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Create the dialog bar on the side

	m_dlgBar.Create(this, IDD_DLGBAR, WS_VISIBLE | WS_CLIPCHILDREN | CBRS_LEFT, 0x1234);

	// CG: The following line was added by the Splash Screen component.
	CSplashWnd::ShowSplashScreen(this);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	return CMDIFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIFrameWnd::OnSize(nType, cx, cy);
	
	CRect rcDlgBarClient;
	m_dlgBar.GetClientRect(&rcDlgBarClient);

	m_dlgBar.MoveWindow(0, 0, rcDlgBarClient.Width(), cy);

	RecalcLayout();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnActivateApp()
//
//   PURPOSE  : Handles application activation
//
//------------------------------------------------------------------

#if _MSC_VER >= 1300
void CMainFrame::OnActivateApp(BOOL bActive, DWORD hTask) 
#else
void CMainFrame::OnActivateApp(BOOL bActive, HTASK hTask) 
#endif
{
	CMDIFrameWnd::OnActivateApp(bActive, hTask);
	
	if (m_bFirstActivation)
	{
		m_bFirstActivation = FALSE;

		return;
	}	
/*
	if (bActive)
	{
		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
		pApp->ReloadResourceFile();
	}	
*/
}

void CMainFrame::OnClose()
{
	int ret = AfxMessageBox("Save work ?", MB_YESNO);

	if (ret == IDYES)
	{
		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
		if (!pApp->SaveSpellsAs())
		{
			return;
		}
	}

	CFrameWnd::OnClose();
}