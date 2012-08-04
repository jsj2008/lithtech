#ifndef __WON_ROUTINGSERVERSHUTDOWNSTARTEDOP_H__
#define __WON_ROUTINGSERVERSHUTDOWNSTARTEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingServerShutdownStartedOp : public RoutingOp
{
private:
	std::wstring mAlertText;
	unsigned long mSecondsUntilShutdown;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingServerShutdownStartedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	const std::wstring& GetAlertText() const { return mAlertText; }
	unsigned long GetSecondsUntilShutdown() const { return mSecondsUntilShutdown; }

	virtual RoutingOpType GetType() const { return RoutingOp_ServerShutdownStarted; }
};

typedef SmartPtr<RoutingServerShutdownStartedOp> RoutingServerShutdownStartedOpPtr;

}; // namespace WONAPI


#endif
