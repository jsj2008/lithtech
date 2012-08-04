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
#include "ClientMultiplayerMgr.h"
#include "CMoveMgr.h"
#include "ClientResShared.h"

// 
// Globals...
//

CClientSaveLoadMgr *g_pClientSaveLoadMgr = LTNULL;
	
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientSaveLoadMgr::CClientSaveLoadMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CClientSaveLoadMgr::CClientSaveLoadMgr( )
{
	ASSERT( g_pClientSaveLoadMgr == LTNULL );

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

	ASSERT( g_pClientSaveLoadMgr != LTNULL );

	g_pClientSaveLoadMgr = LTNULL;
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
//  ROUTINE:	CClientSaveLoadMgr::OnCommandOn
//
//  PURPOSE:	Handle a save/load command
//
// ----------------------------------------------------------------------- //



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
	if( !CanSaveGame() && g_pInterfaceMgr->GetGameState() == GS_PLAYING )
		return false;

	g_pGameClientShell->CSPrint( LoadTempString( IDS_QUICKSAVING ));

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

	g_pPlayerMgr->TurnOffAlternativeCamera(CT_FULLSCREEN);

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

bool CClientSaveLoadMgr::SaveGameSlot( uint32 nSlot, char const* pszSaveName )
{
	// Are we trying to save to the Quick save slot?

	if( nSlot == 0 ) 
		return QuickSave();

	if( !pszSaveName )
		return false;

	// Make sure it's valid to save.
	if( !CanSaveGame( ))
		return false;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_GAME );
	cMsg.Writeuint8( nSlot );
	cMsg.WriteString( pszSaveName );
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

	g_pPlayerMgr->TurnOffAlternativeCamera(CT_FULLSCREEN);

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

	g_pPlayerMgr->TurnOffAlternativeCamera(CT_FULLSCREEN);

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_LOAD_GAME );
	cMsg.Writeuint8( kLoadGameTypeReload );
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

	g_pPlayerMgr->TurnOffAlternativeCamera(CT_FULLSCREEN);

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
	//can't save while on the mission summary screen, because we're between worlds
	if (g_pMissionMgr->IsExitingLevel() || g_pMissionMgr->IsGameOver())
		return false;

	//don't save if we're already trying to switch worlds
	if (g_pGameClientShell->GetSwitchingWorldsState() != eSwitchingWorldsStateFinished &&
		g_pGameClientShell->GetSwitchingWorldsState() != eSwitchingWorldsStateNone)
	{
		return false;
	}

	if (!g_pMoveMgr->CanSave())
		return false;

	// Handle single player.
	if( !IsMultiplayerGame( ))
	{
		return (g_pPlayerMgr->IsPlayerInWorld() && 
			!g_pPlayerMgr->IsPlayerDead() && 
			!g_pPlayerMgr->IsUsingExternalCamera());
	}
	// Handle multiplayer.
	else
	{
		// Remote clients can't do saves.
		if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			return false;

		bool bCanSave = false;

		switch( g_pGameClientShell->GetGameType( ))
		{
			case eGameTypeCooperative:
				bCanSave = true;
				break;
			default:
				bCanSave = false;
				break;
		}

		return bCanSave;
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
	if (g_pMissionMgr->IsExitingLevel() || g_pMissionMgr->IsGameOver())
		return false;

	//don't load if we're already trying to switch worlds
	if (g_pGameClientShell->GetSwitchingWorldsState() != eSwitchingWorldsStateFinished &&
		g_pGameClientShell->GetSwitchingWorldsState() != eSwitchingWorldsStateNone)
	{
		return false;
	}


	// Handle single player.
	if( !IsMultiplayerGame( ))
	{
		return true;
	}
	// Handle multiplayer.
	else
	{
		// Remote clients can do saves.
		if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			return false;

		bool bCanSave = false;

		switch( g_pGameClientShell->GetGameType( ))
		{
			case eGameTypeCooperative:
				bCanSave = true;
				break;
			default:
				bCanSave = false;
				break;
		}

		return bCanSave;
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

	// If we're not the host, then let the user know a save is happening.
	// Don't show autosaves.
	if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) && eSaveDataState != eSaveDataAutoSave )
		g_pGameClientShell->CSPrint( LoadTempString( IDS_SAVINGGAME ));
	
	// Give the server our save data.

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_DATA) ;

	// need to check if we're jumping, if we are, refuse to save
	if (g_pMoveMgr->CanSave())
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

	if (!bSucceed)
	{
		// Can't quicksave now...
		g_pGameClientShell->CSPrint(LoadTempString(IDS_CANTQUICKSAVE));
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\Nosave.wav");
	}

	return true;
}