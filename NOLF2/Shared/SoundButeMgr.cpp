// ----------------------------------------------------------------------- //
//
// MODULE  : SoundButeMgr.cpp
//
// PURPOSE : The SoundButeMgr implementation
//
// CREATED : 11/2/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "stdafx.h"
	#include "SoundButeMgr.h"


//
// Defines...
//

	#define SOUND_BUTES_TAG					"Sound"

	#define SOUND_BUTE_PATH					"Path"
	#define SOUND_BUTE_WEIGHT				"Weight"
	#define SOUND_BUTE_INNER_RAD			"InnerRadius"
	#define SOUND_BUTE_OUTER_RAD			"OuterRadius"
	#define SOUND_BUTE_PITCH				"Pitch"
	#define SOUND_BUTE_VOLUME				"Volume"
	#define SOUND_BUTE_LOOP					"LoopSound"
	#define SOUND_BUTE_LOCAL				"PlaysoundLocal"
	#define SOUND_BUTE_CLIENTLOCAL			"PlayClientLocal"
	#define SOUND_BUTE_REVERB				"AllowReverb"
	#define SOUND_BUTE_ONCE					"PlayOnce"
	#define SOUND_BUTE_PLAYCHANCE			"PlayChance"
	#define SOUND_BUTE_PRIORITY				"Priority"
	#define SOUND_BUTE_LIP_SYNC				"LipSync"
	#define SOUND_BUTE_MIN_DELAY			"MinDelay"
	#define SOUND_BUTE_MAX_DELAY			"MaxDelay"

//
// Globals...
//

	CSoundButeMgr* g_pSoundButeMgr = LTNULL;
	static char s_aTagName[30];


SoundFiles::~SoundFiles()
{
	for( SoundFileList::iterator iter = m_szSounds.begin(); iter != m_szSounds.end(); ++iter )
	{
		debug_deletea( *iter );
	}

	m_szSounds.clear();
	m_fWeights.clear();

	m_nNumSounds = 0;
}

//*********************************************************************************
// Construction/Destruction
//*********************************************************************************

CSoundButeMgr::CSoundButeMgr()
{
	m_nNumSoundButes = 0;
	m_pSoundButes = LTNULL;
	m_pSoundFiles = LTNULL;
}

CSoundButeMgr::~CSoundButeMgr()
{
	Term();
}

//*********************************************************************************
//
//	ROUTINE:	CSoundButeMgr::Init()
//	PURPOSE:	Init Sound mgr
//
//*********************************************************************************
bool CSoundButeMgr::CountTags(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) return true;

	CSoundButeMgr *pButeMgr = (CSoundButeMgr*)pData;
	++pButeMgr->m_nNumSoundButes;

	// Keep iterating.
	return true;
}

bool CSoundButeMgr::LoadButes(const char *szTagName, void *pData)
{
	if(!szTagName || !pData) return true;

	CSoundButeMgr *pButeMgr = (CSoundButeMgr*)pData;

	// Load the attributes.
	pButeMgr->LoadSoundButes( szTagName, 
		                      &pButeMgr->m_pSoundButes[pButeMgr->m_nNumSoundButes], 
							  &pButeMgr->m_pSoundFiles[pButeMgr->m_nNumSoundButes]  );

	// Store away the name-index pair for quick look-up.
	pButeMgr->m_IndexTable[pButeMgr->m_pSoundFiles[pButeMgr->m_nNumSoundButes].m_szName] = pButeMgr->m_nNumSoundButes;

	// Increment our counter.
	++pButeMgr->m_nNumSoundButes;

	// Keep iterating.
	return true;
}

LTBOOL CSoundButeMgr::Init( const char* szAttributeFile )
{
	if (g_pSoundButeMgr || !szAttributeFile) return LTFALSE;
	if (!Parse( szAttributeFile )) return LTFALSE;


	// Set up global pointer...
	g_pSoundButeMgr = this;

	// Calculate the number of pickup attribute sets
	m_nNumSoundButes = 0;
	m_buteMgr.GetTags(CountTags, this);

	// Create the sound attribute list
	m_pSoundButes = debug_newa( SoundBute, m_nNumSoundButes );
	m_pSoundFiles = debug_newa( SoundFiles, m_nNumSoundButes );
	ASSERT( m_pSoundButes && m_pSoundFiles);
	if( m_pSoundButes && m_pSoundFiles )
	{
		// Reset m_nNumSoundButes, so that it can be used to determine
		// the current bute being loaded.
		m_nNumSoundButes = 0;

		// Load them butes!
		m_buteMgr.GetTags(LoadButes, this);
	}

	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

	return LTTRUE;
}


//*********************************************************************************
//
//	ROUTINE:	CSoundButeMgr::Term()
//	PURPOSE:	Clean up.
//
//*********************************************************************************

void CSoundButeMgr::Term()
{
	g_pSoundButeMgr = LTNULL;

	debug_deletea( m_pSoundButes );
	m_pSoundButes = LTNULL;

	debug_deletea( m_pSoundFiles );
	m_pSoundFiles = LTNULL;
}

//*********************************************************************************

int CSoundButeMgr::GetSoundSetFromName(const char *szName) const
{
	// Look the name up in our index table.
	IndexTable::const_iterator iter = m_IndexTable.find(szName);

	// If we found it, return the index.
	if( iter != m_IndexTable.end() )
	{
		return iter->second;
	}

#ifdef SOUND_BUTE_DEBUG
	g_pInterface->CPrint("SoundButeMgr: Could not find set %s! Defaulting to index -1!", szName);
#endif

	// We didn't find it, return -1 to indicate failure.
	return INVALID_SOUND_BUTE;
}

//*********************************************************************************

const SoundBute & CSoundButeMgr::GetSoundBute(const char *szName) const
{
	int nSet = GetSoundSetFromName(szName);

	if( nSet >= 0 && nSet < m_nNumSoundButes )
		return m_pSoundButes[nSet];

	return m_pSoundButes[0];
}

//*********************************************************************************

const SoundFiles & CSoundButeMgr::GetSoundFiles(const char *szName) const
{
	int nSet = GetSoundSetFromName(szName);

	if( nSet >= 0 && nSet < m_nNumSoundButes )
		return m_pSoundFiles[nSet];

	return m_pSoundFiles[0];
}

//*********************************************************************************

const char* CSoundButeMgr::GetRandomSoundFile(uint32 nSet)
{
	CheckSound(nSet);
	
	if(m_pSoundFiles[nSet].m_nNumSounds == 0 )
		return LTNULL;

	ASSERT( m_pSoundFiles[nSet].m_nNumSounds == m_pSoundFiles[nSet].m_szSounds.size() );

	const int nSoundIndex = GetRandom(0,m_pSoundFiles[nSet].m_nNumSounds-1);
	return m_pSoundFiles[nSet].m_szSounds[nSoundIndex];
}

//*********************************************************************************

const char* CSoundButeMgr::GetRandomSoundFileWeighted(uint32 nSet)
{
	CheckSound(nSet);
	
	if( m_pSoundFiles[nSet].m_nNumSounds == 0 )
		return "";

	ASSERT( m_pSoundFiles[nSet].m_nNumSounds == m_pSoundFiles[nSet].m_szSounds.size() );

	// If there's only one sound... go ahead and use that one
	if(m_pSoundFiles[nSet].m_nNumSounds == 1)
	{
		return m_pSoundFiles[nSet].m_szSounds[0];
	}

	// Get a random weight to test with
	LTFLOAT fRand = GetRandom(0.0f, 1.0f);

	for(uint32 i = 0; i < m_pSoundFiles[nSet].m_nNumSounds; i++)
	{
		if(m_pSoundFiles[nSet].m_fWeights[i] >= fRand)
			return m_pSoundFiles[nSet].m_szSounds[i];
	}

	// Otherwise, if we got here... just return the last sound
	return m_pSoundFiles[nSet].m_szSounds[m_pSoundFiles[nSet].m_nNumSounds - 1];
}

//*********************************************************************************

void CSoundButeMgr::LoadSoundButes(const char * szTagName, SoundBute * pButes, SoundFiles * pFiles)
{
	ASSERT( pButes && pFiles );
	if( !pButes || !pFiles )
		return;

	//be sure to clear the flags
	pButes->m_nFlags = 0;


	SAFE_STRCPY(pFiles->m_szName, szTagName);

	// The parent needs to be the base parent.
	SAFE_STRCPY(pFiles->m_szParent, GetString(szTagName, "Parent") );
	while( !GetString(pFiles->m_szParent, "Parent").IsEmpty() )
	{
		SAFE_STRCPY(pFiles->m_szParent, GetString(pFiles->m_szParent, "Parent") );
	}



	// This will be used by weight and sound parameters
	char aAttribName[64];

	// Make a value to accumulate the weights
	LTFLOAT fWeightAccum = 0;


	// Get the sound path and filename
	int nNumSoundButes = 0;
	sprintf(aAttribName, "%s%d", SOUND_BUTE_PATH, nNumSoundButes);

	while( true )
	{
		CString strSoundFileName = GetString(szTagName, aAttribName, CString(""));
		if( !Success( ))
			break;

		if( !strSoundFileName.IsEmpty() )
		{
			// Store the sound file name.
			char *pSound = debug_newa( char, strSoundFileName.GetLength() + 1 );
			pFiles->m_szSounds.push_back( pSound );

			SAFE_STRCPY(pFiles->m_szSounds.back(), strSoundFileName);

			// Read the weight value
			sprintf(aAttribName, "%s%d", SOUND_BUTE_WEIGHT, nNumSoundButes);
			const LTFLOAT fWeight = (float)GetDouble(szTagName, aAttribName, 1.0f);
			pFiles->m_fWeights.push_back( fWeight );

			fWeightAccum += fWeight;
		}

		++nNumSoundButes;
		sprintf(aAttribName, "%s%d", SOUND_BUTE_PATH, nNumSoundButes);
	}

	pFiles->m_nNumSounds = pFiles->m_szSounds.size();

	ASSERT( pFiles->m_fWeights.size() == pFiles->m_nNumSounds );

	// The weight values are evened out into a range that's easy to work with (to speed up calculations later)
	LTFLOAT fCurrentWeightAccum = 0.0f;
	for( SoundFiles::WeightList::iterator weight_iter = pFiles->m_fWeights.begin(); 
	     weight_iter != pFiles->m_fWeights.end(); ++weight_iter )
	{
		fCurrentWeightAccum += (*weight_iter) / fWeightAccum;

		(*weight_iter) = fCurrentWeightAccum;
	}

	pButes->m_fInnerRad	= (LTFLOAT)GetDouble(szTagName, SOUND_BUTE_INNER_RAD);
	pButes->m_fOuterRad	= (LTFLOAT)GetDouble(szTagName, SOUND_BUTE_OUTER_RAD);
	pButes->m_fPlayChance = (LTFLOAT)GetDouble(szTagName, SOUND_BUTE_PLAYCHANCE);

	pButes->m_fPitch		= (LTFLOAT)GetDouble(szTagName, SOUND_BUTE_PITCH);
	if(pButes->m_fPitch != 1.0f) pButes->m_nFlags |= PLAYSOUND_CTRL_PITCH;

	pButes->m_nVolume		= (uint8)GetInt(szTagName, SOUND_BUTE_VOLUME);
	if(pButes->m_nVolume != 100) pButes->m_nFlags |= PLAYSOUND_CTRL_VOL;

	pButes->m_ePriority	= (SoundPriority)GetInt(szTagName, SOUND_BUTE_PRIORITY);

	LTBOOL bTest = (LTBOOL)GetInt(szTagName, SOUND_BUTE_LOCAL);
	pButes->m_nFlags |= bTest?PLAYSOUND_LOCAL:PLAYSOUND_3D;

	bTest = (LTBOOL)GetInt(szTagName, SOUND_BUTE_CLIENTLOCAL);
	if(bTest) pButes->m_nFlags |= PLAYSOUND_CLIENTLOCAL;

	// If we're looping, we better also have the handle.
	bTest = (LTBOOL)GetInt(szTagName, SOUND_BUTE_LOOP);
	if(bTest) pButes->m_nFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;

	bTest = (LTBOOL)GetInt(szTagName, SOUND_BUTE_REVERB);
	if(bTest) pButes->m_nFlags |= PLAYSOUND_REVERB;

	bTest = (LTBOOL)GetInt(szTagName, SOUND_BUTE_ONCE);
	if(bTest) pButes->m_nFlags |= PLAYSOUND_ONCE;

	pButes->m_bLipSync	= (LTBOOL)GetInt(szTagName, SOUND_BUTE_LIP_SYNC);

	pButes->m_fMinDelay	= (LTFLOAT)GetDouble(szTagName, SOUND_BUTE_MIN_DELAY);
	pButes->m_fMaxDelay	= (LTFLOAT)GetDouble(szTagName, SOUND_BUTE_MAX_DELAY);
}


//*********************************************************************************
//
// CSoundButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use SoundMgr
//
//*********************************************************************************

#ifndef _CLIENTBUILD  // Server-side only

// Plugin statics

LTBOOL CSoundButeMgrPlugin::sm_bInitted = LTFALSE;
CSoundButeMgr CSoundButeMgrPlugin::sm_ButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT	CSoundButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath, 
	const char* szPropName, 
	char* const * aszStrings, 
	uint32* pcStrings, 
	const uint32 cMaxStrings, 
	const uint32 cMaxStringLength,
	const char* szParent)
{
	if (!sm_bInitted)
	{
		char szFile[256];

		// Create the bute mgr if necessary
		if(!g_pSoundButeMgr)
		{
			sprintf(szFile, "%s\\%s", szRezPath, SOUND_BUTES_DEFAULT_FILE);
			sm_ButeMgr.SetInRezFile(LTFALSE);
			
			sm_bInitted = sm_ButeMgr.Init( szFile );

			if( !sm_bInitted )
				return LT_UNSUPPORTED;
		}
	}

	
	// Be sure the null sound is selectable.
	if((*pcStrings) + 1 < cMaxStrings)
	{
		strcpy(aszStrings[(*pcStrings)++], "None");
	}

	// Add an entry for each character...
	uint32 nNumSounds = g_pSoundButeMgr->GetNumSoundButes();

	for (uint32 i = 0; i < nNumSounds; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		if((*pcStrings) + 1 < cMaxStrings)
		{
			if(!szParent || !stricmp(g_pSoundButeMgr->GetSoundButeParent(i), szParent))
				strcpy(aszStrings[(*pcStrings)++], g_pSoundButeMgr->GetSoundButeName(i));
		}
	}

	return LT_OK;
}


#endif //_CLIENTBUILD
