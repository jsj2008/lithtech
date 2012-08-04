#ifndef __CLIENTINFOMGR_H
#define __CLIENTINFOMGR_H

#include "ltbasedefs.h"
#include "ClientUtilities.h"

class ILTClient;


struct CLIENT_INFO
{
    CLIENT_INFO()   { team =0; m_Ping = 0.0f; nID = 0; hstrName = LTNULL; nFrags = 0; pPrev = LTNULL; pNext = LTNULL; nLives = 255;}

	float			m_Ping;
    uint32          nID;
	HSTRING			hstrName;
	int				nFrags;

	// Player team.
    uint8           team;

	uint8			nLives;

	CLIENT_INFO*	pPrev;
	CLIENT_INFO*	pNext;
};

class TEAM_INFO
{
public:
	TEAM_INFO();
	~TEAM_INFO();

	char		szName[16];
	HLTCOLOR	hColor;
	HSURFACE	hBanner;
	uint32		nScore;

};

class CClientInfoMgr
{
public:

	CClientInfoMgr();
	~CClientInfoMgr();

    void    Init ();

    void    AddClient (HSTRING hstrName, uint32 nID, int nFragCount, uint8 team);
    void    RemoveClient (uint32 nID);
	void	RemoveAllClients();

    void    AddFrag (uint32 nID);
    void    RemoveFrag (uint32 nID);
    void    SetLives (uint32 nID, uint8 nLives);

	void	SetScores(uint32 nTeam1,uint32 nTeam2) {m_Teams[0].nScore = nTeam1;m_Teams[1].nScore = nTeam2;}

    void    ChangeClientTeam(uint32 nID, uint8 teamId);

	void	SetFragScore(LTBOOL bScore) {m_bFragScore = bScore;}
	void	AddScore(uint32 nID, uint8 teamId, uint32 nScore);

	CLIENT_INFO* GetLocalClient();
	CLIENT_INFO* GetFirstClient() {return m_pClients;}
    CLIENT_INFO* GetClientByID(uint32 nID);

    uint32  GetNumClients();
    char*   GetPlayerName (uint32 nID);

	void	UpdateFragDisplay();

    void    Draw (LTBOOL bDrawSingleFragCount, LTBOOL bDrawAllFragCounts, HSURFACE hDestSurf = LTNULL);

	TEAM_INFO*	GetTeam(int nTeam);

protected:

	void	UpdateClientSort(CLIENT_INFO* pCur);

	CLIENT_INFO*		m_pClients;
	TEAM_INFO			m_Teams[2];

	HSTRING				m_hFragString;
	HSTRING				m_hTeamScore;
	HSTRING				m_hOppScore;

	LTBOOL				m_bFragScore;

	LTIntPt				m_FragPos;
	LTIntPt				m_TeamPos;
	LTIntPt				m_OppPos;

	HLTCOLOR			m_hTeamColor;
	HLTCOLOR			m_hOppColor;

	uint32				m_nLocalID;
	LTBOOL				m_bIsTeamGame;

};

inline 	TEAM_INFO* CClientInfoMgr::GetTeam(int nTeam)
{
	if (nTeam != 1 && nTeam != 2) return LTNULL;
	return &m_Teams[nTeam-1];
}


#endif