//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ClassListDlg.cpp : implementation file
//

#include "bdefs.h"
#include "resource.h"
#include "classlistdlg.h"
#include "editprojectmgr.h"
#include "edithelpers.h"
#include "projectbar.h"
#include "regiondoc.h"
#include "mainfrm.h"
#include "regionview.h"
#include "editregion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ClassListDlg dialog


ClassListDlg::ClassListDlg()	
{
	//{{AFX_DATA_INIT(ClassListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pImageList=NULL;
}

ClassListDlg::~ClassListDlg()
{
	if (m_pImageList)
	{
		delete m_pImageList;
	}
}

void ClassListDlg::DoDataExchange(CDataExchange* pDX)
{
	CMRCSizeDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ClassListDlg)
	DDX_Control(pDX, IDC_CLASS_LIST, m_ClassList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ClassListDlg, CProjectTabControlBar)
	//{{AFX_MSG_MAP(ClassListDlg)
	ON_WM_SIZE()
	ON_NOTIFY( NM_DBLCLK, IDC_CLASS_LIST, OnDblClickClass )
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void ClassListDlg::Update()
{
	DWORD i;
	CEditProjectMgr *pProject;
	int nIndex;

	if(!(pProject = GetProject()))
		return;

	m_ClassList.DeleteAllItems();
	for(i=0; i < pProject->m_nClassDefs; i++)
	{
		if(!(pProject->m_ClassDefs[i].m_ClassFlags & CF_HIDDEN))
		{
			nIndex = m_ClassList.InsertItem(0, pProject->m_ClassDefs[i].m_ClassName, 0);
		}
	}

	for(i=0; i < pProject->m_TemplateClasses; i++)
	{
		nIndex = m_ClassList.InsertItem(0, pProject->m_TemplateClasses[i]->m_ClassName, 1);
	}
}


/////////////////////////////////////////////////////////////////////////////
// ClassListDlg message handlers

void ClassListDlg::OnSize(UINT nType, int cx, int cy) 
{
	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();
}

/************************************************************************/
// Repositions the controls
void ClassListDlg::RepositionControls()
{
	if( ::IsWindow(m_ClassList.m_hWnd) )
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		m_ClassList.MoveWindow( 0, 0, rcClient.Width(), rcClient.Height(), TRUE );	
	}
}

BOOL ClassListDlg::OnInitDialogBar() 
{
	CRect rect;

	CMRCSizeDialogBar::OnInitDialogBar();
	
	GetClientRect( &rect );

	if (m_pImageList)
	{
		delete m_pImageList;
	}
	m_pImageList = new CImageList;

	m_pImageList->Create( IDB_CLASSLIST_ICONS, 16, 2, RGB(0,255,0) );	
	
	m_ClassList.SetImageList(m_pImageList, LVSIL_SMALL);
	m_ClassList.InsertColumn(0,"Name",LVCFMT_LEFT,(rect.Width()-35)/2,-1);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL ClassListDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CMRCSizeDialogBar::OnNotify(wParam, lParam, pResult);
}


void ClassListDlg::OnDblClickClass( NMHDR * pNMHDR, LRESULT * pResult )
{
	int item, len;
	char className[256];
	DWORD i;
	CRegionDoc *pDoc;

	item = m_ClassList.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED);
	if(item != -1)
	{
		len = m_ClassList.GetItemText(item, 0, className, sizeof(className));
		if(len > 0)
		{
			if(pDoc = GetActiveRegionDoc())
			{
				CEditRegion *pRegion=pDoc->GetRegion();

				// Add the object
				CWorldNode *pObject=pDoc->AddObject(className, pDoc->GetRegion()->GetMarker());

				// Bind the object if the spacebar is pressed
				if (!!(GetAsyncKeyState(VK_SPACE) & 0x8000))
				{
					pDoc->BindNodeToSelected(pObject);
				}
				else
				{
					// Put the node under the active parent
					pDoc->BindNode(pObject, pRegion->GetActiveParentNode());
				}

				// Select the node
				pDoc->SelectNode(pObject);
			}
		}
	}
}


void ClassListDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	
}

//clears out the list of classes
void ClassListDlg::ClearAll()
{
	m_ClassList.DeleteAllItems();
}