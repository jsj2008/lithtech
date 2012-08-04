// ----------------------------------------------------------------------- //
//
// MODULE  : GameDatabaseMgr.cpp
//
// PURPOSE : GameDatabaseMgr implementation
//
// CREATED : 06/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "GameDatabaseMgr.h"
	#include "iltfilemgr.h"
	#include "CLTFileToILTInStream.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseMgr::CGameDatabaseMgr()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CGameDatabaseMgr::CGameDatabaseMgr()
:	m_sDatabaseFile		( ),
	m_hDatabase			( NULL )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseMgr::~CGameDatabaseMgr()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CGameDatabaseMgr::~CGameDatabaseMgr()
{
	// Clean up...

	if( m_hDatabase )
		g_pLTDatabase->ReleaseDatabase( m_hDatabase );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseMgr::OpenDatabase()
//
//	PURPOSE:	Open a specific database file...
//
// ----------------------------------------------------------------------- //

bool CGameDatabaseMgr::OpenDatabase( const char *szDatabaseFile, bool bInRezTree /* = true  */ )
{
	if( !g_pLTDatabase || !szDatabaseFile )
		return false;

	// Check if we already have one open.
	if( m_hDatabase )
	{
		// Check if it's the same database.
		if( LTStrIEquals( m_sDatabaseFile.c_str( ), szDatabaseFile ))
		{
			return true;
		}
		// Different database, we'll need to toss the old one.
		else
		{
			g_pLTDatabase->ReleaseDatabase( m_hDatabase );
			m_hDatabase = NULL;
		}
	}

	m_sDatabaseFile = szDatabaseFile;

	//try and open an existing database first
	m_hDatabase = g_pLTDatabase->OpenExistingDatabase(szDatabaseFile);
	if(m_hDatabase)
		return true;

	// Open the file... 
	if( g_pLTBase && bInRezTree)
	{
		// If we have a base interface we know we are running the game and thus need to 
		// open the database file through a stream since its in a REZ file...

		ILTInStream	*pDBStream = g_pLTBase->FileMgr()->OpenFile( szDatabaseFile );
		if( !pDBStream )
		{
			g_pLTBase->CPrint( "ERROR CGameDatabaseMgr couldn't open file: %s!", szDatabaseFile );
			return false;
		}

		// Open the database...
		
		m_hDatabase = g_pLTDatabase->OpenNewDatabase( szDatabaseFile, *pDBStream );
		if( !m_hDatabase )
			return false;

		// Free up the stream...

		pDBStream->Release( );
	}
	else
	{
		// We are trying to open a database while not in game. (ie. WorldEdit)
		// So the database can be opened from the full path...

		CLTFileToILTInStream InFile;
		if(!InFile.Open(szDatabaseFile))
			return false;

		m_hDatabase = g_pLTDatabase->OpenNewDatabase( szDatabaseFile, InFile );
		if( !m_hDatabase )
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseMgr::CopyStringValues()
//
//	PURPOSE:	Copy all the values of a string attribute to a buffer...
//
// ----------------------------------------------------------------------- //

void CGameDatabaseMgr::CopyStringValues( HRECORD hRecord, const char *pszAtt, char *paszBuffer, uint32 nMaxStrings, uint32 nMaxStringLen ) const
{
	if( !hRecord || !pszAtt[0] || !paszBuffer )
		return;

	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute( hRecord, pszAtt );
	uint32 nNumValues = g_pLTDatabase->GetNumValues( hAtt );
	char* pszBufValue;
	
	pszBufValue = paszBuffer;
	for( uint32 nValue = 0; nValue < nNumValues; ++nValue  )
	{
		if( nValue >= nMaxStrings )
			return;
		
		const char *pszValue = g_pLTDatabase->GetString( hAtt, nValue, "" );
		if( LTStrLen( pszValue ) < nMaxStringLen )
			LTStrCpy( pszBufValue, pszValue, nMaxStringLen );
		pszBufValue += nMaxStringLen;
	}
}

#ifdef _CLIENTBUILD

#include "../ClientShellDLL/ClientUtilities.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameDatabaseMgr::GetWStringFromId()
//
//	PURPOSE:	Build a wide string from an attribute of a string table id...
//
// ----------------------------------------------------------------------- //

const wchar_t* CGameDatabaseMgr::GetWStringFromId( HRECORD hRecord, const char *pszAttIdName )
{
	if( !hRecord || !pszAttIdName[0] )
		return L"";

	const char* szStringId = GetString( hRecord, pszAttIdName );
	if( szStringId == NULL )
		return L"";

	return LoadString( szStringId );
}

#endif
