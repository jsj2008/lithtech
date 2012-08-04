#ifndef __LINKLIST_H__
#define __LINKLIST_H__


	#include "struct_bank.h"

	#ifndef ASSERT
		#define ASSERT
	#endif

	#ifndef DWORD
		#define DWORD	unsigned long
	#endif

	typedef void*	LPOS;


	template<class T>
	class CLLNode
	{
		public:
			
			CLLNode<T>		*m_pPrev;
			CLLNode<T>		*m_pNext;
			T				m_pData;
	};

	
	extern StructBank g_LinkListBank;
	extern int g_LinkListRefCount;



	template<class T>
	class CLinkedList
	{
		protected:
			
				
		
		public:

						CLinkedList()
						{
							if(g_LinkListRefCount == 0)
							{
								sb_Init(&g_LinkListBank, sizeof(CLLNode<T>), 50);
							}

							++g_LinkListRefCount;
							m_nElements = 0;
							m_pHead = NULL;
						}

						~CLinkedList()
						{
							RemoveAll();

							--g_LinkListRefCount;
							if(g_LinkListRefCount == 0)
							{
								sb_Term(&g_LinkListBank);
							}
						}

			
			DWORD		GetSize() const		{ return m_nElements; }
			DWORD		GetCount() const		{ return m_nElements; }
			int			IsEmpty()  const		{ return !m_pHead; }

						operator LPOS() {return GetHeadPosition();}
						operator unsigned long() {return m_nElements;}

			T&			GetHead()				{ ASSERT(m_pHead);  return m_pHead->m_pData; }
			T			GetHead()	const		{ ASSERT(m_pHead);  return m_pHead->m_pData; }
			T&			GetTail()				{ ASSERT(m_pHead);	return m_pHead->m_pPrev->m_pData; }
			T			GetTail()	const		{ ASSERT(m_pHead);	return m_pHead->m_pPrev->m_pData; }

			T			RemoveHead();
			T			RemoveTail();

			LPOS		Append(T toAppend) {return AddTail(toAppend);}

			LPOS		AddHead( T newHead );
			LPOS		AddTail( T newTail );

			void		RemoveAll();

			LPOS			GetHeadPosition()	const	{ return m_pHead; }
			LPOS			GetTailPosition()	const	{ return (m_pHead ? m_pHead->m_pPrev : NULL); }

			T&			GetNext( LPOS &lPos )
			{
				CLLNode<T>		**pNode = (CLLNode<T>**)&lPos;
				T				&ret = (*pNode)->m_pData;

				if( (*pNode)->m_pNext == m_pHead )
					*pNode = NULL;
				else
					*pNode = (*pNode)->m_pNext;

				return ret;
			}

			T			GetNext( LPOS &lPos )	const
			{
				CLLNode<T>		**pFirstNode = (CLLNode<T>**)&lPos;
				CLLNode<T>		**pNode = pFirstNode;
				
				if( (*pNode)->m_pNext == m_pHead )
					*pNode = NULL;
				else
					*pNode = (*pNode)->m_pNext;

				return (*pFirstNode)->m_pData;
			}

			T&			GetPrev( LPOS &lPos )
			{
				CLLNode<T>		**pNode = (CLLNode<T>**)&lPos;
				T				&ret = (*pNode)->m_pData;

				if( (*pNode)->m_pPrev == m_pHead )
					*pNode = NULL;
				else
					*pNode = (*pNode)->m_pPrev;

				return ret;
			}
			
			T			GetPrev( LPOS &lPos )	const
			{
				CLLNode<T>		**pFirstNode = (CLLNode<T>**)&lPos;
				CLLNode<T>		**pNode = pFirstNode;

				if( (*pNode)->m_pPrev == m_pHead )
					*pNode = NULL;
				else
					*pNode = (*pNode)->m_pPrev;

				return (*pFirstNode)->m_pData;
			}

			T&			GetAt( LPOS &lPos )			{ return ((CLLNode<T>*)lPos)->m_pData; }
			T			GetAt( LPOS &lPos )	const	{ return ((CLLNode<T>*)lPos)->m_pData; }
			
			void		SetAt( LPOS &lPos, T el )		{ ((CLLNode<T>*)lPos)->m_pData = el; }
			void		RemoveAt( LPOS lPos );

			LPOS			InsertBefore( LPOS lPos, T el );
			LPOS			InsertAfter( LPOS lPos, T el );

			LPOS			Find( T searchFor )			const;
			LPOS			FindIndex( DWORD index )	const;




		protected:

			DWORD		m_nElements;
			CLLNode<T>	*m_pHead;

	};


	
	template<class T>
	T CLinkedList<T>::RemoveHead()
	{
		T	ret;

		ASSERT( m_pHead );
		ret = m_pHead->m_pData;
		RemoveAt( m_pHead );
		
		return ret;
	}


	template<class T>
	T CLinkedList<T>::RemoveTail()
	{
		T	ret;

		ASSERT( m_pHead );
		ret = m_pHead->m_pPrev->m_pData;
		RemoveAt( m_pHead->m_pPrev );
		
		return ret;
	}

	
	template<class T>
	LPOS CLinkedList<T>::AddHead( T newHead )
	{
		CLLNode<T>	*pNew = (CLLNode<T>*)sb_Allocate(&g_LinkListBank);

		pNew->m_pData = newHead;
		if( m_pHead )
		{
			pNew->m_pNext = m_pHead;
			pNew->m_pPrev = m_pHead->m_pPrev;
			m_pHead->m_pPrev = pNew;
		}
		else
		{
			pNew->m_pNext = pNew;
			pNew->m_pPrev = pNew;
		}

		m_pHead = pNew;
		++m_nElements;

		return m_pHead;
	}


	template<class T>
	LPOS CLinkedList<T>::AddTail( T newTail )
	{
		if( m_pHead )
			return InsertAfter( (LPOS)m_pHead->m_pPrev, newTail );
		else
			return AddHead( newTail );
	}


	template<class T>
	void CLinkedList<T>::RemoveAll()
	{
		CLLNode<T>	*pCur, *pNext;

		if( m_pHead )
		{
			pCur = m_pHead;
			do
			{
				pNext = pCur->m_pNext;
				sb_Free(&g_LinkListBank, pCur);
				pCur = pNext;
			} while( pCur != m_pHead );
				
			m_nElements = 0;
			m_pHead = NULL;
		}
	}


	template<class T>
	void CLinkedList<T>::RemoveAt( LPOS pos )
	{
		CLLNode<T>		*pNode = (CLLNode<T>*)pos;

		ASSERT( m_nElements > 0 );
		
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		
		if( pos == m_pHead )
			m_pHead = m_pHead->m_pNext;

		sb_Free(&g_LinkListBank, pNode);
		--m_nElements;

		if( m_nElements == 0 )
			m_pHead = NULL;
	}
	

	template<class T>
	LPOS CLinkedList<T>::InsertBefore( LPOS lPos, T el )
	{
		CLLNode<T>	*pNode = (CLLNode<T>*)lPos;
		CLLNode<T>	*pNew = (CLLNode<T>*)sb_Allocate(&g_LinkListBank);

		pNew->m_pData = el;
		pNew->m_pNext = pNode;
		pNew->m_pPrev = pNode->m_pPrev;
		pNew->m_pPrev->m_pNext = pNew->m_pNext->m_pPrev = pNew;
		
		++m_nElements;
		if( pNew->m_pNext == m_pHead )
			m_pHead = pNew;

		return pNew;
	}


	template<class T>
	LPOS CLinkedList<T>::InsertAfter( LPOS lPos, T el )
	{
		CLLNode<T>	*pNode = (CLLNode<T>*)lPos;
		CLLNode<T>	*pNew = (CLLNode<T>*)sb_Allocate(&g_LinkListBank);

		pNew->m_pData = el;
		pNew->m_pNext = pNode->m_pNext;
		pNew->m_pPrev = pNode;
		pNew->m_pPrev->m_pNext = pNew->m_pNext->m_pPrev = pNew;
		
		++m_nElements;
		return pNew;
	}


	template<class T>
	LPOS CLinkedList<T>::Find( T searchFor )			const
	{
		CLLNode<T>	*pCur;
		
		if( m_pHead )
		{
			pCur = m_pHead;
			do
			{
			
				if( pCur->m_pData == searchFor )
					return pCur;

				pCur = pCur->m_pNext;
			
			} while( pCur != m_pHead );
		}

		return NULL;
	}


	template<class T>
	LPOS CLinkedList<T>::FindIndex( DWORD index )	const
	{
		CLLNode<T>	*pCur;
		
		if( index >= m_nElements )
			return NULL;
	
		pCur = m_pHead;
		do
		{
		
			if( --index == 0 )
				return pCur;

			pCur = pCur->m_pNext;

		} while( pCur != m_pHead );
	
		ASSERT( FALSE );	// Shouldn't ever get here.
		return NULL;
	}




	// Useful template function if your linked list is made up of pointers.
	template<class T>
	void DeleteAndRemoveElements(CLinkedList<T> &theList)
	{
		LPOS lPos;

		for( lPos=theList.GetHeadPosition(); lPos != NULL; )
			delete theList.GetNext(lPos);

		theList.RemoveAll();
	}


#endif // __LINKLIST_H__
	





