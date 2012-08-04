
// Useful little template-based class.

// This allocates and frees, but never actually
// deletes the memory associated with its objects so
// you get faster allocations, but more memory is used.

// The objects you're using in here have to be derived from CGLLNode.

#ifndef __OBJECTBANK_H__
#define __OBJECTBANK_H__


	template<class T>
	class CObjectBank
	{
		public:

					~CObjectBank()
					{
						Term();
					}

			T*		AllocateObject()
			{
				T		*pRet;

				if(m_FreeObjects.GetSize() > 0)
				{
					pRet = m_FreeObjects.GetHead();
					m_FreeObjects.RemoveAt(pRet);
				}
				else
				{
					pRet = new T;
				}

				return pRet;
			}

			void	FreeObject(T *pObj)
			{
				m_FreeObjects.AddHead(pObj);
			}

			void	Term()
			{
				GDeleteAndRemoveElements(m_FreeObjects);
			}
	
		CGLinkedList<T*>	m_FreeObjects;	// Free objects.

	};




#endif  // __OBJECTBANK_H__
