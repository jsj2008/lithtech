// ----------------------------------------------------------------------- //
//
// MODULE  : ClientMultiplayerMgr.cpp
//
// PURPOSE : Clientside multiplayer mgr - Definition
//
// CREATED : 02/05/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientMultiplayerMgr.h"
#include "GameClientShell.h"
#include "MsgIds.h"
#include "CharacterFx.h"
#include "CMoveMgr.h"
#include "iserverdir.h"
#include "MissionMgr.h"
#include "MissionButeMgr.h"
#include "InterfaceMgr.h"
#include "ProfileMgr.h"
#include "WeaponMgr.h"
#include "CRC32.h"

#pragma message( "FIXFIX:  Should really be called ClientConnectionMgr." )


ClientMultiplayerMgr* g_pClientMultiplayerMgr = NULL;


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::ClientMultiplayerMgr
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

ClientMultiplayerMgr::ClientMultiplayerMgr( )
{
	m_nServerPort = -1;
	m_nServerKey = 0;
	m_nDisconnectCode = 0;
	m_bForceDisconnect = false;
	m_pServerDir = 0;
	m_aCurMessageSourceAddr[0] = 0;
	m_aCurMessageSourceAddr[1] = 0;
	m_aCurMessageSourceAddr[2] = 0;
	m_aCurMessageSourceAddr[3] = 0;
	m_nCurMessageSourcePort = 0;

	memset( &m_StartGameRequest, 0, sizeof( m_StartGameRequest ));
	m_StartGameRequest.m_Type = GAMEMODE_NONE;
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	m_nLastConnectionResult = LT_OK;

	g_pClientMultiplayerMgr = this;

	m_nTeam = INVALID_TEAM;
	m_bHasSelectedTeam = false;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::~ClientMultiplayerMgr
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

ClientMultiplayerMgr::~ClientMultiplayerMgr( )
{
	delete GetServerDir();

	g_pClientMultiplayerMgr = NULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::InitSinglePlayer
//
//	PURPOSE:	Send the server the initial single player info
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::InitSinglePlayer()
{

	g_pWeaponMgr->LoadOverrideButes( WEAPON_DEFAULT_FILE );

	//force server to update difficulty
	GameDifficulty eDiff = g_pGameClientShell->GetDifficulty();
	g_pGameClientShell->SetDifficulty(eDiff);

	//force server to update performance settings
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->SendPerformanceMsg();


	return true;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::InitMultiPlayer
//
//	PURPOSE:	Send the server the initial multiplayer info
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::InitMultiPlayer()
{
	if (!IsMultiplayerGame())
		return false;
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	ModelId modelId = eModelIdInvalid;	

	switch (g_pGameClientShell->GetGameType())
	{
	case eGameTypeCooperative:
		if (pProfile->m_nCPPlayerModel >= g_pModelButeMgr->GetNumCPModels())
			pProfile->m_nCPPlayerModel = 0;

		modelId = g_pModelButeMgr->GetCPModel(pProfile->m_nCPPlayerModel);
		break;
	case eGameTypeDeathmatch:
		if (pProfile->m_nDMPlayerModel >= g_pModelButeMgr->GetNumDMModels())
			pProfile->m_nDMPlayerModel = 0;

		modelId = g_pModelButeMgr->GetDMModel(pProfile->m_nDMPlayerModel);
		break;
//		if (pProfile->m_nDMPlayerModel >= g_pModelButeMgr->GetNumDMModels())
//			pProfile->m_nDMPlayerModel = 0;
//
//		modelId = g_pModelButeMgr->GetDMModel(pProfile->m_nDMPlayerModel);
		break;
	};

	CAutoMessage cMsg;
  
	cMsg.Writeuint8( MID_PLAYER_MULTIPLAYER_INIT );

	ASSERT(( uint8 )modelId == modelId );
    cMsg.Writeuint8(( uint8 )modelId );
	cMsg.Writeuint8( m_nTeam );

	for (uint8 i = 0; i < kNumSkills; i++)
	{
		cMsg.Writeuint8(pProfile->m_nPlayerSkills[i]);
	}


    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	//force server to update difficulty

	GameDifficulty eDiff = GD_NORMAL;
	if (g_pGameClientShell->GetGameType() == eGameTypeCooperative)
	{
		eDiff = (GameDifficulty)pProfile->m_ServerGameOptions.GetCoop().m_nDifficulty;
	}
	g_pGameClientShell->SetDifficulty(eDiff);

	//force server to update performance settings
	pProfile->SendPerformanceMsg();



	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::UpdateMultiPlayer
//
//	PURPOSE:	Send the server the updated multiplayer info
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::UpdateMultiPlayer()
{
	if (!IsMultiplayerGame())
		return false;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	ModelId modelId = eModelIdInvalid;	
	
	switch (g_pGameClientShell->GetGameType())
	{
	case eGameTypeCooperative:
		if (pProfile->m_nCPPlayerModel >= g_pModelButeMgr->GetNumCPModels())
			pProfile->m_nCPPlayerModel = 0;

		modelId = g_pModelButeMgr->GetCPModel(pProfile->m_nCPPlayerModel);
		break;
	case eGameTypeDeathmatch:
//	case eGameTypeTeamDeathmatch:
//	case eGameTypeDoomsDay:
		if (pProfile->m_nDMPlayerModel >= g_pModelButeMgr->GetNumDMModels())
			pProfile->m_nDMPlayerModel = 0;

		modelId = g_pModelButeMgr->GetDMModel(pProfile->m_nDMPlayerModel);
		break;
	};

	if( !UpdateNetClientData( ))
		return false;

	CAutoMessage cMsg;
  
	cMsg.Writeuint8( MID_PLAYER_INFOCHANGE );

	cMsg.WriteString(pProfile->m_sPlayerName.c_str());

	ASSERT(( uint8 )modelId == modelId );
    cMsg.Writeuint8(( uint8 )modelId );
	cMsg.Writeuint8( m_nTeam );


    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	return true;
}

// returns true if the passed in address matches the current server address
bool ClientMultiplayerMgr::CheckServerAddress(char const*pszTestAddress, int nPort)
{
	if (!pszTestAddress) return false;
	if (nPort != m_nServerPort) return false;
	return (stricmp(pszTestAddress,m_sServerAddress) == 0);
}


void ClientMultiplayerMgr::DoTaunt(uint32 nClientID, uint32 nTauntID)
{
	//if you're not listening to taunts, you're not allowed to send them
	if (GetConsoleInt("IgnoreTaunts",0) > 0) return;

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

//	char szVar[16] = "";
//	sprintf(szVar,"TauntDM%d",nTaunt);

//	uint32 nTauntID = (uint32)GetConsoleInt(szVar,0);

	if (!nTauntID) return;

	// Don't allow the client to flood the server with taunts...

	CCharacterFX *pFX = g_pGameClientShell->GetPlayerMgr( )->GetMoveMgr()->GetCharacterFX();
	if (pFX && !pFX->IsPlayingTaunt())
	{
		pFX->PlayTaunt(nTauntID);

		// create the message
		char strMessage[256];
		SAFE_STRCPY(strMessage, LoadTempString(nTauntID));

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_TAUNT);
		cMsg.Writeuint32(nTauntID);
	    cMsg.WriteString(strMessage);
	    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}



bool ClientMultiplayerMgr::SetupClient(char const* pszIpAddress, char const* pszHostName, char const* pszPassword)
{
	// Check inputs.
	if( !pszIpAddress || !pszIpAddress[0] )
	{
		ASSERT( !"ClientMultiplayerMgr::SetupClient: Invalid inputs." );
		return false;
	}

	// Start the game...

	memset( &m_StartGameRequest, 0, sizeof( m_StartGameRequest ));
	m_ServerGameOptions.Clear( );
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	if( !UpdateNetClientData( ))
		return false;

	m_StartGameRequest.m_Type = STARTGAME_CLIENTTCP;
	strncpy( m_StartGameRequest.m_TCPAddress, pszIpAddress, MAX_SGR_STRINGLEN);
	if( pszHostName && pszHostName[0] )
	{
		SAFE_STRCPY(m_StartGameRequest.m_HostInfo.m_sName ,pszHostName);
	}

	if (pszPassword)
	{
		m_nClientPass = str_Hash(pszPassword);
	}
	else
	{
		m_nClientPass = 0;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SetupServerSinglePlayer
//
//	PURPOSE:	Setup server for singleplayer game.
//
// ----------------------------------------------------------------------- //
bool ClientMultiplayerMgr::SetupServerSinglePlayer( )
{
	memset(&m_StartGameRequest, 0, sizeof(StartGameRequest));
	m_ServerGameOptions.Clear( );
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));

	// Set the game type to normal (single player).
	m_StartGameRequest.m_Type = STARTGAME_NORMAL;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	m_ServerGameOptions = pProfile->m_ServerGameOptions;
	m_ServerGameOptions.m_bPreCacheAssets = (pProfile->m_sPerformance.nSettings[kPerform_PreCacheAssets] > 0);
	m_ServerGameOptions.m_bPerformanceTest = g_pGameClientShell->IsRunningPerformanceTest();

	return true;
}
// --------------------------------------------------------------------------- //
//
//  ROUTINE:    ClientMultiplayerMgr::SetupServerHost()
//
//  PURPOSE:    Host a game.
//
//	PARAMETERS:	int nPort - Port to use, 0 to use default
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::SetupServerHost( int nPort, bool bLANOnly )
{
	memset( &m_StartGameRequest, 0, sizeof( m_StartGameRequest ));
	m_ServerGameOptions.Clear( );
	memset( &m_NetClientData, 0, sizeof( m_NetClientData ));
	
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	m_ServerGameOptions = pProfile->m_ServerGameOptions;
	m_ServerGameOptions.m_bLANOnly = pProfile->m_ServerGameOptions.m_bLANOnly = bLANOnly;
	m_ServerGameOptions.m_bPreCacheAssets = (pProfile->m_sPerformance.nSettings[kPerform_PreCacheAssets] > 0);

	if( !UpdateNetClientData( ))
		return false;

	m_StartGameRequest.m_Type = STARTGAME_HOST;
	m_StartGameRequest.m_HostInfo.m_Port = nPort;

	switch (g_pGameClientShell->GetGameType())
	{
	case eGameTypeCooperative:
	    m_StartGameRequest.m_HostInfo.m_dwMaxConnections = pProfile->m_ServerGameOptions.GetCoop().m_nMaxPlayers-1;
		LTStrCpy( m_StartGameRequest.m_HostInfo.m_sName, pProfile->m_ServerGameOptions.GetCoop().m_sSessionName.c_str( ),
					sizeof(m_StartGameRequest.m_HostInfo.m_sName));
		break;
	case eGameTypeDeathmatch:
	    m_StartGameRequest.m_HostInfo.m_dwMaxConnections = pProfile->m_ServerGameOptions.GetDeathmatch().m_nMaxPlayers-1;
		LTStrCpy( m_StartGameRequest.m_HostInfo.m_sName, pProfile->m_ServerGameOptions.GetDeathmatch().m_sSessionName.c_str( ),
					sizeof(m_StartGameRequest.m_HostInfo.m_sName));
		break;
	case eGameTypeTeamDeathmatch:
	    m_StartGameRequest.m_HostInfo.m_dwMaxConnections = pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_nMaxPlayers-1;
		LTStrCpy( m_StartGameRequest.m_HostInfo.m_sName, pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_sSessionName.c_str( ),
					sizeof(m_StartGameRequest.m_HostInfo.m_sName));
		break;
	case eGameTypeDoomsDay:
	    m_StartGameRequest.m_HostInfo.m_dwMaxConnections = pProfile->m_ServerGameOptions.GetDoomsday().m_nMaxPlayers-1;
		LTStrCpy( m_StartGameRequest.m_HostInfo.m_sName, pProfile->m_ServerGameOptions.GetDoomsday().m_sSessionName.c_str( ),
					sizeof(m_StartGameRequest.m_HostInfo.m_sName));
		break;
	};


	m_StartGameRequest.m_HostInfo.m_bHasPassword = pProfile->m_ServerGameOptions.m_bUsePassword;
	m_StartGameRequest.m_HostInfo.m_nGameType = (uint8)pProfile->m_ServerGameOptions.m_eGameType;
	
	// Set the name of the mod we are using...
	
	m_ServerGameOptions.m_sModName = GetModName();
	

	// Make sure that the multiplayer mgr doesn't have a server directory in use
	// This must be done because there can only be one IServerDirectory object
	// created at a time for proper shutdown.  (Internal Titan implementation BS...)
	// NYI - Note : This shouldn't be necessary, and if it is, it will cause problems
	// elsewhere.  (Host/Join sequence = 2 objects) Figure out a way to get around this 
	// restriction.
	DeleteServerDir( );

    return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SetDisconnectCode
//
//	PURPOSE:	Sets the disconnection code and message
//
// --------------------------------------------------------------------------- //

void ClientMultiplayerMgr::SetDisconnectCode(uint32 nCode, const char *pMsg)
{
	// Don't override what someone already told us
	if (m_nDisconnectCode)
		return;

	m_nDisconnectCode = nCode;

	if( pMsg )
		m_sDisconnectMsg = pMsg;
	else
		m_sDisconnectMsg.Empty( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::ClearDisconnectCode
//
//	PURPOSE:	Clears the disconnection code and message
//
// --------------------------------------------------------------------------- //

void ClientMultiplayerMgr::ClearDisconnectCode()
{
	m_nDisconnectCode = 0;
	m_sDisconnectMsg.Empty( );
	m_StartGameRequest.m_Type = GAMEMODE_NONE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::CreateServerDir
//
//	PURPOSE:	Creates the client's serverdir for joining a remote game.
//
// --------------------------------------------------------------------------- //

IServerDirectory* ClientMultiplayerMgr::CreateServerDir( )
{
	// Make sure we don't already have one.
	DeleteServerDir( );

	// Get the resource module so we can give it to the serverdir for
	// error messages.
	void* pModule = NULL;
	g_pLTClient->GetEngineHook("cres_hinstance",&pModule);
	HMODULE hModule = (HINSTANCE)pModule;

	m_pServerDir = Factory_Create_IServerDirectory_Titan( true, *g_pLTClient, hModule );
	if( !m_pServerDir )
		return NULL;

	// Set the game's name
	m_pServerDir->SetGameName(g_pVersionMgr->GetNetGameName());
	// Set the version
	m_pServerDir->SetVersion(g_pVersionMgr->GetNetVersion());
	m_pServerDir->SetRegion(g_pVersionMgr->GetNetRegion());
	// Set up the packet header
	CAutoMessage cMsg;
	cMsg.Writeuint8(11); // CMSG_MESSAGE
	cMsg.Writeuint8(MID_MULTIPLAYER_SERVERDIR);
	m_pServerDir->SetNetHeader(*cMsg.Read());

	return m_pServerDir;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::DeleteServerDir
//
//	PURPOSE:	Remove the server dir.
//
// --------------------------------------------------------------------------- //

void ClientMultiplayerMgr::DeleteServerDir( )
{ 
	if( m_pServerDir )
	{
		// No leaking, please...
		delete m_pServerDir; 
		m_pServerDir = NULL; 
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SetCurMessageSource
//
//	PURPOSE:	Set the source address of the message which is currently being processed
//
// --------------------------------------------------------------------------- //

void ClientMultiplayerMgr::SetCurMessageSource(const uint8 aAddr[4], uint16 nPort)
{
	m_aCurMessageSourceAddr[0] = aAddr[0];
	m_aCurMessageSourceAddr[1] = aAddr[1];
	m_aCurMessageSourceAddr[2] = aAddr[2];
	m_aCurMessageSourceAddr[3] = aAddr[3];
	m_nCurMessageSourcePort = nPort;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::GetCurMessageSource
//
//	PURPOSE:	Get the source address of the message which is currently being processed
//
// --------------------------------------------------------------------------- //

void ClientMultiplayerMgr::GetCurMessageSource(uint8 aAddr[4], uint16 *pPort)
{
	aAddr[0] = m_aCurMessageSourceAddr[0];
	aAddr[1] = m_aCurMessageSourceAddr[1];
	aAddr[2] = m_aCurMessageSourceAddr[2];
	aAddr[3] = m_aCurMessageSourceAddr[3];
	*pPort = m_nCurMessageSourcePort;
}


#ifndef SOURCE_RELEASE

#pragma message( "FIXFIX: Remember to remove sections defined without SOURCE_RELEASE defines" )

// Global key used for cross-encryption  NOTE: MUST BE SAME AS ON SERVER!!
#define KEY_MESSER_UPPER 0xDEADFACE

inline uint32 MessUp32BitValue(uint32 nValue, uint32 nRot)
{
	// "Encrypt" the value (Yes, it's a very simple encryption...)
	uint32 nXORShift = nRot & 0xF;
	return ((KEY_MESSER_UPPER << nXORShift) + (KEY_MESSER_UPPER >> (32 - nXORShift))) ^ nValue;
}
#endif // SOURCE_RELEASE

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

bool ClientMultiplayerMgr::OnMessage(uint8 messageID, ILTMessage_Read *pMsg)
{
	// Check inputs.
	if( !pMsg )
	{
		ASSERT( !"ClientMultiplayerMgr::OnMessage: Invalid msg." );
		return false;
	}

	switch(messageID)
	{
		case MID_HANDSHAKE:
			return HandleMsgHandshake( *pMsg );
			break;
		case MID_PLAYER_SINGLEPLAYER_INIT:	
			return HandleMsgPlayerSingleplayerInit	( *pMsg);	
			break;
		case MID_PLAYER_MULTIPLAYER_INIT:	
			return HandleMsgPlayerMultiplayerInit( *pMsg );
			break;
		case MID_MULTIPLAYER_DATA:
			return HandleMsgMultiplayerData( *pMsg );
			break;
		case MID_MULTIPLAYER_SERVERDIR:
			if (GetServerDir())
			{
				char aAddrBuffer[16];
				sprintf(aAddrBuffer, "%d.%d.%d.%d", 
					(uint32)m_aCurMessageSourceAddr[0], 
					(uint32)m_aCurMessageSourceAddr[1], 
					(uint32)m_aCurMessageSourceAddr[2], 
					(uint32)m_aCurMessageSourceAddr[3]);
				return GetServerDir()->HandleNetMessage(*CLTMsgRef_Read(pMsg->SubMsg(pMsg->Tell())), aAddrBuffer, m_nCurMessageSourcePort);
			}
			else
				return false;
			break;
		default:
			break;
		
	}

	return false;
}


bool ClientMultiplayerMgr::HandleMsgHandshake( ILTMessage_Read & msg )
{
	int nHandshakeSub = (int)msg.Readuint8();
	switch (nHandshakeSub)
	{
		case MID_HANDSHAKE_HELLO :
		{
			int nHandshakeVer = (int)msg.Readuint16();
			if (nHandshakeVer != GAME_HANDSHAKE_VER)
			{
				// Disconnect
				m_bForceDisconnect = true;
				SetDisconnectCode(eDisconnect_NotSameGUID,NULL);

				return true;
			}

			// Send back a hello response
			CAutoMessage cResponse;
			cResponse.Writeuint8(MID_HANDSHAKE);
			cResponse.Writeuint8(MID_HANDSHAKE_HELLO);
		    cResponse.Writeuint16(GAME_HANDSHAKE_VER);
			// Send them our secret key
#ifdef SOURCE_RELEASE
			cResponse.Writeuint32( GAME_HANDSHAKE_PASSWORD );
#else // SOURCE_RELEASE
			cResponse.Writeuint32((uint32)this);
#endif // SOURCE_RELEASE
			g_pLTClient->SendToServer(cResponse.Read(), MESSAGE_GUARANTEED);
		}
		break;
		case MID_HANDSHAKE_PASSWORD:
		{
			// Read in their key
			m_nServerKey = msg.Readuint32();
			bool   bNeedPassword = msg.Readbool();

#ifdef SOURCE_RELEASE
			uint32 nPassword = GAME_HANDSHAKE_PASSWORD;
			uint32 nXORMask = GAME_HANDSHAKE_MASK;
#else // SOURCE_RELEASE
			// Get the player name
			char sName[255];
			// (Make sure to clear out the first 4 characters, since that's all we care about)
			sName[0] = sName[1] = sName[2] = sName[3] = 0;
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

			SAFE_STRCPY(sName,pProfile->m_sPlayerGuid.c_str( ));

			// Decrypt it
			m_nServerKey ^= MessUp32BitValue((uint32)this, (uint32)(sName[0]));

			// Encrypt "the string" with it
			uint32 nPassword = *((uint32*)sName);
			uint32 nXORMask = MessUp32BitValue(m_nServerKey, (uint32)(sName[0]));
#endif // SOURCE_RELEASE

			nPassword ^= nXORMask;

			// Game password.
			uint32 nHashedPassword = 0;
			if (bNeedPassword)
			{
				nHashedPassword = m_nClientPass;
			}

			// Get the weapons file CRC
			uint32 nWeaponCRC = g_pWeaponMgr->GetFileCRC();
			// Mask that up too
			nWeaponCRC ^= nXORMask;

			// CRC the modelbutes.txt
			static uint32 nModelButesCRC = CRC32::CalcRezFileCRC( g_pModelButeMgr->GetAttributeFile( ));
			uint32 nModelButesMaskedCRC = nModelButesCRC ^ nXORMask;

			// CRC the surface.txt
			static uint32 nSurfaceCRC = CRC32::CalcRezFileCRC( g_pSurfaceMgr->GetAttributeFile( ));
			uint32 nSurfaceMaskedCRC = nSurfaceCRC ^ nXORMask;

			// CRC the damagefx.txt
			static uint32 nDamageFxCRC = CRC32::CalcRezFileCRC( "attributes\\damagefx.txt" );
			uint32 nDamageFxMaskedCRC = nDamageFxCRC ^ nXORMask;

			// Get the client shell file CRC
			char aClientShellName[MAX_PATH + 1];
			// Just in case getting the file name fails
			aClientShellName[0] = 0; 
			// Get the client shell handle from the engine
			HMODULE hClientShell;
			g_pLTClient->GetEngineHook("cshell_hinstance", (void**)&hClientShell);
			DWORD nResult = GetModuleFileName(hClientShell, aClientShellName, sizeof(aClientShellName));
			uint32 nClientCRC = CRC32::CalcFileCRC(aClientShellName);
			
			// Mask that up too
			nClientCRC ^= nXORMask;

			// Send it back their direction
			CAutoMessage cResponse;
			cResponse.Writeuint8(MID_HANDSHAKE);
			cResponse.Writeuint8(MID_HANDSHAKE_LETMEIN);
			cResponse.Writeuint32(nPassword);
			cResponse.Writeuint32(nWeaponCRC);
			cResponse.Writeuint32(nClientCRC);
			cResponse.Writeuint32(nModelButesMaskedCRC);
			cResponse.Writeuint32(nSurfaceMaskedCRC);
			cResponse.Writeuint32(nDamageFxMaskedCRC);
			cResponse.Writeuint32(nHashedPassword);
			g_pLTClient->SendToServer(cResponse.Read(), MESSAGE_GUARANTEED);

		}
		break;
		case MID_HANDSHAKE_DONE:
		{

			// This just means the server validated us...
		}
		break;
		case MID_HANDSHAKE_WRONGPASS:
		{
			// Oops... wrong password, disconnect
			m_bForceDisconnect = true;
			SetDisconnectCode(eDisconnect_WrongPassword, NULL);
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

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::HandleMsgPlayerSingleplayerInit()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool ClientMultiplayerMgr::HandleMsgPlayerSingleplayerInit (ILTMessage_Read& msg)
{
	return InitSinglePlayer();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::HandleMsgPlayerMultiplayerInit()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool ClientMultiplayerMgr::HandleMsgPlayerMultiplayerInit(ILTMessage_Read& msg)
{
	return InitMultiPlayer();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::HandleMsgMultiplayerData()
//
//	PURPOSE:	Read multiplayer data sent from server.
//
// ----------------------------------------------------------------------- //

bool ClientMultiplayerMgr::HandleMsgMultiplayerData( ILTMessage_Read& msg )
{
    GameType eGameType = ( GameType )msg.Readuint8();
	g_pGameClientShell->SetGameType( eGameType );

	// Check if some joker set us to single player.
	if( eGameType == eGameTypeSingle )
	{
		ASSERT( !"ClientMultiplayerMgr::HandleMsgMultiplayerData: Invalid game type." );
		return false;
	}

	msg.ReadString( m_sServerAddress.GetBuffer( 256 ), 256 );
	m_sServerAddress.ReleaseBuffer( );
	uint32 tmp = msg.Readuint32();
	m_nServerPort = (int)tmp;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::OnEvent()
//
//	PURPOSE:	Called for asynchronous errors that cause the server
//				to shut down
//
// ----------------------------------------------------------------------- //

void ClientMultiplayerMgr::OnEvent(uint32 dwEventID, uint32 dwParam)
{
	if( !IsMultiplayerGame( ))
		return;

	switch(dwEventID)
	{
		// Client disconnected from server.  dwParam will
		// be a error flag found in de_codes.h.
		case LTEVENT_DISCONNECT :
		{
			m_sServerAddress.Empty( );
			m_nServerPort = -1;
			m_sServerName.Empty( );
			m_nServerKey = 0;
		} 
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::Update()
//
//	PURPOSE:	Frame update.
//
// ----------------------------------------------------------------------- //

void ClientMultiplayerMgr::Update( )
{
	// This will happen when something wanted to disconnect, but wasn't
	// in a valid location to do so.  (e.g. when processing packets..)
	if (m_bForceDisconnect)
	{
		g_pLTClient->Disconnect();
		m_bForceDisconnect = false;
		return;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SetService
//
//	PURPOSE:	Selects the connection service for hosting/joining internet games.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::SetService( )
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
//	ROUTINE:	ClientMultiplayerMgr::StartClient
//
//	PURPOSE:	Start a client of a remote server.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::StartClient( )
{

	// Initialize the networking.  Always start a new server with hosted games.
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

	int nRetries = 0;
	while (nRetries >= 0)
	{
		// If successful, then we're done.
		m_nLastConnectionResult = g_pLTClient->StartGame( const_cast< StartGameRequest * >( &m_StartGameRequest ));
		if( m_nLastConnectionResult == LT_OK )
		{
			return true;
		}

		// If we didn't timeout, then there's no reason to try again.
		if( m_nLastConnectionResult != LT_TIMEOUT )
			break;

		// Wait a sec and try again.
		Sleep(1000);
		nRetries--;
	}

	
	return false;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::StartServerHost
//
//	PURPOSE:	Start a hosted game.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::StartServerAsHost( )
{
	// If they want a dedicated server, then launch the serverapp.
	if( m_ServerGameOptions.m_bDedicated )
	{
		if( !g_pGameClientShell->LauncherServerApp( m_ServerGameOptions.m_sProfileName.c_str( )))
			return false;

		return true;
	}

	// Check if we're already connected to a server.
  	if( g_pLTClient->IsConnected( ))
	{
  		// Check if we are already hosting mp.
  		if( IsMultiplayerGame( ) && m_StartGameRequest.m_Type == STARTGAME_HOST )
  		{
  			// Don't need to restart a server.
  			return true;
		}
	}

	// Initialize the networking.  Always start a new server with hosted games.
    m_nLastConnectionResult = g_pLTClient->InitNetworking(NULL, 0);
	if (m_nLastConnectionResult != LT_OK)
	{
        return false;
	}

	// Initialize our protocol.
	if (!SetService())
        return false;

	// Hook up the netgame and clientinfo.
	ServerGameOptions* pServerGameOptions = &m_ServerGameOptions;
	m_StartGameRequest.m_pGameInfo = &pServerGameOptions;
	m_StartGameRequest.m_GameInfoLen = sizeof( pServerGameOptions );
	m_StartGameRequest.m_pClientData = &m_NetClientData;
	m_StartGameRequest.m_ClientDataLen = sizeof( m_NetClientData );

	// Start the server.
	m_nLastConnectionResult = g_pLTClient->StartGame( const_cast< StartGameRequest * >( &m_StartGameRequest ));
	return ( m_nLastConnectionResult == LT_OK );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::StartServerSinglePlayer
//
//	PURPOSE:	Starts the single player server.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::StartServerAsSinglePlayer( )
{
	// Check if we're already connected to a server.
  	if( g_pLTClient->IsConnected( ))
	{
  		// Check if we are already running sp server.
  		if( !IsMultiplayerGame( ) && m_StartGameRequest.m_Type == STARTGAME_NORMAL )
  		{
  			// Don't need to restart a server.
  			return true;
		}
	}

	// Make sure the profile is set.
	m_ServerGameOptions.m_eGameType = g_pGameClientShell->GetGameType( );
	ServerGameOptions* pServerGameOptions = &m_ServerGameOptions;
	m_StartGameRequest.m_pGameInfo = &pServerGameOptions;
	m_StartGameRequest.m_GameInfoLen = sizeof( pServerGameOptions );


	// Start with clean slate
	m_StartGameRequest.m_Type = STARTGAME_NORMAL;

    m_nLastConnectionResult = g_pLTClient->StartGame(&m_StartGameRequest);
	return ( m_nLastConnectionResult == LT_OK );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::StartClientServer
//
//	PURPOSE:	Starts a client/server based on previously set startgamerequest.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::StartClientServer( )
{
	//clear out old session specific data before starting new session
	m_nTeam = INVALID_TEAM;
	m_bHasSelectedTeam = false;

	switch( m_StartGameRequest.m_Type )
	{
		case STARTGAME_NORMAL:
			return StartServerAsSinglePlayer( );
			break;
		case STARTGAME_HOST:
			return StartServerAsHost( );
			break;
		case STARTGAME_CLIENTTCP:
			return StartClient( );
			break;
		default:
			ASSERT( !"ClientMultiplayerMgr::StartClientServer: Invalid gamerequest type." );
			return false;
			break;
	}
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientMultiplayerMgr::SelectTeam()
//
//	PURPOSE:	choose a team.
//
// ----------------------------------------------------------------------- //

void ClientMultiplayerMgr::SelectTeam(uint8 nTeam, bool bPlayerSelected )
{
	if (bPlayerSelected)
		m_bHasSelectedTeam = true;
	m_nTeam = nTeam;
}

// --------------------------------------------------------------------------- //
//
//  ROUTINE:    ClientMultiplayerMgr::UpdateNetClientData
//
//  PURPOSE:    Updates the NetClientData to reflect current settings.
//
// --------------------------------------------------------------------------- //

bool ClientMultiplayerMgr::UpdateNetClientData( )
{
	// Setup our client...
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if( !pProfile )
	{
		ASSERT( !"ClientMultiplayerMgr::UpdateNetClientData: Invalid profile." );
		return false;
	}

// Don't allow this in the final version because of invalid characters

#ifndef _FINAL

	// Check if the user overrode the player name on the command line.
	char szPlayerNameOverride[MAX_PLAYER_NAME] = "";
	LTStrCpy( szPlayerNameOverride, GetConsoleTempString( "playername", "" ), ARRAY_LEN( szPlayerNameOverride ));
	if( szPlayerNameOverride[0] )
	{
		pProfile->m_sPlayerName = szPlayerNameOverride;
	}

#endif

	LTStrCpy(m_NetClientData.m_szName,pProfile->m_sPlayerName.c_str(), ARRAY_LEN( m_NetClientData.m_szName ));

	// Setup the name guid.
	SAFE_STRCPY(m_NetClientData.m_szPlayerGuid,pProfile->m_sPlayerGuid.c_str( ));

	// Setup the model.
	switch (g_pGameClientShell->GetGameType())
	{
		case eGameTypeCooperative:
			m_NetClientData.m_ePlayerModelId = g_pModelButeMgr->GetCPModel( pProfile->m_nCPPlayerModel );
			break;
		case eGameTypeDeathmatch:
			m_NetClientData.m_ePlayerModelId = g_pModelButeMgr->GetDMModel( pProfile->m_nDMPlayerModel );
			break;
	};

	return true;	
}