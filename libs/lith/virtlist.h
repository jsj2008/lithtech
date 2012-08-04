/****************************************************************************
;
;	MODULE:		VIRTBASELIST (.H)
;
;	PURPOSE:
;
;	HISTORY:	05/29/95 [m]
;
;	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __VIRTLIST_H__
#define __VIRTLIST_H__


#ifndef __LITHTYPES_H__
#include "lithtypes.h"
#endif


// User must derive all list elements from this class.
class CVirtBaseListItem {
public:
	CVirtBaseListItem* 	Next()		{ return m_pNext; };							// Returns the next element in the list after this one
	CVirtBaseListItem*	Prev()		{ return m_pPrev; };							// Returns the previous element in the list before this one

	virtual void VirtualFoo() = 0;
public:
	friend class CVirtBaseList;

protected:
	CVirtBaseListItem*	m_pNext;
	CVirtBaseListItem*	m_pPrev;	
};


// User can derive new list classes from this class, or use it as is.
class CVirtBaseList {
public:
	// Constructors and destructors
	CVirtBaseList() { m_pFirst = NULL; m_pLast = NULL; };
	~CVirtBaseList() { };

	// member insert and delete functions
	void	Insert(CVirtBaseListItem* pItem) { InsertFirst(pItem); };				// Same as InsertFirst
	void	InsertFirst(CVirtBaseListItem* pItem);									// Inserts item at the start of the list
	void	InsertLast(CVirtBaseListItem* pItem);									// Inserts item at the end of the list
	void	InsertAfter(CVirtBaseListItem* pBeforeItem, CVirtBaseListItem* pNewItem);	// Inserts item after pBeforeItem in the list (if pBeforeItem is NULL puts at start of list)
	void	InsertBefore(CVirtBaseListItem* pAfterItem, CVirtBaseListItem* pNewItem);	// Inserts item before pAfterItem in the list (if pAfterItem is NULL puts at end of list)
	void	Delete(CVirtBaseListItem* pItem);										// Removes the given item from the list

	// member access functions
	CVirtBaseListItem*	GetFirst() { return m_pFirst; };							// Returns the first element in the list (NULL if list is empty)
	CVirtBaseListItem*	GetLast() { return m_pLast; };								// Returns the last element in the list (NULL if list is empty)

	virtual void VirtualFoo() = 0;
private:
	// internal member functions
	CVirtBaseListItem*	m_pFirst;
	CVirtBaseListItem*  m_pLast;
};


#endif 
