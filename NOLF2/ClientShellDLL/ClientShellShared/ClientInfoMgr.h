#ifndef __CLIENTINFOMGR_H
#define __CLIENTINFOMGR_H

#include "ltbasedefs.h"
#include "ClientUtilities.h"
#include "SharedMission.h"
#include "SharedScoring.h"

class ILTClient;


struct CLIENT_INFO
{
    CLIENT_INFO()   { nPing = 0; nID = 0; pPrev = LTNULL; pNext = LTNULL; bIsAdmin = false; nTeamID = 0;}

	uint16			nPing;
    uint32          nID;
	std::string		sName;
	MissionStats	sStats;
	CPlayerScore	sScore;
	bool			bIsAdmin;
	uint8			nTeamID;

	CLIENT_INFO*	pPrev;
	CLIENT_INFO*	pNext;
};

class CClientInfoMgr
{
public:

	CClientInfoMgr();
	~CClientInfoMgr();

    void    Init ();

    void    AddClient ( char const* pszName, bool bIsAdmin, uint32 nID, uint8 nTeamID);
	void	PlayerConnected( char const* pszName, uint32 nID );
    void    UpdateClient ( char const* pszName, bool bIsAdmin, uint32 nID, uint8 nTeamID);
    void    RemoveClient (uint32 nID);
	void	RemoveAllClients();

	CLIENT_INFO* GetLocalClient();
	CLIENT_INFO* GetFirstClient() {return m_pClients;}
    CLIENT_INFO* GetClientByID(uint32 nID, bool bUpdateOnFailure = true);

    uint32  GetNumClients();
    char const* GetPlayerName (uint32 nID);

	void	UpdateClientSort(CLIENT_INFO* pCur);

	uint8	GetNumPlayersOnTeam(uint8 nTeam = -1);

protected:


	CLIENT_INFO*		m_pClients;

	uint32				m_nLocalID;

};

#endif