#ifndef __WON_ROUTINGCREATEGROUPOP_H__
#define __WON_ROUTINGCREATEGROUPOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingCreateGroupOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mParentGroupId;
	std::wstring mGroupName;
	std::wstring mGroupPassword;
	unsigned short mMaxPlayers;
	unsigned char mJoinFlags;
	unsigned long mGroupFlags;
	unsigned long mAsyncFlags;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingCreateGroupOp(RoutingConnection *theConnection);

	void SetGroupId(unsigned short theId) { mGroupId = theId; }
	void SetParentGroupId(unsigned short theId) { mParentGroupId = theId; }
	void SetGroupName(const std::wstring &theName) { mGroupName = theName; }
	void SetGroupPassword(const std::wstring &thePassword) { mGroupPassword = thePassword; }
	void SetMaxPlayers(unsigned short theMaxPlayers) { mMaxPlayers = theMaxPlayers; }
	void SetJoinFlags(unsigned char theFlags) { mJoinFlags = theFlags; }
	void SetGroupFlags(unsigned long theFlags) { mGroupFlags = theFlags; }
	void SetAsyncFlags(unsigned long theFlags) { mAsyncFlags = theFlags; }
	
	unsigned short GetGroupId() const { return mGroupId; }
	const std::wstring& GetGroupName() const { return mGroupName; }
	const std::wstring& GetGroupPassword() const { return mGroupPassword; }
	unsigned short GetMaxPlayers() const { return mMaxPlayers; }
	unsigned char GetJoinFlags() const { return mJoinFlags; }
	unsigned long GetGroupFlags() const { return mGroupFlags; }
	unsigned long GetAsyncFlags() const { return mAsyncFlags; }

	virtual RoutingOpType GetType() const { return RoutingOp_CreateGroup; }
};


typedef SmartPtr<RoutingCreateGroupOp> RoutingCreateGroupOpPtr;


}; // namespace WONAPI



#endif
