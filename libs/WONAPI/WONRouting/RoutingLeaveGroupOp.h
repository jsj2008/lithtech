#ifndef __WON_ROUTINGLEAVEGROUPOP_H__
#define __WON_ROUTINGLEAVEGROUPOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingLeaveGroupOp : public RoutingOp
{
private:
	unsigned short mGroupId;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingLeaveGroupOp(RoutingConnection *theConnection, unsigned short theGroupId = 0) : RoutingOp(theConnection), mGroupId(theGroupId) {}

	void SetGroupId(unsigned short theId) { mGroupId = theId; }	
	unsigned short GetGroupId() const { return mGroupId; }

	virtual RoutingOpType GetType() const { return RoutingOp_LeaveGroup; }
};


typedef SmartPtr<RoutingLeaveGroupOp> RoutingLeaveGroupOpPtr;


}; // namespace WONAPI



#endif
