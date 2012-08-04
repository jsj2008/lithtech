#ifndef __WON_ROUTINGGROUPDELETEDOP_H__
#define __WON_ROUTINGGROUPDELETEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupDeletedOp : public RoutingOp
{
private:
	unsigned short mGroupId;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupDeletedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupDeleted; }
};

typedef SmartPtr<RoutingGroupDeletedOp> RoutingGroupDeletedOpPtr;

}; // namespace WONAPI


#endif
