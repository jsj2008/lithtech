// ObjectBrowserDlg.cpp : implementation file
//

#include "bdefs.h"
#include "..\dedit.h"
#include "regiondoc.h"
#include "editregion.h"
#include "edithelpers.h"
#include "objectbrowserdlg.h"
#include "mainfrm.h"
#include "nodeview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////
// Structure to hold data about an item
class CBrowseObjectData
{
public:
	CBrowseObjectData(CBaseEditObj* pObj = NULL, CPrefabRef* pPrefab = NULL) :
		m_pPrefab(pPrefab),
		m_pObject(pObj)
	{
	}

	//the prefab that holds this object (NULL if none)
	CPrefabRef*		m_pPrefab;

	//the object in question
	CBaseEditObj*	m_pObject;
};



/////////////////////////////////////////////////////////////////////////////
// CObjectBrowserDlg dialog


CObjectBrowserDlg::CObjectBrowserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectBrowserDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectBrowserDlg)
	m_bGroupByType = TRUE;
	//}}AFX_DATA_INIT

	m_bIsDialogValid=FALSE;
	m_pSelectedObject=NULL;
}


void CObjectBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectBrowserDlg)
	DDX_Control(pDX, IDC_TREE_OBJECTS, m_treeObjects);
	DDX_Control(pDX, IDC_LIST_OBJECTS, m_listObjects);
	DDX_Check(pDX, IDC_CHECK_GROUP_BY_TYPE, m_bGroupByType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CObjectBrowserDlg, CDialog)
	//{{AFX_MSG_MAP(CObjectBrowserDlg)
	ON_BN_CLICKED(IDC_CHECK_GROUP_BY_TYPE, OnCheckGroupByType)
	ON_LBN_SELCHANGE(IDC_LIST_OBJECTS, OnSelchangeListObjects)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_OBJECTS, OnSelchangedTreeObjects)
	ON_NOTIFY(TVN_SELCHANGING, IDC_TREE_OBJECTS, OnSelchangingTreeObjects)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_SELECT, OnButtonSelect)
	ON_BN_CLICKED(IDC_BUTTON_LOCATE, OnButtonLocate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectBrowserDlg message handlers


BOOL CObjectBrowserDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// The dialog is valid
	m_bIsDialogValid=TRUE;

	//clear the selection
	m_pSelectedObject = NULL;
	m_pSelectedPrefab = NULL;

	// Get the active region doc
	CRegionDoc *pDoc=GetActiveRegionDoc();
	ASSERT(pDoc);
	
	// Get the active region
	CEditRegion *pRegion=pDoc->GetRegion();
	ASSERT(pRegion);

	RecurseAddObjects(pRegion->GetRootNode(), "", NULL);
	
	// Make sure that the root is expanded
	m_treeObjects.Expand(TVI_ROOT, TVE_EXPAND);

	// Update the control states
	UpdateControlStates();

	// Select the selected object
	if(!m_sSelectedName.IsEmpty())
	{
		//select the object with the name
		SetSelectedObject(m_sSelectedName);
	}

	// Hide the controls that are to be invisible
	for (int i=0; i < m_controlInvisibleArray.GetSize(); i++)
	{
		GetDlgItem(m_controlInvisibleArray[i])->ShowWindow(SW_HIDE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/************************************************************************/
// Sets whether or not the specified control should be visible or not
void CObjectBrowserDlg::SetControlVisible(int nID, BOOL bVisible)
{
	// If the dialog is valid, then hide or show the control
	if (m_bIsDialogValid)
	{
		int nShowCmd;
		if (bVisible)
		{
			nShowCmd=SW_SHOW;
		}
		else
		{
			nShowCmd=SW_HIDE;
		}

		GetDlgItem(nID)->ShowWindow(nShowCmd);
	}
	else
	{
		// If we are hiding this control, add it to the list to be
		// hidden upon initializing the dialog
		if (!bVisible)
		{
			m_controlInvisibleArray.Add((DWORD)nID);
		}
	}
}

/************************************************************************/
// The window is about to be destroyed
void CObjectBrowserDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	m_bIsDialogValid=FALSE;	
}

/************************************************************************/
// Selects an object based on its name
void CObjectBrowserDlg::SetSelectedObject(const CString& sObjectName)
{
	if (m_bIsDialogValid)
	{	
		// Disable these controls by default
		GetDlgItem(IDC_BUTTON_LOCATE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SELECT)->EnableWindow(FALSE);		

		RecurseSelectObject(m_treeObjects.GetRootItem(), sObjectName);

		int nNumItems = m_listObjects.GetCount();
		// Search the list of objects for an object with the specified name
		for (int i=0; i < nNumItems; i++)
		{
			// Compare the object name with the specified name
			CString sName;
			m_listObjects.GetText(i, sName);
			if (sName.CompareNoCase(sObjectName) == 0)
			{
				m_listObjects.SetCurSel(i);
				m_pSelectedObject = GetListObject(i);
				m_pSelectedPrefab = GetListPrefab(i);
				break;
			}
		}

		if(m_pSelectedObject || m_pSelectedPrefab)
		{
			// Enable the ok, view, and select buttons
			GetDlgItem(IDC_BUTTON_LOCATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_SELECT)->EnableWindow(TRUE);		
		}
	}

	m_sSelectedName = sObjectName;
}

/************************************************************************/
// Updates the control states (visible and enabled) based on the
// options that are selected.
void CObjectBrowserDlg::UpdateControlStates()
{
	// Update the object data
	UpdateData();
	
	// Determine if the objects should be grouped by their type in a tree
	if (m_bGroupByType)
	{
		// Display the appropriate controls
		m_treeObjects.ShowWindow(SW_SHOW);
		m_listObjects.ShowWindow(SW_HIDE);
	}
	else
	{
		// Display the appropriate controls
		m_treeObjects.ShowWindow(SW_HIDE);
		m_listObjects.ShowWindow(SW_SHOW);
	}
}

/************************************************************************/
// The "group by type" box has been checked
void CObjectBrowserDlg::OnCheckGroupByType() 
{
	UpdateControlStates();
}

/************************************************************************/
// The listbox selection has changed
void CObjectBrowserDlg::OnSelchangeListObjects() 
{
	// Get the currently selected index
	int nIndex=m_listObjects.GetCurSel();
	if (nIndex != LB_ERR)
	{
		CString sText;
		m_listObjects.GetText(nIndex, sText);

		// Select the object
		SetSelectedObject(sText);				
	}
}

/************************************************************************/
// Selects a specific object in the tree.  Returns TRUE if the object
// has been selected.  The tree is searched recursively.
BOOL CObjectBrowserDlg::RecurseSelectObject(HTREEITEM hParentItem, const CString& sName)
{
	// Check this node
	if (m_treeObjects.GetItemText(hParentItem).CompareNoCase(sName) == 0)
	{
		m_treeObjects.SelectItem(hParentItem);
		m_treeObjects.EnsureVisible(hParentItem);

		return TRUE;
	}

	// Check the sibling nodes
	HTREEITEM hSiblingNode=m_treeObjects.GetNextSiblingItem(hParentItem);
	while (hSiblingNode)
	{
		// Check this node
		if (m_treeObjects.GetItemText(hSiblingNode).CompareNoCase(sName) == 0)
		{
			m_treeObjects.SelectItem(hSiblingNode);
			m_treeObjects.EnsureVisible(hSiblingNode);

			return TRUE;
		}
		
		// Check the children nodes
		if (m_treeObjects.ItemHasChildren(hSiblingNode))
		{
			// Recurse into the child node
			if (RecurseSelectObject(m_treeObjects.GetChildItem(hSiblingNode), sName))
			{
				return TRUE;
			}
		}

		// Get the next sibling
		hSiblingNode=m_treeObjects.GetNextSiblingItem(hSiblingNode);
	}

	// Check the children nodes
	if (m_treeObjects.ItemHasChildren(hParentItem))
	{
		// Recurse into the child node
		if (RecurseSelectObject(m_treeObjects.GetChildItem(hParentItem), sName))
		{
			return TRUE;
		}
	}

	// The node was not found
	return FALSE;
}

/************************************************************************/
// The selection has changed on the tree control
void CObjectBrowserDlg::OnSelchangedTreeObjects(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	// Get the item of the newly selected object
	HTREEITEM hNewItem=pNMTreeView->itemNew.hItem;

	// Select the object
	SetSelectedObject(m_treeObjects.GetItemText(hNewItem));
	
	*pResult = 0;
}

/************************************************************************/
// The selection is about to change on the tree control
void CObjectBrowserDlg::OnSelchangingTreeObjects(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	// Get the new item
	HTREEITEM hNewItem=pNMTreeView->itemNew.hItem;

	// Check to see if it is under the root item.  If it is, then prevent the selection
	// from taking place.
	if (m_treeObjects.GetParentItem(hNewItem) == NULL)
	{
		// Prevent the selection from happening
		*pResult=TRUE;
	}
	else
	{		
		*pResult = 0;
	}
}

/************************************************************************/
void CObjectBrowserDlg::OnButtonSelect() 
{	
	// Get the active region doc
	CRegionDoc *pDoc=GetActiveRegionDoc();
	ASSERT(pDoc);

	// Get the active region
	CEditRegion *pRegion=pDoc->GetRegion();
	ASSERT(pRegion);

	// Check to see if this node is already selected
	CWorldNode *pSelectedObject = GetSelectedPrefab();
	if(!pSelectedObject)
		pSelectedObject = GetSelectedObject();

	if(!pSelectedObject)
		return;

	int i;
	for (i=0; i < pRegion->GetNumSelections(); i++)
	{
		if (pSelectedObject == pRegion->GetSelection(i))
		{
			// This node is already selected so we don't need
			// to continue on with the selection operation.
			return;
		}
	}

	// Determine if multiselect is on
	BOOL bMultiSelect=FALSE;
	if (GetMainFrame()->GetNodeSelectionMode()==MULTI_SELECTION)
	{
		bMultiSelect=TRUE;
	}

	// Select the node
	pDoc->SelectNode(pSelectedObject, !bMultiSelect);

	// Highlight the node in the node view
	GetNodeView()->HighlightNode(pSelectedObject);
}

/************************************************************************/
// This locates the selected object by centering the green crosshair on
// it and centering the parallel views on the green crosshair.
void CObjectBrowserDlg::OnButtonLocate() 
{
	// Get the active region doc
	CRegionDoc *pDoc=GetActiveRegionDoc();
	ASSERT(pDoc);

	// Get the active region
	CEditRegion *pRegion=pDoc->GetRegion();
	ASSERT(pRegion);

	// Get the selected object
	CWorldNode *pObject = GetSelectedPrefab();
	if(!pObject)
		pObject = GetSelectedObject();

	if(!pObject)
		return;

	// Center the marker on the selected object
	pRegion->m_vMarker=pObject->GetPos();

	// Center the views on the marker
	GetMainFrame()->CenterViewsAtVector(pRegion->m_vMarker, TRUE);
}

//cleans up all the data associated with the tree that we allocated
void CObjectBrowserDlg::CleanUpTreeUserData(HTREEITEM hItem)
{
	//bail out on invalid objects...
	if(hItem == NULL)
		return;

	//free the user data with this object...
	if(hItem != TVI_ROOT)
	{
		CBrowseObjectData* pData = (CBrowseObjectData*)m_treeObjects.GetItemData(hItem);

		//clean it up
		delete pData;

		//and now clear out that info...
		m_treeObjects.SetItemData(hItem, 0);
	
		//now move along onto our siblings
		CleanUpTreeUserData(m_treeObjects.GetNextSiblingItem(hItem));
	}

	//recurse into the children...
	CleanUpTreeUserData(m_treeObjects.GetChildItem(hItem));

}

void CObjectBrowserDlg::CleanUpUserData()
{
	 CleanUpTreeUserData(TVI_ROOT);

	 //free all of the list items
	 for(int nItem = 0; nItem < m_listObjects.GetCount(); nItem++)
	 {
		 //clean up this item
		CBrowseObjectData* pData = (CBrowseObjectData*)m_listObjects.GetItemData(nItem);

		//clean it up
		delete pData;

		//and now clear out that info...
		m_listObjects.SetItemData(nItem, 0);
	 }
}

void CObjectBrowserDlg::OnOK()
{
	CleanUpUserData();
	CDialog::OnOK();
}

void CObjectBrowserDlg::OnCancel()
{
	CleanUpUserData();
	CDialog::OnCancel();
}

//inserts an object into the list control
void CObjectBrowserDlg::InsertObjectIntoList(const CString& sPrepend, CBaseEditObj* pObject, CPrefabRef* pPrefab)
{
	// Add the string
	int nIndex = m_listObjects.AddString(sPrepend + pObject->GetName());

	// Associate this index with the object
	m_listObjects.SetItemData(nIndex, (DWORD)new CBrowseObjectData(pObject, pPrefab));
}

//inserts an object into the tree control
void CObjectBrowserDlg::InsertObjectIntoTree(const CString& sPrepend, CBaseEditObj* pObject, CPrefabRef* pPrefab)
{
	// Find the parent node for this object
	HTREEITEM hParentItem=NULL;

	// Start with the first sibling of the root
	HTREEITEM hSibling= m_treeObjects.GetChildItem(TVI_ROOT);
	while (hSibling)
	{
		// Compare the items label to the class name for this object			
		if (m_treeObjects.GetItemText(hSibling) == pObject->GetClassName())
		{
			hParentItem=hSibling;
			break;
		}

		// Go to the next sibling
		hSibling=m_treeObjects.GetNextSiblingItem(hSibling);
	}

	// If a parent wasn't found, then add the parent item to the root
	if (hParentItem == NULL)
	{
		// Add the new item
		hParentItem=m_treeObjects.InsertItem(pObject->GetClassName(), TVI_ROOT, TVI_SORT);

		// Make the parent item bold
		m_treeObjects.SetItemState(hParentItem, TVIS_BOLD, TVIS_BOLD );			
	}

	// Add this object underneith the parent
	HTREEITEM hItem=m_treeObjects.InsertItem(sPrepend + pObject->GetName(), hParentItem, TVI_SORT);		

	// Associate this item with the object
	m_treeObjects.SetItemData(hItem, (DWORD)new CBrowseObjectData(pObject, pPrefab));
}

//recursively searches the tree for prefabs, and recurses into their tree adding objects
void CObjectBrowserDlg::RecurseAddObjects(CWorldNode* pNode, const CString& sPrepend, CPrefabRef* pPrefab)
{
	//handle NULL nodes that can sometimes occur
	if(!pNode)
		return;

	//see if this object is a prefab ref
	if(pNode->GetType() == Node_PrefabRef)
	{
		//add our name onto the string
		CString sNewPrepend = sPrepend;
		sNewPrepend += pNode->GetName();
		sNewPrepend += ".";

		CPrefabRef* pNewPrefab = (CPrefabRef*)pNode;

		//add objects, but only update the prefab object if we aren't already inside of one, 
		//since we can't select the inner prefab object
		RecurseAddObjects((CWorldNode*)pNewPrefab->GetPrefabTree(), sNewPrepend, (pPrefab) ? pPrefab : pNewPrefab);
	}
	//see if this object is a node
	else if(pNode->GetType() == Node_Object)
	{
		InsertObjectIntoList(sPrepend, pNode->AsObject(), pPrefab);
		InsertObjectIntoTree(sPrepend, pNode->AsObject(), pPrefab);
	}

	//recurse into children
	GPOS Pos = pNode->m_Children;
	while(Pos)
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(Pos);

		RecurseAddObjects(pChild, sPrepend, pPrefab);
	}
}

//easy accessors for accessing the data of an item from the list or tree
CBaseEditObj* CObjectBrowserDlg::GetListObject(int nIndex)
{
	CBrowseObjectData* pData = (CBrowseObjectData*)m_listObjects.GetItemData(nIndex);
	if(pData)
		return pData->m_pObject;
	return NULL;
}

CBaseEditObj* CObjectBrowserDlg::GetTreeObject(HTREEITEM hItem)
{
	CBrowseObjectData* pData = (CBrowseObjectData*)m_treeObjects.GetItemData(hItem);
	if(pData)
		return pData->m_pObject;
	return NULL;
}

CPrefabRef* CObjectBrowserDlg::GetListPrefab(int nIndex)
{
	CBrowseObjectData* pData = (CBrowseObjectData*)m_listObjects.GetItemData(nIndex);
	if(pData)
		return pData->m_pPrefab;
	return NULL;
}

CPrefabRef* CObjectBrowserDlg::GetTreePrefab(HTREEITEM hItem)
{
	CBrowseObjectData* pData = (CBrowseObjectData*)m_treeObjects.GetItemData(hItem);
	if(pData)
		return pData->m_pPrefab;
	return NULL;
}


