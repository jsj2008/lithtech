#ifndef __WON_ROUTINGCONNECTION_H__
#define __WON_ROUTINGCONNECTION_H__
#include "WONShared.h"
#include "WONCommon/SmartPtr.h"
#include "WONServer/ServerConnection.h"
#include "RoutingTypes.h"
#include <list>
 

namespace WONAPI
{

class RoutingOp;
typedef SmartPtr<RoutingOp> RoutingOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingConnection : public ServerConnection
{
protected:
	typedef std::list<RoutingOpPtr> RoutingOpList;
	RoutingOpList mOpList;
	bool mAllDerivedOpsAreUnsolicited;

	friend class RoutingOp;
	void AddOp(RoutingOp *theOp);
	virtual RoutingClientInfoPtr GetNewClientInfo()		{ return new RoutingClientInfo; }
	virtual RoutingMemberInfoPtr GetNewMemberInfo()		{ return new RoutingMemberInfo; }
	virtual RoutingGroupInfoPtr GetNewGroupInfo()		{ return new RoutingGroupInfo; }
	
	virtual void MsgCallback(const ByteBuffer *theMsg);
	RoutingOpPtr GetUnsolicitedOp(unsigned char theMsgType, bool isDerived);
	virtual RoutingOpPtr GetUnsolicitedDerivedOp(unsigned char theMsgType);

	OpCompletionBasePtr mCompletions[RoutingOp_Max];
	OpCompletionBasePtr mDefaultCompletion;

	void CalcCompletion(RoutingOp *theOp);

protected:
	~RoutingConnection();
	virtual void RoutingOpCompleteHook(RoutingOp *) { }
	virtual void CloseHook();
	virtual void KillHook();

public:
	RoutingConnection();
	void SetRoutingCompletion(RoutingOpType theType, OpCompletionBase *theCompletion) { mCompletions[theType] = theCompletion; }
	void SetDefaultRoutingCompletion(OpCompletionBase *theCompletion) { mDefaultCompletion = theCompletion; }

	void SetAllDerivedOpsAreUnsolicited(bool theVal) { mAllDerivedOpsAreUnsolicited = theVal; }


///////////////////////////////////
// Routing Operation Wrappers /////
///////////////////////////////////
	void RegisterClient(const std::wstring &theClientName, const std::wstring &theServerPassword, 
		unsigned long theReconnectId, unsigned long theAsyncMessageFlags, unsigned long theRegisterFlags,
		unsigned long theClientFlags, OpCompletionBase *theCompletion = NULL);
	
	void DisconnectClient(OpCompletionBase *theCompletion = NULL);
	void GetClientList(OpCompletionBase *theCompletion = NULL);
	
	void GetClientCDKeyIdList(OpCompletionBase *theCompletion = NULL, bool bDoSubscribe=true);

	void GetGroupList(unsigned char theFlags, OpCompletionBase *theCompletion = NULL);
	
	void GetMembersOfGroup(unsigned short theGroupId, OpCompletionBase *theCompletion = NULL);
	
	void CreateGroup(unsigned short theGroupId, const std::wstring &theGroupName, 
		const std::wstring &theGroupPassword, unsigned short theMaxPlayers, unsigned long theGroupFlags,
		unsigned char theJoinFlags, unsigned long theAsyncFlags, OpCompletionBase *theCompletion = NULL);

	void JoinGroup(unsigned short theGroupId, const std::wstring &theGroupPassword,
		const std::wstring &theJoinComment, unsigned char theJoinFlags, 
		unsigned char theMemberFlags, OpCompletionBase *theCompletion = NULL);

	void CancelJoinGroup(unsigned short theGroupId, bool leaveIfAlreadyInGroup, OpCompletionBase *theCompletion = NULL);

	void LeaveGroup(unsigned short theGroupId, OpCompletionBase *theCompletion = NULL);
	
	void AcceptClient(unsigned short theGroupId, unsigned short theClientId, bool isAccepted, 
		const std::wstring &theAcceptComment, OpCompletionBase *theCompletion = NULL);

	void InviteClient(unsigned short theGroupId, unsigned short theClientId, bool isInvited, 
		const std::wstring &theInviteComment, OpCompletionBase *theCompletion = NULL);

	void SetGroupName(unsigned short theGroupId, const std::wstring &theName, OpCompletionBase *theCompletion = NULL);
	void SetGroupPassword(unsigned short theGroupId, const std::wstring &thePassword, OpCompletionBase *theCompletion = NULL);
	void SetGroupMaxPlayers(unsigned short theGroupId, unsigned short theMaxPlayers, OpCompletionBase *theCompletion = NULL);
	void SetGroupFlags(unsigned short theGroupId, unsigned long theGroupFlagMask, unsigned long theGroupFlags, unsigned long theAsyncFlagMask, unsigned long theAsyncFlags, OpCompletionBase *theCompletion = NULL);

	void RelinquishCaptaincy(unsigned short theGroupId, unsigned short theNewCaptainId, OpCompletionBase *theCompletion = NULL);

	void SendServerAlert(const std::wstring &theAlertText, OpCompletionBase *theCompletion = NULL);
	void SendServerAlertWithRecipientList(const std::wstring &theAlertText, std::list<unsigned short>& ClientIds, OpCompletionBase *theCompltion = NULL);
	void StartServerShutdown(const std::wstring &theAlertText, unsigned long theSecondsUntilShutdown, OpCompletionBase *theCompletion = NULL);
	void AbortServerShutdown(OpCompletionBase *theCompletion = NULL);

	void SetClientFlags(unsigned long theClientFlagMask, unsigned long theClientFlags, 
		unsigned long theAsyncMessageFlagMask, unsigned long theAsyncMessageFlags,
		OpCompletionBase *theCompletion = NULL);

	void DetectFirewall(unsigned short theListenPort, unsigned long theMaxConnectSeconds, bool doListen, OpCompletionBase *theCompletion = NULL);

	void SendChat(const std::wstring &theText, unsigned short theFlags, unsigned short theClientOrGroupId, OpCompletionBase *theCompletion = NULL);
	void SendChat(const std::wstring &theText, unsigned short theFlags, const RoutingRecipientList &theRecipients, OpCompletionBase *theCompletion = NULL);

	void SendData(const ByteBuffer *theData, unsigned char theFlags, unsigned short theClientOrGroupId, OpCompletionBase *theCompletion = NULL);
	void SendData(const ByteBuffer *theData, unsigned char theFlags, const RoutingRecipientList &theRecipients, OpCompletionBase *theCompletion = NULL);

	void CreateDataObject(unsigned char theFlags, unsigned short theLinkId, const std::string &theDataType, 
		const std::wstring &theDataName, const ByteBuffer *theData, OpCompletionBase *theCompletion = NULL);

	void DeleteDataObject(unsigned short theLinkId, const std::string &theDataType, 
		const std::wstring &theDataName, OpCompletionBase *theCompletion =NULL);

	void ModifyDataObject(unsigned short theLinkId, const std::string &theDataType,
		const std::wstring &theDataName, unsigned short theOffset, bool isInsert,
		const ByteBuffer *theData, OpCompletionBase *theCompletion = NULL);

	void ReadDataObject(unsigned short theLinkId, const std::string &theDataType, 
		unsigned long theFlags, OpCompletionBase *theCompletion = NULL);

	void SubscribeDataObject(unsigned short theLinkId, const std::string &theDataType, 
		unsigned long theFlags, OpCompletionBase *theCompletion = NULL);

	void UnsubscribeDataObject(unsigned short theLinkId, const std::string &theDataType, 
		unsigned long theFlags, OpCompletionBase *theCompletion = NULL);

	void BecomeModerator(bool moderatorOn, OpCompletionBase *theCompletion = NULL);

	void BanClientByClientId(unsigned short theGroupId, bool isBanned, unsigned long theSeconds, 
		const std::wstring &theComment, unsigned short theClientId, OpCompletionBase *theCompletion = NULL);

	void BanClientByWONId(unsigned short theGroupId, bool isBanned, unsigned long theSeconds, 
		const std::wstring &theComment, unsigned long theWONId, OpCompletionBase *theCompletion = NULL);

	void MuteClientByClientId(unsigned short theGroupId, bool isMuted, unsigned long theSeconds, 
		const std::wstring &theComment, unsigned short theClientId, OpCompletionBase *theCompletion = NULL);

	void MuteClientByWONId(unsigned short theGroupId, bool isMuted, unsigned long theSeconds, 
		const std::wstring &theComment, unsigned long theWONId, OpCompletionBase *theCompletion = NULL);

	void GetBadUserList(RoutingBadUserListType theListType, unsigned short theGroupId, unsigned char theFlags, OpCompletionBase *theCompletion = NULL);
	

	void SendComplaint(unsigned short theGroupId, unsigned short theClientId, 
		const std::wstring &theComplaintText, OpCompletionBase *theCompletion = NULL);


};

typedef SmartPtr<RoutingConnection> RoutingConnectionPtr;

}; // namespace WONAPI

#endif
