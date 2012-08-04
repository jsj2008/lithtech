#include "RoutingConnection.h"
#include "RoutingOp.h"
#include "AllRoutingOps.h"
#include "WONCommon/ReadBuffer.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingConnection::RoutingConnection()
{
	mLengthFieldSize = 2;
	mAllDerivedOpsAreUnsolicited = true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingConnection::~RoutingConnection()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::AddOp(RoutingOp *theOp)
{
	AutoCrit aCrit(mDataCrit);

	CalcCompletion(theOp);
	mOpList.push_back(theOp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::CalcCompletion(RoutingOp *theOp)
{
	OpCompletionBasePtr aCompletion = theOp->GetCompletion();
	if(aCompletion.get()==NULL)
		aCompletion = mCompletions[theOp->GetType()];

	if(aCompletion.get()==NULL)
		aCompletion = mDefaultCompletion;

	theOp->SetCompletion(aCompletion);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingOpPtr RoutingConnection::GetUnsolicitedOp(unsigned char theMsgType, bool isDerived)
{
	RoutingOpPtr anOp = NULL;
	if(!isDerived)
	{
		switch(theMsgType)
		{
			case RoutingClientFlagsChanged:				anOp = new RoutingClientFlagsChangedOp(this);		break;
			case RoutingClientJoinedGroup:				anOp = new RoutingClientJoinedGroupOp(this);		break;
			case RoutingClientJoinedServer:				anOp = new RoutingClientJoinedServerOp(this);		break;
			case RoutingClientLeftGroup:				anOp = new RoutingClientLeftGroupOp(this);			break;
			case RoutingClientLeftServer:				anOp = new RoutingClientLeftServerOp(this);			break;
			case RoutingDataObjectCreated:				anOp = new RoutingDataObjectCreatedOp(this);		break;
			case RoutingDataObjectDeleted:				anOp = new RoutingDataObjectDeletedOp(this);		break;
			case RoutingDataObjectModified:				anOp = new RoutingDataObjectModifiedOp(this);		break;
			case RoutingGroupCaptainChanged:			anOp = new RoutingGroupCaptainChangedOp(this);		break;
			case RoutingGroupCreated:					anOp = new RoutingGroupCreatedOp(this);				break;
			case RoutingGroupDeleted:					anOp = new RoutingGroupDeletedOp(this);				break;
			case RoutingGroupFlagsChanged:				anOp = new RoutingGroupFlagsChangedOp(this);		break;
			case RoutingGroupInvitation:				anOp = new RoutingGroupInvitationOp(this);			break;
			case RoutingGroupJoinAttempt:				anOp = new RoutingGroupJoinAttemptOp(this);			break;
			case RoutingGroupMaxPlayersChanged:			anOp = new RoutingGroupMaxPlayersChangedOp(this);	break;
			case RoutingGroupMemberCount:               anOp = new RoutingGroupMemberCountOp(this);         break;
			case RoutingGroupNameChanged:				anOp = new RoutingGroupNameChangedOp(this);			break;
			case RoutingGroupObserverCount:				anOp = new RoutingGroupObserverCountOp(this);		break;
			case RoutingMemberFlagsChanged:				anOp = new RoutingMemberFlagsChangedOp(this);		break;
			case RoutingPeerChat:						anOp = new RoutingPeerChatOp(this);					break;
			case RoutingPeerData:						anOp = new RoutingPeerDataOp(this);					break;
			case RoutingServerAlert:					anOp = new RoutingServerAlertOp(this);				break;
			case RoutingServerShutdownAborted:			anOp = new RoutingServerShutdownAbortedOp(this);	break;
			case RoutingServerShutdownStarted:			anOp = new RoutingServerShutdownStartedOp(this);	break;
			case RoutingYouWereBanned:					anOp = new RoutingYouWereBannedOp(this);			break;
			case RoutingYouWereMuted:					anOp = new RoutingYouWereMutedOp(this);				break;
		}
	}
	else
		anOp = GetUnsolicitedDerivedOp(theMsgType);

	if(anOp.get()!=NULL)
		CalcCompletion(anOp);

	return anOp;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingOpPtr RoutingConnection::GetUnsolicitedDerivedOp(unsigned char)
{
	if(mAllDerivedOpsAreUnsolicited)
		return new RoutingDerivedServerOp(this);
	else
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::MsgCallback(const ByteBuffer *theMsg)
{
	AutoCrit aCrit(mDataCrit);
	
	RoutingOpPtr anOp;
	WONStatus aStatus;
	RoutingOpList::iterator anItr;
	
	try
	{
		ReadBuffer aMsg(theMsg->data(),theMsg->length());
		unsigned char aHeaderType = aMsg.ReadByte();
		unsigned char aServiceType = aMsg.ReadByte();
		unsigned char aMessageType = aMsg.ReadByte();
		int aStartMsgPos = aMsg.pos();

		bool isDerived = aServiceType!=7;
		anOp = GetUnsolicitedOp(aMessageType,isDerived);
		if(anOp.get()!=NULL)
			mOpList.push_front(anOp);

		anItr = mOpList.begin();
		while(anItr!=mOpList.end())
		{
			anOp = *anItr;

			if(isDerived==anOp->IsDerivedServerOp())
			{
				aStatus = anOp->HandleReply(aMessageType,aMsg);
				if(aStatus==WS_RoutingOp_NeedMoreReplies)
					break;
				else if(aStatus!=WS_RoutingOp_DontWantReply)
				{
					mOpList.erase(anItr);
					aCrit.Leave();
					anOp->SetMode(OP_MODE_BLOCK); // set blocking mode so op is not queued
					anOp->ForceFinish(aStatus);
					RoutingOpCompleteHook(anOp);
					anOp->Complete();
					break;
				}
				else
					aMsg.SetPos(aStartMsgPos); // Put the message back in the right place
			}

			++anItr;
		}

//		if(anItr==mOpList.end())
//			// Unhandled reply
	}
	catch(ReadBufferException&)
	{
		if(anOp.get()!=NULL)
		{
			mOpList.erase(anItr);
			aCrit.Leave();
			anOp->SetMode(OP_MODE_BLOCK); // set blocking mode so op is not queued
			anOp->ForceFinish(WS_RoutingOp_ReplyUnpackFailure);
			anOp->Complete();
		}
		// close connection?
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::CloseHook()
{
	RoutingOpList::iterator anItr = mOpList.begin();
	while(anItr!=mOpList.end())
	{
		(*anItr)->Kill();
		++anItr;
	}

	mOpList.clear();
	ServerConnection::CloseHook();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::KillHook()
{
	mDefaultCompletion = NULL;
	for(int i=0; i<RoutingOp_Max; i++)
		mCompletions[i] = NULL;

	ServerConnection::KillHook();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::RegisterClient(const std::wstring &theClientName, const std::wstring &theServerPassword, 
	unsigned long theReconnectId, unsigned long theAsyncMessageFlags, unsigned long theRegisterFlags,
	unsigned long theClientFlags, OpCompletionBase *theCompletion)
{
	RoutingRegisterClientOpPtr anOp = new RoutingRegisterClientOp(this,theClientName,theServerPassword,theReconnectId,theAsyncMessageFlags,theRegisterFlags,theClientFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::DisconnectClient(OpCompletionBase *theCompletion)
{
	RoutingDisconnectClientOpPtr anOp = new RoutingDisconnectClientOp(this);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::GetClientCDKeyIdList(OpCompletionBase *theCompletion, bool bDoSubscribe)
{
	RoutingReadDataObjectOpPtr anOp = new RoutingReadDataObjectOp(this);
	anOp->SetDataType("_KeyId");
	anOp->SetLinkId(RoutingId_Global);
	anOp->SetDoSubscribe(bDoSubscribe);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::GetClientList(OpCompletionBase *theCompletion)
{
	RoutingGetClientListOpPtr anOp = new RoutingGetClientListOp(this);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::GetGroupList(unsigned char theFlags, OpCompletionBase *theCompletion)
{
	RoutingGetGroupListOpPtr anOp = new RoutingGetGroupListOp(this,theFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::GetMembersOfGroup(unsigned short theGroupId, OpCompletionBase *theCompletion)
{
	RoutingGetMembersOfGroupOpPtr anOp = new RoutingGetMembersOfGroupOp(this,theGroupId);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::CreateGroup(unsigned short theGroupId, const std::wstring &theGroupName, 
	const std::wstring &theGroupPassword, unsigned short theMaxPlayers, unsigned long theGroupFlags,
	unsigned char theJoinFlags, unsigned long theAsyncFlags, OpCompletionBase *theCompletion)
{
	RoutingCreateGroupOpPtr anOp = new RoutingCreateGroupOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetGroupName(theGroupName);
	anOp->SetGroupPassword(theGroupPassword);
	anOp->SetMaxPlayers(theMaxPlayers);
	anOp->SetGroupFlags(theGroupFlags);
	anOp->SetJoinFlags(theJoinFlags);
	anOp->SetAsyncFlags(theAsyncFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::JoinGroup(unsigned short theGroupId, const std::wstring &theGroupPassword,
		const std::wstring &theJoinComment, unsigned char theJoinFlags, 
		unsigned char theMemberFlags, OpCompletionBase *theCompletion)
{
	RoutingJoinGroupOpPtr anOp = new RoutingJoinGroupOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetGroupPassword(theGroupPassword);
	anOp->SetJoinComment(theJoinComment);
	anOp->SetJoinFlags(theJoinFlags);
	anOp->SetMemberFlags(theMemberFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::CancelJoinGroup(unsigned short theGroupId, bool leaveIfAlreadyInGroup, OpCompletionBase *theCompletion)
{
	RoutingCancelJoinGroupOpPtr anOp = new RoutingCancelJoinGroupOp(this,theGroupId,leaveIfAlreadyInGroup);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::LeaveGroup(unsigned short theGroupId, OpCompletionBase *theCompletion)
{
	RoutingLeaveGroupOpPtr anOp = new RoutingLeaveGroupOp(this,theGroupId);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::AcceptClient(unsigned short theGroupId, unsigned short theClientId, bool isAccepted, 
									 const std::wstring &theAcceptComment, OpCompletionBase *theCompletion)
{
	RoutingAcceptClientOpPtr anOp = new RoutingAcceptClientOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetClientId(theClientId);
	anOp->SetAccepted(isAccepted);
	anOp->SetAcceptComment(theAcceptComment);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::InviteClient(unsigned short theGroupId, unsigned short theClientId, bool isInvited, 
									 const std::wstring &theInviteComment, OpCompletionBase *theCompletion)
{
	RoutingInviteClientOpPtr anOp = new RoutingInviteClientOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetClientId(theClientId);
	anOp->SetInvited(isInvited);
	anOp->SetInviteComment(theInviteComment);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SetGroupName(unsigned short theGroupId, const std::wstring &theName, OpCompletionBase *theCompletion)
{
	RoutingSetGroupNameOpPtr anOp = new RoutingSetGroupNameOp(this,theGroupId,theName);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SetGroupPassword(unsigned short theGroupId, const std::wstring &thePassword, OpCompletionBase *theCompletion)
{
	RoutingSetGroupNameOpPtr anOp = new RoutingSetGroupNameOp(this,theGroupId,thePassword);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SetGroupMaxPlayers(unsigned short theGroupId, unsigned short theMaxPlayers, OpCompletionBase *theCompletion)
{
	RoutingSetGroupMaxPlayersOpPtr anOp = new RoutingSetGroupMaxPlayersOp(this,theGroupId,theMaxPlayers);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SetGroupFlags(unsigned short theGroupId, unsigned long theGroupFlagMask, unsigned long theGroupFlags, unsigned long theAsyncFlagMask, unsigned long theAsyncFlags, OpCompletionBase *theCompletion)
{
	RoutingSetGroupFlagsOpPtr anOp = new RoutingSetGroupFlagsOp(this,theGroupId,theGroupFlagMask,theGroupFlags,theAsyncFlagMask,theAsyncFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::RelinquishCaptaincy(unsigned short theGroupId, unsigned short theNewCaptainId, OpCompletionBase *theCompletion)
{
	RoutingRelinquishCaptaincyOpPtr anOp = new RoutingRelinquishCaptaincyOp(this,theGroupId,theNewCaptainId);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SendServerAlert(const std::wstring &theAlertText, OpCompletionBase *theCompletion)
{
	RoutingSendServerAlertOpPtr anOp = new RoutingSendServerAlertOp(this,theAlertText);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SendServerAlertWithRecipientList(const std::wstring &theAlertText, std::list<unsigned short>& recipientList, OpCompletionBase *theCompletion)
{
	if (recipientList.empty())
		return;

	RoutingSendServerAlertOpPtr anOp = new RoutingSendServerAlertOp(this,theAlertText);

	std::list<unsigned short>::const_iterator anItr = recipientList.begin();
	for (; anItr != recipientList.end(); ++anItr)
		anOp->AddRecipient(*anItr);

	anOp->SetCompletion(theCompletion);
	anOp->Run();
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::StartServerShutdown(const std::wstring &theAlertText, unsigned long theSecondsUntilShutdown, OpCompletionBase *theCompletion)
{
	RoutingStartServerShutdownOpPtr anOp = new RoutingStartServerShutdownOp(this, theAlertText, theSecondsUntilShutdown);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::AbortServerShutdown(OpCompletionBase *theCompletion)
{
	RoutingAbortServerShutdownOpPtr anOp = new RoutingAbortServerShutdownOp(this);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SetClientFlags(unsigned long theClientFlagMask, unsigned long theClientFlags, 
		unsigned long theAsyncMessageFlagMask, unsigned long theAsyncMessageFlags,
		OpCompletionBase *theCompletion)
{
	RoutingSetClientFlagsOpPtr anOp = new RoutingSetClientFlagsOp(this,theClientFlagMask, theClientFlags, 
		theAsyncMessageFlagMask, theAsyncMessageFlags);

	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::DetectFirewall(unsigned short theListenPort, unsigned long theMaxConnectSeconds, bool doListen, OpCompletionBase *theCompletion)
{
	RoutingDetectFirewallOpPtr anOp = new RoutingDetectFirewallOp(this);
	anOp->SetListenPort(theListenPort);
	anOp->SetMaxConnectWaitTime(theMaxConnectSeconds);
	anOp->SetDoListen(doListen);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SendChat(const std::wstring &theText, unsigned short theFlags, unsigned short theClientOrGroupId, OpCompletionBase *theCompletion)
{
	RoutingRecipientList aList;
	aList.push_back(theClientOrGroupId);
	SendChat(theText,theFlags,aList,theCompletion);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SendChat(const std::wstring &theText, unsigned short theFlags, const RoutingRecipientList &theRecipients, OpCompletionBase *theCompletion)
{
	RoutingSendChatOpPtr anOp = new RoutingSendChatOp(this);
	anOp->SetText(theText);
	anOp->SetFlags(theFlags);
	anOp->SetRecipients(theRecipients);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SendData(const ByteBuffer *theData, unsigned char theFlags, unsigned short theClientOrGroupId, OpCompletionBase *theCompletion)
{
	RoutingRecipientList aList;
	aList.push_back(theClientOrGroupId);
	SendData(theData,theFlags,aList,theCompletion);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SendData(const ByteBuffer *theData, unsigned char theFlags, const RoutingRecipientList &theRecipients, OpCompletionBase *theCompletion)
{
	RoutingSendDataOpPtr anOp = new RoutingSendDataOp(this);
	anOp->SetData(theData);
	anOp->SetFlags(theFlags);
	anOp->SetRecipients(theRecipients);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::CreateDataObject(unsigned char theFlags, unsigned short theLinkId, const std::string &theDataType, 
	const std::wstring &theDataName, const ByteBuffer *theData, OpCompletionBase *theCompletion)
{
	RoutingCreateDataObjectOpPtr anOp = new RoutingCreateDataObjectOp(this);
	anOp->SetFlags(theFlags);
	anOp->SetLinkId(theLinkId);
	anOp->SetDataType(theDataType);
	anOp->SetDataName(theDataName);
	anOp->SetData(theData);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::DeleteDataObject(unsigned short theLinkId, const std::string &theDataType, 
										 const std::wstring &theDataName, OpCompletionBase *theCompletion)
{
	RoutingDeleteDataObjectOpPtr anOp = new RoutingDeleteDataObjectOp(this);
	anOp->SetLinkId(theLinkId);
	anOp->SetDataType(theDataType);
	anOp->SetDataName(theDataName);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::ModifyDataObject(unsigned short theLinkId, const std::string &theDataType,
	const std::wstring &theDataName, unsigned short theOffset, bool isInsert,
	const ByteBuffer *theData, OpCompletionBase *theCompletion)
{
	RoutingModifyDataObjectOpPtr anOp = new RoutingModifyDataObjectOp(this);
	anOp->SetLinkId(theLinkId);
	anOp->SetDataType(theDataType);
	anOp->SetDataName(theDataName);
	anOp->SetOffset(theOffset);
	anOp->SetIsInsert(isInsert);
	anOp->SetData(theData);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::ReadDataObject(unsigned short theLinkId, const std::string &theDataType, 
	unsigned long theFlags, OpCompletionBase *theCompletion)
{
	RoutingReadDataObjectOpPtr anOp = new RoutingReadDataObjectOp(this);
	anOp->SetDoSubscribe(false);
	anOp->SetLinkId(theLinkId);
	anOp->SetDataType(theDataType);
	anOp->SetFlags(theFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SubscribeDataObject(unsigned short theLinkId, const std::string &theDataType, 
	unsigned long theFlags, OpCompletionBase *theCompletion)
{
	RoutingReadDataObjectOpPtr anOp = new RoutingReadDataObjectOp(this);
	anOp->SetDoSubscribe(true);
	anOp->SetLinkId(theLinkId);
	anOp->SetDataType(theDataType);
	anOp->SetFlags(theFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::UnsubscribeDataObject(unsigned short theLinkId, const std::string &theDataType, 
	unsigned long theFlags, OpCompletionBase *theCompletion)
{
	RoutingUnsubscribeDataObjectOpPtr anOp = new RoutingUnsubscribeDataObjectOp(this);
	anOp->SetLinkId(theLinkId);
	anOp->SetDataType(theDataType);
	anOp->SetFlags(theFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::BecomeModerator(bool moderatorOn, OpCompletionBase *theCompletion)
{
	RoutingBecomeModeratorOpPtr anOp = new RoutingBecomeModeratorOp(this,moderatorOn);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::BanClientByClientId(unsigned short theGroupId, bool isBanned, unsigned long theSeconds, 
	const std::wstring &theComment, unsigned short theClientId, OpCompletionBase *theCompletion)
{
	RoutingBanClientOpPtr anOp = new RoutingBanClientOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetIsBanned(isBanned);
	anOp->SetBanTime(theSeconds);
	anOp->SetBanComment(theComment);
	anOp->SetClientId(theClientId);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::BanClientByWONId(unsigned short theGroupId, bool isBanned, unsigned long theSeconds, 
	const std::wstring &theComment, unsigned long theWONId, OpCompletionBase *theCompletion)
{
	RoutingBanClientOpPtr anOp = new RoutingBanClientOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetIsBanned(isBanned);
	anOp->SetBanTime(theSeconds);
	anOp->SetBanComment(theComment);
	anOp->SetWONId(theWONId);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::MuteClientByClientId(unsigned short theGroupId, bool isMuted, unsigned long theSeconds, 
	const std::wstring &theComment, unsigned short theClientId, OpCompletionBase *theCompletion)
{
	RoutingMuteClientOpPtr anOp = new RoutingMuteClientOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetIsMuted(isMuted);
	anOp->SetMuteTime(theSeconds);
	anOp->SetMuteComment(theComment);
	anOp->SetClientId(theClientId);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::MuteClientByWONId(unsigned short theGroupId, bool isMuted, unsigned long theSeconds, 
	const std::wstring &theComment, unsigned long theWONId, OpCompletionBase *theCompletion)
{
	RoutingMuteClientOpPtr anOp = new RoutingMuteClientOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetIsMuted(isMuted);
	anOp->SetMuteTime(theSeconds);
	anOp->SetMuteComment(theComment);
	anOp->SetWONId(theWONId);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::GetBadUserList(RoutingBadUserListType theListType, unsigned short theGroupId, unsigned char theFlags,
									   OpCompletionBase *theCompletion)
{
	RoutingGetBadUserListOpPtr anOp = new RoutingGetBadUserListOp(this,theListType,theGroupId,theFlags);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingConnection::SendComplaint(unsigned short theGroupId, unsigned short theClientId, 
	const std::wstring &theComplaintText, OpCompletionBase *theCompletion)
{
	RoutingSendComplaintOpPtr anOp = new RoutingSendComplaintOp(this);
	anOp->SetGroupId(theGroupId);
	anOp->SetClientId(theClientId);
	anOp->SetText(theComplaintText);
	anOp->SetCompletion(theCompletion);
	anOp->Run();
}
