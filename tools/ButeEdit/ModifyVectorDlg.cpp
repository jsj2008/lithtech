// ModifyVectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ButeMgr.h"
#include "ModifyVectorDlg.h"
#include "BEStringFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyVectorDlg dialog


CModifyVectorDlg::CModifyVectorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyVectorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyVectorDlg)
	m_sVector = _T("");
	m_sName = _T("");
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
	m_vVector=CAVector(0.0, 0.0, 0.0);
}


void CModifyVectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyVectorDlg)
	DDX_Text(pDX, IDC_EDIT_VECTOR, m_sVector);
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyVectorDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyVectorDlg)
	ON_EN_CHANGE(IDC_EDIT_VECTOR, OnChangeEditVector)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyVectorDlg message handlers

/************************************************************************/
// The dialog is initializing
BOOL CModifyVectorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Create the vector string
	CString x;
	x.Format("%.6f", m_vVector.Geti());
	CBEStringFunc::TrimZeros(x);

	CString y;
	y.Format("%.6f", m_vVector.Getj());
	CBEStringFunc::TrimZeros(y);

	CString z;
	z.Format("%.6f", m_vVector.Getk());
	CBEStringFunc::TrimZeros(z);

	// Format the 3 values into the vector string
	m_sVector.Format("%s, %s, %s", x, y, z);

	// Update the data
	UpdateData(FALSE);		
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/************************************************************************/
// The vector string has changed
void CModifyVectorDlg::OnChangeEditVector() 
{
	// Update the data
	UpdateData();

	// Parse the string
	float x, y, z;
	int nFields=sscanf(m_sVector, "%f, %f, %f", &x, &y, &z);

	// Make sure that the 3 fields were parsed
	if (nFields != 3)
	{
		// Disable the OK button
		GetDlgItem(IDOK)->EnableWindow(FALSE);
	}
	else
	{
		// Enable the OK button
		GetDlgItem(IDOK)->EnableWindow(TRUE);

		// Update the vector
		m_vVector=CAVector(x, y, z);
	}		
}

void CModifyVectorDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}
	
	CDialog::OnOK();
}
