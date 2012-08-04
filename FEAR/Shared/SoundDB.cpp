// ----------------------------------------------------------------------- //
//
// MODULE  : SoundDB.cpp
//
// PURPOSE : Implementation of Sound database
//
// CREATED : 02/23/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "SoundDB.h"

//sound category
const char* const SndDB_SoundCat =			"Sound/Sound";
const char* const SndDB_CharacterSoundCat =	"Sound/CharacterSounds";

//sound attributes
const char* const SndDB_sSoundFile =		"SoundFile";
const char* const SndDB_nWeight =			"Weight";
const char* const SndDB_bPlaySoundLocal =	"PlaySoundLocal";
const char* const SndDB_bPlayClientLocal =	"PlayClientLocal";
const char* const SndDB_fInnerRadius =		"InnerRadius";
const char* const SndDB_fPitch =			"Pitch";
const char* const SndDB_nVolume =			"Volume";
const char* const SndDB_bLoop =				"Loop";
const char* const SndDB_bReverb =			"Reverb";
const char* const SndDB_bPlayOnce =			"PlayOnce";
const char* const SndDB_fPlayChance =		"PlayChance";
const char* const SndDB_nPriority =			"Priority";
const char* const SndDB_fDopplerFactor =	"DopplerFactor";
const char* const SndDB_bLipSync =			"LipSync";
const char* const SndDB_fMinDelay =			"MinDelay";
const char* const SndDB_fMaxDelay =			"MaxDelay";
const char* const SndDB_nMixChannel =		"MixChannel";
const char* const SndDB_bWorldEditVisible =	"WorldEditVisible";
const char* const SndDB_sOcclusion =		"Occlusion";
const char* const SndDB_sAltSoundFile =		"AlternateSoundFile";
const char* const SndDB_fSwitchRadius =		"SoundSwitchRadius";
const char* const SndDB_nAltWeight =		"AlternateSoundWeight";
const char* const SndDB_sXActCue =			"XActCue";

//
// Globals...
//

CSoundDB* g_pSoundDB = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundSet::CSoundSet()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //
CSoundSet::CSoundSet()	:
	m_hRecord(NULL),
	m_nTotalWeight(0)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundSet::Init()
//
//	PURPOSE:	Initialize the sound set
//
// ----------------------------------------------------------------------- //
void CSoundSet::Init(HRECORD hRecord)
{
	m_hRecord = hRecord;
	m_nTotalWeight = 0;
	m_nAltTotalWeight = 0;
	m_vecDirty.clear();
	m_vecWeights.clear();
	m_vecAltDirty.clear();
	m_vecAltWeights.clear();

	LTASSERT(hRecord,"Invalid sound set record.");
	if (!m_hRecord)
		return;

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)
	HATTRIBUTE hFiles =	g_pLTDatabase->GetAttribute(m_hRecord,SndDB_sSoundFile);
#endif // PLATFORM_WIN32 || PLATFORM_LINUX

#ifdef PLATFORM_XENON
	HATTRIBUTE hFiles =	g_pLTDatabase->GetAttribute(m_hRecord,SndDB_sXActCue);
#endif // PLATFORM_XENON

	HATTRIBUTE hWeights = g_pLTDatabase->GetAttribute(m_hRecord,SndDB_nWeight);
	LTASSERT(hFiles,"Invalid sound set record.");
	if (!hFiles)
		return;

	uint32 numFiles = g_pLTDatabase->GetNumValues(hFiles);

	for (uint32 n = 0; n < numFiles; ++n)
	{
		m_nTotalWeight += g_pLTDatabase->GetInt32(hWeights,n,1);
		m_vecDirty.push_back(false);
		m_vecWeights.push_back(m_nTotalWeight);
	}

	HATTRIBUTE hAltFiles =	g_pLTDatabase->GetAttribute(m_hRecord,SndDB_sAltSoundFile);
	HATTRIBUTE hAltWeights = g_pLTDatabase->GetAttribute(m_hRecord,SndDB_nAltWeight);
	//LTASSERT(hAltFiles,"Invalid sound set record.");	// actually, it's okay if we don't have alt files..
	if (hFiles)
	{
		numFiles = g_pLTDatabase->GetNumValues(hAltFiles);

		for (uint32 n = 0; n < numFiles; ++n)
		{
			m_nAltTotalWeight += g_pLTDatabase->GetInt32(hAltWeights,n,1);
			m_vecAltDirty.push_back(false);
			m_vecAltWeights.push_back(m_nAltTotalWeight);
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundSet::GetRandomFile()
//
//	PURPOSE:	Get a random sound from the set
//
// ----------------------------------------------------------------------- //
const char* CSoundSet::GetRandomFile()
{
	//if we're not set up or we have no files, bail
	if (!m_hRecord || !m_nTotalWeight || !m_vecWeights.size()) 
		return "";

#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)
	HATTRIBUTE hFiles =	g_pLTDatabase->GetAttribute(m_hRecord,SndDB_sSoundFile);

	//only got one, return it
	if (m_vecWeights.size() == 1)
		return g_pLTDatabase->GetString(hFiles,0,"");

	uint32 nRand = GetRandom(0,m_nTotalWeight-1);

	for (uint32 n = 0; n < m_vecWeights.size(); ++n)
	{
		if (nRand < m_vecWeights[n])
			return g_pLTDatabase->GetString(hFiles,n,"");
	}

	//if we got this far, just return the first value
	return g_pLTDatabase->GetString(hFiles,0,"");
#endif // PLATFORM_WIN32 || PLATFORM_LINUX

#ifdef PLATFORM_XENON
	HATTRIBUTE hCue = g_pLTDatabase->GetAttribute(m_hRecord,SndDB_sXActCue);

	if (hCue)
	{
		return g_pLTDatabase->GetString(hCue,0,"");
	}
	else
	{
		return "";
	}
#endif // PLATFORM_XENON
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundSet::GetRandomNotDirtyFile()
//
//	PURPOSE:	Get a random sound from the set that has not been played recently
//
// ----------------------------------------------------------------------- //
const char* CSoundSet::GetRandomNotDirtyFile()
{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_LINUX)
	//if we're not set up or we have no files, bail
	if (!m_hRecord || !m_nTotalWeight || !m_vecWeights.size()) 
		return "";

	HATTRIBUTE hFiles =	g_pLTDatabase->GetAttribute(m_hRecord,SndDB_sSoundFile);

	//only got one, return it
	if (m_vecWeights.size() == 1)
		return g_pLTDatabase->GetString(hFiles,0,"");

	uint32 nRand = GetRandom(0,m_nTotalWeight-1);

	uint32 n;
	for ( n = 0; n < m_vecWeights.size(); ++n)
	{
		if (nRand < m_vecWeights[n])
			break;
	}

	bool bFoundClean = false;
	uint32 nStart = n;
	while (!bFoundClean)
	{
		//found a clean one
		if (!m_vecDirty[n])
		{
			m_vecDirty[n] = true;
			return g_pLTDatabase->GetString(hFiles,n,"");
		}

		//it's dirty check the next one
		n = (n + 1) % m_vecWeights.size();

		//all dirty
		if (n == nStart)
		{
			ClearDirty();
			m_vecDirty[n] = true;
			return g_pLTDatabase->GetString(hFiles,n,"");
		}

	}

	//if we got this far, just return the first value
	m_vecDirty[0] = true;
	return g_pLTDatabase->GetString(hFiles,0,"");
#endif //PLATFORM_WIN32 || PLATFORM_LINUX


#ifdef PLATFORM_XENON
	//if we're not set up or we have no files, bail
	if (!m_hRecord) 
		return "";

	HATTRIBUTE hCue = g_pLTDatabase->GetAttribute(m_hRecord,SndDB_sXActCue);

	if (hCue)
	{
		return g_pLTDatabase->GetString(hCue,0,"");
	}
	else
	{
		return "";
	}
#endif // PLATFORM_XENON

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundSet::GetRandomAltFile()
//
//	PURPOSE:	Get a random alternate sound from the set
//
// ----------------------------------------------------------------------- //
const char* CSoundSet::GetRandomAltFile()
{
	//if we're not set up or we have no files, bail
	if (!m_hRecord || !m_nAltTotalWeight || !m_vecAltWeights.size()) 
		return "";

	HATTRIBUTE hFiles =	g_pLTDatabase->GetAttribute(m_hRecord,SndDB_sAltSoundFile);

	//only got one, return it
	if (m_vecAltWeights.size() == 1)
		return g_pLTDatabase->GetString(hFiles,0,"");

	uint32 nRand = GetRandom(0,m_nAltTotalWeight-1);

	for (uint32 n = 0; n < m_vecAltWeights.size(); ++n)
	{
		if (nRand < m_vecAltWeights[n])
			return g_pLTDatabase->GetString(hFiles,n,"");
	}

	//if we got this far, just return the first value
	return g_pLTDatabase->GetString(hFiles,0,"");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundSet::GetRandomNotDirtyFile()
//
//	PURPOSE:	Get a random sound from the set that has not been played recently
//
// ----------------------------------------------------------------------- //
void CSoundSet::ClearDirty()
{
	for (uint32 n = 0; n < m_vecDirty.size(); ++n)
	{
		m_vecDirty[n] = false;
	}

	for (uint32 n = 0; n < m_vecAltDirty.size(); ++n)
	{
		m_vecAltDirty[n] = false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::CSoundDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CSoundDB::CSoundDB()
:	CGameDatabaseMgr( ),
	m_hSoundCat(NULL),
	m_hCharacterSoundCat(NULL)
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::~CSoundDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSoundDB::~CSoundDB()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CSoundDB::Init( const char *szDatabaseFile /* = FDB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pSoundDB = this;

	// Get handles to all of the categories in the database...
	m_hSoundCat = g_pLTDatabase->GetCategory(m_hDatabase,SndDB_SoundCat);
	m_hCharacterSoundCat = g_pLTDatabase->GetCategory(m_hDatabase,SndDB_CharacterSoundCat);

	if (!InitSoundSets())
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::Term()
//
//	PURPOSE:	Clean up
//
// ----------------------------------------------------------------------- //
void CSoundDB::Term()
{
	m_vecSoundSets.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::InitSoundSets()
//
//	PURPOSE:	Initialize the random sound selection tables
//
// ----------------------------------------------------------------------- //
bool CSoundDB::InitSoundSets()
{
	uint32 numRecords = GetNumSoundRecords();
	m_vecSoundSets.resize(numRecords);
	if ( m_vecSoundSets.size () != numRecords )
	{
		LTERROR( "Failed to allocate SoundSets!" );
		return false;
	}

	for (uint32 nRecord = 0; nRecord < numRecords; nRecord++)
	{
		CSoundSet* pSet = &m_vecSoundSets[nRecord];
		HRECORD hSR = g_pLTDatabase->GetRecordByIndex(m_hSoundCat,nRecord);
		pSet->Init(hSR);
	}
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::GetNumSoundRecords()
//
//	PURPOSE:	Get the number of sound records available
//
// ----------------------------------------------------------------------- //
uint32 CSoundDB::GetNumSoundRecords()
{
	return g_pLTDatabase->GetNumRecords(m_hSoundCat);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::GetSoundDBRecord()
//
//	PURPOSE:	Get the HRECORD for a given sound
//
// ----------------------------------------------------------------------- //
HRECORD	CSoundDB::GetSoundDBRecord(uint32 nIndex)
{
	return g_pLTDatabase->GetRecordByIndex(m_hSoundCat,nIndex);
}

HRECORD	CSoundDB::GetSoundDBRecord(const char* pName)
{
	return g_pLTDatabase->GetRecord(m_hSoundCat,pName);
}

HRECORD CSoundDB::GetCharacterSoundDBRecord(HRECORD hModel, const char* pName)
{
	HRECORD hCharacterSound = g_pLTDatabase->GetRecord(m_hCharacterSoundCat,pName);
	if (!hCharacterSound)
	{
		CONTENT_WARNING( 
			const char* pszModelName = g_pLTDatabase->GetRecordName(hModel);
			g_pLTBase->CPrint( "Failed to play CharacterSound '%s' (Model: '%s') - no such CharacterSound in the database.", 
				pName, pszModelName ? pszModelName : "<invalid>" );
		)
		return NULL;
	}

	HRECORD hSoundTemplate = g_pModelsDB->GetModelSoundTemplate(hModel);
	if (!hSoundTemplate)
	{
		CONTENT_WARNING(
			const char* pszModelName = g_pLTDatabase->GetRecordName(hModel);
			g_pLTBase->CPrint( "Failed to play CharacterSound '%s' (Model: '%s') - Could not get SoundTemplate from model.", 
				pName, pszModelName ? pszModelName : "<invalid>" );
		)
		return NULL;
	}

	// Look for a specialized match based on the passed in AIAttribute ID

	HATTRIBUTE hPairs = g_pLTDatabase->GetAttribute(hCharacterSound,"Pairs");
	if (!hPairs)
	{
		LTERROR("Cound not find Attribute: 'Pairs'");
		return NULL;
	}

	uint32 nPairs = g_pLTDatabase->GetNumValues(hPairs);
	for (uint32 nIndex = 0; nIndex < nPairs; nIndex++)
	{
		HATTRIBUTE hSoundTemplateAttr = GetStructAttribute(hPairs,nIndex,"Character");
		if (!hSoundTemplateAttr) continue;

		if (GetRecordLink(hSoundTemplateAttr) == hSoundTemplate)
		{
			HATTRIBUTE hSoundAttr = GetStructAttribute(hPairs,nIndex,"Sound");
			if (!hSoundAttr) continue;

			return GetRecordLink(hSoundAttr);
		}
	}

	// Return the default match, which may be NULL (if there is no default for this sound)

	return GetRecordLink(hCharacterSound,"Default");
}

bool CSoundDB::FillSoundRecord(HRECORD hSR, SoundRecord& sr)
{
	if (!hSR)
		return false;

	sr.m_nFlags = 0;

	sr.m_fInnerRad = GetFloat(hSR,SndDB_fInnerRadius);				// Inner sound radius
	sr.m_fOuterRad = GetFloat(hSR,SndDB_fOuterRadius);				// Outer sound radius

	sr.m_fPitch = GetFloat(hSR,SndDB_fPitch);							// The pitch of the sound
	if(sr.m_fPitch != 1.0f) 
		sr.m_nFlags |= PLAYSOUND_CTRL_PITCH;

	sr.m_fPlayChance = GetFloat(hSR,SndDB_fPlayChance);				// The chance that a sound will play
	sr.m_fPlayChance = LTCLAMP(sr.m_fPlayChance,0.0f,1.0f);

	sr.m_nVolume = GetInt32(hSR,SndDB_nVolume);						// The volume of the sound
	if(sr.m_nVolume != 100) 
		sr.m_nFlags |= PLAYSOUND_CTRL_VOL;

	sr.m_ePriority = (SoundPriority)GetInt32(hSR,SndDB_nPriority);	// The priority of the sound

	sr.m_fDopplerFactor = GetFloat(hSR, SndDB_fDopplerFactor);	// the doppler factor

	sr.m_fSoundSwitchRad = GetFloat(hSR,SndDB_fSwitchRadius);				// switch sound radius

	if ( (sr.m_fSoundSwitchRad > 0.1f)   &&		// allow a bit of float error..
		 (sr.m_fSoundSwitchRad < sr.m_fOuterRad) )	// no point if the switch is outside the outer  
													//radius, since it'll be at 0 volume anyway..
	{
		sr.m_bShouldTestSwitchRadius = true;
	}
	else
	{
		sr.m_bShouldTestSwitchRadius = false;
	}


	//sr.m_nFlags |= GetBool(hSR,SndDB_bPlaySoundLocal) ? PLAYSOUND_LOCAL : (PLAYSOUND_3D | PLAYSOUND_USEOCCLUSION);
	//sr.m_nFlags |= GetBool(hSR,SndDB_bPlaySoundLocal) ? PLAYSOUND_LOCAL : (PLAYSOUND_3D);

	if (GetBool(hSR,SndDB_bPlaySoundLocal))
	{
		sr.m_nFlags |= PLAYSOUND_LOCAL;
	}
	else
	{
		sr.m_nFlags |= PLAYSOUND_3D;

		// add occlusion parser here to set flags..
		// only do for 3d sounds...
		const char* pszOcclusion;
		uint32 occFlags;

		pszOcclusion = GetString(hSR, SndDB_sOcclusion);
		occFlags = PLAYSOUND_USEOCCLUSION;

		if ( pszOcclusion && pszOcclusion[0] )
		{
			if (LTStrCmp(pszOcclusion, "Full") == 0)
			{
				occFlags = PLAYSOUND_USEOCCLUSION;
			}
			else if (LTStrCmp(pszOcclusion, "None") == 0)
			{
				occFlags = 0;
			}
			else if (LTStrCmp(pszOcclusion, "NoInnerRadius") == 0)
			{
				occFlags = PLAYSOUND_USEOCCLUSION | PLAYSOUND_USEOCCLUSION_NO_INNER_RADIUS;
			}
		}
		sr.m_nFlags |= occFlags;
	}


	if (GetBool(hSR,SndDB_bPlayClientLocal))
		sr.m_nFlags |= PLAYSOUND_CLIENTLOCAL;

	if (GetBool(hSR,SndDB_bLoop))
		sr.m_nFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;

	if (GetBool(hSR,SndDB_bReverb))
		sr.m_nFlags |= PLAYSOUND_REVERB;
	
	if (GetBool(hSR,SndDB_bPlayOnce))
		sr.m_nFlags |= PLAYSOUND_ONCE;

	sr.m_bLipSync = GetBool(hSR,SndDB_bLipSync);			// Will lip sync if able
	sr.m_fMinDelay = GetFloat(hSR,SndDB_fMinDelay);		// min delay before playing this sound again
	sr.m_fMaxDelay = GetFloat(hSR,SndDB_fMaxDelay);		// max delay before playing this sound again
	sr.m_nMixChannel = (int16)GetInt32(hSR, SndDB_nMixChannel);			// mixer channel for output

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::GetRandomSoundFileWeighted()
//
//	PURPOSE:	Fills in sound record, returns false if record does not exist
//
// ----------------------------------------------------------------------- //
const char* CSoundDB::GetRandomSoundFileWeighted(HRECORD hSR)
{
	LTASSERT(hSR,"Invalid sound set specified.");
	if (!hSR) return "";

	uint32 nIndex = g_pLTDatabase->GetRecordIndex(hSR);

	if ( nIndex >= m_vecSoundSets.size() || nIndex < 0 )
	{
		LTERROR( "Out of bounds soundset index." );
		return "";
	}

	return m_vecSoundSets[nIndex].GetRandomFile();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::GetRandomSoundFileNotDirty()
//
//	PURPOSE:	Fills in sound record, returns false if record does not exist
//
// ----------------------------------------------------------------------- //
const char* CSoundDB::GetRandomSoundFileNotDirty(HRECORD hSR)
{
	LTASSERT(hSR,"Invalid sound set specified.");
	if (!hSR) return "";

	uint32 nIndex = g_pLTDatabase->GetRecordIndex(hSR);

	if ( nIndex >= m_vecSoundSets.size() || nIndex < 0 )
	{
		LTERROR( "Out of bounds soundset index." );
		return "";
	}

	return m_vecSoundSets[nIndex].GetRandomNotDirtyFile();
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDB::GetRandomAltSoundFileWeighted()
//
//	PURPOSE:	Fills in alternate sound record, returns false if record does not exist
//
// ----------------------------------------------------------------------- //
const char* CSoundDB::GetRandomAltSoundFileWeighted(HRECORD hSR)
{
	LTASSERT(hSR,"Invalid sound set specified.");
	if (!hSR) return "";

	uint32 nIndex = g_pLTDatabase->GetRecordIndex(hSR);

	if ( nIndex >= m_vecSoundSets.size() || nIndex < 0 )
	{
		LTASSERT(nIndex < m_vecSoundSets.size() && nIndex >= 0, "Out of bounds soundset index.");
		return "";
	}

	return m_vecSoundSets[nIndex].GetRandomAltFile();
}


#if defined( _SERVERBUILD )

/////////////////////////////////////////////////////////////////////////////
//
//	S E R V E R - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//
// CSoundDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use SoundDB
//
////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDBPlugin::CSoundDBPlugin()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CSoundDBPlugin::CSoundDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDBPlugin::~CSoundDBPlugin()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSoundDBPlugin::~CSoundDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundDBPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CSoundDBPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
	uint32* pcStrings,
	const uint32 cMaxStrings,
	const uint32 cMaxStringLength)
{

	// Be sure the null sound is selectable.
	if((*pcStrings) + 1 < cMaxStrings)
	{
		LTStrCpy(aszStrings[(*pcStrings)++], "", cMaxStringLength);
	}


	// Add an entry for each character...
	uint32 nNumSounds = g_pSoundDB->GetNumSoundRecords();

	for (uint32 i = 0; i < nNumSounds; i++)
	{
		HRECORD hRec = g_pSoundDB->GetSoundDBRecord(i);

		LTASSERT(cMaxStrings > (*pcStrings) + 1, "Too many records in sound database");

		if((*pcStrings) + 1 < cMaxStrings)
		{
			if (hRec && g_pSoundDB->GetBool(hRec,SndDB_bWorldEditVisible))
				LTStrCpy(aszStrings[(*pcStrings)++], g_pLTDatabase->GetRecordName(hRec), cMaxStringLength);

		}
	}

	// Sort all the strings, but leave the "" at the top.
	qsort( aszStrings + 1, *pcStrings - 1, sizeof( char * ), CaseInsensitiveCompare );

	return LT_OK;
}


#endif // _SERVERBUILD
