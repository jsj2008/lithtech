//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// NodeViewTreeCtrl.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "nodeviewtreectrl.h"
#include "worldnode.h"
#include "nodeview.h"
#include "regiondoc.h"
#include "mainfrm.h"
#include "regionview.h"
#include "optionsmisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNodeViewTreeCtrl

CNodeViewTreeCtrl::CNodeViewTreeCtrl()
{
	m_pNodeView=NULL;
	m_bEditingLabel=FALSE;
}

CNodeViewTreeCtrl::~CNodeViewTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CNodeViewTreeCtrl, CMyTreeCtrl)
	//{{AFX_MSG_MAP(CNodeViewTreeCtrl)
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndLabelEdit)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemExpanded)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_PASTE_ALTERNATE, OnEditPasteAlternate)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_POPUP_RENAME, OnPopupRename)
	ON_COMMAND(ID_SELECTION_GROUP, OnSelectionGroup)
	ON_COMMAND(ID_POPUP_SETACTIVEPARENT, OnPopupSetactiveParent)
	ON_COMMAND(ID_POPUP_REFRESH, OnPopupRefresh)
	ON_COMMAND(ID_GOTO_NEXT_SELECTED_NODE, OnGotoNextSelectedNode)
	ON_COMMAND(ID_POPUP_COLLAPSE_CHILDREN, OnPopupCollapseChildren)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNodeViewTreeCtrl message handlers

//------------------------------------------------------------------------------------------
//
//  CNodeViewTreeCtrl::OnButtonUp
//
//  Purpose:	After dragging a node in the tree and releasing the mouse button, the 
//				dragged node's parent is changed to its new parent.
//
//------------------------------------------------------------------------------------------
void CNodeViewTreeCtrl::OnButtonUp()
{
	// Only proceed if user was dragging a node...
	if (m_bDragging)
	{
		KillTimer( m_nTimerIDScroll );		

		// Clean up some temporary images...
		ASSERT(m_pimagelist != NULL);
		m_pimagelist->DragLeave(this);
		m_pimagelist->EndDrag();
		delete m_pimagelist;
		m_pimagelist = NULL;

		// If the user drags to an illegal parent, beep...
		if( m_hitemDrag == m_hitemDrop || m_hitemDrop == GetParentItem( m_hitemDrag ) || 
			IsChildNodeOf( m_hitemDrop, m_hitemDrag ))
		{
			MessageBeep( 0 );
		}
		// If node dragged into empty space, then make root node its parent...
		else if( m_hitemDrop == NULL )
		{
			CNodeView *pNodeView;
			// Get the Node View dialog...
			pNodeView = ( CNodeView * )GetParent( );
			ASSERT( pNodeView );

			MoveItem( m_hitemDrag, 
				pNodeView->GetDoc( )->GetRegion( )->GetRootNode( )->GetItem( ));
		}
		// Else, make new node the parent...
		else
		{
			MoveItem( m_hitemDrag, m_hitemDrop );
		}
		
		m_bDragging = FALSE;
		ReleaseCapture();
		SelectDropTarget(NULL);
	}
}

void CNodeViewTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp();
	CTreeCtrl::OnLButtonUp(nFlags, point);
}

void CNodeViewTreeCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	OnButtonUp();
	CTreeCtrl::OnRButtonUp(nFlags, point);
}

//------------------------------------------------------------------------------------------
//
//  CNodeViewTreeCtrl::MoveItem
//
//  Purpose:	Moves a node and its children to a new parent.
//
//------------------------------------------------------------------------------------------
void CNodeViewTreeCtrl::MoveItem( HTREEITEM hNode, HTREEITEM hParent )
{
	CNodeView *pNodeView;
	CWorldNode *pNode, *pParentNode;

	// Get the Node View dialog...
	pNodeView = ( CNodeView * )GetParent( );
	ASSERT( pNodeView );

	// Get the data of the node and the new parent...
	pNode = ( CWorldNode * )GetItemData( hNode );
	pParentNode = ( CWorldNode * )GetItemData( hParent );
	ASSERT( pNode );
	ASSERT( pParentNode );

	if( pNode == pNodeView->GetDoc( )->GetRegion( )->GetRootNode( ))
	{
		MessageBeep( 0 );
		return;
	}

	// Attach node to new parent...
	pNodeView->GetDoc( )->Modify(new CPreAction(ACTION_MODIFYNODE, pNode), TRUE);

	pNodeView->GetDoc( )->GetRegion( )->AttachNode( pNode, pParentNode );
	pNodeView->GetDoc( )->UpdateAllViews( NULL );

	ASSERT(pNodeView->GetDoc()->GetRegion()->CheckNodes() == NULL);
}

//------------------------------------------------------------------------------------------
//
//  CNodeViewTreeCtrl::TransferItem
//
//  Purpose:	Overrides CMyTreeCtrl::TransferItem.  This was needed so the m_pNode
//				pointers could be updated.
//
//------------------------------------------------------------------------------------------
HTREEITEM CNodeViewTreeCtrl::TransferItem(HTREEITEM hitemDrag, HTREEITEM hitemDrop)
{
	HTREEITEM hNewItem;
	CWorldNode *pNode;

	hNewItem = CMyTreeCtrl::TransferItem( hitemDrag, hitemDrop );
	pNode = ( CWorldNode * )GetItemData( hNewItem );
	pNode->SetItem( hNewItem );

	return hNewItem;
}

//------------------------------------------------------------------------------------------
//
//  CNodeViewTreeCtrl::RecurseSetItemImage
//
//  Purpose:	Sets the tree items image and can recurse into children.
//
//------------------------------------------------------------------------------------------
void CNodeViewTreeCtrl::RecurseSetItemImage(HTREEITEM hItem, BOOL bRecurse, CWorldNode *pActiveParentNode)
{
	HTREEITEM hChild;
		
	// Get the active parent node if needed	
	if (pActiveParentNode == NULL)
	{
		// Get the region
		CNodeView *pNodeView = (CNodeView *)GetParent( );		
		CRegionDoc *pDoc=pNodeView->GetDoc();

		if (pDoc)
		{
			CEditRegion *pRegion=pDoc->GetRegion( );
			pActiveParentNode=pRegion->GetActiveParentNode();
		}
	}

	SetNodeImage( hItem, pActiveParentNode );

	// Check if recursion is requested and possible...
	if(( hChild = GetChildItem( hItem )) == NULL || !bRecurse )
	{
		return;
	}

	// Recurse through children...
	do
	{
		RecurseSetItemImage( hChild, bRecurse, pActiveParentNode );
	}
	while(( hChild = GetNextItem( hChild, TVGN_NEXT )) != NULL);
}

/************************************************************************/
// This function collapses a nodes children
void CNodeViewTreeCtrl::CollapseChildren(HTREEITEM hItem, BOOL bRecurseChildren, BOOL bFirstItem)
{
	// Don't collapse the first node
	if (!bFirstItem)
	{		
		// Collapse the node
		Expand(hItem, TVE_COLLAPSE);

		// Get the node
		CWorldNode *pNode=(CWorldNode *)GetItemData(hItem);
		
		// Clear the expand flag
		pNode->ClearFlag(NODEFLAG_EXPANDED);
	}

	// Collapse the children nodes
	HTREEITEM hChild=GetChildItem(hItem);
	if (!hChild)
	{
		return;
	}

	do
	{
		// If we aren't recursing then just collapse the node
		if (!bRecurseChildren)
		{
			Expand(hChild, TVE_COLLAPSE);

			// Get the node
			CWorldNode *pNode=(CWorldNode *)GetItemData(hChild);
			
			// Clear the expand flag
			pNode->ClearFlag(NODEFLAG_EXPANDED);
		}
		else
		{
			// Recurse through the children and collapse them
			CollapseChildren(hChild, bRecurseChildren, FALSE);
		}

		// Get the next child
		hChild = GetNextItem(hChild, TVGN_NEXT);
	}
	while(hChild != NULL);
}

//------------------------------------------------------------------------------------------
//
//  CNodeViewTreeCtrl::SetNodeImage
//
//  Purpose:	Sets the image based on the node type.
//
//------------------------------------------------------------------------------------------
void CNodeViewTreeCtrl::SetNodeImage( HTREEITEM hItem, CWorldNode *pActiveParentNode )
{
	// Get the region
	CNodeView *pNodeView = (CNodeView *)GetParent( );
	CRegionDoc *pDoc=pNodeView->GetDoc();

	if (pDoc)
	{
		CEditRegion *pRegion=pDoc->GetRegion( );

		// Get the active parent node if needed	
		if (pActiveParentNode == NULL)
		{
			pActiveParentNode=pRegion->GetActiveParentNode();
		}
	}
	
	CWorldNode *pNode;
	node_image nImage;

	pNode = ( CWorldNode *)GetItemData( hItem );
	ASSERT( pNode );

	// Get the correct image...
	switch( pNode->GetType( ))
	{
		case Node_Object:
			nImage = NODE_OBJECT;
			break;
		case Node_Brush:
			nImage = NODE_BRUSH;
			break;
		case Node_PrefabRef:
			nImage = NODE_PREFABREF;
			break;
		default:
			nImage = NODE_NULL;
			break;
	}

	if(pNode->IsFlagSet(NODEFLAG_PATH))
	{
		nImage = NODE_PATH;
	}

	// Set new image...
	SetItemImage( hItem, nImage, nImage );

	// Set the overlay
	UINT mask = NODE_MASK_UNSELECTED;

	if( pNode->IsFlagSet( NODEFLAG_HIDDEN ))
	{
		if( pNode->IsFlagSet(NODEFLAG_FROZEN))
			mask = NODE_MASK_FROZEN_AND_HIDDEN;
		else if( pNode->IsFlagSet(NODEFLAG_SELECTED))
			mask = NODE_MASK_SELECTED_AND_HIDDEN;
		else
			mask = NODE_MASK_HIDDEN;
	}
	else
	{
		if( pNode->IsFlagSet(NODEFLAG_FROZEN))
			mask = NODE_MASK_FROZEN;
		else if( pNode->IsFlagSet(NODEFLAG_SELECTED))
			mask = NODE_MASK_SELECTED;
	}		
	
	// Set the image state
	SetItemState( hItem, INDEXTOSTATEIMAGEMASK( mask ), TVIS_STATEIMAGEMASK );

	// Set the bold state if this node is the active parent
	if (pNode == pActiveParentNode)
	{
		SetItemBold(hItem, TRUE);
	}
	else
	{
		SetItemBold(hItem, FALSE);
	}
}

//------------------------------------------------------------------------------------------
//
//  CNodeViewTreeCtrl::SelectTree
//
//  Purpose:	Selects a tree of nodes.
//
//------------------------------------------------------------------------------------------
void CNodeViewTreeCtrl::SelectTree( HTREEITEM hItem, bool bRecurse, bool bMultiSelect, bool bNotifySelectionChange )
{
	CWorldNode *pNode;
	CRegionDoc *pDoc;

	if( hItem == NULL )
		return;

	pDoc = (( CNodeView * )GetParent( ))->GetDoc( );
	ASSERT( pDoc );
	pNode = ( CWorldNode *)GetItemData( hItem );
	ASSERT( pNode );

	// Not allowed to select the root node...
	// Selecting whole tree from the root node means select all except the root...
	if( pNode == pDoc->GetRegion( )->GetRootNode( ) && !bRecurse )
	{
		MessageBeep( 0 );
		return;
	}

	// Do unselect all if items already selected...
	if(( pNode == pDoc->GetRegion( )->GetRootNode( )) && ( pDoc->GetRegion( )->m_Selections.GetSize( )))
	{
		pDoc->GetRegion( )->RecurseAndUnselect( pNode );
	}
	else
	{
		// Select the node and its children...
		pDoc->GetRegion()->DoSelectionOperation( pNode, 
				(GetMainFrame()->GetNodeSelectionMode()==MULTI_SELECTION) || bMultiSelect,
				bRecurse );

		// Make sure root is not selected...
		if( pNode == pDoc->GetRegion( )->GetRootNode( ))
		{
			pDoc->GetRegion()->UnselectNode( pNode );
		}

	}

	// Update the inclusive selection box around all selections...
	if(bNotifySelectionChange)
		pDoc->NotifySelectionChange();

	// Scroll tree view to newest selection...
	Select( hItem, TVGN_CARET );
}

void CNodeViewTreeCtrl::SelectTree( CWorldNode *pNode, bool bRecurse, bool bMultiSelect, bool bNotifySelectionChange )
{
//	SelectTree( FindItem( GetRootItem( ), pNode ), bRecurse, bMultiSelect, bNotifySelectionChange );
	SelectTree( pNode->GetItem( ), bRecurse, bMultiSelect, bNotifySelectionChange );
}

//------------------------------------------------------------------------------------------
//
//  CNodeViewTreeCtrl::FindItem
//
//  Purpose:	Finds tree item from node.
//
//------------------------------------------------------------------------------------------
HTREEITEM CNodeViewTreeCtrl::FindItem( HTREEITEM hItem, CWorldNode *pNode )
{
	HTREEITEM hChildItem, hFoundItem;

	// This loop checks each of the sibling items...
	do
	{
		// Check if we have found the item...
		if( pNode == ( CWorldNode * )GetItemData( hItem ))
			return hItem;

		// Check if this item has children, if not then no sibling is the item...
		if(( hChildItem = GetChildItem( hItem )) == NULL )
			return NULL;

		// This loop checks each of the children recursively...
		do
		{
			// If the call returns a non-NULL value, the item was found
			// somewhere in the descendants...
			hFoundItem = FindItem( hChildItem, pNode );
			if( hFoundItem != NULL )
				return hFoundItem;
		}
		while(( hChildItem = GetNextItem( hChildItem, TVGN_NEXT )) != NULL);
	}
	while(( hItem = GetNextItem( hItem, TVGN_NEXT )) != NULL );

	// All items were checked and nothing was found...
	return NULL;
}

void CNodeViewTreeCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CRegionDoc *pDoc;
	HTREEITEM hItem;

	pDoc = (( CNodeView * )GetParent( ))->GetDoc( );
	if( pDoc == NULL )
		return;

	// TODO: Add your message handler code here and/or call default
	switch( nChar )
	{			
		case VK_DELETE:
		{
			POSITION pos = pDoc->GetFirstViewPosition();
			CRegionView* pView = ( CRegionView * )pDoc->GetNextView( pos );
			if( pView == NULL )
				return;

			pView->DeleteSelectedNodes( );
			pDoc->NotifySelectionChange();
			return;
		}
		case VK_SPACE:
		{
			hItem = GetSelectedItem( );
			if( hItem )
				SelectTree( hItem, TRUE );
			return;
		}		
	}
			
	CMyTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CNodeViewTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here

	// CG: This block was added by the Pop-up Menu component
	if( GetCount( ))
	{
		UINT nHitTestFlag;
		HTREEITEM hItem;
		CPoint ptClientPoint;

		ptClientPoint = point;
		ScreenToClient( &ptClientPoint );

		hItem = HitTest( ptClientPoint, &nHitTestFlag );
		if( !( nHitTestFlag & TVHT_ONITEM ) /*|| hItem != GetSelectedItem( )*/)
			return;

		SelectItem(hItem);

		if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);

			point = rect.TopLeft();
			point.Offset(5, 5);
		}

		CMenu menu;
		VERIFY(menu.LoadMenu(CG_IDR_POPUP_NODE_VIEW));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);
		CWnd* pWndPopupOwner = this;

		pWndPopupOwner = pWndPopupOwner->GetParent();

		// Get the node view
		CNodeView *pNodeView = (CNodeView *)GetParent( );

		// Get the current node
		CWorldNode *pSelectedNode=pNodeView->GetSelectedNode();
		if (pSelectedNode)
		{
			// Determine if the node is hidden
			if (pSelectedNode->IsFlagSet(NODEFLAG_HIDDEN))
			{
				// Set the check on the Hide item in the menu
				pPopup->CheckMenuItem(ID_HIDE_NODE, MF_BYCOMMAND | MF_CHECKED);
			}
		}

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
			pWndPopupOwner);
	}
}

BOOL CNodeViewTreeCtrl::PreTranslateMessage(MSG* pMsg)
{
	// Shift+F10: show pop-up menu.
	if ((((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) && // If we hit a key and
		(pMsg->wParam == VK_F10) && (GetKeyState(VK_SHIFT) & ~1)) != 0) ||	// it's Shift+F10 OR
		(pMsg->message == WM_CONTEXTMENU))									// Natural keyboard key
	{
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);

		CPoint point = rect.TopLeft();
		point.Offset(5, 5);
		OnContextMenu(NULL, point);

		return TRUE;
	}

	if( pMsg->message == WM_KEYDOWN )
	{
		// Select the item when RETURN hit if we aren't editing a label
		if( pMsg->wParam == VK_RETURN)
		{
			if (!m_bEditingLabel)
			{
				HTREEITEM hItem = GetSelectedItem( );
				if( hItem )
				{
					SelectTree( hItem, TRUE );
				}
			}
			
			::TranslateMessage( pMsg );
			::DispatchMessage( pMsg );			

			// DO NOT process further...
			return TRUE;
		}
	}

	// Translate any accelerator key presses
	if (pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
	{
		// Get the main frame
		CMainFrame *pFrame=(CMainFrame *)AfxGetMainWnd();
		
		HACCEL hAccel = pFrame->m_hAccelTable;
		return ::TranslateAccelerator(GetSafeHwnd(), hAccel, pMsg);		
	}

	return CMyTreeCtrl::PreTranslateMessage(pMsg);
}

void CNodeViewTreeCtrl::OnBeginLabelEdit( NMHDR *pNMHDR, LRESULT *pResult )
{
	// Set a bool indicated that we are editing a label
	m_bEditingLabel=TRUE;

	// Handle the text editing differently depending on the current view type	
	switch (GetNodeView()->GetViewType())
	{
	case CNodeView::kViewName:
		{			
			// Allow editing
			*pResult=FALSE;
			break;
		}
	case CNodeView::kViewClass:
		{
			// Display message box
			MessageBox("Class types cannot be changed in the node view.", "Node View", MB_OK | MB_ICONEXCLAMATION);

			// Cancel editing
			*pResult=TRUE;
			break;
		}
	case CNodeView::kViewBoth:
		{
			// Get the selected item
			HTREEITEM hItem=GetSelectedItem();

			// Get the node
			CWorldNode *pNode = (CWorldNode *)GetItemData(hItem);

			// First set the label text to just the name
			CString sLabel=m_pNodeView->GetNodeLabel(pNode, CNodeView::kViewName);

			// Set the text for the label
			SetItemText(hItem, sLabel);
			UpdateWindow();

			// Set the text for the edit control
			GetEditControl()->SetWindowText(sLabel);

			// Allow editing
			*pResult=FALSE;
			break;
		}
	default:
		{
			// Cancel editing
			*pResult=TRUE;
			ASSERT(FALSE);
		}
	}

	TV_DISPINFO *pTVDispInfo = ( TV_DISPINFO * )pNMHDR;

	// Limit text...
	GetEditControl( )->LimitText( CWorldNode::MAX_LABEL );	
}

void CNodeViewTreeCtrl::OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	TV_DISPINFO		*ptvinfo;
	CWorldNode *pNode;
	HTREEITEM hItem;	

	// We are not editing a label any more
	m_bEditingLabel=FALSE;

	ptvinfo = (TV_DISPINFO *)pnmhdr;
	if (ptvinfo->item.pszText != NULL)
	{
		hItem = ptvinfo->item.hItem;

		// Make sure the new label isn't too long...
		if( strlen( ptvinfo->item.pszText ) > CWorldNode::MAX_LABEL )
			ptvinfo->item.pszText[CWorldNode::MAX_LABEL] = '\0';
		
		// Get the node
		pNode = (CWorldNode *)GetItemData( hItem );

		// Special case container nodes
		if (pNode->GetType() == Node_Null)		
		{
			pNode->SetNodeLabel(ptvinfo->item.pszText);
		}
		else
		{
			// Set the name for the node
			pNode->SetName(ptvinfo->item.pszText);
		}

		// Update the label		
		m_pNodeView->UpdateTreeItemLabel(pNode);
		
		// Setup an undo.
		CRegionDoc *pDoc = (( CNodeView * )GetParent( ))->GetDoc( );
		if(pDoc != NULL)
			pDoc->Modify(new CPreAction(ACTION_MODIFYNODE, pNode), TRUE);		

		// Update the properties dialog
		pDoc->SetupPropertiesDlg(FALSE);
	}
	*pLResult = FALSE;
}

/************************************************************************/
//handle a double click, as either a double click if it is on any part but
//the check boxes, or as a single click on those boxes
void CNodeViewTreeCtrl::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	UINT nHitTestFlag;
	HitTest( point, &nHitTestFlag );
	if( nHitTestFlag & TVHT_ONITEMSTATEICON )
	{
		// if a checkbox was double-clicked on, just treat it as a single-click.
		OnLButtonDown( nFlags, point );
		return;
	}

	CMyTreeCtrl::OnLButtonDblClk( nFlags, point );
}
/************************************************************************/
// A tree item was expanded
void CNodeViewTreeCtrl::OnItemExpanded(LPNMHDR pnmhdr, LRESULT *pLResult)
{
	// Get the tree view structure
	LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmhdr;

	// The item
	HTREEITEM hItem=pnmtv->itemNew.hItem;

	// The world node for this item
	CWorldNode *pNode = (CWorldNode *)GetItemData(hItem);

	// The item was expanded
	if (pnmtv->action & TVE_EXPAND)
	{
		// Set the expand flag
		pNode->SetFlags(pNode->GetFlags() | NODEFLAG_EXPANDED);
	}
	// The item was collapsed
	else if (pnmtv->action & TVE_COLLAPSE)
	{		
		// Clear the expand flag
		pNode->ClearFlag(NODEFLAG_EXPANDED);
	}
}

//the width of the checkbox icon
#define CHECKBOX_WIDTH		8

void CNodeViewTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	HTREEITEM hItem;
	UINT nHitTestFlag;
	bool bMultiSelect = false;
	
	hItem = HitTest( point, &nHitTestFlag );
	
	if(( nHitTestFlag & TVHT_ONITEMSTATEICON ))
	{
		//make sure to grab the focus and select this item. This will make sure that properties
		//are brought up to date
		SetFocus();

		UINT nSelHitTestFlag;
		HitTest( CPoint( point.x + CHECKBOX_WIDTH, point.y ), &nSelHitTestFlag );
		if( nSelHitTestFlag & TVHT_ONITEMSTATEICON )
		{
			// the first checkbox was clicked on.
			CWorldNode* pNode = (CWorldNode*)GetItemData( hItem );
			
			//don't bother with trying to select it if it is frozen
			if(pNode->IsFlagSet(NODEFLAG_FROZEN))
			{
				return;
			}

			//see if the user wants to select multiple nodes, we don't need to worry about
			//this if we are already in multi select mode
			if(	(GetMainFrame()->GetNodeSelectionMode() == SINGLE_SELECTION) && 
				(nFlags & MK_CONTROL))
			{
				bMultiSelect = true;
			}
			
			// Select only the parent node if the shift key is down
			if (nFlags & MK_SHIFT)
			{
				//We want to force the multi select to TRUE which allows the proper
				//behavior since shift should act the same regardless of the mode.
				SelectTree( hItem, false, true );
			}
			else
			{
				// Select the parent and the children
				SelectTree( hItem, true, bMultiSelect );
			}
		}

		//the user didn't click on the select box, they clicked on the freeze box,
		//so it should be frozen
		else
		{
			CWorldNode* pNode = (CWorldNode*)GetItemData( hItem );
			BOOL bFreeze			= TRUE;
			BOOL bFreezeChildren	= TRUE;

			if( pNode->IsFlagSet( NODEFLAG_FROZEN ) )
			{
				bFreeze = FALSE;
			}	

			//see if they want to include the children or not
			if( nFlags & MK_SHIFT )
			{
				bFreezeChildren = FALSE;
			}

			CNodeView* pNodeView = (CNodeView*)GetParent();
			CRegionDoc* pDoc = pNodeView->GetDoc();
			
			// Setup the undo information
			if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
			{
				PreActionList actionList;

				if (bFreezeChildren)
				{
					POSITION pos = pDoc->GetFirstViewPosition();
					CRegionView* pView = ( CRegionView * )pDoc->GetNextView( pos );
					if( pView == NULL )  ASSERT(false);

					pView->UndoHelper(&actionList, pNode, NODEFLAG_FROZEN, !bFreeze);
				}
				else
				{
					actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pNode));	
				}
				pDoc->Modify(&actionList, TRUE);
			}
			else
				pDoc->Modify();


			pDoc->GetRegion()->FreezeNode(pNode, bFreeze, true, bFreezeChildren);
			
			// Update the node image
			pNodeView->UpdateNodeImage( pNode );
			
			// Redraw the views
			pDoc->RedrawAllViews( );
		}
	}
	else
	{
		CMyTreeCtrl::OnLButtonDown(nFlags, point);		
	}
}

/************************************************************************/
// Sends a windows message to the region view
void CNodeViewTreeCtrl::SendMessageToRegionView(UINT message, WPARAM wParam, LPARAM lParam)
{
	CDocument *pDoc = (( CNodeView * )GetParent( ))->GetDoc( );
	ASSERT( pDoc );

	// Get the current view
	POSITION pos = pDoc->GetFirstViewPosition();
	CRegionView* pView = ( CRegionView * )pDoc->GetNextView( pos );
	if( pView == NULL )
		return;
	
	// Send the paste message
	pView->SendMessage(message, wParam, lParam);
}

/************************************************************************/
// Paste
void CNodeViewTreeCtrl::OnEditPaste() 
{	
	// Send the paste message
	SendMessageToRegionView(WM_COMMAND, MAKELONG(ID_EDIT_PASTE, 1), NULL);
}

/************************************************************************/
// Paste alternate
void CNodeViewTreeCtrl::OnEditPasteAlternate() 
{
	// Send the paste message
	SendMessageToRegionView(WM_COMMAND, MAKELONG(ID_EDIT_PASTE_ALTERNATE, 1), NULL);
}

/************************************************************************/
// Copy
void CNodeViewTreeCtrl::OnEditCopy() 
{	
	// Send the copy message
	SendMessageToRegionView(WM_COMMAND, MAKELONG(ID_EDIT_COPY, 1), NULL);
}

/************************************************************************/
// Undo
void CNodeViewTreeCtrl::OnEditUndo() 
{	
	// Send the undo message
	SendMessageToRegionView(WM_COMMAND, MAKELONG(ID_EDIT_UNDO, 1), NULL);	
}

/************************************************************************/
// The group hotkey was pressed
void CNodeViewTreeCtrl::OnSelectionGroup() 
{
	// Send the group message
	SendMessageToRegionView(WM_COMMAND, MAKELONG(ID_SELECTION_GROUP, 1), NULL);		
}

/************************************************************************/
// Rename the current node
void CNodeViewTreeCtrl::OnPopupRename() 
{
	HTREEITEM hSelectedItem=GetSelectedItem();
	if (hSelectedItem)
	{
		EditLabel(hSelectedItem);
	}	
}

/************************************************************************/
// Set active parent
void CNodeViewTreeCtrl::OnPopupSetactiveParent() 
{
	// Get the node view dialog
	CNodeView *pNodeView=(CNodeView *)GetParent( );			
	pNodeView->SendMessage(WM_COMMAND, MAKELONG(ID_POPUP_SETACTIVEPARENT, 1), NULL);
}

/************************************************************************/
// The Refresh command was selected
void CNodeViewTreeCtrl::OnPopupRefresh() 
{
	// Get the node view dialog
	CNodeView *pNodeView=(CNodeView *)GetParent( );			
	pNodeView->SendMessage(WM_COMMAND, MAKELONG(ID_POPUP_REFRESH, 1), NULL);	
}

/************************************************************************/
// The key to go to the next selected node was pressed
void CNodeViewTreeCtrl::OnGotoNextSelectedNode() 
{
	// Get the node view dialog
	CNodeView *pNodeView=(CNodeView *)GetParent( );			
	pNodeView->SendMessage(WM_COMMAND, MAKELONG(ID_GOTO_NEXT_SELECTED_NODE, 1), NULL);	
}

void CNodeViewTreeCtrl::OnPopupCollapseChildren() 
{		
	// Get the node view dialog
	CNodeView *pNodeView=(CNodeView *)GetParent( );			
	pNodeView->SendMessage(WM_COMMAND, MAKELONG(ID_POPUP_COLLAPSE_CHILDREN, 1), NULL);	
}
