#ifndef __WON_ROUTINGGROUPOBSERVERCOUNTOP_H__
#define __WON_ROUTINGGROUPOBSERVERCOUNTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupObserverCountOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mObserverCount;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupObserverCountOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetObserverCount() const { return mObserverCount; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupObserverCount; }
};

typedef SmartPtr<RoutingGroupObserverCountOp> RoutingGroupObserverCountOpPtr;

}; // namespace WONAPI


#endif
