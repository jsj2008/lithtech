// ----------------------------------------------------------------------- //
//
// MODULE  : CommandDB.h
//
// PURPOSE : Definition of Command database
//
// CREATED : 03/23/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMANDDB_H__
#define __COMMANDDB_H__

//
// Includes...
//

#include "GameDatabaseMgr.h"

class CCommandMgrPlugin;
class CCommandDB;
extern CCommandDB* g_pCommandDB;

class CCommandDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CCommandDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {}

	void	PostStartWorld( uint8 nLoadGameFlags );
	void	AllObjectsCreated();


	bool	Pre_CheckGlobalCmds( ILTPreInterface *pInterface, CCommandMgrPlugin *pCmdMgrPlugin, const char *pCmd = NULL );
	bool	Pre_CheckLevelCmds( ILTPreInterface *pInterface, CCommandMgrPlugin *pCmdMgrPlugin, const char *pCmd = NULL );

private : // Methods...

	void	ProcessGlobalCmds( );
	void	ProcessLevelCmds( );

private : // Members...

	HCATEGORY	m_hCommandsCat;

};

#endif  // __COMMANDDB_H__
