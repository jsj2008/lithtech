#ifndef __WON_ROUTINGSENDSERVERALERTOP_H__
#define __WON_ROUTINGSENDSERVERALERTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSendServerAlertOp : public RoutingOp
{
private:
	std::wstring mAlertText;
	std::list<unsigned short> mRecipientList;
	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSendServerAlertOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}
	RoutingSendServerAlertOp(RoutingConnection *theConnection, const std::wstring &theAlertText) : RoutingOp(theConnection), mAlertText(theAlertText) {}
	
	void SetAlertText(const std::wstring &theAlertText) { mAlertText = theAlertText; }
	const std::wstring& GetAlertText() const { return mAlertText; }
	void AddRecipient(const unsigned short theClientOrGroupId) { mRecipientList.push_back(theClientOrGroupId); }
	void RemoveRecipient(const unsigned short theClientOrGroupId) { mRecipientList.remove(theClientOrGroupId); }
	std::list<unsigned short>& GetRecipientList() { return mRecipientList; }

	virtual RoutingOpType GetType() const { return RoutingOp_SendServerAlert; }
};


typedef SmartPtr<RoutingSendServerAlertOp> RoutingSendServerAlertOpPtr;


}; // namespace WONAPI



#endif
