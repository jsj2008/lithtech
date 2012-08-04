//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_NODEVIEWTREECTRL_H__24F9D862_D691_11D0_99E3_0060970987C3__INCLUDED_)
#define AFX_NODEVIEWTREECTRL_H__24F9D862_D691_11D0_99E3_0060970987C3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// NodeViewTreeCtrl.h : header file
//

class CWorldNode;

#include "mtreectl.h"

/////////////////////////////////////////////////////////////////////////////
// CNodeViewTreeCtrl window

class CNodeViewTreeCtrl : public CMyTreeCtrl
{
// Construction
public:
	CNodeViewTreeCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNodeViewTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual		~CNodeViewTreeCtrl();
	void		OnButtonUp( );
	void		MoveItem( HTREEITEM hNode, HTREEITEM hParent );	
	void		SelectTree( HTREEITEM hItem, bool bRecurse, bool bMultiSelect=false, bool bNotifySelectionChange=true );
	void		SelectTree( CWorldNode *pNode, bool bRecurse, bool bMultiSelect=false, bool bNotifySelectionChange=true );

	HTREEITEM	FindItem( HTREEITEM hItem, CWorldNode *pNode );
	HTREEITEM	TransferItem(HTREEITEM hitem, HTREEITEM hNewParent);

	BOOL		PreTranslateMessage(MSG* pMsg);

	// Sets a pointer to the parenting node view
	void		SetNodeView(CNodeView *pView)		{ m_pNodeView=pView; }
	CNodeView	*GetNodeView()						{ return m_pNodeView; }

	// These functions must call CEditRegion::GetActiveParentNode() in order to determine if a node is to
	// be displayed bold or not.  However, GetActiveParentNode is not very effecient and therefore
	// the active parent node can be retrieved once and then passed into these functions if they are
	// to be called multiple times.  Passing in NULL means that the function will get the active parent
	// on its own through the CEditRegion::GetActiveParentNode() method.
	void	RecurseSetItemImage( HTREEITEM hItem, BOOL bRecurse, CWorldNode *pActiveParentNode=NULL );
	void	SetNodeImage( HTREEITEM hItem, CWorldNode *pActiveParentNode=NULL);

	// This function collapses a nodes children.
	// bFirstItem is used to determine if the first node is being modified while recursing.  Please
	// do not change its value from the default.
	void	CollapseChildren(HTREEITEM hItem, BOOL bRecurseChildren, BOOL bFirstItem=TRUE);

protected:
	// Sends a windows message to the region view
	void	SendMessageToRegionView(UINT message, WPARAM wParam = 0, LPARAM lParam = 0);

	// Generated message map functions
protected:
	//{{AFX_MSG(CNodeViewTreeCtrl)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEndLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
	afx_msg void OnBeginLabelEdit(LPNMHDR pnmhdr, LRESULT *pLResult);
	afx_msg void OnItemExpanded(LPNMHDR pnmhdr, LRESULT *pLResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk( UINT nFlags, CPoint point );	//(YF 11-27)
	afx_msg void OnEditPaste();
	afx_msg void OnEditPasteAlternate();
	afx_msg void OnEditCopy();
	afx_msg void OnEditUndo();
	afx_msg void OnPopupRename();
	afx_msg void OnSelectionGroup();
	afx_msg void OnPopupSetactiveParent();
	afx_msg void OnPopupRefresh();
	afx_msg void OnGotoNextSelectedNode();
	afx_msg void OnPopupCollapseChildren();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
	CNodeView		*m_pNodeView;
	BOOL			m_bEditingLabel;	// This is set to TRUE if we are editing a label
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NODEVIEWTREECTRL_H__24F9D862_D691_11D0_99E3_0060970987C3__INCLUDED_)
