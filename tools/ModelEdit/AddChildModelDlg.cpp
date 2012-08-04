// AddChildModelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "AddChildModelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddChildModelDlg

IMPLEMENT_DYNAMIC(CAddChildModelDlg, CFileDialog)

CAddChildModelDlg::CAddChildModelDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_bScaleSkeleton = FALSE;

	m_ofn.Flags |= OFN_ENABLETEMPLATE;
	m_ofn.lpTemplateName = MAKEINTRESOURCE( IDD_ADDCHILDMODEL );
}

void CAddChildModelDlg::OnInitDone( )
{
	CButton *pCheck;

	pCheck = ( CButton * )GetDlgItem( IDC_SCALESKELETON );
	if( pCheck )
		pCheck->SetCheck( m_bScaleSkeleton );
}

void CAddChildModelDlg::OnCheck() 
{
	CButton *pCheck;

	pCheck = ( CButton * )GetDlgItem( IDC_SCALESKELETON );
	if( pCheck )
		m_bScaleSkeleton = pCheck->GetCheck( );
}




BEGIN_MESSAGE_MAP(CAddChildModelDlg, CFileDialog)
	//{{AFX_MSG_MAP(CAddChildModelDlg)
		ON_BN_CLICKED(IDC_IMPORT_USEUVCOORDS, OnCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

