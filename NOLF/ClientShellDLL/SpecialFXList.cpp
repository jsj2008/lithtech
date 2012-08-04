//////////////////////////////////////////////////////////////////////////////
// Implementation of the CSpecialFXList class

#include "stdafx.h"
#include "SpecialFXList.h"

#include "SFXMgr.h"

CSpecialFXList::~CSpecialFXList()
{
	if (m_pArray)
	{

		for (unsigned int i=0; i < m_nArraySize; i++)
		{
			if (m_pArray[i])
			{
				CSFXMgr::DeleteSFX(m_pArray[i]);
			}
		}

		debug_deletea(m_pArray);
		m_pArray = LTNULL;
	}

	if (m_pAgeArray)
	{
		debug_deletea(m_pAgeArray);
		m_pAgeArray = LTNULL;
	}
}

LTBOOL CSpecialFXList::Add(CSpecialFX* pFX)
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

		CSFXMgr::DeleteSFX(m_pArray[nSlot]);
		m_pArray[nSlot]	= pFX;
		m_pAgeArray[nSlot] = 0;
	}
	else
	{
		m_nElements++;
	}

    return LTTRUE;
}

LTBOOL CSpecialFXList::Remove(CSpecialFX* pFX)
{
    if (!pFX || !m_pArray) return LTFALSE;

	for (unsigned int i=0; i < m_nArraySize; i++)
	{
		if (m_pArray[i] == pFX)
		{
			CSFXMgr::DeleteSFX(m_pArray[i]);
            m_pArray[i] = LTNULL;
			m_pAgeArray[i] = 0;
			m_nElements--;
            return LTTRUE;
		}
	}

    return LTFALSE;
}
