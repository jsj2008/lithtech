// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisMgr.cpp
//
// PURPOSE : DebrisMgr - Implementation
//
// CREATED : 3/17/2000
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
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
#define DEBRISMGR_DEBRIS_RENDERSTYLE			"RenderStyle"
#define DEBRISMGR_DEBRIS_MINWORLDVEL			"MinWorldVel"
#define DEBRISMGR_DEBRIS_MAXWORLDVEL			"MaxWorldVel"
#define DEBRISMGR_DEBRIS_WORLDSPACEFX			"WorldSpaceFX"
#define DEBRISMGR_DEBRIS_IMPACTSPACEFX			"ImpactSpaceFX"

static char s_aTagName[30];
static char s_aAttName[100];

// Global pointer to surface mgr...

CDebrisMgr* g_pDebrisMgr = LTNULL;


#ifndef _CLIENTBUILD

// Plugin statics
#ifndef __PSX2
CDebrisMgr CDebrisMgrPlugin::sm_DebrisMgr;
#endif

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

LTBOOL CDebrisMgr::Init(const char* szAttributeFile)
{
    if (g_pDebrisMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

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

DEBRIS* CDebrisMgr::GetDebris(char const* pName)
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



#if defined(_CLIENTBUILD) || defined(__PSX2)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisMgr::CreateDebris
//
//	PURPOSE:	Create the specified debris object
//
// ----------------------------------------------------------------------- //

HOBJECT CDebrisMgr::CreateDebris(uint8 nDebrisId, const LTVector &vPos, uint8 nNumber)
{
	DEBRIS* pDebris = GetDebris(nDebrisId);
    if (!pDebris) return LTNULL;

	if (pDebris->nNumModels < 1 || !pDebris->szModel[0][0] || nNumber < 1) return LTNULL;

    LTVector vScale(1.0f, 1.0f, 1.0f);
	vScale *= GetRandom(pDebris->fMinScale, pDebris->fMaxScale);

	// Determine the debris model to use, if nNumber is greater than the maximum
	// number of models, wrap around to a valid index
	// 
	int nMaxIndex = pDebris->nNumModels - 1;
	int nIndex = nNumber-1;
	
	while (nIndex > nMaxIndex)
	{
		nIndex -= pDebris->nNumModels;
	}

	char* pFilename = pDebris->szModel[nIndex];

    if (!pFilename) return LTNULL;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	createStruct.m_Flags = FLAG_VISIBLE; // | FLAG_NOLIGHT;
	createStruct.m_Pos = vPos;

	pDebris->blrSkinReader.CopyList(0, createStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	pDebris->blrRenderStyleReader.CopyList(0, createStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

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


#endif  // _CLIENTBUILD || __PSX2




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DEBRIS::DEBRIS
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DEBRIS::DEBRIS()
{
	nId				= DEBRISMGR_INVALID_ID;
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
	vMinWorldVel = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MINWORLDVEL);
	vMaxWorldVel = buteMgr.GetVector(aTagName, DEBRISMGR_DEBRIS_MAXWORLDVEL);

	nNumber		 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_NUMBER);
	nMinBounce	 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_MINBOUNCE);
	nMaxBounce	 = buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_MAXBOUNCE);

	//load in the impact effect
	buteMgr.GetString(aTagName, DEBRISMGR_DEBRIS_WORLDSPACEFX, szWorldSpaceFX, sizeof(szWorldSpaceFX));
	buteMgr.GetString(aTagName, DEBRISMGR_DEBRIS_IMPACTSPACEFX, szImpactSpaceFX, sizeof(szImpactSpaceFX));

	eSurfaceType = (SurfaceType) buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_SURFACEID);
    bRotate      = (LTBOOL) buteMgr.GetInt(aTagName, DEBRISMGR_DEBRIS_ROTATE);

	buteMgr.GetString(aTagName, DEBRISMGR_DEBRIS_NAME, "" ,szName, ARRAY_LEN(szName));

	char szStr[128]="";
    int i;
    for (i=1; i <= DEBRIS_MAX_MODELS; i++)
	{
		sprintf(s_aAttName, "%s%d", DEBRISMGR_DEBRIS_MODEL, i);
		buteMgr.GetString(aTagName, s_aAttName,"",szStr,sizeof(szStr));

		if (strlen(szStr))
		{
			strncpy(szModel[i-1], szStr, ARRAY_LEN(szModel[i-1]));
			nNumModels++;
		}
	}

	blrSkinReader.Read(&buteMgr, s_aTagName, DEBRISMGR_DEBRIS_SKIN, DEBRIS_MAX_FILE_PATH);
	blrRenderStyleReader.Read(&buteMgr, s_aTagName, DEBRISMGR_DEBRIS_RENDERSTYLE, DEBRIS_MAX_FILE_PATH);

	for (i=1; i <= DEBRIS_MAX_BOUNCE_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", DEBRISMGR_DEBRIS_BOUNCESND, i);
		buteMgr.GetString(aTagName, s_aAttName,"",szStr,sizeof(szStr));

		if (strlen(szStr))
		{
			strncpy(szBounceSnd[i-1], szStr, ARRAY_LEN(szBounceSnd[i-1]));
			nNumBounceSnds++;
		}
	}

	for (i=1; i <= DEBRIS_MAX_EXPLODE_SNDS; i++)
	{
		sprintf(s_aAttName, "%s%d", DEBRISMGR_DEBRIS_EXPLODESND, i);
		buteMgr.GetString(aTagName, s_aAttName,"",szStr,sizeof(szStr));

		if (strlen(szStr))
		{
			strncpy(szExplodeSnd[i-1], szStr, ARRAY_LEN(szExplodeSnd[i-1]));
			nNumExplodeSnds++;
		}
	}


    return LTTRUE;
}

#ifndef _CLIENTBUILD
#ifndef __PSX2

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
        sm_DebrisMgr.Init(szFile);
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

#endif // !__PSX2

#endif // #ifndef _CLIENTBUILD