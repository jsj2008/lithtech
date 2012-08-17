/****************************************************************************
;
;   MODULE:     LithBaseList (.H)
;
;   PURPOSE:
;
;   HISTORY:    04/19/98 [blb] (made from BaseList in Shared)
;
;   NOTICE:     Copyright (c) 1998, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __LITHBASELIST_H__
#define __LITHBASELIST_H__

#ifndef NULL
#define NULL 0
#endif

template<class ItemType>
class CLithBaseList;


// User must derive all list elements from this class.
template<class Type>
class CLithBaseListItem 
{
public:
    Type*   Next()      { return (Type*)m_pNext; };                         // Returns the next element in the list after this one
    Type*   Prev()      { return (Type*)m_pPrev; };                         // Returns the previous element in the list before this one

public:
    friend class CLithBaseList<Type>;

protected:
    Type*   m_pNext;
    Type*   m_pPrev;    
};


// User can derive new list classes from this class, or use it as is.
template<class ItemType>
class CLithBaseList
{
public:
    // Constructors and destructors
    CLithBaseList() { m_pFirst = NULL; m_pLast = NULL; };
    ~CLithBaseList() { };

    // member insert and delete functions
    inline  void    Insert(ItemType* pItem) { InsertFirst(pItem); };            // Same as InsertFirst
    inline  void    InsertFirst(ItemType* pItem);                               // Inserts item at the start of the list
    inline  void    InsertLast(ItemType* pItem);                                // Inserts item at the end of the list
    inline  void    InsertAfter(ItemType* pBeforeItem, ItemType* pNewItem);     // Inserts item after pBeforeItem in the list (if pBeforeItem is NULL puts at start of list)
    inline  void    InsertBefore(ItemType* pAfterItem, ItemType* pNewItem);     // Inserts item before pAfterItem in the list (if pAfterItem is NULL puts at end of list)
    inline  void    Delete(ItemType* pItem);                                    // Removes the given item from the list

    // member access functions
    ItemType*   GetFirst() { return (ItemType*)m_pFirst; };                     // Returns the first element in the list (NULL if list is empty)
    ItemType*   GetLast() { return (ItemType*)m_pLast; };                       // Returns the last element in the list (NULL if list is empty)

private:
    // internal member functions
    ItemType*  m_pFirst;
    ItemType*  m_pLast;
};

template<class ItemType>
void CLithBaseList<ItemType>::InsertFirst(ItemType* pItem) {
  ASSERT(pItem != NULL);
  pItem->m_pNext = m_pFirst;
  pItem->m_pPrev = NULL;
  if (m_pFirst != NULL) m_pFirst->m_pPrev = pItem;
  else m_pLast = pItem;
  m_pFirst = pItem;
};

template<class ItemType>
void CLithBaseList<ItemType>::InsertLast(ItemType* pItem) {
  ASSERT(pItem != NULL);
  pItem->m_pNext = NULL;
  pItem->m_pPrev = m_pLast;
  if (m_pLast != NULL) m_pLast->m_pNext = pItem;
  else m_pFirst = pItem;
  m_pLast = pItem;
};

template<class ItemType>
void CLithBaseList<ItemType>::InsertAfter(ItemType* pBeforeItem, ItemType* pNewItem) {
  ASSERT(pNewItem != NULL);
  if (pBeforeItem == NULL) InsertFirst(pNewItem); // Insert at start of list if pBeforeItem is NULL
  if (pBeforeItem->m_pNext != NULL) pBeforeItem->m_pNext->m_pPrev = pNewItem;
  else m_pLast = pNewItem;
  pNewItem->m_pPrev = pBeforeItem;
  pNewItem->m_pNext = pBeforeItem->m_pNext;
  pBeforeItem->m_pNext = pNewItem;
};

template<class ItemType>
void CLithBaseList<ItemType>::InsertBefore(ItemType* pAfterItem, ItemType* pNewItem) {
  ASSERT(pNewItem != NULL);
  if (pAfterItem == NULL) InsertLast(pNewItem); // Insert at end of list if pAfterItem is NULL
  if (pAfterItem->m_pPrev != NULL) pAfterItem->m_pPrev->m_pNext = pNewItem;
  else m_pFirst = pNewItem;
  pNewItem->m_pNext= pAfterItem;
  pNewItem->m_pPrev = pAfterItem->m_pPrev;
  pAfterItem->m_pPrev = pNewItem;
};

template<class ItemType>
void CLithBaseList<ItemType>::Delete(ItemType* pItem) {
  ASSERT(pItem != NULL);
  if (pItem->m_pPrev != NULL) pItem->m_pPrev->m_pNext = pItem->m_pNext;
  else m_pFirst = pItem->m_pNext;
  if (pItem->m_pNext != NULL) pItem->m_pNext->m_pPrev = pItem->m_pPrev;
  else m_pLast = pItem->m_pPrev;
#ifdef _debug
  pItem->m_pNext = NULL;
  pItem->m_pPrev = NULL;
#endif
};

#endif 
