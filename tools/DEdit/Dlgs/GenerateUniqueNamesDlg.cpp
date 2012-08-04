// GenerateUniqueNamesDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "generateuniquenamesdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGenerateUniqueNamesDlg dialog


CGenerateUniqueNamesDlg::CGenerateUniqueNamesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGenerateUniqueNamesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGenerateUniqueNamesDlg)
	m_bUpdateRefProps = TRUE;
	m_bUpdateSelPropsOnly = FALSE;
	m_bDisplayReportOfChanges = TRUE;
	//}}AFX_DATA_INIT
}


void CGenerateUniqueNamesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenerateUniqueNamesDlg)
	DDX_Check(pDX, IDC_CHECK_UPDATE_REFERENCING_PROPERTIES, m_bUpdateRefProps);
	DDX_Check(pDX, IDC_CHECK_UPDATE_SELECTED_ONLY, m_bUpdateSelPropsOnly);
	DDX_Check(pDX, IDC_CHECK_DISPLAY_REPORT, m_bDisplayReportOfChanges);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenerateUniqueNamesDlg, CDialog)
	//{{AFX_MSG_MAP(CGenerateUniqueNamesDlg)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_REFERENCING_PROPERTIES, OnCheckUpdateReferencingProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenerateUniqueNamesDlg message handlers

void CGenerateUniqueNamesDlg::OnCheckUpdateReferencingProperties() 
{
	// Update the data
	UpdateData();
	
	// Enable/Disable the controls if necessary
	if (m_bUpdateRefProps)
	{
		GetDlgItem(IDC_CHECK_UPDATE_SELECTED_ONLY)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_UPDATE_SELECTED_ONLY)->EnableWindow(FALSE);
	}
}

BOOL CGenerateUniqueNamesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Enable/Disable the controls if necessary
	if (m_bUpdateRefProps)
	{
		GetDlgItem(IDC_CHECK_UPDATE_SELECTED_ONLY)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_UPDATE_SELECTED_ONLY)->EnableWindow(FALSE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
