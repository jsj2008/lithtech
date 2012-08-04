// ----------------------------------------------------------------------- //
//
// MODULE  : ClientConnectionMgr.h
//
// PURPOSE : Clientside connection mgr - Declaration
//
// CREATED : 02/05/02
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTMULTIPLAYERMGR_H_
#define __CLIENTMULTIPLAYERMGR_H_

#include "VarTrack.h"
#include "NetDefs.h"
#include "ProfileUtils.h"
#include "iltclientcontenttransfer.h"
#include "iltcontenttransfernotification.h"

class IGameSpyBrowser;


enum eDisconnectCodes
{
	eDisconnect_None = 0,
	eDisconnect_NotSameGUID,
	eDisconnect_WrongPassword,
	eDisconnect_OverridesFailed,
	eDisconnect_ContentTransferFailed,
	eDisconnect_InvalidAssets,
	eDisconnect_BadCdKey,
	eDisconnect_Banned,
	eDisconnect_TimeOut,
	eDisconnect_PunkBuster,
};


// Controls state machine of connecting with a server
enum ESocketConnectionState
{
	eSocketConnectionState_Disconnected,
	eSocketConnectionState_NatNeg,
	eSocketConnectionState_SettleComm,
	eSocketConnectionState_Connecting,
	eSocketConnectionState_Connected,
	eSocketConnectionState_Failure,
	eSocketConnectionState_Aborted
};

class ClientConnectionMgr : public ILTContentTransferNotification
{
public:
	
	ClientConnectionMgr( );
	~ClientConnectionMgr( );

	// Update called once per frame.
	void		Update( );

	bool		InitMultiPlayer();
	bool		SendMultiPlayerInfo();
	bool		InitSinglePlayer();

	// send the keep alive message to the server
	void		SendKeepAliveMessage();

	// Disconnect on the next update
	void		ForceDisconnect() {	m_bForceDisconnect = true;}	

	// Pass the pNatNegBrowser if server requires NAT Negotiations.
	bool		SetupClient(wchar_t const* pszHostName, wchar_t const* pszPassword,
							bool bDoNatNegotiations, bool bConnectViaPublic, 
							char const* pszPublicAddress, char const* pszPrivateAddress );

	// Setup the server for singleplayer.
	bool		SetupServerSinglePlayer( );

	// Setup the server to host a multiplayer game.
	bool		SetupServerHost( );

	// Handles engine event.
	void		OnEvent(uint32 dwEventID, uint32 dwParam);

	// Handles message from engine.
	bool		OnMessage(uint8 messageID, ILTMessage_Read *pMsg);

	//Handles world change while still on same server
	void		OnExitWorld();


	void		DoBroadcast(uint32 nClientID,const char* szBroadcastID);


	// Accessors to setup server information.
	StartGameRequest& GetStartGameRequest( ) { return m_StartGameRequest; }
	NetClientData&	GetNetClientData( ) { return m_NetClientData; }

	// Start a client of a server based on previously set startgamerequest.
	bool		StartClientServer( );


	// Handle connecting to a server from specific IP.
	bool		DoConnectToIP( char const* pszIPandPort, wchar_t const* pwszPassword, bool bDoNatNegotiations );

	// This client is connected to remote server.
	bool		IsConnectedToRemoteServer( ) const { return ( m_StartGameRequest.m_Type == STARTGAME_CLIENT ); }
	bool		IsConnectedToLANServer( ) const { return ( m_bClientLAN ); }
	bool		IsClientLoggedIn() const {return m_bClientLoggedIn; }

	//returns true if the passed in address matches the current server address
	bool		CheckServerAddress(char const*pszTestAddress, int nPort);
	// Called when the engine wants to tell the game a disconnection code (a.k.a. hack)
	virtual void SetDisconnectCode(uint32 nCode, const char *pMsg);
	// Internal game-side support for the disconnection code
	void		ClearDisconnectCode();
	uint32		GetDisconnectCode() { return m_nDisconnectCode; }
	const char *GetDisconnectMsg() { return m_sDisconnectMsg.c_str(); }
	
	// accessors for content transfer errors
	const char*    GetContentTransferErrorFilename() { return m_strContentTransferErrorFilename.c_str(); }
	const wchar_t* GetContentTransferErrorServerMessage() { return m_wstrContentTransferErrorServerMessage.c_str(); }

	const char *GetServerAddress() const {return m_sServerAddress.c_str();}
	const wchar_t *GetServerName() const {return m_sServerName.c_str();}

	LTRESULT	GetLastConnectionResult() {return m_nLastConnectionResult;}

	void		SelectTeam(uint8 nTeam, bool bPlayerSelected);
	bool		HasSelectedTeam() const { return m_bHasSelectedTeam; } 

	void		SelectLoadout(uint8 nLoadout);
	bool		HasSelectedLoadout() const { return m_bHasSelectedLoadout; } 
	uint8		GetLoadout() const { return m_nLoadout; }

	void		SetModName( const char *pszModName ) { m_sModName = pszModName; }
	const char*	GetModName( ) const { return m_sModName.c_str(); }

	NetGameInfo& GetNetGameInfo( ) { return m_NetGameInfo; }

	// Get retail server browser.
	IGameSpyBrowser* GetServerBrowser( ) { return m_pGameSpyBrowser; }
	// Creates the server browser.
	bool		CreateServerBrowser( );
	// Deletes the browser.
	void		TermBrowser( );

	// Gets the connection state sent from the server.
	EClientConnectionState GetRecvClientConnectionState( ) const { return m_eRecvClientConnectionState; }
	void SetSentClientConnectionState( EClientConnectionState eClientConnectionState ) { m_eSentClientConnectionState = eClientConnectionState; }
	EClientConnectionState GetSentClientConnectionState( ) const { return m_eSentClientConnectionState; }

	bool IsConnectionInProgress() const { return (m_eSocketConnectionState == eSocketConnectionState_Connecting); }

	uint8		GetTeam( ) const { return m_nTeam; }

	uint8		GetCurrentRound( ) const { return m_nCurrentRound; }

	// Called when done with loading and postloading screens.
	void		SendClientInWorldMessage( );

	// disconnects the client from the server
	void DisconnectFromServer();

	// Setup the bandwidth settings based on lan.
	void SetupClientBandwidth( bool bLan );

	bool LoadMultiplayerOverrides(uint8* pDecompressedOverridesData, uint32 nDecompressedOverridesSize);
	void UnloadMultiplayerOverrides();	

	// ILTContentTransferNotification implementation
	virtual void ClientContentTransferStartingNotification(uint32 nTotalBytes,
														   uint32 nTotalFiles);

	virtual void FileReceiveProgressNotification(const std::string& strFilename,
												 uint32				nFileBytesReceived,
												 uint32				nFileBytesTotal,
												 uint32				nTransferBytesTotal,
												 float				fTransferRate);

	virtual void FileReceiveCompletedNotification(const std::string& strFilename,
												  uint32			 nFileBytesTotal,
												  uint32			 nTransferBytesTotal,
												  float				 fTransferRate);

	virtual void ClientContentTransferErrorNotification(EContentTransferError eError,
														const char*			  pszFilename,
						  							    const wchar_t*        pwszMessage);

	virtual void ClientContentTransferCompletedNotification();
	virtual bool ClientContentTransferInProgress() const {return m_bClientContentTransferInProgress;}

	DECLARE_EVENT( ClientLoggedInEvent );

protected :

	bool		HandleMsgClientConnection( ILTMessage_Read& msg );
	bool		HandleMsgMultiplayerData( ILTMessage_Read& pMsg );

	bool		StartServerAsSinglePlayer( );
	bool		StartServerAsHost( );
	bool		StartClient( );

	void		UpdateSocketConnectionState_NatNeg( );
	void		UpdateSocketConnectionState_SettleComm( );
	void		UpdateSocketConnectionState_Connecting( );

	// Sets the networking service.
	bool		SetService( );

	bool		SetNetClientData( );

	// Update the state machine.
	void		UpdateSocketConnectionState( );
	ESocketConnectionState GetSocketConnectionState( ) const { return m_eSocketConnectionState; }
	
	void		SetContentTransferOptions();

private:

	// Must be filled out before StartLocalGame, StartNetworkGameAsXXX calls.
	StartGameRequest	m_StartGameRequest;
	NetClientData		m_NetClientData;
	NetGameInfo			m_NetGameInfo;

	//these refer to the server we are currently connected to
	std::string	m_sServerAddress;
	int			m_nServerPort;
	std::wstring	m_sServerName;

	//used for handshaking with server
	uint32		m_nServerKey;
	uint32		m_nClientPass;

	// Disconnection code/msg storage
	uint32		m_nDisconnectCode;
	std::string	m_sDisconnectMsg;

	LTRESULT	m_nLastConnectionResult;

	// content transfer related error information
	std::string	 m_strContentTransferErrorFilename;
	std::wstring m_wstrContentTransferErrorServerMessage;
	bool		m_bClientContentTransferInProgress;
	
		
	// Connection handling
	bool			m_bForceDisconnect;	// Set this flag to disconnect on the next update

	uint8	m_nTeam;
	bool	m_bHasSelectedTeam;

	uint8	m_nLoadout;
	bool	m_bHasSelectedLoadout;
	
	// The name of our selected mod...
	std::string		m_sModName;

	// Gamespy browser.
	IGameSpyBrowser* m_pGameSpyBrowser;

	// Should do natnegotions on next client connect.
	bool m_bDoNatNegotiations;

	// The client connection state received from the server.
	EClientConnectionState m_eRecvClientConnectionState;
	// The client connection state sent to the server.
	EClientConnectionState m_eSentClientConnectionState;

	ESocketConnectionState m_eSocketConnectionState;

	bool					m_bClientLoggedIn;

	// Used to let comm settle after natneg.
	StopWatchTimer m_SettleCommTimer;

	// Try initial connect through public address.
	bool m_bConnectViaPublic;

	// public address to try to connect to.
	std::string m_sConnectPublicAddress;
	// private address to try to connect to.
	std::string m_sConnectPrivateAddress;

	// databases for handling overrides
	HDATABASE m_hGameDatabase;
	HDATABASE m_hOverridesDatabase;

	// Times when we should send a tickle message to keep our connection alive.
	StopWatchTimer m_GameTickleTimer;

	//are we connected to a LAN server
	bool	m_bClientLAN;

	// Current round reported by server.
	uint8	m_nCurrentRound;
};

extern ClientConnectionMgr* g_pClientConnectionMgr;

#endif // __CLIENTMULTIPLAYERMGR_H_

