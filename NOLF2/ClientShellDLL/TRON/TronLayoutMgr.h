// ----------------------------------------------------------------------- //
//
// MODULE  : TronLayoutMgr.h
//
// PURPOSE : Attribute file manager for interface layout info
//			 Tron-specific items
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_TRONLAYOUTMGR_H_)
#define _TRONLAYOUTMGR_H_

#include "LayoutMgr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//class CTronLayoutMgr;
//extern CTronLayoutMgr* g_pTronLayoutMgr;
 
struct INT_SUBMODEL
{
	INT_SUBMODEL()	{szName[0] = szModel[0] = 0;}

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szName[128];
	char	szModel[128];

	CButeListReader blrSkins;
	CButeListReader blrRenderStyles;
};

struct INT_MENUPIECE // it's basically INT_CHAR
{
	INT_MENUPIECE()	{szName[0] = szModel[0] = 0; vPos.Init(); }

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);

	char	szName[128];
	char	szModel[128];
	char	szLoopAnim[128];
	char	szFunction[128];

	CButeListReader blrSkins;
	CButeListReader blrRenderStyles;

    LTVector vPos;
};

class CTronLayoutMgr : public CLayoutMgr
{
public:
	CTronLayoutMgr();

    LTBOOL			Init(const char* szAttributeFile=LO_DEFAULT_FILE);

	INT_MENUPIECE* CTronLayoutMgr::GetMenuPiece(char * szModelName);
	INT_SUBMODEL	*GetSubModel(const char*szSubModel);

	int			GetHealthEnergyFont(int nLayout);

	LTIntPt		GetInfoTextPos(int nLayout);
	uint8		GetInfoTextSize(int nLayout);
	uint8		GetInfoTextJustify(int nLayout);
	uint32		GetInfoTextColor(int nLayout);
	uint32		GetInfoTextBadColor(int nLayout);
	uint16		GetInfoTextWidth(int nLayout);

	// Progress HUD parameters
    LTRect      GetProgressBoxRect(char *pAttribute);
	int			GetProgressTitleFont();
	int			GetProgressTitleFontSize();
	uint32		GetProgressTitleColor();
	int			GetProgressObjectiveFont();
	int			GetProgressObjectiveFontSize();
	uint32		GetProgressObjectiveColor();
	uint32		GetProgressCompletedObjectiveColor();
	uint32		GetProgressSecondaryObjectiveColor();
	int			GetProgressProceduralFont();
	int			GetProgressProceduralFontSize();
	uint32		GetProgressProceduralColor();
	int			GetProgressProceduralHeight();

private:
	typedef std::vector<INT_SUBMODEL *> SubModelArray;
	SubModelArray m_SubModelArray;

	typedef std::vector<INT_MENUPIECE *> MenuPieceArray;
	MenuPieceArray m_MenuPieceArray;
};

#endif // _TRONLAYOUTMGR_H_