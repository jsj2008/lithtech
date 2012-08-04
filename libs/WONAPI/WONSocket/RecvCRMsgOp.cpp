#include "RecvCRMsgOp.h"
using namespace WONAPI;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus RecvCRMsgOp::Start()
{
	mCurBytes.Reset();
	mMsg = NULL;
	return Continue();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus RecvCRMsgOp::Continue()
{
	char aByte;
	
	while(true)
	{
		mSocket->WaitForRead(TimeLeft());
		WONStatus aStatus = mSocket->RecvBytes(&aByte,1);
		if(aStatus==WS_Success)
		{
			mCurBytes.AppendByte(aByte);
			if(aByte=='\n')
			{
				if(mCurBytes.length()>=2 && *(mCurBytes.data()+mCurBytes.length()-2)=='\r')
				{
					char *aPtr = mCurBytes.data();
					aPtr[mCurBytes.length() - 2] = 0;
					mCurBytes.SetSize(mCurBytes.length()-1);
					mMsg = mCurBytes.ToByteBuffer();
					return WS_Success;
				}
			}
		}
		else if(aStatus!=WS_TimedOut)
			return aStatus;
		else if(TimeLeft()==0)
			return WS_TimedOut;
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


