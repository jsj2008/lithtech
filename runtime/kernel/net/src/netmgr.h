#ifndef __NETMGR_H__
#define __NETMGR_H__


#define INVALID_CONNID	NULL


class CBaseDriver;

#ifndef __RATETRACKER_H__
#include "ratetracker.h"
#endif

#ifndef __SYSCOUNTER_H__
#include "syscounter.h"
#endif

#ifndef __OBJECT_BANK_H__
#include "object_bank.h"
#endif

#include "packet.h"

#include <deque>

// How often it sends a 'sync packet' so the other computer can flush its lists...
#define SYNCPACKET_FREQUENCY	30	

// CNetMgr flags.
#define NETMGR_GETTINGPACKETS		1


// How many padding bytes CNetMgr adds to packets (if possible).
#define DRIVER_PADDING	2


// Connection flags.
#define CONNFLAG_LOCAL				(1<<0)



class LatentPacket : public CGLLNode
{
public:
	float			m_SendTimeCounter;
	CPacket_Read	m_cPacket;
	uint32			m_nPacketFlags;
};


class CQueuedPacket : public CGLLNode
{
public:
	CQueuedPacket() : m_bGuaranteed(false) {}

	CPacket_Read m_cPacket;

	bool			m_bGuaranteed;
};
typedef CGLinkedList<CQueuedPacket*> QueuedPacketList;


// This is the engine's tcp/ip driver.
#define NETDRIVER_TCPIP		(1<<0)



// This is the base data structure the internal stuff uses
// to represent a net connection.
class CBaseConn
{
	public:

						CBaseConn();
						~CBaseConn();
		
		virtual bool	IsInTrouble() { return false; }

		// Returns the number of available bits over the given time period, based on the connection's bandwidth
		// The number is negative if the send queue will be overflowing the connection for that time period
		virtual int32		GetAvailableBandwidth(float fTime) const { return 0x7FFFFFFF; }
		// Returns the number of bits per second which can be sent by this connection (in theory...)
		virtual uint32		GetBandwidth() const { return 0x7FFFFFFF; }
		// Changes the bandwidth limits on this connection
		// Returns false if that is not supported on this connection
		virtual bool		SetBandwidth(uint32 nBPS) { return false; }
		// Get the average bandwidth usage of this connection
		virtual uint32		GetOutgoingBandwidthUsage() const { return 0; }
		virtual uint32		GetIncomingBandwidthUsage() const { return 0; }
		// Overhead of networking (lost packets, network headers, etc.)
		virtual uint32		GetTransportOverhead() const { return 0; }

		// Returns the average packet loss for this connection
		virtual float		GetPacketLoss() const { return 0.0f; }
		
		// BPS=bytes per second, PPS=packets per second
		RateTracker		m_SendBPS;	
		RateTracker		m_SendPPS;
		RateTracker		m_RecvBPS;
		RateTracker		m_RecvPPS;

		// Driver this conn is connected thru.
		CBaseDriver		*m_pDriver;

		// Outgoing latent packet queue
		CGLinkedList<LatentPacket*>	m_Latent;

		// Connection flags.
		uint32			m_ConnFlags;

	// Ping stuff.
	public:
		
		virtual float	GetPing() { return 0.0f; }

	// Information functions
	public:
		
		// Get the IP address associated with this connection
		// Returns false if there is no IP address associated with this connection
		virtual bool GetIPAddress(uint8 pAddr[4], uint16 *pPort) { return false; }
};

enum EDisconnectReason { DISCONNECTREASON_VOLUNTARY_CLIENTSIDE =0,
                         DISCONNECTREASON_LOCALDRIVER =1,
                         DISCONNECTREASON_CONNFLAG_FORCEDISCONNECT =2,
                         DISCONNECTREASON_DEAD =3,
                         DISCONNECTREASON_SHUTDOWN =4,
                         DISCONNECTREASON_VOLUNTARY_SERVERSIDE =5,
                         DISCONNECTREASON_KICKED =6 };

class BaseService;
// Base class for a network driver (such as DirectPlay).
class CBaseDriver
{
	friend class CNetMgr;
	
	
	public:

							CBaseDriver()
							{
								m_DriverFlags = 0;
							}

		virtual				~CBaseDriver() {}

		virtual bool		Init()=0;
		virtual void		Term()=0;

		virtual void		Update()=0;

		virtual void		LocalConnect(CBaseDriver *pOther) {ASSERT(false);}

		// Service list accessors.
		virtual	LTRESULT	GetServiceList(NetService* &pListHead) { return LT_ERROR; }

		// Returns true if we are able to select the given service.
		virtual	LTRESULT	SelectService(BaseService *pService) { return LT_ERROR; }

		// Session list accessors.
		virtual LTRESULT	GetSessionList(NetSession* &pListHead, const char *pInfo) { return LT_ERROR; }

		virtual LTRESULT	StartQuery(const char *pInfo) {return LT_ERROR;}
		virtual LTRESULT	UpdateQuery() {return LT_ERROR;}
		virtual LTRESULT	GetQueryResults(NetSession* &pListHead) {return LT_ERROR;}
		virtual LTRESULT	EndQuery() {return LT_ERROR;}

		// Hosts a session with the given info.  Possibly fills in hSession (the internet
		// driver doesn't fill it in but returns LT_OK).
		virtual	LTRESULT	HostSession(NetHost* pHost) {return LT_ERROR;}

		// Joins the given session.
		virtual LTRESULT	JoinSession(NetSession *pSession) { return LT_ERROR; }

		// Updates the sessions' name (only the host can do this).
		virtual LTRESULT	SetSessionName(const char* sName) { return LT_ERROR; }

		// Gets sessions' name.
		virtual LTRESULT	GetSessionName(char* sName, uint32 dwBufferSize) { return LT_ERROR; }

		// Gets sessions' maximum number of remote clients.
		virtual LTRESULT	GetMaxConnections(uint32 &nMaxConnections) { return LT_ERROR; }

		// Determines if we were lobby launched.
		virtual	bool		IsLobbyLaunched() { return(false); }

		// Gets the lobby launch info if available.
		virtual	bool		GetLobbyLaunchInfo(void** ppLobbyLaunchData) { return(false); }

		// Hosts a lobby launched session with the given info.
		virtual	LTRESULT	HostLobbyLaunchSession(NetHost* pHost) {return LT_ERROR;}

		// Joins the given session.
		virtual bool		JoinLobbyLaunchSession() { return(false); }

		// Gets the tcp/ip address if available.
		virtual	bool		GetLocalIpAddress(char* sBuffer, uint32 dwBufferSize, uint16 &hostPort) { return(false); }

		// Joins the first session on the given tcp/ip address...
		virtual LTRESULT	ConnectTCP(const char* sAddress) { return(false); }

		virtual LTRESULT	SendTcpIp(const CPacket_Read &cMsg, const char *sAddr, uint32 port) {return LT_ERROR;}

		virtual LTRESULT	StartPing(const char *pAddr, uint16 nPort, uint32 *pPingID) { return LT_ERROR; }
		virtual LTRESULT	GetPingStatus(uint32 nPingID, uint32 *pStatus, uint32 *pLatency) { return LT_ERROR; }
		virtual LTRESULT	RemovePing(uint32 nPingID) { return LT_ERROR; }

		// Update the GUID
		virtual void		UpdateGUID(LTGUID &cGUID) { }

	public:

		char				m_Name[64];

		// Combination of NETDRIVER_ flags.
		uint32				m_DriverFlags;

		// MUST call the net mugger's DisconnectNotify.
		virtual void		Disconnect( CBaseConn *id, EDisconnectReason reason )=0;

		// Driver-level functions.
		virtual bool		SendPacket(const CPacket_Read &cPacket, CBaseConn *idSendTo, bool bGuaranteed)=0;
		virtual bool		GetPacket(CPacket_Read *pPacket, CBaseConn **pSender)=0;

		CNetMgr				*m_pNetMgr;
};


class BaseService
{
public:
	virtual	~BaseService() {}

	CBaseDriver	*m_pDriver; // Where it came from.
};



// Drivers should derive from this for their sessions so the NetMgr knows what
// driver the session came from.
class NetMgrSession : public NetSession
{
public:
				NetMgrSession(CBaseDriver *pDriver)
				{
					m_pDriver = pDriver;
				}

	virtual		~NetMgrSession() {}

	CBaseDriver	*m_pDriver;
};


class CNetHandler
{
	public:

		virtual			~CNetHandler() {}

		// Return true to accept connection.  false to ignore.
		virtual bool	NewConnectionNotify(CBaseConn *id, bool bIsLocal)=0;
		virtual void	DisconnectNotify(CBaseConn *id, EDisconnectReason eDisconnectReaseon)=0;
		
		// If an unknown packet comes in on tcp/ip (unknown sender and unknown packet ID)
		// this is called).
		virtual void	HandleUnknownPacket(const CPacket_Read &cPacket, uint8 senderAddr[4], uint16 senderPort)=0;

		// Called when a disconnection event occurs
		virtual void	SetDisconnectCode(uint32 nCode, const char *pMsg) {};
};


#define NETMGR_TRAVELDIR_UNKNOWN			0
#define NETMGR_TRAVELDIR_SERVER2CLIENT		1
#define NETMGR_TRAVELDIR_CLIENT2SERVER		2


class CNetMgr
{
	friend class CBaseConn;

	public:
						CNetMgr();
						~CNetMgr();

		bool			Init(char *pPlayerName);
		void			Term();
		
		// Creates all the necessary drivers.
		LTRESULT			InitDrivers(); 
		void			TermDrivers();

		LTRESULT		GetServiceList(NetService* &pListHead);
		LTRESULT		FreeServiceList(NetService *pListHead);
		LTRESULT		SelectService(HNETSERVICE hService);
		
		LTRESULT		GetSessionList(NetSession* &pListHead, const char *pInfo);
		LTRESULT		FreeSessionList(NetSession *pListHead);

		LTRESULT		GetSessionName(char *pName, uint32 bufLen);
		LTRESULT		SetSessionName(const char *pName);

		LTRESULT		GetLocalIpAddress(char *pAddress, uint32 bufLen, uint16 &hostPort);

		LTRESULT		GetMaxConnections(uint32 &nMaxConnections);

		void			NetDebugOut(int debugLevel, char *pMsg, ...);
		void			NetDebugOut2(CBaseConn *pConn, int debugLevel, char *pMsg, ...);

		void			SetNetHandler(CNetHandler *pHandler);

		LTGUID*			GetAppGuid() { return(&m_guidApp); }
		void			SetAppGuid(LTGUID* pAppGuid);


		// pPrefix is inserted in front of some debugging messages.
		void			Update(char *pPrefix, float fCurTime, bool bAllowTimeout=true);

		CBaseDriver*	AddDriver( const char *pDriverInfo );
		CBaseDriver*	GetDriver( const char* sDriver );
		void			RemoveDriver( CBaseDriver *pDriver );

		void			SetMainDriver(CBaseDriver* pDriver) { m_pMainDriver = pDriver; }
		CBaseDriver*	GetMainDriver() { return(m_pMainDriver); }

		// This will still call DisconnectNotify() on you.
		void			Disconnect(CBaseConn *id, EDisconnectReason reason);
							
		bool			SendPacket(const CPacket_Read &cPacket, CBaseConn *idSendTo, uint32 packetFlags = MESSAGE_GUARANTEED);
		
		void			StartGettingPackets();
		void			EndGettingPackets();
		bool			GetPacket(uint8 nTravelDir, CPacket_Read *pPacket, CBaseConn **pSender);

	// Misc helpers.
	public:
		
		bool			LagOrSend(const CPacket_Read &cPacket, CBaseConn *idSendTo, uint32 packetFlags);

		void			IncRecvCounter(CBaseConn *id, uint32 packetLen);
		void			IncSendCounter(CBaseConn *id, uint32 packetLen);

		// Just tells if there are any connections.
		bool			IsConnected()	{ return m_Connections > 0; }

		float			GetConnPing(CBaseConn *id) {return id->GetPing();}
	
	// Functions for drivers to call.  Just calls through to the handler.
	public:

		bool			NewConnectionNotify(CBaseConn *id);
		void			DisconnectNotify(CBaseConn *id, EDisconnectReason eDisconnectReason );


	// Internal stuff.
	protected:

		bool			ReallySendPacket(const CPacket_Read &cPacket, CBaseConn *idSendTo, uint32 nPacketFlags);

		bool			HandleReceivedPacket(CPacket_Read &cPacket, CBaseConn *pSender, bool bMaybeDrop );

	public:

		// State flags.
		uint32					m_Flags;

		// Used for latency simulation.
		ObjectBank<LatentPacket>	m_LatentPacketBank;

		// Elements in here are allocated (and owned) by the drivers.
		CMoArray<CBaseConn*>	m_Connections;
		
		CMoArray<CBaseDriver*>	m_Drivers;

		char					m_PlayerName[100];
		CNetHandler				*m_pHandler;

		char					*m_pCurPrefix;

		float					m_FrameTime;
	
	// Status stuff.
	public:

		uint32					m_nDroppedPackets;
		RateTracker				m_SendBPS;	

		float					m_fLastTime;

		LTGUID					m_guidApp;

		CBaseDriver*			m_pMainDriver;

	// Internal stuff
	private:

		typedef std::deque<CBaseConn*> TDelayedConnectionQueue;
		TDelayedConnectionQueue m_aDelayedConnections;
};


// Helper routines.
uint16 GetWordCRC(const CPacket_Read &cPacket);


#endif  //__NETMGR_H__

