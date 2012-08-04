// ----------------------------------------------------------------------- //
//
// MODULE  : SoundMixerDB.h
//
// PURPOSE : Definition of Sound Mixer database
//
// CREATED : 03/29/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUNDMIXERDB_H__
#define __SOUNDMIXERDB_H__


//
// Includes...
//

#include "GameDatabaseMgr.h"

typedef HRECORD	HSOUNDMIXER;



//
// Defines...
//
const char* const SoundMixerDB_rName =				"MixerName";
const char* const SoundMixerDB_nID =				"ID";
const char* const SoundMixerDB_afVolume =			"Volume";
const char* const SoundMixerDB_afEffectLevel =		"EffectLevel";
const char* const SoundMixerDB_afLowPass =			"LowPass";
const char* const SoundMixerDB_afPitchLevel =		"PitchShift";
const char* const SoundMixerDB_afOcclusionFactor =	"OcclusionFactor";
const char* const SoundMixerDB_nFadeInTime =		"FadeInTime";
const char* const SoundMixerDB_nPriority =			"Priority";
const char* const SoundMixerDB_bTempMixer =			"bTempMixer";
const char* const SoundMixerDB_nMixerTime =			"MixerTime";
const char* const SoundMixerDB_nFadeOutTime =		"FadeOutTime";
const char* const SoundMixerDB_nLayer =				"MixerLayer";


class CSoundMixerDB;
extern CSoundMixerDB* g_pSoundMixerDB;

class CSoundMixerDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CSoundMixerDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {};

	uint32	GetNumSoundMixerTypes();

	HSOUNDMIXER	GetSoundMixer(uint32 nIndex);
	HSOUNDMIXER	GetSoundMixer(const char* pszSoundMixer);

	HCATEGORY	GetMixerCategory() const	{ return m_hSoundMixerCat; };

private	:	// Members...

	HCATEGORY	m_hSoundMixerCat;

};


////////////////////////////////////////////////////////////////////////////
//
// CSoundMixerDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use SoundMixerDB
//
////////////////////////////////////////////////////////////////////////////
#ifdef _SERVERBUILD

#include "iobjectplugin.h"


class CSoundMixerDBPlugin : public IObjectPlugin
{
public:
	/*
	CSoundMixerDBPlugin();
	CSoundMixerDBPlugin( const CSoundMixerDBPlugin &other );
	CSoundMixerDBPlugin& operator=( const CSoundMixerDBPlugin &other );
	~CSoundMixerDBPlugin();
	  */

public:

	NO_INLINE static CSoundMixerDBPlugin& Instance() { static CSoundMixerDBPlugin sPlugin; return sPlugin; }

	static bool PopulateStringList(char** aszStrings, uint32* pcStrings,
		const uint32 cMaxStrings, const uint32 cMaxStringLength);

};


#endif // _SERVERBUILD


#endif  // __SOUNDMIXERDB_H__

