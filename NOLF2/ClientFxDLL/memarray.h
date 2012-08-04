//----------------------------------------------------------
//
//	MODULE	: MEMARRAY.H
//
//	PUROSE	: CMemArray definition file
//
//	CREATED	: 10 / 27 / 1996
//
//----------------------------------------------------------

#ifndef __MEMARRAY_H_
	#define __MEMARRAY_H_

	// Defines....

	#define DEFAULT_GROWBY			50

	// Class declaration....
	
	template <class T> class CMemArray
	{
		public:

			// Constructor
						
									CMemArray();

			// Destructor

									~CMemArray() { Term(); }

			// Member Functions

			void					Term();

			T*						Add(T data);
			T						Get(uint32 i);
			void					Remove(uint32 i);
			void					Remove(T pPtr);
			void					RemoveAll();
			void					SetNumElements(uint32 nElements) { m_nElements = nElements; }
			void					SetGrowBy(int GrowBy) { m_nGrowBy = GrowBy; }

			// Operators

			T operator				[] (uint32 i) { return m_pData[i]; }

			// Accessors

			T*						GetData() { return m_pData; }
			uint32					GetSize() { return m_nElements; }

		private:

			// Member Variables

			T					   *m_pData;
			uint32					m_nElements;
			uint32					m_nSize;
			uint32					m_nGrowBy;
	};

	//----------------------------------------------------------
	//
	// FUNCTION : CMemArray()
	//
	// PURPOSE	: Standard constructor
	//
	//----------------------------------------------------------

	template <class T> inline  CMemArray<T>::CMemArray()
	{
		m_pData		= NULL;
		m_nElements = 0;
		m_nSize		= 0;
		m_nGrowBy	= DEFAULT_GROWBY;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CMemArray::Term()
	//
	// PURPOSE	: Terminates a CMemArray
	//
	//----------------------------------------------------------

	template <class T> inline void CMemArray<T>::Term()
	{
		if (m_pData) debug_deletea( m_pData );

		m_pData		= NULL;
		m_nElements = 0;
		m_nSize		= 0;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CMemArray::Add()
	//
	// PURPOSE	: Adds and element to the array
	//
	//----------------------------------------------------------

	template <class T> inline T* CMemArray<T>::Add(T data)
	{		
		if ((m_nSize) && (m_nElements < m_nSize - 1))
		{
			// Simply add the data to the array

			m_pData[m_nElements ++] = data;

			return m_pData + m_nElements - 1;
		}
		else
		{
			// If m_pData is NULL, create array

			if (!m_pData)
			{
				m_pData = debug_newa( T, m_nGrowBy );
				if (!m_pData) return NULL;

				m_nSize = m_nGrowBy;

				// Add the data

				m_pData[m_nElements ++] = data;

				return m_pData + m_nElements - 1;
			}
			else
			{
				// Must grow the array

				T* pTmp = debug_newa( T, m_nSize + m_nGrowBy );
				if (!pTmp) return NULL;

				m_nSize += m_nGrowBy;

				// Copy m_pData to pTmp;

				memcpy(pTmp, m_pData, m_nElements * sizeof(T));

				// Delete the old array

				debug_deletea( m_pData );

				// And set the new array

				m_pData = pTmp;

				// Add the data

				m_pData[m_nElements ++] = data;

				return m_pData + m_nElements - 1;
			}
		}

		return NULL;
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CMemArray::Get()
	//
	// PURPOSE	: Gets an entry in the array
	//
	//----------------------------------------------------------

	template <class T> inline T CMemArray<T>::Get(uint32 i)
	{
		ASSERT(m_pData);
		ASSERT(i < m_nElements);

		return m_pData[i];
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CMemArray::Remove()
	//
	// PURPOSE	: Removes an entry from the array
	//
	//----------------------------------------------------------

	template <class T> inline void CMemArray<T>::Remove(uint32 i)
	{
		ASSERT(m_pData);
		ASSERT(m_nElements > 0);
		ASSERT(i < m_nElements);

		// Remove the array element

		T *pPtr = m_pData + i;

		if (i < m_nElements)
		{
			// Copy all elements down by one

			for (uint32 j = i; j < m_nElements - 1; j ++)
			{
				m_pData[j] = m_pData[j + 1];
			}
		}

		m_nElements --;

		if (m_nElements == 0) RemoveAll();
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CMemArray::Remove()
	//
	// PURPOSE	: Removes an entry from the array
	//
	//----------------------------------------------------------

	template <class T> inline void CMemArray<T>::Remove(T pPtr)
	{
		ASSERT(m_pData);
		ASSERT(m_nElements > 0);

		// Remove the array element

		uint32 i;

		for (i = 0; i < m_nElements; i ++)
		{
			if (m_pData[i] = pPtr) break;
		}

		if (i < m_nElements)
		{
			// Copy all elements down by one

			for (uint32 j = i; j < m_nElements - 1; j ++)
			{
				m_pData[j] = m_pData[j + 1];
			}
		}

		m_nElements --;

		if (m_nElements == 0) RemoveAll();
	}

	//----------------------------------------------------------
	//
	// FUNCTION : CMemArray::RemoveAll()
	//
	// PURPOSE	: Removes all elements from array
	//
	//----------------------------------------------------------

	template <class T> inline void CMemArray<T>::RemoveAll()
	{
		Term();
	}

	//----------------------------------------------------------
	//
	// FUNCTION : 
	//
	// PURPOSE	: 
	//
	//----------------------------------------------------------

	//----------------------------------------------------------
	//
	// FUNCTION : 
	//
	// PURPOSE	: 
	//
	//----------------------------------------------------------

	//----------------------------------------------------------
	//
	// FUNCTION : 
	//
	// PURPOSE	: 
	//
	//----------------------------------------------------------

#endif