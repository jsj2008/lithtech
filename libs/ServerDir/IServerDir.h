//////////////////////////////////////////////////////////////////////////////
// Server directory interface header

#ifndef __ISERVERDIR_H__
#define __ISERVERDIR_H__

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SERVERDIR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SERVERDIR_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef SERVERDIR_EXPORTS
#define SERVERDIR_API __declspec(dllexport)
#else
#define SERVERDIR_API __declspec(dllimport)
#endif

#include "iltmessage.h"

class ILTCSBase;


#include <string>
#include <vector>

/* Server directory interface
	Design goals:
		Abstraction of the underlying server directory API
		Sequential asynchronous operation for a single client.  
			- This is not intended as a shared interface.  That would require
				callbacks, request ID's, and data associations.
			- Only one operation at a time will be processed.
*/

class SERVERDIR_API IServerDirectory
{
public:
	virtual ~IServerDirectory() {}

// Types
public:

	// The requests that can be performed by the server directory
	enum ERequest {
		// Don't do anything
		eRequest_Nothing, 
		// Pause the request processing
		// Note : The result of this request will not be considered either 
		// a success or an error, and will preserve the most recent state of
		// processing.
		eRequest_Pause,
		// Get the active Message Of The Day
		eRequest_MOTD, 
		// Get the current game version information
		eRequest_Validate_Version, 
		// Validate the current CD key
		eRequest_Validate_CDKey, 
		// Validate the client against the active peer
		eRequest_Validate_Client, 
		// Validate the server
		// Note : This must be processed on the server side in order to validate clients
		eRequest_Validate_Server, 
		// Publish the active peer as a server
		// Note : This request requires that the active peer be set to local 
		// (i.e. NULL) and that all active peer info be filled in.
		eRequest_Publish_Server,
		// Remove the active peer from the directory list
		// Note : This request requires that the active peer be set to local 
		eRequest_Remove_Server,
		// Update the current list of available servers
		eRequest_Update_List,
		// Update the pings of the current server list
		eRequest_Update_Pings,
		// Get the name for the active peer
		eRequest_Peer_Name,
		// Get the summary information for the active server
		eRequest_Peer_Summary,
		// Get the detail information for the active server
		eRequest_Peer_Details,

		// Tag for determining the number of requests
		eRequest_TotalNum
	};

	// A list of requests
	typedef std::vector<ERequest> TRequestList;

	// The results of a request
	enum ERequestResult {
		// The request succeeded
		eRequestResult_Success,
		// The request is still being processed
		eRequestResult_InProgress,
		// The request never got processed
		eRequestResult_Aborted,
		// The request failed
		eRequestResult_Failed,
		// The active peer's name was not set at the time of the request
		eRequestResult_Invalid_PeerInfo_Name,
		// The active peer's summary info was not set at the time of the request
		eRequestResult_Invalid_PeerInfo_Summary,
		// The active peer's detail info was not set at the time of the request
		eRequestResult_Invalid_PeerInfo_Details,
		// The current CD key was invalid
		eRequestResult_Invalid_CDKey,

		// Tag for determining the number of results
		eRequestResult_TotalNum
	};

	// Status of the server directory interface
	enum EStatus {
		// No requests are active or pending
		eStatus_Waiting,
		// Requests are being processed
		eStatus_Processing,
		// Requests are pending, but not active
		eStatus_Paused,
		// Request processing is paused due to an error in a request
		// Note : When an error is encountered, processing will be paused,
		// and the the request will be removed from the queue
		eStatus_Error,

		// Tag for determining the number of status states
		eStatus_TotalNum
	};

	// Message Of The Day types
	enum EMOTD {
		// The system MOTD
		eMOTD_System,
		// The game-specific MOTD
		eMOTD_Game,

		// Tag for determining the number of MOTD types
		eMOTD_TotalNum
	};

	// Peer info types
	enum EPeerInfo {
		// The port of the peer (message contains one uint16)
		ePeerInfo_Port,
		// The name of the peer (message contains one string)
		ePeerInfo_Name,
		// The summary information of the peer (game-dependent)
		ePeerInfo_Summary,
		// The detail information of the peer (game-dependent)
		ePeerInfo_Details,
		// The service-dependent peer info.
		ePeerInfo_Service,
		// Has the peer been validated? (message contains one byte 0/1)
		ePeerInfo_Validated,
		// How long (in seconds) has this peer been in the list? (message contains one float)
		ePeerInfo_Age,
		// What's the response time of the peer (message contains one uint16)
		ePeerInfo_Ping,

		// Tag for determining the number of peer info types
		ePeerInfo_TotalNum
	};

	// A list of servers
	typedef std::vector<std::string> TPeerList;

// Functions
public:

	//////////////////////////////////////////////////////////////////////////////
	// Request handling

	// Add a request to the end of the queue
	// Returns false if unable to add the request to the queue
	// Note : A return value of true does NOT imply that the request completed
	// successfully.
	virtual bool QueueRequest(ERequest eNewRequest) = 0;
	// Add a list of requests to the queue
	// Returns false if any of the requests could not be added.  (And does not
	// add any of them in that case.)
	virtual bool QueueRequestList(const TRequestList &cNewRequests) = 0;
	// Retrieve the list of waiting requests.  
	// The first entry is the entry which is currently being processed.
	virtual TRequestList GetWaitingRequestList() const = 0;
	// Clear the request list
	// Note : Request list processing must be paused to clear the request list
	// Returns false if the request list is currently being processed.
	virtual bool ClearRequestList() = 0;
	// Pause processing, process the given request, return the result,
	// and go back to the previous state.
	// Returns the result of the request
	virtual ERequestResult ProcessRequest(ERequest eNewRequest, uint32 nTimeout) = 0;

	// Pauses the request list processing
	// Note : This will cancel the active request, and schedule it for 
	// re-processing when processing is resumed.
	virtual bool PauseRequestList() = 0;
	// Process (or continue processing) the request list
	virtual bool ProcessRequestList() = 0;

	// Wait until the next request is completed and return the result
	virtual ERequestResult BlockOnActiveRequest(uint32 nTimeout) = 0;
	// Wait until the next request matching eBlockRequest is completed and 
	// return its result
	// Note : Returns eRequestResult_Aborted if an earlier request fails
	virtual ERequestResult BlockOnRequest(ERequest eBlockRequest, uint32 nTimeout) = 0;
	// Wait until the next request matching one of the entries in 
	// eBlockRequestList is completed and return its result
	// Note : Returns eRequestResult_Aborted if an earlier request fails
	virtual ERequestResult BlockOnRequestList(const TRequestList &cBlockRequestList, uint32 nTimeout) = 0;
	// Wait until we go out of the processing state
	virtual ERequestResult BlockOnProcessing(uint32 nTimeout) = 0;

	// Is this request in request list?
	virtual bool IsRequestPending(ERequest ePendingRequest) const = 0;

	// A list of servers
	typedef std::vector<std::string> TPeerList;
	// Returns the most recently successful request
	virtual ERequest GetLastSuccessfulRequest() const = 0;
	// Returns the most recently failed request
	virtual ERequest GetLastErrorRequest() const = 0;
	// Returns the currently active request
	virtual ERequest GetActiveRequest() const = 0;

	// Get the most recently processed request
	virtual ERequest GetLastRequest() const = 0;
	// Get the result of the most recently processed result
	virtual ERequestResult GetLastRequestResult() const = 0;
	// Get the string associated with the most recently processed result
	virtual const char* GetLastRequestResultString() const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Status

	// Get the current status
	virtual EStatus GetCurStatus() const = 0;
	// Get the descriptive text associated with the current status
	virtual const char* GetCurStatusString() const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Startup - service specific.

	virtual void SetStartupInfo( ILTMessage_Read &cMsg ) = 0;
	virtual void GetStartupInfo( ILTMessage_Write &cMsg ) = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Game name

	virtual void SetGameName(const char *pName) = 0;
	virtual const char* GetGameName() const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// CD Keys

	// Set the current CD key
	virtual bool SetCDKey(const char *pKey) = 0;
	// Get the current CD key
	virtual bool GetCDKey(std::string *pKey) = 0;
	// Is the current CD key valid?
	// Note : This will return false until a eRequest_Validate_CDKey request has been processed
	virtual bool IsCDKeyValid() const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Game Version

	// Set the current version
	virtual void SetVersion(const char *pVersion) = 0;
	// Set the current region
	virtual void SetRegion(const char *pRegion) = 0;
	// Is the current version valid?
	// Note : Returns false if eRequest_Validate_Version has not been processed
	virtual bool IsVersionValid() const = 0;
	// Is the current version the newest version?
	// Note : Returns false if eRequest_Validate_Version has not been processed
	virtual bool IsVersionNewest() const = 0;
	// Is a patch available?
	// Note : Returns false if eRequest_Validate_Version has not been processed
	virtual bool IsPatchAvailable() const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Message of the Day

	// Is the MOTD "new"?
	// Note : Returns false if eRequest_MOTD has not been processed
	virtual bool IsMOTDNew(EMOTD eMOTD) const = 0;
	// Get the current MOTD
	virtual char const* GetMOTD(EMOTD eMOTD) const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Active peer info
	
	// Sets the active peer address and port
	// Note : Use NULL to indicate the local machine
	// Returns false if there was some problem
	virtual bool SetActivePeer(const char *pAddr) = 0;
	// Get the currently selected active peer
	// Returns false if there was some problem  (e.g. a null parameter)
	virtual bool GetActivePeer(std::string *pAddr, bool *pLocal) const = 0;
	// Remove the active peer from the peer list
	// The active peer will be set to the local machine
	// Returns false if the peer is the local machine
	virtual bool RemoveActivePeer() = 0;

	// Change the active peer info
	virtual bool SetActivePeerInfo(EPeerInfo eInfoType, ILTMessage_Read &cMsg) = 0;
	// Has the active peer info been queried from the directory server?
	// Returns true if the required information has been queried
	virtual bool HasActivePeerInfo(EPeerInfo eInfoType) const = 0;
	// Get the current active server info
	// Returns false if that information has not been queried for the active peer,
	// or if pMsg is null
	virtual bool GetActivePeerInfo(EPeerInfo eInfoType, ILTMessage_Write *pMsg) const = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Peer listing

	// Get the current list of known peers
	// Note : This list will be empty until a eRequest_Update_List request has been
	// successfully processed, or SetActivePeer has been called with a non-local peer
	virtual TPeerList GetPeerList() const = 0;
	// Clear the current list of known peers
	virtual void ClearPeerList() = 0;

	//////////////////////////////////////////////////////////////////////////////
	// Net message handling

	// Handle an incoming net message
	// Returns false if the message was not recognized
	// Note : Remove all external headers before calling this function
	virtual bool HandleNetMessage(ILTMessage_Read &cMsg, const char *pSender, uint16 nPort) = 0;
	// Set the header which will be appended to the beginning of all outgoing messages
	virtual bool SetNetHeader(ILTMessage_Read &cMsg) = 0;
};

// Create an IServerDirectory interface for the Titan API
// (delete it when you're finished)
// PARAMETER:	bClientSide			- Set to true if instantiated on the client.
// PARAMETER:	ltCSBase			- used by serverdir so it can talk to the engine directly.
// PARAMETER:	hResourceModule		- optional resource module to use with error strings.
SERVERDIR_API IServerDirectory *Factory_Create_IServerDirectory_Titan( bool bClientSide, ILTCSBase& ltCSBase, HMODULE hResourceModule );

#endif //__ISERVERDIR_H__