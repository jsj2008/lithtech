/****************************************************************************
;
;   MODULE:     BASELISTCOUNTER (.H)
;
;   PURPOSE:
;
;   HISTORY:    
;
;   NOTICE:     Copyright (c) 1998, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __BASELISTCOUNTER_H__
#define __BASELISTCOUNTER_H__

#ifndef __LITHTYPES_H__
#include "lithtypes.h"
#endif

#ifndef __BASELIST_H__
#include "baselist.h"
#endif

#ifndef __LIMITS_H__
#include "limits.h"
#define __LIMITS_H__
#endif


// User must derive all list elements from this class.
class CBaseListCounterItem : public CBaseListItem {
public:
    CBaseListCounterItem*   Next()      { return (CBaseListCounterItem*)CBaseListItem::Next(); };                           // Returns the next element in the list after this one
    CBaseListCounterItem*   Prev()      { return (CBaseListCounterItem*)CBaseListItem::Prev(); };                           // Returns the previous element in the list before this one

public:
    friend class CBaseListCounter;

protected:
};


// User can derive new list classes from this class, or use it as is.
class CBaseListCounter : public CLTBaseList {
public:
    // Constructors and destructors
    CBaseListCounter() { m_nCount = 0; CLTBaseList::CLTBaseList(); };
    ~CBaseListCounter() { };

    // member insert and delete functions
    void    Insert(CBaseListItem* pItem) { InsertFirst(pItem); };               // Same as InsertFirst
    void    InsertFirst(CBaseListItem* pItem);                                  // Inserts item at the start of the list
    void    InsertLast(CBaseListItem* pItem);                                   // Inserts item at the end of the list
    void    InsertAfter(CBaseListItem* pBeforeItem, CBaseListItem* pNewItem);   // Inserts item after pBeforeItem in the list (if pBeforeItem is NULL puts at start of list)
    void    InsertBefore(CBaseListItem* pAfterItem, CBaseListItem* pNewItem);   // Inserts item before pAfterItem in the list (if pAfterItem is NULL puts at end of list)
    void    Delete(CBaseListItem* pItem);                                       // Removes the given item from the list

    // member access functions
    CBaseListCounterItem*   GetFirst() { return (CBaseListCounterItem*)CLTBaseList::GetFirst(); }; // Returns the first element in the list (NULL if list is empty)
    CBaseListCounterItem*   GetLast() { return (CBaseListCounterItem*)CLTBaseList::GetLast(); };   // Returns the last element in the list (NULL if list is empty)

    // this function will reinitialize the list first and last pointer making the list seem empty without deleting the items
    // this does not clean up the item pointers so this should only be used when the items in the list will never be used again
    void FastDeleteAll() { m_nCount = 0; CLTBaseList::FastDeleteAll(); };

    // counter functions
    unsigned int GetNumItems() { return m_nCount; };
    BOOL IsEmpty() { return (m_nCount == 0); };

protected:
    // internal member variables
    unsigned int m_nCount;

};


inline void CBaseListCounter::InsertFirst(CBaseListItem* pItem)
{
    ASSERT(m_nCount < UINT_MAX);
    m_nCount++;
    CLTBaseList::InsertFirst(pItem);
};

inline void CBaseListCounter::InsertLast(CBaseListItem* pItem)
{
    ASSERT(m_nCount < UINT_MAX);
    m_nCount++;
    CLTBaseList::InsertLast(pItem);
};

inline void CBaseListCounter::InsertAfter(CBaseListItem* pBeforeItem, CBaseListItem* pNewItem)
{
    ASSERT(m_nCount < UINT_MAX);
    m_nCount++;
    CLTBaseList::InsertAfter(pBeforeItem, pNewItem);
};

inline void CBaseListCounter::InsertBefore(CBaseListItem* pAfterItem, CBaseListItem* pNewItem)
{
    ASSERT(m_nCount < UINT_MAX);
    m_nCount++;
    CLTBaseList::InsertBefore(pAfterItem, pNewItem);
};

inline void CBaseListCounter::Delete(CBaseListItem* pItem)
{
    ASSERT(m_nCount > 0);
    m_nCount--;
    CLTBaseList::Delete(pItem);
};


#endif 
