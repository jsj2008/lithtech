//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// CylinderPrimDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "cylinderprimdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCylinderPrimDlg dialog


CCylinderPrimDlg::CCylinderPrimDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCylinderPrimDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCylinderPrimDlg)
	m_sCaption = "Primitive";
	m_nHeight = 128;
	m_nNumSides = 6;
	m_nRadius = 16;
	//}}AFX_DATA_INIT
}


void CCylinderPrimDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCylinderPrimDlg)
	DDX_Text(pDX, IDC_CYLINDER_HEIGHT, m_nHeight);
	DDV_MinMaxUInt(pDX, m_nHeight, 1, 65535);
	DDX_Text(pDX, IDC_CYLINDER_NUSIDES, m_nNumSides);
	DDV_MinMaxUInt(pDX, m_nNumSides, 3, 65535);
	DDX_Text(pDX, IDC_CYLINDER_RADIUS, m_nRadius);
	DDV_MinMaxUInt(pDX, m_nRadius, 1, 65535);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCylinderPrimDlg, CDialog)
	//{{AFX_MSG_MAP(CCylinderPrimDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCylinderPrimDlg message handlers

BOOL CCylinderPrimDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetWindowText( m_sCaption );

	return TRUE;
}
