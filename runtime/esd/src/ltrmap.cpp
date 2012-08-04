/****************************************************************************
;
;	MODULE:		LTRMap (.CPP)
;
;	PURPOSE:	Support class for RealVideo
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrmap.h"
#include "ltrconout.h"

//-----------------------------------------------------------------------------
// LTRMap member functions
//-----------------------------------------------------------------------------
void* LTRMap::GetFirstValue()
{
    m_nCursor = 0;

    if (m_nMapSize)
    {
       return m_pValueArray[m_nCursor];
    }
    else
    {
       return NULL;
    }
}

//-----------------------------------------------------------------------------
void* LTRMap::GetNextValue()
{
    m_nCursor++;

    if (m_nCursor < m_nMapSize)
    {
       return m_pValueArray[m_nCursor];
    }
    else
    {
       return NULL;
    }
}

//-----------------------------------------------------------------------------
BOOL LTRMap::Lookup(void* Key, void*& Value) const
{
    BOOL bFound = FALSE;
    int nIndex = 0;

    // If Key is alrady in the list, replace value
    for (; nIndex < m_nMapSize; nIndex++)
    {
	if (m_pKeyArray[nIndex] == Key)
	{
	    Value = m_pValueArray[nIndex];
	    bFound = TRUE;
	    goto exit;
	}
    }

exit:
    return bFound;    
}

//-----------------------------------------------------------------------------
void LTRMap::RemoveKey(void* Key)
{
    BOOL bFound = FALSE;
    int nIndex = 0;

    // If Key is alrady in the list, replace value
    for (; nIndex < m_nMapSize; nIndex++)
    {
	if (m_pKeyArray[nIndex] == Key)
	{
	    if (nIndex < (m_nMapSize-1))
	    {
		memmove(&(m_pKeyArray[nIndex]),&(m_pKeyArray[nIndex+1]),sizeof(void*)*(m_nMapSize-(nIndex+1)));
		memmove(&(m_pValueArray[nIndex]),&(m_pValueArray[nIndex+1]),sizeof(void*)*(m_nMapSize-(nIndex+1)));
	    }
	    m_nMapSize--;
	    goto exit;
	}
    }

exit:
    (NULL); // We're done!
}

//-----------------------------------------------------------------------------
void LTRMap::RemoveValue(void* Value)
{
    BOOL bFound = FALSE;
    int nIndex = 0;

    // If Value is alrady in the list, replace value
    for (; nIndex < m_nMapSize; nIndex++)
    {
	if (m_pValueArray[nIndex] == Value)
	{
	    if (nIndex < (m_nMapSize-1))
	    {
		memmove(&(m_pKeyArray[nIndex]),&(m_pKeyArray[nIndex+1]),sizeof(void*)*(m_nMapSize-(nIndex+1)));
		memmove(&(m_pValueArray[nIndex]),&(m_pValueArray[nIndex+1]),sizeof(void*)*(m_nMapSize-(nIndex+1)));
	    }
	    m_nMapSize--;
	    goto exit;
	}
    }

exit:
    (NULL); // We're done!
}

//-----------------------------------------------------------------------------
void LTRMap::SetAt(void* Key, void* Value)
{
    int nIndex = 0;

    // If Key is alrady in the list, replace value
    for (; nIndex < m_nMapSize; nIndex++)
    {
	if (m_pKeyArray[nIndex] == Key)
	{
	    m_pValueArray[nIndex] = Value;
	    goto exit;
	}
    }

    // If we have room, add it to the end!
    if (m_nAllocSize == m_nMapSize)
    {
	m_nAllocSize += AllocationSize;
	void** pNewKeys;
	LT_MEM_TRACK_ALLOC(pNewKeys = new void*[m_nAllocSize],LT_MEM_TYPE_MISC);
	void** pNewValues;
	LT_MEM_TRACK_ALLOC(pNewValues = new void*[m_nAllocSize],LT_MEM_TYPE_MISC);

	memcpy(pNewKeys,m_pKeyArray,sizeof(void*)*m_nMapSize);
	memcpy(pNewValues,m_pValueArray,sizeof(void*)*m_nMapSize);

	delete [] m_pKeyArray;
	delete [] m_pValueArray;

	m_pKeyArray = pNewKeys;
	m_pValueArray = pNewValues;
    }

    m_pKeyArray[m_nMapSize] = Key;
    m_pValueArray[m_nMapSize] = Value;
    m_nMapSize++;

exit:
    (NULL); // We're done!
}
#endif // LITHTECH_ESD