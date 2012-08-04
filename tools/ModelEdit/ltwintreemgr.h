////////////////////////////////////////////////////////////////
//
// ltwintreemgr.h
//
// This is the tree manager, that handles the management of
// the tree data structure as well as the management of the
// windows tree control. 
//
// Author: John O'Rorke
// Created: 7/21/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __LTWINTREEMGR_H__
#define __LTWINTREEMGR_H__

#include "stdafx.h"

#ifndef __LTWINTREEKEY_H__
#	include "ltwintreekey.h"
#endif

#ifndef __LTWINTREEICON_H__
#	include "ltwintreeicon.h"
#endif


// messages to be sent to the parent window on certain events
#define WM_LTTREE_SELCHANGED	(WM_USER+471)	// the selection has changed
#define WM_LTTREE_EDITTEXT		(WM_USER+472)	// text has been successfully edited


class CLTWinTreeItem;
class CLTWinTreeItemIter;


class CLTWinTreeMgr :
	public CTreeCtrl
{
public:

	//the callback prototype used for visiting each node
	typedef BOOL (*VisitCallback)(CLTWinTreeItem*, CLTWinTreeMgr*, DWORD);

	//callback that can be registered for when the selection changes
	typedef void (*ChangeSelCallback)(CLTWinTreeItem*, CLTWinTreeMgr*, DWORD);


	//default constructor
	CLTWinTreeMgr();

	//default destructor
	virtual ~CLTWinTreeMgr();

	//adds a newly created item to the tree after belonging to the specified parent
	// If the parent is NULLITEM it is added as a sibling to the root. It will fail
	// if it cannot find the appropriate parent. This object is then orphaned, and
	// it is the tree manager's responsibility to release the memory. Due to this
	// all the items MUST be created on the heap using new. The return value is the
	// key for the new item, which uniquely identifies it. Keys CANNOT be
	// transferred between trees
	virtual CLTWinTreeKey	AddItem(CLTWinTreeItem* pNewItem, 
									CLTWinTreeKey Parent = NULLITEM,
									BOOL bSortChildren = TRUE);

	//deletes a specified item from the tree as well as its children. Returns
	//true if it successfully found and deleted the item, false otherwise
	virtual BOOL			DeleteItem(CLTWinTreeKey Item);

	//creates an iterator that begins iterating through the root
	CLTWinTreeItemIter*		CreateIter();

	//sets the text of an item given a key
	virtual BOOL			SetItemText(CLTWinTreeKey Item, const char* pszText);

	//sets the icon of an item given a key
	BOOL					SetItemIcon(CLTWinTreeKey Item, CLTWinTreeIcon Icon);

	//converts a key to an item
	CLTWinTreeItem*	KeyToItem(CLTWinTreeKey Key);

	//converts an item to a key
	CLTWinTreeKey	ItemToKey(CLTWinTreeItem* pItem);

	//this will visit each node in turn, and call the passed calback
	//in once per item. It will continue iteration as long as the
	//function returns true and there are items left to be iterated.
	//it performs a depth first search to get each item. In addition,
	//the user data will be passed to the function each time it is
	//called. It will begin at the passed in node and visit all of the
	//children of that node. If it is null, it will start at the root
	void			VisitEachItem(	VisitCallback CallbackFunc, 
									DWORD UserData,
									CLTWinTreeKey Start = NULLITEM);

	//adds an icon to the available list, and returns the index that
	//will refer to the icon
	CLTWinTreeIcon	AddIcon(LPCTSTR pszIcon);

	//deletes the entire tree
	virtual void	DeleteTree();

	//clears the selection, setting all item's flags for set to false
	void ClearSelection();

	//expand all items in the tree
	void ExpandAll();

	// collapse all items in the tree
	void CollapseAll();

	//enables the dragging and dropping of items
	void	EnableDragDrop(BOOL bEnable = TRUE);

	//enables the editing of text in the control
	void	EnableEditText(BOOL bEnable = TRUE);

	//enables multiple selection of nodes
	void	EnableMultiSelect(BOOL bEnable = TRUE);

	//registers a callback for when selections change
	void	RegisterCallback(ChangeSelCallback Callback, DWORD nUserData);

	//unregisters the associated callback
	void	UnregisterCallback();

private:

	//NOTIFY CALLBACKS
	afx_msg void OnBeginEditText(NMHDR*, LRESULT*);
	afx_msg void OnEndEditText(NMHDR*, LRESULT*);
	afx_msg void OnBeginDrag(NMHDR*, LRESULT*);
	afx_msg void OnSelChanged(NMHDR*, LRESULT*);

	//WINDOWS MESSAGE FOR DRAGGING OPERATIONS
	afx_msg void OnMouseMove(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint pt);

	//unlinks the item from all surrounding siblings
	void	UnlinkFromSiblings(CLTWinTreeItem* pItem);

	//handles the deletion of a tree, it starts on the specified node,
	// and recursively deletes all children of that node
	void	RecurseDeleteChildren(CLTWinTreeItem* pItem);

	//returns a pointer to a valid drop target if one exists, otherwise
	//NULL
	CLTWinTreeItem*	GetDropTarget(CPoint pt);

	//recursively builds a dragged tree as a parent to the passed in
	//item.
	void RebuildDraggedTree(CLTWinTreeItem* pItem, CLTWinTreeItem* pParent);

	//selects a specified node in multiple selection mode
	void MultiSelectItem(HTREEITEM hItem, BOOL bSelect);

	//callback function to visit each node and clear the selection flag
	static BOOL ClearSelectCallback(CLTWinTreeItem*, CLTWinTreeMgr*, DWORD);

	//calback function to visit each node and expand or collapse it
	static BOOL ExpandCollapseCallback(CLTWinTreeItem* item, CLTWinTreeMgr* mgr, DWORD collapse);

	//callback function to visit each node and update their drawing method
	static BOOL UpdateItemCallback(CLTWinTreeItem*, CLTWinTreeMgr*, DWORD);

	//triggers the change selection callback
	void	TriggerChangeSel(CLTWinTreeItem* pItem);

	//determines if an item is currently being dragged
	BOOL	IsInDrag() const;


	//do not allow copying of trees
	CLTWinTreeMgr(const CLTWinTreeMgr&)	{}

	//handle to the first selected node for multiple selection
	CLTWinTreeItem*	m_pFirstSelected;

	//the root item
	CLTWinTreeItem*	m_pRoot;

	//the image list for the tree control
	CImageList		m_ImageList;

	//determines if dragging and dropping is supported
	BOOL			m_bDragDrop;

	//determines if editing text is supported
	BOOL			m_bEditText;

	//determine if we are currently in a drag mode
	BOOL			m_bDragging;

	//determines if multiple nodes can be selected
	BOOL			m_bMultiSelect;

	//the item that is currently being dragged
	CLTWinTreeItem*	m_pDragItem;

	//image list for dragged item
	CImageList*		m_pDragImage;

	//the callback to call whenever the selection changes
	ChangeSelCallback	m_pfnChangeSel;

	//user data for the change selection callback
	DWORD				m_nChangeSelUserData;

	DECLARE_MESSAGE_MAP();

};

#endif
