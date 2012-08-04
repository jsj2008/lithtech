// VectorButton.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "VectorButton.h"
#include "EditVectorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVectorButton

CVectorButton::CVectorButton(DWORD dwID)
{
	m_dwID = dwID;
}

CVectorButton::~CVectorButton()
{
}


BEGIN_MESSAGE_MAP(CVectorButton, CButton)
	//{{AFX_MSG_MAP(CVectorButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVectorButton message handlers

void CVectorButton::OnClicked() 
{
	CString sWndTxt;
	CEdit *pEdit;

	// Retrieve the three vector components

	float fVec[3];	

	pEdit = (CEdit *)GetParent()->GetDlgItem(m_dwID - 2);
	pEdit->GetWindowText(sWndTxt);
	fVec[0] = (float)atof(sWndTxt.GetBuffer(sWndTxt.GetLength()));

	pEdit = (CEdit *)GetParent()->GetDlgItem(m_dwID - 1);
	pEdit->GetWindowText(sWndTxt);
	fVec[1] = (float)atof(sWndTxt.GetBuffer(sWndTxt.GetLength()));

	pEdit = (CEdit *)GetParent()->GetDlgItem(m_dwID);
	pEdit->GetWindowText(sWndTxt);
	fVec[2] = (float)atof(sWndTxt.GetBuffer(sWndTxt.GetLength()));
	
	CEditVectorDlg dlg(fVec);

	if (dlg.DoModal() == IDOK)
	{
		char sTmp[256];

		sprintf(sTmp, VECTOR_PRECISION, dlg.m_tran.x);
		pEdit = (CEdit *)GetParent()->GetDlgItem(m_dwID - 2);
		pEdit->SetWindowText(sTmp);

		sprintf(sTmp, VECTOR_PRECISION, dlg.m_tran.y);
		pEdit = (CEdit *)GetParent()->GetDlgItem(m_dwID - 1);
		pEdit->SetWindowText(sTmp);

		sprintf(sTmp, VECTOR_PRECISION, dlg.m_tran.z);
		pEdit = (CEdit *)GetParent()->GetDlgItem(m_dwID);
		pEdit->SetWindowText(sTmp);
	}
}
