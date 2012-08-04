/****************************************************************************
;
;	MODULE:		BARELIST (.CPP)
;
;	PURPOSE:
;
;	HISTORY:	05/29/95 [m]
;
;	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "BareList.h"

void CBareList::InsertFirst(CBareListItem* pItem) {
  pItem->m_pNext = m_pFirst;
  pItem->m_pPrev = 0;
  if (m_pFirst != 0) m_pFirst->m_pPrev = pItem;
  else m_pLast = pItem;
  m_pFirst = pItem;
  m_nCount++;
};

void CBareList::InsertLast(CBareListItem* pItem) {
  pItem->m_pNext = 0;
  pItem->m_pPrev = m_pLast;
  if (m_pLast != 0) m_pLast->m_pNext = pItem;
  else m_pFirst = pItem;
  m_pLast = pItem;
  m_nCount++;
};

void CBareList::InsertAfter(CBareListItem* pBeforeItem, CBareListItem* pNewItem) {
  if (pBeforeItem == 0) InsertFirst(pNewItem); // Insert at start of list if pBeforeItem is 0
  if (pBeforeItem->m_pNext != 0) pBeforeItem->m_pNext->m_pPrev = pNewItem;
  else m_pLast = pNewItem;
  pNewItem->m_pPrev = pBeforeItem;
  pNewItem->m_pNext = pBeforeItem->m_pNext;
  pBeforeItem->m_pNext = pNewItem;
  m_nCount++;
};

void CBareList::InsertBefore(CBareListItem* pAfterItem, CBareListItem* pNewItem) {
  if (pAfterItem == 0) InsertLast(pNewItem); // Insert at end of list if pAfterItem is 0
  if (pAfterItem->m_pPrev != 0) pAfterItem->m_pPrev->m_pNext = pNewItem;
  else m_pFirst = pNewItem;
  pNewItem->m_pNext= pAfterItem;
  pNewItem->m_pPrev = pAfterItem->m_pPrev;
  pAfterItem->m_pPrev = pNewItem;
  m_nCount++;
};

void CBareList::Delete(CBareListItem* pItem) {
  if (pItem->m_pPrev != 0) pItem->m_pPrev->m_pNext = pItem->m_pNext;
  else m_pFirst = pItem->m_pNext;
  if (pItem->m_pNext != 0) pItem->m_pNext->m_pPrev = pItem->m_pPrev;
  else m_pLast = pItem->m_pPrev;
  m_nCount--;
};
