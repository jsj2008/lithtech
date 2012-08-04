#ifndef __WON_ROUTINGRELINQUISHCAPTAINCYOP_H__
#define __WON_ROUTINGRELINQUISHCAPTAINCYOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingRelinquishCaptaincyOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mNewCaptainId;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingRelinquishCaptaincyOp(RoutingConnection *theConnection, unsigned short theGroupId = 0, unsigned short theNewCaptainId = RoutingId_Invalid) : RoutingOp(theConnection), mGroupId(theGroupId), mNewCaptainId(theNewCaptainId) {}

	void SetGroupId(unsigned short theId) { mGroupId = theId; }	
	void SetNewCaptainId(unsigned short theId) { mNewCaptainId = theId; }

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetNewCaptainId() const { return mNewCaptainId; }

	virtual RoutingOpType GetType() const { return RoutingOp_RelinquishCaptaincy; }
};


typedef SmartPtr<RoutingRelinquishCaptaincyOp> RoutingRelinquishCaptaincyOpPtr;


}; // namespace WONAPI



#endif
