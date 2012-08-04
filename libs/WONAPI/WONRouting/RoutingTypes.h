#ifndef __WON_ROUTINGTYPES_H__
#define __WON_ROUTINGTYPES_H__
#include "WONShared.h"
#include "WONCommon/SmartPtr.h"
#include "WONCommon/ByteBuffer.h"
#include <list>
#include <map>

namespace WONAPI
{

enum RoutingIds
{
	RoutingId_ClientMin					= 2048, 
	RoutingId_ClientMax					= 0xfffe, 
	RoutingId_Global					= 0xffff,
	RoutingId_Invalid					= 0xffff
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingClientFlags
{

	RoutingClientFlag_IsModerator						= 0x0001,
	RoutingClientFlag_IsMuted							= 0x0002,
	RoutingClientFlag_DirtyWordFilteringOff				= 0x0004,
	RoutingClientFlag_IsAway							= 0x0008,
	RoutingClientFlag_IsNotBehindFirewall               = 0x0010

	// High order 8 bits (0x7f000000) allowed for developer use
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingMemberFlags
{
	RoutingMemberFlag_IsModerator						= 0x0001,
	RoutingMemberFlag_IsMuted							= 0x0002,
	RoutingMemberFlag_IsObserver						= 0x0004
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingGroupFlags
{ 
	RoutingGroupFlag_Claimed								= 0x0001,
	RoutingGroupFlag_Closed									= 0x0002,
	RoutingGroupFlag_PasswordProtected						= 0x0004,
	RoutingGroupFlag_AllowOutsidersToChat					= 0x0008,
	RoutingGroupFlag_AllowObservers							= 0x0010,
	RoutingGroupFlag_AllowObserversToChat					= 0x0020,
	RoutingGroupFlag_DetailedObservers						= 0x0040,
	RoutingGroupFlag_InviteOnly								= 0x0080,
	RoutingGroupFlag_AskCaptainToJoin						= 0x0100,
	RoutingGroupFlag_AskCaptainToObserve					= 0x0200,
	RoutingGroupFlag_NoCaptain								= 0x0400,
	RoutingGroupFlag_IsChatRoom								= 0x0800

	// High order 8 bits (0x7f000000) allowed for developer use
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingGroupAsyncFlags
{ 
	RoutingGroupAsyncFlag_DistributeClientName           = 0x0001,
	RoutingGroupAsyncFlag_DistributeClientFlags          = 0x0002,
	RoutingGroupAsyncFlag_RestrictGroupCreated           = 0x0004,
	RoutingGroupAsyncFlag_RestrictGroupDeleted           = 0x0008,
	RoutingGroupAsyncFlag_RestrictGroupCaptainChanged    = 0x0010,
	RoutingGroupAsyncFlag_RestrictGroupFlagsChanged      = 0x0020,
	RoutingGroupAsyncFlag_RestrictGroupMaxPlayersChanged = 0x0040,
	RoutingGroupAsyncFlag_RestrictGroupNameChanged       = 0x0080,
	RoutingGroupAsyncFlag_RestrictGroupObserverCount     = 0x0100,
	RoutingGroupAsyncFlag_RestrictClientJoinedGroup      = 0x0200,
	RoutingGroupAsyncFlag_RestrictClientLeftGroup        = 0x0400,
	RoutingGroupAsyncFlag_RestrictGroupMemberCount       = 0x0800
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingChatFlags
{
	RoutingChatFlag_SendReply								= 0x0001,
	RoutingChatFlag_IsEmote									= 0x0002,
	RoutingChatFlag_IsWhisper								= 0x0004,
	RoutingChatFlag_SkipMe									= 0x0008
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingRegisterClientFlags
{
	RoutingRegisterClientFlag_LoginTypeNormal				= 0x0000,
	RoutingRegisterClientFlag_LoginTypeCertificate			= 0x0001, // get name from certificate
	RoutingRegisterClientFlag_LoginTypeGuest				= 0x0002, // login as guest (name assigned as Guestxxxx)
	RoutingRegisterClientFlag_LoginInvisible				= 0x0003, // invisible login for system processes

	RoutingRegisterClientFlag_GetClientList					= 0x0004,
	RoutingRegisterClientFlag_InitialModeratorStatus        = 0x0008
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingAsyncMessageFlags 
{ 
	RoutingAsyncMessageFlag_ClientJoinedServer     = 0x000001,
	RoutingAsyncMessageFlag_ClientLeftServer       = 0x000002,
	RoutingAsyncMessageFlag_ClientFlagsChanged     = 0x000004,
	RoutingAsyncMessageFlag_ClientJoinedGroup      = 0x000008,
	RoutingAsyncMessageFlag_ClientLeftGroup        = 0x000010,
	RoutingAsyncMessageFlag_MemberFlagsChanged     = 0x000020,
	RoutingAsyncMessageFlag_GroupCreated           = 0x000040,
	RoutingAsyncMessageFlag_GroupDeleted           = 0x000080,
	RoutingAsyncMessageFlag_GroupCaptainChanged    = 0x000100,
	RoutingAsyncMessageFlag_GroupFlagsChanged      = 0x000200,
	RoutingAsyncMessageFlag_GroupMaxPlayersChanged = 0x000400,
	RoutingAsyncMessageFlag_GroupNameChanged       = 0x000800,
	RoutingAsyncMessageFlag_GroupInvitation        = 0x001000,
	RoutingAsyncMessageFlag_GroupObserverCount     = 0x002000,
	RoutingAsyncMessageFlag_DataObjectCreated      = 0x004000,
	RoutingAsyncMessageFlag_DataObjectDeleted      = 0x008000,
	RoutingAsyncMessageFlag_DataObjectModified     = 0x010000,
	RoutingAsyncMessageFlag_PeerChat               = 0x020000,
	RoutingAsyncMessageFlag_PeerData               = 0x040000,
	RoutingAsyncMessageFlag_ServerAlert            = 0x080000,
	RoutingAsyncMessageFlag_YouWereBanned          = 0x100000,
	RoutingAsyncMessageFlag_YouWereMuted           = 0x200000,
	RoutingAsyncMessageFlag_GroupMemberCount       = 0x400000
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingCreateGroupJoinFlags
{
	RoutingCreateGroupJoinFlag_DontJoin = 0x00,
	RoutingCreateGroupJoinFlag_JoinAsPlayer = 0x01,
	RoutingCreateGroupJoinFlag_JoinAsObserver = 0x02,
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingCreateDataObjectFlags
{
	RoutingCreateDataObjectFlag_ReplaceIfExists = 0x01
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingJoinGroupFlags
{
	RoutingJoinGroupFlag_GetMembersOfGroup = 0x01,
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingGetBadUserListFlags
{
	RoutingGetBadUserList_IncludeExpirationDiff             = 0x01,
	RoutingGetBadUserList_IncludeModeratorWONUserId         = 0x02,
	RoutingGetBadUserList_IncludeModeratorName              = 0x04,
	RoutingGetBadUserList_IncludeModeratorComment           = 0x08
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingGetGroupListFlags
{
	RoutingGetGroupList_IncludeGroupName					= 0x0001,
	RoutingGetGroupList_IncludeCaptainId					= 0x0002,
	RoutingGetGroupList_IncludeMaxPlayers					= 0x0004,
	RoutingGetGroupList_IncludeGroupFlags					= 0x0008,
	RoutingGetGroupList_IncludeAsyncFlags					= 0x0010,
	RoutingGetGroupList_IncludeMemberCount					= 0x0020,
	RoutingGetGroupList_IncludeObserverCount				= 0x0040,
	RoutingGetGroupList_IncludeMembers						= 0x0080,

	RoutingGetGroupList_IncludeAll							= 0x00ff,
	RoutingGetGroupList_IncludeAllButMembers				= 0x007f
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingSendDataFlags
{
	RoutingSendDataFlag_SendReply							= 0x01,
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingReadDataObjectFlags
{
	RoutingReadDataObjectFlag_GetGroupData					= 0x0001
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingBadUserListType
{
	RoutingBadUserListType_BannedUsers = 1,
	RoutingBadUserListType_MutedUsers = 2
};

typedef std::list<unsigned short> RoutingRecipientList;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct RoutingClientInfo : public RefCount
{
	std::wstring mName;
	unsigned short mId;
	unsigned long mFlags;

	RoutingClientInfo() : mId(0), mFlags(0) {}
};

typedef SmartPtr<RoutingClientInfo> RoutingClientInfoPtr;
typedef std::map<unsigned short,RoutingClientInfoPtr> RoutingClientMap;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct RoutingMemberInfo : public RefCount
{
	unsigned short mClientId;
	unsigned char mFlags;
	RoutingClientInfoPtr mClientInfo; // initialized by RoutingConnection if group is distributing client names or flags 
	
	bool IsObserver() { return mFlags&RoutingMemberFlag_IsObserver?true:false; }
	bool IsMuted() { return (mFlags&RoutingMemberFlag_IsMuted) || (mClientInfo.get()?mClientInfo->mFlags&RoutingClientFlag_IsMuted:0); }
	bool IsModerator() { return (mFlags&RoutingMemberFlag_IsModerator) || (mClientInfo.get()?mClientInfo->mFlags&RoutingClientFlag_IsModerator:0); }

	RoutingMemberInfo() : mClientId(0), mFlags(0) {}
};
typedef SmartPtr<RoutingMemberInfo> RoutingMemberInfoPtr;
typedef std::map<unsigned short,RoutingMemberInfoPtr> RoutingMemberMap;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct RoutingGroupInfo : public RefCount
{
	std::wstring mName;
	unsigned short mId;
	unsigned short mParentId;
	unsigned short mCaptainId;
	unsigned short mMaxPlayers;
	unsigned long mFlags;
	unsigned long mAsyncFlags;
	unsigned short mObserverCount; // only applicable if non-detailed observers is on
	unsigned short mMemberCount;

	RoutingMemberMap mMemberMap;

	RoutingGroupInfo() : mId(0), mParentId(0xFFFF), mCaptainId(0), mMaxPlayers(0), mFlags(0), mAsyncFlags(0), mObserverCount(0), mMemberCount(0) {}
};

typedef SmartPtr<RoutingGroupInfo> RoutingGroupInfoPtr;
typedef std::map<unsigned short, RoutingGroupInfoPtr> RoutingGroupMap;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct RoutingDataObject : public RefCount
{
	unsigned short mLinkId;
	std::string mType;
	std::wstring mName;
	ByteBufferPtr mData;

	RoutingDataObject() : mLinkId(0) {}
};

typedef SmartPtr<RoutingDataObject> RoutingDataObjectPtr;
typedef std::list<RoutingDataObjectPtr> RoutingDataObjectList;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct RoutingBadUser
{
	std::wstring mName;
	unsigned long mWONId;
	unsigned long mExpirationDiff;
	unsigned long mModeratorWONId;
	std::wstring mModeratorName;
	std::wstring mModeratorComment;

	RoutingBadUser() : mWONId(0), mExpirationDiff(0), mModeratorWONId(0) {}
	RoutingBadUser(const std::wstring &theName, unsigned long theId, time_t theExpirationDiff, unsigned long theModeratorId, const std::wstring& theModeratorName, const std::wstring& theModeratorComment) : mName(theName), mWONId(theId), mExpirationDiff(theExpirationDiff), mModeratorWONId(theModeratorId), mModeratorName(theModeratorName), mModeratorComment(theModeratorComment) {}
};

typedef std::list<RoutingBadUser> RoutingBadUserList;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingOpType
{
	RoutingOp_Undefined = 0,
	RoutingOp_AbortServerShutdown,
	RoutingOp_AcceptClient,
	RoutingOp_BanClient,
	RoutingOp_BecomeModerator,
	RoutingOp_CancelJoinGroup,
	RoutingOp_ClientFlagsChanged,
	RoutingOp_ClientJoinedGroup,
	RoutingOp_ClientJoinedServer,
	RoutingOp_ClientLeftGroup,
	RoutingOp_ClientLeftServer,
	RoutingOp_CreateDataObject,
	RoutingOp_CreateGroup,
	RoutingOp_DataObjectCreated,
	RoutingOp_DataObjectDeleted,
	RoutingOp_DataObjectModified,
	RoutingOp_DeleteDataObject,
	RoutingOp_DetectFirewall,
	RoutingOp_DisconnectClient,
	RoutingOp_GetBadUserList,
	RoutingOp_GetClientList,
	RoutingOp_GetGroupList,
	RoutingOp_GetMembersOfGroup,
	RoutingOp_GroupCaptainChanged,
	RoutingOp_GroupCreated,
	RoutingOp_GroupDeleted,
	RoutingOp_GroupFlagsChanged,
	RoutingOp_GroupInvitation,
	RoutingOp_GroupJoinAttempt,
	RoutingOp_GroupMaxPlayersChanged,
	RoutingOp_GroupMemberCount,
	RoutingOp_GroupNameChanged,
	RoutingOp_GroupObserverCount,
	RoutingOp_InviteClient,
	RoutingOp_JoinGroup,
	RoutingOp_LeaveGroup,
	RoutingOp_MemberFlagsChanged,
	RoutingOp_ModifyDataObject,
	RoutingOp_MuteClient,
	RoutingOp_PeerChat,
	RoutingOp_PeerData,
	RoutingOp_ReadDataObject,
	RoutingOp_RegisterClient,
	RoutingOp_RelinquishCaptaincy,
	RoutingOp_SendChat,
	RoutingOp_SendComplaint,
	RoutingOp_SendData,
	RoutingOp_SendServerAlert,
	RoutingOp_ServerAlert,
	RoutingOp_ServerShutdownAborted,
	RoutingOp_ServerShutdownStarted,
	RoutingOp_SetClientFlags,
	RoutingOp_SetGroupFlags,
	RoutingOp_SetGroupMaxPlayers,
	RoutingOp_SetGroupName,
	RoutingOp_SetGroupPassword,
	RoutingOp_StartServerShutdown,
	RoutingOp_UnsubscribeDataObject,
	RoutingOp_YouWereBanned,
	RoutingOp_YouWereMuted,
	RoutingOp_DerivedServerOp,

	RoutingOp_Max
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum RoutingMsgTypes
{
	// Unused/Invalid Routing Message Type
	RoutingMsgMin                = 0,
	RoutingUndefined             = 0,

	// Routing Server Messages
	RoutingAbortServerShutdownRequest   =1,
	RoutingAcceptClientRequest          =2,
	RoutingBanClientRequest             =3,
	RoutingBecomeModeratorRequest       =4,
	RoutingCancelJoinGroupRequest       =5,
	RoutingClientFlagsChanged           =6,
	RoutingClientJoinedGroup            =7,
	RoutingClientJoinedServer           =8,
	RoutingClientLeftGroup              =9,
	RoutingClientLeftServer             =10,
	RoutingCreateDataObjectRequest      =11,
	RoutingCreateGroupReply             =12,
	RoutingCreateGroupRequest           =13,
	RoutingDataObjectCreated            =14,
	RoutingDataObjectDeleted            =15,
	RoutingDataObjectModified           =16,
	RoutingDeleteDataObjectRequest      =17,
	RoutingDetectFirewallRequest        =18,
	RoutingDetectFirewallResult         =19,
	RoutingDisconnectClientRequest      =20,
	RoutingGetBadUserListReply          =21,
	RoutingGetBadUserListRequest        =22,
	RoutingGetClientListReply           =23,
	RoutingGetClientListRequest         =24,
	RoutingGetGroupListReply            =25,
	RoutingGetGroupListRequest          =26,
	RoutingGetMembersOfAllGroupsReply   =27,
	RoutingGetMembersOfAllGroupsRequest =28,
	RoutingGetMembersOfGroupReply       =29,
	RoutingGetMembersOfGroupRequest     =30,
	RoutingGroupCaptainChanged          =31,
	RoutingGroupCreated                 =32,
	RoutingGroupDeleted                 =33,
	RoutingGroupFlagsChanged            =34,
	RoutingGroupInvitation              =35,
	RoutingGroupJoinAttempt             =36,
	RoutingGroupMaxPlayersChanged       =37,
	RoutingGroupNameChanged             =38,
	RoutingGroupObserverCount           =39,
	RoutingInviteClientRequest          =40,
	RoutingJoinGroupReply               =41,
	RoutingJoinGroupRequest             =42,
	RoutingLeaveGroupRequest            =43,
	RoutingMemberFlagsChanged           =44,
	RoutingModifyDataObjectRequest      =45,
	RoutingMuteClientRequest            =46,
	RoutingPeerChat                     =47,
	RoutingPeerData                     =48,
	RoutingReadDataObjectReply          =49,
	RoutingReadDataObjectRequest        =50,
	RoutingRegisterClientReply          =51,
	RoutingRegisterClientRequest        =52,
	RoutingRelinquishCaptaincyRequest   =53, 
	RoutingSendChatRequest              =54,
	RoutingSendDataRequest              =55,
	RoutingSendServerAlertRequest       =56,
	RoutingServerAlert                  =57,
	RoutingServerShutdownAborted        =58,
	RoutingServerShutdownStarted        =59,
	RoutingSetClientFlagsRequest        =60,
	RoutingSetGroupFlagsRequest         =61,
	RoutingSetGroupMaxPlayersRequest    =62,
	RoutingSetGroupNameRequest          =63,
	RoutingSetGroupPasswordRequest      =64,
	RoutingStartServerShutdownRequest   =65,
	RoutingStatusReply                  =66,
	RoutingSubscribeDataObjectRequest   =67,
	RoutingUnsubscribeDataObjectRequest =68,
	RoutingYouWereBanned                =69,
	RoutingYouWereMuted                 =70,
	RoutingGroupMemberCount             =71,

	RoutingLargestValuePlusOne,
};

}; // namespace WONAPI

#endif
