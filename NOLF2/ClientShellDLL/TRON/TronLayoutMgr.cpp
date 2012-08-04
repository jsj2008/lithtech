// ----------------------------------------------------------------------- //
//
// MODULE  : TronLayoutMgr.cpp
//
// PURPOSE : Attribute file manager for interface layout info
//			 Tron-specific functionality
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronLayoutMgr.h"

#define LO_SUB_TAG					"Subroutine"

#define	LO_SUB_NAME					"Name"
#define	LO_SUB_MOD					"Model"
#define	LO_SUB_SKIN					"Skin"
#define	LO_SUB_STYLE				"RenderStyle"

#define LO_MENUPIECE_TAG			"MenuPiece"

#define	LO_MENUPIECE_NAME			"Name"
#define	LO_MENUPIECE_MOD			"Model"
#define	LO_MENUPIECE_SKIN			"Skin"
#define	LO_MENUPIECE_STYLE			"RenderStyle"
#define	LO_MENUPIECE_POS			"Pos"
#define LO_MENUPIECE_LOOPANIM		"LoopAnim"
#define LO_MENUPIECE_FUNCTION		"Function"

#define LO_HUD_TAG					"HUDLayout"
#define LO_HUD_HEALTH_ENERGY_FONT	"HealthEnergyFont"

#define LO_HUD_INFO_POS				"InfoTextPos"
#define LO_HUD_INFO_SZ				"InfoTextSize"
#define LO_HUD_INFO_WD				"InfoTextWidth"
#define LO_HUD_INFO_JUST			"InfoTextJustify"
#define LO_HUD_INFO_ALPHA			"InfoTextAlpha"
#define LO_HUD_INFO_COLOR			"InfoTextColor"
#define LO_HUD_INFO_BAD_ALPHA		"InfoTextBadAlpha"
#define LO_HUD_INFO_BAD_COLOR		"InfoTextBadColor"

#define LO_PROGRESSBOX_TAG			"ProgressBox"
#define LO_PROGRESSBOX_TITLEFONT	"TitleFont"
#define LO_PROGRESSBOX_TITLEFONTSIZE "TitleFontSize"
#define LO_PROGRESSBOX_TITLEALPHA	"TitleAlpha"
#define LO_PROGRESSBOX_TITLECOLOR	"TitleColor"
#define LO_PROGRESSBOX_OBJECTIVEFONT "ObjectiveFont"
#define LO_PROGRESSBOX_OBJECTIVEFONTSIZE "ObjectiveFontSize"
#define LO_PROGRESSBOX_OBJECTIVEALPHA	"ObjectiveAlpha"
#define LO_PROGRESSBOX_OBJECTIVECOLOR	"ObjectiveColor"
#define LO_PROGRESSBOX_COMPLETEDOBJECTIVEALPHA	"CompletedObjectiveAlpha"
#define LO_PROGRESSBOX_COMPLETEDOBJECTIVECOLOR	"CompletedObjectiveColor"
#define LO_PROGRESSBOX_SECONDARYOBJECTIVEALPHA	"SecondaryObjectiveAlpha"
#define LO_PROGRESSBOX_SECONDARYOBJECTIVECOLOR	"SecondaryObjectiveColor"
#define LO_PROGRESSBOX_PROCEDURALFONT "ProceduralFont"
#define LO_PROGRESSBOX_PROCEDURALFONTSIZE "ProceduralFontSize"
#define LO_PROGRESSBOX_PROCEDURALALPHA "ProceduralAlpha"
#define LO_PROGRESSBOX_PROCEDURALCOLOR "ProceduralColor"
#define LO_PROGRESSBOX_PROCEDURALHEIGHT "ProceduralHeight"

static char s_aTagName[30];
static char s_aAttName[30];

CLayoutMgr* g_pLayoutMgr = LTNULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LTBOOL INT_SUBMODEL::Init(CButeMgr & buteMgr, char* aTagName)
{

	buteMgr.GetString(aTagName, LO_SUB_NAME, "", szName, sizeof(szName));
	if (strlen(szName) == 0) return LTFALSE;

	buteMgr.GetString(aTagName, LO_SUB_MOD, "", szModel, sizeof(szModel));
	if (strlen(szModel) == 0) return LTFALSE;

	blrSkins.Read(&buteMgr, aTagName, LO_SUB_SKIN, 128);
	blrRenderStyles.Read(&buteMgr, aTagName, LO_SUB_STYLE, 128);

	return LTTRUE;
}

LTBOOL INT_MENUPIECE::Init(CButeMgr & buteMgr, char* aTagName)
{

	buteMgr.GetString(aTagName, LO_MENUPIECE_NAME, "", szName, sizeof(szName));
	if (strlen(szName) == 0) return LTFALSE;

	buteMgr.GetString(aTagName, LO_MENUPIECE_MOD, "", szModel, sizeof(szModel));
	if (strlen(szModel) == 0) return LTFALSE;

	buteMgr.GetString(aTagName, LO_MENUPIECE_LOOPANIM, "", szLoopAnim, sizeof(szLoopAnim));
//	if (strlen(szLoopAnim) == 0) return LTFALSE;

	blrSkins.Read(&buteMgr, aTagName, LO_MENUPIECE_SKIN, 128);
	blrRenderStyles.Read(&buteMgr, aTagName, LO_MENUPIECE_STYLE, 128);

	// Grab the optional "function" field.
	buteMgr.GetString(aTagName, LO_MENUPIECE_FUNCTION, "", szFunction, sizeof(szFunction));

    CAVector zero(0.0, 0.0, 0.0);
    vPos = buteMgr.GetVector(aTagName, LO_MENUPIECE_POS, zero);

	return LTTRUE;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTronLayoutMgr::CTronLayoutMgr() : CLayoutMgr()
{
	g_pLayoutMgr = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronLayoutMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CTronLayoutMgr::Init(const char* szAttributeFile)
{
	if (!CLayoutMgr::Init(szAttributeFile))
		return LTFALSE;

	// Menu Pieces
	sprintf(s_aTagName, "%s0", LO_MENUPIECE_TAG);
	int numPieces = 0;
	while (m_buteMgr.Exist(s_aTagName))
	{
		INT_MENUPIECE *pPiece = debug_new(INT_MENUPIECE);
		if (pPiece->Init(m_buteMgr,s_aTagName))
			m_MenuPieceArray.push_back(pPiece);
		else
		{
			debug_delete(pPiece);
		}
		numPieces++;
		sprintf(s_aTagName, "%s%d", LO_MENUPIECE_TAG, numPieces);
	}

	sprintf(s_aTagName, "%s0", LO_SUB_TAG);
	int numSubs = 0;
	while (m_buteMgr.Exist(s_aTagName))
	{
		INT_SUBMODEL *pSubModel = debug_new(INT_SUBMODEL);
		if (pSubModel->Init(m_buteMgr,s_aTagName))
		{
			m_SubModelArray.push_back(pSubModel);
		}
		else
		{
			debug_delete(pSubModel);
		}
		numSubs++;
		sprintf(s_aTagName, "%s%d", LO_SUB_TAG, numSubs);
	}

	return LTTRUE;
}


INT_SUBMODEL* CTronLayoutMgr::GetSubModel(const char*szSubModel)
{
	SubModelArray::iterator iter = m_SubModelArray.begin();
	while (iter != m_SubModelArray.end())
	{
		if (stricmp(szSubModel,(*iter)->szName) == 0)
			return (*iter);
		iter++;
	}
	return LTNULL;
}

INT_MENUPIECE* CTronLayoutMgr::GetMenuPiece(char * szPieceName)
{
	int i = 0;
	while (i < (int)m_MenuPieceArray.size())
	{
		if (stricmp(szPieceName,m_MenuPieceArray[i]->szName) == 0)
			return m_MenuPieceArray[i];
		i++;
	}
	return LTNULL;
}

int CTronLayoutMgr::GetHealthEnergyFont(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_HEALTH_ENERGY_FONT))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetInt(s_aTagName, LO_HUD_HEALTH_ENERGY_FONT);
}

LTIntPt CTronLayoutMgr::GetInfoTextPos(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_INFO_POS))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return GetPoint(s_aTagName, LO_HUD_INFO_POS);
}

uint32 CTronLayoutMgr::GetInfoTextColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_INFO_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_INFO_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_INFO_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_INFO_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint32 CTronLayoutMgr::GetInfoTextBadColor(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_INFO_BAD_ALPHA))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	LTFLOAT	fAlpha = GetFloat(s_aTagName, LO_HUD_INFO_BAD_ALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);


    LTVector vColor;
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_INFO_BAD_COLOR))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	vColor = m_buteMgr.GetVector(s_aTagName, LO_HUD_INFO_BAD_COLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

uint8 CTronLayoutMgr::GetInfoTextJustify(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_INFO_JUST))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_INFO_JUST);
}

uint8 CTronLayoutMgr::GetInfoTextSize(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_INFO_SZ))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint8)GetInt(s_aTagName, LO_HUD_INFO_SZ);
}

uint16 CTronLayoutMgr::GetInfoTextWidth(int nLayout)
{
	sprintf(s_aTagName, "%s%d", LO_HUD_TAG, nLayout);
	if (!m_buteMgr.Exist(s_aTagName,LO_HUD_INFO_WD))
		sprintf(s_aTagName, "%s0", LO_HUD_TAG);

	return (uint16)GetInt(s_aTagName, LO_HUD_INFO_WD);
}

LTRect CTronLayoutMgr::GetProgressBoxRect(char *pAttribute)
{
	if (!m_buteMgr.Exist(LO_PROGRESSBOX_TAG, pAttribute))
		sprintf(s_aTagName, "%s0", pAttribute);

	return GetRect(LO_PROGRESSBOX_TAG, pAttribute);
}

int	CTronLayoutMgr::GetProgressTitleFont()
{
	return GetInt(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_TITLEFONT, 0);
}

int	CTronLayoutMgr::GetProgressTitleFontSize()
{
	return GetInt(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_TITLEFONTSIZE, 16);
}

uint32 CTronLayoutMgr::GetProgressTitleColor()
{
	LTFLOAT	fAlpha = GetFloat(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_TITLEALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);

    LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_TITLECOLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);

}

int	CTronLayoutMgr::GetProgressObjectiveFont()
{
	return GetInt(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_OBJECTIVEFONT, 0);
}

int	CTronLayoutMgr::GetProgressObjectiveFontSize()
{
	return GetInt(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_OBJECTIVEFONTSIZE, 16);
}

uint32 CTronLayoutMgr::GetProgressObjectiveColor()
{
	LTFLOAT	fAlpha = GetFloat(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_OBJECTIVEALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);

    LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_OBJECTIVECOLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

uint32 CTronLayoutMgr::GetProgressCompletedObjectiveColor()
{
	LTFLOAT	fAlpha = GetFloat(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_COMPLETEDOBJECTIVEALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);

    LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_COMPLETEDOBJECTIVECOLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

uint32 CTronLayoutMgr::GetProgressSecondaryObjectiveColor()
{
	LTFLOAT	fAlpha = GetFloat(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_SECONDARYOBJECTIVEALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);

    LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_SECONDARYOBJECTIVECOLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

int	CTronLayoutMgr::GetProgressProceduralFont()
{
	return GetInt(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_PROCEDURALFONT, 0);
}

int	CTronLayoutMgr::GetProgressProceduralFontSize()
{
	return GetInt(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_PROCEDURALFONTSIZE, 16);
}

uint32 CTronLayoutMgr::GetProgressProceduralColor()
{
	LTFLOAT	fAlpha = GetFloat(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_PROCEDURALALPHA);
	uint8 nA = (uint8)(255.0f * fAlpha);

    LTVector vColor;

	vColor = m_buteMgr.GetVector(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_PROCEDURALCOLOR);

	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	return SET_ARGB(nA,nR,nG,nB);
}

int	CTronLayoutMgr::GetProgressProceduralHeight()
{
	return GetInt(LO_PROGRESSBOX_TAG, LO_PROGRESSBOX_PROCEDURALHEIGHT, 64);
}
