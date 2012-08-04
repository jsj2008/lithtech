#include "RoutingGetGroupListReqOp.h"
#include "WONCommon/WriteBuffer.h"
#include "WONCommon/ReadBuffer.h"


using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
RoutingGetGroupListReqOp::RoutingGetGroupListReqOp(const IPAddr &theAddr, unsigned char theFlags) :
	ServerRequestOp(theAddr),
	mOp(new RoutingGetGroupListOp)
{
	mLengthFieldSize = 2;
	mOp->SetFlags(theFlags);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGetGroupListReqOp::GetNextRequest()
{
	WriteBuffer aReq(mLengthFieldSize);
	aReq.AppendByte(3); // Mini Header
	aReq.AppendByte(7); // RoutingServerG2 service
	aReq.AppendByte(RoutingGetGroupListRequest);  // GetGroupList
	aReq.AppendShort(mOp->GetFlags());
	mRequest = aReq.ToByteBuffer();
	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus RoutingGetGroupListReqOp::CheckResponse()
{
	ReadBuffer aMsg(mResponse->data(),mResponse->length());
	unsigned char aHeaderType = aMsg.ReadByte();
	unsigned char aServiceType = aMsg.ReadByte();
	unsigned char aMessageType = aMsg.ReadByte();

	if(aHeaderType!=3 || aServiceType!=7 || aMessageType!=RoutingGetGroupListReply)
		return InvalidReplyHeader();

	WONStatus aStatus = (WONStatus)aMsg.ReadShort();
	if(aStatus!=WS_Success)
		return aStatus;

	if(!mOp->ParseGroupListObj(aMsg.data() + aMsg.pos(), aMsg.Available()))
		return WS_MessageUnpackFailure;
	else
		return WS_Success;
}
