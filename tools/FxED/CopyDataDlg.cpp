// CopyDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "CopyDataDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CCopyDataDlg::s_bCopyColour = TRUE;
BOOL CCopyDataDlg::s_bCopyMotion = TRUE;
BOOL CCopyDataDlg::s_bCopyScale  = TRUE;

/////////////////////////////////////////////////////////////////////////////
// CCopyDataDlg dialog


CCopyDataDlg::CCopyDataDlg(BOOL bColour, BOOL bMotion, BOOL bScale, CWnd* pParent /*=NULL*/)
	: CDialog(CCopyDataDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCopyDataDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bEnableColour = bColour;
	m_bEnableScale = bScale;
	m_bEnableMotion = bMotion;
}


void CCopyDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCopyDataDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		if (!m_bEnableColour) GetDlgItem(IDC_COLORKEYS)->EnableWindow(FALSE);
		if (!m_bEnableScale) GetDlgItem(IDC_SCALEKEYS)->EnableWindow(FALSE);
		if (!m_bEnableMotion) GetDlgItem(IDC_MOTIONKEYS)->EnableWindow(FALSE);

		if (m_bEnableColour) ((CButton *)GetDlgItem(IDC_COLORKEYS))->SetCheck(CCopyDataDlg::s_bCopyColour);
		if (m_bEnableScale) ((CButton *)GetDlgItem(IDC_SCALEKEYS))->SetCheck(CCopyDataDlg::s_bCopyScale);
		if (m_bEnableMotion) ((CButton *)GetDlgItem(IDC_MOTIONKEYS))->SetCheck(CCopyDataDlg::s_bCopyMotion);
	}
}


BEGIN_MESSAGE_MAP(CCopyDataDlg, CDialog)
	//{{AFX_MSG_MAP(CCopyDataDlg)
	ON_BN_CLICKED(IDC_COLORKEYS, OnColourkeys)
	ON_BN_CLICKED(IDC_MOTIONKEYS, OnMotionkeys)
	ON_BN_CLICKED(IDC_SCALEKEYS, OnScalekeys)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyDataDlg message handlers

void CCopyDataDlg::OnColourkeys() 
{
	CCopyDataDlg::s_bCopyColour = !CCopyDataDlg::s_bCopyColour;
}

void CCopyDataDlg::OnMotionkeys() 
{
	CCopyDataDlg::s_bCopyMotion = !CCopyDataDlg::s_bCopyMotion;
}

void CCopyDataDlg::OnScalekeys() 
{
	CCopyDataDlg::s_bCopyScale = !CCopyDataDlg::s_bCopyScale;
}
