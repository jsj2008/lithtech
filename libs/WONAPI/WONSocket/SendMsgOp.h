#ifndef __WON_SENDMSGOP_H__
#define __WON_SENDMSGOP_H__
#include "WONShared.h"
#include "SendBytesOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SendMsgOp : public SendBytesOp
{
protected:
	ByteBufferPtr mOriginalBytes;
	virtual WONStatus Start();
	virtual WONStatus Continue();

public:
	SendMsgOp(const ByteBuffer *theBytes, AsyncSocket *theSocket = NULL) : SendBytesOp(NULL, theSocket), mOriginalBytes(theBytes) {}
};

typedef SmartPtr<SendMsgOp> SendMsgOpPtr;


}; // namespace WONAPI

#endif
