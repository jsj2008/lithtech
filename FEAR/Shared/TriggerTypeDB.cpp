// ----------------------------------------------------------------------- //
//
// MODULE  : TriggerTypeDB.cpp
//
// PURPOSE : Defines TriggerTypeDB implementation
//
// CREATED : 3/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "TriggerTypeDB.h"

const char* const TTDB_TriggerTypeCat				= "TriggerTypes/Types";
const char* const TTDB_TriggerTypeIconName			= "IconName";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerTypeDB::TriggerTypeDB()
//
//	PURPOSE:	Contructor/Destructor to handle setup and cleanup
//
// ----------------------------------------------------------------------- //

TriggerTypeDB::TriggerTypeDB() 
{
}

TriggerTypeDB::~TriggerTypeDB() 
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerTypeDB::Init()
//
//	PURPOSE:	Initializes triggertype db.
//
// ----------------------------------------------------------------------- //

bool TriggerTypeDB::Init( const char *szDatabaseFile )
{
	return CategoryDB::Init( TTDB_TriggerTypeCat, szDatabaseFile );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerTypeDB::GetTriggerTypeRecordCount()
//
//	PURPOSE:	Returns the number of TriggerType records.
//
// ----------------------------------------------------------------------- //

int TriggerTypeDB::GetTriggerTypeRecordCount() const
{
	return g_pLTDatabase->GetNumRecords(GetCategory( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerTypeDB::GetTriggerTypeRecord()
//
//	PURPOSE:	Returns the record handle for the TriggerType record at 
//				this index.  Asserts and returns NULL if the index is out 
//				of range.
//
// ----------------------------------------------------------------------- //

HRECORD	TriggerTypeDB::GetTriggerTypeRecord(int nIndex) const
{
	return g_pLTDatabase->GetRecordByIndex(GetCategory( ), nIndex);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerTypeDB::GetTriggerRecord()
//
//	PURPOSE:	Returns a handle to the TriggerType record with the 
//				passed in name.  Asserts and returns NULL if no such 
//				record exists.  If the special record "<none>" is 
//				requested, NULL is returned without an assert as this is
//				a valid name for no record
//
// ----------------------------------------------------------------------- //

HRECORD	TriggerTypeDB::GetTriggerRecord(const char* const pszRecordName) const
{
	if ( 0 == LTStrICmp("<none>", pszRecordName) )
	{
		return NULL;
	}

	return CGameDatabaseReader::GetRecord(GetCategory( ), pszRecordName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerTypeDB::GetTriggerTypeRecordName()
//
//	PURPOSE:	Returns the name of the passed in record.  Asserts and 
//				returns an empty string ("") if any failure occurs.
//
// ----------------------------------------------------------------------- //

const char* const TriggerTypeDB::GetTriggerTypeRecordName(HRECORD hRecord) const
{
	return GetRecordName(hRecord);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerTypeDB::GetIconTexture()
//
//	PURPOSE:	Returns the name of the icon texture associated with the 
//				passed in record.  Asserts and returns an empty string 
//				("") if a failure occurs.
//
// ----------------------------------------------------------------------- //

const char* const TriggerTypeDB::GetIconTexture(HRECORD hRecord) const
{
	return GetString(hRecord, TTDB_TriggerTypeIconName);
}


////////////////////////////////////////////////////////////////////////////
//
// TriggerTypeDBPlugin is used to help facilitate populating the WorldEdit
// object properties that use TriggerTypeDB
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TriggerTypeDBPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool TriggerTypeDBPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings)
	{
		LTASSERT(0, "TriggerTypeDBPlugin::PopulateStringList : NULL pointer to output array");	
		return false;
	}

	// Add an entry for each type of trigger

	int nNumTriggerTypes = TriggerTypeDB::Instance().GetTriggerTypeRecordCount();
	if (nNumTriggerTypes <= 0)
	{
		LTASSERT(0, "TriggerTypeDBPlugin::PopulateStringList : No TriggerTypes.");	
		return false;
	}

	// Manually add <none> as the default (and valid) name.

	LTASSERT(cMaxStrings > (*pcStrings) + 1, "TriggerTypeDBPlugin::PopulateStringList : TriggerType Count exceeded max.");
	if (cMaxStrings > (*pcStrings) + 1)
	{
		LTStrCpy(aszStrings[(*pcStrings)++], "<none>", cMaxStringLength);
	}

	for (int i = 0; i < nNumTriggerTypes; i++)
	{
		LTASSERT(cMaxStrings > (*pcStrings) + 1, "TriggerTypeDBPlugin::PopulateStringList : TriggerType Count exceeded max.");

		HRECORD hRecord = TriggerTypeDB::Instance().GetTriggerTypeRecord(i);
		const char* const pszTriggerTypeName = TriggerTypeDB::Instance().GetTriggerTypeRecordName(hRecord);

		if (!pszTriggerTypeName)
		{
			LTASSERT(0, "TriggerTypeDBPlugin::PopulateStringList : Failed to find TriggerType name.");
		}

        uint32 dwTriggerTypeNameLen = LTStrLen(pszTriggerTypeName);

		if (dwTriggerTypeNameLen < cMaxStringLength
			&& ((*pcStrings) + 1) < cMaxStrings)
		{
			LTStrCpy(aszStrings[(*pcStrings)++], pszTriggerTypeName, cMaxStringLength);
		}
	}

    return true;
}
