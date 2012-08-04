// ----------------------------------------------------------------------- //
//
// MODULE  : ClientConnectionMgr.cpp
//
// PURPOSE : Clientside connection mgr - Definition
//
// CREATED : 02/05/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientConnectionMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "CharacterFx.h"
#include "CMoveMgr.h"
#include "MissionMgr.h"
#include "MissionDB.h"
#include "InterfaceMgr.h"
#include "ProfileMgr.h"
#include "crc32utils.h"
#include "BroadcastDB.h"
#if !defined(PLATFORM_XENON)
#include "IGameSpy.h"
#endif
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"
#include "MissionDB.h"
#include "ltfileoperations.h"
#include "CharacterFX.h"
#include "iltfilemgr.h"
#include "TargetMgr.h"
#include "PlayerCamera.h"

// the ILTClientContentTransfer interface
#include "iltclientcontenttransfer.h"
static ILTClientContentTransfer* g_pLTClientContentTransfer;
define_holder(ILTClientContentTransfer, g_pLTClientContentTransfer);

#if !defined(PLATFORM_XENON)
#include "ILTGameUtil.h"
static ILTGameUtil *g_pLTGameUtil;
define_holder(ILTGameUtil, g_pLTGameUtil);
#endif // !defined(PLATFORM_XENON)

ClientConnectionMgr* g_pClientConnectionMgr = NULL;

#if !defined(PLATFORM_XENON)

inline unsigned char INADDR_B1(const sockaddr_in& addr)
{ return addr.sin_addr.S_un.S_un_b.s_b1; }
inline unsigned char INADDR_B2(const sockaddr_in& addr)
{ return addr.sin_addr.S_un.S_un_b.s_b2; }
inline unsigned char INADDR_B3(const sockaddr_in& addr)
{ return addr.sin_addr.S_un.S_un_b.s_b3; }
inline unsigned char INADDR_B4(const sockaddr_in& addr)
{ return addr.sin_addr.S_un.S_un_b.s_b4; }
#define EXPAND_BASEADDR(addr)\
              INADDR_B1(addr), INADDR_B2(addr), INADDR_B3(addr), INADDR_B4(addr)

#define EXPAND_ADDR(addr) \
	EXPAND_BASEADDR(addr),\
	ntohs((addr).sin_port)

static const char* ADDR_PRINTF = "%d.%d.%d.%d:%d";

#endif // !PLATFORM_XENON

static const char* DOWNLOADED_CONTENT_DIRECTORY = FILE_PATH_SEPARATOR "Custom" FILE_PATH_SEPARATOR "DownloadedContent";



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::ClientConnectionMgr
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

ClientConnectionMgr::ClientConnectionMgr( )
{
	m_nServerPort = -1;
	m_nServerKey = 0;
	m_nDisconnectCode = 0;
	m_bForceDisconnect = false;

	m_StartGameRequest.m_Type = GAMEMODE_NONE;
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	m_nLastConnectionResult = LT_OK;

	g_pClientConnectionMgr = this;

	m_nTeam = INVALID_TEAM;
	m_bHasSelectedTeam = false;

	m_nCurrentRound = 0;

	m_nLoadout = -1;
	m_bHasSelectedLoadout = false;

	m_pGameSpyBrowser	= NULL;
	m_bDoNatNegotiations = false;
	m_bConnectViaPublic = true;

	m_SettleCommTimer.SetEngineTimer( RealTimeTimer::Instance( ));

	m_eRecvClientConnectionState = eClientConnectionState_None;
	m_eSentClientConnectionState = eClientConnectionState_None;
	m_bClientLoggedIn = false;


	m_eSocketConnectionState = eSocketConnectionState_Disconnected;

	// build and set content transfer options
	SetContentTransferOptions();

	// database handles
	m_hGameDatabase		 = NULL;
	m_hOverridesDatabase = NULL;

	m_GameTickleTimer.SetEngineTimer( RealTimeTimer::Instance( ));

	m_bClientLAN = false;

	m_bClientContentTransferInProgress = false;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::~ClientConnectionMgr
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

ClientConnectionMgr::~ClientConnectionMgr( )
{
	TermBrowser( );

	g_pClientConnectionMgr = NULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::SendMultiPlayerInfo
//
//	PURPOSE:	Send the server the updated multiplayer info
//
// --------------------------------------------------------------------------- //

bool ClientConnectionMgr::SendMultiPlayerInfo()
{
	if (!IsMultiplayerGameClient())
		return false;

	if( !SetNetClientData( ))
		return false;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_INFOCHANGE );
	cMsg.WriteWString(pProfile->m_sPlayerName.c_str());
	cMsg.Writeuint8( GameModeMgr::Instance().m_grbUseTeams ? m_NetClientData.m_nTeamModelIndex : m_NetClientData.m_nDMModelIndex );
	cMsg.Writeuint8( m_nTeam );
	cMsg.Writeuint8( m_nLoadout );
	// Send patch as file title only to reduce bandwidth.
	char szPatchTitle[MAX_PATH] = "";
	LTFileOperations::SplitPath( pProfile->m_sPlayerPatch.c_str( ), NULL, szPatchTitle, NULL );
	cMsg.WriteString(szPatchTitle);

    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	return true;
}

// returns true if the passed in address matches the current server address
bool ClientConnectionMgr::CheckServerAddress(char const*pszTestAddress, int nPort)
{
	if (!pszTestAddress) return false;
	if (nPort != m_nServerPort) return false;
	return (LTStrICmp(pszTestAddress,m_sServerAddress.c_str()) == 0);
}


void ClientConnectionMgr::DoBroadcast(uint32 nClientID, const char* szBroadcastID)
{
	if (!g_pPlayerMgr->IsPlayerAlive())
		return;

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;
	if (!szBroadcastID) return;
	HRECORD hRec = DATABASE_CATEGORY( Broadcast ).GetRecordByName( szBroadcastID);
	if (!hRec) return;

	// Don't allow the client to flood the server with broadcasts...

	CCharacterFX *pFX = g_pGameClientShell->GetLocalCharacterFX();
	if (pFX && !pFX->IsPlayingBroadcast())
	{
		uint32 nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hRec );
		int8 nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);
		pFX->PlayBroadcast(nBroadcastID, true, nClientID, nPriority, true);

		bool bSendLocation = false;
		LTVector vPos;
		if (DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, NavMarker ) &&
			DATABASE_CATEGORY( Broadcast ).GetPlacement(hRec) == kNavMarkerProjected
			)
		{
			float fRng = g_pPlayerMgr->GetTargetMgr()->GetTargetRange();
			if (fRng < kMaxDistance)
			{
				bSendLocation = true;
				vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();
				vPos += g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation().Forward() * fRng;
			}
		}

		// create the message
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_BROADCAST);
		cMsg.WriteDatabaseRecord( g_pLTDatabase, hRec );
		cMsg.Writeuint32( nBroadcastID  );
		cMsg.Writebool(bSendLocation);
		if (bSendLocation)
		{
			cMsg.WriteCompLTVector(vPos);
		}
	    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}



bool ClientConnectionMgr::SetupClient(wchar_t const* pszHostName, wchar_t const* pszPassword,
							bool bDoNatNegotiations, bool bConnectViaPublic, 
							char const* pszPublicAddress, char const* pszPrivateAddress )
{
	// Check inputs.
	if( !pszPublicAddress || !pszPublicAddress[0] || !pszPrivateAddress || !pszPrivateAddress[0] )
	{
		LTERROR( "Invalid inputs." );
		return false;
	}

	m_sServerName = pszHostName ? pszHostName : L"";
	m_sConnectPublicAddress = pszPublicAddress;
	m_sConnectPrivateAddress = pszPrivateAddress;
	m_bConnectViaPublic = bConnectViaPublic;

	// Setup the startgame info.
	StartGameRequest startGameRequest;
	m_StartGameRequest = startGameRequest;
	m_StartGameRequest.m_Type = STARTGAME_CLIENT;
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	// Record the default address to try.
	LTStrCpy( m_StartGameRequest.m_TCPAddress, ( m_bConnectViaPublic ? m_sConnectPublicAddress.c_str( ) : 
		m_sConnectPrivateAddress.c_str( )), LTARRAYSIZE( m_StartGameRequest.m_TCPAddress ));

	// Assume we'll be joining an internet game.
	SetupClientBandwidth( false );

	if (pszPassword)
	{
		m_nClientPass = str_Hash(pszPassword);
	}
	else
	{
		m_nClientPass = 0;
	}

	// Check if they want us to do natnegotiations.
	m_bDoNatNegotiations = bDoNatNegotiations;

	// Setup the remaining data.  This can depend on the setup above.
	if( !SetNetClientData( ))
		return false;

	g_pInterfaceMgr->GetClientInfoMgr()->SetupMultiplayer();

	// Force reselect of loadout.
	m_bHasSelectedLoadout = false;
	m_nLoadout = -1;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::SetupServerSinglePlayer
//
//	PURPOSE:	Setup server for singleplayer game.
//
// ----------------------------------------------------------------------- //
bool ClientConnectionMgr::SetupServerSinglePlayer( )
{
	StartGameRequest startGameRequest;
	m_StartGameRequest = startGameRequest;
	// Set the game type to normal (single player).
	m_StartGameRequest.m_Type = STARTGAME_NORMAL;
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	m_NetGameInfo.m_sGameMode = GameModeMgr::GetSinglePlayerRecordName( );
	m_NetGameInfo.m_sProfileName = pProfile->m_sFileName;
	m_NetGameInfo.m_sServerOptionsFile = "";
	m_NetGameInfo.m_bPerformanceTest = g_pGameClientShell->IsRunningPerformanceTest();
	m_NetGameInfo.m_sMissionOrder = GetConsoleString( "MissionOrder", MDB_MissionOrderDefault );

	// Setup the remaining data.  This can depend on the setup above.
	if( !SetNetClientData( ))
		return false;

	g_pInterfaceMgr->GetClientInfoMgr()->ClearMultiplayer();

	return true;
}
// --------------------------------------------------------------------------- //
//
//  ROUTINE:    ClientConnectionMgr::SetupServerHost()
//
//  PURPOSE:    Host a game.
//
//	PARAMETERS:	int nPort - Port to use, 0 to use default
//
// --------------------------------------------------------------------------- //

bool ClientConnectionMgr::SetupServerHost( )
{
	StartGameRequest startGameRequest;
	m_StartGameRequest = startGameRequest;
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	m_StartGameRequest.m_Type = STARTGAME_HOST;
	m_StartGameRequest.m_HostInfo.m_Port = GameModeMgr::Instance( ).m_ServerSettings.m_nPort;
	LTStrCpy(m_StartGameRequest.m_HostInfo.m_sBindToAddr, GameModeMgr::Instance( ).m_ServerSettings.m_sBindToAddr.c_str(), LTARRAYSIZE(m_StartGameRequest.m_HostInfo.m_sBindToAddr));

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	m_NetGameInfo.m_sGameMode = GameModeMgr::Instance( ).m_ServerSettings.m_sGameMode;
	m_NetGameInfo.m_sProfileName = pProfile->m_sFileName;
	m_NetGameInfo.m_sServerOptionsFile = pProfile->m_sServerOptionsFile;
	m_NetGameInfo.m_bPerformanceTest = false;
	m_NetGameInfo.m_sModName = GetModName( );

	// Setup the remaining data.  This can depend on the setup above.
	if( !SetNetClientData( ))
		return false;

	const wchar_t* pTempName = L"";
	m_StartGameRequest.m_HostInfo.m_dwMaxConnections = GameModeMgr::Instance( ).m_grnMaxPlayers - 1;

	if (!GameModeMgr::Instance( ).m_ServerSettings.m_bLANOnly)
	{
		m_StartGameRequest.m_HostInfo.m_dwMaxConnections = 
			LTMIN(m_StartGameRequest.m_HostInfo.m_dwMaxConnections,
			(uint32)GameModeMgr::Instance( ).m_ServerSettings.GetMaxPlayersForBandwidth());
	}

	g_pInterfaceMgr->GetClientInfoMgr()->SetupMultiplayer();

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::SetDisconnectCode
//
//	PURPOSE:	Sets the disconnection code and message
//
// --------------------------------------------------------------------------- //

void ClientConnectionMgr::SetDisconnectCode(uint32 nCode, const char *pMsg)
{
	// Don't override what someone already told us
	if (m_nDisconnectCode)
		return;

	m_nDisconnectCode = nCode;

	if( pMsg )
		m_sDisconnectMsg = pMsg;
	else
		m_sDisconnectMsg.clear( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::ClearDisconnectCode
//
//	PURPOSE:	Clears the disconnection code and message
//
// --------------------------------------------------------------------------- //

void ClientConnectionMgr::ClearDisconnectCode()
{
	m_nDisconnectCode = 0;
	m_sDisconnectMsg.clear( );
	m_StartGameRequest.m_Type = GAMEMODE_NONE;

	m_strContentTransferErrorFilename.clear();
	m_wstrContentTransferErrorServerMessage.clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

bool ClientConnectionMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// Check inputs.
	if( !pMsg )
	{
		ASSERT( !"ClientConnectionMgr::OnMessage: Invalid msg." );
		return false;
	}

	switch(messageID)
	{
		case MID_CLIENTCONNECTION:
			return HandleMsgClientConnection( *pMsg );
			break;
		case MID_MULTIPLAYER_DATA:
			return HandleMsgMultiplayerData( *pMsg );
			break;
		default:
			break;
		
	}

	return false;
}


bool ClientConnectionMgr::HandleMsgClientConnection( ILTMessage_Read & msg )
{
	m_eRecvClientConnectionState = ( EClientConnectionState )msg.Readuint8();

	// Clear out the sent state since we haven't responded yet.
	m_eSentClientConnectionState = eClientConnectionState_None;

	switch( m_eRecvClientConnectionState )
	{
		case eClientConnectionState_Hello :
		{
			m_bClientLoggedIn = false;
			uint16 nHandshakeVer = g_pLTGameUtil->ReadHandshakingVersion( msg, GAME_HANDSHAKE_VER_MASK );
			if (nHandshakeVer != GAME_HANDSHAKE_VER)
			{
				// Disconnect
				m_bForceDisconnect = true;
				SetDisconnectCode(eDisconnect_NotSameGUID,NULL);
				return true;
			}

			// Read in the game mode and set up for the default settings.
			HRECORD hGameModeRecord = msg.ReadDatabaseRecord( g_pLTDatabase, DATABASE_CATEGORY( GameModes ).GetCategory( ));
			if( !GameModeMgr::Instance( ).ResetToMode( hGameModeRecord ) || !GameModeMgr::Instance().ReadFromMsg( msg ))
			{
				// Disconnect
				m_bForceDisconnect = true;
				SetDisconnectCode(eDisconnect_NotSameGUID,NULL);
				return true;
			}

			char szWorldName[MAX_PATH*2];
			msg.ReadString( szWorldName, LTARRAYSIZE( szWorldName ));
			g_pMissionMgr->ClientHandshaking( szWorldName );

			// Send back a hello response
			CAutoMessage cResponse;
			cResponse.Writeuint8(MID_CLIENTCONNECTION);
			cResponse.Writeuint8(eClientConnectionState_Hello);
			g_pLTGameUtil->WriteHandshakingVersion( cResponse, GAME_HANDSHAKE_VER, GAME_HANDSHAKE_VER_MASK );
			// Send them our secret key
			g_pLTGameUtil->WriteClientKey1( cResponse, ( uint32 )this );
			g_pLTClient->SendToServer(cResponse.Read(), MESSAGE_GUARANTEED);

			m_eSentClientConnectionState = eClientConnectionState_Hello;
		}
		break;
		case eClientConnectionState_KeyChallenge:
		{
			// Read in their encrypted key challenge
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			m_nServerKey = g_pLTGameUtil->ReadServerKey( msg, ( uint32 )this, pProfile->m_PlayerGuid );
			bool   bNeedPassword = msg.Readbool();

			// Game password.
			uint32 nHashedPassword = 0;
			if (bNeedPassword)
			{
				nHashedPassword = m_nClientPass;
			}

			// Check if we need to send the cdkey.
			bool bNeedsCDKey = msg.Readbool( );

			CAutoMessage cResponse;
			cResponse.Writeuint8(MID_CLIENTCONNECTION);
			cResponse.Writeuint8(eClientConnectionState_KeyChallenge);
			g_pLTGameUtil->WriteServerKeyData( cResponse, pProfile->m_PlayerGuid, m_nServerKey, m_nServerKey );
			cResponse.Writeuint32(nHashedPassword);

			// write PunkBuster state
			g_pLTGameUtil->WritePunkBusterState( cResponse, pProfile->m_PlayerGuid, m_nServerKey, g_pGameClientShell->GetPunkBusterClient());

#if !defined(PLATFORM_XENON)
			if( bNeedsCDKey )
			{
				char szChallenge[256] = "";
				char szChallengeResponse[256] = "";

				// Read the challenge from the server and reflect it back.
				msg.ReadString( szChallenge, LTARRAYSIZE( szChallenge ));
				cResponse.WriteString( szChallenge );

				char szCDKey[256] = "";
				uint32 nChallengeResponseLen = LTARRAYSIZE( szChallengeResponse );

#if defined(ENABLE_CDKEY_1_CHECK)
				// Get the main cdkey and write the challenge response out.
				g_pVersionMgr->GetCDKey( szCDKey, LTARRAYSIZE( szCDKey ));
				g_pLTGameUtil->CDKeyComputeChallengeResponse( szCDKey, szChallenge, szChallengeResponse, nChallengeResponseLen );
				cResponse.WriteString( szChallengeResponse );
#endif

#if defined(ENABLE_CDKEY_2_CHECK)
				// Now check if they have a presale cdkey.
				g_pVersionMgr->GetPreSaleCDKey( szCDKey, LTARRAYSIZE( szCDKey ));
				g_pLTGameUtil->CDKeyComputeChallengeResponse( szCDKey, szChallenge, szChallengeResponse, nChallengeResponseLen );
				cResponse.WriteString( szChallengeResponse );
#endif
			}
#endif // PLATFORM_XENON

			g_pLTClient->SendToServer(cResponse.Read(), MESSAGE_GUARANTEED);

			m_eSentClientConnectionState = eClientConnectionState_KeyChallenge;
		}
		break;
		case eClientConnectionState_Overrides:
		{
			// get the overrides data and size information from the message
			uint32 nDecompressedOverridesSize = msg.Readuint32();
			uint32 nCompressedOverridesSize = msg.Readuint32();

			uint8* pCompressedOverridesData = NULL;
			LT_MEM_TRACK_ALLOC(pCompressedOverridesData = new uint8[nCompressedOverridesSize], LT_MEM_TYPE_GAMECODE);
			msg.ReadData(pCompressedOverridesData, nCompressedOverridesSize * 8);

			// allocate a buffer for the decompressed data
			uint8* pDecompressedOverridesData = NULL;
			LT_MEM_TRACK_ALLOC(pDecompressedOverridesData = new uint8[nDecompressedOverridesSize], LT_MEM_TYPE_GAMECODE);

			// decompress the data
			uint32 nDecompressedSize = 0;
			g_pLTClient->DecompressBuffer(pCompressedOverridesData, nCompressedOverridesSize, pDecompressedOverridesData, nDecompressedOverridesSize, nDecompressedSize);

			// free the compressed data
			delete [] pCompressedOverridesData;

			// check the sizes
			if (nDecompressedOverridesSize != nDecompressedSize)
			{
				// buffer did not decompress correctly
				m_bForceDisconnect = true;
				SetDisconnectCode(eDisconnect_OverridesFailed, NULL);
				delete [] pDecompressedOverridesData;
				break;
			}

			// load the overrides if we're not the host
			if (m_StartGameRequest.m_Type != STARTGAME_HOST)
			{
				if (!LoadMultiplayerOverrides(pDecompressedOverridesData, nDecompressedOverridesSize))
				{
					// overrides failed
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_OverridesFailed, NULL);
				}
			}

			// free the decompressed data buffer
			delete [] pDecompressedOverridesData;
		}
		break;
		case eClientConnectionState_ContentTransfer:
		{
			m_eSentClientConnectionState = eClientConnectionState_ContentTransfer;

			// set content transfer options in case they have changed
			SetContentTransferOptions();

			// set notification sink to handle any content transfer events
			g_pLTClientContentTransfer->SetNotificationSink(this, k_nNotifyContentTransferStarting | k_nNotifyFileReceiveProgress | k_nNotifyFileReceiveCompleted | k_nNotifyContentTransferCompleted);
		}
		break;
		case eClientConnectionState_CRCCheck:
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

			// Get the client shell handle from the engine
			HMODULE hClientShell;
			g_pLTClient->GetEngineHook("cshell_hinstance", (void**)&hClientShell);

			// Send it back their direction
			CAutoMessage cResponse;
			cResponse.Writeuint8(MID_CLIENTCONNECTION);
			cResponse.Writeuint8(eClientConnectionState_CRCCheck);
			g_pLTGameUtil->WriteCRCMessage( cResponse, pProfile->m_PlayerGuid, m_nServerKey, hClientShell, DB_Default_File );
			g_pLTClient->SendToServer(cResponse.Read(), MESSAGE_GUARANTEED);

			m_eSentClientConnectionState = eClientConnectionState_CRCCheck;
		}
		break;
		case eClientConnectionState_LoggedIn:
		{
			GameModeMgr& gameModeMgr = GameModeMgr::Instance();
			// See if there is a server message.
			if( msg.Readbool())
			{
				wchar_t wszUserMessage[GameModeMgr::kMaxUserMessageLen+1];
				msg.ReadWString( wszUserMessage, LTARRAYSIZE( wszUserMessage ));
				gameModeMgr.m_ServerSettings.m_sServerMessage = wszUserMessage;

			}
			else
			{
				gameModeMgr.m_ServerSettings.m_sServerMessage = L"";
			}

			// See if there is a override briefing message.
			if( msg.Readbool())
			{
				wchar_t wszUserMessage[GameModeMgr::kMaxUserMessageLen+1];
				msg.ReadWString( wszUserMessage, LTARRAYSIZE( wszUserMessage ));
				gameModeMgr.m_ServerSettings.m_sBriefingOverrideMessage = wszUserMessage;
			}
			else
			{
				gameModeMgr.m_ServerSettings.m_sBriefingOverrideMessage = L"";
			}


			//inform anyone who cares that we're now connected, and updated...
			ClientLoggedInEvent.DoNotify( );
			m_bClientLoggedIn = true;

		}
		break;
		case eClientConnectionState_Loading:
		{
		}
		break;
		case eClientConnectionState_InWorld:
		{
			m_nCurrentRound = msg.Readuint8();
		}
		break;
		case eClientConnectionState_Error:
		{
			// read the error code
			EClientConnectionError eClientConnectionError = ( EClientConnectionError )msg.Readuint8( );
			m_bClientLoggedIn = false;

			// read the message (if any)
			char szMessage[g_nMaximumConnectionErrorLength] = { 0 };
			if (msg.Readbool())
			{
				msg.ReadString(szMessage, LTARRAYSIZE(szMessage));
			}

			// handle the error
			switch( eClientConnectionError )
			{
				case eClientConnectionError_InvalidAssets:
				{
					// Oops... wrong password, disconnect
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_InvalidAssets, NULL);
				}
				break;
				case eClientConnectionError_WrongPassword:
				{
					// Oops... wrong password, disconnect
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_WrongPassword, NULL);
				}
				break;
				case eClientConnectionError_BadCDKey:
				{
					// Bad cdkey.
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_BadCdKey, NULL);
				}
				break;
				case eClientConnectionError_ContentTransferFailed:
				{
					// content transfer failed
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_ContentTransferFailed, NULL);
				}
				break;
				case eClientConnectionError_Banned:
				{
					// I'm a baaaaaad boy....
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_Banned, NULL);
				}
				break;
				case eClientConnectionError_TimeOut:
				{
					// Timed out.
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_TimeOut, NULL);
				}
				break;
				case eClientConnectionError_PunkBuster:
				{
					// PunkBuster error
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_PunkBuster, szMessage);
				}
				break;
				default :
				{
					// Disconnect
					m_bForceDisconnect = true;
					SetDisconnectCode(eDisconnect_NotSameGUID, NULL);
				}
				break;
			}
		}
		break;
		default :
		{
			// Disconnect
			m_bClientLoggedIn = false;
			m_bForceDisconnect = true;
			SetDisconnectCode(eDisconnect_NotSameGUID, NULL);
		}
		break;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::SendClientInWorldMessage
//
//	PURPOSE:	Sent when client done with loading and postloading screens.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::SendClientInWorldMessage( )
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	//force server to update difficulty
	GameDifficulty eDiff = g_pGameClientShell->GetDifficulty();
	g_pGameClientShell->SetDifficulty(eDiff);

	// Update the server of gore and detail settings...
	g_pGameClientShell->UpdateGoreSettings( );
	g_pGameClientShell->SendPerformanceSettingsToServer( );

	//force server to update performance settings
	pProfile->SendWeaponPriorityMsg();

	// get the world CRC value if we're a multiplayer client
	uint32 nWorldCRC = 0;
	if (IsMultiplayerGameClient())
	{
		g_pLTClient->GetWorldCRC(nWorldCRC);
	}

	// Send acknowledgment back that we are loaded.
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8(eClientConnectionState_InWorld);
	cMsg.WriteWString(pProfile->m_sPlayerName.c_str());
	cMsg.Writeuint8( GameModeMgr::Instance().m_grbUseTeams ? m_NetClientData.m_nTeamModelIndex : m_NetClientData.m_nDMModelIndex );
	cMsg.Writeuint8( m_nTeam );
	cMsg.Writeuint8( m_nLoadout );
	// Send patch as file title only to reduce bandwidth.
	char szPatchTitle[MAX_PATH] = "";
	LTFileOperations::SplitPath( pProfile->m_sPlayerPatch.c_str( ), NULL, szPatchTitle, NULL );
	cMsg.WriteString(szPatchTitle);
	g_pLTGameUtil->WriteServerKeyData( cMsg, pProfile->m_PlayerGuid, m_nServerKey, nWorldCRC );
	g_pLTClient->SendToServer(cMsg.Read(),MESSAGE_GUARANTEED);

	m_eSentClientConnectionState = eClientConnectionState_InWorld;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameClientShell::DisconnectFromServer
//
//	PURPOSE:	Disconnects the client from the multiplayer server.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::DisconnectFromServer()
{
	// restore client database
	UnloadMultiplayerOverrides();

	// abort content transfer if necessary
	g_pLTClientContentTransfer->AbortContentDownload();

	// disconnect from the server
	g_pLTClient->Disconnect();

	// rebuild and reinitialize the mission database
	g_pMissionDB->Reset();
	g_pMissionDB->Init(DB_Default_File);
	g_pMissionDB->CreateMPDB();

	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, MDB_MP_File, LTARRAYSIZE( szPath ));
	g_pMissionDB->Init( szPath );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::ClientContentTransferStartingNotification
//
//	PURPOSE:	Notification handler for the start of the content transfer.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::ClientContentTransferStartingNotification(uint32 nTotalBytes,
																	uint32 nTotalFiles)
{
#if !defined(_FINAL)
	g_pLTClient->CPrint("Content transfer starting: total transfer size [%d] bytes [%d] files", nTotalBytes, nTotalFiles);
#endif

	g_pInterfaceMgr->GetLoadingScreen()->ClientContentTransferStartingNotification(nTotalBytes,nTotalFiles);
	m_bClientContentTransferInProgress = true;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::FileReceiveProgressNotification
//
//	PURPOSE:	Notification handler for progress about a file transfer.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::FileReceiveProgressNotification(const std::string& strFilename,
														  uint32			 nFileBytesReceived,
														  uint32			 nFileBytesTotal,
														  uint32			 nTransferBytesTotal,
														  float				 fTransferRate)
{
#if !defined(_FINAL)
	g_pLTClient->CPrint("Receiving '%s': %d of %d bytes (%8.1f k/s)", 
						strFilename.c_str(), 
						nFileBytesReceived, 
						nFileBytesTotal, 
						fTransferRate / 1024.0f); // convert to k-per-second
#endif

	g_pInterfaceMgr->GetLoadingScreen()->FileReceiveProgressNotification(strFilename,
																		 nFileBytesReceived,
																		 nFileBytesTotal,
																		 nTransferBytesTotal,
																		 fTransferRate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::FileReceiveCompletedNotification
//
//	PURPOSE:	Notification handler for completion of a file transfer.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::FileReceiveCompletedNotification(const std::string& strFilename,
														   uint32			  nFileBytesTotal,
														   uint32			  nTransferBytesTotal,
														   float			  fTransferRate)
{
#if !defined(_FINAL)
	g_pLTClient->CPrint("Finished '%s': %d bytes received (%8.1f k/s)", 
						strFilename.c_str(), 
						nFileBytesTotal, 
						fTransferRate / 1024.0f); // convert to k-per-second
#endif

	g_pInterfaceMgr->GetLoadingScreen()->FileReceiveCompletedNotification(strFilename,
																		  nFileBytesTotal,
																		  nTransferBytesTotal,
																		  fTransferRate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::ErrorNotification
//
//	PURPOSE:	Notification handler for an error during content transfer.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::ClientContentTransferErrorNotification(EContentTransferError eError, 
																 const char*    pszFilename,
									  					   	     const wchar_t* pwszMessage)
{
#if !defined(_FINAL)
	g_pLTClient->CPrint("Content transfer error: [%d] [%ls]", (uint8)eError, pwszMessage);
#endif
	m_bClientContentTransferInProgress = false;

	const char* pszErrorMsg = NULL;
	switch (eError)
	{
	case eContentTransferErrorClientDownloadNotAllowed:
		pszErrorMsg = "CONTENTDL_ERROR_NOTALLOWED";
		break;
	case eContentTransferErrorClientDownloadTooLarge:
		pszErrorMsg = "CONTENTDL_ERROR_TOOLARGE";
		break;
	case eContentTransferErrorClientMissingNonDownloadableArchive:
		pszErrorMsg = "CONTENTDL_ERROR_MISSING";
		break;
	case eContentTransferErrorServerAtMaximumDownloads:
		pszErrorMsg = "CONTENTDL_ERROR_SERVERMAX";
		break;
	case eContentTransferErrorFileTransfer:
		pszErrorMsg = "CONTENTDL_ERROR_FILE";
		break;
	case eContentTransferErrorUnknown:
	default:
		pszErrorMsg = "CONTENTDL_ERROR_UNKNOWN";
		break;
	}
		
	// store the content transfer related error information, if it is available
	m_strContentTransferErrorFilename.clear();
	m_wstrContentTransferErrorServerMessage.clear();

	if (pszFilename)
	{
		m_strContentTransferErrorFilename = pszFilename;
	}
	
	if (pwszMessage)
	{
		m_wstrContentTransferErrorServerMessage = pwszMessage;
	}

	// any error means we can't connect
	m_bForceDisconnect = true;
	SetDisconnectCode(eDisconnect_ContentTransferFailed, pszErrorMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::ClientContentTransferCompletedNotification
//
//	PURPOSE:	Notification handler for an error during content transfer.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::ClientContentTransferCompletedNotification()
{
#if !defined(_FINAL)
	g_pLTClient->CPrint("Content transfer complete.");
#endif
	m_bClientContentTransferInProgress = false;

	g_pInterfaceMgr->GetLoadingScreen()->ClientContentTransferCompletedNotification();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::HandleMsgMultiplayerData()
//
//	PURPOSE:	Read multiplayer data sent from server.
//
// ----------------------------------------------------------------------- //

bool ClientConnectionMgr::HandleMsgMultiplayerData( ILTMessage_Read& msg )
{
	char pszServerAddress[256];
	msg.ReadString( pszServerAddress, LTARRAYSIZE(pszServerAddress));
	m_sServerAddress = pszServerAddress;

	uint32 tmp = msg.Readuint32();
	m_nServerPort = (int)tmp;

	uint32 nID = (uint32) msg.Readuint32();

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	g_pInterfaceMgr->GetClientInfoMgr()->PlayerConnected( pProfile->m_sPlayerName.c_str(), pProfile->m_sPlayerPatch.c_str(), nID );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::OnEvent(uint32 dwEventID, uint32 dwParam)
{
	if( !IsMultiplayerGameClient( ))
		return;

	switch(dwEventID)
	{
		// Client disconnected from server.  dwParam will
		// be a error flag found in de_codes.h.
		case LTEVENT_DISCONNECT :
		{
			m_sServerAddress.clear( );
			m_nServerPort = -1;
			m_sServerName.clear( );
			m_nServerKey = 0;
		} 
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::Update()
//
//	PURPOSE:	Frame update.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::Update( )
{
	// This will happen when something wanted to disconnect, but wasn't
	// in a valid location to do so.  (e.g. when processing packets..)
	if (m_bForceDisconnect)
	{
		DisconnectFromServer();
		m_bForceDisconnect = false;
		return;
	}

	if (m_eSentClientConnectionState == eClientConnectionState_ContentTransfer)
	{
		g_pLTClientContentTransfer->OnUpdate();
	}

	UpdateSocketConnectionState( );

	// Send a tickle message to make sure we don't get disconnected for no activity.
	if( IsConnectedToRemoteServer( ))
	{
		if( !m_GameTickleTimer.IsStarted() || m_GameTickleTimer.IsTimedOut())
		{
			float fAutoBootTime;
			if( g_pLTClient->GetSConValueFloat( "AutoBootTime", fAutoBootTime ) != LT_OK )
				fAutoBootTime = 60.0f;
			float fSendRate = fAutoBootTime / 4.0f;

			m_GameTickleTimer.Start( fSendRate );

			SendKeepAliveMessage();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::SendKeepAliveMessage
//
//	PURPOSE:	Sends a keep alive message to the server to avoid 
//				disconnection due to timeout.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::SendKeepAliveMessage( )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENTCONNECTION);
	cMsg.Writeuint8(eClientConnectionState_Tickle);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::SetService
//
//	PURPOSE:	Selects the connection service for hosting/joining internet games.
//
// --------------------------------------------------------------------------- //

bool ClientConnectionMgr::SetService( )
{
	NetService *pCur, *pListHead;
	HNETSERVICE hNetService;

	pCur      = NULL;
	pListHead = NULL;
	hNetService = NULL;

    if( g_pLTClient->GetServiceList( pListHead ) != LT_OK || !pListHead )
        return false;

	// Find the service specified.
	pCur = pListHead;
	while( pCur )
	{
		if( pCur->m_dwFlags & NETSERVICE_TCPIP )
		{
			hNetService = pCur->m_handle;
			break;
		}

		pCur = pCur->m_pNext;
	}

	// Free the service list.
    g_pLTClient->FreeServiceList( pListHead );

	// Check if tcp not found.
	if( !hNetService )
        return false;

	// Select it.
    if( g_pLTClient->SelectService( hNetService ) != LT_OK )
        return false;

    return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::StartClient
//
//	PURPOSE:	Start a client of a remote server.
//
// --------------------------------------------------------------------------- //

bool ClientConnectionMgr::StartClient( )
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	// Start off disconnected.
	m_eSocketConnectionState = eSocketConnectionState_Disconnected;

	// Initialize the networking.
    m_nLastConnectionResult = g_pLTClient->InitNetworking(NULL, 0);
	if (m_nLastConnectionResult != LT_OK)
	{
        return false;
	}

	// Initialize our protocol.
	if (!SetService())
        return false;

	// Hook up the netgame and clientinfo.
	m_StartGameRequest.m_pClientData = &m_NetClientData;
	m_StartGameRequest.m_ClientDataLen = sizeof( m_NetClientData );

	// Do prediction in mp.
	g_pLTClient->SetConsoleVariableFloat("Prediction", 1.0f);

	// If we don't need natneg, then go right to connecting.
	if( !m_bDoNatNegotiations )
	{
		m_eSocketConnectionState = eSocketConnectionState_Connecting;
		return true;
	}

	// Convert the address string into ip and port.
	char szIP[256];
	uint16 nPort;
	char* pszPortDelim = strchr( m_StartGameRequest.m_TCPAddress, ':' );
	if( !pszPortDelim || !pszPortDelim[1] )
		return false;
	uint32 nIPLen = pszPortDelim - m_StartGameRequest.m_TCPAddress;
	strncpy( szIP, m_StartGameRequest.m_TCPAddress, nIPLen );
	szIP[nIPLen] = 0;
	nPort = atoi( pszPortDelim + 1 );

	// Start a ping request.  This just gets the networking rolling.
	SOCKET hSocket;
	if( LT_OK != g_pLTClient->OpenSocket( &hSocket ))
		return false;

	// Make sure we have a serverbrowser object.
	if( !m_pGameSpyBrowser )
	{
		CreateServerBrowser( );
	}

	if( m_pGameSpyBrowser && hSocket )
	{
		// Start natneg process.
		if( m_pGameSpyBrowser->RequestNatNegotiation( hSocket, szIP, nPort ))
		{
			m_StartGameRequest.m_nSocket = hSocket;
			m_eSocketConnectionState = eSocketConnectionState_NatNeg;
			return true;
		}
	}
	
	// Couldn't get natneg going, just do regular connecting.
	m_eSocketConnectionState = eSocketConnectionState_Connecting;
	return true;

#else // PLATFORM_XENON
	return false;
#endif // PLATFORM_XENON

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	SynchronizeHostPBSettings
//
//	PURPOSE:	Synchronize the host client pb settings with the server pb settings.
//
// --------------------------------------------------------------------------- //
static void SynchronizeHostPBSettings( )
{
	// If the host wants the server to use pb, then make sure the host's client is using pb.  If the
	// host doesn't want the server to use pb, then it doesn't matter what the host's client is
	// set to.
	IPunkBusterServer::StartupInfo PBstartupInfo;
	IPunkBusterServer* pPunkBusterServer = g_pLTGameUtil->CreatePunkBusterServer( PBstartupInfo );
	if( pPunkBusterServer )
	{
		// Get the true setting.
		if( pPunkBusterServer->IsEnabled( ))
		{
			IPunkBusterClient* pPunkBusterClient = g_pGameClientShell->GetPunkBusterClient();
			if( pPunkBusterClient && !pPunkBusterClient->IsEnabled( ))
			{
				pPunkBusterClient->Enable( );
			}
		}

		// Don't need this any longer.
		g_pLTGameUtil->DestroyPunkBusterServer( pPunkBusterServer );
		pPunkBusterServer = NULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::StartServerHost
//
//	PURPOSE:	Start a hosted game.
//
// --------------------------------------------------------------------------- //

bool ClientConnectionMgr::StartServerAsHost( )
{
	// If they want a dedicated server, then launch the serverapp.
	if( GameModeMgr::Instance( ).m_ServerSettings.m_bDedicated )
	{
		if( !g_pGameClientShell->LauncherServerApp( 
			g_pProfileMgr->GetCurrentProfile()->m_sServerOptionsFile.c_str( )))
			return false;

		return true;
	}

	// Check if we're already connected to a server.
  	if( g_pLTClient->IsConnected( ))
	{
  		// Check if we are already hosting mp.
  		if( IsMultiplayerGameClient( ) && m_StartGameRequest.m_Type == STARTGAME_HOST )
  		{
  			// Don't need to restart a server.
  			return true;
		}
	}

	// Make sure our pb settings are synchronized with the server.
	SynchronizeHostPBSettings( );

	SetupClientBandwidth( GameModeMgr::Instance( ).m_ServerSettings.m_bLANOnly );

	// Initialize the networking.  Always start a new server with hosted games.
    m_nLastConnectionResult = g_pLTClient->InitNetworking(NULL, 0);
	if (m_nLastConnectionResult != LT_OK)
	{
        return false;
	}

	// Initialize our protocol.
	if (!SetService())
        return false;

	// Make sure we throw away the serverbrowser objects.
	TermBrowser( );

	// Hook up the netgame and clientinfo.
	NetGameInfo* pNetGameInfo = &m_NetGameInfo;
	m_StartGameRequest.m_pGameInfo = &pNetGameInfo;
	m_StartGameRequest.m_GameInfoLen = sizeof( pNetGameInfo );
	m_StartGameRequest.m_pClientData = &m_NetClientData;
	m_StartGameRequest.m_ClientDataLen = sizeof( m_NetClientData );

	// Go right to being connected.
	m_eSocketConnectionState = eSocketConnectionState_Connected;

	// Do prediction in mp.
	g_pLTClient->SetConsoleVariableFloat("Prediction", 1.0f);

	// Start the server.
	m_nLastConnectionResult = g_pLTClient->StartGame( const_cast< StartGameRequest * >( &m_StartGameRequest ));

	return ( m_nLastConnectionResult == LT_OK );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::StartServerSinglePlayer
//
//	PURPOSE:	Starts the single player server.
//
// --------------------------------------------------------------------------- //

bool ClientConnectionMgr::StartServerAsSinglePlayer( )
{
	// Check if we're already connected to a server.
  	if( g_pLTClient->IsConnected( ))
	{
  		// Check if we are already running sp server.
  		if( !IsMultiplayerGameClient( ) && m_StartGameRequest.m_Type == STARTGAME_NORMAL )
  		{
  			// Don't need to restart a server.
  			return true;
		}
	}

	// Hook up the netgame and clientinfo.
	NetGameInfo* pNetGameInfo = &m_NetGameInfo;
	m_StartGameRequest.m_pGameInfo = &pNetGameInfo;
	m_StartGameRequest.m_GameInfoLen = sizeof( pNetGameInfo );
	m_StartGameRequest.m_pClientData = &m_NetClientData;
	m_StartGameRequest.m_ClientDataLen = sizeof( m_NetClientData );

	// Make sure we throw away the serverbrowser objects.
	TermBrowser( );

	// Start with clean slate
	m_StartGameRequest.m_Type = STARTGAME_NORMAL;

	// Go right to being connected.
	m_eSocketConnectionState = eSocketConnectionState_Connected;

	// No prediction in sp.
	g_pLTClient->SetConsoleVariableFloat("Prediction", 0.0f);

    m_nLastConnectionResult = g_pLTClient->StartGame(&m_StartGameRequest);

	return ( m_nLastConnectionResult == LT_OK );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::StartClientServer
//
//	PURPOSE:	Starts a client/server based on previously set startgamerequest.
//
// --------------------------------------------------------------------------- //

bool ClientConnectionMgr::StartClientServer( )
{
	//clear out old session specific data before starting new session
	m_nTeam = INVALID_TEAM;
	m_bHasSelectedTeam = false;
	m_bHasSelectedLoadout = false;
	m_NetGameInfo.m_eServerStartResult = eServerStartResult_None;

	switch( m_StartGameRequest.m_Type )
	{
		case STARTGAME_NORMAL:
			return StartServerAsSinglePlayer( );
			break;
		case STARTGAME_HOST:
			return StartServerAsHost( );
			break;
		case STARTGAME_CLIENT:
			return StartClient( );
			break;
		default:
			ASSERT( !"ClientConnectionMgr::StartClientServer: Invalid gamerequest type." );
			return false;
			break;
	}
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::SelectTeam()
//
//	PURPOSE:	choose a team.
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::SelectTeam(uint8 nTeam, bool bPlayerSelected )
{
	if (bPlayerSelected)
		m_bHasSelectedTeam = true;
	m_nTeam = nTeam;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::SelectLoadout()
//
//	PURPOSE:	choose a weapon loadout
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::SelectLoadout(uint8 nLoadout )
{
	m_bHasSelectedLoadout = true;
	m_nLoadout = nLoadout;
}


// --------------------------------------------------------------------------- //
//
//  ROUTINE:    ClientConnectionMgr::SetNetClientData
//
//  PURPOSE:    Updates the NetClientData to reflect current settings.
//
// --------------------------------------------------------------------------- //

bool ClientConnectionMgr::SetNetClientData( )
{
	// Setup our client...
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
	{
		ASSERT( !"ClientConnectionMgr::SetNetClientData: Invalid profile." );
		return false;
	}

	// Check if the user overrode the player name on the command line.
	wchar_t szPlayerNameOverride[MAX_PLAYER_NAME] = L"";
	std::wstring sTemp;
	sTemp = MPA2W( GetConsoleTempString( "playername", "" ));
	LTStrCpy( szPlayerNameOverride, FixPlayerName( sTemp.c_str( )), ARRAY_LEN( szPlayerNameOverride ));

	if( szPlayerNameOverride[0] )
	{
		WriteConsoleString("playername"," ");
		pProfile->m_sPlayerName = szPlayerNameOverride;
	}

	LTStrCpy(m_NetClientData.m_szName,pProfile->m_sPlayerName.c_str(), ARRAY_LEN( m_NetClientData.m_szName ));

	// Setup the name guid.
	m_NetClientData.m_PlayerGuid = pProfile->m_PlayerGuid;

	if( IsMultiplayerGameClient( ))
	{
		m_NetClientData.m_nDMModelIndex = pProfile->m_nDMPlayerModel;
		m_NetClientData.m_nTeamModelIndex = pProfile->m_nTeamPlayerModel;
	}

	return true;	
}

#if !defined(PLATFORM_XENON)

// ----------------------------------------------------------------------- //
// Function name   : SetupGameSpyBrowser
// Description     : Sets the gamespybrowser object up with property keys.
// Return type     : static bool - true on success.
// Argument        : IGameSpyBrowser& gameSpyBrowser - browser to setup.
// ----------------------------------------------------------------------- //
static bool SetupGameSpyBrowser( IGameSpyBrowser& gameSpyBrowser )
{
	// Register the keys used.
	gameSpyBrowser.RegisterKey( "hostname" );
	gameSpyBrowser.RegisterKey( "mapname" );
	gameSpyBrowser.RegisterKey( "mappath" );
	gameSpyBrowser.RegisterKey( "numplayers" );
	gameSpyBrowser.RegisterKey( "maxplayers" );
	gameSpyBrowser.RegisterKey( "gametype" );
	gameSpyBrowser.RegisterKey( "gamemode" );
	gameSpyBrowser.RegisterKey( "password" );
	gameSpyBrowser.RegisterKey( "gamever" );
	gameSpyBrowser.RegisterKey( "fraglimit" );
	gameSpyBrowser.RegisterKey( "timelimit" );
	gameSpyBrowser.RegisterKey( "player_" );
	gameSpyBrowser.RegisterKey( "score_" );
	gameSpyBrowser.RegisterKey( "ping_" );
	gameSpyBrowser.RegisterKey( "gamevariant" );
	gameSpyBrowser.RegisterKey( "options" );
	gameSpyBrowser.RegisterKey( "hasoverrides" );
	gameSpyBrowser.RegisterKey( "downloadablefiles" );
	gameSpyBrowser.RegisterKey( "overridesdata" );
	gameSpyBrowser.RegisterKey( "dedicated" );
	gameSpyBrowser.RegisterKey( "linux" );
	gameSpyBrowser.RegisterKey( "punkbuster" );

	// Use these keys for the summary.
	gameSpyBrowser.AddSummaryKey( "hostname" );
	gameSpyBrowser.AddSummaryKey( "numplayers" );
	gameSpyBrowser.AddSummaryKey( "maxplayers" );
	gameSpyBrowser.AddSummaryKey( "gametype" );
	gameSpyBrowser.AddSummaryKey( "gamever" );
	gameSpyBrowser.AddSummaryKey( "gamevariant" );
	gameSpyBrowser.AddSummaryKey( "password" );
	gameSpyBrowser.AddSummaryKey( "hasoverrides" );
	gameSpyBrowser.AddSummaryKey( "downloadablefiles" );
	gameSpyBrowser.AddSummaryKey( "dedicated" );
	gameSpyBrowser.AddSummaryKey( "linux" );
	gameSpyBrowser.AddSummaryKey( "punkbuster" );
	gameSpyBrowser.AddSummaryKey( "mapname" );
	gameSpyBrowser.AddSummaryKey( "mappath" );

	return true;
}

#endif // !PLATFORM_XENON

// ----------------------------------------------------------------------- //
// Function name   : ClientConnectionMgr::CreateServerBrowser
// Description     : Creates the serverbrowser objects needed for joining
//						public/lan servers.
// Return type     : static bool - true on success.
// ----------------------------------------------------------------------- //
bool ClientConnectionMgr::CreateServerBrowser( )
{
#if !defined(PLATFORM_XENON)

	if( m_pGameSpyBrowser  )
		return true;

	// Create the retail browser.
	IGameSpyBrowser::StartupInfo startupInfo;
	m_pGameSpyBrowser = g_pLTGameUtil->CreateGameSpyBrowser( startupInfo );
	SetupGameSpyBrowser( *m_pGameSpyBrowser );

#endif // !PLATFORM_XENON

	return true;
}

// ----------------------------------------------------------------------- //
// Function name   : ClientConnectionMgr::TermBrowser
// Description     : Deletes the server browser object.  Good idea
//						to delete them when you are done with them,
//						since they can store lots of data in the server lists.
// ----------------------------------------------------------------------- //
void ClientConnectionMgr::TermBrowser( )
{
#if !defined(PLATFORM_XENON)

	// Delete the browsers now that we're joining.
	if( !m_pGameSpyBrowser )
		return;

	g_pLTGameUtil->DestroyGameSpyBrowser( m_pGameSpyBrowser );
	m_pGameSpyBrowser = NULL;

#endif // !PLATFORM_XENON

}

// ----------------------------------------------------------------------- //
// Function name   : ClientConnectionMgr::UpdateConnectionState
// Description     : Updates the statemachine.
// ----------------------------------------------------------------------- //
void ClientConnectionMgr::UpdateSocketConnectionState( )
{
	switch( m_eSocketConnectionState )
	{
		case eSocketConnectionState_Disconnected:
			break;
		case eSocketConnectionState_NatNeg:
			{
				UpdateSocketConnectionState_NatNeg( );
			}
			break;
		case eSocketConnectionState_SettleComm:
			{
				UpdateSocketConnectionState_SettleComm( );
			}
			break;
		case eSocketConnectionState_Connecting:
			{
				UpdateSocketConnectionState_Connecting( );
			}
			break;
		case eSocketConnectionState_Connected:
			break;
		case eSocketConnectionState_Aborted:
			{
				m_eSocketConnectionState = eSocketConnectionState_Disconnected;
			}
			break;
		case eSocketConnectionState_Failure:
			{
				g_pInterfaceMgr->LoadFailed( );
				m_eSocketConnectionState = eSocketConnectionState_Disconnected;
			}
			break;
	}
}

// ----------------------------------------------------------------------- //
// Function name   : ClientConnectionMgr::UpdateState_NatNeg
// Description     : Updates the natneg state.
// ----------------------------------------------------------------------- //
void ClientConnectionMgr::UpdateSocketConnectionState_NatNeg( )
{

#if !defined(PLATFORM_XENON)

	switch( m_pGameSpyBrowser->GetBrowserStatus( ))
	{
		case IGameSpyBrowser::eBrowserStatus_Processing:
			break;
		case IGameSpyBrowser::eBrowserStatus_Error:
		case IGameSpyBrowser::eBrowserStatus_Idle:
			// Drop the socket, we don't need it any more.
			m_StartGameRequest.m_nSocket = StartGameRequest::kInvalidSocket;
			m_eSocketConnectionState = eSocketConnectionState_Connecting;
			break;
		case IGameSpyBrowser::eBrowserStatus_Complete:
			sockaddr_in sockAddr;
			m_pGameSpyBrowser->GetNatNegotiationResult( ( sockaddr* )( &sockAddr ));

			// Change our address to the one found.
			LTSNPrintF( m_StartGameRequest.m_TCPAddress, LTARRAYSIZE( m_StartGameRequest.m_TCPAddress ), ADDR_PRINTF, EXPAND_ADDR( sockAddr ));

			m_SettleCommTimer.Start( 2.0f );
			m_eSocketConnectionState = eSocketConnectionState_SettleComm;
			break;
	}

#endif // !PLATFORM_XENON

}

// ----------------------------------------------------------------------- //
// Function name   : ClientConnectionMgr::UpdateState_SettleComm
// Description     : Lets comm settle for a few seconds.
// ----------------------------------------------------------------------- //
void ClientConnectionMgr::UpdateSocketConnectionState_SettleComm( )
{
	if( m_SettleCommTimer.IsTimedOut( ))
	{
		m_eSocketConnectionState = eSocketConnectionState_Connecting;
	}
}

// ----------------------------------------------------------------------- //
// Function name   : ClientConnectionMgr::UpdateState_Connecting
// Description     : Updates the connecting state.
// ----------------------------------------------------------------------- //
void ClientConnectionMgr::UpdateSocketConnectionState_Connecting( )
{
	// We will try to connect to the server using several methods.  First, we'll use
	// the default address that we have determined through gamespy.  This may or may
	// not have been setup through natneg.  Then we'll try to connect to the private address.
	// If that doesn't work, we'll try the public address.  If none of those work, we'll 
	// give up.
	enum EConnectAddress
	{
		eConnectAddress_Default,
		eConnectAddress_Private,
		eConnectAddress_Public,
		eConnectAddress_Failed,
	};
	EConnectAddress eConnectAddress = eConnectAddress_Default;
	while( eConnectAddress != eConnectAddress_Failed )
	{
		// If successful, then we're done.
		m_nLastConnectionResult = g_pLTClient->StartGame( const_cast< StartGameRequest * >( &m_StartGameRequest ));
		if( m_nLastConnectionResult == LT_OK )
		{
			m_eSocketConnectionState = eSocketConnectionState_Connected;
			break;
		}
		else if ( m_nLastConnectionResult == LT_ESCABORT )
		{
			m_eSocketConnectionState = eSocketConnectionState_Aborted;
			break;
		}
		else if( m_nLastConnectionResult == LT_REJECTED || m_nLastConnectionResult == LT_NOTSAMEGUID)
		{
			m_eSocketConnectionState = eSocketConnectionState_Failure;
			break;
		}

		// Go to the next address to try.
		if( eConnectAddress == eConnectAddress_Default )
		{
			strncpy( m_StartGameRequest.m_TCPAddress, m_sConnectPrivateAddress.c_str( ), MAX_SGR_STRINGLEN);
			eConnectAddress = eConnectAddress_Private;
		}
		else if( eConnectAddress == eConnectAddress_Private )
		{
			strncpy( m_StartGameRequest.m_TCPAddress, m_sConnectPublicAddress.c_str( ), MAX_SGR_STRINGLEN);
			eConnectAddress = eConnectAddress_Public;
		}
		else
		{
			eConnectAddress = eConnectAddress_Failed;
		}
	}

	if( eConnectAddress == eConnectAddress_Failed )
	{
		m_eSocketConnectionState = eSocketConnectionState_Failure;
	}

	// Make sure we throw away the serverbrowser objects.  Throw away
	// after startgame call just in case more comm was required.
	TermBrowser( );
}

bool ClientConnectionMgr::LoadMultiplayerOverrides(uint8* pDecompressedOverridesData, uint32 nDecompressedOverridesSize)
{
	// associate an ILTInStream with the overrides buffer
	ILTInStream* pOverridesStream = NULL;
	g_pLTClient->OpenStreamFromBuffer(pDecompressedOverridesData, nDecompressedOverridesSize, false, &pOverridesStream);

	if (!pOverridesStream)
	{
		// could not open stream
		return false;
	}

	// open the overrides database
	m_hOverridesDatabase = OpenOverridesDatabase(*pOverridesStream, g_pszConstraintsFilename);

	if (!m_hOverridesDatabase)
	{
		// could not open overrides database
		pOverridesStream->Release();
		return false;
	}

	// get the game database
	m_hGameDatabase = OpenGameDatabase(DB_Default_File);

	if (!m_hGameDatabase)
	{
		// could not open game database
		pOverridesStream->Release();
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		return false;
	}

	// swap the values
	if (!g_pLTDatabase->SwapDatabaseValues(m_hOverridesDatabase, m_hGameDatabase))
	{
		// overrides failed
		pOverridesStream->Release();
		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		return false;
	}

	pOverridesStream->Release();

	return true;
}

void ClientConnectionMgr::UnloadMultiplayerOverrides()
{
	if (m_hOverridesDatabase)
	{
		LTASSERT(m_hGameDatabase, "null game database handle encountered");

		// restore original database state and release
		if (!g_pLTDatabase->SwapDatabaseValues(m_hOverridesDatabase, m_hGameDatabase))
		{
			LTERROR("failed to restore game database state");
		}

		g_pLTDatabase->ReleaseDatabase(m_hGameDatabase);
		m_hGameDatabase = NULL;

		g_pLTDatabase->ReleaseDatabase(m_hOverridesDatabase);
		m_hOverridesDatabase = NULL;
	}
}

void ClientConnectionMgr::SetContentTransferOptions()
{
	SClientContentTransferOptions cContentTransferOptions;

	cContentTransferOptions.m_bAllowContentDownload         = g_pProfileMgr->GetCurrentProfile()->m_bAllowContentDownload;
	cContentTransferOptions.m_bAllowContentDownloadRedirect = g_pProfileMgr->GetCurrentProfile()->m_bAllowContentDownloadRedirect;
	cContentTransferOptions.m_nMaxDownloadSize			    = g_pProfileMgr->GetCurrentProfile()->m_nMaximumDownloadSize;

	char szCustomContentDirectory[MAX_PATH];
	LTFileOperations::GetUserDirectory(szCustomContentDirectory, LTARRAYSIZE(szCustomContentDirectory));
	LTStrCat(szCustomContentDirectory, DOWNLOADED_CONTENT_DIRECTORY, LTARRAYSIZE(szCustomContentDirectory));
	LTStrCpy(cContentTransferOptions.m_szCustomContentDirectory, szCustomContentDirectory, LTARRAYSIZE(cContentTransferOptions.m_szCustomContentDirectory));

	g_pLTClientContentTransfer->SetOptions(cContentTransferOptions);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::OnExitWorld()
//
//	PURPOSE:	Handle exiting a world
//
// ----------------------------------------------------------------------- //

void ClientConnectionMgr::OnExitWorld()
{
	if( GameModeMgr::Instance().m_grbUseTeams && GameModeMgr::Instance().m_grbSwitchTeamsBetweenRounds)
	{
		if (m_nTeam != INVALID_TEAM)
		{
			m_nTeam = (m_nTeam + 1) % MAX_TEAMS;
		}
	}
	// If we're switching missions, then players may want to switch teams and weapons.
	if( !IsMultiplayerGameClient( ) || !g_pMissionMgr->IsNewMission( ))
		return;


	// Force reselect of loadout.
	m_bHasSelectedLoadout = false;

	// If we're on an elimination mode, then we can't switch teams in the middle.  Have
	// the user reselect their team at the beginning of the round.
	if( GameModeMgr::Instance().m_grbUseTeams)
	{
		if (!GameModeMgr::Instance().m_grbAllowRespawnFromDeath)
		{
			m_bHasSelectedTeam = false;
		}
		
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::SetupClientBandwidth()
//
//	PURPOSE:	Setup the bandwidth settings based on lan.
//
// ----------------------------------------------------------------------- //
void ClientConnectionMgr::SetupClientBandwidth( bool bLan )
{

	m_bClientLAN = bLan;

	// Use max on LAN games
	uint32 nMinBandwidth = (bLan) ? k_nMaxBandwidth : 0;

	uint32 nBandwidthTargetClient;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	//set mp console vars here
	if (pProfile->m_nBandwidthClient < eBandwidth_Custom)
		nBandwidthTargetClient = g_BandwidthClient[pProfile->m_nBandwidthClient].m_nBandwidthTargetClient;
	else
		nBandwidthTargetClient = pProfile->m_nBandwidthClientCustom;

	nBandwidthTargetClient = LTCLAMP( nBandwidthTargetClient * 1024, nMinBandwidth, k_nMaxBandwidth);
	WriteConsoleFloat("BandwidthTargetClient",( float )nBandwidthTargetClient );

	// Find the appropriate csendrate for this bandwidth.
	for( uint32 i = 0; i < eBandwidth_Custom; i++ )
	{
		if( nBandwidthTargetClient <= g_BandwidthClient[i].m_nBandwidthTargetClient * 1024 )
		{
			WriteConsoleFloat( "CSendRate", ( float )g_BandwidthClient[i].m_nCSendRate );
			break;
		}
	}

	// Didn't find the sendrate in the table, just set the max.
	if( i == eBandwidth_Custom )
	{
		WriteConsoleFloat( "CSendRate", ( float )g_BandwidthClient[eBandwidth_Custom-1].m_nCSendRate );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientConnectionMgr::DoConnectToIP
//
//	PURPOSE:	Handle connecting to a server from specific IP.
//
// ----------------------------------------------------------------------- //
bool ClientConnectionMgr::DoConnectToIP( char const* pszIPandPort, wchar_t const* pwszPassword, bool bDoNatNegotiations )
{
	bool bOk = true;
	CUserProfile* pProfile = g_pProfileMgr->GetCurrentProfile();

	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, FILE_PATH_SEPARATOR, LTARRAYSIZE( szPath ));
	LTStrCat( szPath, MDB_MP_File, LTARRAYSIZE( szPath ));
	bOk = g_pMissionDB->Init( szPath );

	bOk = bOk && !LTStrEmpty( pszIPandPort );
	bOk = bOk && g_pClientConnectionMgr->SetupClient( NULL, pwszPassword, bDoNatNegotiations, true, pszIPandPort, pszIPandPort );
	bOk = bOk && g_pMissionMgr->StartGameAsClient( );

	return bOk;
}
