//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// AlphaFromColorDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "alphafromcolordlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AlphaFromColorDlg dialog


AlphaFromColorDlg::AlphaFromColorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AlphaFromColorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(AlphaFromColorDlg)
	m_Offset = _T("0");
	m_Scale = _T("100");
	//}}AFX_DATA_INIT
}


void AlphaFromColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AlphaFromColorDlg)
	DDX_Control(pDX, IDC_SCALESPIN, m_ScaleSpin);
	DDX_Control(pDX, IDC_OFFSETSPIN, m_OffsetSpin);
	DDX_Text(pDX, IDC_OFFSET, m_Offset);
	DDX_Text(pDX, IDC_SCALE, m_Scale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AlphaFromColorDlg, CDialog)
	//{{AFX_MSG_MAP(AlphaFromColorDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AlphaFromColorDlg message handlers

BOOL AlphaFromColorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_OffsetSpin.SetRange(-15, 15);
	m_OffsetSpin.SetPos(atoi(m_Offset));

	m_ScaleSpin.SetRange(0, 500);
	m_ScaleSpin.SetPos(atoi(m_Scale));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
