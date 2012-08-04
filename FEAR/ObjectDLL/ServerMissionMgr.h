// ----------------------------------------------------------------------- //
//
// MODULE  : ServerMissionMgr.h
//
// PURPOSE : Definition of class to handle managment of missions and worlds.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERMISSIONMGR_H__
#define __SERVERMISSIONMGR_H__

#include "ClientServerShared.h"
#include "PlayerTracker.h"
#include "ServerMissionSettings.h"
#include "GameModeMgr.h"

typedef std::vector< uint32, LTAllocator<uint32, LT_MEM_TYPE_OBJECTSHELL> > Campaign;

class CServerMissionMgr : public IPlayerTrackerReceiver
{
	public:

		CServerMissionMgr();
		virtual ~CServerMissionMgr();

		// Called when level switch is needed.
		bool			ExitLevelSwitch( );

		// Called to force switch to next mission
		bool			NextMission( );

		// Called to force switch to next round, if on last round will call NextMission().
		bool			NextRound( );

		// Called to force switch to specific mission within a campaign.
		bool			SwitchToCampaignIndex( uint32 nCampaignIndex );

		// Called when level switch is needed.
		bool			ExitLevelToLevel( char const* pszNewLevel );

		// Called when loading a game
		bool			ExitLevelToSavedGame( char const* pszNewLevel );

		// Called when level transition is needed.
		bool			ExitLevelTransition( uint32 nNewLevel );

		// Handles messages.
		bool			OnMessage( HCLIENT hSender, ILTMessage_Read& msg );

		// Handles when a player reaches the in world state.
		void			OnPlayerInWorld( CPlayerObj& player );

		// track mission failure states
		void			SetMissionFailed(bool bFail);
		bool			MissionFailed() { return m_bMissionFailed; }

		// Handle when we have to abort a playertracker.
		virtual void	OnPlayerTrackerAbort( );

		bool			IsCustomLevel()   const {return m_bCustomLevel;}
		uint32				GetCurrentMission() const {return m_nCurrentMission;}
		uint32				GetCurrentLevel()	const {return m_nCurrentLevel;}
		uint32				GetCurrentRound( ) const { return m_nCurrentRound; }
		uint32				GetCurrentCampaignIndex( ) const { return m_nCurCampaignIndex; }

		char const*		GetLevelFromMission( uint32 nMission, uint32 nLevel );
		bool			SetMissionBasedOnLevel( char const* pszFilename );
		uint32				FindNextCampaignIndex( uint32 nStartingCampaignIndex, uint32 nMissionIndex );

		//access server options
		ServerMissionSettings	GetServerSettings() const {return m_ServerSettings; }
		void			SetServerSettings(ServerMissionSettings& ServerSettings);

		Campaign&		GetCampaign( ) { return m_Campaign; }

		// Sets a team as the winner.
		void			SetTeamWon( uint8 nTeamId );

		// Checks if an score limit victory condition has been achieved.
		void CheckScoreLimitWin( );

		// Checks if the time limit has run out.
		void CheckTimeLimitWin( );

		// Checks if an eliminationwin has been achieved.
		void CheckEliminationWin( );

		// Checks if we have enough players to play.
		bool CheckMinimumPlayersToPlay( );

		EServerGameState GetServerGameState( ) const { return m_eServerGameState; }

		DECLARE_EVENT( BeginRound );
		DECLARE_EVENT( EndRound );
		DECLARE_EVENT( EndMap );
		DECLARE_EVENT( GameRulesUpdated );

	public:

		// Initializes the object.  Overrides should call up.
		virtual bool	Init( );

		// Terminates the object.  Overrides should call up.
		virtual void	Term();

		// Save/load state of missionmgr.  Overrides should call up.
		virtual bool	Save( ILTMessage_Write& msg, uint32 dwSaveFlags );
		virtual bool	Load( ILTMessage_Read& msg, uint32 dwSaveFlags );

		// Called every frame.  Overrides should call up.
		virtual void	Update( );

		// Called when the clients enter the world
		virtual void	LevelStarted();

		void PostStartWorld( );

		bool IsEndRoundConditionMet( ) const { return ( GetEndRoundCondition() != GameModeMgr::eEndRoundCondition_None ); }
		GameModeMgr::EndRoundCondition GetEndRoundCondition( ) const { return m_eEndRoundCondition; }
		void SetEndRoundCondition( GameModeMgr::EndRoundCondition eEndRoundCondition );

		// Sends the servergamestate to the clients.
		void			SendServerGameState( HCLIENT hClient );

		// Updates the time limit on the server based on game options
		void			UpdateTimeLimit();

		// retrieve the time spent in the current round, returns 0 if not in a playing state or no timer is started
		double			GetTimeInRound();

	protected:

		void			SetServerGameState( EServerGameState eValue );

		// Does loading of next level.
		virtual bool	FinishExitLevel( );

		// Init game.
		virtual bool	StartGame( ILTMessage_Read& msg );

		// Called when the game has finished the last mission.
		virtual bool	EndGame( );

	protected:

		// Handle start of game.
		bool			HandleStartGame( HCLIENT hSender, ILTMessage_Read& msg );

		// Handle start level
		bool			HandleStartLevel( HCLIENT hSender, ILTMessage_Read& msg );

		// Handle client's exitlevel.
		bool			HandleExitLevel( HCLIENT hSender, ILTMessage_Read& msg );

		//handle updating multiplayer options while in game
		virtual bool	HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg );

		// Call to send client exit level message with current level and mission.
		bool			SendExitLevelMessage( );

		// Sets up the campaign to run.  The campaign is the list of missions in
		// the order they should be played.
		bool			SetupCampaign( );

		// Called by exitlevelXXX routines to go to a new level.
		bool			ExitLevel( char const* pszNewLevel, bool bTransitionToLevel );

		// Sets the time left on the countdown timer.
		bool			SetDisplayTimer( double fCountDownTime );

		// writes the end-of-round scores to the scoring log
		void			WriteScoresToLog();
		void			WritePlayerEntryToLog(HCLIENT hClient);

	// Saved data.
	protected:

		// List of missions to play.
		Campaign		m_Campaign;
		uint32			m_nCurCampaignIndex;

		// has the player (or players) failed the mission
		bool			m_bMissionFailed;

		ServerMissionSettings	m_ServerSettings;

	// Unsaved data.
	protected:

		EServerGameState m_eServerGameState;

		// Used when we are tracking players we sent messages to.
		PlayerTracker	m_PlayerTracker;

		// Controls whether we switch levels or transition levels.
		bool			m_bTransitionLevels;

		// Set when we're switching missions.
		bool			m_bNewMission;

		// Set in OnPlayerTrackerAbort.  Update polls for it.  Tells us to continue with save.
		bool			m_bPlayerTrackerAborted;

		bool			m_bCustomLevel;
		uint32				m_nCurrentMission;
		uint32				m_nCurrentLevel;
		uint32				m_nCurrentRound;
		std::string		m_sCurrentWorldName;

		StopWatchTimer	m_tmrStartTime;

		LTObjRef		m_hDisplayTimer;

		// Period of time players are allowed to join.
		StopWatchTimer	m_tmrState;

		// Shows why the round ended.
		GameModeMgr::EndRoundCondition m_eEndRoundCondition;

		// Winning team.
		uint8			m_nWinningTeam;

		// Winning player.
		LTObjRef		m_hWinningPlayer;

		// Set when we first get enough players to play round.
		bool m_bHaveMinimumPlayersToPlay;

		// Set when the level is first started.
		bool m_bLevelStarted;
};

extern CServerMissionMgr* g_pServerMissionMgr;

#endif // __SERVERMISSIONMGR_H__
