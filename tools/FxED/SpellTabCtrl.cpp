// SpellTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "SpellEd.h"
#include "SpellTabCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellTabCtrl

CSpellTabCtrl::CSpellTabCtrl()
{
}

CSpellTabCtrl::~CSpellTabCtrl()
{
}


BEGIN_MESSAGE_MAP(CSpellTabCtrl, CTabCtrl)
	//{{AFX_MSG_MAP(CSpellTabCtrl)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellTabCtrl message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CSpellTabCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CTabCtrl::OnSize(nType, cx, cy);

	CRect rcDlgBar;
	GetWindowRect(&rcDlgBar);
	AdjustRect(FALSE, rcDlgBar);	
	GetParent()->ScreenToClient(&rcDlgBar);

	m_spellCtrl.MoveWindow(rcDlgBar.left, rcDlgBar.top, rcDlgBar.Width(), rcDlgBar.Height());
}

//------------------------------------------------------------------
//
//   FUNCTION : OnCreate()
//
//   PURPOSE  : Handles WM_CREATE
//
//------------------------------------------------------------------

int CSpellTabCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the embedded spell tree control

	m_spellCtrl.Create(WS_VISIBLE | WS_CHILD | TVS_SHOWSELALWAYS | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS, CRect(0, 0, 0, 0), GetParent(), 0x11);
	m_spellCtrl.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_NOMOVE);

	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	m_spellCtrl.SetImageList(pApp->GetImageList(), TVSIL_NORMAL);

	m_spellCtrl.AddGroup(NULL, "[All Fx]");
	m_spellCtrl.SetItemState(m_spellCtrl.GetRootItem(), TVIS_BOLD, TVIS_BOLD);
	
	return 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnLButtonDblClk()
//
//   PURPOSE  : Handles WM_LBUTTONDBLCLK
//
//------------------------------------------------------------------

void CSpellTabCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	CTabCtrl::OnLButtonDblClk(nFlags, point);
}

//------------------------------------------------------------------
//
//   FUNCTION : PostNcDestroy()
//
//   PURPOSE  : Called after window is destroyed
//
//------------------------------------------------------------------

void CSpellTabCtrl::PostNcDestroy() 
{
	CTabCtrl::PostNcDestroy();
}
