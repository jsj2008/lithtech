// ----------------------------------------------------------------------- //
//
// MODULE  : VersionMgr.h
//
// PURPOSE : Definition of versioning manager
//
// CREATED : 11/16/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __VERSION_MGR_H__
#define __VERSION_MGR_H__

#include "ltbasetypes.h"

extern LTGUID GAMEGUID;

extern uint16 const GAME_HANDSHAKE_VER_MAJOR;
extern uint16 const GAME_HANDSHAKE_VER_MINOR;
extern uint16 const GAME_HANDSHAKE_VER;
extern uint16 const GAME_HANDSHAKE_VER_MASK;

// Defines the retail mod name.  Used to
// compare mp games from any locale.  Does not get localized.
#define RETAIL_MOD_NAME "Retail"
#define RETAILXP_MOD_NAME "RetailExpansion"

class CVersionMgr
{
	public :

		CVersionMgr();

		bool Init( );

		void GetDisplayVersion( wchar_t* pszDisplayVersion, uint32 nDisplayVersionLen ) const;
		const char* GetBuild() const { return m_sBuild.c_str( ); }
		const uint32 GetSaveVersion();
		const char* GetNetVersion() const;
		const char* GetPatchVersion() const { return m_sPatchVersion.c_str( ); }

		void Update();

#ifdef _CLIENTBUILD
		//low violence needs to be implemented on clientside only
		inline bool	IsLowViolence() const { return m_bLowViolence; }

		void GetCDKey( char* pszCDKey, uint32 nCDKeySize );
		void SetCDKey( char const* pszCDKey );
		void GetPreSaleCDKey( char* pszPreSaleCDKey, uint32 nSize );

		// Get absolute path to the game system ini file.
		char const* GetGameSystemIniFile( ) const { return m_sGameSystemIniFile.c_str(); }

#endif

		typedef uint32 TSaveVersion;

		// The SaveVersions should be set to the build number of the released version.  The CurrentBuild
		// should always be set to the curent development build number.  The latest save version should also 
		// equal the CurrentBuild.

		static const TSaveVersion	kSaveVersion__1_0;
		static const TSaveVersion	kSaveVersion__1_03;
		static const TSaveVersion	kSaveVersion__1_04;

	private:

		static const TSaveVersion	kSaveVersion__CurrentBuild;

	public:

		void			SetCurrentSaveVersion( TSaveVersion nCurrentSaveVersion ) { m_nCurrentSaveVersion = nCurrentSaveVersion; }
		TSaveVersion	GetCurrentSaveVersion( ) const;

		void			SetLGFlags( uint8 nLGFlags ) { m_nLastLGFlags = nLGFlags; }
		uint8			GetLGFlags() const {return m_nLastLGFlags;}

	protected :

		// Holds absolute path to the game system ini file.
		std::string		m_sGameSystemIniFile;

#ifdef _CLIENTBUILD

		bool			m_bLowViolence;

#endif // _CLIENTBUILD

		std::string		m_sBuild;
		std::string		m_sPatchVersion;

		// This is the save version of the currently loading save game.
		// This number is only valid during a ILTServer::RestoreObjects call.
		TSaveVersion	m_nCurrentSaveVersion;

		uint8			m_nLastLGFlags;
};

extern CVersionMgr* g_pVersionMgr;

#endif //__VERSION_MGR_H__
