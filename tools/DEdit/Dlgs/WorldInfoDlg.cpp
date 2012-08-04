//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// WorldInfoDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "worldinfodlg.h"

#include "editpoly.h"
#include "editregion.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWorldInfoDlg dialog


CWorldInfoDlg::CWorldInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWorldInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWorldInfoDlg)
	m_WorldName = _T("");
	m_NumPolies = _T("");
	m_NumPoints = _T("");
	m_NumBrushes = _T("");
	m_WorldInfoString = _T("");
	//}}AFX_DATA_INIT

	m_pRegion = NULL;
}


void CWorldInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWorldInfoDlg)
	DDX_Text(pDX, IDC_WORLDNAME, m_WorldName);
	DDX_Text(pDX, IDC_NUMPOLIES, m_NumPolies);
	DDX_Text(pDX, IDC_NUMPOINTS, m_NumPoints);
	DDX_Text(pDX, IDC_NUMBRUSHES, m_NumBrushes);
	DDX_Text(pDX, IDC_INFOSTRING, m_WorldInfoString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWorldInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CWorldInfoDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldInfoDlg message handlers

BOOL CWorldInfoDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if( m_pRegion )
	{
		m_WorldName.FormatMessage( IDS_WORLDNAME, (LPCTSTR)m_InputWorldName );
		
		m_NumBrushes.FormatMessage( IDS_NUMBRUSHES, m_pRegion->m_Brushes.GetSize() );
		m_NumPolies.FormatMessage( IDS_NUMPOLIES, m_pRegion->GetTotalNumPolies() );
		m_NumPoints.FormatMessage( IDS_NUMPOINTS, m_pRegion->GetTotalNumPoints() );

		m_WorldInfoString = m_pRegion->m_pInfoString;
	
		UpdateData( FALSE );
	}

	SetFocus();	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
