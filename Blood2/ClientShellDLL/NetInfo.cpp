/****************************************************************************
;
;	 MODULE:		NetInfo (.CPP)
;
;	PURPOSE:		Network game info
;
;	HISTORY:		07/05/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "Windows.h"
#include "NetInfo.h"
#include "Sparam.h"
#include "Assert.h"
#include "..\Shared\ClientRes.h"
#include <mbstring.h>


// Statics...

char s_sNameWithPing[256];


// Functions...

/* *********************************************************************** */
/* CNinfoMgr                                                               */

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CNinfoMgr::Init(CClientDE* pClientDE)
{
	// Set simple members...

	m_pClientDE = pClientDE;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoMgr::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CNinfoMgr::Term()
{
	RemoveGames();
	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoMgr::GetGame
//
//	PURPOSE:	Gets the game object has the given net session handle
//
// ----------------------------------------------------------------------- //

CNinfoGame* CNinfoMgr::GetGame(NetSession* pNetSession)
{
	// Sanity checks...

	if (!pNetSession) return(NULL);


	// Look for a game object with this net session handle...

	CNinfoGame* pGame = GetFirstGame();

	while (pGame)
	{
		if (pGame->GetSessionPointer() == pNetSession)
		{
			return(pGame);
		}

		pGame = GetNextGame();
	}


	// If we get here, we didn't find a match...

	return(NULL);
}

CNinfoGame* CNinfoMgr::GetGame(DWORD dwCRC)
{
	// Look for a game object with this CRC...

	CNinfoGame* pGame = GetFirstGame();

	while (pGame)
	{
		if (pGame->GetCRC() == dwCRC)
		{
			return(pGame);
		}

		pGame = GetNextGame();
	}


	// If we get here, we didn't find a match...

	return(NULL);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoMgr::AddGame
//
//	PURPOSE:	Adds a new game object
//
// ----------------------------------------------------------------------- //

CNinfoGame* CNinfoMgr::AddGame(const char* sInfo, NetSession* pNetSession, DWORD dwNumDPlayPlayers)
{
	// Sanity checks...

	if (!sInfo) return(NULL);
	if (!pNetSession) return(NULL);


	// Get the name of this game session...

	char sName[NML_NAME];

	if (!Sparam_Get(sName, sInfo, NST_GAMENAME))
	{
		return(NULL);
	}


	// Get the host of this game session...

	char sHost[NML_HOST];

	if (!Sparam_Get(sHost, sInfo, NST_GAMEHOST))
	{
		return(NULL);
	}


	// Get the level of this game...

	char sLevel[NML_LEVEL];
	char sBuf[NML_LEVEL];

	if (!Sparam_Get(sBuf, sInfo, NST_GAMELEVEL))
	{
		return(NULL);
	}

	int nLen   = strlen(sBuf);
	int nIndex = 0;

	for (int i = nLen-2; i > 0; i--)
	{
		if (sBuf[i] == '\\' || sBuf[i] == '/')
		{
			nIndex = i+1;
			i      = 0;
		}
	}

	if (nIndex < 0) nIndex = 0;
	if (nIndex >= nLen) nIndex = 0;

	strcpy(sLevel, &sBuf[nIndex]);


	// Get the type of this game...

	char sTemp[NML_NAME];
	int  nType;

	if (!Sparam_Get(sTemp, sInfo, NST_GAMETYPE))
	{
		return(NULL);
	}

	nType = atoi(sTemp);


	// Create a new game object...

	CNinfoGame* pGame = new CNinfoGame();
	if (!pGame) return(NULL);


	// Init the new game object...

	if (!pGame->Init(m_pClientDE, sName, sHost, sLevel, nType, pNetSession, dwNumDPlayPlayers, pNetSession->m_HostIP, pNetSession->m_HostPort))
	{
		delete pGame;
		return(NULL);
	}


	// Add each player in this game...

	int count = 0;

	if (Sparam_Get(sTemp, sInfo, NST_PLRCOUNT))
	{
		count = atoi(sTemp);
	}

	for (i = 1; i <= count; i++)
	{
		char sBase[32];
		wsprintf(sBase, "%s%i", NST_PLRNAME_BASE, i);

		if (Sparam_Get(sName, sInfo, sBase))
		{
			wsprintf(sBase, "%s%i", NST_PLRFRAG_BASE, i);

			if (Sparam_Get(sTemp, sInfo, sBase))
			{
				pGame->AddPlayer(sName, atoi(sTemp));
			}
		}
	}


	// Add the game to our internal collection...

	if (!AddGame(pGame))
	{
		delete pGame;
		return(NULL);
	}


	// All done...

	return(pGame);
}

BOOL CNinfoMgr::AddGame(CNinfoGame* pGame)
{
	// Sanity checks...

	if (!pGame) return(FALSE);
	if (m_cGames >= NML_GAMES) return(FALSE);


	// Add the game to our internal collection...

	m_aGames[m_cGames] = pGame;
	m_cGames++;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoMgr::GetFirstGame
//
//	PURPOSE:	Gets the first game in our internal collection
//
// ----------------------------------------------------------------------- //

CNinfoGame* CNinfoMgr::GetFirstGame()
{
	// Sanity checks...

	if (m_cGames == 0) return(NULL);


	// Get the first game and prepare for iterating...

	m_iGame = 0;
	return(m_aGames[m_iGame]);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoMgr::GetNextGame
//
//	PURPOSE:	Gets the next game in our internal collection
//
//	WARNING:	You must call GetFirstGame() before calling this function
//
// ----------------------------------------------------------------------- //

CNinfoGame* CNinfoMgr::GetNextGame()
{
	// Sanity checks...

	if (m_iGame+1 >= m_cGames) return(NULL);


	// Get the first game and prepare for iterating...

	m_iGame++;
	return(m_aGames[m_iGame]);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoMgr::RemoveGames
//
//	PURPOSE:	Removes all the games in our internal collection
//
// ----------------------------------------------------------------------- //

void CNinfoMgr::RemoveGames()
{
	// Remove each game in our internal collection...

	for (int i = 0; i < m_cGames; i++)
	{
		CNinfoGame* pGame = m_aGames[i];
		assert(pGame && pGame->IsValid());

		if (pGame) delete pGame;
	}


	// Reset our internal collection...

	m_cGames = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoMgr::CreateSessionString
//
//	PURPOSE:	Static function to fill in a string with all the given
//				game info.
//
// ----------------------------------------------------------------------- //

BOOL CNinfoMgr::CreateSessionString(char* sString, char* sName, char* sHost, char* sLevel, int nType)
{
	// Sanity checks...

	if (!sString) return(FALSE);
	if (!sName) return(FALSE);


	// Set the name info...

	Sparam_Add(sString, NST_GAMENAME, sName);
	Sparam_Add(sString, NST_GAMEHOST, sHost);
	Sparam_Add(sString, NST_GAMELEVEL, sLevel);
	Sparam_Add(sString, NST_GAMETYPE, nType);
	Sparam_Add(sString, NST_GAMETIME, 0);


	// All done...

	return(TRUE);
}


/* *********************************************************************** */
/* CNinfoGame                                                              */

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoGame::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CNinfoGame::Init(CClientDE* pClientDE, char* sName, char* sHost, char* sLevel, int nType, NetSession* pNetSession, DWORD dwNumDPlayPlayers, char *pHostIP, DWORD hostPort)
{
	// Sanity checks...

	if (!pClientDE) return(FALSE);
	if (!sName) return(FALSE);
	if (!sLevel) return(FALSE);
	if (!pNetSession) return(FALSE);


	// Set simple members...

	Clear();

	strcpy(m_sName, sName);
	strcpy(m_sHost, sHost);
	strcpy(m_sLevel, sLevel);
	SAFE_STRCPY(m_sHostIP, pHostIP);
	m_dwHostPort = hostPort;

	m_nType         = nType;
	m_pNetSession   = pNetSession;
	m_pClientDE     = pClientDE;
	m_cDPlayPlayers = dwNumDPlayPlayers;
	m_nPing         = pNetSession->m_Ping;

	if (m_cDPlayPlayers > 0) m_cDPlayPlayers--;	// don't count the server player


	// Set string helper members...

	int nStringID = IDS_GAMETYPE_UNKNOWN;

	switch (m_nType)
	{
		case NGT_DEATHMATCH:	nStringID = IDS_GAMETYPE_DEATHMATCH; break;
		case NGT_CAPTUREFLAG:	nStringID = IDS_GAMETYPE_CAPTUREFLAG; break;
		case NGT_TEAMS:			nStringID = IDS_GAMETYPE_TEAM; break;

#ifdef _ADDON
		case NGT_SOCCER:		nStringID = IDS_GAMETYPE_SOCCER; break;
		case NGT_TOETAG:		nStringID = IDS_GAMETYPE_TOETAG; break;
#endif

	}

	HSTRING hString = m_pClientDE->FormatString(nStringID);

	if (hString)
	{
		_mbsncpy((unsigned char*)m_sType, (const unsigned char*)m_pClientDE->GetStringData(hString), 30);
		m_pClientDE->FreeString(hString);
	}


	// Calculate a CRC value...

	m_dwCRC = m_pNetSession->m_guidInst.a;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoGame::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CNinfoGame::Term()
{
	RemovePlayers();
	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoGame::AddPlayer
//
//	PURPOSE:	Adds a new player object
//
// ----------------------------------------------------------------------- //

CNinfoPlayer* CNinfoGame::AddPlayer(char* sName, int nFrags)
{
	// Sanity checks...

	if (!sName) return(NULL);


	// Create a new player object...

	CNinfoPlayer* pPlayer = new CNinfoPlayer();
	if (!pPlayer) return(NULL);


	// Init the new player object...

	if (!pPlayer->Init(sName, nFrags))
	{
		delete pPlayer;
		return(NULL);
	}


	// Add the game to our internal collection...

	if (!AddPlayer(pPlayer))
	{
		delete pPlayer;
		return(NULL);
	}


	// All done...

	return(pPlayer);
}

BOOL CNinfoGame::AddPlayer(CNinfoPlayer* pPlayer)
{
	// Sanity checks...

	if (!pPlayer) return(FALSE);
	if (m_cPlayers >= NML_PLAYERS) return(FALSE);


	// Add the Player to our internal collection...

	m_aPlayers[m_cPlayers] = pPlayer;
	m_cPlayers++;


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoGame::GetFirstPlayer
//
//	PURPOSE:	Gets the first Player in our internal collection
//
// ----------------------------------------------------------------------- //

CNinfoPlayer* CNinfoGame::GetFirstPlayer()
{
	// Sanity checks...

	if (m_cPlayers == 0) return(NULL);


	// Get the first Player and prepare for iterating...

	m_iPlayer = 0;
	return(m_aPlayers[m_iPlayer]);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoGame::GetNextPlayer
//
//	PURPOSE:	Gets the next Player in our internal collection
//
//	WARNING:	You must call GetFirstPlayer() before calling this function
//
// ----------------------------------------------------------------------- //

CNinfoPlayer* CNinfoGame::GetNextPlayer()
{
	// Sanity checks...

	if (m_iPlayer+1 >= m_cPlayers) return(NULL);


	// Get the first Player and prepare for iterating...

	m_iPlayer++;
	return(m_aPlayers[m_iPlayer]);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoGame::RemovePlayers
//
//	PURPOSE:	Removes all the Players in our internal collection
//
// ----------------------------------------------------------------------- //

void CNinfoGame::RemovePlayers()
{
	// Remove each Player in our internal collection...

	for (int i = 0; i < m_cPlayers; i++)
	{
		CNinfoPlayer* pPlayer = m_aPlayers[i];
		assert(pPlayer);

		if (pPlayer) delete pPlayer;
	}


	// Reset our internal collection...

	m_cPlayers = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoGame::GetNameWithPing
//
//	PURPOSE:	Gets the name and ping time if available
//
// ----------------------------------------------------------------------- //

char* CNinfoGame::GetNameWithPing()
{
	// Build the string to use...
	if(m_nPing <= 0)
		wsprintf(s_sNameWithPing, "[-] %s [%s:%d]", GetName(), GetHostIP(), GetHostPort());
	else
		wsprintf(s_sNameWithPing, "[%i] %s [%s:%d]", GetPing(), GetName(), GetHostIP(), GetHostPort());


	// All done...

	return(s_sNameWithPing);
}


/* *********************************************************************** */
/* CNinfoPlayer                                                            */

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CNinfoPlayer::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

BOOL CNinfoPlayer::Init(char* sName, int nFrags)
{
	// Sanity checks...

	if (!sName) return(FALSE);


	// Set simple members...

	Clear();

	strcpy(m_sName, sName);

	m_nFrags = nFrags;


	// All done...

	return(TRUE);
}


