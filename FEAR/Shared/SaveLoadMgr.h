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

#include "ltfileoperations.h"

//
// Defines...
//

#define SLMGR_MAX_SAVE_SLOTS 10
#define SLMGR_MAX_INISTR_LEN 256
#define SLMGR_MAX_INIKEY_LEN 32

#define SAVE_DIR                "Save"
#define SINGLEPLAYER_FOLDER     "SinglePlayer"
#define MULTIPLAYER_FOLDER      "MultiPlayer"
#define RELOADLEVEL_DIR         "Reload"
#define CHECKPOINTSAVE_DIR      "Checkpoint"
#define WORKING_DIR             "Working"
#define QUICKSAVE_DIR           "QuickSave"
#define KEEPALIVE_FILENAME      "Kpalv.sav"
#define TRANSITION_FILENAME     "Trans.sav"
#define RELOADLEVEL_FILENAME    "Reload.sav"
#define CHECKPOINTSAVE_FILENAME "Checkpoint.sav"
#define QUICKSAVE_FILENAME      "Quick.sav"
#define SAVEGAMEINI_FILENAME    "Save1001.ini"  // v1.001
#define QUICKSAVE_INIKEY        "SaveGame00"
#define CONTINUE_INIKEY         "Continue"
#define RELOADLEVEL_INIKEY      "Reload"
#define CHECKPOINTSAVE_INIKEY   "CheckpointSave"
#define SLOTSAVE_INIKEY         "SaveGame"
#define SLOTSAVE_BASE           "Slot"


//--------------------------------------------------------------------------------------------
/** @author ????
 *  @date   ??/??/????
 *
 *
 *  @modified @author Jeff Cotton
 *            @date   11/10/2004
 *            Claimed ownership
 */
//--------------------------------------------------------------------------------------------
class CSaveLoadMgr
{
	public : // Methods...

		CSaveLoadMgr();
		virtual ~CSaveLoadMgr();

		virtual bool Init( char const* pszProfileName, bool bUseMultiplayerFolders );
		virtual void Term();

		void SetUseMultiplayerFolders( bool bUseMultiplayerFolders ) { m_bUseMultiplayerFolders = bUseMultiplayerFolders; }
		bool UseMultiplayerFolders( ) { return m_bUseMultiplayerFolders; }

		bool DeleteSpecificSaveDir( const char *pName );

		bool QuickSaveExists();
		bool ReloadSaveExists();
		bool CheckpointSaveExists();
		bool CanContinueGame();
		bool SlotSaveExists( uint32 nSlot );

		// Reads the save information in the save ini.
		bool ReadSaveINI( const char * pKey,
		                  wchar_t*     pwszSaveTitle,
		                  uint32       nSaveTitleSize, 
		                  char*        pszWorldName,
		                  uint32       nWorldNameSize,
		                  time_t*      pSaveTime );

		// Read the continue info, that can then be used with ReadSaveINI.
		bool ReadContinueINI( char*  pszSaveKey,
		                      uint32 nSaveKeySize,
		                      char*  pszSaveDir, 
		                      uint32 nSaveDirSize,
		                      char*  pszSaveFile,
		                      uint32 nSaveFileSize );

		char const * const GetSlotSaveKey( uint32 nSlot ) const
		{
			static char szKey[SLMGR_MAX_INIKEY_LEN] = "";
			LTSNPrintF( szKey, LTARRAYSIZE( szKey ), "%s%02d", SLOTSAVE_INIKEY, nSlot );

			return szKey;
		}

		bool ClearWorkingDir();

		char const* GetProfileName( ) const { return m_sProfileName.c_str(); }

		char const* const GetTransitionFile( char const * pProfile ) const
		{
			static char szSaveFile[MAX_PATH] = "";
			LTSNPrintF( szSaveFile, LTARRAYSIZE( szSaveFile ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), TRANSITION_FILENAME );

			return szSaveFile;
		}

		// Get name of save file for given world name.
		char const* GetWorldSaveFile( char const* pszWorld, char const* pProfile )
		{
			static char szSavePath[MAX_PATH] = "";
			char fname[_MAX_FNAME];

			// Split the worldname up into parts so we can get the title.
			LTFileOperations::SplitPath( pszWorld, NULL, fname, NULL );

			// Create a save file name based on the world title.
			LTSNPrintF( szSavePath, LTARRAYSIZE( szSavePath ), "%s" FILE_PATH_SEPARATOR "%s", GetSaveWorkingDir( pProfile ), fname );

			return szSavePath;
		}

protected:

		// Create all the save folders needed.
		bool	BuildProfileSaveDir( );

		// Methods for easily getting paths and filenames..
		char const* const GetProfileSaveDir( const char *pProfile ) const
		{
			static char szProfileSaveDir[MAX_PATH] = "";

			szProfileSaveDir[0] = '\0';

			if ( pProfile )
			{
				LTStrCat( szProfileSaveDir, SAVE_DIR FILE_PATH_SEPARATOR, MAX_PATH );
				LTStrCat( szProfileSaveDir, pProfile, MAX_PATH );
			}

			return szProfileSaveDir;
		}

		char const* const GetRootSaveDir( const char* pProfile ) const
		{
			static char szSavePath[MAX_PATH] = "";
			char const* pszGameType = m_bUseMultiplayerFolders ? MULTIPLAYER_FOLDER : SINGLEPLAYER_FOLDER;
			LTSNPrintF( szSavePath, LTARRAYSIZE( szSavePath ), "%s" FILE_PATH_SEPARATOR "%s", GetProfileSaveDir( pProfile ), pszGameType );

			return szSavePath;
		}

		char const* const GetSaveINIFile( const char *pProfile ) const
		{
			static char szSaveINIPath[MAX_PATH] = "";
			LTSNPrintF( szSaveINIPath, LTARRAYSIZE( szSaveINIPath ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), SAVEGAMEINI_FILENAME );

			return szSaveINIPath;
		}

		char const* const GetSaveWorkingDir( const char *pProfile ) const
		{
			static char szWorkingPath[MAX_PATH] = "";
			LTSNPrintF( szWorkingPath, LTARRAYSIZE( szWorkingPath ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), WORKING_DIR );

			return szWorkingPath;
		}

		char const* const GetQuickSaveDir( const char *pProfile ) const
		{
			static char szQSavePath[MAX_PATH] = "";
			LTSNPrintF( szQSavePath, LTARRAYSIZE( szQSavePath ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), QUICKSAVE_DIR );

			return szQSavePath;
		}

		char const* const GetQuickSaveFile( const char *pProfile ) const
		{
			static char szQSaveFile[MAX_PATH] = "";
			LTSNPrintF( szQSaveFile, LTARRAYSIZE( szQSaveFile ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), QUICKSAVE_FILENAME );

			return szQSaveFile;
		}

		char const* const GetSlotSaveDir( uint32 nSlot, const char *pProfile ) const
		{
			static char szSlotSavePath[MAX_PATH] = "";
			LTSNPrintF( szSlotSavePath, LTARRAYSIZE( szSlotSavePath ), "%s" FILE_PATH_SEPARATOR "%s%02d", GetRootSaveDir( pProfile ), SLOTSAVE_BASE, nSlot );

			return szSlotSavePath;
		}

		char const* const GetSlotSaveFile( uint32 nSlot, const char *pProfile ) const
		{
			static char szSlotSaveFile[MAX_PATH] = "";
			LTSNPrintF( szSlotSaveFile, LTARRAYSIZE( szSlotSaveFile ), "%s" FILE_PATH_SEPARATOR "%s%02d.sav", GetSlotSaveDir( nSlot, pProfile ), SLOTSAVE_BASE, nSlot );

			return szSlotSaveFile;
		}

		char const* const GetReloadLevelDir( const char *pProfile ) const
		{
			static char szReloadSavePath[MAX_PATH] = "";
			LTSNPrintF( szReloadSavePath, LTARRAYSIZE( szReloadSavePath ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), RELOADLEVEL_DIR );

			return szReloadSavePath;
		}

		char const* const GetReloadLevelFile( const char *pProfile ) const
		{
			static char szReloadSaveFile[MAX_PATH] = "";
			LTSNPrintF( szReloadSaveFile, LTARRAYSIZE( szReloadSaveFile ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), RELOADLEVEL_FILENAME );

			return szReloadSaveFile;
		}

		char const* const GetCheckpointSaveDir( const char *pProfile ) const
		{
			static char szSavePath[MAX_PATH] = "";
			LTSNPrintF( szSavePath, LTARRAYSIZE( szSavePath ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), CHECKPOINTSAVE_DIR );

			return szSavePath;
		}

		char const* const GetCheckpointSaveFile( const char *pProfile ) const
		{
			static char szSaveFile[MAX_PATH] = "";
			LTSNPrintF( szSaveFile, LTARRAYSIZE( szSaveFile ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), CHECKPOINTSAVE_FILENAME );
			return szSaveFile;
		}

		char const* const GetKeepAliveFile( const char *pProfile ) const
		{
			static char szSaveFile[MAX_PATH] = "";
			LTSNPrintF( szSaveFile, LTARRAYSIZE( szSaveFile ), "%s" FILE_PATH_SEPARATOR "%s", GetRootSaveDir( pProfile ), KEEPALIVE_FILENAME );

			return szSaveFile;
		}

		void WriteSaveINI( const char *pKey, const wchar_t* pwszSaveTitle, const char* pszWorldName );

		bool CopyWorkingDir( const char *pDestDir );
		bool CopyToWorkingDir( const char *pSrcDir );

		bool SaveExists( char const* pszSaveIniKey, char const* pszSaveFile );

		void WriteContinueINI( char const* pszSaveKey, char const* pszSaveDir, char const* pszSaveFile );

	private:

		std::string m_sProfileName;
		bool        m_bUseMultiplayerFolders;
};

#endif // __SAVE_LOAD_MGR_H__
