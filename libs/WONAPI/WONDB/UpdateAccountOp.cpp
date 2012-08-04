

#include "UpdateAccountOp.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void UpdateAccountOp::Init()
{
	mLengthFieldSize = 4;
	mProfileData = new ProfileData;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UpdateAccountOp::UpdateAccountOp(ServerContext *theProfileContext, AuthContext* theAuthContext)
	: ServerRequestOp(theProfileContext)
{
	SetAuthType(AUTH_TYPE_SESSION, theAuthContext);
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
UpdateAccountOp::UpdateAccountOp(const IPAddr &theAddr, AuthContext* theAuthContext)
	: ServerRequestOp(theAddr)
{
	SetAuthType(AUTH_TYPE_SESSION, theAuthContext);
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void UpdateAccountOp::Reset()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus UpdateAccountOp::GetNextRequest()
{
	WriteBuffer aBuf(mLengthFieldSize);

	aBuf.AppendByte(5);			// small message
	aBuf.AppendShort(14);		// DBProxy Service
	aBuf.AppendShort(1002);		// Message Type		: DBProxy
	aBuf.AppendShort(4);		// Sub Message Type : Update Account 

	aBuf.AppendWString(mPassword);
	aBuf.AppendString(mEmail);
	mProfileData->PackRequestData(aBuf, true);

	mRequest = aBuf.ToByteBuffer();

	return WS_ServerReq_Recv;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
WONStatus UpdateAccountOp::CheckResponse()
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

	if(aMessageType!=1002 || aSubMessageType!=5) // Update Account Reply
		return InvalidReplyHeader();

	//mProfileData->UnpackSetReply(aMsg); //no optional info yet
	return (WONStatus)aStatus;
}
