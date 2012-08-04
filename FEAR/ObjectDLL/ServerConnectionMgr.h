// ----------------------------------------------------------------------- //
//
// MODULE  : ServerConnectionMgr.h
//
// PURPOSE : Definition/Implementation of the server client connection manager.
//
// CREATED : 06/22/04
//
// (c) 1996-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SERVERCONNECTIONMGR_H_
#define _SERVERCONNECTIONMGR_H_

#include "EventCaster.h"
#include "SharedScoring.h"

#define MAX_CLIENT_NAME_LENGTH		100

class ConnectionStateMachine;

// ----------------------------------------------------------------------- //
//
// class GameClientData
//
// Definition/Implementation class that represents client on the server.
//
// ----------------------------------------------------------------------- //
class GameClientData : public ILTObjRefReceiver
{
	public:

		GameClientData( HCLIENT hClient );
		~GameClientData( );

	public:

		bool Init( );

		void Update( );
		bool OnMessage( HCLIENT hClient, ILTMessage_Read& msg );

		HCLIENT GetClient( ) const { return m_hClient; }

		HOBJECT GetPlayer( ) const { return m_hPlayer; }

		// Directly sets the current player.
		void SetPlayer( HOBJECT hPlayer ) { m_hPlayer = hPlayer; PlayerSwitched.DoNotify(); }
		// Creates a player to switch to.
		bool CreateStandbyPlayer( );
		// Switches to the previously created standby player.
		bool SwitchToStandbyPlayer( );

		void SetUniqueName( wchar_t const* pszUniqueName ) { m_sUniqueName = pszUniqueName ? pszUniqueName : L""; }
		wchar_t const* GetUniqueName( ) const { return m_sUniqueName.c_str(); }

		bool SetClientConnectionState( EClientConnectionState eClientConnectionState );
		EClientConnectionState GetClientConnectionState( ) const;

		void SetLastClientUpdateTimeMS( uint32 nTimeMS ) { m_nLastClientUpdateTimeMS = nTimeMS; }
		uint32 GetLastClientUpdateTimeMS( ) const { return m_nLastClientUpdateTimeMS; }


		void SetLastTeamId( uint8 nTeamId ) { m_nLastTeamId = nTeamId; }
		uint8 GetLastTeamId( ) const { return m_nLastTeamId; }

		void	SetRequestedTeam( uint8 nTeamId ) { m_nRequestedTeam = nTeamId; }
		uint8	GetRequestedTeam( ) const { return m_nRequestedTeam; }
		bool	RequestedTeamChange() {return (m_nRequestedTeam != GetLastTeamId( )); }

		void SetKickTime( uint32 nTime ) { m_nKickTime = nTime; }
		uint32 GetKickTime( ) const { return m_nKickTime; }

		void SetHasPassedSecurity( bool bPassedSecurity ) { m_bPassedSecurity = bPassedSecurity; }
		bool HasPassedSecurity( ) const { return m_bPassedSecurity; }
		
		bool SetNetClientData( NetClientData const& netClientData );
		bool GetNetClientData( NetClientData& netClientData ) const;

		bool IsClientInWorld( ) const { return m_bClientInWorld; }
		void SetClientInWorld( bool bValue ) 
		{ 
			m_bClientInWorld = bValue; 
			if( !m_bClientInWorld )
			{
				m_hPlayer = NULL; 
				m_hStandbyPlayer = NULL; 
				m_Score.Init( g_pLTServer->GetClientID( GetClient() ) );
			}
		}

		uint8 GetLoadout( ) const { return m_nLoadout; }
		void SetLoadout( uint8 nLoadout ) { m_nLoadout = nLoadout; }

		uint8 GetMPModelIndex( ) const;
		void SetMPModelIndex( uint8 nModelIndex );
		ModelsDB::HMODEL GetMPModel( ) const;

		const char* GetInsignia() const { return m_sInsignia.c_str(); }
		void SetInsignia( const char* szInsignia);

		CPlayerScore* GetPlayerScore() { return &m_Score; }

		void SetAllowPreSaleContent( bool bValue ) { m_bAllowPreSaleContent = bValue; }
		bool GetAllowPreSaleContent( ) const { return m_bAllowPreSaleContent; }

		// Event to fire when client is removed.
		DECLARE_EVENT( RemoveClient );
		// Event to fire when client switches players.
		DECLARE_EVENT( PlayerSwitched );

		void	AddProximityMine(HOBJECT hProx);
		void	OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Stores last start point used.
		void	SetLastStartPoint( HOBJECT hStartPoint ) { m_hLastStartPoint = hStartPoint; }
		HOBJECT	GetLastStartPoint( ) const { return m_hLastStartPoint; }

		// Accessor to whether client is ready to play or not.
		bool	IsReady( ) const;

		// Accessors to client move code, a frame code to make sure client movement
		// messages are processed in order.
		void	SetClientMoveCode( uint8 nClientMoveCode ) { m_nClientMoveCode = nClientMoveCode; }
		uint8	GetClientMoveCode( ) const { return m_nClientMoveCode; }

		void	SetIsPunkBusterEnabled( const bool bIsPunkBusterEnabled ) { m_bIsPunkBusterEnabled = bIsPunkBusterEnabled; }
		bool	GetIsPunkBusterEnabled() const { return m_bIsPunkBusterEnabled; }


	private:

		HCLIENT					m_hClient;
		LTObjRef				m_hPlayer;
		LTObjRef				m_hStandbyPlayer;
		EClientConnectionState	m_eClientConnectionState;	
		uint32					m_nLastClientUpdateTimeMS;
		std::wstring			m_sUniqueName;
		uint8					m_nLastTeamId;

		uint32					m_nKickTime;
		bool					m_bPassedSecurity;

		// Received from client when they reach the in world state of the connection.
		bool					m_bClientInWorld;

		// this will be a valid team id if the player has requested to change teams
		uint8		m_nRequestedTeam;

		// this will specify the weapon loadout that the player requested
		uint8		m_nLoadout;

		// State machine that client connecting to server.
		ConnectionStateMachine*	m_pConnectionStateMachine;

		CPlayerScore		m_Score;

		std::string			m_sInsignia;

		ObjRefNotifierList m_ProximityMines;					//proxies that we own

		// The last start point we used.
		LTObjRef	m_hLastStartPoint;

		bool m_bAllowPreSaleContent;

		// Frame code sent to client to make sure client's movement messages are in correct order.
		uint8	m_nClientMoveCode;

		// client's PunkBuster state
		bool	m_bIsPunkBusterEnabled;

};

// ----------------------------------------------------------------------- //
//
// class ServerConnectionMgr
//
// Definition/Implementation of the ServerConnectionMgr class.  
//
// ----------------------------------------------------------------------- //
class ServerConnectionMgr
{
	DECLARE_SINGLETON( ServerConnectionMgr );

	public:

		// Frame update.
		void			Update( );
        
		// Gets the clientdata given a HCLIENT.
		GameClientData* GetGameClientData( HCLIENT hClient );
	
		// Gets the clientdata given a client Id
		GameClientData* GetGameClientDataByClientId( uint32 nClientId );


		void			OnAddClient( HCLIENT hClient );
		void			OnRemoveClient( HCLIENT hClient );

		bool			OnMessage(HCLIENT hSender, ILTMessage_Read& msg);

		bool			PostStartWorld( );

		typedef std::vector< GameClientData* > GameClientDataList;
		GameClientDataList& GetGameClientDataList( ) { return m_GameClientDataList; }

		// Check to see if the client's connection is showing trouble.
		bool ClientConnectionInTrouble( HCLIENT hClient );

		// Generates a unique name based on existing clients.
		void GenerateUniqueName( HCLIENT hClient, wchar_t const* pszNameBase, wchar_t* pszUniqueName, int nUniqueNameSize ) const;

		// get client inactivity timeout period
		uint32 GetAutoBootTimeMS( ) const { return m_nAutoBootTimeMS; }

		// Boots the client and sends the reason to them.
		bool BootWithReason( GameClientData& gameClientData, EClientConnectionError eConnectionError, const char* pszMessage );

	private:

		// Gets the iterator to a gameclientdata based on HCLIENT;
		GameClientDataList::iterator GetGameClientDataIter( HCLIENT hClient );

		// Updates clients waiting for authorization to join.
		bool UpdateClientsWaitingForAuth( );

		// Iterate through list of clients, kicking any that are in trouble.
		bool KickBadClients( );

		// Checks if a unique name is already taken.
		bool IsNameTaken( wchar_t const* pszName, HCLIENT hSkipClient ) const;

		// Handle MID_CLIENTCONNECTION messages.
		void HandleClientConnection(HCLIENT hClient, ILTMessage_Read& msg);

	private:

		// List of clients currently connected.
		GameClientDataList	m_GameClientDataList;

		// slot-based index into the list of game client data
		GameClientData**	m_ppGameClientDataSlotArray;

		// maximum number of players allowed by the server
		uint32 m_nMaxPlayers;

		// client inactivity period timeout
		uint32 m_nAutoBootTimeMS;

};

#endif
