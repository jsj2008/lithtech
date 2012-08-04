#ifndef __WON_ROUTINGGETMEMBERSOFGROUPOP_H__
#define __WON_ROUTINGGETMEMBERSOFGROUPOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGetMembersOfGroupOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mObserverCount;
	bool mHasClientNames;
	bool mHasClientFlags;
	RoutingMemberMap mMemberMap;
	friend class RoutingJoinGroupOp;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGetMembersOfGroupOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}
	RoutingGetMembersOfGroupOp(RoutingConnection *theConnection, unsigned short theGroupId) : RoutingOp(theConnection), mGroupId(theGroupId) {}

	void SetGroupId(unsigned short theId) { mGroupId = theId; }
	
	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetObserverCount() const { return mObserverCount; }
	bool HasClientNames() const { return mHasClientNames; }
	bool HasClientFlags() const { return mHasClientFlags; }
	const RoutingMemberMap& GetMemberMap() const { return mMemberMap; }

	virtual RoutingOpType GetType() const { return RoutingOp_GetMembersOfGroup; }
};

typedef SmartPtr<RoutingGetMembersOfGroupOp> RoutingGetMembersOfGroupOpPtr;


}; // namespace WONAPI



#endif
