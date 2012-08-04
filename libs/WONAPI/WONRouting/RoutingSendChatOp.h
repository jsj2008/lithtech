#ifndef __WON_ROUTINGSENDCHATOP_H__
#define __WON_ROUTINGSENDCHATOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSendChatOp : public RoutingOp
{
private:
	unsigned short mFlags;
	std::wstring mText;
	RoutingRecipientList mRecipients;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSendChatOp(RoutingConnection *theConnection) : RoutingOp(theConnection), mFlags(0) {}
	void SetFlags(unsigned short theFlags) { mFlags = theFlags; }
	void SetText(const std::wstring& theText) { mText = theText; }
	void SetRecipients(const RoutingRecipientList &theRecipients) { mRecipients = theRecipients; }
	void ClearRecipients() { mRecipients.clear(); }
	void AddRecipient(unsigned short theClientOrGroupId) { mRecipients.push_back(theClientOrGroupId); }

	unsigned short GetFlags() const { return mFlags; }
	const std::wstring& GetText() const { return mText; }
	const RoutingRecipientList& GetRecipients() const { return mRecipients; }

	virtual RoutingOpType GetType() const { return RoutingOp_SendChat; }
};

typedef SmartPtr<RoutingSendChatOp> RoutingSendChatOpPtr;

}; // namespace WONAPI


#endif
