#ifndef __WON_ROUTINGACCEPTCLIENTOP_H__
#define __WON_ROUTINGACCEPTCLIENTOP_H__
#include "WONShared.h"
#include "RoutingOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingAcceptClientOp : public RoutingOp
{
private:
	unsigned short mGroupId;
	unsigned short mClientId;
	bool mAccepted;
	std::wstring mAcceptComment;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingAcceptClientOp(RoutingConnection *theConnection);

	void SetGroupId(unsigned short theId) { mGroupId = theId; }
	void SetClientId(unsigned short theId) { mClientId = theId; }
	void SetAcceptComment(const std::wstring &theComment) { mAcceptComment = theComment; }
	void SetAccepted(bool isAccepted) { mAccepted = isAccepted; }
	
	unsigned short GetGroupId() const { return mGroupId; }
	unsigned short GetClientId() const { return mClientId; }
	bool GetAccepted() const { return mAccepted; }
	const std::wstring& GetAcceptComment() const { return mAcceptComment; }

	virtual RoutingOpType GetType() const { return RoutingOp_AcceptClient; }
};


typedef SmartPtr<RoutingAcceptClientOp> RoutingAcceptClientOpPtr;


}; // namespace WONAPI



#endif
