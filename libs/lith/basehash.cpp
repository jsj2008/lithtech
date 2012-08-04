
#ifdef _WINDOWS
#include "windows.h"
#endif

#include "basehash.h"


CBaseHashItem* CBaseHashItem::Next() {
  CBaseHashItem* pItem;
  pItem = (CBaseHashItem*)CBaseListItem::Next();
  if (pItem == NULL) {
    unsigned int Bin = m_nCurBin + 1;
    while (Bin < m_pParentHash->m_nNumBins) {
	  pItem = (CBaseHashItem*)m_pParentHash->m_pBinAry[Bin].m_lstItems.GetFirst();
	  if (pItem != NULL) break;
	  Bin++;
    };
  }
  return pItem;
};

CBaseHashItem* CBaseHashItem::Prev() {
  CBaseHashItem* pItem;
  pItem = (CBaseHashItem*)CBaseListItem::Prev();
  if (pItem == NULL) {
    unsigned int Bin = m_nCurBin;
    while (Bin > 0) {
	  Bin --;
	  pItem = (CBaseHashItem*)m_pParentHash->m_pBinAry[Bin].m_lstItems.GetLast();
	  if (pItem != NULL) break;
    };
  }
  return pItem;
};

CBaseHash::CBaseHash() {
  m_nNumBins = 0;
  m_pBinAry = NULL;
};

CBaseHash::CBaseHash(unsigned int NumBins) {
  m_nNumBins = NumBins;
  m_pBinAry = new CHashBin[m_nNumBins];
  ASSERT(m_pBinAry != NULL);
};	

CBaseHash::~CBaseHash() {
  if (m_pBinAry != NULL) delete [] m_pBinAry;
};

void CBaseHash::Insert(CBaseHashItem* pItem) {
  pItem->m_pParentHash = this;
  unsigned int CurBin = pItem->HashFunc();
  ASSERT(CurBin < m_nNumBins);
  pItem->m_nCurBin = CurBin;
  m_pBinAry[CurBin].m_lstItems.InsertFirst(pItem);
};

void CBaseHash::Delete(CBaseHashItem* pItem) {
  ASSERT(pItem->m_nCurBin < m_nNumBins);
  m_pBinAry[pItem->m_nCurBin].m_lstItems.Delete(pItem);
};

CBaseHashItem* CBaseHash::GetFirst() {
  unsigned int Bin = 0;
  CBaseHashItem* pElem;
  do {
    pElem = (CBaseHashItem*)m_pBinAry[Bin].m_lstItems.GetFirst();
	Bin++;
  } while ((pElem == NULL) && (Bin < m_nNumBins));
  return pElem;
};

CBaseHashItem* CBaseHash::GetLast() {
  unsigned int Bin = m_nNumBins-1;
  CBaseHashItem* pElem;
  do {
    pElem = (CBaseHashItem*)m_pBinAry[Bin].m_lstItems.GetLast();
	if (Bin > 0) Bin--;
	else break;
  } while (pElem == NULL);
  return pElem;
};

CBaseHashItem*  CBaseHash::GetFirstInBin(unsigned int Bin) {
  ASSERT(Bin < m_nNumBins);
  return (CBaseHashItem*)m_pBinAry[Bin].m_lstItems.GetFirst();
};
