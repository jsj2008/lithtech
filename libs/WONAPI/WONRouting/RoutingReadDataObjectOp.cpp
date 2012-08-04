#include "RoutingReadDataObjectOp.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingReadDataObjectOp::RoutingReadDataObjectOp()
{
	mLinkId = 0;
	mFlags = 0;
	mDoSubscribe = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingReadDataObjectOp::RoutingReadDataObjectOp(RoutingConnection *theConnection) : RoutingOp(theConnection)
{
	mLinkId = 0;
	mFlags = 0;
	mDoSubscribe = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingReadDataObjectOp::SendRequest()
{
	mDataObjects.clear();

	InitSendMsg(mDoSubscribe?RoutingSubscribeDataObjectRequest:RoutingReadDataObjectRequest);
	mSendMsg.AppendShort(mLinkId);
	mSendMsg.AppendString(mDataType,1);
	mSendMsg.AppendByte(mFlags);
	SendMsg();
	AddOp();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingReadDataObjectOp::HandleReply(unsigned char theMsgType, ReadBuffer &theMsg)
{
	if(theMsgType!=RoutingReadDataObjectReply)
		return WS_RoutingOp_DontWantReply;

	WONStatus aStatus = (WONStatus)theMsg.ReadShort();
	if(aStatus==WS_Success)
	{
		ParseReplyExceptForStatus(theMsg);
	}

	return aStatus;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RoutingReadDataObjectOp::ParseReplyExceptForStatus(ReadBuffer &theMsg)
{
	unsigned short aNumObjects = theMsg.ReadShort();
	for(int i=0; i<aNumObjects; i++)
	{
		RoutingDataObjectPtr anObject = new RoutingDataObject;
		anObject->mLinkId = theMsg.ReadShort();
		theMsg.ReadString(anObject->mType,1);
		theMsg.ReadWString(anObject->mName,1);
		anObject->mData = theMsg.ReadBuf(2);
		mDataObjects.push_back(anObject);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool RoutingReadDataObjectOp::ParseReadDataObjectObj(const void *theData, unsigned long theDataLen)
{
	try
	{
		ReadBuffer aMsg(theData,theDataLen);
		ParseReplyExceptForStatus(aMsg);
		return true;
	}
	catch(ReadBufferException&)
	{
	}
	return false;
}