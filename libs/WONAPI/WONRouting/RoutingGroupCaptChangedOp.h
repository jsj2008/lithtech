#ifndef __WON_ROUTINGGROUPCAPTAINCHANGEDOP_H__
#define __WON_ROUTINGGROUPCAPTAINCHANGEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupCaptainChangedOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mNewCaptainId;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupCaptainChangedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetNewCaptainId() const { return mNewCaptainId; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupCaptainChanged; }
};

typedef SmartPtr<RoutingGroupCaptainChangedOp> RoutingGroupCaptainChangedOpPtr;

}; // namespace WONAPI


#endif
