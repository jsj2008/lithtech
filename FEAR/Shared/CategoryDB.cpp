// ----------------------------------------------------------------------- //
//
// MODULE  : CategoryDB.cpp
//
// PURPOSE : Helper class to define interface to a DB Category
//
// CREATED : 5/1/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "CategoryDB.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CategoryDB::Init()
//
//	PURPOSE:	Handle initializing the category.  Returns 
//				false if the initialization fails.
//
// ----------------------------------------------------------------------- //

bool CategoryDB::Init( char const* pszCategory, char const* pszDatabaseFile )
{
	if( !OpenDatabase( pszDatabaseFile ))
		return false;

	// Get handles to all of the category in the database.
	m_hCategory = g_pLTDatabase->GetCategory(m_hDatabase, pszCategory);
	if( !m_hCategory )
		return false;
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CategoryDB::Term()
//
//	PURPOSE:	Clears out object.
//
// ----------------------------------------------------------------------- //

void CategoryDB::Term( )
{
	m_hCategory = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CategoryPlugin::PopulateStringList
//
//	PURPOSE:	Fills in string list with all the records in a category.
//				Preceeds the list with "<none>".
//
// ----------------------------------------------------------------------- //
bool CategoryPlugin::PopulateStringList( HCATEGORY hCategory, char** aszStrings, uint32* pcStrings,
										const uint32 cMaxStrings, const uint32 cMaxStringLength )
{
	// Check inputs.
	if( !aszStrings || !pcStrings || !hCategory )
	{
		LTERROR( "Invalid parameter supplied" );
		return false;
	}

	// Add the "<none>" record.
	static uint32 nLen = LTStrLen( StringList_None );
	if( nLen < cMaxStringLength && (( *pcStrings ) + 1 ) < cMaxStrings )
	{
		LTStrCpy( aszStrings[(*pcStrings)++], StringList_None, cMaxStringLength );
	}

	// Iterate through each record and add it to the list.
	uint32 nNumRecords = g_pLTDatabase->GetNumRecords( hCategory );
	for( uint32 nIndex = 0; nIndex < nNumRecords && *pcStrings < cMaxStrings; nIndex++ )
	{
		HRECORD hRecord = g_pLTDatabase->GetRecordByIndex( hCategory, nIndex );
		if( !hRecord )
			continue;

		const char* pszName = g_pLTDatabase->GetRecordName( hRecord );
		if( !pszName )
			continue;

		// Skip records with names that are too long.
		uint32 nStrLen = LTStrLen( pszName );
		if( nStrLen >= cMaxStringLength )
		{
			LTERROR( "Record name too long." );
			continue;
		}

		// Add the string to the list.
		LTStrCpy( aszStrings[(*pcStrings)++], pszName, cMaxStringLength );
	}

	return true;
}

