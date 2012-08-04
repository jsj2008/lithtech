// ----------------------------------------------------------------------- //
//
// MODULE  : GameDatabaseMgr.h
//	
// PURPOSE : GameDatabaseMgr definition - Base class for all DB mgrs
//
// CREATED : 06/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_DATABASE_MGR_H__
#define __GAME_DATABASE_MGR_H__

//
// Includes...
//

#include "DatabaseUtils.h"
#include "resourceextensions.h"

#if defined(PROJECT_DARK)

// DB files are not split for localization.
#define DB_Default_File "Database" FILE_PATH_SEPARATOR "Dark"  RESEXT_DOT(RESEXT_GAMEDATABASE_PACKED)
#define DB_Default_Localized_File DB_Default_File

#elif defined(PROJECT_FEAR)

// DB files are split for localization.
#define DB_Default_File "Database\\" RESEXT_DATABASE_ROOT  RESEXT_DOT(RESEXT_GAMEDATABASE_PACKED)
#define DB_Default_Localized_File "DatabaseLocalized\\" RESEXT_DATABASE_ROOT "L" RESEXT_DOT(RESEXT_GAMEDATABASE_PACKED)
#define DB_USE_CROSS_DB_RECORDLINKS

#endif


//
// Defines...
//

extern IDatabaseMgr *g_pLTDatabase;
extern IDatabaseCreatorMgr *g_pLTDatabaseCreator;


class CGameDatabaseMgr : public CGameDatabaseReader
{
	public :	// Methods...

		CGameDatabaseMgr( );
		virtual ~CGameDatabaseMgr( );

		virtual bool	Init( const char *szDatabaseFile = DB_Default_File ) = 0;

		void	CopyStringValues( HRECORD hRecord, const char *pszAtt, char* paszBuffer, uint32 nMaxStrings, uint32 nMaxStringLen ) const;

#ifdef _CLIENTBUILD
		// Build a wide string from an attribute of a string table id...
		const wchar_t* GetWStringFromId( HRECORD hRecord, const char *pszAttIdName );
#endif

		// Wrappers for the internal database...

		uint32	GetRecordIndex( HRECORD hRecord ) const
		{
			return g_pLTDatabase->GetRecordIndex( hRecord );
		}

		const char* GetRecordName( HRECORD hRecord ) const 
		{
			return g_pLTDatabase->GetRecordName( hRecord );
		}

		uint32	GetNumValues( HATTRIBUTE hAttrib )
		{
			return g_pLTDatabase->GetNumValues( hAttrib );
		}

		uint32	GetNumValues( HRECORD hRecord, const char *pszAtt ) const
		{
			return g_pLTDatabase->GetNumValues( GetAttribute( hRecord, pszAtt ));
		}

		HDATABASE		GetDatabase( ) const { return m_hDatabase; }

		//////////////////////////////////////////////////////////////////////////
		// Function name   : GetRecordLinkToLocalizedDB
		// Description     : Gets a recordlink from the main db into the localization db.
		//						If the database isn't split, it will do a regular record link.
		// Return type     : HRECORD - Resulting record or NULL if not found.
		// Argument        : HATTRIBUTE hOriginAttrib - String based Recordlink attribute in the originating database.
		// Argument        : uint32 nOriginValueIndex - Value index of the hOriginAttrib
		// Argument        : HCATEGORY hTargetCat - Category of the record in the target database.
		//////////////////////////////////////////////////////////////////////////
		HRECORD GetRecordLinkToLocalizedDB( HATTRIBUTE hOriginAttrib, uint32 nOriginValueIndex, HCATEGORY hTargetCat )
		{
			#ifdef DB_USE_CROSS_DB_RECORDLINKS
				return GetCrossDatabaseRecordLink( hOriginAttrib, nOriginValueIndex, hTargetCat );
			#else
				return g_pLTDatabase->GetRecordLink( hOriginAttrib, nOriginValueIndex, hTargetCat );
			#endif // DB_USE_CROSS_DB_RECORDLINKS
		}

	protected :	// Members...

		// The name of the file used for the database...
		std::string		m_sDatabaseFile;

		// The main database handle...
		HDATABASE		m_hDatabase;


	protected : // Methods...

		bool			OpenDatabase( const char *szDatabaseFile, bool bInRezTree = true );

		

};

#endif // __GAME_DATABASE_MGR_H__
