// ----------------------------------------------------------------------- //
//
// MODULE  : SoundOcclusionDB.cpp
//
// PURPOSE : Implementation of Sound Occusion surface database
//
// CREATED : 06/04/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "SoundOcclusionDB.h"

//
// Defines...
//

//FX categories
const char* const SoundOcclusionDB_SoundOcclusionCat =		"Sound/Occlusion";


//
// Globals...
//

CSoundOcclusionDB* g_pSoundOcclusionDB = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDB::CSoundOcclusionDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CSoundOcclusionDB::CSoundOcclusionDB()
:	CGameDatabaseMgr( ),
	m_hSoundOcclusionCat(NULL)
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDB::~CSoundOcclusionDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSoundOcclusionDB::~CSoundOcclusionDB()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CSoundOcclusionDB::Init( const char *szDatabaseFile /* = FDB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pSoundOcclusionDB = this;

	// Get handles to all of the categories in the database...
	m_hSoundOcclusionCat = g_pLTDatabase->GetCategory(m_hDatabase,SoundOcclusionDB_SoundOcclusionCat);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDB::GetSoundOcclusion
//
//	PURPOSE:	Get sound Occlusion record
//
// ----------------------------------------------------------------------- //

HSOUNDOCCLUSION	CSoundOcclusionDB::GetSoundOcclusion(uint32 nIndex)
{
	if (INVALID_GAME_DATABASE_INDEX == nIndex) return NULL;
	return g_pLTDatabase->GetRecordByIndex(m_hSoundOcclusionCat,nIndex);
}

HSOUNDOCCLUSION	CSoundOcclusionDB::GetSoundOcclusion(const char* pszSoundOcclusion)
{
	return g_pLTDatabase->GetRecord(m_hSoundOcclusionCat,pszSoundOcclusion);
}

uint32 CSoundOcclusionDB::GetSoundOcclusionVisualizationColor(HSOUNDOCCLUSION hOcclusion)
{
	return g_pSoundOcclusionDB->GetInt32(hOcclusion, SoundOcclusionDB_cVisColor);
}

uint32 CSoundOcclusionDB::GetSoundOcclusionID(HSOUNDOCCLUSION hOcclusion)
{
	return g_pSoundOcclusionDB->GetInt32(hOcclusion, SoundOcclusionDB_nID);
}

float CSoundOcclusionDB::GetSoundOcclusionDampening(HSOUNDOCCLUSION hOcclusion)
{
	return g_pSoundOcclusionDB->GetFloat(hOcclusion, SoundOcclusionDB_fDampening);
}

float CSoundOcclusionDB::GetSoundOcclusionLFRatio(HSOUNDOCCLUSION hOcclusion)
{
	return g_pSoundOcclusionDB->GetFloat(hOcclusion, SoundOcclusionDB_fLowFreqRatio);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDB::GetNumSoundOcclusionTypes()
//
//	PURPOSE:	Get number of sound occlusion record
//
// ----------------------------------------------------------------------- //

uint32 CSoundOcclusionDB::GetNumSoundOcclusionTypes()
{
	return g_pLTDatabase->GetNumRecords(m_hSoundOcclusionCat);
}

#if defined( _SERVERBUILD )

/////////////////////////////////////////////////////////////////////////////
//
//	S E R V E R - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//
// CSoundOcclusionDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use SoundOcclusionDB
//
////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDBPlugin::CSoundOcclusionDBPlugin()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

/*
CSoundOcclusionDBPlugin::CSoundOcclusionDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDBPlugin::~CSoundOcclusionDBPlugin()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSoundOcclusionDBPlugin::~CSoundOcclusionDBPlugin()
{
}
*/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDBPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool CSoundOcclusionDBPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
										  const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return false;
	LTASSERT(aszStrings && pcStrings, "CSoundOcclusionDBPlugin::PopulateStringList");

	//add an entry for the 'None' option as well just to make it more user friendly
	if(*pcStrings < cMaxStrings)
	{
		LTStrCpy( aszStrings[(*pcStrings)++], "None", cMaxStringLength );
	}

	// Add an entry for each sound occlusion type
	uint32 nNumSoundOcclusion = CSoundOcclusionDB::Instance().GetNumSoundOcclusionTypes();
	LTASSERT(nNumSoundOcclusion < INVALID_GAME_DATABASE_INDEX, "CSoundOcclusionDBPlugin::PopulateStringList");

	HSOUNDOCCLUSION hSoundOcclusion = NULL;

	for (uint32 i=0; i < nNumSoundOcclusion; i++)
	{
		LTASSERT(cMaxStrings > (*pcStrings) + 1, "CSoundOcclusionDBPlugin::PopulateStringList");

		hSoundOcclusion = CSoundOcclusionDB::Instance().GetSoundOcclusion(i);
		if (!hSoundOcclusion)
			continue;
		
		const char* pszName = CSoundOcclusionDB::Instance().GetRecordName(hSoundOcclusion);
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
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundOcclusionDBPlugin::GetOcclusionSurfaceInfoList
//
//	PURPOSE:	Populate the list  with all the occlusion info
//
// ----------------------------------------------------------------------- //

bool CSoundOcclusionDBPlugin::GetOcclusionSurfaceInfoList(OcclusionSurfaceEditInfo* aSurfaceList, uint32* pnNumSurfaces,
												 const uint32 nMaxSurfaces, const uint32 cMaxStringLength)
{
	if (!aSurfaceList || !pnNumSurfaces) return false;
	LTASSERT(aSurfaceList && pnNumSurfaces, "CSoundOcclusionDBPlugin::PopulateStringList");

	// Add an entry for each sound occlusion type
	uint32 nNumSoundOcclusion = CSoundOcclusionDB::Instance().GetNumSoundOcclusionTypes();
	LTASSERT(nNumSoundOcclusion < INVALID_GAME_DATABASE_INDEX, "CSoundOcclusionDBPlugin::PopulateStringList");

	HSOUNDOCCLUSION hSoundOcclusion = NULL;
	OcclusionSurfaceEditInfo* pSurface=NULL;

	*pnNumSurfaces = nNumSoundOcclusion;
	for (uint32 i=0; i < nNumSoundOcclusion; i++)
	{
		pSurface = &(aSurfaceList[i]);

		// clear the surface..
		pSurface->pcName[0] = 0;
		pSurface->pcDescription[0] = 0;
		pSurface->nID = 255;
		pSurface->color = 0;


		// get the current occlusion surface..
		hSoundOcclusion = CSoundOcclusionDB::Instance().GetSoundOcclusion(i);
		if (!hSoundOcclusion)
			continue;

		// now get the record name...
		const char* pszName = CSoundOcclusionDB::Instance().GetRecordName(hSoundOcclusion);
		if (!pszName)
			continue;

		uint32 dwNameLen = LTStrLen(pszName);

		if (dwNameLen && (dwNameLen < cMaxStringLength))
		{
			LTStrCpy(pSurface->pcName, pszName, cMaxStringLength);
		}

		float damp, lfratio;

		damp = CSoundOcclusionDB::Instance().GetSoundOcclusionDampening(hSoundOcclusion);
		lfratio = CSoundOcclusionDB::Instance().GetSoundOcclusionLFRatio(hSoundOcclusion);
		pSurface->color = CSoundOcclusionDB::Instance().GetSoundOcclusionVisualizationColor(hSoundOcclusion);

		char buffer[255];	// this should be enough..

		// use a temporary buffer to help prevent overruns..
		sprintf(buffer, "Dampening [%4.2f]dB LowFreq [%1.3f]", damp, lfratio);
		LTStrCpy(pSurface->pcDescription, buffer, cMaxStringLength);

		pSurface->nID = CSoundOcclusionDB::Instance().GetSoundOcclusionID(hSoundOcclusion);

	}

	return true;
}


#endif // _SERVERBUILD

