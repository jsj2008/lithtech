#ifndef __WON_ROUTINGDERIVEDSERVEROP_H__
#define __WON_ROUTINGDERIVEDSERVEROP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingDerivedServerOp : public RoutingOp
{
protected:
	unsigned char mMsgType;
	ByteBufferPtr mRecvMsg;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingDerivedServerOp(RoutingConnection *theConnection) : RoutingOp(theConnection), mMsgType(0) { mIsDerivedServerOp = true; } 	

	virtual RoutingOpType GetType() const { return RoutingOp_DerivedServerOp; }

	unsigned char GetMsgType() { return mMsgType; }
	const ByteBuffer* GetRecvMsg() const { return mRecvMsg; }
};

typedef SmartPtr<RoutingDerivedServerOp> RoutingDerivedServerOpPtr;

}; // namespace WONAPI


#endif
