//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// NodeView.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "nodeview.h"
#include "worldnode.h"
#include "regiondoc.h"
#include "regionview.h"
#include "mainfrm.h"
#include "edit_actions.h"
#include "projectbar.h"
#include "node_ops.h"
#include "optionsmisc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNodeView dialog


CNodeView::CNodeView()	
{
	//{{AFX_DATA_INIT(CNodeView)
	m_nViewType=0;
	//}}AFX_DATA_INIT

	m_pDoc = NULL;
}

CNodeView::~CNodeView( )
{
	Term( );
}

void CNodeView::DoDataExchange(CDataExchange* pDX)
{
	CMRCSizeDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNodeView)
	DDX_Control(pDX, IDC_NODEVIEW_TREE, m_NodeViewTree);
	DDX_Radio(pDX, IDC_RADIO_NODEVIEW_NAME, m_nViewType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNodeView, CProjectTabControlBar)
	//{{AFX_MSG_MAP(CNodeView)
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_ADD_NULL_NODE, OnAddNullNode)
	ON_COMMAND(ID_HIDE_NODE, OnHideNode)
	ON_COMMAND(ID_UNHIDEALL_NODES, OnUnhideAllNodes)
	ON_COMMAND(ID_POPUP_SETPATH, OnSetPath)
	ON_COMMAND(ID_POPUP_UNSETPATH, OnUnsetPath)
	ON_COMMAND(ID_POPUP_HIDEALLNODES, OnPopupHideAllNodes)
	ON_COMMAND(ID_POPUP_SETACTIVEPARENT, OnPopupSetActiveParent)
	ON_BN_CLICKED(IDC_RADIO_NODEVIEW_NAME, OnRadioNodeViewName)
	ON_BN_CLICKED(IDC_RADIO_NODEVIEW_CLASS, OnRadioNodeViewClass)
	ON_BN_CLICKED(IDC_RADIO_NODEVIEW_BOTH, OnRadioNodeViewBoth)
	ON_COMMAND(ID_POPUP_RENAME, OnPopupRename)
	ON_COMMAND(ID_UNHIDE_NODE_AND_CHILDREN, OnUnhideNodeAndChildren)
	ON_COMMAND(ID_HIDE_NODE_AND_CHILDREN, OnHideNodeAndChildren)
	ON_COMMAND(ID_POPUP_MOVE_TAGGED_NODES, OnPopupMoveTaggedNodes)
	ON_COMMAND(ID_POPUP_REFRESH, OnPopupRefresh)
	ON_COMMAND(ID_GOTO_NEXT_SELECTED_NODE, OnGotoNextSelectedNode)
	ON_COMMAND(ID_POPUP_COLLAPSE_CHILDREN, OnPopupCollapseChildren)
	ON_COMMAND(ID_POPUP_DISCONNECT_PREFAB, OnPopupDisconnectPrefab)
	ON_COMMAND(ID_DELETE_EMPTY_CONTAINERS, OnDeleteEmptyContainers)
	ON_COMMAND(ID_DELETE_EMPTY_CONTAINER_CASCADE, OnDeleteCascadeContainers)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNodeView message handlers

void CNodeView::OnSize(UINT nType, int cx, int cy) 
{
	CRect		rect;

	CMRCSizeDialogBar::OnSize(nType, cx, cy);
	
	// Reposition the controls
	RepositionControls();	
}

/************************************************************************/
// Repositions the controls
void CNodeView::RepositionControls()
{
	if( ::IsWindow(m_hWnd) && ::IsWindow(m_NodeViewTree.m_hWnd) )
	{
		CRect rect;
		GetClientRect( &rect );

		// Get the current screen rectangle for the node view
		CRect rectNodeView;
		m_NodeViewTree.GetWindowRect(rectNodeView);

		// Convert the screen coordinates to client coordinates
		ScreenToClient(rectNodeView);

		// Resize the tree control but don't move the top location
		m_NodeViewTree.MoveWindow( 0, rectNodeView.top, rect.Width(), rect.Height()-rectNodeView.top );
	}	
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::Init
//
//  Purpose:	Initializes the controls
//
//------------------------------------------------------------------------------------------
void CNodeView::Init( CRegionDoc *pDoc )
{
	if( pDoc != NULL && !pDoc->m_WorldName.IsEmpty( ))
	{
		if( pDoc == m_pDoc )
			return;
		else
		{
			m_pDoc = pDoc;
						
			Update( );
		}
	}

	// Initialize the tree control pointer
	m_NodeViewTree.SetNodeView(this);
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::Term
//
//  Purpose:	Terminates dialog
//
//------------------------------------------------------------------------------------------
void CNodeView::Term( )
{
	if( ::IsWindow( m_NodeViewTree.m_hWnd ))
	{
		// Turn off the drawing of the window
		if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
		{
			m_NodeViewTree.SetRedraw(FALSE);
		}

		ClearAllItems( );

		// Turn off the drawing of the window
		if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
		{
			m_NodeViewTree.SetRedraw(TRUE);
		}
	}

	m_pDoc = NULL;
}


/************************************************************************/
// The sorting function used to sort the nodes in the tree
int CALLBACK CNodeView::TreeSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	// Get the nodes
	CWorldNode *pNode1=(CWorldNode *)lParam1;
	CWorldNode *pNode2=(CWorldNode *)lParam2;

	// Check to see if the first node is a container
	if (pNode1->GetType() == Node_Null)
	{
		// If the second node is not a container, then it does not go before this node
		if (pNode2->GetType() != Node_Null)
		{
			return -1;
		}

		// Since both nodes are containers, compare their labels
		return strcmp(pNode1->GetNodeLabel(), pNode2->GetNodeLabel());
	}
	// Check to see if the second node is a container
	else if (pNode2->GetType() == Node_Null)
	{
		// This node goes before the first node
		return 1;
	}

	//we need to do a custom string comparison here, so that way we can factor in
	//the numbers at the end.
	const char* pszLeft  = pNode1->GetName();
	const char* pszRight = pNode2->GetName();

	while(1)
	{
		//check for the end of strings
		if(!*pszLeft)
			return -1;

		if(!*pszRight)
			return 1;

		//check for if we have hit a number
		if(isdigit(*pszLeft) && isdigit(*pszRight))
		{
			//we have hit a digit....convert to integer and compare
			int nLeft	= atoi(pszLeft);
			int nRight	= atoi(pszRight);

			//check and see if they are equal values, if so, we need to resolve
			//descrepencies such as Tree01 and Tree1
			if(nLeft == nRight)
				return strcmp(pszLeft, pszRight);

			//return the difference
			return nLeft - nRight;
		}

		if(*pszLeft != *pszRight)
			return *pszLeft - *pszRight;

		pszLeft++;
		pszRight++;
	}
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::Update
//
//  Purpose:	Updates all contained controls
//
//------------------------------------------------------------------------------------------
void CNodeView::Update( )
{	
	UpdateTree( );
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::UpdateTree
//
//  Purpose:	Update the tree with new data.  Should be called whenever
//				tree modified or dialog initialized.
//
//------------------------------------------------------------------------------------------
void CNodeView::UpdateTree( )
{		
	// Clear the items
	ClearAllItems();

	if( !m_pDoc )
		return;
	
	// Get the root node for world and create tree...
	CWorldNode *pNode = m_pDoc->GetRegion( )->GetRootNode( );
	
	// Turn off the drawing of the window
	if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
	{
		m_NodeViewTree.SetRedraw(FALSE);
	}

	// Add the items
	HTREEITEM hNode = AddNode( pNode );
	AddItemsToTree(pNode);

	// Expand the items with the NODEFLAG_EXPAND flag set
	ExpandItems(pNode);

	// Sort the nodes
	m_NodeViewTree.SortNodeAndChildren(m_NodeViewTree.GetRootItem(), TreeSortFunc);

	// Set the item images
	m_NodeViewTree.RecurseSetItemImage( hNode, TRUE );

	if (GetProjectBar()->m_pVisibleDlg == GetNodeView())
	{
		// Turn redraw back on
		m_NodeViewTree.SetRedraw(TRUE);

		// Redraw the window
		m_NodeViewTree.Invalidate();	
	}
}


//------------------------------------------------------------------------------------------
//
//  CNodeView::AddItemsToTree
//
//  Purpose:	Recursively called to add children to a parent node of the tree control.
//
//------------------------------------------------------------------------------------------
void CNodeView::AddItemsToTree( CWorldNode *pParentNode)
{
	HTREEITEM hNode;
	GPOS pos;
	CWorldNode *pChild;

	// Loop through the children of the node and insert them...
	for(pos=pParentNode->m_Children; pos; )
	{
		pChild = pParentNode->m_Children.GetNext(pos);

		hNode = AddNode(pChild);
		AddItemsToTree(pChild);
	}
}

/************************************************************************/
// Expand the items with the NODEFLAG_EXPAND flag set
void CNodeView::ExpandItems(CWorldNode *pParentNode)
{
	// Check to see if this should be expanded
	if (pParentNode->IsFlagSet(NODEFLAG_EXPANDED))
	{
		// Expand the node
		m_NodeViewTree.Expand(pParentNode->GetItem(), TVE_EXPAND);
	}

	// Loop through all of the children
	GPOS pos=pParentNode->m_Children;
	while (pos)	
	{
		CWorldNode *pChild = pParentNode->m_Children.GetNext(pos);
		ExpandItems(pChild);
	}
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::AddNode
//
//  Purpose:	Adds a CWorldNode object to tree.
//
//------------------------------------------------------------------------------------------
HTREEITEM CNodeView::AddNode( CWorldNode *pNode )
{
	HTREEITEM hNode, hParentItem;

	hParentItem = NULL;

	ASSERT( pNode );

	if( pNode->GetParent( ))
		hParentItem = pNode->GetParent( )->GetItem( );
	if( hParentItem == NULL )
		hParentItem = TVI_ROOT;

	// Get the node's current item...
	hNode = pNode->GetItem( );

	if( hNode != NULL )
	{
		hNode = SetNode( pNode );
	}
	else
	{
		HTREEITEM hInsertAfter;

		// Put the item first if it is a container
		if (pNode->GetType() == Node_Null)
		{
			hInsertAfter=TVI_FIRST;
		}
		else
		{
			// Otherwise put the node last
			hInsertAfter=TVI_LAST;
		}

		hNode = m_NodeViewTree.InsertItem( TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE, 
			GetNodeLabel(pNode, GetViewType()), 0, 0, 0, 0, (LPARAM) pNode, hParentItem , hInsertAfter);

		pNode->SetItem( hNode );
		m_NodeViewTree.SetNodeImage( hNode );
	}

	return hNode;
}


//------------------------------------------------------------------------------------------
//
//  CNodeView::SetNode
//
//  Purpose:	Sets a CWorldNode object's item attributes.
//
//------------------------------------------------------------------------------------------
HTREEITEM CNodeView::SetNode( CWorldNode *pNode )
{
	HTREEITEM hNode, hParentItem, hNewNode;

	hParentItem = NULL;

	if( pNode->GetParent( ))
		hParentItem = pNode->GetParent( )->GetItem( );
	if( hParentItem == NULL )
		hParentItem = TVI_ROOT;

	// Get node's current item...
	hNode = pNode->GetItem( );

	// If node has no current item, then let AddNode do the work...
	if( hNode == NULL )
	{
		hNode = AddNode( pNode );
	}
	// If node already has an item, reset it to the node's current stats...
	else
	{		
		m_NodeViewTree.SetItem( hNode, TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE, 
								GetNodeLabel(pNode, GetViewType()), 0, 0, 0, 0, (LPARAM) pNode );
		hNewNode = m_NodeViewTree.TransferItem( hNode, hParentItem );
		m_NodeViewTree.DeleteItem( hNode );
		hNode = hNewNode;

		m_NodeViewTree.RecurseSetItemImage( hNode, TRUE );
	}

	return hNode;
}


//------------------------------------------------------------------------------------------
//
//  CNodeView::DeleteNode
//
//  Purpose:	Deletes a CWorldNode object's item and all its child items.
//
//------------------------------------------------------------------------------------------
void CNodeView::DeleteNode( CWorldNode *pNode )
{
	HTREEITEM hNode;

	ASSERT( pNode );

	hNode = pNode->GetItem( );
	if( hNode == NULL )
		return;

	pNode->SetItem( NULL );

	m_NodeViewTree.DeleteItem( hNode );
}


//------------------------------------------------------------------------------
//
//  CNodeView::RemoveFromTree( ) 
//
//  Purpose:	Removes a HTREEITEM from a tree.  Removed from children of object's 
//				parent.  Object's Children's parent set to object's parent.
//				Does not change the CWorldNode object.  This function must be
//				paired with a CWorldNode::RemoveFromTree call.
//
//------------------------------------------------------------------------------
void CNodeView::RemoveFromTree( CWorldNode *pNode )
{
	CWorldNode *pParentNode;
	HTREEITEM hParentItem, hItem, hNewItem;
	DWORD i;
	GPOS pos;

	ASSERT( pNode );
	if(!pNode->GetItem())
		return;

	pParentNode = pNode->GetParent( );
	ASSERT( pParentNode );
	hParentItem = pParentNode->GetItem( );
	ASSERT( hParentItem );

	// Give object's children to their grandparent...
	for(pos=pNode->m_Children; pos; )
	{
		hItem = pNode->m_Children.GetNext(pos)->GetItem( );
		ASSERT( hItem );
		hNewItem = m_NodeViewTree.TransferItem( hItem, hParentItem );
		m_NodeViewTree.DeleteItem( hItem );
	}
}


//------------------------------------------------------------------------------------------
//
//  CNodeView::UpdateNodeImage
//
//  Purpose:	Updates the node's and its children's images.
//
//------------------------------------------------------------------------------------------
void CNodeView::UpdateNodeImage( CWorldNode *pNode, BOOL bRecurse )
{
	HTREEITEM hNode;

	hNode = pNode->GetItem( );
	if( hNode == NULL )
		return;

	m_NodeViewTree.RecurseSetItemImage( hNode, bRecurse );
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::UpdateTreeItemLabel
//
//  Purpose:	Updates the node's text
//
//------------------------------------------------------------------------------------------
void CNodeView::UpdateTreeItemLabel(CWorldNode *pNode)
{
	m_NodeViewTree.SetItemText(pNode->GetItem(), GetNodeLabel(pNode, GetViewType()));
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::UpdateTreeItemLabel
//
//  Purpose:	Recursively updates the nodes label and its children
//
//------------------------------------------------------------------------------------------
void CNodeView::RecurseUpdateLabels(CWorldNode *pNode)
{
	// Update the children
	GPOS pos=pNode->m_Children;
	while (pos)
	{
		CWorldNode *pCurrentNode=(CWorldNode *)pNode->m_Children.GetNext(pos);
		RecurseUpdateLabels(pCurrentNode);		
	}	
	
	// Update the label
	UpdateTreeItemLabel(pNode);
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::ClearAllItems
//
//  Purpose:	Initializes the controls
//
//------------------------------------------------------------------------------------------
void CNodeView::ClearAllItems( )
{	
	CWorldNode *pNode;

	// Get the root node for world and create tree...
	if( m_pDoc && ( pNode = m_pDoc->GetRegion( )->GetRootNode( )))
		RecurseClearAllItems( pNode );

	m_NodeViewTree.DeleteAllItems( );
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::RecurseClearAllItems
//
//  Purpose:	Recursively called to clear HTREEITEM parameter.
//
//------------------------------------------------------------------------------------------
void CNodeView::RecurseClearAllItems( CWorldNode *pNode )
{
	CWorldNode *pChildNode;
	GPOS pos;

	pNode->SetItem( NULL );

	// Loop through the children of the node and clear their item...
	for(pos=pNode->m_Children; pos; )
	{
		pChildNode = pNode->m_Children.GetNext(pos);

		if(pChildNode)
			RecurseClearAllItems( pChildNode );
	}
}

//------------------------------------------------------------------------------------------
//
//  CNodeView::OnInitDialogBar
//
//  Purpose:	Initializes the dialog (like OnInitDialog)
//
//------------------------------------------------------------------------------------------
BOOL CNodeView::OnInitDialogBar() 
{	
	// Call the base class
	CMRCSizeDialogBar::OnInitDialogBar();

	// TODO: Add extra initialization here
	CImageList			*pimagelist;
	CImageList			*pStateImageList;

	// Create the image list used in tree and list controls...
	pimagelist = new CImageList();
	ASSERT( pimagelist );
	pimagelist->Create( IDB_NODE_ICONS, 20, 9, RGB(255,0,255) );
	m_NodeViewTree.SetImageList( pimagelist, TVSIL_NORMAL );

	pStateImageList = new CImageList( );
	ASSERT( pStateImageList );
	pStateImageList->Create( IDB_NODE_STATE_ICONS, 16, 2, RGB(0,255,0) );
	m_NodeViewTree.SetImageList( pStateImageList, TVSIL_STATE );

	// Update the controls...
	Update( );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNodeView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if( ::IsWindow( m_NodeViewTree.m_hWnd ))
		m_NodeViewTree.SendMessage( WM_CONTEXTMENU, (LPARAM) this, MAKELPARAM( point.x, point.y ));
	
}

BOOL CNodeView::PreTranslateMessage(MSG* pMsg)
{
	// CG: This block was added by the Pop-up Menu component
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
	}

	return CMRCSizeDialogBar::PreTranslateMessage(pMsg);
}


void CNodeView::OnAddNullNode() 
{
	CRegionDoc *pDoc;
	HTREEITEM hItem, hParentItem;
	CWorldNode *pNode;

	pDoc = GetDoc( );
	if( pDoc == NULL )
		return;	
	
	pNode = pDoc->GetRegion()->AddNullNode(GetSelectedNode());
	ASSERT( pNode );

	// Select the node
	HighlightNode(pNode);

	//now allow the editing of the container name
	OnPopupRename();
}

/************************************************************************/
// Hides a node
void CNodeView::OnHideNode() 
{
	// Get the selected node	
	CWorldNode *pNode=GetSelectedNode();
	if (!pNode)
	{
		return;
	}

	BOOL bHide=FALSE;
	BOOL bChildren=FALSE;

	// Check to see if the node is currently hidden
	if (pNode->IsFlagSet(NODEFLAG_HIDDEN))
	{
		bHide=FALSE;
	}	
	else
	{
		bHide=TRUE;
	}

	// Change the hidden status
	ChangeHiddenStatusOfSelection(bHide, bChildren);
}

/************************************************************************/
// Hides a node and its children
void CNodeView::OnHideNodeAndChildren() 
{
	BOOL bHide=TRUE;
	BOOL bChildren=TRUE;

	// Change the hidden status
	ChangeHiddenStatusOfSelection(bHide, bChildren);
}

/************************************************************************/
// Unhides a node and its children
void CNodeView::OnUnhideNodeAndChildren() 
{
	BOOL bHide=FALSE;
	BOOL bChildren=TRUE;

	// Change the hidden status
	ChangeHiddenStatusOfSelection(bHide, bChildren);	
}

/************************************************************************/
// Changes the hidden status for the selected node.
// bHide			- Set to TRUE if you wish to hide the node
//					- Set to FALSE if you wish to show the node
// bIncludeChildren	- Set to TRUE if you wish to also change the status of the nodes children
void CNodeView::ChangeHiddenStatusOfSelection(BOOL bHide, BOOL bIncludeChildren)
{
	// Get the document
	CRegionDoc *pDoc = GetDoc( );
	if(!pDoc)
	{
		return;
	}

	// Get the selected node
	CWorldNode *pNode=GetSelectedNode();
	if (!pNode)
	{
		ASSERT(FALSE);
		return;
	}

	// Setup the undo information 
	if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
	{
		POSITION pos = pDoc->GetFirstViewPosition();
		CRegionView* pView = (CRegionView *)pDoc->GetNextView(pos); 
		ASSERT(pView);

		if (bIncludeChildren)
		{
			PreActionList actionList;
			pView->UndoHelper(&actionList, pNode);
			pDoc->Modify(&actionList, TRUE);
		}
		else
		{
			pDoc->Modify(new CPreAction(ACTION_MODIFYNODE, pNode), TRUE);
		}
	}
	else
		pDoc->Modify();

	if (bHide)
	{
		// Hide the node
		pNode->HideNode(bIncludeChildren);
	}
	else
	{
		// Show the node
		pNode->ShowNode(bIncludeChildren);
	}	

	// Update the node image
	UpdateNodeImage(pNode);
	
	// Redraw the views
	pDoc->RedrawAllViews( );
}

/************************************************************************/
// Returns the selected WorldNode
CWorldNode *CNodeView::GetSelectedNode()
{
	// Get the document
	CRegionDoc *pDoc = GetDoc( );
	if(!pDoc)
	{
		return NULL;
	}

	// Get the selected item
	HTREEITEM hItem = m_NodeViewTree.GetSelectedItem( );
	if (!hItem)
	{
		return NULL;
	}

	// Get the node for the item
	CWorldNode *pNode = (CWorldNode *)m_NodeViewTree.GetItemData( hItem );

	// Return the node
	return pNode;
}

/************************************************************************/
// Highlights a node in the tree control and ensures that it is visible
void CNodeView::HighlightNode(CWorldNode *pNode)
{
	if (pNode)
	{
		m_NodeViewTree.SelectItem(pNode->GetItem());
		m_NodeViewTree.EnsureVisible(pNode->GetItem());
	}
}

void CNodeView::OnOK( )
{
	HTREEITEM hItem;

	hItem = m_NodeViewTree.GetSelectedItem( );
	m_NodeViewTree.SelectTree( hItem, TRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeView::OnUnhideAllNodes
//
//	PURPOSE:	Unhide all hidden nodes
//
// ----------------------------------------------------------------------- //

void CNodeView::OnUnhideAllNodes()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateStatusText(0,"Unhiding all nodes...");

	BeginWaitCursor();

	// Setup the undo information 
	if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
	{
		CRegionDoc *pDoc = GetDoc( );
		if(pDoc)
		{
			POSITION pos = pDoc->GetFirstViewPosition();
			CRegionView* pView = (CRegionView *)pDoc->GetNextView(pos); 
			ASSERT(pView);

			PreActionList actionList;
			pView->UndoHelper(&actionList, pView->GetRegion()->GetRootNode());
			pDoc->Modify(&actionList, TRUE);
		}
	}
	else
		GetDoc()->Modify();


	RecurseHideAll(m_NodeViewTree.GetRootItem(), FALSE);
	UpdateNodeImage(( CWorldNode * )m_NodeViewTree.GetItemData(m_NodeViewTree.GetRootItem()) );
	EndWaitCursor();

	pFrame->UpdateStatusText(0,"Ready");

	GetDoc( )->RedrawAllViews( );
}

void CNodeView::OnPopupHideAllNodes() 
{	
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->UpdateStatusText(0,"Hiding all nodes...");

	BeginWaitCursor();

	// Setup the undo information 
	if(GetApp()->GetOptions().GetMiscOptions()->IsUndoFreezeHide())
	{
		CRegionDoc *pDoc = GetDoc( );
		if(pDoc)
		{
			POSITION pos = pDoc->GetFirstViewPosition();
			CRegionView* pView = (CRegionView *)pDoc->GetNextView(pos); 
			ASSERT(pView);

			PreActionList actionList;
			pView->UndoHelper(&actionList, pView->GetRegion()->GetRootNode());
			pDoc->Modify(&actionList, TRUE);
		}
	}
	else
		GetDoc()->Modify();


	RecurseHideAll(m_NodeViewTree.GetRootItem(), TRUE);
	UpdateNodeImage(( CWorldNode * )m_NodeViewTree.GetItemData(m_NodeViewTree.GetRootItem()) );
	EndWaitCursor();

	pFrame->UpdateStatusText(0,"Ready");

	GetDoc( )->RedrawAllViews( );	
}

void CNodeView::RecurseHideAll(HTREEITEM hItem, BOOL bHide)
{
	CWorldNode* pNode = ( CWorldNode * )m_NodeViewTree.GetItemData(hItem);

	for(GPOS pos=pNode->m_Children; pos; )
	{
		CWorldNode* pChild = pNode->m_Children.GetNext(pos);
		if ( bHide )
		{				
			pChild->HideNode();			
		}
		else
		{
			pChild->ShowNode();				
		}

		RecurseHideAll(pChild->m_hItem, bHide);
	}
	return;
}

void CNodeView::OnSetPath()
{
	HTREEITEM hItem;
	CWorldNode *pNode, *pParent;
	GPOS pos;
	CEditRegion *pRegion;

	if(!GetDoc())
		return;

	pRegion = GetDoc()->GetRegion();
	hItem = m_NodeViewTree.GetSelectedItem( );
	pNode = ( CWorldNode * )m_NodeViewTree.GetItemData( hItem );
	ASSERT( pNode );

	pRegion->AddNodeToPath(pNode);

	UpdateNodeImage(pNode);
	GetDoc( )->RedrawAllViews( );
}


void CNodeView::OnUnsetPath()
{
	HTREEITEM hItem;
	CWorldNode *pNode, *pParent;
	GPOS pos;
	CEditRegion *pRegion;
	DWORD index;

	if(!GetDoc())
		return;

	pRegion = GetDoc()->GetRegion();
	hItem = m_NodeViewTree.GetSelectedItem( );
	pNode = ( CWorldNode * )m_NodeViewTree.GetItemData( hItem );
	ASSERT( pNode );

	pRegion->RemoveNodeFromPath(pNode);

	UpdateNodeImage(pNode);
	GetDoc( )->RedrawAllViews( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeView::OnPopupSetActiveParent
//
//	PURPOSE:	Set the active parent to the selected node
//
// ----------------------------------------------------------------------- //

void CNodeView::OnPopupSetActiveParent() 
{
	// Get the document
	CRegionDoc *pDoc = GetDoc( );
	if( pDoc == NULL )
		return;

	// Get the region and the node that was clicked on
	CEditRegion *pRegion=GetDoc()->GetRegion();;
	HTREEITEM hItem = m_NodeViewTree.GetSelectedItem( );
	CWorldNode *pNode= ( CWorldNode * )m_NodeViewTree.GetItemData( hItem );

	ASSERT( pNode );
	
	if (pNode)
	{
		// Get the current active parent
		CWorldNode *pCurrentActiveParent=pRegion->GetActiveParentNode();

		// Set the new active parent
		pRegion->SetActiveParentNode(pNode);

		// Update the node images
		UpdateNodeImage(pNode);
		UpdateNodeImage(pCurrentActiveParent);
	}
	
	GetDoc( )->RedrawAllViews( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeView::GetNodeLabel
//
//	PURPOSE:	Returns the label that should be used for a node given the current view mode
//
// ----------------------------------------------------------------------- //
CString	CNodeView::GetNodeLabel(CWorldNode *pNode, int nViewType)
{
	CString sLabel;

	// Special case container nodes
	if (pNode->GetType() == Node_Null)
	{
		sLabel=pNode->GetNodeLabel();

		// If the label is empty, then use container instead
		if (sLabel.GetLength() == 0)
		{
			pNode->SetNodeLabel("Container");
			sLabel=pNode->GetNodeLabel();
		}
		return sLabel;
	}
	else
	{
		// Switch on the current view type (Node Name, Class Type, or Both)
		switch (nViewType)
		{
		case kViewName:
			{			
				return pNode->GetName();
				break;
			}
		case kViewClass:
			{
				return pNode->GetClassName();
				break;
			}
		case kViewBoth:
			{
				CString sLabel;
				sLabel.Format("%s (%s)", pNode->GetName(), pNode->GetClassName());
				return sLabel;				
			}
		default:
			{
				ASSERT(FALSE);
			}
		}
	}

	ASSERT(FALSE);
	return "Unknown";
}

void CNodeView::OnRadioNodeViewName() 
{
	UpdateData();

	// Update the labels
	if (m_pDoc)
	{
		RecurseUpdateLabels(m_pDoc->GetRegion( )->GetRootNode());
	}
}

void CNodeView::OnRadioNodeViewClass() 
{
	UpdateData();

	// Update the labels
	if (m_pDoc)
	{
		RecurseUpdateLabels(m_pDoc->GetRegion( )->GetRootNode());	
	}
}

void CNodeView::OnRadioNodeViewBoth() 
{
	UpdateData();

	// Update the labels
	if (m_pDoc)
	{
		RecurseUpdateLabels(m_pDoc->GetRegion( )->GetRootNode());
	}
}

/************************************************************************/
// Rename the current node
void CNodeView::OnPopupRename() 
{
	HTREEITEM hSelectedItem=m_NodeViewTree.GetSelectedItem();
	if (hSelectedItem)
	{
		m_NodeViewTree.EditLabel(hSelectedItem);
	}
}

/************************************************************************/
// Moves the tagged nodes to the selected node
void CNodeView::OnPopupMoveTaggedNodes() 
{
	// Get the selected node	
	CWorldNode *pNode=GetSelectedNode();
	if (!pNode)
	{
		return;
	}

	// Get the document
	CRegionDoc *pDoc=GetDoc();
	if (!pDoc)
	{
		return;
	}

	// Move the nodes
	pDoc->MoveSelectedNodes(pNode);
}

/************************************************************************/
// The Refresh command was selected
void CNodeView::OnPopupRefresh() 
{
	if (m_pDoc != NULL)
	{						
		m_NodeViewTree.SortNodeAndChildren(m_NodeViewTree.GetRootItem(), TreeSortFunc);
	}	
}

/************************************************************************/
// This goes to the next tagged node in the node view
void CNodeView::OnGotoNextSelectedNode() 
{
	// Make sure that we have a document
	if (!m_pDoc)
	{
		return;
	}

	// Get the region
	CEditRegion *pRegion=m_pDoc->GetRegion();

	// Make sure that there is a region and a selected node
	if (!(pRegion && pRegion->GetNumSelections() > 0))
	{
		return;
	}
		
	// Get the currently selected node
	HTREEITEM hSelectedItem=m_NodeViewTree.GetSelectedItem();

	// If there isn't a selected item, then set the root as the selected item
	if (hSelectedItem == NULL)
	{
		hSelectedItem=m_NodeViewTree.GetRootItem();
	}
	
	// Find the next selected node, starting at the root and passing the selected node
	BOOL bFound=FALSE;
	CWorldNode *pNextNode=RecurseFindNextTaggedNode(m_NodeViewTree.GetRootItem(), hSelectedItem, bFound);

	// If the node wasn't found, then start searching from the root again but this time
	// the first tagged node qualifies since one wasn't found after the selected node.
	if (!pNextNode)
	{
		bFound=TRUE;
		pNextNode=RecurseFindNextTaggedNode(m_NodeViewTree.GetRootItem(), hSelectedItem, bFound);
	}

	// Highlight the node
	if (pNextNode)
	{
		HighlightNode(pNextNode);
	}
}

/************************************************************************/
// Find the next selected node, starting at the parent and passing the selected node
CWorldNode *CNodeView::RecurseFindNextTaggedNode(HTREEITEM hParentItem, HTREEITEM hSelectedItem, BOOL &bFoundRef)
{
	// Check to see if this is the selected node
	if (hParentItem == hSelectedItem)
	{
		bFoundRef=TRUE;
	}
	else
	{
		// Get the node
		CWorldNode *pParentNode=(CWorldNode *)m_NodeViewTree.GetItemData(hParentItem);
		if (!pParentNode)
		{
			ASSERT(FALSE);
			return NULL;
		}

		// If this node tagged, then return it if we have already passed the selected node
		if (pParentNode->IsFlagSet(NODEFLAG_SELECTED) && bFoundRef)
		{
			return pParentNode;
		}
	}
	

	// Check the children items
	if (m_NodeViewTree.ItemHasChildren(hParentItem))
	{
		CWorldNode *pFoundNode=RecurseFindNextTaggedNode(m_NodeViewTree.GetChildItem(hParentItem), hSelectedItem, bFoundRef);

		if (pFoundNode)
		{
			return pFoundNode;
		}
	}

	// Check the sibling items
	HTREEITEM hSibling=m_NodeViewTree.GetNextSiblingItem(hParentItem);
	if (hSibling)
	{
		// Search the sibling item
		CWorldNode *pFoundNode=RecurseFindNextTaggedNode(hSibling, hSelectedItem, bFoundRef);
		if (pFoundNode)
		{
			return pFoundNode;
		}	
	}

	// The node was not found
	return NULL;
}

/************************************************************************/
// This is called to collapse a node and all its children nodes
void CNodeView::OnPopupCollapseChildren() 
{
	// Collapse the children of the selected item
	m_NodeViewTree.CollapseChildren(m_NodeViewTree.GetSelectedItem(), TRUE /* Recurse children */ );

	// Ensure that the selected node is visible
	m_NodeViewTree.EnsureVisible(m_NodeViewTree.GetSelectedItem());
}

BOOL CNodeView::OnControlBarClose() 
{
	// Call the base class
	return CMRCSizeDialogBar::OnControlBarClose();
}

//callback for reporting errors while instantiating prefabs
static void ReportPrefabError(const char* pszError)
{
	GetDebugDlg()->ShowWindow(SW_SHOW);
	GetDebugDlg()->AddMessage(pszError);
}

void CNodeView::OnPopupDisconnectPrefab()
{
	CWorldNode *pNode = GetSelectedNode();

	if (!pNode)
		return;

	if (pNode->m_Type != Node_PrefabRef)
		return;

	CPrefabRef *pPrefabRef = (CPrefabRef*)pNode;

	PreActionList undoList;

	CWorldNode *pNewRoot = pPrefabRef->InstantiatePrefab(GetDoc()->GetRegion(), ReportPrefabError, &undoList);

	HighlightNode(pNewRoot);

	undoList.AddTail(new CPreAction(ACTION_MODIFYNODE, pPrefabRef));
	GetDoc()->Modify(&undoList);

	no_DestroyNode(GetDoc()->GetRegion(), pPrefabRef, TRUE);

	GetDoc()->GetRegion()->ClearSelections();
	GetDoc()->GetRegion()->RecurseAndSelect(pNewRoot);

	GetDoc()->UpdateSelectionBox();
	GetDoc()->RedrawAllViews();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeView::OnDeleteEmptyContainers
//
//	PURPOSE:	Recursive wrapper for RecurseDeleteEmptyContainers()
//				that obtains a pointer to the currently selected node, and
//				passes it into the function.  
//				(Invoked from the NodeView Popup)
//
// ----------------------------------------------------------------------- //
void CNodeView::OnDeleteEmptyContainers()
{
	CWorldNode *pNode = GetSelectedNode();

	ASSERT(pNode != LTNULL);

	RecurseDeleteEmptyContainers(pNode);

	m_pDoc->NotifySelectionChange();
	m_pDoc->SetModifiedFlag();

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeView::RecurseDeleteEmptyContainers
//
//	PURPOSE:	Given a node in the world tree, removes branches of empty
//				container nodes.
//
// ----------------------------------------------------------------------- //
void CNodeView::RecurseDeleteEmptyContainers( CWorldNode *pNode )
{
	GPOS pos;
	for(pos=pNode->m_Children; pos; )
	{
		if(!pos)
		{
			return;
		}

		RecurseDeleteEmptyContainers(pNode->m_Children.GetNext(pos));
	}

	// Check to see if this node is a container
	if(pNode->GetType() == Node_Null)
	{
		GPOS child;
		child = pNode->m_Children;

		// If the container node doesn't have children... BLAM!!!
		if(!child)
		{
			// Remove this node from node view tree...
			RemoveFromTree( pNode );
			DeleteNode( pNode );

			if( pNode->GetParent( ))
			{
				pNode->RemoveFromTree( );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeView::OnDeleteCascadeContainers
//
//	PURPOSE:	Recursive wrapper for RecurseDeleteSuperfluousContainers()
//				that obtains a pointer to the currently selected node, and
//				passes it into the function.  
//				(Invoked from the NodeView Popup)
//
// ----------------------------------------------------------------------- //
void CNodeView::OnDeleteCascadeContainers()
{
	CWorldNode *pNode = GetSelectedNode();

	ASSERT(pNode != LTNULL);

	RecurseDeleteSuperfluousContainers(pNode);

	m_pDoc->NotifySelectionChange();
	m_pDoc->SetModifiedFlag();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNodeView::RecurseDeleteSuperfluousContainers
//
//	PURPOSE:	Given a node in the world tree, removes all redundant
//				container nodes (removes single child nodes that are 
//				containers)
//
// ----------------------------------------------------------------------- //
void CNodeView::RecurseDeleteSuperfluousContainers( CWorldNode *pNode )
{
	GPOS pos;
	for(pos=pNode->m_Children; pos; )
	{
		if(!pos)
		{
			return;
		}

		RecurseDeleteSuperfluousContainers(pNode->m_Children.GetNext(pos));
	}

	// Check to see if this node is a container
	if(pNode->GetType() == Node_Null)
	{
		// Make sure that we are not deleting a container that is a child of the root
		CWorldNode * pParent = pNode->GetParent();
		if((pParent) && (pParent != m_pDoc->GetRegion()->GetRootNode()))
		{
			// Make sure there is only one child of the current node
			if(pNode->m_Children.GetCount() == 1)
			{
				// Make sure that the child is a container
				CWorldNode * pChild = pNode->m_Children.GetTail();
				if(pChild->GetType() == Node_Null)
				{
					//Remove this node!
					RemoveFromTree(pNode);
					DeleteNode(pNode);

					if(pNode->GetParent())
					{
						pNode->RemoveFromTree();
					}				
				}
			}
		}
	}
}

