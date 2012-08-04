//----------------------------------------------------------
//
//	MODULE	: FASTLIST.H
//
//	PUROSE	: CFastList definition file
//
//	CREATED	: 10 / 27 / 1996
//
//----------------------------------------------------------

#ifndef __FASTLIST_H_
	#define __FASTLIST_H_

	// Includes....

	template <class T> struct CFastListNode
	{
		public:

				// Constructor
										CFastListNode() { m_pPrev = NULL; m_pNext = NULL; }
									
				// Accessors		
									
				CFastListNode*			GetPrev() { return m_pPrev; }
				CFastListNode*			GetNext() { return m_pNext; }
				T						GetData() { return m_Data; }
									
				CFastListNode		   *m_pPrev;
				CFastListNode		   *m_pNext;
				T						m_Data;
		};
	
	template <class T> class CFastList
	{				
		public:

			// Constructor
								
										CFastList();
										CFastList(int nTotalElem);
										
			// Destructor				
										
										~CFastList() { Term(); }
										
			// Member Functions			
										
			void						Term(BOOL bDeAlloc = TRUE);
										
			BOOL						AddHead(T data);
			BOOL						AddTail(T data);
			BOOL						InsertAfter(CFastListNode<T> *pNode, T data);
			BOOL						InsertBefore(CFastListNode<T> *pNode, T data);
			T							RemoveHead();
			T							RemoveTail();
			void						Remove(CFastListNode<T> *pNode);
			void						Remove(T data);
			void						RemoveAll() { Term(); }
			CFastListNode<T>*			Find(T data);
			T							Get(DWORD dwIndex);
			int							GetIndex(T data);

			// Accessors				
										
			CFastListNode<T>*			GetBlock() { return m_pBlock; }
			CFastListNode<T>*			GetHead() { return m_pHead; }
			CFastListNode<T>*			GetTail() { return m_pTail; }
			DWORD						GetSize() { return m_nSize; }

		private:

			// Member Functions

			void						AllocMem(int nTotalElem);

			// Member Variables

			CFastListNode<T>	       *m_pBlock;
			CFastListNode<T>		   *m_pFreeList;
			CFastListNode<T>		   *m_pHead;
			CFastListNode<T>		   *m_pTail;
			DWORD						m_nSize;
			DWORD						m_nTotalElem;
			BOOL						m_bAllocated;
	};

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList()
	//
	// PURPOSE	: Standard constructor
	//
	//----------------------------------------------------------

	template <class T> inline CFastList<T>::CFastList()
	{
		m_pBlock	 = NULL;
		m_pHead		 = NULL;
		m_pTail		 = NULL;
		m_nSize		 = 0;

		// Allocate the memory

		AllocMem(100);
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList()
	//
	// PURPOSE	: Standard constructor
	//
	//----------------------------------------------------------

	template <class T> inline CFastList<T>::CFastList(int nTotalElem)
	{
		m_pBlock	 = NULL;
		m_pHead		 = NULL;
		m_pTail		 = NULL;
		m_nSize		 = 0;

		// Allocate the memory

		AllocMem(nTotalElem);
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::Term()
	//
	// PURPOSE	: Terminates a CFastList
	//
	//----------------------------------------------------------

	template <class T> inline void CFastList<T>::Term(BOOL bDeAlloc)
	{
		if (m_pBlock) debug_deletea(m_pBlock);
		m_pBlock = NULL;
		m_pHead = NULL;
		m_pTail = NULL;
		m_pFreeList = NULL;

		m_nSize = 0;

		if (!bDeAlloc) AllocMem(m_nTotalElem);
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::AddHead()
	//
	// PURPOSE	: Adds an element to the head of the array
	//
	//----------------------------------------------------------

	template <class T> inline BOOL CFastList<T>::AddHead(T data)
	{
		if (!m_pHead)
		{		
			m_pHead = debug_new(CFastListNode);
			if (!m_pRoot) return FALSE;

			m_pHead->m_Data = data;

			if (!m_pTail) m_pTail = m_pHead;
		}
		else
		{
			CFastListNode *pNewNode = debug_new(CFastListNode);
			if (!pNewNode) return FALSE;
			
			m_pHead->m_pPrev  = pNewNode;
			pNewNode->m_pNext = m_pHead;
			pNewNode->m_Data  = data;

			m_pHead = pNewNode;
		}

		m_nSize ++;

		// Success !!
		
		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::AddTail()
	//
	// PURPOSE	: Adds an element to the tail of the array
	//
	//----------------------------------------------------------

	template <class T> inline BOOL CFastList<T>::AddTail(T data)
	{
		if (!m_pTail)
		{
			m_pTail = m_pFreeList;
			if (!m_pTail) return FALSE;

			m_pFreeList = m_pFreeList->m_pNext;
			m_pFreeList->m_pPrev = NULL;
			
			m_pTail->m_pPrev = NULL;
			m_pTail->m_pNext = NULL;

			m_pTail->m_Data = data;

			if (!m_pHead) m_pHead = m_pTail;
		}
		else
		{
			CFastListNode<T> *pNewNode = m_pFreeList;
			if (!pNewNode) return FALSE;

			m_pFreeList = m_pFreeList->m_pNext;
			m_pFreeList->m_pPrev = NULL;

			pNewNode->m_pPrev = NULL;
			pNewNode->m_pNext = NULL;

			m_pTail->m_pNext  = pNewNode;
			pNewNode->m_pPrev = m_pTail;
			pNewNode->m_Data  = data;

			m_pTail = pNewNode;
		}

		m_nSize ++;
		
		// Success !!
		
		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::InsertAfter()
	//
	// PURPOSE	: Inserts an element into the list
	//
	//----------------------------------------------------------

	template <class T> inline BOOL CFastList<T>::InsertAfter(CFastListNode<T> *pNode, T data)
	{
		CFastListNode<T> *pNewNode = m_pFreeList;
		if (!pNewNode) return FALSE;

		m_pFreeList = m_pFreeList->m_pNext;
		m_pFreeList->m_pPrev = NULL;

		pNewNode->m_pNext = NULL;
		pNewNode->m_pPrev = NULL;

		// Copy in data

		pNewNode->m_Data = data;

		pNewNode->m_pPrev = pNode;
		
		if (pNode->m_pNext)
		{
			pNewNode->m_pNext = pNode->m_pNext;
			pNode->m_pNext->m_pPrev = pNewNode;
		}
		else
		{
			m_pTail = pNewNode;
		}
		
		pNode->m_pNext = pNewNode;

		m_nSize ++;
		
		// Success !!
		
		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::InsertBefore()
	//
	// PURPOSE	: Inserts an element into the list
	//
	//----------------------------------------------------------

	template <class T> inline BOOL CFastList<T>::InsertBefore(CFastListNode<T> *pNode, T data)
	{
		CFastListNode<T> *pNewNode = m_pFreeList;
		if (!pNewNode) return FALSE;

		m_pFreeList = m_pFreeList->m_pNext;
		m_pFreeList->m_pPrev = NULL;

		pNewNode->m_pNext = pNode;
		
		if (pNode->m_pPrev)
		{
			pNewNode->m_pPrev = pNode->m_pPrev;
		}
		else
		{
			m_pHead = pNewNode;
		}
		
		pNode->m_pPrev = pNewNode;

		m_nSize ++;
		
		// Success !!
		
		return TRUE;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::RemoveHead()
	//
	// PURPOSE	: Removes head element
	//
	//----------------------------------------------------------

	template <class T> inline T CFastList<T>::RemoveHead()
	{
		CFastListNode<T> *pNode;
		T Data;

		if (m_pHead)
		{
			pNode = m_pHead;
			Data  = m_pHead->m_Data;

			m_pHead = m_pHead->m_pNext;
		
			if (!m_pHead)
			{
				// List is empty..

				m_pTail = NULL;
			}
			else
			{
				if (m_pHead->m_pNext)
				{
					// Correct link
	
					m_pHead->m_pNext->m_pPrev = m_pHead;
				}
			}
			
			pNode->m_pNext = m_pFreeList->m_pNext;
			pNode->m_pPrev = m_pFreeList;
			m_pFreeList->m_pNext = pNode;
			
			m_nSize --;
		}

		return Data;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::RemoveTail()
	//
	// PURPOSE	: Removes head element
	//
	//----------------------------------------------------------

	template <class T> inline T CFastList<T>::RemoveTail()
	{
		CFastListNode<T> *pNode;
		T Data;

		if (m_pTail)
		{	
			pNode = m_pTail;
			Data  = m_pTail->m_Data;

			if (m_pTail->m_pPrev)
			{
				m_pTail = m_pTail->m_pPrev;								
			}
			else
			{
				// List is empty..

				m_pHead = NULL;
				m_pTail = NULL;
			}

			m_nSize --;

			pNode->m_pNext = m_pFreeList->m_pNext;
			pNode->m_pPrev = m_pFreeList;
			m_pFreeList->m_pNext = pNode;
		}

		return Data;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::Remove()
	//
	// PURPOSE	: Removes an element from the list
	//
	//----------------------------------------------------------

	template <class T> inline void CFastList<T>::Remove(CFastListNode<T> *pNode)
	{
		CFastListNode<T> *pPrev = pNode->m_pPrev;
		CFastListNode<T> *pNext = pNode->m_pNext;

		if ((pPrev) && (pNext))
		{
			pPrev->m_pNext = pNext;
			pNext->m_pPrev = pPrev;
		}

		if ((!pPrev) && (pNext))
		{
			pNext->m_pPrev = NULL;
			m_pHead = pNext;
		}

		if ((pPrev) && (!pNext))
		{
			pPrev->m_pNext = NULL;
			m_pTail = pPrev;
		}

		if ((!pPrev) && (!pNext))
		{
			m_pTail = NULL;
			m_pHead = NULL;
		}

		// Delete link
		
		pNode->m_pNext = m_pFreeList->m_pNext;
		pNode->m_pPrev = m_pFreeList;
		m_pFreeList->m_pNext = pNode;

		m_nSize --;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::Remove()
	//
	// PURPOSE	: Removes an element from the list (SLOW)
	//
	//----------------------------------------------------------

	template <class T> inline void CFastList<T>::Remove(T data)
	{
		CFastListNode<T> *pNode = m_pHead;
		if (!pNode) return;

		while (pNode)
		{
			if (pNode->m_Data == data)
			{
				Remove(pNode);
				return;
			}

			pNode = pNode->m_pNext;
		}
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::Find()
	//
	// PURPOSE	: Finds a data element in the list
	//
	//----------------------------------------------------------

	template <class T> inline CFastListNode<T>* CFastList<T>::Find(T data)
	{
		CFastListNode<T> *pNode = m_pHead;

		if (!pNode) return NULL;

		while (pNode)
		{
			if (pNode->m_Data == data)
			{
				return pNode;
			}

			pNode = pNode->m_pNext;
		}

		return NULL;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::Get()
	//
	// PURPOSE	: Returns data for a given index
	//
	//----------------------------------------------------------

	template <class T> inline T CFastList<T>::Get(DWORD dwIndex)
	{
		CFastListNode<T> *pNode = m_pHead;

		if (!pNode) return NULL;

		for (DWORD i = 0; i < dwIndex; i ++)
		{
			pNode = pNode->GetNext();
		}

		return pNode->GetData();
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CFastList::GetIndex()
	//
	// PURPOSE	: Returns an index for a given pointer
	//
	//----------------------------------------------------------

	template <class T> inline int CFastList<T>::GetIndex(T data)
	{
		int i = 0;
		CFastListNode<T> *pNode = m_pHead;

		if (!pNode) return NULL;

		while (pNode)
		{
			if (pNode->m_Data == data)
			{
				return i;
			}

			pNode = pNode->m_pNext;
			i ++;
		}

		return -1;
	}

	//------------------------------------------------------------------
	//
	//   FUNCTION : Alloc()
	//
	//   PURPOSE  : Allocates memory block for list and links it
	//
	//------------------------------------------------------------------

	template <class T> inline void CFastList<T>::AllocMem(int nTotalElem)
	{
		if (m_pBlock) debug_deletea(m_pBlock);

		m_pBlock	 = debug_newa(CFastListNode<T>, nTotalElem);
		m_nTotalElem = nTotalElem;

		m_pFreeList  = m_pBlock;

		// Link up all the nodes

		CFastListNode<T> *pStart = m_pBlock;

		pStart->m_pPrev = NULL;
		pStart->m_pNext = pStart + 1;
		pStart ++;

		for (int i = 1; i < nTotalElem - 1; i ++)
		{
			pStart->m_pNext = pStart + 1;
			pStart->m_pPrev = pStart - 1;

			pStart ++;
		}

		pStart->m_pPrev = pStart - 1;
		pStart->m_pNext = NULL;
	}

#endif