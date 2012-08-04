#ifndef __WON_CONNECTOP_H__
#define __WON_CONNECTOP_H__
#include "WONShared.h"
#include "SocketOp.h"

namespace WONAPI
{

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class ConnectOp : public SocketOp
{
private:
	IPAddr mAddr;

public:
	ConnectOp(const IPAddr &theAddr, AsyncSocket *theSocket = NULL) : SocketOp(theSocket), mAddr(theAddr) { mSocketEvent[SocketEvent_Write] = true; mSocketEvent[SocketEvent_Except] = true; }
	const IPAddr& GetAddr() { return mAddr; }

protected:
	virtual WONStatus Start();
	virtual WONStatus Continue();
};

typedef SmartPtr<ConnectOp> ConnectOpPtr;


}; // namespace WONAPI

#endif
