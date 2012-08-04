#ifndef __WON_ROUTINGSERVERSHUTDOWNABORTEDOP_H__
#define __WON_ROUTINGSERVERSHUTDOWNABORTEDOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingServerShutdownAbortedOp : public RoutingOp
{
private:
	std::wstring mAlertText;

	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingServerShutdownAbortedOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	const std::wstring& GetAlertText() const { return mAlertText; }

	virtual RoutingOpType GetType() const { return RoutingOp_ServerShutdownAborted; }
};

typedef SmartPtr<RoutingServerShutdownAbortedOp> RoutingServerShutdownAbortedOpPtr;

}; // namespace WONAPI


#endif
