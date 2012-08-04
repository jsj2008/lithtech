#include "WONAPI.h"
#include "SmartRoutingConnection.h"
#include "AllRoutingOps.h"

using namespace WONAPI;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SmartRoutingConnection::SmartRoutingConnection()
{
	mDefaultGroupId = 0;
	mMyClientId = 0;
	mLastInputError = InputError_None;
	mIgnoreFilePath = WONAPICore::GetDefaultFileDirectory() + "_wonignore.bin";
	mDoColonEmote = true;
	mReadClientCDKeyId = false;

	AddDefaultUserCommandStrings();
	AddDefaultModeratorCommandStrings();
	SetAllowPartialNameMatch(true);
	SetServerWideCommands(false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingClientInfoPtr SmartRoutingConnection::GetClientRef(unsigned short theClientId)
{
	AutoCrit aCrit(mDataCrit);

	if(mHandlerClient.get()!=NULL && mHandlerClient->mId==theClientId)
		return mHandlerClient;

	RoutingClientMap::iterator anItr = mClientMap.find(theClientId);
	if(anItr==mClientMap.end())
		return NULL;
	else
		return anItr->second;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingGroupInfoPtr SmartRoutingConnection::GetGroupRef(unsigned short theGroupId)
{
	AutoCrit aCrit(mDataCrit);

	if(mHandlerGroup.get()!=NULL && mHandlerGroup->mId==theGroupId)
		return mHandlerGroup;

	RoutingGroupMap::iterator anItr = mGroupMap.find(theGroupId);
	if(anItr==mGroupMap.end())
		return NULL;
	else
		return anItr->second;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingMemberInfoPtr SmartRoutingConnection::GetMemberRef(unsigned short theClientId, int theGroupId)
{
	AutoCrit aCrit(mDataCrit);
	if(theGroupId==-1)
		theGroupId = mDefaultGroupId;

	if(mHandlerMember.get()!=NULL && mHandlerMember->mClientId==theClientId && mHandlerGroup->mId==theGroupId)
		return mHandlerMember;

	RoutingGroupInfoPtr aGroup = GetGroupRef(theGroupId);
	if(aGroup.get()==NULL)
		return NULL;

	RoutingMemberMap::iterator anItr = aGroup->mMemberMap.find(theClientId);
	if(anItr==aGroup->mMemberMap.end())
		return NULL;
	else
		return anItr->second;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool CopyClient(SmartRoutingConnection::ClientInfo &theClientCopy, RoutingClientInfo *theClient)
{
	if(theClient==NULL)
		return false;

	theClientCopy.mFlags = theClient->mFlags;
	theClientCopy.mId = theClient->mId;
	theClientCopy.mName = theClient->mName;

	SmartRoutingClientInfo* aSmartClient = (SmartRoutingClientInfo*)theClient;
	theClientCopy.mKeyId = aSmartClient->mKeyId;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool CopyGroup(SmartRoutingConnection::GroupInfo &theGroupCopy, RoutingGroupInfo *theGroup)
{
	if(theGroup==NULL)
		return false;

	theGroupCopy.mCaptainId = theGroup->mCaptainId;
	theGroupCopy.mFlags = theGroup->mFlags;
	theGroupCopy.mAsyncFlags = theGroup->mAsyncFlags;
	theGroupCopy.mId = theGroup->mId;
	theGroupCopy.mMaxPlayers = theGroup->mMaxPlayers;
	theGroupCopy.mName = theGroup->mName;
	theGroupCopy.mObserverCount = theGroup->mObserverCount;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool CopyMember(SmartRoutingConnection::MemberInfo &theMemberCopy, RoutingMemberInfo *theMember)
{
	if(theMember==NULL)
		return false;
	
	RoutingClientInfo *aClient = theMember->mClientInfo;
	if(aClient!=NULL)
	{
		theMemberCopy.mName = aClient->mName;
		theMemberCopy.mClientFlags = aClient->mFlags;
	}

	theMemberCopy.mClientId = theMember->mClientId;
	theMemberCopy.mMemberFlags = theMember->mFlags;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::GetClient(unsigned short theClientId, ClientInfo &theClient)
{
	AutoCrit aCrit(mDataCrit);
	RoutingClientInfoPtr aClient = GetClientRef(theClientId);
	return CopyClient(theClient,aClient);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::GetGroup(unsigned short theGroupId, GroupInfo &theGroup)
{
	AutoCrit aCrit(mDataCrit);
	RoutingGroupInfoPtr aGroup = GetGroupRef(theGroupId);
	return CopyGroup(theGroup,aGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::GetMember(unsigned short theGroupId, unsigned short theClientId, MemberInfo &theMember)
{
	AutoCrit aCrit(mDataCrit);
	RoutingMemberInfoPtr aMember = GetMemberRef(theClientId,theGroupId);
	return CopyMember(theMember,aMember);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::StartClientItr()
{
	AutoCrit aCrit(mDataCrit);
	mClientItr = mClientMap.begin();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::GetNextClient(ClientInfo &theInfo)
{
	AutoCrit aCrit(mDataCrit);
	if(mClientItr==mClientMap.end())
		return false;

	CopyClient(theInfo,mClientItr->second);
	++mClientItr;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::StartGroupItr()
{
	AutoCrit aCrit(mDataCrit);
	mGroupItr = mGroupMap.begin();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::GetNextGroup(GroupInfo &theInfo)
{
	AutoCrit aCrit(mDataCrit);
	if(mGroupItr==mGroupMap.end())
		return false;

	CopyGroup(theInfo,mGroupItr->second);
	++mGroupItr;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::StartMemberItr(unsigned short theGroupId)
{
	AutoCrit aCrit(mDataCrit);
	mMemberItrGroup = GetGroupRef(theGroupId);

	if(mMemberItrGroup.get()==NULL)
		return false;

	mMemberItr = mMemberItrGroup->mMemberMap.begin();
	return true;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::GetNextMember(MemberInfo &theMember)
{
	AutoCrit aCrit(mDataCrit);
	if(mMemberItrGroup.get()==NULL)
		return false;

	if(mMemberItr==mMemberItrGroup->mMemberMap.end())
		return false;

	CopyMember(theMember,mMemberItr->second);
	++mMemberItr;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::EraseClient(const RoutingClientMap::iterator &theItr)
{
	if(theItr==mClientItr)
		++mClientItr;

	mClientMap.erase(theItr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::EraseClient(unsigned short theId)
{
	RoutingClientMap::iterator anItr = mClientMap.find(theId);
	if(anItr!=mClientMap.end())
		EraseClient(anItr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::EraseGroup(const RoutingGroupMap::iterator &theItr)
{
	if(theItr->second.get()==mMemberItrGroup.get())
		mMemberItrGroup = NULL;

	if(theItr==mGroupItr)
		++mGroupItr;

	mGroupMap.erase(theItr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::EraseMember(RoutingMemberMap &theMap, const RoutingMemberMap::iterator &theItr)
{
	if(theItr==mMemberItr)
		++mMemberItr;

	theMap.erase(theItr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SetClientName(RoutingClientInfo *theClient, const std::wstring &theName)
{
	SmartRoutingClientInfo *aClient = (SmartRoutingClientInfo*)theClient;
	aClient->mName = theName;
	aClient->mIgnored = mIgnoreSet.find(theName)!=mIgnoreSet.end();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SmartRoutingClientInfo* SmartRoutingConnection::EnsureInClientMap(unsigned short theClientId)
{
	RoutingClientMap::iterator anItr = mClientMap.insert(RoutingClientMap::value_type(theClientId, reinterpret_cast<RoutingClientInfo*>(NULL))).first;
	if(anItr->second==NULL)
	{
		anItr->second = GetNewClientInfo();
		anItr->second->mId = theClientId;
	}

	return (SmartRoutingClientInfo*)anItr->second.get();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::InitHandlerVariables(RoutingClientInfo *theClient)
{
	mHandlerClient = theClient;
	mHandlerGroup = NULL;
	mHandlerMember = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::InitHandlerVariables(RoutingGroupInfo *theGroup, RoutingMemberInfo *theMember)
{
	mHandlerGroup = theGroup;
	mHandlerMember = theMember;
	if(theMember!=NULL)
		mHandlerClient = theMember->mClientInfo;
	else
		mHandlerClient = NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::RoutingOpCompleteHook(RoutingOp *theOp)
{
	AutoCrit aCrit(mDataCrit);
	mHandlerClient = NULL;
	mHandlerGroup = NULL;
	mHandlerMember = NULL;

	switch(theOp->GetType())
	{
		case RoutingOp_ClientFlagsChanged:		HandleClientFlagsChanged	((RoutingClientFlagsChangedOp*)	theOp);	break;
		case RoutingOp_ClientJoinedGroup:		HandleClientJoinedGroup		((RoutingClientJoinedGroupOp*)	theOp);	break;
		case RoutingOp_ClientJoinedServer:		HandleClientJoinedServer	((RoutingClientJoinedServerOp*)	theOp);	break;
		case RoutingOp_ClientLeftGroup:			HandleClientLeftGroup		((RoutingClientLeftGroupOp*)	theOp);	break;
		case RoutingOp_ClientLeftServer:		HandleClientLeftServer		((RoutingClientLeftServerOp*)	theOp);	break;

		case RoutingOp_GetClientList:			HandleGetClientList			((RoutingGetClientListOp*)		theOp); break;
		case RoutingOp_GetGroupList:			HandleGetGroupList			((RoutingGetGroupListOp*)		theOp); break;
		case RoutingOp_GetMembersOfGroup:		HandleGetMembersOfGroup		((RoutingGetMembersOfGroupOp*)	theOp); break;

		case RoutingOp_GroupCaptainChanged:		HandleGroupCaptainChanged	((RoutingGroupCaptainChangedOp*)	theOp); break;
		case RoutingOp_GroupCreated:			HandleGroupCreated			((RoutingGroupCreatedOp*)			theOp); break;
		case RoutingOp_GroupDeleted:			HandleGroupDeleted			((RoutingGroupDeletedOp*)			theOp); break;
		case RoutingOp_GroupFlagsChanged:		HandleGroupFlagsChanged		((RoutingGroupFlagsChangedOp*)		theOp); break;
		case RoutingOp_GroupMaxPlayersChanged:	HandleGroupMaxPlayersChanged((RoutingGroupMaxPlayersChangedOp*)	theOp); break;
		case RoutingOp_GroupNameChanged:		HandleGroupNameChanged		((RoutingGroupNameChangedOp*)		theOp); break;
		case RoutingOp_GroupObserverCount:		HandleGroupObserverCount	((RoutingGroupObserverCountOp*)		theOp); break;

		case RoutingOp_JoinGroup:				HandleJoinGroup				((RoutingJoinGroupOp*)			theOp); break;
		case RoutingOp_LeaveGroup:				HandleLeaveGroup			((RoutingLeaveGroupOp*)			theOp); break;
		case RoutingOp_MemberFlagsChanged:		HandleMemberFlagsChanged	((RoutingMemberFlagsChangedOp*)	theOp); break;
		case RoutingOp_RegisterClient:			HandleRegisterClient		((RoutingRegisterClientOp*)		theOp); break;
		case RoutingOp_SetClientFlags:			HandleSetClientFlags		((RoutingSetClientFlagsOp*)		theOp); break;

		case RoutingOp_ReadDataObject:			HandleReadDataObject		((RoutingReadDataObjectOp*)			theOp); break;
		case RoutingOp_DataObjectCreated:		HandleDataObjectCreated		((RoutingDataObjectCreatedOp*)		theOp); break;
		case RoutingOp_DataObjectModified:		HandleDataObjectModified	((RoutingDataObjectModifiedOp*)		theOp); break;
		case RoutingOp_DataObjectDeleted:		HandleDataObjectDeleted		((RoutingDataObjectDeletedOp*)		theOp); break;



		// For ignoring
		case RoutingOp_GroupInvitation:			HandleGroupInvitation		((RoutingGroupInvitationOp*)	theOp); break;
		case RoutingOp_GroupJoinAttempt:		HandleGroupJoinAttempt		((RoutingGroupJoinAttemptOp*)	theOp); break;
		case RoutingOp_PeerChat:				HandlePeerChat				((RoutingPeerChatOp*)			theOp); break;

	}	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleRegisterClient(RoutingRegisterClientOp *theOp)
{
	if(!theOp->Succeeded())
		return;

	LoadIgnoreSet();

	mMyClientId = theOp->GetClientId();
	mAsyncFlags = theOp->GetAsyncMessageFlags();

	mGroupMap.clear();
	mClientMap = theOp->GetClientMap();
	if(mClientMap.empty()) // didn't get client list (insert myself)
	{
		mMyClient = GetNewClientInfo();
		mMyClient->mName = theOp->GetClientName();
		mMyClient->mId = theOp->GetClientId();
		mMyClient->mFlags = theOp->GetClientFlags();
		mClientMap[mMyClient->mId] = mMyClient;
	}
	else
		mMyClient = GetClientRef(mMyClientId);

	SyncClientMapWithIgnoreList();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleSetClientFlags(RoutingSetClientFlagsOp *theOp)
{
	if(!theOp->Succeeded())
		return;

	unsigned long aMask = theOp->GetAsyncMessageFlagMask();
	unsigned long aNewFlags = theOp->GetAsyncMessageFlags();

	aNewFlags &= aMask;
	mAsyncFlags &= ~aMask;
	mAsyncFlags |= aNewFlags;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGetClientList(RoutingGetClientListOp *theOp)
{
	const RoutingClientMap &aMap = theOp->GetClientMap();
	RoutingClientMap::const_iterator aNewItr = aMap.begin();
	RoutingClientMap::iterator anOldItr = mClientMap.begin();

	while(anOldItr!=mClientMap.end() && aNewItr!=aMap.end())
	{
		if(anOldItr->first < aNewItr->first)
			EraseClient(anOldItr++);
		else 
		{
			if(aNewItr->first < anOldItr->first)
				anOldItr = mClientMap.insert(anOldItr, *aNewItr);
			else
			{
				anOldItr->second->mName = aNewItr->second->mName;
				anOldItr->second->mFlags = aNewItr->second->mFlags;
			}
			
			anOldItr++;
			aNewItr++;
		}
	}

	while(anOldItr!=mClientMap.end())
		EraseClient(anOldItr++);

	while(aNewItr!=aMap.end())	
	{
		anOldItr = mClientMap.insert(anOldItr, *aNewItr);
		aNewItr++;
	}


	mMyClient = GetClientRef(mMyClientId);
	SyncClientMapWithIgnoreList();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleReadDataObject(RoutingReadDataObjectOp *theOp)
{
	// return if not parsing client CDKey ID's 
	if (theOp->GetDataType() != "_KeyId")
		return;

	RoutingDataObjectList::const_iterator anItr = theOp->GetDataObjects().begin();
	for (; anItr != theOp->GetDataObjects().end(); ++anItr)
		SetClientCDKeyIdFromDataObject((*anItr)->mLinkId, (*anItr)->mData);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleDataObjectCreated(RoutingDataObjectCreatedOp *theOp)
{
	if (theOp->GetDataType() != "_KeyId")
		return;

	SetClientCDKeyIdFromDataObject(theOp->GetLinkId(), theOp->GetData());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleDataObjectModified(RoutingDataObjectModifiedOp *theOp)
{
	if (theOp->GetDataType() != "_KeyId")
		return;

	SetClientCDKeyIdFromDataObject(theOp->GetLinkId(), theOp->GetData());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleDataObjectDeleted(RoutingDataObjectDeletedOp	*theOp)
{
	// client will be removed from the client map, no special processing needed
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SetClientCDKeyIdFromDataObject(unsigned short theClientID, ByteBufferPtr theBuf)
{
	AutoCrit aCrit(mDataCrit);

	// Set the client key hash
	RoutingClientInfoPtr theClientInfo = GetClientRef(theClientID);

	if (theClientInfo.get() == NULL)
		return;

	SmartRoutingClientInfoPtr theSmartClientInfo = (SmartRoutingClientInfo*)theClientInfo.get();

	unsigned long anId = 0;
	ReadBuffer aReadBuf(theBuf->data(), theBuf->length());
	try
	{
		aReadBuf.ReadLong();	// the community
		anId = aReadBuf.ReadLong();
	}
	catch(ReadBufferException&)
	{
	}

	theSmartClientInfo->mKeyId = anId;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleClientJoinedGroup(RoutingClientJoinedGroupOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	RoutingMemberInfoPtr aMember = GetNewMemberInfo();
	aMember->mClientId = theOp->GetClientId();
	aMember->mFlags = theOp->GetMemberFlags();

	SmartRoutingClientInfo *aClient = EnsureInClientMap(theOp->GetClientId());

	aMember->mClientInfo = aClient;
	if(!theOp->GetClientName().empty())
		SetClientName(aClient, theOp->GetClientName());

	if(theOp->HasClientFlags())
		aClient->mFlags = theOp->GetClientFlags();

	pair<RoutingMemberMap::iterator,bool> aRet;
	aRet = aGroup->mMemberMap.insert(RoutingMemberMap::value_type(aMember->mClientId,aMember));
	if(aRet.second)
		aClient->mNumGroups++;

	InitHandlerVariables(aGroup,aRet.first->second);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleClientLeftGroup(RoutingClientLeftGroupOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	int aClientId = theOp->GetClientId();

	RoutingMemberMap::iterator anItr = aGroup->mMemberMap.find(aClientId);
	if(anItr!=aGroup->mMemberMap.end())
	{
		SmartRoutingClientInfo *aClient = (SmartRoutingClientInfo*)anItr->second->mClientInfo.get();
		aClient->mNumGroups--;
		if((mAsyncFlags & RoutingAsyncMessageFlag_ClientLeftServer) && aClient->mId!=mMyClientId && aClient->mNumGroups==0)
			EraseClient(aClient->mId);

		InitHandlerVariables(aGroup,anItr->second);
		EraseMember(aGroup->mMemberMap, anItr);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGetMembersOfGroup(RoutingGetMembersOfGroupOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	aGroup->mObserverCount = theOp->GetObserverCount();
	SyncMemberMapWithClientMap(aGroup->mMemberMap, theOp->GetMemberMap());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SyncMemberMapWithClientMap(RoutingMemberMap &theOldMap, const RoutingMemberMap &theNewMap)
{
	AutoCrit aCrit(mDataCrit);
	RoutingMemberMap::iterator anOldItr = theOldMap.begin();
	RoutingMemberMap::const_iterator aNewItr = theNewMap.begin();

	while(anOldItr!=theOldMap.end() || aNewItr!=theNewMap.end())
	{
		if(aNewItr==theNewMap.end() || (anOldItr!=theOldMap.end() && anOldItr->first<aNewItr->first))
		{
			SmartRoutingClientInfo *anOldClient = (SmartRoutingClientInfo*)anOldItr->second->mClientInfo.get();
			anOldClient->mNumGroups--;
			if((mAsyncFlags & RoutingAsyncMessageFlag_ClientLeftServer) && anOldClient->mId!=mMyClientId && anOldClient->mNumGroups==0)
				EraseClient(anOldClient->mId);
			
			EraseMember(theOldMap, anOldItr++);
		}
		else if(anOldItr==theOldMap.end() || aNewItr->first<anOldItr->first)
		{
			RoutingMemberInfo *aNewMember = aNewItr->second;

			SmartRoutingClientInfo *anOldClient = EnsureInClientMap(aNewItr->first);
			SmartRoutingClientInfo *aNewClient = (SmartRoutingClientInfo*)aNewMember->mClientInfo.get();
			if(aNewClient!=NULL)
			{
				if(!aNewClient->mName.empty())
					SetClientName(anOldClient, aNewClient->mName);

				if(aNewClient->mFlags!=0)
					anOldClient->mFlags = aNewClient->mFlags;
			}

			aNewMember->mClientInfo = anOldClient;
			anOldClient->mNumGroups++;
			
			theOldMap.insert(*aNewItr);
			++aNewItr;
		}
		else
		{
			anOldItr->second->mFlags = aNewItr->second->mFlags;
			RoutingClientInfo *aNewClient = aNewItr->second->mClientInfo;
			if(aNewClient!=NULL)
			{
				RoutingClientInfo *anOldClient = anOldItr->second->mClientInfo;
				if(!aNewClient->mName.empty())
					SetClientName(anOldClient, aNewClient->mName);

				if(aNewClient->mFlags!=0)
					anOldClient->mFlags = aNewClient->mFlags;
			}

			++anOldItr;
			++aNewItr;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SyncMemberMapWithClientMap(RoutingMemberMap &theMap)
{
	AutoCrit aCrit(mDataCrit);
	RoutingMemberMap::iterator anItr = theMap.begin();

	while(anItr!=theMap.end())
	{
		RoutingMemberInfo *aNewMember = anItr->second;

		SmartRoutingClientInfo *anOldClient = EnsureInClientMap(anItr->first);
		SmartRoutingClientInfo *aNewClient = (SmartRoutingClientInfo*)aNewMember->mClientInfo.get();
		if(aNewClient!=NULL)
		{
			if(!aNewClient->mName.empty())
				SetClientName(anOldClient, aNewClient->mName);

			if(aNewClient->mFlags!=0)
				anOldClient->mFlags = aNewClient->mFlags;
		}

		aNewMember->mClientInfo = anOldClient;
		anOldClient->mNumGroups++;		
		
		++anItr;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGetGroupList(RoutingGetGroupListOp *theOp)
{
	const RoutingGroupMap &aNewMap = theOp->GetGroupMap();
	RoutingGroupMap::const_iterator aNewItr = aNewMap.begin();
	RoutingGroupMap::iterator anOldItr = mGroupMap.begin();
	RoutingMemberMap anEmptyMemberMap;

	bool hasMembers = theOp->HasMembers();

	while(anOldItr!=mGroupMap.end() || aNewItr!=aNewMap.end())
	{
		if(aNewItr==aNewMap.end() || (anOldItr!=mGroupMap.end() && anOldItr->first<aNewItr->first))
		{
			SyncMemberMapWithClientMap(anOldItr->second->mMemberMap, anEmptyMemberMap);
			EraseGroup(anOldItr++);
		}
		else if(anOldItr==mGroupMap.end() || aNewItr->first<anOldItr->first)
		{
			if(hasMembers)
				SyncMemberMapWithClientMap(aNewItr->second->mMemberMap);
			
			mGroupMap.insert(*aNewItr);
			++aNewItr;
		}
		else
		{
			RoutingGroupInfo *anOldGroup = anOldItr->second;
			RoutingGroupInfo *aNewGroup = aNewItr->second;

			anOldGroup->mName = aNewGroup->mName;
			anOldGroup->mId = aNewGroup->mId;
			anOldGroup->mCaptainId = aNewGroup->mCaptainId;
			anOldGroup->mMaxPlayers = aNewGroup->mMaxPlayers;
			anOldGroup->mFlags = aNewGroup->mFlags;
			anOldGroup->mAsyncFlags = aNewGroup->mAsyncFlags;

			if(hasMembers)
			{
				anOldGroup->mObserverCount = aNewGroup->mObserverCount;
				SyncMemberMapWithClientMap(anOldGroup->mMemberMap, aNewGroup->mMemberMap);
			}

			++anOldItr;
			++aNewItr;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleJoinGroup(RoutingJoinGroupOp *theOp)
{
	if(!theOp->Succeeded())
		return;

	int aGroupId = theOp->GetGroupId();
	if(theOp->Succeeded())
		mDefaultGroupId = aGroupId;
	
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	if(theOp->GetJoinFlags()&RoutingJoinGroupFlag_GetMembersOfGroup)
		SyncMemberMapWithClientMap(aGroup->mMemberMap, theOp->GetMemberMap());

	InitHandlerVariables(aGroup);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleLeaveGroup(RoutingLeaveGroupOp *)
{
	// erase group from map or erase member list?
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleClientFlagsChanged(RoutingClientFlagsChangedOp *theOp)
{
	int aClientId = theOp->GetClientId();
	RoutingClientInfoPtr aClient = GetClientRef(aClientId);
	if(aClient.get()==NULL)
		return;

	aClient->mFlags = theOp->GetNewClientFlags();
	InitHandlerVariables(aClient);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleClientJoinedServer(RoutingClientJoinedServerOp *theOp)
{
	RoutingClientInfoPtr aClient = GetNewClientInfo();
	aClient->mFlags = theOp->GetClientFlags();
	aClient->mId = theOp->GetClientId();
	SetClientName(aClient, theOp->GetClientName());
	mClientMap[aClient->mId] = aClient;

	InitHandlerVariables(aClient);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleClientLeftServer(RoutingClientLeftServerOp *theOp)
{
	int aClientId = theOp->GetClientId();

	RoutingClientMap::iterator anItr = mClientMap.find(aClientId);
	if(anItr==mClientMap.end())
		return;

	InitHandlerVariables(anItr->second);
	EraseClient(anItr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SyncClientMapWithIgnoreList()
{
	AutoCrit aCrit(mDataCrit);

	RoutingClientMap::iterator aClientItr = mClientMap.begin();
	while(aClientItr!=mClientMap.end())
	{
		((SmartRoutingClientInfo*)aClientItr->second.get())->mIgnored = (mIgnoreSet.find(aClientItr->second->mName)!=mIgnoreSet.end());
		++aClientItr;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupCaptainChanged(RoutingGroupCaptainChangedOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	aGroup->mCaptainId = theOp->GetNewCaptainId();
	InitHandlerVariables(aGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupMaxPlayersChanged(RoutingGroupMaxPlayersChangedOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	aGroup->mMaxPlayers = theOp->GetNewMaxPlayers();
	InitHandlerVariables(aGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupCreated(RoutingGroupCreatedOp *theOp)
{
	RoutingGroupInfoPtr aGroup = GetNewGroupInfo();
	aGroup->mCaptainId = RoutingId_Invalid;
	aGroup->mFlags = theOp->GetGroupFlags();
	aGroup->mAsyncFlags = theOp->GetAsyncFlags();
	aGroup->mId = theOp->GetGroupId();
	aGroup->mMaxPlayers = theOp->GetMaxPlayers();
	aGroup->mName = theOp->GetGroupName();

	mGroupMap[aGroup->mId] = aGroup;
	InitHandlerVariables(aGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupDeleted(RoutingGroupDeletedOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupMap::iterator anItr = mGroupMap.find(aGroupId);
	if(anItr==mGroupMap.end())
		return;
		
	InitHandlerVariables(anItr->second);
	EraseGroup(anItr);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupFlagsChanged(RoutingGroupFlagsChangedOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	aGroup->mFlags = theOp->GetNewGroupFlags();
	InitHandlerVariables(aGroup);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupNameChanged(RoutingGroupNameChangedOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	aGroup->mName = theOp->GetNewGroupName();
	InitHandlerVariables(aGroup);
} 

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupObserverCount(RoutingGroupObserverCountOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	aGroup->mObserverCount = theOp->GetObserverCount();
	InitHandlerVariables(aGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleMemberFlagsChanged(RoutingMemberFlagsChangedOp *theOp)
{
	int aGroupId = theOp->GetGroupId();
	RoutingGroupInfoPtr aGroup = GetGroupRef(aGroupId);
	if(aGroup.get()==NULL)
		return;

	int aClientId = theOp->GetClientId();
	RoutingMemberMap::iterator anItr = aGroup->mMemberMap.find(aClientId);
	if(anItr==aGroup->mMemberMap.end())
		return;

	anItr->second->mFlags = theOp->GetNewMemberFlags();
	InitHandlerVariables(aGroup, anItr->second);

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::CheckIgnore(RoutingOp *theOp, unsigned short theSenderId)
{
	RoutingClientInfoPtr aClient = GetClientRef(theSenderId);
	SmartRoutingClientInfo *aSmartClient = (SmartRoutingClientInfo*)aClient.get();
	if(aSmartClient==NULL)
		return;

	if(aSmartClient->mIgnored)
		theOp->SetCompletion(NULL); // swallow the completion
	else
		InitHandlerVariables(aSmartClient);	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandlePeerChat(RoutingPeerChatOp *theOp)
{
	CheckIgnore(theOp, theOp->GetSenderId());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupInvitation(RoutingGroupInvitationOp *theOp)
{
	CheckIgnore(theOp, theOp->GetCaptainId());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::HandleGroupJoinAttempt(RoutingGroupJoinAttemptOp *theOp)
{
	CheckIgnore(theOp, theOp->GetClientId());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::GetLastTargetMember(MemberInfo &theMember)
{
	AutoCrit aCrit(mDataCrit);
	return CopyMember(theMember,mLastTargetMember);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::GetLastTargetClient(ClientInfo &theClient)
{
	AutoCrit aCrit(mDataCrit);
	return CopyClient(theClient, mLastTargetClient);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SetIgnoreListFile(const std::string &thePath)
{
	AutoCrit aCrit(mDataCrit);
	mIgnoreFilePath = thePath;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::LoadIgnoreSet()
{
	AutoCrit aCrit(mDataCrit);
	if(mIgnoreFilePath.empty())
		return;

	FILE *aFile = fopen(mIgnoreFilePath.c_str(),"rb");
	if(aFile==NULL)
		return;

	wstring aName;
	while(!feof(aFile))
	{
		wchar_t aChar = 0;
		if(fread(&aChar,sizeof(wchar_t),1,aFile)!=1)
			break;

		if(aChar!=0)
			aName+=aChar;
		else
		{
			mIgnoreSet.insert(aName);
			aName.erase();
		}
	}

	fclose(aFile);
	return;

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SaveIgnoreSet()
{
	AutoCrit aCrit(mDataCrit);
	if(mIgnoreFilePath.empty())
		return;

	FILE *aFile = fopen(mIgnoreFilePath.c_str(),"wb");
	if(aFile==NULL)
		return;

	IgnoreSet::iterator anItr = mIgnoreSet.begin();
	while(anItr!=mIgnoreSet.end())
	{
		// Linux basic_string problem (basic_string::c_str() always tries to return a char* )
		fwrite(anItr->data(),sizeof(wchar_t),anItr->length(),aFile);
		wchar_t aZero = 0;
		fwrite(&aZero,sizeof(wchar_t),1,aFile);

		++anItr;
	}

	fclose(aFile);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static bool Compare(const std::wstring &s1, const std::wstring &s2, bool &partialMatch)
{
	partialMatch = false;
	wstring::const_iterator i1 = s1.begin();
	wstring::const_iterator i2 = s2.begin();

	while ((i1 != s1.end()) && (i2 != s2.end()))
	{
		if (towupper(*i1) != towupper(*i2))
			return false;

		i1++;  i2++;
	}

	if(s1.size()<=s2.size())
		partialMatch = true;

	return (s1.size() == s2.size());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void SkipWhitespace(const std::wstring &s1, int &thePos)
{
	while(thePos<s1.length() && iswspace(s1[thePos]))
		thePos++;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static std::wstring GetWord(const std::wstring &s1, int &thePos, bool doToUpper, bool checkQuotes)
{
	SkipWhitespace(s1,thePos);
	if(thePos>=s1.length())
		return L"";

	std::wstring aWord;
	if(checkQuotes && s1[thePos]==L'"')
	{
		int anEndQuotePos = s1.find_first_of(L'"',thePos+1);
		if(anEndQuotePos!=string::npos)
		{
			aWord = s1.substr(thePos+1,anEndQuotePos-thePos-1);
			thePos = anEndQuotePos+1;
			if(doToUpper)
				aWord = WStringToUpperCase(aWord);

			return aWord;
		}
	}

	while(thePos<s1.length())
	{
		wchar_t aChar = s1[thePos];
		if(doToUpper)
			aChar = towupper(aChar);

		if(aChar==L' ')
			return aWord;
		
		aWord+=aChar;
		thePos++;
	}

	return aWord;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static std::wstring GetRestOfLine(const std::wstring &s1, int thePos)
{
	SkipWhitespace(s1,thePos);
	if(thePos>=s1.length())
		return L"";

	std::wstring aWord;
	if(s1[thePos]==L'"')
	{
		int anEndQuotePos = s1.find_first_of(L'"',thePos+1);
		if(anEndQuotePos!=string::npos)
		{
			aWord = s1.substr(thePos+1,anEndQuotePos-thePos-1);
			return aWord;
		}
	}

	int anEndLinePos = s1.find_last_not_of(' ');
	if(anEndLinePos==string::npos || anEndLinePos<=thePos)
		return L"";

	return s1.substr(thePos,anEndLinePos-thePos+1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SmartRoutingConnection::MatchResult SmartRoutingConnection::GetClientByNameRef(const std::wstring &theName, 
																			   RoutingClientInfoPtr &theClientOut)
{
	AutoCrit aCrit(mDataCrit);
	MatchResult aResult = MatchResult_NotFound;

	RoutingClientMap::iterator anItr = mClientMap.begin();
	for(; anItr!=mClientMap.end(); ++anItr)
	{
		RoutingClientInfo *aClient = anItr->second;

		bool partialMatch = false;
		bool exactMatch = false;

		exactMatch = Compare(theName,aClient->mName,partialMatch);
		if(exactMatch)
		{
			if(aResult==MatchResult_Exact)
				return MatchResult_Ambiguous;

			aResult = MatchResult_Exact;
			theClientOut = anItr->second;
		}
		else if(partialMatch)
		{
			if(aResult==MatchResult_NotFound)
			{
				aResult = MatchResult_Partial;
				theClientOut = anItr->second;
			}
			else if(aResult==MatchResult_Partial)
				aResult = MatchResult_Ambiguous;
		}
	}

	return aResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SmartRoutingConnection::MatchResult SmartRoutingConnection::GetMemberByNameRef(RoutingGroupInfoPtr theGroup, 
																 const std::wstring &theName, 
																 RoutingMemberInfoPtr &theMemberOut)
{
	AutoCrit aCrit(mDataCrit);
	MatchResult aResult = MatchResult_NotFound;

	RoutingMemberMap::iterator anItr = theGroup->mMemberMap.begin();
	for(; anItr!=theGroup->mMemberMap.end(); ++anItr)
	{
		RoutingClientInfo *aClient = anItr->second->mClientInfo;
		if(aClient==NULL)
			continue;

		bool partialMatch = false;
		bool exactMatch = false;

		exactMatch = Compare(theName,aClient->mName,partialMatch);
		if(exactMatch)
		{
			if(aResult==MatchResult_Exact)
				return MatchResult_Ambiguous;

			aResult = MatchResult_Exact;
			theMemberOut = anItr->second;
		}
		else if(partialMatch)
		{
			if(aResult==MatchResult_NotFound)
			{
				aResult = MatchResult_Partial;
				theMemberOut = anItr->second;
			}
			else if(aResult==MatchResult_Partial)
				aResult = MatchResult_Ambiguous;
		}
	}

	return aResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
SmartRoutingConnection::MatchResult SmartRoutingConnection::GetMemberByNameRef(unsigned short theGroupId, 
																 const std::wstring &theName, 
																 RoutingMemberInfoPtr &theMemberOut)
{
	AutoCrit aCrit(mDataCrit);

	RoutingGroupInfoPtr aGroup = GetGroupRef(theGroupId);
	if(aGroup.get()==NULL)
		return MatchResult_NotFound;
	else
		return GetMemberByNameRef(aGroup,theName,theMemberOut);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SetIgnore(RoutingClientInfo *theClient, bool on)
{
	AutoCrit aCrit(mDataCrit);

	((SmartRoutingClientInfo*)theClient)->mIgnored = on;
	if(on)
		mIgnoreSet.insert(theClient->mName);
	else
		mIgnoreSet.erase(theClient->mName);

	SaveIgnoreSet();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::HandleCommand(const std::wstring &theInput, int theGroupIdContext)
{
	AutoCrit aCrit(mDataCrit);

	int aPos = 1;
	std::wstring aCommand = GetWord(theInput,aPos,true,false);
	
	CommandStringMap::iterator anItr = mCommandStringMap.find(aCommand);
	if(anItr==mCommandStringMap.end())
	{
		mLastInputCommand = InputCommand_None;
		mLastInputError = InputError_InvalidCommand;
		return false;
	}

	mLastInputCommand = anItr->second;

	// Extract name if needed
	bool needName = false;
	switch(mLastInputCommand)
	{
		case InputCommand_Ignore:
		case InputCommand_Unmute:
		case InputCommand_ServerUnmute:
			mLastMatchName = GetRestOfLine(theInput,aPos);
			needName = true;
			break;

		case InputCommand_Whisper:
		case InputCommand_Mute:
		case InputCommand_ServerMute:
		case InputCommand_Ban:
		case InputCommand_ServerBan:
		case InputCommand_Invite:
		case InputCommand_Uninvite:
			mLastMatchName = GetWord(theInput,aPos,false,true);
			needName = true;
			break;
	}
	
	// Find corresponding client/member
	if(needName)
	{
		MatchResult aResult;
		
		if(mServerWideCommands)
			aResult = GetClientByNameRef(mLastMatchName, mLastTargetClient);
		else
		{
			aResult = GetMemberByNameRef(theGroupIdContext,mLastMatchName,mLastTargetMember);
			if(mLastTargetMember.get()!=NULL)
				mLastTargetClient = mLastTargetMember->mClientInfo;
		}
			
		if(aResult < mMatchTolerance)
		{
			if(aResult==MatchResult_Ambiguous)
				mLastInputError = InputError_ClientAmbiguous;
			else
				mLastInputError = InputError_ClientNotFound;

			return false;
		}
	}

	SkipWhitespace(theInput,aPos);

	switch(mLastInputCommand)
	{
		///////////////////////////////////////////
		///////////////////////////////////////////
		case InputCommand_Whisper:
		{
			if(aPos>=theInput.length())
			{
				mLastInputError = InputError_NoChat;
				return false;
			}

			std::wstring aChat = theInput.substr(aPos);
			SendChat(aChat,RoutingChatFlag_IsWhisper,mLastTargetClient->mId);
			return true;
		}

		///////////////////////////////////////////
		///////////////////////////////////////////
		case InputCommand_Emote:
		{
			if(aPos>=theInput.length())
			{
				mLastInputError = InputError_NoChat;
				return false;
			}

			std::wstring aChat = theInput.substr(aPos-1);
			SendChat(aChat,RoutingChatFlag_IsEmote,theGroupIdContext);
			return true;
		}

		///////////////////////////////////////////
		///////////////////////////////////////////
		case InputCommand_Ignore:
		{
			SetIgnore(mLastTargetClient,!((SmartRoutingClientInfo*)mLastTargetClient.get())->mIgnored);
			return true;
		}

		///////////////////////////////////////////
		///////////////////////////////////////////
		case InputCommand_Away:
		{
			bool away = (mMyClient->mFlags&RoutingClientFlag_IsAway)?true:false;
			SetClientFlags(RoutingClientFlag_IsAway,away?0:RoutingClientFlag_IsAway,0,0);
			return true;
		}

		///////////////////////////////////////////
		///////////////////////////////////////////
		case InputCommand_Invite:
		case InputCommand_Uninvite:
		{
			std::wstring aComment = theInput.substr(aPos);
			bool isInvited = mLastInputCommand==InputCommand_Invite;
			InviteClient(theGroupIdContext, mLastTargetClient->mId, isInvited, aComment);
			return true;
		}

		///////////////////////////////////////////
		///////////////////////////////////////////
		case InputCommand_BecomeModerator:
		{
			BecomeModerator(!(mMyClient->mFlags&RoutingClientFlag_IsModerator));
			return true;
		}

		///////////////////////////////////////////
		///////////////////////////////////////////
		case InputCommand_Mute:
		case InputCommand_ServerMute:
		case InputCommand_Ban:
		case InputCommand_ServerBan:
		{			
			DWORD aSeconds = 3600; // 1 hour
			string aTimeStr = WStringToString(GetWord(theInput,aPos,true,false));
			if(aTimeStr=="INFINITE")
				aSeconds = 0;
			else if(aTimeStr.length()>0)
			{
				char aUnitChar = aTimeStr[aTimeStr.length()-1];
				if(isalpha(aUnitChar))
					aTimeStr = aTimeStr.substr(0,aTimeStr.length()-1);

				aSeconds = atoi(aTimeStr.c_str());
				switch(aUnitChar)
				{
					case 'D': aSeconds*=24;
					case 'H': aSeconds*=60;	
					case 'M': aSeconds*=60; 
					case 'S': break;
					default: 
						mLastInputError = InputError_BadTime;
						return false;
				}

				if(aSeconds==0)
				{
					mLastInputError = InputError_BadTime;
					return false;
				}
			}
				
			switch(mLastInputCommand)
			{
				case InputCommand_Mute:
					MuteClientByClientId(theGroupIdContext,true,aSeconds,L"",mLastTargetClient->mId); 
					break;
			
				case InputCommand_ServerMute:
					MuteClientByClientId(RoutingId_Global,true,aSeconds,L"",mLastTargetClient->mId); 
					break;

				case InputCommand_Ban:
					BanClientByClientId(theGroupIdContext,true,aSeconds,L"",mLastTargetClient->mId); 
					break;

				case InputCommand_ServerBan:
					BanClientByClientId(RoutingId_Global,true,aSeconds,L"",mLastTargetClient->mId);
					break;
			}

			return true;
		}

		///////////////////////////////////////////
		///////////////////////////////////////////
		case InputCommand_Unmute:
		case InputCommand_ServerUnmute:
		{
			int aGroupId = theGroupIdContext;
			if(mLastInputCommand==InputCommand_ServerUnmute)
				aGroupId = RoutingId_Global;

			MuteClientByClientId(aGroupId,false,0,L"",mLastTargetClient->mId);
			return true;
		}
		
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool SmartRoutingConnection::HandleInput(const std::wstring &theInput, int theGroupIdContext)
{
	AutoCrit aCrit(mDataCrit);
	
	mLastInputError = InputError_None;
	mLastTargetMember = NULL;
	mLastTargetClient = NULL;
	mLastMatchName.erase();

	if(mMyClient.get()==NULL)
	{
		mLastInputError = InputError_NotLoggedIn;
		return false;
	}
		
	if(theGroupIdContext==-1)
		theGroupIdContext = mDefaultGroupId;

	if(theInput[0]==L'/')
		return HandleCommand(theInput, theGroupIdContext);
	
	if(mDoColonEmote && theInput[0]==L':')
	{
		mLastInputCommand = InputCommand_Emote;

		wstring aChat = theInput.substr(1);
		int aPos = 0;
		SkipWhitespace(aChat,aPos);
		if(aPos==aChat.length())
		{
			mLastInputError = InputError_NoChat;
			return false;
		}

		SendChat(aChat,RoutingChatFlag_IsEmote,theGroupIdContext);
		return true;
	}

	mLastInputCommand = InputCommand_Broadcast;

	int aPos = 0;
	SkipWhitespace(theInput,aPos);
	if(aPos==theInput.length())
	{
		mLastInputError = InputError_NoChat;
		return false;
	}

	SendChat(theInput,0,theGroupIdContext);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::AddCommandString(const std::wstring &theStr, InputCommand theCommand)
{
	AutoCrit aCrit(mDataCrit);
	mCommandStringMap[WStringToUpperCase(theStr)] = theCommand;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::AddDefaultUserCommandStrings()
{
	AutoCrit aCrit(mDataCrit);
	AddCommandString(L"W",			InputCommand_Whisper);
	AddCommandString(L"MSG",		InputCommand_Whisper);

	AddCommandString(L"ME",			InputCommand_Emote);
	AddCommandString(L"EMOTE",		InputCommand_Emote);	
	
	AddCommandString(L"I",			InputCommand_Ignore);
	AddCommandString(L"IGNORE",		InputCommand_Ignore);

	AddCommandString(L"AWAY",		InputCommand_Away);
	AddCommandString(L"AFK",		InputCommand_Away);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::AddDefaultModeratorCommandStrings()
{
	AutoCrit aCrit(mDataCrit);

	AddCommandString(L"MOD",		InputCommand_BecomeModerator);
	AddCommandString(L"SMUTE",		InputCommand_ServerMute);
	AddCommandString(L"SBAN",		InputCommand_ServerBan);
	AddCommandString(L"MUTE",		InputCommand_Mute);
	AddCommandString(L"BAN",		InputCommand_Ban);
	AddCommandString(L"UNMUTE",		InputCommand_Unmute);
	AddCommandString(L"SUNMUTE",	InputCommand_ServerUnmute);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::ClearCommandStrings()
{
	AutoCrit aCrit(mDataCrit);
	mCommandStringMap.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SetAllowPartialNameMatch(bool allow)
{
	AutoCrit aCrit(mDataCrit);
	if(allow)
		mMatchTolerance = MatchResult_Partial;
	else
		mMatchTolerance = MatchResult_Exact;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SetServerWideCommands(bool serverWideCommandsOn)
{
	AutoCrit aCrit(mDataCrit);
	mServerWideCommands = serverWideCommandsOn;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::SetDoColonEmote(bool doColonEmote)
{
	mDoColonEmote = doColonEmote;	
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SmartRoutingConnection::CloseHook()
{
	mMyClientId = 0;
	mMyClient = NULL;
	RoutingConnection::CloseHook();
}
