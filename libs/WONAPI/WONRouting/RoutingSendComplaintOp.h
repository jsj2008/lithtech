#ifndef __WON_ROUTINGSENDCOMPLAINTOP_H__
#define __WON_ROUTINGSENDCOMPLAINTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSendComplaintOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mClientId;
	std::wstring mText;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSendComplaintOp(RoutingConnection *theConnection, unsigned short theGroupId = 0, unsigned short theClientId = 0) : RoutingOp(theConnection), mGroupId(theGroupId), mClientId(theClientId) {}
	void SetGroupId(unsigned short theGroupId) { mGroupId = theGroupId; }
	void SetClientId(unsigned short theClientId) { mClientId = theClientId; }
	void SetText(const std::wstring& theText) { mText = theText; }

	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetClientId() const { return mClientId; }
	const std::wstring& GetText() const { return mText; }

	virtual RoutingOpType GetType() const { return RoutingOp_SendComplaint; }
};

typedef SmartPtr<RoutingSendComplaintOp> RoutingSendComplaintOpPtr;

}; // namespace WONAPI


#endif
