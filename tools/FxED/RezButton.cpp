// RezButton.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "RezButton.h"
#include "RezDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRezButton

CRezButton::CRezButton(CString sExt, CString sInitial)
{
	m_sExt = sExt;
	m_sRez = sInitial;
}

CRezButton::~CRezButton()
{
}


BEGIN_MESSAGE_MAP(CRezButton, CButton)
	//{{AFX_MSG_MAP(CRezButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRezButton message handlers

void CRezButton::OnClicked() 
{
	CRezDlg dlg(m_sExt, m_sRez);

	if (dlg.DoModal() == IDOK)
	{
		m_sRez = dlg.m_sPath;

		SetWindowText(m_sRez);
	}
}

int CRezButton::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CButton::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}
