// ----------------------------------------------------------------------- //
//
// MODULE  : GameServerShell.h
//
// PURPOSE : Game's Server Shell - Definitions
//
// CREATED : 9/18/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_SERVER_SHELL_H__
#define __GAME_SERVER_SHELL_H__


#include "iservershell.h"
#include "ClientServerShared.h"
#include "CheatDefs.h"
#include "NetDefs.h"
#include "VarTrack.h"
#include "SoundMgr.h"
#include "CommandMgr.h"
#include "IDList.h"
#include "MsgIDs.h"
#include "ProfileUtils.h"
#include "VersionMgr.h"
#include "MissionDB.h"
#include "WeaponDB.h"
#include "EngineTimer.h"
#include "ltfilewrite.h"
#include "ScmdConsoleDriver_PunkBuster.h"
#ifdef PLATFORM_WIN32
#include <winsock.h>
#endif
#include "EventCaster.h"

class CGlobalServerMgr;
class CPlayerObj;
class CAIClassFactory;
class CObjectTemplateMgr;
class CServerMissionMgr;
class CServerSaveLoadMgr;
class CCharacterMgr;
class CCharacterAlignmentMgr;
class CParsedMsg;
class IGameSpyServer;
class IPunkBusterServer;
class CAIMgr;
class GameClientData;

// Forward declaration of the server shell pointer
class CGameServerShell;
extern CGameServerShell *g_pGameServerShell;

class CGameServerShell : public IServerShellStub
{
	public :

		declare_interface(CGameServerShell);

		CGameServerShell();
		virtual ~CGameServerShell();

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		GameDifficulty	GetDifficulty()	const				{ return m_eDifficulty; }
		void			SetDifficulty(GameDifficulty eDiff)	{ m_eDifficulty = eDiff; }

		void		SetLGFlags( uint8 nLGFlags );
		uint8		GetLGFlags( )	{ return m_nLastLGFlags; }

		void		SetUniqueObjectID( uint32 nID );
		uint32		GetUniqueObjectID() const { return m_nUniqueObjectID; }
		void		IncrementUniqueObjectID() { m_nUniqueObjectID++; }

		void		SetUpdateGameServ()	{ m_bUpdateGameServ = true; }
		LTRESULT	ServerAppMessageFn( ILTMessage_Read& msg );

		// Counted pausing.
		void		PauseGame( bool bPause );

		// Counted pausing, unless bForce is specified.
		void		PauseGame( bool bPause, bool bForce );

		// Single pause allowed for client.
		void		ClientPauseGame( bool bPause );

		// Check if server is paused.
		bool		IsPaused( ) const { return SimulationTimer::Instance().IsTimerPaused( ); }

		// Are we running a performance test?
		bool		IsPerformanceTest() const { return m_NetGameInfo.m_bPerformanceTest; }

		char const*	GetCurLevel();
		void		SetCurLevel( char const* pszWorldName );

		uint32		GetPlayerPing(CPlayerObj* pPlayer);

		void		SetActivePlayer( HOBJECT hActivePlayer ) { m_hActivePlayer = hActivePlayer; }
		HOBJECT		GetActivePlayer( ) { return m_hActivePlayer; }

		NetGameInfo& GetNetGameInfo( ) { return m_NetGameInfo; }

		// Object template mgr access
		CObjectTemplateMgr			*GetObjectTemplates()			{ return m_pObjectTemplates; }
		const CObjectTemplateMgr	*GetObjectTemplates()	const	{ return m_pObjectTemplates; }

		void	SendPlayerInfoMsgToClients(HCLIENT hClients, CPlayerObj *pPlayer, uint8 nInfoType);

		void	ApplyDecal(HOBJECT hModel, HMODELNODE hNode, HRECORD hDecalType, const LTVector& vPos, const LTVector& vDir);

		LTRESULT	ProcessPacket(ILTMessage_Read *pMsg, uint8 senderAddr[4], uint16 senderPort);

		// Is infinite ammo turned on?
		bool	IsInfiniteAmmo() const { return m_bInfiniteAmmo; }

		// Accessor the simulation timer scale.  All simulation timer changes should go through this
		// accessores rather than the engine directly.
		bool	SetSimulationTimerScale( uint32 nNumerator, uint32 nDenominator );

		uint32 GetDecompressedOverridesDataSize() const { return m_nDecompressedOverridesSize; }
		uint32 GetCompressedOverridesDataSize() const { return m_nCompressedOverridesSize; }
		const uint8* const GetCompressedOverridesData() const { return m_pCompressedOverridesBuffer; }

		// write an entry to the multiplayer server scoring log
		void	WriteMultiplayerScoringLogEntry(const char* pszEntry);

		virtual CPlayerObj* CreatePlayer( GameClientData* pGameClientData );

		// Check if we're in slowmo mode.
		bool IsInSlowMo( ) const { return ( GetSlowMoRecord( ) != NULL ); }
		HRECORD GetSlowMoRecord( ) const { return m_hSlowMoRecord; }
		StopWatchTimer& GetSlowMoTimer( ) { return m_SlowMoTimer; }
		bool EnterSlowMo( HRECORD hSlowMoRecord, float fSlowMoCharge, HCLIENT hActivator, uint32 nSlowMoFlags );
		void UpdateSlowMo( );
		void ExitSlowMo( bool bDoTransition, float fCharge );

		typedef std::vector<uint16> InstantDamageTypes;
		bool ShouldSendInstantDamageToClient( uint16 nInstantDamageType );

		typedef std::vector<uint16> DeathDamageTypes;
		bool ShouldSendDeathDamageToClient( uint16 nDeathDamageType );

		// Determines if an object, with the specified user settings, should be in the world due to client settings...
		bool	ShouldRemoveBasedOnClientSettings( uint32 nUserFlags );

		// RemoveClient event is fired when a client is removed.  Sends RemoveClientNotifyParams
		struct RemoveClientNotifyParams : public EventCaster::NotifyParams
		{
			RemoveClientNotifyParams( EventCaster& eventCaster, HCLIENT hClient ) : EventCaster::NotifyParams( eventCaster )
			{
				m_hClient = hClient;
			}

			HCLIENT m_hClient;
		};
		DECLARE_EVENT( RemoveClient );

	protected :

		virtual LTRESULT	OnServerInitialized();
		virtual void		OnServerTerm();

		void			OnAddClient(HCLIENT hClient);
		void			OnRemoveClient(HCLIENT hClient);
		LPBASECLASS		OnClientEnterWorld(HCLIENT hClient);
		void			OnClientExitWorld(HCLIENT hClient);

		virtual	void	OnMessage(HCLIENT hSender, ILTMessage_Read *pMsg);
		void			PreStartWorld(bool bSwitchingWorlds);
		void			PostStartWorld();

		//**************************************************************************
		//SEM interface handling
		//**************************************************************************
	public :
		// Calling these functions enter and exit the server shell to maintain the
		// engine interface overlap
		void		EnterServerShell();
		void		ExitServerShell();

		// Convenience class for entering & exiting the server shell based on scope
		// This is generally only intended for internal use, but may be needed for
		// use callbacks that don't originate from the server shell
		class CServerShellScopeTracker
		{
		public:
			CServerShellScopeTracker()
			{
				if (g_pGameServerShell)
					g_pGameServerShell->EnterServerShell();
			}
			~CServerShellScopeTracker()
			{
				if (g_pGameServerShell)
					g_pGameServerShell->ExitServerShell();
			}
		};


		void			UpdateClientPingTimes();
		virtual void	PreUpdate();
		virtual	void	Update(float timeElapsed);
		virtual void	PostUpdate();

		void		ShowMultiplayerSummary();

		// Called by message from serverapp.
		bool		OnServerShellInit( );

		SwitchingWorldsState GetSwitchingWorldsState( ) const { return m_eSwitchingWorldsState; }

		// Gamespy interface.
		IGameSpyServer* GetGameSpyServer( ) const { return m_pGameSpyServer; }

		// PunkBuster interface.
		IPunkBusterServer* GetPunkBusterServer( ) const { return m_pPunkBusterServer; }

		void	ServerAppAddClient( HCLIENT hClient );
		void	ServerAppRemoveClient( HCLIENT hClient );

		// Sends the simulation timer scale to the client.
		bool	SendSimulationTimerScale( HCLIENT hClient );

		// Records the last time when a player died.
		void	SetLastPlayerDeathTime( double fValue ) { m_fLastPlayerDeathTime = fValue; }
		double	GetLastPlayerDeathTime( ) const { return m_fLastPlayerDeathTime; }

		// PunkBuster callbacks
		static void	PunkBusterGetMaxClientsCallback(int& nMaxClients);
		static void PunkBusterGetServerAddressCallback(char* pszServerAddressBuffer, int nServerAddressBufferLength);
		static void	PunkBusterDropClientCallback(const int nIndex, const char* pszReason);
		static void PunkBusterGetClientStatsCallback(const int nIndex, char* pszStatsBuffer, int nStatsBufferLength);
		static void PunkBusterGetGameVersionCallback(char* pszGameVersionBuffer, int nGameVersionBufferLength);
		static void	PunkBusterGetClientInfoCallback(const int nIndex, 
													char* pszNameBuffer, int nNameBufferLength, 
													char* pszGUIDBuffer, int nGUIDBufferLength, 
													char* pszAddrBuffer, int nAddrBufferLength);
		static void	PunkBusterGetSocketCallback(SOCKET *sock);
		static void	PunkBusterSendGameMessageCallback(const int nIndex, char *data, int datalen);
		static void	PunkBusterDisplayMessageCallback(char *data, int datalen);
		static void	PunkBusterGetMapNameCallback(char *data, int datalen);
		static void	PunkBusterGetServerHostNameCallback(char *data, int datalen);
		static void	PunkBusterGetGameNameCallback(char *data, int datalen);
		static void PunkBusterGetConsoleVariableCallback(const char* pszName, char* pszValueBuffer, int nValueBufferLength);
		static void PunkBusterRestartMapCallback();
		static void PunkBusterChangeMapCallback(const char* pszName);
		static void PunkBusterHandleAdminCommandCallback(const char* pszCommand);
		static void PunkBusterForceSpectatorModeCallback(const int nIndex, bool bForceSpectatorMode);

	protected :


		//			Handle OnMessage Type			(sender,  message)
		void	HandlePlayerUpdate				(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponFire				(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponFinish				(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponFinishRagdoll		(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponReload				(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponSwap				(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponSound				(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponSoundLoop			(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponChange				(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerActivate			(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerClientMsg			(HCLIENT, ILTMessage_Read*);
		void	HandleFragSelf					(HCLIENT, ILTMessage_Read*);
		void	HandleDoDamage					(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerRespawn				(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerEvent				(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerRequestSpectatorMode(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerBroadcast			(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerTeleport			(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerGear				(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerSlowMo				(HCLIENT, ILTMessage_Read*);
		void	HandleMultiplayerUpdate			(HCLIENT, ILTMessage_Read*);
		void	HandleMultiplayerPlayerUpdate	(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerMessage				(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerGhostMessage		(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerChatMode			(HCLIENT, ILTMessage_Read*);
		void	HandlePlayerCheat				(HCLIENT, ILTMessage_Read*);
		void	HandleGamePause					(HCLIENT, ILTMessage_Read*);
		void	HandleGameUnpause				(HCLIENT, ILTMessage_Read*);
		void	HandleLoadGame					(HCLIENT, ILTMessage_Read*);
		void	HandleConsoleTrigger			(HCLIENT, ILTMessage_Read*);
		void	HandleConsoleCommand			(HCLIENT, ILTMessage_Read*);
		void	HandleDifficulty				(HCLIENT, ILTMessage_Read*);
		void	HandleStimulus					(HCLIENT, ILTMessage_Read*);
		void	HandleRenderStimulus			(HCLIENT, ILTMessage_Read*);
		void	HandleObjectAlpha				(HCLIENT, ILTMessage_Read*);
		void	HandleAddGoal					(HCLIENT, ILTMessage_Read*);
		void	HandleRemoveGoal				(HCLIENT, ILTMessage_Read*);
		void	HandleDecision					(HCLIENT, ILTMessage_Read*);
		void	HandleAIDebug					(HCLIENT, ILTMessage_Read*);
		void	HandleClearProgressiveDamage	(HCLIENT, ILTMessage_Read*);
		void	HandleSaveGame					(HCLIENT, ILTMessage_Read*);
		void	HandleConsoleClientFX			(HCLIENT, ILTMessage_Read*);
		void	HandleObjectMessage				(HCLIENT, ILTMessage_Read*);
		void	HandlePickupItemActivate		(HCLIENT, ILTMessage_Read*);
		void	HandlePickupItemActivateEx		(HCLIENT, ILTMessage_Read*);
		void	HandleAnimTrackersMessage		(HCLIENT, ILTMessage_Read*);
		void	HandleWeaponPriority			(HCLIENT, ILTMessage_Read*);
		void	HandleSonic						(HCLIENT, ILTMessage_Read*);
		void	HandleDynAnimProp				(HCLIENT, ILTMessage_Read*);
		void	HandleDropGrenade				(HCLIENT, ILTMessage_Read*);
		void	HandleArrivedAtNode				(HCLIENT, ILTMessage_Read*);
		void	HandleSoundBroadcastDB			(HCLIENT, ILTMessage_Read*);
		void	HandleGoreSettingMessage		(HCLIENT, ILTMessage_Read*);
		void	HandlePerformanceSettingMessage	(HCLIENT, ILTMessage_Read*);
		void	HandlePunkBusterMessage			(HCLIENT, ILTMessage_Read*);

		//		Cheat Type			(player*,     data)
		void	CheatGod			( CPlayerObj *pPlayer, uint32);
		void	CheatAmmo			( CPlayerObj *pPlayer, uint32);
		void	CheatArmor			( CPlayerObj *pPlayer, uint32);
		void	CheatHealth			( CPlayerObj *pPlayer, uint32);
		void	CheatClip			( CPlayerObj *pPlayer, uint32);
		void	CheatInvisible		( CPlayerObj *pPlayer, uint32);
		void	CheatTeleport		( CPlayerObj *pPlayer, uint32);
		void	CheatWeapons		( CPlayerObj *pPlayer, uint32);
		void	CheatTears			( CPlayerObj *pPlayer, uint32);
		void	CheatRemoveAI		( CPlayerObj *pPlayer, uint32);
		void	CheatModSquad		( CPlayerObj *pPlayer, uint32);
		void	CheatFullGear		( CPlayerObj *pPlayer, uint32);
		void	CheatExitLevel		( CPlayerObj *pPlayer, uint32);
		void	CheatNextMission	( CPlayerObj *pPlayer, uint32);
		void	CheatBootPlayer		( CPlayerObj *pPlayer, uint32);
		void	CheatGimmeGun		( CPlayerObj *pPlayer, HWEAPON hWeapon );
		void	CheatGimmeMod		( CPlayerObj *pPlayer, HMOD hMod );
		void	CheatGimmeGear		( CPlayerObj *pPlayer, HGEAR hGear );
		void	CheatGimmeAmmo		( CPlayerObj *pPlayer, HAMMO hAmmo );
		void	CheatBodyGolfing	( CPlayerObj *pPlayer, uint32);

		void	HandleCheatRemoveAI(uint32 nData);
		void	HandleConsoleCmdMsg(HCLIENT hSender, ILTMessage_Read *pMsg);

		// Server App functions.
		void	ServerAppShellUpdate( );
		void	ServerAppPreStartWorld( );
		void	ServerAppPostStartWorld( );

		// Matches a clientname to a clientref.
		CPlayerObj*	MatchClientNameToPlayer( char const* pszClientName );

		// Find a clientref that won't match any existing client.
		CPlayerObj*	FindFreePlayerFromClientRefs( );

		// Pick a clientref for our client.
		CPlayerObj*	PickClientRefPlayer( HCLIENT hClient );

		// Switching world state.
		void	StartSwitchingWorlds( );
		void	UpdateSwitchingWorlds( );
		void	FinishSwitchingWorlds( );

		// Sets state and tells clients.
		void	SetSwitchingWorldsState( SwitchingWorldsState eSwitchingWorldsState );

		// Sets up weaponmgr with weapon restrictions.
		void	ApplyWeaponRestrictions( );

		// helper for loading multiplayer database overrides
		bool	LoadMultiplayerOverrides(ILTInStream& OverridesStream);

		// helper for initializing the multiplayer scoring log
		bool	InitializeMultiplayerScoringLog();

		// helper for building the GameSpy overrides text string
		bool    BuildDownloadableArchiveFilesString(std::string& strDownloadableArchiveFiles);

		// Removes object that should not be in the world due to client settings...
		void	RemoveObjectsBasedOnClientSettings( );
		bool	ShouldRemoveObjectBasedOnClientSettings( HOBJECT hObject );


	protected:

		float				m_ClientPingSendCounter;

		// Time of day variables.
		VarTrack			m_SayTrack;

		CCharacterMgr*			m_pCharacterMgr;

		CAIClassFactory*		m_pAIClassFactory;
		
		CAIMgr*					m_pAIMgr;

		bool				m_bUpdateGameServ;
		NetGameInfo			m_NetGameInfo;

		uint8				m_nLastLGFlags;
		GameDifficulty		m_eDifficulty;
		bool				m_bGoreAllowed;
		EEngineLOD			m_eWorldObjectsLOD;

		bool				m_bFirstUpdate;

		// The GlobalServerMgr handles the Init of the the ButeFiles as well
		// as the the destruction of the ButeFiles on deletion.
		CGlobalServerMgr*	m_pGlobalMgr;
		CCommandMgr			m_CmdMgr;			// Same as g_pCmdMgr

		int			m_nGetPlayerIndex;
		std::string	m_sWorld;

		bool		InitGameType( NetGameInfo const& netGameInfo );
		void		UpdateMultiplayer();
		void		FirstUpdate();

		bool		StartNextMultiplayerLevel();

		bool		UpdateGameServer();
		bool		UpdateGameSpy( );

		// The object template mgr
		CObjectTemplateMgr *m_pObjectTemplates;

		CServerMissionMgr*		m_pServerMissionMgr;
		CServerSaveLoadMgr*		m_pServerSaveLoadMgr;

		LTObjRef		m_hActivePlayer;

		int			m_nPauseCount;
		bool		m_bClientPaused;

		SwitchingWorldsState	m_eSwitchingWorldsState;

		CVersionMgr	m_VersionMgr; // Same as g_pVersionMgr

		uint32	m_nLastPublishTime;

		// Sequential ID assigned to unnamed game objects (e.g. name29)
		uint32	m_nUniqueObjectID;

		// Gamespy interface.
		IGameSpyServer* m_pGameSpyServer;

		// PunkBuster interface.
		IPunkBusterServer* m_pPunkBusterServer;

		bool			m_bInfiniteAmmo;

		// The recursion count for tracking the server shell scope
		uint32			m_nEntryCount;
		// The ILTCSBase interface that should be used outside of the current server scope
		ILTCSBase *		m_pExternalScopeBaseInterface;

		// database instances (used for multiplayer overrides)
		HDATABASE		m_hGameDatabase;
		HDATABASE		m_hOverridesDatabase;

		// multiplayer overrides buffers (decompressed is used during server publish, compressed
		// is sent to connecting clients)
		uint32			m_nDecompressedOverridesSize;
		uint8*			m_pCompressedOverridesBuffer;
		uint32			m_nCompressedOverridesSize;
		char*			m_pszServerOverrides;

		// multiplayer scoring log file
		CLTFileWrite	m_cMultiplayerScoringLogFile;

		// Tracks time we stay in slowmo.
		StopWatchTimer m_SlowMoTimer;
		// The record defining the slowmo data for our current slowmo mode.
		HRECORD		m_hSlowMoRecord;
		// Team that activated slowmo in mp.
		uint8		m_nSlowMoActivatorTeamId;

		InstantDamageTypes m_lstSendInstantDamageTypes;
		DeathDamageTypes m_lstSendDeathDamageTypes;

		// Records the last time a player died.
		double m_fLastPlayerDeathTime;

		// SCMD console driver for PunkBuster web interface
		ScmdConsoleDriver_PunkBuster m_cScmdPunkBusterDriver;		
};

extern class CGameServerShell* g_pGameServerShell;


#endif  // __GAME_SERVER_SHELL_H__
