#ifndef __WON_WONSTATUS_H__
#define __WON_WONSTATUS_H__
#include "WONShared.h"
#include "WONCommon/Platform.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

enum WONStatus
{
	WS_Success											= 0,
	WS_Pending											= 100000,
	WS_TimedOut,
	WS_Killed,												
	WS_Incomplete,
	WS_None,												
	WS_AwaitingCompletion,
	WS_InvalidMessage,										
	WS_FailedToOpenFile,									
	WS_FailedToWriteToFile,		
	WS_BlockMode_NotSupported,
	WS_AsyncMode_NotSupported,
	WS_InvalidPublicKey,
	WS_MessageUnpackFailure,
	WS_AuthRequired,
	WS_InvalidAddress,

// AsyncSocket
	WS_AsyncSocket_InvalidSocket,
	WS_AsyncSocket_PartialSendTo,						
	WS_AsyncSocket_StreamNotAllowed,
	WS_AsyncSocket_DatagramNotAllowed,					
	WS_AsyncSocket_Shutdown,					
	WS_AsyncSocket_ConnectFailed,

// SocketOp
	WS_SocketOp_InvalidSocket,						


// RecvMsgOp
	WS_RecvMsg_InvalidLengthFieldSize,
	WS_RecvMsg_InvalidMessageLength,					

// Auth Peer to Peer
	WS_PeerAuthClient_Challenge1DecryptFailure,
	WS_PeerAuthClient_Challenge1InvalidSecretLen,				
	WS_PeerAuthClient_Challenge1InvalidSecretKey,				
	WS_PeerAuthClient_Challenge1CertificateUnpackFailure,
	WS_PeerAuthClient_Challenge1CertificateVerifyFailure,		
	WS_PeerAuthClient_Challenge2EncryptFailure,	
	WS_PeerAuthClient_CompleteDecryptFailure,				
	WS_PeerAuthClient_CompleteInvalidSecretLen,				
	WS_PeerAuthClient_CompleteInvalidSecretKey,				
	WS_PeerAuthClient_InvalidServiceType,				
	WS_PeerAuthClient_UnexpectedChallenge,					
	WS_PeerAuthClient_InvalidMessage,					
	WS_PeerAuthClient_MsgUnpackFailure,						

	WS_PeerAuthServer_NotStarted,
	WS_PeerAuthServer_InvalidServiceType,
	WS_PeerAuthServer_UnexpectedRequest,
	WS_PeerAuthServer_UnexpectedChallenge,
	WS_PeerAuthServer_InvalidMessage,
	WS_PeerAuthServer_MsgUnpackFailure,		
	WS_PeerAuthServer_InvalidAuthMode,
	WS_PeerAuthServer_InvalidEncryptType,
	WS_PeerAuthServer_InvalidClientCertificate,
	WS_PeerAuthServer_ExpiredClientCertificate,
	WS_PeerAuthServer_FailedToVerifyClientCertificate,
	WS_PeerAuthServer_FailedToEncryptWithClientPubKey,
	WS_PeerAuthServer_FailedToDecryptWithPrivateKey,
	WS_PeerAuthServer_InvalidSecretA,
	WS_PeerAuthServer_InvalidSecretB,

// PeerAuthOp
	WS_PeerAuth_GetCertFailure,

// Server Request Op
	WS_ServerReq_FailedAllServers,
	WS_ServerReq_NoServersSpecified,
	WS_ServerReq_NeedAuthContext,					
	WS_ServerReq_TryNextServer,					
	WS_ServerReq_Send,						
	WS_ServerReq_Recv,
	WS_ServerReq_ExitCommunicationLoop,
	WS_ServerReq_SessionExpired,
	WS_ServerReq_GetCertFailure,
	WS_ServerReq_InvalidReplyHeader,
	WS_ServerReq_UnpackFailure,


// Get Cert Op
	WS_GetCert_PubKeyEncryptFailure,
	WS_GetCert_InvalidPubKeyReply,				
	WS_GetCert_InvalidLoginReply,					
	WS_GetCert_UnableToFindHashFile,
	WS_GetCert_UnexpectedLoginChallenge,			
	WS_GetCert_NeedVerifierKey,			
	WS_GetCert_FailedToVerifyPubKeyBlock,
	WS_GetCert_InvalidCertificate,
	WS_GetCert_InvalidPubKeyBlock,
	WS_GetCert_DecryptFailure,
	WS_GetCert_InvalidPrivateKey,
	WS_GetCert_InvalidSecretConfirm,
	WS_GetCert_MissingCertificate,

// Auth Data
	WS_AuthData_SetCertificateUnpackFailed,
	WS_AuthData_SetPubKeyBlockUnpackFailed,			
	WS_AuthData_SetCertificateVerifyFailed,			
	WS_AuthData_SetPrivateKeyFailed,			

// Auth Session
	WS_AuthSession_EncryptFailure,
	WS_AuthSession_DecryptSessionIdMismatch,
	WS_AuthSession_DecryptFailure,		
	WS_AuthSession_DecryptBadLen,					
	WS_AuthSession_DecryptInvalidSequenceNum,
	WS_AuthSession_DecryptUnpackFailure,		

// Server Connection
	WS_ServerConnection_RecvNotAllowedInAsyncMode,

// FTP Op
	WS_FTP_InvalidResponse,
	WS_FTP_StatusError,
	WS_FTP_InvalidPasvResponse,

// HTTP Op
	WS_HTTP_InvalidHeader,
	WS_HTTP_StatusError,
	WS_HTTP_InvalidRedirect,
	WS_HTTP_TooManyRedirects,

// GetMOTD Op
	WS_GetMOTD_SysNotFound,
	WS_GetMOTD_GameNotFound,

// BaseMsgRequest 
	WS_BaseMsgRequest_NoRequest,
	WS_BaseMsgRequest_UnpackFailure,					
	WS_BaseMsgRequest_BadHeaderType,			
	
// RoutingOp
	WS_RoutingOp_ReplyUnpackFailure,
	WS_RoutingOp_DontWantReply,
	WS_RoutingOp_NeedMoreReplies,

// GameSpy Support
	WS_GameSpySupport_UnhandledError,
	WS_GameSpySupport_WinSockError,
	WS_GameSpySupport_BindError,
	WS_GameSpySupport_DNSError,
	WS_GameSpySupport_ConnError,


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Server Status   ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

	WS_CommServ_Failure								= -1,						// -1 General Failure no status
	WS_CommServ_NotAvailable						= WS_CommServ_Failure-1,    // -2 Server not available
	WS_CommServ_NotAllowed							= WS_CommServ_Failure-2,    // -3 Request not allowed
	WS_CommServ_InvalidParameters					= WS_CommServ_Failure-3,    // -4 Request had invalid parameters
	WS_CommServ_AlreadyExists						= WS_CommServ_Failure-4,    // -5 Item being added already exists
	WS_CommServ_MessageExceedsMax					= WS_CommServ_Failure-5,    // -6 Maximum message is exceeded
	WS_CommServ_SessionNotFound						= WS_CommServ_Failure-6,	// -7 Message is invalid
	WS_CommServ_NotAuthorized						= WS_CommServ_Failure-7,	// -8 Server/Client is not Authorized to perform this action
	WS_CommServ_ResourcesLow						= WS_CommServ_Failure-8,	// -9 Unable to create resources are low
	WS_CommServ_NoPortAvailable						= WS_CommServ_Failure-9,	// -10 No port in range was available
	WS_CommServ_TimedOut							= WS_CommServ_Failure-10,	// -11 A request has timed out
	WS_CommServ_NotSupported						= WS_CommServ_Failure-11,	// -12 A request is not supported
	WS_CommServ_DecryptionFailed					= WS_CommServ_Failure-12,   // -13 Decryption of a message failed
	WS_CommServ_InvalidSessionId					= WS_CommServ_Failure-13,   // -14 Bad session id in encrypted message header
	WS_CommServ_InvalidSequenceNumber				= WS_CommServ_Failure-14,   // -15 Bad sequence number in encrypted message
	WS_CommServ_InvalidMessage						= WS_CommServ_Failure-15,	// -16 Message is invalid
	WS_CommServ_BehindFirewall						= WS_CommServ_Failure-16,	// -17 Behind firewall
	WS_CommServ_BadPeerCertificate					= WS_CommServ_Failure-17,	// -18 Bad Certificate
	WS_CommServ_ExpiredPeerCertificate				= WS_CommServ_Failure-18,	// -19 Expired Certificate
	WS_CommServ_StatusCommon_BannedAddress			= WS_CommServ_Failure-19,	// -20 Request from a banned IP address

// AuthServer Status
	WS_AuthServ_FailureStart						= -1499,
    WS_AuthServ_BadPubKeyBlock						= WS_AuthServ_FailureStart-1,
    WS_AuthServ_BadCDKey							= WS_AuthServ_FailureStart-2,
	WS_AuthServ_CDKeyBanned							= WS_AuthServ_FailureStart-3,
	WS_AuthServ_CDKeyInUse							= WS_AuthServ_FailureStart-4,
	WS_AuthServ_CRCFailed							= WS_AuthServ_FailureStart-5,
    WS_AuthServ_UserExists							= WS_AuthServ_FailureStart-6,
    WS_AuthServ_UserNotFound						= WS_AuthServ_FailureStart-7,
    WS_AuthServ_BadPassword							= WS_AuthServ_FailureStart-8,
    WS_AuthServ_InvalidUserName						= WS_AuthServ_FailureStart-9,
	WS_AuthServ_BadCommunity						= WS_AuthServ_FailureStart-10,
    WS_AuthServ_InvalidCDKey						= WS_AuthServ_FailureStart-11,
    WS_AuthServ_NotInCommunity						= WS_AuthServ_FailureStart-12,
    WS_AuthServ_UserSeqInUse                        = WS_AuthServ_FailureStart-13,
    WS_AuthServ_SetCUDError							= WS_AuthServ_FailureStart-14,

// DirServer Status
	WS_DirServ_FailureStart							= -1199,
	WS_DirServ_DirNotFound							= WS_DirServ_FailureStart-1,		// -1200 Requested directory not found
	WS_DirServ_ServiceNotFound						= WS_DirServ_FailureStart-2,		// -1201 Requested service not found
	WS_DirServ_DirExists							= WS_DirServ_FailureStart-3,		// -1202 Directory already exists
	WS_DirServ_ServiceExists						= WS_DirServ_FailureStart-4,		// -1203 Service alread exists
	WS_DirServ_DirIsFull							= WS_DirServ_FailureStart-5,		// -1204 Directory is full
	WS_DirServ_AlreadyConnected						= WS_DirServ_FailureStart-6,		// -1205 Peer already connected
	WS_DirServ_EntityTooLarge						= WS_DirServ_FailureStart-7,		// -1206 Entity size (bytes) too large
	WS_DirServ_MaxDataObjects						= WS_DirServ_FailureStart-8,		// -1207 Max data object for entity exceeded
	WS_DirServ_BadDataOffset						= WS_DirServ_FailureStart-9,		// -1208 Offset exceeds length of data in data object
	WS_DirServ_InvalidPath							= WS_DirServ_FailureStart-10,		// -1209 Invalid path
	WS_DirServ_InvalidGetFlags						= WS_DirServ_FailureStart-11,		// -1210 Invalid GetFlags
	WS_DirServ_InvalidKey							= WS_DirServ_FailureStart-12,		// -1211 Invalid directory or service key
	WS_DirServ_InvalidMode							= WS_DirServ_FailureStart-13,		// -1212 Invalid GetNumEntries mode
	WS_DirServ_InvalidLifespan						= WS_DirServ_FailureStart-14,		// -1213 Invalid lifespan
	WS_DirServ_InvalidDataObject					= WS_DirServ_FailureStart-15,		// -1214 Invalid data object
	WS_DirServ_NoDataObjects						= WS_DirServ_FailureStart-16,		// -1215 No data objects when at least one required
	WS_DirServ_DataObjectExists						= WS_DirServ_FailureStart-17,		// -1216 Data object already exists.
	WS_DirServ_DataObjectNotFound					= WS_DirServ_FailureStart-18,		// -1217 Data object does not exist.
	WS_DirServ_InvalidACLType						= WS_DirServ_FailureStart-19,		// -1218 Invalid ACL type.
	WS_DirServ_PermissionExists						= WS_DirServ_FailureStart-20,		// -1219 ACL permission already exists.
	WS_DirServ_PermissionNotFound					= WS_DirServ_FailureStart-21,		// -1220 ACL permission does not exist.
	WS_DirServ_MaxPermissions						= WS_DirServ_FailureStart-22,		// -1221 Max permissions for ACL exceeded
	WS_DirServ_NoACLs								= WS_DirServ_FailureStart-23,		// -1222 No ACLs when at least one required
	WS_DirServ_MultiGetPartialFailure				= WS_DirServ_FailureStart-24,       // -1223 GetDir did not succeed on all requests
	WS_DirServ_MultiGetFailedAllRequests			= WS_DirServ_FailureStart-25,       // -1224 MultiGetDir failed for every request

// EventServer Status
	WS_EventServ_FailureStart						= -1899,
	WS_EventServ_BufferingError						= WS_EventServ_FailureStart-1,
	WS_EventServ_InvalidDateTime					= WS_EventServ_FailureStart-2, // date time passed in RecordEvent message was invalid
	WS_EventServ_InvalidDataType					= WS_EventServ_FailureStart-3,

	// Routing ServerG2
	WS_RoutingServG2_BlankNamesNotAllowed       = -2500,
	WS_RoutingServG2_CaptainRejectedYou         = -2501,
	WS_RoutingServG2_ClientAlreadyBanned        = -2502,
	WS_RoutingServG2_ClientAlreadyExists        = -2503,
	WS_RoutingServG2_ClientAlreadyInGroup       = -2504,
	WS_RoutingServG2_ClientAlreadyMuted         = -2505,
	WS_RoutingServG2_ClientAlreadyRegistered    = -2506,
	WS_RoutingServG2_ClientBanned               = -2507,
	WS_RoutingServG2_ClientDoesNotExist         = -2508,
	WS_RoutingServG2_ClientNotBanned            = -2509,
	WS_RoutingServG2_ClientNotInGroup           = -2510,
	WS_RoutingServG2_ClientNotInvited           = -2511,
	WS_RoutingServG2_ClientNotMuted             = -2512,
	WS_RoutingServG2_ClientNotPermitted         = -2513,
	WS_RoutingServG2_ConnectFailure             = -2514,
	WS_RoutingServG2_DuplicateDataObject        = -2515,
	WS_RoutingServG2_GroupAlreadyClaimed        = -2516,
	WS_RoutingServG2_GroupAlreadyExists         = -2517,
	WS_RoutingServG2_GroupClosed                = -2518,
	WS_RoutingServG2_GroupDeleted               = -2519,
	WS_RoutingServG2_GroupDoesNotExist          = -2520,
	WS_RoutingServG2_GroupFull                  = -2521,
	WS_RoutingServG2_GuestNamesReserved         = -2522,
	WS_RoutingServG2_InvalidContentType         = -2523,
	WS_RoutingServG2_InvalidPassword            = -2524,
	WS_RoutingServG2_InvalidWONUserId           = -2525,
	WS_RoutingServG2_LoginTypeNotSupported      = -2526,
	WS_RoutingServG2_MustBeAdmin                = -2527,
	WS_RoutingServG2_MustBeAuthenticated        = -2528,
	WS_RoutingServG2_MustBeCaptain              = -2529,
	WS_RoutingServG2_MustBeCaptainOrModerator   = -2530,
	WS_RoutingServG2_MustBeClient               = -2531,
	WS_RoutingServG2_MustBeModerator            = -2532,
	//		WS_RoutingServG2_MustBeOwner                = -2533,
	WS_RoutingServG2_MustBeSelf                 = -2534,
	WS_RoutingServG2_MustBeVisible              = -2535,
	WS_RoutingServG2_NoPendingJoin              = -2536,
	WS_RoutingServG2_NoUsernameInCertificate    = -2537,
	WS_RoutingServG2_ObjectDoesNotExist         = -2538,
	WS_RoutingServG2_ObserversNotAllowed        = -2539,
	WS_RoutingServG2_OffsetTooLarge             = -2540,
	WS_RoutingServG2_Pending                    = -2541,
	WS_RoutingServG2_ServerFull                 = -2542,
	WS_RoutingServG2_ShutdownTimerAlreadyExists = -2543,
	WS_RoutingServG2_ShutdownTimerDoesNotExist  = -2544,
	WS_RoutingServG2_SubscriptionAlreadyExists  = -2545,
	WS_RoutingServG2_SubscriptionDoesNotExist   = -2546,
	WS_RoutingServG2_SubscriptionDoesNotExists  = -2547,
	WS_RoutingServG2_Throttled                  = -2548,
	WS_RoutingServG2_TooManyMemberships         = -2549,
	WS_RoutingServG2_TooManyRecipients          = -2550,

	// ProfileServer status
	WS_ProfileServ_ProfileDoesNotExist			= -2600,
	WS_ProfileServ_UserNotFound					= -2601,
	WS_ProfileServ_AgeRangeNotFound				= -2602,
	WS_ProfileServ_CountryNotFound				= -2603,
	WS_ProfileServ_PartySeqAlreadySet			= -2604,
	WS_ProfileServ_EmailAlreadyUsed				= -2605,
	WS_ProfileServ_NewsletterEntryExists		= -2606,
	WS_ProfileServ_PartySourceEntryExists		= -2607,
	WS_ProfileServ_BadWONUserPassword			= -2608,
	WS_ProfileServ_WONUserAlreadyExists			= -2609,
	WS_ProfileServ_DatabaseError				= -2610,
	WS_ProfileServ_InvalidWONUserName			= -2611,
	WS_ProfileServ_AccountCreationDisabled		= -2612,
	WS_ProfileServ_OtherError					= -2613,
	WS_ProfileServ_UnfoundNewsletterName		= -2614,
	WS_ProfileServ_TooManyNewsletters			= -2615,
	WS_ProfileServ_NoNewslettersProvided		= -2616,
	WS_ProfileServ_RemoveNewsLettersFailed		= -2617,

	// DBProxy Server
	WS_DBProxyServ_DBError						= -2800,
	WS_DBProxyServ_NoData						= -2801,
	WS_DBProxyServ_NoModule						= -2802,
	WS_DBProxyServ_UserExists					= -2803,
	WS_DBProxyServ_InvalidUserName				= -2804,
	WS_DBProxyServ_KeyInUse						= -2805,
	WS_DBProxyServ_UserDoesNotExist				= -2806,
	WS_DBProxyServ_InvalidCDKey					= -2807,
	WS_DBProxyServ_KeyNotUsed					= -2808,
	WS_DBProxyServ_EMailPasswordDuplication		= -2809,
	WS_DBProxyServ_AccountCreateDisabled		= -2810,
	WS_DBProxyServ_InvalidPassword				= -2811,
	WS_DBProxyServ_AgeNotOldEnough				= -2812,
	WS_DBProxyServ_ItemHasDependencies			= -2813,
	WS_DBProxyServ_OutOfDate					= -2814,	// invalid version
	WS_DBProxyServ_OutOfDateNoUpdate			= -2815,	// no pathches
	WS_DBProxyServ_ValidNotLatest				= -2816,	// optional patch available

	WS_DBProxyServ_GameSpecificErrorStart		= -2900,
	WS_DBProxyServ_GameSpecificWarriorNameDoesNotExist = -2903,		// Tribes2 Specific
	WS_DBProxyServ_GameSpecificErrorEnd			= -2999,


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////     Winsock Status   ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
	WS_WSAEINTR											= WSAEINTR,
	WS_WSAEBADF											= WSAEBADF,
	WS_WSAEACCES										= WSAEACCES,
	WS_WSAEFAULT										= WSAEFAULT, 
	WS_WSAEINVAL										= WSAEINVAL,  
	WS_WSAEMFILE										= WSAEMFILE, 

	WS_WSAEWOULDBLOCK									= WSAEWOULDBLOCK,         
	WS_WSAEINPROGRESS									= WSAEINPROGRESS,
	WS_WSAEALREADY										= WSAEALREADY,
	WS_WSAENOTSOCK										= WSAENOTSOCK,
	WS_WSAEDESTADDRREQ									= WSAEDESTADDRREQ,
	WS_WSAEMSGSIZE										= WSAEMSGSIZE,
	WS_WSAEPROTOTYPE									= WSAEPROTOTYPE,
	WS_WSAENOPROTOOPT									= WSAENOPROTOOPT,         
	WS_WSAEPROTONOSUPPORT								= WSAEPROTONOSUPPORT,
	WS_WSAESOCKTNOSUPPORT								= WSAESOCKTNOSUPPORT,
	WS_WSAEOPNOTSUPP									= WSAEOPNOTSUPP,
	WS_WSAEPFNOSUPPORT									= WSAEPFNOSUPPORT,
	WS_WSAEAFNOSUPPORT									= WSAEAFNOSUPPORT,
	WS_WSAEADDRINUSE									= WSAEADDRINUSE,
	WS_WSAEADDRNOTAVAIL									= WSAEADDRNOTAVAIL,
	WS_WSAENETDOWN										= WSAENETDOWN,	

	WS_WSAENETUNREACH									= WSAENETUNREACH,
	WS_WSAENETRESET										= WSAENETRESET,           
	WS_WSAECONNABORTED									= WSAECONNABORTED,        
	WS_WSAECONNRESET									= WSAECONNRESET,    
	WS_WSAENOBUFS										= WSAENOBUFS,       
	WS_WSAEISCONN										= WSAEISCONN,           
	WS_WSAENOTCONN										= WSAENOTCONN,           
	WS_WSAESHUTDOWN										= WSAESHUTDOWN,          
	WS_WSAETOOMANYREFS									= WSAETOOMANYREFS,        
	WS_WSAETIMEDOUT										= WSAETIMEDOUT,          
	WS_WSAECONNREFUSED									= WSAECONNREFUSED,      
	WS_WSAELOOP											= WSAELOOP,              
	WS_WSAENAMETOOLONG									= WSAENAMETOOLONG,      
	WS_WSAEHOSTDOWN										= WSAEHOSTDOWN,          
	WS_WSAEHOSTUNREACH									= WSAEHOSTUNREACH,       
	WS_WSAENOTEMPTY										= WSAENOTEMPTY,         
	WS_WSAEPROCLIM										= WSAEPROCLIM,           
	WS_WSAEUSERS										= WSAEUSERS,             
	WS_WSAEDQUOT										=  WSAEDQUOT,            
	WS_WSAESTALE										=  WSAESTALE,            
	WS_WSAEREMOTE										= WSAEREMOTE,            

	WS_WSASYSNOTREADY 									= WSASYSNOTREADY,        
	WS_WSAVERNOTSUPPORTED 								= WSAVERNOTSUPPORTED,    
	WS_WSANOTINITIALISED								= WSANOTINITIALISED,    
	WS_WSAEDISCON 										= WSAEDISCON,
//	WS_EPIPE											= EPIPE,
#else
	WS_WSAEINTR											= EINTR,
	WS_WSAEBADF 										= EBADF,
	WS_WSAEACCES 										= EACCES,
	WS_WSAEFAULT 										= EFAULT, 
	WS_WSAEINVAL 										= EINVAL,  
	WS_WSAEMFILE 										= EMFILE, 

	WS_WSAEWOULDBLOCK 									= EWOULDBLOCK,         
	WS_WSAEINPROGRESS 									= EINPROGRESS,
	WS_WSAEALREADY 										= EALREADY,
	WS_WSAENOTSOCK 										= ENOTSOCK,
	WS_WSAEDESTADDRREQ 									= EDESTADDRREQ,
	WS_WSAEMSGSIZE 										= EMSGSIZE,
	WS_WSAEPROTOTYPE 									= EPROTOTYPE,
	WS_WSAENOPROTOOPT 									= ENOPROTOOPT,         
	WS_WSAEPROTONOSUPPORT 								= EPROTONOSUPPORT,
	WS_WSAESOCKTNOSUPPORT 								= ESOCKTNOSUPPORT,
	WS_WSAEOPNOTSUPP 									= EOPNOTSUPP,
	WS_WSAEPFNOSUPPORT 									= EPFNOSUPPORT,
	WS_WSAEAFNOSUPPORT 									= EAFNOSUPPORT,
	WS_WSAEADDRINUSE 									= EADDRINUSE,
	WS_WSAEADDRNOTAVAIL 								= EADDRNOTAVAIL,
	WS_WSAENETDOWN 										= ENETDOWN,	

	WS_WSAENETUNREACH 									= ENETUNREACH,
	WS_WSAENETRESET 									= ENETRESET,           
	WS_WSAECONNABORTED 									= ECONNABORTED,        
	WS_WSAECONNRESET 									= ECONNRESET,    
	WS_WSAENOBUFS 										= ENOBUFS,       
	WS_WSAEISCONN 										= EISCONN,           
	WS_WSAENOTCONN 										= ENOTCONN,           
	WS_WSAESHUTDOWN 									= ESHUTDOWN,          
	WS_WSAETOOMANYREFS 									= ETOOMANYREFS,        
	WS_WSAETIMEDOUT 									= ETIMEDOUT,          
	WS_WSAECONNREFUSED 									= ECONNREFUSED,      
	WS_WSAELOOP 										= ELOOP,              
	WS_WSAENAMETOOLONG 									= ENAMETOOLONG,      
	WS_WSAEHOSTDOWN 									= EHOSTDOWN,          
	WS_WSAEHOSTUNREACH 									= EHOSTUNREACH,       
	WS_WSAHOST_NOT_FOUND 								= HOST_NOT_FOUND,     
	WS_EPIPE											= EPIPE,
#endif // WIN32

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const char* WONStatusToString(WONStatus theError);

}; // namespace WONAPI

#endif
