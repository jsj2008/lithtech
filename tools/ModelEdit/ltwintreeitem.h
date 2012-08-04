////////////////////////////////////////////////////////////////
//
// ltwintreeitem.h
//
// This is the item class used by CLTWinTreeMgr. They form the
// basis for a tree data structure. This class is meant to be
// derived from by user data types, and then used within the
// tree. Through GetType, numerous item types can be used in a
// single tree. To access the tree, iterators are used, for more
// information, see ltwintreeitemiter.h
//
// Author: John O'Rorke
// Created: 7/21/00
// Modification History:
//
////////////////////////////////////////////////////////////////
#ifndef __LTWINTREEITEM_H__
#define __LTWINTREEITEM_H__

#ifndef __LTWINTREEICON_H__
#	include "ltwintreeicon.h"
#endif

#include "stdafx.h"

//forward reference so that we can declare it a friend
class CLTWinTreeMgr;

//forward reference the iterator class
class CLTWinTreeItemIter;

class CLTWinTreeItem
{
public:

	//unique identifier for this item
	enum	{	BASE_ITEM	= 0xff	};

	//default constructor
	CLTWinTreeItem(const char* pszText, CLTWinTreeIcon nIcon = DEFAULTICON);

	//deault destructor
	virtual ~CLTWinTreeItem();

	//creates an iterator that iterates through all the siblings, starting
	//with this current tree item. NOTE: the callee is responsible for
	//deleting the iterator
	CLTWinTreeItemIter*		CreateSiblingIter();

	//crates an iterator that iterates through all the children, starting
	//with the first child. NOTE: the callee is responsible for
	//deleting the iterator
	CLTWinTreeItemIter*		CreateChildIter();

	//used for finding equal items
	virtual BOOL operator==(const CLTWinTreeItem& rhs) const;

	//gets the unique identifier for the type of item this is
	virtual DWORD	GetType() const;

	//determines if this item is selected or not
	BOOL IsSelected() const;

	//gets the text associated with the item
	const char* GetText() const;
	
	//returns the index to the icon associated with this item
	CLTWinTreeIcon	GetIcon() const;

	//determines if this is a valid drop target for the passed
	//in item
	virtual BOOL	IsDropTarget(const CLTWinTreeItem* pItem) const;

	//determines if the text can be modified by the user
	BOOL			IsTextEditable() const;

	//sets whether or not the text is modifiable
	void			SetTextEditable(BOOL bEditable);

	//determines if this object is, or contains the object as a child
	BOOL			ContainsObject(const CLTWinTreeItem* pItem);

private:

	//make the tree manager a friend so it can handle list management
	friend class CLTWinTreeMgr;

	// make the iterator class a friend so it can look at the next
	// sibling
	friend class CLTWinTreeItemIter;

	//don't allow copying of these items
	CLTWinTreeItem(const CLTWinTreeItem& rhs)	{}

	//handle to the tree item in the actual tree control
	//note: This is UI dependent, and when porting to other
	//UIs, this should be changed
	HTREEITEM			m_hTreeItem;

	//Pointer to the next sibling in the list
	CLTWinTreeItem*		m_pNextSibling;

	//pointer to the previous sibling in the list
	CLTWinTreeItem*		m_pPrevSibling;

	//pointer to the first child
	CLTWinTreeItem*		m_pFirstChild;

	//pointer to the parent item
	CLTWinTreeItem*		m_pParent;

	//boolean to determine if this item is selected
	BOOL				m_bSelected;

	//the index for the icon of this item
	CLTWinTreeIcon		m_Icon;

	//the text for the item. This is private and can only
	//be set by the constructor so that the text on the item
	//doesn't get out of sync with the tree control. The manager
	//can also set the text
	CString				m_sText;

	//determines if the user can modify the name of this item
	BOOL				m_bEditText;
};

#endif
