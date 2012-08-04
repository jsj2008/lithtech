#ifndef __WON_SENDBYTESTOOP_H__
#define __WON_SENDBYTESTOOP_H__
#include "WONShared.h"
#include "SocketOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class SendBytesToOp : public SocketOp
{
protected:
	ByteBufferPtr mBytes;
	IPAddr mAddr;

protected:
	virtual WONStatus Continue();

public:
	SendBytesToOp(const ByteBuffer *theBytes, const IPAddr &theAddr, AsyncSocket *theSocket = NULL) : SocketOp(theSocket), mBytes(theBytes), mAddr(theAddr) { mSocketEvent[SocketEvent_Write] = true; }
};

typedef SmartPtr<SendBytesToOp> SendBytesToOpPtr;

}; // namespace WONAPI

#endif
