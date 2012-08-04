#ifndef __WON_ROUTINGSENDDATAOP_H__
#define __WON_ROUTINGSENDDATAOP_H__
#include "WONShared.h"
#include "RoutingOp.h"
#include <string>

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RoutingSendDataOp : public RoutingOp
{
private:
	unsigned char mFlags;
	ByteBufferPtr mData;
	RoutingRecipientList mRecipients;

	virtual void SendRequest();
	virtual WONStatus HandleReply(unsigned char theMsgType, ReadBuffer &theMsg);

public:
	RoutingSendDataOp(RoutingConnection *theConnection) : RoutingOp(theConnection), mFlags(0) {}
	void SetFlags(unsigned char theFlags) { mFlags = theFlags; }
	void SetData(const ByteBuffer* theData) { mData = theData; }
	void SetRecipients(const RoutingRecipientList &theRecipients) { mRecipients = theRecipients; }
	void ClearRecipients() { mRecipients.clear(); }
	void AddRecipient(unsigned short theClientOrGroupId) { mRecipients.push_back(theClientOrGroupId); }

	unsigned char GetFlags() const { return mFlags; }
	ByteBufferPtr GetData() const { return mData; }
	const RoutingRecipientList& GetRecipients() const { return mRecipients; }

	virtual RoutingOpType GetType() const { return RoutingOp_SendData; }
};

typedef SmartPtr<RoutingSendDataOp> RoutingSendDataOpPtr;

}; // namespace WONAPI


#endif
