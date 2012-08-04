#ifndef __WON_ROUTINGPEERDATAOP_H__
#define __WON_ROUTINGPEERDATAOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingPeerDataOp : public RoutingOp
{
private:
	unsigned short mSenderId;
	ByteBufferPtr mData;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingPeerDataOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetSenderId() const { return mSenderId; }
	ByteBufferPtr GetData() const { return mData; }

	virtual RoutingOpType GetType() const { return RoutingOp_PeerData; }
};

typedef SmartPtr<RoutingPeerDataOp> RoutingPeerDataOpPtr;

}; // namespace WONAPI


#endif
