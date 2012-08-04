#ifndef __WON_ROUTINGSETGROUPPASSWORDOP_H__
#define __WON_ROUTINGSETGROUPPASSWORDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSetGroupPasswordOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	std::wstring mGroupPassword;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSetGroupPasswordOp(RoutingConnection *theConnection) : RoutingOp(theConnection), mGroupId(0) {}
	RoutingSetGroupPasswordOp(RoutingConnection *theConnection, unsigned short theGroupId, const std::wstring &thePassword) : RoutingOp(theConnection), mGroupId(theGroupId), mGroupPassword(thePassword) {}

	void SetGroupId(unsigned short theId) { mGroupId = theId; }	
	void SetGroupPassword(const std::wstring &theGroupPassword) { mGroupPassword = theGroupPassword; }

	unsigned short GetGroupId() const { return mGroupId; }
	const std::wstring& GetGroupPassword() const { return mGroupPassword; }

	virtual RoutingOpType GetType() const { return RoutingOp_SetGroupPassword; }
};


typedef SmartPtr<RoutingSetGroupPasswordOp> RoutingSetGroupPasswordOpPtr;


}; // namespace WONAPI



#endif
