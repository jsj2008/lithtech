/****************************************************************************
;
;	 MODULE:		NetInfo (.H)
;
;	PURPOSE:		Network game info
;
;	HISTORY:		07/05/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _NETINFO_H_
#define _NETINFO_H_


// Includes...

#include "iltclient.h"
#include "..\Shared\NetDefs.h"


// Classes...

class CNinfoPlayer
{
	// Member functions...

public:
	CNinfoPlayer() { Clear(); }
	~CNinfoPlayer() { Term(); }

	BOOL				Init(char* sName, int nFrags);
	void				Term() { Clear(); }
	void				Clear();

	char*				GetName() { return(m_sName); }
	int					GetFrags() { return(m_nFrags); }


	// Member variables...

private:
	char				m_sName[NML_NAME];
	int					m_nFrags;
};

class CNinfoGame
{
	// Member functions...

public:
	CNinfoGame() { Clear(); }
	~CNinfoGame() { Term(); }

	BOOL				Init(char* sName, char* sHost, char* sLevel, int nType, NetSession* pNetSession, DWORD dwNumDPlayPlayers, char *pHostIP, DWORD hostPort);
	void				Term();
	void				Clear();

	BOOL				IsValid() { return(!!m_pNetSession); }

	char*				GetName() { return(m_sName); }
	char*				GetNameWithPing();
	char*				GetHost() { return(m_sHost); }
	char*				GetHostIP() { return m_sHostIP; }
	DWORD				GetHostPort() { return m_dwHostPort; }
	char*				GetLevel() { return(m_sLevel); }
	int					GetType() { return(m_nType); }
	char*				GetTypeString() { return(m_sType); }
	int					GetNumPlayers() { return(m_cPlayers); }
	int					GetNumDPlayPlayers() { return(m_cDPlayPlayers); }
	int					GetPing() { return(m_nPing); }
	NetSession*			GetSessionPointer() { return(m_pNetSession); }
	DWORD				GetCRC() { return(m_dwCRC); }

	CNinfoPlayer*		AddPlayer(char* sName, int nFrags);
	CNinfoPlayer*		GetFirstPlayer();
	CNinfoPlayer*		GetNextPlayer();
	void				RemovePlayers();

private:
	BOOL				AddPlayer(CNinfoPlayer* pPlayer);


	// Member variables...

private:
	char				m_sHostIP[MAX_HOSTIP_LEN];
	DWORD				m_dwHostPort;

	char				m_sName[NML_NAME];
	char				m_sHost[NML_HOST];
	char				m_sLevel[NML_LEVEL];
	char				m_sType[32];
	int					m_nType;
	int					m_cPlayers;
	int					m_cDPlayPlayers;
	int					m_iPlayer;
	int					m_nPing;
	DWORD				m_dwCRC;
	CNinfoPlayer*		m_aPlayers[NML_PLAYERS];
	NetSession*			m_pNetSession;
};

class CNinfoMgr
{
	// Member functions...

public:
	CNinfoMgr() { Clear(); }
	~CNinfoMgr() { Term(); }

	BOOL				Init();
	void				Term();
	void				Clear();

	CNinfoGame*			GetGame(NetSession* pNetSession);
	CNinfoGame*			GetGame(DWORD dwCRC);
	int					GetNumGames() { return(m_cGames); }

	CNinfoGame*			AddGame(const char* sInfo, NetSession* pNetSession, DWORD dwNumDPlayPlayers);
	CNinfoGame*			GetFirstGame();
	CNinfoGame*			GetNextGame();
	void				RemoveGames();

private:
	BOOL				AddGame(CNinfoGame* pGame);

public:
	static	BOOL		CreateSessionString(char* sString, char* sName, char* sHost, char* sLevel, int nType);


	// Member variables...

private:
	CNinfoGame*			m_aGames[NML_GAMES];
	int					m_cGames;
	int					m_iGame;
};


// Inlines...

inline void CNinfoPlayer::Clear()
{
	m_sName[0] = '\0';
	m_nFrags   = 0;
}

inline void CNinfoGame::Clear()
{
	m_sName[0]    = '\0';
	m_sHost[0]    = '\0';
	m_sLevel[0]   = '\0';
	m_sType[0]    = '\0';
	m_sHostIP[0] = 0;
	m_dwHostPort = 0;
	m_cPlayers    = 0;
	m_iPlayer     = 0;
	m_nType       = 0;
	m_nPing       = 0;
	m_dwCRC       = 0;
	m_pNetSession = NULL;
}

inline void CNinfoMgr::Clear()
{
	m_cGames = 0;
	m_iGame  = 0;
}


// EOF...

#endif