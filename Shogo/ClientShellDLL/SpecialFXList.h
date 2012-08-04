// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialFXList.h
//
// PURPOSE : List of CSpecialFX class objects
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __SPECIAL_FX_LIST_H__
#define __SPECIAL_FX_LIST_H__

#include "ltlink.h"
#include "SpecialFX.h"

#define  DEFAULT_MAX_NUM	50
#define	 MAX_NUM_LINKS	  	500	

class CSpecialFXList
{
	public :

		int GetSize()		const { return m_nArraySize; }
		int GetNumItems()	const { return m_nElements; }
		LTBOOL IsEmpty()		const { return (LTBOOL)(m_nElements == 0); }

		CSpecialFXList()
		{
			m_nArraySize = LTNULL;
			m_pArray	 = LTNULL;
			m_pAgeArray  = LTNULL;
			m_nElements  = 0;
		}

		LTBOOL Create(unsigned int nMaxNum=DEFAULT_MAX_NUM)
		{
			if (m_pArray) return LTFALSE;  // Already created 

			m_nArraySize = nMaxNum < MAX_NUM_LINKS ? nMaxNum : MAX_NUM_LINKS;
			m_pArray = new CSpecialFX* [m_nArraySize];
			if (!m_pArray) return LTFALSE;

			m_pAgeArray = new uint32[m_nArraySize];
			if (!m_pAgeArray) return LTFALSE;

			m_nElements = 0;
			
			memset(m_pArray, 0, sizeof(CSpecialFX*)*m_nArraySize);
			memset(m_pAgeArray, 0, sizeof(uint32)*m_nArraySize);

			return LTTRUE;
		}

		CSpecialFX* CSpecialFXList::operator[] (unsigned int nIndex)
		{
			if (!m_pArray || nIndex < 0 || nIndex >= m_nArraySize) return LTNULL;
			return m_pArray[nIndex];
		}

		~CSpecialFXList()
		{
			if (m_pArray)
			{

				for (unsigned int i=0; i < m_nArraySize; i++)
				{
					if (m_pArray[i])
					{
						delete m_pArray[i];
					}
				}

				delete [] m_pArray;
			}

			if (m_pAgeArray)
			{
				delete [] m_pAgeArray;
			}
		}

		LTBOOL Add(CSpecialFX* pFX)
		{
			LTBOOL bFoundSlot = LTFALSE;

			if (!m_pArray || !m_pAgeArray) return LTFALSE;

			for (unsigned int i=0; i < m_nArraySize; i++)
			{
				if (!m_pArray[i] && !bFoundSlot)
				{
					m_pArray[i]	= pFX;
					m_pAgeArray[i] = 0;
					bFoundSlot = LTTRUE;
				}
				else if (m_pArray[i])
				{
					m_pAgeArray[i]++;  // Age all elements in array;
				}
			}


			// See if array is full...If so, find the oldest element and 
			// remove it...

			if (!bFoundSlot)
			{
				int nSlot = 0;
				uint32 dwOldestYet = 0;
				for (unsigned int i=0; i < m_nArraySize; i++)
				{
					if (m_pAgeArray[i] > dwOldestYet)
					{
						nSlot = i;
						dwOldestYet = m_pAgeArray[i];
					}
				}

				// Replace the element at nSlot with the new fx 
		
				delete m_pArray[nSlot];
				m_pArray[nSlot]	= pFX;
				m_pAgeArray[nSlot] = 0;
			}
			else
			{
				m_nElements++;
			}

			return LTTRUE;
		}

		LTBOOL Remove(CSpecialFX* pFX)
		{
			if (!pFX || !m_pArray) return LTFALSE;

			for (unsigned int i=0; i < m_nArraySize; i++)
			{
				if (m_pArray[i] == pFX)
				{
					delete m_pArray[i];
					m_pArray[i]	= LTNULL;
					m_pAgeArray[i] = 0;
					m_nElements--;
					return LTTRUE;
				}
			}

			return LTFALSE;
		}	
	
	private :

		CSpecialFX**	m_pArray;		// Array of special fx
		uint32*			m_pAgeArray;	// Age special fx in array
		unsigned int	m_nArraySize;	// Size of array
		unsigned int	m_nElements;	// Number of elements in array
};

#endif // __SPECIAL_FX_LIST_H__