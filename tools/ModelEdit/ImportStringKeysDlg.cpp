// ImportStringKeysDlg.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "ImportStringKeysDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportStringKeysDlg dialog


CImportStringKeysDlg::CImportStringKeysDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportStringKeysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportStringKeysDlg)
	//}}AFX_DATA_INIT
}


void CImportStringKeysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportStringKeysDlg)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportStringKeysDlg, CDialog)
	//{{AFX_MSG_MAP(CImportStringKeysDlg)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportStringKeysDlg message handlers

void CImportStringKeysDlg::AddMsg(const char *sMsg, BYTE r, BYTE g, BYTE b)
{		
	CEdit *pCtrl = (CEdit *)GetDlgItem(IDC_PROGRESS);
	if (!pCtrl) return;

	// set to bottom of text
	pCtrl->SetSel(-1,-1);

	// set the color of the text to output
	CHARFORMAT cf;
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	cf.crTextColor = (b << 16) | (g << 8) | r;
//	pCtrl->SetSelectionCharFormat(cf);

	// insert text
	pCtrl->ReplaceSel(sMsg);

	// move current view to new next
	int nMinPos;
	int nMaxPos;
	pCtrl->GetScrollRange( SB_VERT, &nMinPos, &nMaxPos );
	pCtrl->SetScrollPos( SB_VERT, nMaxPos, TRUE );
}

BOOL CImportStringKeysDlg::DestroyWindow() 
{
	return CDialog::DestroyWindow();
}

void CImportStringKeysDlg::OnClose() 
{
	CDialog::OnClose();
}

void CImportStringKeysDlg::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CImportStringKeysDlg::OnOK() 
{
	Clear();

	ShowWindow(SW_HIDE);
}

void CImportStringKeysDlg::Clear()
{
	CEdit *pCtrl = (CEdit *)GetDlgItem(IDC_PROGRESS);
	pCtrl->SetWindowText("");
}
