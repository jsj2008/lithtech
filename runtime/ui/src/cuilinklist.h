//-------------------------------------------------------------------
//
//   MODULE    : CUILINKLIST.H
//
//   PURPOSE   : defines the CUILinkList and CUIListNode utility
//				 classes
//
//   CREATED   : 1/01 
//
//   COPYRIGHT : (C) 2001 LithTech Inc
//
//-------------------------------------------------------------------


#ifndef __CUILINKLIST_H__
#define __CUILINKLIST_H__


#ifndef __CUI_H__
#include "cui.h"
#endif


//  ---------------------------------------------------------------------------
class CUIListNode
{
	
	public:
			
		CUIListNode(void* pData);
			
	public:
	
		CUIListNode* m_pNext;
		CUIListNode* m_pPrev;
		void*        m_pData; 
};

	
//  ---------------------------------------------------------------------------
class CUILinkList
{
	public:
			
		CUILinkList();
		~CUILinkList();

		const char* GetClassName() { return "CUILinkList"; }
				
		CUIListNode* GetHead();
		CUIListNode* GetTail();
		CUIListNode* GetNext(CUIListNode* pNode);
		CUIListNode* GetPrev(CUIListNode* pNode);
		CUIListNode* Find(void* pData);
		
		void*        GetData(CUIListNode* pNode);
		
		void		 Add(void* pData);
		bool		 Remove(CUIListNode* pNode);
		
		bool		 InsertBefore(CUIListNode* pNode, void* pData);	
		bool		 InsertAfter(CUIListNode* pNode, void* pData);	
		
		// debug
		void Print();
					
	private:
		
		CUIListNode*  m_pHead;
		CUIListNode*  m_pTail;
		uint32        m_size;			
				
};


#endif //__CUILINKLIST_H__
