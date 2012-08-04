#include "SendBytesOp.h"
using namespace WONAPI;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus SendBytesOp::Start()
{
	mBytes.Rewind();
	return Continue();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus SendBytesOp::Continue()
{
	while(mBytes.pos() < mBytes.length())
	{
		mSocket->WaitForWrite(TimeLeft());

		int aSentLen = 0;
		WONStatus aStatus = mSocket->SendBytes(mBytes.data() + mBytes.pos(), mBytes.length() - mBytes.pos(), &aSentLen); 
		if(aSentLen>0)
			mBytes.ReadBytes(aSentLen);

		if(aStatus!=WS_TimedOut)
			return aStatus;
		
		if(TimeLeft()==0)
			return WS_TimedOut;
	}
	

	return WS_Success;
}
