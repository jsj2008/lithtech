// ----------------------------------------------------------------------- //
//
// MODULE  : SaveLoadMgr.h
//
// PURPOSE : Manages the Saving and Loading of games.
//
// CREATED : 12/06/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SAVE_LOAD_MGR_H__
#define __SAVE_LOAD_MGR_H__

//
// Defines...
//

#define SLMGR_MAX_SAVE_SLOTS	10
#define SLMGR_MAX_INISTR_LEN	256
#define SLMGR_MAX_INIKEY_LEN	32

#define SAVE_DIR				"Save"
#define SINGLEPLAYER_FOLDER		"SinglePlayer"
#define MULTIPLAYER_FOLDER		"MultiPlayer"
#define	RELOADLEVEL_DIR			"Reload"
#define WORKING_DIR				"Working"
#define QUICKSAVE_DIR			"QuickSave"
#define KEEPALIVE_FILENAME		"Kpalv.sav"
#define TRANSITION_FILENAME		"Trans.sav"
#define RELOADLEVEL_FILENAME	"Reload.sav"
#define QUICKSAVE_FILENAME		"Quick.sav"
#define SAVEGAMEINI_FILENAME	"Save1001.ini"  // v1.001
#define QUICKSAVE_INIKEY		"SaveGame00"
#define CONTINUE_INIKEY			"Continue"
#define RELOADLEVEL_INIKEY		"Reload"
#define SLOTSAVE_INIKEY			"SaveGame"
#define SLOTSAVE_BASE			"Slot"



class CSaveLoadMgr
{
	public : // Methods...

		CSaveLoadMgr( );
		virtual ~CSaveLoadMgr( );

		virtual bool	Init( char const* pszProfileName, bool bUseMultiplayerFolders );
		virtual void	Term( );

		void	SetUseMultiplayerFolders( bool bUseMultiplayerFolders ) { m_bUseMultiplayerFolders = bUseMultiplayerFolders; }
		bool	UseMultiplayerFolders( ) { return m_bUseMultiplayerFolders; }

		bool	DeleteSpecificSaveDir( const char *pName );

		bool	QuickSaveExists();
		bool	ReloadSaveExists();
		bool	CanContinueGame();
		bool	SlotSaveExists( uint32 nSlot );

		// Reads the save information in the save ini.
		bool	ReadSaveINI( const char *pKey, char* pszSaveTitle, uint32 nSaveTitleSize, 
							   char* pszWorldName, uint32 nWorldNameSize,
							   time_t* pSaveTime );

		// Read the continue info, that can then be used with ReadSaveINI.
		bool	ReadContinueINI( char* pszSaveKey, uint32 nSaveKeySize, char* pszSaveDir, 
								uint32 nSaveDirSize, char* pszSaveFile, uint32 nSaveFileSize );

		char const* const GetSlotSaveKey( uint32 nSlot )
		{
			static char szKey[SLMGR_MAX_INIKEY_LEN] = {0};
			sprintf( szKey, "%s%02d", SLOTSAVE_INIKEY, nSlot );

			return szKey;
		}

		bool	ClearWorkingDir();

		char const* GetProfileName( ) const { return m_sProfileName; }

		// Create all the save folders needed.
		bool	BuildProfileSaveDir( );

		// Methods for easily getting paths and filenames..
		char const* const GetProfileSaveDir( const char *pProfile = NULL ) const
		{
			static char szSavePath[MAX_PATH];
			char const* pszProfileName = pProfile ? pProfile : GetProfileName( );
			sprintf( szSavePath, "%s\\%s", SAVE_DIR, pszProfileName );

			return szSavePath;
		}

		char const* const GetRootSaveDir( const char *pProfile = NULL ) const
		{
			static char szSavePath[MAX_PATH];
			char const* pszGameType = m_bUseMultiplayerFolders ? MULTIPLAYER_FOLDER : SINGLEPLAYER_FOLDER;
			sprintf( szSavePath, "%s\\%s", GetProfileSaveDir( ), pszGameType );

			return szSavePath;
		}

		char const* const GetSaveINIFile( const char *pProfile = NULL ) const
		{
			static char szSaveINIPath[MAX_PATH];
			sprintf( szSaveINIPath, "%s\\%s", GetRootSaveDir( pProfile ), SAVEGAMEINI_FILENAME );

			return szSaveINIPath;
		}

		char const* const GetSaveWorkingDir( const char *pProfile = NULL ) const
		{
			static char szWorkingPath[MAX_PATH];
			sprintf( szWorkingPath, "%s\\%s", GetRootSaveDir( pProfile ), WORKING_DIR );

			return szWorkingPath;
		}

		char const* const GetQuickSaveDir( const char *pProfile = NULL ) const
		{
			static char szQSavePath[MAX_PATH];
			sprintf( szQSavePath, "%s\\%s", GetRootSaveDir( pProfile ), QUICKSAVE_DIR );

			return szQSavePath;
		}

		char const* const GetQuickSaveFile( const char *pProfile = NULL ) const
		{
			static char szQSaveFile[MAX_PATH];
			sprintf( szQSaveFile, "%s\\%s", GetRootSaveDir( pProfile ), QUICKSAVE_FILENAME );

			return szQSaveFile;
		}

		char const* const GetSlotSaveDir( uint32 nSlot, const char *pProfile = NULL ) const
		{
			static char szSlotSavePath[MAX_PATH];
			sprintf( szSlotSavePath, "%s\\%s%02d", GetRootSaveDir( pProfile ), SLOTSAVE_BASE, nSlot );

			return szSlotSavePath;
		}

		char const* const GetSlotSaveFile( uint32 nSlot, const char *pProfile = NULL ) const
		{
			static char szSlotSaveFile[MAX_PATH];
			sprintf( szSlotSaveFile, "%s\\%s%02d.sav", GetSlotSaveDir( nSlot, pProfile ), SLOTSAVE_BASE, nSlot );

			return szSlotSaveFile;
		}

		char const* const GetReloadLevelDir( const char *pProfile = NULL ) const
		{
			static char szReloadSavePath[MAX_PATH];
			sprintf( szReloadSavePath, "%s\\%s", GetRootSaveDir( pProfile ), RELOADLEVEL_DIR );

			return szReloadSavePath;
		}

		char const* const GetReloadLevelFile( const char *pProfile = NULL ) const
		{
			static char szReloadSaveFile[MAX_PATH];
			sprintf( szReloadSaveFile, "%s\\%s", GetRootSaveDir( pProfile ), RELOADLEVEL_FILENAME );

			return szReloadSaveFile;
		}

		char const* const GetKeepAliveFile( const char *pProfile = NULL ) const
		{
			static char szSaveFile[MAX_PATH];
			sprintf( szSaveFile, "%s\\%s", GetRootSaveDir( pProfile ), KEEPALIVE_FILENAME );

			return szSaveFile;
		}

		char const* const GetTransitionFile( const char *pProfile = NULL ) const
		{
			static char szSaveFile[MAX_PATH];
			sprintf( szSaveFile, "%s\\%s", GetRootSaveDir( pProfile ), TRANSITION_FILENAME );

			return szSaveFile;
		}

		// Get name of save file for given world name.
		char const* GetWorldSaveFile( char const* pszWorld, char const* pszProfile = NULL )
		{
			static char szSavePath[MAX_PATH];
			char fname[_MAX_FNAME];

			// Split the worldname up into parts so we can get the title.
			_splitpath( pszWorld, NULL, NULL, fname, NULL );

			// Create a save file name based on the world title.
			sprintf( szSavePath, "%s\\%s", GetSaveWorkingDir(), fname );

			return szSavePath;
		}



	protected: // Methods...

		void	WriteSaveINI( const char *pKey, const char* pszSaveTitle, const char* pszWorldName );

		bool	CopyWorkingDir( const char *pDestDir );
		bool	CopyToWorkingDir( const char *pSrcDir );

		bool	SaveExists( char const* pszSaveIniKey, char const* pszSaveFile );

		void	WriteContinueINI( char const* pszSaveKey, char const* pszSaveDir, char const* pszSaveFile );

	private:

		CString		m_sProfileName;
		bool		m_bUseMultiplayerFolders;
};

#endif // __SAVE_LOAD_MGR_H__