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

#include "stdafx.h"
#include "ScmdServer.h"
#include "ScmdShared.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "ServerMissionMgr.h"
#include "MissionButeMgr.h"
#include "BanIPMgr.h"

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

		// Handle addban message.
		bool HandleAddBan( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle removeban message.
		bool HandleRemoveBan( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle listbans message.
		bool HandleListBans( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle banid message.
		bool HandleBanClient( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle listgameoptions message.
		bool HandleListGameOptions( HCLIENT hClient, ILTMessage_Read& msg );

		// Handle setgameoption message.
		bool HandleSetGameOption( HCLIENT hClient, ILTMessage_Read& msg );

		// Handles directing message to serverapp or hclient.
		bool SendMessage( HCLIENT hClient, ILTMessage_Write& msg );

		// Sends simple command status message to client.
		bool SendStatusMessage( HCLIENT hClient, ScmdCommand eScmdCommand, 
				ScmdCommandStatus eScmdCommandStatus );

	private:

		// Object has been initialized.
		bool m_bInitialized;

		// Indicates an admin is currently logged in.
		AdminControl m_eAdminControl;

		// Current admin controller.  NULL if serverapp.
		HCLIENT m_hAdminClient;

		// Password required to take control.
		std::string m_sPassword;

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
	m_sPassword = g_pGameServerShell->GetServerGameOptions( ).m_sScmdPassword;
	m_bAllowScmdCommands = g_pGameServerShell->GetServerGameOptions( ).m_bAllowScmdCommands;

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
		case MID_SCMD:			
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
//	ROUTINE:	ScmdServer_Impl::SendMessage
//
//	PURPOSE:	Handles directing message to serverapp or hclient.
//
//	RETURN:		true on success.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::SendMessage( HCLIENT hClient, ILTMessage_Write& msg )
{
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
	cMsg.Writeuint8( MID_SCMD );
	cMsg.Writeuint8( eScmdCommand );
	cMsg.Writeuint8( eScmdCommandStatus );
	SendMessage( hClient, *cMsg );

	return true;
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
	if( !m_bAllowScmdCommands )
	{
		return true;
	}

	// Read the scmd command.
	ScmdCommand eScmdCommand = ( ScmdCommand )msg.Readuint8( );

	// Check if we're not controlled by an admin.  Since Login is the only
	// command you can send without being logged in, it has special handling.
	if( eScmdCommand != kScmdCommandLogin &&
		( m_eAdminControl == kAdminControlNone ) ||
		( m_eAdminControl == kAdminControlClient && hClient != m_hAdminClient ) ||
		( m_eAdminControl == kAdminControlServerapp && hClient != NULL ))
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

		case kScmdCommandAddBan:
			return HandleAddBan( hClient, msg );
			break;

		case kScmdCommandRemoveBan:
			return HandleRemoveBan( hClient, msg );
			break;

		case kScmdCommandListBans:
			return HandleListBans( hClient, msg );
			break;

		case kScmdCommandBanClient:
			return HandleBanClient( hClient, msg );
			break;

		case kScmdCommandListGameOptions:
			return HandleListGameOptions( hClient, msg );
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

#pragma message( "FIXFIX:  Need to make the password use a public key, rather than just hash." )

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

	// Admin is now controlled.
	m_eAdminControl = ( hClient ) ? kAdminControlClient : kAdminControlServerapp;

	// This is our admin.  It will be NULL for a standalone server.
	m_hAdminClient = hClient;

	// If the admin was just taken by a client, then tell all the clients.
	if( m_eAdminControl == kAdminControlClient )
	{
		CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->GetClientUserData( hClient );
		g_pGameServerShell->SendPlayerInfoMsgToClients( NULL, pPlayerObj, MID_PI_UPDATE );
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
	// Check if we should send a message to all clients about our state change.
	// Only need to do this if the admin was a client.
	bool bSendPlayerInfoMsg = ( m_eAdminControl == kAdminControlClient );

	// No longer admin controlled.
	m_eAdminControl = kAdminControlNone;

	// Forget our client.
	m_hAdminClient = NULL;

	// If the admin was just taken by a client, then tell all the clients.
	if( bSendPlayerInfoMsg )
	{
		CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->GetClientUserData( hClient );
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
	std::string sPlayerHandle = "";

	// Write out the message header.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD );
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
		CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->GetClientUserData( hIterClient );
		if( !pPlayerObj )
		{
			sPlayerHandle = "";
		}
		else
		{
			sPlayerHandle = pPlayerObj->GetNetUniqueName( );
		}
		cMsg.WriteString( sPlayerHandle.c_str( ));

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
	cMsg.Writeuint8( MID_SCMD );
	cMsg.Writeuint8( kScmdCommandListMissions );
	cMsg.Writeuint8( kScmdCommandStatusOk );

	// Write out the current campaign.
	cMsg.Writeuint8(( uint8 )g_pServerMissionMgr->GetCurrentCampaignIndex( ));

	Campaign& campaign = g_pServerMissionMgr->GetCampaign( );

	// Write out the number of entries.
	cMsg.Writeuint8( campaign.size( ));

	// Write out all the missions.
	char fname[_MAX_FNAME] = "";
	for( Campaign::iterator iter = campaign.begin( ); iter != campaign.end( ); iter++ )
	{
		// Get the mission in the campaign.
		int nMission = *iter;
		MISSION* pMission = g_pMissionButeMgr->GetMission( nMission );
		if( !pMission )
			continue;

		// Show the current level if this is the current mission.
		int nLevel = 0;
		if( g_pServerMissionMgr->GetCurrentMission( ) == nMission )
		{
			nLevel = g_pServerMissionMgr->GetCurrentLevel( );
		}

		LEVEL& level = pMission->aLevels[nLevel];
		_splitpath( level.szLevel, NULL, NULL, fname, NULL );
 		cMsg.WriteString( fname );
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
			CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->GetClientUserData( hIterClient );
			if( !pPlayerObj )
			{
				continue;
			}
			else
			{
				sPlayerHandle = pPlayerObj->GetNetUniqueName( );
			}

			// See if this matches the name we want to boot.
			if( stricmp( pszBootName, sPlayerHandle.c_str( )) == 0 )
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
	char szPlayerName[MAX_PLAYER_NAME];
	msg.ReadString( szPlayerName, ARRAY_LEN( szPlayerName ));

	// Boot the client.
	bool bBooted = BootClient( false, 0, szPlayerName );

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
//	ROUTINE:	ScmdServer_Impl::HandleAddBan
//
//	PURPOSE:	Handles addban command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleAddBan( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Read the IP.
	char szBanIP[16];
	msg.ReadString( szBanIP, ARRAY_LEN( szBanIP ));

	bool bSuccess = BanIPMgr::Instance( ).AddBan( szBanIP );

	// Tell the client if it worked or not.
	if( !SendStatusMessage( hClient, kScmdCommandAddBan, ( bSuccess ) ? kScmdCommandStatusOk : 
		kScmdCommandStatusFailed ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleRemoveBan
//
//	PURPOSE:	Handles removeban command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleRemoveBan( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Read the banid.
	uint8 nBanId = msg.Readuint8( );

	bool bOk = true;

	// Index into the ban list.
	BanIPMgr::BanList const& banList = BanIPMgr::Instance( ).GetBanList( );

	// Make sure the id is in range.
	if( nBanId >= banList.size( ))
	{
		bOk = false;
	}

	if( bOk )
	{
		// Index into ban list and remove the ban.
		BanIPMgr::BanList::const_iterator iter = banList.begin( );
		for( uint8 nId = 0; iter != banList.end( ); iter++, nId++ )
		{
			if( nId == nBanId )
			{
				BanIPMgr::ClientIP const& clientIP = *iter;
				bOk = BanIPMgr::Instance( ).RemoveBan( clientIP );
				break;
			}
		}
	}

	// Tell the client if it worked or not.
	if( !SendStatusMessage( hClient, kScmdCommandRemoveBan, ( bOk ) ? kScmdCommandStatusOk : 
		kScmdCommandStatusFailed ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleListBans
//
//	PURPOSE:	Handles listbans command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleListBans( HCLIENT hClient, ILTMessage_Read& msg )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD );
	cMsg.Writeuint8( kScmdCommandListBans );
	cMsg.Writeuint8( kScmdCommandStatusOk );

	char szBannedIP[16] = "";

	// Tell client about all the bans.
	BanIPMgr::BanList const& banList = BanIPMgr::Instance( ).GetBanList( );
	cMsg.Writeuint16( banList.size( ));
	for( BanIPMgr::BanList::const_iterator iter = banList.begin( ); iter != banList.end( ); iter++ )
	{
		BanIPMgr::ClientIP const& clientIP = *iter;

		BanIPMgr::Instance( ).ConvertClientIPToString( clientIP, szBannedIP, ARRAY_LEN( szBannedIP ));
		cMsg.WriteString( szBannedIP );
	}

	SendMessage( hClient, *cMsg );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdServer_Impl::HandleBanClient
//
//	PURPOSE:	Handles banid command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdServer_Impl::HandleBanClient( HCLIENT hClient, ILTMessage_Read& msg )
{
	// Read the id of the client to boot.
	uint16 nClientId = msg.Readuint16( );

	bool bOk = true;
	HCLIENT hBanClient = g_pLTServer->GetClientHandle( nClientId );
	if( !hBanClient )
	{
		bOk = false;
	}

	if( bOk )
	{
		uint8 aClientIP[4];
		uint16 nPort;
		g_pLTServer->GetClientAddr( hBanClient, aClientIP, &nPort );

		BanIPMgr::ClientIP clientIP = { aClientIP[0], aClientIP[1], aClientIP[2], aClientIP[3] };

		// Ban the client.
		bOk = BanIPMgr::Instance( ).AddBan( clientIP );
	}

	// Tell the client if it worked.
	ScmdCommandStatus eScmdCommandStatus = ( bOk ) ? kScmdCommandStatusOk : kScmdCommandStatusFailed;
	if( !SendStatusMessage( hClient, kScmdCommandBanClient, eScmdCommandStatus ))
		return false;

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
	ServerMissionSettings sms = g_pServerMissionMgr->GetServerSettings();

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD );
	cMsg.Writeuint8( kScmdCommandListGameOptions );
	cMsg.Writeuint8( kScmdCommandStatusOk );

	cMsg.Writeuint8( g_pGameServerShell->GetGameType( ));

	switch( g_pGameServerShell->GetGameType( ))
	{
		case eGameTypeDeathmatch:
		{
			cMsg.Writeuint8( sms.m_nRunSpeed );
			cMsg.Writeuint8( sms.m_nScoreLimit );
			cMsg.Writeuint8( sms.m_nTimeLimit );
			cMsg.Writeuint8( sms.m_nRounds );
		}
		break;

		case eGameTypeTeamDeathmatch:
		{
			cMsg.Writeuint8( sms.m_nRunSpeed );
			cMsg.Writeuint8( sms.m_nScoreLimit );
			cMsg.Writeuint8( sms.m_nTimeLimit );
			cMsg.Writeuint8( sms.m_nRounds );
			cMsg.Writebool( sms.m_bFriendlyFire) ;
		}
		break;

		case eGameTypeDoomsDay:
		{
			cMsg.Writeuint8( sms.m_nRunSpeed );
			cMsg.Writeuint8( sms.m_nTimeLimit );
			cMsg.Writeuint8( sms.m_nRounds );
			cMsg.Writebool( sms.m_bFriendlyFire );
		}
		break;

		case eGameTypeCooperative:
		{
			cMsg.Writebool( sms.m_bFriendlyFire) ;
			cMsg.Writeuint8( sms.m_nMPDifficulty );
			cMsg.Writefloat( sms.m_fPlayerDiffFactor );
		}
		break;

		default:
		{
		}
		break;
	}

	// Send the message.
	SendMessage( hClient, *cMsg );

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SetGameOption
//
//	PURPOSE:	Handles setgameoption command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

template< class T, class U >
void static SetGameOption( U& setting, T value, T lower, T upper )
{
	U clampedVal = ( U )Clamp( value, lower, upper );
	setting = clampedVal;
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
	char szVal[256];
	msg.ReadString( szVal, ARRAY_LEN( szVal ));

	ServerMissionSettings sms = g_pServerMissionMgr->GetServerSettings();
	switch( g_pGameServerShell->GetGameType( ))
	{
		case eGameTypeDeathmatch:
		{
			switch( nGameOption )
			{
				// Runspeed.
				case 0:
				{
					SetGameOption( sms.m_nRunSpeed, atoi( szVal ), 100, 150 );
				}
				break;
				// Score limit.
				case 1:
				{
					SetGameOption( sms.m_nScoreLimit, atoi( szVal ), 0, 255 );
				}
				break;
				// Time limit.
				case 2:
				{
					SetGameOption( sms.m_nTimeLimit, atoi( szVal ), 0, 255 );
				}
				break;
				// Rounds.
				case 3:
				{
					SetGameOption( sms.m_nRounds, atoi( szVal ), 1, 255 );
				}
				break;
				default:
				{
					bOk = false;
				}
				break;
			}
		}
		break;

		case eGameTypeTeamDeathmatch:
		{
			switch( nGameOption )
			{
				// Runspeed.
				case 0:
				{
					SetGameOption( sms.m_nRunSpeed, atoi( szVal ), 100, 150 );
				}
				break;
				// Score limit.
				case 1:
				{
					SetGameOption( sms.m_nScoreLimit, atoi( szVal ), 0, 255 );
				}
				break;
				// Time limit.
				case 2:
				{
					SetGameOption( sms.m_nTimeLimit, atoi( szVal ), 0, 255 );
				}
				break;
				// Rounds.
				case 3:
				{
					SetGameOption( sms.m_nRounds, atoi( szVal ), 1, 255 );
				}
				break;
				// Friendly fire.
				case 4:
				{
					SetGameOption( sms.m_bFriendlyFire, ( bool )( !!atoi( szVal )), false, true );
				}
				break;
				default:
				{
					bOk = false;
				}
				break;
			}
		}
		break;

		case eGameTypeDoomsDay:
		{
			switch( nGameOption )
			{
				// Runspeed.
				case 0:
				{
					SetGameOption( sms.m_nRunSpeed, atoi( szVal ), 100, 150 );
				}
				break;
				// Time limit.
				case 1:
				{
					SetGameOption( sms.m_nTimeLimit, atoi( szVal ), 0, 255 );
				}
				break;
				// Rounds.
				case 2:
				{
					SetGameOption( sms.m_nRounds, atoi( szVal ), 1, 255 );
				}
				break;
				// Friendly fire.
				case 3:
				{
					SetGameOption( sms.m_bFriendlyFire, ( bool )( !!atoi( szVal )), false, true );
				}
				break;
				default:
				{
					bOk = false;
				}
				break;
			}
		}
		break;

		case eGameTypeCooperative:
		{
			switch( nGameOption )
			{
				// Friendly fire.
				case 0:
				{
					SetGameOption( sms.m_bFriendlyFire, ( bool )( !!atoi( szVal )), false, true );
				}
				break;

				// mp difficulty.
				case 1:
				{
					SetGameOption( sms.m_nMPDifficulty, atoi( szVal ), 0, 255 );
				}
				break;

				// player diff factor.
				case 2:
				{
					SetGameOption( sms.m_fPlayerDiffFactor, ( float )atof( szVal ), 0.0f, 20.0f );
				}
				break;

				default:
				{
					bOk = false;
				}
				break;
			}
		}
		break;

		default:
		{
			bOk = false;
		}
		break;
	}
	
	// We need to tell the host client about the new settings.
	if( bOk )
	{
		// Record any changes.
		g_pServerMissionMgr->SetServerSettings(sms);

		// Try to find a local host if one exists.
		HCLIENT hHost = g_pLTServer->GetNextClient( NULL );
		while( hHost )
		{
			uint32 nClientInfoFlags = g_pLTServer->GetClientInfoFlags( hHost );
			if( nClientInfoFlags & CIF_LOCAL )
			{
				break;
			}

			hHost = g_pLTServer->GetNextClient( hHost );
		}
		
		// If we have a host, tell them about the new settings.
		if( hHost )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_MULTIPLAYER_OPTIONS );
			cMsg.Writeuint8( sms.m_nRunSpeed);
			cMsg.Writeuint8( sms.m_nScoreLimit);
			cMsg.Writeuint8( sms.m_nTimeLimit);
			cMsg.Writeuint8( sms.m_nRounds);
			cMsg.Writebool( sms.m_bFriendlyFire);
			cMsg.Writeuint8( sms.m_nMPDifficulty);
			cMsg.Writefloat( sms.m_fPlayerDiffFactor);
			g_pLTServer->SendToClient( cMsg.Read( ), hHost, MESSAGE_GUARANTEED );
		}
	}

	SendStatusMessage( hClient, kScmdCommandSetGameOption, ( bOk ) ? kScmdCommandStatusOk : kScmdCommandStatusFailed );

	return true;
}