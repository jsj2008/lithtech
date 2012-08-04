// ----------------------------------------------------------------------- //
//
// MODULE  : ServerConnectionMgr.cpp
//
// PURPOSE : Definition/Implementation of the server client connection manager.
//
// CREATED : 06/22/04
//
// (c) 1996-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ServerConnectionMgr.h"
#include "ServerMissionMgr.h"
#include "GameServerShell.h"
#include "BanIPMgr.h"
#include "BanUserMgr.h"
#include "PlayerObj.h"
#include "ScmdServer.h"
#include "crc32utils.h"
#include "GameModeMgr.h"
#include "TeamMgr.h"
#include "ServerDB.h"
#include "StateMachine.h"
#include "iltfilemgr.h"
#include "GameStartPoint.h"
#include "ltfileoperations.h"
#include "GameStartPointMgr.h"

#if !defined(PLATFORM_XENON)
#include "iltgameutil.h"
static ILTGameUtil *g_pLTGameUtil;
define_holder(ILTGameUtil, g_pLTGameUtil);
#endif // !defined(PLATFORM_XENON)

#if !defined(PLATFORM_XENON)
#include "igamespy.h"
#endif // !PLATFORM_XENON

// the ILTServerContentTransfer interface
#include "iltservercontenttransfer.h"
static ILTServerContentTransfer* g_pLTServerContentTransfer;
define_holder(ILTServerContentTransfer, g_pLTServerContentTransfer);

// notification interface for content transfer
#include "iltcontenttransfernotification.h"

// Delay to wait in ms before kicking someone that has violated some condition.
#define KICKDELAY 5000

// VarTrack variables for debugging
#if defined(_SERVERBUILD) && !defined(_FINAL)
VarTrack	g_vtServerDisableConnectionTimeout;
VarTrack	g_vtServerDisableCRCCheck;
VarTrack	g_vtServerDisableCDKeyCheck;
#endif // _SERVERBUILD && !_FINAL

#if defined(PLATFORM_LINUX)
const unsigned int k_nWin32WcharSize		    = 2;
const unsigned int k_nLinuxWcharSize         = sizeof(wchar_t);
const unsigned int k_nWin32NetClientDataSize = sizeof(NetClientData) - (MAX_PLAYER_NAME * (k_nLinuxWcharSize - k_nWin32WcharSize));
const unsigned int k_nPlayerGUIDStartIndex   = MAX_PLAYER_NAME * k_nWin32WcharSize;
const unsigned int k_nDMModelStartIndex	     = k_nPlayerGUIDStartIndex + sizeof( LTGUID );
const unsigned int k_nTeamModelStartIndex    = k_nDMModelStartIndex + sizeof( uint8 );
#endif // PLATFORM_LINUX

class ContentTransferNotification : public ILTContentTransferNotification
{
	// ILTContentTransferNotification overrides.
	void FileSendProgressNotification(HCLIENT hClient, 
		const std::string& strFilename, 
		uint32			 nBytesSent,
		uint32			 nBytesTotal,
		float				 fTransferRate);
	void FileSendCompletedNotification(HCLIENT hClient,
		const std::string& strFilename,
		uint32			  nBytesTotal,
		float			  fTransferRate);
	void ServerContentTransferCompletedNotification(HCLIENT hClient);
	void ServerContentTransferErrorNotification(HCLIENT hClient, EContentTransferError eError);
};

// ILTContentTransferNotification implementation
void ContentTransferNotification::FileSendProgressNotification(HCLIENT			 hClient, 
														  const std::string& strFilename, 
														  uint32			 nBytesSent,
														  uint32			 nBytesTotal,
														  float				 fTransferRate)
{
#ifndef _FINAL
	g_pLTServer->CPrint("Sending '%s': %d of %d bytes (%8.1f k/s)", 
		strFilename.c_str(), 
		nBytesSent, 
		nBytesTotal, 
		fTransferRate / 1024.0f);
#endif // _FINAL
}

void ContentTransferNotification::FileSendCompletedNotification(HCLIENT			  hClient,
														   const std::string& strFilename,
														   uint32			  nBytesTotal,
														   float			  fTransferRate)
{
#ifndef _FINAL
	g_pLTServer->CPrint("Finished '%s'... %d bytes (%8.1f k/s)", 
		strFilename.c_str(), 
		nBytesTotal, 
		fTransferRate / 1024.0f);
#endif // _FINAL
}

void ContentTransferNotification::ServerContentTransferCompletedNotification(HCLIENT hClient)
{
	// find the game client data for this client
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData(hClient);

	// transition to loading state
	pGameClientData->SetClientConnectionState(eClientConnectionState_CRCCheck);
}

void ContentTransferNotification::ServerContentTransferErrorNotification(HCLIENT hClient, EContentTransferError eError)
{
	// find the game client data for this client
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData(hClient);

	// send the client to the error state
	ServerConnectionMgr::Instance( ).BootWithReason( *pGameClientData, eClientConnectionError_ContentTransferFailed, NULL );
}

static ContentTransferNotification s_ContentTransferNotification;
static bool s_bContentTransferInitialized = false;

class ConnectionStateMachine : public MacroStateMachine
{
public:

	ConnectionStateMachine( )
	{
		m_pGameClientData = NULL;
		m_StateTimer.SetEngineTimer( RealTimeTimer::Instance());
		m_bSentHello = false;
	}

	bool Init( GameClientData& gameClientData )
	{
		// Keep backpointer to containing object.
		m_pGameClientData = &gameClientData;
		return true;
	}

	bool Error_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Error_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool Error_OnExit( MacroStateMachine::EventParams& eventParams );
	bool Hello_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Hello_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool Hello_OnMessage( MacroStateMachine::EventParams& eventParams );
	bool KeyChallenge_OnMessage( MacroStateMachine::EventParams& eventParams );
	bool WaitingForAuth_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool Overrides_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool ContentTransfer_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool ContentTransfer_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool CRCCheck_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool CRCCheck_OnMessage( MacroStateMachine::EventParams& eventParams );
	bool LoggedIn_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Loading_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool Loading_OnMessage( MacroStateMachine::EventParams& eventParams );
	bool Loaded_OnUpdate( MacroStateMachine::EventParams& eventParams );
	bool InWorld_OnEnter( MacroStateMachine::EventParams& eventParams );
	bool InWorld_OnMessage( MacroStateMachine::EventParams& eventParams );
	bool PostLoadWorld_OnUpdate( MacroStateMachine::EventParams& eventParams );

	// State handler table.
	MSM_BeginTable( ConnectionStateMachine )
		MSM_BeginState( eClientConnectionState_Error )
			MSM_OnEnter( Error_OnEnter )
			MSM_OnUpdate( Error_OnUpdate )
			MSM_OnExit( Error_OnExit )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_Hello )
			MSM_OnEnter( Hello_OnEnter )
			MSM_OnUpdate( Hello_OnUpdate )
			MSM_OnMsg( Hello_OnMessage )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_KeyChallenge )
			MSM_OnMsg( KeyChallenge_OnMessage )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_WaitingForAuth )
			MSM_OnUpdate( WaitingForAuth_OnUpdate )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_Overrides )
			MSM_OnEnter( Overrides_OnEnter )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_ContentTransfer )
			MSM_OnEnter( ContentTransfer_OnEnter )
			MSM_OnUpdate( ContentTransfer_OnUpdate )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_CRCCheck )
			MSM_OnEnter( CRCCheck_OnEnter )
			MSM_OnMsg( CRCCheck_OnMessage )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_LoggedIn )
			MSM_OnEnter( LoggedIn_OnEnter )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_Loading )
			MSM_OnEnter( Loading_OnEnter )
			MSM_OnMsg( Loading_OnMessage )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_Loaded )
			MSM_OnUpdate( Loaded_OnUpdate )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_InWorld )
			MSM_OnEnter( InWorld_OnEnter )
			MSM_OnMsg( InWorld_OnMessage )
		MSM_EndState( )
		MSM_BeginState( eClientConnectionState_PostLoadWorld )
			MSM_OnUpdate( PostLoadWorld_OnUpdate )
		MSM_EndState( )
	MSM_EndTable( )

private:

	void InitializeContentTransfer();

private:

	GameClientData* m_pGameClientData;
	StopWatchTimer m_StateTimer;
	bool m_bSentHello;
};

bool ConnectionStateMachine::Error_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Kick ourselves after a short time.  This allows any error messages to get
	// properly sent before removing the client and it also confuses hackers from knowing
	// just what got them kicked.
	m_pGameClientData->SetKickTime( RealTimeTimer::Instance( ).GetTimerAccumulatedMS() + ( KICKDELAY ));
	return true;
}

bool ConnectionStateMachine::Error_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// Check if we should wait more before booting this guy.
	if( m_pGameClientData->GetKickTime( ) > RealTimeTimer::Instance( ).GetTimerAccumulatedMS( ))
		return true;

	g_pLTServer->KickClient( m_pGameClientData->GetClient( ));
	return true;
}

bool ConnectionStateMachine::Error_OnExit( MacroStateMachine::EventParams& eventParams )
{
	// Can never leave error state.
	return false;
}

bool ConnectionStateMachine::Hello_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Haven't sent hello yet.
	m_bSentHello = false;

	// Check if we're the host.  The host needs to wait for its public ip to come back before
	// it can proceed with the connection state.  This is so it can register its own cdkey properly.
	// Not important for lan only.
	bool bIsHost = g_pLTServer->GetClientInfoFlags( m_pGameClientData->GetClient( )) & CIF_LOCAL;
	if( bIsHost && !GameModeMgr::Instance( ).m_ServerSettings.m_bLANOnly )
	{
		// Wait a maximum of 1 second for the host to get its public ip back.
		m_StateTimer.Start( 1.0f );
	}

	return true;
}

bool ConnectionStateMachine::Hello_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// Already sent hello, just waiting for response.
	if( m_bSentHello )
		return true;

	// Check if our state timer is going.
	if( m_StateTimer.IsStarted() && !m_StateTimer.IsTimedOut())
	{
		// Check if we got our public ip.  If we did, then we can proceed with the connection.
		char szPublicIP[16];
		uint16 nPublicPort;
		if( !g_pGameServerShell->GetGameSpyServer( )->GetPublicIPandPort( szPublicIP, LTARRAYSIZE( szPublicIP ), nPublicPort ))
		{
			// Wait longer.
			return true;
		}

		m_StateTimer.Stop();
	}

	// Start a handshake with the client
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8(eClientConnectionState_Hello);
	g_pLTGameUtil->WriteHandshakingVersion( cMsg, GAME_HANDSHAKE_VER, GAME_HANDSHAKE_VER_MASK );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, GameModeMgr::Instance().GetGameModeRecord( ));
	GameModeMgr::Instance().WriteToMsg( cMsg );
	cMsg.WriteString( g_pGameServerShell->GetCurLevel( ));
	g_pLTServer->SendToClient(cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	g_pServerMissionMgr->SendServerGameState( m_pGameClientData->GetClient( ));

	if( IsMultiplayerGameServer( ))
	{
		// Send connected message.
		cMsg.Reset();
		cMsg.Writeuint8(MID_MULTIPLAYER_DATA);
		char sBuf[32];
		int  nBufSize = 30;
		WORD wPort = 0;
		g_pLTServer->GetTcpIpAddress(sBuf, nBufSize, wPort);
		cMsg.WriteString(sBuf);
		cMsg.Writeuint32((uint32) wPort);
		uint32  nClientID = g_pLTServer->GetClientID( m_pGameClientData->GetClient( ));
		cMsg.Writeuint32( nClientID );
		g_pLTServer->SendToClient(cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

		if( GameModeMgr::Instance( ).m_grbUseTeams )
		{
			// tell the player about the current teams
			CTeamMgr::Instance().UpdateClient( m_pGameClientData->GetClient( ));
		}

		// Don't just broadcast the list.  That would send messages to clients that aren't fully connected yet.
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		for( ; iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
		{
			CPlayerObj* pExistingPlayer = *iter;
			if( !pExistingPlayer || !pExistingPlayer->GetClient() )
				continue;

			GameClientData* pExistingGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( pExistingPlayer->GetClient( ));
			if( !pExistingGameClientData )
				continue;

			// Only tell new clients about clients that are inworld.  If they're not inworld yet, 
			// this client will be given a join message when they do reach that state.
			if( pExistingGameClientData->GetClientConnectionState( ) != eClientConnectionState_InWorld )
				continue;

			// Tell the new Client about each client already on the server...
			g_pGameServerShell->SendPlayerInfoMsgToClients( m_pGameClientData->GetClient(), pExistingPlayer, MID_PI_EXIST);
		}
	}

	// Record that we've sent the hello, now wait for the response.
	m_bSentHello = true;

	return true;
}

bool ConnectionStateMachine::Hello_OnMessage( MacroStateMachine::EventParams& eventParams )
{
	MessageEventParams& msgEventParams = ( MessageEventParams& )eventParams;

	// Make sure we got the right message.
	EClientConnectionState eClientConnectionState = ( EClientConnectionState )msgEventParams.m_pMsg->Readuint8();
	if( eClientConnectionState != eClientConnectionState_Hello )
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	// Check the client's version
	int nHandshakeVer = (int)g_pLTGameUtil->ReadHandshakingVersion( *msgEventParams.m_pMsg, GAME_HANDSHAKE_VER_MASK );
	if (nHandshakeVer != GAME_HANDSHAKE_VER)
	{
		// If they got here, they ignored our different version number.  Boot 'em!
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	// Read in the client's sooper-secret key
	uint32 nClientKey1 = g_pLTGameUtil->ReadClientKey1( *msgEventParams.m_pMsg );

	// Check if we're the host.
	bool bIsHost = g_pLTServer->GetClientInfoFlags( m_pGameClientData->GetClient( )) & CIF_LOCAL;

	NetClientData ncd;
	if( !m_pGameClientData->GetNetClientData( ncd ))
	{
		// Invalid netclientdata.
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	// Write out a keychallenge message, encrypted with their key
	// Note : A key from them that null encrypts our key doesn't really effect
	// anything..  I mean, they DID send the key, and they're the ones that need
	// to use our key anyway..
	CAutoMessage cResponse;
	cResponse.Writeuint8(MID_CLIENTCONNECTION);
	cResponse.Writeuint8(eClientConnectionState_KeyChallenge);
	g_pLTGameUtil->WriteServerKey( cResponse, nClientKey1, ncd.m_PlayerGuid, (uint32)m_pGameClientData );
	cResponse.Writebool((!bIsHost && GameModeMgr::Instance( ).m_ServerSettings.m_bUsePassword));

#if !defined(ENABLE_CDKEY_1_CHECK) && !defined(ENABLE_CDKEY_2_CHECK)

	bool bDoCDKey = false;

#else

	bool bDoCDKey = ( g_pGameServerShell->GetGameSpyServer() != NULL );

#endif

	// Write out the cdkey challenge.
	if( bDoCDKey )
	{
#if !defined(PLATFORM_XENON)
		// Say we are including the cdkey challenge.
		cResponse.Writebool( true );

		// Write the challenge message.
		char szChallenge[16];
		uint32 nSizeChallenge = LTARRAYSIZE( szChallenge );
		g_pGameServerShell->GetGameSpyServer()->GetCDKeyChallenge( szChallenge, nSizeChallenge );
		cResponse.WriteString( szChallenge );
#endif // !PLATFORM_XENON
	}
	else
	{
		// Not doing cdkey challenge.
		cResponse.Writebool( false );
	}

	g_pLTServer->SendToClient(cResponse.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	// Now waiting for password response.
	m_pGameClientData->SetClientConnectionState( eClientConnectionState_KeyChallenge );

	return true;
}

bool ConnectionStateMachine::KeyChallenge_OnMessage( MacroStateMachine::EventParams& eventParams )
{
	MessageEventParams& msgEventParams = ( MessageEventParams& )eventParams;

	// Make sure we got the right message.
	EClientConnectionState eClientConnectionState = ( EClientConnectionState )msgEventParams.m_pMsg->Readuint8();
	if( eClientConnectionState != eClientConnectionState_KeyChallenge )
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	NetClientData ncd;
	if( !m_pGameClientData->GetNetClientData(ncd))
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	uint32 nServerKey = (uint32)m_pGameClientData;
	uint32 nServerClientKey = g_pLTGameUtil->ReadServerKeyData( *msgEventParams.m_pMsg, ncd.m_PlayerGuid, (uint32)m_pGameClientData );

	bool bInvalidPassword = false;

	// Check if we're the host.
	bool bIsHost = g_pLTServer->GetClientInfoFlags( m_pGameClientData->GetClient( )) & CIF_LOCAL;

	// Validate
	if( nServerClientKey != nServerKey )
	{
		bInvalidPassword = true;
	}
	else
	{
		uint32 nHashedPassword = msgEventParams.m_pMsg->Readuint32();
		uint32 nServerHashedPassword = str_Hash(GameModeMgr::Instance( ).m_ServerSettings.m_sPassword.c_str( ));

		// Validate
		if (!bIsHost && GameModeMgr::Instance( ).m_ServerSettings.m_bUsePassword && nHashedPassword != nServerHashedPassword)
		{
			bInvalidPassword = true;
		}
	}

	if (bInvalidPassword)
	{
		// They don't know the password - tell the client so it can disconnect itself.
		ServerConnectionMgr::Instance( ).BootWithReason( *m_pGameClientData, eClientConnectionError_WrongPassword, NULL );
		return true;
	}

	// read PunkBuster state
	bool bIsPunkBusterEnabled = g_pLTGameUtil->ReadPunkBusterState(*msgEventParams.m_pMsg, ncd.m_PlayerGuid, (uint32)m_pGameClientData);
	m_pGameClientData->SetIsPunkBusterEnabled(bIsPunkBusterEnabled);

#if !defined(ENABLE_CDKEY_1_CHECK) && !defined(ENABLE_CDKEY_2_CHECK)

	bool bDoCDKey = false;

#else

	bool bDoCDKey = ( g_pGameServerShell->GetGameSpyServer() != NULL );

#endif

	// Read the CDKey challenge response.
	// Tell gamespy about the challenge response.
	if( bDoCDKey )
	{
#if !defined(PLATFORM_XENON)
		char szChallenge[256] = "";

#if defined(ENABLE_CDKEY_1_CHECK) || defined(ENABLE_CDKEY_2_CHECK)
		msgEventParams.m_pMsg->ReadString( szChallenge, LTARRAYSIZE( szChallenge ));
#endif

		char szChallengeResponse[256] = "";
#if defined(ENABLE_CDKEY_1_CHECK)
		msgEventParams.m_pMsg->ReadString( szChallengeResponse, LTARRAYSIZE( szChallengeResponse ));
#endif

		char szPreSaleChallengeResponse[256] = "";
#if defined(ENABLE_CDKEY_2_CHECK)
		msgEventParams.m_pMsg->ReadString( szPreSaleChallengeResponse, LTARRAYSIZE( szPreSaleChallengeResponse ));
#endif

		// If they didn't have a presale challenge response, then try their main cdkey challenge response for the
		// presale cdkey.
		if( LTStrEmpty( szPreSaleChallengeResponse ))
		{
			LTStrCpy( szPreSaleChallengeResponse, szChallengeResponse, LTARRAYSIZE( szPreSaleChallengeResponse ));
		}

		uint32 nClientId = g_pLTServer->GetClientID( m_pGameClientData->GetClient( ));

		char szIpAddr[16];
		uint16 nPort;
		// Get the ip address from gamespy if we're host and not on lan.
		if( bIsHost && !GameModeMgr::Instance( ).m_ServerSettings.m_bLANOnly )
		{
			g_pGameServerShell->GetGameSpyServer( )->GetPublicIPandPort( szIpAddr, LTARRAYSIZE( szIpAddr ), nPort );
		}
		// Just ask engine what ip of remote host is.
		else
		{
			uint8 aTcpIp[4];
			g_pLTServer->GetClientAddr( m_pGameClientData->GetClient( ), aTcpIp, &nPort );
			LTSNPrintF( szIpAddr, LTARRAYSIZE( szIpAddr ), "%d.%d.%d.%d", aTcpIp[0], aTcpIp[1], aTcpIp[2], aTcpIp[3] );
		}

		g_pGameServerShell->GetGameSpyServer()->ProcessCDKeyChallengeResponse( nClientId, szIpAddr, szChallenge, 
			szChallengeResponse, szPreSaleChallengeResponse );
#endif // !PLATFORM_XENON
	}


#ifndef _FINAL
	// check to see if the skip CRC console variable is set
	if (g_vtServerDisableCDKeyCheck.GetFloat() != 0)
	{
		// move to the loading state
		g_pLTServer->CPrint("Skipping CDKey check.");
		bDoCDKey = false;
	}
#endif

	if( bDoCDKey )
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_WaitingForAuth );
	}
	else
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Overrides );
	}

	return true;
}

bool ConnectionStateMachine::WaitingForAuth_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	// Check if we're using gamespy.
	if( g_pGameServerShell->GetGameSpyServer( ))
	{
		uint32 nClientId = g_pLTServer->GetClientID( m_pGameClientData->GetClient( ));
		IGameSpyServer::EChallengeResponse eMainChallengeResponse = IGameSpyServer::eChallengeResponse_Invalid;
		IGameSpyServer::EChallengeResponse ePreSaleChallengeResponse = IGameSpyServer::eChallengeResponse_Invalid;
		g_pGameServerShell->GetGameSpyServer( )->GetChallengeResponse( nClientId, eMainChallengeResponse, ePreSaleChallengeResponse );

		bool bRequiresCDKey1 = false;
		bool bRequiresCDKey2 = false;
#ifdef ENABLE_CDKEY_1_CHECK
		bRequiresCDKey1 = true;
#endif
#ifdef ENABLE_CDKEY_2_CHECK
		bRequiresCDKey2 = true;
#endif
		
		// Check the main response.
		switch( eMainChallengeResponse )
		{
			// Still waiting for response on authorization.
		case IGameSpyServer::eChallengeResponse_Processing:
			break;
			// Got in!
		case IGameSpyServer::eChallengeResponse_Accepted:
		case IGameSpyServer::eChallengeResponse_Accepted_Timeout:
			{
				if( bRequiresCDKey2 )
				{
					// Keep waiting if we haven't gotten the presale response yet.
					if( ePreSaleChallengeResponse == IGameSpyServer::eChallengeResponse_Processing )
						break;

					// Check if presale accepted.  If not accepted, then they just won't be
					// able to use presale content.  Don't allow timeout acceptance.  Gamespy has a bug
					// that gives false-positives if gameserver is behind a NAT.
					if( ePreSaleChallengeResponse == IGameSpyServer::eChallengeResponse_Accepted )
					{
						m_pGameClientData->SetAllowPreSaleContent( true );
					}
					else
					{
						m_pGameClientData->SetAllowPreSaleContent( false );
					}
				}

				// Can only ban users if cdkey required, since it uses the cdkey hash.
				if( bRequiresCDKey1 )
				{
					// now that we have the client cdkey hash, check to see if it is banned
					const char* pszUserCDKeyHash = NULL;
					g_pGameServerShell->GetGameSpyServer()->GetUserCDKeyHash(nClientId, pszUserCDKeyHash);
					if (BanUserMgr::Instance().IsClientBanned(pszUserCDKeyHash))
					{
						ServerConnectionMgr::Instance( ).BootWithReason( *m_pGameClientData, eClientConnectionError_Banned, NULL );
						break;
					}

					// check for a PunkBuster ban on this client
					uint8  aClientAddress[4] = { 0 };
					uint16 nPort;
					g_pLTServer->GetClientAddr(m_pGameClientData->GetClient(), aClientAddress, &nPort);
					
					char szClientAddress[255] = { 0 };
					LTSNPrintF(szClientAddress, LTARRAYSIZE(szClientAddress), "%d.%d.%d.%d:%d",
								aClientAddress[0], szClientAddress[1], szClientAddress[2], szClientAddress[3], nPort);
					
					int nIsPunkBusterEnabled = m_pGameClientData->GetIsPunkBusterEnabled() ? 1 : 0;
					const char* pszBanReason = g_pGameServerShell->GetPunkBusterServer()->IsClientBanned(szClientAddress, nIsPunkBusterEnabled, pszUserCDKeyHash);
					if (pszBanReason)
					{
						ServerConnectionMgr::Instance( ).BootWithReason( *m_pGameClientData, eClientConnectionError_PunkBuster, pszBanReason );
						break;			
					}
				}

				// transition to overrides state
				m_pGameClientData->SetClientConnectionState( eClientConnectionState_Overrides );
				break;
			}
			// Problems with cdkey.
		case IGameSpyServer::eChallengeResponse_Invalid:
		case IGameSpyServer::eChallengeResponse_Rejected:
			{
				if( bRequiresCDKey1 )
				{
					// Tell client it has an invalid cdkey.
					ServerConnectionMgr::Instance( ).BootWithReason( *m_pGameClientData, eClientConnectionError_BadCDKey, NULL );
					break;
				}
				else if( bRequiresCDKey2 )
				{
					// Keep waiting if we haven't gotten the presale response yet.
					if( ePreSaleChallengeResponse == IGameSpyServer::eChallengeResponse_Processing )
						break;

					// Check if presale accepted.  If not accepted, then they just won't be
					// able to use presale content.  Don't allow timeout acceptance.  Gamespy has a bug
					// that gives false-positives if gameserver is behind a NAT.
					if( ePreSaleChallengeResponse == IGameSpyServer::eChallengeResponse_Accepted )
					{
						m_pGameClientData->SetAllowPreSaleContent( true );
					}
					else
					{
						m_pGameClientData->SetAllowPreSaleContent( false );
					}

					// transition to overrides state
					m_pGameClientData->SetClientConnectionState( eClientConnectionState_Overrides );
					break;
				}
			}
		}
	}
	else
#endif // !PLATFORM_XENON
	{
		// Don't need to wait, give them authentication.
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Overrides );
	}

	return true;
}

bool ConnectionStateMachine::Overrides_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// if we have multiplayer overrides, send the compressed overrides buffer
	// to the client
	if (g_pGameServerShell->GetCompressedOverridesData())
	{
		CAutoMessage cOverridesMessage;
		cOverridesMessage.Writeuint8(MID_CLIENTCONNECTION);
		cOverridesMessage.Writeuint8(eClientConnectionState_Overrides);
		cOverridesMessage.Writeuint32(g_pGameServerShell->GetDecompressedOverridesDataSize());
		cOverridesMessage.Writeuint32(g_pGameServerShell->GetCompressedOverridesDataSize());
		cOverridesMessage.WriteData(g_pGameServerShell->GetCompressedOverridesData(), g_pGameServerShell->GetCompressedOverridesDataSize() * 8);

		g_pLTServer->SendToClient(cOverridesMessage.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);
	}

	m_pGameClientData->SetClientConnectionState( eClientConnectionState_ContentTransfer );

	return true;
}

bool ConnectionStateMachine::ContentTransfer_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	if (!s_bContentTransferInitialized)
	{
		InitializeContentTransfer();
	}

	// if this is the local client we can skip content transfer
	uint32 nClientFlags = g_pLTServer->GetClientInfoFlags(m_pGameClientData->GetClient());
	if (nClientFlags & CIF_LOCAL)
	{
		// transition to the CRC check state
		m_pGameClientData->SetClientConnectionState(eClientConnectionState_CRCCheck);
		return true;
	}

	// tell client to transition to content transfer state
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8(eClientConnectionState_ContentTransfer);
	g_pLTServer->SendToClient(cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	// call the server content transfer handler to send content transfer info to the client
	g_pLTServerContentTransfer->SendTransferInfoToClient(m_pGameClientData->GetClient());

	return true;
}

bool ConnectionStateMachine::ContentTransfer_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// update the server content transfer handler
	if (g_pLTServerContentTransfer->OnUpdate(m_pGameClientData->GetClient()) == LT_OK)
	{
		// client is alive so update the time
		m_pGameClientData->SetLastClientUpdateTimeMS(RealTimeTimer::Instance().GetTimerAccumulatedMS());
	}

	return true;
}



// this is called one time to initialize the content transfer facility
void ConnectionStateMachine::InitializeContentTransfer()
{
	ServerSettings& cServerSettings = GameModeMgr::Instance( ).m_ServerSettings;

	// build redirect list
	uint32 nNumberOfRedirectURLs = cServerSettings.m_sRedirectURLs.size();
	char** ppRedirectURLsArray = debug_newa(char*, nNumberOfRedirectURLs);
	uint32 nArrayIndex = 0;

	for (StringArray::iterator itRedirectURL = cServerSettings.m_sRedirectURLs.begin();
		itRedirectURL != cServerSettings.m_sRedirectURLs.end();
		++itRedirectURL)
	{
		std::string strRedirectURL = *itRedirectURL;
		ppRedirectURLsArray[nArrayIndex] = debug_newa(char, strRedirectURL.size() + 1);
		LTStrCpy(ppRedirectURLsArray[nArrayIndex], strRedirectURL.c_str(), strRedirectURL.size() + 1);

		++nArrayIndex;
	}

	// build the options structure for content download
	SServerContentTransferOptions cOptions;

	// copy the redirect URLs prior to passing them in to ensure a contiguous array
	StringArray cRedirectArray;
	cRedirectArray.resize(cServerSettings.m_sRedirectURLs.size());
	cRedirectArray = cServerSettings.m_sRedirectURLs;

	cOptions.m_bAllowContentDownload      = cServerSettings.m_bAllowContentDownload;
	cOptions.m_nClientTimeoutMS           = ServerConnectionMgr::Instance().GetAutoBootTimeMS();
	cOptions.m_nMaxDownloadRatePerClient  = cServerSettings.m_nMaxDownloadRatePerClient;
	cOptions.m_nMaxDownloadRateAllClients = cServerSettings.m_nMaxDownloadRateAllClients;
	cOptions.m_nMaxSimultaneousDownloads  = cServerSettings.m_nMaxSimultaneousDownloads;
	cOptions.m_nMaxDownloadSize			  = cServerSettings.m_nMaxDownloadSize;
	cOptions.m_ppszRedirectURLs			  = (const char**) ppRedirectURLsArray;
	cOptions.m_nNumberOfRedirectURLs	  = nNumberOfRedirectURLs;
	cOptions.m_pwszContentDownloadMessage = cServerSettings.m_sContentDownloadMessage.c_str();

	// set the notification sink and options
	uint32 nNotificationOptions = k_nNotifyContentTransferCompleted | k_nNotifyFileSendProgress | k_nNotifyFileSendCompleted;
	g_pLTServerContentTransfer->SetNotificationSink(&s_ContentTransferNotification, nNotificationOptions);
	g_pLTServerContentTransfer->SetOptions(cOptions);	

	// free the redirect array
	for (nArrayIndex = 0; nArrayIndex < nNumberOfRedirectURLs; ++nArrayIndex)
	{
		delete [] ppRedirectURLsArray[nArrayIndex];
	}	
	delete [] ppRedirectURLsArray;

	s_bContentTransferInitialized = true;
}

bool ConnectionStateMachine::CRCCheck_OnEnter( MacroStateMachine::EventParams& eventParams )
{
#ifndef _FINAL
	// check to see if the skip CRC console variable is set
	if (g_vtServerDisableCRCCheck.GetFloat() != 0)
	{
		// move to the loading state
		g_pLTServer->CPrint("Skipping CRC check.");
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_LoggedIn );
		return true;
	}
#endif

	// send CRC request message
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8(eClientConnectionState_CRCCheck);
	g_pLTServer->SendToClient(cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	return true;
}

bool ConnectionStateMachine::CRCCheck_OnMessage( MacroStateMachine::EventParams& eventParams )
{
	MessageEventParams& msgEventParams = ( MessageEventParams& )eventParams;

	// Make sure we got the right message.
	EClientConnectionState eClientConnectionState = ( EClientConnectionState )msgEventParams.m_pMsg->Readuint8();
	if( eClientConnectionState != eClientConnectionState_CRCCheck )
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	NetClientData ncd;
	if( !m_pGameClientData->GetNetClientData(ncd))
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	bool bValidAssets = g_pLTGameUtil->ReadCRCMessage( *msgEventParams.m_pMsg, ncd.m_PlayerGuid, (uint32)m_pGameClientData, DB_Default_File );

#ifndef _DEBUG
	if( !bValidAssets )
	{
		// One or more files failed the CRC check - tell the client so it can disconnect itself.
		ServerConnectionMgr::Instance( ).BootWithReason( *m_pGameClientData, eClientConnectionError_InvalidAssets, NULL );
		return true;
	}
#endif

	m_pGameClientData->SetClientConnectionState( eClientConnectionState_LoggedIn );

	return true;
}
bool ConnectionStateMachine::LoggedIn_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Don't need to do password and stuff anymore.
	m_pGameClientData->SetHasPassedSecurity( true );

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	// Tell the client they passed the test...
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8((uint8)eClientConnectionState_LoggedIn);
	// Tell client about optional server message.
	bool bHasServerMessage = !gameModeMgr.m_ServerSettings.m_sServerMessage.empty();
	cMsg.Writebool( bHasServerMessage );
	if( bHasServerMessage )
	{
		cMsg.WriteWString( gameModeMgr.m_ServerSettings.m_sServerMessage.c_str( ));
	}
	// Tell client about optional briefing override message.
	bool bHasOverrideBriefingMessage = !gameModeMgr.m_ServerSettings.m_sBriefingOverrideMessage.empty();
	cMsg.Writebool( bHasOverrideBriefingMessage );
	if( bHasOverrideBriefingMessage )
	{
		cMsg.WriteWString( gameModeMgr.m_ServerSettings.m_sBriefingOverrideMessage.c_str( ));
	}
	g_pLTServer->SendToClient(cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	// Tell the client about rotation.
	cMsg.Reset( );
	cMsg.Writeuint8( MID_MAPLIST );

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

		cMsg.WriteString( g_pMissionDB->GetWorldName(hLevel,true) );

	}

	g_pLTServer->SendToClient( cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	return m_pGameClientData->SetClientConnectionState( eClientConnectionState_Loading );
}


bool ConnectionStateMachine::Loading_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Consider us not in the world yet.
	m_pGameClientData->SetClientInWorld( false );

	// Tell server to let the client in.  Server may already have let client in
	// due a level switch.
	g_pLTServer->FinishClientConnect( m_pGameClientData->GetClient( ));

	// Tell the client they passed the test...
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8((uint8)eClientConnectionState_Loading);
	g_pLTServer->SendToClient(cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	// Tell the client about the pause state.
	cMsg.Reset();
	cMsg.Writeuint8( MID_GAME_PAUSE );
	cMsg.Writebool( SimulationTimer::Instance( ).IsTimerPaused( ));
	g_pLTServer->SendToClient( cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED );

	cMsg.Reset( );
	cMsg.Writeuint8( MID_SWITCHINGWORLDSSTATE );
	cMsg.Writeuint8(( uint8 )g_pGameServerShell->GetSwitchingWorldsState( ));
	g_pLTServer->SendToClient( cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	// Tell the client about our simulation timer scale.
	g_pGameServerShell->SendSimulationTimerScale( m_pGameClientData->GetClient( ));

	g_pLTServer->SendToClient( cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	return true;
}

bool ConnectionStateMachine::Loading_OnMessage( MacroStateMachine::EventParams& eventParams )
{
	MessageEventParams& msgEventParams = ( MessageEventParams& )eventParams;

	// Make sure we got the right message.
	EClientConnectionState eClientConnectionState = ( EClientConnectionState )msgEventParams.m_pMsg->Readuint8();
	if( eClientConnectionState != eClientConnectionState_Loading )
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	// Set the client to be inworld.
	m_pGameClientData->SetClientConnectionState( eClientConnectionState_Loaded );

	return true;
}

bool ConnectionStateMachine::Loaded_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// Wait for the client to get a player.  This can take a frame or two for the 
	// server to call OnClientEnterWorld.
	CPlayerObj* pPlayerObj = GetPlayerFromHClient( m_pGameClientData->GetClient( ));
	if( !pPlayerObj )
		return true;

	m_pGameClientData->SetClientConnectionState( eClientConnectionState_InWorld );

	return true;
}

bool ConnectionStateMachine::InWorld_OnEnter( MacroStateMachine::EventParams& eventParams )
{
	// Set the client to be in world.
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8((uint8)eClientConnectionState_InWorld);
	cMsg.Writeuint8( g_pServerMissionMgr->GetCurrentRound( ));
	g_pLTServer->SendToClient(cMsg.Read(), m_pGameClientData->GetClient( ), MESSAGE_GUARANTEED);

	if( IsMultiplayerGameServer( ))
	{
		CPlayerObj* pPlayerObj = GetPlayerFromHClient( m_pGameClientData->GetClient( ));
		if( !pPlayerObj )
		{
			LTERROR( "Should have a player at this point." );
			return false;
		}

		// Put the player at a startpoint.
		GameStartPoint* pStartPt = GameStartPointMgr::Instance().FindStartPoint(pPlayerObj);
		LTVector vPos(0.0f, 0.0f, 0.0f);
		LTRotation rRot;
		if (pStartPt)
		{
			// Set our starting values...
			g_pLTServer->GetObjectPos(pStartPt->m_hObject, &vPos);
			g_pLTServer->GetObjectRotation(pStartPt->m_hObject, &rRot);
		}
		pPlayerObj->Teleport( vPos, rRot );
		m_pGameClientData->SetLastStartPoint( NULL );
	}

	return true;
}

bool ConnectionStateMachine::InWorld_OnMessage( MacroStateMachine::EventParams& eventParams )
{
	MessageEventParams& msgEventParams = ( MessageEventParams& )eventParams;

	// Make sure we got the right message.
	EClientConnectionState eClientConnectionState = ( EClientConnectionState )msgEventParams.m_pMsg->Readuint8();
	if( eClientConnectionState != eClientConnectionState_InWorld )
	{
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	CPlayerObj* pPlayerObj = GetPlayerFromHClient( m_pGameClientData->GetClient( ));
	if( !pPlayerObj )
	{
		LTERROR( "Invalid player." );
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	NetClientData ncd;
	if( !m_pGameClientData->GetNetClientData( ncd ))
	{
		LTERROR( "Invalid net client data." );
		m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
		return true;
	}

	// Get the name the client would like.
	msgEventParams.m_pMsg->ReadWString( ncd.m_szName, LTARRAYSIZE( ncd.m_szName ));
	// Set our new changed netclientdata.
	m_pGameClientData->SetNetClientData( ncd );

	// Read in model information.
	uint8 nMPModelIdnex = msgEventParams.m_pMsg->Readuint8( );
	m_pGameClientData->SetMPModelIndex( nMPModelIdnex );

	// Setup team info.
	uint8 nTeam = msgEventParams.m_pMsg->Readuint8( );
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		CTeamMgr::Instance().AddPlayer( g_pLTServer->GetClientID( m_pGameClientData->GetClient( )), nTeam );
		CTeamMgr::Instance().UpdateClient();
		m_pGameClientData->SetLastTeamId( nTeam );
	}

	// Setup loadout.
	uint8 nLoadout = msgEventParams.m_pMsg->Readuint8( );
	m_pGameClientData->SetLoadout( nLoadout );

	// Get the insignia.  Only sent as a file title to reduce bandwidth, so we have to recreate the full path.
	char szPatchTitle[MAX_PATH] = "";
	msgEventParams.m_pMsg->ReadString(szPatchTitle,LTARRAYSIZE(szPatchTitle));
	char szPatchDir[MAX_PATH] = "";
	g_pModelsDB->GetInsigniaFolder( szPatchDir, LTARRAYSIZE( szPatchDir ));
	char szPatchPath[MAX_PATH*2];
	LTSNPrintF( szPatchPath, LTARRAYSIZE( szPatchPath ), "%s%s.dds", szPatchDir, szPatchTitle );
	m_pGameClientData->SetInsignia(szPatchPath);

	// get the world CRC value from the message
	uint32 nClientWorldCRC = g_pLTGameUtil->ReadServerKeyData( *msgEventParams.m_pMsg, ncd.m_PlayerGuid, ( uint32 )m_pGameClientData );

	// compare it to the local value if we're a multiplayer host
	if (IsMultiplayerGameServer())
	{
		uint32 nServerWorldCRC = 0;
		if (g_pLTServer->GetWorldCRC(nServerWorldCRC) != LT_OK)
		{
			m_pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
			return true;
		}

		// this flag is used to skip the CRC check if the console variable is set (always
		// false in final builds)
		bool bSkipCRCCheck = false;

#ifndef _FINAL
		// check to see if the skip CRC console variable is set
		if (g_vtServerDisableCRCCheck.GetFloat() != 0)
		{
			g_pLTServer->CPrint("Skipping world asset CRC check.");
			bSkipCRCCheck = true;
		}
#endif

		if (!bSkipCRCCheck && (nClientWorldCRC != nServerWorldCRC))
		{
			ServerConnectionMgr::Instance( ).BootWithReason( *m_pGameClientData, eClientConnectionError_InvalidAssets, NULL );
			return true;
		}
	}

	// Send message to all clients other than ourselves.  We need to send to all
	// clients not just players because some other clients may still be in the process
	// of getting players.
	ServerConnectionMgr::GameClientDataList::iterator iter = ServerConnectionMgr::Instance().GetGameClientDataList( ).begin( );
	for( ; iter != ServerConnectionMgr::Instance().GetGameClientDataList( ).end( ); iter++ )
	{
		GameClientData* pOtherGameClientData = *iter;

		// Skip ourselves.
		if( pOtherGameClientData->GetClient() == m_pGameClientData->GetClient( ))
		{
			continue;
		}

		// Send a message to each existing clients, letting them know a  new client has joined the game...
		g_pGameServerShell->SendPlayerInfoMsgToClients(pOtherGameClientData->GetClient( ), pPlayerObj, MID_PI_JOIN);
	}


	// Hand init info off to the player.
	if( !pPlayerObj->ClientInit( ))
	{
		LTERROR( "Could not intialize player with client data." );
		return false;
	}

	m_pGameClientData->SetClientInWorld( true );

	if( IsMultiplayerGameServer( ))
	{
		g_pGameServerShell->SendPlayerInfoMsgToClients( NULL, pPlayerObj, MID_PI_UPDATE );
		pPlayerObj->SetSpectatorMode( eSpectatorMode_Fly, false );
	}
	else
	{
		pPlayerObj->Respawn();
		g_pServerMissionMgr->OnPlayerInWorld( *pPlayerObj );
	}

	return true;
}

bool ConnectionStateMachine::PostLoadWorld_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	m_pGameClientData->SetClientConnectionState( eClientConnectionState_Loading );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::GameClientData()
//
//	PURPOSE:	ctor
//
// ----------------------------------------------------------------------- //
GameClientData::GameClientData( HCLIENT hClient )
{
	m_hClient = hClient;
	m_nLastClientUpdateTimeMS = RealTimeTimer::Instance().GetTimerAccumulatedMS();
	m_eClientConnectionState = eClientConnectionState_None;
	m_nLastTeamId = INVALID_TEAM;
	m_nKickTime = 0;
	m_bPassedSecurity = false;
	m_nRequestedTeam = INVALID_TEAM;
	m_bClientInWorld = false;
	m_nLoadout = (uint8) -1;
	m_pConnectionStateMachine = NULL;
	m_bAllowPreSaleContent = true;
	m_nClientMoveCode = 0;
	m_bIsPunkBusterEnabled = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::~GameClientData()
//
//	PURPOSE:	dtor
//
// ----------------------------------------------------------------------- //
GameClientData::~GameClientData( )
{
	// Fire the remove client event.
	RemoveClient.DoNotify();

	if( m_hStandbyPlayer )
	{
		g_pLTServer->RemoveObject( m_hStandbyPlayer );
		m_hStandbyPlayer = NULL;
	}

	if( m_pConnectionStateMachine )
	{
		delete m_pConnectionStateMachine;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::Init()
//
//	PURPOSE:	Initializer
//
// ----------------------------------------------------------------------- //
bool GameClientData::Init( )
{
	LT_MEM_TRACK_ALLOC(m_pConnectionStateMachine = new ConnectionStateMachine, LT_MEM_TYPE_GAMECODE);
	if( !m_pConnectionStateMachine )
		return false;

	return m_pConnectionStateMachine->Init( *this );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::SetClientConnectionState()
//
//	PURPOSE:	State accessors
//
// ----------------------------------------------------------------------- //
bool GameClientData::SetClientConnectionState( EClientConnectionState eClientConnectionState )
{ 
	return m_pConnectionStateMachine->SetState( eClientConnectionState ); 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::SetClientConnectionState()
//
//	PURPOSE:	State accessors
//
// ----------------------------------------------------------------------- //
EClientConnectionState GameClientData::GetClientConnectionState( ) const
{
	return ( EClientConnectionState )m_pConnectionStateMachine->GetState();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::Update()
//
//	PURPOSE:	Update function.
//
// ----------------------------------------------------------------------- //
void GameClientData::Update()
{
	// Update the connection statemachine.
	if( !m_pConnectionStateMachine->Update( ))
	{
		// Error in update, go to the error state.
		SetClientConnectionState( eClientConnectionState_Error );
	}

	// Nothing further to do if SP.
	if( !IsMultiplayerGameServer( ))
		return;

	//update our scoring timers
	m_Score.Update();

	// Nothing further for the host to do.
	if( g_pLTServer->GetClientInfoFlags( GetClient( )) & CIF_LOCAL )
		return;

	// Check if client was previously kicked and is still sticking around.
	// Check if any client has not talked to us in a very long time.  If so, it may
	// mean there is a bug in the networking code that refuses to kick clients not responding.
	// Because this bug is rare, it's difficult to track in the networking.  This is just a bit
	// of fullproofing.  The autoboottime is in seconds, so we need to convert it to ms.
	uint32 nCurrentTime = RealTimeTimer::Instance().GetTimerAccumulatedMS();

	// Get the time since the ping.  Account for wrapping.
	uint32 nDeltaTime = ( nCurrentTime > GetLastClientUpdateTimeMS( )) ? 
		( nCurrentTime - GetLastClientUpdateTimeMS( ) ) : 
		( nCurrentTime + ~( GetLastClientUpdateTimeMS( ) - 1 ));

#ifndef _FINAL
	// check to see if the connection timeout has been disabled for debugging
	if (g_vtServerDisableConnectionTimeout.GetFloat() != 0)
	{
		return;
	}
#endif

	// Check if they haven't talked in a while,
	if( nDeltaTime >= ServerConnectionMgr::Instance().GetAutoBootTimeMS() )
	{
		ServerConnectionMgr::Instance( ).BootWithReason( *this, eClientConnectionError_TimeOut, NULL );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::OnMessage()
//
//	PURPOSE:	Handles messages that pertain to connections.
//
// ----------------------------------------------------------------------- //

bool GameClientData::OnMessage(HCLIENT hSender, ILTMessage_Read& msg)
{
	// Record that the client is talking to us.
	if( IsMultiplayerGameServer( ) && !( g_pLTServer->GetClientInfoFlags( hSender ) & CIF_LOCAL ))
	{
		SetLastClientUpdateTimeMS( RealTimeTimer::Instance().GetTimerAccumulatedMS());
	}

	msg.SeekTo(0);
	uint8 nMsgId = msg.Readuint8();
	if( nMsgId != MID_CLIENTCONNECTION )
		return false;

	// Check if it's just a tickle message.
	uint8 nMsgState = msg.Peekuint8();
	if( nMsgState == eClientConnectionState_Tickle )
		return true;

	CLTMsgRef_Read cSubMsg( msg.SubMsg( msg.Tell( )));
	return m_pConnectionStateMachine->DoMessage( *cSubMsg );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GameClientData::SetNetClientData
//
//  PURPOSE:	Set the client data when client joins.
//
// ----------------------------------------------------------------------- //

bool GameClientData::SetNetClientData( NetClientData const& netClientData )
{
	wchar_t szUniqueName[MAX_PLAYER_NAME];
	ServerConnectionMgr::Instance().GenerateUniqueName( GetClient( ), netClientData.m_szName, szUniqueName, 
		LTARRAYSIZE( szUniqueName ));
	SetUniqueName( szUniqueName );

#if defined(PLATFORM_LINUX)
	
	// initialize data buffer
	uint8 clientDataBuffer[k_nWin32NetClientDataSize];
	::memset(clientDataBuffer, 0, sizeof(clientDataBuffer));
	
	// manually parse player name and build a Win32 (2-byte) wchar_t array
	uint16 szWin32PlayerName[MAX_PLAYER_NAME];
	::memset(szWin32PlayerName, 0, sizeof(szWin32PlayerName));
	for (int nIndex = 0; nIndex < MAX_PLAYER_NAME; ++nIndex)
	{
		uint16* pWin32Char = &szWin32PlayerName[nIndex];
		::memcpy((void*)pWin32Char, (void*)&netClientData.m_szName[nIndex], k_nWin32WcharSize);
	}
	::memcpy(clientDataBuffer, szWin32PlayerName, sizeof(szWin32PlayerName) * sizeof(k_nWin32WcharSize));
	
	// copy player GUID
	LTGUID* pPlayerGuid = (LTGUID*)(clientDataBuffer + k_nPlayerGUIDStartIndex); //32
	::memcpy(clientDataBuffer + k_nPlayerGUIDStartIndex, (void*)&netClientData.m_PlayerGuid, sizeof(LTGUID));
	
	// copy player model index
	*(clientDataBuffer + k_nDMModelStartIndex)   = netClientData.m_nDMModelIndex;
	*(clientDataBuffer + k_nTeamModelStartIndex) = netClientData.m_nTeamModelIndex;
	
	// set the data buffer
	g_pLTServer->SetClientData(GetClient(), clientDataBuffer, k_nWin32NetClientDataSize);
	
#else // PLATFORM_LINUX

	int nNcdSize = sizeof( netClientData );
	g_pLTServer->SetClientData( GetClient( ), ( uint8* )&netClientData, nNcdSize );

#endif // PLATFORM_LINUX
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GameClientData::SetInsignia
//
//  PURPOSE:	Set the insignia.
//
// ----------------------------------------------------------------------- //

void GameClientData::SetInsignia( const char* pszInsignia )
{
	// See if they are allowed presale content, which means we don't have
	// to validate their input.
	bool bIsEmpty = LTStrEmpty( pszInsignia );
	if( GetAllowPreSaleContent( ) && !bIsEmpty )
	{
		m_sInsignia = pszInsignia;
		return;
	}

	char szInsigniaTitle[MAX_PATH] = "";
	LTFileOperations::SplitPath( pszInsignia, NULL, szInsigniaTitle, NULL );

	// They aren't allowed presale content, so validate the input against the presale patches.
	if( !bIsEmpty )
	{
		// Iterate through the presale keys and see if the input is one of them.
		HATTRIBUTE hPreSaleInsignia = g_pModelsDB->GetPreSaleInsigniaAttribute();
		uint32 nNumPreSale = g_pLTDatabase->GetNumValues( hPreSaleInsignia );
		bool bIsPreSalePatch = false;
		for( uint32 nIndex = 0; nIndex < nNumPreSale; nIndex++ )
		{
			char const* pszPreSalePatch = g_pLTDatabase->GetString( hPreSaleInsignia, nIndex, "" );
			if( LTStrEmpty( pszPreSalePatch ))
				continue;

			char szPreSaleTitle[MAX_PATH] = "";
			LTFileOperations::SplitPath( pszPreSalePatch, NULL, szPreSaleTitle, NULL );

			// Compare the paths.  If it matches, then the input is not allowed.
			if( LTStrIEquals( szInsigniaTitle, szPreSaleTitle ))
			{
				bIsPreSalePatch = true;
				break;
			}
		}

		// It's not a presale patch, so it's ok to use.
		if( !bIsPreSalePatch )
		{
			m_sInsignia = pszInsignia;
			return;
		}
	}

	// Just use the default.
	m_sInsignia = g_pLTDatabase->GetString( g_pModelsDB->GetInsigniaAttribute(), 0, "" );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GameClientData::GetNetClientData
//
//  PURPOSE:	Get the client data from server.
//
// ----------------------------------------------------------------------- //

bool GameClientData::GetNetClientData( NetClientData& netClientData ) const
{
	memset( &netClientData, 0, sizeof( netClientData ));

#if defined(PLATFORM_LINUX)

	// initialize data buffer
	uint8 clientDataBuffer[k_nWin32NetClientDataSize];
	::memset(clientDataBuffer, 0, sizeof(clientDataBuffer));
	int nReadSize = sizeof(clientDataBuffer);
	if (!g_pLTServer->GetClientData(GetClient( ), clientDataBuffer, nReadSize))
	{
		return false;
	}

	// manually parse player name and build a wchar_t array
	wchar_t szWidePlayerName[MAX_PLAYER_NAME];
	::memset(szWidePlayerName, 0, sizeof(szWidePlayerName));
	for (int nIndex = 0; nIndex < MAX_PLAYER_NAME; ++nIndex)
	{
		wchar_t* pWideChar = &szWidePlayerName[nIndex];
		::memcpy((void*)pWideChar, (void*)&clientDataBuffer[nIndex*k_nWin32WcharSize], k_nWin32WcharSize);
	}
	::memcpy(netClientData.m_szName, szWidePlayerName, sizeof(szWidePlayerName));

	// copy player GUID
	LTGUID* pPlayerGuid = (LTGUID*)(clientDataBuffer + k_nPlayerGUIDStartIndex); //32
	::memcpy((void*)&netClientData.m_PlayerGuid, pPlayerGuid, sizeof( LTGUID ));

	// copy player model index
	char* pPlayerModel =  (char*)(clientDataBuffer + k_nDMModelStartIndex);
	netClientData.m_nDMModelIndex = (uint8) *pPlayerModel;
	pPlayerModel =  (char*)(clientDataBuffer + k_nTeamModelStartIndex);
	netClientData.m_nTeamModelIndex = (uint8) *pPlayerModel;

#else // PLATFORM_LINUX

	int nNcdSize = sizeof( netClientData );
	if( !g_pLTServer->GetClientData( GetClient( ), ( uint8* )&netClientData, nNcdSize ))
		return false;

#endif // PLATFORM_LINUX

	return true;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::CreateStandbyPlayer
//
//	PURPOSE:	Creates a standby player to switch to later.
//
// ----------------------------------------------------------------------- //
bool GameClientData::CreateStandbyPlayer( )
{
	// This will be filled in with clientdata for mp games.
	NetClientData ncd;
	if( !GetNetClientData( ncd ))
	{
		LTERROR( "Invalid netclientdata." );
		return false;
	}

	CPlayerObj* pPlayer = g_pGameServerShell->CreatePlayer( this );
	if( !pPlayer )
	{
		LTERROR( "Could not create player." );
		return false;
	}

	// Clear the client info on the old player.
	CPlayerObj* pOldPlayer = ( CPlayerObj* )g_pLTServer->HandleToObject( GetPlayer( ));
	if( pOldPlayer )
	{
		// Set the new player to the old player's transform.
		LTRigidTransform rtPlayer;
		g_pLTServer->GetObjectTransform( pOldPlayer->m_hObject, &rtPlayer );
//		g_pLTServer->SetObjectTransform( pPlayer->m_hObject, rtPlayer );
	}

	m_hStandbyPlayer = pPlayer->m_hObject;

	// Not solid.
	g_pCommonLT->SetObjectFlags( m_hStandbyPlayer, OFT_Flags, 0, FLAG_SOLID );

	// Hide the player while they are still joining.
	pPlayer->HideCharacter( true );

	// Teleport to our same position so we remove ourselves from containers.
	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hStandbyPlayer, &vPos );
	g_pLTServer->SetObjectPos( m_hStandbyPlayer, vPos );

	// Need to be invulnerable until we're loaded.
	pPlayer->GetDestructible( )->SetCanDamage( false );

	// transfer the lock spectator mode state to the new player
	pPlayer->SetLockSpectatorMode(pOldPlayer->IsLockSpectatorMode());

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::SwitchToStandbyPlayer
//
//	PURPOSE:	Sets our new player.
//
// ----------------------------------------------------------------------- //
bool GameClientData::SwitchToStandbyPlayer( )
{
	CPlayerObj* pStandbyPlayer = ( CPlayerObj* )g_pLTServer->HandleToObject( m_hStandbyPlayer );
	if( !pStandbyPlayer )
	{
		LTERROR( "Invalid standby player." );
		return false;
	}

	// Clear the client info on the old player.
	CPlayerObj* pOldPlayer = ( CPlayerObj* )g_pLTServer->HandleToObject( GetPlayer( ));
	if( pOldPlayer )
	{
		pOldPlayer->SetClient( NULL );
	}

	// Tell the engine to use this new object as the playerobj.
	g_pLTServer->SetClientObject( GetClient( ), pStandbyPlayer->m_hObject );

	// Remember our client.
	pStandbyPlayer->SetClient(GetClient( ));

	// Reset from client information.
	pStandbyPlayer->ClientInit();

	if( pOldPlayer )
	{
		// Set the new player to the old player's transform.
		LTRigidTransform rtPlayer;
		g_pLTServer->GetObjectTransform( pOldPlayer->m_hObject, &rtPlayer );
		g_pLTServer->SetObjectTransform( pStandbyPlayer->m_hObject, rtPlayer );

		pStandbyPlayer->GetInventory( )->TransferPersistentInventory( pOldPlayer->GetInventory( ));
	}

	// Set our player.
	SetPlayer( m_hStandbyPlayer );
	m_hStandbyPlayer = NULL;

	pStandbyPlayer->SendAllFXToClients( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	GameClientData::AddProximityMine
//
//  PURPOSE:	We set a prox mine and need to keep track of it...
//
// ----------------------------------------------------------------------- //
void GameClientData::AddProximityMine(HOBJECT hProx)
{
	while (m_ProximityMines.size() >= g_pServerDB->GetProximityLimit())
	{
		
		HOBJECT hObj = *(m_ProximityMines.begin());
		if( hObj )
		{
			g_pLTServer->RemoveObject(hObj);	
			m_ProximityMines.erase(m_ProximityMines.begin());
		}

	}

	LTObjRefNotifier ref( *this );
	ref = hProx;
	m_ProximityMines.push_back(ref);


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::OnLinkBroken
//
//	PURPOSE:	Remove from the prox list
//
// ----------------------------------------------------------------------- //

void GameClientData::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	// Remove remote charge attachments...
	ObjRefNotifierList::iterator iter = m_ProximityMines.begin();
	while( iter != m_ProximityMines.end( )) 
	{
		if( &( *iter ) == pRef )
		{
			m_ProximityMines.erase( iter );
			break;
		}
		++iter;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::SetMPModelIndex
//
//	PURPOSE:	Sets the model index for mp games.
//
// ----------------------------------------------------------------------- //
void GameClientData::SetMPModelIndex( uint8 nMPModelIndex )
{
	// Check if they are not allowed presale content.  We may need
	// to restrict their choice of model.
	if( !GetAllowPreSaleContent( ))
	{
		ModelsDB::HMODEL hModel = NULL;
		if( GameModeMgr::Instance().m_grbUseTeams )
		{
			hModel = g_pModelsDB->GetFriendlyTeamModel( nMPModelIndex );
		}
		else
		{
			hModel = g_pModelsDB->GetDMModel( nMPModelIndex );
		}

		// See if this model is tagged as presale.
		if( g_pModelsDB->GetModelPreSale( hModel ))
		{
			// Just use the first model.
			nMPModelIndex = 0;
		}
	}

	NetClientData ncd;
	GetNetClientData( ncd );
	if( GameModeMgr::Instance().m_grbUseTeams )
	{
		ncd.m_nTeamModelIndex = nMPModelIndex;
	}
	else
	{
		ncd.m_nDMModelIndex = nMPModelIndex;
	}
	SetNetClientData( ncd );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::GetModelIndex
//
//	PURPOSE:	Gets the model index for mp games.
//
// ----------------------------------------------------------------------- //
uint8 GameClientData::GetMPModelIndex( ) const
{
	NetClientData ncd;
	GetNetClientData( ncd );
	if( GameModeMgr::Instance().m_grbUseTeams )
	{
		return ncd.m_nTeamModelIndex;
	}
	else
	{
		return ncd.m_nDMModelIndex;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::GetModel
//
//	PURPOSE:	Gets the model for mp games.
//
// ----------------------------------------------------------------------- //
ModelsDB::HMODEL GameClientData::GetMPModel( ) const
{
	if( !IsMultiplayerGameServer( ))
		return NULL;

	if( GameModeMgr::Instance().m_grbUseTeams )
	{
		return g_pModelsDB->GetFriendlyTeamModel( GetMPModelIndex( ));
	}
	else
	{
		return g_pModelsDB->GetDMModel( GetMPModelIndex( ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GameClientData::IsReady
//
//	PURPOSE:	Accessor to whether client is ready to play or not.
//
// ----------------------------------------------------------------------- //
bool GameClientData::IsReady( ) const
{
	if( !GetPlayer( ) || !IsClientInWorld( ))
		return false;

	if( IsMultiplayerGameServer( ))
	{
		// Make sure they have picked their loadout.
		if( GameModeMgr::Instance( ).m_grbUseLoadout && GetLoadout() == (uint8) -1 )
			return false;

		// Make sure they have picked a team.
		if( GameModeMgr::Instance().m_grbUseTeams )
		{
			if( GetLastTeamId( ) == INVALID_TEAM && GetRequestedTeam() == INVALID_TEAM )
				return false;

			return true;
		}
		else
		{
			return true;
		}
	}
	else
	{
		// SP player is ready as soon as he's in the world.
		return true;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::ServerConnectionMgr()
//
//	PURPOSE:	ctor
//
// ----------------------------------------------------------------------- //
ServerConnectionMgr::ServerConnectionMgr( )
{
	VarTrack cvfAutoBootTime;
	cvfAutoBootTime.Init( g_pLTBase, "AutoBootTime", NULL, 120.0f );
	m_nAutoBootTimeMS = ( uint32 )( cvfAutoBootTime.GetFloat( ) * 1000.0f );

#ifndef _FINAL
	SERVER_CODE
	(
		// initialize the disable connection timeout variable
		if (g_pLTServer && !g_vtServerDisableConnectionTimeout.IsInitted() && (GetCurExecutionShellContext() == eExecutionShellContext_Server)) 
		{
			g_vtServerDisableConnectionTimeout.Init(g_pLTServer, "DisableConnectionTimeout", NULL, 0);
		}

		// initialize the disable CRC check variable
		if (g_pLTServer && !g_vtServerDisableCRCCheck.IsInitted() && (GetCurExecutionShellContext() == eExecutionShellContext_Server)) 
		{
			g_vtServerDisableCRCCheck.Init(g_pLTServer, "DisableCRCCheck", NULL, 0);
		}

		// initialize the disable CRC check variable
		if (g_pLTServer && !g_vtServerDisableCDKeyCheck.IsInitted() && (GetCurExecutionShellContext() == eExecutionShellContext_Server)) 
		{
			g_vtServerDisableCDKeyCheck.Init(g_pLTServer, "DisableCDKeyCheck", NULL, 0);
		}
	)
#endif // !_FINAL

	// initialize our slot-based client array based on the maximum number of players
	m_nMaxPlayers = GameModeMgr::Instance().m_grnMaxPlayers;

	LT_MEM_TRACK_ALLOC(m_ppGameClientDataSlotArray = new GameClientData*[m_nMaxPlayers], LT_MEM_TYPE_GAMECODE);
	memset(m_ppGameClientDataSlotArray, 0, sizeof(GameClientData*) * m_nMaxPlayers);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::ServerConnectionMgr()
//
//	PURPOSE:	dtor
//
// ----------------------------------------------------------------------- //
ServerConnectionMgr::~ServerConnectionMgr( )
{
	delete [] m_ppGameClientDataSlotArray;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::OnAddClient()
//
//	PURPOSE:	Called when a new client connection is added.
//
// ----------------------------------------------------------------------- //
void ServerConnectionMgr::OnAddClient( HCLIENT hClient )
{
	// Add this client to our list of clients.
	GameClientData* pGameClientData = NULL;
	LT_MEM_TRACK_ALLOC(pGameClientData = new GameClientData(hClient), LT_MEM_TYPE_GAMECODE);
	if( !pGameClientData->Init())
	{
		LTERROR( "Could not intialize client data." );
		delete pGameClientData;
		return;
	}
	m_GameClientDataList.push_back(pGameClientData);

	// add the pointer to our slot based list for fast lookups by client ID
	uint32 nSlotID = g_pLTServer->GetClientID(hClient);
	LTASSERT(m_ppGameClientDataSlotArray[nSlotID] == NULL, "Attempt to add new client to a non-null slot");

	m_ppGameClientDataSlotArray[nSlotID] = pGameClientData;

	// Check if the client IP is banned.
	if( !BanIPMgr::Instance( ).OnAddClient( hClient ))
	{
		ServerConnectionMgr::Instance( ).BootWithReason( *pGameClientData, eClientConnectionError_Banned, NULL );
		return;
	}

	// Tell serverapp about it.
	g_pGameServerShell->ServerAppAddClient( hClient );

	// This will be filled in with clientdata for mp games.
	NetClientData ncd;
	memset( &ncd, 0, sizeof( ncd ));

	char szClientName[64];

	// Must have client data for mp game.
	if( IsMultiplayerGameServer( ))
	{
		// Make sure the NetClientData is valid.
		if( !pGameClientData->GetNetClientData( ncd ))
		{
			pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
			return;
		}

		LTGUIDToString( szClientName, LTARRAYSIZE( szClientName ), ncd.m_PlayerGuid );

		// Check if the model is valid.
		ModelsDB::HMODEL hModel = NULL;
		if( GameModeMgr::Instance().m_grbUseTeams )
		{
			hModel = g_pModelsDB->GetDMModel( ncd.m_nDMModelIndex );
		}
		else
		{
			hModel = g_pModelsDB->GetFriendlyTeamModel( ncd.m_nTeamModelIndex );
		}
		if( !hModel )
		{
			pGameClientData->SetClientConnectionState( eClientConnectionState_Error );
			return;
		}

		// Give us a unique net name on the server.
		wchar_t szUniqueName[MAX_PLAYER_NAME];
		GenerateUniqueName( hClient, ncd.m_szName, szUniqueName, LTARRAYSIZE( szUniqueName ));
		pGameClientData->SetUniqueName( szUniqueName );

		// write a scoring entry if necessary
		if (GameModeMgr::Instance().m_ServerSettings.m_bEnableScoringLog)
		{	
			char szEntry[256];
			LTSNPrintF(szEntry, LTARRAYSIZE(szEntry), "Client connected: %s", MPW2A(ncd.m_szName).c_str());

			g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntry);
		}

		// the local client is always logged into scmd
		if (g_pLTServer->GetClientInfoFlags(hClient) & CIF_LOCAL)
		{
			// logout any current scmd user
			HCLIENT hCurrentAdmin = ScmdServer::Instance().GetAdminClient();
			if (hCurrentAdmin)
			{
				ScmdServer::Instance().ForceAdminClientLogout();
			}

			// log this client into scmd
			ScmdServer::Instance().SetAdminClient(hClient);
		}
	}
	else
	{
		LTStrCpy( szClientName, "singleplayer", LTARRAYSIZE( szClientName ));
	}

	// Set our clientname from the netclientdata.
	g_pLTServer->SetClientName( hClient, szClientName, LTStrLen( szClientName ) + 1 );

	// If this is a remote client of multiplayer, then we need to do some handshaking to let them in.
	if( IsMultiplayerGameServer( ))
	{
		// use LTSNPrintF to form the string in order to protect against embedded formatting
		char szMsg[256] = {0};
		LTSNPrintF(szMsg, LTARRAYSIZE(szMsg), "Player '%ls' connected.", pGameClientData->GetUniqueName());
		g_pLTServer->CPrintNoArgs(szMsg);

		pGameClientData->SetClientConnectionState( eClientConnectionState_Hello );
	}
	else
	{
		pGameClientData->SetClientConnectionState( eClientConnectionState_LoggedIn );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::OnRemoveClient()
//
//	PURPOSE:	Called when a client disconnects from a server
//
// ----------------------------------------------------------------------- //

void ServerConnectionMgr::OnRemoveClient(HCLIENT hClient)
{
	if (IsMultiplayerGameServer( ))
	{
		// abort content transfer with this client if necessary
		g_pLTServerContentTransfer->AbortTransfer(hClient);

		// Send a message to all clients, letting them know this user is leaving the world
		uint32 nClientID = g_pLTServer->GetClientID(hClient);
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_REMOVED);
		cMsg.Writeuint32(nClientID);
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

		if (GameModeMgr::Instance( ).m_grbUseTeams)
		{
			CTeamMgr::Instance().RemovePlayer( nClientID );
		}

		GameClientData* pGameClientData = GetGameClientData(hClient);
		
		if (pGameClientData)
		{
			// write a scoring entry if necessary
			if (GameModeMgr::Instance().m_ServerSettings.m_bEnableScoringLog)
			{	
				NetClientData ncd;
				pGameClientData->GetNetClientData(ncd);

				char szEntry[256];
				LTSNPrintF(szEntry, LTARRAYSIZE(szEntry), "Client disconnected: %s", MPW2A(ncd.m_szName).c_str());

				g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntry);
			}

			// use LTSNPrintF to form the string in order to protect against embedded formatting
			char szMsg[256] = {0};
			LTSNPrintF(szMsg, LTARRAYSIZE(szMsg), "Player '%ls' disconnected.", pGameClientData->GetUniqueName());
			g_pLTServer->CPrintNoArgs(szMsg);
		}
	}

	// Clear our client data...
	CPlayerObj* pPlayer = GetPlayerFromHClient(hClient);
	if (pPlayer)
	{
		pPlayer->SetClient(NULL);
	}

	// Tell server app about it.
	g_pGameServerShell->ServerAppRemoveClient( hClient );

	// Tell the scmd about it.
	ScmdServer::Instance( ).OnRemoveClient( hClient );

	// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// Tell gamespy we don't need the cdkey anymore.
	if( g_pGameServerShell->GetGameSpyServer( ))
	{
		uint32 nClientId = g_pLTServer->GetClientID( hClient );
		g_pGameServerShell->GetGameSpyServer( )->ReleaseCDKey( nClientId );
	}

#endif // !PLATFORM_XENON

	GameClientDataList::iterator iter = GetGameClientDataIter( hClient );
	if( iter != m_GameClientDataList.end( ))
	{
		GameClientData* pGameClientData = *iter;
		m_GameClientDataList.erase( iter );
		delete pGameClientData;
	}

	// remove the pointer from our slot-based array
	uint32 nSlotID = g_pLTServer->GetClientID(hClient);
	LTASSERT(m_ppGameClientDataSlotArray[nSlotID] != NULL, "Attempt to remove null entry from slot array");
	m_ppGameClientDataSlotArray[nSlotID] = NULL;

	// See if the disconnect caused the end of the round.
	g_pServerMissionMgr->CheckEliminationWin();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::Update
//
//	PURPOSE:	Frame update.
//
// ----------------------------------------------------------------------- //
void ServerConnectionMgr::Update()
{
	// Update all the client connections.
	GameClientDataList::iterator iter = m_GameClientDataList.begin( );
	for( ; iter != m_GameClientDataList.end( ); iter++ )
	{
		GameClientData* pGameClientData = *iter;
		pGameClientData->Update( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::GetClientDataIter
//
//	PURPOSE:	Gets the gameclientdata iterator given a HCLIENT.
//
// ----------------------------------------------------------------------- //

ServerConnectionMgr::GameClientDataList::iterator ServerConnectionMgr::GetGameClientDataIter( HCLIENT hClient )
{
	// Find the client in our list.
	GameClientDataList::iterator iter = m_GameClientDataList.begin( );
	for( ; iter != m_GameClientDataList.end( ); iter++ )
	{
		GameClientData* pGameClientData = *iter;
		if( pGameClientData->GetClient( ) == hClient )
		{
			break;
		}
	}

	return iter;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::GetClientData
//
//	PURPOSE:	Gets the clientdata given a HCLIENT.
//
// ----------------------------------------------------------------------- //

GameClientData* ServerConnectionMgr::GetGameClientData( HCLIENT hClient )
{
	// Find the client in our list.
	GameClientDataList::iterator iter = GetGameClientDataIter( hClient );
	if( iter == m_GameClientDataList.end( ))
		return NULL;

	return *iter;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::GetClientDataByClientId
//
//	PURPOSE:	Gets the clientdata given a client Id
//
// ----------------------------------------------------------------------- //

GameClientData* ServerConnectionMgr::GetGameClientDataByClientId( uint32 nClientId )
{
	// return the GameClientData pointer at the specified slot in the array
	// (NULL indicates there is no player currently in this slot)
	return m_ppGameClientDataSlotArray[nClientId];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::OnMessage()
//
//	PURPOSE:	Handles messages that pertain to connections.
//
// ----------------------------------------------------------------------- //

bool ServerConnectionMgr::OnMessage(HCLIENT hSender, ILTMessage_Read& msg)
{
	// Hand message off to our gameclientdata.
	GameClientData* pGameClientData = GetGameClientData( hSender );
	if( !pGameClientData )
		return false;

	return pGameClientData->OnMessage( hSender, msg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::PostStartWorld()
//
//	PURPOSE:	Called after level has switched and clients are waiting to enter.
//
// ----------------------------------------------------------------------- //

bool ServerConnectionMgr::PostStartWorld()
{
	// Sets all clients to the postloadworld state.  This has to happen in the postload
	// since we need to make sure clients are completely out of the previous level.
	// If a client hasn't finished the security checks, then let them
	// finish that.
	GameClientDataList::iterator iter = m_GameClientDataList.begin( );
	for( ; iter != m_GameClientDataList.end( ); iter++ )
	{
		GameClientData* pGameClientData = *iter;
		if( pGameClientData->HasPassedSecurity( ))
			pGameClientData->SetClientConnectionState( eClientConnectionState_PostLoadWorld );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ServerConnectionMgr::IsNameTaken
//
//  PURPOSE:	Checks if any client has the given name.
//
// ----------------------------------------------------------------------- //
bool ServerConnectionMgr::IsNameTaken( wchar_t const* pszName, HCLIENT hSkipClient ) const
{
	// Remove this client from our list of clients.
	GameClientDataList::const_iterator iter = m_GameClientDataList.begin( );
	for( ; iter != m_GameClientDataList.end( ); iter++ )
	{
		GameClientData* const pGameClientData = *iter;

		if( pGameClientData->GetClient( ) == hSkipClient )
			continue;

		// Check if our name is the same as this client.
		if( LTStrEquals( pszName, pGameClientData->GetUniqueName( )))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::GenerateUniqueName
//
//  PURPOSE:	Generates a unqiue name for all players given a base name.
//
// ----------------------------------------------------------------------- //
void ServerConnectionMgr::GenerateUniqueName( HCLIENT hClient, wchar_t const* pszNameBase, wchar_t* pszUniqueName, int nUniqueNameSize ) const
{
	// We are intentionally "fixing" the player name so that if for some reason the client
	// does send a bad name up to us, we will still generate a decent server-side name.
	wchar_t szUniqueName[256];
	LTStrCpy( szUniqueName, FixPlayerName(pszNameBase), LTARRAYSIZE( szUniqueName ));

	// If we end up with an empty name set us to a nice default name.
	if( szUniqueName[0] == '\0' )
	{
		const wchar_t* const szDefaultPlayerName = LoadString("IDS_PLAYER_NAME");

		LTStrCpy( szUniqueName, szDefaultPlayerName, LTARRAYSIZE( szUniqueName ));
	}

	// If the base name is taken, then generate a new name with a number on the 
	// end of the base name.
	if( IsNameTaken( szUniqueName, hClient ))
	{
		int nCount = 0;
		for(;;)
		{
			LTSNPrintF( szUniqueName, LTARRAYSIZE( szUniqueName ), L"%ls%d", pszNameBase, nCount );
			if( !IsNameTaken( szUniqueName, hClient ))
				break;

			nCount++;
		}
	}

	// Copy the string.
	LTStrCpy( pszUniqueName, szUniqueName, nUniqueNameSize );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::ClientConnectionInTrouble
//
//	PURPOSE:	Checks if client connection is in trouble.
//
// --------------------------------------------------------------------------- //
bool ServerConnectionMgr::ClientConnectionInTrouble( HCLIENT hClient )
{
	// Check inputs.
	if( hClient == NULL )
	{
		LTERROR( "Invalid inputs." );
		return false;
	}

#ifndef _FINAL
	// check the disable connection timeout debugging variable
	if (g_vtServerDisableConnectionTimeout.GetFloat() != 0)
	{
		// do not kick the client
		return false;
	}
#endif


	// If the player's ping is larger than the max allowed, drop him.
	float fClientPing = 0.0f;
	if( g_pLTServer->GetClientPing( hClient, fClientPing ) != LT_OK )
	{
		LTERROR( "Bad client handle." );
		return false;
	}

	// Check if client is under limit.
	float fMaxClientPing = GetConsoleFloat( "MaxClientPing", 30000.0f );
	if( fClientPing  < fMaxClientPing )
		return false;

	// Client connection is in trouble.
	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ServerConnectionMgr::BootWithReason
//
//	PURPOSE:	Boots the client and sends the reason to them.
//
// --------------------------------------------------------------------------- //
bool ServerConnectionMgr::BootWithReason( GameClientData& gameClientData, EClientConnectionError eConnectionError, const char* pszMessage)
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8((uint8)eClientConnectionState_Error);
	cMsg.Writeuint8((uint8)eConnectionError);
	cMsg.Writebool(pszMessage ? true : false);
	if (pszMessage)
	{
		cMsg.WriteString(pszMessage);
	}
	g_pLTServer->SendToClient(cMsg.Read(), gameClientData.GetClient( ), MESSAGE_GUARANTEED);
	gameClientData.SetClientConnectionState( eClientConnectionState_Error );

	return true;
}

