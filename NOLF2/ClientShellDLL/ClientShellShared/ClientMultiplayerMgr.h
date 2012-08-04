// ----------------------------------------------------------------------- //
//
// MODULE  : ClientMultiplayerMgr.h
//
// PURPOSE : Clientside multiplayer mgr - Declaration
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

class IServerDirectory;


enum eDisconnectCodes
{
	eDisconnect_None = 0,
	eDisconnect_NotSameGUID,
	eDisconnect_WrongPassword
};


class ClientMultiplayerMgr
{
public:
	
	ClientMultiplayerMgr( );
	~ClientMultiplayerMgr( );

	// Update called once per frame.
	void		Update( );

	bool		InitMultiPlayer();
	bool		UpdateMultiPlayer();
	bool		UpdateNetClientData( );
	bool		InitSinglePlayer();

	// Disconnect on the next update
	void		ForceDisconnect() {	m_bForceDisconnect = true;}	


	bool		SetupClient(char const* pszIpAddress, char const* pszHostName, char const* pszPassword);

	// Setup the server for singleplayer.
	bool		SetupServerSinglePlayer( );

	// Setup the server to host a multiplayer game.
	bool		SetupServerHost( int nPort, bool bLanOnly );

	// Handles engine event.
	void		OnEvent(uint32 dwEventID, uint32 dwParam);

	// Handles message from engine.
	bool		OnMessage(uint8 messageID, ILTMessage_Read *pMsg);


	void		DoTaunt(uint32 nClientID,uint32 nTaunt);


	// Sets the networking service.
	bool		SetService( );

	// Accessors to setup server information.
	StartGameRequest& GetStartGameRequest( ) { return m_StartGameRequest; }
	ServerGameOptions&	GetServerGameOptions( ) { return m_ServerGameOptions; }
	NetClientData&	GetNetClientData( ) { return m_NetClientData; }

	// Start a client of a server based on previously set startgamerequest.
	bool		StartClientServer( );


	// This client is connected to remote server.
	bool		IsConnectedToRemoteServer( ) { return ( m_StartGameRequest.m_Type == STARTGAME_CLIENTTCP ); }

	//returns true if the passed in address matches the current server address
	bool		CheckServerAddress(char const*pszTestAddress, int nPort);
	// Called when the engine wants to tell the game a disconnection code (a.k.a. hack)
	virtual void SetDisconnectCode(uint32 nCode, const char *pMsg);
	// Internal game-side support for the disconnection code
	void		ClearDisconnectCode();
	uint32		GetDisconnectCode() { return m_nDisconnectCode; }
	const char *GetDisconnectMsg() { return m_sDisconnectMsg; }

	const char *GetServerAddress() const {return m_sServerAddress;}
	const char *GetServerName() const {return m_sServerName;}

	// Server directory access
	// Note : The server directory is owned by this object once you pass it in.
	IServerDirectory* GetServerDir() { return m_pServerDir; }
	IServerDirectory* CreateServerDir( );
	void		DeleteServerDir( );

	// Get/Set the source address of the message which is currently being processed
	void		SetCurMessageSource(const uint8 aAddr[4], uint16 nPort);
	void		GetCurMessageSource(uint8 aAddr[4], uint16 *pPort);

	LTRESULT	GetLastConnectionResult() {return m_nLastConnectionResult;}

	void		SelectTeam(uint8 nTeam, bool bPlayerSelected);
	bool		HasSelectedTeam() { return m_bHasSelectedTeam; }

	void		SetModName( const char *pszModName ) { m_sModName = pszModName; }
	const char*	GetModName( ) const { return m_sModName.c_str(); }
	
protected :

	bool		HandleMsgHandshake( ILTMessage_Read& msg );
	bool		HandleMsgMultiplayerData( ILTMessage_Read& pMsg );
	bool		HandleMsgPlayerMultiplayerInit( ILTMessage_Read& msg );
	bool		HandleMsgPlayerSingleplayerInit (ILTMessage_Read& msg);

	bool		StartServerAsSinglePlayer( );
	bool		StartServerAsHost( );
	bool		StartClient( );

private:

	// Must be filled out before StartLocalGame, StartNetworkGameAsXXX calls.
	StartGameRequest	m_StartGameRequest;
	ServerGameOptions	m_ServerGameOptions;
	NetClientData	m_NetClientData;


	//these refer to the server we are currently connected to
	CString		m_sServerAddress;
	int			m_nServerPort;
	CString		m_sServerName;

	//used for handshaking with server
	uint32		m_nServerKey;
	uint32		m_nClientPass;

	// Disconnection code/msg storage
	uint32		m_nDisconnectCode;
	CString		m_sDisconnectMsg;

	LTRESULT	m_nLastConnectionResult;

	// Connection handling
	bool			m_bForceDisconnect;	// Set this flag to disconnect on the next update

		// Server directory interface
	IServerDirectory*	m_pServerDir;

	// Current message source
	uint8		m_aCurMessageSourceAddr[4];
	uint16		m_nCurMessageSourcePort;


	uint8	m_nTeam;
	bool	m_bHasSelectedTeam;
	
	// The name of our selected mod...
	std::string		m_sModName;
	
};

extern ClientMultiplayerMgr* g_pClientMultiplayerMgr;

#endif // __CLIENTMULTIPLAYERMGR_H_

