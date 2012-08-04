#ifndef __WON_ROUTINGPEERCHATOP_H__
#define __WON_ROUTINGPEERCHATOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingPeerChatOp : public RoutingOp
{
private:
	unsigned short mSenderId;
	unsigned short mFlags;
	std::wstring mText;
	RoutingRecipientList mRecipients;


	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingPeerChatOp(RoutingConnection *theConnection) : RoutingOp(theConnection) {}

	unsigned short GetSenderId() const { return mSenderId; }
	unsigned short GetFlags() const { return mFlags; }
	const std::wstring& GetText() const { return mText; }
	const RoutingRecipientList& GetRecipients() const { return mRecipients; }

	bool IsEmote() const { return (mFlags&RoutingChatFlag_IsEmote)?true:false; }
	bool IsWhisper() const { return (mFlags&RoutingChatFlag_IsWhisper)?true:false; }

	virtual RoutingOpType GetType() const { return RoutingOp_PeerChat; }
};

typedef SmartPtr<RoutingPeerChatOp> RoutingPeerChatOpPtr;

}; // namespace WONAPI


#endif
