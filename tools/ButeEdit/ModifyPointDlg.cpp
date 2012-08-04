// ModifyPointDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyPointDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyPointDlg dialog


CModifyPointDlg::CModifyPointDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyPointDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyPointDlg)
	m_sName = _T("");
	m_sPoint = _T("");
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
	m_point=CPoint(0,0);
}


void CModifyPointDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyPointDlg)
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Text(pDX, IDC_EDIT_POINT, m_sPoint);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyPointDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyPointDlg)
	ON_EN_CHANGE(IDC_EDIT_POINT, OnChangeEditPoint)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyPointDlg message handlers

BOOL CModifyPointDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Create the point string
	m_sPoint.Format("%d, %d", m_point.x, m_point.y);

	// Update the data
	UpdateData(FALSE);
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyPointDlg::OnChangeEditPoint() 
{
	// Update the data
	UpdateData();

	// Parse the string
	int x, y;
	int nFields=sscanf(m_sPoint, "%d, %d", &x, &y);

	// Make sure that the 2 fields were parsed
	if (nFields != 2)
	{
		// Disable the OK button
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	else
	{
		// Enable the OK button
		GetDlgItem(IDOK)->EnableWindow(TRUE);

		// Update the vector
		m_point=CPoint(x, y);
	}	
}


void CModifyPointDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}
	
	CDialog::OnOK();
}
