#ifndef __CLIENTINFOMGR_H
#define __CLIENTINFOMGR_H

#include "ltbasedefs.h"
#include "ClientUtilities.h"
#include "SharedMission.h"
#include "SharedScoring.h"
#include "EventCaster.h"
#include "TeamMgr.h"

class ILTClient;

class ClientDisplayData
{
public:
	ClientDisplayData();
	virtual ~ClientDisplayData();

	HRENDERTARGET	hNameTarget;

	std::string		sNameMaterialFile;
	HMATERIAL		hNameMaterial;

	std::string		sInsigniaMaterialFile;
	HMATERIAL		hInsigniaMaterial;
};

struct CLIENT_INFO
{
    CLIENT_INFO();
	~CLIENT_INFO();

	uint16			nPing;
    uint32          nID;
	std::wstring	sName;
	std::string		sInsignia;
	MissionStats	sStats;
	CPlayerScore	sScore;
	bool			bIsAdmin;
	uint8			nTeamID;

	ClientDisplayData sDisplayData;

	CLIENT_INFO*	pPrev;
	CLIENT_INFO*	pNext;
};

class CClientInfoMgr
{
public:

	CClientInfoMgr();
	~CClientInfoMgr();

    void    Init ();
	void    SetupMultiplayer();
	void	ClearMultiplayer();

    void    AddClient ( const wchar_t* pszName, const char* pszInsignia,  bool bIsAdmin, uint32 nID, uint8 nTeamID);
	void	PlayerConnected( const wchar_t* pszName, const char* pszInsignia, uint32 nID );
    void    UpdateClient ( const wchar_t* pszName, const char* pszInsignia, bool bIsAdmin, uint32 nID, uint8 nTeamID);
    void    RemoveClient (uint32 nID);
	void	RemoveAllClients();

	CLIENT_INFO* GetLocalClient();
	CLIENT_INFO* GetFirstClient() const {return m_pClients;}
    CLIENT_INFO* GetClientByID(uint32 nID, bool bUpdateOnFailure = true);

    uint32  GetNumClients();
    const wchar_t* GetPlayerName(uint32 nID);
	const char* GetPlayerInsignia(uint32 nID);
	uint8	GetPlayerTeam(uint32 nID);

	void	UpdateClientSort(CLIENT_INFO* pCur);

	uint8	GetLocalTeam();
	bool	IsLocalTeam( uint8 nTeamID);

	uint8	GetNumPlayersOnTeam(uint8 nTeam = -1);

	DECLARE_EVENT( ScoresChangedEvent );

	// Event fires when player changes teams.  Sends PlayerChangedTeamsNotifyParams.
	struct PlayerChangedTeamsNotifyParams : public EventCaster::NotifyParams
	{
		PlayerChangedTeamsNotifyParams( EventCaster& eventCaster, CLIENT_INFO* pClientInfo ) :
			EventCaster::NotifyParams( eventCaster )
		{
			m_pClientInfo = pClientInfo;
		}

		CLIENT_INFO* m_pClientInfo;
	};
	DECLARE_EVENT( PlayerChangedTeamsEvent );

protected:
	typedef std::vector<HRENDERTARGET> TRenderTargetList;
	TRenderTargetList	m_aNameTargets;

	CLIENT_INFO*		m_pClients;

	uint32				m_nLocalID;

	//returns true if Client A has a better Score than Client B
	bool				IsScoreBetter( CLIENT_INFO* pA,  CLIENT_INFO* pB);


	const char*			VerifyInsignia(const char* pszInsignia);

};

#endif