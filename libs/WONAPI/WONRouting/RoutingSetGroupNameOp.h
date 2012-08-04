#ifndef __WON_ROUTINGSETGROUPNAMEOP_H__
#define __WON_ROUTINGSETGROUPNAMEOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSetGroupNameOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	std::wstring mGroupName;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSetGroupNameOp(RoutingConnection *theConnection) : RoutingOp(theConnection), mGroupId(0) {}
	RoutingSetGroupNameOp(RoutingConnection *theConnection, unsigned short theGroupId, const std::wstring &theName) : RoutingOp(theConnection), mGroupId(theGroupId), mGroupName(theName) {}

	void SetGroupId(unsigned short theId) { mGroupId = theId; }	
	void SetGroupName(const std::wstring &theGroupName) { mGroupName = theGroupName; }

	unsigned short GetGroupId() const { return mGroupId; }
	const std::wstring& GetGroupName() const { return mGroupName; }

	virtual RoutingOpType GetType() const { return RoutingOp_SetGroupName; }
};


typedef SmartPtr<RoutingSetGroupNameOp> RoutingSetGroupNameOpPtr;


}; // namespace WONAPI



#endif
