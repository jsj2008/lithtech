// ----------------------------------------------------------------------- //
//
// MODULE  : PropTypeMgr.cpp
//
// PURPOSE : PropTypeMgr - Implementation
//
// CREATED : 4/27/2000
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
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
#define PTMGR_ADDITIVE				"Additive"
#define PTMGR_MULTIPLY				"Multiply"
#define PTMGR_RAYHIT				"RayHit"
#define PTMGR_ACTIVATEABLE			"Activateable"
#define PTMGR_ACTIVATETYPE			"ActivateType"
#define PTMGR_SEARCHABLE			"Searchable"
#define PTMGR_TOUCHABLE				"Touchable"
#define PTMGR_STIMRADIUS			"StimRadius"
#define PTMGR_TOUCHSOUNDRADIUS		"TouchSoundRadius"
#define PTMGR_TOUCHSOUND			"TouchSound"
#define PTMGR_TOUCHALARMLEVEL		"TouchAlarmLevel"
#define PTMGR_DESTROYALARMLEVEL		"DestroyAlarmLevel"
#define PTMGR_DESTROYFILENAME		"DestroyFilename"
#define PTMGR_DESTROYSKIN			"DestroySkin"
#define PTMGR_DESTROYRENDERSTYLE	"DestroyRenderStyle"
#define PTMGR_HITSOUND				"HitSound"
#define	PTMGR_HITSOUNDRADIUS		"HitSoundRadius"
#define PTMGR_HITALARMLEVEL			"HitAlarmLevel"
#define PTMGR_HITPOINTS				"HitPoints"
#define PTMGR_FILENAME				"Filename"
#define PTMGR_SKIN					"Skin"
#define PTMGR_DEBRISTYPE			"DebrisType"
#define PTMGR_RENDERSTYLE			"RenderStyle"

#define PTMGR_DEFAULT_TOUCHSOUNDRADIUS	0.f
#define PTMGR_DEFAULT_STIMRADIUS		0.f
#define PTMGR_DEFAULT_HITSOUNDRADIUS	0.0f
#define PTMGR_DEFAULT_ALARMLEVEL		0
#define PTMGR_DEFAULT_ACTIVATEABLE		0
#define PTMGR_DEFAULT_SEARCHABLE		0
#define PTMGR_DEFAULT_TOUCHABLE			1

static char s_aTagName[30];
static char s_aAttName[100];
static char s_aBuffer[100];

// Global pointer to surface mgr...

CPropTypeMgr*   g_pPropTypeMgr = LTNULL;

// Plugin statics

#ifndef __PSX2
CPropTypeMgr CPropTypeMgrPlugin::sm_PropTypeMgr;
#endif

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

LTBOOL CPropTypeMgr::Init(const char* szAttributeFile)
{
    if (g_pPropTypeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;

	g_pPropTypeMgr = this;


	// Read in the properties for each prop type record...

	int nNum = 0;
	sprintf(s_aTagName, "%s%d", PTMGR_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PROPTYPE* pPropType = debug_new(PROPTYPE);

		if (pPropType && pPropType->Init(m_buteMgr, s_aTagName, nNum))
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
		if (*pCur && !(*pCur)->sType.empty( ) && (_stricmp((*pCur)->sType.c_str( ), pType) == 0))
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
//	ROUTINE:	PROP_DISTURB::PROP_DISTURB
//
//	PURPOSE:	Con/Destructor
//
// ----------------------------------------------------------------------- //

PROP_DISTURB::PROP_DISTURB()
{
	fStimRadius			= 0.f;

	fTouchSoundRadius	= 0.f;
	nTouchAlarmLevel	= 0;

	nDestroyAlarmLevel	= 0;
	
	nHitAlarmLevel		= 0;
	fHitSoundRadius		= 0.0f;

	nPropTypeId			= -1;
}

PROP_DISTURB::~PROP_DISTURB()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROPTYPE::PROPTYPE
//
//	PURPOSE:	Con/Destructor
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
    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;
    bRayHit         = LTTRUE;
    bActivateable   = LTFALSE;
    bSearchable		= LTFALSE;
    bTouchable		= LTTRUE;

	nHitPoints		= 0;

	pDisturb		= LTNULL;
}

PROPTYPE::~PROPTYPE()
{
	debug_delete(pDisturb);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROPTYPE::Init
//
//	PURPOSE:	Build the prop type struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROPTYPE::Init(CButeMgr & buteMgr, char* aTagName, uint32 nId)
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
    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_ADDITIVE, false);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_MULTIPLY, false);
    bRayHit         = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_RAYHIT);
    bActivateable   = (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_ACTIVATEABLE, PTMGR_DEFAULT_ACTIVATEABLE);
    bSearchable		= (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_SEARCHABLE, PTMGR_DEFAULT_SEARCHABLE);
    bTouchable		= (LTBOOL) buteMgr.GetInt(aTagName, PTMGR_TOUCHABLE, PTMGR_DEFAULT_TOUCHABLE);

	nHitPoints		= buteMgr.GetInt(aTagName, PTMGR_HITPOINTS);

	buteMgr.GetString(aTagName, PTMGR_TYPE, s_aBuffer, ARRAY_LEN(s_aBuffer));
	if(s_aBuffer[0])
	{
		sType = s_aBuffer;
	}

	buteMgr.GetString(aTagName, PTMGR_FILENAME, s_aBuffer, ARRAY_LEN(s_aBuffer));
	if(s_aBuffer[0])
	{
		sFilename = s_aBuffer;
	}


	blrPropSkinReader.Read(&buteMgr, s_aTagName, PTMGR_SKIN, PROPTYPE_MAX_FILE_PATH);
	blrPropRenderStyleReader.Read(&buteMgr, s_aTagName, PTMGR_RENDERSTYLE, PROPTYPE_MAX_FILE_PATH);

	buteMgr.GetString(aTagName, PTMGR_DEBRISTYPE, s_aBuffer, ARRAY_LEN(s_aBuffer));
	if(s_aBuffer[0])
	{
		sDebrisType = s_aBuffer;
	}

	buteMgr.GetString( aTagName, PTMGR_ACTIVATETYPE, "", s_aBuffer, ARRAY_LEN( s_aBuffer ));
	if( s_aBuffer[0] )
	{
		sActivateType = s_aBuffer;
	}


	LTFLOAT fTouchSoundRadius = (LTFLOAT)buteMgr.GetDouble(aTagName, PTMGR_TOUCHSOUNDRADIUS, PTMGR_DEFAULT_TOUCHSOUNDRADIUS);
	LTFLOAT fStimRadius = (LTFLOAT)buteMgr.GetDouble(aTagName, PTMGR_STIMRADIUS, PTMGR_DEFAULT_STIMRADIUS);
	
	char	szHitSound[PROPTYPE_MAX_FILE_PATH] = {0};
	buteMgr.GetString( aTagName, PTMGR_HITSOUND, "", szHitSound, ARRAY_LEN(szHitSound) );

	buteMgr.GetString(aTagName, PTMGR_DESTROYFILENAME, s_aBuffer, ARRAY_LEN(s_aBuffer));
	if( (fTouchSoundRadius > 0.f) || (fStimRadius > 0.f) || (s_aBuffer[0] != '\0') || (szHitSound[0] != '\0') )
	{
		pDisturb = debug_new(PROP_DISTURB);
		pDisturb->nPropTypeId = nId;

		if(s_aBuffer[0])
		{
			pDisturb->sDestroyFilename = s_aBuffer;
		}

		buteMgr.GetString(aTagName, PTMGR_TOUCHSOUND, s_aBuffer, ARRAY_LEN(s_aBuffer));
		if(s_aBuffer[0])
		{
			pDisturb->sTouchSound = s_aBuffer;
		}

		pDisturb->fStimRadius = fStimRadius;

		pDisturb->fTouchSoundRadius = fTouchSoundRadius;
		pDisturb->nTouchAlarmLevel = buteMgr.GetInt(aTagName, PTMGR_TOUCHALARMLEVEL, PTMGR_DEFAULT_ALARMLEVEL);

		pDisturb->nDestroyAlarmLevel = buteMgr.GetInt(aTagName, PTMGR_DESTROYALARMLEVEL, PTMGR_DEFAULT_ALARMLEVEL);

		pDisturb->blrDestroySkinReader.Read(&buteMgr, s_aTagName, PTMGR_DESTROYSKIN, PROPTYPE_MAX_FILE_PATH);
		pDisturb->blrDestroyRenderStyleReader.Read(&buteMgr, s_aTagName, PTMGR_DESTROYRENDERSTYLE, PROPTYPE_MAX_FILE_PATH);

		if( szHitSound[0] )
		{
			pDisturb->sHitSound = szHitSound;
		}

		pDisturb->fHitSoundRadius = (LTFLOAT)buteMgr.GetDouble( aTagName, PTMGR_HITSOUNDRADIUS, PTMGR_DEFAULT_HITSOUNDRADIUS );
		pDisturb->nHitAlarmLevel = buteMgr.GetInt( aTagName, PTMGR_HITALARMLEVEL, PTMGR_DEFAULT_ALARMLEVEL );
	
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROPTYPE::SetupModel
//
//	PURPOSE:	Setup the model based on this prop type
//
// ----------------------------------------------------------------------- //

void PROPTYPE::SetupModel(ObjectCreateStruct* pStruct)
{
	if (!pStruct) return;

	SAFE_STRCPY(pStruct->m_Filename, sFilename.c_str( ));

	// Skins

	blrPropSkinReader.CopyList(0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);

	// Render Styles

	blrPropRenderStyleReader.CopyList(0, pStruct->m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);

	//If no render style was specified, use the default

	if (strlen(pStruct->m_RenderStyleName) == 0)
	{
		strncpy(pStruct->m_RenderStyleName, "rs\\default.ltb", MAX_CS_FILENAME_LEN);
	}
}

////////////////////////////////////////////////////////////////////////////
//
// CPropTypeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use CPropTypeMgr
//
////////////////////////////////////////////////////////////////////////////
#ifndef __PSX2

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
        sm_PropTypeMgr.Init(szFile);
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
		if (pPropType && !pPropType->sType.empty( ))
		{
            uint32 dwImpactFXNameLen = pPropType->sType.length( );

			if (dwImpactFXNameLen < cMaxStringLength && ((*pcStrings) + 1) < cMaxStrings)
			{
				strcpy(aszStrings[(*pcStrings)++], pPropType->sType.c_str( ));
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
        sm_PropTypeMgr.Init(szFile);
	}

	PROPTYPE* pPropType = g_pPropTypeMgr->GetPropType((char*)szPropValue);
	if (!pPropType || pPropType->sFilename.empty( ))
	{
		return LT_UNSUPPORTED;
	}

	strcpy(szModelFilenameBuf, pPropType->sFilename.c_str( ));

	// Need to convert the .ltb filename to one that DEdit understands...
	
	ConvertLTBFilename(szModelFilenameBuf);

	return LT_OK;
}
#endif