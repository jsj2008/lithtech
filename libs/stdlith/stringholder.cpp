//------------------------------------------------------------------
//
//	FILE	  : StringHolder.cpp
//
//	PURPOSE	  : Implementation for the CStringHolder class.
//
//	CREATED	  : 5/1/96
//
//	COPYRIGHT : Monolith 1996 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....
#include <string.h>
#include "stringholder.h"


#define DEFAULT_ALLOC_SIZE	150


CStringHolder::CStringHolder()
{
	m_AllocSize = DEFAULT_ALLOC_SIZE;
}


CStringHolder::CStringHolder( uint16 allocSize )
{
	m_AllocSize = allocSize;
}


CStringHolder::~CStringHolder()
{
	ClearStrings();
}


void CStringHolder::SetAllocSize( uint16 size )
{
	ASSERT( m_Strings.GetSize() == 0 );
	m_AllocSize = size;
}


char *CStringHolder::AddString( const char *pString, LTBOOL bFindFirst )
{
	uint16	len = strlen(pString);
	return AddString( pString, bFindFirst, len );
}


char *CStringHolder::AddString( const char *pString, LTBOOL bFindFirst, uint32 len )
{
	uint32 i;
	uint16 allocSize;
	LTBOOL bFoundOne;
	char *pRetVal;
	SBank *pBank, theString;


	ASSERT( len <= m_AllocSize );

	// See if we can find it in an array first.
	if( bFindFirst )
	{
		pRetVal = FindString( pString );
		if( pRetVal )
			return pRetVal;
	}

	// See if it'll fit in any of the arrays.
	bFoundOne = FALSE;
	for( i=0; i < m_Strings; i++ )
	{
		pBank = &m_Strings[i];

		if((len+pBank->m_StringSize+1) < pBank->m_AllocSize)
		{
			bFoundOne = TRUE;
			break;
		}
	}

	if( !bFoundOne )
	{
		allocSize = (uint16)(len+1);
		if(m_AllocSize > allocSize)
			allocSize = m_AllocSize;
		
		theString.m_pString = new char[allocSize];
		if( !theString.m_pString )
			return NULL;
		
		theString.m_AllocSize = allocSize;
		theString.m_StringSize = 0;
		if( !m_Strings.Append(theString) )
		{
			theString.m_pString;
			return NULL;
		}
		
		i = (uint16)(m_Strings - 1);
	}

	pBank = &m_Strings[i];
	pRetVal = pBank->m_pString + pBank->m_StringSize;

	memcpy( pRetVal, pString, len );
	pRetVal[len] = 0;

	pBank->m_StringSize += (uint16)(len+1);
	ASSERT( pBank->m_StringSize <= pBank->m_AllocSize );

	return pRetVal;
}


char *CStringHolder::FindString( const char *pString )
{
	uint16 a, i, len;
	char *pBaseString;
	SBank *pBank;

	for( a=0; a < m_Strings; a++ )
	{
		pBank = &m_Strings[a];
		pBaseString = pBank->m_pString;
		
		i=0;
		while( i < pBank->m_StringSize )
		{
			len = strlen( &pBaseString[i] );
			if( strcmp(pString, &pBaseString[i]) == 0 )
				return &pBaseString[i];
	
			i += len + 1;
		}	
	}

	return NULL;
}



void CStringHolder::ClearStrings()
{
	uint32 i;

	for( i=0; i < m_Strings; i++ )
	{
		delete m_Strings[i].m_pString;
	}

	m_Strings.SetSize(0);
}


