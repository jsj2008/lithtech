/****************************************************************************
;
;	 MODULE:		NetDefs (.CPP)
;
;	PURPOSE:		Network game definitions
;
;	HISTORY:		11/12/99 [kls] This file was created
;
;	COMMENT:		Copyright (c) 1999, Monolith Productions Inc.
;
****************************************************************************/

#include "stdafx.h"
#include "NetDefs.h"
#include "ButeMgr.h"

#include "ltbasedefs.h"

// Guids...

#ifdef _DEMO
LTGUID NOLFGUID = {  // {8AFA62A2-01AE-11d5-B95C-00609709830E}
	0x8afa62a2, 0x1ae, 0x11d5, 0xb9, 0x5c, 0x0, 0x60, 0x97, 0x9, 0x83, 0xe 
};
#else
LTGUID NOLFGUID = { // {1DFB2BC1-EB40-11d2-B7D2-0060971766C1}
	0x8afa62a1, 0x1ae, 0x11d5, 0xb9, 0x5c, 0x0, 0x60, 0x97, 0x9, 0x83, 0xe 
};
#endif


static const char* g_kaGameTypeString[] =
{
	"Single",
	"H.A.R.M. vs. UNITY",
	"Deathmatch",
};

const int g_knNumGameTypes = sizeof(g_kaGameTypeString) / sizeof(g_kaGameTypeString[0]);

const char* GameTypeToString(GameType eType)
{
	if (eType >= 0 && eType < g_knNumGameTypes)
	{
		return g_kaGameTypeString[eType];
	}

    return LTNULL;
}

ServerOption::ServerOption()
{
	nId = -1;

    szVariable[0] = LTNULL;
    szServVariable[0] = LTNULL;

	nNameId = 0;
	nHelpId = 0;

	eType = SO_UNKNOWN;
	eGameType = SINGLE;

	nNumStrings = 0;

	nSliderMin = 0;
	nSliderMax = 0;
	nSliderInc = 0;

	fSliderScale = 1.0f;


}

LTBOOL ServerOption::InitializeFromBute(CButeMgr & buteMgr, const char* aTagName)
{
    if (!aTagName) return LTFALSE;


	CString str = buteMgr.GetString(aTagName, SO_VARIABLE);
	if (!str.IsEmpty())
	{
		strncpy(szVariable, (char*)(LPCSTR)str, sizeof(szVariable));
	}
	else
        szVariable[0] = LTNULL;

	str = buteMgr.GetString(aTagName, SO_SERV_VARIABLE);
	if (!str.IsEmpty())
	{
		strncpy(szServVariable, (char*)(LPCSTR)str, sizeof(szServVariable));
	}
	else
        strncpy(szServVariable, szVariable, sizeof(szServVariable));

	nNameId = buteMgr.GetInt(aTagName, SO_NAME,0);
	nHelpId = buteMgr.GetInt(aTagName, SO_HELP,0);
	eType = (eOptionType) buteMgr.GetInt(aTagName, SO_TYPE,(int)SO_UNKNOWN);
	eGameType = (GameType) buteMgr.GetInt(aTagName, SO_GAME_TYPE,(int)SINGLE);

	nNumStrings = 0;
	str = buteMgr.GetString(aTagName, SO_STRINGS);
	if (!str.IsEmpty())
	{
		char szTemp[128];
		char *pNum;
		strncpy(szTemp, (char*)(LPCSTR)str, sizeof(szTemp));

		pNum = strtok(szTemp,",");
		while (pNum && nNumStrings < SO_MAX_STRINGS)
		{
			nStringId[nNumStrings] = atoi(pNum);
			nNumStrings++;
			pNum = strtok(NULL,",");
		}
	}


	for (int i = nNumStrings; i < SO_MAX_STRINGS; i++)
		nStringId[i] = 0;

    CPoint zero(0, 0);
    CPoint tmp = buteMgr.GetPoint(aTagName, SO_RANGE, zero);
	nSliderMin = tmp.x;
	nSliderMax = tmp.y;

	nSliderInc = buteMgr.GetInt(aTagName, SO_INCREMENT,0);
    fSliderScale = (LTFLOAT)buteMgr.GetDouble(aTagName, SO_SCALE,1.0f);

    fDefault = fSliderScale * (LTFLOAT)buteMgr.GetDouble(aTagName, SO_DEFAULT,0.0f);

    return LTTRUE;
}