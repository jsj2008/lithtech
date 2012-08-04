// ModifyRangeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ButeEdit.h"
#include "ModifyRangeDlg.h"
#include "BEStringFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CModifyRangeDlg dialog


CModifyRangeDlg::CModifyRangeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CModifyRangeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModifyRangeDlg)
	m_sName = _T("");
	m_sRange = _T("");
	m_bReplaceAll = FALSE;
	//}}AFX_DATA_INIT
	m_range=CARange(0,0);
}


void CModifyRangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModifyRangeDlg)
	DDX_Text(pDX, IDC_EDIT_NAME, m_sName);
	DDX_Text(pDX, IDC_EDIT_RANGE, m_sRange);
	DDX_Check(pDX, IDC_CHECK_REPLACE_ALL, m_bReplaceAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModifyRangeDlg, CDialog)
	//{{AFX_MSG_MAP(CModifyRangeDlg)
	ON_EN_CHANGE(IDC_EDIT_RANGE, OnChangeEditRange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModifyRangeDlg message handlers

BOOL CModifyRangeDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Create the range string
	CString sMin;
	sMin.Format("%.6f", m_range.GetMin());
	CBEStringFunc::TrimZeros(sMin);

	CString sMax;
	sMax.Format("%.6f", m_range.GetMax());
	CBEStringFunc::TrimZeros(sMax);
	
	// Format the 3 values into the vector string
	m_sRange.Format("%s, %s", sMin, sMax);

	// Update the data
	UpdateData(FALSE);
	
	// Update the state of the common controls
	UpdateCommonControlStates(this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModifyRangeDlg::OnChangeEditRange() 
{
	// Update the data
	UpdateData();

	// Parse the string
	float fMin, fMax;
	int nFields=sscanf(m_sRange, "%f, %f", &fMin, &fMax);

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
		m_range=CARange(fMin, fMax);
	}	
}

void CModifyRangeDlg::OnOK() 
{
	// Make sure that the common data is okay
	UpdateData();
	if (!IsOkayToClose(this, m_sName))
	{
		return;
	}
	
	CDialog::OnOK();
}
