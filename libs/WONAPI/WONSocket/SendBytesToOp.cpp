#include "SendBytesToOp.h"
using namespace WONAPI;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus SendBytesToOp::Continue()
{
	mSocket->WaitForWrite(TimeLeft());
	
	WONStatus aStatus;
	if(mSocket->IsConnected())
		aStatus = mSocket->SendBytes(mBytes->data(), mBytes->length());
	else
		aStatus = mSocket->SendBytesTo(mBytes->data(), mBytes->length(), mAddr);

	return aStatus;
}
