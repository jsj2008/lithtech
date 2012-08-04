// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFilterDB.cpp
//
// PURPOSE : Implementation of the soundfilter database
//
// CREATED : 3/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "SoundFilterDB.h"

namespace
{
	//sound filter category
	const char* const SFDB_SoundFilterCat				= "SoundFilters/Filters";
	const char* const SFDB_SoundFilterName				= "FilterName";

	// 'Special' sound filter record names.
	const char* const SFDB_sSoundFilterRecordName_Dynamic		= "Dynamic";
	const char* const SFDB_sSoundFilterRecordName_Unfiltered	= "UnFiltered";

	enum ENUM_SoundFilterParameter
	{
		kSoundFilterParameter_InvalidType = -1,

		#define SOUNDFILTERPARAMETER_TYPE_AS_ENUM 1
		#include "SoundFilterParameterEnum.h"
		#undef SOUNDFILTERPARAMETER_TYPE_AS_ENUM

		kSoundFilterParameter_Count
	};

	const char* const GetSoundFilterParameterName(ENUM_SoundFilterParameter eParamId)
	{
		static const char* s_aszSoundFilterParameterName[] =
		{
			#define SOUNDFILTERPARAMETER_TYPE_AS_STRING 1
			#include "SoundFilterParameterEnum.h"
			#undef SOUNDFILTERPARAMETER_TYPE_AS_STRING
		};

		if (eParamId < 0 || eParamId > kSoundFilterParameter_Count)
		{
			return "";
		}

		return s_aszSoundFilterParameterName[eParamId];
	}

};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::SoundFilterDB
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SoundFilterDB::SoundFilterDB()
:	CGameDatabaseMgr( ),
	m_hSoundFilterCategory(NULL)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::~SoundFilterDB
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SoundFilterDB::~SoundFilterDB()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::Init
//
//	PURPOSE:	Handles initializing the sound filter database
//
// ----------------------------------------------------------------------- //

bool SoundFilterDB::Init( const char *szDatabaseFile )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Get handles to all of the categories in the database...
	m_hSoundFilterCategory = GetCategory(m_hDatabase, SFDB_SoundFilterCat);

	m_hUnfilteredFilterRecord = CGameDatabaseReader::GetRecord(m_hSoundFilterCategory, SFDB_sSoundFilterRecordName_Unfiltered);
	m_hDynamicFilterRecord = CGameDatabaseReader::GetRecord(m_hSoundFilterCategory, SFDB_sSoundFilterRecordName_Dynamic);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Handles shutting down the sound filter database
//
// ----------------------------------------------------------------------- //

void SoundFilterDB::Term()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetFilterRecordCount()
//
//	PURPOSE:	Returns the number of filter parameters
//
// ----------------------------------------------------------------------- //

int SoundFilterDB::GetFilterRecordCount() const
{
	return g_pLTDatabase->GetNumRecords(m_hSoundFilterCategory);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetFilterRecord()
//
//	PURPOSE:	Returns the record at the passed in index.  If there is no 
//				such record, asserts and returns NULL.
//
// ----------------------------------------------------------------------- //

HRECORD SoundFilterDB::GetFilterRecord(int iIndex) const
{
	return g_pLTDatabase->GetRecordByIndex(m_hSoundFilterCategory, iIndex);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetUnfilteredFilterRecord()
//
//	PURPOSE:	Returns the special unfiltered filter record
//
// ----------------------------------------------------------------------- //

HRECORD SoundFilterDB::GetUnfilteredFilterRecord() const
{
	return m_hUnfilteredFilterRecord;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetDynamicFilterRecord()
//
//	PURPOSE:	Returns the special dynamic filter record
//
// ----------------------------------------------------------------------- //

HRECORD SoundFilterDB::GetDynamicFilterRecord() const
{
	return m_hDynamicFilterRecord;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetFilter()
//
//	PURPOSE:	Returns the record refering to the filter with the passed in name.
//
// ----------------------------------------------------------------------- //

HRECORD SoundFilterDB::GetFilterRecord(const char* const pszFilterName) const
{
	return CGameDatabaseReader::GetRecord(m_hSoundFilterCategory, pszFilterName);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetFilterName()
//
//	PURPOSE:	Returns the name of the filter defined by the passed in sound 
//				filter.  If the filter is invalid, an empty string is 
//				returned.
//
// ----------------------------------------------------------------------- //

const char*	SoundFilterDB::GetFilterName(HRECORD hSoundFilterRecord) const
{
	return GetString(hSoundFilterRecord, SFDB_SoundFilterName);	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetFilterRecordName()
//
//	PURPOSE:	Returns the name of the filter defined by the passed in sound 
//				filter.  If the filter is invalid, an empty string is 
//				returned.
//
// ----------------------------------------------------------------------- //

const char*	SoundFilterDB::GetFilterRecordName(HRECORD hSoundFilterRecord) const
{
	return GetRecordName(hSoundFilterRecord);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetFilterParmeterCount()
//
//	PURPOSE:	Returns the number of filter parameters
//
// ----------------------------------------------------------------------- //

int SoundFilterDB::GetFilterParmeterCount() const
{
	return kSoundFilterParameter_Count;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetFilterParmeterName()
//
//	PURPOSE:	Returns the name of the sound filter parameter at the 
//				requested index.  Assets and returns an empty string if 
//				the index is out of range.
//
// ----------------------------------------------------------------------- //

const char* SoundFilterDB::GetFilterParmeterName(HRECORD hSoundFilterRecord, int iFilterIndex) const
{
	return GetSoundFilterParameterName((ENUM_SoundFilterParameter)iFilterIndex);	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::GetFilterParmeterValue()
//
//	PURPOSE:	Returns the value of the parameter at the passed in index.  
//				Asserts and returns 0 if the index is out of range.
//
// ----------------------------------------------------------------------- //

float SoundFilterDB::GetFilterParmeterValue(HRECORD hSoundFilterRecord, int iFilterIndex) const
{
	return GetFloat(hSoundFilterRecord, GetSoundFilterParameterName((ENUM_SoundFilterParameter)iFilterIndex));	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::IsFilterDynamic()
//
//	PURPOSE:	Returns true if the passed in filter is dynamic.  Asserts 
//				and returns false if the passed in filter is NULL.
//
// ----------------------------------------------------------------------- //

bool SoundFilterDB::IsFilterDynamic(HRECORD hSoundFilterRecord) const
{
	if (NULL == hSoundFilterRecord)
	{
		LTASSERT(0, "SoundFilterDB::IsFilterDynamic : Passed in SoundFilterRecord is NULL." );
		return false;
	}

	return LTStrIEquals(SFDB_sSoundFilterRecordName_Dynamic, GetFilterRecordName(hSoundFilterRecord));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDB::IsFilterUnFiltered()
//
//	PURPOSE:	Returns true if the passed in filter is unfiltered. Asserts 
//				and returns false if the passed in filter is NULL.
//
// ----------------------------------------------------------------------- //

bool SoundFilterDB::IsFilterUnFiltered(HRECORD hSoundFilterRecord) const
{
	if (NULL == hSoundFilterRecord)
	{
		LTASSERT(0, "SoundFilterDB::IsFilterUnFiltered : Passed in SoundFilterRecord is NULL." );
		return false;
	}

	return LTStrIEquals(SFDB_sSoundFilterRecordName_Unfiltered, GetFilterRecordName(hSoundFilterRecord));
}


#if !defined( _CLIENTBUILD )
////////////////////////////////////////////////////////////////////////////
//
// SoundFilterDBPlugin is used to help facilitate populating the WorldEdit
// object properties that use SoundFilterDB
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundFilterDBPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool SoundFilterDBPlugin::PopulateStringList(char** aszStrings, 
											 uint32* pcStrings,
											 const uint32 cMaxStrings, 
											 const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings)
	{
		LTASSERT(0, "SoundFilterDBPlugin::PopulateStringList : NULL pointer to output array");	
		return false;
	}

	// Add an entry for each SOUNDFILTER type

	int nNumFilters = SoundFilterDB::Instance().GetFilterRecordCount();
	if (nNumFilters <= 0)
	{
		LTASSERT(0, "SoundFilterDBPlugin::PopulateStringList : No filters.");	
		return false;
	}

	for (int i = 0; i < nNumFilters; i++)
	{
		LTASSERT(cMaxStrings > (*pcStrings) + 1, "SoundFilterDBPlugin::PopulateStringList : Filter Count exceeded max.");

		HRECORD hRecord = SoundFilterDB::Instance().GetFilterRecord(i);
		const char* const pszFilterName = SoundFilterDB::Instance().GetFilterRecordName(hRecord);

		if (!pszFilterName)
		{
			LTASSERT(0, "SoundFilterDBPlugin::PopulateStringList : Failed to find filter name.");
		}

        uint32 dwFilterNameLen = LTStrLen(pszFilterName);

		if (dwFilterNameLen < cMaxStringLength
			&& ((*pcStrings) + 1) < cMaxStrings)
		{
			LTStrCpy(aszStrings[(*pcStrings)++], pszFilterName, cMaxStringLength);
		}
	}

    return true;
}

#endif // #ifndef _CLIENTBUILD
