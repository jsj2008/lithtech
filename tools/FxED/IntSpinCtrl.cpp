// IntSpinCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "IntSpinCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntSpinCtrl

CIntSpinCtrl::CIntSpinCtrl(CEdit *pBuddy)
{
	m_pEdit = pBuddy;
}

CIntSpinCtrl::~CIntSpinCtrl()
{
}


BEGIN_MESSAGE_MAP(CIntSpinCtrl, CSpinButtonCtrl)
	//{{AFX_MSG_MAP(CIntSpinCtrl)
	ON_NOTIFY_REFLECT(UDN_DELTAPOS, OnDeltapos)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntSpinCtrl message handlers

void CIntSpinCtrl::OnDeltapos(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	int delta = -pNMUpDown->iDelta;

	char sTmp[256];

	int iVal;

	m_pEdit->GetWindowText(sTmp, 256);
	iVal = atoi(sTmp);

	iVal += delta;
	if (iVal < 0) iVal = 0;
	sprintf(sTmp, "%d", iVal);
	
	m_pEdit->SetWindowText(sTmp);
	
	*pResult = 0;
}
