// ----------------------------------------------------------------------- //
//
// MODULE  : SoundOcclusionDB.h
//
// PURPOSE : Definition of Sound Occlusion Surfaces database
//
// CREATED : 06/04/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUNDOCCLUSIONDB_H__
#define __SOUNDOCCLUSIONDB_H__


//
// Includes...
//

#include "GameDatabaseMgr.h"

typedef HRECORD	HSOUNDOCCLUSION;



//
// Defines...
//
const char* const SoundOcclusionDB_nID =				"ID";
const char* const SoundOcclusionDB_fDampening =			"Dampening";
const char* const SoundOcclusionDB_fLowFreqRatio =		"LowFrequencyRatio";
const char* const SoundOcclusionDB_cVisColor =			"VisualizationColor";


class CSoundOcclusionDB;
extern CSoundOcclusionDB* g_pSoundOcclusionDB;

class CSoundOcclusionDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CSoundOcclusionDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {};

	uint32	GetNumSoundOcclusionTypes();

	HSOUNDOCCLUSION	GetSoundOcclusion(uint32 nIndex);
	HSOUNDOCCLUSION	GetSoundOcclusion(const char* pszSoundOcclusion);

	uint32 GetSoundOcclusionVisualizationColor(HSOUNDOCCLUSION hOcclusion);
	uint32 GetSoundOcclusionID(HSOUNDOCCLUSION hOcclusion);
	float GetSoundOcclusionDampening(HSOUNDOCCLUSION hOcclusion);
	float GetSoundOcclusionLFRatio(HSOUNDOCCLUSION hOcclusion);


private	:	// Members...

	HCATEGORY	m_hSoundOcclusionCat;

};


////////////////////////////////////////////////////////////////////////////
//
// CSoundOcclusionDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use SoundOcclusionDB
//
////////////////////////////////////////////////////////////////////////////
#ifdef _SERVERBUILD

#include "iobjectplugin.h"

class CSoundOcclusionDBPlugin : public IObjectPlugin
{
public:
	/*
	CSoundOcclusionDBPlugin();
	CSoundOcclusionDBPlugin( const CSoundOcclusionDBPlugin &other );
	CSoundOcclusionDBPlugin& operator=( const CSoundOcclusionDBPlugin &other );
	~CSoundOcclusionDBPlugin();
	  */

public:

	NO_INLINE static CSoundOcclusionDBPlugin& Instance() { static CSoundOcclusionDBPlugin sPlugin; return sPlugin; }

	static bool PopulateStringList(char** aszStrings, uint32* pcStrings,
		const uint32 cMaxStrings, const uint32 cMaxStringLength);

	static bool CSoundOcclusionDBPlugin::GetOcclusionSurfaceInfoList(OcclusionSurfaceEditInfo* aSurfaceList, uint32* pnNumSurfaces,
		const uint32 nMaxSurfaces, const uint32 cMaxStringLength);


};


#endif // _SERVERBUILD


#endif  // __SOUNDOCCLUSIONDB_H__

