// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisMgr.cpp
//
// PURPOSE : DebrisMgr - Implementation
//
// CREATED : 3/17/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebrisMgr.h"
#include "CommonUtilities.h"

#define DEBRISMGR_DEBRIS_TAG					"Debris"

#define DEBRISMGR_DEBRIS_NAME					"Name"
#define DEBRISMGR_DEBRIS_SURFACEID				"SurfaceId"
#define DEBRISMGR_DEBRIS_MINSCALE				"MinScale"
#define DEBRISMGR_DEBRIS_MAXSCALE				"MaxScale"
#define DEBRISMGR_DEBRIS_MINVEL					"MinVel"
#define DEBRISMGR_DEBRIS_MAXVEL					"MaxVel"
#define DEBRISMGR_DEBRIS_MINDOFFSET				"MinDebrisOffset"
#define DEBRISMGR_DEBRIS_MAXDOFFSET				"MaxDebrisOffset"
#define DEBRISMGR_DEBRIS_MINLIFETIME			"MinLifetime"
#define DEBRISMGR_DEBRIS_MAXLIFETIME			"MaxLifetime"
#define DEBRISMGR_DEBRIS_FADETIME				"Fadetime"
#define DEBRISMGR_DEBRIS_NUMBER					"Number"
#define DEBRISMGR_DEBRIS_ROTATE					"Rotate"
#define DEBRISMGR_DEBRIS_MINBOUNCE				"MinBounce"
#define DEBRISMGR_DEBRIS_MAXBOUNCE				"MaxBounce"
#define DEBRISMGR_DEBRIS_GRAVITYSCALE			"GravityScale"
#define DEBRISMGR_DEBRIS_MODEL					"Model"
#define DEBRISMGR_DEBRIS_SKIN					"Skin"
#define DEBRISMGR_DEBRIS_ALPHA					"Alpha"
#define DEBRISMGR_DEBRIS_BOUNCESND				"BounceSnd"
#define DEBRISMGR_DEBRIS_EXPLODESND				"ExplodeSnd"

static char s_aTagName[30];
static char s_aAttName[100];

// Global pointer to surface mgr...

CDebrisMgr* g_pDebrisMgr = LTNULL;


#ifndef _CLIENTBUILD

// Plugin statics
CDebrisMgr CDebrisMgrPlugin::sm_DebrisMgr;

#endif // _CLIENTBUILD


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::CDebrisMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDebrisMgr::CDebrisMgr()
{
    m_DebrisList.Init(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::~CDebrisMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDebrisMgr::~CDebrisMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pDebrisMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	g_pDebrisMgr = this;


	// Read in the properties for each debis record...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", DEBRISMGR_DEBRIS_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		DEBRIS* pDebris = debug_new(DEBRIS);

		if (pDebris && pDebris->Init(m_buteMgr, s_aTagName))
		{
			pDebris->nId = nNum;
			m_DebrisList.AddTail(pDebris);
		}
		else
		{
			debug_delete(pDebris);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", DEBRISMGR_DEBRIS_TAG, nNum);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetDebris
//
//	PURPOSE:	Get the specified debris record
//
// ----------------------------------------------------------------------- //

DEBRIS* CDebrisMgr::GetDebris(uint8 nId)
{
    DEBRIS** pCur  = LTNULL;

	pCur = m_DebrisList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nId)
		{
			return *pCur;
		}

		pCur = m_DebrisList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetDebris
//
//	PURPOSE:	Get the specified debris record
//
// ----------------------------------------------------------------------- //

DEBRIS* CDebrisMgr::GetDebris(char* pName)
{
    if (!pName) return LTNULL;

    DEBRIS** pCur  = LTNULL;

	pCur = m_DebrisList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_DebrisList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CDebrisMgr::Term()
{
    g_pDebrisMgr = LTNULL;

	m_DebrisList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::CacheAll
//
//	PURPOSE:	Cache all the debris related resources
//
// ----------------------------------------------------------------------- //

void CDebrisMgr::CacheAll()
{
#ifndef _CLIENTBUILD  // Server-side only

	// Cache all the debris data...

    DEBRIS** pCurDebris  = LTNULL;
	pCurDebris = m_DebrisList.GetItem(TLIT_FIRST);

	while (pCurDebris)
	{
		if (*pCurDebris)
		{
			(*pCurDebris)->Cache(this);
		}

		pCurDebris = m_DebrisList.GetItem(TLIT_NEXT);
	}

#endif
}


#if defined(_CLIENTBUILD)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::CreateDebris
//
//	PURPOSE:	Create the specified debris object
//
// ----------------------------------------------------------------------- //

HOBJECT CDebrisMgr::CreateDebris(uint8 nDebrisId, LTVector vPos)
{
	DEBRIS* pDebris = GetDebris(nDebrisId);
    if (!pDebris) return LTNULL;

	if (pDebris->nNumModels < 1 || !pDebris->szModel[0][0] ||
        !pDebris->szSkin[0]) return LTNULL;

    LTVector vScale(1.0f, 1.0f, 1.0f);
	vScale *= GetRandom(pDebris->fMinScale, pDebris->fMaxScale);

	int nIndex = GetRandom(0, pDebris->nNumModels - 1);
	char* pFilename = pDebris->szModel[nIndex];

    if (!pFilename) return LTNULL;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	SAFE_STRCPY(createStruct.m_SkinName, pDebris->szSkin);
	createStruct.m_Flags = FLAG_VISIBLE; // | FLAG_NOLIGHT;
	createStruct.m_Pos = vPos;

    HOBJECT hObj = g_pLTClient->CreateObject(&createStruct);

	//Add code to set alpha!!!!
	//TO DO!!!


    g_pLTClient->SetObjectScale(hObj, &vScale);

	return hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetExplodeSound
//
//	PURPOSE:	Get an explode sound
//
// ----------------------------------------------------------------------- //

char* CDebrisMgr::GetExplodeSound(uint8 nDebrisId)
{
	DEBRIS* pDebris = GetDebris(nDebrisId);
    if (!pDebris || !pDebris->szExplodeSnd[0][0]) return LTNULL;

	int nIndex = GetRandom(0, pDebris->nNumExplodeSnds - 1);
	return pDebris->szExplodeSnd[nIndex];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::GetBounceSound
//
//	PURPOSE:	Get a bounce sound
//
// ----------------------------------------------------------------------- //

char* CDebrisMgr::GetBounceSound(uint8 nDebrisId)
{
	DEBRIS* pDebris = GetDebris(nDebrisId);
    if (!pDebris || !pDebris->szBounceSnd[0][0]) return LTNULL;

	int nIndex = GetRandom(0, pDebris->nNumBounceSnds - 1);
	return pDebris->szBounceSnd[nIndex];
}


#endif  // _CLIENTBUILD




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS::DEBRIS
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DEBRIS::DEBRIS()
{
	nId				= 0;
	eSurfaceType	= ST_UNKNOWN;
	fMinScale		= 1.0f;
	fMaxScale		= 1.0f;
	fMinLifetime	= 0.0f;
	fMaxLifetime	= 0.0f;
	fFadetime		= 0.0f;
	fGravityScale	= 1.0f;
	fAlpha			= 1.0f;

	vMinVel.Init();
	vMaxVel.Init();
	vMinDOffset.Init();
	vMaxDOffset.Init();

	nNumber			= 0;
    bRotate         = LTFALSE;

	nNumModels		= 0;
    int i;
    for (i=0; i < DEBRIS_MAX_MODELS; i++)
	{
		szModel[i][0]	= '\0';
	}

	nNumBounceSnds	= 0;
	for (i=0; i < DEBRIS_MAX_BOUNCE_SNDS; i++)
	{
		szBounceSnd[i][0]	= '\0';
	}

	nNumExplodeSnds	= 0;
	for (i=0; i < DEBRIS_MAX_EXPLODE_SNDS; i++)
	{
		szExplodeSnd[i][0]	= '\0';
	}

	szName[0]	= '\0';
	szSkin[0]	= '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS::Init
//
//	PURPOSE:	Build the surface struct
//
// ----------------------------------------------------------------------- //

LTBOOL DEBRIS::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	fMinScale	 = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_MINSCALE);
	fMaxScale	 = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_MAXSCALE);
	fMinLifetime = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_MINLIFETIME);
	fMaxLifetime = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_MAXLIFETIME);
	fFadetime	 = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_FADETIME);
	fGravityScale = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_GRAVITYSCALE);
	fAlpha		  = (float) buteMgr.GetDouble(aTagName, DEBRISMGR_DEBRIS_ALPHA);

	vMinVel		 = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MINVEL);
	vMaxVel		 = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MAXVEL);
	vMinDOffset	 = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MINDOFFSET);
	vMaxDOffset	 = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MAXDOFFSET);

	nNumber		 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_NUMBER);
	nMinBounce	 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_MINBOUNCE);
	nMaxBounce	 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_MAXBOUNCE);

	eSurfaceType = (SurfaceType) buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_SURFACEID);
    bRotate      = (LTBOOL) buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_ROTATE);

	CString str = buteMgr.GetString(aTagName, DEBRISMGR_DEBRIS_NAME);
	if (!str.IsEmpty())
	{
		strncpy(szName, (char*)(LPCSTR)str, ARRAY_LEN(szName));
	}

	str = buteMgr.GetString(aTagName, DEBRISMGR_DEBRIS_SKIN);
	if (!str.IsEmpty())
	{
		strncpy(szSkin, (char*)(LPCSTR)str, ARRAY_LEN(szSkin));
	}

    int i;
    for (i=1; i <= DEBRIS_MAX_MODELS; i++)
	{
		sprintf(s_aAttName, "%s%d", DEBRISMGR_DEBRIS_MODEL, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szModel[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szModel[i-1]));
			nNumModels++;
		}
	}

	for (i=1; i <= DEBRIS_MAX_BOUNCE_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", DEBRISMGR_DEBRIS_BOUNCESND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szBounceSnd[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szBounceSnd[i-1]));
			nNumBounceSnds++;
		}
	}

	for (i=1; i <= DEBRIS_MAX_EXPLODE_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", DEBRISMGR_DEBRIS_EXPLODESND, i);
		str = buteMgr.GetString(aTagName, s_aAttName);

		if (!str.IsEmpty())
		{
			strncpy(szExplodeSnd[i-1], (char*)(LPCSTR)str, ARRAY_LEN(szExplodeSnd[i-1]));
			nNumExplodeSnds++;
		}
	}


    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS::Cache
//
//	PURPOSE:	Cache all the resources associated with the surface
//
// ----------------------------------------------------------------------- //

void DEBRIS::Cache(CDebrisMgr* pDebrisMgr)
{
#ifndef _CLIENTBUILD

	if (!pDebrisMgr) return;

	// Cache skin...

	if (szSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szSkin);
	}

	// Cache models...
    int i;
    for (i=0; i < DEBRIS_MAX_MODELS; i++)
	{
		if (szModel[i][0])
		{
            g_pLTServer->CacheFile(FT_MODEL, szModel[i]);
		}
	}

	// Cache sounds...

	for (i=0; i < DEBRIS_MAX_BOUNCE_SNDS; i++)
	{
		if (szBounceSnd[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szBounceSnd[i]);
		}
	}

	for (i=0; i < DEBRIS_MAX_EXPLODE_SNDS; i++)
	{
		if (szExplodeSnd[i][0])
		{
            g_pLTServer->CacheFile(FT_SOUND, szExplodeSnd[i]);
		}
	}

#endif  // _CLIENTBUILD
}


#ifndef _CLIENTBUILD

////////////////////////////////////////////////////////////////////////////
//
// CDebrisMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CDebrisMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CDebrisMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pDebrisMgr)
	{
		// This will set the g_pDebrisMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, DEBRISMGR_DEFAULT_FILE);
        sm_DebrisMgr.SetInRezFile(LTFALSE);
        sm_DebrisMgr.Init(g_pLTServer, szFile);
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	_ASSERT(aszStrings && pcStrings && g_pDebrisMgr);
    if (!aszStrings || !pcStrings || !g_pDebrisMgr) return LTFALSE;

	// Add an entry for each debris type

	int nNumDebris = g_pDebrisMgr->GetNumDebris();
	_ASSERT(nNumDebris > 0);

    DEBRIS* pDebris = LTNULL;

	for (int i=0; i < nNumDebris; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pDebris = g_pDebrisMgr->GetDebris(i);
		if (pDebris && pDebris->szName[0])
		{
            uint32 dwDebrisNameLen = strlen(pDebris->szName);

			if (dwDebrisNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pDebris->szName);
			}
		}
	}

    return LTTRUE;
}

#endif // #ifndef _CLIENTBUILD