#include "ltwintreemgr.h"
#include "ltwintreeitemiter.h"
#include "ltwintreeitem.h"

BEGIN_MESSAGE_MAP(CLTWinTreeMgr, CTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT,	OnBeginEditText)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT,		OnEndEditText)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG,		OnBeginDrag)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED,		OnSelChanged)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

//default constructor
CLTWinTreeMgr::CLTWinTreeMgr() :
	m_pRoot(NULL),
	m_bEditText(FALSE),
	m_bDragDrop(FALSE),
	m_bDragging(FALSE),
	m_pDragImage(NULL),
	m_pDragItem(NULL),
	m_bMultiSelect(FALSE),
	m_pfnChangeSel(NULL),
	m_nChangeSelUserData(0)
{
	//create the image list to hold the icons in
	m_ImageList.Create(16, 16, ILC_COLOR, 1, 1);
}

//default destructor
CLTWinTreeMgr::~CLTWinTreeMgr()
{
	DeleteTree();
}


//adds a newly created item to the tree after belonging to the specified parent
// If the parent is NULLITEM it is added as a sibling to the root. It will fail
// if it cannot find the appropriate parent. This object is then orphaned, and
// it is the tree manager's responsibility to release the memory. Due to this
// all the items MUST be created on the heap using new. The return value is the
// key for the new item, which uniquely identifies it. Keys CANNOT be
// transferred between trees
CLTWinTreeKey CLTWinTreeMgr::AddItem(	CLTWinTreeItem* pNewItem, 
										CLTWinTreeKey Parent,
										BOOL bSortChildren)
{
	//sanity check
	if(pNewItem == NULL)
	{
		return NULLITEM;
	}

	//first add into the tree heiarchy

	//create a pointer to the parent
	CLTWinTreeItem* pParent = KeyToItem(Parent);
	HTREEITEM hParent = TVI_ROOT;

	//see if it belongs to the root
	if(pParent == NULL)
	{
		//setup the item
		pNewItem->m_pNextSibling	= m_pRoot;
		pNewItem->m_pPrevSibling	= NULL;
		pNewItem->m_pParent			= NULL;
		pNewItem->m_pFirstChild		= NULL;

		//setup the back link from the first root item
		if(m_pRoot)
		{
			m_pRoot->m_pPrevSibling = pNewItem;
		}

		//setup the owner
		m_pRoot = pNewItem;
	}
	else
	{
		//doesn't belong to root, so add it to the parent item

		//setup the item
		pNewItem->m_pNextSibling	= pParent->m_pFirstChild;
		pNewItem->m_pPrevSibling	= NULL;
		pNewItem->m_pParent			= pParent;
		pNewItem->m_pFirstChild		= NULL;

		//setup the link of the first sibling back to this one
		if(pParent->m_pFirstChild)
		{
			pParent->m_pFirstChild->m_pPrevSibling = pNewItem;
		}

		//setup the owner
		pParent->m_pFirstChild = pNewItem;
		hParent = pParent->m_hTreeItem;
	}	

	//add it to the control, use different methods depending on
	//if it has an icon or not
	if(pNewItem->GetIcon() >= 0)
	{
		pNewItem->m_hTreeItem = InsertItem(	pNewItem->GetText(), 
										pNewItem->GetIcon(),
										pNewItem->GetIcon(),
										hParent);
	}
	else
	{
		pNewItem->m_hTreeItem = InsertItem(	pNewItem->GetText(), hParent);
	}

	//set the item data as a pointer to the list item
	SetItemData(pNewItem->m_hTreeItem, (DWORD)pNewItem);

	if(bSortChildren)
	{
		//make sure the children are in alphabetacal ordering
		SortChildren(hParent);
	}


	//now give the user the key back
	return ItemToKey(pNewItem);
}


//deletes a specified item from the tree as well as its children. Returns
//true if it successfully found and deleted the item, false otherwise
BOOL CLTWinTreeMgr::DeleteItem(CLTWinTreeKey Item)
{
	//sanity check
	if(Item == NULLITEM)
	{
		return FALSE;
	}

	//get a pointer to the actual object
	CLTWinTreeItem* pItem = KeyToItem(Item);

	//start off by recursively deleting all its children
	while(pItem->m_pFirstChild)
	{
		DeleteItem(ItemToKey(pItem->m_pFirstChild));
	}

	//unlink the item
	UnlinkFromSiblings(pItem);

	//remove the item from the actual control if the window is still valid
	//(this check needs to be done so it can be destroyed, but if it is called
	//after the window has been destroyed and the tree items already freed)
	if(m_hWnd)
	{
		CTreeCtrl::DeleteItem(pItem->m_hTreeItem);
	}

	delete pItem;
	return TRUE;
}


//creates an iterator that begins iterating through the root
CLTWinTreeItemIter* CLTWinTreeMgr::CreateIter()
{
	if(m_pRoot)
	{
		return m_pRoot->CreateSiblingIter();
	}
	return NULL;
}


//sets the text of an item given a key
BOOL CLTWinTreeMgr::SetItemText(CLTWinTreeKey Item, const char* pszText)
{
	//sanity check
	if(Item == NULLITEM)
	{
		return FALSE;
	}

	CLTWinTreeItem* pItem = KeyToItem(Item);

	//modify the actual object
	pItem->m_sText = pszText;

	//modify the control
	CTreeCtrl::SetItemText(pItem->m_hTreeItem, pszText);

	return TRUE;
}

//sets the icon of an item given a key
BOOL CLTWinTreeMgr::SetItemIcon(CLTWinTreeKey Item, CLTWinTreeIcon Icon)
{
	//sanity check
	if(Item == NULLITEM)
	{
		return FALSE;
	}

	CLTWinTreeItem* pItem = KeyToItem(Item);

	//modify the actual object
	pItem->m_Icon = Icon;

	//modify the control
	CTreeCtrl::SetItemImage(pItem->m_hTreeItem, Icon, Icon);

	return TRUE;
}



//converts a key to an item
CLTWinTreeItem* CLTWinTreeMgr::KeyToItem(CLTWinTreeKey Key)
{
	return (CLTWinTreeItem*)Key;
}


//converts an item to a key
CLTWinTreeKey CLTWinTreeMgr::ItemToKey(CLTWinTreeItem* pItem)
{
	return (CLTWinTreeKey)pItem;
}


//deletes the entire tree
void CLTWinTreeMgr::DeleteTree()
{
	//just delete every single item
	while(m_pRoot)
	{
		DeleteItem(ItemToKey(m_pRoot));
	}
}

//this will visit each node in turn, and call the passed calback
//in once per item. It will continue iteration as long as the
//function returns true and there are items left to be iterated.
//it performs a depth first search to get each item. In addition,
//the user data will be passed to the function each time it is
//called. It will begin at the passed in node and visit all of the
//children of that node. If it is null, it will start at the root
void CLTWinTreeMgr::VisitEachItem(VisitCallback CallbackFunc, DWORD UserData, CLTWinTreeKey Start)
{
	//first get the iterator
	CLTWinTreeItemIter* pIter;

	if(Start == NULLITEM)
	{
		//it is a root iterator we need
		pIter = CreateIter();
	}
	else
	{
		//it is a child iterator
		CLTWinTreeItem* pItem = KeyToItem(Start);
		pIter = pItem->CreateChildIter();
	}

	//make sure the iterator is valid
	if(pIter == NULL)
	{
		return;
	}

	//now visit each node
	for(; pIter->IsMore(); pIter->Next() )
	{
		CallbackFunc(pIter->Current(), this, UserData);
		//recursively visit the items
		VisitEachItem(CallbackFunc, UserData, ItemToKey(pIter->Current()));
	}

	//clean up
	delete pIter;
}

//adds an icon to the available list, and returns the index that
//will refer to the icon
int CLTWinTreeMgr::AddIcon(LPCTSTR pszIcon)
{
	//load the icon
	HICON hIcon = LoadIcon(AfxGetInstanceHandle(), pszIcon);

	//add it to the image list
	int rv = m_ImageList.Add(hIcon);

	//delete the icon
	DestroyIcon(hIcon);

	//make sure the image list is installed
	SetImageList(&m_ImageList, TVSIL_NORMAL);

	//return the index
	return rv;
}


//enables the dragging and dropping of items
void CLTWinTreeMgr::EnableDragDrop(BOOL bEnable)
{
	//modify the window flags
	if(bEnable)
	{
		ModifyStyle(TVS_DISABLEDRAGDROP, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);   
	}
	else
	{
		ModifyStyle(0, TVS_DISABLEDRAGDROP, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);   
	}

	//save the flag
	m_bDragDrop = bEnable;
}

//enables the editing of text in the control
void CLTWinTreeMgr::EnableEditText(BOOL bEnable)
{
	//modify the window flags
	if(bEnable)
	{
		ModifyStyle(0, TVS_EDITLABELS, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);   
	}
	else
	{
		ModifyStyle(TVS_EDITLABELS, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);   
	}

	//save the flag
	m_bEditText = bEnable;
}


void CLTWinTreeMgr::OnBeginEditText(NMHDR* pHdr, LRESULT* pResult)
{
	//get the correct pointer
	LPNMTVDISPINFO pTreeView = (LPNMTVDISPINFO)pHdr;

	//get the item it is pointing to
	CLTWinTreeItem* pItem = (CLTWinTreeItem*)(pTreeView->item.lParam);

	//if we can't edit the label, return TRUE to cancel it
	*pResult = (m_bEditText && pItem->IsTextEditable()) ? FALSE : TRUE;
}

void CLTWinTreeMgr::OnEndEditText(NMHDR* pHdr, LRESULT* pResult)
{
	//get the correct pointer
	LPNMTVDISPINFO pTreeView = (LPNMTVDISPINFO)pHdr;

	//get the item it is pointing to
	CLTWinTreeItem* pItem = (CLTWinTreeItem*)(pTreeView->item.lParam);

	//see if the user cancelled out of the operation
	if(pTreeView->item.pszText == NULL)
	{
		*pResult = FALSE;
	}
	else
	{
		//they didn't cancel out, so lets set the text
		SetItemText(ItemToKey(pItem), pTreeView->item.pszText);

		// let the parent know about the change
		GetParent()->SendMessage( WM_LTTREE_EDITTEXT, (WPARAM)this, (LPARAM)pItem );

		*pResult = TRUE;
	}
}

void CLTWinTreeMgr::OnBeginDrag(NMHDR* pHdr, LRESULT* pResult)
{
	if(m_bDragDrop)
	{
		ClearSelection();

		//get the correct pointer
		LPNMTREEVIEW pTreeView = (LPNMTREEVIEW)pHdr;

		//set the flag that we are dragging
		m_bDragging = TRUE;

		//save the item being dragged
		m_pDragItem = (CLTWinTreeItem*)(pTreeView->itemNew.lParam);

		//create the image list, and make sure it is valid
		m_pDragImage = CreateDragImage(pTreeView->itemNew.hItem);

		//capture the mouse to ensure we get the button up command
		SetCapture();

		if(m_pDragImage)
		{	
			//setup the drag image list
			if(m_pDragImage->BeginDrag(0, CPoint(0, 0)) == FALSE)
			{
				delete m_pDragImage;
				m_pDragImage = NULL;
			}
		}


	}
}

//returns a pointer to a valid drop target if one exists, otherwise
//NULL
CLTWinTreeItem*	CLTWinTreeMgr::GetDropTarget(CPoint pt)
{
	//make sure we are in a drag operation
	if(!IsInDrag())
	{
		return NULL;
	}

	//see what item we hit
	HTREEITEM hHit = HitTest(pt);

	if(hHit)
	{
		//we hit something, get the item
		CLTWinTreeItem* pItem = (CLTWinTreeItem*)GetItemData(hHit);

		//see if this is a drop target
		if(pItem->IsDropTarget(m_pDragItem))
		{
			//it is a drop target, but is it a child of the dragged item,
			//because you can't have them moving itself to a child
			if(m_pDragItem->ContainsObject(pItem) == FALSE)
			{
				return pItem;
			}
		}
	}

	//couldn't find a valid hit target
	return NULL;
}


void CLTWinTreeMgr::OnMouseMove(UINT nFlags, CPoint pt)
{
	CTreeCtrl::OnMouseMove(nFlags, pt);

	//make sure we are in a drag operation
	if(!IsInDrag())
	{
		return;
	}


	//hilite the drop target if one exists
	CLTWinTreeItem* pHit = GetDropTarget(pt);

	if(pHit)
	{
		//this is a valid drop target, so make it look like one
		SelectDropTarget(pHit->m_hTreeItem);
	}

	//move the drag image
	if(m_pDragImage)
	{
		m_pDragImage->DragMove(pt);
	}


}

void CLTWinTreeMgr::OnLButtonUp(UINT nFlags, CPoint pt)
{
	CTreeCtrl::OnLButtonUp(nFlags, pt);

	//make sure we are in a drag operation
	if(!IsInDrag())
	{
		return;
	}

	//see if we have a place to drop it
	CLTWinTreeItem* pHit = GetDropTarget(pt);

	if(pHit)
	{
		//we are moving it, so lets make the move

		//first erase the old tree
		CTreeCtrl::DeleteItem(m_pDragItem->m_hTreeItem);

		//unlink the item
		UnlinkFromSiblings(m_pDragItem);

		//now build the new tree  recursively
		RebuildDraggedTree(m_pDragItem, pHit);

		//select the moved item
		CTreeCtrl::SelectItem(m_pDragItem->m_hTreeItem);
	}

	//clear the drop target
	SelectDropTarget(NULL);

	//clean up the drag stuff
	m_bDragging		= FALSE;
	m_pDragItem		= NULL;

	//clean up the image list
	if(m_pDragImage)
	{
		m_pDragImage->EndDrag();
		delete m_pDragImage;
		m_pDragImage	= NULL;
	}

	//clean up the cursor
	ReleaseCapture();


}

//recursively builds a dragged tree as a parent to the passed in
//item.
void CLTWinTreeMgr::RebuildDraggedTree(CLTWinTreeItem* pItem, CLTWinTreeItem* pParent)
{
	//save the child pointer since it will be lost in add item
	CLTWinTreeItem* pChild = pItem->m_pFirstChild;

	//add the item to the parent
	AddItem(pItem, pParent);

	//now recurse on all children
	while(pChild)
	{
		//save the next node, since that will be lost in the function
		CLTWinTreeItem* pNext = pChild->m_pNextSibling;
		//recurse
		RebuildDraggedTree(pChild, pItem);
		//move on to the next item
		pChild = pNext;
	}
}

//unlinks the item from all surrounding siblings
void CLTWinTreeMgr::UnlinkFromSiblings(CLTWinTreeItem* pItem)
{
	//see if it is first in list
	if(pItem->m_pPrevSibling == NULL)
	{
		//it is, so we need to adjust the parent's first child ptr
		if(pItem->m_pParent)
		{
			//has a parent
			pItem->m_pParent->m_pFirstChild = pItem->m_pNextSibling;
		}
		else
		{
			//doesn't have a parent, belongs to the root, need to adjust
			//starting root node
			m_pRoot = pItem->m_pNextSibling;
		}
	}
	else
	{
		//has a previous sibling, link that to the next sibling
		pItem->m_pPrevSibling->m_pNextSibling = pItem->m_pNextSibling;
	}

	//now need to adjust the next sibling to point to the prev
	if(pItem->m_pNextSibling)
	{
		pItem->m_pNextSibling->m_pPrevSibling = pItem->m_pPrevSibling;
	}
}


void CLTWinTreeMgr::OnLButtonDown(UINT nFlags, CPoint pt)
{
	//determine if we are in multiple node selection. If
	//we aren't, just pass it back to the base class
	if(m_bMultiSelect == FALSE)
	{
		CTreeCtrl::OnLButtonDown(nFlags, pt);
		return;
	}

	//get the hit item
	HTREEITEM hHit = HitTest(pt);

	//make sure we hit something
	if(hHit == NULL)
	{
		ClearSelection();
		return;
	}

	CLTWinTreeItem* pItem = (CLTWinTreeItem*)GetItemData(hHit);
	

	if(pItem)
	{
		//see what modifiers are active

		//only go into the shift if there is already an item selected, and
		//if the two items are contained under the same parent
		if(	(nFlags & MK_SHIFT) && 
			m_pFirstSelected && 
			(pItem->m_pParent == m_pFirstSelected->m_pParent))
		{
			//get the handle to the parent tree item
			HTREEITEM hParent = (pItem->m_pParent) ? pItem->m_pParent->m_hTreeItem : TVI_ROOT;
			//get the first child
			HTREEITEM hCur = GetChildItem(hParent);

			//clear out the entire selection
			ClearSelection();

			BOOL bSelect = FALSE;

			//go through all children
			while(hCur)
			{
				//see if we have entered the range
				if((hCur == m_pFirstSelected->m_hTreeItem) || (hCur == hHit))
				{
					//we have, so toggle the flag
					bSelect = (bSelect) ? FALSE : TRUE;
					MultiSelectItem(hCur, TRUE);
				}
				else if(bSelect)
				{
					//we are in the range, so set the item to true
					MultiSelectItem(hCur, TRUE);
				}
				hCur = GetNextSiblingItem(hCur);
			}						
		}
		else if(nFlags & MK_CONTROL)
		{
			//add this item to the selection
			//or invert it if it is already selected
			CLTWinTreeItem* pItem = (CLTWinTreeItem*)GetItemData(hHit);

			if(pItem)
			{
				MultiSelectItem(hHit, (pItem->m_bSelected) ? FALSE : TRUE);
				//save the first selected if needed
				if(!m_pFirstSelected)
				{
					m_pFirstSelected = pItem;
				}
			}
		}
		else
		{
			//they just want to select a single one
			ClearSelection();
			MultiSelectItem(hHit, TRUE);
			//save the first selected
			m_pFirstSelected = (CLTWinTreeItem*)GetItemData(hHit);
		}
	}

	CTreeCtrl::OnLButtonDown(nFlags, pt);
}

//clears the selection, setting all item's flags for set to false
void CLTWinTreeMgr::ClearSelection()
{
	VisitEachItem(ClearSelectCallback, 0);
}

//callback function to visit each node and clear the selection flag
BOOL CLTWinTreeMgr::ClearSelectCallback(CLTWinTreeItem* pItem, CLTWinTreeMgr* pMgr, DWORD)
{
	if(pItem->m_bSelected)
	{
		pMgr->MultiSelectItem(pItem->m_hTreeItem, FALSE);
	}
	return TRUE;
}

// expand all items in the tree
void CLTWinTreeMgr::ExpandAll( void )
{
	VisitEachItem( ExpandCollapseCallback, 0 );
}

// collapse all items in the tree
void CLTWinTreeMgr::CollapseAll( void )
{
	VisitEachItem( ExpandCollapseCallback, 1 );
}

// callback funtion to visit each node and expand or collapse it (0=expand, 1=collapse)
BOOL CLTWinTreeMgr::ExpandCollapseCallback( CLTWinTreeItem* item, CLTWinTreeMgr* mgr, DWORD collapse )
{
	mgr->Expand( item->m_hTreeItem, collapse ? TVE_COLLAPSE : TVE_EXPAND );
	return TRUE;
}

//selects a specified node in multiple selection mode
void CLTWinTreeMgr::MultiSelectItem(HTREEITEM hItem, BOOL bSelect)
{
	//get the item
	if(hItem == NULL)
	{
		return;
	}

	CLTWinTreeItem* pItem = (CLTWinTreeItem*)GetItemData(hItem);

	if(pItem && (pItem->m_bSelected != bSelect))
	{

		//save it
		pItem->m_bSelected = bSelect;

		//set the flag to the appropriate item
		SetItemState(hItem, (bSelect) ? TVIS_SELECTED : 0, TVIS_SELECTED);

		//trigger the callback
		TriggerChangeSel(pItem);
	}
}

void CLTWinTreeMgr::OnSelChanged(NMHDR* pHdr, LRESULT* pResult)
{
	if(m_bMultiSelect)
	{
		//in multi select mode, we need to update the view
		//to reflect the selected nodes, otherwise all the
		//others are cleared
		VisitEachItem(UpdateItemCallback, 0);
		Invalidate();
		*pResult = FALSE;
	}
	else
	{
		//in single selection node, we just need to make sure the selected node
		//is tracked in the data tree

		//convert the pointer
		LPNMTREEVIEW pView = (LPNMTREEVIEW)pHdr;

		//clear out any that could have been selected before
		ClearSelection();

		//set the selected flag to true
		MultiSelectItem(pView->itemNew.hItem, TRUE);
	}		
}

//callback function to visit each node and update their drawing method
BOOL CLTWinTreeMgr::UpdateItemCallback(CLTWinTreeItem* pItem, CLTWinTreeMgr* pMgr, DWORD)
{
	//set the flag to the appropriate item
	pMgr->SetItemState(pItem->m_hTreeItem, (pItem->m_bSelected) ? TVIS_SELECTED : 0, TVIS_SELECTED);
	return TRUE;
}

//enables multiple selection of nodes
void CLTWinTreeMgr::EnableMultiSelect(BOOL bEnable)
{
	ClearSelection();
	m_bMultiSelect = bEnable;
}

//registers a callback for when selections change
void CLTWinTreeMgr::RegisterCallback(ChangeSelCallback Callback, DWORD nUserData)
{
	m_pfnChangeSel = Callback;
	m_nChangeSelUserData = nUserData;
}

//unregisters the associated callback
void CLTWinTreeMgr::UnregisterCallback()
{
	m_nChangeSelUserData = 0;
	m_pfnChangeSel = NULL;
}
//triggers the change selection callback
void CLTWinTreeMgr::TriggerChangeSel(CLTWinTreeItem* pItem)
{
	if(m_pfnChangeSel)
	{
		m_pfnChangeSel(pItem, this, m_nChangeSelUserData);
	}

	// send the message to the parent window
	GetParent()->SendMessage( WM_LTTREE_SELCHANGED, (WPARAM)this );
}


//determines if an item is currently being dragged
BOOL CLTWinTreeMgr::IsInDrag() const
{
	return (m_bDragging && m_pDragItem) ? TRUE : FALSE;
}
