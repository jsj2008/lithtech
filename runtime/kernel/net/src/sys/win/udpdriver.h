
#ifndef __UDPDRIVER_H__
#define __UDPDRIVER_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __SYSSOCKET_H__
#include "syssocket.h"
#endif

#ifndef __NETMGR_H__
#include "netmgr.h"
#endif

#ifndef __LTLINK_H__
#include "ltlink.h"
#endif

#include "syslthread.h"
#include "sysudpthread.h"

#include "listqueue.h"
#include "staticfifo.h"
#include <deque>
#include <map>

#define MAX_UDP_QUERY_TIMES 32
#define BROADCAST_QUERYNUM  0xFF

// What we use for an HNETSESSION.
class CUDPSession : public NetMgrSession 
{
public:
    CUDPSession(CBaseDriver *pDriver) : NetMgrSession(pDriver) {}
    
    sockaddr_in m_Addr; // Destination computer's handle.
};

class CUDPQuery
{
public:
    
    CUDPQuery() 
	{
        m_pName = LTNULL;
        m_QueryTime = 0.0f;
        m_CurQueryNum = 0;
        memset(m_QueryTimes, 0, sizeof(m_QueryTimes));
        m_Ping = 0.0f;
		m_nQueriesRemaining = 0;
    }

    CUDPQuery(const CUDPQuery &other) : m_pName(LTNULL) 
	{
        Copy(other);
    }

    ~CUDPQuery() 
	{
        if (m_pName) 
		{
            delete m_pName;
        }
    }       

    inline void operator=(const CUDPQuery &other) 
	{
        Copy(other);
    }

    void Copy(const CUDPQuery &other) 
	{
        if (m_pName) 
		{
            delete m_pName;
        }

        memcpy(this, &other, sizeof(*this));
        if (other.m_pName) {
            LT_MEM_TRACK_ALLOC(m_pName = new char[strlen(other.m_pName) + 1],LT_MEM_TYPE_NETWORKING);
            strcpy(m_pName, other.m_pName);
        }
    }

    // Used to calculate ping time for the servers.
    float m_QueryTimes[MAX_UDP_QUERY_TIMES];
    uint32 m_CurQueryNum;
	uint32 m_nQueriesRemaining;

    sockaddr_in m_Addr;         // The real connection address.
    char *m_pName;       // Session name.  LTNULL if we haven't gotten a response.
	bool m_bPassword;	// Does it have a password?
	uint8 m_nGameType;	// what type of game?
    float m_QueryTime;    // When we sent our query.
    float m_Ping;
};

class CUDPConn : public CBaseConn 
{
	struct CPacketFrame;
	friend struct CPacketFrame;
	class CFrameQueue;
	friend class CFrameQueue;
public:
	CUDPConn();
	~CUDPConn();

    // Remote address.
    sockaddr_in m_RemoteAddr;

    CMLLNode m_Node;

    SOCKET m_Socket;

	// Returns false if there was an error (connection problems, usually..)
	bool QueuePacket(const CPacket_Read &cPacket, bool bGuaranteed);

	// Handle an incoming packet
	enum EIncomingPacketResult {
		eIPR_OK = 0,
		eIPR_Disconnect = 1
	};
	EIncomingPacketResult HandleIncomingPacket(const CPacket_Read &cPacket);

	// Gets the disconnect reason if incoming packing is eIPR_Disconnect.
	EDisconnectReason GetLastDisconnectReason( ) const { return m_eLastDisconnectReason; }

	// Flush any outgoing packets
	bool FlushOutgoingQueues();

	// This should be called whenever the driver gets an update...
	// bFrameUpdate should only be true once per engine update
	void Update(bool bFrameUpdate);

	// Get the next waiting packet
	CPacket_Read GetPacket();

	// Send a disconnect message
	void SendDisconnectMessage( EDisconnectReason eDisconnectReason );

	// How long has it been since we last heard from this guy?
	uint32 GetTimeSinceLastCommunication();

	void SetMaxBandwidth(uint32 nValue) { m_nMaxBandwidth = nValue; SetBandwidth(LTMIN(m_nReportedBandwidth, m_nMaxBandwidth)); }

// Implementation of CBaseConn functions
public:

	virtual bool IsInTrouble();

	virtual int32		GetAvailableBandwidth(float fTime) const;
	virtual uint32		GetBandwidth() const;
	virtual bool		SetBandwidth(uint32 nBPS);
	virtual uint32		GetOutgoingBandwidthUsage() const;
	virtual uint32		GetIncomingBandwidthUsage() const;
	virtual uint32		GetTransportOverhead() const;
	virtual float		GetPacketLoss() const;
	
	virtual float GetPing() { return m_fReportedPing; }

	virtual bool GetIPAddress(uint8 pAddr[4], uint16 *pPort) 
	{ 
		pAddr[0] = (uint8)m_RemoteAddr.sin_addr.s_addr;
		pAddr[1] = (uint8)m_RemoteAddr.sin_addr.s_addr >> 8;
		pAddr[2] = (uint8)m_RemoteAddr.sin_addr.s_addr >> 16;
		pAddr[3] = (uint8)m_RemoteAddr.sin_addr.s_addr >> 24;
		
		*pPort = (uint16)m_RemoteAddr.sin_port;
		return true;
	}
	
private:
	// A queue of packets
	class CPacketQueue : public CListQueue<CPacket_Read>
	{
	public:
		typedef CListQueue<CPacket_Read> t_Parent;
		// Clear the packet when removing from the list
		void pop_front(t_Parent &cPool) {
			front() = CPacket_Read();
			t_Parent::pop_front(cPool);
		}
		void clear(t_Parent &cPool) {
			while (!empty())
			{
				pop_front(cPool);
			}
		}
	};

	// A frame for a set of sent guaranteed packets
	struct CPacketFrame
	{
		uint32 m_nFrameCode;
		uint32 m_nFrameSize;
		uint32 m_nFirstSent, m_nLastSent;
		bool m_bReceivedOO;
		bool m_bContinued;
		uint32 m_nSendCount;
		CPacketQueue m_cPackets;
	};

	// Constants
	enum {
		k_nFrameBits = 13, // Frame code bit count (allows 2-minute wrap at 60Hz)
		k_nFrameMask = (1 << k_nFrameBits) - 1, // Frame code bit mask
		k_nTargetPacketSize = 8800, // Target UDP packet size
		k_nMinPartialSize = 256, // Don't write partial packets smaller than this
		k_nMaxSizeIndicatorSize = 20, // Maximum size of a size indicator
		k_nHeartbeatDelay = 17, // Wait for at least this many ms between heartbeats
		k_nPingDelay = 200, // Wait at least this long between ping requests
		k_nPingHistorySize = 16, // Number of pings to average
		k_nPingNAKMultiplier = 1, // Wait Ping * this number of ms before re-sending packets
		k_nPingFirstNAKMultiplier = 1, // Wait Ping * this number of ms extra before re-sending packets the first time
		k_nMaxPingNAKTime = 1000, // Don't wait longer than this to send a NAK
		k_nTroubleDelay = 120000, // After 2 minutes, consider the connection in trouble
		k_nNAKFlushTroubleDelay = 120000, // After 2 minutes of trying to flush, it's in trouble
		k_nGuaranteedBlockCount = 2, // If we've retried a packet in the queue more times than this, 
									 // stop sending guaranteed packets for a while
		k_nGuaranteedTrickleCount = 10, // If we've incremented our resend count this many times, resend only the first frame
		k_nGuaranteedOverflowCount = 8, // If we get more messages than this in our NAK queue, pause
		k_nFingerprintBits = 8, // Number of bits in the fingerprint
		k_nUDPPacketOverhead = 28 * 8, // UDP packet overhead
		k_nStartPing = 200, // Initial ping value, for handling lost initial packets
		k_nMaxOutOfOrderPackets = 8, // Maximum number of out of order packets to track
		k_nFlowControlPeriod = 200, // Flow control operates at this resolution
		k_nFlowControlMinPeriod = 17, // Don't allow sub-period flow control updates smaller than this
		k_nBandwidthUsageTarget = 90, // Try to target using this much of our bandwidth
		k_nUnguaranteedDropDelay = 2, // Don't hang on to unguaranteed data longer than ping * this number
		k_nDisconnectSleep = 100, // How long to wait after each disconnect message to avoid blocking the outgoing pipe
		k_nTrickleNAKDelay = 500, // Wait at least this long between NAKs while trickling
	};

	// Internal UDP commands
	enum {
		k_nUDPCommand_Heartbeat = 0,
		k_nUDPCommand_Disconnect = 1,
		k_nUDPCommandBits = 1, // Number of bits used per internal UDP command
		k_nUDPCommand_Heartbeat_Size = k_nUDPCommandBits + k_nFrameBits + 1 + k_nMaxOutOfOrderPackets,
	};

	EIncomingPacketResult HandleUDP(CPacket_Read &cPacket);
	void HandleGuaranteed(CPacket_Read &cPacket, bool bOutOfOrder);
	void HandleUnguaranteed(CPacket_Read &cPacket);

	// Is it time for a heartbeat?
	bool ShouldSendHeartbeat();
	// Should we send an empty frame?
	bool ShouldSendEmptyFrame();
	// Should we send a packet?
	bool ShouldSendPacket(bool bOnlyBandwidth);

	// How big is the size indicator for a packet nSize bits long
	static uint32 GetSizeIndicatorSize(uint32 nSize);
	// Write a size indicator to the packet
	static void WriteSizeIndicator(CPacket_Write &cPacket, uint32 nSize);
	// Read a size indicator from the packet
	static uint32 ReadSizeIndicator(CPacket_Read &cPacket);

	// Write the different portions of the UDP packet
	void WriteHeartbeat(CPacket_Write &cPacket);
	void WriteGuaranteed(CPacket_Write &cPacket, bool bForceEmpty);
	void WriteUnguaranteed(CPacket_Write &cPacket);

	// Clear the frame history up to and including nFrameCode
	void ClearFrameHistory(uint32 nFrameCode);
	// Remove nFrameCode from the frame history
	bool RemoveFrameHistory(uint32 nFrameCode, CPacketFrame **pFrame);

	// Add a ping to the ping history
	void AddPing(uint32 nTime);

	// Re-send packets which never got ACK'ed
	void ReSendLostPackets();
	void ReSendFrame(CPacketFrame &cFrame);

	// Update the current ping time
	void UpdatePing();
	// Update the bandwidth stats
	void UpdateBandwidth();

	// Send a final packet
	bool SendPacket(const CPacket_Read &cPacket);

	// Find out how bit a UDP packet would be for a packet of a given size
	static uint32 GetUDPPacketSize(uint32 nSize);
	// Calculate a fingerprint for a packet
	static uint32 GetPacketFingerprint(const CPacket_Read &cPacket);

	// Incoming packet queuing
	static CPacketQueue s_cPacketTrash;

	// Time queues
	typedef CListQueue<uint32> CTimeQueue;
	static CTimeQueue s_cTimeTrash;

	CPacketQueue m_cIncomingQueue;

	// Only one thread updating at a time, please...
	LCriticalSection m_cUpdateCS;

	bool m_bPauseGuaranteed;
	CPacketQueue m_cOutgoingGuaranteedQueue;
	uint32 m_nOutgoingGCount;
	uint32 m_nOutgoingGSize;
	CPacketQueue m_cOutgoingUnguaranteedQueue;
	CTimeQueue m_cOutgoingUnguaranteedTimeQueue;
	uint32 m_nOutgoingUCount;
	uint32 m_nOutgoingUSize;
	uint32 m_nOutgoingFrameCode;

	CPacket_Write m_cIncomingIncompletePacket;
	uint32 m_nIncomingLastFrame;

	uint32 m_nLastHeartbeatTime;
	bool m_bACKPending;

	// Guaranteed packet history
	class CFrameQueue : public CListQueue<CPacketFrame>
	{
	public:
		typedef CListQueue<CPacketFrame> t_Parent;
		void pop_front(t_Parent &cPool, CPacketQueue &cQueuePool) {
			front().m_cPackets.clear(cQueuePool);
			t_Parent::pop_front(cPool);
		}
	};
	static CFrameQueue s_cFrameTrash;
	CFrameQueue m_cFrameHistory;

	uint32 m_aPingHistory[k_nPingHistorySize];
	uint32 m_nNumPings, m_nPingOffset;
	float m_fCurPing, m_fReportedPing;
	bool m_bWaitingForPingResponse;
	uint32 m_nLastPingTimeStamp;

	// NAK flushing...
	bool m_bFlushingNAKs;
	uint32 m_nNAKFlushStartTime;

	// Bandwidth tracking
	uint32 m_nBandwidth, m_nReportedBandwidth, m_nMaxBandwidth;
	uint32 m_nLastSendTime, m_nLastRecvTime;
	// A tracking entry
	struct CBandwidthPeriod
	{
		CBandwidthPeriod() {}
		CBandwidthPeriod(uint32 nSize, uint32 nTime) : m_nSize(nSize), m_nTime(nTime) {}
		CBandwidthPeriod(const CBandwidthPeriod &cOther) { new(this) CBandwidthPeriod(cOther.m_nSize, cOther.m_nTime); }
		CBandwidthPeriod &operator=(const CBandwidthPeriod &cOther) {
			m_nSize = cOther.m_nSize;
			m_nTime = cOther.m_nTime;
			return *this;
		}
		CBandwidthPeriod &operator+=(const CBandwidthPeriod &cOther) {
			m_nSize += cOther.m_nSize;
			m_nTime += cOther.m_nTime;
			return *this;
		}
		uint32 m_nSize;
		uint32 m_nTime;
	};

	enum { k_nBandwidthTrackingMaxPeriod = 100 }; // Maximum period length in the bandwidth history
	enum { k_nBandwidthTrackingQueueSize = 15 };
	typedef CStaticFIFO<CBandwidthPeriod, k_nBandwidthTrackingQueueSize> TBandwidthHistory;
	TBandwidthHistory m_aOutgoingBandwidthHistory;
	TBandwidthHistory m_aIncomingBandwidthHistory;

	enum { k_nPacketLossTrackingQueueSize = 64 };
	typedef CStaticFIFO<bool, k_nPacketLossTrackingQueueSize> TPacketLossHistory;
	TPacketLossHistory m_aPacketLossHistory;

	void AccumulateHistory(TBandwidthHistory &cHistory);

	// An out of order packet
	class COutOfOrderPacket
	{
	public:
		COutOfOrderPacket() : m_nFrameCode(0), m_bInUse(false)
		{
		}
		CPacket_Read m_cPacket;
		uint32 m_nFrameCode;
		bool m_bInUse;
	};
	// The out of order packet buffer
	COutOfOrderPacket m_aOutOfOrderPackets[k_nMaxOutOfOrderPackets];
	uint32 m_nNumOutOfOrderPackets;

	void SaveOutOfOrderPacket(CPacket_Read &cPacket, uint32 nFrameCode);

	// Per-frame flow control
	void ResetFlowControl();
	void UpdateOutgoingFlowControl(uint32 nPacketSize, uint32 nCurTime);
	bool IsFlowControlBlocked(uint32 nPacketSize) const;

	bool m_bFlowControlInitialized;
	uint32 m_nFlowControlLastPeriodSent;
	uint32 m_nFlowControlCurPeriodSent;
	uint32 m_nFlowControlPeriodTime;
	uint32 m_nFlowControlBitsPerPeriod;

	// Stores the disconnect reason if told to disconnect.
	EDisconnectReason m_eLastDisconnectReason;
};

const uint16 DEFAULT_LISTENPORT =27888;

const float QUERY_TIME = 3.5f; // Query for 2.5 seconds.
const float QUERY_SEND_INTERVAL = 1.0f; // Time between server queries.

struct SUDPError {
	int m_ErrorCode;
	char *m_ErrorString;
};
#define DEFINE_UDP_ERROR(x) { x, #x },

const SUDPError g_UDPErrorStrings[] =
{

#ifdef _WIN32
	DEFINE_UDP_ERROR(WSAEINTR)
	DEFINE_UDP_ERROR(WSAEBADF)
	DEFINE_UDP_ERROR(WSAEACCES)
	DEFINE_UDP_ERROR(WSAEFAULT)
	DEFINE_UDP_ERROR(WSAEINVAL)
	DEFINE_UDP_ERROR(WSAEMFILE)
	DEFINE_UDP_ERROR(WSAEWOULDBLOCK)
	DEFINE_UDP_ERROR(WSAEINPROGRESS)
	DEFINE_UDP_ERROR(WSAEALREADY)
	DEFINE_UDP_ERROR(WSAENOTSOCK)
	DEFINE_UDP_ERROR(WSAEDESTADDRREQ)
	DEFINE_UDP_ERROR(WSAEMSGSIZE)
	DEFINE_UDP_ERROR(WSAEPROTOTYPE)
	DEFINE_UDP_ERROR(WSAENOPROTOOPT)
	DEFINE_UDP_ERROR(WSAEPROTONOSUPPORT)
	DEFINE_UDP_ERROR(WSAESOCKTNOSUPPORT)
	DEFINE_UDP_ERROR(WSAEOPNOTSUPP)
	DEFINE_UDP_ERROR(WSAEPFNOSUPPORT)
	DEFINE_UDP_ERROR(WSAEAFNOSUPPORT)
	DEFINE_UDP_ERROR(WSAEADDRINUSE)
	DEFINE_UDP_ERROR(WSAEADDRNOTAVAIL)
	DEFINE_UDP_ERROR(WSAENETDOWN)
	DEFINE_UDP_ERROR(WSAENETUNREACH)
	DEFINE_UDP_ERROR(WSAENETRESET)
	DEFINE_UDP_ERROR(WSAECONNABORTED)
	DEFINE_UDP_ERROR(WSAECONNRESET)
	DEFINE_UDP_ERROR(WSAENOBUFS)
	DEFINE_UDP_ERROR(WSAEISCONN)
	DEFINE_UDP_ERROR(WSAENOTCONN)
	DEFINE_UDP_ERROR(WSAESHUTDOWN)
	DEFINE_UDP_ERROR(WSAETOOMANYREFS)
	DEFINE_UDP_ERROR(WSAETIMEDOUT)
	DEFINE_UDP_ERROR(WSAECONNREFUSED)
	DEFINE_UDP_ERROR(WSAELOOP)
	DEFINE_UDP_ERROR(WSAENAMETOOLONG)
	DEFINE_UDP_ERROR(WSAEHOSTDOWN)
	DEFINE_UDP_ERROR(WSAEHOSTUNREACH)
	DEFINE_UDP_ERROR(WSAENOTEMPTY)
	DEFINE_UDP_ERROR(WSAEPROCLIM)
	DEFINE_UDP_ERROR(WSAEUSERS)
	DEFINE_UDP_ERROR(WSAEDQUOT)
	DEFINE_UDP_ERROR(WSAESTALE)
	DEFINE_UDP_ERROR(WSAEREMOTE)
	DEFINE_UDP_ERROR(WSAEDISCON)
	DEFINE_UDP_ERROR(WSASYSNOTREADY)
	DEFINE_UDP_ERROR(WSAVERNOTSUPPORTED)
	DEFINE_UDP_ERROR(WSANOTINITIALISED)

#else
#ifdef _LINUX

	DEFINE_UDP_ERROR(EINTR)
	DEFINE_UDP_ERROR(EAGAIN)
	DEFINE_UDP_ERROR(EWOULDBLOCK)	
	DEFINE_UDP_ERROR(ENOTSOCK)
	DEFINE_UDP_ERROR(EDESTADDRREQ)
	DEFINE_UDP_ERROR(EMSGSIZE)
	DEFINE_UDP_ERROR(EPROTOTYPE)
	DEFINE_UDP_ERROR(ENOPROTOOPT)
	DEFINE_UDP_ERROR(EPROTONOSUPPORT)
	DEFINE_UDP_ERROR(ESOCKTNOSUPPORT)
	DEFINE_UDP_ERROR(EOPNOTSUPP)
	DEFINE_UDP_ERROR(EPFNOSUPPORT)
	DEFINE_UDP_ERROR(EAFNOSUPPORT)
	DEFINE_UDP_ERROR(EADDRINUSE)
	DEFINE_UDP_ERROR(EADDRNOTAVAIL)
	DEFINE_UDP_ERROR(ENETDOWN)
	DEFINE_UDP_ERROR(ENETUNREACH)
	DEFINE_UDP_ERROR(ENETRESET)
	DEFINE_UDP_ERROR(ECONNABORTED)
	DEFINE_UDP_ERROR(ECONNRESET)
	DEFINE_UDP_ERROR(ENOBUFS)
	DEFINE_UDP_ERROR(EISCONN)
	DEFINE_UDP_ERROR(ENOTCONN)
	DEFINE_UDP_ERROR(ESHUTDOWN)	
	DEFINE_UDP_ERROR(ETOOMANYREFS)
	DEFINE_UDP_ERROR(ETIMEDOUT)	
	DEFINE_UDP_ERROR(ECONNREFUSED)	
	DEFINE_UDP_ERROR(EHOSTDOWN)	
	DEFINE_UDP_ERROR(EHOSTUNREACH)	
	DEFINE_UDP_ERROR(EALREADY)	
	DEFINE_UDP_ERROR(EINPROGRESS)	

#endif
#endif

};


const uint32 NUM_UDPERRORSTRINGS = sizeof(g_UDPErrorStrings) / sizeof(g_UDPErrorStrings[0]);

class CUDPDriver : public CBaseDriver 
{
public:

    CUDPDriver();
    virtual ~CUDPDriver();

    virtual bool Init();

    void Term2(bool bFullShutdown, bool bShutdownSocket);
    virtual void Term();

    
    static bool SendTo(SOCKET theSocket, const CPacket_Read &cPacket, sockaddr_in *pSendTo);
    CUDPConn *FindConnByAddr(sockaddr_in *pAddr);


    virtual LTRESULT GetServiceList(NetService* &pListHead);
    virtual LTRESULT SelectService(BaseService *pService) {return LT_OK;}

    virtual LTRESULT StartQuery(const char *pInfo);
    virtual LTRESULT UpdateQuery();
    virtual LTRESULT GetQueryResults(NetSession* &pListHead);
    virtual LTRESULT EndQuery();

    virtual LTRESULT GetSessionList(NetSession* &pListHead, const char *pInfo);

    virtual LTRESULT GetSessionName(char* sName, uint32 dwBufferSize);
    virtual LTRESULT SetSessionName(const char* sName);

    virtual LTRESULT GetHasPassword(bool *bHasPassword) 
	{ 
		CSAccess cCS(&m_cCS_SessionData); 
		*bHasPassword = m_bPassword; 
		return LT_OK; 
	}
	
    virtual LTRESULT SetHasPassword(bool bHasPassword)  
	{ 
		CSAccess cCS(&m_cCS_SessionData); 
		m_bPassword = bHasPassword; 
		return LT_OK; 
	}

    virtual LTRESULT GetGameType(uint8 *nGameType) 
	{ 
		CSAccess cCS(&m_cCS_SessionData); 
		*nGameType = m_nGameType; 
		return LT_OK; 
	}
	
    virtual LTRESULT SetGameType(uint8 nGameType)  
	{ 
		CSAccess cCS(&m_cCS_SessionData); 
		m_nGameType = nGameType; 
		return LT_OK; 
	}

    virtual LTRESULT GetMaxConnections(uint32 &nMaxConnections);

    virtual void Update();

    virtual LTRESULT HostSession(NetHost* pHost);
    virtual LTRESULT JoinSession(NetSession *pSession);

    virtual bool GetLocalIpAddress(char* sBuffer, uint32 dwBufferSize, uint16 &hostPort);
    LTRESULT ReallyJoinSession( bool bOpenNewSocket, sockaddr_in *addr);
    virtual LTRESULT ConnectTCP( const char* sAddress );
    virtual LTRESULT SendTcpIp(const CPacket_Read &cMsg, const char *sAddr, uint32 port);

	virtual LTRESULT StartPing(const char *pAddr, uint16 nPort, uint32 *pPingID);
	virtual LTRESULT GetPingStatus(uint32 nPingID, uint32 *pStatus, uint32 *pLatency);
	virtual LTRESULT RemovePing(uint32 nPingID);

	virtual LTRESULT CUDPDriver::OpenSocket( SOCKET* phSocket );

	virtual void UpdateGUID(LTGUID &cGUID);
	
	enum {
		UNCONNECTED_DATA_TOKEN = 0x9919D9C7, // Packet identifier for unconnected communication
		UNCONNECTED_MSG_QUERY = 0,
		UNCONNECTED_MSG_QUERY_RESPONSE = 1,
		UNCONNECTED_MSG_CONNECT = 2,
		UNCONNECTED_MSG_CONNECT_RESPONSE = 3,
		UNCONNECTED_MSG_PING = 4,
		UNCONNECTED_MSG_PING_RESPONSE = 5,
		UNCONNECTED_MSG_BITS = 3
	};

protected:
    // MUST call the net mugger's DisconnectNotify.
    void Disconnect(CBaseConn *id, EDisconnectReason reason, bool bSendMessage);
    virtual void Disconnect(CBaseConn *id, EDisconnectReason reason) { Disconnect(id, reason, true); }

    // Driver-level functions.
	virtual bool SendPacket(const CPacket_Read &cPacket, CBaseConn *idSendTo, bool bGuaranteed);
	virtual bool GetPacket(CPacket_Read *pPacket, CBaseConn **pSender);

	enum { CONN_SEND_INTERVAL =300, CONN_WAIT_TIME =10000 };

	void HandleUnconnectedData(CPacket_Read &cPacket, sockaddr_in *pSender);

	enum {
		k_nReconnection_Delay = 10000, // Re-connection lockout delay, in ms
		k_nListenThread_Timeout = 30000, // Time-out on the listen thread, in ms
	};
	
private:

	// Threading support
	void StartThread_Listen();
	void StopThread_Listen();
	static uint32 ThreadBootstrap_Listen(void* pUserData);
	uint32 Thread_Listen();
	CLTThread m_cListenThread;
	CLTThreadEvent m_cEvent_Thread_Listen_Ready;
	CLTThreadEvent m_hEvent_Thread_Listen_Shutdown;
	CLTThreadEvent m_hEvent_Thread_Listen_Pause;
	CLTThreadEvent m_hEvent_Thread_Listen_Paused;


/*
	// Threading support
	void StartThread_Listen();
	void StopThread_Listen();
	
//	static unsigned long _stdcall ThreadBootstrap_Listen(void *pUserData);

	uint32 ThreadBootstrap_Listen(void *pUserData)


	uint32 Thread_Listen();
	
//	HANDLE m_hThread_Listen;
//	CWinSync_Event m_cEvent_Thread_Listen_Ready;
//	CWinSync_Event m_hEvent_Thread_Listen_Shutdown;
*/


private:

	void FlushInternalQueues();

	// Unknown message queue
	struct CUnknownMessage
	{
		CPacket_Read m_cPacket;
		sockaddr_in m_cSender;
	};
	
	LCriticalSection m_cCS_UnknownMessages;
	std::deque<CUnknownMessage> m_cUnknownMessages;

	void FlushUnknownMessageQueue();

	// Disconnection queue
	struct CDisconnectRequest
	{
		CUDPConn *m_pConnection;
		EDisconnectReason m_eReason;
	};
	
	LCriticalSection m_cCS_DisconnectQueue;
	
	std::deque<CDisconnectRequest> m_cDisconnectQueue;

	void FlushDisconnectQueue();

	// Connection queue
	struct CConnectRequest
	{
		CUDPConn *m_pConnection;
	};
	
	LCriticalSection m_cCS_ConnectQueue;
	
	std::deque<CConnectRequest> m_cConnectQueue;

	void FlushConnectQueue();

	// Ping handling
	LTRESULT SendPing(const char *pAddr, uint16 nPort, uint32 nID);

	enum {
		k_nPing_Bits = 24,
		k_nPing_Timeout = 999,
	};
	uint32 m_nCurPingID;
	struct CPingRequest
	{
		uint32 m_nID;
		uint32 m_nTimeStamp;
		uint32 m_nStatus;
	};
	typedef std::map<uint32, CPingRequest> TPingMap;
	LCriticalSection m_cCS_ActivePings;
	TPingMap m_cActivePings;

	// Misc. data
	LCriticalSection m_cCS_GUID;
	LTGUID m_cGUID;
	
public:
    SOCKET SetupClientSocket();


public:
    CMoArray<CUDPQuery> m_Queries;
    SOCKET m_QuerySocket;
    float m_LastQuerySendTime;
    sockaddr_in m_QueryAddr;

    sockaddr_in m_HostAddr;

    SOCKET m_Socket;

	LCriticalSection m_cCS_SessionData;
	
    bool m_bHosting;
    uint32 m_nMaxConnections; // Max remote connections if we're the host.
    char *m_SessionName;
	bool m_bPassword;
	uint8 m_nGameType;

	bool IsHosting() 
	{ 
		CSAccess cCS(&m_cCS_SessionData); 
		return m_bHosting; 
	}

	LCriticalSection m_cCS_Connections;
    CMultiLinkList<CUDPConn*> m_Connections;

    bool m_bWSAInitted;
    BaseService m_DummyService;
};

inline unsigned char INADDR_B1(const sockaddr_in& addr)
{ return (unsigned char ) addr.sin_addr.s_addr; }
inline unsigned char INADDR_B2(const sockaddr_in& addr)
{ return (unsigned char ) (addr.sin_addr.s_addr >> 8); }
inline unsigned char INADDR_B3(const sockaddr_in& addr)
{ return (unsigned char ) (addr.sin_addr.s_addr >> 16); }
inline unsigned char INADDR_B4(const sockaddr_in& addr)
{ return (unsigned char ) (addr.sin_addr.s_addr >> 24); }

#define EXPAND_BASEADDR(addr)\
              INADDR_B1(addr), INADDR_B2(addr), INADDR_B3(addr), INADDR_B4(addr)

#define EXPAND_ADDR(addr) \
	EXPAND_BASEADDR(addr),\
	ntohs((addr).sin_port)

static const char* ADDR_PRINTF = "%d.%d.%d.%d:%d";

#endif  // __UDPDRIVER_H__





