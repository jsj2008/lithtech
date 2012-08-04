#ifndef __WON_ROUTINGJOINGROUPOP_H__
#define __WON_ROUTINGJOINGROUPOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingJoinGroupOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	std::wstring mGroupPassword;
	std::wstring mJoinComment;
	unsigned char mJoinFlags;
	unsigned char mMemberFlags;

	RoutingMemberMap mMemberMap;


	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingJoinGroupOp(RoutingConnection *theConnection);

	void SetGroupId(unsigned short theId) { mGroupId = theId; }
	void SetGroupPassword(const std::wstring &thePassword) { mGroupPassword = thePassword; }
	void SetJoinComment(const std::wstring &theComment) { mJoinComment = theComment; }
	void SetJoinFlags(unsigned char theFlags) { mJoinFlags = theFlags; }
	void SetMemberFlags(unsigned char theFlags) { mMemberFlags = theFlags; }
	
	unsigned short GetGroupId() const { return mGroupId; }
	const std::wstring& GetGroupPassword() const { return mGroupPassword; }
	const std::wstring& GetJoinComment() const { return mJoinComment; }
	unsigned char GetJoinFlags() const { return mJoinFlags; }
	unsigned char GetMemberFlags() const { return mMemberFlags; }

	const RoutingMemberMap& GetMemberMap() { return mMemberMap; }

	virtual RoutingOpType GetType() const { return RoutingOp_JoinGroup; }
};


typedef SmartPtr<RoutingJoinGroupOp> RoutingJoinGroupOpPtr;


}; // namespace WONAPI



#endif
