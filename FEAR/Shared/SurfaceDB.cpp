/// ----------------------------------------------------------------------- //
//
// MODULE  : SurfaceDB.cpp
//
// PURPOSE : Implementation of Surface database
//
// CREATED : 03/04/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "SurfaceDB.h"


//
// Defines...
//

//sound category
const char* const SrfDB_SurfaceCat =	"Surfaces/Surfaces";
const char* const SrfDB_WeaponCat =		"Surfaces/WeaponFX";
const char* const SrfDB_ImpactCat =		"Surfaces/Impacts";

//surface attributes
const char* const SrfDB_sSoundFile =			"SoundFile";

//
// Globals...
//

CSurfaceDB* g_pSurfaceDB = NULL;



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceDB::CSurfaceDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CSurfaceDB::CSurfaceDB()
:	CGameDatabaseMgr( ),
	m_hSurfaceCat(NULL),
	m_hWeaponFXCat(NULL),
	m_hImpactCat(NULL),
	m_vecSurfaces(SrfDB_MaxSurfaces,(HRECORD)NULL)
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceDB::~CSurfaceDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSurfaceDB::~CSurfaceDB()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CSurfaceDB::Init( const char *szDatabaseFile /* = FDB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pSurfaceDB = this;

	// Get handles to all of the categories in the database...
	m_hSurfaceCat = g_pLTDatabase->GetCategory(m_hDatabase,SrfDB_SurfaceCat);
	m_hWeaponFXCat = g_pLTDatabase->GetCategory(m_hDatabase,SrfDB_WeaponCat);
	m_hImpactCat = g_pLTDatabase->GetCategory(m_hDatabase,SrfDB_ImpactCat);

	uint32 nNumSurfaces = g_pLTDatabase->GetNumRecords(m_hSurfaceCat);
	for (uint32 n = 0;n < nNumSurfaces;++n) 
	{
		HRECORD hSurf = g_pLTDatabase->GetRecordByIndex(m_hSurfaceCat,n);

		uint32 nID = GetInt32(hSurf,SrfDB_Srf_nId);
		const char *pszName = GetRecordName(hSurf);
		LTASSERT(nID < SrfDB_MaxSurfaces,"Invalid surface ID.");
		if (nID >= SrfDB_MaxSurfaces) 
			return false;

		if (m_vecSurfaces[nID] != NULL)
		{
			const char *pszName2 = GetRecordName(m_vecSurfaces[nID]);
			char szMsg[256];
			LTSNPrintF(szMsg,LTARRAYSIZE(szMsg),"%s and %s share the same surface ID.",pszName,pszName2);
			LTERROR(szMsg);
			return false;
		}

		m_vecSurfaces[nID] = hSurf;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceDB::Term()
//
//	PURPOSE:	Clean up
//
// ----------------------------------------------------------------------- //
void CSurfaceDB::Term()
{
	m_vecSurfaces.clear();
}

HSURFACE CSurfaceDB::GetSurface(SurfaceType eId)
{
	LTASSERT(eId < SrfDB_MaxSurfaces,"Invalid surface ID.");
	if (eId >= SrfDB_MaxSurfaces) 
		return NULL;

	return m_vecSurfaces[eId];
	
}

HSURFACE CSurfaceDB::GetSurface(const char* pName)
{
	return g_pLTDatabase->GetRecord(m_hSurfaceCat,pName);
}

SurfaceType CSurfaceDB::GetSurfaceType(HSURFACE hSurface)
{
	if (!hSurface)
		return ST_DEFAULT;

	return SurfaceType(g_pSurfaceDB->GetInt32(hSurface,SrfDB_Srf_nId));
}

HSRF_IMPACT CSurfaceDB::GetSurfaceImpactFX(HRECORD hSurface, const char* pszWeaponFXName)
{
	if (!hSurface || !pszWeaponFXName)
		return NULL;

	HRECORD hWeaponFX = GetRecordLink(hSurface,SrfDB_Srf_rWeaponFX);
	if (!hWeaponFX) return NULL;

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hWeaponFX,pszWeaponFXName);
	LTASSERT(hAtt,"Invalid Surface WeaponFX name");

	return g_pLTDatabase->GetRecordLink(hAtt,0,NULL);
}

HATTRIBUTE CSurfaceDB::GetLandingSoundStruct(HRECORD hSurface)
{
	if (!hSurface)
	{
		return NULL;
	}
	return (g_pLTDatabase->GetAttribute(hSurface, SrfDB_Srf_LandingSoundList));
}

int32 CSurfaceDB::GetLandingSoundStructRange(HATTRIBUTE hStruct, int32 index)
{
	HATTRIBUTE hRangeLink;

	if (!hStruct)
	{
		return 0;
	}
	hRangeLink = GetStructAttribute(hStruct, index, SrfDB_Srf_nLandingSoundRange);

	if (!hRangeLink)
	{
		return 0;
	}
	return (g_pLTDatabase->GetInt32(hRangeLink, 0, 0));
}

HRECORD CSurfaceDB::GetLandingSoundStructSound(HATTRIBUTE hStruct, int32 index)
{
	HATTRIBUTE hSoundLink;

	if (!hStruct)
	{
		return 0;
	}
	hSoundLink = GetStructAttribute(hStruct, index, SrfDB_Srf_sLandingSoundBute);

	if (!hSoundLink)
	{
		return 0;
	}
	return (g_pLTDatabase->GetRecordLink(hSoundLink, 0, NULL));
}


uint32 CSurfaceDB::GetNumSurfaces() const
{
	return g_pLTDatabase->GetNumRecords(m_hSurfaceCat);
}

HSURFACE CSurfaceDB::GetSurfaceByIndex(uint32 nIndex)
{
	return g_pLTDatabase->GetRecordByIndex(m_hSurfaceCat,nIndex);
}

#if !defined( _CLIENTBUILD )

/////////////////////////////////////////////////////////////////////////////
//
//	S E R V E R - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////
//
// CSurfaceDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use SurfaceDB
//
////////////////////////////////////////////////////////////////////////////
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceDBPlugin::CSurfaceDBPlugin()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CSurfaceDBPlugin::CSurfaceDBPlugin()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceDBPlugin::~CSurfaceDBPlugin()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSurfaceDBPlugin::~CSurfaceDBPlugin()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSurfaceDBPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

bool CSurfaceDBPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
									 const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	LTASSERT(aszStrings && pcStrings && g_pSurfaceDB, "Invalid parameter supplied");
	if (!aszStrings || !pcStrings || !g_pSurfaceDB) return false;

	// Add an entry for each surface

	uint32 nNumSurface = g_pSurfaceDB->GetNumSurfaces();
	HRECORD hSurface = NULL;

	for (uint32 i=0; i < nNumSurface; i++)
	{
		LTASSERT(cMaxStrings > (*pcStrings) + 1, "Too many strings encountered");

		hSurface = g_pSurfaceDB->GetSurfaceByIndex(i);
		if (!hSurface)
			continue;

		const char* pszName = g_pSurfaceDB->GetRecordName(hSurface);
		if (!pszName)
			continue;

		uint32 dwSurfaceNameLen = LTStrLen(pszName);

		if (dwSurfaceNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pszName);
		}
	}

	return true;

}

#endif // _CLIENTBUILD

