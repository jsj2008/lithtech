#ifndef __WON_ACCEPTOP_H__
#define __WON_ACCEPTOP_H__
#include "WONShared.h"
#include "SocketOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class AcceptOp : public SocketOp
{
private:
	AsyncSocketPtr mAcceptedSocket;

protected:
	virtual WONStatus Continue();

public:
	AcceptOp(AsyncSocket *theSocket = NULL) : SocketOp(theSocket) { mSocketEvent[SocketEvent_Read] = true; }
	AsyncSocket* GetAcceptedSocket() { return mAcceptedSocket; }

	void SetAcceptedSocket(AsyncSocket *theSocket) { mAcceptedSocket = theSocket; }

	virtual SocketOp* Duplicate() { return new AcceptOp(mSocket); }
};

typedef SmartPtr<AcceptOp> AcceptOpPtr;

}; // namespace WONAPI

#endif
