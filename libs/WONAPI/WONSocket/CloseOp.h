#ifndef __WON_CLOSEOP_H__
#define __WON_CLOSEOP_H__
#include "WONShared.h"
#include "SocketOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CloseOp : public SocketOp
{
protected:
	WONStatus Continue() { mSocket->Close(); return WS_Success; }
public:
	CloseOp(AsyncSocket *theSocket = NULL) : SocketOp(theSocket) {}
};

typedef SmartPtr<CloseOp> CloseOpPtr;


}; // namespace WONAPI

#endif
