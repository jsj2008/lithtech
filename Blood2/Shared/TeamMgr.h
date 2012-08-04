/****************************************************************************
;
;	 MODULE:		TEAMMGR (.H)
;
;	PURPOSE:		Team Manager for multiplayer teams
;
;	HISTORY:		12/21/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _TEAMMGR_H_
#define _TEAMMGR_H_


// Includes...

#include "BareList.h"
#include "BaseDefs_de.h"


// Defines...

#define TM_MAX_NAME			128
#define TM_MAX_PLAYERS		256

#define TM_KEY_FRAGS		1
#define TM_KEY_DEATHS		2
#define TM_KEY_SCORE		3

#define TM_SORT_DESCENDING	1
#define TM_SORT_ASCENDING	2

#define TM_ID_NULL			0xFFFFFFFF


// Classes...

class CTeamPlayer : public CBareListItem
{
	// Member functions...

public:
	CTeamPlayer() { Clear(); }
	~CTeamPlayer() { Term(); }

	DBOOL					Init(DDWORD dwPlayerID, DDWORD dwFlags = 0);
	void					Term();
	void					Clear();

	DDWORD					GetID() { return(m_dwID); }
	int						GetFrags() { return(m_nFrags); }
	int						GetDeaths() { return(m_nDeaths); }
	DDWORD					GetFlags() { return(m_dwFlags); }
	CTeamPlayer* 			GetNext() { return((CTeamPlayer*)CBareListItem::Next()); }
	CTeamPlayer*			GetPrev() { return((CTeamPlayer*)CBareListItem::Prev()); }

	void					SetFrags(int nFrags) { m_nFrags = nFrags; }
	void					SetDeaths(int nDeaths) { m_nDeaths = nDeaths; }

	void					AddFrags(int nFrags) { m_nFrags += nFrags; }
	void					AddDeaths(int nDeaths) { m_nDeaths += nDeaths; }

	void					Reset();


	// Member variables...

private:
	DDWORD					m_dwID;
	int						m_nFrags;
	int						m_nDeaths;
	DDWORD					m_dwFlags;
};

class CTeamPlayerList : public CBareList
{
	// Member variabls...

public:
	CTeamPlayer*			GetFirst() { return((CTeamPlayer*)CBareList::GetFirst()); }
	CTeamPlayer*			GetLast() { return((CTeamPlayer*)CBareList::GetLast()); }
	void					DeleteAll();
};

class CTeam : public CBareListItem
{
	// Member functions...

public:
	CTeam() { Clear(); }
	~CTeam() { Term(); }

	DBOOL					Init(DDWORD dwTeamID, char* sTeamName = NULL, DDWORD dwFlags = 0);
	void					Term();
	void					Clear();

	char*					GetName() { return(m_sName); }
	DDWORD					GetID() { return(m_dwID); }
	DDWORD					GetFlags() { return(m_dwFlags); }
	CTeamPlayer*			GetPlayer(DDWORD dwPlayerID);
	CTeamPlayer*			GetFirstPlayer() { return(m_lsPlayers.GetFirst()); }
	CTeamPlayer*			GetNextPlayer(CTeamPlayer* pPlayer) { if (pPlayer) { return(pPlayer->GetNext()); } else return(NULL); }
	int						GetNumPlayers() { return(m_lsPlayers.GetNumItems()); }
	int						GetFrags() { return(m_nFrags); }
	int						GetDeaths() { return(m_nDeaths); }
	int						GetScore() { return(m_nScore); }
	CTeam* 					GetNext() { return((CTeam*)CBareListItem::Next()); }
	CTeam*					GetPrev() { return((CTeam*)CBareListItem::Prev()); }

	DBOOL					SetScore(int nScore);

	CTeamPlayer*			AddPlayer(DDWORD dwPlayerID, DDWORD dwFlags = 0);
	DBOOL					AddPlayer(CTeamPlayer* pPlayer);

	DBOOL					AddScore(int nScore);
	DBOOL					AddFrags(DDWORD dwPlayerID, int nFrags);
	DBOOL					AddDeaths(DDWORD dwPlayerID, int nDeaths);

	void					Reset();

	DBOOL					RemovePlayer(DDWORD dwPlayerID);
	DBOOL					RemovePlayer(CTeamPlayer* pPlayer);

	DBOOL					ChangePlayerTeam(DDWORD dwPlayerID, CTeam* pNewTeam, DBOOL bReset);
	DBOOL					ChangePlayerTeam(CTeamPlayer* pPlayer, CTeam* pNewTeam, DBOOL bReset);

	void					ResetAllPlayers();
	void					DeleteAllPlayers();

	void					SortPlayersByScore(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_FRAGS, nDir); }
	void					SortPlayersByFrags(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_FRAGS, nDir); }
	void					SortPlayersByDeaths(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_DEATHS, nDir); }
	void					SortPlayers(int nSortKey, int nDir);

private:
	DBOOL					IsGreater(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey);
	DBOOL					IsLess(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey);
	DBOOL					IsEqual(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey);

	// Member variables...

private:
	char					m_sName[TM_MAX_NAME];
	DDWORD					m_dwID;
	DDWORD					m_dwFlags;
	CTeamPlayerList			m_lsPlayers;
	int						m_nFrags;
	int						m_nDeaths;
	int						m_nScore;
};

class CTeamList : public CBareList
{
	// Member variabls...

public:
	CTeam* 					GetFirst() { return((CTeam*)CBareList::GetFirst()); }
	CTeam*					GetLast() { return((CTeam*)CBareList::GetLast()); }
	void					DeleteAll();
};

class CTeamMgr
{
	// Member functions...

public:
	CTeamMgr() { Clear(); ClearTeamTransIDs(); }
	~CTeamMgr() { Term(); }

	DBOOL					Init();
	void					Term();
	void					Clear();

	int						GetNumTeams() { return(m_lsTeams.GetNumItems()); }
	CTeam*					GetTeam(DDWORD dwTeamID);
	CTeam*					GetTeam(const char* sTeamName);
	CTeam*					GetTeamFromPlayerID(DDWORD dwPlayerID);
	CTeam*					GetTeamWithMostPlayers(DBOOL bRandomizeTies);
	CTeam*					GetTeamWithLeastPlayers(DBOOL bRandomizeTies);
	CTeam*					GetFirstTeam() { return(m_lsTeams.GetFirst()); }
	CTeam*					GetNextTeam(CTeam* pTeam) { if (pTeam) { return(pTeam->GetNext()); } else return(NULL); }

	DBOOL					SetTeamScore(DDWORD dwTeamID, int nScore);
	DBOOL					SetTeamScore(char* sTeamName, int nScore);
	DBOOL					SetTeamScore(CTeam* pTeam, int nScore);

	CTeam*					AddTeam(DDWORD dwTeamID, char* sTeamName = NULL, DDWORD dwFlags = 0);

	CTeamPlayer*			AddPlayer(DDWORD dwTeamID, DDWORD dwPlayerID, DDWORD dwFlags = 0);
	CTeamPlayer*			AddPlayer(char* sTeamName, DDWORD dwPlayerID, DDWORD dwFlags = 0);
	CTeamPlayer*			AddPlayer(CTeam* pTeam, DDWORD dwPlayerID, DDWORD dwFlags = 0);

	DBOOL					AddTeamScore(DDWORD dwTeamID, int nScore);
	DBOOL					AddTeamScore(char* sTeamName, int nScore);
	DBOOL					AddTeamScore(CTeam* pTeam, int nScore);

	DBOOL					AddPlayerFrags(DDWORD dwPlayerID, int nFrags);
	DBOOL					AddPlayerFrags(DDWORD dwTeamID, DDWORD dwPlayerID, int nFrags);
	DBOOL					AddPlayerFrags(char* sTeamName, DDWORD dwPlayerID, int nFrags);
	DBOOL					AddPlayerFrags(CTeam* pTeam, DDWORD dwPlayerID, int nFrags);

	DBOOL					AddPlayerDeaths(DDWORD dwPlayerID, int nDeaths);
	DBOOL					AddPlayerDeaths(DDWORD dwTeamID, DDWORD dwPlayerID, int nFrags);
	DBOOL					AddPlayerDeaths(char* sTeamName, DDWORD dwPlayerID, int nFrags);
	DBOOL					AddPlayerDeaths(CTeam* pTeam, DDWORD dwPlayerID, int nDeaths);

	DBOOL					RemoveTeam(DDWORD dwTeamID);
	DBOOL					RemoveTeam(char* sTeamName);
	DBOOL					RemoveTeam(CTeam* pTeam);

	DBOOL					RemovePlayer(DDWORD dwPlayerID);

	DBOOL					ChangePlayerTeam(DDWORD dwPlayerID, DDWORD dwNewTeamID, DBOOL bReset);
	DBOOL					ChangePlayerTeam(DDWORD dwPlayerID, char* sNewTeamName, DBOOL bReset);
	DBOOL					ChangePlayerTeam(DDWORD dwPlayerID, CTeam* pNewTeam, DBOOL bReset);

	void					ResetAllTeams();
	void					DeleteAllTeams();

	void					SortTeamsByScore(int nDir = TM_SORT_DESCENDING) { SortTeams(TM_KEY_SCORE, nDir); }
	void					SortTeamsByFrags(int nDir = TM_SORT_DESCENDING) { SortTeams(TM_KEY_FRAGS, nDir); }
	void					SortTeamsByDeaths(int nDir = TM_SORT_DESCENDING) { SortTeams(TM_KEY_DEATHS, nDir); }
	void					SortTeams(int nSortKey, int nDir);

	void					SortPlayersByScore(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_FRAGS, nDir); }
	void					SortPlayersByFrags(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_FRAGS, nDir); }
	void					SortPlayersByDeaths(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_DEATHS, nDir); }
	void					SortPlayers(int nSortKey, int nDir);

	DBOOL					IsOnSameTeam(DDWORD dwPlayerID1, DDWORD dwPlayerID2, CTeam** ppTeam = NULL);

	DBOOL					CreateTeamTransIDs();
	DDWORD					GetTeamTransID(DDWORD dwPlayerID);
	void					RemoveTeamTransID(DDWORD dwPlayerID);
	void					ClearTeamTransIDs();

private:
	DBOOL					IsGreater(CTeam* pTeam1, CTeam* pTeam2, int nSortKey);
	DBOOL					IsLess(CTeam* pTeam1, CTeam* pTeam2, int nSortKey);
	DBOOL					IsEqual(CTeam* pTeam1, CTeam* pTeam2, int nSortKey);


	// Member variables...

private:
	CTeamList				m_lsTeams;
	DDWORD					m_aTeamTransIDs[TM_MAX_PLAYERS];
};


// Inlines...

inline void CTeamPlayer::Clear()
{
	m_dwID    = TM_ID_NULL;
	m_nFrags  = 0;
	m_nDeaths = 0;
	m_dwFlags = 0;
}

inline void CTeamPlayer::Term()
{
	Clear();
}

inline void CTeamPlayer::Reset()
{
	m_nFrags  = 0;
	m_nDeaths = 0;
}

inline void CTeamPlayerList::DeleteAll()
{
	CTeamPlayer* pPlayer = GetFirst();

	while (pPlayer)
	{
		CTeamPlayer* pTemp = pPlayer;
		pPlayer = pTemp->GetNext();
		Delete(pTemp);
		delete pTemp;
	}
}

inline void CTeam::Clear()
{
	m_sName[0] = '\0';
	m_dwID     = TM_ID_NULL;
	m_nScore   = 0;
	m_nFrags   = 0;
	m_nDeaths  = 0;
	m_dwFlags  = 0;
}

inline void CTeam::DeleteAllPlayers()
{
	m_lsPlayers.DeleteAll();
}

inline DBOOL CTeam::IsGreater(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey)
{
	if (!pPlr1 || !pPlr2) return(DFALSE);

	switch (nSortKey)
	{
		case TM_KEY_FRAGS:  return(pPlr1->GetFrags() > pPlr2->GetFrags());
		case TM_KEY_DEATHS: return(pPlr1->GetDeaths() > pPlr2->GetDeaths());
	}

	return(DFALSE);
}

inline DBOOL CTeam::IsLess(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey)
{
	if (!pPlr1 || !pPlr2) return(DFALSE);

	switch (nSortKey)
	{
		case TM_KEY_FRAGS:  return(pPlr1->GetFrags() < pPlr2->GetFrags());
		case TM_KEY_DEATHS: return(pPlr1->GetDeaths() < pPlr2->GetDeaths());
	}

	return(DFALSE);
}

inline DBOOL CTeam::IsEqual(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey)
{
	if (!pPlr1 || !pPlr2) return(DFALSE);

	switch (nSortKey)
	{
		case TM_KEY_FRAGS:  return(pPlr1->GetFrags() == pPlr2->GetFrags());
		case TM_KEY_DEATHS: return(pPlr1->GetDeaths() == pPlr2->GetDeaths());
	}

	return(DFALSE);
}

inline void CTeamList::DeleteAll()
{
	CTeam* pTeam = GetFirst();

	while (pTeam)
	{
		CTeam* pTemp = pTeam;
		pTeam = pTemp->GetNext();
		Delete(pTemp);
		delete pTemp;
	}
}

inline void CTeamMgr::Clear()
{

}

inline void CTeamMgr::ClearTeamTransIDs()
{
	for (int i = 0; i < TM_MAX_PLAYERS; i++)
	{
		m_aTeamTransIDs[i] = TM_ID_NULL;
	}
}

inline void CTeamMgr::DeleteAllTeams()
{
	m_lsTeams.DeleteAll();
}

inline DBOOL CTeamMgr::IsGreater(CTeam* pTeam1, CTeam* pTeam2, int nSortKey)
{
	if (!pTeam1 || !pTeam2) return(DFALSE);

	switch (nSortKey)
	{
		case TM_KEY_SCORE:  return(pTeam1->GetScore() > pTeam2->GetScore());
		case TM_KEY_FRAGS:  return(pTeam1->GetFrags() > pTeam2->GetFrags());
		case TM_KEY_DEATHS: return(pTeam1->GetDeaths() > pTeam2->GetDeaths());
	}

	return(DFALSE);
}

inline DBOOL CTeamMgr::IsLess(CTeam* pTeam1, CTeam* pTeam2, int nSortKey)
{
	if (!pTeam1 || !pTeam2) return(DFALSE);

	switch (nSortKey)
	{
		case TM_KEY_SCORE:  return(pTeam1->GetScore() < pTeam2->GetScore());
		case TM_KEY_FRAGS:  return(pTeam1->GetFrags() < pTeam2->GetFrags());
		case TM_KEY_DEATHS: return(pTeam1->GetDeaths() < pTeam2->GetDeaths());
	}

	return(DFALSE);
}

inline DBOOL CTeamMgr::IsEqual(CTeam* pTeam1, CTeam* pTeam2, int nSortKey)
{
	if (!pTeam1 || !pTeam2) return(DFALSE);

	switch (nSortKey)
	{
		case TM_KEY_SCORE:  return(pTeam1->GetScore() == pTeam2->GetScore());
		case TM_KEY_FRAGS:  return(pTeam1->GetFrags() == pTeam2->GetFrags());
		case TM_KEY_DEATHS: return(pTeam1->GetDeaths() == pTeam2->GetDeaths());
	}

	return(DFALSE);
}

inline DBOOL CTeamMgr::IsOnSameTeam(DDWORD dwPlayerID1, DDWORD dwPlayerID2, CTeam** ppTeam)
{
	CTeam* pTeam1 = GetTeamFromPlayerID(dwPlayerID1);
	if (!pTeam1) return(DFALSE);

	return (pTeam1->GetPlayer(dwPlayerID2) != NULL);
}


// EOF

#endif

