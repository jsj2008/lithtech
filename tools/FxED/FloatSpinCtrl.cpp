// FloatSpinCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "FloatSpinCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFloatSpinCtrl

CFloatSpinCtrl::CFloatSpinCtrl(CEdit *pBuddy)
{
	m_pEdit = pBuddy;
}

CFloatSpinCtrl::~CFloatSpinCtrl()
{
}


BEGIN_MESSAGE_MAP(CFloatSpinCtrl, CSpinButtonCtrl)
	//{{AFX_MSG_MAP(CFloatSpinCtrl)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS, OnDeltapos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFloatSpinCtrl message handlers

void CFloatSpinCtrl::OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int delta = -pNMUpDown->iDelta;

	char sTmp[256];

	float fVal;

	m_pEdit->GetWindowText(sTmp, 256);
	fVal = (float)atof(sTmp);

	fVal += (float)delta / 4;
	if (fVal < 0.0f) fVal = 0.0f;	
	sprintf(sTmp, FLOAT_PRECISION, fVal);
	
	m_pEdit->SetWindowText(sTmp);
	
	*pResult = 0;
}
