// ----------------------------------------------------------------------- //
//
// MODULE  : IntelMgr.cpp
//
// PURPOSE : IntelMgr - Implementation
//
// CREATED : 7/25/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "IntelMgr.h"
#include "CommonUtilities.h"
#include "DebrisMgr.h"
#include "WeaponMgr.h"

#define INTELMGR_TAG					"Intel"

#define INTELMGR_NAME					"Name"
#define INTELMGR_DEFAULTTEXTID			"DefaultTextId"
#define INTELMGR_FILENAME				"Filename"
#define INTELMGR_SKIN					"Skin"
#define INTELMGR_POPUPID				"DefaultPopupId"
#define INTELMGR_PICKUPSND				"PickupSound"
#define INTELMGR_RESPAWNSND				"RespawnSound"


static char s_aTagName[30];
static char s_aAttName[100];

// Global pointer to surface mgr...

CIntelMgr*   g_pIntelMgr = LTNULL;

// Plugin statics
#ifndef __PSX2
CIntelMgr CIntelMgrPlugin::sm_IntelMgr;
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgr::CIntelMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CIntelMgr::CIntelMgr()
{
    m_IntelList.Init(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgr::~CIntelMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CIntelMgr::~CIntelMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CIntelMgr::Init(const char* szAttributeFile)
{
    if (g_pIntelMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	g_pIntelMgr = this;


	// Read in the properties for each prop type record...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", INTELMGR_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		INTEL* pIntel = debug_new(INTEL);

		if (pIntel && pIntel->Init(m_buteMgr, s_aTagName))
		{
			pIntel->nId = nNum;
			m_IntelList.AddTail(pIntel);
		}
		else
		{
			debug_delete(pIntel);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", INTELMGR_TAG, nNum);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgr::GetIntel
//
//	PURPOSE:	Get the specified Intel record
//
// ----------------------------------------------------------------------- //

INTEL* CIntelMgr::GetIntel(uint32 nId)
{
    INTEL** pCur  = LTNULL;

	pCur = m_IntelList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nId)
		{
			return *pCur;
		}

		pCur = m_IntelList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgr::GetIntel
//
//	PURPOSE:	Get the specified Intel record
//
// ----------------------------------------------------------------------- //

INTEL* CIntelMgr::GetIntel(char* pName)
{
    if (!pName) return LTNULL;

    INTEL** pCur  = LTNULL;

	pCur = m_IntelList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_IntelList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CIntelMgr::Term()
{
    g_pIntelMgr = LTNULL;

	m_IntelList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	INTEL::INTEL
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

INTEL::INTEL()
{
	nId				= 0;

	szName			= LTNULL;
	szFilename		= LTNULL;
	szSkin			= LTNULL;
	szPickupSnd		= LTNULL;
	szRespawnSnd	= LTNULL;

	nDefaultTextId	= 0;

	nPopupId		= 0;
}

INTEL::~INTEL()
{
	debug_deletea( szName );
	debug_deletea( szFilename );
	debug_deletea( szSkin );
	debug_deletea( szPickupSnd );
	debug_deletea( szRespawnSnd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	INTEL::Init
//
//	PURPOSE:	Build the prop type struct
//
// ----------------------------------------------------------------------- //

LTBOOL INTEL::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	nDefaultTextId	= buteMgr.GetInt(aTagName, INTELMGR_DEFAULTTEXTID);

	szName			= GetString( buteMgr, aTagName, INTELMGR_NAME, INTEL_MAX_NAME_LENGTH );
	szFilename		= GetString( buteMgr, aTagName, INTELMGR_FILENAME, INTEL_MAX_FILE_PATH );
	szSkin			= GetString( buteMgr, aTagName, INTELMGR_SKIN, INTEL_MAX_FILE_PATH );
	szPickupSnd		= GetString( buteMgr, aTagName, INTELMGR_PICKUPSND, INTEL_MAX_FILE_PATH );
	szRespawnSnd	= GetString( buteMgr, aTagName, INTELMGR_RESPAWNSND, INTEL_MAX_FILE_PATH );

	nPopupId	= (uint8)buteMgr.GetInt(aTagName, INTELMGR_POPUPID);

    return LTTRUE;
}

#ifndef __PSX2
////////////////////////////////////////////////////////////////////////////
//
// CIntelMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CIntelMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CIntelMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pIntelMgr)
	{
		// This will set the g_pIntelMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, INTELMGR_DEFAULT_FILE);
        sm_IntelMgr.SetInRezFile(LTFALSE);
        sm_IntelMgr.Init(szFile);
	}

	if (!PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength))
	{
		return LT_UNSUPPORTED;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CIntelMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each Intel type

	int nNumIntel = g_pIntelMgr->GetNumIntels();
	_ASSERT(nNumIntel > 0);

    INTEL* pIntel = LTNULL;

	for (int i=0; i < nNumIntel; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pIntel = g_pIntelMgr->GetIntel(i);
		if (pIntel && pIntel->szName[0])
		{
            uint32 dwImpactFXNameLen = strlen(pIntel->szName);

			if (dwImpactFXNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pIntel->szName);
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelMgrPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this prop
//
// ----------------------------------------------------------------------- //

LTRESULT CIntelMgrPlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	if (!g_pIntelMgr)
	{
		// This will set the g_pIntelMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, INTELMGR_DEFAULT_FILE);
        sm_IntelMgr.SetInRezFile(LTFALSE);
        sm_IntelMgr.Init(szFile);
	}

	INTEL* pIntel = LTNULL;
	if (strlen(szPropValue))
		pIntel = g_pIntelMgr->GetIntel((char*)szPropValue);
	else
		pIntel = g_pIntelMgr->GetIntel((uint32)0);

	if (!pIntel || !pIntel->szFilename[0])
	{
		return LT_UNSUPPORTED;
	}

	strcpy(szModelFilenameBuf, pIntel->szFilename);
	
	// Need to convert the .ltb filename to one that DEdit understands...

	ConvertLTBFilename(szModelFilenameBuf);


	return LT_OK;
}
#endif