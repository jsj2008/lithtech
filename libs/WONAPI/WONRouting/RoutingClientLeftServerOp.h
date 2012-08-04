#ifndef __WON_ROUTINGCLIENTLEFTSERVEROP_H__
#define __WON_ROUTINGCLIENTLEFTSERVEROP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingClientLeftServerOp : public RoutingOp
{
private:
	unsigned short mClientId;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingClientLeftServerOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetClientId() const { return mClientId; }

	virtual RoutingOpType GetType() const { return RoutingOp_ClientLeftServer; }
};

typedef SmartPtr<RoutingClientLeftServerOp> RoutingClientLeftServerOpPtr;

}; // namespace WONAPI


#endif
