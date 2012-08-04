// SpherePrimitiveDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "sphereprimitivedlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpherePrimitiveDlg dialog


CSpherePrimitiveDlg::CSpherePrimitiveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpherePrimitiveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpherePrimitiveDlg)
	m_fRadius = 50.0f;
	m_nVerticalSubdivisions = 4;
	m_nSides = 6;
	//}}AFX_DATA_INIT
}


void CSpherePrimitiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpherePrimitiveDlg)
	DDX_Text(pDX, IDC_EDIT_RADIUS, m_fRadius);
	DDV_MinMaxFloat(pDX, m_fRadius, 0.f, 65000.f);
	DDX_Text(pDX, IDC_EDIT_VERTICAL_SUBDIVISIONS, m_nVerticalSubdivisions);
	DDV_MinMaxInt(pDX, m_nVerticalSubdivisions, 3, 64);
	DDX_Text(pDX, IDC_EDIT_SIDES, m_nSides);
	DDV_MinMaxInt(pDX, m_nSides, 3, 64);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpherePrimitiveDlg, CDialog)
	//{{AFX_MSG_MAP(CSpherePrimitiveDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpherePrimitiveDlg message handlers
