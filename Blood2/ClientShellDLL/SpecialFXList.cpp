// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialFXList.cpp
//
// PURPOSE : List of CSpecialFX class objects
//
// CREATED : 02/11/99
//
// ----------------------------------------------------------------------- //

#include "SpecialFXList.h"

CSpecialFX* CSpecialFXList::Add(CSpecialFX* pFX, CSpecialFX* pParentFX)
{
	DBOOL bFoundSlot = DFALSE;
	CSpecialFX* pOldFX = DNULL;
	int nOldestSlot = 0;
	DDWORD dwOldestYet = 0;

	if (!m_pArray || !m_pAgeArray) return DNULL;

	for (unsigned int i=0; i < m_nArraySize; i++)
	{
		if (!m_pArray[i] && !bFoundSlot)
		{
			m_pArray[i]	= pFX;
			m_pAgeArray[i] = 0;
			bFoundSlot = DTRUE;
		}
		else if (m_pArray[i])
		{
			m_pAgeArray[i]++;  // Age all elements in array;
		}

		// Keep track of the oldest..
		if (m_pAgeArray[i] > dwOldestYet)
		{
			nOldestSlot = i;
			dwOldestYet = m_pAgeArray[i];
		}
	}


	// See if array is full...If so, find the oldest element and 
	// remove it...

	if (!bFoundSlot)
	{
		// Replace the element at nSlot with the new fx 

		pOldFX = m_pArray[nOldestSlot];


		// Make sure we don't replace our parent..

		if (pParentFX && pOldFX == pParentFX)
		{
			// Find a slot that's not our parent...

			DDWORD dwOldestYet = 0;

			nOldestSlot = -1;

			for (i=0; i < m_nArraySize; i++)
			{
				if (m_pArray[i] != pParentFX)
				{
					if (m_pAgeArray[i] >= dwOldestYet)
					{
						nOldestSlot = i;
						dwOldestYet = m_pAgeArray[i];
						pOldFX = m_pArray[nOldestSlot];
					}
				}
			}

			if (nOldestSlot == -1) return(DNULL);
		}


		// Replace the oldest slot with this new fx...

		m_pArray[nOldestSlot] = pFX;
		m_pAgeArray[nOldestSlot] = 0;
	}
	else
		m_nElements++;

	return pOldFX;
}
