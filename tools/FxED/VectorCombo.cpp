// VectorCombo.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "VectorCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CVectorCombo

CVectorCombo::CVectorCombo(DWORD dwID)
{
	m_dwID = dwID;
}

CVectorCombo::~CVectorCombo()
{
}


BEGIN_MESSAGE_MAP(CVectorCombo, CComboBox)
	//{{AFX_MSG_MAP(CVectorCombo)
	ON_WM_CREATE()
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVectorCombo message handlers


int CVectorCombo::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	AddString("Forward");
	AddString("Up");
	AddString("Down");
	AddString("Right");
	
	return 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSelChange()
//
//   PURPOSE  : Called when selection is changed
//
//------------------------------------------------------------------

void CVectorCombo::OnSelChange() 
{
	int nCurSel = GetCurSel();
	if (nCurSel == CB_ERR) return;

	float x = 0.0f, y = 0.0f, z = 0.0f;

	switch (nCurSel)
	{
		case 0 :
		{
			x = 0.0f;
			y = 0.0f;
			z = 1.0f;
		}
		break;

		case 1 :
		{
			x = 0.0f;
			y = 1.0f;
			z = 0.0f;
		}
		break;

		case 2 :
		{
			x = 0.0f;
			y = -1.0f;
			z = 0.0f;
		}
		break;

		case 3 :
		{
			x = 1.0f;
			y = 0.0f;
			z = 0.0f;
		}
		break;
	}

	// Get the three previous controls....

	CEdit *pEditX = (CEdit *)GetParent()->GetDlgItem(m_dwID - 2);
	CEdit *pEditY = (CEdit *)GetParent()->GetDlgItem(m_dwID - 1);
	CEdit *pEditZ = (CEdit *)GetParent()->GetDlgItem(m_dwID);

	char sTmp[256];
	sprintf(sTmp, VECTOR_PRECISION, x);
	pEditX->SetWindowText(sTmp);

	sprintf(sTmp, VECTOR_PRECISION, y);
	pEditY->SetWindowText(sTmp);

	sprintf(sTmp, VECTOR_PRECISION, z);
	pEditZ->SetWindowText(sTmp);

	SetCurSel(-1);
}
