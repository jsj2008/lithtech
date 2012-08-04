// FloatButton.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "FloatButton.h"
#include "FloatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFloatButton

CFloatButton::CFloatButton()
{
	m_pFloat = NULL;
}

CFloatButton::~CFloatButton()
{
}


BEGIN_MESSAGE_MAP(CFloatButton, CButton)
	//{{AFX_MSG_MAP(CFloatButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFloatButton message handlers

void CFloatButton::OnClicked() 
{
	CFloatDlg dlg(*m_pFloat, "Choose Value");

	if (dlg.DoModal() == IDOK)
	{
		*m_pFloat = dlg.m_float;

		char sTmp[256];
		sprintf(sTmp, FLOAT_PRECISION, *m_pFloat);

		SetWindowText(sTmp);

		GetParent()->Invalidate();
		GetParent()->PostMessage(WM_VALUEUPDATE, 0, 0);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : SetValue()
//
//   PURPOSE  : Sets the value of the float button
//
//------------------------------------------------------------------

void CFloatButton::SetValue(float *pfVal)
{
	m_pFloat = pfVal;

	char sTmp[256];
	sprintf(sTmp, FLOAT_PRECISION, *m_pFloat);

	SetWindowText(sTmp);
}
