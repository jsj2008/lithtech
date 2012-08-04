/****************************************************************************
;
;	 MODULE:		MYGAMESPYMGR (.CPP)
;
;	PURPOSE:		Derived Game Spy Manager for this game server
;
;	HISTORY:		09/21/98  [blg]  This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/


// Includes...

#include "StdAfx.h"
#include "Resource.h"
#include "MyGameSpyMgr.h"
#include "GameServDlg.h"
#include "NetStart.h"


// Externs...

extern	ServerInfo		g_ServerInfo;
extern	ServerOptions	g_ServerOptions;
extern	NetGame			g_NetGame;
extern	CGameServDlg*	g_pDialog;


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

	if (!g_pDialog) return(FALSE);


	// Prepare the level name by stripping out the prefixes...

	char sLevel[128];
	char sTemp[128];
	strcpy(sLevel, g_pDialog->GetCurLevel());
	strcpy(sTemp, g_pDialog->GetCurLevel());

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

	SendResponseInfo("hostname", g_ServerInfo.m_sName);
	SendResponseInfo("hostport", GetGamePort());
	SendResponseInfo("mapname", sLevel);
	SendResponseInfo("gametype", g_pDialog->GetGameSpyGameType());
	SendResponseInfo("numplayers", g_pDialog->GetNumPlayers());
	SendResponseInfo("maxplayers", g_ServerInfo.m_dwMaxPlayers);
	SendResponseInfo("gamemode", g_pDialog->GetGameSpyGameMode());


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
	// Send the rules...

	if (g_NetGame.m_byEnd == NGE_TIME || g_NetGame.m_byEnd == NGE_FRAGSANDTIME)
	{
		SendResponseInfo("timelimit", g_NetGame.m_dwEndTime);
	}
	else
	{
		SendResponseInfo("timelimit", 0);
	}

	if (g_NetGame.m_byEnd == NGE_FRAGS || g_NetGame.m_byEnd == NGE_FRAGSANDTIME)
	{
		SendResponseInfo("timelimit", g_NetGame.m_dwEndFrags);
	}
	else
	{
		SendResponseInfo("timelimit", 0);
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

	if (!g_pDialog) return(FALSE);


	// Send the info for each player...

	int count = 0;

	PLAYERINFO* pPi = g_pDialog->GetFirstPlayerInfo();

	while (pPi)
	{
		char sPlayer[32];
		sprintf(sPlayer, "player_%i", count);
		SendResponseInfo(sPlayer, pPi->sName);

		char sFrags[32];
		sprintf(sFrags, "frags_%i", count);
		SendResponseInfo(sFrags, pPi->nFrags);

		char sPing[32];
		sprintf(sPing, "ping_%i", count);
		SendResponseInfo(sPing, pPi->dwPing);

		count++;
		pPi = g_pDialog->GetNextPlayerInfo();
	}


	// All done...

	return(TRUE);
}


