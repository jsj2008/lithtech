#ifndef __WON_SMARTROUTINGCONNECTION_H__
#define __WON_SMARTROUTINGCONNECTION_H__
#include "WONShared.h"

#include "RoutingConnection.h"
#include "WONCommon/StringUtil.h"
#include "AllRoutingOps.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct SmartRoutingClientInfo : public RoutingClientInfo
{
	unsigned short mNumGroups;
	bool mIgnored;
	unsigned long mKeyId;

	SmartRoutingClientInfo() : RoutingClientInfo(), mNumGroups(0), mIgnored(false), mKeyId(0) {}
};
typedef SmartPtr<SmartRoutingClientInfo> SmartRoutingClientInfoPtr;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SmartRoutingConnection : public RoutingConnection
{
public:
	enum MatchResult
	{
		MatchResult_NotFound = 0,
		MatchResult_Ambiguous,
		MatchResult_Partial,
		MatchResult_Exact
	};

	enum InputError
	{
		InputError_None,
		InputError_InvalidCommand,
		InputError_ClientNotFound, 
		InputError_ClientAmbiguous, 
		InputError_NoChat,
		InputError_NotLoggedIn,
		InputError_BadTime,
	};

	enum InputCommand
	{
		InputCommand_None,
		InputCommand_Broadcast,
		InputCommand_Whisper,
		InputCommand_Emote,
		InputCommand_Ignore,
		InputCommand_Away,
		InputCommand_BecomeModerator,
		InputCommand_ServerMute,
		InputCommand_ServerBan,
		InputCommand_Mute,
		InputCommand_Ban,
		InputCommand_Unmute,
		InputCommand_ServerUnmute,
		InputCommand_Invite,
		InputCommand_Uninvite
	};

	// Simple threadsafe data structures
	struct ClientInfo
	{
		std::wstring mName;
		unsigned short mId;
		unsigned long mFlags;
		unsigned long mKeyId;

		ClientInfo() : mId(0), mFlags(0), mKeyId(0) {}
	};

	struct MemberInfo
	{
		std::wstring mName;
		unsigned short mClientId;
		unsigned long mClientFlags;
		unsigned char mMemberFlags;

		MemberInfo() : mClientId(0), mClientFlags(0), mMemberFlags(0) {}
	};

	struct GroupInfo
	{
		std::wstring mName;
		unsigned short mId;
		unsigned short mCaptainId;
		unsigned short mMaxPlayers;
		unsigned long mFlags;
		unsigned long mAsyncFlags;
		unsigned short mObserverCount; // only applicable if non-detailed observers is on		

		GroupInfo() : mId(0), mCaptainId(0), mMaxPlayers(0), mFlags(0), mAsyncFlags(0), mObserverCount(0) {}
	};

protected:
	virtual RoutingClientInfoPtr GetNewClientInfo()	{ return new SmartRoutingClientInfo; }
	void SetClientName(RoutingClientInfo *theClient, const std::wstring &theName);

protected:
	RoutingGroupMap mGroupMap;
	RoutingClientMap mClientMap;

	RoutingClientMap::iterator mClientItr;
	RoutingGroupMap::iterator mGroupItr;
	RoutingMemberMap::iterator mMemberItr;
	RoutingGroupInfoPtr mMemberItrGroup;

	void EraseClient(const RoutingClientMap::iterator &theItr);
	void EraseClient(unsigned short theId);
	void EraseGroup(const RoutingGroupMap::iterator &theItr);
	void EraseMember(RoutingMemberMap &theMap, const RoutingMemberMap::iterator &theItr);
	SmartRoutingClientInfo* EnsureInClientMap(unsigned short theClientId);

	unsigned short mDefaultGroupId;
	RoutingClientInfoPtr mMyClient;
	unsigned short mMyClientId;
	unsigned long mAsyncFlags;

	typedef std::map<std::wstring, InputCommand> CommandStringMap;
	CommandStringMap mCommandStringMap;
	MatchResult mMatchTolerance;
	bool mServerWideCommands;
	bool mDoColonEmote;

	InputError mLastInputError;
	InputCommand mLastInputCommand;
	RoutingMemberInfoPtr mLastTargetMember;
	RoutingClientInfoPtr mLastTargetClient;
	std::wstring mLastMatchName;

	RoutingClientInfoPtr mHandlerClient;
	RoutingGroupInfoPtr mHandlerGroup;
	RoutingMemberInfoPtr mHandlerMember;

	bool mReadClientCDKeyId;

	typedef std::set<std::wstring,WStringLessNoCase> IgnoreSet;
	IgnoreSet mIgnoreSet;
	void SaveIgnoreSet();
	void LoadIgnoreSet();
	std::string mIgnoreFilePath;

	void InitHandlerVariables(RoutingClientInfo *theClient);
	void InitHandlerVariables(RoutingGroupInfo *theGroup, RoutingMemberInfo *theMember = NULL);

	virtual void RoutingOpCompleteHook(RoutingOp *theOp);

	void HandleClientFlagsChanged(RoutingClientFlagsChangedOp *theOp);
	void HandleClientJoinedGroup(RoutingClientJoinedGroupOp *theOp);
	void HandleClientJoinedServer(RoutingClientJoinedServerOp *theOp);
	void HandleClientLeftGroup(RoutingClientLeftGroupOp *theOp);
	void HandleClientLeftServer(RoutingClientLeftServerOp *theOp);
	void HandleSetClientFlags(RoutingSetClientFlagsOp *theOp);

	void HandleGetClientList		(RoutingGetClientListOp		*theOp);
	void HandleGetGroupList			(RoutingGetGroupListOp		*theOp);
	void HandleGetMembersOfGroup	(RoutingGetMembersOfGroupOp *theOp);
	void HandleGroupCaptainChanged	(RoutingGroupCaptainChangedOp *theOp);
	void HandleGroupCreated			(RoutingGroupCreatedOp		*theOp);
	void HandleGroupDeleted			(RoutingGroupDeletedOp		*theOp);
	void HandleGroupFlagsChanged	(RoutingGroupFlagsChangedOp *theOp);
	void HandleGroupNameChanged		(RoutingGroupNameChangedOp	*theOp);
	void HandleGroupObserverCount	(RoutingGroupObserverCountOp *theOp);
	void HandleGroupMaxPlayersChanged(RoutingGroupMaxPlayersChangedOp *theOp); 
	void HandleJoinGroup			(RoutingJoinGroupOp			*theOp);
	void HandleLeaveGroup			(RoutingLeaveGroupOp		*theOp);
	void HandleMemberFlagsChanged	(RoutingMemberFlagsChangedOp *theOp);
	void HandleRegisterClient		(RoutingRegisterClientOp	*theOp);
	void HandleReadDataObject		(RoutingReadDataObjectOp	*theOp);
	void HandleDataObjectCreated	(RoutingDataObjectCreatedOp *theOp);
	void HandleDataObjectModified	(RoutingDataObjectModifiedOp *theOp);
	void HandleDataObjectDeleted	(RoutingDataObjectDeletedOp	*theOp);

	// Handlers which swallow completions for ignored clients
	void CheckIgnore(RoutingOp *theOp, unsigned short theSenderId);
	void HandlePeerChat(RoutingPeerChatOp *theOp);
	void HandleGroupInvitation(RoutingGroupInvitationOp *theOp);
	void HandleGroupJoinAttempt(RoutingGroupJoinAttemptOp *theOp);

	void SetClientCDKeyIdFromDataObject(unsigned short theClientID, ByteBufferPtr theBuf);
	void SyncMemberMapWithClientMap(RoutingMemberMap &theMap);
	void SyncMemberMapWithClientMap(RoutingMemberMap &theOldMap, const RoutingMemberMap &theNewMap);
	void SyncClientMapWithIgnoreList();

	bool GetName(std::wstring &theInput, std::wstring &theName);
	bool HandleCommand(const std::wstring &theInput, int theGroupIdContext);

	virtual void CloseHook();

public:
	SmartRoutingConnection();

	CriticalSection& GetDataCrit() { return mDataCrit; }

	// Functions which return non-Threadsafe data (need to acquire class critical critical section if not in the API thread --> GetDataCrit().Enter()...GetDataCrit().Leave() or AutoCrit aCrit(GetDataCrit()) )
	RoutingClientInfoPtr GetClientRef(unsigned short theClientId);
	RoutingGroupInfoPtr GetGroupRef(unsigned short theGroupId);
	RoutingMemberInfoPtr GetMemberRef(unsigned short theClientId, int theGroupId = -1);

	MatchResult GetClientByNameRef(const std::wstring &theName, RoutingClientInfoPtr &theClientOut);
	MatchResult GetMemberByNameRef(RoutingGroupInfoPtr theGroup, const std::wstring &theName, RoutingMemberInfoPtr &theMemberOut);
	MatchResult GetMemberByNameRef(unsigned short theGroupId, const std::wstring &theName, RoutingMemberInfoPtr &theMemberOut);
	const RoutingGroupMap& GetGroupMapRef() { return mGroupMap; }
	const RoutingClientMap& GetClientMapRef() { return mClientMap; }

	// Functions which return threadsafe data (i.e. copies)
	bool GetClient(unsigned short theClientId, ClientInfo &theClient);
	bool GetGroup(unsigned short theGroupId, GroupInfo &theGroup);
	bool GetMember(unsigned short theGroupId, unsigned short theClientId, MemberInfo &theMember);
	void StartClientItr(); // Start client list traversal
	bool GetNextClient(ClientInfo &theInfo); // Get next client in the client list
	void StartGroupItr(); // Start group list traversal
	bool GetNextGroup(GroupInfo &theInfo); // Get next group in the group list
	bool StartMemberItr(unsigned short theGroupId); // Start member list traversal
	bool GetNextMember(MemberInfo &theMember); // Get next member in the member list
	unsigned short GetMyClientId() { return mMyClientId; }

	// Functions used for handling chat input
	bool HandleInput(const std::wstring &theInput, int theGroupIdContext = -1);
	InputError GetLastInputError() { return mLastInputError; }
	InputCommand GetLastInputCommand() { return mLastInputCommand; }
	RoutingMemberInfoPtr GetLastTargetMemberRef() { return mLastTargetMember; }
	RoutingClientInfoPtr GetLastTargetClientRef() { return mLastTargetClient; }
	bool GetLastTargetClient(ClientInfo &theClient);
	bool GetLastTargetMember(MemberInfo &theMember);
	const std::wstring& GetLastMatchName() { return mLastMatchName; }

	// Functions for configuring how input is handled
	void AddCommandString(const std::wstring &theStr, InputCommand theCommand); // Associate command strings with commands (may have multiple strings for each command)
	void ClearCommandStrings();
	void AddDefaultUserCommandStrings(); // w, msg, me, emote, i, ignore, away, afk
	void AddDefaultModeratorCommandStrings(); // mod, mute, smute, ban, sban, unmute, sunmute 
	void SetAllowPartialNameMatch(bool allow); // allow matching prefix of name (e.g. "/w Brian Hello" will whisper to Brian124 if there's no other user whose name starts with Brian.) 
	void SetServerWideCommands(bool serverWideCommandsOn); // commands such as whisper and ignore search the entire client list instead of just the members of the group specified in HandleInput
	void SetDoColonEmote(bool doColonEmote); // send emote when user starts chat with a ':'

	// Ignore Lists
	void SetIgnoreListFile(const std::string &thePath);
	void DisableIgnoreListFile() { SetIgnoreListFile(""); }
	void SetIgnore(RoutingClientInfo *theClient, bool on);
};

typedef SmartPtr<SmartRoutingConnection> SmartRoutingConnectionPtr;

}; // namespace WONAPI

#endif
