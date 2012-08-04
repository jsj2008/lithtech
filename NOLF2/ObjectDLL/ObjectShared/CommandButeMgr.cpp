// ----------------------------------------------------------------------- //
//
// MODULE  : CommandButeMgr.cpp
//
// PURPOSE : CommandButeMgr implementation
//
// CREATED : 11/08/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "CommandButeMgr.h"
	#include "CommandMgr.h"
	#include "ClientServerShared.h"
	#include "GameServerShell.h"
	
	extern class CGameServerShell* g_pGameServerShell;
//
// Define...
//

	#define	CBMGR_TAG_GLOBALS		"Globals"
	#define CBMGR_CMD				"Cmd"

//
// Globals...

	CCommandButeMgr *g_pCommandButeMgr = LTNULL;


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::CCommandButeMgr
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCommandButeMgr::CCommandButeMgr( )
:	m_dwLastChecksum	( 0 ),
	m_bValid			( false )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::~CCommandButeMgr
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCommandButeMgr::~CCommandButeMgr( )
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::Init
//
//  PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandButeMgr::Init( const char *szAttributeFile )
{
	if( g_pCommandButeMgr || !szAttributeFile ) return LTFALSE;
	
	m_bValid = false;
    if( !Parse( szAttributeFile )) return LTFALSE;
	m_bValid = true;

	// Set the singelton

	g_pCommandButeMgr = this;

	// Don't read in anything yet.
	// The commands will be read in when entering a new level
	
	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::Term
//
//  PURPOSE:	Clean up
//
// ----------------------------------------------------------------------- //

void CCommandButeMgr::Term( )
{
	m_buteMgr.Term();
	g_pCommandButeMgr = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::PostStartWorld
//
//  PURPOSE:	Read in and process the global commands
//
// ----------------------------------------------------------------------- //

void CCommandButeMgr::PostStartWorld( uint8 nLoadGameFlags )
{
	if( !g_pCommandButeMgr || !g_pCmdMgr )
		return;

	// Only process the globals once...

	if( nLoadGameFlags == LOAD_NEW_GAME )
		ProcessGlobalCmds();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::PostStartWorld
//
//  PURPOSE:	Read in and process the level commands
//
// ----------------------------------------------------------------------- //

void CCommandButeMgr::AllObjectsCreated()
{
	if( !g_pCommandButeMgr || !g_pCmdMgr )
		return;

	// Process level specific commands. Don't do it if we're
	// loading from a save game, since they will be restored
	// as part of the save.
	if( g_pGameServerShell->GetLGFlags( ) != LOAD_RESTORE_GAME )
		ProcessLevelCmds();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::ProcessGlobalCmds
//
//  PURPOSE:	Run through our global commands and process them...
//
// ----------------------------------------------------------------------- //

void CCommandButeMgr::ProcessGlobalCmds( )
{
	if( !g_pCmdMgr )
		return;

	// Read in the Global commands...

	if( m_buteMgr.Exist( CBMGR_TAG_GLOBALS ))
	{
		int		cNumCmds = 0;
		char	szAttribute[256];
		char	szCmd[256];

		
		while( true )
		{
			// Read in the command

			sprintf( szAttribute, "%s%i", CBMGR_CMD, cNumCmds );
			m_buteMgr.GetString( CBMGR_TAG_GLOBALS, szAttribute, szCmd, sizeof( szCmd ) );
			if( !m_buteMgr.Success( ))
				break;

			ConParse cpCmd;
			cpCmd.Init( szCmd );

			if( g_pCommonLT->Parse( &cpCmd ) == LT_OK )
			{
				// We only allow variable declarations for global commands...

				if( (cpCmd.m_nArgs == 3) && (!_stricmp( cpCmd.m_Args[0], "INT" )) )
				{
					// Process it...

					if( g_pCmdMgr->IsValidCmd( szCmd ))
						g_pCmdMgr->Process( szCmd, (ILTBaseClass*)NULL, (ILTBaseClass*)NULL );

					// Get the new var and and let it know it's a global...
				
					VAR_STRUCT *pVar = g_pCmdMgr->GetVar( cpCmd.m_Args[1], false );
					pVar->m_bGlobal = LTTRUE;
				}
			}

			++cNumCmds;
		}
	}

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::ProcessLevelCmds
//
//  PURPOSE:	Run through the current level commands and process them...
//
// ----------------------------------------------------------------------- //

void CCommandButeMgr::ProcessLevelCmds( )
{
	if( !g_pCmdMgr )
		return;

	// Get the world file name and search for the level name...

	char szTag[128] = {0};
	char szWorldFile[128] = {0};
	SAFE_STRCPY( szWorldFile, g_pGameServerShell->GetCurLevel() );

	char *pToken = LTNULL;

	pToken = strtok( szWorldFile, "\\" );
	while( pToken )
	{
		SAFE_STRCPY( szTag, pToken );
		pToken = strtok( NULL, "\\" );
	}


	// Look for an entry in commands.txt for this level...

	if( m_buteMgr.Exist( szTag ))
	{
		int		cNumCmds = 0;
		char	szAttribute[256];
		char	szCmd[256];

		
		while( true )
		{
			// Read in the command

			sprintf( szAttribute, "%s%i", CBMGR_CMD, cNumCmds );
			m_buteMgr.GetString( szTag, szAttribute, szCmd, sizeof( szCmd ) );
			if( !m_buteMgr.Success( ))
				break;

			// Process it...

			if( g_pCmdMgr->IsValidCmd( szCmd ))
				g_pCmdMgr->Process( szCmd, (ILTBaseClass*)NULL, (ILTBaseClass*)NULL );

			++cNumCmds;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::FileChanged
//
//  PURPOSE:	Has the file changed since last checking?
//				You must reload a file to see if it changed.
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandButeMgr::FileChanged( )
{
	if( m_dwLastChecksum != m_buteMgr.GetChecksum() )
	{
		m_dwLastChecksum = m_buteMgr.GetChecksum();
		return LTTRUE;
	}

	// It's the same
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::Pre_Reload
//
//  PURPOSE:	Reload from within DEdit...
//
// ----------------------------------------------------------------------- //

void CCommandButeMgr::Pre_Reload( ILTPreInterface *pInterface )
{
	Term();
	m_buteMgr.Term();

	char szFile[256];
	sprintf(szFile, "%s\\%s", pInterface->GetProjectDir(), CBMGR_DEFAULT_FILE);
    SetInRezFile(LTFALSE);
    
	if( !Init(szFile) )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( " ERROR - %s", m_buteMgr.GetErrorString() );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::PreCheckGlobalCmds
//
//  PURPOSE:	Read in the global commands and send them to the Plugin to be checked
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandButeMgr::Pre_CheckGlobalCmds( ILTPreInterface *pInterface, CCommandMgrPlugin *pCmdMgrPlugin, const char *pCmd )
{
	if( !pInterface || !pCmdMgrPlugin )
		return LTFALSE;

	// Make sure the attribute file was loaded correctly...

	if( !m_bValid )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( " ERROR - File '%s' was not parsed correctly!", CBMGR_DEFAULT_FILE );
		return LTFALSE;
	}

	// Read in the Global commands...

	if( m_buteMgr.Exist( CBMGR_TAG_GLOBALS ))
	{
		int		cNumCmds = 0;
		char	szAttribute[256];
		char	szCmd[256];

		pInterface->CPrint( " - Checking file: '%s'", CBMGR_DEFAULT_FILE );

		
		while( true )
		{
			// Read in the command
			sprintf( szAttribute, "%s%i", CBMGR_CMD, cNumCmds );
			if( !m_buteMgr.Exist( CBMGR_TAG_GLOBALS, szAttribute ) )
				break;

			m_buteMgr.GetString( CBMGR_TAG_GLOBALS, szAttribute, szCmd, sizeof( szCmd ) );
			if( !m_buteMgr.Success( ))
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "    %s - GetString() was unsuccessful for property '%s'", CBMGR_TAG_GLOBALS, szAttribute );
				return LTFALSE;
			}
			

			ConParse cpCmd;
			cpCmd.Init( szCmd );

			if( pInterface->Parse( &cpCmd ) == LT_OK )
			{
				// We only allow variable declarations for global commands...

				if( (cpCmd.m_nArgs == 3) && (!_stricmp( cpCmd.m_Args[0], "INT" )) )
				{
					// Process it...
					
					if( !pCmdMgrPlugin->IsValidCmd( pInterface, szCmd ))
					{
						pInterface->ShowDebugWindow( LTTRUE );
						pInterface->CPrint( "    %s - Property '%s' has invalid command '%s'!", CBMGR_TAG_GLOBALS, szAttribute, szCmd );

						return LTFALSE;
					}
				}
				else
				{
					pInterface->ShowDebugWindow( LTTRUE );
					pInterface->CPrint( "ERROR! - Pre_CheckGlobalCmds()" );
					pInterface->CPrint( "    Command '%s' is not a valid global command!", szCmd );

					return LTFALSE;
				}
			}

			++cNumCmds;
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandButeMgr::Pre_CheckLevelCmds
//
//  PURPOSE:	Read in the level commands and send them to the Plugin to be checked
//
// ----------------------------------------------------------------------- //

LTBOOL CCommandButeMgr::Pre_CheckLevelCmds( ILTPreInterface *pInterface, CCommandMgrPlugin *pCmdMgrPlugin, const char *pCmd )
{
	if( !pInterface || !pCmdMgrPlugin )
		return LTFALSE;

	const char *pLevelTag = pInterface->GetWorldName();
	if( !pLevelTag )
		return LTFALSE;

	if( m_buteMgr.Exist( pLevelTag ))
	{
		int		cNumCmds = 0;
		char	szAttribute[256];
		char	szCmd[256];

		while( true )
		{
			// Read in the command
			sprintf( szAttribute, "%s%i", CBMGR_CMD, cNumCmds );
			m_buteMgr.GetString( pLevelTag, szAttribute, szCmd, sizeof( szCmd ) );
			if( !m_buteMgr.Success( ))
				break;

			if( !pCmdMgrPlugin->IsValidCmd( pInterface, szCmd ))
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "    %s - Property '%s' has invalid command '%s'!", pLevelTag, szAttribute, szCmd );

				return LTFALSE;
			}
			
			++cNumCmds;
		}
	}

	return LTTRUE;
}