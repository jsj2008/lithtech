// ModifyRectangleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyRectangleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyRectangleDlg dialog


CModifyRectangleDlg::CModifyRectangleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyRectangleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyRectangleDlg)
	m_sName = _T("");
	m_sRectangle = _T("");
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
	m_rcRect=CRect(0,0,0,0);
}


void CModifyRectangleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyRectangleDlg)
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Text(pDX, IDC_EDIT_RECTANGLE, m_sRectangle);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyRectangleDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyRectangleDlg)
	ON_EN_CHANGE(IDC_EDIT_RECTANGLE, OnChangeEditRectangle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyRectangleDlg message handlers

BOOL CModifyRectangleDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Create the rectangle string
	m_sRectangle.Format("%d, %d, %d, %d", m_rcRect.left, m_rcRect.top, m_rcRect.right, m_rcRect.bottom);
	UpdateData(FALSE);
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyRectangleDlg::OnChangeEditRectangle() 
{
	// Update the data
	UpdateData();

	// Parse the string
	int nLeft, nTop, nRight, nBottom;
	int nFields=sscanf(m_sRectangle, "%d, %d, %d, %d", &nLeft, &nTop, &nRight, &nBottom);

	// Make sure that the 4 fields were parsed
	if (nFields != 4)
	{
		// Disable the OK button
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	else
	{
		// Enable the OK button
		GetDlgItem(IDOK)->EnableWindow(TRUE);

		// Update the vector
		m_rcRect=CRect(nLeft, nTop, nRight, nBottom);
	}		
}

void CModifyRectangleDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}
	
	CDialog::OnOK();
}
