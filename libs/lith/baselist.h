/****************************************************************************
;
;   MODULE:     BASELIST (.H)
;
;   PURPOSE:
;
;   HISTORY:    05/29/95 [m]
;
;   NOTICE:     Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __BASELIST_H__
#define __BASELIST_H__

#ifndef __LITHTYPES_H__
#include "lithtypes.h"
#endif


// User must derive all list elements from this class.
class CBaseListItem {
public:
    CBaseListItem*  Next()      { return m_pNext; };                            // Returns the next element in the list after this one
    CBaseListItem*  Prev()      { return m_pPrev; };                            // Returns the previous element in the list before this one

public:
    friend class CLTBaseList;

protected:
    CBaseListItem*  m_pNext;
    CBaseListItem*  m_pPrev;    
};


// User can derive new list classes from this class, or use it as is.
class CLTBaseList {
public:
    // Constructors and destructors
    CLTBaseList() { m_pFirst = NULL; m_pLast = NULL; };
    ~CLTBaseList() { };

    // member insert and delete functions
    void    Insert(CBaseListItem* pItem) { InsertFirst(pItem); };               // Same as InsertFirst
    void    InsertFirst(CBaseListItem* pItem);                                  // Inserts item at the start of the list
    void    InsertLast(CBaseListItem* pItem);                                   // Inserts item at the end of the list
    void    InsertAfter(CBaseListItem* pBeforeItem, CBaseListItem* pNewItem);   // Inserts item after pBeforeItem in the list (if pBeforeItem is NULL puts at start of list)
    void    InsertBefore(CBaseListItem* pAfterItem, CBaseListItem* pNewItem);   // Inserts item before pAfterItem in the list (if pAfterItem is NULL puts at end of list)
    void    Delete(CBaseListItem* pItem);                                       // Removes the given item from the list

    // member access functions
    CBaseListItem*  GetFirst() { return m_pFirst; };                            // Returns the first element in the list (NULL if list is empty)
    CBaseListItem*  GetLast() { return m_pLast; };                              // Returns the last element in the list (NULL if list is empty)

    // this function will reinitialize the list first and last pointer making the list seem empty without deleting the items
    // this does not clean up the item pointers so this should only be used when the items in the list will never be used again
    void FastDeleteAll() { m_pFirst = NULL; m_pLast = NULL; };

protected:
    // internal member variables
    CBaseListItem*  m_pFirst;
    CBaseListItem*  m_pLast;
};


#endif 
