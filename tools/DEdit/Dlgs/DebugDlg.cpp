//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// DebugDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "debugdlg.h"
#include "dedit_concommand.h"
#include "regiondoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDebugDlg dialog


CDebugDlg::CDebugDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDebugDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDebugDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDebugDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDebugDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDebugDlg, CDialog)
	//{{AFX_MSG_MAP(CDebugDlg)
	ON_COMMAND(IDHIDE, OnHide)
	ON_COMMAND(IDC_RUNCOMMAND, OnRunCommand)
	ON_BN_CLICKED(IDC_CLEAR_DEBUG_TEXT, Clear)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_PROPERTIES, OnUpdateProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDebugDlg message handlers

void CDebugDlg::AddMessage(const char *pStr, ...)
{
	va_list marker;

	va_start(marker, pStr);
	AddMessage2(pStr, marker);
	va_end(marker);
}


void CDebugDlg::AddMessage2(const char *pStr, va_list marker)
{
	char str[512];
	CWnd *pWnd;
	int nLen;

	if(!m_hWnd)
		return;

	pWnd = GetDlgItem(IDC_MESSAGES);

	LTVSNPrintF(str, sizeof(str), pStr, marker);
	LTStrCat(str, "\r\n", sizeof(str));

	nLen = pWnd->SendMessage(EM_GETLIMITTEXT, 0, 0);
	pWnd->SendMessage(EM_SETSEL, nLen, nLen);
	pWnd->SendMessage(EM_REPLACESEL, FALSE, (LPARAM)str);
}


void CDebugDlg::OnHide()
{
	ShowWindow(SW_HIDE);
}


void CDebugDlg::OnRunCommand()
{
	CWnd *pWnd;

	pWnd = GetDlgItem(IDC_COMMANDSTRING);
	if(pWnd)
	{
		char pszText[512];
		pWnd->GetWindowText(pszText, 512);
		AddMessage("--- %s ---", pszText);
		
		dedit_CommandHandler(pszText);
	}
}

void CDebugDlg::Clear()
{
	CWnd *pWnd = GetDlgItem(IDC_MESSAGES);

	if (!pWnd)
		return;

	pWnd->SendMessage(WM_SETTEXT, 0, (LPARAM)"");
}

void CDebugDlg::OnUpdateProperties()
{
	CRegionDoc* pDoc = ::GetActiveRegionDoc();

	if(!pDoc)
	{
		MessageBox("There is currently no region selected", "Cannot update object properties", MB_OK | MB_ICONEXCLAMATION);
		return;
	}

	pDoc->UpdateAllObjectProperties();
}
