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

#include "stdafx.h"
#include "teammgr.h"


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

LTBOOL CTeamPlayer::Init(uint32 dwPlayerID, uint32 dwFlags)
{
	// Set simple members...

	m_dwID    = dwPlayerID;
	m_dwFlags = dwFlags;


	// All done...

    return(LTTRUE);
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

LTBOOL CTeam::Init(uint32 dwTeamID, char* sTeamName, uint32 dwFlags)
{
	// Clear simple members...

	Clear();


	// Set simple members...

	m_dwID    = dwTeamID;
	m_dwFlags = dwFlags;

	strncpy(m_sName, sTeamName, TM_MAX_NAME);


	// All done...

    return(LTTRUE);
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

CTeamPlayer* CTeam::AddPlayer(uint32 dwPlayerID, uint32 dwFlags)
{
	// Make sure this player doesn't already exist...

	if (GetPlayer(dwPlayerID))
	{
		return(NULL);
	}


	// Create and init a new player...

	CTeamPlayer* pPlayer = debug_new(CTeamPlayer);
	if (!pPlayer) return(NULL);

	if (!pPlayer->Init(dwPlayerID, dwFlags))
	{
		debug_delete(pPlayer);
        return(LTFALSE);
	}


	// Add the new player to our list...

	if (!AddPlayer(pPlayer))
	{
        return(LTFALSE);
	}


	// All done...

	return(pPlayer);
}

LTBOOL CTeam::AddPlayer(CTeamPlayer* pPlayer)
{
	// Sanity checks...

    if (!pPlayer) return(LTFALSE);


	// Add the player to our list...

	m_lsPlayers.InsertLast(pPlayer);


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::RemovePlayer
//
//	PURPOSE:	Removes the given player from the team
//
// ----------------------------------------------------------------------- //

LTBOOL CTeam::RemovePlayer(uint32 dwPlayerID)
{
	// Get this player...

	CTeamPlayer* pPlayer = GetPlayer(dwPlayerID);
    if (!pPlayer) return(LTFALSE);


	// Remove this player...

	return(RemovePlayer(pPlayer));
}

LTBOOL CTeam::RemovePlayer(CTeamPlayer* pPlayer)
{
	// Sanity checks...

    if (!pPlayer) return(LTFALSE);


	// Delete this player from the list...

	m_lsPlayers.Delete(pPlayer);


	// Delete the player object...

	debug_delete(pPlayer);


	// All done...

    return(LTTRUE);
}

LTBOOL CTeam::ChangePlayerTeam(uint32 dwPlayerID, CTeam* pNewTeam, LTBOOL bReset)
{
	// Sanity checks...

    if (!pNewTeam) return(LTFALSE);


	// Get the player...

	CTeamPlayer* pPlayer = GetPlayer(dwPlayerID);
    if (!pPlayer) return(LTFALSE);


	// Change the player's team...

	return(ChangePlayerTeam(pPlayer, pNewTeam, bReset));

}

LTBOOL CTeam::ChangePlayerTeam(CTeamPlayer* pPlayer, CTeam* pNewTeam, LTBOOL bReset)
{
	// Sanity checks...

    if (!pPlayer) return(LTFALSE);
    if (!pNewTeam) return(LTFALSE);


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
        return(LTFALSE);
	}


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::GetPlayer
//
//	PURPOSE:	Gets the player with the given client handle
//
// ----------------------------------------------------------------------- //

CTeamPlayer* CTeam::GetPlayer(uint32 dwPlayerID)
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

LTBOOL CTeam::SetScore(int nScore)
{
	// Set the given score value...

	m_nScore = nScore;


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddScore
//
//	PURPOSE:	Adds the given score value to the team score
//
// ----------------------------------------------------------------------- //

LTBOOL CTeam::AddScore(int nScore)
{
	// Add the given score value...

	m_nScore += nScore;


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddFrags
//
//	PURPOSE:	Adds the given frags to the team score
//
// ----------------------------------------------------------------------- //

LTBOOL CTeam::AddFrags(uint32 dwPlayerID, int nFrags)
{
	// Find the player...

	CTeamPlayer* pPlayer = GetPlayer(dwPlayerID);
    if (!pPlayer) return(LTFALSE);


	// Add the frags to the player and the team...

	pPlayer->AddFrags(nFrags);
	m_nFrags += nFrags;


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddDeaths
//
//	PURPOSE:	Adds the given deaths to the team score
//
// ----------------------------------------------------------------------- //

LTBOOL CTeam::AddDeaths(uint32 dwPlayerID, int nDeaths)
{
	// Find the player...

	CTeamPlayer* pPlayer = GetPlayer(dwPlayerID);
    if (!pPlayer) return(LTFALSE);


	// Add the frags to the player and the team...

	pPlayer->AddDeaths(nDeaths);
	m_nDeaths += nDeaths;


	// All done...

    return(LTTRUE);
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

LTBOOL CTeamMgr::Init()
{
	// Clear all members...

	Clear();


	// All done...

    return(LTTRUE);
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

CTeam* CTeamMgr::GetTeam(uint32 dwTeamID)
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

CTeam* CTeamMgr::GetTeamFromPlayerID(uint32 dwPlayerID)
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

CTeam* CTeamMgr::AddTeam(uint32 dwTeamID, char* sTeamName, uint32 dwFlags)
{
	// Make sure this team doesn't already exist...

	if (GetTeam(dwTeamID)) return(NULL);
	if (GetTeam(sTeamName)) return(NULL);


	// Create and init a new team...

	CTeam* pTeam = debug_new(CTeam);
	if (!pTeam) return(NULL);

	if (!pTeam->Init(dwTeamID, sTeamName, dwFlags))
	{
		debug_delete(pTeam);
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

CTeamPlayer* CTeamMgr::AddPlayer(uint32 dwTeamID, uint32 dwPlayerID, uint32 dwFlags)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Add the player to the team...

	return(AddPlayer(pTeam, dwPlayerID, dwFlags));
}

CTeamPlayer* CTeamMgr::AddPlayer(char* sTeamName, uint32 dwPlayerID, uint32 dwFlags)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Add the player to the team...

	return(AddPlayer(pTeam, dwPlayerID, dwFlags));
}

CTeamPlayer* CTeamMgr::AddPlayer(CTeam* pTeam, uint32 dwPlayerID, uint32 dwFlags)
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

LTBOOL CTeamMgr::SetTeamScore(uint32 dwTeamID, int nScore)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Set the score...

	return(SetTeamScore(pTeam, nScore));
}

LTBOOL CTeamMgr::SetTeamScore(char* sTeamName, int nScore)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Set the score...

	return(SetTeamScore(pTeam, nScore));
}

LTBOOL CTeamMgr::SetTeamScore(CTeam* pTeam, int nScore)
{
	// Sanity checks...

    if (!pTeam) return(LTFALSE);


	// Set the score...

	pTeam->SetScore(nScore);


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddTeamScore
//
//	PURPOSE:	Adds the given score value to the specified team
//
// ----------------------------------------------------------------------- //

LTBOOL CTeamMgr::AddTeamScore(uint32 dwTeamID, int nScore)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Add the score...

	return(AddTeamScore(pTeam, nScore));
}

LTBOOL CTeamMgr::AddTeamScore(char* sTeamName, int nScore)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Add the score...

	return(AddTeamScore(pTeam, nScore));
}

LTBOOL CTeamMgr::AddTeamScore(CTeam* pTeam, int nScore)
{
	// Sanity checks...

    if (!pTeam) return(LTFALSE);


	// Add the score...

	pTeam->AddScore(nScore);


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddPlayerFrags
//
//	PURPOSE:	Adds the given frags to the given team and player
//
// ----------------------------------------------------------------------- //

LTBOOL CTeamMgr::AddPlayerFrags(uint32 dwPlayerID, int nFrags)
{
	// Get the team...

	CTeam* pTeam = GetTeamFromPlayerID(dwPlayerID);
    if (!pTeam) return(LTFALSE);


	// Add the player frags...

	return(AddPlayerFrags(pTeam, dwPlayerID, nFrags));
}

LTBOOL CTeamMgr::AddPlayerFrags(uint32 dwTeamID, uint32 dwPlayerID, int nFrags)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Add the player frags...

	return(AddPlayerFrags(pTeam, dwPlayerID, nFrags));
}

LTBOOL CTeamMgr::AddPlayerFrags(char* sTeamName, uint32 dwPlayerID, int nFrags)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Add the player frags...

	return(AddPlayerFrags(pTeam, dwPlayerID, nFrags));
}

LTBOOL CTeamMgr::AddPlayerFrags(CTeam* pTeam, uint32 dwPlayerID, int nFrags)
{
	// Sanity checks...

    if (!pTeam) return(LTFALSE);


	// Add the player frags...

	pTeam->AddFrags(dwPlayerID, nFrags);


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddPlayerDeaths
//
//	PURPOSE:	Adds the given deaths to the given team and player
//
// ----------------------------------------------------------------------- //

LTBOOL CTeamMgr::AddPlayerDeaths(uint32 dwPlayerID, int nDeaths)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwPlayerID);
    if (!pTeam) return(LTFALSE);


	// Add the player deaths...

	return(AddPlayerDeaths(pTeam, dwPlayerID, nDeaths));
}

LTBOOL CTeamMgr::AddPlayerDeaths(uint32 dwTeamID, uint32 dwPlayerID, int nDeaths)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
	if (!pTeam) return(NULL);


	// Add the player deaths...

	return(AddPlayerDeaths(pTeam, dwPlayerID, nDeaths));
}

LTBOOL CTeamMgr::AddPlayerDeaths(char* sTeamName, uint32 dwPlayerID, int nDeaths)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
	if (!pTeam) return(NULL);


	// Add the player deaths...

	return(AddPlayerDeaths(pTeam, dwPlayerID, nDeaths));
}

LTBOOL CTeamMgr::AddPlayerDeaths(CTeam* pTeam, uint32 dwPlayerID, int nDeaths)
{
	// Sanity checks...

    if (!pTeam) return(LTFALSE);


	// Add the player deaths...

	pTeam->AddDeaths(dwPlayerID, nDeaths);


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::RemoveTeam
//
//	PURPOSE:	Removes the given team
//
// ----------------------------------------------------------------------- //

LTBOOL CTeamMgr::RemoveTeam(uint32 dwTeamID)
{
	// Get the team...

	CTeam* pTeam = GetTeam(dwTeamID);
    if (!pTeam) return(LTFALSE);


	// Remove the team...

	return(RemoveTeam(pTeam));
}

LTBOOL CTeamMgr::RemoveTeam(char* sTeamName)
{
	// Get the team...

	CTeam* pTeam = GetTeam(sTeamName);
    if (!pTeam) return(LTFALSE);


	// Remove the team...

	return(RemoveTeam(pTeam));
}

LTBOOL CTeamMgr::RemoveTeam(CTeam* pTeam)
{
	// Sanity checks...

    if (!pTeam) return(LTFALSE);


	// Delete the team from the list...

	m_lsTeams.Delete(pTeam);
	debug_delete(pTeam);


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::RemovePlayer
//
//	PURPOSE:	Removes the given player from the team it's on
//
// ----------------------------------------------------------------------- //

LTBOOL CTeamMgr::RemovePlayer(uint32 dwPlayerID)
{
	// Find the team this player is on...

	CTeam* pTeam = GetTeamFromPlayerID(dwPlayerID);
    if (!pTeam) return(LTFALSE);


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

LTBOOL CTeamMgr::ChangePlayerTeam(uint32 dwPlayerID, uint32 dwNewTeamID, LTBOOL bReset)
{
	// Get the new team...

	CTeam* pNewTeam = GetTeam(dwNewTeamID);
    if (!pNewTeam) return(LTFALSE);


	// Change the player's team...

	return(ChangePlayerTeam(dwPlayerID, pNewTeam, bReset));
}

LTBOOL CTeamMgr::ChangePlayerTeam(uint32 dwPlayerID, char* sNewTeamName, LTBOOL bReset)
{
	// Get the new team...

	CTeam* pNewTeam = GetTeam(sNewTeamName);
    if (!pNewTeam) return(LTFALSE);


	// Change the player's team...

	return(ChangePlayerTeam(dwPlayerID, pNewTeam, bReset));
}

LTBOOL CTeamMgr::ChangePlayerTeam(uint32 dwPlayerID, CTeam* pNewTeam, LTBOOL bReset)
{
	// Sanity checks...

    if (!pNewTeam) return(LTFALSE);


	// Get the current team for the player...

	CTeam* pCurTeam = GetTeamFromPlayerID(dwPlayerID);
    if (!pCurTeam) return(LTFALSE);


	// Change the player's team to the new one...

	if (!pCurTeam->ChangePlayerTeam(dwPlayerID, pNewTeam, bReset))
	{
        return(LTFALSE);
	}


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::GetTeamWithMostPlayers
//
//	PURPOSE:	Gets the team that has the most players
//
// ----------------------------------------------------------------------- //

CTeam* CTeamMgr::GetTeamWithMostPlayers(LTBOOL bRandomizeTies)
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

CTeam* CTeamMgr::GetTeamWithLeastPlayers(LTBOOL bRandomizeTies)
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

LTBOOL CTeamMgr::CreateTeamTransIDs()
{
	// Reset the list...

	ClearTeamTransIDs();


	// Write out each player ID on each team...

	CTeam* pTeam = GetFirstTeam();

	while (pTeam)
	{
        uint32       dwTeamID = pTeam->GetID();
		CTeamPlayer* pPlayer  = pTeam->GetFirstPlayer();

		while (pPlayer)
		{
            uint32 dwPlayerID = pPlayer->GetID();

			if (dwPlayerID < TM_MAX_PLAYERS)
			{
				m_aTeamTransIDs[dwPlayerID] = dwTeamID;
			}

			pPlayer = pTeam->GetNextPlayer(pPlayer);
		}

		pTeam = GetNextTeam(pTeam);
	}


	// All done...

    return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::GetTeamTransID
//
//	PURPOSE:	Gets a level transition team ID from the given player ID
//
// ----------------------------------------------------------------------- //

uint32 CTeamMgr::GetTeamTransID(uint32 dwPlayerID)
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

void CTeamMgr::RemoveTeamTransID(uint32 dwPlayerID)
{
	// Remove this player ID entry in our array...

	if (dwPlayerID < TM_MAX_PLAYERS)
	{
		m_aTeamTransIDs[dwPlayerID] = TM_ID_NULL;
	}
}