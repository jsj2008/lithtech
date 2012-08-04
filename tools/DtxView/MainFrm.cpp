//------------------------------------------------------------------
//
//  FILE      : MainFrm.cpp
//
//  PURPOSE   :	implementation of the CMainFrame class
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "DtxView.h"

#include "MainFrm.h"
#include "DtxViewView.h"
#include "DtxViewDoc.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_STATUSBAR_IMAGE_INFO, OnUpdateImageInfo)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_STATUSBAR_IMAGE_INFO
};



CMainFrame::CMainFrame()
{
}



CMainFrame::~CMainFrame()
{
}



int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP	| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(1, ID_STATUSBAR_IMAGE_INFO, SBPS_NORMAL, 220);

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}



BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
	{
		return FALSE;
	}

	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}



#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}



void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG



void CMainFrame::OnUpdateImageInfo(CCmdUI *pCmdUI)
{
	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *)GetActiveFrame();
	if (pChild != NULL)
	{
		// Get the active view attached to the active MDI child window.
		CDtxViewView *pView = (CDtxViewView*)pChild->GetActiveView();
		CDtxViewDoc *pDoc = pView->GetDocument();

		CString strInfo;
		if (pDoc->GetImageInfoString(&strInfo))
		{
			pCmdUI->Enable();
			pCmdUI->SetText(strInfo);
		}
		else
		{
			pCmdUI->Enable(FALSE);
		}
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}
