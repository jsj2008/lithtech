/****************************************************************************
;
;	 MODULE:		TEAMMGR (.CPP)
;
;	PURPOSE:		Team Manager for multiplayer teams
;
;	HISTORY:		12/21/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/

// Includes...

#include "TeamMgr.h"


// Functions...

// *********************************************************************** //
// CTeamPlayer

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamPlayer::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

DBOOL CTeamPlayer::Init(DDWORD dwPlayerID, DDWORD dwFlags)
{
	// Set simple members...

	m_dwID    = dwPlayerID;
	m_dwFlags = dwFlags;


	// All done...

	return(DTRUE);
}


// *********************************************************************** //
// CTeam

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

DBOOL CTeam::Init(DDWORD dwTeamID, char* sTeamName, DDWORD dwFlags)
{
	// Clear simple members...

	Clear();


	// Set simple members...

	m_dwID    = dwTeamID;
	m_dwFlags = dwFlags;

	strncpy(m_sName, sTeamName, TM_MAX_NAME);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CTeam::Term()
{
	// Delete all of the players...

	DeleteAllPlayers();


	// Clear simple members...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::Reset
//
//	PURPOSE:	Resets our values and all of the players in the list
//
// ----------------------------------------------------------------------- //

void CTeam::Reset()
{
	// Reset our score values...

	m_nScore  = 0;
	m_nFrags  = 0;
	m_nDeaths = 0;


	// Reset all of the players...

	ResetAllPlayers();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddPlayer
//
//	PURPOSE:	Adds the given player to the team
//
// ----------------------------------------------------------------------- //

CTeamPlayer* CTeam::AddPlayer(DDWORD dwPlayerID, DDWORD dwFlags)
{
	// Make sure this player doesn't already exist...

	if (GetPlayer(dwPlayerID))
	{
		return(NULL);
	}


	// Create and init a new player...

	CTeamPlayer* pPlayer = new CTeamPlayer;
	if (!pPlayer) return(NULL);

	if (!pPlayer->Init(dwPlayerID, dwFlags))
	{
		delete pPlayer;
		return(DFALSE);
	}


	// Add the new player to our list...

	if (!AddPlayer(pPlayer))
	{
		return(DFALSE);
	}


	// All done...

	return(pPlayer);
}

DBOOL CTeam::AddPlayer(CTeamPlayer* pPlayer)
{
	// Sanity checks...

	if (!pPlayer) return(DFALSE);


	// Add the player to our list...

	m_lsPlayers.InsertLast(pPlayer);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::RemovePlayer
//
//	PURPOSE:	Removes the given player from the team
//
// ----------------------------------------------------------------------- //

DBOOL CTeam::RemovePlayer(DDWORD dwPlayerID)
{
	// Get this player...

	CTeamPlayer* pPlayer = GetPlayer(dwPlayerID);
	if (!pPlayer) return(DFALSE);


	// Remove this player...

	return(RemovePlayer(pPlayer));
}

DBOOL CTeam::RemovePlayer(CTeamPlayer* pPlayer)
{
	// Sanity checks...

	if (!pPlayer) return(DFALSE);


	// Delete this player from the list...

	m_lsPlayers.Delete(pPlayer);


	// Delete the player object...

	delete pPlayer;


	// All done...

	return(DTRUE);
}

DBOOL CTeam::ChangePlayerTeam(DDWORD dwPlayerID, CTeam* pNewTeam, DBOOL bReset)
{
	// Sanity checks...

	if (!pNewTeam) return(DFALSE);


	// Get the player...

	CTeamPlayer* pPlayer = GetPlayer(dwPlayerID);
	if (!pPlayer) return(DFALSE);


	// Change the player's team...

	return(ChangePlayerTeam(pPlayer, pNewTeam, bReset));

}

DBOOL CTeam::ChangePlayerTeam(CTeamPlayer* pPlayer, CTeam* pNewTeam, DBOOL bReset)
{
	// Sanity checks...

	if (!pPlayer) return(DFALSE);
	if (!pNewTeam) return(DFALSE);


	// Delete this player from the list...

	m_lsPlayers.Delete(pPlayer);


	// Reset this player if requested...

	if (bReset)
	{
		pPlayer->Reset();
	}


	// Add this player to the new team...

	if (!pNewTeam->AddPlayer(pPlayer))
	{
		return(DFALSE);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::GetPlayer
//
//	PURPOSE:	Gets the player with the given client handle
//
// ----------------------------------------------------------------------- //

CTeamPlayer* CTeam::GetPlayer(DDWORD dwPlayerID)
{
	// Look for this player...

	CTeamPlayer* pPlayer = GetFirstPlayer();

	while (pPlayer)
	{
		if (pPlayer->GetID() == dwPlayerID)
		{
			return(pPlayer);
		}

		pPlayer = GetNextPlayer(pPlayer);
	}


	// If we get here, this player is not in the team...

	return(NULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::SetScore
//
//	PURPOSE:	Sets the given score value
//
// ----------------------------------------------------------------------- //

DBOOL CTeam::SetScore(int nScore)
{
	// Set the given score value...

	m_nScore = nScore;

	
	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddScore
//
//	PURPOSE:	Adds the given score value to the team score
//
// ----------------------------------------------------------------------- //

DBOOL CTeam::AddScore(int nScore)
{
	// Add the given score value...

	m_nScore += nScore;

	
	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddFrags
//
//	PURPOSE:	Adds the given frags to the team score
//
// ----------------------------------------------------------------------- //

DBOOL CTeam::AddFrags(DDWORD dwPlayerID, int nFrags)
{
	// Find the player...

	CTeamPlayer* pPlayer = GetPlayer(dwPlayerID);
	if (!pPlayer) return(DFALSE);


	// Add the frags to the player and the team...

	pPlayer->AddFrags(nFrags);
	m_nFrags += nFrags;


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddDeaths
//
//	PURPOSE:	Adds the given deaths to the team score
//
// ----------------------------------------------------------------------- //

DBOOL CTeam::AddDeaths(DDWORD dwPlayerID, int nDeaths)
{
	// Find the player...

	CTeamPlayer* pPlayer = GetPlayer(dwPlayerID);
	if (!pPlayer) return(DFALSE);


	// Add the frags to the player and the team...

	pPlayer->AddDeaths(nDeaths);
	m_nDeaths += nDeaths;


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::ResetPlayers
//
//	PURPOSE:	Resets each player in the team
//
// ----------------------------------------------------------------------- //

void CTeam::ResetAllPlayers()
{
	CTeamPlayer* pPlayer = m_lsPlayers.GetFirst();

	while (pPlayer)
	{
		pPlayer->Reset();
		pPlayer = pPlayer->GetNext();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::SortPlayers
//
//	PURPOSE:	Sorts the players using the given sort key
//
// ----------------------------------------------------------------------- //

void CTeam::SortPlayers(int nSortKey, int nDir)
{
	// Sanity checks...

	if (nSortKey == TM_KEY_SCORE) return;
	if (GetNumPlayers() <= 1) return;


	// Remove each player from our list, and insert it, sorted, into a temp list...

	CTeamPlayerList lsTemp;

	CTeamPlayer* pCurPlr = GetFirstPlayer();

	while (pCurPlr)
	{
		CTeamPlayer* pNextPlr = GetNextPlayer(pCurPlr);

		m_lsPlayers.Delete(pCurPlr);


		// Insert the player from our main list, sorted, into our temp list...

		CTeamPlayer* pTmpPlr = lsTemp.GetFirst();

		if (!pTmpPlr)
		{
			lsTemp.Insert(pCurPlr);
		}
		else
		{
			while (pTmpPlr)
			{
				if (nDir == TM_SORT_DESCENDING && IsGreater(pCurPlr, pTmpPlr, nSortKey))
				{
					lsTemp.InsertBefore(pTmpPlr, pCurPlr);
					pTmpPlr = NULL;
				}
				else if (nDir == TM_SORT_ASCENDING && IsLess(pCurPlr, pTmpPlr, nSortKey))
				{
					lsTemp.InsertBefore(pTmpPlr, pCurPlr);
					pTmpPlr = NULL;
				}
				else
				{
					pTmpPlr = pTmpPlr->GetNext();
					if (!pTmpPlr)
					{
						lsTemp.InsertLast(pCurPlr);
					}
				}
			}
		}

		pCurPlr = pNextPlr;
	}


	// Re-insert all items into our main list...

	CTeamPlayer* pPlr = lsTemp.GetFirst();

	while (pPlr)
	{
		CTeamPlayer* pNextPlr = pPlr->GetNext();
		m_lsPlayers.InsertLast(pPlr);
		pPlr = pNextPlr;
	}
}


// *********************************************************************** //
// CTeamMgr

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::Init()
{
	// Clear all members...

	Clear();


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CTeamMgr::Term()
{
	// Delete all of the teams...

	DeleteAllTeams();


	// Clear all members...

	Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::GetTeam
//
//	PURPOSE:	Gets the team with the given ID
//
// ----------------------------------------------------------------------- //

CTeam* CTeamMgr::GetTeam(DDWORD dwTeamID)
{
	// Look for a team with the given ID...

	CTeam* pTeam = GetFirstTeam();

	while (pTeam)
	{
		if (pTeam->GetID() == dwTeamID)
		{
			return(pTeam);
		}

		pTeam = GetNextTeam(pTeam);
	}


	// If we get here, we didn't find the team...

	return(NULL);
}

CTeam* CTeamMgr::GetTeam(const char* sName)
{
	// Sanity checks...

	if (!sName) return(NULL);
	if (sName[0] == '\0') return(NULL);


	// Look for a team with the given name...

	CTeam* pTeam = GetFirstTeam();

	while (pTeam)
	{
		if (strncmp(pTeam->GetName(), sName, TM_MAX_NAME) == 0)
		{
			return(pTeam);
		}

		pTeam = GetNextTeam(pTeam);
	}


	// If we get here, we didn't find the team...

	return(NULL);
}

CTeam* CTeamMgr::GetTeamFromPlayerID(DDWORD dwPlayerID)
{
	// Look for the player on each team...

	CTeam* pTeam = GetFirstTeam();

	while (pTeam)
	{
		CTeamPlayer* pPlayer = pTeam->GetFirstPlayer();

		while (pPlayer)
		{
			if (pPlayer->GetID() == dwPlayerID)
			{
				return(pTeam);
			}

			pPlayer = pTeam->GetNextPlayer(pPlayer);
		}

		pTeam = GetNextTeam(pTeam);
	}


	// If we get here, we didn't find the player's team...

	return(NULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddTeam
//
//	PURPOSE:	Adds a new team with the given values
//
// ----------------------------------------------------------------------- //

CTeam* CTeamMgr::AddTeam(DDWORD dwTeamID, char* sTeamName, DDWORD dwFlags)
{
	// Make sure this team doesn't already exist...

	if (GetTeam(dwTeamID)) return(NULL);
	if (GetTeam(sTeamName)) return(NULL);


	// Create and init a new team...

	CTeam* pTeam = new CTeam();
	if (!pTeam) return(NULL);

	if (!pTeam->Init(dwTeamID, sTeamName, dwFlags))
	{
		delete pTeam;
		return(NULL);
	}


	// Add the new team to our list...

	m_lsTeams.InsertLast(pTeam);


	// All done...

	return(pTeam);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddPlayer
//
//	PURPOSE:	Adds a new player to the specified team
//
// ----------------------------------------------------------------------- //

CTeamPlayer* CTeamMgr::AddPlayer(DDWORD dwTeamID, DDWORD dwPlayerID, DDWORD dwFlags)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Add the player to the team...

	return(AddPlayer(pTeam, dwPlayerID, dwFlags));
}

CTeamPlayer* CTeamMgr::AddPlayer(char* sTeamName, DDWORD dwPlayerID, DDWORD dwFlags)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Add the player to the team...

	return(AddPlayer(pTeam, dwPlayerID, dwFlags));
}

CTeamPlayer* CTeamMgr::AddPlayer(CTeam* pTeam, DDWORD dwPlayerID, DDWORD dwFlags)
{
	// Sanity checks...

	if (!pTeam) return(NULL);


	// Add the player to the team...	

	return(pTeam->AddPlayer(dwPlayerID, dwFlags));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::SetTeamScore
//
//	PURPOSE:	Sets the given score value for the specified team
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::SetTeamScore(DDWORD dwTeamID, int nScore)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Set the score...

	return(SetTeamScore(pTeam, nScore));
}

DBOOL CTeamMgr::SetTeamScore(char* sTeamName, int nScore)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Set the score...

	return(SetTeamScore(pTeam, nScore));
}

DBOOL CTeamMgr::SetTeamScore(CTeam* pTeam, int nScore)
{
	// Sanity checks...

	if (!pTeam) return(DFALSE);


	// Set the score...

	pTeam->SetScore(nScore);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddTeamScore
//
//	PURPOSE:	Adds the given score value to the specified team
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::AddTeamScore(DDWORD dwTeamID, int nScore)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Add the score...

	return(AddTeamScore(pTeam, nScore));
}

DBOOL CTeamMgr::AddTeamScore(char* sTeamName, int nScore)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Add the score...

	return(AddTeamScore(pTeam, nScore));
}

DBOOL CTeamMgr::AddTeamScore(CTeam* pTeam, int nScore)
{
	// Sanity checks...

	if (!pTeam) return(DFALSE);


	// Add the score...

	pTeam->AddScore(nScore);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddPlayerFrags
//
//	PURPOSE:	Adds the given frags to the given team and player
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::AddPlayerFrags(DDWORD dwPlayerID, int nFrags)
{
	// Get the team...

	CTeam* pTeam = GetTeamFromPlayerID(dwPlayerID);
	if (!pTeam) return(DFALSE);


	// Add the player frags...

	return(AddPlayerFrags(pTeam, dwPlayerID, nFrags));
}

DBOOL CTeamMgr::AddPlayerFrags(DDWORD dwTeamID, DDWORD dwPlayerID, int nFrags)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Add the player frags...

	return(AddPlayerFrags(pTeam, dwPlayerID, nFrags));
}

DBOOL CTeamMgr::AddPlayerFrags(char* sTeamName, DDWORD dwPlayerID, int nFrags)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Add the player frags...

	return(AddPlayerFrags(pTeam, dwPlayerID, nFrags));
}

DBOOL CTeamMgr::AddPlayerFrags(CTeam* pTeam, DDWORD dwPlayerID, int nFrags)
{
	// Sanity checks...

	if (!pTeam) return(DFALSE);


	// Add the player frags...

	pTeam->AddFrags(dwPlayerID, nFrags);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddPlayerDeaths
//
//	PURPOSE:	Adds the given deaths to the given team and player
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::AddPlayerDeaths(DDWORD dwPlayerID, int nDeaths)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwPlayerID);
	if (!pTeam) return(DFALSE);


	// Add the player deaths...

	return(AddPlayerDeaths(pTeam, dwPlayerID, nDeaths));
}

DBOOL CTeamMgr::AddPlayerDeaths(DDWORD dwTeamID, DDWORD dwPlayerID, int nDeaths)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Add the player deaths...

	return(AddPlayerDeaths(pTeam, dwPlayerID, nDeaths));
}

DBOOL CTeamMgr::AddPlayerDeaths(char* sTeamName, DDWORD dwPlayerID, int nDeaths)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Add the player deaths...

	return(AddPlayerDeaths(pTeam, dwPlayerID, nDeaths));
}

DBOOL CTeamMgr::AddPlayerDeaths(CTeam* pTeam, DDWORD dwPlayerID, int nDeaths)
{
	// Sanity checks...

	if (!pTeam) return(DFALSE);


	// Add the player deaths...

	pTeam->AddDeaths(dwPlayerID, nDeaths);


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::RemoveTeam
//
//	PURPOSE:	Removes the given team
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::RemoveTeam(DDWORD dwTeamID)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(DFALSE);


	// Remove the team...

	return(RemoveTeam(pTeam));
}

DBOOL CTeamMgr::RemoveTeam(char* sTeamName)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(DFALSE);


	// Remove the team...

	return(RemoveTeam(pTeam));
}

DBOOL CTeamMgr::RemoveTeam(CTeam* pTeam)
{
	// Sanity checks...

	if (!pTeam) return(DFALSE);


	// Delete the team from the list...

	m_lsTeams.Delete(pTeam);
	delete pTeam;


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::RemovePlayer
//
//	PURPOSE:	Removes the given player from the team it's on
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::RemovePlayer(DDWORD dwPlayerID)
{
	// Find the team this player is on...

	CTeam* pTeam = GetTeamFromPlayerID(dwPlayerID);
	if (!pTeam) return(DFALSE);


	// Remove the player from our team-trans list...

	RemoveTeamTransID(dwPlayerID);


	// Remove the player from this team...

	return(pTeam->RemovePlayer(dwPlayerID));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::ResetAllTeams
//
//	PURPOSE:	Resets all of the team in the list
//
// ----------------------------------------------------------------------- //

void CTeamMgr::ResetAllTeams()
{
	CTeam* pTeam = m_lsTeams.GetFirst();

	while (pTeam)
	{
		pTeam->Reset();
		pTeam = pTeam->GetNext();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::SortTeams
//
//	PURPOSE:	Sorts the teams using the given sort key
//
// ----------------------------------------------------------------------- //

void CTeamMgr::SortTeams(int nSortKey, int nDir)
{
	// Sanity check...

	if (GetNumTeams() <= 1) return;


	// Remove each team from our list, and insert it, sorted, into a temp list...

	CTeamList lsTemp;

	CTeam* pCurTeam = GetFirstTeam();

	while (pCurTeam)
	{
		CTeam* pNextTeam = GetNextTeam(pCurTeam);

		m_lsTeams.Delete(pCurTeam);


		// Insert the team from our main list, sorted, into our temp list...

		CTeam* pTmpTeam = lsTemp.GetFirst();

		if (!pTmpTeam)
		{
			lsTemp.Insert(pCurTeam);
		}
		else
		{
			while (pTmpTeam)
			{
				if (nDir == TM_SORT_DESCENDING && IsGreater(pCurTeam, pTmpTeam, nSortKey))
				{
					lsTemp.InsertBefore(pTmpTeam, pCurTeam);
					pTmpTeam = NULL;
				}
				else if (nDir == TM_SORT_ASCENDING && IsLess(pCurTeam, pTmpTeam, nSortKey))
				{
					lsTemp.InsertBefore(pTmpTeam, pCurTeam);
					pTmpTeam = NULL;
				}
				else
				{
					pTmpTeam = pTmpTeam->GetNext();
					if (!pTmpTeam)
					{
						lsTemp.InsertLast(pCurTeam);
					}
				}
			}
		}

		pCurTeam = pNextTeam;
	}


	// Re-insert all items into our main list...

	CTeam* pTeam = lsTemp.GetFirst();

	while (pTeam)
	{
		CTeam* pNextTeam = pTeam->GetNext();
		m_lsTeams.InsertLast(pTeam);
		pTeam = pNextTeam;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::SortPlayers
//
//	PURPOSE:	Sorts the players on each team using the given sort key
//
// ----------------------------------------------------------------------- //

void CTeamMgr::SortPlayers(int nSortKey, int nDir)
{
	// Sort the players on each team...

	CTeam* pTeam = GetFirstTeam();

	while (pTeam)
	{
		pTeam->SortPlayers(nSortKey, nDir);

		pTeam = GetNextTeam(pTeam);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::ChangePlayerTeam
//
//	PURPOSE:	Removes the player from its current team and adds it
//				to the specified team.
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::ChangePlayerTeam(DDWORD dwPlayerID, DDWORD dwNewTeamID, DBOOL bReset)
{
	// Get the new team...

	CTeam* pNewTeam = GetTeam(dwNewTeamID);
	if (!pNewTeam) return(DFALSE);


	// Change the player's team...

	return(ChangePlayerTeam(dwPlayerID, pNewTeam, bReset));
}

DBOOL CTeamMgr::ChangePlayerTeam(DDWORD dwPlayerID, char* sNewTeamName, DBOOL bReset)
{
	// Get the new team...

	CTeam* pNewTeam = GetTeam(sNewTeamName);
	if (!pNewTeam) return(DFALSE);


	// Change the player's team...

	return(ChangePlayerTeam(dwPlayerID, pNewTeam, bReset));
}

DBOOL CTeamMgr::ChangePlayerTeam(DDWORD dwPlayerID, CTeam* pNewTeam, DBOOL bReset)
{
	// Sanity checks...

	if (!pNewTeam) return(DFALSE);


	// Get the current team for the player...

	CTeam* pCurTeam = GetTeamFromPlayerID(dwPlayerID);
	if (!pCurTeam) return(DFALSE);


	// Change the player's team to the new one...

	if (pCurTeam->ChangePlayerTeam(dwPlayerID, pNewTeam, bReset))
	{
		return(DFALSE);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::GetTeamWithMostPlayers
//
//	PURPOSE:	Gets the team that has the most players
//
// ----------------------------------------------------------------------- //

CTeam* CTeamMgr::GetTeamWithMostPlayers(DBOOL bRandomizeTies)
{
	// Look for the team with the most players...

	int    cPlayers  = -1;
	CTeam* pMostTeam = NULL;

	CTeam* pTeam = GetFirstTeam();

	while (pTeam)
	{
		if (pTeam->GetNumPlayers() > cPlayers)
		{
			pMostTeam = pTeam;
			cPlayers  = pTeam->GetNumPlayers();
		}
		else if (pTeam->GetNumPlayers() == cPlayers)
		{
			if (bRandomizeTies)
			{
				if ((rand() % 100) > 50)
				{
					pMostTeam = pTeam;
					cPlayers  = pTeam->GetNumPlayers();
				}
			}
		}

		pTeam = GetNextTeam(pTeam);
	}


	// All done...

	return(pMostTeam);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::GetTeamWithLeastPlayers
//
//	PURPOSE:	Gets the team that has the least players
//
// ----------------------------------------------------------------------- //

CTeam* CTeamMgr::GetTeamWithLeastPlayers(DBOOL bRandomizeTies)
{
	// Look for the team with the most players...

	int    cPlayers   = 9999999;
	CTeam* pLeastTeam = NULL;

	CTeam* pTeam = GetFirstTeam();

	while (pTeam)
	{
		if (pTeam->GetNumPlayers() < cPlayers)
		{
			pLeastTeam = pTeam;
			cPlayers   = pTeam->GetNumPlayers();
		}
		else if (pTeam->GetNumPlayers() == cPlayers)
		{
			if (bRandomizeTies)
			{
				if ((rand() % 100) > 50)
				{
					pLeastTeam = pTeam;
					cPlayers   = pTeam->GetNumPlayers();
				}
			}
		}

		pTeam = GetNextTeam(pTeam);
	}


	// All done...

	return(pLeastTeam);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::CreateTeamTransIDs
//
//	PURPOSE:	Creates a list of player team IDs
//
// ----------------------------------------------------------------------- //

DBOOL CTeamMgr::CreateTeamTransIDs()
{
	// Reset the list...

	ClearTeamTransIDs();


	// Write out each player ID on each team...

	CTeam* pTeam = GetFirstTeam();

	while (pTeam)
	{
		DDWORD       dwTeamID = pTeam->GetID();
		CTeamPlayer* pPlayer  = pTeam->GetFirstPlayer();

		while (pPlayer)
		{
			DDWORD dwPlayerID = pPlayer->GetID();

			if (dwPlayerID < TM_MAX_PLAYERS)
			{
				m_aTeamTransIDs[dwPlayerID] = dwTeamID;
			}

			pPlayer = pTeam->GetNextPlayer(pPlayer);
		}

		pTeam = GetNextTeam(pTeam);
	}


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::GetTeamTransID
//
//	PURPOSE:	Gets a level transition team ID from the given player ID
//
// ----------------------------------------------------------------------- //

DDWORD CTeamMgr::GetTeamTransID(DDWORD dwPlayerID)
{
	// Look for this player's previous team ID in our arrary...

	if (dwPlayerID < TM_MAX_PLAYERS)
	{
		return(m_aTeamTransIDs[dwPlayerID]);
	}
	else
	{
		return(0);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::RemoveTeamTransID
//
//	PURPOSE:	Removes the given player ID from the team transition info
//
// ----------------------------------------------------------------------- //

void CTeamMgr::RemoveTeamTransID(DDWORD dwPlayerID)
{
	// Remove this player ID entry in our array...

	if (dwPlayerID < TM_MAX_PLAYERS)
	{
		m_aTeamTransIDs[dwPlayerID] = TM_ID_NULL;
	}
}
