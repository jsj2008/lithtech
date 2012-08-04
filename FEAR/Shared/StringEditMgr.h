// ----------------------------------------------------------------------- //
//
// MODULE  : StringEditMgr.h
//
// PURPOSE : Declares the CGameStringEditMgr singleton class.
//
// CREATED : 09/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __STRINGEDITMGR_H__
#define __STRINGEDITMGR_H__

#include "platform.h"
#include "istringeditmgr.h"
#include "resourceextensions.h"

#if defined(PLATFORM_LINUX)
	#define SKDB_DLL_NAME		"libStringEditRuntime.so"
#else
	#define SKDB_DLL_NAME		"StringEditRuntime.dll"
#endif

#if defined(PROJECT_DARK)
#define FILE_STRINGEDIT_DB	"database_string" FILE_PATH_SEPARATOR "DARK" RESEXT_DOT(RESEXT_STRINGEDIT_PACKED)
#elif defined(PROJECT_FEAR)
#define FILE_STRINGEDIT_DB	"StringDatabase\\" GAME_NAME RESEXT_DOT(RESEXT_STRINGEDIT_PACKED)
#endif

// Defines for getting the string keeper functions...
typedef IStringEditMgr* (*PFNGETISTRINGEDITMGR)();

// String Keeper DLL accessors
extern IStringEditMgr*	g_pLTIStringEdit;
extern HSTRINGEDIT		g_pLTDBStringEdit;

class CGameStringEditMgr
{
private:
	// interface reference count
	int32							m_nStringEditRef;

	// user callback
	static void		StringEditErrorCallback( HSTRINGEDIT hStringEdit, const char* szStringID, const char* szError, void* pUserData );

	// prevent instantiation outside of the singleton
	CGameStringEditMgr();

	// prevent copying
	CGameStringEditMgr( const CGameStringEditMgr& ) { LTERROR("Invalid copy constructor"); }

public:
	bool			LoadInterface( const char *pszDllFile = SKDB_DLL_NAME );
	void			FreeInterface();

	// opens a database
	HSTRINGEDIT	OpenDatabase( const char *szDatabaseFile, bool bInRezTree = true );
	// closes a database
	void			CloseDatabase( HSTRINGEDIT hDatabase = g_pLTDBStringEdit );

	// singleton access
	static CGameStringEditMgr&	GetSingleton();
};

/*
 *	Inlines
 */
// Use these to make sure the database dll is loaded and freed correctly...
// Every load must be matched with a free...
inline bool LoadStringEditInterface( const char *pszDllFile = SKDB_DLL_NAME ){
	return CGameStringEditMgr::GetSingleton().LoadInterface( pszDllFile );
}
inline void FreeStringEditInterface(){
	CGameStringEditMgr::GetSingleton().FreeInterface();
}

// opens and closes a string keeper database

inline HSTRINGEDIT OpenStringEditDatabase( const char *szDatabaseFile = FILE_STRINGEDIT_DB, bool bInRezTree = true ) {
	return CGameStringEditMgr::GetSingleton().OpenDatabase( szDatabaseFile, bInRezTree );
}
inline void CloseStringEditDatabase( HSTRINGEDIT hDatabase = g_pLTDBStringEdit ) {
	CGameStringEditMgr::GetSingleton().CloseDatabase( hDatabase );
}

#endif  // __STRINGEDITMGR_H__

