// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerSummaryMgr.cpp
//
// PURPOSE : PlayerSummaryMgr implementation - Player summary
//
// CREATED : 9/08/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerSummary.h"
#include "GameButeMgr.h"
#include "ClientServerShared.h"
#include "MsgIds.h"
#include "CommonUtilities.h"

#if defined(_CLIENTBUILD)
// **************** Client only includes
#include "GameClientShell.h"
#include "WinUtil.h"
extern CGameClientShell* g_pGameClientShell;
extern LTBOOL g_bAllowAllMissions;
#endif

#ifndef _CLIENTBUILD
// **************** Server only includes
#include "GameServerShell.h"
#include "PlayerObj.h"
#endif

#define PSMGR_DEFAULT_CRYPT_KEY					"ThisIsTheKey"

#define PSMGR_MISSIONSUMMARY_TAG				"MissionSummary"
#define PSMGR_STATUS_TAG						"Status"
#define PSMGR_WEAPONS							"Weapons"
#define PSMGR_AMMO								"Ammo"
#define PSMGR_MODS								"Mods"
#define PSMGR_GEAR								"Gear"
#define PSMGR_MISSION							"NextMission"


static char s_aAttributeFile[256];
static char s_aTagName[30];
static char s_aAttName[100];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::CPlayerSummaryMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerSummaryMgr::CPlayerSummaryMgr()
{
	m_pCryptKey  = PSMGR_DEFAULT_CRYPT_KEY;
    m_bInRezFile = LTFALSE;
	m_buteMgr.Init(GBM_DisplayError);

    m_MissionSumList.Init(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::~CPlayerSummaryMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPlayerSummaryMgr::~CPlayerSummaryMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::Term()
{
	m_buteMgr.Term();
	m_MissionSumList.Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerSummaryMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (!szAttributeFile) return LTFALSE;
    if (!g_pMissionMgr) return LTFALSE;

#if defined(_CLIENTBUILD)
	//hack to create a summary.sav
	if (!CWinUtil::FileExist(PLAYERSUMMARY_FILENAME))
		CWinUtil::WinWritePrivateProfileString (GAME_NAME, "temp", "555", PLAYERSUMMARY_FILENAME);
#endif

    LTBOOL bSave = LTFALSE;
    if (!Parse(pInterface, szAttributeFile))
	{
		GBM_DisplayError("CPlayerSummaryMgr couldn't parse file.");
        bSave = LTTRUE;
	}

	strncpy(s_aAttributeFile, szAttributeFile, sizeof(s_aAttributeFile));

	// Read in the data for each mission...Add any tags as necessary...
	// NOTE: This is done starting with the last mission so that the missions
	// are organized sequentially in the summary file (makes it easier
	// to read while debugging).

	int nNumMissions = g_pMissionMgr->GetNumMissions();
	for (int nNum=nNumMissions-1; nNum >= 0; nNum--)
	{
		sprintf(s_aTagName, "%s%d", PSMGR_MISSIONSUMMARY_TAG, nNum);

		if (!m_buteMgr.Exist(s_aTagName))
		{
			m_buteMgr.AddTag(s_aTagName);
            bSave = LTTRUE;
		}

		MISSIONSUMMARY* pMissionSummary = debug_new(MISSIONSUMMARY);

		MISSION* pMission = g_pMissionMgr->GetMission(nNum);
		if (pMissionSummary && pMissionSummary->Init(m_buteMgr, s_aTagName, pMission))
		{
			pMissionSummary->nId = nNum;
			m_MissionSumList.Add(pMissionSummary); // Add at head
		}
		else
		{
			debug_delete(pMissionSummary);
            return LTFALSE;
		}
	}

	MISSIONSUMMARY* pMissionSummary = debug_new(MISSIONSUMMARY);
	pMissionSummary->nId = -1;
	m_MissionSumList.Add(pMissionSummary); // Add at head

	// Get the player rank data...

	m_PlayerRank.ReadRankData(m_buteMgr);

	// If we added anything to the summary file, save it...(this should
	// only happen the very first time a mission is loaded)

	if (bSave)
	{
        Save(pInterface);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::Save()
//
//	PURPOSE:	Save the current player summary...
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerSummaryMgr::Save(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	// Only save in single player...

	if (IsMultiplayerGame()) return LTFALSE;
#ifndef _CLIENTBUILD
	// Save the mission summary data...

    MISSIONSUMMARY** pCur  = LTNULL;

	pCur = m_MissionSumList.GetItem(TLIT_FIRST);

	int nNum = 0;
	while (pCur)
	{
		sprintf(s_aTagName, "%s%d", PSMGR_MISSIONSUMMARY_TAG, (*pCur)->nId);
		nNum++;

		if (*pCur)
		{
			// Always save the global data...(to the summary file)

			(*pCur)->WriteGlobalData(m_buteMgr, s_aTagName);

			// Save the "instant" data.  That is, save the data that must be
			// saved for a snap-shot of the game...

			if (hWrite)
			{
                (*pCur)->WriteInstantData(pInterface, hWrite);
			}
		}

		pCur = m_MissionSumList.GetItem(TLIT_NEXT);
	}
#endif

	// Save the attributes as if it were an attribute file.  If the file doesn't
	// exist it will be created here.

	LTBOOL bRet = m_buteMgr.Save(s_aAttributeFile);


	// For things to save correctly we need all the bute mgr data
	// to have been read in from the file...So, do a refresh...

    RefreshData(pInterface);

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::Load()
//
//	PURPOSE:	Load the current player summary...
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerSummaryMgr::Load(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	// Only save in single player...

	if (IsMultiplayerGame()) return LTFALSE;

	// Save the mission summary data...

    MISSIONSUMMARY** pCur  = LTNULL;

	pCur = m_MissionSumList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur)
		{
            (*pCur)->ReadInstantData(pInterface, hRead);
		}

		pCur = m_MissionSumList.GetItem(TLIT_NEXT);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::RefreshData()
//
//	PURPOSE:	Make sure our bute mgr is up-to-date with what is in the
//				file (i.e., keep client and server version in synch).
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::RefreshData(ILTCSBase *pInterface)
{
	m_buteMgr.Term();
	m_buteMgr.Init(GBM_DisplayError);
    Parse(pInterface, s_aAttributeFile);
}



// *******************************************************************************
// **************** Server only functions ****************************************
// *******************************************************************************
#ifndef _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::HandleLevelStart()
//
//	PURPOSE:	Handle a level starting...
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::HandleLevelStart()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();
	int nLevelNum	= pMissionData->GetLevelNum();

	if (nMissionNum < 0 || nLevelNum < 0) return;

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;


	// Clear out the number the player has currently found in this
	// level...

	pMissionSummary->Levels[nLevelNum].nCurNumIntel = 0;


	// Scan through the level and determine the number of Intelligence
	// objects that are available...(NOTE: we don't store this value in the
	// bute file since it depends on the number of objects in each level making
	// up the mission).

	// Calculate the total number of intelligence objects...

	pMissionSummary->Levels[nLevelNum].nTotalIntel = CalcNumIntelObjects();


	// Save this value out so we won't have to calculate it again...

    Save(g_pLTServer);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::HandleLevelEnd()
//
//	PURPOSE:	Handle a level ending...
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::HandleLevelEnd(CPlayerObj* pPlayer)
{
	if (!pPlayer) return;

	UpdateTotalMissionTime();

	// Calculate the player's ranking...

	if (!IsMultiplayerGame())
	{
		CalcPlayerGlobalRank();
		WriteRankData();
	}

    Save(g_pLTServer);

	SendDataToClient(pPlayer->GetClient());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::IncIntelligenceCount()
//
//	PURPOSE:	Increment the count of intelligence found
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::IncIntelligenceCount()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();
	int nLevelNum	= pMissionData->GetLevelNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	if (nLevelNum < 0 || nLevelNum >= pMissionSummary->nNumLevels) return;

	(pMissionSummary->Levels[nLevelNum].nCurNumIntel)++;

	if (pMissionSummary->Levels[nLevelNum].nCurNumIntel >
		pMissionSummary->Levels[nLevelNum].nTotalIntel)
	{
		pMissionSummary->Levels[nLevelNum].nCurNumIntel = pMissionSummary->Levels[nLevelNum].nTotalIntel;
	}
/*
	if (pMissionSummary->Levels[nLevelNum].nCurNumIntel >
		pMissionSummary->Levels[nLevelNum].nMaxNumIntel)
	{
		pMissionSummary->Levels[nLevelNum].nMaxNumIntel = pMissionSummary->Levels[nLevelNum].nCurNumIntel;
	}
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::UpdateTotalMissionTime()
//
//	PURPOSE:	Update our total mission time
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::UpdateTotalMissionTime()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

    pMissionSummary->fTotalMissionTime += g_pLTServer->GetTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::IncShotsFired()
//
//	PURPOSE:	Increment the number of shots fired
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::IncShotsFired()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumShotsFired)++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::IncNumHits()
//
//	PURPOSE:	Increment the number of hits made
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::IncNumHits(HitLocation eHitLocation)
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumHits)++;

	if (eHitLocation < HL_NUM_LOCS)
	{
		(pMissionSummary->dwHitLocations[eHitLocation])++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::IncNumTimesDetected()
//
//	PURPOSE:	Increment the number of times detected
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::IncNumTimesDetected()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumTimesDetected)++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::IncNumDisturbances()
//
//	PURPOSE:	Increment the number of times player caused disturbances
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::IncNumDisturbances()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumDisturbances)++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::IncNumBodies()
//
//	PURPOSE:	Increment the number of bodies found by AI
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::IncNumBodies()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumBodies)++;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::IncNumKills()
//
//	PURPOSE:	Increment the number of kills
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::IncNumEnemyKills()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumEnemyKills)++;
}

void CPlayerSummaryMgr::IncNumFriendKills()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumFriendKills)++;
}

void CPlayerSummaryMgr::IncNumNeutralKills()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumNeutralKills)++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::IncNumTimesHit()
//
//	PURPOSE:	Increment the number of times hit
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::IncNumTimesHit()
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;

	(pMissionSummary->dwNumTimesHit)++;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::SendDataToClient
//
//	PURPOSE:	Send the player summary data to the client
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::SendDataToClient(HCLIENT hClient)
{
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;


    HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClient, MID_PLAYER_SUMMARY);
	if (hWrite)
	{
        pMissionSummary->WriteClientData(g_pLTServer, hWrite);
        g_pLTServer->EndMessage(hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::WriteRankData()
//
//	PURPOSE:	Write the current player rank to the bute file
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerSummaryMgr::WriteRankData()
{
	// Only save in single player...

    if (IsMultiplayerGame()) return LTFALSE;

	// Save the player rank data...

	m_PlayerRank.WriteRankData(m_buteMgr);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::CalcNumIntelObjects()
//
//	PURPOSE:	Calculate the number of intelligence objects in the level
//
// ----------------------------------------------------------------------- //

int CPlayerSummaryMgr::CalcNumIntelObjects()
{
	int nNum = 0;

    HCLASS hClass = g_pLTServer->GetClass("Intelligence");

    HOBJECT hObj = g_pLTServer->GetNextObject(LTNULL);
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			nNum++;
		}

        hObj = g_pLTServer->GetNextObject(hObj);
	}

    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hClass))
		{
			nNum++;
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}

	return nNum;
}

#endif //#ifndef _CLIENTBUILD
// *******************************************************************************



// *******************************************************************************
// **************** Client only functions ****************************************
// *******************************************************************************
#if defined(_CLIENTBUILD)
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::ReadClientData
//
//	PURPOSE:	Read the player summary data sent from the server
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::ReadClientData(HMESSAGEREAD hRead)
{
	CMissionData* pMissionData = g_pInterfaceMgr->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSIONSUMMARY* pMissionSummary = GetMissionSummary(nMissionNum);
	if (!pMissionSummary) return;


    pMissionSummary->ReadClientData(g_pLTClient, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::ReadRankData()
//
//	PURPOSE:	Read the current player rank from the bute file
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerSummaryMgr::ReadRankData()
{
	// Only read in single player...

	if (IsMultiplayerGame()) return LTFALSE;

	// Make sure we are up-to-date...

    RefreshData(g_pLTClient);

	// Get the player rank data...

	m_PlayerRank.ReadRankData(m_buteMgr);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::WriteWeaponData()
//
//	PURPOSE:	Write the current player weapon data to the bute file
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::WriteWeaponData(LTBOOL *pbCanUseWeapon, int numWeapons)
{
	char temp[128];
	if (numWeapons >= 127) numWeapons = 127;
    int i;
    for (i = 0;i < numWeapons; i++)
	{
		temp[i] = (pbCanUseWeapon[i] ? '1' : '0');
	}
    temp[i] = LTNULL;
	m_buteMgr.SetString(PSMGR_STATUS_TAG,PSMGR_WEAPONS,CString(temp));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::ReadWeaponData()
//
//	PURPOSE:	Read the current player weapon data from the bute file
//
// ----------------------------------------------------------------------- //


void CPlayerSummaryMgr::ReadWeaponData(LTBOOL *pbCanUseWeapon, int numWeapons)
{
	CString temp;
	temp = m_buteMgr.GetString(PSMGR_STATUS_TAG,PSMGR_WEAPONS);
	char *pStr = (char*)(LPCSTR)temp;
	int i = 0;
	int num = strlen(pStr);
	while (i < num && i < numWeapons)
	{
		pbCanUseWeapon[i] = (pStr[i] == '1');
		i++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::WriteAmmoData()
//
//	PURPOSE:	Write the current player Ammo data to the bute file
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::WriteAmmoData(LTBOOL *pbCanUseAmmo, int numAmmos)
{
	char temp[128];
	if (numAmmos >= 127) numAmmos = 127;
    int i;
    for (i = 0;i < numAmmos; i++)
	{
		temp[i] = (pbCanUseAmmo[i] ? '1' : '0');
	}
    temp[i] = LTNULL;
	m_buteMgr.SetString(PSMGR_STATUS_TAG,PSMGR_AMMO,CString(temp));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::ReadAmmoData()
//
//	PURPOSE:	Read the current player Ammo data from the bute file
//
// ----------------------------------------------------------------------- //


void CPlayerSummaryMgr::ReadAmmoData(LTBOOL *pbCanUseAmmo, int numAmmos)
{
	CString temp;
	temp = m_buteMgr.GetString(PSMGR_STATUS_TAG,PSMGR_AMMO);
	char *pStr = (char*)(LPCSTR)temp;
	int i = 0;
	int num = strlen(pStr);
	while (i < num && i < numAmmos)
	{
		pbCanUseAmmo[i] = (pStr[i] == '1');
		i++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::WriteModData()
//
//	PURPOSE:	Write the current player Mod data to the bute file
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::WriteModData(LTBOOL *pbCanUseMod, int numMods)
{
	char temp[128];
	if (numMods >= 127) numMods = 127;
    int i;
    for (i = 0;i < numMods; i++)
	{
		temp[i] = (pbCanUseMod[i] ? '1' : '0');
	}
    temp[i] = LTNULL;
	m_buteMgr.SetString(PSMGR_STATUS_TAG,PSMGR_MODS,CString(temp));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::ReadModData()
//
//	PURPOSE:	Read the current player Mod data from the bute file
//
// ----------------------------------------------------------------------- //


void CPlayerSummaryMgr::ReadModData(LTBOOL *pbCanUseMod, int numMods)
{
	CString temp;
	temp = m_buteMgr.GetString(PSMGR_STATUS_TAG,PSMGR_MODS);
	char *pStr = (char*)(LPCSTR)temp;
	int i = 0;
	int num = strlen(pStr);
	while (i < num && i < numMods)
	{
		pbCanUseMod[i] = (pStr[i] == '1');
		i++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::WriteGearData()
//
//	PURPOSE:	Write the current player Gear data to the bute file
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::WriteGearData(LTBOOL *pbCanUseGear, int numGears)
{
	char temp[128];
	if (numGears >= 127) numGears = 127;
    int i;
    for (i = 0;i < numGears; i++)
	{
		temp[i] = (pbCanUseGear[i] ? '1' : '0');
	}
    temp[i] = LTNULL;
	m_buteMgr.SetString(PSMGR_STATUS_TAG,PSMGR_GEAR,CString(temp));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::ReadGearData()
//
//	PURPOSE:	Read the current player Gear data from the bute file
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::ReadGearData(LTBOOL *pbCanUseGear, int numGears)
{
	CString temp;
	temp = m_buteMgr.GetString(PSMGR_STATUS_TAG,PSMGR_GEAR);
	char *pStr = (char*)(LPCSTR)temp;
	int i = 0;
	int num = strlen(pStr);
	while (i < num && i < numGears)
	{
		pbCanUseGear[i] = (pStr[i] == '1');
		i++;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::GetNextMission()
//
//	PURPOSE:	Get the number of the next uncompleted mission
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerSummaryMgr::GetNextMission()
{
	int mis = m_buteMgr.GetInt(PSMGR_STATUS_TAG,PSMGR_MISSION,0);
	if (g_bAllowAllMissions)
		mis = g_pMissionMgr->GetNumMissions();
	return mis;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::CompleteMission()
//
//	PURPOSE:	Mark the specified mission as complete
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::CompleteMission(int nMissionNum)
{
	if (nMissionNum < 0 || nMissionNum >= g_pMissionMgr->GetNumMissions() || g_bAllowAllMissions) return;
	int mis = m_buteMgr.GetInt(PSMGR_STATUS_TAG,PSMGR_MISSION,0);

	g_pInterfaceMgr->GetPlayerStats()->SaveInventory();

	if ((nMissionNum+1) > mis)
		m_buteMgr.SetInt(PSMGR_STATUS_TAG,PSMGR_MISSION,(nMissionNum+1));
    Save(g_pLTClient);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::ClearStatus()
//
//	PURPOSE:	Clear the players status
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::ClearStatus()
{
	// Get the player rank data...
	m_PlayerRank.ClearRankData(m_buteMgr);

    MISSIONSUMMARY** pCur  = LTNULL;

	pCur = m_MissionSumList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur)
		{
			sprintf(s_aTagName, "%s%d", PSMGR_MISSIONSUMMARY_TAG, (*pCur)->nId);
			(*pCur)->ClearGlobalData(m_buteMgr,s_aTagName);

		}

		pCur = m_MissionSumList.GetItem(TLIT_NEXT);
	}

	m_buteMgr.SetString(PSMGR_STATUS_TAG,PSMGR_WEAPONS,CString(""));
	m_buteMgr.SetString(PSMGR_STATUS_TAG,PSMGR_AMMO,CString(""));
	m_buteMgr.SetString(PSMGR_STATUS_TAG,PSMGR_MODS,CString(""));
	m_buteMgr.SetString(PSMGR_STATUS_TAG,PSMGR_GEAR,CString(""));
	m_buteMgr.SetInt(PSMGR_STATUS_TAG,PSMGR_MISSION,0);
    Save(g_pLTClient);
}


#endif //_CLIENTBUILD
// *******************************************************************************








// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::GetMissionSummary
//
//	PURPOSE:	Get the specified mission summary
//
// ----------------------------------------------------------------------- //

MISSIONSUMMARY* CPlayerSummaryMgr::GetMissionSummary(int nMissionId)
{
//  if (nMissionId < 0 || nMissionId > m_MissionSumList.GetLength()) return LTNULL;

    MISSIONSUMMARY** pCur  = LTNULL;

	pCur = m_MissionSumList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nMissionId)
		{
			return *pCur;
		}

		pCur = m_MissionSumList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::CalcCurPlayerRank()
//
//	PURPOSE:	Calculate the player's current rank
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::CalcCurPlayerRank()
{
	// Calculate the player's rank for the current mission...

#ifdef _CLIENTBUILD
	CMissionData* pMissionData = g_pInterfaceMgr->GetMissionData();
#else
	CMissionData* pMissionData = g_pGameServerShell->GetMissionData();
#endif

	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();

	MISSION* pMission = g_pMissionMgr->GetMission(nMissionNum);
	MISSIONSUMMARY* pSummary = GetMissionSummary(nMissionNum);

	CalcMissionRank(pMission, pSummary);


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::CalcPlayerGlobalRank()
//
//	PURPOSE:	Calculate the player's global rank
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::CalcPlayerGlobalRank()
{
	m_PlayerRank.Reset();


	MISSIONRATING* pMissionRating = g_pMissionMgr->GetMissionRating();

	// Add up the rewards from all the missions...
	for (int i=0; i < g_pMissionMgr->GetNumMissions(); i++)
	{
		MISSION* pMission = g_pMissionMgr->GetMission(i);
		MISSIONSUMMARY* pSummary = GetMissionSummary(i);


		CalcMissionRank(pMission, pSummary);

		// Need to calculate these for each mission based on the best
		// rank in the summary file, and the reward points in the
		// missions bute file...

		RANKBONUS sBonus;
		pMission->GetRankBonus(pSummary->fBestRank,&sBonus);

		// calculate reputation;
		m_PlayerRank.nReputation += sBonus.nReputationPoints;
		if (m_PlayerRank.nReputation > pMissionRating->nReputationMax)
			m_PlayerRank.nReputation = pMissionRating->nReputationMax;

		// Calculate health multiplier...
        LTFLOAT fHealthMult = pMissionRating->fHealthInc * sBonus.nHealthPoints;
		m_PlayerRank.fHealthMultiplier += fHealthMult;

		if (m_PlayerRank.fHealthMultiplier > pMissionRating->fHealthMax)
		{
			m_PlayerRank.fHealthMultiplier = pMissionRating->fHealthMax;
		}

		// Calculate Armor multiplier...
        LTFLOAT fArmorMult = pMissionRating->fArmorInc * sBonus.nArmorPoints;
		m_PlayerRank.fArmorMultiplier += fArmorMult;

		if (m_PlayerRank.fArmorMultiplier > pMissionRating->fArmorMax)
		{
			m_PlayerRank.fArmorMultiplier = pMissionRating->fArmorMax;
		}

		// Calculate Ammo multiplier...
        LTFLOAT fAmmoMult = pMissionRating->fAmmoInc * sBonus.nAmmoPoints;
		m_PlayerRank.fAmmoMultiplier += fAmmoMult;

		if (m_PlayerRank.fAmmoMultiplier > pMissionRating->fAmmoMax)
		{
			m_PlayerRank.fAmmoMultiplier = pMissionRating->fAmmoMax;
		}


		// Calculate Damage multiplier...
        LTFLOAT fDamageMult = pMissionRating->fDamageInc * sBonus.nDamagePoints;
		m_PlayerRank.fDamageMultiplier += fDamageMult;

		if (m_PlayerRank.fDamageMultiplier > pMissionRating->fDamageMax)
		{
			m_PlayerRank.fDamageMultiplier = pMissionRating->fDamageMax;
		}


		// Calculate Perturb multiplier...
        LTFLOAT fPerturbMult = pMissionRating->fPerturbInc * sBonus.nPerturbPoints;
		m_PlayerRank.fPerturbMultiplier += fPerturbMult;

		if (m_PlayerRank.fPerturbMultiplier < pMissionRating->fPerturbMin)
		{
			m_PlayerRank.fPerturbMultiplier = pMissionRating->fPerturbMin;
		}

		// Calculate Stealth multiplier...
        LTFLOAT fStealthMult = pMissionRating->fStealthInc * sBonus.nStealthPoints;
		m_PlayerRank.fStealthMultiplier += fStealthMult;

		if (m_PlayerRank.fStealthMultiplier < pMissionRating->fStealthMin)
		{
			m_PlayerRank.fStealthMultiplier = pMissionRating->fStealthMin;
		}

	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::CalcMissionRank()
//
//	PURPOSE:	Calculate the player's rank for the specified mission...
//
// ----------------------------------------------------------------------- //

void CPlayerSummaryMgr::CalcMissionRank(MISSION* pMission, MISSIONSUMMARY* pSummary)
{
	if (!pSummary || !pMission) return;

	MISSIONRATING* pMissionRating = g_pMissionMgr->GetMissionRating();
    LTFLOAT fSuccessPts  = (LTFLOAT) (pMissionRating->nSuccessPts) / 100.0f;
    LTFLOAT fIntelPts    = (LTFLOAT) (pMissionRating->nIntelPts) / 100.0f;

	int nCurIntelFound = 0;
	int nMaxIntelFound = 0;
	int nOldMaxIntelFound = 0;
	int nTotalIntel    = 0;

	for (int i=0; i < pSummary->nNumLevels; i++)
	{
		nCurIntelFound		+= pSummary->Levels[i].nCurNumIntel;
		nTotalIntel			+= pSummary->Levels[i].nTotalIntel;
		nOldMaxIntelFound	+= pSummary->Levels[i].nMaxNumIntel;
		if (pSummary->Levels[i].nCurNumIntel > pSummary->Levels[i].nMaxNumIntel)
		{
			pSummary->Levels[i].nMaxNumIntel = pSummary->Levels[i].nCurNumIntel;
		}
		nMaxIntelFound		+= pSummary->Levels[i].nMaxNumIntel;
	}

	if (nTotalIntel > 0)
	{
		if (nMaxIntelFound > nTotalIntel)
		{
			nMaxIntelFound = nTotalIntel;

		}

        LTFLOAT fPercentage = float(nMaxIntelFound) / float(nTotalIntel);
		pSummary->fBestRank = fSuccessPts + fIntelPts * fPercentage;

		fPercentage = float(nCurIntelFound) / float(nTotalIntel);
		pSummary->fCurRank = fSuccessPts + fIntelPts * fPercentage;

		fPercentage = float(nOldMaxIntelFound) / float(nTotalIntel);
		pSummary->fOldBestRank = fSuccessPts + fIntelPts * fPercentage;



	}


}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerSummaryMgr::Parse()
//
//	PURPOSE:	Parse from a rez file
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerSummaryMgr::Parse(ILTCSBase *pInterface, const char* sButeFile)
{
	// Sanity checks...

    if (!sButeFile)	return(LTFALSE);


	BOOL bRet = TRUE;

	// If we're going to allow the bute file to be saved by the game, it must
	// be read in from a file (not a .rez file)...

	// Append the NOLF directory onto the filename if this file is normally
	// stored in the .rez file...

	if (m_bInRezFile)
	{
		m_strAttributeFile.Format("NOLF\\%s", sButeFile);
	}
	else
	{
		m_strAttributeFile.Format("%s", sButeFile);
	}


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