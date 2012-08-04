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

typedef std::vector< int > Campaign;

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
		bool			SwitchToCampaignIndex( int nCampaignIndex );

		// Called when level switch is needed.
		bool			ExitLevelToLevel( char const* pszNewLevel );

		// Called when loading a game
		bool			ExitLevelToSavedGame( char const* pszNewLevel );

		// Called when level transition is needed.
		bool			ExitLevelTransition( int nNewLevel );

		// Handles messages.
		bool			OnMessage( HCLIENT hSender, ILTMessage_Read& msg );

		// track mission failure states
		void			SetMissionFailed(bool bFail);
		bool			MissionFailed() { return m_bMissionFailed; }

		// Handle when we have to abort a playertracker.
		virtual void	OnPlayerTrackerAbort( );

		LTBOOL			IsCustomLevel()   const {return m_bCustomLevel;}
		int				GetCurrentMission() const {return m_nCurrentMission;}
		int				GetCurrentLevel()	const {return m_nCurrentLevel;}
		int				GetCurrentRound( ) const { return m_nCurrentRound; }
		int				GetCurrentCampaignIndex( ) const { return m_nCurCampaignIndex; }

		char const*		GetLevelFromMission( int nMission, int nLevel );
		bool			SetMissionBasedOnLevel( char const* pszFilename );
		int				FindNextCampaignIndex( int nStartingCampaignIndex, int nMissionIndex );

		//access server options
		ServerMissionSettings	GetServerSettings() const {return m_ServerSettings; }
		void			SetServerSettings(ServerMissionSettings& ServerSettings);

		Campaign&		GetCampaign( ) { return m_Campaign; }

	// Game type overrides.
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

		// Called to check if gametype allows respawning.
		virtual bool	CanRespawn( CPlayerObj const& player ) const { return true; }

		// Called when the clients enter the world
		virtual void	LevelStarted();

	// Game type overrides.
	protected:

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
		virtual bool	HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg ) {return false;}

		// Call to send client exit level message with current level and mission.
		bool			SendExitLevelMessage( );

		// Sets up the campaign to run.  The campaign is the list of missions in
		// the order they should be played.
		bool			SetupCampaign( char const* pszCampaignName );

		// Called by exitlevelXXX routines to go to a new level.
		bool			ExitLevel( char const* pszNewLevel, bool bTransitionToLevel );

	// Saved data.
	protected:

		// Campaign file specified at MID_STARTGAME.
		CString			m_sCampaignFile;

		// List of missions to play.
		Campaign		m_Campaign;
		int				m_nCurCampaignIndex;

		// Loop when we reach the last level of the last mission.
		bool			m_bLoopCampaign;

		// has the player (or players) failed the mission
		bool			m_bMissionFailed;

		ServerMissionSettings	m_ServerSettings;

	// Unsaved data.
	protected:

		// Used when we are tracking players we sent messages to.
		PlayerTracker	m_PlayerTracker;

		// Controls whether we switch levels or transition levels.
		bool			m_bTransitionLevels;

		// Set when we're switching missions.
		bool			m_bNewMission;

		// Set while we're waiting for exitlevel back from clients.
		bool			m_bExitingLevel;

		// Set in OnPlayerTrackerAbort.  Update polls for it.  Tells us to continue with save.
		bool			m_bPlayerTrackerAborted;

		bool			m_bCustomLevel;
		int				m_nCurrentMission;
		int				m_nCurrentLevel;
		int				m_nCurrentRound;
		CString			m_sCurrentWorldName;

		float			m_fStartTime;

		LTObjRef		m_hDisplayTimer;

};

extern CServerMissionMgr* g_pServerMissionMgr;

#endif // __SERVERMISSIONMGR_H__