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

#include "dlink.h"
#include "SpecialFX.h"

#define  DEFAULT_MAX_NUM	50
#define	 MAX_NUM_LINKS	  	500	

class CSpecialFXList
{
	public :

		int GetSize()		const { return m_nArraySize; }
		int GetNumItems()	const { return m_nElements; }
		DBOOL IsEmpty()		const { return (DBOOL)(m_nElements == 0); }

		CSpecialFXList()
		{
			m_nArraySize = DNULL;
			m_pArray	 = DNULL;
			m_pAgeArray  = DNULL;
			m_nElements  = 0;
		}

		DBOOL Create(unsigned int nMaxNum=DEFAULT_MAX_NUM)
		{
			if (m_pArray) return DFALSE;  // Already created 

			m_nArraySize = nMaxNum < MAX_NUM_LINKS ? nMaxNum : MAX_NUM_LINKS;
			m_pArray = new CSpecialFX* [m_nArraySize];
			if (!m_pArray) return DFALSE;

			m_pAgeArray = new DDWORD[m_nArraySize];
			if (!m_pAgeArray) return DFALSE;

			m_nElements = 0;
			
			memset(m_pArray, 0, sizeof(CSpecialFX*)*m_nArraySize);
			memset(m_pAgeArray, 0, sizeof(DDWORD)*m_nArraySize);

			return DTRUE;
		}

		CSpecialFX* CSpecialFXList::operator[] (unsigned int nIndex)
		{
			if (!m_pArray || nIndex < 0 || nIndex >= m_nArraySize) return DNULL;
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

		CSpecialFX* Add(CSpecialFX* pFX, CSpecialFX* pParentFX = DNULL);

		DBOOL Remove(CSpecialFX* pFX)
		{
			if (!pFX || !m_pArray) return DFALSE;

			for (unsigned int i=0; i < m_nArraySize; i++)
			{
				if (m_pArray[i] == pFX)
				{
					delete m_pArray[i];
					m_pArray[i]	= DNULL;
					m_pAgeArray[i] = 0;
					m_nElements--;
					return DTRUE;
				}
			}

			return DFALSE;
		}	
	
	private :

		CSpecialFX**	m_pArray;		// Array of special fx
		DDWORD*			m_pAgeArray;	// Age special fx in array
		unsigned int	m_nArraySize;	// Size of array
		unsigned int	m_nElements;	// Number of elements in array
};

#endif // __SPECIAL_FX_LIST_H__