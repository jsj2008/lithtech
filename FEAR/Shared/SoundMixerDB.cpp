// ----------------------------------------------------------------------- //
//
// MODULE  : SoundMixerDB.cpp
//
// PURPOSE : Implementation of Sound Mixer database
//
// CREATED : 03/29/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "SoundMixerDB.h"

//
// Defines...
//

//FX categories
const char* const SoundMixerDB_SoundMixerCat =		"Sound/Mixers";


//
// Globals...
//

CSoundMixerDB* g_pSoundMixerDB = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMixerDB::CSoundMixerDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CSoundMixerDB::CSoundMixerDB()
:	CGameDatabaseMgr( ),
	m_hSoundMixerCat(NULL)
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMixerDB::~CSoundMixerDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSoundMixerDB::~CSoundMixerDB()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMixerDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CSoundMixerDB::Init( const char *szDatabaseFile /* = FDB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pSoundMixerDB = this;

	// Get handles to all of the categories in the database...
	m_hSoundMixerCat = g_pLTDatabase->GetCategory(m_hDatabase,SoundMixerDB_SoundMixerCat);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMixerDB::GetSoundMixer
//
//	PURPOSE:	Get sound mixer record
//
// ----------------------------------------------------------------------- //

HSOUNDMIXER	CSoundMixerDB::GetSoundMixer(uint32 nIndex)
{
	if (INVALID_GAME_DATABASE_INDEX == nIndex) return NULL;
	return g_pLTDatabase->GetRecordByIndex(m_hSoundMixerCat,nIndex);
}

HSOUNDMIXER	CSoundMixerDB::GetSoundMixer(const char* pszSoundMixer)
{
	return g_pLTDatabase->GetRecord(m_hSoundMixerCat,pszSoundMixer);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMixerDB::GetNumSoundMixerTypes()
//
//	PURPOSE:	Get number of sound mixer record
//
// ----------------------------------------------------------------------- //

uint32 CSoundMixerDB::GetNumSoundMixerTypes()
{
	return g_pLTDatabase->GetNumRecords(m_hSoundMixerCat);
}

#if defined( _SERVERBUILD )

/////////////////////////////////////////////////////////////////////////////
//
//	S E R V E R - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//
// CSoundMixerDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use SoundMixerDB
//
////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMixerDBPlugin::CSoundMixerDBPlugin()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

/*
CSoundMixerDBPlugin::CSoundMixerDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMixerDBPlugin::~CSoundMixerDBPlugin()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSoundMixerDBPlugin::~CSoundMixerDBPlugin()
{
}
*/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundMixerDBPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool CSoundMixerDBPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
										  const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return false;
	LTASSERT(aszStrings && pcStrings, "CSoundMixerDBPlugin::PopulateStringList");

	//add an entry for the 'None' option as well just to make it more user friendly
	if(*pcStrings < cMaxStrings)
	{
		LTStrCpy( aszStrings[(*pcStrings)++], "None", cMaxStringLength );
	}

	// Add an entry for each sound mixer type
	uint32 nNumSoundMixer = CSoundMixerDB::Instance().GetNumSoundMixerTypes();
	LTASSERT(nNumSoundMixer < INVALID_GAME_DATABASE_INDEX, "CSoundMixerDBPlugin::PopulateStringList");

	HSOUNDMIXER hSoundMixer = NULL;

	for (uint32 i=0; i < nNumSoundMixer; i++)
	{
		LTASSERT(cMaxStrings > (*pcStrings) + 1, "CSoundMixerDBPlugin::PopulateStringList");

		hSoundMixer = CSoundMixerDB::Instance().GetSoundMixer(i);
		if (!hSoundMixer)
			continue;
		
		const char* pszName = CSoundMixerDB::Instance().GetRecordName(hSoundMixer);
		if (!pszName)
			continue;

		uint32 dwImpactFXNameLen = LTStrLen(pszName);

		if (dwImpactFXNameLen &&
			dwImpactFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pszName);
		}
	}

	return true;
}

#endif // _SERVERBUILD
