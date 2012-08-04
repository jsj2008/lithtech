// ----------------------------------------------------------------------- //
//
// MODULE  : MissionMgr.h
//
// PURPOSE : Definition of class to handle managment of missions and worlds.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MISSION_MGR_H__
#define __MISSION_MGR_H__

#include "ClientServerShared.h"
#include "MsgIds.h"

class CMissionMgr 
{
	public:

		CMissionMgr();
		~CMissionMgr();

		// Initialize the object.
		bool			Init( );

		// Terminate the object.
		void			Term();

		// Start game from scratch.
		bool			StartGameNew( );

		// Start game from specific level.
		bool			StartGameFromLevel( char const* pszLevelName );

		// Start game from quicksave.
		bool			StartGameFromQuickSave( );

		// Start game from save slot.
		bool			StartGameFromSaveSlot( int nSlot );

		// Start game from reload.
		bool			StartGameFromReload( );

		// Start game from checkpointsave.
		bool			StartGameFromCheckpointSave( );

		// Start game from continue game.
		bool			StartGameFromContinue( );

		// Start game as client of remote server.
		bool			StartGameAsClient( );

		// Start perfromance test level
		bool			StartPerformanceLevel();

		// Handles a client getting handshaking info from server.
		bool			ClientHandshaking( char const* pszWorldName );

		// Call when mission was failed.
		bool			HandleMissionFailed();

		bool			IsCustomLevel()   const {return m_bCustomLevel;}
		uint32			GetCurrentMission() const {return m_nCurrentMission;}
		uint32			GetCurrentLevel()	const {return m_nCurrentLevel;}
		char const*		GetCurrentWorldName() const	{ return m_sCurrentWorldName.c_str( ); }
		bool			IsNewMission( ) const { return m_bNewMission; }
		bool			IsRestoringLevel( ) const { return m_bRestoringLevel; }

		uint32			GetNewMission() const {return m_nNewMission;}
		uint32			GetNewLevel()	const {return m_nNewLevel;}
		char const*		GetNewWorldName() const	{ return m_sNewWorldName.c_str( ); }

		// Handles messages.
		bool			OnMessage( uint8 messageID, ILTMessage_Read& msg );

		// Handle pre-start world.
		bool			PreLoadWorld( char const* pszNewWorldName );

		// Called when interfacemgr is finished doing exit level tasks.
		bool			FinishExitLevel( );

		// Called when interfacemgr is finished doing start game tasks.
		bool			FinishStartGame( );

		
		bool			IsExitingLevel()	{ return m_bExitingLevel; }

		// Check if the last MID_EXIT_LEVEL was also the end of a mission.
		bool			IsExitingMission()	{ return m_bExitingMission; }

		//Has last mission been completed? 
		bool			IsGameOver() {return m_bGameOver;}

		//we've dealt with the end of game summary
		void			ClearGameOver() {m_bGameOver = false;}

		//does the specified world exist?
		bool			WorldExists(const char *pszWorldName);

		// Get the current server gamestate.
		EServerGameState GetServerGameState( ) const { return m_eServerGameState; }

		enum eStartGameState
		{
			eStartGameUnknown,
			eStartGameFromLevel,
			eStartGameFromQuickSave,
			eStartGameFromSaveSlot,
			eStartGameFromReload,
			eStartGameFromCheckpointSave,
			eStartGameFromContinue,
			eStartGameAsClient,
			eStartGameStarted,

		};

		// clear the servers map list
		void		ClearMapList();

		const WStringArray& GetMapList() const {return m_sMapList;}

	private:

		// Finish starting games.
		bool	FinishStartGameFromLevel( );
		bool	FinishStartGameFromQuickSave( );
		bool	FinishStartGameFromSaveSlot();
		bool	FinishStartGameFromReload( );
		bool	FinishStartGameFromCheckpointSave( );
		bool	FinishStartGameFromContinue( );
		bool	FinishStartGameAsClient( );

		// Sets our current world value.
		bool SetCurrentWorld( char const* pszWorldName );

		// Called when the server sends its EServerGameState.
		bool			HandleServerGameState( ILTMessage_Read& msg );

		// Called when exit level is sent from server.
		bool			HandleExitLevel( ILTMessage_Read& msg );

		// Called when end game is sent from server.
		bool			HandleEndGame( ILTMessage_Read& msg );

		// Called when the map rotation is sent from server.
		bool			HandleMapList( ILTMessage_Read& msg );

		// Setup new level information.
		bool			SetNewLevel( char const* pszWorldName );

		// Go to the loading state.
		bool			SetLoadingLevel( );

		// Clears mission information between missions.
		bool			ClearMissionInfo( );

		// Send the message to the server to start game.
		bool			SendStartGameMessage( );


	private:

		bool			m_bCustomLevel;           // Is the current level a custom level
		uint32			m_nCurrentMission;
		uint32			m_nCurrentLevel;
		std::string		m_sCurrentWorldName;	// Current world that's running

		// True after receiving an EndGame message, cleared after leaving the mission summary screen
		bool			m_bGameOver;

		// True, when we're waiting to send the server an exitlevel confirmation.
		bool			m_bExitingLevel;
		
		// True, when we're exiting the last level of a mission
		bool			m_bExitingMission;

		// Used by FinishExitWorld to setup the loading state.
		int				m_nNewMission;
		int				m_nNewLevel;
		std::string		m_sNewWorldName;

		eStartGameState	m_eStartGameState;
		int				m_nSaveSlot;

		// Level switch is a new mission.
		bool			m_bNewMission;

		// Level switch is restored from a previous transition.
		bool			m_bRestoringLevel;

		// Level switch is from a load game and the server is not waiting for us
		bool			m_bServerWaiting;

		// The servers game state.
		EServerGameState m_eServerGameState;

		//server's map rotation
		WStringArray	m_sMapList;	

};

extern CMissionMgr* g_pMissionMgr;

#endif