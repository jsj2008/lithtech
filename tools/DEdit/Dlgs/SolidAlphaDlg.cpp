//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// SolidAlphaDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "solidalphadlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SolidAlphaDlg dialog


SolidAlphaDlg::SolidAlphaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SolidAlphaDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(SolidAlphaDlg)
	m_Scale = _T("100");
	//}}AFX_DATA_INIT
}


void SolidAlphaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SolidAlphaDlg)
	DDX_Control(pDX, IDC_SCALESPIN, m_ScaleSpin);
	DDX_Text(pDX, IDC_SCALE, m_Scale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SolidAlphaDlg, CDialog)
	//{{AFX_MSG_MAP(SolidAlphaDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SolidAlphaDlg message handlers

BOOL SolidAlphaDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_ScaleSpin.SetRange(0, 100);
	m_ScaleSpin.SetPos(atoi(m_Scale));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
