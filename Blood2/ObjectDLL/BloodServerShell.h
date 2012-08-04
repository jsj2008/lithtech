// ----------------------------------------------------------------------- //
//
// MODULE  : BloodServerShell.cpp
//
// PURPOSE : Blood's Server Shell - Definition
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //

#ifndef __BLOODSERVERSHELL_H__
#define __BLOODSERVERSHELL_H__

#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"
#include "cpp_servershell_de.h"
#include "PathMgr.h"
#include "NetDefs.h"
#include "SharedDefs.h"
#include "VoiceMgr.h"
#include "TeamMgr.h"

#define MAX_CLIENTS		MAX_MULTI_PLAYERS

class CPlayerObj;


class CBloodServerShell : public CServerShellDE
{
	public :

		CBloodServerShell();
		~CBloodServerShell();
        
		void		FragStatus();
		GameType	GetGameType()	const { return (GameType)m_GameInfo.m_byType; }
		const NetGame *	GetNetGameInfo()	const { return &m_GameInfo; }
		DBOOL		IsMultiplayerGame() { return (m_GameInfo.m_byType != GAMETYPE_SINGLE); }
		DBOOL		IsMultiplayerTeamBasedGame();
		DBOOL		IsMultiplayerTeams() { return(m_GameInfo.m_byType == NGT_TEAMS); }
		DBOOL		IsMultiplayerCtf() { return(m_GameInfo.m_byType == NGT_CAPTUREFLAG); }
		DBOOL		IsMultiplayerSoccer() { return(m_GameInfo.m_byType == NGT_SOCCER); }
        
		PathMgr*    GetPathMgr() { return &m_PathMgr; }
		CVoiceMgr*	GetVoiceMgr() { return &m_VoiceMgr; }
		CTeamMgr*	GetTeamMgr() { return(&m_TeamMgr); }
		void		SetStartPointName(HSTRING hstrStartPt);
		HSTRING		GetStartPointName() { return m_hstrStartPointName; }
		void		SendBlood2ServConsoleMessage(char* sMsg);
		void		SetUpdateBlood2Serv() { m_bUpdateBlood2Serv = DTRUE; }
		HCLIENT		FindClient(HOBJECT hObject);

#ifdef _ADD_ON
		DBOOL		IsAddon() { return m_bAddonLevel; };
#endif

	protected :

		BaseClass*	OnClientEnterWorld(HCLIENT hClient, void* pClientData, DDWORD dwClientDataLen);
		void		OnClientExitWorld(HCLIENT hClient);
		void		OnMessage(HCLIENT hSender, DBYTE messageID, HMESSAGEREAD hMessage);
		void		OnCommandOn(HCLIENT hClient, int command);
		void		PreStartWorld(DBOOL bSwitchingWorlds);
		void		PostStartWorld();
		void		Update(DFLOAT timeElapsed);
		void		UpdateClientPingTimes();
		void		OnRemoveClient(HCLIENT hClient);

		DBOOL		LoadWorld(HMESSAGEREAD hMessage);
		DBOOL		SaveGame(DBYTE bySaveType, DBOOL bSavePlayers, DBOOL bSaveConsole);
		DBOOL		RestoreGame(DBYTE bySaveType);
//		DBOOL		BuildSavePath(char *pBuffer, char *pFilename);
		DBOOL		KeepAliveSave();
		DBOOL		KeepAliveLoad();

		void		CacheFiles();

		void		DoLevelChangeCharacterCheck(char* sLevel);

	private:

		int			m_NumPlayers;
		BaseClass*	CreatePlayer(HCLIENT hClient);
		void		RespawnPlayer(BaseClass* pBaseClass, HCLIENT hClient);
		void		ConsoleMessage(HCLIENT hClient, char *msg);
		void		AddPlayerMessage(HCLIENT hDestClient, HCLIENT hNewPlayerClient);
		void		AddPlayersMessage(HCLIENT hClient);
		void		RemovePlayerMessage(HCLIENT hClient);
		void		SetGameInfo(DBYTE nGameFlags, DBYTE nDifficulty);
		void		SetupGameInfo();

		DBOOL		UpdateSessionName();
		DBOOL		UpdateBlood2Server();
		void		UpdateMultiplayer();
		void		StartNextMultiplayerLevel();
		void		StartNextMultiplayerLevelAck();
		DRESULT		ServerAppMessageFn(char* sMsg);


		void		ClearClientList() { for (int i = 0; i < MAX_CLIENTS; i++) { m_aClients[i] = DNULL; } }
		DBOOL		AddClientToList(HCLIENT hClient);
		DBOOL		RemoveClientFromList(HCLIENT hClient);
		DBOOL		IsClientInList(HCLIENT hClient);
		CPlayerObj*	GetPlayerFromClientList(HCLIENT hClient);

		CVoiceMgr	m_VoiceMgr;
		CTeamMgr	m_TeamMgr;
		PathMgr		m_PathMgr;
		HSTRING		m_hstrStartPointName;
		DBOOL		m_bKeepAlive;
#ifdef _ADD_ON
		DBOOL		m_bAddonLevel;
#endif

		HCLIENT		m_aClients[MAX_CLIENTS];	

		NetGame		m_GameInfo;
		DBOOL		m_bBlood2ServHosted;
		DBOOL		m_bUpdateBlood2Serv;
		int			m_nCurLevel;
};


// Externs...

extern CBloodServerShell* g_pBloodServerShell;
extern char g_szVarRevisiting[];
extern char g_szVarDifficulty[];
extern char g_szVarGameType[];


// Inlines...

inline DBOOL CBloodServerShell::IsMultiplayerTeamBasedGame()
{
	if (m_GameInfo.m_byType == NGT_CAPTUREFLAG) return(DTRUE);
	if (m_GameInfo.m_byType == NGT_TEAMS) return(DTRUE);
	if (m_GameInfo.m_byType == NGT_SOCCER) return(DTRUE);

	return(DFALSE);
}


// EOF...

#endif  // __BLOODSERVERSHELL_H__

