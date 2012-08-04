#ifndef __WON_ROUTINGCANCELJOINGROUPOP_H__
#define __WON_ROUTINGCANCELJOINGROUPOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingCancelJoinGroupOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	bool mLeaveIfAlreadyInGroup;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingCancelJoinGroupOp(RoutingConnection *theConnection, unsigned short theGroupId = 0, bool leaveIfAlreadyInGroup =true) : RoutingOp(theConnection), mGroupId(theGroupId), mLeaveIfAlreadyInGroup(leaveIfAlreadyInGroup) { }

	void SetGroupId(unsigned short theId) { mGroupId = theId; }
	void SetLeaveIfAlreadyInGroup(bool leaveIfAlreadyInGroup) { mLeaveIfAlreadyInGroup = leaveIfAlreadyInGroup; }

	unsigned short GetGroupId() const { return mGroupId; }
	bool GetLeaveIfAlreadyInGroup() const { return mLeaveIfAlreadyInGroup; }

	virtual RoutingOpType GetType() const { return RoutingOp_CancelJoinGroup; }
};


typedef SmartPtr<RoutingCancelJoinGroupOp> RoutingCancelJoinGroupOpPtr;


}; // namespace WONAPI



#endif
