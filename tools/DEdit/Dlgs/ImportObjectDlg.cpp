//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ImportObjectDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "importobjectdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportObjectDlg dialog


CImportObjectDlg::CImportObjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImportObjectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportObjectDlg)
	m_nUpdateRadio = 0;
	//}}AFX_DATA_INIT
}


void CImportObjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportObjectDlg)
	DDX_Radio(pDX, IDC_RADIO_UPDATE_PROPERTIES, m_nUpdateRadio);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportObjectDlg, CDialog)
	//{{AFX_MSG_MAP(CImportObjectDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportObjectDlg message handlers
