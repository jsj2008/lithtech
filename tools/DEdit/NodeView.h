//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_NODEVIEW_H__24F9D861_D691_11D0_99E3_0060970987C3__INCLUDED_)
#define AFX_NODEVIEW_H__24F9D861_D691_11D0_99E3_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NodeView.h : header file
//
#include "nodeviewtreectrl.h"

class CRegionDoc;
class CWorldNode;

enum node_image
{
	NODE_NULL = 0,
	NODE_BRUSH,
	NODE_OBJECT,
	NODE_PATH,
	NODE_PREFABREF
};

enum node_image_mask
{
	NODE_MASK_UNSELECTED = 1,
	NODE_MASK_SELECTED,
	NODE_MASK_FROZEN,
	NODE_MASK_HIDDEN,
	NODE_MASK_SELECTED_AND_HIDDEN,
	NODE_MASK_FROZEN_AND_HIDDEN
};

/////////////////////////////////////////////////////////////////////////////
// CNodeView dialog

class CNodeView : public CProjectTabControlBar
{
public:
	// View types for the view
	enum ViewType
	{
		kViewName=0,
		kViewClass,
		kViewBoth
	};

// Construction
public:
	CNodeView();   // standard constructor
	~CNodeView( );

// Dialog Data
	//{{AFX_DATA(CNodeView)
	enum { IDD = IDD_NODEVIEW_TABDLG };
	CNodeViewTreeCtrl	m_NodeViewTree;
	int					m_nViewType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNodeView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK( );
	//}}AFX_VIRTUAL

	virtual BOOL PreTranslateMessage(MSG* pMsg);

// Implementation
public:	
	// Initializes the nodeview
	void		Init( CRegionDoc *pDoc = NULL );
	void		Term( );

	void		Update( );
	void		UpdateTree( );

	HTREEITEM	AddNode( CWorldNode *pNode );
	HTREEITEM	SetNode( CWorldNode *pNode );
	void		DeleteNode( CWorldNode *pNode );
	void		UpdateNodeImage( CWorldNode *pNode, BOOL bRecurse=TRUE );
	void		UpdateTreeItemLabel ( CWorldNode *pNode );
	void		RecurseUpdateLabels(CWorldNode *pNode);
	void		RemoveFromTree( CWorldNode *pNode );
	void		ClearAllItems( );
	void		RecurseClearAllItems( CWorldNode *pNode );	

	// Returns the document
	CRegionDoc *GetDoc( ) const { return m_pDoc; }
		
	// Returns the current view type (ViewName, ViewClass, or ViewBoth)
	int			GetViewType()			{ return m_nViewType; }

	// Returns the label that should be used for a node given the current view mode
	CString		GetNodeLabel(CWorldNode *pNode, int nViewType);

	// Returns the selected WorldNode
	CWorldNode	*GetSelectedNode();

	// Highlights a node in the tree control and ensures that it is visible
	void		HighlightNode(CWorldNode *pNode);
	
	// Recursively add node and all children to the node tree
	void		AddItemsToTree(CWorldNode *pParentNode);

protected:
	void		RecurseHideAll(HTREEITEM hItem, BOOL bHide);

	// The sorting function used to sort the nodes in the tree
	static int CALLBACK TreeSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort); 

	// Expand the items with the NODEFLAG_EXPAND flag set
	void		ExpandItems(CWorldNode *pParentNode);

	// Changes the hidden status for the selected node.
	// bHide			- Set to TRUE if you wish to hide the node
	//					- Set to FALSE if you wish to show the node
	// bIncludeChildren	- Set to TRUE if you wish to also change the status of the nodes children
	void		ChangeHiddenStatusOfSelection(BOOL bHide, BOOL bIncludeChildren);	

	// Find the next selected node, starting at the parent and passing the selected node
	CWorldNode	*RecurseFindNextTaggedNode(HTREEITEM hParentItem, HTREEITEM hSelectedItem, BOOL &bFoundRef);

	CRegionDoc *m_pDoc;

	// Initializes the dialog (like OnInitDialog)
	BOOL		OnInitDialogBar();

	// The close button was pressed
	BOOL		OnControlBarClose();

	// Repositions the controls
	virtual void	RepositionControls();

	void		RecurseDeleteEmptyContainers( CWorldNode *pNode );	
	void		RecurseDeleteSuperfluousContainers( CWorldNode *pNode );	


	// Generated message map functions
	//{{AFX_MSG(CNodeView)
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnAddNullNode();
	afx_msg void OnHideNode();
	afx_msg void OnUnhideAllNodes();
	afx_msg void OnSetPath();
	afx_msg void OnUnsetPath();
	afx_msg void OnPopupHideAllNodes();
	afx_msg void OnPopupSetActiveParent();
	afx_msg void OnRadioNodeViewName();
	afx_msg void OnRadioNodeViewClass();
	afx_msg void OnRadioNodeViewBoth();
	afx_msg void OnPopupRename();
	afx_msg void OnUnhideNodeAndChildren();
	afx_msg void OnHideNodeAndChildren();
	afx_msg void OnPopupMoveTaggedNodes();
	afx_msg void OnPopupRefresh();
	afx_msg void OnGotoNextSelectedNode();
	afx_msg void OnPopupCollapseChildren();
	afx_msg void OnPopupDisconnectPrefab();
	afx_msg void OnClose();
	afx_msg void OnDeleteEmptyContainers();
	afx_msg void OnDeleteCascadeContainers();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEVIEW_H__24F9D861_D691_11D0_99E3_0060970987C3__INCLUDED_)
