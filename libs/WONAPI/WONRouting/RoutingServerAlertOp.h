#ifndef __WON_ROUTINGSERVERALERTOP_H__
#define __WON_ROUTINGSERVERALERTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingServerAlertOp : public RoutingOp
{
private:
	std::wstring mAlertText;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingServerAlertOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	const std::wstring& GetAlertText() const { return mAlertText; }

	virtual RoutingOpType GetType() const { return RoutingOp_ServerAlert; }
};

typedef SmartPtr<RoutingServerAlertOp> RoutingServerAlertOpPtr;

}; // namespace WONAPI


#endif
