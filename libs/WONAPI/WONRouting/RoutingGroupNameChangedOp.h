#ifndef __WON_ROUTINGGROUPNAMECHANGEDOP_H__
#define __WON_ROUTINGGROUPNAMECHANGEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingGroupNameChangedOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	std::wstring mNewGroupName;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingGroupNameChangedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetGroupId() const { return mGroupId; }
	const std::wstring& GetNewGroupName() const { return mNewGroupName; }

	virtual RoutingOpType GetType() const { return RoutingOp_GroupNameChanged; }
};

typedef SmartPtr<RoutingGroupNameChangedOp> RoutingGroupNameChangedOpPtr;

}; // namespace WONAPI


#endif
