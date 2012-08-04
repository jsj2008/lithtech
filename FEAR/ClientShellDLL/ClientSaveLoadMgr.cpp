// ----------------------------------------------------------------------- //
//
// MODULE  : SaveLoadMgr.cpp
//
// PURPOSE : Manages the Saving and Loading of games..
//
// CREATED : 12/06/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "ClientSaveLoadMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "MsgIds.h"
#include "ClientConnectionMgr.h"
#include "CMoveMgr.h"
#include "HUDMessageQueue.h"
#include "PlayerCamera.h"
#include "PlayerBodyMgr.h"
#include "LadderMgr.h"

// 
// Globals...
//

CClientSaveLoadMgr *g_pClientSaveLoadMgr = NULL;

VarTrack			g_vtCheckPointOptimizeVideoMemory;

	
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::CClientSaveLoadMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CClientSaveLoadMgr::CClientSaveLoadMgr( )
{
	ASSERT( g_pClientSaveLoadMgr == NULL );

	g_pClientSaveLoadMgr = this;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::~CClientSaveLoadMgr
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CClientSaveLoadMgr::~CClientSaveLoadMgr( )
{
	Term( );

	ASSERT( g_pClientSaveLoadMgr != NULL );

	g_pClientSaveLoadMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::Init
//
//  PURPOSE:	Initializes the object.
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::Init( char const* pszProfileName, bool bUseMultiplayerFolders )
{
	if( !CSaveLoadMgr::Init( pszProfileName, bUseMultiplayerFolders ))
		return false;

	g_vtCheckPointOptimizeVideoMemory.Init(g_pLTBase, "CheckPointOptimizeVideoMemory", NULL, 1.0f);

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::Term
//
//  PURPOSE:	Terminates the object.
//
// ----------------------------------------------------------------------- //

void CClientSaveLoadMgr::Term( )
{
	CSaveLoadMgr::Term( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::QuickSave
//
//  PURPOSE:	Handle quick saving of the game...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::QuickSave( )
{
	// Make sure it's valid to save.
	if( !CanSaveGame())
		return false;

	g_pGameMsgs->AddMessage( LoadString( "IDS_QUICKSAVING" ));

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_GAME );
	cMsg.Writeuint8( 0 );
    g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::QuickLoad
//
//  PURPOSE:	Handle the loading of the quick save file...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::QuickLoad( )
{
	if( !CanLoadGame( ) || !QuickSaveExists( ))
		return false;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeQuick );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::SaveGameSlot
//
//  PURPOSE:	Save a named game to a specific slot...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::SaveGameSlot( uint32 nSlot, wchar_t const* pwszSaveName )
{
	// Are we trying to save to the Quick save slot?

	if( nSlot == 0 ) 
		return QuickSave();

	if( !pwszSaveName )
		return false;

	// Make sure it's valid to save.
	if( !CanSaveGame( ))
		return false;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_GAME );
	cMsg.Writeuint8( nSlot );
	cMsg.WriteWString( pwszSaveName );
    g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::LoadGameSlot
//
//  PURPOSE:	Load a game from a specific slot...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::LoadGameSlot( uint32 nSlot )
{
	if( !CanLoadGame( ) || !SlotSaveExists( nSlot )) 
		return false;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeSlot );
	cMsg.Writeuint8( nSlot );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::ReloadLevel
//
//  PURPOSE:	Reload the current level...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::ReloadLevel( )
{
	if( !CanLoadGame( ) || !ReloadSaveExists() ) 
		return false;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeReload );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::LoadCheckpointSave
//
//  PURPOSE:	Load the checkpointsave.
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::LoadCheckpointSave( )
{
	if( !CanLoadGame( ) || !CheckpointSaveExists() ) 
		return false;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeCheckpointSave );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::ContinueGame
//
//  PURPOSE:	Load the last saved game...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::ContinueGame()
{
	// Are we allowed to continue?
	if( !CanLoadGame( ) || !CanContinueGame() ) 
		return false;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeContinue );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::CanSaveGame
//
//  PURPOSE:	Test if we are allowed to save...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::CanSaveGame()	const
{ 
	//saving is currently not supported in Xenon
#if defined (PLATFORM_XENON)
	return false;
#endif

	//can't save while on the mission summary screen, because we're between worlds
	if (g_pMissionMgr->IsExitingLevel() || g_pMissionMgr->IsGameOver())
		return false;

	//don't save if we're already trying to switch worlds
	if (g_pGameClientShell->GetSwitchingWorldsState() != eSwitchingWorldsStateFinished &&
		g_pGameClientShell->GetSwitchingWorldsState() != eSwitchingWorldsStateNone)
	{
		return false;
	}

	if( !g_pMoveMgr->CanSave() || !CPlayerBodyMgr::Instance( ).CanSave( ))
		return false;

	// Handle single player.
	if( !IsMultiplayerGameClient( ))
	{
		return (g_pPlayerMgr->IsPlayerInWorld() && 
			g_pPlayerMgr->IsPlayerAlive() && 
			(g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() != CPlayerCamera::kCM_Cinematic) );
	}
	// Handle multiplayer.
	else
	{
		//no save in multiplayer
		return false;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::CanLoadGame
//
//  PURPOSE:	Test if we are allowed to Load...
//
// ----------------------------------------------------------------------- //

bool CClientSaveLoadMgr::CanLoadGame( ) const
{ 
	//can't load while on the mission summary screen
	if (g_pMissionMgr->IsExitingLevel())
		return false;

	//don't load if we're already trying to switch worlds
	if (g_pGameClientShell->GetSwitchingWorldsState() != eSwitchingWorldsStateFinished &&
		g_pGameClientShell->GetSwitchingWorldsState() != eSwitchingWorldsStateNone)
	{
		return false;
	}


	// Handle single player.
	if( !IsMultiplayerGameClient( ))
	{
		return true;
	}
	// Handle multiplayer.
	else
	{
		//no save in multiplayer
		return false;

	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSaveLoadMgr::OnMessage
//
//	PURPOSE:	Handle messages.
//
// --------------------------------------------------------------------------- //

bool CClientSaveLoadMgr::OnMessage( uint8 messageID, ILTMessage_Read& msg )
{
	switch (messageID)
	{
		case MID_SAVE_DATA:		HandleSaveData			( msg );	return true; break;
		case MID_SAVE_GAME:		HandleSaveGameMsg		( msg );	return true; break;
		default:
			break;
	}

	return false;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSaveLoadMgr::HandleSaveData
//
//	PURPOSE:	Handle savedata message from server.
//
// --------------------------------------------------------------------------- //

bool CClientSaveLoadMgr::HandleSaveData( ILTMessage_Read& msg )
{
	SaveDataState eSaveDataState = static_cast<SaveDataState>(msg.Readuint8());

	switch( eSaveDataState )
	{
		case eSaveDataStateNone:
		case eSaveDataStateSwitchLevels:
		case eSaveDataStateTransitionLevels:
		case eSaveDataReloadSave:
			break;
		case eSaveDataSaveGameSlot:
		case eSaveDataQuickSave:
			if( g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
			{
				g_pGameMsgs->AddMessage( LoadString( "IDS_SAVINGGAME" ));
			}			
			break;
		case eSaveDataCheckpointSave:
			g_pGameMsgs->AddMessage( LoadString( "IDS_SAVINGCHECKPOINT" ));

			if(g_vtCheckPointOptimizeVideoMemory.GetFloat() != 0.0f)
			{
				// Since user is already waiting a few seconds to save, do the
				// optimizevideomemory now.
				g_pInterfaceMgr->DoOptimizeVideoMemory( );
			}
			break;
		default:
			LTERROR( "Invalid save state." );
			break;
	}

	// Give the server our save data.

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_DATA) ;

	// If we're just doing a keepalive save or autosave, then we don't have any failure conditions.
	bool bCanSave = false;
	if( eSaveDataState == eSaveDataStateSwitchLevels ||
		eSaveDataState == eSaveDataReloadSave )
	{
		bCanSave = true;
	}
	else 
	{
		// Movement and/or PlayerBody my not allow saving in certain circumstances...
		bCanSave = CanSaveGame();
	}
	
	if( bCanSave )
	{
		cMsg.Writebool(true);
		g_pGameClientShell->BuildClientSaveMsg( cMsg, eSaveDataState );
	}
	else
	{
		cMsg.Writebool(false);
	}


	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CClientSaveLoadMgr::HandleSaveGameMsg
//
//	PURPOSE:	Handle savedata message from server.
//
// --------------------------------------------------------------------------- //

bool CClientSaveLoadMgr::HandleSaveGameMsg( ILTMessage_Read& msg )
{
	bool bSucceed = msg.Readbool();
	bool bAbort = msg.Readbool( );

	if (!bSucceed)
	{
		if( bAbort )
		{
			g_pInterfaceMgr->LoadFailed( );
			return true;
		}

		// Can't quicksave now...
		g_pGameMsgs->AddMessage(LoadString("IDS_CANTQUICKSAVE"));
		g_pClientSoundMgr->PlayInterfaceDBSound("CantSave");
	}

	return true;
}
