#include "SendMsgOp.h"
#include "NetStats.h"
using namespace WONAPI;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus SendMsgOp::Start()
{
	mBytesBuf = mOriginalBytes;
	WONStatus aStatus = mSocket->RunSendMsgTransform(mBytesBuf);
	if(aStatus!=WS_Success)
		return aStatus;
	else
	{
		mBytes.SetData(mBytesBuf->data(),mBytesBuf->length());
		return Continue();
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus SendMsgOp::Continue()
{
	WONStatus aStatus = SendBytesOp::Continue();
	if(aStatus==WS_Success)
		NetStats::IncrementMessagesSent(1);

	return aStatus;
}
