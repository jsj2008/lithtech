// ----------------------------------------------------------------------- //
//
// MODULE  : GameServerShell.cpp
//
// PURPOSE : Game's Server Shell - Definition
//
// CREATED : 9/18/97
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_SERVER_SHELL_H__
#define __GAME_SERVER_SHELL_H__


#include "iservershell.h"
#include "ClientServerShared.h"
#include "CheatDefs.h"
#include "CharacterMgr.h"
#include "GameStartPoint.h"
#include "NetDefs.h"
#include "CVarTrack.h"
#include "GlobalServerMgr.h"
#include "WeaponMgr.h"
#include "AIButeMgr.h"
#include "AttachButeMgr.h"
#include "ServerButeMgr.h"
#include "ModelButeMgr.h"
#include "SoundMgr.h"
#include "CommandMgr.h"
#include "MissionData.h"
#include "TeamMgr.h"
#include "MyGameSpyMgr.h"
#include "ServerOptionMgr.h"
#include "Objectives.h"

#define MAX_CLIENTS		MAX_MULTI_PLAYERS
#define MAX_TIME_RAMPS	12
#define MAX_GEN_STRING	128


// Used for time of day colors.
class TimeRamp
{
	public:

		TimeRamp()
		{
			Clear();
		}

        TimeRamp(float time, LTVector color)
		{
			Clear();
			m_Time = time;
			m_Color = color;
		}

		~TimeRamp()
		{
			if(m_hMessage)
			{
                g_pLTServer->FreeString(m_hMessage);
			}
		}

		void Clear()
		{
			m_Time = -1.0f;
			m_Color.Init();
			m_hMessage = NULL;
			m_hTarget = NULL;
		}

		float		m_Time;
        LTVector     m_Color; // 0-1
		HSTRING		m_hTarget;	// Message target...
		HSTRING		m_hMessage;	// Message..
};


class CPlayerObj;

class CGameServSendHandler : public CGameSpySender
{
public:
	void				SendTo(const void *pData, unsigned long len, const char *sAddr, unsigned long port);
};

class CGameServerShell : public IServerShell
{
	public :

        CGameServerShell(ILTServer *pServerDE);
		~CGameServerShell();

		void	ExitLevel(LTBOOL bSendMsgToClient=LTTRUE);

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		GameType		GetGameType()	const { return (GameType)m_GameInfo.m_byType; }
		GameDifficulty	GetDifficulty() const { return m_eDifficulty; }
		LTBOOL			GetFadeBodies() const { return m_bFadeBodies; }
        LTBOOL           IsMultiplayerTeamBasedGame();

        void        SetUpdateGameServ() { m_bUpdateGameServ = LTTRUE; }
        LTRESULT     ServerAppMessageFn(char* sMsg, int nLen);

        void        PauseGame(LTBOOL b=LTTRUE);

		TimeRamp*	GetTimeRamp(uint32 i);

		CMissionData*	GetMissionData()			  { return &m_MissionData; }
        LTBOOL           UseMissionData()        const { return m_bUseMissionData; }
        void            SetUseMissionData(LTBOOL bUse) { m_bUseMissionData = bUse; }
        ObjectivesList* GetObjectives(uint8 byTeam);
        ObjectivesList* GetCompletedObjectives(uint8 byTeam);

		inline CTeamMgr*			GetTeamMgr()			{ return(&m_TeamMgr); }
		inline CServerOptionMgr*	GetServerOptionMgr()	{ return(&m_ServerOptionMgr); }

		LTBOOL		InitGameSpy();
		LTBOOL		UpdateGameSpyMgr();
		void		TermGameSpy();

		char*		GetCurLevel();
		char*		GetHostName();
		char*		GetGameSpyGameType();
		int			GetNumPlayers();
		int			GetMaxPlayers();
		LTBOOL		FillGameDataStruct(GAMEDATA* pGD);

		CPlayerObj*	GetFirstNetPlayer();
		CPlayerObj*	GetNextNetPlayer();
		int			GetPlayerPing(CPlayerObj* pPlayer);

		LTRESULT	ProcessPacket(char* sData, uint32 dataLen, uint8 senderAddr[4], uint16 senderPort);

		GameStartPoint* FindStartPoint(CPlayerObj* pPlayer);

	protected :

		void		OnAddClient(HCLIENT hClient);
		void		OnRemoveClient(HCLIENT hClient);
        LPBASECLASS OnClientEnterWorld(HCLIENT hClient, void *pClientData, uint32 clientDataLen);
		void		OnClientExitWorld(HCLIENT hClient);

        void        SendPlayerInfoMsgToClients(HCLIENT hClients, CPlayerObj *pPlayer, LTBOOL bNewPlayer);
        void        OnMessage(HCLIENT hSender, uint8 messageID, HMESSAGEREAD hMessage);
		void		OnCommandOn(HCLIENT hClient, int command);
        void        PreStartWorld(LTBOOL bSwitchingWorlds);
		void		PostStartWorld();
		void		CacheFiles();
		void		UpdateClientPingTimes();
        void        Update(LTFLOAT timeElapsed);
		void		SendTimeOfDay(HCLIENT hClient);
		void		SendGameDataToClient(HCLIENT hClient);
		void		SendServerOptionsToClient(HCLIENT hClient);

		void		ShowMultiplayerSummary();

		// Process the network handshaking from a client
		void		ProcessHandshake(HCLIENT hClient, HMESSAGEREAD hMessage);

	private :

		void HandleUpdatePlayerMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
        void HandleCheatCode(CPlayerObj* pPlayer, CheatCode nCheatCode, uint8 nData);
		void HandleCheatSpectatorMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
        void HandlePlayerMsg(HCLIENT hSender, uint8 messageID, HMESSAGEREAD hMessage);
        void HandleCheatRemoveAI(uint8 nData);
        void HandleTriggerBoxCheat(uint8 nData);

		void HandlePlayerSummary(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandlePlayerChangeTeam(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleUpdateServerOptions(HMESSAGEREAD hMessage);
		void HandlePlayerExitLevel(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleLoadGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleSaveGameMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleConsoleTriggerMsg(HCLIENT hSender, HMESSAGEREAD hMessage);
		void HandleConsoleCmdMsg(HCLIENT hSender, HMESSAGEREAD hMessage);

		void HandleMissionInfoMsg(HCLIENT hSender, HMESSAGEREAD hMessage);

		CPlayerObj* CreatePlayer(HCLIENT hClient);
		void RespawnPlayer(CPlayerObj* pPlayer, HCLIENT hClient);

		void UpdateBoundingBoxes();
		void RemoveBoundingBoxes();

		// Server App functions.
		void ServerAppAddClient( HCLIENT hClient );
		void ServerAppRemoveClient( HCLIENT hClient );
		void ServerAppShellUpdate( );
		void ServerAppPreStartWorld( );
		void ServerAppPostStartWorld( );


		TimeRamp		m_TimeRamps[MAX_TIME_RAMPS];
        uint32          m_nTimeRamps;   // Only calculated right before using the time ramps.
										// Based on the first one with m_Time == -1.
        uint32          m_iPrevRamp;    // The previous time ramp we were sitting on (used to
										// trigger time events).

		float			m_ClientPingSendCounter;

		// Time of day variables.
		CVarTrack		m_SayTrack;
		CVarTrack		m_ShowTimeTrack;
		CVarTrack		m_WorldTimeTrack;
		CVarTrack		m_WorldTimeSpeedTrack;
		float			m_TODSeconds;	// Current time of day
        float           m_TODCounter;
        uint8           m_WorldTimeColor[3]; // 0-64 so we don't send updates all the time.
        uint8           m_SunVec[3];

		HSTRING			m_hstrStartPointName;

		CCharacterMgr	m_charMgr;
		HCLIENT			m_aClients[MAX_CLIENTS];

		NetGame			m_GameInfo;
		int				m_nCurLevel;
        LTBOOL           m_bHitFragLimit;
        LTBOOL           m_bUpdateGameServ;
        LTBOOL           m_bGameServHosted;

        uint8           m_nLastLGFlags;
		GameDifficulty	m_eDifficulty;
		LTBOOL			m_bFadeBodies;

		HCONVAR			m_hSwitchWorldVar;
        LTBOOL           m_bFirstUpdate;

		CGlobalServerMgr	m_GlobalMgr;
		CCommandMgr			m_CmdMgr;			// Same as g_pCmdMgr

		CMissionData		m_MissionData;
        LTBOOL               m_bUseMissionData;

		ObjectivesList	m_Objectives[NUM_TEAMS+1];
		ObjectivesList	m_CompletedObjectives[NUM_TEAMS+1];


		LTBOOL			m_bShowMultiplayerSummary;
		LTBOOL			m_bStartNextMultiplayerLevel;
		LevelEnd		m_eLevelEnd;
		LTFLOAT			m_fSummaryEndTime;

        CTeamMgr            m_TeamMgr;
		CServerOptionMgr	m_ServerOptionMgr;
		CMyGameSpyMgr		m_GameSpyMgr;

        LTBOOL              m_bDontUseGameSpy;
		int					m_nGetPlayerIndex;
		int					m_nMaxPlrs;
		char				m_sSpyGameHost[MAX_GEN_STRING];
		char				m_sWorld[MAX_GEN_STRING];
		char				m_sGameSpyGameType[MAX_GEN_STRING];

        CGameServSendHandler    m_SendHandler;

		void CacheModels();
		void CacheTextures();
		void CacheSprites();
		void CacheSounds();
		void SetupGameInfo();
		void SetServerOptions();
		void UpdateMultiplayer();
		void CheckMultiSwitchWorlds();
		void FirstUpdate();
		void UpdateTimeOfDay(LTFLOAT timeElapsed);

        LTRESULT SwitchToWorld(char *pWorldName, char *pNextWorldName);
		void StartNextMultiplayerLevel();
        void ReportError(HCLIENT hClient, uint8 nErrorType);

        LTBOOL UpdateSessionName();
        LTBOOL UpdateGameServer();
		void  FillGameData(GAMEDATA *gameData);

        void        ClearClientList() { for (int i = 0; i < MAX_CLIENTS; i++) { m_aClients[i] = LTNULL; } }
        LTBOOL       AddClientToList(HCLIENT hClient);
        LTBOOL       RemoveClientFromList(HCLIENT hClient);
        LTBOOL       IsClientInList(HCLIENT hClient);
		CPlayerObj*	GetPlayerFromClientList(HCLIENT hClient);

		LTBOOL IsPositionOccupied(LTVector & vPos, CPlayerObj* pPlayer);
};


inline  LTBOOL CGameServerShell::IsMultiplayerTeamBasedGame()
{
	return ((GameType)m_GameInfo.m_byType == COOPERATIVE_ASSAULT);
}

#endif  // __GAME_SERVER_SHELL_H__