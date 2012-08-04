// ----------------------------------------------------------------------- //
//
// MODULE  : CommandDB.cpp
//
// PURPOSE : Implementation of Command database
//
// CREATED : 03/23/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "CommandDB.h"
#include "CommandMgr.h"
#include "GameServerShell.h"



//
// Defines...
//

//category
const char* const CmdDB_CommandsCat =		"Commands";
const char* const CmdDB_Globals =			"Globals";
const char* const CmdDB_Commands =			"Commands";

//
// Globals...
//
CCommandDB* g_pCommandDB = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandDB::CCommandDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CCommandDB::CCommandDB():	
	CGameDatabaseMgr( ),
	m_hCommandsCat(NULL)
	{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandDB::~CCommandDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CCommandDB::~CCommandDB()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCommandDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CCommandDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pCommandDB = this;

	// Get handles to all of the categories in the database...
	m_hCommandsCat = g_pLTDatabase->GetCategory(m_hDatabase,CmdDB_CommandsCat);


	return true;
}
;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandDB::PostStartWorld
//
//  PURPOSE:	Read in and process the global commands
//
// ----------------------------------------------------------------------- //

void CCommandDB::PostStartWorld( uint8 nLoadGameFlags )
{
	if( !g_pCommandDB || !g_pCmdMgr )
		return;

	// Only process the globals once...

	if( nLoadGameFlags == LOAD_NEW_GAME )
		ProcessGlobalCmds();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandDB::PostStartWorld
//
//  PURPOSE:	Read in and process the level commands
//
// ----------------------------------------------------------------------- //

void CCommandDB::AllObjectsCreated()
{
	if( !g_pCommandDB || !g_pCmdMgr )
		return;

	// Process level specific commands. Don't do it if we're
	// loading from a save game, since they will be restored
	// as part of the save.
	if( g_pGameServerShell->GetLGFlags( ) != LOAD_RESTORE_GAME )
		ProcessLevelCmds();
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandDB::ProcessLevelCmds
//
//  PURPOSE:	Run through the current level commands and process them...
//
// ----------------------------------------------------------------------- //

void CCommandDB::ProcessLevelCmds( )
{
	if( !g_pCmdMgr )
		return;

	char szTag[128] = {0};
	char szWorldFile[128] = {0};
	LTStrCpy( szWorldFile, g_pGameServerShell->GetCurLevel(), LTARRAYSIZE(szWorldFile) );
	char *pToken = NULL;

	pToken = strtok( szWorldFile, "\\" );
	while( pToken )
	{
		LTStrCpy( szTag, pToken, LTARRAYSIZE(szTag) );
		pToken = strtok( NULL, "\\" );
	}

	HRECORD hCommands = g_pLTDatabase->GetRecord(m_hCommandsCat,szTag);
	if (!hCommands)
		return;

	uint32 nCommands = GetNumValues(hCommands,CmdDB_Commands);
	for (uint32 i = 0; i < nCommands; ++i)
	{
		const char* pszCmd = GetString(hCommands,CmdDB_Commands,i);
		if (pszCmd && !LTStrEmpty(pszCmd))
			g_pCmdMgr->QueueCommand( pszCmd, (ILTBaseClass*)NULL, (ILTBaseClass*)NULL );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandDB::ProcessGlobalCmds
//
//  PURPOSE:	Run through our global commands and process them...
//
// ----------------------------------------------------------------------- //

void CCommandDB::ProcessGlobalCmds( )
{
	if( !g_pCmdMgr )
		return;

	HRECORD hGlobals = g_pLTDatabase->GetRecord( m_hCommandsCat, CmdDB_Globals );
	if( !hGlobals )
		return;

	// Add each global command to the CommandMgr.
	uint32 nNumCommands = GetNumValues(hGlobals,CmdDB_Commands);
	for( uint32 nCommand = 0; nCommand < nNumCommands; ++nCommand )
	{
		const char* pszCmd = GetString( hGlobals, CmdDB_Commands, nCommand );
		if( pszCmd && !LTStrEmpty( pszCmd ))
		{
			g_pCmdMgr->AddGlobalCommand( pszCmd, static_cast<ILTBaseClass*>(NULL), static_cast<ILTBaseClass*>(NULL) );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandDB::PreCheckGlobalCmds
//
//  PURPOSE:	Read in the global commands and send them to the Plugin to be checked
//
// ----------------------------------------------------------------------- //

bool CCommandDB::Pre_CheckGlobalCmds( ILTPreInterface *pInterface, CCommandMgrPlugin *pCmdMgrPlugin, const char *pCmd )
{
	if( !pInterface || !pCmdMgrPlugin || !g_pCommandDB )
		return false;

	// Read in the Global commands, if none exist then we'll consider them valid
	HRECORD hGlobals = g_pLTDatabase->GetRecord(m_hCommandsCat,CmdDB_Globals);
	if (!hGlobals)
		return true;

	uint32 nCommands = GetNumValues(hGlobals,CmdDB_Commands);
	for (uint32 i = 0; i < nCommands; ++i)
	{
		const char* pszCmd = GetString(hGlobals,CmdDB_Commands,i);
		if (pszCmd && !LTStrEmpty(pszCmd))
		{
			ConParse cpCmd;
			cpCmd.Init( pszCmd );

			if( pInterface->Parse( &cpCmd ) == LT_OK )
			{
				// We only allow variable declarations for global commands...

				if( (cpCmd.m_nArgs == 3) && (!LTStrICmp( cpCmd.m_Args[0], "INT" )) )
				{
					// Process it...

					if( !pCmdMgrPlugin->IsValidCmd( pInterface, pszCmd ))
					{
						pInterface->ShowDebugWindow( true );
						pInterface->CPrint( "    Record %s - Value %d has invalid command '%s'!", CmdDB_Globals, i, pszCmd );

						return false;
					}
				}
				else
				{
					pInterface->ShowDebugWindow( true );
					pInterface->CPrint( "ERROR! - Pre_CheckGlobalCmds()" );
					pInterface->CPrint( "    Command '%s' is not a valid global command!", pszCmd );

					return false;
				}
			}

		}
	}

	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandDB::Pre_CheckLevelCmds
//
//  PURPOSE:	Read in the level commands and send them to the Plugin to be checked
//
// ----------------------------------------------------------------------- //

bool CCommandDB::Pre_CheckLevelCmds( ILTPreInterface *pInterface, CCommandMgrPlugin *pCmdMgrPlugin, const char *pCmd )
{
	if( !pInterface || !pCmdMgrPlugin )
		return false;

	//get the name of the level
	const char *pLevel = pInterface->GetWorldName();
	if( !pLevel )
		return false;

	//get the commands associated with the level
	HRECORD hCommands = g_pLTDatabase->GetRecord(m_hCommandsCat,pLevel);

	//if we don't find a record, then there are no invalid commands
	if (!hCommands)
		return true;

	uint32 nCommands = GetNumValues(hCommands,CmdDB_Commands);
	for (uint32 i = 0; i < nCommands; ++i)
	{
		const char* pszCmd = GetString(hCommands,CmdDB_Commands,i);
		if( !pCmdMgrPlugin->IsValidCmd( pInterface, pszCmd ))
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "    Record %s - Value %d has invalid command '%s'!", pLevel, i, pszCmd );

			return false;
		}

	}

	return true;
}
