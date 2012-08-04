
// The 100th linked list class I've got in Stdlith :)
// This one is for REALLY simple doubly linked lists..
// You need a GLink for the list head, and this just defines
// really common routines to insert, remove, and tie off.

#ifndef __GLINK_H__
#define __GLINK_H__


typedef struct GLink_t
{
    struct GLink_t *m_pNext, *m_pPrev;
    void *m_pData;
} GLink;

inline void gn_TieOff(GLink *pLink)
{
    pLink->m_pNext = pLink->m_pPrev = pLink;
}

inline void gn_Insert(GLink *pAfter, GLink *pLink)
{
    pLink->m_pPrev = pAfter;
    pLink->m_pNext = pAfter->m_pNext;
    pLink->m_pPrev->m_pNext = pLink->m_pNext->m_pPrev = pLink;
}

inline void gn_Remove(GLink *pLink)
{
    pLink->m_pPrev->m_pNext = pLink->m_pNext;
    pLink->m_pNext->m_pPrev = pLink->m_pPrev;
}


#endif  
