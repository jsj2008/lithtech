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

#define INTELMGR_TAG					"Intel"

#define INTELMGR_NAME					"Name"
#define INTELMGR_DEFAULTTEXTID			"DefaultTextId"
#define INTELMGR_CHROME					"Chrome"
#define INTELMGR_CHROMAKEY				"Chromakey"
#define INTELMGR_FILENAME				"Filename"
#define INTELMGR_SKIN					"Skin"
#define INTELMGR_SCALEFXNAME			"ScaleName"


static char s_aTagName[30];
static char s_aAttName[100];

// Global pointer to surface mgr...

CIntelMgr*   g_pIntelMgr = LTNULL;

// Plugin statics
CIntelMgr CIntelMgrPlugin::sm_IntelMgr;

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

LTBOOL CIntelMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pIntelMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

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
//	ROUTINE:	CIntelMgr::CacheAll
//
//	PURPOSE:	Cache all the Prop Type related resources
//
// ----------------------------------------------------------------------- //

void CIntelMgr::CacheAll()
{
	// Cache all the Intel data...

    INTEL** pCurIntel  = LTNULL;
	pCurIntel = m_IntelList.GetItem(TLIT_FIRST);

	while (pCurIntel)
	{
		if (*pCurIntel)
		{
			(*pCurIntel)->Cache(this);
		}

		pCurIntel = m_IntelList.GetItem(TLIT_NEXT);
	}
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

	szName[0]		= '\0';
	szFilename[0]	= '\0';
	szSkin[0]		= '\0';

	nDefaultTextId	= 0;
	bChrome			= LTFALSE;
	bChromakey		= LTFALSE;

	nNumScaleFXNames = 0;
	for (int i=0; i < INTEL_MAX_SCALE_FX; i++)
	{
		szScaleFXNames[i][0] = '\0';
	}
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
    bChrome		    = (LTBOOL) buteMgr.GetInt(aTagName, INTELMGR_CHROME);
    bChromakey      = (LTBOOL) buteMgr.GetInt(aTagName, INTELMGR_CHROMAKEY);

	CString str = buteMgr.GetString(aTagName, INTELMGR_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, INTELMGR_FILENAME);
	if (!str.IsEmpty())
	{
		strncpy(szFilename, (char*)(LPCSTR)str, ARRAY_LEN(szFilename));
	}

	str = buteMgr.GetString(aTagName, INTELMGR_SKIN);
	if (!str.IsEmpty())
	{
		strncpy(szSkin, (char*)(LPCSTR)str, ARRAY_LEN(szSkin));
	}

	nNumScaleFXNames = 0;
	for (int i=0; i < INTEL_MAX_SCALE_FX; i++)
	{
		sprintf(s_aAttName, "%s%d", INTELMGR_SCALEFXNAME, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szScaleFXNames[i], (char*)(LPCSTR)str, ARRAY_LEN(szScaleFXNames[i]));
			nNumScaleFXNames++;
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	INTEL::Cache
//
//	PURPOSE:	Cache all the resources associated with the surface
//
// ----------------------------------------------------------------------- //

void INTEL::Cache(CIntelMgr* pIntelMgr)
{
	if (!pIntelMgr) return;

	// Cache skin...

	if (szSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szSkin);
	}

	// Cache models...

	if (szFilename[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szFilename);
	}

	// Cache the scale fx...

    for (int i=0; i < INTEL_MAX_SCALE_FX; i++)
	{
		if (szScaleFXNames[i][0])
		{
			CScaleFX* pFX = g_pFXButeMgr->GetScaleFX(szScaleFXNames[i]);
			if (pFX)
			{
				pFX->Cache();
			}
		}
	}
}



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
        sm_IntelMgr.Init(g_pLTServer, szFile);
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
        sm_IntelMgr.Init(g_pLTServer, szFile);
	}

	INTEL* pIntel = g_pIntelMgr->GetIntel((char*)szPropValue);
	if (!pIntel || !pIntel->szFilename[0])
	{
		return LT_UNSUPPORTED;
	}

	strcpy(szModelFilenameBuf, pIntel->szFilename);

	return LT_OK;
}
