#include "PingOp.h"
#include "WONCommon/WriteBuffer.h"
#include "WONCommon/ReadBuffer.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
PingOp::PingOp(const IPAddr &theAddr, unsigned char theLengthFieldSize, bool extended) 
	: ServerRequestOp(theAddr), mExtended(extended) 
{
	mLengthFieldSize = theLengthFieldSize;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

WONStatus PingOp::GetNextRequest()
{
	WriteBuffer aBuf(mLengthFieldSize);
	aBuf.AppendByte(3);					// mini message
	aBuf.AppendByte(1);					// common service
	aBuf.AppendByte(5);					// ping message
	aBuf.AppendLong(GetTickCount());	// start tick
	aBuf.AppendBool(mExtended);			// extended ping flag
	mRequest = aBuf.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

WONStatus PingOp::CheckResponse()
{
	ReadBuffer aMsg(mResponse->data(),mResponse->length());
	unsigned char aHeaderType = aMsg.ReadByte();
	unsigned char aServiceType = aMsg.ReadByte();
	unsigned char aMessageType = aMsg.ReadByte();
	if(aHeaderType!=3 || aServiceType!=1 || aMessageType!=6)
		return InvalidReplyHeader();

	// Read startTick and determine lag
	mLag = GetTickCount() - aMsg.ReadLong();

	if(aMsg.HasMoreBytes())
	{
		aMsg.ReadString(mAppName);
		aMsg.ReadString(mLogicalName);
		mVersion = aMsg.ReadLong();
		aMsg.ReadString(mImage);
		mPID = aMsg.ReadLong();
		aMsg.ReadString(mPorts);
		mIsAuth = aMsg.ReadBool();
		if (aMsg.HasMoreBytes())
		{
			aMsg.ReadString(mRegData);
			aMsg.ReadString(mServData);
		}
	}

	return WS_Success;
}
	
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void PingOp::Reset()
{
	mAppName.erase();
	mLogicalName.erase();
	mVersion = 0;
	mImage.erase();
	mPID = 0;
	mPorts.erase();
	mIsAuth = false;
	mRegData.erase();
	mServData.erase();
}
