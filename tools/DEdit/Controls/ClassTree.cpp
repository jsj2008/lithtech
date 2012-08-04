//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// ClassTree.cpp : implementation file
//

#include "bdefs.h"
#include "classtree.h"
#include "editprojectmgr.h"
#include "edithelpers.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClassTree

CClassTree::CClassTree()
{
}

CClassTree::~CClassTree()
{
}


BEGIN_MESSAGE_MAP(CClassTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CClassTree)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CClassTree custom functions

void CClassTree::UpdateContents(BOOL bIncludeTemplates)
{
	DWORD i;
	CProjectClass		*pRootClass;
	HTREEITEM			hRoot;
	
	// Get the project
	CEditProjectMgr *pProject=GetProject();

	// Delete all of the items
	DeleteAllItems();	

	// Add the root item.
	for(i=0; i < GetProject()->m_Classes; i++)
	{
		pRootClass = GetProject()->m_Classes[i];
		if(pRootClass)
		{
			hRoot = InsertItem( TVIF_TEXT|TVIF_PARAM, pRootClass->m_pClass->m_ClassName, 
						0, 0, 0, 0, (LPARAM)pRootClass->m_pClass, TVI_ROOT, TVI_SORT );

			AddItemsToTree( hRoot, pRootClass->m_Children );
		}
	}

	// Add the template classes
	if (bIncludeTemplates)
	{		
		for (i=0; i < pProject->GetTemplateClasses().GetSize(); i++)
		{
			// Get the template class
			TemplateClass *pTemplate=GetProject()->GetTemplateClasses().GetAt(i);
			
			if (pTemplate)
			{
				// Find the parent class
				HTREEITEM hParentItem=RecurseAndFindClass( GetRootItem(), pTemplate->m_ParentClassName);
				if (hParentItem)
				{
					// Insert the item
					InsertItem(TVIF_TEXT|TVIF_PARAM, pTemplate->m_ClassName, 0, 0, 0, 0, NULL, hParentItem, TVI_SORT);
				}
			}
		}
	}

	// Make sure that the root node is expanded
	Expand(GetRootItem(), TVE_EXPAND);
}


void CClassTree::AddItemsToTree( HTREEITEM hParent, CMoArray<CProjectClass*> &classes )
{
	HTREEITEM hCur;
	ClassDef *pClass;
	DWORD i;
	
	for(i=0; i < classes; i++ )
	{
		pClass = classes[i]->m_pClass;

		if(pClass->m_ClassFlags & CF_HIDDEN)
		{
			AddItemsToTree(hParent, classes[i]->m_Children);
		}
		else
		{
			hCur = InsertItem(TVIF_TEXT|TVIF_PARAM, pClass->m_ClassName, 0, 0, 0, 0, NULL, hParent, TVI_SORT);		
			AddItemsToTree(hCur, classes[i]->m_Children);
		}
	}
}

void CClassTree::SelectClass( const char *pName )
{
	HTREEITEM		hItem;
	
	if( GetRootItem() )
	{
		hItem = RecurseAndFindClass( GetRootItem(), pName );
		if( hItem )
		{
			EnsureVisible( hItem );
			SelectItem( hItem );
		}
	}
}


HTREEITEM CClassTree::RecurseAndFindClass( HTREEITEM hItem, const char *pName )
{	
	HTREEITEM		hTest, hChildTest;
	
	hTest = hItem;
	
	// Search the siblings.
	while(hTest)
	{		
		if( strcmp(pName, GetItemText(hTest)) == 0 )
		{
			return hTest;
		}
		
		hTest = GetNextSiblingItem( hTest );
	}

	// Search the children.
	hTest = GetChildItem( hItem );
	while( hTest )
	{
		hChildTest = RecurseAndFindClass( hTest, pName );
		if( hChildTest )
			return hChildTest;

		hTest = GetNextSiblingItem( hTest );
	}
	
	return NULL;
}


/************************************************************************/
// Returns the selected class
CString CClassTree::GetSelectedClass()
{
	HTREEITEM hItem=GetSelectedItem();

	if (hItem)
	{
		return GetItemText(hItem);
	}
	else
	{
		return "";
	}
}

/////////////////////////////////////////////////////////////////////////////
// CClassTree message handlers

BOOL CClassTree::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;
	
	return CTreeCtrl::PreCreateWindow(cs);
}

