//////////////////////////////////////////////////////////////////////////////
// Titan-specific server directory interface implementation

#ifndef __CSERVERDIR_TITAN_H__
#define __CSERVERDIR_TITAN_H__

#define SERVERDIR_EXPORTS
#include "iserverdir.h"
#include "iserverdir_titan.h"

#include <deque>
#include <vector>
#include <string>

#include "winsync.h"

#pragma warning (disable:4503)
#include "wonapi.h"
#include "wonauth/authcontext.h"
#include "wondir/getdirop.h"
#include "wondir/addentityop.h"
#include "wondir/renewentityop.h"
#include "wondir/removeentityop.h"
#include "wonauth/getcertop.h"
#include "wonmisc/getmotdop.h"
#include "wonmisc/checkvalidversionop.h"
#include "wonauth/peerauthclient.h"
#include "wonauth/peerauthserver.h"
#include "wonmisc/gamespysupport.h"


#include "ClientResShared.h"

using namespace WONAPI;

// Forward declarations
class CSDTitan_RequestEntry;
class CSDTitan_RequestEntry_Noop;
class CSDTitan_RequestEntry_TitanOp;
class CSDTitan_RequestEntry_MOTD;
class CSDTitan_RequestEntry_Validate_Version;
class CSDTitan_RequestEntry_Validate_CDKey;
class CSDTitan_RequestEntry_Publish_Server;
class CSDTitan_RequestEntry_Remove_Server;
class CSDTitan_RequestEntry_UpdateList;
class CSDTitan_RequestEntry_UpdatePings;
class CSDTitan_RequestEntry_MsgOp;
class CSDTitan_RequestEntry_Peer_Details;
class CSDTitan_RequestEntry_Validate_Client;
class CSDTitan_RequestEntry_Validate_Server;

const char* GetTempString(int messageCode);

// Internal communication message ID's
enum ECSDTitan_Msg {
	// Detail info query
	eSDTitan_Msg_DetailInfo_Query = 0,
	// Detail info response
	eSDTitan_Msg_DetailInfo_Response = 1,
	// Peer authentication message sent from the client
	eSDTitan_Msg_PeerAuth_Client = 2,
	// Peer authentication message sent from the server
	eSDTitan_Msg_PeerAuth_Server = 3
};

//////////////////////////////////////////////////////////////////////////////
// CServerDirectory_Titan

class CServerDirectory_Titan : public IServerDirectory
{
	friend CSDTitan_RequestEntry;
	friend CSDTitan_RequestEntry_Noop;
	friend CSDTitan_RequestEntry_TitanOp;
	friend CSDTitan_RequestEntry_MOTD;
	friend CSDTitan_RequestEntry_Validate_Version;
	friend CSDTitan_RequestEntry_Validate_CDKey;
	friend CSDTitan_RequestEntry_Publish_Server;
	friend CSDTitan_RequestEntry_Remove_Server;
	friend CSDTitan_RequestEntry_UpdateList;
	friend CSDTitan_RequestEntry_UpdatePings;
	friend CSDTitan_RequestEntry_MsgOp;
	friend CSDTitan_RequestEntry_Peer_Details;
	friend CSDTitan_RequestEntry_Validate_Client;
	friend CSDTitan_RequestEntry_Validate_Server;

public:
	CServerDirectory_Titan( );
	virtual ~CServerDirectory_Titan();

	//////////////////////////////////////////////////////////////////////////////
	// Request handling

	virtual bool QueueRequest(ERequest eNewRequest);
	virtual bool QueueRequestList(const TRequestList &cNewRequests);
	virtual TRequestList GetWaitingRequestList() const;
	virtual bool ClearRequestList();
	virtual ERequestResult ProcessRequest(ERequest eNewRequest, uint32 nTimeout);

	virtual bool PauseRequestList();
	virtual bool ProcessRequestList();

	virtual ERequestResult BlockOnActiveRequest(uint32 nTimeout);
	virtual ERequestResult BlockOnRequest(ERequest eBlockRequest, uint32 nTimeout);
	virtual ERequestResult BlockOnRequestList(const TRequestList &cBlockRequestList, uint32 nTimeout);
	virtual ERequestResult BlockOnProcessing(uint32 nTimeout);

	virtual bool IsRequestPending(ERequest ePendingRequest) const;

	virtual ERequest GetLastSuccessfulRequest() const;
	virtual ERequest GetLastErrorRequest() const;
	virtual ERequest GetActiveRequest() const;

	virtual ERequest GetLastRequest() const;
	virtual ERequestResult GetLastRequestResult() const;
	virtual char const* GetLastRequestResultString() const;

	//////////////////////////////////////////////////////////////////////////////
	// Status

	virtual EStatus GetCurStatus() const;
	virtual char const* GetCurStatusString() const;

	//////////////////////////////////////////////////////////////////////////////
	// Startup - service specific.

	virtual void SetStartupInfo( ILTMessage_Read &cMsg );
	virtual void GetStartupInfo( ILTMessage_Write &cMsg );

	//////////////////////////////////////////////////////////////////////////////
	// Game name

	virtual void SetGameName(const char *pName);
	virtual char const* GetGameName() const;

	//////////////////////////////////////////////////////////////////////////////
	// CD Keys

	virtual bool SetCDKey(const char *pKey);
	virtual bool GetCDKey(std::string *pKey);
	virtual bool IsCDKeyValid() const;

	//////////////////////////////////////////////////////////////////////////////
	// Game Version

	virtual void SetVersion(const char *pVersion);
	virtual void SetRegion(const char *pRegion);
	virtual bool IsVersionValid() const;
	virtual bool IsVersionNewest() const;
	virtual bool IsPatchAvailable() const;

	//////////////////////////////////////////////////////////////////////////////
	// Message of the Day

	virtual bool IsMOTDNew(EMOTD eMOTD) const;
	virtual char const* GetMOTD(EMOTD eMOTD) const;

	//////////////////////////////////////////////////////////////////////////////
	// Active peer info

	virtual bool SetActivePeer(const char *pAddr);
	virtual bool GetActivePeer(std::string *pAddr, bool *pLocal) const;
	virtual bool RemoveActivePeer();

	virtual bool SetActivePeerInfo(EPeerInfo eInfoType, ILTMessage_Read &cMsg);
	virtual bool HasActivePeerInfo(EPeerInfo eInfoType) const;
	virtual bool GetActivePeerInfo(EPeerInfo eInfoType, ILTMessage_Write *pMsg) const;

	//////////////////////////////////////////////////////////////////////////////
	// Server listing

	virtual TPeerList GetPeerList() const;
	virtual void ClearPeerList();

	//////////////////////////////////////////////////////////////////////////////
	// Net message handling

	virtual bool HandleNetMessage(ILTMessage_Read &cStr, const char *pSender, uint16 nPort);
	virtual bool SetNetHeader(ILTMessage_Read &cStr);

// Internal types
private:
	typedef std::deque<CSDTitan_RequestEntry*> TRequestEntryList;

	struct SMOTDData
	{
		SMOTDData() : m_sMessage(""), m_bNew(false) {}
		std::string m_sMessage;
		bool m_bNew;
	};

	struct SPeerData
	{
		SPeerData();
		IPAddr m_cAddr;
		std::string m_sName;
		typedef std::vector<uint8> TRawData;
		TRawData m_cSummary;
		TRawData m_cDetails;
 		uint32 m_nPing;

		enum EValidationState {
			eValidationState_None,
			eValidationState_Partial,
			eValidationState_Complete
		};
		EValidationState m_eValidation;

		float m_fCreationTime;

		bool m_bHasName;
		bool m_bHasSummary;
		bool m_bHasDetails;
		bool m_bHasPing;

		PeerInfo_Service_Titan m_PeerInfoService;
	};
	typedef std::vector<SPeerData> TPeerDataList;

// Functions which get called by the request entry classes
protected:
	void FinishRequest(CSDTitan_RequestEntry *pRequest, ERequestResult eResult, const std::string &sResultStr);

	void SetMOTD(EMOTD eMOTD, const char *pMessage, bool bIsNew);
	enum EVersionState {
		eVersion_Unknown,
		eVersion_Latest,
		eVersion_Old,
		eVersion_Patchable
	};
	void SetVersionState(EVersionState eState);
	char const* GetVersion() const;
	char const* GetRegion() const;

	void SetAuthContextValid(bool bValue);

	AuthContextPtr GetAuthContext() { return m_pAuthContext; }
	ServerContextPtr GetDirServerList() { return m_pDirServerList; }
	CDKey &GetTitanCDKey() { return m_cCDKey; }
	CWinSync_CS &GetPeerInfoCS() { return m_cPeerCS; }
	SPeerData &GetLocalPeerInfo() { return m_aPeerInfo.front(); }
	SPeerData &GetActivePeerInfo() { return m_aPeerInfo[m_nActivePeerIndex]; }
	TPeerDataList &GetPeerInfo() { return m_aPeerInfo; }
	uint32 GetActivePeerIndex() const { return m_nActivePeerIndex; }
	void SetActivePeerIndex(uint32 nNewIndex) { m_nActivePeerIndex = nNewIndex; }

	CWinSync_CS &GetPeerAuthCS() { return m_cValidationCS; }
	PeerAuthServer &GetPeerAuthServer() { return m_cPeerAuthServer; }
	PeerAuthClient &GetPeerAuthClient() { return m_cPeerAuthClient; }

	void SetCurStatusString(const char *pMessage);

	// Create a message with the standard header already inserted
	ILTMessage_Write *CreateMessage();

	const ILTMessage_Read &GetNetHeader() { return *m_pMsgHeader; }

// Internal functions
private:

	// Check to see if the state is correct for processing the request
	bool ValidateRequest(ERequest eNewRequest) const;

	CSDTitan_RequestEntry *MakeRequestEntry(ERequest eNewRequest);

	// Put a request in the queue, and start it up if necessary
	void StartRequest(CSDTitan_RequestEntry *pRequest);

	// Load the directory server list
	void LoadDirServerList();

	// The proper way to check m_bIsAuthContextValid
	bool IsAuthContextValid() const;

	// Internal pause functions
	void TemporaryPause(const char *pNewStatusStr, EStatus *pOldStatus, std::string *pOldStatusStr);
	void TemporaryResume(EStatus eOldStatus, const char *pOldStatusStr);

	// Start the active request
	void StartActiveRequest();
	// Cancel the active request
	void CancelActiveRequest();

	// Block on the current completion state
	ERequestResult BlockOnCompletion(uint32 nTimeout);

	// StartRenderThread functions
	static unsigned long _stdcall StartRenderThread_Bootstrap(void *pParam);
	uint32 StartRenderThread_Run();

	// Handle a network message
	bool HandleNetMsg_DetailInfo_Query(ILTMessage_Read &cMsg, IPAddr &cAddr);
	bool HandleNetMsg_PeerAuth_Client(ILTMessage_Read &cMsg, IPAddr &cAddr);

	// Sets/Gets the service peer info.
	bool SetPeerInfoService( SPeerData& activePeer, PeerInfo_Service_Titan const& info );

	// Create/destroy the gamespysupport object.
	bool CreateGameSpySupport( );
	void DestroyGameSpySupport( );
	bool IsGameSupportCreated( ) const { return ( m_pGameSpySupport != NULL ); }

private:
	CWinSync_Event m_cShutdownEvent;

	WONAPICore m_cWONAPICore;

	ServerContextPtr m_pDirServerList;
	AuthContextPtr m_pAuthContext;

	CWinSync_CS m_cAuthContextCS;
	bool m_bIsAuthContextValid;

	CWinSync_CS m_cGameNameCS;
	std::string m_sGameName;

	CDKey m_cCDKey;

	// Message of the day
	CWinSync_CS m_cMOTDCS;
	SMOTDData m_aMOTD[eMOTD_TotalNum];

	// Version
	CWinSync_CS m_cVersionCS;
	EVersionState m_eVersionState;
	std::string m_sCurVersion;
	std::string m_sCurRegion;

	// Peer info
	CWinSync_CS m_cPeerCS;
	TPeerDataList m_aPeerInfo;
	uint32 m_nActivePeerIndex;

	// Validation info
	CWinSync_CS m_cValidationCS;
	PeerAuthServer m_cPeerAuthServer;
	PeerAuthClient m_cPeerAuthClient;

	// Message headers
	CWinSync_CS m_cMsgHeaderCS;
	CLTMsgRef_Read m_pMsgHeader;

	// Critical section for managing the processing state
	CWinSync_CS m_cStateCS;

	// Status
	EStatus m_eCurStatus;
	std::string m_sCurStatusString;

	// The request entries
	TRequestEntryList m_cRequestEntryList;

	// Blocking state
	enum ECompletionBlock {
		eCompletionBlock_None,
		eCompletionBlock_Active,
		eCompletionBlock_List,
		eCompletionBlock_Processing
	};
	ECompletionBlock m_eCompletionAction;
	TRequestList m_cCompletionList;
	CWinSync_PulseEvent m_cCompletionEvent;
	CWinSync_Event m_cCompletionEvent_Continue;

	// Most recent request results
	ERequest m_eLastSuccessfulRequest;
	ERequest m_eLastErrorRequest;
	ERequest m_eLastRequest;
	ERequestResult m_eLastRequestResult;
	std::string m_sLastRequestResultString;

	// Thread for starting request entries
	HANDLE m_hStartRequestThread;
	CWinSync_Event m_cSRTReadyEvent;
	CWinSync_PulseEvent m_cSRTStartEvent;

	// Startup info.
	StartupInfo_Titan m_StartupInfo;

	// Gamespysupport.
	CWinSync_CS m_cGameSpySupportCS;
	GameSpySupportPtr m_pGameSpySupport;
};

//////////////////////////////////////////////////////////////////////////////
// Request entry classes

class CSDTitan_RequestEntry
{
public :
	CSDTitan_RequestEntry(CServerDirectory_Titan *pDir);

	virtual ~CSDTitan_RequestEntry() {}

	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Nothing; }

	// Called by the server directory

	// Start the request
	virtual void Start() { Finish(IServerDirectory::eRequestResult_Success, std::string("")); }
	// Cancel the request
	virtual void Cancel() {}
	// Notification that a message was received
	virtual bool OnMessage(ILTMessage_Read &cMsg, const IPAddr &cSource) { return false; }
protected:
	// Call this function to indicate to the directory that this request has finished
	// processing.
	// Note : You may very well get deleted by calling this function
	virtual void Finish(IServerDirectory::ERequestResult eResult, const std::string &sResultString);

	// Get the directory pointer
	CServerDirectory_Titan *GetDir() { return m_pDir; }

private:
	CServerDirectory_Titan *m_pDir;
};

// No-op request entry.  (Base class with request type per-instance)
class CSDTitan_RequestEntry_Noop : public CSDTitan_RequestEntry
{
public :
	CSDTitan_RequestEntry_Noop(CServerDirectory_Titan *pDir, IServerDirectory::ERequest eRequest);

	virtual IServerDirectory::ERequest GetRequest() const { return m_eRequest; }
private:
	IServerDirectory::ERequest m_eRequest;
};

class CSDTitan_RequestEntry_Pause : public CSDTitan_RequestEntry
{
public:
	CSDTitan_RequestEntry_Pause(CServerDirectory_Titan *pDir) : CSDTitan_RequestEntry(pDir) {}
	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Pause; }
};

class CSDTitan_RequestEntry_TitanOp : public CSDTitan_RequestEntry
{
public:
	CSDTitan_RequestEntry_TitanOp(CServerDirectory_Titan *pDir);

	// Starts the AsyncOp
	virtual void Start();
	// Kills the AsyncOp
	virtual void Cancel();
protected:

	OpCompletionBase *GetCompletionFn();

	virtual void OnCompletion();

	// Cancel the completion (to release the CS)
	virtual void CancelCompletion() { m_cCompletionCS.Leave(); }

	// Reset the state of the object
	virtual void Reset() {}
private:
	virtual AsyncOpPtr GetAsyncOp() = 0;
	virtual const char *GetStatusString() = 0;
	virtual const char *GetSuccessString() = 0;
	virtual const char *GetFailureString() = 0;

	static void Completion_Bootstrap(AsyncOpPtr pAsyncOp, CSDTitan_RequestEntry_TitanOp *pRequestEntry);

	CWinSync_CS m_cCompletionCS;
};


class CSDTitan_RequestEntry_MOTD : public CSDTitan_RequestEntry_TitanOp
{
public:
	CSDTitan_RequestEntry_MOTD(CServerDirectory_Titan *pDir);

	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_MOTD; }

protected:
	virtual void OnCompletion();
	virtual void Reset();
private:
	virtual AsyncOpPtr GetAsyncOp() { return (AsyncOpPtr)m_pMOTDOp; }
	virtual const char *GetStatusString() { return GetTempString(IDS_WON_MOTD_STATUS); }
	virtual const char *GetSuccessString() { return GetTempString(IDS_WON_MOTD_SUCCESS); }
	virtual const char *GetFailureString() { return GetTempString(IDS_WON_MOTD_FAIL); }

	GetMOTDOpPtr m_pMOTDOp;
};

class CSDTitan_RequestEntry_Validate_Version : public CSDTitan_RequestEntry_TitanOp
{
public:
	CSDTitan_RequestEntry_Validate_Version(CServerDirectory_Titan *pDir);

	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Validate_Version; }

protected:
	virtual void OnCompletion();
	virtual void Reset();
private:
	virtual AsyncOpPtr GetAsyncOp() { return m_pActiveOp; }
	virtual const char *GetStatusString() { return GetTempString(IDS_WON_VERSION_STATUS); }
	virtual const char *GetSuccessString() { return GetTempString(IDS_WON_VERSION_SUCCESS); }
	virtual const char *GetFailureString() { return GetTempString(IDS_WON_VERSION_FAIL); }

	AsyncOpPtr m_pActiveOp;
	GetDirOpPtr m_pGetDirOp;
	CheckValidVersionOpPtr m_pVersionOp;
};

class CSDTitan_RequestEntry_Validate_CDKey : public CSDTitan_RequestEntry_TitanOp
{
public:
	CSDTitan_RequestEntry_Validate_CDKey(CServerDirectory_Titan *pDir);

	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Validate_CDKey; }
protected:
	virtual void OnCompletion();
	virtual void Reset();
private:
	virtual AsyncOpPtr GetAsyncOp() { return m_pActiveOp; }
	virtual const char *GetStatusString() { return GetTempString(IDS_WON_CDKEY_STATUS); }
	virtual const char *GetSuccessString() { return GetTempString(IDS_WON_CDKEY_SUCCESS); }
	virtual const char *GetFailureString() { return GetTempString(IDS_WON_CDKEY_FAIL); }

	AsyncOpPtr m_pActiveOp;
	GetDirOpPtr m_pGetDirOp;
	GetCertOpPtr m_pGetCertOp;
};

class CSDTitan_RequestEntry_Publish_Server : public CSDTitan_RequestEntry_TitanOp
{
public:
	CSDTitan_RequestEntry_Publish_Server(CServerDirectory_Titan *pDir);

	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Publish_Server; }
protected:
	virtual void OnCompletion();
	virtual void Reset();
private:
	virtual AsyncOpPtr GetAsyncOp() { return m_pActiveOp; }
	virtual const char *GetStatusString() { return GetTempString(IDS_WON_PUB_STATUS); }
	virtual const char *GetSuccessString() { return GetTempString(IDS_WON_PUB_SUCCESS); }
	virtual const char *GetFailureString() { return GetTempString(IDS_WON_PUB_FAIL); }

	AsyncOpPtr m_pActiveOp;
	AddServiceOpPtr m_pAddServiceOp;
	RemoveServiceOpPtr m_pRemoveServiceOp;
};

class CSDTitan_RequestEntry_Remove_Server : public CSDTitan_RequestEntry_TitanOp
{
public:
	CSDTitan_RequestEntry_Remove_Server(CServerDirectory_Titan *pDir);

	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Remove_Server; }
protected:
	virtual void Reset();
private:
	virtual AsyncOpPtr GetAsyncOp() { return (AsyncOpPtr)m_pRemoveServiceOp; }
	virtual const char *GetStatusString() { return GetTempString(IDS_WON_REM_STATUS); }
	virtual const char *GetSuccessString() { return GetTempString(IDS_WON_REM_SUCCESS); }
	virtual const char *GetFailureString() { return GetTempString(IDS_WON_REM_FAIL); }

	RemoveServiceOpPtr m_pRemoveServiceOp;
};

class CSDTitan_RequestEntry_UpdateList : public CSDTitan_RequestEntry_TitanOp
{
public:
	CSDTitan_RequestEntry_UpdateList(CServerDirectory_Titan *pDir);

	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Update_List; }

protected:
	virtual void OnCompletion();
	virtual void Reset();
private:
	virtual AsyncOpPtr GetAsyncOp() { return (AsyncOpPtr)m_pGetDirOp; }
	virtual const char *GetStatusString() { return GetTempString(IDS_WON_LIST_STATUS); }
	virtual const char *GetSuccessString() { return GetTempString(IDS_WON_LIST_SUCCESS); }
	virtual const char *GetFailureString() { return GetTempString(IDS_WON_LIST_FAIL); }

	GetDirOpPtr m_pGetDirOp;
};

// Update the pings on the servers
class CSDTitan_RequestEntry_UpdatePings : public CSDTitan_RequestEntry
{
public :
	CSDTitan_RequestEntry_UpdatePings(CServerDirectory_Titan *pDir);

	virtual ~CSDTitan_RequestEntry_UpdatePings();

	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Update_Pings; }

	virtual void Start();
	virtual void Cancel();

protected:
	virtual void Reset();

	void QueueAddressesForPing();
	void RetireFinishedPings();
	void FlushPingQueue();
	void SavePingResult(const IPAddr &cAddr, uint32 nPingTime);

	enum {
		k_nPingDelay = 10, // Number of ms to wait between sending out pings
		k_nPingCount = 5, // Number of pings to send out at a time
	};

	struct SPingReq
	{
		IPAddr m_cAddr;
		uint32 m_nID;
	};

	typedef std::vector<IPAddr> TAddrList;
	TAddrList m_aAddressQueue;
	typedef std::vector<SPingReq> TPingList;
	TPingList m_aPingQueue;

	CWinSync_Event m_cCancelEvent;
	CWinSync_Event m_cBusyEvent, m_cNotBusyEvent;
};

// Base class for a message-based request entry
class CSDTitan_RequestEntry_MsgOp : public CSDTitan_RequestEntry
{
public:
	CSDTitan_RequestEntry_MsgOp(CServerDirectory_Titan *pDir);

	virtual bool OnMessage(ILTMessage_Read &cMsg, const IPAddr &cSource);
	virtual void Cancel();
	virtual void Reset();
protected:
	enum EWaitResult {
		eWait_Success = 0,
		eWait_TimeOut = 1,
		eWait_Cancel = 2,
	};
	virtual EWaitResult WaitForMessage(uint32 nTimeout);
private:
	virtual bool HandleMessage(ILTMessage_Read &cMsg) { return true; }

protected:
	CWinSync_Event m_cCancelEvent, m_cCancelReadyEvent;
	CWinSync_Event m_cContinueEvent;
	CWinSync_Event m_cIsWaiting;
	IPAddr m_cDestination;
};

class CSDTitan_RequestEntry_Peer_Details : public CSDTitan_RequestEntry_MsgOp
{
public:
	CSDTitan_RequestEntry_Peer_Details(CServerDirectory_Titan *pDir);

	virtual void Start();

private:
	enum {
		k_nResponseTimeout = 1000,
		k_nNumRetries = 5,
	};
	virtual bool HandleMessage(ILTMessage_Read &cMsg);
};

class CSDTitan_RequestEntry_Validate_Client : public CSDTitan_RequestEntry_MsgOp
{
public:
	CSDTitan_RequestEntry_Validate_Client(CServerDirectory_Titan *pDir);

	virtual void Start();
private:
	virtual bool HandleMessage(ILTMessage_Read &cMsg);
};

class CSDTitan_RequestEntry_Validate_Server : public CSDTitan_RequestEntry
{
public:
	CSDTitan_RequestEntry_Validate_Server(CServerDirectory_Titan *pDir);
	virtual IServerDirectory::ERequest GetRequest() const { return IServerDirectory::eRequest_Validate_Server; }
	virtual void Start();
};

#endif //__CSERVERDIR_TITAN_H__
