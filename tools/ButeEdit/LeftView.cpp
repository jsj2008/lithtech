// LeftView.cpp : implementation of the CLeftView class
//

#include "stdafx.h"
#include "ButeEdit.h"

#include "ButeEditDoc.h"
#include "LeftView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	//{{AFX_MSG_MAP(CLeftView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CTreeView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CTreeView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeftView construction/destruction

CLeftView::CLeftView()
{
	// TODO: add construction code here

}

CLeftView::~CLeftView()
{
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CLeftView drawing

void CLeftView::OnDraw(CDC* pDC)
{
	CButeEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


/////////////////////////////////////////////////////////////////////////////
// CLeftView printing

BOOL CLeftView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLeftView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CLeftView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CLeftView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	// Populate the tree control
	BuildTreeControl();	
}

/************************************************************************/
// Build the tree control
void CLeftView::BuildTreeControl()
{
	// Get the document
	CButeEditDoc* pDoc = GetDocument();

	// Make sure that there isn't an item selected
	GetTreeCtrl().SelectItem(NULL);

	// Delete all of the items
	GetTreeCtrl().SetRedraw(FALSE);
	GetTreeCtrl().DeleteAllItems();

	int i;
	for (i=0; i < pDoc->GetNumTags(); i++)
	{
		// Insert the item
		GetTreeCtrl().InsertItem(pDoc->GetTag(i));
	}
	GetTreeCtrl().SortChildren(NULL);
	GetTreeCtrl().SetRedraw(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CButeEditDoc* CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CButeEditDoc)));
	return (CButeEditDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLeftView message handlers

// The tree control changed its focus
void CLeftView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	// Get the document
	CButeEditDoc* pDoc = GetDocument();
	
	// Get the name of the selected tag
	HTREEITEM hItem=GetTreeCtrl().GetSelectedItem();
	if (hItem)
	{
		CString sItemName=GetTreeCtrl().GetItemText(hItem);

		// Select the item
		pDoc->SetSelectedTag(sItemName);
	}
	else
	{
		pDoc->SetSelectedTag("");
	}
	
	// Update the views
	pDoc->UpdateAllViews(this);

	*pResult = 0;
}

/************************************************************************/
// Selects a specific tag in the tree control
void CLeftView::SelectTag(CString sTagName)
{
	// Find the item
	HTREEITEM hItem=FindItem(NULL, sTagName);

	// Select the item
	if (hItem)
	{
		GetTreeCtrl().EnsureVisible(hItem);
		GetTreeCtrl().SelectItem(hItem);
	}
}

/************************************************************************/
// Finds an	item in the tree control based on its name
HTREEITEM CLeftView::FindItem(HTREEITEM hParent, CString sName)
{
	// Get the tree control
	CTreeCtrl &treeCtrl=GetTreeCtrl();

	// Check the name if this item
	if (hParent)
	{
		// Get the text for this item
		CString sParent=treeCtrl.GetItemText(hParent);
		if (sParent == sName)
		{
			return hParent;
		}
	}

	// Search the children nodes
	HTREEITEM hChild=treeCtrl.GetChildItem(hParent);
	if (hChild)
	{
		// Check the child item
		HTREEITEM hReturn=FindItem(hChild, sName);

		// Check to see if a valid item was returned
		if (hReturn)
		{
			return hReturn;
		}

		// Check the siblings of the child
		HTREEITEM hSibling=treeCtrl.GetNextSiblingItem(hChild);
		while (hSibling)
		{
			// Seach the siblings			
			HTREEITEM hReturn=FindItem(hSibling, sName);

			// Check to see if a valid item was returned
			if (hReturn)
			{
				return hReturn;
			}

			hSibling=treeCtrl.GetNextSiblingItem(hSibling);
		}
	}

	// The item was not found
	return NULL;
}
