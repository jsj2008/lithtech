// SpellDefTab.cpp : implementation file
//

#include "stdafx.h"
#include "SpellEd.h"
#include "SpellDefTab.h"
#include "SpellEdDoc.h"
#include "SpellEdView.h"
#include "Spell.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int g_bpdWidth;
int g_bpdHeight;

int g_rdWidth;
int g_rdHeight;

/////////////////////////////////////////////////////////////////////////////
// CSpellDefTab

CSpellDefTab::CSpellDefTab()
{
}

CSpellDefTab::~CSpellDefTab()
{
}


BEGIN_MESSAGE_MAP(CSpellDefTab, CTabCtrl)
	//{{AFX_MSG_MAP(CSpellDefTab)
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(TCN_SELCHANGE, OnSelChange)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellDefTab message handlers

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CSpellDefTab
//
//------------------------------------------------------------------

BOOL CSpellDefTab::Init()
{
	CRect rcDlg;

	CSpellEdView *pView = (CSpellEdView *)GetParent();
	CSpell *pSpell = pView->GetSpell();

	InsertItem(0, "Base Parameters");
	InsertItem(1, "Resolution");
	InsertItem(2, "FX - Cast");
	InsertItem(3, "FX - Active");
	InsertItem(4, "FX - Resolution");

	m_baseParmDlg.SetSpell(pSpell);
	m_resolutionDlg.SetSpell(pSpell);

	// Create the base parameters dialog
	
	m_baseParmDlg.Create(IDD_SPELLBASEPARAMETERS, this);
	m_baseParmDlg.GetClientRect(&rcDlg);

	g_bpdWidth  = rcDlg.Width();
	g_bpdHeight = rcDlg.Height();

	// Create the resolution dialog
	
	m_resolutionDlg.Create(IDD_SPELLRESOLUTION, this);
	m_resolutionDlg.GetClientRect(&rcDlg);

	// Create the track windows

	DWORD dwStyle = WS_CHILD | WS_VISIBLE;
	DWORD dwExStyle = WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE;

	m_castTrackWnd.SetPhase(pSpell->GetCastPhase());
	m_castTrackWnd.SetSpell(pSpell);
	m_castTrackWnd.CreateEx(dwExStyle, NULL, "Cast Track Window", dwStyle, 0, 0, 0, 0, this->GetSafeHwnd(), NULL);

	m_activeTrackWnd.SetPhase(pSpell->GetActivePhase());
	m_castTrackWnd.SetSpell(pSpell);
	m_activeTrackWnd.CreateEx(dwExStyle, NULL, "Active Track Window", dwStyle, 0, 0, 0, 0, this->GetSafeHwnd(), NULL);

	m_resolutionTrackWnd.SetPhase(pSpell->GetResolutionPhase());
	m_castTrackWnd.SetSpell(pSpell);
	m_resolutionTrackWnd.CreateEx(dwExStyle, NULL, "Resolution Track Window", dwStyle, 0, 0, 0, 0, this->GetSafeHwnd(), NULL);

	// Add all the windows to the dialog list

	m_collWnds.Add(&m_baseParmDlg);
	m_collWnds.Add(&m_resolutionDlg);
	m_collWnds.Add(&m_castTrackWnd);
	m_collWnds.Add(&m_activeTrackWnd);
	m_collWnds.Add(&m_resolutionTrackWnd);

	g_rdWidth  = rcDlg.Width();
	g_rdHeight = rcDlg.Height();

	// Resize all the dialogs....

	CRect rcTab;
	GetWindowRect(&rcTab);
	AdjustRect(FALSE, rcTab);	
	ScreenToClient(&rcTab);

	int cx = (rcTab.left + rcTab.right) >> 1;
	int cy = (rcTab.top + rcTab.bottom) >> 1;

	m_baseParmDlg.GetClientRect(&rcDlg);		
	int xStart = cx - (rcDlg.Width() >> 1);
	int yStart = cy - (rcDlg.Height() >> 1);
	if (xStart < rcTab.left) xStart = rcTab.left;
	if (yStart < rcTab.top) yStart = rcTab.top;
	m_baseParmDlg.MoveWindow(xStart, yStart, g_bpdWidth, g_bpdHeight);
	
	m_resolutionDlg.GetClientRect(&rcDlg);		
	xStart = cx - (rcDlg.Width() >> 1);
	yStart = cy - (rcDlg.Height() >> 1);
	if (xStart < rcTab.left) xStart = rcTab.left;
	if (yStart < rcTab.top) yStart = rcTab.top;
	m_resolutionDlg.MoveWindow(xStart, yStart, g_rdWidth, g_rdHeight);

	m_castTrackWnd.MoveWindow(rcTab.left, rcTab.top, rcTab.Width(), rcTab.Height());
	m_activeTrackWnd.MoveWindow(rcTab.left, rcTab.top, rcTab.Width(), rcTab.Height());
	m_resolutionTrackWnd.MoveWindow(rcTab.left, rcTab.top, rcTab.Width(), rcTab.Height());

	ShowTab(0);

	m_bitmap.LoadBitmap(IDB_SPELLWRAP);
	m_memDC.CreateCompatibleDC(NULL);
	m_memDC.SelectObject(&m_bitmap);

	dwStyle = ::GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	::SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle | WS_CLIPCHILDREN);
	
	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CSpellDefTab::OnSize(UINT nType, int cx, int cy) 
{
	CRect rcDlg;

	CTabCtrl::OnSize(nType, cx, cy);
	
	if ((IsWindow(m_baseParmDlg.GetSafeHwnd())) &&
	    (IsWindow(m_resolutionDlg.GetSafeHwnd())))
	{
		// Resize all the dialogs....

		CRect rcTab;
		GetWindowRect(&rcTab);
		AdjustRect(FALSE, rcTab);	
		ScreenToClient(&rcTab);

		int cx = (rcTab.left + rcTab.right) >> 1;
		int cy = (rcTab.top + rcTab.bottom) >> 1;

		m_baseParmDlg.GetClientRect(&rcDlg);		
		int xStart = cx - (rcDlg.Width() >> 1);
		int yStart = cy - (rcDlg.Height() >> 1);
		if (xStart < rcTab.left) xStart = rcTab.left;
		if (yStart < rcTab.top) yStart = rcTab.top;
		m_baseParmDlg.MoveWindow(xStart, yStart, g_bpdWidth, g_bpdHeight);
		
		m_resolutionDlg.GetClientRect(&rcDlg);		
		xStart = cx - (rcDlg.Width() >> 1);
		yStart = cy - (rcDlg.Height() >> 1);
		if (xStart < rcTab.left) xStart = rcTab.left;
		if (yStart < rcTab.top) yStart = rcTab.top;
		m_resolutionDlg.MoveWindow(xStart, yStart, g_rdWidth, g_rdHeight);

		m_rcDisp = rcTab;
		
		m_castTrackWnd.MoveWindow(rcTab.left, rcTab.top, rcTab.Width(), rcTab.Height());
		m_activeTrackWnd.MoveWindow(rcTab.left, rcTab.top, rcTab.Width(), rcTab.Height());
		m_resolutionTrackWnd.MoveWindow(rcTab.left, rcTab.top, rcTab.Width(), rcTab.Height());
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : ShowTab()
//
//   PURPOSE  : Shows a specific spell tab
//
//------------------------------------------------------------------

BOOL CSpellDefTab::ShowTab(int nTab)
{
	for (int i = 0; i < m_collWnds.GetSize(); i ++)
	{
		CWnd *pWnd = (CWnd *)m_collWnds[i];

		pWnd->ShowWindow(SW_HIDE);
		pWnd->EnableWindow(FALSE);
	}
	
	CWnd *pWnd = (CWnd *)m_collWnds[nTab];
	pWnd->ShowWindow(SW_SHOW);
	pWnd->EnableWindow(TRUE);

	// Success !!

	return TRUE;
}

void CSpellDefTab::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Show the correct tab

	ShowTab(GetCurSel());
	
	*pResult = 0;
}

BOOL CSpellDefTab::OnEraseBkgnd(CDC* pDC) 
{
	CTabCtrl::OnEraseBkgnd(pDC);

	if (GetCurSel() >= 2) return TRUE;

	CRect rcTab;
	GetWindowRect(&rcTab);
	AdjustRect(FALSE, rcTab);	
	ScreenToClient(&rcTab);

	int nWidth  = (rcTab.Width() / 128) + 1;
	int nHeight = (rcTab.Height() / 128) + 1;

	for (int i = 0; i < nHeight; i ++)
	{
		for (int j = 0; j < nWidth; j ++)
		{
			pDC->BitBlt(rcTab.left + (j * 128), rcTab.top + (i * 128), 128, 128, &m_memDC, 0, 0, SRCCOPY);
		}
	}

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : PreCreateWindow()
//
//   PURPOSE  : Called before window creation
//
//------------------------------------------------------------------

BOOL CSpellDefTab::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= WS_CLIPCHILDREN;
	
	return CTabCtrl::PreCreateWindow(cs);
}
