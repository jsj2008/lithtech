#ifndef __WON_ROUTINGDISCONNECTCLIENTOP_H__
#define __WON_ROUTINGDISCONNECTCLIENTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingDisconnectClientOp : public RoutingOp
{
private:
	virtual void SendRequest();

public:
	RoutingDisconnectClientOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}
	virtual RoutingOpType GetType() const { return RoutingOp_DisconnectClient; }
};

typedef SmartPtr<RoutingDisconnectClientOp> RoutingDisconnectClientOpPtr;


}; // namespace WONAPI



#endif
