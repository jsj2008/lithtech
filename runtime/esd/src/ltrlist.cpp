/****************************************************************************
;
;	MODULE:		LTRList (.CPP)
;
;	PURPOSE:	Support class for RealVideo
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrlist.h"
#include "ltrconout.h"

//-----------------------------------------------------------------------------
// LTRListNode member functions
//-----------------------------------------------------------------------------
LTRListNode::LTRListNode()
{
	m_pData = NULL;
	m_pNext = NULL;
}

//-----------------------------------------------------------------------------
LTRListNode::~LTRListNode()
{
}

//-----------------------------------------------------------------------------
// LTRList member functions
//-----------------------------------------------------------------------------
LTRList::LTRList()
{
	m_pHead = NULL;
	m_pLast = NULL;
	m_pTail = NULL;
	m_ulNumElements = 0;
}

//-----------------------------------------------------------------------------
LTRList::~LTRList()
{
}

//-----------------------------------------------------------------------------
void LTRList::AddHead(void* p)
{
    LTRListNode* pNode;
	LT_MEM_TRACK_ALLOC(pNode = new LTRListNode,LT_MEM_TYPE_MISC);
    pNode->m_pData = p;
    pNode->m_pNext = m_pHead;
    m_pHead = pNode;
    if (!pNode->m_pNext)
    {
	m_pTail = pNode;
    }
    m_ulNumElements++;
}

//-----------------------------------------------------------------------------
void LTRList::Add(void* p)
{
    LTRListNode* pNode;
	LT_MEM_TRACK_ALLOC(pNode = new LTRListNode,LT_MEM_TYPE_MISC);
    pNode->m_pData = p;
    pNode->m_pNext = NULL;

    if (m_pTail)
    {
	m_pTail->m_pNext = pNode;
	m_pTail = pNode;
    }
    else
    {
	m_pHead = pNode;
	m_pTail = pNode;
    }
    m_ulNumElements++;
}

//-----------------------------------------------------------------------------
void* LTRList::GetFirst()
{
    if (!m_pHead)
	return 0;

    m_pLast = m_pHead;
    return m_pHead->m_pData;
}

//-----------------------------------------------------------------------------
void* LTRList::GetNext()
{
    if (!m_pLast)
	return 0;

    if (!m_pLast->m_pNext)
    {
	m_pLast = 0;
	return 0;
    }

    m_pLast = m_pLast->m_pNext;
    return m_pLast->m_pData;
}

//-----------------------------------------------------------------------------
void* LTRList::RemoveHead()
{
    m_pLast = 0;
    if (!m_pHead)
	return 0;

    void* pRet = m_pHead->m_pData;
    LTRListNode* pDel = m_pHead;
    m_pHead = m_pHead->m_pNext;
    delete pDel;
    m_ulNumElements--;
    if (!m_pHead)
    {
	m_pTail = 0;
    }

    return pRet;
}

//-----------------------------------------------------------------------------
void* LTRList::GetLast()
{
    if (!m_pTail)
	return 0;

    return m_pTail->m_pData;
}

//-----------------------------------------------------------------------------
LTRListNodePosition LTRList::GetHeadPosition()
{
    return m_pHead;
}

//-----------------------------------------------------------------------------
LTRListNodePosition LTRList::GetNextPosition(LTRListNodePosition lp)
{
    if (!lp)
	return 0;

    return lp->m_pNext;
}

//-----------------------------------------------------------------------------
void LTRList::InsertAfter(LTRListNodePosition lp, void* p)
{
    if (!lp)
	return;

    LTRListNode* pNode;
	LT_MEM_TRACK_ALLOC(pNode = new LTRListNode,LT_MEM_TYPE_MISC);
    pNode->m_pData = p;
    pNode->m_pNext = lp->m_pNext;
    lp->m_pNext = pNode;
    if (lp == m_pTail)
    {
	m_pTail = pNode;
    }
    m_ulNumElements++;
}

//-----------------------------------------------------------------------------
void* LTRList::RemoveAfter(LTRListNodePosition lp)
{
    if (!lp)
	return 0;

    LTRListNode* pDel;
    pDel = lp->m_pNext;
    if (!pDel)
    {
	return 0;
    }

    void* pRet = pDel->m_pData;
    lp->m_pNext = pDel->m_pNext;
    if (m_pTail == pDel)
    {
	m_pTail = lp;
    }
    delete pDel;
    m_ulNumElements--;
    return pRet;
}

//-----------------------------------------------------------------------------
void* LTRList::GetAt(LTRListNodePosition lp)
{
    if (!lp)
	return 0;

    return lp->m_pData;
}
#endif // LITHTECH_ESD