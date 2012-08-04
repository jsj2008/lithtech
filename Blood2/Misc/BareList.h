/****************************************************************************
;
;	MODULE:		BARELIST (.H)
;
;	PURPOSE:
;
;	HISTORY:	05/29/95 [m]
;
;	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#ifndef _BARELIST_H_
#define _BARELIST_H_


// Libs...

#ifdef _DEBUG
#pragma comment (lib, "BareList.lib")
#else
#pragma comment (lib, "BareList.lib")
#endif


// Classes...

// User must derive all list elements from this class.
class CBareListItem {
public:
	CBareListItem* 	Next()		{ return m_pNext; };							// Returns the next element in the list after this one
	CBareListItem*	Prev()		{ return m_pPrev; };							// Returns the previous element in the list before this one

public:
	friend class CBareList;

protected:
	CBareListItem*	m_pNext;
	CBareListItem*	m_pPrev;	
};


// User can derive new list classes from this class, or use it as is.
class CBareList {
public:
	// Constructors and destructors
	CBareList() { m_nCount = 0; m_pFirst = 0; m_pLast = 0; };
	~CBareList() { };

	// member insert and delete functions
	void	Insert(CBareListItem* pItem) { InsertFirst(pItem); };				// Same as InsertFirst
	void	InsertFirst(CBareListItem* pItem);									// Inserts item at the start of the list
	void	InsertLast(CBareListItem* pItem);									// Inserts item at the end of the list
	void	InsertAfter(CBareListItem* pBeforeItem, CBareListItem* pNewItem);	// Inserts item after pBeforeItem in the list (if pBeforeItem is NULL puts at start of list)
	void	InsertBefore(CBareListItem* pAfterItem, CBareListItem* pNewItem);	// Inserts item before pAfterItem in the list (if pAfterItem is NULL puts at end of list)
	void	Delete(CBareListItem* pItem);										// Removes the given item from the list

	// member access functions
	CBareListItem*	GetFirst() { return m_pFirst; };							// Returns the first element in the list (NULL if list is empty)
	CBareListItem*	GetLast() { return m_pLast; };								// Returns the last element in the list (NULL if list is empty)

	// this function will reinitialize the list first and last pointer making the list seem empty without deleting the items
	// this does not clean up the item pointers so this should only be used when the items in the list will never be used again
	void FastDeleteAll() { m_nCount = 0; m_pFirst = 0; m_pLast = 0; };

	// counter functions
	unsigned int GetNumItems() { return m_nCount; };
	int IsEmpty() { return (m_nCount == 0); };

protected:
	// internal member variables
	CBareListItem*	m_pFirst;
	CBareListItem*  m_pLast;
	unsigned int    m_nCount;
};


#endif 
