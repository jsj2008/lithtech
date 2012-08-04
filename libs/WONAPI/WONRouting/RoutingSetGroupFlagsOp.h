#ifndef __WON_ROUTINGSETGROUPFLAGSOP_H__
#define __WON_ROUTINGSETGROUPFLAGSOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSetGroupFlagsOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned long mGroupFlagMask;
	unsigned long mGroupFlags;
	unsigned long mAsyncFlagMask;
	unsigned long mAsyncFlags;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSetGroupFlagsOp(RoutingConnection *theConnection, unsigned short theGroupId =0, unsigned long theGroupFlagMask =0, unsigned long theGroupFlags =0, unsigned long theAsyncFlagMask = 0, unsigned long theAsyncFlags = 0) : 
		RoutingOp(theConnection), mGroupId(theGroupId), mGroupFlagMask(theGroupFlagMask), mGroupFlags(theGroupFlags), mAsyncFlagMask(theAsyncFlagMask), mAsyncFlags(theAsyncFlags) {}

	void SetGroupId(unsigned short theId) { mGroupId = theId; }	
	void SetGroupFlagMask(unsigned long theGroupFlagMask) { mGroupFlagMask = theGroupFlagMask; }
	void SetGroupFlags(unsigned long theGroupFlags) { mGroupFlags = theGroupFlags; }
	void SetAsyncFlagMask(unsigned long theAsyncFlagMask) { mAsyncFlagMask = theAsyncFlagMask; }
	void SetAsyncFlags(unsigned long theAsyncFlags) { mAsyncFlags = theAsyncFlags; }

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned long GetGroupFlagMask() const { return mGroupFlagMask; }
	unsigned long GetGroupFlags() const { return mGroupFlags; }
	unsigned long GetAsyncFlagMask() const { return mAsyncFlagMask; }
	unsigned long GetAsyncFlags() const { return mAsyncFlags; }

	virtual RoutingOpType GetType() const { return RoutingOp_SetGroupFlags; }
};


typedef SmartPtr<RoutingSetGroupFlagsOp> RoutingSetGroupFlagsOpPtr;


}; // namespace WONAPI



#endif
