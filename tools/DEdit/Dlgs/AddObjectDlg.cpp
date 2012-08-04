//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// AddObjectDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "addobjectdlg.h"
#include "editobjects.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAddObjectDlg dialog


CAddObjectDlg::CAddObjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAddObjectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddObjectDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CAddObjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddObjectDlg)
	DDX_Control(pDX, IDC_OBJECTTREE, m_ObjectTree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddObjectDlg, CDialog)
	//{{AFX_MSG_MAP(CAddObjectDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddObjectDlg message handlers

int CAddObjectDlg::DoModal() 
{
	return CDialog::DoModal();
}


void CAddObjectDlg::OnOK() 
{
	HTREEITEM	hItem = m_ObjectTree.GetSelectedItem();

	if( hItem )
		m_SelectedTypeName = m_ObjectTree.GetItemText( hItem );
	else	
		m_SelectedTypeName = "";
	
	CDialog::OnOK();
}

BOOL CAddObjectDlg::OnInitDialog() 
{
	HTREEITEM	hItem;


	CDialog::OnInitDialog();


	m_ObjectTree.DeleteAllItems();

	hItem = m_ObjectTree.InsertItem( g_BaseTypeName );
	m_ObjectTree.InsertItem( g_LightTypeName, hItem );
	m_ObjectTree.InsertItem( g_ModelTypeName, hItem );
	m_ObjectTree.InsertItem( g_SpriteTypeName, hItem );
	m_ObjectTree.InsertItem( g_SoundTypeName, hItem );

	m_ObjectTree.Expand( hItem, TVE_EXPAND );
	m_ObjectTree.SelectItem( hItem );


	m_ObjectTree.SetFocus();


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
