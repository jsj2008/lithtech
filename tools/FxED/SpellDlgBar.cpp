// SpellDlgBar.cpp : implementation file
//

#include "stdafx.h"
#include "SpellEd.h"
#include "SpellDlgBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellDlgBar dialog


CSpellDlgBar::CSpellDlgBar(CWnd* pParent /*=NULL*/)
	: CDialogBar()
{
	m_pSpellCtrl = NULL;

	//{{AFX_DATA_INIT(CSpellDlgBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSpellDlgBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellDlgBar)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpellDlgBar, CDialogBar)
	//{{AFX_MSG_MAP(CSpellDlgBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellDlgBar message handlers

void CSpellDlgBar::OnSize(UINT nType, int cx, int cy) 
{
	CDialogBar::OnSize(nType, cx, cy);

	if (!IsWindow(m_tabCtrl.GetSafeHwnd()))
	{
		// Create everything

		m_tabCtrl.Create(WS_VISIBLE | WS_CLIPCHILDREN, CRect(0, 0, cx, cy), this, 0x1234);

		TC_ITEM tcItm;

		tcItm.mask = TCIF_TEXT;
		tcItm.pszText = "FX";
		tcItm.cchTextMax = 6;

		m_tabCtrl.InsertItem(0, &tcItm);
	}

	CRect rcDlgBar;
	GetClientRect(&rcDlgBar);

	m_tabCtrl.MoveWindow(2, 4, rcDlgBar.Width() - 4, rcDlgBar.Height() - 8);
}

BOOL CSpellDlgBar::DestroyWindow() 
{
	return CDialogBar::DestroyWindow();
}
