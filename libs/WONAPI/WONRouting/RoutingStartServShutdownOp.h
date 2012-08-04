#ifndef __WON_ROUTINGSTARTSERVERSHUTDOWNOP_H__
#define __WON_ROUTINGSTARTSERVERSHUTDOWNOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingStartServerShutdownOp : public RoutingOp
{
private:
	std::wstring mAlertText;
	unsigned long mSecondsUntilShutdown;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingStartServerShutdownOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}
	RoutingStartServerShutdownOp(RoutingConnection *theConnection, const std::wstring &theAlertText, unsigned long theSecondsUntilShutdown) : RoutingOp(theConnection), mAlertText(theAlertText), mSecondsUntilShutdown(theSecondsUntilShutdown) {}

	void SetAlertText(const std::wstring &theAlertText) { mAlertText = theAlertText; }
	void SetSecondsUntilShutdown(unsigned long theSeconds) { mSecondsUntilShutdown = theSeconds; }

	const std::wstring& GetAlertText() const { return mAlertText; }
	unsigned long GetSecondsUntilShutdown() const { return mSecondsUntilShutdown; }

	virtual RoutingOpType GetType() const { return RoutingOp_StartServerShutdown; }
};


typedef SmartPtr<RoutingStartServerShutdownOp> RoutingStartServerShutdownOpPtr;


}; // namespace WONAPI



#endif
