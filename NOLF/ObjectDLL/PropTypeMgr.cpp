// ----------------------------------------------------------------------- //
//
// MODULE  : PropTypeMgr.cpp
//
// PURPOSE : PropTypeMgr - Implementation
//
// CREATED : 4/27/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PropTypeMgr.h"
#include "CommonUtilities.h"
#include "DebrisMgr.h"

#define PTMGR_TAG					"PropType"

#define PTMGR_TYPE					"Type"
#define PTMGR_SCALE					"Scale"
#define PTMGR_OBJECTCOLOR			"ObjectColor"
#define PTMGR_ALPHA					"Alpha"
#define PTMGR_VISIBLE				"Visible"
#define PTMGR_SOLID					"Solid"
#define PTMGR_SHADOW				"Shadow"
#define PTMGR_GRAVITY				"Gravity"
#define PTMGR_MOVETOFLOOR			"MoveToFloor"
#define PTMGR_DETAILTEXTURE			"DetailTexture"
#define PTMGR_CHROME				"Chrome"
#define PTMGR_CHROMAKEY				"Chromakey"
#define PTMGR_ADDITIVE				"Additive"
#define PTMGR_MULTIPLY				"Multiply"
#define PTMGR_RAYHIT				"RayHit"
#define PTMGR_TOUCHSOUNDRADIUS		"TouchSoundRadius"
#define PTMGR_HITPOINTS				"HitPoints"
#define PTMGR_FILENAME				"Filename"
#define PTMGR_SKIN					"Skin"
#define PTMGR_TOUCHSOUND			"TouchSound"
#define PTMGR_DEBRISTYPE			"DebrisType"

static char s_aTagName[30];
static char s_aAttName[100];

// Global pointer to surface mgr...

CPropTypeMgr*   g_pPropTypeMgr = LTNULL;

// Plugin statics

CPropTypeMgr CPropTypeMgrPlugin::sm_PropTypeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgr::CPropTypeMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPropTypeMgr::CPropTypeMgr()
{
    m_PropTypeList.Init(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgr::~CPropTypeMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPropTypeMgr::~CPropTypeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CPropTypeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pPropTypeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	g_pPropTypeMgr = this;


	// Read in the properties for each prop type record...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", PTMGR_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PROPTYPE* pPropType = debug_new(PROPTYPE);

		if (pPropType && pPropType->Init(m_buteMgr, s_aTagName))
		{
			pPropType->nId = nNum;
			m_PropTypeList.AddTail(pPropType);
		}
		else
		{
			debug_delete(pPropType);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", PTMGR_TAG, nNum);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgr::GetPropType
//
//	PURPOSE:	Get the specified PropType record
//
// ----------------------------------------------------------------------- //

PROPTYPE* CPropTypeMgr::GetPropType(uint32 nId)
{
    PROPTYPE** pCur  = LTNULL;

	pCur = m_PropTypeList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nId)
		{
			return *pCur;
		}

		pCur = m_PropTypeList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgr::GetPropType
//
//	PURPOSE:	Get the specified PropType record
//
// ----------------------------------------------------------------------- //

PROPTYPE* CPropTypeMgr::GetPropType(char* pType)
{
    if (!pType) return LTNULL;

    PROPTYPE** pCur  = LTNULL;

	pCur = m_PropTypeList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szType[0] && (_stricmp((*pCur)->szType, pType) == 0))
		{
			return *pCur;
		}

		pCur = m_PropTypeList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CPropTypeMgr::Term()
{
    g_pPropTypeMgr = LTNULL;

	m_PropTypeList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgr::CacheAll
//
//	PURPOSE:	Cache all the Prop Type related resources
//
// ----------------------------------------------------------------------- //

void CPropTypeMgr::CacheAll()
{
	// Cache all the PropType data...

    PROPTYPE** pCurPropType  = LTNULL;
	pCurPropType = m_PropTypeList.GetItem(TLIT_FIRST);

	while (pCurPropType)
	{
		if (*pCurPropType)
		{
			(*pCurPropType)->Cache(this);
		}

		pCurPropType = m_PropTypeList.GetItem(TLIT_NEXT);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROPTYPE::PROPTYPE
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PROPTYPE::PROPTYPE()
{
	nId				= 0;

	vScale.Init(1, 1, 1);
	vObjectColor.Init(255, 255, 255);

	fAlpha			= 1.0f;

    bVisible        = LTTRUE;
    bSolid          = LTFALSE;
    bShadow         = LTFALSE;
    bGravity        = LTFALSE;
    bMoveToFloor    = LTTRUE;
    bDetailTexture  = LTFALSE;
    bChrome         = LTFALSE;
    bChromaKey      = LTFALSE;
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
    bRayHit         = LTTRUE;

	nTouchSoundRadius	= 0;
	nHitPoints			= 0;

	szType[0]		= '\0';
	szFilename[0]	= '\0';
	szSkin[0]		= '\0';
	szDebrisType[0]	= '\0';
	szTouchSound[0]	= '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROPTYPE::Init
//
//	PURPOSE:	Build the prop type struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROPTYPE::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	vScale			= buteMgr.GetVector(aTagName, PTMGR_SCALE);
	vObjectColor	= buteMgr.GetVector(aTagName, PTMGR_OBJECTCOLOR);

    fAlpha          = (LTFLOAT) buteMgr.GetDouble(aTagName, PTMGR_ALPHA);

    bVisible        = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_VISIBLE);
    bSolid          = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_SOLID);
    bGravity        = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_GRAVITY);
    bShadow         = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_SHADOW);
    bMoveToFloor    = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_MOVETOFLOOR);
    bDetailTexture  = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_DETAILTEXTURE);
    bChrome         = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_CHROME);
    bChromaKey      = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_CHROMAKEY);
    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_ADDITIVE);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_MULTIPLY);
    bRayHit         = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_RAYHIT);

	nTouchSoundRadius	= buteMgr.GetInt(aTagName, PTMGR_TOUCHSOUNDRADIUS);
	nHitPoints			= buteMgr.GetInt(aTagName, PTMGR_HITPOINTS);

	CString str = buteMgr.GetString(aTagName, PTMGR_TYPE);
	if (!str.IsEmpty())
	{
		strncpy(szType, (char*)(LPCSTR)str, ARRAY_LEN(szType));
	}

	str = buteMgr.GetString(aTagName, PTMGR_FILENAME);
	if (!str.IsEmpty())
	{
		strncpy(szFilename, (char*)(LPCSTR)str, ARRAY_LEN(szFilename));
	}

	str = buteMgr.GetString(aTagName, PTMGR_SKIN);
	if (!str.IsEmpty())
	{
		strncpy(szSkin, (char*)(LPCSTR)str, ARRAY_LEN(szSkin));
	}

	str = buteMgr.GetString(aTagName, PTMGR_TOUCHSOUND);
	if (!str.IsEmpty())
	{
		strncpy(szTouchSound, (char*)(LPCSTR)str, ARRAY_LEN(szTouchSound));
	}

	str = buteMgr.GetString(aTagName, PTMGR_DEBRISTYPE);
	if (!str.IsEmpty())
	{
		strncpy(szDebrisType, (char*)(LPCSTR)str, ARRAY_LEN(szDebrisType));
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROPTYPE::Cache
//
//	PURPOSE:	Cache all the resources associated with the surface
//
// ----------------------------------------------------------------------- //

void PROPTYPE::Cache(CPropTypeMgr* pPropTypeMgr)
{
	if (!pPropTypeMgr) return;

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

	// Cache sounds...

	if (szTouchSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szTouchSound);
	}

	// Cache debris types...

	if (szDebrisType[0])
	{
		DEBRIS* pDebris = g_pDebrisMgr->GetDebris(szDebrisType);
		if (pDebris)
		{
			pDebris->Cache(g_pDebrisMgr);
		}
	}
}



////////////////////////////////////////////////////////////////////////////
//
// CPropTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CPropTypeMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //

LTRESULT CPropTypeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pPropTypeMgr)
	{
		// This will set the g_pPropTypeMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, PTMGR_DEFAULT_FILE);
        sm_PropTypeMgr.SetInRezFile(LTFALSE);
        sm_PropTypeMgr.Init(g_pLTServer, szFile);
	}

	if (!PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength))
	{
		return LT_UNSUPPORTED;
	}

	return LT_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CPropTypeMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each PropType type

	int nNumPropType = g_pPropTypeMgr->GetNumPropTypes();
	_ASSERT(nNumPropType > 0);

    PROPTYPE* pPropType = LTNULL;

	for (int i=0; i < nNumPropType; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pPropType = g_pPropTypeMgr->GetPropType(i);
		if (pPropType && pPropType->szType[0])
		{
            uint32 dwImpactFXNameLen = strlen(pPropType->szType);

			if (dwImpactFXNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pPropType->szType);
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPropTypeMgrPlugin::PreHook_Dims
//
//	PURPOSE:	Determine the dims for this prop
//
// ----------------------------------------------------------------------- //

LTRESULT CPropTypeMgrPlugin::PreHook_Dims(
			const char* szRezPath,
			const char* szPropValue,
			char* szModelFilenameBuf,
			int	  nModelFilenameBufLen,
			LTVector & vDims)
{

	if (!szModelFilenameBuf || nModelFilenameBufLen < 1) return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';

	if (!g_pPropTypeMgr)
	{
		// This will set the g_pPropTypeMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, PTMGR_DEFAULT_FILE);
        sm_PropTypeMgr.SetInRezFile(LTFALSE);
        sm_PropTypeMgr.Init(g_pLTServer, szFile);
	}

	PROPTYPE* pPropType = g_pPropTypeMgr->GetPropType((char*)szPropValue);
	if (!pPropType || !pPropType->szFilename[0])
	{
		return LT_UNSUPPORTED;
	}

	strcpy(szModelFilenameBuf, pPropType->szFilename);

	return LT_OK;
}
