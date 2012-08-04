// ----------------------------------------------------------------------- //
//
// MODULE  : ButeListReader.cpp
//
// PURPOSE : ButeListReader - Implementation.  Used to read 
//			 lists from bute.txt files into various buteMgrs.
//
// CREATED : 5/30/2001
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ButeListReader.h"
#include "ButeMgr.h"

char CButeListReader::ms_aAttName[100];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButeListReader::CButeListReader
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CButeListReader::CButeListReader()
{
	m_cItems	= 0;
	m_szItems	= '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButeListReader::~CButeListReader
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CButeListReader::~CButeListReader()
{
	for(uint8 iItem = 0; iItem < m_cItems; ++iItem)
	{
		debug_deletea(m_szItems[iItem]);
	}
	debug_deletea(m_szItems);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButeListReader::Read
//
//	PURPOSE:	Read list items from buteMgr.
//
// ----------------------------------------------------------------------- //

void CButeListReader::Read(CButeMgr* pButeMgr, const char* pszTagName, const char* pszAttName, 
						   const uint32 nStrLen)
{
	ASSERT(pButeMgr && pszTagName && pszAttName && nStrLen);

	// Count the items.
	uint8 cTemp = 0;
	sprintf(ms_aAttName, "%s%d", pszAttName, cTemp);
	while (pButeMgr->Exist(pszTagName, ms_aAttName))
	{
		++cTemp;
		sprintf(ms_aAttName, "%s%d", pszAttName, cTemp);
	}

	// Read in the items.
	char szTemp[256];
	if(cTemp > 0)
	{
		ASSERT(cTemp < 256);

		// free the list if there is one
		if ( m_szItems )
		{
			for(uint8 iItem = 0; iItem < m_cItems; ++iItem)
			{
				// delete the element
				debug_deletea(m_szItems[iItem]);
				m_szItems[iItem] = LTNULL;
			}

			// delete the array
			debug_deletea(m_szItems);
			m_szItems = LTNULL;

			// reset the array size
			m_cItems = 0;
		}

		m_cItems = cTemp;
		m_szItems = debug_newa(char*, cTemp);

		for(uint8 iItem=0; iItem < m_cItems; ++iItem)
		{
			sprintf(ms_aAttName, "%s%d", pszAttName, iItem);
			pButeMgr->GetString(pszTagName, ms_aAttName, szTemp, sizeof(szTemp));
			ASSERT(strlen(szTemp) < nStrLen);
			m_szItems[iItem] = debug_newa(char, strlen(szTemp) + 1);
			strcpy(m_szItems[iItem], szTemp);
		}
	}	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButeListReader::CopyList
//
//	PURPOSE:	Copy list items into array.
//
// ----------------------------------------------------------------------- //

void CButeListReader::CopyList(uint8 iStart, char* paszDest, uint32 nStrLen) const
{
	for(uint8 iItem = iStart; iItem < m_cItems; ++iItem)
	{
		ASSERT(strlen(m_szItems[iItem]) < nStrLen);
		SAFE_STRCPY(paszDest + (iItem * nStrLen), m_szItems[iItem]);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButeListReader::ReadAndCopy
//
//	PURPOSE:	Read and Copy list items into array
//
// ----------------------------------------------------------------------- //

void CButeListReader::ReadAndCopy(CButeMgr* pButeMgr, const char* pszTagName, const char* pszAttName, 
									 char* paszDest, uint32 nStrLen)
{
	// This function is called instead of the other Read() and Copy(), 
	// when we do not want to store any values, or allocate any memory.
	ASSERT(pButeMgr && pszTagName && pszAttName);
	ASSERT(m_cItems == 0);

	// Copy the items.
	char szTemp[256];
	uint8 iItem = 0;
	sprintf(ms_aAttName, "%s%d", pszAttName, iItem);
	while (pButeMgr->Exist(pszTagName, ms_aAttName))
	{
		
		pButeMgr->GetString(pszTagName, ms_aAttName,szTemp,sizeof(szTemp));
		ASSERT(strlen(szTemp) < nStrLen);
		SAFE_STRCPY(paszDest + (iItem * nStrLen), szTemp);
		++iItem;
		sprintf(ms_aAttName, "%s%d", pszAttName, iItem);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CButeListReader::SetItem
//
//	PURPOSE:	Set value of an item.  If no items, allocate space and set one item.
//
// ----------------------------------------------------------------------- //

void CButeListReader::SetItem(uint8 iItem, const char* szItem, const uint32 nStrLen)
{
	ASSERT(szItem != LTNULL);

	if(m_cItems == 0)
	{
		ASSERT(iItem == 0);
		ASSERT( 0 == m_szItems );
		m_cItems = 1;
		m_szItems = debug_newa(char*, 1);
		m_szItems[0] = debug_newa(char, strlen(szItem) + 1);
	}

	ASSERT(iItem < m_cItems);
	ASSERT(strlen(szItem) < nStrLen);
	strcpy(m_szItems[iItem], szItem);
}
