
#include "GetAccountOp.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetAccountOp::Init()
{
	mLengthFieldSize = 4;
	mProfileData = new ProfileData;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetAccountOp::GetAccountOp(ServerContext *theProfileContext, AuthContext* theAuthContext)
	: ServerRequestOp(theProfileContext)
{
	SetAuthType(AUTH_TYPE_SESSION, theAuthContext);
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetAccountOp::GetAccountOp(const IPAddr &theAddr, AuthContext* theAuthContext)
	: ServerRequestOp(theAddr)
{
	SetAuthType(AUTH_TYPE_SESSION, theAuthContext);
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetAccountOp::Reset()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetAccountOp::GetNextRequest()
{
	WriteBuffer aBuf(mLengthFieldSize);

	aBuf.AppendByte(5);			// small message
	aBuf.AppendShort(14);		// DBProxy Service
	aBuf.AppendShort(1002);		// Message Type		: DBProxy
	aBuf.AppendShort(6);		// Sub Message Type : Get Account 

	mProfileData->PackRequestData(aBuf, false);
	mRequest = aBuf.ToByteBuffer();

	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus GetAccountOp::CheckResponse()
{
	ReadBuffer aMsg(mResponse->data(),mResponse->length());
	unsigned char aHeaderType = aMsg.ReadByte();
	unsigned short aServiceType = aMsg.ReadShort();
	unsigned short aMessageType = aMsg.ReadShort();
	unsigned short aSubMessageType = aMsg.ReadShort();
	WONStatus aStatus = (WONStatus)aMsg.ReadShort();
	aMsg.ReadString(mErrorString);
	unsigned char aSeq = aMsg.ReadByte();

	if(aHeaderType!=5 || aServiceType!=14)
		return InvalidReplyHeader();

	if(aStatus!=WS_Success)
		return aStatus;

	if(aMessageType!=1002 || aSubMessageType!=7) // Get Account Reply
		return InvalidReplyHeader();

	aMsg.ReadString(mEmail,1);

	mProfileData->UnpackGetData(aMsg); 

	return (WONStatus)aStatus;
}
