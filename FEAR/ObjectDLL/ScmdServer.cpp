// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdServer.cpp
//
// PURPOSE : Server side handling of SCMD commands.  Provides remote control of
//				server.
//
// CREATED : 10/21/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ScmdServer.h"
#include "ScmdShared.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "ServerMissionMgr.h"
#include "MissionDB.h"
#include "BanIPMgr.h"
#include "BanUserMgr.h"
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"
#include "igamespy.h"

class ScmdServer_Impl : public ScmdServer
{
	friend class ScmdServer;

	protected:

		// Not allowed to create directly.  Use Instance().
		ScmdServer_Impl( );

		// Copy ctor and assignment operator not implemented and should never be used.
		ScmdServer_Impl( ScmdServer_Impl const& other );
		ScmdServer_Impl& operator=( ScmdServer_Impl const& other );

	public:

		// This destructor should be private, but if it is, the compiler complains
		// that the Instance function does not have access to it.  Instance should
		// have access since it's a member function.  Compiler bug?
		virtual ~ScmdServer_Impl();

	public:

		// Initializes the object.
		virtual bool Init( );

		// Terminates the object.
		virtual void Term( );

		// Called to handle messages.
		virtual bool OnMessage( HCLIENT hClient, ILTMessage_Read& msg );

		// Called when client drops.
		virtual bool OnRemoveClient( HCLIENT hClient );

		// Gets the current admin control state.
		virtual AdminControl GetAdminControl( ) { return m_eAdminControl; }

		// Get the HCLIENT logged in as admin, NULL for serverapp.
		virtual HCLIENT GetAdminClient( ) { return m_hAdminClient; }

		// make this client the current admin
		virtual bool SetAdminClient(HCLIENT hClient);

		// force the current admin client to be logged out
		virtual void ForceAdminClientLogout();

	private:

		// Handles SCMD messages
		bool HandleScmdMessage( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle login message.
		bool HandleLogin( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle logout message.
		bool HandleLogout( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle clients message.
		bool HandleListClients( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle listmissions message.
		bool HandleListMissions( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle nextmission message.
		bool HandleNextMission( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle nextround message.
		bool HandleNextRound( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle setmission message.
		bool HandleSetMission( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle bootname message.
		bool HandleBootName( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle bootid message.
		bool HandleBootId( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle banuser message.
		bool HandleBanUser( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle unbanuser message.
		bool HandleUnbanUser( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle listuserbans message.
		bool HandleListUserBans( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle listgameoptions message.
		bool HandleListGameOptions( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle showgameoption message.
		bool HandleShowGameOption( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle setgameoption message.
		bool HandleSetGameOption( HCLIENT hClient, ILTMessage_Read& msg );

		// Handles directing message to serverapp or hclient.
		bool SendMessage( HCLIENT hClient, ILTMessage_Write& msg );

		// Sends simple command status message to client.
		bool SendStatusMessage( HCLIENT hClient, ScmdCommand eScmdCommand, 
				ScmdCommandStatus eScmdCommandStatus );

		// helper for handling admin client login
		void LoginAdminClient(HCLIENT hClient);

		// helper for handling admin client logout
		void LogoutAdminClient(HCLIENT hClient);

	private:

		// Object has been initialized.
		bool m_bInitialized;

		// Indicates an admin is currently logged in.
		AdminControl m_eAdminControl;

		// Current admin controller.  NULL if serverapp.
		HCLIENT m_hAdminClient;

		// Password required to take control.
		std::wstring m_sPassword;

		// Indicates someone can login as SCMD admin.
		bool m_bAllowScmdCommands;

};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

ScmdServer& ScmdServer::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static ScmdServer_Impl sSingleton;
	return sSingleton;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::ScmdServer_Impl
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ScmdServer_Impl::ScmdServer_Impl( )
{
	m_bInitialized = false;
	m_eAdminControl = kAdminControlNone;
	m_hAdminClient = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::~ScmdServer_Impl
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ScmdServer_Impl::~ScmdServer_Impl( )
{
	Term( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::Init
//
//	PURPOSE:	Initializes object.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::Init( )
{
	// Start fresh.
	Term( );

	m_eAdminControl = kAdminControlNone;
	m_hAdminClient = NULL;
	m_sPassword = GameModeMgr::Instance( ).m_ServerSettings.m_sScmdPassword;
	m_bAllowScmdCommands = GameModeMgr::Instance( ).m_ServerSettings.m_bAllowScmdCommands;

	m_bInitialized = true;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::Term
//
//	PURPOSE:	Terminates object.
//
// ----------------------------------------------------------------------- //

void ScmdServer_Impl::Term( )
{
	m_bInitialized = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::OnMessage
//
//	PURPOSE:	Handles reading of SCMD message header.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::OnMessage( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Make sure we're initialized.
	if( !m_bInitialized )
		return false;

	msg.SeekTo( 0 );
	uint8 messageID = msg.Readuint8( );

	switch( messageID )
	{
		case MID_SCMD_COMMAND:			
			HandleScmdMessage( hClient, msg );	
			return true;

		default:
			break;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::OnRemoveClient
//
//	PURPOSE:	Handles when a client drops.
//
//	RETURN:		true on success.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::OnRemoveClient( HCLIENT hClient )
{
	// Make sure we're initialized.
	if( !m_bInitialized )
		return false;

	// Check if we don't have a HCLIENT admin.
	if( m_eAdminControl != kAdminControlClient )
		return true;

	// Check if this is not our admin client.
	if( hClient != m_hAdminClient )
		return true;

	// No longer controlled.
	m_hAdminClient = NULL;
	m_eAdminControl = kAdminControlNone;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::ForceAdminClientLogout
//
//	PURPOSE:	Make this client the current admin.
//
//	RETURN:		true on success.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::SetAdminClient(HCLIENT hClient)
{
	// if there's already an admin we can't set the client
	if (m_eAdminControl != kAdminControlNone)
	{
		return false;
	}

	// call the helper to set the admin client
	LoginAdminClient(hClient);

	// success
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::ForceAdminClientLogout
//
//	PURPOSE:	Force the current admin client to be logged out
//
//	RETURN:		None.
//
// ----------------------------------------------------------------------- //

void ScmdServer_Impl::ForceAdminClientLogout()
{
	if (m_hAdminClient && (m_eAdminControl == kAdminControlClient))
	{
		LogoutAdminClient(m_hAdminClient);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::SendMessage
//
//	PURPOSE:	Handles directing message to serverapp or hclient.
//
//	RETURN:		true on success.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::SendMessage( HCLIENT hClient, ILTMessage_Write& msg )
{
	// if the hClient is a local ScmdConsole, send the message back to the shell
	if (hClient == (HCLIENT)&ScmdConsole::Instance())
	{
		((IServerShell*)g_pGameServerShell)->OnMessage((HCLIENT)&ScmdServer::Instance(), msg.Read());
		return true;
	}

	// Check if there's a client to receive this.
	if( hClient )
	{
		if( g_pLTServer->SendToClient( msg.Read(), hClient, MESSAGE_GUARANTEED ) != LT_OK )
			return false;

		return true;
	}

	// No client, send it to the serverapp.
	g_pLTServer->SendToServerApp( *( msg.Read( )));

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::SendStatusMessage
//
//	PURPOSE:	Sends command status message to client.
//
//	RETURN:		true on success.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::SendStatusMessage( HCLIENT hClient, ScmdCommand eScmdCommand, 
										ScmdCommandStatus eScmdCommandStatus )
{
	// Tell the client it worked.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_RESPONSE );
	cMsg.Writeuint8( eScmdCommand );
	cMsg.Writeuint8( eScmdCommandStatus );
	SendMessage( hClient, *cMsg );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::LoginAdminClient
//
//	PURPOSE:	Helper for logging in the admin client.
//
//	RETURN:		None
//
// ----------------------------------------------------------------------- //

void ScmdServer_Impl::LoginAdminClient(HCLIENT hClient)
{
	// Admin is now controlled.
	m_eAdminControl = ( hClient ) ? kAdminControlClient : kAdminControlServerapp;

	// This is our admin.  It will be NULL for a standalone server.
	m_hAdminClient = hClient;
}			

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::LogoutAdminClient
//
//	PURPOSE:	Helper for logging out the admin client.
//
//	RETURN:		None
//
// ----------------------------------------------------------------------- //


void ScmdServer_Impl::LogoutAdminClient(HCLIENT hClient)
{
	// No longer admin controlled.
	m_eAdminControl = kAdminControlNone;

	// Forget our client.
	m_hAdminClient = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleScmdMessage
//
//	PURPOSE:	Handles reading of SCMD command.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleScmdMessage( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Check if SCMD commands are not allowed.
	if( !m_bAllowScmdCommands && hClient )
	{
		return true;
	}

	// Read the scmd command.
	ScmdCommand eScmdCommand = ( ScmdCommand )msg.Readuint8( );

	// Check if we're not controlled by an admin.  Since Login is the only
	// command you can send without being logged in, it has special handling.
	if( hClient && eScmdCommand != kScmdCommandLogin &&
		(( m_eAdminControl == kAdminControlNone ) ||
		 ( m_eAdminControl == kAdminControlClient && hClient != m_hAdminClient )) )
	{
		// Doesn't have privileges.
		SendStatusMessage( hClient, eScmdCommand, kScmdCommandStatusNotLoggedIn );
		return false;
	}


	switch( eScmdCommand )
	{
		case kScmdCommandLogin:
			return HandleLogin( hClient, msg );
			break;

		case kScmdCommandLogout:
			return HandleLogout( hClient, msg );
			break;

		case kScmdCommandListClients:
			return HandleListClients( hClient, msg );
			break;

		case kScmdCommandListMissions:
			return HandleListMissions( hClient, msg );
			break;

		case kScmdCommandNextMission:
			return HandleNextMission( hClient, msg );
			break;

		case kScmdCommandNextRound:
			return HandleNextRound( hClient, msg );
			break;

		case kScmdCommandSetMission:
			return HandleSetMission( hClient, msg );
			break;

		case kScmdCommandBootName:
			return HandleBootName( hClient, msg );
			break;

		case kScmdCommandBootId:
			return HandleBootId( hClient, msg );
			break;

// Banning by user requires CDKEY 1
#ifdef ENABLE_CDKEY_1_CHECK
		case kScmdCommandBanUser:
			return HandleBanUser( hClient, msg );
			break;

		case kScmdCommandUnbanUser:
			return HandleUnbanUser( hClient, msg );
			break;

		case kScmdCommandListUserBans:
			return HandleListUserBans( hClient, msg );
			break;
#endif

		case kScmdCommandListGameOptions:
			return HandleListGameOptions( hClient, msg );
			break;

		case kScmdCommandShowGameOption:
			return HandleShowGameOption( hClient, msg );
			break;

		case kScmdCommandSetGameOption:
			return HandleSetGameOption( hClient, msg );
			break;

	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleLogin
//
//	PURPOSE:	Handles login command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleLogin( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Check if we're already controlled by an admin.
	if( m_eAdminControl != kAdminControlNone )
	{
		if( !SendStatusMessage( hClient, kScmdCommandLogin, kScmdCommandStatusAdminAlreadyLoggedIn ))
			return false;

		return true;
	}

#pragma MESSAGE( "FIXFIX:  Need to make the password use a public key, rather than just hash." )

	// Read the password they sent in.
	uint32 nHashedPassword = msg.Readuint32( );

	// Read the current password.

	uint32 nCurrentHashedPassword = str_Hash( m_sPassword.c_str( ));

	// Make sure the hashed values match.
	if( nCurrentHashedPassword != nHashedPassword )
	{
		if( !SendStatusMessage( hClient, kScmdCommandLogin, kScmdCommandStatusIncorrectPassword ))
			return false;

		return true;
	}

	// call the login helper
	LoginAdminClient(hClient);

	// If the admin was just taken by a client, then tell all the clients.
	if (hClient != (HCLIENT)&ScmdConsole::Instance())
	{
		if( m_eAdminControl == kAdminControlClient )
		{
			CPlayerObj* pPlayerObj = GetPlayerFromHClient( hClient );
			g_pGameServerShell->SendPlayerInfoMsgToClients( NULL, pPlayerObj, MID_PI_UPDATE );
		}
	}

	// Tell the client it worked.
	if( !SendStatusMessage( hClient, kScmdCommandLogin, kScmdCommandStatusOk ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleLogout
//
//	PURPOSE:	Handles logout command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleLogout( HCLIENT hClient, ILTMessage_Read& msg )
{
	// local client cannot logout
	if (g_pLTServer->GetClientInfoFlags(hClient) & CIF_LOCAL)
	{
		return true;
	}

	// Check if we should send a message to all clients about our state change.
	// Only need to do this if the admin was a client.
	bool bSendPlayerInfoMsg = ( m_eAdminControl == kAdminControlClient );

	// call the logout helper
	LogoutAdminClient(hClient);	

	// If the admin was just taken by a client, then tell all the clients.
	if( bSendPlayerInfoMsg  && hClient != (HCLIENT)&ScmdConsole::Instance())
	{
		CPlayerObj* pPlayerObj = GetPlayerFromHClient( hClient );
		g_pGameServerShell->SendPlayerInfoMsgToClients( NULL, pPlayerObj, MID_PI_UPDATE );
	}

	// Tell the client it worked.
	if( !SendStatusMessage( hClient, kScmdCommandLogout, kScmdCommandStatusOk ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleListClients
//
//	PURPOSE:	Handles clients command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleListClients( HCLIENT hClient, ILTMessage_Read& msg )
{
	float fPing = 0.0f;
    uint16 nClientID = 0;
	uint8 aClientIP[4];
	uint16 nPort;
	std::wstring sPlayerHandle = L"";

	// Write out the message header.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_RESPONSE );
	cMsg.Writeuint8( kScmdCommandListClients );
	cMsg.Writeuint8( kScmdCommandStatusOk );

	// Iterate over all the clients and put their id, ping, ip and name in the message.
    HCLIENT hIterClient = g_pLTServer->GetNextClient( NULL );
    while( hIterClient )
	{
        nClientID = ( uint16 )g_pLTServer->GetClientID( hIterClient );
        g_pLTServer->GetClientPing( hIterClient, fPing );
		g_pLTServer->GetClientAddr( hIterClient, aClientIP, &nPort );

		cMsg.Writeuint16(( uint16 )nClientID );
		cMsg.Writeuint16(( uint16 )( fPing + 0.5f ));
		cMsg.Writeuint8( aClientIP[0] );
		cMsg.Writeuint8( aClientIP[1] );
		cMsg.Writeuint8( aClientIP[2] );
		cMsg.Writeuint8( aClientIP[3] );

		// Get the player for this client.  If there is none,
		// then we can't determine the name.
		CPlayerObj* pPlayerObj = GetPlayerFromHClient( hIterClient );
		if( !pPlayerObj )
		{
			sPlayerHandle = L"";
		}
		else
		{
			sPlayerHandle = pPlayerObj->GetNetUniqueName( );
		}
		cMsg.WriteWString( sPlayerHandle.c_str( ));

	    hIterClient = g_pLTServer->GetNextClient( hIterClient );
	}

	// Write out the the list terminator.
	cMsg.Writeuint16(0xFFFF);
	SendMessage( hClient, *cMsg );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleListMissions
//
//	PURPOSE:	Handles listmissions command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleListMissions( HCLIENT hClient, ILTMessage_Read& msg )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_RESPONSE );
	cMsg.Writeuint8( kScmdCommandListMissions );
	cMsg.Writeuint8( kScmdCommandStatusOk );

	// Write out the current campaign.
	cMsg.Writeuint8(( uint8 )g_pServerMissionMgr->GetCurrentCampaignIndex( ));

	Campaign& campaign = g_pServerMissionMgr->GetCampaign( );

	// Write out the number of entries.
	cMsg.Writeuint8(( uint8 )campaign.size( ));

	// Write out all the missions.
	for( Campaign::iterator iter = campaign.begin( ); iter != campaign.end( ); iter++ )
	{
		// Get the mission in the campaign.
		int nMission = *iter;
		HRECORD hMission = g_pMissionDB->GetMission( nMission );
		if( !hMission )
			continue;

		// Show the current level if this is the current mission.
		int nLevel = 0;
		if( g_pServerMissionMgr->GetCurrentMission( ) == nMission )
		{
			nLevel = g_pServerMissionMgr->GetCurrentLevel( );
		}

		HRECORD hLevel = g_pMissionDB->GetLevel(hMission,nLevel);
		if( !hLevel )
			continue;

 		cMsg.WriteWString( MPA2W(g_pMissionDB->GetWorldName(hLevel,false)).c_str() );
	}

	// Send the message.
	SendMessage( hClient, *cMsg );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleNextMission
//
//	PURPOSE:	Handles nextmission command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleNextMission( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Tell the client it worked.
	if( !SendStatusMessage( hClient, kScmdCommandNextMission, kScmdCommandStatusOk ))
		return false;

	g_pServerMissionMgr->NextMission( );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleNextRound
//
//	PURPOSE:	Handles nextround command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleNextRound( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Tell the client it worked.
	if( !SendStatusMessage( hClient, kScmdCommandNextRound, kScmdCommandStatusOk ))
		return false;

	g_pServerMissionMgr->NextRound( );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleSetMission
//
//	PURPOSE:	Handles setmission command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleSetMission( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Read the mission index desired.
	uint8 nMissionIndex = msg.Readuint8( );

	// Check to make sure the mission index is within range.
	Campaign& campaign = g_pServerMissionMgr->GetCampaign( );
	if( nMissionIndex >= campaign.size( ))
	{
		// Tell the client it failed.
		if( !SendStatusMessage( hClient, kScmdCommandSetMission, kScmdCommandStatusFailed ))
			return false;

		return true;
	}

	// Tell the client it worked.
	if( !SendStatusMessage( hClient, kScmdCommandSetMission, kScmdCommandStatusOk ))
		return false;

	// Do the switch.
	g_pServerMissionMgr->SwitchToCampaignIndex( nMissionIndex );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BootClient
//
//	PURPOSE:	Boots a client by id or name.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

static bool BootClient( bool bById, uint16 nBootClientId, char const* pszBootName )
{
    uint32 nClientId = 0;
	std::string sPlayerHandle = "";
	bool bBooted = false;

	// Iterate over all the clients and put their id, ping, ip and name in the message.
    HCLIENT hIterClient = NULL;
    for( hIterClient = g_pLTServer->GetNextClient( NULL ); hIterClient; 
		hIterClient = g_pLTServer->GetNextClient( hIterClient ))
	{
        nClientId = g_pLTServer->GetClientID( hIterClient );

		// Check if we're booting by ID.
		if( bById )
		{
			if( nClientId == nBootClientId )
			{
				g_pLTServer->KickClient( hIterClient );
				bBooted = true;
				break;
			}
		}
		else
		{
			// Get the player for this client.  If there is none,
			// then we can't determine the name.
			CPlayerObj* pPlayerObj = GetPlayerFromHClient( hIterClient );
			if( !pPlayerObj )
			{
				continue;
			}
			else
			{
#pragma TODO("ScmdServer_Impl::BootClient() update to use unicode player names")
				sPlayerHandle = MPW2A(pPlayerObj->GetNetUniqueName( )).c_str();
			}

			// See if this matches the name we want to boot.
			if( LTStrICmp( pszBootName, sPlayerHandle.c_str( )) == 0 )
			{
				bBooted = ( g_pLTServer->KickClient( hIterClient ) == LT_OK );
				break;
			}
		}
	}

	return bBooted;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleBootName
//
//	PURPOSE:	Handles bootname command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleBootName( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Read the name of the player to boot.
	wchar_t wsPlayerName[MAX_PLAYER_NAME];
	msg.ReadWString( wsPlayerName, LTARRAYSIZE( wsPlayerName ));

	// Boot the client.
	bool bBooted = BootClient( false, 0, MPW2A(wsPlayerName).c_str() );

	// Tell the client if it worked.
	ScmdCommandStatus eScmdCommandStatus = ( bBooted ) ? kScmdCommandStatusOk : kScmdCommandStatusFailed;
	if( !SendStatusMessage( hClient, kScmdCommandBootName, eScmdCommandStatus ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleBootId
//
//	PURPOSE:	Handles bootid command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleBootId( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Read the id of the client to boot.
	uint16 nClientId = msg.Readuint16( );

	// Boot the client.
	bool bBooted = BootClient( true, nClientId, "" );

	// Tell the client if it worked.
	ScmdCommandStatus eScmdCommandStatus = ( bBooted ) ? kScmdCommandStatusOk : kScmdCommandStatusFailed;
	if( !SendStatusMessage( hClient, kScmdCommandBootId, eScmdCommandStatus ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleBanUser
//
//	PURPOSE:	Handles banuser command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleBanUser( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Read the id of the client to ban.
	uint16 nClientId = msg.Readuint16( );

	bool bOk = true;
	HCLIENT hBanClient = g_pLTServer->GetClientHandle( nClientId );
	if( !hBanClient )
	{
		bOk = false;
	}

	if( bOk )
	{
		// get the user's unique CD Key hash
		const char* pszUserCDKeyHash = NULL;
		g_pGameServerShell->GetGameSpyServer()->GetUserCDKeyHash(nClientId, pszUserCDKeyHash);	

		// get the user's player name
		const wchar_t* pwszPlayerName;
		CPlayerObj* pPlayerObj = GetPlayerFromHClient(hBanClient);
		if (!pPlayerObj)
		{
			pwszPlayerName = L"";
		}
		else
		{
			pwszPlayerName = pPlayerObj->GetNetUniqueName();
		}

		// add them to the ban list
		BanUserMgr::Instance().AddBan(pszUserCDKeyHash, MPW2A(pwszPlayerName).c_str());
	}

	// Tell the client if it worked.
	ScmdCommandStatus eScmdCommandStatus = ( bOk ) ? kScmdCommandStatusOk : kScmdCommandStatusFailed;
	if( !SendStatusMessage( hClient, kScmdCommandBanUser, eScmdCommandStatus ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleUnbanUser
//
//	PURPOSE:	Handles unbanuser command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleUnbanUser( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Read the banid.
	uint8 nBanId = msg.Readuint8( );

	bool bOk = true;

	// retrieve the list of user bans
	const BanUserMgr::TBanList& banList = BanUserMgr::Instance().GetBanList();

	// Make sure the id is in range.
	if( nBanId >= banList.size( ))
	{
		bOk = false;
	}

	if( bOk )
	{
		// Index into ban list and remove the ban.
		BanUserMgr::TBanList::const_iterator iter = banList.begin( );
		for( uint8 nId = 0; iter != banList.end( ); iter++, nId++ )
		{
			if( nId == nBanId )
			{
				const BanUserMgr::SUserBanEntry& sUserBanEntry = *iter;
				BanUserMgr::Instance( ).RemoveBan( sUserBanEntry );
				break;
			}
		}
	}

	// Tell the client if it worked or not.
	if( !SendStatusMessage( hClient, kScmdCommandUnbanUser, ( bOk ) ? kScmdCommandStatusOk : 
		kScmdCommandStatusFailed ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleListUserBans
//
//	PURPOSE:	Handles listuserbans command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleListUserBans( HCLIENT hClient, ILTMessage_Read& msg )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_RESPONSE );
	cMsg.Writeuint8( kScmdCommandListUserBans );
	cMsg.Writeuint8( kScmdCommandStatusOk );

	char szBannedIP[16] = "";

	// Tell client about all the bans.
	BanUserMgr::TBanList const& banList = BanUserMgr::Instance( ).GetBanList( );
	cMsg.Writeuint16( banList.size( ));
	for( BanUserMgr::TBanList::const_iterator iter = banList.begin( ); iter != banList.end( ); iter++ )
	{
		const BanUserMgr::SUserBanEntry& sUserBanEntry = *iter;
		cMsg.WriteString( sUserBanEntry.strPlayerName.c_str() );
	}

	SendMessage( hClient, *cMsg );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleListGameOptions
//
//	PURPOSE:	Handles listgameoptions command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleListGameOptions( HCLIENT hClient, ILTMessage_Read& msg )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_RESPONSE );
	cMsg.Writeuint8( kScmdCommandListGameOptions );
	cMsg.Writeuint8( kScmdCommandStatusOk );

	// Send the message.
	SendMessage( hClient, *cMsg );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleShowGameOption
//
//	PURPOSE:	Handles showgameoption command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleShowGameOption( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Get the game option they are setting.
	uint8 nGameOption = msg.Readuint8( );

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_RESPONSE );
	cMsg.Writeuint8( kScmdCommandShowGameOption );
	cMsg.Writeuint8( kScmdCommandStatusOk );
	cMsg.Writeuint8( nGameOption );

	// Send the message.
	SendMessage( hClient, *cMsg );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleSetGameOption
//
//	PURPOSE:	Handles setgameoption command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleSetGameOption( HCLIENT hClient, ILTMessage_Read& msg )
{
	bool bOk = true;

	// Get the game option they are setting.
	uint8 nGameOption = msg.Readuint8( );

	// Read in the value.
	wchar_t wsVal[256];
	msg.ReadWString( wsVal, LTARRAYSIZE( wsVal ));

	// Iterate through the list of gamerules.  Only count them if they were shown on the scmd list.
	GameRule* pFound = NULL;
	uint32 nIndex = 0;
	for( GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( ); 
		iter != GameRule::GetGameRuleList().end( ); iter++ )
	{
		GameRule* pGameRule = *iter;

		// Skip rules not shown on the list.
		if( !pGameRule->IsCanModify( ) || !pGameRule->IsShowInOptions( ) || !pGameRule->IsCanModifyAtRuntime( ))
			continue;

		if( nIndex == nGameOption )
		{
			pFound = pGameRule;
			break;
		}

		nIndex++;
	}

	// We need to tell the host client about the new settings.
	if( !pFound )
	{
		SendStatusMessage( hClient, kScmdCommandSetGameOption, kScmdCommandStatusFailed );
		return true;
	}

	// Do special handling of enum types, since their value is passed by index, not their raw value type of string.
	GameRuleEnum* pGameRuleEnum = dynamic_cast< GameRuleEnum* >( pFound );
	if( pGameRuleEnum )
	{
		uint32 nValueIndex = LTStrToLong( wsVal );
		char const* pszRawValue = pGameRuleEnum->GetIndexToRawValue( nValueIndex );
		if( !pszRawValue[0] )
		{
			SendStatusMessage( hClient, kScmdCommandSetGameOption, kScmdCommandStatusFailed );
			return true;
		}
		pGameRuleEnum->GetValue( ) = pszRawValue;
	}
	else
	{
		pFound->FromString( wsVal, false );
	}

	g_pServerMissionMgr->UpdateTimeLimit();

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_MULTIPLAYER_OPTIONS );
	GameModeMgr::Instance().WriteToMsg( cMsg );

	// tell all clients that the options have changed
	HCLIENT hConnectedClient = g_pLTServer->GetNextClient( NULL );
	CLTMsgRef_Read cMsgRead = cMsg.Read();

	while( hConnectedClient )
	{
		g_pLTServer->SendToClient( cMsgRead, hConnectedClient, MESSAGE_GUARANTEED );
		hConnectedClient = g_pLTServer->GetNextClient( hConnectedClient );
	}

	// tell the serverapp
	CAutoMessage cServerAppMsg;
	cServerAppMsg.Writeuint8( SERVERAPP_MULTIPLAYEROPTIONS );
	GameModeMgr::Instance().WriteToMsg( cServerAppMsg );
	g_pLTServer->SendToServerApp( *cServerAppMsg.Read() );
	
	SendStatusMessage( hClient, kScmdCommandSetGameOption, ( bOk ) ? kScmdCommandStatusOk : kScmdCommandStatusFailed );

	return true;
}
