/****************************************************************************
;
;	 MODULE:		MYGAMESPYMGR (.CPP)
;
;	PURPOSE:		Derived Game Spy Manager for this game server
;
;	HISTORY:		09/21/98  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 2000, Monolith Productions, Inc.
;
****************************************************************************/


// Includes...

#include "StdAfx.h"
#include "GameServerShell.h"
#include "MyGameSpyMgr.h"
#include "PlayerObj.h"


// Externs...

extern	CGameServerShell*   g_pGameServerShell;


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMyGameSpyMgr::OnInfoQuery
//
//	PURPOSE:	Handles the "info" query
//
// ----------------------------------------------------------------------- //

BOOL CMyGameSpyMgr::OnInfoQuery()
{
	// Sanity checks...

	if (!g_pGameServerShell) return(FALSE);


	// Prepare the level name by stripping out the prefixes...

	char sLevel[128];
	char sTemp[128];
	strcpy(sLevel, g_pGameServerShell->GetCurLevel());
	strcpy(sTemp, sLevel);

	int nLen = strlen(sTemp);

	if (nLen > 2)
	{
		int i = nLen - 1;

		while (i > 0 && sTemp[i] != '\\')
		{
			i--;
		}

		if (i < nLen - 1)
		{
			if (sTemp[i] == '\\') i++;
			strcpy(sLevel, &sTemp[i]);
		}
	}


	// Send the info...

	SendResponseInfo("hostname", g_pGameServerShell->GetHostName());
	SendResponseInfo("hostport", GetGamePort());
	SendResponseInfo("mapname", sLevel);
	SendResponseInfo("gametype", g_pGameServerShell->GetGameSpyGameType());
	SendResponseInfo("numplayers", g_pGameServerShell->GetNumPlayers());
	SendResponseInfo("maxplayers", g_pGameServerShell->GetMaxPlayers());


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMyGameSpyMgr::OnRulesQuery
//
//	PURPOSE:	Handles the "rules" query
//
// ----------------------------------------------------------------------- //

BOOL CMyGameSpyMgr::OnRulesQuery()
{
	// Sanity checks...

	if (!g_pGameServerShell) return(FALSE);


	// Get the game data...

	GAMEDATA gd;

	if (!g_pGameServerShell->FillGameDataStruct(&gd))
	{
		return(FALSE);
	}


	// Send the rules...

	int	nNumOptions = (int)g_pServerOptionMgr->GetNumOptions();
	if (nNumOptions > MAX_GAME_OPTIONS)
		nNumOptions = MAX_GAME_OPTIONS;

	char pTemp[16] = "";
	for (int i = 0; i < nNumOptions; i++)
	{
		OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
		if (g_pGameServerShell->GetGameType() == pOpt->eGameType || pOpt->eGameType == SINGLE)
		{
			if (pOpt->fSliderScale > 0.0f)
				SendResponseInfo(pOpt->szVariable,(int)(pOpt->GetValue() /pOpt->fSliderScale ));
			else
				SendResponseInfo(pOpt->szVariable,(int)pOpt->GetValue());
		}
	}

	
	if (gd.m_bUsePassword)
	{
		SendResponseInfo("Password", gd.m_szPassword);
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMyGameSpyMgr::OnPlayersQuery
//
//	PURPOSE:	Handles the "players" query
//
// ----------------------------------------------------------------------- //

BOOL CMyGameSpyMgr::OnPlayersQuery()
{
	// Sanity checks...

	if (!g_pGameServerShell) return(FALSE);


	// Send the info for each player...

	int count = 0;

	CPlayerObj* pPlr = g_pGameServerShell->GetFirstNetPlayer();

	while (pPlr)
	{
		char sPlayer[32];
		sprintf(sPlayer, "player_%i", count);
		SendResponseInfo(sPlayer, pPlr->GetNetName());

		char sFrags[32];
		sprintf(sFrags, "frags_%i", count);
		SendResponseInfo(sFrags, pPlr->GetFragCount());

		char sPing[32];
		sprintf(sPing, "ping_%i", count);
		int nPing = g_pGameServerShell->GetPlayerPing(pPlr);
		if (nPing <= 0) nPing = 1;
		SendResponseInfo(sPing, nPing);

		count++;
		pPlr = g_pGameServerShell->GetNextNetPlayer();
	}


	// All done...

	return(TRUE);
}


