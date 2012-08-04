//----------------------------------------------------------
//
//	MODULE	: LINKLIST.H
//
//	PUROSE	: CLinkList definition file
//
//	CREATED	: 10 / 27 / 1996
//
//----------------------------------------------------------

#ifndef __LINKLIST_H_
	#define __LINKLIST_H_

	// Includes....

	template <class T> struct CLinkListNode
	{
		public:

				// Constructor
									CLinkListNode() { m_pPrev = NULL; m_pNext = NULL; }

				// Accessors
				
				CLinkListNode*		GetPrev() { return m_pPrev; }
				CLinkListNode*		GetNext() { return m_pNext; }
				T					GetData() { return m_Data; }
		
				CLinkListNode	   *m_pPrev;
				CLinkListNode	   *m_pNext;
				T					m_Data;
		};
	
	template <class T> class CLinkList
	{				
		public:

			// Constructor
								
									CLinkList();

			// Destructor
					
									~CLinkList() { Term(); }

			// Member Functions
			
			void					Term();

			bool					AddHead(T data);
			bool					AddTail(T data);
			bool					InsertAfter(CLinkListNode<T> *pNode, T data);
			bool					InsertBefore(CLinkListNode<T> *pNode, T data);
			T						RemoveHead();
			T						RemoveTail();
			void					Remove(CLinkListNode<T> *pNode);
			void					Remove(T data);
			void					RemoveAll() { Term(); }
			CLinkListNode<T>*		Find(T data);
			T						Get(uint32 dwIndex);
			int						GetIndex(T data);

			//Given another linked list, this will add all elements onto the existing list
			//and remove them from the passed in list
			void					AppendList(CLinkList<T>& List);

			// Accessors
			
			CLinkListNode<T>*		GetHead() { return m_pHead; }
			CLinkListNode<T>*		GetTail() { return m_pTail; }
			uint32					GetSize() { return m_nSize; }

		private:

			// Member Variables

			CLinkListNode<T>	   *m_pHead;
			CLinkListNode<T>	   *m_pTail;
			uint32					m_nSize;
	};

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList()
	//
	// PURPOSE	: Standard constructor
	//
	//----------------------------------------------------------

	template <class T> inline CLinkList<T>::CLinkList()
	{
		m_pHead = NULL;
		m_pTail = NULL;
		m_nSize = 0;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::Term()
	//
	// PURPOSE	: Terminates a CLinkList
	//
	//----------------------------------------------------------

	template <class T> inline void CLinkList<T>::Term()
	{
		if (m_pHead)
		{
			CLinkListNode<T> *pNext = m_pHead;

			while (pNext)
			{
				CLinkListNode<T> *pCur = pNext;

				// Proceed to next link
				
				if (pNext) pNext = pNext->m_pNext;

				// Delete link

				delete pCur;
			}

			m_pHead = NULL;
			m_pTail = NULL;
		}		

		m_nSize = 0;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::AddHead()
	//
	// PURPOSE	: Adds an element to the head of the array
	//
	//----------------------------------------------------------

	template <class T> inline bool CLinkList<T>::AddHead(T data)
	{
		if (!m_pHead)
		{		
			m_pHead = new CLinkListNode<T>;
			if (!m_pHead) return false;

			m_pHead->m_Data = data;

			if (!m_pTail) m_pTail = m_pHead;
		}
		else
		{
			CLinkListNode<T> *pNewNode = new CLinkListNode<T>;
			if (!pNewNode) return false;
			
			m_pHead->m_pPrev  = pNewNode;
			pNewNode->m_pNext = m_pHead;
			pNewNode->m_Data  = data;

			m_pHead = pNewNode;
		}

		m_nSize ++;

		// Success !!
		
		return true;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::AddTail()
	//
	// PURPOSE	: Adds an element to the tail of the array
	//
	//----------------------------------------------------------

	template <class T> inline bool CLinkList<T>::AddTail(T data)
	{
		if (!m_pTail)
		{
			m_pTail = new CLinkListNode<T>;
			if (!m_pTail) return false;

			m_pTail->m_Data = data;

			if (!m_pHead) m_pHead = m_pTail;
		}
		else
		{
			CLinkListNode<T> *pNewNode = new CLinkListNode<T>;
			if (!pNewNode) return false;

			m_pTail->m_pNext  = pNewNode;
			pNewNode->m_pPrev = m_pTail;
			pNewNode->m_Data  = data;

			m_pTail = pNewNode;
		}

		m_nSize ++;
		
		// Success !!
		
		return true;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::AppendList()
	//
	// PURPOSE	: Given another linked list, this will add all elements onto the 
	//			  existing list and remove them from the passed in list
	//
	//----------------------------------------------------------
	template <class T> inline void CLinkList<T>::AppendList(CLinkList<T>& List)
	{
		//ignore empty lists
		if(List.m_nSize == 0)
		{
			return;
		}

		//make sure that the list has valid pointers
		assert(List.m_pHead);
		assert(List.m_pTail);

		//attach the list onto our list
		if(m_pTail)
		{
			m_pTail->m_pNext = List.m_pHead;
			List.m_pHead->m_pPrev = m_pTail;
		}
		else
		{
			m_pHead = List.m_pHead;
		}

		//update our tail pointer and counts
		m_pTail = List.m_pTail;
		m_nSize += List.m_nSize;

		//clear out the old list
		List.m_nSize = 0;
		List.m_pHead = NULL;
		List.m_pTail = NULL;
	}


	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::InsertAfter()
	//
	// PURPOSE	: Inserts an element into the list
	//
	//----------------------------------------------------------

	template <class T> inline bool CLinkList<T>::InsertAfter(CLinkListNode<T> *pNode, T data)
	{
		CLinkListNode<T> *pNewNode = new CLinkListNode<T>;
		if (!pNewNode) return false;

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
		
		return true;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::InsertBefore()
	//
	// PURPOSE	: Inserts an element into the list
	//
	//----------------------------------------------------------

	template <class T> inline bool CLinkList<T>::InsertBefore(CLinkListNode<T> *pNode, T data)
	{
		CLinkListNode *pNewNode = new CLinkListNode;
		if (!pNewNode) return false;

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
		
		return true;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::RemoveHead()
	//
	// PURPOSE	: Removes head element
	//
	//----------------------------------------------------------

	template <class T> inline T CLinkList<T>::RemoveHead()
	{
		CLinkListNode<T> *pNode;
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
			
			delete pNode;
			
			m_nSize --;
		}

		return Data;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::RemoveTail()
	//
	// PURPOSE	: Removes head element
	//
	//----------------------------------------------------------

	template <class T> inline T CLinkList<T>::RemoveTail()
	{
		CLinkListNode<T> *pNode;
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

			delete pNode;
		}

		return Data;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::Remove()
	//
	// PURPOSE	: Removes an element from the list
	//
	//----------------------------------------------------------

	template <class T> inline void CLinkList<T>::Remove(CLinkListNode<T> *pNode)
	{
		CLinkListNode<T> *pPrev = pNode->m_pPrev;
		CLinkListNode<T> *pNext = pNode->m_pNext;

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
		
		delete pNode;

		m_nSize --;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::Remove()
	//
	// PURPOSE	: Removes an element from the list (SLOW)
	//
	//----------------------------------------------------------

	template <class T> inline void CLinkList<T>::Remove(T data)
	{
		CLinkListNode<T> *pNode = m_pHead;
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
	// FUNCTION : CLinkList::Find()
	//
	// PURPOSE	: Finds a data element in the list
	//
	//----------------------------------------------------------

	template <class T> inline CLinkListNode<T>* CLinkList<T>::Find(T data)
	{
		CLinkListNode<T> *pNode = m_pHead;

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
	// FUNCTION : CLinkList::Get()
	//
	// PURPOSE	: Returns data for a given index
	//
	//----------------------------------------------------------

	template <class T> inline T CLinkList<T>::Get(uint32 dwIndex)
	{
		CLinkListNode<T> *pNode = m_pHead;

		if (!pNode) return NULL;

		for (uint32 i = 0; i < dwIndex; i ++)
		{
			if (!pNode) return 0;

			pNode = pNode->GetNext();
		}

		return pNode->GetData();
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CLinkList::GetIndex()
	//
	// PURPOSE	: Returns an index for a given pointer
	//
	//----------------------------------------------------------

	template <class T> inline int CLinkList<T>::GetIndex(T data)
	{
		int i = 0;
		CLinkListNode<T> *pNode = m_pHead;

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

#endif