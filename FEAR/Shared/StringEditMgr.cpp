// ----------------------------------------------------------------------- //
//
// MODULE  : StringEditMgr.cpp
//
// PURPOSE : Defines the CGameStringEditMgr singleton class.
//
// CREATED : 09/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "StringEditMgr.h"
#include "ltlibraryloader.h"
#include "iltfilemgr.h"
#include "CLTFileToILTInStream.h"
#include "sys/win/mpstrconv.h"

extern ILTCSBase* g_pLTBase;

#if defined(PLATFORM_SEM)
extern "C" MODULE_EXPORT IStringEditMgr* GetIStringEditMgr();
#endif // PLATFORM_SEM

// global pointer for direct access of the string keeper interface
IStringEditMgr* g_pLTIStringEdit = NULL;
// global pointer for direct access of the string keeper database
HSTRINGEDIT g_pLTDBStringEdit = NULL;

// module handle
static HLTMODULE s_hStringEditInst = NULL;

// static database path
static const char* s_szDatabaseFile = NULL;

// constructor
CGameStringEditMgr::CGameStringEditMgr()
{
	m_nStringEditRef = 0;
}

// singleton object
CGameStringEditMgr& CGameStringEditMgr::GetSingleton()
{
	static CGameStringEditMgr s_Singleton;
	return s_Singleton;
}


// called to load the string keeper interface
bool CGameStringEditMgr::LoadInterface( const char *pszDllFile /*= SKDB_DLL_NAME*/ )
{
	++m_nStringEditRef;

	if( g_pLTIStringEdit == NULL )
	{
#if defined(PLATFORM_SEM)
		g_pLTIStringEdit = GetIStringEditMgr();
#else // PLATFORM_SEM
		// The database was not setup yet.  Do so now...
		if( !s_hStringEditInst && pszDllFile )
			s_hStringEditInst = LTLibraryLoader::OpenLibrary( pszDllFile );

		if( s_hStringEditInst )
		{
			// Get the database interface function...

			HLTPROC hProc = LTLibraryLoader::GetProcAddress( s_hStringEditInst, "GetIStringEditMgr" );

			LTASSERT( hProc != NULL, "Unable to retrieve the StringEditMgr function!" );
			if( hProc )
			{
				PFNGETISTRINGEDITMGR SKfn = (PFNGETISTRINGEDITMGR)hProc;
				g_pLTIStringEdit = SKfn();
			}
			else
			{
				FreeStringEditInterface( );
			}
		}

		// if we got the mgr then register the error callback
		if( g_pLTIStringEdit != NULL )
		{
			g_pLTIStringEdit->RegisterErrorCallback( StringEditErrorCallback, NULL );
		}

#endif // PLATFORM_SEM
	}

	return (g_pLTIStringEdit != NULL);
}

// called to release the interface
void CGameStringEditMgr::FreeInterface()
{
	// Decrease the ref...
	--m_nStringEditRef;
	LTASSERT( m_nStringEditRef >= 0, "Freed more string keepers than created!" );

	if( m_nStringEditRef == 0 )
	{
		// No more refs so free it...

		if( g_pLTDBStringEdit != NULL )
		{
			LTERROR( "Database still loaded!  Free the string keeper database prior to unloading the string keeper interface." );
			g_pLTIStringEdit->ReleaseStringEdit( g_pLTDBStringEdit );
		}

		// unregister the callback
		if( g_pLTIStringEdit != NULL )
		{
			g_pLTIStringEdit->UnregisterErrorCallback( StringEditErrorCallback );
		}

		if( s_hStringEditInst )
		{
			LTLibraryLoader::CloseLibrary( s_hStringEditInst );
		}

		s_hStringEditInst	= NULL;
		g_pLTIStringEdit	= NULL;
		g_pLTDBStringEdit	= NULL;
		s_szDatabaseFile	= NULL;
	}
}

// opens a string keeper database
HSTRINGEDIT CGameStringEditMgr::OpenDatabase( const char *szDatabaseFile, bool bInRezTree /*= true*/ )
{
	LTASSERT( szDatabaseFile, "CGameStringEditMgr::OpenDatabase" );
	if( szDatabaseFile == NULL )
		return NULL;

	// make sure we have the string keeper interface
	if( g_pLTIStringEdit == NULL )
	{
		LTERROR( "StringEditRuntime interface not loaded!" );
		return NULL;
	}

	// check if it's already open
	if( (s_szDatabaseFile != NULL) && (g_pLTDBStringEdit != NULL) &&
		LTStrIEquals(s_szDatabaseFile, szDatabaseFile) )
		return g_pLTDBStringEdit;

	// free the existing database if it is open
	if( g_pLTDBStringEdit != NULL )
	{
		g_pLTIStringEdit->ReleaseStringEdit( g_pLTDBStringEdit );
		g_pLTDBStringEdit = NULL;
		s_szDatabaseFile	= NULL;
	}

	// make sure we have a g_pLTBase pointer
	if( (g_pLTBase != NULL) && (g_pLTBase->FileMgr() != NULL) )
	{
		ILTInStream* pStream = g_pLTBase->FileMgr()->OpenFile( szDatabaseFile );
		if( pStream == NULL )
		{
			LTERROR( "Failed to open stream object for read." );
			return NULL;
		}

		g_pLTDBStringEdit = g_pLTIStringEdit->CreateFromFile( pStream );
		if( g_pLTDBStringEdit == NULL )
		{
			pStream->Release();
			LTERROR( "Failed to create string database from stream object." );
			return NULL;
		}

		pStream->Release();
	}
	else
	{
		// We are trying to open a database while not in game. (ie. WorldEdit)
		// So the database can be opened from the full path...

		CLTFileToILTInStream InStream;
		if( !InStream.Open(szDatabaseFile) )
		{
			LTERROR( "Failed to open string database from file." );
			return NULL;
		}

		g_pLTDBStringEdit = g_pLTIStringEdit->CreateFromFile( &InStream );
		if( g_pLTDBStringEdit == NULL )
		{
			LTERROR( "Failed to create string database from file." );
			return NULL;
		}
	}

	// save the pointer to the path name
	// this memory CANNOT be freed after this call
	s_szDatabaseFile = szDatabaseFile;

	return g_pLTDBStringEdit;
}

// closes a database
void CGameStringEditMgr::CloseDatabase( HSTRINGEDIT hDatabase /*= g_pLTDBStringEdit*/ )
{
	// make sure we have the string keeper interface
	if( g_pLTIStringEdit == NULL )
	{
		LTERROR( "StringEditRuntime interface not loaded!" );
		return;
	}

	if( hDatabase == g_pLTDBStringEdit )
	{
		g_pLTDBStringEdit = NULL;
		s_szDatabaseFile	= NULL;
	}

	g_pLTIStringEdit->ReleaseStringEdit( hDatabase );
}

// called whenever there is an error
void CGameStringEditMgr::StringEditErrorCallback( HSTRINGEDIT hStringEdit, const char* szStringID, const char* szError, void* pUserData )
{
	if( g_pLTBase == NULL )
		return;

	if( g_pLTDBStringEdit != hStringEdit )
	{
		g_pLTBase->CPrint( "StringEditRuntime: [Unknown] - '%s', %s", szStringID, szError );
	}
	else
	{
		g_pLTBase->CPrint( "StringEditRuntime: [%s] - '%s', %s", s_szDatabaseFile, szStringID, szError );
	}
}

