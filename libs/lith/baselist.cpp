/****************************************************************************
;
;	MODULE:		BASELIST (.CPP)
;
;	PURPOSE:
;
;	HISTORY:	05/29/95 [m]
;
;	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#ifdef _WINDOWS
#include "windows.h"
#endif

#include "baselist.h"

void CLTBaseList::InsertFirst(CBaseListItem* pItem) {
  ASSERT(pItem != NULL);
  pItem->m_pNext = m_pFirst;
  pItem->m_pPrev = NULL;
  if (m_pFirst != NULL) m_pFirst->m_pPrev = pItem;
  else m_pLast = pItem;
  m_pFirst = pItem;
};

void CLTBaseList::InsertLast(CBaseListItem* pItem) {
  ASSERT(pItem != NULL);
  pItem->m_pNext = NULL;
  pItem->m_pPrev = m_pLast;
  if (m_pLast != NULL) m_pLast->m_pNext = pItem;
  else m_pFirst = pItem;
  m_pLast = pItem;
};

void CLTBaseList::InsertAfter(CBaseListItem* pBeforeItem, CBaseListItem* pNewItem) {
  ASSERT(pNewItem != NULL);
  if (pBeforeItem == NULL) InsertFirst(pNewItem); // Insert at start of list if pBeforeItem is NULL
  if (pBeforeItem->m_pNext != NULL) pBeforeItem->m_pNext->m_pPrev = pNewItem;
  else m_pLast = pNewItem;
  pNewItem->m_pPrev = pBeforeItem;
  pNewItem->m_pNext = pBeforeItem->m_pNext;
  pBeforeItem->m_pNext = pNewItem;
};

void CLTBaseList::InsertBefore(CBaseListItem* pAfterItem, CBaseListItem* pNewItem) {
  ASSERT(pNewItem != NULL);
  if (pAfterItem == NULL) InsertLast(pNewItem); // Insert at end of list if pAfterItem is NULL
  if (pAfterItem->m_pPrev != NULL) pAfterItem->m_pPrev->m_pNext = pNewItem;
  else m_pFirst = pNewItem;
  pNewItem->m_pNext= pAfterItem;
  pNewItem->m_pPrev = pAfterItem->m_pPrev;
  pAfterItem->m_pPrev = pNewItem;
};

void CLTBaseList::Delete(CBaseListItem* pItem) {
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
