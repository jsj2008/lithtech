// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFilterMgr.cpp
//
// PURPOSE : SoundFilterMgr - Implementation
//
// CREATED : 7/16/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SoundFilterMgr.h"
#include "CommonUtilities.h"

#define SFM_TAG					"SoundFilter"

#define SFM_NAME				"Name"
#define SFM_FILTERNAME			"FilterName"
#define SFM_VARIABLE			"Var"
#define SFM_VALUE				"Value"

static char s_aTagName[30];
static char s_aAttName[100];

// Global pointer to surface mgr...

CSoundFilterMgr* g_pSoundFilterMgr = LTNULL;


#ifndef _CLIENTBUILD
#ifndef __PSX2
// Plugin statics

CSoundFilterMgr CSoundFilterMgrPlugin::sm_SoundFilterMgr;

#endif // !__PSX2
#endif // _CLIENTBUILD


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFilterMgr::CSoundFilterMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSoundFilterMgr::CSoundFilterMgr()
{
    m_FilterList.Init(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFilterMgr::~CSoundFilterMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSoundFilterMgr::~CSoundFilterMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFilterMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CSoundFilterMgr::Init(const char* szAttributeFile)
{
    if (g_pSoundFilterMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	g_pSoundFilterMgr = this;

	// Read in the properties for each sound filter record...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", SFM_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		SOUNDFILTER* pFilter = debug_new(SOUNDFILTER);

		if (pFilter && pFilter->Init(m_buteMgr, s_aTagName))
		{
			pFilter->nId = nNum;
			m_FilterList.AddTail(pFilter);
		}
		else
		{
			debug_delete(pFilter);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", SFM_TAG, nNum);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFilterMgr::GetFilter
//
//	PURPOSE:	Get the specified sound filter record
//
// ----------------------------------------------------------------------- //

SOUNDFILTER* CSoundFilterMgr::GetFilter(uint8 nId)
{
    SOUNDFILTER** pCur  = LTNULL;

	pCur = m_FilterList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nId)
		{
			return *pCur;
		}

		pCur = m_FilterList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFilterMgr::GetFilter
//
//	PURPOSE:	Get the specified sound filter record
//
// ----------------------------------------------------------------------- //

SOUNDFILTER* CSoundFilterMgr::GetFilter(char* pName)
{
    if (!pName) return LTNULL;

    SOUNDFILTER** pCur  = LTNULL;

	pCur = m_FilterList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_FilterList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFilterMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CSoundFilterMgr::Term()
{
    g_pSoundFilterMgr = LTNULL;

	m_FilterList.Clear();
}



#ifdef _CLIENTBUILD



#endif  // _CLIENTBUILD




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFILTER::SOUNDFILTER
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SOUNDFILTER::SOUNDFILTER()
{
	nId				= 0;
	szName[0]		= '\0';
	szFilterName[0]	= '\0';

	for (int i=0; i < SFM_MAX_VARIABLES; i++)
	{
		szVars[i][0] = '\0';
		fValues[i] = 0.0f;
	}

	nNumVars		= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFILTER::Init
//
//	PURPOSE:	Build the surface struct
//
// ----------------------------------------------------------------------- //

LTBOOL SOUNDFILTER::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, SFM_NAME, szName, ARRAY_LEN(szName));
	buteMgr.GetString(aTagName, SFM_FILTERNAME, szFilterName, ARRAY_LEN(szFilterName));

    int i;
    for (i=0; i < SFM_MAX_VARIABLES; i++)
	{
		sprintf(s_aAttName, "%s%d", SFM_VARIABLE, i);
		buteMgr.GetString(aTagName, s_aAttName, "", szVars[i], ARRAY_LEN(szVars[i]));
		if (buteMgr.Success( ))
		{
			nNumVars++;
		}
	}

	int nNumValues = 0;
	for (i=0; i < SFM_MAX_VARIABLES; i++)
	{
		sprintf(s_aAttName, "%s%d", SFM_VALUE, i);
		fValues[i] = (LTFLOAT) buteMgr.GetDouble(aTagName, s_aAttName, 0.0 );
		if (buteMgr.Success( ))
		{
			nNumValues++;
		}
	}

	if (nNumVars != nNumValues)
	{
		_ASSERT(LTFALSE);
		return LTFALSE;
	}

    return LTTRUE;
}



#ifndef _CLIENTBUILD
#ifndef __PSX2

////////////////////////////////////////////////////////////////////////////
//
// CSoundFilterMgrPlugin is used to help facilitate populating the DEdit
// object properties that use CSoundFilterMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFilterMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CSoundFilterMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pSoundFilterMgr)
	{
		// This will set the g_pSoundFilterMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, SFM_DEFAULT_FILE);
        sm_SoundFilterMgr.SetInRezFile(LTFALSE);
        sm_SoundFilterMgr.Init(szFile);
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSoundFilterMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CSoundFilterMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	_ASSERT(aszStrings && pcStrings && g_pSoundFilterMgr);
    if (!aszStrings || !pcStrings || !g_pSoundFilterMgr) return LTFALSE;

	// Add an entry for each SOUNDFILTER type

	int nNumFilters = g_pSoundFilterMgr->GetNumFilters();
	_ASSERT(nNumFilters > 0);

    SOUNDFILTER* pSF = LTNULL;

	for (int i=0; i < nNumFilters; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pSF = g_pSoundFilterMgr->GetFilter(i);
		if (pSF && pSF->szName[0])
		{
            uint32 dwFilterNameLen = strlen(pSF->szName);

			if (dwFilterNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pSF->szName);
			}
		}
	}

    return LTTRUE;
}

#endif // !__PSX2
#endif // #ifndef _CLIENTBUILD