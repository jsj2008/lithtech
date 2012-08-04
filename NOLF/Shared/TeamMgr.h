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

#include "barelist.h"
#include "ltbasedefs.h"
#include "DebugNew.h"


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

    LTBOOL                   Init(uint32 dwPlayerID, uint32 dwFlags = 0);
	void					Term();
	void					Clear();

    uint32                  GetID() { return(m_dwID); }
	int						GetFrags() { return(m_nFrags); }
	int						GetDeaths() { return(m_nDeaths); }
    uint32                  GetFlags() { return(m_dwFlags); }
	CTeamPlayer* 			GetNext() { return((CTeamPlayer*)CBareListItem::Next()); }
	CTeamPlayer*			GetPrev() { return((CTeamPlayer*)CBareListItem::Prev()); }

	void					SetFrags(int nFrags) { m_nFrags = nFrags; }
	void					SetDeaths(int nDeaths) { m_nDeaths = nDeaths; }

	void					AddFrags(int nFrags) { m_nFrags += nFrags; }
	void					AddDeaths(int nDeaths) { m_nDeaths += nDeaths; }

	void					Reset();


	// Member variables...

private:
    uint32                  m_dwID;
	int						m_nFrags;
	int						m_nDeaths;
    uint32                  m_dwFlags;
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

    LTBOOL                   Init(uint32 dwTeamID, char* sTeamName = NULL, uint32 dwFlags = 0);
	void					Term();
	void					Clear();

	char*					GetName() { return(m_sName); }
    uint32                  GetID() { return(m_dwID); }
    uint32                  GetFlags() { return(m_dwFlags); }
    CTeamPlayer*            GetPlayer(uint32 dwPlayerID);
	CTeamPlayer*			GetFirstPlayer() { return(m_lsPlayers.GetFirst()); }
	CTeamPlayer*			GetNextPlayer(CTeamPlayer* pPlayer) { if (pPlayer) { return(pPlayer->GetNext()); } else return(NULL); }
	int						GetNumPlayers() { return(m_lsPlayers.GetNumItems()); }
	int						GetFrags() { return(m_nFrags); }
	int						GetDeaths() { return(m_nDeaths); }
	int						GetScore() { return(m_nScore); }
	CTeam* 					GetNext() { return((CTeam*)CBareListItem::Next()); }
	CTeam*					GetPrev() { return((CTeam*)CBareListItem::Prev()); }

    LTBOOL                   SetScore(int nScore);

    CTeamPlayer*            AddPlayer(uint32 dwPlayerID, uint32 dwFlags = 0);
    LTBOOL                   AddPlayer(CTeamPlayer* pPlayer);

    LTBOOL                   AddScore(int nScore);
    LTBOOL                   AddFrags(uint32 dwPlayerID, int nFrags);
    LTBOOL                   AddDeaths(uint32 dwPlayerID, int nDeaths);

	void					Reset();

    LTBOOL                   RemovePlayer(uint32 dwPlayerID);
    LTBOOL                   RemovePlayer(CTeamPlayer* pPlayer);

    LTBOOL                   ChangePlayerTeam(uint32 dwPlayerID, CTeam* pNewTeam, LTBOOL bReset);
    LTBOOL                   ChangePlayerTeam(CTeamPlayer* pPlayer, CTeam* pNewTeam, LTBOOL bReset);

	void					ResetAllPlayers();
	void					DeleteAllPlayers();

	void					SortPlayersByScore(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_FRAGS, nDir); }
	void					SortPlayersByFrags(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_FRAGS, nDir); }
	void					SortPlayersByDeaths(int nDir = TM_SORT_DESCENDING) { SortPlayers(TM_KEY_DEATHS, nDir); }
	void					SortPlayers(int nSortKey, int nDir);

private:
    LTBOOL                   IsGreater(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey);
    LTBOOL                   IsLess(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey);
    LTBOOL                   IsEqual(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey);

	// Member variables...

private:
	char					m_sName[TM_MAX_NAME];
    uint32                  m_dwID;
    uint32                  m_dwFlags;
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

    LTBOOL                   Init();
	void					Term();
	void					Clear();

	int						GetNumTeams() { return(m_lsTeams.GetNumItems()); }
    CTeam*                  GetTeam(uint32 dwTeamID);
	CTeam*					GetTeam(const char* sTeamName);
    CTeam*                  GetTeamFromPlayerID(uint32 dwPlayerID);
    CTeam*                  GetTeamWithMostPlayers(LTBOOL bRandomizeTies);
    CTeam*                  GetTeamWithLeastPlayers(LTBOOL bRandomizeTies);
	CTeam*					GetFirstTeam() { return(m_lsTeams.GetFirst()); }
	CTeam*					GetNextTeam(CTeam* pTeam) { if (pTeam) { return(pTeam->GetNext()); } else return(NULL); }

    LTBOOL                   SetTeamScore(uint32 dwTeamID, int nScore);
    LTBOOL                   SetTeamScore(char* sTeamName, int nScore);
    LTBOOL                   SetTeamScore(CTeam* pTeam, int nScore);

    CTeam*                  AddTeam(uint32 dwTeamID, char* sTeamName = NULL, uint32 dwFlags = 0);

    CTeamPlayer*            AddPlayer(uint32 dwTeamID, uint32 dwPlayerID, uint32 dwFlags = 0);
    CTeamPlayer*            AddPlayer(char* sTeamName, uint32 dwPlayerID, uint32 dwFlags = 0);
    CTeamPlayer*            AddPlayer(CTeam* pTeam, uint32 dwPlayerID, uint32 dwFlags = 0);

    LTBOOL                   AddTeamScore(uint32 dwTeamID, int nScore);
    LTBOOL                   AddTeamScore(char* sTeamName, int nScore);
    LTBOOL                   AddTeamScore(CTeam* pTeam, int nScore);

    LTBOOL                   AddPlayerFrags(uint32 dwPlayerID, int nFrags);
    LTBOOL                   AddPlayerFrags(uint32 dwTeamID, uint32 dwPlayerID, int nFrags);
    LTBOOL                   AddPlayerFrags(char* sTeamName, uint32 dwPlayerID, int nFrags);
    LTBOOL                   AddPlayerFrags(CTeam* pTeam, uint32 dwPlayerID, int nFrags);

    LTBOOL                   AddPlayerDeaths(uint32 dwPlayerID, int nDeaths);
    LTBOOL                   AddPlayerDeaths(uint32 dwTeamID, uint32 dwPlayerID, int nFrags);
    LTBOOL                   AddPlayerDeaths(char* sTeamName, uint32 dwPlayerID, int nFrags);
    LTBOOL                   AddPlayerDeaths(CTeam* pTeam, uint32 dwPlayerID, int nDeaths);

    LTBOOL                   RemoveTeam(uint32 dwTeamID);
    LTBOOL                   RemoveTeam(char* sTeamName);
    LTBOOL                   RemoveTeam(CTeam* pTeam);

    LTBOOL                   RemovePlayer(uint32 dwPlayerID);

    LTBOOL                   ChangePlayerTeam(uint32 dwPlayerID, uint32 dwNewTeamID, LTBOOL bReset);
    LTBOOL                   ChangePlayerTeam(uint32 dwPlayerID, char* sNewTeamName, LTBOOL bReset);
    LTBOOL                   ChangePlayerTeam(uint32 dwPlayerID, CTeam* pNewTeam, LTBOOL bReset);

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

    LTBOOL                   IsOnSameTeam(uint32 dwPlayerID1, uint32 dwPlayerID2, CTeam** ppTeam = NULL);

    LTBOOL                   CreateTeamTransIDs();
    uint32                  GetTeamTransID(uint32 dwPlayerID);
    void                    RemoveTeamTransID(uint32 dwPlayerID);
	void					ClearTeamTransIDs();

private:
    LTBOOL                   IsGreater(CTeam* pTeam1, CTeam* pTeam2, int nSortKey);
    LTBOOL                   IsLess(CTeam* pTeam1, CTeam* pTeam2, int nSortKey);
    LTBOOL                   IsEqual(CTeam* pTeam1, CTeam* pTeam2, int nSortKey);


	// Member variables...

private:
	CTeamList				m_lsTeams;
    uint32                  m_aTeamTransIDs[TM_MAX_PLAYERS];
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
		debug_delete(pTemp);
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

inline LTBOOL CTeam::IsGreater(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey)
{
    if (!pPlr1 || !pPlr2) return(LTFALSE);

	switch (nSortKey)
	{
		case TM_KEY_FRAGS:  return(pPlr1->GetFrags() > pPlr2->GetFrags());
		case TM_KEY_DEATHS: return(pPlr1->GetDeaths() > pPlr2->GetDeaths());
	}

    return(LTFALSE);
}

inline LTBOOL CTeam::IsLess(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey)
{
    if (!pPlr1 || !pPlr2) return(LTFALSE);

	switch (nSortKey)
	{
		case TM_KEY_FRAGS:  return(pPlr1->GetFrags() < pPlr2->GetFrags());
		case TM_KEY_DEATHS: return(pPlr1->GetDeaths() < pPlr2->GetDeaths());
	}

    return(LTFALSE);
}

inline LTBOOL CTeam::IsEqual(CTeamPlayer* pPlr1, CTeamPlayer* pPlr2, int nSortKey)
{
    if (!pPlr1 || !pPlr2) return(LTFALSE);

	switch (nSortKey)
	{
		case TM_KEY_FRAGS:  return(pPlr1->GetFrags() == pPlr2->GetFrags());
		case TM_KEY_DEATHS: return(pPlr1->GetDeaths() == pPlr2->GetDeaths());
	}

    return(LTFALSE);
}

inline void CTeamList::DeleteAll()
{
	CTeam* pTeam = GetFirst();

	while (pTeam)
	{
		CTeam* pTemp = pTeam;
		pTeam = pTemp->GetNext();
		Delete(pTemp);
		debug_delete(pTemp);
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

inline LTBOOL CTeamMgr::IsGreater(CTeam* pTeam1, CTeam* pTeam2, int nSortKey)
{
    if (!pTeam1 || !pTeam2) return(LTFALSE);

	switch (nSortKey)
	{
		case TM_KEY_SCORE:  return(pTeam1->GetScore() > pTeam2->GetScore());
		case TM_KEY_FRAGS:  return(pTeam1->GetFrags() > pTeam2->GetFrags());
		case TM_KEY_DEATHS: return(pTeam1->GetDeaths() > pTeam2->GetDeaths());
	}

    return(LTFALSE);
}

inline LTBOOL CTeamMgr::IsLess(CTeam* pTeam1, CTeam* pTeam2, int nSortKey)
{
    if (!pTeam1 || !pTeam2) return(LTFALSE);

	switch (nSortKey)
	{
		case TM_KEY_SCORE:  return(pTeam1->GetScore() < pTeam2->GetScore());
		case TM_KEY_FRAGS:  return(pTeam1->GetFrags() < pTeam2->GetFrags());
		case TM_KEY_DEATHS: return(pTeam1->GetDeaths() < pTeam2->GetDeaths());
	}

    return(LTFALSE);
}

inline LTBOOL CTeamMgr::IsEqual(CTeam* pTeam1, CTeam* pTeam2, int nSortKey)
{
    if (!pTeam1 || !pTeam2) return(LTFALSE);

	switch (nSortKey)
	{
		case TM_KEY_SCORE:  return(pTeam1->GetScore() == pTeam2->GetScore());
		case TM_KEY_FRAGS:  return(pTeam1->GetFrags() == pTeam2->GetFrags());
		case TM_KEY_DEATHS: return(pTeam1->GetDeaths() == pTeam2->GetDeaths());
	}

    return(LTFALSE);
}

inline LTBOOL CTeamMgr::IsOnSameTeam(uint32 dwPlayerID1, uint32 dwPlayerID2, CTeam** ppTeam)
{
	CTeam* pTeam1 = GetTeamFromPlayerID(dwPlayerID1);
    if (!pTeam1) return(LTFALSE);

	return (pTeam1->GetPlayer(dwPlayerID2) != NULL);
}


// EOF

#endif