#ifndef __FASTLINKLIST_H__
#define __FASTLINKLIST_H__


	template<class T>
	class CFastLLNode
	{
		public:
			
			T	*m_pFastLLNext, *m_pFastLLPrev;

	};


	template<class T>
	class CFastLinkedList
	{
		public:

						CFastLinkedList()
						{
							m_Head.m_pFastLLNext = m_Head.m_pFastLLPrev = &m_Tail;
							m_Tail.m_pFastLLNext = m_Tail.m_pFastLLPrev = &m_Head;
						}	
			
			// pCur = list.GetStart();
			// while(list.NotAtEnd(pCur))
			//     list.Increment(pCur);

			T			*GetStart() const
			{
				return (T*)m_Head.m_pFastLLNext;
			}

			BOOL		NotAtEnd(T *thePointer) const
			{
				return thePointer != &m_Tail;
			}

			void		Increment(T* &thePointer) const
			{
				thePointer = (T*)thePointer->m_pFastLLNext;
			}

			void		AddAfter(T *thePointer, T *pAfter)
			{
				thePointer->m_pFastLLPrev = pAfter;
				thePointer->m_pFastLLNext = pAfter->m_pFastLLNext;
				thePointer->m_pFastLLPrev->m_pFastLLNext = 
					thePointer->m_pFastLLNext->m_pFastLLPrev = thePointer;
			}

			void		AddHead(T *thePointer)
			{
				thePointer->m_pFastLLPrev = &m_Head;
				thePointer->m_pFastLLNext = m_Head.m_pFastLLNext;
				thePointer->m_pFastLLPrev->m_pFastLLNext = 
					thePointer->m_pFastLLNext->m_pFastLLPrev = thePointer;
			}

			void		RemoveAt(T *thePointer)
			{
				thePointer->m_pFastLLPrev->m_pFastLLNext = thePointer->m_pFastLLNext;
				thePointer->m_pFastLLNext->m_pFastLLPrev = thePointer->m_pFastLLPrev;
			}

			T	m_Head, m_Tail;

	};


#endif  // __FASTLINKLIST_H__

