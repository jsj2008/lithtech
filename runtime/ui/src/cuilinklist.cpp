//-------------------------------------------------------------------
//
//   MODULE    : CUIFONTMANAGER.CPP
//
//   PURPOSE   : implements the CUILinkList and CUIListNode classes
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUILINKLIST_H__
#include "cuilinklist.h"
#endif

#ifndef __CUIDEBUG_H__
#include "cuidebug.h"
#endif


//
//	NODE members
//

//  ---------------------------------------------------------------------------
CUIListNode::CUIListNode(void* data)
{
	m_pData = data;
	m_pNext = NULL;
	m_pPrev = NULL;
}


//
//	LIST members
//

//  ---------------------------------------------------------------------------
CUILinkList::CUILinkList()
{
	m_pHead = NULL;
	m_pTail = NULL;
	m_size = 0;
}


//  ---------------------------------------------------------------------------
CUILinkList::~CUILinkList()
{
	while (m_size) this->Remove(m_pHead);
}


//  ---------------------------------------------------------------------------
CUIListNode* CUILinkList::GetHead()
{
	return m_pHead;
}


//  ---------------------------------------------------------------------------
CUIListNode* CUILinkList::GetTail()
{
	return m_pTail;
}


//  ---------------------------------------------------------------------------
CUIListNode* CUILinkList::GetNext(CUIListNode* pNode)
{
	if (pNode) return pNode->m_pNext;
	else	   return NULL;
}


//  ---------------------------------------------------------------------------
CUIListNode* CUILinkList::GetPrev(CUIListNode* pNode)
{
	if (pNode) return pNode->m_pPrev;
	else       return NULL;
}


//  ---------------------------------------------------------------------------
void* CUILinkList::GetData(CUIListNode* pNode)
{
	if (pNode) return pNode->m_pData;
	else	   return NULL;
}


//  ---------------------------------------------------------------------------
CUIListNode* CUILinkList::Find(void* pData)	
{
	CUIListNode* pNode = m_pHead;

	while (pNode) {
		
		if (pNode->m_pData == pData) {
			return pNode;
		}
		
		pNode = this->GetNext(pNode);
	}
	
	// didn't find what we were looking for...
	return NULL;
}


//  ---------------------------------------------------------------------------
void CUILinkList::Add(void* pData)
{
	CUIListNode* pNode;
	LT_MEM_TRACK_ALLOC(pNode = new CUIListNode(pData),LT_MEM_TYPE_UI);
	
	if (!m_pTail) {
		// list was empty
		m_pHead = pNode;		
	}
	else {
		m_pTail->m_pNext = pNode;		
	}
	
	pNode->m_pPrev = m_pTail;
	
	m_pTail = pNode;
	
	m_size++;
}


//  ---------------------------------------------------------------------------
bool CUILinkList::Remove(CUIListNode* pNode)
{
	if (!pNode) return false;
	
	CUIListNode* pNext = pNode->m_pNext;
	CUIListNode* pPrev = pNode->m_pPrev;
	
	// is our node the only node?
	if (m_pHead == pNode && m_pTail == pNode) {
		m_pHead = NULL;
		m_pTail = NULL;
	}
	else {
		// is our node the head?
		if (m_pHead == pNode) {
			m_pHead = pNext;
			if (m_pHead) m_pHead->m_pPrev = NULL;		
		} 
		else {
			// is our node the tail?
			if (m_pTail == pNode) {
				m_pTail = pPrev;
				if (m_pTail) m_pTail->m_pNext = NULL;						
			}
			else {
				// normal removal
				pNext->m_pPrev = pPrev;
				pPrev->m_pNext = pNext;			
			}
		}
	}
	
	delete pNode;
	m_size--;

	return true;
}
		

//  ---------------------------------------------------------------------------
bool CUILinkList::InsertBefore(CUIListNode* pNode, void* pData)
{
	if (!pNode) return false;
	 
	CUIListNode* pIns;
	LT_MEM_TRACK_ALLOC(pIns = new CUIListNode(pData),LT_MEM_TYPE_UI);
	
	if (pNode == m_pHead) {
		m_pHead = pIns;
	}
	else {
		pNode->m_pPrev->m_pNext = pIns;
	}

	pIns->m_pNext = pNode;
	pIns->m_pPrev = pNode->m_pPrev; 
	pNode->m_pPrev = pIns;	
	
	m_size++;

	return true;
}


//  ---------------------------------------------------------------------------
bool CUILinkList::InsertAfter(CUIListNode* pNode, void* pData)
{
	if (!pNode) return false;
	 
	CUIListNode* pIns;
	LT_MEM_TRACK_ALLOC(pIns = new CUIListNode(pData),LT_MEM_TYPE_UI);
	
	if (pNode == m_pTail) {
		m_pTail = pIns;
	}
	else {
		pNode->m_pNext->m_pPrev = pIns;
	}

	pIns->m_pPrev = pNode;
	pIns->m_pNext = pNode->m_pNext; 
	pNode->m_pNext = pIns;
	
	m_size++;	

	return true;
}


