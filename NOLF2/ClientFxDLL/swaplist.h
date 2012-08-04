//------------------------------------------------------------------
//
//   MODULE    : SWAPLIST.H
//
//   PURPOSE   : Implements static array using swap method
//
//   CREATED   : On 5/13/99 At 2:59:31 PM
//
//   COPYRIGHT : (C) 1999 Monolith Productions Inc
//
//------------------------------------------------------------------

#ifndef __SWAPLIST_H_
	#define __SWAPLIST_H_

	template <class T> class CSwapList
	{	
		public :
			
			// Constructor

										CSwapList()
										{
											m_pData  = NULL;
											m_dwSize = 0;
											m_dwUsed = 0;
										}

			// Destructor

										~CSwapList()
										{
											Term();
										}

			// Member Functions

			bool						Init(uint32 dwSize);
			void						Term();

			T*							Alloc();
			void						Free(T *pElem);

			// Accessors
				
			T*							GetData() { return m_pData; }
			uint32						GetSize() { return m_dwSize; }
			uint32						GetUsed() { return m_dwUsed; }
									
									
		private :					
									
			T						   *m_pData;
			uint32						m_dwSize;
			uint32						m_dwUsed;
	};

	//------------------------------------------------------------------
	//
	//   FUNCTION : Init()
	//
	//   PURPOSE  : Initialises class CSwapList
	//
	//------------------------------------------------------------------

	template <class T> inline bool CSwapList<T>::Init(uint32 dwSize)
	{
		m_pData = debug_newa( T, dwSize );
		if (!m_pData) return false;

		m_dwSize = dwSize;
		m_dwUsed = 0;

		// Success !!

		return true;
	}

	//------------------------------------------------------------------
	//
	//   FUNCTION : Term()
	//
	//   PURPOSE  : Terminates class CSwapList
	//
	//------------------------------------------------------------------

	template <class T> inline void CSwapList<T>::Term()
	{
		if (m_pData) debug_deletea( m_pData );
		m_pData  = NULL;
		m_dwSize = 0;
		m_dwUsed = 0;
	}

	//------------------------------------------------------------------
	//
	//   FUNCTION : Alloc()
	//
	//   PURPOSE  : Allocates a new element
	//
	//------------------------------------------------------------------

	template <class T> inline T* CSwapList<T>::Alloc()
	{
		if (m_dwUsed < m_dwSize)
		{
			T *pNewElem = m_pData + m_dwUsed;
			m_dwUsed ++;

			return pNewElem;
		}
		else
		{
			return NULL;
		}
	}

	//------------------------------------------------------------------
	//
	//   FUNCTION : Free()
	//
	//   PURPOSE  : Frees an element
	//
	//------------------------------------------------------------------

	template <class T> inline void CSwapList<T>::Free(T *pElem)
	{
		if (!m_dwUsed) return;
		
		if ((pElem - m_pData) == (int)(m_dwUsed - 1))
		{
			m_dwUsed --;
			return;
		}

		// Swap the last used element with this one

		*pElem = m_pData[m_dwUsed - 1];
		m_dwUsed --;
	}

#endif