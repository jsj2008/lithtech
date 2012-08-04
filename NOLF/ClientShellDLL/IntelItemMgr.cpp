// ----------------------------------------------------------------------- //
//
// MODULE  : IntelItemMgr.cpp
//
// PURPOSE : IntelItemMgr implementation - Player summary
//
// CREATED : 9/08/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "IntelItemMgr.h"
#include "GameButeMgr.h"
#include "ClientServerShared.h"
#include "MsgIds.h"
#include "CommonUtilities.h"

#include "GameClientShell.h"
#include "WinUtil.h"
extern CGameClientShell* g_pGameClientShell;
extern LTBOOL g_bAllowAllMissions;

#define IMGR_DEFAULT_CRYPT_KEY					"KeyForIntelItemMgr"

#define IMGR_MISSION_TAG						"Mission"
#define IMGR_INTEL								"Intel"

static char s_aAttributeFile[256];
static char s_aTagName[30];
static char s_aAttName[100];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelItemMgr::CIntelItemMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CIntelItemMgr::CIntelItemMgr()
{
//	m_pCryptKey  = IMGR_DEFAULT_CRYPT_KEY;
	m_pCryptKey  = LTNULL;

	m_buteMgr.Init(GBM_DisplayError);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelItemMgr::~CIntelItemMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CIntelItemMgr::~CIntelItemMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelItemMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CIntelItemMgr::Term()
{
	m_buteMgr.Term();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelItemMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CIntelItemMgr::Init(const char* szAttributeFile)
{
    if (!szAttributeFile) return LTFALSE;

	strncpy(s_aAttributeFile, szAttributeFile, sizeof(s_aAttributeFile));

	//hack to create a intel.sav
	if (!CWinUtil::FileExist(s_aAttributeFile))
	{
		fstream f;
		f.open(s_aAttributeFile, ios::out);
		f.write(" ",5);
		f.close();
	}

	if (!Parse(szAttributeFile))
	{
		m_buteMgr.SetPoint("temp","temp", CPoint(0,0));
		m_buteMgr.Save(s_aAttributeFile);
		RefreshData();
	}
    return LTTRUE;
}


void CIntelItemMgr::AddItem(uint8 nType, uint32 nID)
{
	if (g_pGameClientShell->IsMultiplayerGame()) return;
	if (g_pGameClientShell->IsCustomLevel()) return;

	int nMissionNum = g_pGameClientShell->GetCurrentMission();
	int nIndex = 0;
	CPoint item((int)nType,(int)nID);

	LTBOOL bFound = LTFALSE;
	sprintf(s_aTagName,"%s%d",IMGR_MISSION_TAG,nMissionNum);
	sprintf(s_aAttName,"%s%d",IMGR_INTEL,nIndex);

	CPoint test(-1,-1);
	while (!bFound && m_buteMgr.Exist(s_aTagName,s_aAttName))
	{
        CPoint zero(0,0);
        test = m_buteMgr.GetPoint(s_aTagName,s_aAttName,zero);
		bFound = (test.y == item.y);
		if (!bFound)
		{
			nIndex++;
			sprintf(s_aAttName,"%s%d",IMGR_INTEL,nIndex);
		}
	}
	if (!bFound)
	{
		g_pLTClient->CPrint("%s,%s (%d,%d)",s_aTagName,s_aAttName, item.x, item.y);
		m_buteMgr.SetPoint(s_aTagName,s_aAttName, item);
		m_buteMgr.Save(s_aAttributeFile);
		RefreshData();
	}
}

int CIntelItemMgr::GetNumItems(int nMissionNum)
{
	int nIndex = 0;
	sprintf(s_aTagName,"%s%d",IMGR_MISSION_TAG,nMissionNum);
	sprintf(s_aAttName,"%s%d",IMGR_INTEL,nIndex);

	while (m_buteMgr.Exist(s_aTagName,s_aAttName))
	{
		nIndex++;
		sprintf(s_aAttName,"%s%d",IMGR_INTEL,nIndex);
	}
	g_pLTClient->CPrint("items in gallery:%d",nIndex);
	return nIndex;

}
void CIntelItemMgr::GetItem(int nMissionNum, int nIndex, IntelItem* pItem)
{
	if (!pItem) return;
	sprintf(s_aTagName,"%s%d",IMGR_MISSION_TAG,nMissionNum);
	sprintf(s_aAttName,"%s%d",IMGR_INTEL,nIndex);
    CPoint zero(0,0);
    CPoint item = m_buteMgr.GetPoint(s_aTagName,s_aAttName,zero);
	pItem->nType = (uint8)item.x;
	pItem->nID = (uint32)item.y;

}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelItemMgr::Parse()
//
//	PURPOSE:	Parse from a rez file
//
// ----------------------------------------------------------------------- //

LTBOOL CIntelItemMgr::Parse(const char* sButeFile)
{
	// Sanity checks...

    if (!sButeFile)	return(LTFALSE);


	BOOL bRet = TRUE;

	m_strAttributeFile = sButeFile;

	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(m_strAttributeFile, m_pCryptKey);
	}
	else
	{
		bRet = m_buteMgr.Parse(m_strAttributeFile);
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIntelItemMgr::RefreshData()
//
//	PURPOSE:	Make sure our bute mgr is up-to-date with what is in the
//				file
//
// ----------------------------------------------------------------------- //

void CIntelItemMgr::RefreshData()
{
	m_buteMgr.Term();
	m_buteMgr.Init(GBM_DisplayError);
    if (!Parse(s_aAttributeFile))
		g_pLTClient->CPrint("error parsing in RefreshData()");
}


