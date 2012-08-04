// ----------------------------------------------------------------------- //
//
// MODULE  : GameServerShell.h
//
// PURPOSE : Game's Server Shell - Definition
//
// CREATED : 9/18/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_SERVER_SHELL_H__
#define __GAME_SERVER_SHELL_H__


#include "iservershell.h"
#include "ClientServerShared.h"
#include "CheatDefs.h"
#include "NetDefs.h"
#include "CVarTrack.h"
#include "AttachButeMgr.h"
#include "ServerButeMgr.h"
#include "ModelButeMgr.h"
#include "SoundMgr.h"
#include "CommandMgr.h"
#include "IDList.h"
#include "MusicMgr.h"
#include "MsgIds.h"
#include "ProfileUtils.h"

#define MAX_CLIENTS		MAX_MULTI_PLAYERS
#define MAX_TIME_RAMPS	12
#define MAX_GEN_STRING	128

class CAIStimulusMgr;
class CAICentralKnowledgeMgr;
class CGlobalServerMgr;
class CPlayerObj;
class GameStartPoint;
class CAIClassFactory;
class CServerTrackedNodeMgr;
class IServerDirectory;
class CObjectTemplateMgr;
class CLiteObjectMgr;
class CServerMissionMgr;
class CServerSaveLoadMgr;
class Body;
class CCharacterMgr;
class CParsedMsg;
struct ObjectiveMsgInfo;

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


struct ClientData
{
	ClientData( );

	HCLIENT m_hClient;
	uint32 m_nLastClientUpdateTime;
};


class CGameServerShell : public IServerShellStub
{
	public :

        CGameServerShell();
		virtual ~CGameServerShell();

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);


		ServerGameOptions const& GetServerGameOptions( ) const { return m_ServerGameOptions; }
		ServerGameOptions& GetServerGameOptions( ) { return m_ServerGameOptions; }

		GameType		GetGameType()	const { return m_ServerGameOptions.m_eGameType; }
		GameDifficulty	GetDifficulty() const { return m_eDifficulty; }
		void			SetDifficulty(GameDifficulty eDiff)  { m_eDifficulty = eDiff; }

		void		SetLGFlags( uint8 nLGFlags );
		uint8		GetLGFlags( ) { return m_nLastLGFlags; }

        void        SetUpdateGameServ() { m_bUpdateGameServ = LTTRUE; }
        LTRESULT     ServerAppMessageFn( ILTMessage_Read& msg );

		// Counted pausing.
        void        PauseGame( bool bPause );

		// Counted pausing, unless bForce is specified.
		void		PauseGame( bool bPause, bool bForce );

		// Single pause allowed for client.
		void		ClientPauseGame( bool bPause );

		// Check if server is paused.
		bool		IsPaused( );

		// Are we running a performance test?
		bool		IsPerformanceTest() const { return m_ServerGameOptions.m_bPerformanceTest; }

		// Check if the client wants us to pre-cache assets
		bool		PreCacheAssets() {return m_bPreCacheAssets; }

		TimeRamp*	GetTimeRamp(uint32 i);

		IDList*		GetObjectives()				{ return &m_Objectives; }
		IDList*		GetOptionalObjectives()		{ return &m_OptionalObjectives; }
		IDList*		GetCompletedObjectives()	{ return &m_CompletedObjectives; }
		IDList*		GetParameters()				{ return &m_Parameters; }

		char const*	GetCurLevel();
		void		SetCurLevel( char const* pszWorldName );
		char const*	GetHostName();
		int			GetNumPlayers();

		CPlayerObj*	GetFirstNetPlayer();
		CPlayerObj*	GetNextNetPlayer();
		uint32		GetPlayerPing(CPlayerObj* pPlayer);
		bool		ClientConnectionInTrouble( HCLIENT hClient );

		void		SetActivePlayer( HOBJECT hActivePlayer ) { m_hActivePlayer = hActivePlayer; }
		HOBJECT		GetActivePlayer( ) { return m_hActivePlayer; }

		LTRESULT	ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort);

		GameStartPoint* FindStartPoint(CPlayerObj* pPlayer);
		virtual bool DropInventoryObject(Body* pBody) = 0;

		// Server directory access
		// Note : The server directory is owned by this object once you pass it in.
		IServerDirectory *GetServerDir() { return m_pServerDir; }
		void		SetServerDir(IServerDirectory *pDir);

		// Get/Set the source address of the message which is currently being processed
		void		SetCurMessageSource(const uint8 aAddr[4], uint16 nPort);
		void		GetCurMessageSource(uint8 aAddr[4], uint16 *pPort);

		// Object template mgr access
		CObjectTemplateMgr *GetObjectTemplates() { return m_pObjectTemplates; }
		const CObjectTemplateMgr *GetObjectTemplates() const { return m_pObjectTemplates; }

		// Lite object mgr access
		CLiteObjectMgr *GetLiteObjectMgr() { return m_pLiteObjectMgr; }
		const CLiteObjectMgr *GetLiteObjectMgr() const { return m_pLiteObjectMgr; }

		// Cap the number of bodies in a radius.
		virtual bool	IsCapNumberOfBodies( ) { return false; }
		
		// Are we able to use the radar functionality
		virtual bool	ShouldUseRadar( ) { return false; }

		// Get the min music mood for the current level.
		CMusicMgr::Mood	GetMinMusicMood() const { return m_eMinMusicMood; }

		bool	ProcessObjectiveMessage( const CParsedMsg &cMsg, ObjectiveMsgInfo *pInfo );
		void	ResetObjectives();
		
		// These are called by CPlayerObj so the objectives will properly transition.
		void	SaveObjectives( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void	LoadObjectives( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

        void        SendPlayerInfoMsgToClients(HCLIENT hClients, CPlayerObj *pPlayer, uint8 nInfoType);

	protected :

	    virtual LTRESULT	OnServerInitialized();
		virtual void		OnServerTerm();

		void		OnAddClient(HCLIENT hClient);
		void		OnRemoveClient(HCLIENT hClient);
        LPBASECLASS OnClientEnterWorld(HCLIENT hClient);
		void		OnClientExitWorld(HCLIENT hClient);

        virtual void OnMessage(HCLIENT hSender, ILTMessage_Read *pMsg);
        void        PreStartWorld(bool bSwitchingWorlds);
		void		PostStartWorld();
		void		CacheFiles();
		void		UpdateClientPingTimes();
        virtual void Update(LTFLOAT timeElapsed);
		void		SendMultiPlayerDataToClient(HCLIENT hClient);
		void		SendObjectivesDataToClient( HCLIENT hClient );

		void		ShowMultiplayerSummary();

		// Process the network handshaking from a client
		void		ProcessHandshake(HCLIENT hClient, ILTMessage_Read *pMsg);

		// Called by message from serverapp.
		bool		OnServerShellInit( );


	protected :

		
		//			Handle OnMessage Type			(sender,  message)
        void        HandlePlayerUpdate				(HCLIENT, ILTMessage_Read*);
        void        HandleWeaponFire				(HCLIENT, ILTMessage_Read*);
		void		HandleWeaponReload				(HCLIENT, ILTMessage_Read*);
		void		HandleWeaponSound				(HCLIENT, ILTMessage_Read*);
        void		HandleWeaponSoundLoop			(HCLIENT, ILTMessage_Read*);
		void        HandleWeaponChange				(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerActivate			(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerClientMsg			(HCLIENT, ILTMessage_Read*);
		void		HandleFragSelf					(HCLIENT, ILTMessage_Read*);
        void        HandlePlayerRespawn				(HCLIENT, ILTMessage_Read*);
        void        HandlePlayerCancelRevive		(HCLIENT, ILTMessage_Read*);
        void        HandleClientLoaded				(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerTaunt				(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerTeleport			(HCLIENT, ILTMessage_Read*);
		void		HandleMultiplayerUpdate			(HCLIENT, ILTMessage_Read*);
		void		HandleMultiplayerServerDir		(HCLIENT, ILTMessage_Read*);
		void		HandleMultiplayerInit			(HCLIENT, ILTMessage_Read*);
		void		HandleMultiplayerPlayerUpdate	(HCLIENT, ILTMessage_Read*);
		void        HandleHandshake					(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerMessage				(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerGhostMessage		(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerChatMode			(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerCheat				(HCLIENT, ILTMessage_Read*);
		void		HandlePlayerSkills				(HCLIENT, ILTMessage_Read*);
		void        HandleGamePause					(HCLIENT, ILTMessage_Read*);
		void        HandleGameUnpause				(HCLIENT, ILTMessage_Read*);
		void        HandleLoadGame					(HCLIENT, ILTMessage_Read*);
		void		HandleConsoleTrigger			(HCLIENT, ILTMessage_Read*);
		void        HandleConsoleCommand			(HCLIENT, ILTMessage_Read*);
		void		HandleDifficulty				(HCLIENT, ILTMessage_Read*);
		void		HandleDefault					(HCLIENT, ILTMessage_Read*);
		void		HandleStimulus					(HCLIENT, ILTMessage_Read*);
		void		HandleRenderStimulus			(HCLIENT, ILTMessage_Read*);
		void		HandleObjectAlpha				(HCLIENT, ILTMessage_Read*);
		void		HandleAddGoal					(HCLIENT, ILTMessage_Read*);
		void		HandleRemoveGoal				(HCLIENT, ILTMessage_Read*);
		void		HandleGadgetTarget				(HCLIENT, ILTMessage_Read*);
		void		HandleSearch					(HCLIENT, ILTMessage_Read*);
		void		HandleDecision					(HCLIENT, ILTMessage_Read*);
		void		HandleClearProgressiveDamage	(HCLIENT, ILTMessage_Read*);
		void        HandleProjectileMessage         (HCLIENT, ILTMessage_Read*);
		void        HandleSaveGame					(HCLIENT, ILTMessage_Read*);
		void		HandleConsoleClientFX			(HCLIENT, ILTMessage_Read*);
		void		HandleObjectMessage				(HCLIENT, ILTMessage_Read*);
		void		HandlePerformanceMessage		(HCLIENT, ILTMessage_Read*);


		//			Cheat Type				(player*,     data)
		void		CheatGod				(CPlayerObj*, uint32);
		void		CheatAmmo				(CPlayerObj*, uint32);
		void        CheatArmor				(CPlayerObj*, uint32);
		void        CheatHealth				(CPlayerObj*, uint32);
		void		CheatClip				(CPlayerObj*, uint32);
		void		CheatInvisible			(CPlayerObj*, uint32);
		void        CheatTeleport			(CPlayerObj*, uint32);
		void		CheatWeapons			(CPlayerObj*, uint32);
		void		CheatTears				(CPlayerObj*, uint32);
		void		CheatRemoveAI			(CPlayerObj*, uint32);
		void		CheatSnowmobile			(CPlayerObj*, uint32);
		void		CheatModSquad			(CPlayerObj*, uint32);
		void		CheatFullGear			(CPlayerObj*, uint32);
		void		CheatExitLevel			(CPlayerObj*, uint32);
		void		CheatNextMission		(CPlayerObj*, uint32);
		void		CheatBootPlayer			(CPlayerObj*, uint32);
		void		CheatGimmeGun			(CPlayerObj*, uint32);
		void		CheatGimmeMod			(CPlayerObj*, uint32);
		void		CheatGimmeGear			(CPlayerObj*, uint32);
		void		CheatGimmeAmmo			(CPlayerObj*, uint32);
		void        CheatSkillz				(CPlayerObj*, uint32);
		void        CheatBodyGolfing		(CPlayerObj*, uint32);
		void        CheatDefault			(CPlayerObj*, uint32);

        void HandleCheatRemoveAI(uint32 nData);
		void HandleUpdateServerOptions(ILTMessage_Read *pMsg);
		void HandleConsoleCmdMsg(HCLIENT hSender, ILTMessage_Read *pMsg);

		virtual CPlayerObj* CreatePlayer(HCLIENT hClient, ModelId ePlayerModelId) = 0;
		void RespawnPlayer(CPlayerObj* pPlayer, HCLIENT hClient);

		void UpdateBoundingBoxes();
		void RemoveBoundingBoxes();

		// Server App functions.
		void ServerAppAddClient( HCLIENT hClient );
		void ServerAppRemoveClient( HCLIENT hClient );
		void ServerAppShellUpdate( );
		void ServerAppPreStartWorld( );
		void ServerAppPostStartWorld( );

		// Matches a clientname to a clientref.
		CPlayerObj* MatchClientNameToPlayer( char const* pszClientName );

		// Find a clientref that won't match any existing client.
		CPlayerObj* FindFreePlayerFromClientRefs( );

		// Pick a clientref for our client.
		CPlayerObj* PickClientRefPlayer( HCLIENT hClient );

		// Switching world state.
		void StartSwitchingWorlds( );
		void UpdateSwitchingWorlds( );
		void FinishSwitchingWorlds( );

		// Sets state and tells clients.
		void SetSwitchingWorldsState( SwitchingWorldsState eSwitchingWorldsState );

		// Autosave the world
		void AutoSaveWorld( );

		// Sets up weaponmgr with weapon restrictions.
		void ApplyWeaponRestrictions( );

		// Gets the clientdata given a HCLIENT.
		ClientData* GetClientData( HCLIENT hClient );

	protected:

		TimeRamp			m_TimeRamps[MAX_TIME_RAMPS];
        uint32				m_nTimeRamps;   // Only calculated right before using the time ramps.
											// Based on the first one with m_Time == -1.
        uint32				m_iPrevRamp;    // The previous time ramp we were sitting on (used to
											// trigger time events).

		float				m_ClientPingSendCounter;

		// Time of day variables.
		CVarTrack			m_SayTrack;
		CVarTrack			m_ShowTimeTrack;
		CVarTrack			m_WorldTimeTrack;
		CVarTrack			m_WorldTimeSpeedTrack;
		float				m_TODSeconds;	// Current time of day
        float				m_TODCounter;
        uint8				m_SunVec[3];

		HSTRING				m_hstrStartPointName;

		CCharacterMgr*		m_pCharacterMgr;
		HCLIENT				m_aClients[MAX_CLIENTS];

		CServerTrackedNodeMgr*	m_pServerTrackedNodeMgr;

		CAIStimulusMgr*			m_pAIStimulusMgr;
		CAICentralKnowledgeMgr*	m_pAICentralKnowledgeMgr;
		CAIClassFactory*		m_pAIClassFactory;

		ServerGameOptions	m_ServerGameOptions;
        LTBOOL				m_bUpdateGameServ;

        uint8				m_nLastLGFlags;
		GameDifficulty		m_eDifficulty;

		HCONVAR				m_hSwitchWorldVar;
        LTBOOL				m_bFirstUpdate;

		// The GlobalServerMgr handles the Init of the the ButeFiles as well
		// as the the destruction of the ButeFiles on deletion.
		CGlobalServerMgr*	m_pGlobalMgr;
		CCommandMgr			m_CmdMgr;			// Same as g_pCmdMgr

		IDList		m_Objectives;
		IDList		m_OptionalObjectives;
		IDList		m_CompletedObjectives;
		IDList		m_Parameters;


		LTBOOL				m_bShowMultiplayerSummary;
		LTBOOL				m_bStartNextMultiplayerLevel;
		LTFLOAT				m_fSummaryEndTime;

		int					m_nGetPlayerIndex;
		char				m_sSpyGameHost[MAX_GEN_STRING];
		CString				m_sWorld;

		void CacheLevelSpecificFiles();

		bool InitGameType();
		void UpdateMultiplayer();
		void FirstUpdate();

		bool StartNextMultiplayerLevel();
        void ReportError(HCLIENT hClient, uint8 nErrorType);

        LTBOOL UpdateSessionName();
        LTBOOL UpdateGameServer();

        void		ClearClientList() { for (int i = 0; i < MAX_CLIENTS; i++) { m_aClients[i] = LTNULL; } }
        LTBOOL		AddClientToList(HCLIENT hClient);
        LTBOOL		RemoveClientFromList(HCLIENT hClient);
        LTBOOL		IsClientInList(HCLIENT hClient);
		CPlayerObj*	GetPlayerFromClientList(HCLIENT hClient);
		HCLIENT		GetClientFromID(uint32 nID);

		LTBOOL IsPositionOccupied(LTVector & vPos, CPlayerObj* pPlayer);

		// Server directory interface
		IServerDirectory*	m_pServerDir;

		// Current message source
		uint8		m_aCurMessageSourceAddr[4];
		uint16		m_nCurMessageSourcePort;

		// The object template mgr
		CObjectTemplateMgr *m_pObjectTemplates;

		// The lite object mgr
		CLiteObjectMgr	*m_pLiteObjectMgr;

		CServerMissionMgr*		m_pServerMissionMgr;
		CServerSaveLoadMgr*		m_pServerSaveLoadMgr;

		LTObjRef		m_hActivePlayer;

		int			m_nPauseCount;
		bool		m_bClientPaused;

		SwitchingWorldsState	m_eSwitchingWorldsState;

		// Flag to see if we have cached level-specific resources to ensure
		// these files are only cached once per level load...
		bool		m_bCachedLevelFiles;

		// CRC for cshell.dll
		uint32		m_nCShellCRC;

		bool		m_bPreCacheAssets;

		CMusicMgr::Mood	m_eMinMusicMood;

		typedef std::vector< ClientData* > ClientDataList;
		ClientDataList	m_ClientDataList;
};

extern class CGameServerShell* g_pGameServerShell;


#endif  // __GAME_SERVER_SHELL_H__
