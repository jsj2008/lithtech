// IntButton.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "IntButton.h"
#include "IntDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntButton

CIntButton::CIntButton()
{
}

CIntButton::~CIntButton()
{
}


BEGIN_MESSAGE_MAP(CIntButton, CButton)
	//{{AFX_MSG_MAP(CIntButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntButton message handlers

//------------------------------------------------------------------
//
//   FUNCTION : SetValue()
//
//   PURPOSE  : Sets the value of the float button
//
//------------------------------------------------------------------

void CIntButton::SetValue(CString sDlgDisp, int *pInt)
{
	m_pInt = pInt;

	char sTmp[256];
	sprintf(sTmp, "%d", *pInt);

	SetWindowText(sTmp);

	m_sDlgDisp = sDlgDisp;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnClicked()
//
//   PURPOSE  : Called when the button is pressed
//
//------------------------------------------------------------------

void CIntButton::OnClicked() 
{
	CIntDlg dlg(*m_pInt, m_sDlgDisp);

	if (dlg.DoModal() == IDOK)
	{
		*m_pInt = dlg.m_int;

		char sTmp[256];
		sprintf(sTmp, "%d", *m_pInt);

		SetWindowText(sTmp);

		GetParent()->Invalidate();
		GetParent()->PostMessage(WM_VALUEUPDATE, 0, 0);
	}
}
