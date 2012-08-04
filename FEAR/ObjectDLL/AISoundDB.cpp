// ----------------------------------------------------------------------- //
//
// MODULE  : AISoundDB.cpp
//
// PURPOSE : Implementation of AI Sound database
//
// CREATED : 03/19/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "AISoundDB.h"

//
// Defines...
//

const char* const AISDB_TemplateCat =	"Sound/AISoundTemplate";
const char* const AISDB_SetCat =		"Sound/AISoundSet";


//
// Globals...
//

CAISoundDB* g_pAISoundDB = NULL;



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundDB::CAISoundDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CAISoundDB::CAISoundDB()
	: CGameDatabaseMgr(),
	  m_hSoundTemplateCat(NULL)

{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundDB::~CAISoundDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CAISoundDB::~CAISoundDB()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundDB::Init()
//
//	PURPOSE:	Initialize the AI database...
//
// ----------------------------------------------------------------------- //

bool CAISoundDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ) )
	{
		return false;
	}

	// Set the global database pointer...

	g_pAISoundDB = this;

	m_hSoundTemplateCat = g_pLTDatabase->GetCategory(m_hDatabase,AISDB_TemplateCat);
	m_hSoundSetCat = g_pLTDatabase->GetCategory(m_hDatabase,AISDB_SetCat);

	if (!InitSoundSets())
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundDB::Term()
//
//	PURPOSE:	Clean up
//
// ----------------------------------------------------------------------- //
void CAISoundDB::Term()
{
	m_vecSoundSets.clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISoundDB::InitSoundSets()
//
//	PURPOSE:	Initialize the random sound selection tables
//
// ----------------------------------------------------------------------- //
bool CAISoundDB::InitSoundSets()
{
	uint32 numRecords = g_pLTDatabase->GetNumRecords(m_hSoundSetCat);
	m_vecSoundSets.resize(numRecords);
	if ( m_vecSoundSets.size () != numRecords )
	{
		LTERROR( "Failed to allocate SoundSets!" );
		return false;
	}

	for (uint32 nRecord = 0; nRecord < numRecords; nRecord++)
	{
		CSoundSet* pSet = &m_vecSoundSets[nRecord];
		HRECORD hSR = g_pLTDatabase->GetRecordByIndex(m_hSoundSetCat,nRecord);
		pSet->Init(hSR);
	}
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetRandomSoundFilename()
//
//	PURPOSE:	Get a random sound filename...
//
// ----------------------------------------------------------------------- //

void  CAISoundDB::GetRandomSoundFilename(HRECORD hSoundTemplate, const char* pAttributeBase, char *pBuf, uint16 nBufLen)
{
	if (!pBuf) return;
	if (!pAttributeBase)
	{
		pBuf[0] = (char)NULL;
		return;
	}

	LTASSERT(hSoundTemplate,"Invalid sound template specified.");
	if (!hSoundTemplate)
	{
		pBuf[0] = (char)NULL;
		return;
	}

	HRECORD hSR = GetRecordLink(hSoundTemplate,pAttributeBase);
	if (!hSR)
	{
		pBuf[0] = (char)NULL;
		return;
	}

	uint32 nIndex = g_pLTDatabase->GetRecordIndex(hSR);

	if ( nIndex >= m_vecSoundSets.size() || nIndex < 0 )
	{
		LTERROR( "Out of bounds soundset index." );
		pBuf[0] = (char)NULL;
		return;
	}

	LTStrCpy(pBuf,m_vecSoundSets[nIndex].GetRandomNotDirtyFile(),nBufLen);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetRandomSoundFilename()
//
//	PURPOSE:	Get a random sound filename in the passed in set
//
// ----------------------------------------------------------------------- //

void CAISoundDB::GetRandomSoundFilename( HRECORD hAISoundSet, char *pBuf, uint16 nBufLen )
{
	// Sound is not of the correct category.  This may be due to a difference
	// between the database version and code.

	if ( m_hSoundTemplateCat != g_pLTDatabase->GetRecordParent( hAISoundSet ) )
	{
		pBuf[0] = (char)NULL;
		return;
	}

	uint32 nIndex = g_pLTDatabase->GetRecordIndex(hAISoundSet);
	if ( INVALID_GAME_DATABASE_INDEX == nIndex )
	{
		pBuf[0] = (char)NULL;
		return;
	}

	if ( nIndex >= m_vecSoundSets.size() || nIndex < 0 )
	{
		LTASSERT(nIndex < m_vecSoundSets.size() && nIndex >= 0, "Out of bounds soundset index.");
		pBuf[0] = (char)NULL;
		return;
	}

	LTStrCpy(pBuf,m_vecSoundSets[nIndex].GetRandomNotDirtyFile(),nBufLen);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:    CGameSoundMgr::GetMixChannel()
//
//	PURPOSE:	Gets the character's mix channel
//
// ----------------------------------------------------------------------- //

int32	CAISoundDB::GetMixChannel(HRECORD hSoundTemplate)
{
	if (hSoundTemplate)
	{
		return g_pAISoundDB->GetInt32(hSoundTemplate,AISoundDB_nMixChannel);
	}
	else
	{
		return PLAYSOUND_MIX_SPEECH;
	}
}
