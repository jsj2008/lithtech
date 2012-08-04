//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// RotateSelection.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "rotateselection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotateSelection dialog


CRotateSelection::CRotateSelection(CWnd* pParent /*=NULL*/)
	: CDialog(CRotateSelection::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRotateSelection)
	m_rRotation = 0.0f;
	//}}AFX_DATA_INIT
}


void CRotateSelection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRotateSelection)
	DDX_Text(pDX, IDC_EC_ROTATION, m_rRotation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRotateSelection, CDialog)
	//{{AFX_MSG_MAP(CRotateSelection)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_45, OnButtonRotate45)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_90, OnButtonRotate90)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_180, OnButtonRotate180)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_NEG_45, OnButtonRotateNeg45)
	ON_BN_CLICKED(IDC_BUTTON_ROTATE_NEG_90, OnButtonRotateNeg90)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRotateSelection message handlers

void CRotateSelection::OnButtonRotate45() 
{
	m_rRotation=45.0f;
	UpdateData(FALSE);
	EndDialog(IDOK);
}

void CRotateSelection::OnButtonRotate90() 
{
	m_rRotation=90.0f;
	UpdateData(FALSE);
	EndDialog(IDOK);	
}

void CRotateSelection::OnButtonRotate180() 
{
	m_rRotation=180.0f;
	UpdateData(FALSE);
	EndDialog(IDOK);
}

void CRotateSelection::OnButtonRotateNeg45() 
{
	m_rRotation=-45.0f;
	UpdateData(FALSE);
	EndDialog(IDOK);
}

void CRotateSelection::OnButtonRotateNeg90() 
{
	m_rRotation=-90.0f;
	UpdateData(FALSE);
	EndDialog(IDOK);	
}
