// ----------------------------------------------------------------------- //
//
// MODULE  : ModelButeMgr.cpp
//
// PURPOSE : ModelButeMgr implementation - Controls attributes of all ModelButes
//
// CREATED : 12/02/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ModelButeMgr.h"
#include "CommonUtilities.h"

// Globals/statics

CModelButeMgr* g_pModelButeMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];
static char s_szBuffer[1024];

// Defines

#define	MODELBMGR_MODEL					"Model"

	#define	MODELBMGR_MODEL_NAME								"Name"
	#define	MODELBMGR_MODEL_SEX									"Sex"
	#define	MODELBMGR_MODEL_NATIONALITY							"Nationality"
	#define	MODELBMGR_MODEL_SKELETON							"Skeleton"
	#define	MODELBMGR_MODEL_TYPE								"Type"
	#define MODELBMGR_MODEL_ENVIRONMENTMAP						"EnvironmentMap"
	#define	MODELBMGR_MODEL_MASS								"Mass"
	#define	MODELBMGR_MODEL_HIT_POINTS							"HitPoints"
	#define	MODELBMGR_MODEL_MAX_HIT_POINTS						"MaxHitPoints"
	#define	MODELBMGR_MODEL_ARMOR								"Armor"
	#define	MODELBMGR_MODEL_MAX_ARMOR							"MaxArmor"

#define	MODELBMGR_STYLE					"Style"

	#define	MODELBMGR_STYLE_NAME								"Name"

#define MODELBMGR_SKELETON				"Skeleton"

	#define MODELBMGR_SKELETON_DEFAULT_FRONT_DEATH_ANI			"DefaultFrontDeathAni"
	#define MODELBMGR_SKELETON_DEFAULT_BACK_DEATH_ANI			"DefaultBackDeathAni"
	#define MODELBMGR_SKELETON_DEFAULT_FRONT_LONGRECOIL_ANI		"DefaultFrontLongRecoilAni"
	#define MODELBMGR_SKELETON_DEFAULT_BACK_LONGRECOIL_ANI		"DefaultBackLongRecoilAni"
	#define MODELBMGR_SKELETON_DEFAULT_FRONT_SHORTRECOIL_ANI	"DefaultFrontShortRecoilAni"
	#define MODELBMGR_SKELETON_DEFAULT_BACK_SHORTRECOIL_ANI		"DefaultBackShortRecoilAni"
	#define MODELBMGR_SKELETON_DEFAULT_HIT_NODE					"DefaultHitNode"

	#define MODELBMGR_SKELETON_NODE_NAME						"Node%dName"
	#define MODELBMGR_SKELETON_NODE_FLAGS						"Node%dFlags"
	#define MODELBMGR_SKELETON_NODE_FRONT_DEATH_ANI				"Node%dFrontDeathAni"
	#define MODELBMGR_SKELETON_NODE_BACK_DEATH_ANI				"Node%dBackDeathAni"
	#define MODELBMGR_SKELETON_NODE_FRONT_LONGRECOIL_ANI		"Node%dFrontLongRecoilAni"
	#define MODELBMGR_SKELETON_NODE_BACK_LONGRECOIL_ANI			"Node%dBackLongRecoilAni"
	#define MODELBMGR_SKELETON_NODE_FRONT_SHORTRECOIL_ANI		"Node%dFrontShortRecoilAni"
	#define MODELBMGR_SKELETON_NODE_BACK_SHORTRECOIL_ANI		"Node%dBackShortRecoilAni"
	#define MODELBMGR_SKELETON_NODE_DAMAGE_FACTOR				"Node%dDamageFactor"
	#define MODELBMGR_SKELETON_NODE_PARENT						"Node%dParent"
	#define MODELBMGR_SKELETON_NODE_RECOILPARENT				"Node%dRecoilParent"
	#define MODELBMGR_SKELETON_NODE_LOCATION					"Node%dLocation"
	#define MODELBMGR_SKELETON_NODE_RADIUS						"Node%dRadius"
	#define MODELBMGR_SKELETON_NODE_PRIORITY					"Node%dPriority"

#define MODELBMGR_NODE_SCRIPT			"NodeScript"

	#define MODELBMGR_NSCRIPT_NODE_NAME							"NodeName"
	#define MODELBMGR_NSCRIPT_FLAGS								"Flags"

	#define MODELBMGR_NSCRIPT_NODE_TIME							"Pt%dTime"
	#define MODELBMGR_NSCRIPT_NODE_POSITION_OFFSET				"Pt%dPosOffset"
	#define MODELBMGR_NSCRIPT_NODE_ROTATION_OFFSET				"Pt%dRotOffset"

#ifndef _CLIENTBUILD

// Plugin statics
LTBOOL CModelButeMgrPlugin::sm_bInitted = LTFALSE;
CModelButeMgr CModelButeMgrPlugin::sm_ButeMgr;

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::CModelButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CModelButeMgr::CModelButeMgr()
{
	m_cModels = 0;
    m_aModels = LTNULL;

	m_cStyles = 0;
    m_aStyles = LTNULL;

	m_cSkeletons = 0;
    m_aSkeletons = LTNULL;

	m_cNScripts = 0;
    m_aNScripts = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::~CModelButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CModelButeMgr::~CModelButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CModelButeMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pModelButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;

	// Set up global pointer

	g_pModelButeMgr = this;

	// Count the node groups

	m_cSkeletons = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, m_cSkeletons);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cSkeletons++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, m_cSkeletons);
	}

	m_aSkeletons = debug_newa(CModelButeMgr::CSkeleton, m_cSkeletons);

	// Read in the nodegroups

	for ( int iSkeleton = 0 ; iSkeleton < m_cSkeletons ; iSkeleton++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_SKELETON, iSkeleton);

		// Get the skeleton's default death/long recoil anis

		strcpy(m_aSkeletons[iSkeleton].m_szDefaultFrontDeathAni, m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_FRONT_DEATH_ANI));
		strcpy(m_aSkeletons[iSkeleton].m_szDefaultBackDeathAni, m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_BACK_DEATH_ANI));

		strcpy(m_aSkeletons[iSkeleton].m_szDefaultFrontLongRecoilAni, m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_FRONT_LONGRECOIL_ANI));
		strcpy(m_aSkeletons[iSkeleton].m_szDefaultBackLongRecoilAni, m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_BACK_LONGRECOIL_ANI));

		strcpy(m_aSkeletons[iSkeleton].m_szDefaultFrontShortRecoilAni, m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_FRONT_SHORTRECOIL_ANI));
		strcpy(m_aSkeletons[iSkeleton].m_szDefaultBackShortRecoilAni, m_buteMgr.GetString(s_aTagName, MODELBMGR_SKELETON_DEFAULT_BACK_SHORTRECOIL_ANI));

		// Get the skeleton's default hit node

        m_aSkeletons[iSkeleton].m_eModelNodeDefaultHit = (ModelNode)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_SKELETON_DEFAULT_HIT_NODE);

		// Count the number of nodes

		m_aSkeletons[iSkeleton].m_cNodes = 0;
		sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, m_aSkeletons[iSkeleton].m_cNodes);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			m_aSkeletons[iSkeleton].m_cNodes++;
			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, m_aSkeletons[iSkeleton].m_cNodes);
		}

		// Get all of our nodes

		m_aSkeletons[iSkeleton].m_aNodes = debug_newa(CModelButeMgr::CNode, m_aSkeletons[iSkeleton].m_cNodes);

		for ( int iNode = 0 ; iNode < m_aSkeletons[iSkeleton].m_cNodes ; iNode++ )
		{
			// Get the node's name

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_NAME, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szName, m_buteMgr.GetString(s_aTagName, s_aAttName));

			// Get the node's special flags

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_FLAGS, iNode);
			m_aSkeletons[iSkeleton].m_aNodes[iNode].m_dwFlags = m_buteMgr.GetDword(s_aTagName, s_aAttName);

			// Get the node's death/long recoil animations

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_FRONT_DEATH_ANI, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szFrontDeathAni, m_buteMgr.GetString(s_aTagName, s_aAttName));

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_BACK_DEATH_ANI, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szBackDeathAni, m_buteMgr.GetString(s_aTagName, s_aAttName));

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_FRONT_LONGRECOIL_ANI, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szFrontLongRecoilAni, m_buteMgr.GetString(s_aTagName, s_aAttName));

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_BACK_LONGRECOIL_ANI, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szBackLongRecoilAni, m_buteMgr.GetString(s_aTagName, s_aAttName));

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_FRONT_SHORTRECOIL_ANI, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szFrontShortRecoilAni, m_buteMgr.GetString(s_aTagName, s_aAttName));

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_BACK_SHORTRECOIL_ANI, iNode);
			strcpy(m_aSkeletons[iSkeleton].m_aNodes[iNode].m_szBackShortRecoilAni, m_buteMgr.GetString(s_aTagName, s_aAttName));

			// Get the node's damage factor

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_DAMAGE_FACTOR, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fDamageFactor = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's parent

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_PARENT, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_eModelNodeParent = (ModelNode)(uint8)m_buteMgr.GetInt(s_aTagName, s_aAttName);

			// Get the node's recoil parent

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_RECOILPARENT, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_eModelNodeRecoilParent = (ModelNode)(uint8)m_buteMgr.GetInt(s_aTagName, s_aAttName);

			// Get the node's hit location

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_LOCATION, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_eHitLocation = (HitLocation)(uint8)m_buteMgr.GetInt(s_aTagName, s_aAttName, HL_UNKNOWN);

			// Get the node's hit radius

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_RADIUS, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fHitRadius = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Get the node's hit radius

			sprintf(s_aAttName, MODELBMGR_SKELETON_NODE_PRIORITY, iNode);
            m_aSkeletons[iSkeleton].m_aNodes[iNode].m_fHitPriority = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);
		}
	}

	// Count the scripts

	m_cNScripts = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, m_cNScripts);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cNScripts++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, m_cNScripts);
	}

	m_aNScripts = debug_newa(CModelButeMgr::CNScript, m_cNScripts);

	// Read in the scripts

	for ( int iNScript = 0 ; iNScript < m_cNScripts ; iNScript++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_NODE_SCRIPT, iNScript);

		// Get the node's name
		strcpy(m_aNScripts[iNScript].m_szName, m_buteMgr.GetString(s_aTagName, MODELBMGR_NSCRIPT_NODE_NAME));

		// Get the scripts flags
        m_aNScripts[iNScript].m_bFlags = (uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_NSCRIPT_FLAGS);

		// Count the number of script sections
		m_aNScripts[iNScript].m_cNScriptPts = 0;
		sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, m_aNScripts[iNScript].m_cNScriptPts);

		while (m_buteMgr.Exist(s_aTagName, s_aAttName))
		{
			m_aNScripts[iNScript].m_cNScriptPts++;
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, m_aNScripts[iNScript].m_cNScriptPts);
		}

		// Get all of our script sections
		m_aNScripts[iNScript].m_aNScriptPts = debug_newa(CModelButeMgr::CNScriptPt, m_aNScripts[iNScript].m_cNScriptPts);

		for ( int iNScriptPt = 0 ; iNScriptPt < m_aNScripts[iNScript].m_cNScriptPts ; iNScriptPt++ )
		{
			// Get the node's time
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_TIME, iNScriptPt);
            m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, s_aAttName);

			// Each time must be greater than the previous... and the first one must be zero
			if(iNScriptPt > 0)
				_ASSERT(m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime > m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt - 1].m_fTime);
			else
				_ASSERT(m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_fTime == 0.0f);

			// Get the node's offset position
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_POSITION_OFFSET, iNScriptPt);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vPosOffset = m_buteMgr.GetVector(s_aTagName, s_aAttName);

			// Get the node's offset rotation
			sprintf(s_aAttName, MODELBMGR_NSCRIPT_NODE_ROTATION_OFFSET, iNScriptPt);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset = m_buteMgr.GetVector(s_aTagName, s_aAttName);

			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.x *= (MATH_PI / 180.0f);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.y *= (MATH_PI / 180.0f);
			m_aNScripts[iNScript].m_aNScriptPts[iNScriptPt].m_vRotOffset.z *= (MATH_PI / 180.0f);
		}
	}

	// Count the styles

	m_cStyles = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_STYLE, m_cStyles);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cStyles++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_STYLE, m_cStyles);
	}

	m_aStyles = debug_newa(CModelButeMgr::CStyle, m_cStyles);

	// Read in the styles

	for ( int iStyle = 0 ; iStyle < m_cStyles ; iStyle++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_STYLE, iStyle);

		strcpy(m_aStyles[iStyle].m_szName, m_buteMgr.GetString(s_aTagName, MODELBMGR_STYLE_NAME));
	}

	// Count the models

	m_cModels = 0;
	sprintf(s_aTagName, "%s%d", MODELBMGR_MODEL, m_cModels);

	while (m_buteMgr.Exist(s_aTagName))
	{
		m_cModels++;
		sprintf(s_aTagName, "%s%d", MODELBMGR_MODEL, m_cModels);
	}

	m_aModels = debug_newa(CModelButeMgr::CModel, m_cModels);

	// Read in the models

	for ( int iModel = 0 ; iModel < m_cModels ; iModel++ )
	{
		sprintf(s_aTagName, "%s%d", MODELBMGR_MODEL, iModel);

		strcpy(m_aModels[iModel].m_szName, m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_NAME));
		strcpy(m_aModels[iModel].m_szSex, m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_SEX));
		strcpy(m_aModels[iModel].m_szNationality, m_buteMgr.GetString(s_aTagName, MODELBMGR_MODEL_NATIONALITY));
        m_aModels[iModel].m_eModelSkeleton = (ModelSkeleton)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_MODEL_SKELETON);
        m_aModels[iModel].m_eModelType = (ModelType)(uint8)m_buteMgr.GetInt(s_aTagName, MODELBMGR_MODEL_TYPE);
        m_aModels[iModel].m_bModelEnvironmentMap = (LTBOOL)m_buteMgr.GetBool(s_aTagName, MODELBMGR_MODEL_ENVIRONMENTMAP);
        m_aModels[iModel].m_fModelMass = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_MASS);
        m_aModels[iModel].m_fModelHitPoints = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_HIT_POINTS);
        m_aModels[iModel].m_fModelMaxHitPoints = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_MAX_HIT_POINTS);
        m_aModels[iModel].m_fModelArmor = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_ARMOR);
        m_aModels[iModel].m_fModelMaxArmor = (LTFLOAT)m_buteMgr.GetDouble(s_aTagName, MODELBMGR_MODEL_MAX_ARMOR);
	}

	// Free up butemgr's memory and what-not.

	m_buteMgr.Term();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CModelButeMgr::Term()
{
	if (m_aModels)
	{
		debug_deletea(m_aModels);
        m_aModels = LTNULL;
	}

	if (m_aStyles)
	{
		debug_deletea(m_aStyles);
        m_aStyles = LTNULL;
	}

	if (m_aSkeletons)
	{
		for ( int iSkeleton = 0 ; iSkeleton < m_cSkeletons ; iSkeleton++ )
		{
			debug_deletea(m_aSkeletons[iSkeleton].m_aNodes);
		}

		debug_deletea(m_aSkeletons);
        m_aSkeletons = LTNULL;
	}

	if (m_aNScripts)
	{
		for ( int iNScript = 0 ; iNScript < m_cNScripts ; iNScript++ )
		{
			debug_deletea(m_aNScripts[iNScript].m_aNScriptPts);
		}

		debug_deletea(m_aNScripts);
        m_aNScripts = LTNULL;
	}

    g_pModelButeMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetModel...
//
//	PURPOSE:	Various model attribute lookups
//
// ----------------------------------------------------------------------- //

ModelId CModelButeMgr::GetModelId(const char *szName)
{
	_ASSERT(szName);

    int iModel;
    for ( iModel = 0 ; iModel < m_cModels ; iModel++ )
	{
        if ( !_stricmp(szName, m_aModels[iModel].m_szName) )
		{
			return (ModelId)iModel;
		}
	}

	// Couldn't find anything, return any intentionally bogus value

// !!!    pInterface->CPrint("CModelButeMgr::GetModel - Could not find Model \"%s\"", szName);
    _ASSERT(LTFALSE);

	return (ModelId)iModel;
}

const char* CModelButeMgr::GetModelName(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_szName;
}

const char* CModelButeMgr::GetModelSex(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_szSex;
}

const char* CModelButeMgr::GetModelNationality(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_szNationality;
}

ModelType CModelButeMgr::GetModelType(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_eModelType;
}

LTBOOL CModelButeMgr::GetModelEnvironmentMap(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_bModelEnvironmentMap;
}

ModelSkeleton CModelButeMgr::GetModelSkeleton(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_eModelSkeleton;
}

LTFLOAT CModelButeMgr::GetModelMass(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_fModelMass;
}

LTFLOAT CModelButeMgr::GetModelHitPoints(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_fModelHitPoints;
}

LTFLOAT CModelButeMgr::GetModelMaxHitPoints(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_fModelMaxHitPoints;
}

LTFLOAT CModelButeMgr::GetModelArmor(ModelId eModelId)

{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_fModelArmor;
}

LTFLOAT CModelButeMgr::GetModelMaxArmor(ModelId eModelId)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);

	return m_aModels[eModelId].m_fModelMaxArmor;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetStyle...
//
//	PURPOSE:	Various style attribute lookups
//
// ----------------------------------------------------------------------- //

ModelStyle CModelButeMgr::GetStyle(const char *szName)
{
	_ASSERT(szName);

    int iStyle;
    for ( iStyle = 0 ; iStyle < m_cStyles ; iStyle++ )
	{
        if ( !_stricmp(szName, m_aStyles[iStyle].m_szName) )
		{
			return (ModelStyle)iStyle;
		}
	}

	// Couldn't find anything, return any intentionally bogus value

// !!!    pInterface->CPrint("CModelButeMgr::GetStyle - Could not find Style \"%s\"", szName);
    _ASSERT(LTFALSE);

	return (ModelStyle)iStyle;
}

const char*	CModelButeMgr::GetStyleName(ModelStyle eStyle)
{
	return m_aStyles[eStyle].m_szName;
}

#ifndef _CLIENTBUILD

ModelStyle CModelButeMgr::GetModelStyleFromProperty(char* pPropVal)
{
	if (!pPropVal || !pPropVal[0]) return (ModelStyle)0;

	for ( int iStyle = 0 ; iStyle < m_cStyles ; iStyle++ )
	{
        LTBOOL bStyle = LTFALSE;

		if (_stricmp(m_aStyles[iStyle].m_szName, pPropVal) == 0)
		{
			return (ModelStyle)iStyle;
		}
	}

	// We don't assert if we can't find a style because this AI may have been spawned in

	return (ModelStyle)0;
}

#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetSkeleton...
//
//	PURPOSE:	Various skeleton attribute lookups
//
// ----------------------------------------------------------------------- //

int CModelButeMgr::GetSkeletonNumNodes(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0;

	return m_aSkeletons[eModelSkeleton].m_cNodes;
}

ModelNode CModelButeMgr::GetSkeletonNode(ModelSkeleton eModelSkeleton, const char* szName)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return eModelNodeInvalid;

	for ( int iNode = 0 ; iNode < m_aSkeletons[eModelSkeleton].m_cNodes ; iNode++ )
	{
        if ( !_stricmp(m_aSkeletons[eModelSkeleton].m_aNodes[iNode].m_szName, szName) )
		{
			return (ModelNode)iNode;
		}
	}

	return eModelNodeInvalid;
}

const char* CModelButeMgr::GetSkeletonDefaultFrontDeathAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultFrontDeathAni;
}

const char* CModelButeMgr::GetSkeletonDefaultBackDeathAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultBackDeathAni;
}

const char* CModelButeMgr::GetSkeletonDefaultFrontLongRecoilAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultFrontLongRecoilAni;
}

const char* CModelButeMgr::GetSkeletonDefaultBackLongRecoilAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultBackLongRecoilAni;
}

const char* CModelButeMgr::GetSkeletonDefaultFrontShortRecoilAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultFrontShortRecoilAni;
}

const char* CModelButeMgr::GetSkeletonDefaultBackShortRecoilAni(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_szDefaultBackShortRecoilAni;
}

ModelNode CModelButeMgr::GetSkeletonDefaultHitNode(ModelSkeleton eModelSkeleton)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return eModelNodeInvalid;

	return m_aSkeletons[eModelSkeleton].m_eModelNodeDefaultHit;
}

const char* CModelButeMgr::GetSkeletonNodeName(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szName;
}

uint32 CModelButeMgr::GetSkeletonNodeFlags(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_dwFlags;
}

const char* CModelButeMgr::GetSkeletonNodeFrontDeathAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szFrontDeathAni;
}

const char* CModelButeMgr::GetSkeletonNodeBackDeathAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szBackDeathAni;
}

const char* CModelButeMgr::GetSkeletonNodeFrontLongRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szFrontLongRecoilAni;
}

const char* CModelButeMgr::GetSkeletonNodeBackLongRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szBackLongRecoilAni;
}

const char* CModelButeMgr::GetSkeletonNodeFrontShortRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szFrontShortRecoilAni;
}

const char* CModelButeMgr::GetSkeletonNodeBackShortRecoilAni(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	if (eModelSkeletonInvalid == eModelSkeleton) return LTNULL;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_szBackShortRecoilAni;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeDamageFactor(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fDamageFactor;
}

ModelNode CModelButeMgr::GetSkeletonNodeParent(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return eModelNodeInvalid;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eModelNodeParent;
}

ModelNode CModelButeMgr::GetSkeletonNodeRecoilParent(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return eModelNodeInvalid;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eModelNodeRecoilParent;
}

HitLocation CModelButeMgr::GetSkeletonNodeLocation(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return HL_UNKNOWN;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_eHitLocation;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeHitRadius(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fHitRadius;
}

LTFLOAT CModelButeMgr::GetSkeletonNodeHitPriority(ModelSkeleton eModelSkeleton, ModelNode eModelNode)
{
	_ASSERT(eModelSkeleton >= 0 && eModelSkeleton < m_cSkeletons);
	_ASSERT(eModelNode >= 0 && eModelNode < m_aSkeletons[eModelSkeleton].m_cNodes);
	if (eModelSkeletonInvalid == eModelSkeleton) return 0.0f;

	return m_aSkeletons[eModelSkeleton].m_aNodes[eModelNode].m_fHitPriority;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::GetNScript...
//
//	PURPOSE:	Various node script attribute lookups
//
// ----------------------------------------------------------------------- //

int CModelButeMgr::GetNScriptNumPts(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);

	return m_aNScripts[eModelNScript].m_cNScriptPts;
}

const char* CModelButeMgr::GetNScriptNodeName(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);

	return m_aNScripts[eModelNScript].m_szName;
}

uint8 CModelButeMgr::GetNScriptFlags(ModelNScript eModelNScript)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);

	return m_aNScripts[eModelNScript].m_bFlags;
}

LTFLOAT CModelButeMgr::GetNScriptPtTime(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);

	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_fTime;
}

const LTVector& CModelButeMgr::GetNScriptPtPosOffset(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);

	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_vPosOffset;
}

const LTVector& CModelButeMgr::GetNScriptPtRotOffset(ModelNScript eModelNScript, int nPt)
{
	_ASSERT(eModelNScript >= 0 && eModelNScript < m_cNScripts);
	_ASSERT(nPt >= 0 && nPt < m_aNScripts[eModelNScript].m_cNScriptPts);

	return m_aNScripts[eModelNScript].m_aNScriptPts[nPt].m_vRotOffset;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgr::Get*Filename
//
//	PURPOSE:	Filename generating functions
//
// ----------------------------------------------------------------------- //

const char* CModelButeMgr::GetModelFilename(ModelId eModelId, ModelStyle eModelStyle, const char* szCinematicExtension /* = LTNULL */)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	_ASSERT(eModelStyle >= 0 && eModelStyle < m_cStyles);

	sprintf(s_szBuffer, "chars\\models\\%s_%s%s.abc", GetModelName(eModelId), GetStyleName(eModelStyle), szCinematicExtension ? szCinematicExtension : "");
	return s_szBuffer;
}

const char* CModelButeMgr::GetBodySkinFilename(ModelId eModelId, ModelStyle eModelStyle, const char* szBodySkinExtension /* = LTNULL */)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	_ASSERT(eModelStyle >= 0 && eModelStyle < m_cStyles);

	sprintf(s_szBuffer, "chars\\skins\\%s_%s%s.dtx", GetModelName(eModelId), GetStyleName(eModelStyle), szBodySkinExtension ? szBodySkinExtension : "");
	return s_szBuffer;
}

const char* CModelButeMgr::GetHeadSkinFilename(ModelId eModelId, ModelStyle eModelStyle, const char* szHeadOverride /* = LTNULL */)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	_ASSERT(eModelStyle >= 0 && eModelStyle < m_cStyles);

	sprintf(s_szBuffer, "chars\\skins\\%s_%s%s.dtx", GetModelName(eModelId), GetStyleName(eModelStyle), szHeadOverride ? szHeadOverride : "_head");
	return s_szBuffer;
}

const char*	CModelButeMgr::GetHandsSkinFilename(ModelStyle eModelStyle)
{
	_ASSERT(eModelStyle >= 0 && eModelStyle < m_cStyles);

	sprintf(s_szBuffer, "guns\\skins_pv\\%sHands_pv.dtx", GetStyleName(eModelStyle));
	return s_szBuffer;
}


const char* CModelButeMgr::GetMultiModelFilename(ModelId eModelId, ModelStyle eModelStyle)
{
	_ASSERT(eModelId >= 0 && eModelId < m_cModels);
	_ASSERT(eModelStyle >= 0 && eModelStyle < m_cStyles);

	sprintf(s_szBuffer, "chars\\models\\multi\\%s_%s_multi.abc", GetModelName(eModelId), GetStyleName(eModelStyle));
	return s_szBuffer;
}



////////////////////////////////////////////////////////////////////////////
//
// CModelButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use WeaponMgr
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENTBUILD  // Server-side only

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Initialize the bute mgr...
//
// ----------------------------------------------------------------------- //

LTRESULT CModelButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	if (!sm_bInitted)
	{
		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, MBMGR_DEFAULT_FILE);
        sm_ButeMgr.SetInRezFile(LTFALSE);
        sm_ButeMgr.Init(g_pLTServer, szFile);
        sm_bInitted = LTTRUE;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModelButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Initialize the bute mgr...
//
// ----------------------------------------------------------------------- //

void CModelButeMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!aszStrings || !pcStrings) return;

	int nNumStyles = sm_ButeMgr.GetNumStyles();

	_ASSERT(cMaxStrings > (*pcStrings) + nNumStyles);

	for (int i=0; i < sm_ButeMgr.GetNumStyles(); i++)
	{
		const char* pStyleName = sm_ButeMgr.GetStyleName((ModelStyle)i);

		if (pStyleName && pStyleName[0])
		{
			_ASSERT(cMaxStringLength > strlen(pStyleName));

			if ((cMaxStrings > (*pcStrings) + 1) &&
				(cMaxStringLength > strlen(pStyleName)))
			{
				strcpy(aszStrings[(*pcStrings)++], pStyleName);
			}
		}
	}
}

#endif // _CLIENTBUILD