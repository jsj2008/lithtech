// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSaveLoadMgr.h
//
// PURPOSE : Manages the Saving and Loading of games for server.
//
// CREATED : 02/07/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERSAVELOADMGR_H__
#define __SERVERSAVELOADMGR_H__

#include "SaveLoadMgr.h"
#include "PlayerTracker.h"

// 
// Globals...
//

class CServerSaveLoadMgr;
extern CServerSaveLoadMgr *g_pServerSaveLoadMgr;

class CServerSaveLoadMgr : public CSaveLoadMgr, public IPlayerTrackerReceiver
{
	public : // Methods...

		CServerSaveLoadMgr( );
		virtual ~CServerSaveLoadMgr( );
	
		// Initializes the object given a profile name.
		virtual bool	Init( char const* pszProfileName, bool bUseMultiplayerFolders );

		// Terminates the object.
		virtual void	Term( );

		// Load new level with no restoring of keepalives or save games.
		bool			LoadNewLevel( char const* pszNewLevel );

		// Switch levels with/without keepalives.
		bool			SwitchLevels( char const* pszNewLevel, bool bDoKeepAliveSave );

		// Transition between levels with transition volumes.
		bool			TransitionLevels( char const* pszNewLevel );

		// Load a game from a specific slot.
		bool			LoadGameSlot( int nSlot );

		// Save a named game to a specific slot.
		bool			SaveGameSlot( int nSlot, const wchar_t *pszSaveName );

		// Handle the loading of the quick save file.
		bool			QuickLoad( );

		// Handle quick saving of the game.
		bool			QuickSave( int nMission, int nLevel );

		// Reload the current level.
		bool			ReloadLevel( );

		// Handle automatic saving of beginning of level.
		bool			ReloadSave( int nMission, int nLevel );

		// Load the checkpointsave file.
		bool			LoadCheckpointSave( );

		// Handle saving of checkpoint.
		bool			SaveCheckpointSave( int nMission, int nLevel );

		// Load the last saved game.
		bool			ContinueGame();

		// Handle messages.
		bool			OnMessage( HCLIENT hSender, ILTMessage_Read& msg );

		// Handle when we have to abort a playertracker.
		virtual void	OnPlayerTrackerAbort( );

		// Test if we are allowed to save.
		bool			CanSaveGame() const;

		// Accessor to the cansave override.
		void			SetCanSaveOverride( bool bCanSaveOverride ) { m_bCanSaveOverride = bCanSaveOverride; }
		bool			GetCanSaveOverride( ) const { return m_bCanSaveOverride; }

		// Called every frame.
		void			Update( );

		// Get the current state the saveloadmgr is in.
		SaveDataState	GetSaveDataState( ) { return m_eSaveDataState; }

		// Set if we are waiting to hear back from the clients to begin an autoload.
		void			WaitForAutoLoadResponses( );

		// Is a level currently being loaded...
		bool			IsLoadingLevel( ) const { return m_bLoadingLevel; }

		// Called before exiting a level to another level.
		void			PreStartWorld( );


	private:

		// Load the level with/without worldobjects.
		bool			LoadLevel( char const* pszFilename, bool bLoadWorldObjects );

		// Save the keepalives to file.
		bool			SaveKeepAlives( char const* pszFilename );

		// Restore objects from a savefile.
		bool			RestoreObjectsFromSaveFile( char const* pszFilename, bool bKeepAlives );

		// Run the loaded world.
		bool			RunWorld( );

		// Sends message to clients asking for savedata.
		bool			AskClientsForSaveData( SaveDataState eSaveDataState );

		// Handle load game
		bool			HandleLoadGame( HCLIENT hSender, ILTMessage_Read& msg );

		// Handle savedata messages in response to AskClientsForSaveData.
		bool			HandleSaveData( HCLIENT hSender, ILTMessage_Read& msg );

		// Handle a client being ready to begin an AutoLoad.
		bool			HandleClientReadyForAutoLoad( HCLIENT hSender, ILTMessage_Read& msg );

		// Clears the client save data from all the players.
		void			ClearClientSaveData( );

		// Finish the savedata state we started.
		bool			FinishSaveData( );

		// Called by HandleSaveData when all clients have reported savedata.
		bool			FinishSwitchLevels( );

		// Called by HandleSaveData when all clients have reported savedata.
		bool			FinishTransitionLevels( );

		// Called by HandleSaveData when all clients have reported savedata.
		bool			FinishSaveGame( );

		// Called by HandleClientReadyForAutoLoad when all clients have responded.
		bool			FinishAutoLoad( );

		// Loads a save file.
		bool			LoadSaveFile( char const* pszSaveIniKey, char const* pszSaveDir, 
										  char const* pszSaveFile );

		// One of the clients couldn't save
		void			HandleSaveFailed();



	// Data.
	private :

		// Our current savedata state.
		SaveDataState	m_eSaveDataState;

		// Used when we are tracking players we sent messages to.
		PlayerTracker	m_SaveDataPlayerTracker;
		
		// Holds new level to go to for switching worlds and transitions.
		std::string		m_sSaveDataNewLevel;

		// Save game info.  Used with QuickSave and SaveGameSlot.
		std::string		m_sSaveGameDir;
		std::string		m_sSaveGameFile;
		std::string		m_sSaveGameKey;
		std::wstring	m_wsSaveGameTitle;

		// Set in OnPlayerTrackerAbort.  Update polls for it.  Tells us to continue with save.
		bool			m_bPlayerTrackerAborted;

		bool			m_bWaitingForAutoLoadResponses;

		// Used when waiting for autoload responses from clients.
		PlayerTracker	m_AutoLoadResponsePlayerTracker;

		// Is a level currently being loaded...
		bool			m_bLoadingLevel;

		// Override to disallow saving.
		bool			m_bCanSaveOverride;
};

#endif // __SERVERSAVELOADMGR_H__
